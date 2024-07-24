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
#include "fsl_power.h"
#include "boot.h"

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

void SBL_EnableRemap(uint32_t start_addr, uint32_t end_addr, uint32_t off)
{
      __DMB();
      *((volatile uint32_t*) FLASH_REMAP_END_REG) = end_addr;
      *((volatile uint32_t*) FLASH_REMAP_OFFSET_REG) = off;
      *((volatile uint32_t*) FLASH_REMAP_START_REG) = start_addr | 0x1;
      __DSB();
      __ISB();
}

void SBL_DisableRemap(void)
{
    __DMB();
    /* Disable REMAPEN bit first! */
    *((volatile uint32_t*) FLASH_REMAP_START_REG) = 0;
    *((volatile uint32_t*) FLASH_REMAP_END_REG) = 0;
    *((volatile uint32_t*) FLASH_REMAP_OFFSET_REG) = 0;
    __DSB();
    __ISB();
}
#endif /* CONFIG_MCUBOOT_FLASH_REMAP_ENABLE */

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Make sure casper ram buffer has power up */
    POWER_DisablePD(kPDRUNCFG_APD_CASPER_SRAM);
    POWER_DisablePD(kPDRUNCFG_PPD_CASPER_SRAM);
    POWER_ApplyPD();

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
}
