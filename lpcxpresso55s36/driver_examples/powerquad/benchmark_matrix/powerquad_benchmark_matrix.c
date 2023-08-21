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
/* CMSIS DSP compatible matrix function*/
static void arm_mat_add_q15Test(void);
static void arm_mat_add_q31Test(void);
static void arm_mat_add_f32Test(void);
static void arm_mat_sub_q15Test(void);
static void arm_mat_sub_q31Test(void);
static void arm_mat_sub_f32Test(void);
static void arm_mat_mult_q15Test(void);
static void arm_mat_mult_q31Test(void);
static void arm_mat_mult_f32Test(void);
static void arm_mat_inverse_f32Test(void);
static void arm_mat_trans_q15Test(void);
static void arm_mat_trans_q31Test(void);
static void arm_mat_trans_f32Test(void);
static void arm_mat_scale_q15Test(void);
static void arm_mat_scale_q31Test(void);
static void arm_mat_scale_f32Test(void);

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
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    PRINTF("\r\nCMSIS DSP benchmark matrix test start.\r\n");

    PQ_Init(POWERQUAD);

    TEST_InitTime();

    /* CMSIS DSP compatible matrix function*/
    arm_mat_add_q15Test();
    arm_mat_add_q31Test();
    arm_mat_add_f32Test();
    arm_mat_sub_q15Test();
    arm_mat_sub_q31Test();
    arm_mat_sub_f32Test();
    arm_mat_mult_q15Test();
    arm_mat_mult_q31Test();
    arm_mat_mult_f32Test();
    arm_mat_inverse_f32Test();
    arm_mat_trans_q15Test();
    arm_mat_trans_q31Test();
    arm_mat_trans_f32Test();
    arm_mat_scale_q15Test();
    arm_mat_scale_q31Test();
    arm_mat_scale_f32Test();

    PRINTF("\r\nCMSIS DSP benchmark matrix test succeeded.\r\n");

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

/* Q15 Matrix Addition */
static void arm_mat_add_q15Test(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_matrix_instance_q15 addMatrixA15;
    arm_matrix_instance_q15 addMatrixB15;
    arm_matrix_instance_q15 addMatrixR;

    q15_t A15[MATRIX_LEN]       = {1, 2, 3, 4};
    q15_t B15[MATRIX_LEN]       = {1, 2, 3, 4};
    q15_t addResult[MATRIX_LEN] = {0};
    q15_t addRef[MATRIX_LEN]    = {2, 4, 6, 8};

    /* Initialise Matrix Instance addMatrixM1 with numRows, numCols and data array(M1) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_q15(&addMatrixA15, srcRows, srcColumns, A15);

    /* Initialise Matrix Instance addMatrixM1 with numRows, numCols and data array(M1) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_q15(&addMatrixB15, srcRows, srcColumns, B15);

    /* Initialise Matrix Instance addMatrixM1 with numRows, numCols and data array(M1) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATRIX_TEST_LOOP; i++)
    {
        arm_mat_init_q15(&addMatrixR, srcRows, srcColumns, addResult);
        arm_mat_add_q15(&addMatrixA15, &addMatrixB15, &addMatrixR);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(addRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(addRef[i] == addMatrixR.pData[i]);
    }
}

/* Q31 Matrix Addition */
static void arm_mat_add_q31Test(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_matrix_instance_q31 addMatrixA31;
    arm_matrix_instance_q31 addMatrixB31;
    arm_matrix_instance_q31 addMatrixR;

    q31_t A31[MATRIX_LEN]       = {1, 2, 3, 4};
    q31_t B31[MATRIX_LEN]       = {1, 2, 3, 4};
    q31_t addResult[MATRIX_LEN] = {0};
    q31_t addRef[MATRIX_LEN]    = {2, 4, 6, 8};

    /* Initialise Matrix Instance addMatrixM1 with numRows, numCols and data array(M1) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_q31(&addMatrixA31, srcRows, srcColumns, A31);

    /* Initialise Matrix Instance addMatrixM1 with numRows, numCols and data array(M1) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_q31(&addMatrixB31, srcRows, srcColumns, B31);

    /* Initialise Matrix Instance addMatrixM1 with numRows, numCols and data array(M1) */
    srcRows          = MATRIX_ROW;
    srcColumns       = MATRIX_COL;
    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATRIX_TEST_LOOP; i++)
    {
        arm_mat_init_q31(&addMatrixR, srcRows, srcColumns, addResult);
        arm_mat_add_q31(&addMatrixA31, &addMatrixB31, &addMatrixR);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(addRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(addRef[i] == addMatrixR.pData[i]);
    }
}

/* Float Matrix Addition */
static void arm_mat_add_f32Test(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_matrix_instance_f32 addMatrixM1;
    arm_matrix_instance_f32 addMatrixM2;
    arm_matrix_instance_f32 addMatrixR;

    float32_t M1[MATRIX_LEN]        = {11.3, 22.0, 33.0, 44.0};
    float32_t M2[MATRIX_LEN]        = {1.4, 2.0, 3.0, 4.0};
    float32_t addResult[MATRIX_LEN] = {0.0};
    float32_t addRef[MATRIX_LEN]    = {12.7, 24.0, 36.0, 48.0};

    /* Initialise Matrix Instance addMatrixM1 with numRows, numCols and data array(M1) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_f32(&addMatrixM1, srcRows, srcColumns, M1);

    /* Initialise Matrix Instance addMatrixM1 with numRows, numCols and data array(M1) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_f32(&addMatrixM2, srcRows, srcColumns, M2);

    /* Initialise Matrix Instance addMatrixM1 with numRows, numCols and data array(M1) */
    srcRows          = MATRIX_ROW;
    srcColumns       = MATRIX_COL;
    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATRIX_TEST_LOOP; i++)
    {
        arm_mat_init_f32(&addMatrixR, srcRows, srcColumns, addResult);
        arm_mat_add_f32(&addMatrixM1, &addMatrixM2, &addMatrixR);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(addRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(addRef[i] == addMatrixR.pData[i]);
    }
}

/* Q15 Matrix Subtraction */
static void arm_mat_sub_q15Test(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_matrix_instance_q15 subMatrixA15;
    arm_matrix_instance_q15 subMatrixB15;
    arm_matrix_instance_q15 subMatrixR;

    q15_t A15[MATRIX_LEN]       = {2, 4, 6, 8};
    q15_t B15[MATRIX_LEN]       = {1, 2, 3, 4};
    q15_t subResult[MATRIX_LEN] = {0};
    q15_t subRef[MATRIX_LEN]    = {1, 2, 3, 4};

    /* Initialise Matrix Instance subMatrixA15 with numRows, numCols and data array(A15) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_q15(&subMatrixA15, srcRows, srcColumns, A15);

    /* Initialise Matrix Instance subMatrixB15 with numRows, numCols and data array(B15) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_q15(&subMatrixB15, srcRows, srcColumns, B15);

    /* Initialise Matrix Instance subMatrixR with numRows, numCols and data array(subResult) */
    srcRows          = MATRIX_ROW;
    srcColumns       = MATRIX_COL;
    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATRIX_TEST_LOOP; i++)
    {
        arm_mat_init_q15(&subMatrixR, srcRows, srcColumns, subResult);
        arm_mat_sub_q15(&subMatrixA15, &subMatrixB15, &subMatrixR);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(subRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(subRef[i] == subMatrixR.pData[i]);
    }
}

/* Q31 Matrix Subtraction */
static void arm_mat_sub_q31Test(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_matrix_instance_q31 subMatrixA31;
    arm_matrix_instance_q31 subMatrixB31;
    arm_matrix_instance_q31 subMatrixR;

    q31_t A31[MATRIX_LEN]       = {2, 4, 6, 8};
    q31_t B31[MATRIX_LEN]       = {1, 2, 3, 4};
    q31_t subResult[MATRIX_LEN] = {0};
    q31_t subRef[MATRIX_LEN]    = {1, 2, 3, 4};

    /* Initialise Matrix Instance subMatrixA31 with numRows, numCols and data array(A31) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_q31(&subMatrixA31, srcRows, srcColumns, A31);

    /* Initialise Matrix Instance subMatrixB31 with numRows, numCols and data array(B31) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_q31(&subMatrixB31, srcRows, srcColumns, B31);

    /* Initialise Matrix Instance subMatrixR with numRows, numCols and data array(subResult) */
    srcRows          = MATRIX_ROW;
    srcColumns       = MATRIX_COL;
    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATRIX_TEST_LOOP; i++)
    {
        arm_mat_init_q31(&subMatrixR, srcRows, srcColumns, subResult);
        arm_mat_sub_q31(&subMatrixA31, &subMatrixB31, &subMatrixR);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(subRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(subRef[i] == subMatrixR.pData[i]);
    }
}

/* Float Matrix Subtraction */
static void arm_mat_sub_f32Test(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_matrix_instance_f32 subMatrixM1;
    arm_matrix_instance_f32 subMatrixM2;
    arm_matrix_instance_f32 subMatrixR;

    float32_t M1[MATRIX_LEN]        = {11.0, 22.0, 33.0, 44.0};
    float32_t M2[MATRIX_LEN]        = {1.0, 2.0, 3.0, 4.0};
    float32_t subResult[MATRIX_LEN] = {0.0};
    float32_t subRef[MATRIX_LEN]    = {10.0, 20.0, 30.0, 40.0};

    /* Initialise Matrix Instance subMatrixM1 with numRows, numCols and data array(M1) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_f32(&subMatrixM1, srcRows, srcColumns, M1);

    /* Initialise Matrix Instance subMatrixM2 with numRows, numCols and data array(M1) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_f32(&subMatrixM2, srcRows, srcColumns, M2);

    /* Initialise Matrix Instance subMatrixR with numRows, numCols and data array(M1) */
    srcRows          = MATRIX_ROW;
    srcColumns       = MATRIX_COL;
    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATRIX_TEST_LOOP; i++)
    {
        arm_mat_init_f32(&subMatrixR, srcRows, srcColumns, subResult);
        arm_mat_sub_f32(&subMatrixM1, &subMatrixM2, &subMatrixR);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(subRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(subRef[i] == subMatrixR.pData[i]);
    }
}

/* Q15 Matrix Multiplication */
static void arm_mat_mult_q15Test(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_matrix_instance_q15 multMatrixA15;
    arm_matrix_instance_q15 multMatrixB15;
    arm_matrix_instance_q15 multMatrixR;

    q15_t A15[MATRIX_LEN]        = {0};
    q15_t B15[MATRIX_LEN]        = {0};
    q15_t multResult[MATRIX_LEN] = {0};
    q15_t multRef[MATRIX_LEN]    = {0};

    /* 0.5I * 0.5I = 0.25I */
    for (uint32_t i = 0; i < MATRIX_ROW; i++)
    {
        A15[(MATRIX_COL + 1) * i]     = 1 << 14; /* 0.5 */
        B15[(MATRIX_COL + 1) * i]     = 1 << 14; /* 0.5 */
        multRef[(MATRIX_COL + 1) * i] = 1 << 13; /* 0.25 */
    }

    /* Initialise Matrix Instance multMatrixA15 with numRows, numCols and data array(A15) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_q15(&multMatrixA15, srcRows, srcColumns, A15);

    /* Initialise Matrix Instance multMatrixB15 with numRows, numCols and data array(B15) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_q15(&multMatrixB15, srcRows, srcColumns, B15);

    /* Initialise Matrix Instance multMatrixR with numRows, numCols and data array(multResult) */
    srcRows          = MATRIX_ROW;
    srcColumns       = MATRIX_COL;
    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATRIX_TEST_LOOP; i++)
    {
        arm_mat_init_q15(&multMatrixR, srcRows, srcColumns, multResult);
        arm_mat_mult_q15(&multMatrixA15, &multMatrixB15, &multMatrixR, NULL);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(multRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(multRef[i] - multMatrixR.pData[i]) <= 2);
    }
}

/* Q31 Matrix Multiplication */
static void arm_mat_mult_q31Test(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_matrix_instance_q31 multMatrixA31;
    arm_matrix_instance_q31 multMatrixB31;
    arm_matrix_instance_q31 multMatrixR;

    q31_t A31[MATRIX_LEN]        = {0};
    q31_t B31[MATRIX_LEN]        = {0};
    q31_t multResult[MATRIX_LEN] = {0};
    q31_t multRef[MATRIX_LEN]    = {0};

    /* 0.5I * 0.5I = 0.25I */
    for (uint32_t i = 0; i < MATRIX_ROW; i++)
    {
        A31[(MATRIX_COL + 1) * i]     = 1 << 30; /* 0.5 */
        B31[(MATRIX_COL + 1) * i]     = 1 << 30; /* 0.5 */
        multRef[(MATRIX_COL + 1) * i] = 1 << 29; /* 0.25 */
    }

    /* Initialise Matrix Instance multMatrixA31 with numRows, numCols and data array(A31) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_q31(&multMatrixA31, srcRows, srcColumns, A31);

    /* Initialise Matrix Instance multMatrixB15 with numRows, numCols and data array(B15) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_q31(&multMatrixB31, srcRows, srcColumns, B31);

    /* Initialise Matrix Instance multMatrixR with numRows, numCols and data array(multResult) */
    srcRows          = MATRIX_ROW;
    srcColumns       = MATRIX_COL;
    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATRIX_TEST_LOOP; i++)
    {
        arm_mat_init_q31(&multMatrixR, srcRows, srcColumns, multResult);
        arm_mat_mult_q31(&multMatrixA31, &multMatrixB31, &multMatrixR);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(multRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(multRef[i] - multMatrixR.pData[i]) <= 2);
    }
}

/* Float Matrix Multiplication */
static void arm_mat_mult_f32Test(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_matrix_instance_f32 multMatrixM1;
    arm_matrix_instance_f32 multMatrixM2;
    arm_matrix_instance_f32 multMatrixR;

    float32_t M1[MATRIX_LEN]         = {0};
    float32_t M2[MATRIX_LEN]         = {0};
    float32_t multResult[MATRIX_LEN] = {0};
    float32_t multRef[MATRIX_LEN]    = {0};

    /* 0.5I * 0.5I = 0.25I */
    for (uint32_t i = 0; i < MATRIX_ROW; i++)
    {
        M1[(MATRIX_COL + 1) * i]      = 0.5f;
        M2[(MATRIX_COL + 1) * i]      = 0.5f;
        multRef[(MATRIX_COL + 1) * i] = 0.25f;
    }

    /* Initialise Matrix Instance multMatrixM1 with numRows, numCols and data array(M1) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_f32(&multMatrixM1, srcRows, srcColumns, M1);

    /* Initialise Matrix Instance multMatrixM2 with numRows, numCols and data array(M2) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_f32(&multMatrixM2, srcRows, srcColumns, M2);

    /* Initialise Matrix Instance multMatrixR with numRows, numCols and data array(multResult) */
    srcRows          = MATRIX_ROW;
    srcColumns       = MATRIX_COL;
    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATRIX_TEST_LOOP; i++)
    {
        arm_mat_init_f32(&multMatrixR, srcRows, srcColumns, multResult);
        arm_mat_mult_f32(&multMatrixM1, &multMatrixM2, &multMatrixR);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(multRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(multRef[i] == multMatrixR.pData[i]);
    }
}

/* Float Matrix Inverse */
static void arm_mat_inverse_f32Test(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_matrix_instance_f32 inverseMatrix;
    arm_matrix_instance_f32 inverseMatrixR;
    uint32_t i;

    float32_t M1[9 * 9]            = {0.0f};
    float32_t inverseResult[9 * 9] = {0.0};
    float32_t inverseRef[9 * 9]    = {0.0};

    /* inv(I) = I */
    for (i = 0; i < 9; i++)
    {
        M1[i * 9 + i]         = 1.0f;
        inverseRef[i * 9 + i] = 1.0f;
    }

    /* Initialise Matrix Instance inverseMatrix with numRows, numCols and data array(M1) */
    srcRows    = 9;
    srcColumns = 9;
    arm_mat_init_f32(&inverseMatrix, srcRows, srcColumns, M1);

    /* Initialise Matrix Instance inverseMatrixR with numRows, numCols and data array(inverseResult) */
    srcRows          = 9;
    srcColumns       = 9;
    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATRIX_TEST_LOOP; i++)
    {
        arm_mat_init_f32(&inverseMatrixR, srcRows, srcColumns, inverseResult);
        arm_mat_inverse_f32(&inverseMatrix, &inverseMatrixR);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < 9 * 9; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(inverseRef[i] - inverseMatrixR.pData[i])) < 0.00001);
    }
}

/* Float Matrix Transpose */
static void arm_mat_trans_f32Test(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_matrix_instance_f32 transMatrix;
    arm_matrix_instance_f32 transMatrixR;

    float32_t transpose[MATRIX_LEN] = {0.0};
    float32_t transposeResult[MATRIX_LEN];
    float32_t transposeRel[MATRIX_LEN] = {0.0};

    for (uint32_t i = 0; i < MATRIX_ROW; i++)
    {
        transpose[i]                 = 1.0f;
        transposeRel[MATRIX_COL * i] = 1.0f;
    }

    /* Initialise Matrix Instance transMatrix with numRows, numCols and data array(transpose) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_f32(&transMatrix, srcRows, srcColumns, transpose);

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATRIX_TEST_LOOP; i++)
    {
        arm_mat_init_f32(&transMatrixR, srcRows, srcColumns, transposeResult);
        arm_mat_trans_f32(&transMatrix, &transMatrixR);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(transpose); i++)
    {
        EXAMPLE_ASSERT_TRUE(transposeRel[i] == transMatrixR.pData[i]);
    }
}

/* Q31 Matrix Transpose */
static void arm_mat_trans_q31Test(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_matrix_instance_q31 transMatrix;
    arm_matrix_instance_q31 transMatrixR;

    q31_t transpose[MATRIX_LEN]       = {0};
    q31_t transposeRel[MATRIX_LEN]    = {0};
    q31_t transposeResult[MATRIX_LEN] = {0};

    for (uint32_t i = 0; i < MATRIX_ROW; i++)
    {
        transpose[i]                 = 1 << 30; /* 0.5 */
        transposeRel[MATRIX_COL * i] = 1 << 30; /* 0.5 */
    }

    /* Initialise Matrix Instance transMatrix with numRows, numCols and data array(transpose) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_q31(&transMatrix, srcRows, srcColumns, transpose);

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATRIX_TEST_LOOP; i++)
    {
        arm_mat_init_q31(&transMatrixR, srcRows, srcColumns, transposeResult);
        arm_mat_trans_q31(&transMatrix, &transMatrixR);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(transpose); i++)
    {
        EXAMPLE_ASSERT_TRUE(transposeRel[i] == transMatrixR.pData[i]);
    }
}

/* Q15 Matrix Transpose */
static void arm_mat_trans_q15Test(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_matrix_instance_q15 transMatrix;
    arm_matrix_instance_q15 transMatrixR;

    q15_t transpose[MATRIX_LEN]       = {0};
    q15_t transposeRel[MATRIX_LEN]    = {0};
    q15_t transposeResult[MATRIX_LEN] = {0};

    for (uint32_t i = 0; i < MATRIX_ROW; i++)
    {
        transpose[i]                 = 1 << 14; /* 0.5 */
        transposeRel[MATRIX_COL * i] = 1 << 14; /* 0.5 */
    }

    /* Initialise Matrix Instance transMatrix with numRows, numCols and data array(transpose) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_q15(&transMatrix, srcRows, srcColumns, transpose);

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATRIX_TEST_LOOP; i++)
    {
        arm_mat_init_q15(&transMatrixR, srcRows, srcColumns, transposeResult);
        arm_mat_trans_q15(&transMatrix, &transMatrixR);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(transpose); i++)
    {
        EXAMPLE_ASSERT_TRUE(transposeRel[i] == transMatrixR.pData[i]);
    }
}

/* Float Matrix Scale */
static void arm_mat_scale_f32Test(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_matrix_instance_f32 scaleMatrix;
    arm_matrix_instance_f32 scaleMatrixR;

    float32_t scale[MATRIX_LEN]       = {0.0f};
    float32_t scaler                  = 2;
    float32_t scaleResult[MATRIX_LEN] = {0};
    float32_t scaleRel[MATRIX_LEN]    = {0.0f};

    for (uint32_t i = 0; i < MATRIX_LEN; i++)
    {
        scale[i]    = (float)i;
        scaleRel[i] = (float)i * scaler;
    }

    /* Initialise Matrix Instance scaleMatrix with numRows, numCols and data array(scale) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_f32(&scaleMatrix, srcRows, srcColumns, scale);

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATRIX_TEST_LOOP; i++)
    {
        arm_mat_init_f32(&scaleMatrixR, srcRows, srcColumns, scaleResult);
        arm_mat_scale_f32(&scaleMatrix, scaler, &scaleMatrixR);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(scale); i++)
    {
        EXAMPLE_ASSERT_TRUE(scaleRel[i] == scaleMatrixR.pData[i]);
    }
}

/* Q31 Matrix Scale */
static void arm_mat_scale_q31Test(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_matrix_instance_q31 scaleMatrix;
    arm_matrix_instance_q31 scaleMatrixR;

    float floatInput;
    float floatScaler = 0.5f;
    q31_t scaler      = FLOAT_2_Q31(floatScaler);
    q31_t scale[MATRIX_LEN];
    q31_t scaleResult[MATRIX_LEN] = {0};
    q31_t scaleRel[MATRIX_LEN]    = {0};

    for (uint32_t i = 0; i < MATRIX_LEN; i++)
    {
        floatInput  = (float)i / (float)MATRIX_LEN;
        scale[i]    = FLOAT_2_Q31(floatInput);
        scaleRel[i] = FLOAT_2_Q31(floatInput * floatScaler);
    }

    /* Initialise Matrix Instance scaleMatrix with numRows, numCols and data array(scale) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_q31(&scaleMatrix, srcRows, srcColumns, scale);

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATRIX_TEST_LOOP; i++)
    {
        arm_mat_init_q31(&scaleMatrixR, srcRows, srcColumns, scaleResult);
        arm_mat_scale_q31(&scaleMatrix, scaler, 0, &scaleMatrixR);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(scale); i++)
    {
        EXAMPLE_ASSERT_TRUE(scaleRel[i] == scaleMatrixR.pData[i]);
    }
}

/* Q15 Matrix Scale */
static void arm_mat_scale_q15Test(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_matrix_instance_q15 scaleMatrix;
    arm_matrix_instance_q15 scaleMatrixR;

    float floatInput;
    float floatScaler = 0.5f;
    q15_t scaler      = FLOAT_2_Q15(floatScaler);
    q15_t scale[MATRIX_LEN];
    q15_t scaleResult[MATRIX_LEN] = {0};
    q15_t scaleRel[MATRIX_LEN]    = {0};

    for (uint32_t i = 0; i < MATRIX_LEN; i++)
    {
        floatInput  = (float)i / (float)MATRIX_LEN;
        scale[i]    = FLOAT_2_Q15(floatInput);
        scaleRel[i] = FLOAT_2_Q15(floatInput * floatScaler);
    }

    /* Initialise Matrix Instance scaleMatrix with numRows, numCols and data array(scale) */
    srcRows    = MATRIX_ROW;
    srcColumns = MATRIX_COL;
    arm_mat_init_q15(&scaleMatrix, srcRows, srcColumns, scale);

    uint32_t oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < MATRIX_TEST_LOOP; i++)
    {
        arm_mat_init_q15(&scaleMatrixR, srcRows, srcColumns, scaleResult);
        arm_mat_scale_q15(&scaleMatrix, scaler, 0, &scaleMatrixR);
    }

    PRINTF("%s: %d ms\r\n", __func__, TEST_GetTime() - oldTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(scale); i++)
    {
        EXAMPLE_ASSERT_TRUE(scaleRel[i] == scaleMatrixR.pData[i]);
    }
}
