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

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    CLOCK_EnableClock(kCLOCK_Flexspi);
    BOARD_InitDebugConsole();

    /* Disable prefetch buffer and flash cache */    
    SYSCON->NVM_CTRL |= SYSCON_NVM_CTRL_DIS_MBECC_ERR_INST_MASK | SYSCON_NVM_CTRL_DIS_MBECC_ERR_DATA_MASK;
    SYSCON->NVM_CTRL |= SYSCON_NVM_CTRL_DIS_FLASH_SPEC_MASK | SYSCON_NVM_CTRL_DIS_DATA_SPEC_MASK;
    SYSCON->LPCAC_CTRL |= SYSCON_LPCAC_CTRL_DIS_LPCAC_MASK;
    
    PRINTF("hello sbl.\n");
    
    (void)sbl_boot_main();

    return 0;
}

void SBL_DisablePeripherals(void)
{
}

status_t CRYPTO_InitHardware(void)
{
    return kStatus_Success;
}

void SBL_EnableRemap(uint32_t start_addr, uint32_t end_addr, uint32_t off)
{
    (void) start_addr;
    (void) end_addr;
    (void) off;
    
    /* Remapping size set to full range of 1MB */

    NPX0->REMAP = (31 << NPX_REMAP_LIM_SHIFT)   | 0x5A5A;
    NPX0->REMAP = (31 << NPX_REMAP_LIMDP_SHIFT) | 0xA5A5;
}

void SBL_DisableRemap(void)
{
    NPX0->REMAP = 0x5A5A;
    NPX0->REMAP = 0xA5A5;
}