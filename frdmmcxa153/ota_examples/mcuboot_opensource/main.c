/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_glikey.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "mflash_drv.h"
#include "boot.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define MBC_BLOCK_SIZE  (8 * 1024)
#define MBC_BLOCK_CNT   16
#define MBC_GLBAC_RW    0
#define MBC_GLBAC_RX    4
#define MBC_GLIKEY_IDX  15

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void configure_flash_access(const uint8_t *glbac);
void glikey_write_enable(uint32_t index);

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint8_t glbac_sbl[MBC_BLOCK_CNT] = {4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t glbac_app[MBC_BLOCK_CNT] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0};

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

       Sector 0  -  3;  Bootloader   ;  R/X
       Sector 4  -  9;  App Primary  ;  R/W before boot; R/X after boot
       Sector 10 - 15;  App Secondary;  R/W
       Sector 16     ;  Unused       ;  R/W
    */

#if 0
    /* For debugging MBC when needed */
    for (int i=0; i < 8; i++)
    {
        PRINTF("MBC0_MEMN_GLBAC%u %x\n", i, MBC0->MBC0_MEMN_GLBAC[i]);
    }

    PRINTF("MBC0_DOM_MEM0_BLK_CFG_W0 %x\n", MBC0->MBC_DOM0[0].MBC0_DOM_MEM0_BLK_CFG_W0);
    PRINTF("MBC0_DOM_MEM0_BLK_CFG_W1 %x\n", MBC0->MBC_DOM0[0].MBC0_DOM_MEM0_BLK_CFG_W1);
#endif

    glikey_write_enable(MBC_GLIKEY_IDX);
    configure_flash_access(glbac_sbl);

    (void)sbl_boot_main();

    return 0;
}

void configure_flash_access(const uint8_t *glbac)
{
    uint32_t val;
    uint32_t *cfg_w = (uint32_t *) &(MBC0->MBC_INDEX[0].MBC_DOM0_MEM0_BLK_CFG_W[0]);

    for (int i=0; i < 2; i++)
    {
        for (int block=0; block < 8; block++)
        {
            /* prepare */
            val = *cfg_w;
            val &= ~(0x7 << (block*4));
            val |= (*glbac & 0x7) << (block*4);

            /* set */
            *cfg_w = val;

            /* read back */
            val = (*cfg_w >> (block*4)) & 0x7;
            if (val != *glbac)
            {
                PRINTF("\n! Failed to set MBACSEL%u for block %u !\n", block, i*8 + block);
            }
            glbac++;
        }
        cfg_w++;
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

void SBL_DisablePeripherals(void)
{
    /* reconfigure flash access rights for app about to be booted, lock GLIKEY */
    configure_flash_access(glbac_app);
    GLIKEY_EndOperation(GLIKEY0);

    DbgConsole_Deinit();
}

status_t CRYPTO_InitHardware(void)
{
    return kStatus_Success;
}
