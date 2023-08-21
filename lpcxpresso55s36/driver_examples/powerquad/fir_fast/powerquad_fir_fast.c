/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_powerquad.h"
#include "math.h"

#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_POWERQUAD POWERQUAD

#define EXAMPLE_ASSERT_TRUE(x)            \
    if (!(x))                             \
    {                                     \
        PRINTF("%s error\r\n", __func__); \
        while (1)                         \
        {                                 \
        }                                 \
    }

#define EXAMPLE_FIR_DATA_LEN     128
#define EXAMPLE_FIR_TAP_LEN      32
#define EXAMPLE_CONV_A_LEN       128
#define EXAMPLE_CONV_B_LEN       32
#define EXAMPLE_CONV_RESULT_LEN  (EXAMPLE_CONV_A_LEN + EXAMPLE_CONV_B_LEN - 1)
#define EXAMPLE_CORR_A_LEN       128
#define EXAMPLE_CORR_B_LEN       32
#define EXAMPLE_CORR_RESULT_LEN  (EXAMPLE_CORR_A_LEN + EXAMPLE_CORR_B_LEN - 1)
#define EXAMPLE_CALCULATION_LOOP 100000

/*
 * Note:
 * In this example, the matrix scale function is used to convert input data to
 * private RAM. The max col matrix scale function supports is 16. For easy, this
 * example only supports the length multiple of 16.
 */
#if ((EXAMPLE_FIR_TAP_LEN & 0xF) || (EXAMPLE_CONV_B_LEN & 0xF) || (EXAMPLE_CORR_B_LEN & 0xF))
#error Reconfigure the length to multiple of 16
#endif

/*
 * Power Quad driver uses the first 4K private RAM, the RAM starts from 0xE0001000
 * could be used for other purpose.
 */
#define EXAMPLE_PRIVATE_RAM ((void *)0xE0001000)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void PQ_FIRFixed16Example(void);
static void PQ_FIRFixed32Example(void);
static void PQ_FIRFloatExample(void);
static void PQ_ConvolutionFixed16Example(void);
static void PQ_ConvolutionFixed32Example(void);
static void PQ_ConvolutionFloatExample(void);
static void PQ_CorrelationFixed16Example(void);
static void PQ_CorrelationFixed32Example(void);
static void PQ_CorrelationFloatExample(void);

static void EXAMPLE_InitTime(void);
static uint32_t EXAMPLE_GetTime(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static volatile uint32_t s_timeMs;

static const float s_firInputData[EXAMPLE_FIR_DATA_LEN] = {
    -16.0, -15.0, -14.0, -13.0, -12.0, -11.0, -10.0, -9.0, -8.0, -7.0, -6.0, -5.0, -4.0, -3.0, -2.0, -1.0,
    0.0,   1.0,   2.0,   3.0,   4.0,   5.0,   6.0,   7.0,  8.0,  9.0,  10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
    -16.0, -15.0, -14.0, -13.0, -12.0, -11.0, -10.0, -9.0, -8.0, -7.0, -6.0, -5.0, -4.0, -3.0, -2.0, -1.0,
    0.0,   1.0,   2.0,   3.0,   4.0,   5.0,   6.0,   7.0,  8.0,  9.0,  10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
    -16.0, -15.0, -14.0, -13.0, -12.0, -11.0, -10.0, -9.0, -8.0, -7.0, -6.0, -5.0, -4.0, -3.0, -2.0, -1.0,
    0.0,   1.0,   2.0,   3.0,   4.0,   5.0,   6.0,   7.0,  8.0,  9.0,  10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
    -16.0, -15.0, -14.0, -13.0, -12.0, -11.0, -10.0, -9.0, -8.0, -7.0, -6.0, -5.0, -4.0, -3.0, -2.0, -1.0,
    0.0,   1.0,   2.0,   3.0,   4.0,   5.0,   6.0,   7.0,  8.0,  9.0,  10.0, 11.0, 12.0, 13.0, 14.0, 15};

static const float s_firTaps[EXAMPLE_FIR_TAP_LEN] = {
    -16.0, -15.0, -14.0, -13.0, -12.0, -11.0, -10.0, -9.0, -8.0, -7.0, -6.0, -5.0, -4.0, -3.0, -2.0, -1.0,
    0.0,   1.0,   2.0,   3.0,   4.0,   5.0,   6.0,   7.0,  8.0,  9.0,  10.0, 11.0, 12.0, 13.0, 14.0, 15.0};

static const float s_firOutputRef[EXAMPLE_FIR_DATA_LEN] = {
    256.0,   480.0,   673.0,   836.0,   970.0,   1076.0,  1155.0,  1208.0,  1236.0,  1240.0,  1221.0,  1180.0,  1118.0,
    1036.0,  935.0,   816.0,   680.0,   528.0,   361.0,   180.0,   -14.0,   -220.0,  -437.0,  -664.0,  -900.0,  -1144.0,
    -1395.0, -1652.0, -1914.0, -2180.0, -2449.0, -2720.0, -2224.0, -1760.0, -1328.0, -928.0,  -560.0,  -224.0,  80.0,
    352.0,   592.0,   800.0,   976.0,   1120.0,  1232.0,  1312.0,  1360.0,  1376.0,  1360.0,  1312.0,  1232.0,  1120.0,
    976.0,   800.0,   592.0,   352.0,   80.0,    -224.0,  -560.0,  -928.0,  -1328.0, -1760.0, -2224.0, -2720.0, -2224.0,
    -1760.0, -1328.0, -928.0,  -560.0,  -224.0,  80.0,    352.0,   592.0,   800.0,   976.0,   1120.0,  1232.0,  1312.0,
    1360.0,  1376.0,  1360.0,  1312.0,  1232.0,  1120.0,  976.0,   800.0,   592.0,   352.0,   80.0,    -224.0,  -560.0,
    -928.0,  -1328.0, -1760.0, -2224.0, -2720.0, -2224.0, -1760.0, -1328.0, -928.0,  -560.0,  -224.0,  80.0,    352.0,
    592.0,   800.0,   976.0,   1120.0,  1232.0,  1312.0,  1360.0,  1376.0,  1360.0,  1312.0,  1232.0,  1120.0,  976.0,
    800.0,   592.0,   352.0,   80.0,    -224.0,  -560.0,  -928.0,  -1328.0, -1760.0, -2224.0, -2720.0,
};

static const float s_convA[EXAMPLE_CONV_A_LEN] = {
    -16.0, -15.0, -14.0, -13.0, -12.0, -11.0, -10.0, -9.0, -8.0, -7.0, -6.0, -5.0, -4.0, -3.0, -2.0, -1.0,
    0.0,   1.0,   2.0,   3.0,   4.0,   5.0,   6.0,   7.0,  8.0,  9.0,  10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
    -16.0, -15.0, -14.0, -13.0, -12.0, -11.0, -10.0, -9.0, -8.0, -7.0, -6.0, -5.0, -4.0, -3.0, -2.0, -1.0,
    0.0,   1.0,   2.0,   3.0,   4.0,   5.0,   6.0,   7.0,  8.0,  9.0,  10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
    -16.0, -15.0, -14.0, -13.0, -12.0, -11.0, -10.0, -9.0, -8.0, -7.0, -6.0, -5.0, -4.0, -3.0, -2.0, -1.0,
    0.0,   1.0,   2.0,   3.0,   4.0,   5.0,   6.0,   7.0,  8.0,  9.0,  10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
    -16.0, -15.0, -14.0, -13.0, -12.0, -11.0, -10.0, -9.0, -8.0, -7.0, -6.0, -5.0, -4.0, -3.0, -2.0, -1.0,
    0.0,   1.0,   2.0,   3.0,   4.0,   5.0,   6.0,   7.0,  8.0,  9.0,  10.0, 11.0, 12.0, 13.0, 14.0, 15};

static const float s_convB[EXAMPLE_CONV_B_LEN] = {
    -16.0, -15.0, -14.0, -13.0, -12.0, -11.0, -10.0, -9.0, -8.0, -7.0, -6.0, -5.0, -4.0, -3.0, -2.0, -1.0,
    0.0,   1.0,   2.0,   3.0,   4.0,   5.0,   6.0,   7.0,  8.0,  9.0,  10.0, 11.0, 12.0, 13.0, 14.0, 15.0};

static const float s_convOutputRef[EXAMPLE_CONV_RESULT_LEN] = {
    256.0,   480.0,   673.0,   836.0,   970.0,   1076.0,  1155.0,  1208.0,  1236.0,  1240.0,  1221.0,  1180.0,  1118.0,
    1036.0,  935.0,   816.0,   680.0,   528.0,   361.0,   180.0,   -14.0,   -220.0,  -437.0,  -664.0,  -900.0,  -1144.0,
    -1395.0, -1652.0, -1914.0, -2180.0, -2449.0, -2720.0, -2224.0, -1760.0, -1328.0, -928.0,  -560.0,  -224.0,  80.0,
    352.0,   592.0,   800.0,   976.0,   1120.0,  1232.0,  1312.0,  1360.0,  1376.0,  1360.0,  1312.0,  1232.0,  1120.0,
    976.0,   800.0,   592.0,   352.0,   80.0,    -224.0,  -560.0,  -928.0,  -1328.0, -1760.0, -2224.0, -2720.0, -2224.0,
    -1760.0, -1328.0, -928.0,  -560.0,  -224.0,  80.0,    352.0,   592.0,   800.0,   976.0,   1120.0,  1232.0,  1312.0,
    1360.0,  1376.0,  1360.0,  1312.0,  1232.0,  1120.0,  976.0,   800.0,   592.0,   352.0,   80.0,    -224.0,  -560.0,
    -928.0,  -1328.0, -1760.0, -2224.0, -2720.0, -2224.0, -1760.0, -1328.0, -928.0,  -560.0,  -224.0,  80.0,    352.0,
    592.0,   800.0,   976.0,   1120.0,  1232.0,  1312.0,  1360.0,  1376.0,  1360.0,  1312.0,  1232.0,  1120.0,  976.0,
    800.0,   592.0,   352.0,   80.0,    -224.0,  -560.0,  -928.0,  -1328.0, -1760.0, -2224.0, -2720.0, -2480.0, -2240.0,
    -2001.0, -1764.0, -1530.0, -1300.0, -1075.0, -856.0,  -644.0,  -440.0,  -245.0,  -60.0,   114.0,   276.0,   425.0,
    560.0,   680.0,   784.0,   871.0,   940.0,   990.0,   1020.0,  1029.0,  1016.0,  980.0,   920.0,   835.0,   724.0,
    586.0,   420.0,   225.0,
};

static const float s_corrA[EXAMPLE_CORR_A_LEN] = {
    -16.0, -15.0, -14.0, -13.0, -12.0, -11.0, -10.0, -9.0, -8.0, -7.0, -6.0, -5.0, -4.0, -3.0, -2.0, -1.0,
    0.0,   1.0,   2.0,   3.0,   4.0,   5.0,   6.0,   7.0,  8.0,  9.0,  10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
    -16.0, -15.0, -14.0, -13.0, -12.0, -11.0, -10.0, -9.0, -8.0, -7.0, -6.0, -5.0, -4.0, -3.0, -2.0, -1.0,
    0.0,   1.0,   2.0,   3.0,   4.0,   5.0,   6.0,   7.0,  8.0,  9.0,  10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
    -16.0, -15.0, -14.0, -13.0, -12.0, -11.0, -10.0, -9.0, -8.0, -7.0, -6.0, -5.0, -4.0, -3.0, -2.0, -1.0,
    0.0,   1.0,   2.0,   3.0,   4.0,   5.0,   6.0,   7.0,  8.0,  9.0,  10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
    -16.0, -15.0, -14.0, -13.0, -12.0, -11.0, -10.0, -9.0, -8.0, -7.0, -6.0, -5.0, -4.0, -3.0, -2.0, -1.0,
    0.0,   1.0,   2.0,   3.0,   4.0,   5.0,   6.0,   7.0,  8.0,  9.0,  10.0, 11.0, 12.0, 13.0, 14.0, 15};

static const float s_corrB[EXAMPLE_CORR_B_LEN] = {
    -16.0, -15.0, -14.0, -13.0, -12.0, -11.0, -10.0, -9.0, -8.0, -7.0, -6.0, -5.0, -4.0, -3.0, -2.0, -1.0,
    0.0,   1.0,   2.0,   3.0,   4.0,   5.0,   6.0,   7.0,  8.0,  9.0,  10.0, 11.0, 12.0, 13.0, 14.0, 15.0};

static const float s_corrOutputRef[EXAMPLE_CORR_RESULT_LEN] = {
    -240.0,  -449.0,  -628.0,  -778.0,  -900.0,  -995.0,  -1064.0, -1108.0, -1128.0, -1125.0, -1100.0, -1054.0, -988.0,
    -903.0,  -800.0,  -680.0,  -544.0,  -393.0,  -228.0,  -50.0,   140.0,   341.0,   552.0,   772.0,   1000.0,  1235.0,
    1476.0,  1722.0,  1972.0,  2225.0,  2480.0,  2736.0,  2240.0,  1776.0,  1344.0,  944.0,   576.0,   240.0,   -64.0,
    -336.0,  -576.0,  -784.0,  -960.0,  -1104.0, -1216.0, -1296.0, -1344.0, -1360.0, -1344.0, -1296.0, -1216.0, -1104.0,
    -960.0,  -784.0,  -576.0,  -336.0,  -64.0,   240.0,   576.0,   944.0,   1344.0,  1776.0,  2240.0,  2736.0,  2240.0,
    1776.0,  1344.0,  944.0,   576.0,   240.0,   -64.0,   -336.0,  -576.0,  -784.0,  -960.0,  -1104.0, -1216.0, -1296.0,
    -1344.0, -1360.0, -1344.0, -1296.0, -1216.0, -1104.0, -960.0,  -784.0,  -576.0,  -336.0,  -64.0,   240.0,   576.0,
    944.0,   1344.0,  1776.0,  2240.0,  2736.0,  2240.0,  1776.0,  1344.0,  944.0,   576.0,   240.0,   -64.0,   -336.0,
    -576.0,  -784.0,  -960.0,  -1104.0, -1216.0, -1296.0, -1344.0, -1360.0, -1344.0, -1296.0, -1216.0, -1104.0, -960.0,
    -784.0,  -576.0,  -336.0,  -64.0,   240.0,   576.0,   944.0,   1344.0,  1776.0,  2240.0,  2736.0,  2480.0,  2225.0,
    1972.0,  1722.0,  1476.0,  1235.0,  1000.0,  772.0,   552.0,   341.0,   140.0,   -50.0,   -228.0,  -393.0,  -544.0,
    -680.0,  -800.0,  -903.0,  -988.0,  -1054.0, -1100.0, -1125.0, -1128.0, -1108.0, -1064.0, -995.0,  -900.0,  -778.0,
    -628.0,  -449.0,  -240.0,
};

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{
    /* Board pin, clock, debug console init */
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    PRINTF("POWERQUAD FIR fast example started\r\n");

    EXAMPLE_InitTime();

    PQ_Init(DEMO_POWERQUAD);

    PQ_FIRFixed16Example();
    PQ_FIRFixed32Example();
    PQ_FIRFloatExample();
    PQ_ConvolutionFixed16Example();
    PQ_ConvolutionFixed32Example();
    PQ_ConvolutionFloatExample();
    PQ_CorrelationFixed16Example();
    PQ_CorrelationFixed32Example();
    PQ_CorrelationFloatExample();

    PRINTF("POWERQUAD FIR fast example successed\r\n");

    while (1)
    {
    }
}

void SysTick_Handler(void)
{
    s_timeMs++;
}

static void EXAMPLE_InitTime(void)
{
    s_timeMs = 0;

    /* Configure to 1 ms. */
    SysTick_Config(SystemCoreClock / 1000);
}

static uint32_t EXAMPLE_GetTime(void)
{
    return s_timeMs;
}

static void PQ_FIRFixed16Example(void)
{
    int16_t input[EXAMPLE_FIR_DATA_LEN];
    int16_t tap[EXAMPLE_FIR_TAP_LEN];
    int16_t output[EXAMPLE_FIR_DATA_LEN];
    uint32_t i;
    uint32_t oldTime;
    pq_config_t pqConfig;

    /* Initialize the data. */
    for (i = 0; i < EXAMPLE_FIR_DATA_LEN; i++)
    {
        input[i] = (int16_t)s_firInputData[i];
    }
    for (i = 0; i < EXAMPLE_FIR_TAP_LEN; i++)
    {
        tap[i] = (int16_t)s_firTaps[i];
    }
    memset(output, 0, sizeof(output));

    /*
     * Normal method:
     * The input data and taps are all in system RAM, powerquad fetches the data
     * through the same bus.
     */
    pqConfig.inputAFormat   = kPQ_16Bit;
    pqConfig.inputAPrescale = 0;
    pqConfig.inputBFormat   = kPQ_16Bit;
    pqConfig.inputBPrescale = 0;
    pqConfig.outputFormat   = kPQ_16Bit;
    pqConfig.outputPrescale = 0;
    pqConfig.tmpFormat      = kPQ_Float;
    pqConfig.tmpPrescale    = 0;
    pqConfig.machineFormat  = kPQ_Float;
    pqConfig.tmpBase        = (uint32_t *)0xE0000000;

    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    oldTime = EXAMPLE_GetTime();
    for (i = 0; i < EXAMPLE_CALCULATION_LOOP; i++)
    {
        PQ_FIR(DEMO_POWERQUAD, input, EXAMPLE_FIR_DATA_LEN, tap, EXAMPLE_FIR_TAP_LEN, output, PQ_FIR_FIR);
        PQ_WaitDone(DEMO_POWERQUAD);
    }

    PRINTF("%s: %d ms\r\n", "PQ fir fixed 16-bit normal method", (int)(EXAMPLE_GetTime() - oldTime));

    for (i = 0; i < EXAMPLE_FIR_DATA_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs((int16_t)s_firOutputRef[i] - output[i]) <= 1);
    }

    /*
     * Fast method
     *
     * The taps is convert and saved to private RAM, thus the PQ could
     * fetch data through two path. The taps is converted to float format
     * and saved to private ram.
     */
    pqConfig.inputAFormat   = kPQ_16Bit;
    pqConfig.inputAPrescale = 0;
    pqConfig.inputBFormat   = kPQ_16Bit;
    pqConfig.inputBPrescale = 0;
    pqConfig.outputFormat   = kPQ_Float;
    pqConfig.outputPrescale = 0;
    pqConfig.tmpFormat      = kPQ_Float;
    pqConfig.tmpPrescale    = 0;
    pqConfig.machineFormat  = kPQ_Float;
    pqConfig.tmpBase        = (uint32_t *)0xE0000000;

    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    PQ_MatrixScale(DEMO_POWERQUAD, POWERQUAD_MAKE_MATRIX_LEN(16, EXAMPLE_FIR_TAP_LEN / 16, 0), 1.0, tap,
                   EXAMPLE_PRIVATE_RAM);
    PQ_WaitDone(POWERQUAD);

    memset(output, 0, sizeof(output));

    /* In the next calculation, data in private ram is used. */
    pqConfig.inputBFormat = kPQ_Float;
    pqConfig.outputFormat = kPQ_16Bit;
    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    oldTime = EXAMPLE_GetTime();
    for (i = 0; i < EXAMPLE_CALCULATION_LOOP; i++)
    {
        PQ_FIR(DEMO_POWERQUAD, input, EXAMPLE_FIR_DATA_LEN, EXAMPLE_PRIVATE_RAM, EXAMPLE_FIR_TAP_LEN, output,
               PQ_FIR_FIR);
        PQ_WaitDone(DEMO_POWERQUAD);
    }

    PRINTF("%s: %d ms\r\n", "PQ fir fixed 16-bit fast method", (int)(EXAMPLE_GetTime() - oldTime));

    for (i = 0; i < EXAMPLE_FIR_DATA_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs((int16_t)s_firOutputRef[i] - output[i]) <= 1);
    }
}

static void PQ_FIRFixed32Example(void)
{
    int32_t input[EXAMPLE_FIR_DATA_LEN];
    int32_t tap[EXAMPLE_FIR_TAP_LEN];
    int32_t output[EXAMPLE_FIR_DATA_LEN];
    uint32_t i;
    uint32_t oldTime;
    pq_config_t pqConfig;

    /* Initialize the data. */
    for (i = 0; i < EXAMPLE_FIR_DATA_LEN; i++)
    {
        input[i] = (int32_t)s_firInputData[i];
    }
    for (i = 0; i < EXAMPLE_FIR_TAP_LEN; i++)
    {
        tap[i] = (int32_t)s_firTaps[i];
    }
    memset(output, 0, sizeof(output));

    /*
     * Normal method:
     * The input data and taps are all in system RAM, powerquad fetches the data
     * through the same bus.
     */
    pqConfig.inputAFormat   = kPQ_32Bit;
    pqConfig.inputAPrescale = 0;
    pqConfig.inputBFormat   = kPQ_32Bit;
    pqConfig.inputBPrescale = 0;
    pqConfig.outputFormat   = kPQ_32Bit;
    pqConfig.outputPrescale = 0;
    pqConfig.tmpFormat      = kPQ_Float;
    pqConfig.tmpPrescale    = 0;
    pqConfig.machineFormat  = kPQ_Float;
    pqConfig.tmpBase        = (uint32_t *)0xE0000000;

    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    oldTime = EXAMPLE_GetTime();
    for (i = 0; i < EXAMPLE_CALCULATION_LOOP; i++)
    {
        PQ_FIR(DEMO_POWERQUAD, input, EXAMPLE_FIR_DATA_LEN, tap, EXAMPLE_FIR_TAP_LEN, output, PQ_FIR_FIR);
        PQ_WaitDone(DEMO_POWERQUAD);
    }

    PRINTF("%s: %d ms\r\n", "PQ fir fixed 32-bit normal method", (int)(EXAMPLE_GetTime() - oldTime));

    for (i = 0; i < EXAMPLE_FIR_DATA_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs((int32_t)s_firOutputRef[i] - output[i]) <= 1);
    }

    /*
     * Fast method
     *
     * The taps is convert and saved to private RAM, thus the PQ could
     * fetch data through two path. The taps is converted to float format
     * and saved to private ram.
     */
    pqConfig.inputAFormat   = kPQ_32Bit;
    pqConfig.inputAPrescale = 0;
    pqConfig.inputBFormat   = kPQ_32Bit;
    pqConfig.inputBPrescale = 0;
    pqConfig.outputFormat   = kPQ_Float;
    pqConfig.outputPrescale = 0;
    pqConfig.tmpFormat      = kPQ_Float;
    pqConfig.tmpPrescale    = 0;
    pqConfig.machineFormat  = kPQ_Float;
    pqConfig.tmpBase        = (uint32_t *)0xE0000000;

    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    PQ_MatrixScale(DEMO_POWERQUAD, POWERQUAD_MAKE_MATRIX_LEN(16, EXAMPLE_FIR_TAP_LEN / 16, 0), 1.0, tap,
                   EXAMPLE_PRIVATE_RAM);
    PQ_WaitDone(POWERQUAD);

    memset(output, 0, sizeof(output));

    /* In the next calculation, data in private ram is used. */
    pqConfig.inputBFormat = kPQ_Float;
    pqConfig.outputFormat = kPQ_32Bit;
    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    oldTime = EXAMPLE_GetTime();
    for (i = 0; i < EXAMPLE_CALCULATION_LOOP; i++)
    {
        PQ_FIR(DEMO_POWERQUAD, input, EXAMPLE_FIR_DATA_LEN, EXAMPLE_PRIVATE_RAM, EXAMPLE_FIR_TAP_LEN, output,
               PQ_FIR_FIR);
        PQ_WaitDone(DEMO_POWERQUAD);
    }

    PRINTF("%s: %d ms\r\n", "PQ fir fixed 32-bit fast method", (int)(EXAMPLE_GetTime() - oldTime));

    for (i = 0; i < EXAMPLE_FIR_DATA_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs((int32_t)s_firOutputRef[i] - output[i]) <= 1);
    }
}

/* Float FIR */
static void PQ_FIRFloatExample(void)
{
    float input[EXAMPLE_FIR_DATA_LEN];
    float tap[EXAMPLE_FIR_TAP_LEN];
    float output[EXAMPLE_FIR_DATA_LEN];
    uint32_t i;
    uint32_t oldTime;
    pq_config_t pqConfig;

    /* Initialize the data. */
    memcpy(input, s_firInputData, sizeof(input));
    memcpy(tap, s_firTaps, sizeof(tap));
    memset(output, 0, sizeof(output));

    /*
     * Normal method:
     * The input data A and B are all in system RAM, powerquad fetches the data
     * through the same bus.
     */
    pqConfig.inputAFormat   = kPQ_Float;
    pqConfig.inputAPrescale = 0;
    pqConfig.inputBFormat   = kPQ_Float;
    pqConfig.inputBPrescale = 0;
    pqConfig.outputFormat   = kPQ_Float;
    pqConfig.outputPrescale = 0;
    pqConfig.tmpFormat      = kPQ_Float;
    pqConfig.tmpPrescale    = 0;
    pqConfig.machineFormat  = kPQ_Float;
    pqConfig.tmpBase        = (uint32_t *)0xE0000000;

    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    oldTime = EXAMPLE_GetTime();
    for (i = 0; i < EXAMPLE_CALCULATION_LOOP; i++)
    {
        PQ_FIR(DEMO_POWERQUAD, input, EXAMPLE_FIR_DATA_LEN, tap, EXAMPLE_FIR_TAP_LEN, output, PQ_FIR_FIR);
        PQ_WaitDone(DEMO_POWERQUAD);
    }

    PRINTF("%s: %d ms\r\n", "PQ fir float normal method", (int)(EXAMPLE_GetTime() - oldTime));

    for (i = 0; i < EXAMPLE_FIR_DATA_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(s_firOutputRef[i] - output[i])) < 0.00001);
    }

    /*
     * Fast method
     *
     * The input data B is convert and saved to private RAM, thus the PQ could
     * fetch data through two path. The input data B is converted to float format
     * and saved to private ram.
     */
    pqConfig.inputAFormat   = kPQ_Float;
    pqConfig.inputAPrescale = 0;
    pqConfig.inputBFormat   = kPQ_Float;
    pqConfig.inputBPrescale = 0;
    pqConfig.outputFormat   = kPQ_Float;
    pqConfig.outputPrescale = 0;
    pqConfig.tmpFormat      = kPQ_Float;
    pqConfig.tmpPrescale    = 0;
    pqConfig.machineFormat  = kPQ_Float;
    pqConfig.tmpBase        = (uint32_t *)0xE0000000;

    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    PQ_MatrixScale(DEMO_POWERQUAD, POWERQUAD_MAKE_MATRIX_LEN(16, EXAMPLE_FIR_TAP_LEN / 16, 0), 1.0, tap,
                   EXAMPLE_PRIVATE_RAM);
    PQ_WaitDone(POWERQUAD);

    memset(output, 0, sizeof(output));

    /* In the next calculation, data in private ram is used. */
    pqConfig.inputBFormat = kPQ_Float;
    pqConfig.outputFormat = kPQ_Float;
    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    oldTime = EXAMPLE_GetTime();
    for (i = 0; i < EXAMPLE_CALCULATION_LOOP; i++)
    {
        PQ_FIR(DEMO_POWERQUAD, input, EXAMPLE_FIR_DATA_LEN, EXAMPLE_PRIVATE_RAM, EXAMPLE_FIR_TAP_LEN, output,
               PQ_FIR_FIR);
        PQ_WaitDone(DEMO_POWERQUAD);
    }

    PRINTF("%s: %d ms\r\n", "PQ fir float fast method", (int)(EXAMPLE_GetTime() - oldTime));

    for (i = 0; i < EXAMPLE_FIR_DATA_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(s_firOutputRef[i] - output[i])) < 0.00001);
    }
}

static void PQ_ConvolutionFixed16Example(void)
{
    int16_t A[EXAMPLE_CONV_A_LEN];
    int16_t B[EXAMPLE_CONV_B_LEN];
    int16_t output[EXAMPLE_CONV_RESULT_LEN];
    uint32_t i;
    uint32_t oldTime;
    pq_config_t pqConfig;

    /* Initialize the data. */
    for (i = 0; i < EXAMPLE_CONV_A_LEN; i++)
    {
        A[i] = (int16_t)s_convA[i];
    }
    for (i = 0; i < EXAMPLE_CONV_B_LEN; i++)
    {
        B[i] = (int16_t)s_convB[i];
    }

    memset(output, 0, sizeof(output));

    /*
     * Normal method:
     * The input data A and B are all in system RAM, powerquad fetches the data
     * through the same bus.
     */
    pqConfig.inputAFormat   = kPQ_16Bit;
    pqConfig.inputAPrescale = 0;
    pqConfig.inputBFormat   = kPQ_16Bit;
    pqConfig.inputBPrescale = 0;
    pqConfig.outputFormat   = kPQ_16Bit;
    pqConfig.outputPrescale = 0;
    pqConfig.tmpFormat      = kPQ_Float;
    pqConfig.tmpPrescale    = 0;
    pqConfig.machineFormat  = kPQ_Float;
    pqConfig.tmpBase        = (uint32_t *)0xE0000000;

    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    oldTime = EXAMPLE_GetTime();
    for (i = 0; i < EXAMPLE_CALCULATION_LOOP; i++)
    {
        PQ_FIR(DEMO_POWERQUAD, A, EXAMPLE_CONV_A_LEN, B, EXAMPLE_CONV_B_LEN, output, PQ_FIR_CONVOLUTION);
        PQ_WaitDone(DEMO_POWERQUAD);
    }

    PRINTF("%s: %d ms\r\n", "PQ conv fixed 16-bit normal method", (int)(EXAMPLE_GetTime() - oldTime));

    for (i = 0; i < EXAMPLE_CONV_RESULT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs((int16_t)s_convOutputRef[i] - output[i]) <= 1);
    }

    /*
     * Fast method
     *
     * The input data B is convert and saved to private RAM, thus the PQ could
     * fetch data through two path. The input data B is converted to float format
     * and saved to private ram.
     */
    pqConfig.inputAFormat   = kPQ_16Bit;
    pqConfig.inputAPrescale = 0;
    pqConfig.inputBFormat   = kPQ_16Bit;
    pqConfig.inputBPrescale = 0;
    pqConfig.outputFormat   = kPQ_Float;
    pqConfig.outputPrescale = 0;
    pqConfig.tmpFormat      = kPQ_Float;
    pqConfig.tmpPrescale    = 0;
    pqConfig.machineFormat  = kPQ_Float;
    pqConfig.tmpBase        = (uint32_t *)0xE0000000;

    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    PQ_MatrixScale(DEMO_POWERQUAD, POWERQUAD_MAKE_MATRIX_LEN(16, EXAMPLE_CONV_B_LEN / 16, 0), 1.0, B,
                   EXAMPLE_PRIVATE_RAM);
    PQ_WaitDone(POWERQUAD);

    memset(output, 0, sizeof(output));

    /* In the next calculation, data in private ram is used. */
    pqConfig.inputBFormat = kPQ_Float;
    pqConfig.outputFormat = kPQ_16Bit;
    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    oldTime = EXAMPLE_GetTime();
    for (i = 0; i < EXAMPLE_CALCULATION_LOOP; i++)
    {
        PQ_FIR(DEMO_POWERQUAD, A, EXAMPLE_CONV_A_LEN, EXAMPLE_PRIVATE_RAM, EXAMPLE_CONV_B_LEN, output,
               PQ_FIR_CONVOLUTION);
        PQ_WaitDone(DEMO_POWERQUAD);
    }

    PRINTF("%s: %d ms\r\n", "PQ conv fixed 16-bit fast method", (int)(EXAMPLE_GetTime() - oldTime));

    for (i = 0; i < EXAMPLE_CONV_RESULT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs((int16_t)s_convOutputRef[i] - output[i]) <= 1);
    }
}

static void PQ_ConvolutionFixed32Example(void)
{
    int32_t A[EXAMPLE_CONV_A_LEN];
    int32_t B[EXAMPLE_CONV_B_LEN];
    int32_t output[EXAMPLE_CONV_RESULT_LEN];
    uint32_t i;
    uint32_t oldTime;
    pq_config_t pqConfig;

    /* Initialize the data. */
    for (i = 0; i < EXAMPLE_CONV_A_LEN; i++)
    {
        A[i] = (int32_t)s_convA[i];
    }
    for (i = 0; i < EXAMPLE_CONV_B_LEN; i++)
    {
        B[i] = (int32_t)s_convB[i];
    }

    memset(output, 0, sizeof(output));

    /*
     * Normal method:
     * The input data A and B are all in system RAM, powerquad fetches the data
     * through the same bus.
     */
    pqConfig.inputAFormat   = kPQ_32Bit;
    pqConfig.inputAPrescale = 0;
    pqConfig.inputBFormat   = kPQ_32Bit;
    pqConfig.inputBPrescale = 0;
    pqConfig.outputFormat   = kPQ_32Bit;
    pqConfig.outputPrescale = 0;
    pqConfig.tmpFormat      = kPQ_Float;
    pqConfig.tmpPrescale    = 0;
    pqConfig.machineFormat  = kPQ_Float;
    pqConfig.tmpBase        = (uint32_t *)0xE0000000;

    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    oldTime = EXAMPLE_GetTime();
    for (i = 0; i < EXAMPLE_CALCULATION_LOOP; i++)
    {
        PQ_FIR(DEMO_POWERQUAD, A, EXAMPLE_CONV_A_LEN, B, EXAMPLE_CONV_B_LEN, output, PQ_FIR_CONVOLUTION);
        PQ_WaitDone(DEMO_POWERQUAD);
    }

    PRINTF("%s: %d ms\r\n", "PQ conv fixed 32-bit normal method", (int)(EXAMPLE_GetTime() - oldTime));

    for (i = 0; i < EXAMPLE_CONV_RESULT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs((int32_t)s_convOutputRef[i] - output[i]) <= 1);
    }

    /*
     * Fast method
     *
     * The input data B is convert and saved to private RAM, thus the PQ could
     * fetch data through two path. The input data B is converted to float format
     * and saved to private ram.
     */
    pqConfig.inputAFormat   = kPQ_32Bit;
    pqConfig.inputAPrescale = 0;
    pqConfig.inputBFormat   = kPQ_32Bit;
    pqConfig.inputBPrescale = 0;
    pqConfig.outputFormat   = kPQ_Float;
    pqConfig.outputPrescale = 0;
    pqConfig.tmpFormat      = kPQ_Float;
    pqConfig.tmpPrescale    = 0;
    pqConfig.machineFormat  = kPQ_Float;
    pqConfig.tmpBase        = (uint32_t *)0xE0000000;

    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    PQ_MatrixScale(DEMO_POWERQUAD, POWERQUAD_MAKE_MATRIX_LEN(16, EXAMPLE_CONV_B_LEN / 16, 0), 1.0, B,
                   EXAMPLE_PRIVATE_RAM);
    PQ_WaitDone(POWERQUAD);

    memset(output, 0, sizeof(output));

    /* In the next calculation, data in private ram is used. */
    pqConfig.inputBFormat = kPQ_Float;
    pqConfig.outputFormat = kPQ_32Bit;
    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    oldTime = EXAMPLE_GetTime();
    for (i = 0; i < EXAMPLE_CALCULATION_LOOP; i++)
    {
        PQ_FIR(DEMO_POWERQUAD, A, EXAMPLE_CONV_A_LEN, EXAMPLE_PRIVATE_RAM, EXAMPLE_CONV_B_LEN, output,
               PQ_FIR_CONVOLUTION);
        PQ_WaitDone(DEMO_POWERQUAD);
    }

    PRINTF("%s: %d ms\r\n", "PQ conv fixed 32-bit fast method", (int)(EXAMPLE_GetTime() - oldTime));

    for (i = 0; i < EXAMPLE_CONV_RESULT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs((int32_t)s_convOutputRef[i] - output[i]) <= 1);
    }
}

/* Float Convolution */
static void PQ_ConvolutionFloatExample(void)
{
    float A[EXAMPLE_CONV_A_LEN];
    float B[EXAMPLE_CONV_B_LEN];
    float output[EXAMPLE_CONV_RESULT_LEN];
    uint32_t i;
    uint32_t oldTime;
    pq_config_t pqConfig;

    /* Initialize the data. */
    memcpy(A, s_convA, sizeof(A));
    memcpy(B, s_convB, sizeof(B));
    memset(output, 0, sizeof(output));

    /*
     * Normal method:
     * The input data A and B are all in system RAM, powerquad fetches the data
     * through the same bus.
     */
    pqConfig.inputAFormat   = kPQ_Float;
    pqConfig.inputAPrescale = 0;
    pqConfig.inputBFormat   = kPQ_Float;
    pqConfig.inputBPrescale = 0;
    pqConfig.outputFormat   = kPQ_Float;
    pqConfig.outputPrescale = 0;
    pqConfig.tmpFormat      = kPQ_Float;
    pqConfig.tmpPrescale    = 0;
    pqConfig.machineFormat  = kPQ_Float;
    pqConfig.tmpBase        = (uint32_t *)0xE0000000;

    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    oldTime = EXAMPLE_GetTime();
    for (i = 0; i < EXAMPLE_CALCULATION_LOOP; i++)
    {
        PQ_FIR(DEMO_POWERQUAD, A, EXAMPLE_CONV_A_LEN, B, EXAMPLE_CONV_B_LEN, output, PQ_FIR_CONVOLUTION);
        PQ_WaitDone(DEMO_POWERQUAD);
    }

    PRINTF("%s: %d ms\r\n", "PQ conv float normal method", (int)(EXAMPLE_GetTime() - oldTime));

    for (i = 0; i < EXAMPLE_CONV_RESULT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(s_convOutputRef[i] - output[i])) < 0.00001);
    }

    /*
     * Fast method
     *
     * The input data B is convert and saved to private RAM, thus the PQ could
     * fetch data through two path. The input data B is converted to float format
     * and saved to private ram.
     */
    pqConfig.inputAFormat   = kPQ_Float;
    pqConfig.inputAPrescale = 0;
    pqConfig.inputBFormat   = kPQ_Float;
    pqConfig.inputBPrescale = 0;
    pqConfig.outputFormat   = kPQ_Float;
    pqConfig.outputPrescale = 0;
    pqConfig.tmpFormat      = kPQ_Float;
    pqConfig.tmpPrescale    = 0;
    pqConfig.machineFormat  = kPQ_Float;
    pqConfig.tmpBase        = (uint32_t *)0xE0000000;

    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    PQ_MatrixScale(DEMO_POWERQUAD, POWERQUAD_MAKE_MATRIX_LEN(16, EXAMPLE_CONV_B_LEN / 16, 0), 1.0, B,
                   EXAMPLE_PRIVATE_RAM);
    PQ_WaitDone(POWERQUAD);

    memset(output, 0, sizeof(output));

    /* In the next calculation, data in private ram is used. */
    pqConfig.inputBFormat = kPQ_Float;
    pqConfig.outputFormat = kPQ_Float;
    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    oldTime = EXAMPLE_GetTime();
    for (i = 0; i < EXAMPLE_CALCULATION_LOOP; i++)
    {
        PQ_FIR(DEMO_POWERQUAD, A, EXAMPLE_CONV_A_LEN, EXAMPLE_PRIVATE_RAM, EXAMPLE_CONV_B_LEN, output,
               PQ_FIR_CONVOLUTION);
        PQ_WaitDone(DEMO_POWERQUAD);
    }

    PRINTF("%s: %d ms\r\n", "PQ conv float fast method", (int)(EXAMPLE_GetTime() - oldTime));

    for (i = 0; i < EXAMPLE_CONV_RESULT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(s_convOutputRef[i] - output[i])) < 0.00001);
    }
}

static void PQ_CorrelationFixed16Example(void)
{
    int16_t A[EXAMPLE_CORR_A_LEN];
    int16_t B[EXAMPLE_CORR_B_LEN];
    int16_t output[EXAMPLE_CORR_RESULT_LEN];
    uint32_t i;
    uint32_t oldTime;
    pq_config_t pqConfig;

    /* Initialize the data. */
    for (i = 0; i < EXAMPLE_CORR_A_LEN; i++)
    {
        A[i] = (int16_t)s_corrA[i];
    }
    for (i = 0; i < EXAMPLE_CORR_B_LEN; i++)
    {
        B[i] = (int16_t)s_corrB[i];
    }

    memset(output, 0, sizeof(output));

    /*
     * Normal method:
     * The input data A and B are all in system RAM, powerquad fetches the data
     * through the same bus.
     */
    pqConfig.inputAFormat   = kPQ_16Bit;
    pqConfig.inputAPrescale = 0;
    pqConfig.inputBFormat   = kPQ_16Bit;
    pqConfig.inputBPrescale = 0;
    pqConfig.outputFormat   = kPQ_16Bit;
    pqConfig.outputPrescale = 0;
    pqConfig.tmpFormat      = kPQ_Float;
    pqConfig.tmpPrescale    = 0;
    pqConfig.machineFormat  = kPQ_Float;
    pqConfig.tmpBase        = (uint32_t *)0xE0000000;

    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    oldTime = EXAMPLE_GetTime();
    for (i = 0; i < EXAMPLE_CALCULATION_LOOP; i++)
    {
        PQ_FIR(DEMO_POWERQUAD, A, EXAMPLE_CORR_A_LEN, B, EXAMPLE_CORR_B_LEN, output, PQ_FIR_CORRELATION);
        PQ_WaitDone(DEMO_POWERQUAD);
    }

    PRINTF("%s: %d ms\r\n", "PQ corr fixed 16-bit normal method", (int)(EXAMPLE_GetTime() - oldTime));

    for (i = 0; i < EXAMPLE_CORR_RESULT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs((int16_t)s_corrOutputRef[i] - output[i]) <= 1);
    }

    /*
     * Fast method
     *
     * The input data B is convert and saved to private RAM, thus the PQ could
     * fetch data through two path. The input data B is converted to float format
     * and saved to private ram.
     */
    pqConfig.inputAFormat   = kPQ_16Bit;
    pqConfig.inputAPrescale = 0;
    pqConfig.inputBFormat   = kPQ_16Bit;
    pqConfig.inputBPrescale = 0;
    pqConfig.outputFormat   = kPQ_Float;
    pqConfig.outputPrescale = 0;
    pqConfig.tmpFormat      = kPQ_Float;
    pqConfig.tmpPrescale    = 0;
    pqConfig.machineFormat  = kPQ_Float;
    pqConfig.tmpBase        = (uint32_t *)0xE0000000;

    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    PQ_MatrixScale(DEMO_POWERQUAD, POWERQUAD_MAKE_MATRIX_LEN(16, EXAMPLE_CORR_B_LEN / 16, 0), 1.0, B,
                   EXAMPLE_PRIVATE_RAM);
    PQ_WaitDone(POWERQUAD);

    memset(output, 0, sizeof(output));

    /* In the next calculation, data in private ram is used. */
    pqConfig.inputBFormat = kPQ_Float;
    pqConfig.outputFormat = kPQ_16Bit;
    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    oldTime = EXAMPLE_GetTime();
    for (i = 0; i < EXAMPLE_CALCULATION_LOOP; i++)
    {
        PQ_FIR(DEMO_POWERQUAD, A, EXAMPLE_CORR_A_LEN, EXAMPLE_PRIVATE_RAM, EXAMPLE_CORR_B_LEN, output,
               PQ_FIR_CORRELATION);
        PQ_WaitDone(DEMO_POWERQUAD);
    }

    PRINTF("%s: %d ms\r\n", "PQ corr fixed 16-bit fast method", (int)(EXAMPLE_GetTime() - oldTime));

    for (i = 0; i < EXAMPLE_CORR_RESULT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs((int16_t)s_corrOutputRef[i] - output[i]) <= 1);
    }
}

static void PQ_CorrelationFixed32Example(void)
{
    int32_t A[EXAMPLE_CORR_A_LEN];
    int32_t B[EXAMPLE_CORR_B_LEN];
    int32_t output[EXAMPLE_CORR_RESULT_LEN];
    uint32_t i;
    uint32_t oldTime;
    pq_config_t pqConfig;

    /* Initialize the data. */
    for (i = 0; i < EXAMPLE_CORR_A_LEN; i++)
    {
        A[i] = (int32_t)s_corrA[i];
    }
    for (i = 0; i < EXAMPLE_CORR_B_LEN; i++)
    {
        B[i] = (int32_t)s_corrB[i];
    }

    memset(output, 0, sizeof(output));

    /*
     * Normal method:
     * The input data A and B are all in system RAM, powerquad fetches the data
     * through the same bus.
     */
    pqConfig.inputAFormat   = kPQ_32Bit;
    pqConfig.inputAPrescale = 0;
    pqConfig.inputBFormat   = kPQ_32Bit;
    pqConfig.inputBPrescale = 0;
    pqConfig.outputFormat   = kPQ_32Bit;
    pqConfig.outputPrescale = 0;
    pqConfig.tmpFormat      = kPQ_Float;
    pqConfig.tmpPrescale    = 0;
    pqConfig.machineFormat  = kPQ_Float;
    pqConfig.tmpBase        = (uint32_t *)0xE0000000;

    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    oldTime = EXAMPLE_GetTime();
    for (i = 0; i < EXAMPLE_CALCULATION_LOOP; i++)
    {
        PQ_FIR(DEMO_POWERQUAD, A, EXAMPLE_CORR_A_LEN, B, EXAMPLE_CORR_B_LEN, output, PQ_FIR_CORRELATION);
        PQ_WaitDone(DEMO_POWERQUAD);
    }

    PRINTF("%s: %d ms\r\n", "PQ corr fixed 32-bit normal method", (int)(EXAMPLE_GetTime() - oldTime));

    for (i = 0; i < EXAMPLE_CORR_RESULT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs((int32_t)s_corrOutputRef[i] - output[i]) <= 1);
    }

    /*
     * Fast method
     *
     * The input data B is convert and saved to private RAM, thus the PQ could
     * fetch data through two path. The input data B is converted to float format
     * and saved to private ram.
     */
    pqConfig.inputAFormat   = kPQ_32Bit;
    pqConfig.inputAPrescale = 0;
    pqConfig.inputBFormat   = kPQ_32Bit;
    pqConfig.inputBPrescale = 0;
    pqConfig.outputFormat   = kPQ_Float;
    pqConfig.outputPrescale = 0;
    pqConfig.tmpFormat      = kPQ_Float;
    pqConfig.tmpPrescale    = 0;
    pqConfig.machineFormat  = kPQ_Float;
    pqConfig.tmpBase        = (uint32_t *)0xE0000000;

    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    PQ_MatrixScale(DEMO_POWERQUAD, POWERQUAD_MAKE_MATRIX_LEN(16, EXAMPLE_CORR_B_LEN / 16, 0), 1.0, B,
                   EXAMPLE_PRIVATE_RAM);
    PQ_WaitDone(POWERQUAD);

    memset(output, 0, sizeof(output));

    /* In the next calculation, data in private ram is used. */
    pqConfig.inputBFormat = kPQ_Float;
    pqConfig.outputFormat = kPQ_32Bit;
    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    oldTime = EXAMPLE_GetTime();
    for (i = 0; i < EXAMPLE_CALCULATION_LOOP; i++)
    {
        PQ_FIR(DEMO_POWERQUAD, A, EXAMPLE_CORR_A_LEN, EXAMPLE_PRIVATE_RAM, EXAMPLE_CORR_B_LEN, output,
               PQ_FIR_CORRELATION);
        PQ_WaitDone(DEMO_POWERQUAD);
    }

    PRINTF("%s: %d ms\r\n", "PQ corr fixed 32-bit fast method", (int)(EXAMPLE_GetTime() - oldTime));

    for (i = 0; i < EXAMPLE_CORR_RESULT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs((int32_t)s_corrOutputRef[i] - output[i]) <= 1);
    }
}

/* Float Correlation */
static void PQ_CorrelationFloatExample(void)
{
    float A[EXAMPLE_CORR_A_LEN];
    float B[EXAMPLE_CORR_B_LEN];
    float output[EXAMPLE_CORR_RESULT_LEN];
    uint32_t i;
    uint32_t oldTime;
    pq_config_t pqConfig;

    /* Initialize the data. */
    memcpy(A, s_corrA, sizeof(A));
    memcpy(B, s_corrB, sizeof(B));
    memset(output, 0, sizeof(output));

    /*
     * Normal method:
     * The input data A and B are all in system RAM, powerquad fetches the data
     * through the same bus.
     */
    pqConfig.inputAFormat   = kPQ_Float;
    pqConfig.inputAPrescale = 0;
    pqConfig.inputBFormat   = kPQ_Float;
    pqConfig.inputBPrescale = 0;
    pqConfig.outputFormat   = kPQ_Float;
    pqConfig.outputPrescale = 0;
    pqConfig.tmpFormat      = kPQ_Float;
    pqConfig.tmpPrescale    = 0;
    pqConfig.machineFormat  = kPQ_Float;
    pqConfig.tmpBase        = (uint32_t *)0xE0000000;

    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    oldTime = EXAMPLE_GetTime();
    for (i = 0; i < EXAMPLE_CALCULATION_LOOP; i++)
    {
        PQ_FIR(DEMO_POWERQUAD, A, EXAMPLE_CORR_A_LEN, B, EXAMPLE_CORR_B_LEN, output, PQ_FIR_CORRELATION);
        PQ_WaitDone(DEMO_POWERQUAD);
    }

    PRINTF("%s: %d ms\r\n", "PQ corr float normal method", (int)(EXAMPLE_GetTime() - oldTime));

    for (i = 0; i < EXAMPLE_CORR_RESULT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(s_corrOutputRef[i] - output[i])) < 0.00001);
    }

    /*
     * Fast method
     *
     * The input data B is convert and saved to private RAM, thus the PQ could
     * fetch data through two path. The input data B is converted to float format
     * and saved to private ram.
     */
    pqConfig.inputAFormat   = kPQ_Float;
    pqConfig.inputAPrescale = 0;
    pqConfig.inputBFormat   = kPQ_Float;
    pqConfig.inputBPrescale = 0;
    pqConfig.outputFormat   = kPQ_Float;
    pqConfig.outputPrescale = 0;
    pqConfig.tmpFormat      = kPQ_Float;
    pqConfig.tmpPrescale    = 0;
    pqConfig.machineFormat  = kPQ_Float;
    pqConfig.tmpBase        = (uint32_t *)0xE0000000;

    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    PQ_MatrixScale(DEMO_POWERQUAD, POWERQUAD_MAKE_MATRIX_LEN(16, EXAMPLE_CORR_B_LEN / 16, 0), 1.0, B,
                   EXAMPLE_PRIVATE_RAM);
    PQ_WaitDone(POWERQUAD);

    memset(output, 0, sizeof(output));

    /* In the next calculation, data in private ram is used. */
    pqConfig.inputBFormat = kPQ_Float;
    pqConfig.outputFormat = kPQ_Float;
    PQ_SetConfig(DEMO_POWERQUAD, &pqConfig);

    oldTime = EXAMPLE_GetTime();
    for (i = 0; i < EXAMPLE_CALCULATION_LOOP; i++)
    {
        PQ_FIR(DEMO_POWERQUAD, A, EXAMPLE_CORR_A_LEN, EXAMPLE_PRIVATE_RAM, EXAMPLE_CORR_B_LEN, output,
               PQ_FIR_CORRELATION);
        PQ_WaitDone(DEMO_POWERQUAD);
    }

    PRINTF("%s: %d ms\r\n", "PQ corr float fast method", (int)(EXAMPLE_GetTime() - oldTime));

    for (i = 0; i < EXAMPLE_CORR_RESULT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(s_corrOutputRef[i] - output[i])) < 0.00001);
    }
}
