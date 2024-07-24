/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "boot.h"

#if defined(FSL_FEATURE_SOC_CAAM_COUNT) && (FSL_FEATURE_SOC_CAAM_COUNT > 0)
#include "fsl_caam.h"
#endif
#if defined(FSL_FEATURE_SOC_TRNG_COUNT) && (FSL_FEATURE_SOC_TRNG_COUNT > 0)
#include "fsl_trng.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
#ifdef CONFIG_MCUBOOT_FLASH_REMAP_ENABLE

#define IOMUXC_GPR_GPR30_REG 0x400CC420 /* Flash remapping start address  */
#define IOMUXC_GPR_GPR31_REG 0x400CC424 /* Flash remapping end address    */
#define IOMUXC_GPR_GPR32_REG 0x400CC428 /* Flash remapping offset address */

void SBL_EnableRemap(uint32_t start_addr, uint32_t end_addr, uint32_t off)
{
    uint32_t *remap_start  = (uint32_t *)IOMUXC_GPR_GPR30_REG;
    uint32_t *remap_end    = (uint32_t *)IOMUXC_GPR_GPR31_REG;
    uint32_t *remap_offset = (uint32_t *)IOMUXC_GPR_GPR32_REG;

    *remap_start  = start_addr | 0x1;
    *remap_end    = end_addr;
    *remap_offset = off;
}

void SBL_DisableRemap(void)
{
    uint32_t *remap_start  = (uint32_t *)IOMUXC_GPR_GPR30_REG;
    uint32_t *remap_end    = (uint32_t *)IOMUXC_GPR_GPR31_REG;
    uint32_t *remap_offset = (uint32_t *)IOMUXC_GPR_GPR32_REG;

    /* Disable offset first! */
    *remap_offset = 0;
    *remap_start  = 0;
    *remap_end    = 0;
}
#endif

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

    SCB_DisableDCache();

    PRINTF("hello sbl.\r\n");

#if defined(MCUBOOT_DIRECT_XIP) && defined(CONFIG_MCUBOOT_FLASH_REMAP_ENABLE)
    /* Make sure flash remapping function is disabled before running the
     * bootloader application .
     */
    PRINTF("Disabling flash remapping function\n");
    SBL_DisableRemap();
#endif

    (void)sbl_boot_main();

    return 0;
}

void SBL_DisablePeripherals(void)
{
    SCB_DisableDCache();
    SCB_DisableICache();
    ARM_MPU_Disable();

#if defined(COMPONENT_MCUBOOT_SECURE)
#if defined(FSL_FEATURE_SOC_CAAM_COUNT) && (FSL_FEATURE_SOC_CAAM_COUNT > 0) && defined(CRYPTO_USE_DRIVER_CAAM)
    CAAM_Deinit(CAAM);
#endif
#if defined(FSL_FEATURE_SOC_TRNG_COUNT) && (FSL_FEATURE_SOC_TRNG_COUNT > 0)
    TRNG_Deinit(TRNG);
#endif
#endif
}
