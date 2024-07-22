/*
 * Copyright 2021-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*${header:start}*/
#include "board_platform.h"
#include "board.h"
#include "board_dcdc.h"
#include "clock_config.h"
#include "fwk_platform.h"
#include "fwk_debug.h"
#if defined(gBoardUseFro32k_d) && (gBoardUseFro32k_d > 0)
#include "fsl_ccm32k.h"
#endif

#if defined(BOARD_DBG_SWO_CORE_FUNNEL) && (BOARD_DBG_SWO_CORE_FUNNEL != 0)
#include "fwk_debug_swo.h"
#endif
/*${header:end}*/

/*${function:start}*/

/* -------------------------------------------------------------------------- */
/*                             Private prototypes                             */
/* -------------------------------------------------------------------------- */

#if !defined(FPGA_SUPPORT) || (FPGA_SUPPORT == 0)
static void BOARD_InitOsc32MHzConfig(void);
#endif
/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

void BOARD_InitHardware(void)
{
    BOARD_DBGCONFIGINIT(TRUE); // internal debug

#if !defined(FPGA_SUPPORT) || (FPGA_SUPPORT == 0)
    /* Set default value before XTAL start up but can be updated at run time through
       PLATFORM_Update32MhzTrimFromHwParam() from value stored in flash in hw param*/
    BOARD_InitOsc32MHzConfig();

#if defined(gAppHighSystemClockFrequency_d) && (gAppHighSystemClockFrequency_d > 0)
    /* Set Core frequency to 96Mhz , core voltage to 1.1v */
    BOARD_BootClockHSRUN();
#else
    /* Set Core frequency to 48Mhz , core voltage to 1.0v */
    BOARD_BootClockRUN();
#endif

#if defined(gBoardUseFro32k_d) && (gBoardUseFro32k_d > 0)
    /* Enable and select the fro32k as 32k clock source and disable osc32k
     * The NBU firmware embeds a module capable of trimming the fro32k to get it as close as possible from 32768Hz
     * frequency
     * */
    (void)PLATFORM_InitFro32K();
#else
    /* Initialize and start osc32k
     * Note: osc32k is not yet selected as 32k clock source, the switch is done by the radio when the oscillator is
     * ready. For this mechanism to work, the osc32k must be enabled before starting the radio core, then the radio core
     * will detect the osc32k is enabled and switch to it when it is ready.
     * The switch can still be done by the host processor by calling PLATFORM_SwitchToOsc32k if needed.
     * The fro32k MUST NOT be disabled now, this will be handled by the radio core or in PLATFORM_SwitchToOsc32k. */
    (void)PLATFORM_InitOsc32K();
#endif

    /* Initialize DCDC and apply optimized configuration */
    BOARD_InitDcdc();
#endif

#if defined(gDebugConsoleEnable_d) && (gDebugConsoleEnable_d == 1)
    BOARD_InitDebugConsole();
#endif

    /* Specific pins and clocks for serial, timers, ... are initialized in board_comp.c api
     * see BOARD_InitSerialManager() for example */
    BOARD_InitPins();

#if defined(BOARD_DBG_SWO_CORE_FUNNEL) && (BOARD_DBG_SWO_CORE_FUNNEL != 0)
    /* Configure SoC so that SWO can be controlled */
    DBG_InitSWO(BOARD_DBG_SWO_CORE_FUNNEL);
#endif
}

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

#if !defined(FPGA_SUPPORT) || (FPGA_SUPPORT == 0)
static void BOARD_InitOsc32MHzConfig(void)
{
    uint32_t rfmc_xo;
    /* Apply a trim value to the crystal oscillator */
    rfmc_xo = RFMC->XO_TEST;

#if defined(BOARD_32MHZ_XTAL_CDAC_VALUE)
    rfmc_xo &= ~(RFMC_XO_TEST_CDAC_MASK);
    rfmc_xo |= RFMC_XO_TEST_CDAC(BOARD_32MHZ_XTAL_CDAC_VALUE);
#endif

#if defined(BOARD_32MHZ_XTAL_ISEL_VALUE)
    rfmc_xo &= ~(RFMC_XO_TEST_ISEL_MASK);
    rfmc_xo |= RFMC_XO_TEST_ISEL(BOARD_32MHZ_XTAL_ISEL_VALUE);
#endif

    RFMC->XO_TEST = rfmc_xo;
    return;
}
/*${function:end}*/
#endif