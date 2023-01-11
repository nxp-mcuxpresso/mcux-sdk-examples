/*
 * Copyright 2019 NXP
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

/* CMSIS DSP compatible FIR serials*/
static void arm_fir_q15Test(void);
static void arm_fir_q31Test(void);
static void arm_fir_f32Test(void);
static void arm_conv_q15Test(void);
static void arm_conv_q31Test(void);
static void arm_conv_f32Test(void);
static void arm_correlate_q15Test(void);
static void arm_correlate_q31Test(void);
static void arm_correlate_f32Test(void);

/* Biquad filter. */
static void biquad_cascade_f32Test(void);

static void TEST_InitTime(void);
static uint32_t TEST_GetTime(void);

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
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Power up PQ RAM. */
    POWER_DisablePD(kPDRUNCFG_PPD_PQ_SRAM);
    /* Apply power setting. */
    POWER_ApplyPD();

    PRINTF("\r\nCMSIS DSP benchmark filter test start.\r\n");

    PQ_Init(POWERQUAD);

    TEST_InitTime();

    /* CMSIS DSP compatible FIR serials*/
    arm_fir_q15Test();
    arm_fir_q31Test();
    arm_fir_f32Test();
    arm_conv_q15Test();
    arm_conv_q31Test();
    arm_conv_f32Test();
    arm_correlate_q15Test();
    arm_correlate_q31Test();
    arm_correlate_f32Test();

    /* biquad. */
    biquad_cascade_f32Test();

    PRINTF("\r\nCMSIS DSP benchmark filter test succeeded.\r\n");

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

static const float firInput[FIR_INPUT_LEN] = {
    0.16, 0.15, 0.14, 0.13, 0.12, 0.11, 0.10, 0.09, 0.08, 0.07, 0.06, 0.05, 0.04, 0.03, 0.02, 0.01,
};

static const float firTaps[FIR_TAP_LEN] = {
    0.05, 0.03, 0.07, 0.03, 0.06, 0.03, 0.09, 0.08, 0.06, 0.11, 0.09, 0.07,
};

static const float firRef[FIR_INPUT_LEN] = {
    0.0112, 0.0249, 0.0409, 0.0478, 0.0573, 0.0676, 0.0674, 0.0717,
    0.0706, 0.0756, 0.0735, 0.0743, 0.0666, 0.0589, 0.0512, 0.0435,
};

/* Q15 FIR */
static void arm_fir_q15Test(void)
{
    q15_t dataForFIR[FIR_INPUT_LEN];
    q15_t taps[FIR_TAP_LEN];
    q15_t FIRRef[FIR_INPUT_LEN];
    q15_t FIRResult[FIR_INPUT_LEN] = {0};
    q15_t state[FIR_INPUT_LEN + FIR_TAP_LEN - 1];
    uint32_t i;

    for (i = 0; i < FIR_INPUT_LEN; i++)
    {
        dataForFIR[i] = FLOAT_2_Q15(firInput[i]);
    }

    for (i = 0; i < FIR_TAP_LEN; i++)
    {
        taps[i] = FLOAT_2_Q15(firTaps[i]);
    }

    for (i = 0; i < FIR_INPUT_LEN; i++)
    {
        FIRRef[i] = FLOAT_2_Q15(firRef[i]);
    }
    arm_fir_instance_q15 fir;

    uint32_t oldTime = TEST_GetTime();
    for (i = 0; i < FIR_TEST_LOOP; i++)
    {
        arm_fir_init_q15(&fir, FIR_TAP_LEN, taps, state, FIR_INPUT_LEN);
        arm_fir_q15(&fir, dataForFIR, FIRResult, FIR_INPUT_LEN);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (i = 0; i < ARRAY_SIZE(FIRRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(FIRRef[i] - FIRResult[i]) <= 1);
    }
}

/* Q31 FIR */
static void arm_fir_q31Test(void)
{
    q31_t dataForFIR[FIR_INPUT_LEN];
    q31_t taps[FIR_TAP_LEN];
    q31_t FIRRef[FIR_INPUT_LEN];
    q31_t FIRResult[FIR_INPUT_LEN] = {0};
    q31_t state[FIR_INPUT_LEN + FIR_TAP_LEN - 1];
    uint32_t i;

    for (i = 0; i < FIR_INPUT_LEN; i++)
    {
        dataForFIR[i] = FLOAT_2_Q31(firInput[i]);
    }

    for (i = 0; i < FIR_TAP_LEN; i++)
    {
        taps[i] = FLOAT_2_Q31(firTaps[i]);
    }

    for (i = 0; i < FIR_INPUT_LEN; i++)
    {
        FIRRef[i] = FLOAT_2_Q31(firRef[i]);
    }

    arm_fir_instance_q31 fir;

    uint32_t oldTime = TEST_GetTime();
    for (i = 0; i < FIR_TEST_LOOP; i++)
    {
        arm_fir_init_q31(&fir, FIR_TAP_LEN, taps, state, FIR_INPUT_LEN);
        arm_fir_q31(&fir, dataForFIR, FIRResult, FIR_INPUT_LEN);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (i = 0; i < ARRAY_SIZE(FIRRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(FIRRef[i] - FIRResult[i]) <= 20);
    }
}

/* Float FIR */
static void arm_fir_f32Test(void)
{
    float32_t dataForFIR[FIR_INPUT_LEN];
    float32_t taps[FIR_TAP_LEN];
    float32_t FIRRef[FIR_INPUT_LEN];
    float32_t FIRResult[FIR_INPUT_LEN] = {0};
    float32_t state[FIR_INPUT_LEN + FIR_TAP_LEN - 1];
    uint32_t i;

    for (i = 0; i < FIR_INPUT_LEN; i++)
    {
        dataForFIR[i] = firInput[i] * 100.0f;
    }

    for (i = 0; i < FIR_TAP_LEN; i++)
    {
        taps[i] = firTaps[i] * 100.0f;
    }

    for (i = 0; i < FIR_INPUT_LEN; i++)
    {
        FIRRef[i] = firRef[i] * 10000.0f;
    }

    arm_fir_instance_f32 fir;

    uint32_t oldTime = TEST_GetTime();
    for (i = 0; i < FIR_TEST_LOOP; i++)
    {
        arm_fir_init_f32(&fir, FIR_TAP_LEN, taps, state, FIR_INPUT_LEN);
        arm_fir_f32(&fir, dataForFIR, FIRResult, FIR_INPUT_LEN);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (i = 0; i < ARRAY_SIZE(FIRRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(FIRRef[i] - FIRResult[i])) < 0.0001);
    }
}

static const float convInputA[] = {0.01f, 0.02f, 0.03f, 0.04f, 0.05f};

static const float convInputB[] = {0.02f, 0.04f, 0.06f, 0.08f, 0.10f};

static const float convRef[] = {
    0.0002, 0.0008, 0.0020, 0.0040, 0.0070, 0.0088, 0.0092, 0.0080, 0.0050,
};

/* Q15 Convolution */
static void arm_conv_q15Test(void)
{
    q15_t inputA[CONV_A_LEN];
    q15_t inputB[CONV_B_LEN];
    q15_t ref[CONV_RESULT_LEN];
    q15_t result[CONV_RESULT_LEN] = {0};
    uint32_t i;

    for (i = 0; i < CONV_A_LEN; i++)
    {
        inputA[i] = FLOAT_2_Q15(convInputA[i]);
    }

    for (i = 0; i < CONV_B_LEN; i++)
    {
        inputB[i] = FLOAT_2_Q15(convInputB[i]);
    }

    for (i = 0; i < CONV_RESULT_LEN; i++)
    {
        ref[i] = FLOAT_2_Q15(convRef[i]);
    }

    uint32_t oldTime = TEST_GetTime();
    for (i = 0; i < FIR_TEST_LOOP; i++)
    {
        arm_conv_q15(inputA, CONV_A_LEN, inputB, CONV_B_LEN, result);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (i = 0; i < ARRAY_SIZE(result); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
    }
}

/* Q31 Convolution */
static void arm_conv_q31Test(void)
{
    q31_t inputA[CONV_A_LEN];
    q31_t inputB[CONV_B_LEN];
    q31_t ref[CONV_RESULT_LEN];
    q31_t result[CONV_RESULT_LEN] = {0};
    uint32_t i;

    for (i = 0; i < CONV_A_LEN; i++)
    {
        inputA[i] = FLOAT_2_Q31(convInputA[i]);
    }

    for (i = 0; i < CONV_B_LEN; i++)
    {
        inputB[i] = FLOAT_2_Q31(convInputB[i]);
    }

    for (i = 0; i < CONV_RESULT_LEN; i++)
    {
        ref[i] = FLOAT_2_Q31(convRef[i]);
    }

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < FIR_TEST_LOOP; i++)
    {
        arm_conv_q31(inputA, CONV_A_LEN, inputB, CONV_B_LEN, result);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(ref); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 10);
    }
}

/* Float Convolution */
static void arm_conv_f32Test(void)
{
    float_t inputA[CONV_A_LEN];
    float_t inputB[CONV_B_LEN];
    float_t ref[CONV_RESULT_LEN];
    float_t result[CONV_RESULT_LEN] = {0};
    uint32_t i;

    for (i = 0; i < CONV_A_LEN; i++)
    {
        inputA[i] = convInputA[i] * 100.0f;
    }

    for (i = 0; i < CONV_B_LEN; i++)
    {
        inputB[i] = convInputB[i] * 100.0f;
    }

    for (i = 0; i < CONV_RESULT_LEN; i++)
    {
        ref[i] = convRef[i] * 10000.0f;
    }

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < FIR_TEST_LOOP; i++)
    {
        arm_conv_f32(inputA, CONV_A_LEN, inputB, CONV_B_LEN, result);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(ref); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(ref[i] - result[i])) < 0.00001);
    }
}

static const float corrInputA[] = {0.01f, 0.02f, 0.03f, 0.04f, 0.05f};

static const float corrInputB[] = {0.02f, 0.04f, 0.06f, 0.08f, 0.10f};

static const float corrRef[] = {
    0.0010, 0.0028, 0.0052, 0.0080, 0.0110, 0.0080, 0.0052, 0.0028, 0.0010,
};

/* Q15 Correlation */
static void arm_correlate_q15Test(void)
{
    q15_t inputA[CORR_A_LEN];
    q15_t inputB[CORR_B_LEN];
    q15_t ref[CORR_RESULT_LEN];
    q15_t result[CORR_RESULT_LEN] = {0};
    uint32_t i;

    for (i = 0; i < CORR_A_LEN; i++)
    {
        inputA[i] = FLOAT_2_Q15(corrInputA[i]);
    }

    for (i = 0; i < CORR_B_LEN; i++)
    {
        inputB[i] = FLOAT_2_Q15(corrInputB[i]);
    }

    for (i = 0; i < CORR_RESULT_LEN; i++)
    {
        ref[i] = FLOAT_2_Q15(corrRef[i]);
    }

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < FIR_TEST_LOOP; i++)
    {
        arm_correlate_q15(inputA, CORR_A_LEN, inputB, CORR_B_LEN, result);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(ref); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
    }
}

/* Q31 Correlation */
static void arm_correlate_q31Test(void)
{
    q31_t inputA[CORR_A_LEN];
    q31_t inputB[CORR_B_LEN];
    q31_t ref[CORR_RESULT_LEN];
    q31_t result[CORR_RESULT_LEN] = {0};
    uint32_t i;

    for (i = 0; i < CORR_A_LEN; i++)
    {
        inputA[i] = FLOAT_2_Q31(corrInputA[i]);
    }

    for (i = 0; i < CORR_B_LEN; i++)
    {
        inputB[i] = FLOAT_2_Q31(corrInputB[i]);
    }

    for (i = 0; i < CORR_RESULT_LEN; i++)
    {
        ref[i] = FLOAT_2_Q31(corrRef[i]);
    }

    uint32_t oldTime = TEST_GetTime();
    for (i = 0; i < FIR_TEST_LOOP; i++)
    {
        arm_correlate_q31(inputA, CORR_A_LEN, inputB, CORR_B_LEN, result);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(ref); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 2);
    }
}

/* Float Correlation */
static void arm_correlate_f32Test(void)
{
    float_t inputA[CORR_A_LEN];
    float_t inputB[CORR_B_LEN];
    float_t ref[CORR_RESULT_LEN];
    float_t result[CORR_RESULT_LEN] = {0};
    uint32_t i;

    for (i = 0; i < CORR_A_LEN; i++)
    {
        inputA[i] = corrInputA[i] * 100.0f;
    }

    for (i = 0; i < CORR_B_LEN; i++)
    {
        inputB[i] = corrInputB[i] * 100.0f;
    }

    for (i = 0; i < CORR_RESULT_LEN; i++)
    {
        ref[i] = corrRef[i] * 10000.0f;
    }

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < FIR_TEST_LOOP; i++)
    {
        arm_correlate_f32(inputA, CORR_A_LEN, inputB, CORR_B_LEN, result);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(ref); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(ref[i] - result[i])) < 0.00001);
    }
}

static float biquadRef[] = {
    6.972e-14,  3.283e-12,  7.629e-11,  1.167e-09,  1.322e-08,  0.0000001,  0.0000009,  0.0000054,  0.0000293,
    0.0001384,  0.0005822,  0.0021996,  0.0075272,  0.0234931,  0.0672749,  0.1776533,  0.4345099,  0.9880335,
    2.0955833,  4.1573526,  7.7329249,  13.512977,  22.219593,  34.421108,  50.275739,  69.257846,  89.957492,
    110.05868,  126.57709,  136.37039,  136.8383,   126.63984,  106.20949,  77.882126,  45.538138,  13.826782,
    -12.835452, -31.215065, -39.895922, -39.433084, -32.012352, -20.749309, -8.8552886, 1.0894533,  7.5786087,
    10.271017,  9.7747168,  7.2469856,  3.9594515,  0.9530423,  -1.1485383, -2.1609053, -2.2389947, -1.7197791,
    -0.9692624, -0.2747639, 0.2028707,  0.4247202,  0.4370502,  0.3247521,  0.1720415,  0.0389814,  -0.0455571,
    -0.0790144, -0.0746308, -0.0507493, -0.0232006, -0.0017425, 0.0101189,  0.0133489,  0.0110507,  0.0065408,
    0.002241,   -0.0006423, -0.0019146, -0.0019714, -0.0013919, -0.0006704, -0.0001022, 0.0002114,  0.0002997,
    0.0002485,  0.0001449,  0.0000485,  -0.0000137, -0.0000392, -0.0000389, -0.0000262, -0.0000117, -0.000001,
    0.0000043,  0.0000054,  0.0000042,  0.0000022,  0.0000006,  -0.0000004, -0.0000007, -0.0000006, -0.0000004,
    -0.0000001, 1.515e-08,  7.835e-08,  8.070e-08,  5.465e-08,  2.455e-08,  2.764e-09,  -7.912e-09, -9.996e-09,
    -7.533e-09, -3.882e-09, -9.456e-10, 6.723e-10,  1.155e-09,  9.718e-10,  5.588e-10,  1.866e-10,  -3.966e-11,
    -1.244e-10, -1.183e-10, -7.485e-11, -3.022e-11, -5.283e-13, 1.242e-11,  1.367e-11,  9.454e-12,  4.366e-12,
    6.761e-13,  -1.130e-12,
};

static void biquad_cascade_f32Test(void)
{
    pq_biquad_cascade_df2_instance S;

    uint32_t i, j;
    uint32_t oldTime;
    uint32_t filterInitTime;
    uint32_t allTime;

    pq_biquad_state_t state[BIQUAD_CASCADE_STAGE];

    float32_t input[BIQUAD_CASCADE_DATA_LEN] = {
        1024.0,
    };
    float32_t output[BIQUAD_CASCADE_DATA_LEN];

    oldTime = TEST_GetTime();
    for (i = 0; i < BIQUAD_TEST_LOOP; i++)
    {
        memset(state, 0, sizeof(state));

        for (j = 0; j < BIQUAD_CASCADE_STAGE; j++)
        {
            state[j].param.a_1 = -0.94276158f;
            state[j].param.a_2 = 0.33326621f;
            state[j].param.b_0 = 0.09762616;
            state[j].param.b_1 = 0.19525232;
            state[j].param.b_2 = 0.09762616;
        }

        PQ_BiquadCascadeDf2Init(&S, BIQUAD_CASCADE_STAGE, (pq_biquad_state_t *)&state);
    }

    filterInitTime = TEST_GetTime() - oldTime;

    oldTime = TEST_GetTime();

    for (i = 0; i < BIQUAD_TEST_LOOP; i++)
    {
        memset(state, 0, sizeof(state));

        for (j = 0; j < BIQUAD_CASCADE_STAGE; j++)
        {
            state[j].param.a_1 = -0.94276158f;
            state[j].param.a_2 = 0.33326621f;
            state[j].param.b_0 = 0.09762616;
            state[j].param.b_1 = 0.19525232;
            state[j].param.b_2 = 0.09762616;
        }

        PQ_BiquadCascadeDf2Init(&S, BIQUAD_CASCADE_STAGE, (pq_biquad_state_t *)&state);
        PQ_BiquadCascadeDf2F32(&S, input, output, BIQUAD_CASCADE_DATA_LEN);
    }

    allTime = TEST_GetTime() - oldTime;

    PRINTF("%s: %d ms\r\n", __func__, allTime - filterInitTime);

    for (uint32_t i = 0; i < BIQUAD_CASCADE_DATA_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(output[i] - biquadRef[i])) <= 0.0002);
    }
}
