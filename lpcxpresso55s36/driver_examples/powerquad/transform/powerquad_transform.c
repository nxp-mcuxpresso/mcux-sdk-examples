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

#define FILTER_INPUT_LEN 32
#define MATH_PI          3.1415926535898
#define FLOAT_2_Q31(x)   ((int32_t)((x)*2147483648.0f))
#define FLOAT_2_Q15(x)   (int16_t) __SSAT(((int32_t)((x)*32768.0f)), 16)
#define EXAMPLE_ASSERT_TRUE(x)            \
    if (!(x))                             \
    {                                     \
        PRINTF("%s error\r\n", __func__); \
        while (1)                         \
        {                                 \
        }                                 \
    }

#if (FILTER_INPUT_LEN == 16)
#define FILTER_INPUTA_PRESCALER 4
#elif (FILTER_INPUT_LEN == 32)
#define FILTER_INPUTA_PRESCALER 5
#elif (FILTER_INPUT_LEN == 64)
#define FILTER_INPUTA_PRESCALER 6
#elif (FILTER_INPUT_LEN == 128)
#define FILTER_INPUTA_PRESCALER 7
#elif (FILTER_INPUT_LEN == 256)
#define FILTER_INPUTA_PRESCALER 8
#else
#define FILTER_INPUTA_PRESCALER 9
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void PQ_CFFTFixed16Example(void);
static void PQ_CFFTFixed32Example(void);
static void PQ_RFFTFixed16Example(void);
static void PQ_RFFTFixed32Example(void);
static void PQ_IFFTFixed16Example(void);
static void PQ_IFFTFixed32Example(void);
static void PQ_CDCTFixed16Example(void);
static void PQ_CDCTFixed32Example(void);
static void PQ_RDCTFixed16Example(void);
static void PQ_RDCTFixed32Example(void);
static void PQ_IDCTFixed16Example(void);
static void PQ_IDCTFixed32Example(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/


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

    PRINTF("POWERQUAD transform example started\r\n");

    PQ_Init(DEMO_POWERQUAD);

    PQ_CFFTFixed16Example();
    PQ_CFFTFixed32Example();
    PQ_RFFTFixed16Example();
    PQ_RFFTFixed32Example();
    PQ_IFFTFixed16Example();
    PQ_IFFTFixed32Example();
    PQ_CDCTFixed16Example();
    PQ_CDCTFixed32Example();
    PQ_RDCTFixed16Example();
    PQ_RDCTFixed32Example();
    PQ_IDCTFixed16Example();
    PQ_IDCTFixed32Example();

    PRINTF("POWERQUAD transform example successed\r\n");

    while (1)
    {
    }
}

static void PQ_RFFTFixed16Example(void)
{
    int N = FILTER_INPUT_LEN;
    int16_t inputData[FILTER_INPUT_LEN];
    int16_t rfftResult[FILTER_INPUT_LEN * 2];
    int16_t rfftRef[FILTER_INPUT_LEN * 2] = {
        100, 0,   76,  -50, 29,  -62, -1, -34,  10,   -3,   42,   -8, 51,   -47, 12, -84, -50, -70, -82, -4,  -46, 67,
        40,  82,  109, 17,  98,  -86, 5,  -147, -110, -113, -160, 0,  -110, 112, 5,  146, 98,  86,  109, -17, 40,  -83,
        -46, -68, -82, 4,   -50, 70,  12, 82,   51,   46,   42,   6,  10,   2,   -1, 33,  29,  61,  77,  49};

    for (int i = 0; i < 32; i++)
    {
        inputData[i] = 0;
    }

    inputData[0] = 10;
    inputData[1] = 30;
    inputData[2] = 10;
    inputData[3] = 30;
    inputData[4] = -50;
    inputData[5] = 70;

    pq_config_t pq_cfg;

    pq_cfg.inputAFormat   = kPQ_16Bit;
    pq_cfg.inputAPrescale = FILTER_INPUTA_PRESCALER;
    pq_cfg.inputBFormat   = kPQ_16Bit;
    pq_cfg.inputBPrescale = 0;
    pq_cfg.tmpFormat      = kPQ_16Bit;
    pq_cfg.tmpPrescale    = 0;
    pq_cfg.outputFormat   = kPQ_16Bit;
    pq_cfg.outputPrescale = 2;
    pq_cfg.tmpBase        = (uint32_t *)0xe0000000;
    pq_cfg.machineFormat  = kPQ_32Bit;
    PQ_SetConfig(DEMO_POWERQUAD, &pq_cfg);

    PQ_TransformRFFT(DEMO_POWERQUAD, N, inputData, rfftResult);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < N * 2; i++)
    {
        EXAMPLE_ASSERT_TRUE(rfftRef[i] == rfftResult[i]);
    }
}

static void PQ_RFFTFixed32Example(void)
{
    int N                               = FILTER_INPUT_LEN;
    int32_t inputData[FILTER_INPUT_LEN] = {0};
    int32_t rfftResult[FILTER_INPUT_LEN * 2];
    int32_t rfftRef[FILTER_INPUT_LEN * 2] = {
        100, 0,   76,  -50, 29,  -62, -1, -34,  10,   -3,   42,   -8, 51,   -47, 12, -84, -50, -70, -82, -4,  -46, 67,
        40,  82,  109, 17,  98,  -86, 5,  -147, -110, -113, -160, 0,  -110, 112, 5,  146, 98,  86,  109, -17, 40,  -83,
        -46, -68, -82, 4,   -50, 70,  12, 82,   51,   46,   42,   6,  10,   2,   -1, 33,  29,  61,  77,  49};

    inputData[0] = 10;
    inputData[1] = 30;
    inputData[2] = 10;
    inputData[3] = 30;
    inputData[4] = -50;
    inputData[5] = 70;

    pq_config_t pq_cfg;

    pq_cfg.inputAFormat   = kPQ_32Bit;
    pq_cfg.inputAPrescale = FILTER_INPUTA_PRESCALER;
    pq_cfg.inputBFormat   = kPQ_32Bit;
    pq_cfg.inputBPrescale = 0;
    pq_cfg.tmpFormat      = kPQ_32Bit;
    pq_cfg.tmpPrescale    = 0;
    pq_cfg.outputFormat   = kPQ_32Bit;
    pq_cfg.outputPrescale = 2;
    pq_cfg.tmpBase        = (uint32_t *)0xe0000000;
    pq_cfg.machineFormat  = kPQ_32Bit;
    PQ_SetConfig(DEMO_POWERQUAD, &pq_cfg);

    PQ_TransformRFFT(DEMO_POWERQUAD, N, inputData, rfftResult);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < N * 2; i++)
    {
        EXAMPLE_ASSERT_TRUE(rfftRef[i] == rfftResult[i]);
    }
}

static void PQ_CFFTFixed16Example(void)
{
    int N                                   = FILTER_INPUT_LEN;
    int16_t inputData[FILTER_INPUT_LEN * 2] = {0};
    int16_t cfftResult[FILTER_INPUT_LEN * 2];
    int16_t cfftRef[FILTER_INPUT_LEN * 2] = {
        100, 0,   76,  -50, 29,  -62, -1, -34,  10,   -3,   42,   -8, 51,   -47, 12, -84, -50, -70, -82, -4,  -46, 67,
        40,  82,  109, 17,  98,  -86, 5,  -147, -110, -113, -160, 0,  -110, 112, 5,  146, 98,  86,  109, -17, 40,  -83,
        -46, -68, -82, 4,   -50, 70,  12, 82,   51,   46,   42,   6,  10,   2,   -1, 33,  29,  61,  77,  49};

    inputData[0]  = 10;
    inputData[2]  = 30;
    inputData[4]  = 10;
    inputData[6]  = 30;
    inputData[8]  = -50;
    inputData[10] = 70;

    pq_config_t pq_cfg;

    pq_cfg.inputAFormat   = kPQ_16Bit;
    pq_cfg.inputAPrescale = FILTER_INPUTA_PRESCALER;
    pq_cfg.inputBFormat   = kPQ_16Bit;
    pq_cfg.inputBPrescale = 0;
    pq_cfg.tmpFormat      = kPQ_16Bit;
    pq_cfg.tmpPrescale    = 0;
    pq_cfg.outputFormat   = kPQ_16Bit;
    pq_cfg.outputPrescale = 0;
    pq_cfg.tmpBase        = (uint32_t *)0xe0000000;
    pq_cfg.machineFormat  = kPQ_32Bit;
    PQ_SetConfig(DEMO_POWERQUAD, &pq_cfg);

    PQ_TransformCFFT(DEMO_POWERQUAD, N, inputData, cfftResult);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < N * 2; i++)
    {
        EXAMPLE_ASSERT_TRUE(cfftRef[i] == cfftResult[i]);
    }
}

static void PQ_CFFTFixed32Example(void)
{
    int N                                   = FILTER_INPUT_LEN;
    int32_t inputData[FILTER_INPUT_LEN * 2] = {0};
    int32_t cfftResult[FILTER_INPUT_LEN * 2];
    int32_t cfftRef[FILTER_INPUT_LEN * 2] = {
        100, 0,   76,  -50, 29,  -62, -1, -34,  10,   -3,   42,   -8, 51,   -47, 12, -84, -50, -70, -82, -4,  -46, 67,
        40,  82,  109, 17,  98,  -86, 5,  -147, -110, -113, -160, 0,  -110, 112, 5,  146, 98,  86,  109, -17, 40,  -83,
        -46, -68, -82, 4,   -50, 70,  12, 82,   51,   46,   42,   6,  10,   2,   -1, 33,  29,  61,  77,  49};

    inputData[0]  = 10;
    inputData[2]  = 30;
    inputData[4]  = 10;
    inputData[6]  = 30;
    inputData[8]  = -50;
    inputData[10] = 70;

    pq_config_t pq_cfg;

    pq_cfg.inputAFormat   = kPQ_32Bit;
    pq_cfg.inputAPrescale = FILTER_INPUTA_PRESCALER;
    pq_cfg.inputBFormat   = kPQ_32Bit;
    pq_cfg.inputBPrescale = 0;
    pq_cfg.tmpFormat      = kPQ_32Bit;
    pq_cfg.tmpPrescale    = 0;
    pq_cfg.outputFormat   = kPQ_32Bit;
    pq_cfg.outputPrescale = 0;
    pq_cfg.tmpBase        = (uint32_t *)0xe0000000;
    pq_cfg.machineFormat  = kPQ_32Bit;
    PQ_SetConfig(DEMO_POWERQUAD, &pq_cfg);

    PQ_TransformCFFT(DEMO_POWERQUAD, N, inputData, cfftResult);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < N * 2; i++)
    {
        EXAMPLE_ASSERT_TRUE(cfftRef[i] == cfftResult[i]);
    }
}

static void PQ_IFFTFixed16Example(void)
{
    int N = FILTER_INPUT_LEN;
    int16_t ifftResult[FILTER_INPUT_LEN * 2];
    int16_t inputData[FILTER_INPUT_LEN * 2] = {
        100, 0,   76,  -50, 29,  -62, -1, -34,  10,   -3,   42,   -8, 51,   -47, 12, -84, -50, -70, -82, -4,  -46, 67,
        40,  82,  109, 17,  98,  -86, 5,  -147, -110, -113, -160, 0,  -110, 113, 5,  147, 98,  86,  109, -17, 40,  -83,
        -46, -67, -82, 4,   -50, 70,  12, 84,   51,   47,   42,   8,  10,   3,   -1, 34,  29,  62,  76,  50};
    int16_t ifftRef[FILTER_INPUT_LEN * 2] = {9, 0,  29, 0,  9, 0,  29, 0,  -51, -1, 69, -1, -1, 0,  -1, 0,
                                             0, -1, 0,  -1, 0, -1, 0,  -1, 0,   -1, 0,  -1, 0,  -1, 0,  -1,
                                             0, -1, 0,  -1, 0, -1, 0,  -1, 0,   -1, 0,  -1, 0,  -1, 0,  -1,
                                             0, -1, 0,  -1, 0, -1, 0,  0,  0,   -1, 0,  0,  -1, 0,  -1, 0};

    pq_config_t pq_cfg;

    pq_cfg.inputAFormat   = kPQ_16Bit;
    pq_cfg.inputAPrescale = 0;
    pq_cfg.inputBFormat   = kPQ_16Bit;
    pq_cfg.inputBPrescale = 0;
    pq_cfg.tmpFormat      = kPQ_16Bit;
    pq_cfg.tmpPrescale    = 0;
    pq_cfg.outputFormat   = kPQ_16Bit;
    pq_cfg.outputPrescale = 0;
    pq_cfg.tmpBase        = (uint32_t *)0xe0000000;
    pq_cfg.machineFormat  = kPQ_32Bit;
    PQ_SetConfig(DEMO_POWERQUAD, &pq_cfg);

    PQ_TransformIFFT(DEMO_POWERQUAD, N, inputData, ifftResult);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < N * 2; i++)
    {
        EXAMPLE_ASSERT_TRUE(ifftRef[i] == ifftResult[i]);
    }
}

static void PQ_IFFTFixed32Example(void)
{
    int N = FILTER_INPUT_LEN;
    int32_t ifftResult[FILTER_INPUT_LEN * 2];
    int32_t inputData[FILTER_INPUT_LEN * 2] = {
        100, 0,   76,  -50, 29,  -62, -1, -34,  10,   -3,   42,   -8, 51,   -47, 12, -84, -50, -70, -82, -4,  -46, 67,
        40,  82,  109, 17,  98,  -86, 5,  -147, -110, -113, -160, 0,  -110, 113, 5,  147, 98,  86,  109, -17, 40,  -83,
        -46, -67, -82, 4,   -50, 70,  12, 84,   51,   47,   42,   8,  10,   3,   -1, 34,  29,  62,  76,  50};
    int32_t ifftRef[FILTER_INPUT_LEN * 2] = {9, 0,  29, 0,  9, 0,  29, 0,  -51, -1, 69, -1, -1, 0,  -1, 0,
                                             0, -1, 0,  -1, 0, -1, 0,  -1, 0,   -1, 0,  -1, 0,  -1, 0,  -1,
                                             0, -1, 0,  -1, 0, -1, 0,  -1, 0,   -1, 0,  -1, 0,  -1, 0,  -1,
                                             0, -1, 0,  -1, 0, -1, 0,  0,  0,   -1, 0,  0,  -1, 0,  -1, 0};

    pq_config_t pq_cfg;

    pq_cfg.inputAFormat   = kPQ_32Bit;
    pq_cfg.inputAPrescale = 0;
    pq_cfg.inputBFormat   = kPQ_32Bit;
    pq_cfg.inputBPrescale = 0;
    pq_cfg.tmpFormat      = kPQ_32Bit;
    pq_cfg.tmpPrescale    = 0;
    pq_cfg.outputFormat   = kPQ_32Bit;
    pq_cfg.outputPrescale = 0;
    pq_cfg.tmpBase        = (uint32_t *)0xe0000000;
    pq_cfg.machineFormat  = kPQ_32Bit;
    PQ_SetConfig(DEMO_POWERQUAD, &pq_cfg);

    PQ_TransformIFFT(DEMO_POWERQUAD, N, inputData, ifftResult);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < N * 2; i++)
    {
        EXAMPLE_ASSERT_TRUE(ifftRef[i] == ifftResult[i]);
    }
}

static void PQ_CDCTFixed16Example(void)
{
    int N = FILTER_INPUT_LEN;
    int64_t acc0;
    int64_t acc1;
    int64_t acc2;
    const int32_t *twiddle_table            = dct32_twiddle;
    int16_t inputData[FILTER_INPUT_LEN * 2] = {4, 0, 3, 0, 5, 0, 10, 0, 0, 0, 0, 0, 0};
    int16_t dctResult[FILTER_INPUT_LEN * 2];
    /* Round down and approaching 0 compared with matlab*/
    int16_t dctRef[FILTER_INPUT_LEN * 2] = {3,  0, 5,  0, 4,  0, 3, 0, 2,  0, 1,  0, 0,  0, 0,  0, -1, 0, -2, 0, -2, 0,
                                            -2, 0, -2, 0, -1, 0, 0, 0, 0,  0, 1,  0, 1,  0, 2,  0, 2,  0, 2,  0, 1,  0,
                                            1,  0, 0,  0, 0,  0, 0, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, 0,  0};

    pq_config_t pq_cfg;

    pq_cfg.inputAFormat   = kPQ_16Bit;
    pq_cfg.inputAPrescale = FILTER_INPUTA_PRESCALER;
    pq_cfg.inputBFormat   = kPQ_16Bit;
    pq_cfg.inputBPrescale = 0;
    pq_cfg.tmpFormat      = kPQ_16Bit;
    pq_cfg.tmpPrescale    = 0;
    pq_cfg.outputFormat   = kPQ_16Bit;
    pq_cfg.outputPrescale = 0;
    pq_cfg.tmpBase        = (uint32_t *)0xe0000000;
    pq_cfg.machineFormat  = kPQ_32Bit;
    PQ_SetConfig(DEMO_POWERQUAD, &pq_cfg);

    PQ_TransformCDCT(DEMO_POWERQUAD, N, inputData, dctResult);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (int i = 0; i < N; i++)
    {
        acc0             = (int64_t)dctResult[i * 2] * twiddle_table[i * 2];             /* real * real */
        acc1             = (int64_t)dctResult[(i * 2) + 1] * twiddle_table[(i * 2) + 1]; /* imaginary * imaginary */
        acc2             = acc0 - acc1;
        dctResult[i * 2] = (uint32_t)(acc2 / (1024 * 1024 * 16));
        dctResult[(i * 2) + 1] = 0; /* zero out imaginary */
    }

    for (uint32_t i = 0; i < N * 2; i++)
    {
        EXAMPLE_ASSERT_TRUE(dctRef[i] == dctResult[i]);
    }
}

static void PQ_CDCTFixed32Example(void)
{
    int N = FILTER_INPUT_LEN;
    int64_t acc0;
    int64_t acc1;
    int64_t acc2;
    const int32_t *twiddle_table            = dct32_twiddle;
    int32_t inputData[FILTER_INPUT_LEN * 2] = {4, 0, 3, 0, 5, 0, 10, 0, 0, 0, 0, 0, 0};
    int32_t dctResult[FILTER_INPUT_LEN * 2];
    /* Round down and approaching 0 compared with matlab*/
    int32_t dctRef[FILTER_INPUT_LEN * 2] = {3,  0, 5,  0, 4,  0, 3, 0, 2,  0, 1,  0, 0,  0, 0,  0, -1, 0, -2, 0, -2, 0,
                                            -2, 0, -2, 0, -1, 0, 0, 0, 0,  0, 1,  0, 1,  0, 2,  0, 2,  0, 2,  0, 1,  0,
                                            1,  0, 0,  0, 0,  0, 0, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, 0,  0};

    pq_config_t pq_cfg;

    pq_cfg.inputAFormat   = kPQ_32Bit;
    pq_cfg.inputAPrescale = FILTER_INPUTA_PRESCALER;
    pq_cfg.inputBFormat   = kPQ_32Bit;
    pq_cfg.inputBPrescale = 0;
    pq_cfg.tmpFormat      = kPQ_32Bit;
    pq_cfg.tmpPrescale    = 0;
    pq_cfg.outputFormat   = kPQ_32Bit;
    pq_cfg.outputPrescale = 0;
    pq_cfg.tmpBase        = (uint32_t *)0xe0000000;
    pq_cfg.machineFormat  = kPQ_32Bit;
    PQ_SetConfig(DEMO_POWERQUAD, &pq_cfg);

    PQ_TransformCDCT(DEMO_POWERQUAD, N, inputData, dctResult);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (int i = 0; i < N; i++)
    {
        acc0             = (int64_t)dctResult[i * 2] * twiddle_table[i * 2];             /* real * real */
        acc1             = (int64_t)dctResult[(i * 2) + 1] * twiddle_table[(i * 2) + 1]; /* imaginary * imaginary */
        acc2             = acc0 - acc1;
        dctResult[i * 2] = (uint32_t)(acc2 / (1024 * 1024 * 16));
        dctResult[(i * 2) + 1] = 0; /* zero out imaginary */
    }

    for (uint32_t i = 0; i < 2 * N; i++)
    {
        EXAMPLE_ASSERT_TRUE(dctRef[i] == dctResult[i]);
    }
}

static void PQ_RDCTFixed16Example(void)
{
    int N = FILTER_INPUT_LEN;
    int64_t acc0;
    int64_t acc1;
    int64_t acc2;
    const int32_t *twiddle_table        = dct32_twiddle;
    int16_t inputData[FILTER_INPUT_LEN] = {4, 3, 5, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int16_t dctResult[FILTER_INPUT_LEN * 2];
    /* Round down and approaching 0 compared with matlab*/
    int16_t dctRef[FILTER_INPUT_LEN * 2] = {3,  0, 5,  0, 4,  0, 3, 0, 2,  0, 1,  0, 0,  0, 0,  0, -1, 0, -2, 0, -2, 0,
                                            -2, 0, -2, 0, -1, 0, 0, 0, 0,  0, 1,  0, 1,  0, 2,  0, 2,  0, 2,  0, 1,  0,
                                            1,  0, 0,  0, 0,  0, 0, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, 0,  0};

    pq_config_t pq_cfg;

    pq_cfg.inputAFormat   = kPQ_16Bit;
    pq_cfg.inputAPrescale = FILTER_INPUTA_PRESCALER;
    pq_cfg.inputBFormat   = kPQ_16Bit;
    pq_cfg.inputBPrescale = 0;
    pq_cfg.tmpFormat      = kPQ_16Bit;
    pq_cfg.tmpPrescale    = 0;
    pq_cfg.outputFormat   = kPQ_16Bit;
    pq_cfg.outputPrescale = 0;
    pq_cfg.tmpBase        = (uint32_t *)0xe0000000;
    pq_cfg.machineFormat  = kPQ_32Bit;
    PQ_SetConfig(DEMO_POWERQUAD, &pq_cfg);

    PQ_TransformRDCT(DEMO_POWERQUAD, N, inputData, dctResult);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (int i = 0; i < N; i++)
    {
        acc0             = (int64_t)dctResult[i * 2] * twiddle_table[i * 2];             /* real * real */
        acc1             = (int64_t)dctResult[(i * 2) + 1] * twiddle_table[(i * 2) + 1]; /* imaginary * imaginary */
        acc2             = acc0 - acc1;
        dctResult[i * 2] = (uint32_t)(acc2 / (1024 * 1024 * 16));
        dctResult[(i * 2) + 1] = 0; /* zero out imaginary */
    }

    for (uint32_t i = 0; i < 2 * N; i++)
    {
        EXAMPLE_ASSERT_TRUE(dctRef[i] == dctResult[i]);
    }
}

static void PQ_RDCTFixed32Example(void)
{
    int N = FILTER_INPUT_LEN;
    int64_t acc0;
    int64_t acc1;
    int64_t acc2;
    const int32_t *twiddle_table        = dct32_twiddle;
    int32_t inputData[FILTER_INPUT_LEN] = {4, 3, 5, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int32_t dctResult[FILTER_INPUT_LEN * 2];
    /* Round down and approaching 0 compared with matlab*/
    int32_t dctRef[FILTER_INPUT_LEN * 2] = {3,  0, 5,  0, 4,  0, 3, 0, 2,  0, 1,  0, 0,  0, 0,  0, -1, 0, -2, 0, -2, 0,
                                            -2, 0, -2, 0, -1, 0, 0, 0, 0,  0, 1,  0, 1,  0, 2,  0, 2,  0, 2,  0, 1,  0,
                                            1,  0, 0,  0, 0,  0, 0, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, 0,  0};

    pq_config_t pq_cfg;

    pq_cfg.inputAFormat   = kPQ_32Bit;
    pq_cfg.inputAPrescale = FILTER_INPUTA_PRESCALER;
    pq_cfg.inputBFormat   = kPQ_32Bit;
    pq_cfg.inputBPrescale = 0;
    pq_cfg.tmpFormat      = kPQ_32Bit;
    pq_cfg.tmpPrescale    = 0;
    pq_cfg.outputFormat   = kPQ_32Bit;
    pq_cfg.outputPrescale = 0;
    pq_cfg.tmpBase        = (uint32_t *)0xe0000000;
    pq_cfg.machineFormat  = kPQ_32Bit;
    PQ_SetConfig(DEMO_POWERQUAD, &pq_cfg);

    PQ_TransformRDCT(DEMO_POWERQUAD, N, inputData, dctResult);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (int i = 0; i < N; i++)
    {
        acc0             = (int64_t)dctResult[i * 2] * twiddle_table[i * 2];             /* real * real */
        acc1             = (int64_t)dctResult[(i * 2) + 1] * twiddle_table[(i * 2) + 1]; /* imaginary * imaginary */
        acc2             = acc0 - acc1;
        dctResult[i * 2] = (uint32_t)(acc2 / (1024 * 1024 * 16));
        dctResult[(i * 2) + 1] = 0; /* zero out imaginary */
    }

    for (uint32_t i = 0; i < 2 * N; i++)
    {
        EXAMPLE_ASSERT_TRUE(dctRef[i] == dctResult[i]);
    }
}

static void PQ_IDCTFixed16Example(void)
{
    int N = FILTER_INPUT_LEN;
    int64_t acc0;
    int64_t acc1;
    int64_t acc2;
    int64_t tmp_re, tmp_im;
    const int32_t *twiddle_table = idct32_twiddle;
    int32_t tmp[FILTER_INPUT_LEN * 2];
    int32_t inputData[FILTER_INPUT_LEN * 2] = {
        3, 0, 5, 0, 4, 0, 3, 0, 2, 0, 1, 0, 0, 0, 0, 0, -1, 0, -2, 0, -2, 0, -2, 0, -2, 0, -1, 0, 0,  0, 0, 0,
        1, 0, 1, 0, 2, 0, 2, 0, 2, 0, 1, 0, 1, 0, 0, 0, 0,  0, 0,  0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, 0, 0};
    int32_t idctResult[FILTER_INPUT_LEN * 2];
    int32_t idctRef[FILTER_INPUT_LEN * 2] = {3,  1,  2,  0,  3,  0,  7,  0,  -1, 0,  0,  -1, -1, 0,  0,  0,
                                             -1, -1, 0,  0,  -1, -1, -1, -1, -1, -1, 0,  -1, -1, 0,  -1, -1,
                                             0,  -1, 0,  -1, 0,  -1, 0,  -1, 0,  -1, -1, -1, 0,  -1, 0,  -1,
                                             -1, -1, -1, -1, 0,  -1, -1, -1, 0,  -1, -1, -1, -1, -1, 0,  -1};

    pq_config_t pq_cfg;

    pq_cfg.inputAFormat   = kPQ_32Bit;
    pq_cfg.inputAPrescale = 0;
    pq_cfg.inputBFormat   = kPQ_32Bit;
    pq_cfg.inputBPrescale = 0;
    pq_cfg.tmpFormat      = kPQ_32Bit;
    pq_cfg.tmpPrescale    = 0;
    pq_cfg.outputFormat   = kPQ_32Bit;
    pq_cfg.outputPrescale = 0;
    pq_cfg.tmpBase        = (uint32_t *)0xe0000000;
    pq_cfg.machineFormat  = kPQ_32Bit;
    PQ_SetConfig(DEMO_POWERQUAD, &pq_cfg);

    for (int i = 0; i < N; i++)
    {
        if (i == 0)
        {
            tmp_re = inputData[0];
            tmp_im = 0;
        }
        else
        {
            tmp_re = inputData[i * 2];
            tmp_im = inputData[N * 2 - (i * 2)] * -1;
        }

        acc0       = tmp_re * twiddle_table[i * 2];       /* real * real */
        acc1       = tmp_im * twiddle_table[(i * 2) + 1]; /* imaginary * imaginary */
        acc2       = acc0 - acc1;
        tmp[i * 2] = (uint32_t)(acc2 / (1024 * 1024 * 16));

        acc0             = tmp_re * twiddle_table[(i * 2) + 1]; /* real * imaginary */
        acc1             = tmp_im * twiddle_table[(i * 2)];     /* imaginary * real */
        acc2             = acc0 + acc1;
        tmp[(i * 2) + 1] = (uint32_t)(acc2 / (1024 * 1024 * 16));
    }

    PQ_TransformIDCT(DEMO_POWERQUAD, N, tmp, idctResult);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < 2 * N; i++)
    {
        EXAMPLE_ASSERT_TRUE(idctRef[i] == idctResult[i]);
    }
}

static void PQ_IDCTFixed32Example(void)
{
    int N = FILTER_INPUT_LEN;
    int64_t acc0;
    int64_t acc1;
    int64_t acc2;
    int64_t tmp_re, tmp_im;
    const int32_t *twiddle_table = idct32_twiddle;
    int32_t tmp[FILTER_INPUT_LEN * 2];
    int32_t inputData[FILTER_INPUT_LEN * 2] = {
        3, 0, 5, 0, 4, 0, 3, 0, 2, 0, 1, 0, 0, 0, 0, 0, -1, 0, -2, 0, -2, 0, -2, 0, -2, 0, -1, 0, 0,  0, 0, 0,
        1, 0, 1, 0, 2, 0, 2, 0, 2, 0, 1, 0, 1, 0, 0, 0, 0,  0, 0,  0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, 0, 0};
    int32_t idctResult[FILTER_INPUT_LEN * 2];
    int32_t idctRef[FILTER_INPUT_LEN * 2] = {3,  1,  2,  0,  3,  0,  7,  0,  -1, 0,  0,  -1, -1, 0,  0,  0,
                                             -1, -1, 0,  0,  -1, -1, -1, -1, -1, -1, 0,  -1, -1, 0,  -1, -1,
                                             0,  -1, 0,  -1, 0,  -1, 0,  -1, 0,  -1, -1, -1, 0,  -1, 0,  -1,
                                             -1, -1, -1, -1, 0,  -1, -1, -1, 0,  -1, -1, -1, -1, -1, 0,  -1};

    pq_config_t pq_cfg;

    pq_cfg.inputAFormat   = kPQ_32Bit;
    pq_cfg.inputAPrescale = 0;
    pq_cfg.inputBFormat   = kPQ_32Bit;
    pq_cfg.inputBPrescale = 0;
    pq_cfg.tmpFormat      = kPQ_32Bit;
    pq_cfg.tmpPrescale    = 0;
    pq_cfg.outputFormat   = kPQ_32Bit;
    pq_cfg.outputPrescale = 0;
    pq_cfg.tmpBase        = (uint32_t *)0xe0000000;
    pq_cfg.machineFormat  = kPQ_32Bit;
    PQ_SetConfig(DEMO_POWERQUAD, &pq_cfg);

    for (int i = 0; i < N; i++)
    {
        if (i == 0)
        {
            tmp_re = inputData[0];
            tmp_im = 0;
        }
        else
        {
            tmp_re = inputData[i * 2];
            tmp_im = inputData[N * 2 - (i * 2)] * -1;
        }

        acc0       = tmp_re * twiddle_table[i * 2];       /* real * real */
        acc1       = tmp_im * twiddle_table[(i * 2) + 1]; /* imaginary * imaginary */
        acc2       = acc0 - acc1;
        tmp[i * 2] = (uint32_t)(acc2 / (1024 * 1024 * 16));

        acc0             = tmp_re * twiddle_table[(i * 2) + 1]; /* real * imaginary */
        acc1             = tmp_im * twiddle_table[(i * 2)];     /* imaginary * real */
        acc2             = acc0 + acc1;
        tmp[(i * 2) + 1] = (uint32_t)(acc2 / (1024 * 1024 * 16));
    }

    PQ_TransformIDCT(DEMO_POWERQUAD, N, tmp, idctResult);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < N * 2; i++)
    {
        EXAMPLE_ASSERT_TRUE(idctRef[i] == idctResult[i]);
    }
}
