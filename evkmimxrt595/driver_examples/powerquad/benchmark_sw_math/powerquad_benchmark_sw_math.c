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

/* CMSIS DSP compatible math function*/
static void arm_sqrt_q15Test(void);
static void arm_sqrt_q31Test(void);
static void arm_sin_q15Test(void);
static void arm_sin_q31Test(void);
static void arm_sin_f32Test(void);
static void arm_cos_q15Test(void);
static void arm_cos_q31Test(void);
static void arm_cos_f32Test(void);

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

    PRINTF("\r\nCMSIS DSP benchmark software math test start.\r\n");

    TEST_InitTime();

    /* CMSIS DSP compatible math function*/
    arm_sqrt_q15Test();
    arm_sqrt_q31Test();
    arm_sin_q15Test();
    arm_sin_q31Test();
    arm_sin_f32Test();
    arm_cos_q15Test();
    arm_cos_q31Test();
    arm_cos_f32Test();

    PRINTF("\r\nCMSIS DSP benchmark software math test succeeded.\r\n");

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

/* Q15 sqrt */
static void arm_sqrt_q15Test(void)
{
    q15_t inputValue = FLOAT_2_Q15(0.25);
    q15_t sqrtResult = 0.0f;
    q15_t sqrtRef    = FLOAT_2_Q15(0.5);

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATH_TEST_LOOP; i++)
    {
        arm_sqrt_q15(inputValue, &sqrtResult);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    EXAMPLE_ASSERT_TRUE(abs(sqrtRef - sqrtResult) <= 2);
}

/* Q31 sqrt */
static void arm_sqrt_q31Test(void)
{
    q31_t inputValue = FLOAT_2_Q31(0.25);
    q31_t sqrtResult = 0.0f;
    q31_t sqrtRef    = FLOAT_2_Q31(0.5);

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATH_TEST_LOOP; i++)
    {
        arm_sqrt_q31(inputValue, &sqrtResult);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    EXAMPLE_ASSERT_TRUE(abs(sqrtRef - sqrtResult) <= 2);
}

/* Q15 sin */
static void arm_sin_q15Test(void)
{
    /* sin(pi/6) = 1/2. */
    q15_t inputValue = FLOAT_2_Q15(0.5f / 6.0f);
    q15_t sinResult  = 0;
    q15_t sinRef     = FLOAT_2_Q15(0.5f);

    /* The Q15 input value is in the range [0 +0.9999] and is mapped to a radian
     * value in the range [0 2*pi) */

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATH_TEST_LOOP; i++)
    {
        sinResult = arm_sin_q15(inputValue);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    EXAMPLE_ASSERT_TRUE(abs(sinRef - sinResult) < 10);
}

/* Q31 sin */
static void arm_sin_q31Test(void)
{
    /* sin(pi/6) = 1/2. */
    q31_t inputValue = FLOAT_2_Q31(0.5f / 6.0f);
    q31_t sinResult  = 0;
    q31_t sinRef     = FLOAT_2_Q31(0.5f);

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATH_TEST_LOOP; i++)
    {
        sinResult = arm_sin_q31(inputValue);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    EXAMPLE_ASSERT_TRUE(abs(sinRef - sinResult) < 20000);
}

/* Float sin */
static void arm_sin_f32Test(void)
{
    float input;
    float Result;
    float sinRef;

    input  = 3.0;
    sinRef = 0.141120;

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATH_TEST_LOOP; i++)
    {
        Result = arm_sin_f32(input);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    EXAMPLE_ASSERT_TRUE(fabs((double)(sinRef - Result)) < 0.00001);
}

/* Q15 cos */
static void arm_cos_q15Test(void)
{
    /* cos(pi/3) = 0.5. */
    q15_t inputValue = FLOAT_2_Q15(0.5f / 3.0f);
    q15_t cosResult  = 0;
    q15_t cosRef     = FLOAT_2_Q15(0.5f);

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATH_TEST_LOOP; i++)
    {
        cosResult = arm_cos_q15(inputValue);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    EXAMPLE_ASSERT_TRUE(abs(cosRef - cosResult) < 10);
}

/* Q31 cos */
static void arm_cos_q31Test(void)
{
    /* cos(pi/3) = 0.5. */
    q31_t inputValue = FLOAT_2_Q31(0.5f / 3.0f);
    q31_t cosResult  = 0;
    q31_t cosRef     = FLOAT_2_Q31(0.5f);

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATH_TEST_LOOP; i++)
    {
        cosResult = arm_cos_q31(inputValue);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    EXAMPLE_ASSERT_TRUE(abs(cosRef - cosResult) < 20000);
}

/* Float cos */
static void arm_cos_f32Test(void)
{
    float input;
    float Result;
    float cosRef;

    input  = 1.0;
    cosRef = 0.540302;

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATH_TEST_LOOP; i++)
    {
        Result = arm_cos_f32(input);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    EXAMPLE_ASSERT_TRUE(fabs((double)(cosRef - Result)) < 0.00001);
}
