/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_cache.h"
#include "fsl_trdc.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "boot.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define TRDC_INSTANCE TRDC2

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
#ifdef CONFIG_MCUBOOT_FLASH_REMAP_ENABLE

#error "Not Supported"

#endif

void trdc_setup(void)
{
    int result;

    CLOCK_EnableClock(kCLOCK_Trdc);
    TRDC_Init(TRDC_INSTANCE);

    /*
     * Check ELE FW status
     */

    do
    {
        /*Wait TR empty*/
        while ((MU_APPS_S3MUA->TSR & MU_TSR_TE0_MASK) == 0)
            ;
        /* Send Get FW Status command(0xc5), message size 0x01 */
        MU_APPS_S3MUA->TR[0] = 0x17c50106;
        /*Wait RR Full*/
        while ((MU_APPS_S3MUA->RSR & MU_RSR_RF0_MASK) == 0)
            ;
        (void)MU_APPS_S3MUA->RR[0];
        /*Wait RR Full*/
        while ((MU_APPS_S3MUA->RSR & MU_RSR_RF1_MASK) == 0)
            ;
        /* Get response code, only procedd when code is 0xD6 which is S400_SUCCESS_IND. */
        result = MU_APPS_S3MUA->RR[1];
        /*Wait RR Full*/
        while ((MU_APPS_S3MUA->RSR & MU_RSR_RF2_MASK) == 0)
            ;
        (void)MU_APPS_S3MUA->RR[2];
    } while (result != 0xD6);

    /*
     * Send Release TRDC command for TRDC2 instance
     */

    do
    {
        /*Wait TR empty*/
        while ((MU_APPS_S3MUA->TSR & MU_TSR_TE0_MASK) == 0)
            ;
        /* Send release RDC command(0xc4), message size 2 */
        MU_APPS_S3MUA->TR[0] = 0x17c40206;
        /*Wait TR empty*/
        while ((MU_APPS_S3MUA->TSR & MU_TSR_TE1_MASK) == 0)
            ;
        /* Release TRDC W(TRDC2, 0x78) to the RTD core(cm33, 0x1) */
        MU_APPS_S3MUA->TR[1] = 0x7801;
        /*Wait RR Full*/
        while ((MU_APPS_S3MUA->RSR & MU_RSR_RF0_MASK) == 0)
            ;
        (void)MU_APPS_S3MUA->RR[0];
        /*Wait RR Full*/
        while ((MU_APPS_S3MUA->RSR & MU_RSR_RF1_MASK) == 0)
            ;
        result = MU_APPS_S3MUA->RR[1];
    } while (result != 0xD6);

    /*
     * Prepare TRDC configuration
     */

    /* 1. Get the hardware configuration of the TRDC_INSTANCE module. */
    trdc_hardware_config_t hwConfig;
    TRDC_GetHardwareConfig(TRDC_INSTANCE, &hwConfig);
    /* 2. Set control policies for MRC and MBC access control configuration registers. */
    trdc_memory_access_control_config_t memAccessConfig;
    (void)memset(&memAccessConfig, 0, sizeof(memAccessConfig));
    /* 3. Enable all read/write/execute access for MRC/MBC access control. */
    memAccessConfig.nonsecureUsrX  = 1U;
    memAccessConfig.nonsecureUsrW  = 1U;
    memAccessConfig.nonsecureUsrR  = 1U;
    memAccessConfig.nonsecurePrivX = 1U;
    memAccessConfig.nonsecurePrivW = 1U;
    memAccessConfig.nonsecurePrivR = 1U;
    memAccessConfig.secureUsrX     = 1U;
    memAccessConfig.secureUsrW     = 1U;
    memAccessConfig.secureUsrR     = 1U;
    memAccessConfig.securePrivX    = 1U;
    memAccessConfig.securePrivW    = 1U;
    memAccessConfig.securePrivR    = 1U;
    for (int j = 0U; j < 8U; j++)
    {
        for (int i = 0U; i < hwConfig.mrcNumber; i++)
        {
            TRDC_MrcSetMemoryAccessConfig(TRDC_INSTANCE, &memAccessConfig, i, j);
        }
        for (int i = 0U; i < hwConfig.mbcNumber; i++)
        {
            TRDC_MbcSetMemoryAccessConfig(TRDC_INSTANCE, &memAccessConfig, i, j);
        }
    }

    /*
     * Set the configuration for MRC
     */

    trdc_mrc_region_descriptor_config_t mrcRegionConfig;
    (void)memset(&mrcRegionConfig, 0, sizeof(mrcRegionConfig));

    mrcRegionConfig.memoryAccessControlSelect =
        0U; /*Select whatever memory access control since all are enabled in the preview step*/
    mrcRegionConfig.valid     = true;
    mrcRegionConfig.nseEnable = false;      /* if the cm33 is secure then set to false, otherwise set to true. */
    mrcRegionConfig.mrcIdx    = 1;
    mrcRegionConfig.domainIdx = 2;          /* cm33 uses domain 2 by default */

#if 0    
    mrcRegionConfig.regionIdx = 0;
    mrcRegionConfig.startAddr = 0x47420000; /* Cover the FlexSPI-1 tx/rx fifo */
    mrcRegionConfig.endAddr   = 0x47440000;
    TRDC_MrcSetRegionDescriptorConfig(TRDC_INSTANCE, &mrcRegionConfig);
#endif

    mrcRegionConfig.regionIdx = 1;
    mrcRegionConfig.startAddr = 0x28000000; /* Cover the FlexSPI-1 memory */
    mrcRegionConfig.endAddr   = 0x29000000;
    TRDC_MrcSetRegionDescriptorConfig(TRDC_INSTANCE, &mrcRegionConfig);

    mrcRegionConfig.mrcIdx    = 2;
    mrcRegionConfig.regionIdx = 0;
    mrcRegionConfig.startAddr = 0x20000000; /* Cover the TCM with RAM data */
    mrcRegionConfig.endAddr   = 0x20020000;
    TRDC_MrcSetRegionDescriptorConfig(TRDC_INSTANCE, &mrcRegionConfig);

    mrcRegionConfig.mrcIdx    = 2;
    mrcRegionConfig.regionIdx = 1;
    mrcRegionConfig.startAddr = 0x0ffe0000; /* Cover the TCM with RAM code */
    mrcRegionConfig.endAddr   = 0x10000000;
    TRDC_MrcSetRegionDescriptorConfig(TRDC_INSTANCE, &mrcRegionConfig);
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Cache should be off until MbedTLS HW acceleration supports cache handling */
    XCACHE_DisableCache(XCACHE_PC);
    XCACHE_DisableCache(XCACHE_PS);

    PRINTF("hello sbl.\r\n");

#if defined(MCUBOOT_DIRECT_XIP) && defined(CONFIG_MCUBOOT_FLASH_REMAP_ENABLE)
    /* Make sure flash remapping function is disabled before running the
     * bootloader application .
     */
    PRINTF("Disabling flash remapping function\n");
    SBL_DisableRemap();
#endif

    trdc_setup();

    (void)sbl_boot_main();

    return 0;
}

void SBL_DisablePeripherals(void)
{
    XCACHE_DisableCache(XCACHE_PC);
    XCACHE_DisableCache(XCACHE_PS);
    ARM_MPU_Disable();
}
