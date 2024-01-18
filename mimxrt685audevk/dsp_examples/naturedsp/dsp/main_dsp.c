/*
 * Copyright 2019-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "srtm_naturedsp_test.h"

#include "NatureDSP_Signal.h"

#include <xtensa/config/core.h>
#include <xtensa/xos.h>
#include "board_hifi4.h"
#include "fsl_common.h"
#include "fsl_gpio.h"
#include "fsl_inputmux.h"
#include "pin_mux.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BOARD_XTAL_SYS_CLK_HZ 24000000U /*!< Board xtal_sys frequency in Hz */
#define BOARD_XTAL32K_CLK_HZ  32768U    /*!< Board xtal32K frequency in Hz */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
static void BOARD_InitClock(void)
{
    CLOCK_SetXtalFreq(BOARD_XTAL_SYS_CLK_HZ); /* sets external XTAL OSC freq */
}



int main(void)
{
    char version[30];

    BOARD_InitBootPins();
    BOARD_InitDebugConsole();
    BOARD_InitClock();

    PRINTF("\r\n[DSP Main] Running NatureDSP library on DSP core\r\n");

    NatureDSP_Signal_get_library_version(version);
    PRINTF("[DSP Main] NatureDSP library version: %s\r\n", version);

    NatureDSP_Signal_get_library_api_version(version);
    PRINTF("[DSP Main] NatureDSP library API version: %s\r\n\r\n", version);

    TEST_FFT();
    TEST_VEC_DOT();
    TEST_VEC_ADD();
    TEST_VEC_MAX();
    TEST_IIR();
    TEST_FIR_BLMS();

    while (1)
    {
    }
}
