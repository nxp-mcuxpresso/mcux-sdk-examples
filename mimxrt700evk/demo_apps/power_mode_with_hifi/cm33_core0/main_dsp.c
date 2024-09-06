/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"

#include "NatureDSP_Signal.h"
#include "common.h"
#include "pin_mux.h"
#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef DEMO_POWER_PRINT_ENABLE
#define DEMO_POWER_PRINT_ENABLE 0U
#endif
#if DEMO_POWER_PRINT_ENABLE
#define DEMO_LOG PRINTF
#else
#define DEMO_LOG(...)
#endif

#define FFT_SIZE        1024
#define FIR_SAMPLE_SIZE 1024
#define FIR_FILTER_SIZE 32

static int32_t ALIGN(8) fft_in1[FFT_SIZE] = {0};
static int32_t ALIGN(8) fft_out1[FFT_SIZE];

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
#if defined(PMC0)
void DEMO_InitDebugConsole(void)
{
    uint32_t uartClkSrcFreq;

    CLOCK_AttachClk(kFRO0_DIV1_to_FCCLK0);
    CLOCK_SetClkDiv(kCLOCK_DivFcclk0Clk, 10U);

    /* Attach FC0 clock to LP_FLEXCOMM (debug console) */
    CLOCK_AttachClk(kFCCLK0_to_FLEXCOMM0);

    uartClkSrcFreq = BOARD_DEBUG_UART_CLK_FREQ;

    DbgConsole_Init(BOARD_DEBUG_UART_INSTANCE, BOARD_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, uartClkSrcFreq);
}

#define LOOP_FOR_PRINT 10000U
#else
#define LOOP_FOR_PRINT 2000U
#endif

int main(void)
{
    CLOCK_SetXtalFreq(BOARD_XTAL_SYS_CLK_HZ); /* Note: need tell clock driver the frequency of OSC. */
#if (DEMO_POWER_PRINT_ENABLE != 0U)
    BOARD_InitBootPins();
#if defined(PMC0)
    DEMO_InitDebugConsole();
#else
    BOARD_InitDebugConsole();
#endif
#endif
    DEMO_LOG("\r\nFFT test run on '%s'\r\n", XCHAL_CORE_ID);

    uint32_t loop = 0U;

    while (1)
    {
        fft_cplx32x32(fft_out1, fft_in1, cfft32_32, 3);
        fft_cplx32x32(fft_out1, fft_in1, cfft32_64, 3);
        fft_cplx32x32(fft_out1, fft_in1, cfft32_128, 3);
        fft_cplx32x32(fft_out1, fft_in1, cfft32_256, 3);
        fft_cplx32x32(fft_out1, fft_in1, cfft32_512, 3);

        loop++;
        if (loop >= 500U)
        {
            loop = 0U;
            DEMO_LOG(".");
        }
    }
}
