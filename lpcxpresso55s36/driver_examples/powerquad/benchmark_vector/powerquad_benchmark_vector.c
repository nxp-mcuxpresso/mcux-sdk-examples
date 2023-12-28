/*
 * Copyright 2019-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_powerquad.h"
#include "arm_math.h"
#include "arm_const_structs.h"

/*
 * This test case test the benchmark of the powerquad CMSIS DSP APIs.
 * The result could be compared with the software method using CMSIS DSP.
 */

#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_POWERQUAD POWERQUAD

#define MATRIX_TEST_LOOP 10000
#define MATH_TEST_LOOP   100000
#define FIR_TEST_LOOP    100000
#define FFT_TEST_LOOP    10000
#define VECTOR_TEST_LOOP 10000
#define BIQUAD_TEST_LOOP 10000

#define RFFT_INPUT_LEN 512
#define CFFT_INPUT_LEN 512
#define IFFT_INPUT_LEN 512
#define DCT_INPUT_LEN  512

#define VECTOR_INPUT_LEN 128

#define BIQUAD_CASCADE_STAGE    16
#define BIQUAD_CASCADE_DATA_LEN 128

#define MATRIX_ROW     16
#define MATRIX_COL     16
#define MATRIX_LEN     (MATRIX_COL * MATRIX_ROW)
#define FLOAT_2_Q31(x) ((int32_t)((x)*2147483648.0f))
#define FLOAT_2_Q15(x) (int16_t) __SSAT(((int32_t)((x)*32768.0f)), 16)

#define FIR_INPUT_LEN 16
#define FIR_TAP_LEN   12

#define CONV_A_LEN      5
#define CONV_B_LEN      5
#define CONV_RESULT_LEN (CONV_A_LEN + CONV_B_LEN - 1)

#define CORR_A_LEN      5
#define CORR_B_LEN      5
#define CORR_RESULT_LEN (CORR_A_LEN + CORR_B_LEN - 1)

#define MATH_PI 3.1415926535898

#define PQ_Primitive_Vector32(format, inf)  \
    PQ_Initiate_Vector_Func(pSrc, pDst);    \
    PQ_Vector8_##format(false, false, inf); \
    PQ_Vector8_##format(true, false, inf);  \
    PQ_Vector8_##format(true, false, inf);  \
    PQ_Vector8_##format(true, true, inf);   \
    PQ_End_Vector_Func();

#define EXAMPLE_ASSERT_TRUE(x)            \
    if (!(x))                             \
    {                                     \
        PRINTF("%s error\r\n", __func__); \
        while (1)                         \
        {                                 \
        }                                 \
    }

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* Vectorized math functions with optimized*/
static void PQ_VectorSqrtQ31Test(void);
static void PQ_VectorSqrtQ15Test(void);
static void PQ_VectorSinF32Test(void);
static void PQ_VectorSinQ31Test(void);
static void PQ_VectorSinQ15Test(void);
static void PQ_VectorCosF32Test(void);
static void PQ_VectorCosQ31Test(void);
static void PQ_VectorCosQ15Test(void);

static void PQ_VectorLnFPTest(void);
static void PQ_VectorLnFXTest(void);
static void PQ_VectorInvFPTest(void);
static void PQ_VectorInvFXTest(void);
static void PQ_VectorSqrtFPTest(void);
static void PQ_VectorSqrtFXTest(void);
static void PQ_VectorInvSqrtFPTest(void);
static void PQ_VectorInvSqrtFXTest(void);
static void PQ_VectorEtoxFPTest(void);
static void PQ_VectorEtoxFXTest(void);
static void PQ_VectorEtonxFPTest(void);
static void PQ_VectorEtonxFXTest(void);
static void PQ_16ByteBiquadCascadedf2FPTest(void);
static void PQ_16ByteBiquadCascadedf2FXTest(void);

/* Old vectorized math functions with optimized*/
static void PQ_VectorLnF32Test(void);
static void PQ_VectorLnQ31Test(void);
static void PQ_VectorInvF32Test(void);
static void PQ_VectorInvQ31Test(void);
static void PQ_VectorSqrtF32Test(void);
static void PQ_VectorSqrtQ31Test(void);
static void PQ_VectorInvSqrtF32Test(void);
static void PQ_VectorInvSqrtQ31Test(void);
static void PQ_VectorEtoxF32Test(void);
static void PQ_VectorEtoxQ31Test(void);
static void PQ_VectorEtonxF32Test(void);
static void PQ_VectorEtonxQ31Test(void);
static void PQ_BiquadCascadedf2F32Test(void);
static void PQ_BiquadCascadedf2Q31Test(void);

static void TEST_InitTime(void);
static uint32_t TEST_GetTime(void);

static void PQ_Vector16BiquadDf2FX(int32_t *pSrc, int32_t *pDst);
static void PQ_16ByteBiquadCascadeDf2FX(const pq_biquad_cascade_df2_instance *S,
                                        int32_t *pSrc,
                                        int32_t *pDst,
                                        uint32_t blockSize);
static void PQ_Vector16BiquadCascadeDf2FX(int32_t *pSrc, int32_t *pDst);
static void PQ_16ByteBiquadCascadeDf2FP(const pq_biquad_cascade_df2_instance *S,
                                        float *pSrc,
                                        float *pDst,
                                        uint32_t blockSize);
static void PQ_Vector16BiquadDf2FP(float *pSrc, float *pDst);
static void PQ_Vector16BiquadCascadeDf2FP(float *pSrc, float *pDst);
static void PQ_Vector32EtonxFX(int32_t *pSrc, int32_t *pDst);
static void PQ_Vector32EtonxFP(float *pSrc, float *pDst);
static void PQ_Vector32EtoxFX(int32_t *pSrc, int32_t *pDst);
static void PQ_Vector32InvSqrtFX(int32_t *pSrc, int32_t *pDst);
static void PQ_Vector32LnFP(float *pSrc, float *pDst);
static void PQ_Vector32LnFX(int32_t *pSrc, int32_t *pDst);
static void PQ_Vector32InvFP(float *pSrc, float *pDst);
static void PQ_Vector32InvFX(int32_t *pSrc, int32_t *pDst);
static void PQ_Vector32SqrtFP(float *pSrc, float *pDst);
static void PQ_Vector32SqrtFX(int32_t *pSrc, int32_t *pDst);
static void PQ_Vector32InvSqrtFP(float *pSrc, float *pDst);
static void PQ_Vector32EtoxFP(float *pSrc, float *pDst);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static volatile uint32_t s_timeMs;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*
 * @brief Main function
 */
int main(void)
{
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    PRINTF("\r\nCMSIS DSP benchmark vector test start.\r\n");

    PQ_Init(POWERQUAD);

    TEST_InitTime();

    // Powerquad Benchmark Vector Test
    PQ_VectorLnF32Test();
    PQ_VectorLnQ31Test();
    PQ_VectorInvF32Test();
    PQ_VectorInvQ31Test();
    PQ_VectorSqrtF32Test();
    PQ_VectorSqrtQ31Test();
    PQ_VectorInvSqrtF32Test();
    PQ_VectorInvSqrtQ31Test();
    PQ_VectorEtoxF32Test();
    PQ_VectorEtoxQ31Test();
    PQ_VectorEtonxF32Test();
    PQ_VectorEtonxQ31Test();
    PQ_BiquadCascadedf2F32Test();
    PQ_BiquadCascadedf2Q31Test();

    PQ_VectorLnFPTest();
    PQ_VectorLnFXTest();
    PQ_VectorInvFPTest();
    PQ_VectorInvFXTest();
    PQ_VectorSqrtFPTest();
    PQ_VectorSqrtFXTest();
    PQ_VectorInvSqrtFPTest();
    PQ_VectorInvSqrtFXTest();
    PQ_VectorEtoxFPTest();
    PQ_VectorEtoxFXTest();
    PQ_VectorEtonxFPTest();
    PQ_VectorEtonxFXTest();
    PQ_16ByteBiquadCascadedf2FPTest();
    PQ_16ByteBiquadCascadedf2FXTest();

    /* Vector. */
    PQ_VectorSqrtQ31Test();
    PQ_VectorSqrtQ15Test();
    PQ_VectorSinF32Test();
    PQ_VectorSinQ31Test();
    PQ_VectorSinQ15Test();
    PQ_VectorCosF32Test();
    PQ_VectorCosQ31Test();
    PQ_VectorCosQ15Test();

    PRINTF("\r\nCMSIS DSP benchmark vector test succeeded.\r\n");

    while (1)
    {
    }
}

void SysTick_Handler(void)
{
    s_timeMs++;
}

static void TEST_InitTime(void)
{
    s_timeMs = 0;

    /* Configure to 1 ms. */
    SysTick_Config(SystemCoreClock / 1000);
}

static uint32_t TEST_GetTime(void)
{
    return s_timeMs;
}

static void PQ_VectorSqrtQ15Test(void)
{
    uint32_t i;
    uint32_t oldTime;

    q15_t input[VECTOR_INPUT_LEN];
    q15_t result[VECTOR_INPUT_LEN];
    q15_t ref[VECTOR_INPUT_LEN];
    float inputFloat;

    const pq_prescale_t prescale = {
        .inputPrescale  = -15,
        .outputPrescale = 15,
        .outputSaturate = 0,
    };

    for (i = 0; i < VECTOR_INPUT_LEN; i++)
    {
        inputFloat = (float)i / (float)VECTOR_INPUT_LEN;
        input[i]   = FLOAT_2_Q15(inputFloat);
        ref[i]     = FLOAT_2_Q15(sqrt((double)inputFloat));
    }

    PQ_SetCoprocessorScaler(POWERQUAD, &prescale);

    oldTime = TEST_GetTime();
    for (i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_VectorSqrtFixed16(input, result, VECTOR_INPUT_LEN);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (i = 0; i < VECTOR_INPUT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(result[i] - ref[i]) <= 10);
    }
}

static void PQ_VectorSinF32Test(void)
{
    uint32_t i;
    uint32_t oldTime;

    float32_t input[VECTOR_INPUT_LEN];
    float32_t result[VECTOR_INPUT_LEN];
    float32_t ref[VECTOR_INPUT_LEN];

    for (i = 0; i < VECTOR_INPUT_LEN; i++)
    {
        input[i] = (float)i / (float)VECTOR_INPUT_LEN;
        ref[i]   = sin(input[i]);
    }

    oldTime = TEST_GetTime();
    for (i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_VectorSinF32(input, result, VECTOR_INPUT_LEN);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (i = 0; i < VECTOR_INPUT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(result[i] - ref[i])) <= 0.00005);
    }
}

static void PQ_VectorSinQ31Test(void)
{
    uint32_t i;
    uint32_t oldTime;

    q31_t input[VECTOR_INPUT_LEN];
    q31_t result[VECTOR_INPUT_LEN];
    q31_t ref[VECTOR_INPUT_LEN];
    float inputFloat;

    for (i = 0; i < VECTOR_INPUT_LEN; i++)
    {
        inputFloat = ((float)i * 2.0f / (float)VECTOR_INPUT_LEN) - 1.0f;
        input[i]   = FLOAT_2_Q31(inputFloat);
        ref[i]     = FLOAT_2_Q31(sin((double)(inputFloat)*MATH_PI));
    }

    oldTime = TEST_GetTime();
    for (i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_VectorSinQ31(input, result, VECTOR_INPUT_LEN);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (i = 0; i < VECTOR_INPUT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(result[i] - ref[i]) <= 400);
    }
}

static void PQ_VectorSinQ15Test(void)
{
    uint32_t i;
    uint32_t oldTime;

    q15_t input[VECTOR_INPUT_LEN];
    q15_t result[VECTOR_INPUT_LEN];
    q15_t ref[VECTOR_INPUT_LEN];
    float inputFloat;

    for (i = 0; i < VECTOR_INPUT_LEN; i++)
    {
        inputFloat = ((float)i * 2.0f / (float)VECTOR_INPUT_LEN) - 1.0f;
        input[i]   = FLOAT_2_Q15(inputFloat);
        ref[i]     = FLOAT_2_Q15(sin((double)(inputFloat)*MATH_PI));
    }

    oldTime = TEST_GetTime();
    for (i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_VectorSinQ15(input, result, VECTOR_INPUT_LEN);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (i = 0; i < VECTOR_INPUT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(result[i] - ref[i]) <= 2);
    }
}

static void PQ_VectorCosF32Test(void)
{
    uint32_t i;
    uint32_t oldTime;

    float32_t input[VECTOR_INPUT_LEN];
    float32_t result[VECTOR_INPUT_LEN];
    float32_t ref[VECTOR_INPUT_LEN];

    for (i = 0; i < VECTOR_INPUT_LEN; i++)
    {
        input[i] = (float)i / (float)VECTOR_INPUT_LEN;
        ref[i]   = cos(input[i]);
    }

    oldTime = TEST_GetTime();
    for (i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_VectorCosF32(input, result, VECTOR_INPUT_LEN);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (i = 0; i < VECTOR_INPUT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(result[i] - ref[i])) <= 0.00005);
    }
}

static void PQ_VectorCosQ31Test(void)
{
    uint32_t i;
    uint32_t oldTime;

    q31_t input[VECTOR_INPUT_LEN];
    q31_t result[VECTOR_INPUT_LEN];
    q31_t ref[VECTOR_INPUT_LEN];
    float inputFloat;

    for (i = 0; i < VECTOR_INPUT_LEN; i++)
    {
        inputFloat = ((float)i * 2.0f / (float)VECTOR_INPUT_LEN) - 1.0f;
        input[i]   = FLOAT_2_Q31(inputFloat);
        ref[i]     = FLOAT_2_Q31(cos((double)(inputFloat)*MATH_PI));
    }

    oldTime = TEST_GetTime();
    for (i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_VectorCosQ31(input, result, VECTOR_INPUT_LEN);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (i = 0; i < VECTOR_INPUT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(result[i] - ref[i]) <= 500);
    }
}

static void PQ_VectorCosQ15Test(void)
{
    uint32_t i;
    uint32_t oldTime;

    q15_t input[VECTOR_INPUT_LEN];
    q15_t result[VECTOR_INPUT_LEN];
    q15_t ref[VECTOR_INPUT_LEN];
    float inputFloat;

    for (i = 0; i < VECTOR_INPUT_LEN; i++)
    {
        inputFloat = ((float)i * 2.0f / (float)VECTOR_INPUT_LEN) - 1.0f;
        input[i]   = FLOAT_2_Q15(inputFloat);
        ref[i]     = FLOAT_2_Q15(cos((double)(inputFloat)*MATH_PI));
    }

    oldTime = TEST_GetTime();
    for (i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_VectorCosQ15(input, result, VECTOR_INPUT_LEN);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (i = 0; i < VECTOR_INPUT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(result[i] - ref[i]) <= 2);
    }
}

/* High performance vector functions */
static void PQ_VectorLnFPTest(void)
{
    float32_t input[32]  = {1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6,
                           1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6};
    float32_t result[32] = {0};
    float32_t lnRef[32]  = {0, 0.693147, 1.098612, 1.386294, 1.609438, 1.791760, 1.609438, 1.791760,
                           0, 0.693147, 1.098612, 1.386294, 1.609438, 1.791760, 1.609438, 1.791760,
                           0, 0.693147, 1.098612, 1.386294, 1.609438, 1.791760, 1.609438, 1.791760,
                           0, 0.693147, 1.098612, 1.386294, 1.609438, 1.791760, 1.609438, 1.791760};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_Vector32LnFP(input, result);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 32; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(lnRef[i] - result[i])) < 0.00001);
    }
}

static void PQ_VectorLnFXTest(void)
{
    q31_t input[32]  = {1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6};
    q31_t result[32] = {0};
    q31_t lnRef[32]  = {0, 1, 1, 1, 2, 2, 2, 2, 0, 1, 1, 1, 2, 2, 2, 2, 0, 1, 1, 1, 2, 2, 2, 2, 0, 1, 1, 1, 2, 2, 2, 2};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_Vector32LnFX(input, result);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 32; i++)
    {
        EXAMPLE_ASSERT_TRUE(lnRef[i] == result[i]);
    }
}

static void PQ_VectorInvFPTest(void)
{
    float32_t input[32]  = {1, 2, 4, 5, 10, 20, 10, 20, 1, 2, 4, 5, 10, 20, 10, 20,
                           1, 2, 4, 5, 10, 20, 10, 20, 1, 2, 4, 5, 10, 20, 10, 20};
    float32_t result[32] = {0};
    float32_t invRef[32] = {1, 0.5, 0.25, 0.2, 0.1, 0.05, 0.1, 0.05, 1, 0.5, 0.25, 0.2, 0.1, 0.05, 0.1, 0.05,
                            1, 0.5, 0.25, 0.2, 0.1, 0.05, 0.1, 0.05, 1, 0.5, 0.25, 0.2, 0.1, 0.05, 0.1, 0.05};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_Vector32InvFP(input, result);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 32; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(invRef[i] - result[i])) < 0.00001);
    }
}

static void PQ_VectorInvFXTest(void)
{
    q31_t input[32] = {
        1, 2, 4, 5, 10, 20, 10, 20, 1, 2, 4, 5, 10, 20, 10, 20, 1, 2, 4, 5, 10, 20, 10, 20, 1, 2, 4, 5, 10, 20, 10, 20,
    };
    q31_t result[32] = {0};
    q31_t invRef[32] = {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_Vector32InvFX(input, result);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 32; i++)
    {
        EXAMPLE_ASSERT_TRUE(invRef[i] == result[i]);
    }
}

/* Float Vector Sqrt */
static void PQ_VectorSqrtFPTest(void)
{
    float32_t input[32]   = {0.04, 0.25, 1, 4, 9, 16, 144, 1.69, 0.04, 0.25, 1, 4, 9, 16, 144, 1.69,
                           0.04, 0.25, 1, 4, 9, 16, 144, 1.69, 0.04, 0.25, 1, 4, 9, 16, 144, 1.69};
    float32_t result[32]  = {0};
    float32_t sqrtRef[32] = {0.2, 0.5, 1, 2, 3, 4, 12, 1.3, 0.2, 0.5, 1, 2, 3, 4, 12, 1.3,
                             0.2, 0.5, 1, 2, 3, 4, 12, 1.3, 0.2, 0.5, 1, 2, 3, 4, 12, 1.3};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_Vector32SqrtFP(input, result);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 32; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(sqrtRef[i] - result[i])) < 0.00001);
    }
}

static void PQ_VectorSqrtFXTest(void)
{
    q31_t input[32]   = {0, 1, 1, 4, 144, 169, 144, 169, 0, 1, 1, 4, 144, 169, 144, 169,
                       0, 1, 1, 4, 144, 169, 144, 169, 0, 1, 1, 4, 144, 169, 144, 169};
    q31_t result[32]  = {0};
    q31_t sqrtRef[32] = {0, 1, 1, 2, 12, 13, 12, 13, 0, 1, 1, 2, 12, 13, 12, 13,
                         0, 1, 1, 2, 12, 13, 12, 13, 0, 1, 1, 2, 12, 13, 12, 13};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_Vector32SqrtFX(input, result);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 32; i++)
    {
        EXAMPLE_ASSERT_TRUE(sqrtRef[i] == result[i]);
    }
}

static void PQ_VectorInvSqrtFPTest(void)
{
    float32_t input[32]      = {4, 1, 1, 0.25, 16, 100, 16, 100, 4, 1, 1, 0.25, 16, 100, 16, 100,
                           4, 1, 1, 0.25, 16, 100, 16, 100, 4, 1, 1, 0.25, 16, 100, 16, 100};
    float32_t result[32]     = {0};
    float32_t invSqrtRef[32] = {0.5, 1, 1, 2, 0.25, 0.1, 0.25, 0.1, 0.5, 1, 1, 2, 0.25, 0.1, 0.25, 0.1,
                                0.5, 1, 1, 2, 0.25, 0.1, 0.25, 0.1, 0.5, 1, 1, 2, 0.25, 0.1, 0.25, 0.1};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_Vector32InvSqrtFP(input, result);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 32; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(invSqrtRef[i] - result[i])) < 0.00001);
    }
}

static void PQ_VectorInvSqrtFXTest(void)
{
    q31_t input[32]      = {2, 1, 1, 4, 1, 100, 1, 100, 2, 1, 1, 4, 1, 100, 1, 100,
                       2, 1, 1, 4, 1, 100, 1, 100, 2, 1, 1, 4, 1, 100, 1, 100};
    q31_t result[32]     = {0};
    q31_t invSqrtRef[32] = {1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0,
                            1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_Vector32InvSqrtFX(input, result);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 32; i++)
    {
        EXAMPLE_ASSERT_TRUE(invSqrtRef[i] == result[i]);
    }
}

static void PQ_VectorEtoxFPTest(void)
{
    float32_t input[32]   = {1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6,
                           1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6};
    float32_t result[32]  = {0};
    float32_t etoxRef[32] = {2.718282, 7.389056, 20.085537, 54.598148, 148.413162, 403.428802, 148.413162, 403.428802,
                             2.718282, 7.389056, 20.085537, 54.598148, 148.413162, 403.428802, 148.413162, 403.428802,
                             2.718282, 7.389056, 20.085537, 54.598148, 148.413162, 403.428802, 148.413162, 403.428802,
                             2.718282, 7.389056, 20.085537, 54.598148, 148.413162, 403.428802, 148.413162, 403.428802};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_Vector32EtoxFP(input, result);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 32; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(etoxRef[i] - result[i])) < 0.001);
    }
}

static void PQ_VectorEtoxFXTest(void)
{
    q31_t input[32]  = {1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6};
    q31_t result[32] = {0};
    q31_t etoxRef[32] = {3, 7, 20, 55, 148, 403, 148, 403, 3, 7, 20, 55, 148, 403, 148, 403,
                         3, 7, 20, 55, 148, 403, 148, 403, 3, 7, 20, 55, 148, 403, 148, 403};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_Vector32EtoxFX(input, result);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 32; i++)
    {
        EXAMPLE_ASSERT_TRUE(etoxRef[i] == result[i]);
    }
}

static void PQ_VectorEtonxFPTest(void)
{
    float32_t input[32]    = {1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6,
                           1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6};
    float32_t result[32]   = {0};
    float32_t etonxRef[32] = {0.367879, 0.135335, 0.049787, 0.018316, 0.006738, 0.002479, 0.006738, 0.002479,
                              0.367879, 0.135335, 0.049787, 0.018316, 0.006738, 0.002479, 0.006738, 0.002479,
                              0.367879, 0.135335, 0.049787, 0.018316, 0.006738, 0.002479, 0.006738, 0.002479,
                              0.367879, 0.135335, 0.049787, 0.018316, 0.006738, 0.002479, 0.006738, 0.002479};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_Vector32EtonxFP(input, result);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 32; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(etonxRef[i] - result[i])) < 0.00001);
    }
}

static void PQ_VectorEtonxFXTest(void)
{
    q31_t input[32]  = {1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6};
    q31_t result[32] = {0};
    q31_t etonxRef[32] = {0};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_Vector32EtonxFX(input, result);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 32; i++)
    {
        EXAMPLE_ASSERT_TRUE(etonxRef[i] == result[i]);
    }
}

static void PQ_16ByteBiquadCascadedf2FPTest(void)
{
    uint8_t stage               = 3;
    float32_t dataForBiquad[16] = {1024.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    float32_t biquadResult[16]  = {0};
    float32_t biquadRef[16]     = {16.270306,  -45.510643, 63.659611, -35.223492, -20.120678, 45.005569,
                               -14.585688, -26.120077, 25.420242, 6.363828,   -21.309111, 4.942792,
                               12.659100,  -8.400287,  -5.238772, 7.446815};

    pq_biquad_cascade_df2_instance instance;

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < BIQUAD_TEST_LOOP; i++)
    {
        q31_t state[24] = {0, 0, 0x3e80b780, 0x3f00b780, 0x3e80b780, 0xbe2f4f0e, 0x3e350b0f, 0,
                           0, 0, 0x3e80b780, 0x3f00b780, 0x3e80b780, 0xbe2f4f0e, 0x3e350b0f, 0,
                           0, 0, 0x3e80b780, 0x3f00b780, 0x3e80b780, 0xbe2f4f0e, 0x3e350b0f, 0};
        PQ_BiquadCascadeDf2Init(&instance, stage, (pq_biquad_state_t *)state);
        PQ_16ByteBiquadCascadeDf2FP(&instance, dataForBiquad, biquadResult, 16);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < 16; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(biquadRef[i] - biquadResult[i])) < 0.00001);
    }
}

static void PQ_16ByteBiquadCascadedf2FXTest(void)
{
    uint8_t stage           = 3;
    q31_t dataForBiquad[16] = {1024, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    q31_t biquadResult[16]  = {0};
    q31_t biquadRef[16]     = {16, -46, 64, -35, -20, 45, -15, -26, 25, 6, -21, 5, 13, -8, -5, 7};

    pq_biquad_cascade_df2_instance instance;

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < BIQUAD_TEST_LOOP; i++)
    {
        q31_t state[24] = {0, 0, 0x3e80b780, 0x3f00b780, 0x3e80b780, 0xbe2f4f0e, 0x3e350b0f, 0,
                           0, 0, 0x3e80b780, 0x3f00b780, 0x3e80b780, 0xbe2f4f0e, 0x3e350b0f, 0,
                           0, 0, 0x3e80b780, 0x3f00b780, 0x3e80b780, 0xbe2f4f0e, 0x3e350b0f, 0};
        PQ_BiquadCascadeDf2Init(&instance, stage, (pq_biquad_state_t *)state);
        PQ_16ByteBiquadCascadeDf2FX(&instance, dataForBiquad, biquadResult, 16);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < 16; i++)
    {
        EXAMPLE_ASSERT_TRUE(biquadRef[i] == biquadResult[i]);
    }
}

static void PQ_VectorLnF32Test(void)
{
    float32_t input[32]  = {1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6,
                           1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6};
    float32_t result[32] = {0};
    float32_t lnRef[32]  = {0, 0.693147, 1.098612, 1.386294, 1.609438, 1.791760, 1.609438, 1.791760,
                           0, 0.693147, 1.098612, 1.386294, 1.609438, 1.791760, 1.609438, 1.791760,
                           0, 0.693147, 1.098612, 1.386294, 1.609438, 1.791760, 1.609438, 1.791760,
                           0, 0.693147, 1.098612, 1.386294, 1.609438, 1.791760, 1.609438, 1.791760};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_VectorLnF32(input, result, 8);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 8; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(lnRef[i] - result[i])) < 0.00001);
    }
}

static void PQ_VectorLnQ31Test(void)
{
    q31_t input[32]  = {1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6};
    q31_t result[32] = {0};
    q31_t lnRef[32]  = {0, 1, 1, 1, 2, 2, 2, 2, 0, 1, 1, 1, 2, 2, 2, 2, 0, 1, 1, 1, 2, 2, 2, 2, 0, 1, 1, 1, 2, 2, 2, 2};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_VectorLnFixed32(input, result, 8);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 8; i++)
    {
        EXAMPLE_ASSERT_TRUE(lnRef[i] == result[i]);
    }
}

static void PQ_VectorInvF32Test(void)
{
    float32_t input[32]  = {1, 2, 4, 5, 10, 20, 10, 20, 1, 2, 4, 5, 10, 20, 10, 20,
                           1, 2, 4, 5, 10, 20, 10, 20, 1, 2, 4, 5, 10, 20, 10, 20};
    float32_t result[32] = {0};
    float32_t invRef[32] = {1, 0.5, 0.25, 0.2, 0.1, 0.05, 0.1, 0.05, 1, 0.5, 0.25, 0.2, 0.1, 0.05, 0.1, 0.05,
                            1, 0.5, 0.25, 0.2, 0.1, 0.05, 0.1, 0.05, 1, 0.5, 0.25, 0.2, 0.1, 0.05, 0.1, 0.05};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_VectorInvF32(input, result, 8);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 8; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(invRef[i] - result[i])) < 0.00001);
    }
}

static void PQ_VectorInvQ31Test(void)
{
    q31_t input[32] = {
        1, 2, 4, 5, 10, 20, 10, 20, 1, 2, 4, 5, 10, 20, 10, 20, 1, 2, 4, 5, 10, 20, 10, 20, 1, 2, 4, 5, 10, 20, 10, 20,
    };
    q31_t result[32] = {0};
    q31_t invRef[32] = {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_VectorInvFixed32(input, result, 8);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 8; i++)
    {
        EXAMPLE_ASSERT_TRUE(invRef[i] == result[i]);
    }
}

/* Float Vector Sqrt */
static void PQ_VectorSqrtF32Test(void)
{
    float32_t input[32]   = {0.04, 0.25, 1, 4, 9, 16, 144, 1.69, 0.04, 0.25, 1, 4, 9, 16, 144, 1.69,
                           0.04, 0.25, 1, 4, 9, 16, 144, 1.69, 0.04, 0.25, 1, 4, 9, 16, 144, 1.69};
    float32_t result[32]  = {0};
    float32_t sqrtRef[32] = {0.2, 0.5, 1, 2, 3, 4, 12, 1.3, 0.2, 0.5, 1, 2, 3, 4, 12, 1.3,
                             0.2, 0.5, 1, 2, 3, 4, 12, 1.3, 0.2, 0.5, 1, 2, 3, 4, 12, 1.3};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_VectorSqrtF32(input, result, 8);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 8; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(sqrtRef[i] - result[i])) < 0.00001);
    }
}

static void PQ_VectorSqrtQ31Test(void)
{
    q31_t input[32]   = {0, 1, 1, 4, 144, 169, 144, 169, 0, 1, 1, 4, 144, 169, 144, 169,
                       0, 1, 1, 4, 144, 169, 144, 169, 0, 1, 1, 4, 144, 169, 144, 169};
    q31_t result[32]  = {0};
    q31_t sqrtRef[32] = {0, 1, 1, 2, 12, 13, 12, 13, 0, 1, 1, 2, 12, 13, 12, 13,
                         0, 1, 1, 2, 12, 13, 12, 13, 0, 1, 1, 2, 12, 13, 12, 13};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_VectorSqrtFixed32(input, result, 8);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 8; i++)
    {
        EXAMPLE_ASSERT_TRUE(sqrtRef[i] == result[i]);
    }
}

static void PQ_VectorInvSqrtF32Test(void)
{
    float32_t input[32]      = {4, 1, 1, 0.25, 16, 100, 16, 100, 4, 1, 1, 0.25, 16, 100, 16, 100,
                           4, 1, 1, 0.25, 16, 100, 16, 100, 4, 1, 1, 0.25, 16, 100, 16, 100};
    float32_t result[32]     = {0};
    float32_t invSqrtRef[32] = {0.5, 1, 1, 2, 0.25, 0.1, 0.25, 0.1, 0.5, 1, 1, 2, 0.25, 0.1, 0.25, 0.1,
                                0.5, 1, 1, 2, 0.25, 0.1, 0.25, 0.1, 0.5, 1, 1, 2, 0.25, 0.1, 0.25, 0.1};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_VectorInvSqrtF32(input, result, 8);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 8; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(invSqrtRef[i] - result[i])) < 0.00001);
    }
}

static void PQ_VectorInvSqrtQ31Test(void)
{
    q31_t input[32]      = {2, 1, 1, 4, 1, 100, 1, 100, 2, 1, 1, 4, 1, 100, 1, 100,
                       2, 1, 1, 4, 1, 100, 1, 100, 2, 1, 1, 4, 1, 100, 1, 100};
    q31_t result[32]     = {0};
    q31_t invSqrtRef[32] = {1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0,
                            1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_VectorInvSqrtFixed32(input, result, 8);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 8; i++)
    {
        EXAMPLE_ASSERT_TRUE(invSqrtRef[i] == result[i]);
    }
}

static void PQ_VectorEtoxF32Test(void)
{
    float32_t input[32]   = {1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6,
                           1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6};
    float32_t result[32]  = {0};
    float32_t etoxRef[32] = {2.718282, 7.389056, 20.085537, 54.598148, 148.413162, 403.428802, 148.413162, 403.428802,
                             2.718282, 7.389056, 20.085537, 54.598148, 148.413162, 403.428802, 148.413162, 403.428802,
                             2.718282, 7.389056, 20.085537, 54.598148, 148.413162, 403.428802, 148.413162, 403.428802,
                             2.718282, 7.389056, 20.085537, 54.598148, 148.413162, 403.428802, 148.413162, 403.428802};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_VectorEtoxF32(input, result, 8);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 8; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(etoxRef[i] - result[i])) < 0.001);
    }
}

static void PQ_VectorEtoxQ31Test(void)
{
    q31_t input[32]  = {1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6};
    q31_t result[32] = {0};
    q31_t etoxRef[32] = {3, 7, 20, 55, 148, 403, 148, 403, 3, 7, 20, 55, 148, 403, 148, 403,
                         3, 7, 20, 55, 148, 403, 148, 403, 3, 7, 20, 55, 148, 403, 148, 403};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_VectorEtoxFixed32(input, result, 8);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 8; i++)
    {
        EXAMPLE_ASSERT_TRUE(etoxRef[i] == result[i]);
    }
}

static void PQ_VectorEtonxF32Test(void)
{
    float32_t input[32]    = {1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6,
                           1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6};
    float32_t result[32]   = {0};
    float32_t etonxRef[32] = {0.367879, 0.135335, 0.049787, 0.018316, 0.006738, 0.002479, 0.006738, 0.002479,
                              0.367879, 0.135335, 0.049787, 0.018316, 0.006738, 0.002479, 0.006738, 0.002479,
                              0.367879, 0.135335, 0.049787, 0.018316, 0.006738, 0.002479, 0.006738, 0.002479,
                              0.367879, 0.135335, 0.049787, 0.018316, 0.006738, 0.002479, 0.006738, 0.002479};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_VectorEtonxF32(input, result, 8);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 8; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(etonxRef[i] - result[i])) < 0.00001);
    }
}

static void PQ_VectorEtonxQ31Test(void)
{
    q31_t input[32]  = {1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6, 1, 2, 3, 4, 5, 6, 5, 6};
    q31_t result[32] = {0};
    q31_t etonxRef[32] = {0};

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < VECTOR_TEST_LOOP; i++)
    {
        PQ_VectorEtonxFixed32(input, result, 8);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (int i = 0; i < 8; i++)
    {
        EXAMPLE_ASSERT_TRUE(etonxRef[i] == result[i]);
    }
}

static void PQ_BiquadCascadedf2F32Test(void)
{
    uint8_t stage               = 3;
    float32_t dataForBiquad[16] = {1024.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    float32_t biquadResult[16]  = {0};
    float32_t biquadRef[16]     = {16.270306,  -45.510643, 63.659611, -35.223492, -20.120678, 45.005569,
                               -14.585688, -26.120077, 25.420242, 6.363828,   -21.309111, 4.942792,
                               12.659100,  -8.400287,  -5.238772, 7.446815};

    pq_biquad_cascade_df2_instance instance;

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < BIQUAD_TEST_LOOP; i++)
    {
        q31_t state[24] = {0, 0, 0x3e80b780, 0x3f00b780, 0x3e80b780, 0xbe2f4f0e, 0x3e350b0f, 0,
                           0, 0, 0x3e80b780, 0x3f00b780, 0x3e80b780, 0xbe2f4f0e, 0x3e350b0f, 0,
                           0, 0, 0x3e80b780, 0x3f00b780, 0x3e80b780, 0xbe2f4f0e, 0x3e350b0f, 0};
        PQ_BiquadCascadeDf2Init(&instance, stage, (pq_biquad_state_t *)state);
        PQ_BiquadCascadeDf2F32(&instance, dataForBiquad, biquadResult, 16);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < 16; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(biquadRef[i] - biquadResult[i])) < 0.00001);
    }
}

static void PQ_BiquadCascadedf2Q31Test(void)
{
    uint8_t stage           = 3;
    q31_t dataForBiquad[16] = {1024, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    q31_t biquadResult[16]  = {0};
    q31_t biquadRef[16]     = {16, -46, 64, -35, -20, 45, -15, -26, 25, 6, -21, 5, 13, -8, -5, 7};

    pq_biquad_cascade_df2_instance instance;

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < BIQUAD_TEST_LOOP; i++)
    {
        q31_t state[24] = {0, 0, 0x3e80b780, 0x3f00b780, 0x3e80b780, 0xbe2f4f0e, 0x3e350b0f, 0,
                           0, 0, 0x3e80b780, 0x3f00b780, 0x3e80b780, 0xbe2f4f0e, 0x3e350b0f, 0,
                           0, 0, 0x3e80b780, 0x3f00b780, 0x3e80b780, 0xbe2f4f0e, 0x3e350b0f, 0};
        PQ_BiquadCascadeDf2Init(&instance, stage, (pq_biquad_state_t *)state);
        PQ_BiquadCascadeDf2Fixed32(&instance, dataForBiquad, biquadResult, 16);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < 16; i++)
    {
        EXAMPLE_ASSERT_TRUE(biquadRef[i] == biquadResult[i]);
    }
}

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_16ByteBiquadCascadeDf2FX(const pq_biquad_cascade_df2_instance *S,
                                        int32_t *pSrc,
                                        int32_t *pDst,
                                        uint32_t blockSize) __attribute__((noinline))
#else
static void PQ_16ByteBiquadCascadeDf2FX(const pq_biquad_cascade_df2_instance *S,
                                        int32_t *pSrc,
                                        int32_t *pDst,
                                        uint32_t blockSize)
#endif
{
    uint32_t stage            = S->numStages;
    pq_biquad_state_t *states = S->pState;

    if (pDst != pSrc)
    {
        memcpy(pDst, pSrc, 4 * blockSize);
    }

    if (stage % 2 != 0)
    {
        PQ_BiquadRestoreInternalState(POWERQUAD, 0, states);

        PQ_Vector16BiquadDf2FX(pSrc, pDst);

        PQ_BiquadBackUpInternalState(POWERQUAD, 0, states);

        states++;
        stage--;
    }

    do
    {
        PQ_BiquadRestoreInternalState(POWERQUAD, 0, states);
        states++;
        PQ_BiquadRestoreInternalState(POWERQUAD, 1, states);

        PQ_Vector16BiquadCascadeDf2FX(pDst, pDst);

        states--;
        PQ_BiquadBackUpInternalState(POWERQUAD, 0, states);
        states++;
        PQ_BiquadBackUpInternalState(POWERQUAD, 1, states);

        states++;
        stage -= 2U;
    } while (stage > 0U);
}

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector16BiquadDf2FX(int32_t *pSrc, int32_t *pDst) __attribute__((noinline))
#else
static void PQ_Vector16BiquadDf2FX(int32_t *pSrc, int32_t *pDst)
#endif
{
    PQ_Initiate_Vector_Func(pSrc, pDst);
    PQ_DF2_Vector8_FX(false, false);
    PQ_DF2_Vector8_FX(true, true);
    PQ_End_Vector_Func();
}

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector16BiquadCascadeDf2FX(int32_t *pSrc, int32_t *pDst) __attribute__((noinline))
#else
static void PQ_Vector16BiquadCascadeDf2FX(int32_t *pSrc, int32_t *pDst)
#endif
{
    PQ_Initiate_Vector_Func(pSrc, pDst);
    PQ_DF2_Cascade_Vector8_FX(false, false);
    PQ_DF2_Cascade_Vector8_FX(true, true);
    PQ_End_Vector_Func();
}

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_16ByteBiquadCascadeDf2FP(const pq_biquad_cascade_df2_instance *S,
                                        float *pSrc,
                                        float *pDst,
                                        uint32_t blockSize) __attribute__((noinline))
#else
static void PQ_16ByteBiquadCascadeDf2FP(const pq_biquad_cascade_df2_instance *S,
                                        float *pSrc,
                                        float *pDst,
                                        uint32_t blockSize)
#endif
{
    uint32_t stage            = S->numStages;
    pq_biquad_state_t *states = S->pState;

    if (pDst != pSrc)
    {
        memcpy(pDst, pSrc, 4 * blockSize);
    }

    if (stage % 2 != 0)
    {
        PQ_BiquadRestoreInternalState(POWERQUAD, 0, states);

        PQ_Vector16BiquadDf2FP(pSrc, pDst);

        PQ_BiquadBackUpInternalState(POWERQUAD, 0, states);

        states++;
        stage--;
    }

    do
    {
        PQ_BiquadRestoreInternalState(POWERQUAD, 1, states);
        states++;
        PQ_BiquadRestoreInternalState(POWERQUAD, 0, states);

        PQ_Vector16BiquadCascadeDf2FP(pDst, pDst);

        states--;
        PQ_BiquadBackUpInternalState(POWERQUAD, 1, states);
        states++;
        PQ_BiquadBackUpInternalState(POWERQUAD, 0, states);

        states++;
        stage -= 2U;

    } while (stage > 0U);
}

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector16BiquadDf2FP(float *pSrc, float *pDst) __attribute__((noinline))
#else
static void PQ_Vector16BiquadDf2FP(float *pSrc, float *pDst)
#endif
{
    PQ_Initiate_Vector_Func(pSrc, pDst);
    PQ_DF2_Vector8_FP(false, false);
    PQ_DF2_Vector8_FP(true, true);
    PQ_End_Vector_Func();
}

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector16BiquadCascadeDf2FP(float *pSrc, float *pDst) __attribute__((noinline))
#else
static void PQ_Vector16BiquadCascadeDf2FP(float *pSrc, float *pDst)
#endif
{
    PQ_Initiate_Vector_Func(pSrc, pDst);
    PQ_DF2_Cascade_Vector8_FP(false, false);
    PQ_DF2_Cascade_Vector8_FP(true, true);
    PQ_End_Vector_Func();
}

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32EtoxFP(float *pSrc, float *pDst) __attribute__((noinline))
#else
static void PQ_Vector32EtoxFP(float *pSrc, float *pDst)
#endif
{
    PQ_Primitive_Vector32(FP, PQ_ETOX_INF);
}

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32InvSqrtFP(float *pSrc, float *pDst) __attribute__((noinline))
#else
static void PQ_Vector32InvSqrtFP(float *pSrc, float *pDst)
#endif
{
    PQ_Primitive_Vector32(FP, PQ_ISQRT_INF);
}

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32SqrtFX(int32_t *pSrc, int32_t *pDst) __attribute__((noinline))
#else
static void PQ_Vector32SqrtFX(int32_t *pSrc, int32_t *pDst)
#endif
{
    PQ_Primitive_Vector32(FX, PQ_SQRT_INF);
}

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32SqrtFP(float *pSrc, float *pDst) __attribute__((noinline))
#else
static void PQ_Vector32SqrtFP(float *pSrc, float *pDst)
#endif
{
    PQ_Primitive_Vector32(FP, PQ_SQRT_INF);
}

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32InvFX(int32_t *pSrc, int32_t *pDst) __attribute__((noinline))
#else
static void PQ_Vector32InvFX(int32_t *pSrc, int32_t *pDst)
#endif
{
    PQ_Primitive_Vector32(FX, PQ_INV_INF);
}

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32InvFP(float *pSrc, float *pDst) __attribute__((noinline))
#else
static void PQ_Vector32InvFP(float *pSrc, float *pDst)
#endif
{
    PQ_Primitive_Vector32(FP, PQ_INV_INF);
}

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32LnFX(int32_t *pSrc, int32_t *pDst) __attribute__((noinline))
#else
static void PQ_Vector32LnFX(int32_t *pSrc, int32_t *pDst)
#endif
{
    PQ_Primitive_Vector32(FX, PQ_LN_INF);
}

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32LnFP(float *pSrc, float *pDst) __attribute__((noinline))
#else
static void PQ_Vector32LnFP(float *pSrc, float *pDst)
#endif
{
    PQ_Primitive_Vector32(FP, PQ_LN_INF);
}

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32InvSqrtFX(int32_t *pSrc, int32_t *pDst) __attribute__((noinline))
#else
static void PQ_Vector32InvSqrtFX(int32_t *pSrc, int32_t *pDst)
#endif
{
    PQ_Primitive_Vector32(FX, PQ_ISQRT_INF);
}

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32EtoxFX(int32_t *pSrc, int32_t *pDst) __attribute__((noinline))
#else
static void PQ_Vector32EtoxFX(int32_t *pSrc, int32_t *pDst)
#endif
{
    PQ_Primitive_Vector32(FX, PQ_ETOX_INF);
}

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32EtonxFP(float *pSrc, float *pDst) __attribute__((noinline))
#else
static void PQ_Vector32EtonxFP(float *pSrc, float *pDst)
#endif
{
    PQ_Primitive_Vector32(FP, PQ_ETONX_INF);
}

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32EtonxFX(int32_t *pSrc, int32_t *pDst) __attribute__((noinline))
#else
static void PQ_Vector32EtonxFX(int32_t *pSrc, int32_t *pDst)
#endif
{
    PQ_Primitive_Vector32(FX, PQ_ETONX_INF);
}
