/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "pin_mux.h"
#include "board.h"
#include "fsl_otp.h"
#include "fsl_debug_console.h"

#include <stdbool.h>
#include "fsl_clock.h"
#include "fsl_reset.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief OTP example.
 *
 * @details Just prints version of driver in ROM API.
 */

int main(void)
{
    uint32_t version;

    /* Init hardware*/
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* enable clock to OTP and reset it */
    CLOCK_EnableClock(kCLOCK_Otp);
    RESET_PeripheralReset(kOTP_RST_SHIFT_RSTn);

    BOARD_InitBootPins();

    /* main clock needs to be set to 12 MHz in order to use OTP */
    BOARD_BootClockFRO12M();

    BOARD_InitDebugConsole();

    PRINTF("OTP Peripheral Driver Example\r\n\r\n");

    /* Get version of driver in ROM */
    version = OTP_GetDriverVersion();
    PRINTF("OTP ROM API driver version: 0x%X\r\n\r\n", version);

    while (1)
    {
    }
}
