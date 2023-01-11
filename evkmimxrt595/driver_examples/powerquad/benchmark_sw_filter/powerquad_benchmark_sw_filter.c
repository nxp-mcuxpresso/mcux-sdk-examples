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

    PRINTF("\r\nCMSIS DSP benchmark software filter test start.\r\n");

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

    PRINTF("\r\nCMSIS DSP benchmark software filter test succeeded.\r\n");

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
        EXAMPLE_ASSERT_TRUE(abs(FIRRef[i] - FIRResult[i]) <= 10);
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
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
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
