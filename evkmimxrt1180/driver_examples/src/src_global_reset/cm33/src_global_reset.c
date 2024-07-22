/*
 * Copyright 2019 - 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_soc_src.h"
#include "fsl_common.h"

#include "fsl_rtwdog.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_WDOG_BASE         RTWDOG1
#define DEMO_SRC_BASE          SRC_GENERAL_REG
#define DEMO_GLOBAL_RESET_FLAG kSRC_Wdog1ResetFlag

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_GlobalSystemResetInit(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

void APP_GlobalSystemResetInit(void)
{
    rtwdog_config_t config;

    /*
     * config.enableWdog32 = true;
     * config.clockSource = kWDOG32_ClockSource1;
     * config.prescaler = kWDOG32_ClockPrescalerDivide1;
     * config.testMode = kWDOG32_TestModeDisabled;
     * config.enableUpdate = true;
     * config.enableInterrupt = false;
     * config.enableWindowMode = false;
     * config.windowValue = 0U;
     * config.timeoutValue = 0xFFFFU;
     */
    RTWDOG_GetDefaultConfig(&config);
    config.timeoutValue = 0xFFFFU; /* Timeout value is (0xF + 1)/2 = 8 sec. */
    RTWDOG_Init(DEMO_WDOG_BASE, &config);
    while (1)
    {
    }
}


/*!
 * @brief Main function
 */
int main(void)
{
    uint8_t ch;
    uint32_t flags;

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Misc config to enable the watchdog */
    SRC_GENERAL_REG->SRMASK         = 0;
    BLK_CTRL_S_AONMIX->LP_HANDSHAKE = 0xFFFFFFFF;

    PRINTF("Example: SRC Global System Reset.\r\n");

#if defined(DEMO_GET_RESET_STATUS_FLAGS)
    flags = DEMO_GET_RESET_STATUS_FLAGS;
#else
    flags = SRC_GetResetStatusFlags(DEMO_SRC_BASE);
#endif /* DEMO_GET_RESET_STATUS_FLAGS */

    if ((flags & DEMO_GLOBAL_RESET_FLAG) == 0UL)
    {
        APP_GlobalSystemResetInit();
    }
    else
    {
        PRINTF("Global System Reset Occurred.\r\n");
        SRC_ClearGlobalSystemResetStatus(DEMO_SRC_BASE, DEMO_GLOBAL_RESET_FLAG);
    }
    while (1)
    {
        ch = GETCHAR();
        PUTCHAR(ch);
    }
}
