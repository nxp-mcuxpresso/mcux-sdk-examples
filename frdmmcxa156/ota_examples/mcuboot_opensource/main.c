/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_glikey.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "mflash_drv.h"
#include "boot.h"

#include "fsl_clock.h"
#include "fsl_reset.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define MBC_BLOCK_SIZE  (16 * 1024)
#define MBC_BLOCK_CNT   64
#define MBC_GLBAC_RW    0
#define MBC_GLBAC_RX    4
#define MBC_GLIKEY_IDX  15

/*******************************************************************************
 * Types
 ******************************************************************************/

struct mbc_conf {
    uint32_t addr; /* MBC block aligned */
    uint32_t size; /* MBC block aligned */
    uint8_t  glbac;
};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void mbc_setup(const struct mbc_conf conf[], size_t cnt);
void glikey_write_enable(uint32_t index);
void mbc_print(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* MBC setup for Bootloader */

struct mbc_conf mbc_config_sbl[] =
{
    {.addr = 0x0,     .size = 0x8000,  .glbac = MBC_GLBAC_RX}, /* bootloader            R/X */
    {.addr = 0x8000,  .size = 0x7c000, .glbac = MBC_GLBAC_RW}, /* app primary           R/W */
    {.addr = 0x84000, .size = 0x7c000, .glbac = MBC_GLBAC_RW}, /* app secondary         R/W */
};

/* MBC setup for running app */

struct mbc_conf mbc_config_app[] =
{
    {.addr = 0x0,     .size = 0x8000,  .glbac = MBC_GLBAC_RX}, /* bootloader            R/X */
    {.addr = 0x8000,  .size = 0x78000, .glbac = MBC_GLBAC_RX}, /* app primary           R/X */
    {.addr = 0x80000, .size = 0x4000,  .glbac = MBC_GLBAC_RW}, /* app primary trailer   R/W */
    {.addr = 0x84000, .size = 0x7c000, .glbac = MBC_GLBAC_RW}, /* app secondary         R/W */
};

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */

int main(void)
{
    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("hello sbl.\r\n");

    /* IMPORTANT:

       MCX A series uses MBC IP to control flash access.
       By default the ROM sets all sectors as R/X (Read/Execute, GLBAC4). This can be changed
       in CMPA region to select different GLBACx, however if lock bit in GLBAC is used then
       it will not be possible to change the access control here.
       Bootloader needs to be granted R/W access when doing an image update.
       Right before jumping to the new image the access level needs to be switched back to R/X.
       Sectors used for secondary image need to have R/W access so running app can download
       new image.

       To summarize:

       Sector  0  -   3;  Bootloader   ;  R/X
       Sector  4  -  65;  App Primary  ;  R/W before boot; R/X after boot
       Sector 66  - 127;  App Secondary;  R/W

       In case of MCX A156 the MBC uses 16kB blocks.

       If SWAP algorithm is used then it's also needed to make last sector in primary slot R/W
       so running app is able to validate itself by modifying its trailer.
    */

#if 0
    /* for debugging MBC when needed */
    mbc_print();
#endif
    
    glikey_write_enable(MBC_GLIKEY_IDX);
    mbc_setup(mbc_config_sbl, ARRAY_SIZE(mbc_config_sbl));
    
#if 0
    mbc_print();
#endif

    (void)sbl_boot_main();

    return 0;
}

void mbc_setup(const struct mbc_conf conf[], size_t cnt)
{
    for (int c = 0; c < cnt; c++)
    {
        assert((conf[c].addr % MBC_BLOCK_SIZE) == 0);
        assert((conf[c].size % MBC_BLOCK_SIZE) == 0);
     
        uint32_t block_start = conf[c].addr / MBC_BLOCK_SIZE;
        uint32_t block_cnt   = conf[c].size / MBC_BLOCK_SIZE;
        
        for (int block = block_start; block < (block_start + block_cnt); block++)
        {
            /* 8 block per CFG_W */
            uint32_t wid = block / 8;
            uint32_t bid = block % 8;
            uint8_t glbac = conf[c].glbac;
            __IO uint32_t *cfg_w = &MBC0->MBC_INDEX[0].MBC_DOM0_MEM0_BLK_CFG_W[wid];
            uint32_t val;

            assert(wid < MBC_BLOCK_CNT);
            
            /* prepare */
            val = *cfg_w;
            val &= ~(0x7 << (bid*4));
            val |= (glbac & 0x7) << (bid*4);

            /* set */
            *cfg_w = val;

            /* read back */
            val = (*cfg_w >> (bid*4)) & 0x7;
            if (val != glbac)
            {
                PRINTF("\n! Failed to set MBACSEL%u for block %u !\n", wid, block);
            }
        }
    }
}

void glikey_write_enable(uint32_t index)
{
    status_t status;

    status = GLIKEY_IsLocked(GLIKEY0);
    if (status != kStatus_GLIKEY_NotLocked)
    {
        PRINTF("\n! GLIKEY locked !\n");
        return;
    }

    GLIKEY_SyncReset(GLIKEY0);
    GLIKEY_StartEnable(GLIKEY0, index);
    GLIKEY_ContinueEnable(GLIKEY0, GLIKEY_CODEWORD_STEP1);
    GLIKEY_ContinueEnable(GLIKEY0, GLIKEY_CODEWORD_STEP2);
    GLIKEY_ContinueEnable(GLIKEY0, GLIKEY_CODEWORD_STEP3);
    GLIKEY_ContinueEnable(GLIKEY0, GLIKEY_CODEWORD_STEP4);
    GLIKEY_ContinueEnable(GLIKEY0, GLIKEY_CODEWORD_STEP5);
    GLIKEY_ContinueEnable(GLIKEY0, GLIKEY_CODEWORD_STEP6);
    GLIKEY_ContinueEnable(GLIKEY0, GLIKEY_CODEWORD_STEP7);
    GLIKEY_ContinueEnable(GLIKEY0, GLIKEY_CODEWORD_STEP_EN);
}

void mbc_print(void)
{
    for (int i=0; i < 8; i++)
    {
        PRINTF("MBC0_MEMN_GLBAC%u %x\n", i, MBC0->MBC_INDEX[0].MBC_MEMN_GLBAC[i]);
    }
    
    for (int i=0; i < 8; i++)
    {
        PRINTF("MBC0_DOM_MEM0_BLK_CFG_W0 %x\n", MBC0->MBC_INDEX[0].MBC_DOM0_MEM0_BLK_CFG_W[i]);
    }
}

void SBL_DisablePeripherals(void)
{
    /* reconfigure flash access rights for app about to be booted, lock GLIKEY */
    mbc_setup(mbc_config_app, ARRAY_SIZE(mbc_config_app));
    GLIKEY_EndOperation(GLIKEY0);
    
    DbgConsole_Deinit();
}

status_t CRYPTO_InitHardware(void)
{
    return kStatus_Success;
}
