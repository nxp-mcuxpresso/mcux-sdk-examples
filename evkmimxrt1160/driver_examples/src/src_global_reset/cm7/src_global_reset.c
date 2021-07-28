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
#include "fsl_wdog.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_WDOG_BASE         WDOG1
#define DEMO_SRC_BASE          SRC
#define DEMO_GLOBAL_RESET_FLAG kSRC_M4CoreWdogResetFlag
#if defined(ROM_ECC_ENABLED)
/* When ECC is enabled, SRC->SRSR need to be cleared since only correct SRSR value can trigger ROM ECC preload
 * procedure. In the start up stage the value of SRSR has been saved to SRC->GPR[10]. So that read SRC->GPR[10] to get
 * reset status.*/
#define DEMO_GET_RESET_STATUS_FLAGS (SRC->GPR[10])
#endif

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
    wdog_config_t config;

    /*
     * wdogConfig->enableWdog = true;
     * wdogConfig->workMode.enableWait = true;
     * wdogConfig->workMode.enableStop = false;
     * wdogConfig->workMode.enableDebug = false;
     * wdogConfig->enableInterrupt = false;
     * wdogConfig->enablePowerdown = false;
     * wdogConfig->resetExtension = flase;
     * wdogConfig->timeoutValue = 0xFFU;
     * wdogConfig->interruptTimeValue = 0x04u;
     */
    WDOG_GetDefaultConfig(&config);
    config.timeoutValue = 0xFU; /* Timeout value is (0xF + 1)/2 = 8 sec. */
    WDOG_Init(DEMO_WDOG_BASE, &config);
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
