/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <sbl.h>
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if (defined(COMPONENT_MCU_ISP))
extern int isp_kboot_main(bool isInfiniteIsp);
#endif

#ifdef SOC_REMAP_ENABLE
#define REMAPADDRSTART  (FLEXSPI_BASE + 0x420)
#define REMAPADDREND    (FLEXSPI_BASE + 0x424)
#define REMAPADDROFFSET (FLEXSPI_BASE + 0x428)
#endif

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
#if (defined(COMPONENT_MCU_ISP))
    bool isInfiniteIsp = false;
    (void)isp_kboot_main(isInfiniteIsp);
#endif

    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Make sure casper ram buffer has power up */
    POWER_DisablePD(kPDRUNCFG_APD_CASPER_SRAM);
    POWER_DisablePD(kPDRUNCFG_PPD_CASPER_SRAM);
    POWER_ApplyPD();

    PRINTF("hello sbl.\r\n");

    (void)sbl_boot_main();

    return 0;
}

void SBL_DisablePeripherals(void)
{
}

#ifdef SOC_REMAP_ENABLE
void SBL_EnableRemap(uint32_t start_addr, uint32_t end_addr, uint32_t off)
{
    uint32_t *remap_start  = (uint32_t *)REMAPADDRSTART;
    uint32_t *remap_end    = (uint32_t *)REMAPADDREND;
    uint32_t *remap_offset = (uint32_t *)REMAPADDROFFSET;

    *remap_start  = start_addr + 1;
    *remap_end    = end_addr;
    *remap_offset = off;
}

void SBL_DisableRemap(void)
{
    uint32_t *remap_start  = (uint32_t *)REMAPADDRSTART;
    uint32_t *remap_end    = (uint32_t *)REMAPADDREND;
    uint32_t *remap_offset = (uint32_t *)REMAPADDROFFSET;

    *remap_start  = 0;
    *remap_end    = 0;
    *remap_offset = 0;
}
#endif
