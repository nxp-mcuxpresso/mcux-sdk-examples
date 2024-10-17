/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*${header:start}*/
#include "fsl_device_registers.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_power.h"
/*${header:end}*/

/*${function:start}*/
void BOARD_InitHardware(void)
{
    BOARD_InitBootPins();

    if (BOARD_IS_XIP())
    {
        BOARD_BootClockLPR();
        CLOCK_InitT3RefClk(kCLOCK_T3MciIrc48m);
        CLOCK_EnableClock(kCLOCK_T3PllMci256mClk);
        CLOCK_EnableClock(kCLOCK_Otp);
        CLOCK_EnableClock(kCLOCK_Els);
        CLOCK_EnableClock(kCLOCK_ElsApb);
        RESET_PeripheralReset(kOTP_RST_SHIFT_RSTn);
        RESET_PeripheralReset(kELS_APB_RST_SHIFT_RSTn);
    }
    else
    {
        BOARD_InitBootClocks();
    }

    // BOARD_InitDebugConsole();

    BOARD_InitSleepPinConfig();
    CLOCK_AttachClk(kRC32K_to_CLK32K);
    CLOCK_AttachClk(kCLK32K_to_OSTIMER_CLK);

    /* Reset GMDA */
    RESET_PeripheralReset(kGDMA_RST_SHIFT_RSTn);

    /* Power down CAU sleep clock here. */
    /* CPU2 uses external XTAL32K clock instead when in low power mode. */
    POWER_ConfigCauInSleep(true);
}
/*${function:end}*/
