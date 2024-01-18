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
#include "arm_math.h"

#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_POWERQUAD POWERQUAD

#define MATH_PI        3.1415926535898
#define FLOAT_2_Q31(x) ((int32_t)((x)*2147483648.0f))
#define FLOAT_2_Q15(x) (int16_t) __SSAT(((int32_t)((x)*32768.0f)), 16)

#define RFFT_INPUT_LEN 128
#define CFFT_INPUT_LEN 128
#define IFFT_INPUT_LEN 128
#define DCT_INPUT_LEN  128

#define FIR_INPUT_LEN 16
#define FIR_TAP_LEN   12

#define CONV_A_LEN      5
#define CONV_B_LEN      5
#define CONV_RESULT_LEN (CONV_A_LEN + CONV_B_LEN - 1)

#define CORR_A_LEN      5
#define CORR_B_LEN      5
#define CORR_RESULT_LEN (CORR_A_LEN + CORR_B_LEN - 1)

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
static void arm_sqrt_q15Example(void);
static void arm_sqrt_q31Example(void);
static void arm_sin_q15Example(void);
static void arm_sin_q31Example(void);
static void arm_sin_f32Example(void);
static void arm_cos_q15Example(void);
static void arm_cos_q31Example(void);
static void arm_cos_f32Example(void);

static void arm_mat_add_q15Example(void);
static void arm_mat_add_q31Example(void);
static void arm_mat_add_f32Example(void);
static void arm_mat_sub_q15Example(void);
static void arm_mat_sub_q31Example(void);
static void arm_mat_sub_f32Example(void);
static void arm_mat_mult_q15Example(void);
static void arm_mat_mult_q31Example(void);
static void arm_mat_mult_f32Example(void);
static void arm_mat_inverse_f32Example(void);
static void arm_mat_trans_q15Example(void);
static void arm_mat_trans_q31Example(void);
static void arm_mat_trans_f32Example(void);
static void arm_mat_scale_q15Example(void);
static void arm_mat_scale_q31Example(void);
static void arm_mat_scale_f32Example(void);

static void arm_rfft_q15Example(void);
static void arm_rfft_q31Example(void);
static void arm_cfft_q15Example(void);
static void arm_cfft_q31Example(void);
static void arm_ifft_q15Example(void);
static void arm_ifft_q31Example(void);
static void arm_dct4_q15Example(void);
static void arm_dct4_q31Example(void);

static void arm_fir_q15Example(void);
static void arm_fir_q31Example(void);
static void arm_fir_f32Example(void);
static void arm_conv_q15Example(void);
static void arm_conv_q31Example(void);
static void arm_conv_f32Example(void);
static void arm_correlate_q15Example(void);
static void arm_correlate_q31Example(void);
static void arm_correlate_f32Example(void);

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

    PRINTF("POWERQUAD CMSIS DSP example started\r\n");

    PQ_Init(DEMO_POWERQUAD);

    arm_sqrt_q15Example();
    arm_sqrt_q31Example();
    arm_sin_q15Example();
    arm_sin_q31Example();
    arm_sin_f32Example();
    arm_cos_q15Example();
    arm_cos_q31Example();
    arm_cos_f32Example();

    arm_mat_add_q15Example();
    arm_mat_add_q31Example();
    arm_mat_add_f32Example();
    arm_mat_sub_q15Example();
    arm_mat_sub_q31Example();
    arm_mat_sub_f32Example();
    arm_mat_mult_q15Example();
    arm_mat_mult_q31Example();
    arm_mat_mult_f32Example();
    arm_mat_inverse_f32Example();
    arm_mat_trans_q15Example();
    arm_mat_trans_q31Example();
    arm_mat_trans_f32Example();
    arm_mat_scale_q15Example();
    arm_mat_scale_q31Example();
    arm_mat_scale_f32Example();

    arm_rfft_q15Example();
    arm_rfft_q31Example();
    arm_cfft_q15Example();
    arm_cfft_q31Example();
    arm_ifft_q15Example();
    arm_ifft_q31Example();
    arm_dct4_q15Example();
    arm_dct4_q31Example();

    arm_fir_q15Example();
    arm_fir_q31Example();
    arm_fir_f32Example();
    arm_conv_q15Example();
    arm_conv_q31Example();
    arm_conv_f32Example();
    arm_correlate_q15Example();
    arm_correlate_q31Example();
    arm_correlate_f32Example();

    PRINTF("POWERQUAD CMSIS DSP successed\r\n");

    while (1)
    {
    }
}

/* Q15 sqrt */
static void arm_sqrt_q15Example(void)
{
    q15_t input      = FLOAT_2_Q15(0.25f);
    q15_t sqrtResult = 0;
    q15_t sqrtRef    = FLOAT_2_Q15(0.5f);

    arm_sqrt_q15(input, &sqrtResult);

    EXAMPLE_ASSERT_TRUE(abs(sqrtRef - sqrtResult) <= 2);
}

/* Q31 sqrt */
static void arm_sqrt_q31Example(void)
{
    q31_t input      = FLOAT_2_Q31(0.25f);
    q31_t sqrtResult = 0;
    q31_t sqrtRef    = FLOAT_2_Q31(0.5f);

    arm_sqrt_q31(input, &sqrtResult);

    EXAMPLE_ASSERT_TRUE(abs(sqrtRef - sqrtResult) <= 2);
}

/* Q15 sin */
static void arm_sin_q15Example(void)
{
    /* sin(pi/6) = 1/2. */
    q15_t input     = FLOAT_2_Q15(0.5f / 6.0f);
    q15_t sinResult = 0;
    q15_t sinRef    = FLOAT_2_Q15(0.5f);

    /* The Q15 input value is in the range [0 +0.9999] and is mapped to a radian
     * value in the range [0 2*pi) */
    sinResult = arm_sin_q15(input);

    EXAMPLE_ASSERT_TRUE(abs(sinRef - sinResult) < 10);
}

/* Q31 sin */
static void arm_sin_q31Example(void)
{
    /* sin(pi/6) = 1/2. */
    q31_t inputValue = FLOAT_2_Q31(0.5f / 6.0f);
    q31_t sinResult  = 0;
    q31_t sinRef     = FLOAT_2_Q31(0.5f);

    sinResult = arm_sin_q31(inputValue);

    EXAMPLE_ASSERT_TRUE(abs(sinRef - sinResult) < 20000);
}

/* Float sin */
static void arm_sin_f32Example(void)
{
    float input;
    float Result;
    float sinRef;

    input  = 3.0;
    sinRef = 0.141120;

    Result = arm_sin_f32(input);

    EXAMPLE_ASSERT_TRUE(fabs((double)(sinRef - Result)) < 0.00001);
}

/* Q15 cos */
static void arm_cos_q15Example(void)
{
    /* cos(pi/3) = 0.5. */
    q15_t inputValue = FLOAT_2_Q15(0.5f / 3.0f);
    q15_t cosResult  = 0;
    q15_t cosRef     = FLOAT_2_Q15(0.5f);

    cosResult = arm_cos_q15(inputValue);

    EXAMPLE_ASSERT_TRUE(abs(cosRef - cosResult) < 10);
}

/* Q31 cos */
static void arm_cos_q31Example(void)
{
    /* cos(pi/3) = 0.5. */
    q31_t inputValue = FLOAT_2_Q31(0.5f / 3.0f);
    q31_t cosResult  = 0;
    q31_t cosRef     = FLOAT_2_Q31(0.5f);

    cosResult = arm_cos_q31(inputValue);

    EXAMPLE_ASSERT_TRUE(abs(cosRef - cosResult) < 20000);
}

/* Float cos */
static void arm_cos_f32Example(void)
{
    float input;
    float Result;
    float cosRef;

    input  = 1.0;
    cosRef = 0.540302;

    Result = arm_cos_f32(input);

    EXAMPLE_ASSERT_TRUE(fabs((double)(cosRef - Result)) < 0.00001);
}

/* Q15 Matrix Addition */
static void arm_mat_add_q15Example(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_status status;
    arm_matrix_instance_q15 addMatrixA15;
    arm_matrix_instance_q15 addMatrixB15;
    arm_matrix_instance_q15 addMatrixR;

    q15_t A15[4]       = {1, 2, 3, 4};
    q15_t B15[4]       = {1, 2, 3, 4};
    q15_t addResult[4] = {0};
    q15_t addRef[4]    = {2, 4, 6, 8};

    /* Initialise Matrix Instance addMatrixM1 with numRows, numCols and data array(M1) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q15(&addMatrixA15, srcRows, srcColumns, A15);

    /* Initialise Matrix Instance addMatrixM1 with numRows, numCols and data array(M1) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q15(&addMatrixB15, srcRows, srcColumns, B15);

    /* Initialise Matrix Instance addMatrixM1 with numRows, numCols and data array(M1) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q15(&addMatrixR, srcRows, srcColumns, addResult);

    status = arm_mat_add_q15(&addMatrixA15, &addMatrixB15, &addMatrixR);
    EXAMPLE_ASSERT_TRUE(status == ARM_MATH_SUCCESS);

    for (uint32_t i = 0; i < 4; i++)
    {
        EXAMPLE_ASSERT_TRUE(addRef[i] == addMatrixR.pData[i]);
    }
}

/* Q31 Matrix Addition */
static void arm_mat_add_q31Example(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_status status;
    arm_matrix_instance_q31 addMatrixA31;
    arm_matrix_instance_q31 addMatrixB31;
    arm_matrix_instance_q31 addMatrixR;

    q31_t A31[4]       = {1, 2, 3, 4};
    q31_t B31[4]       = {1, 2, 3, 4};
    q31_t addResult[4] = {0};
    q31_t addRef[4]    = {2, 4, 6, 8};

    /* Initialise Matrix Instance addMatrixM1 with numRows, numCols and data array(M1) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q31(&addMatrixA31, srcRows, srcColumns, A31);

    /* Initialise Matrix Instance addMatrixM1 with numRows, numCols and data array(M1) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q31(&addMatrixB31, srcRows, srcColumns, B31);

    /* Initialise Matrix Instance addMatrixM1 with numRows, numCols and data array(M1) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q31(&addMatrixR, srcRows, srcColumns, addResult);

    status = arm_mat_add_q31(&addMatrixA31, &addMatrixB31, &addMatrixR);
    EXAMPLE_ASSERT_TRUE(status == ARM_MATH_SUCCESS);

    for (uint32_t i = 0; i < 4; i++)
    {
        EXAMPLE_ASSERT_TRUE(addRef[i] == addMatrixR.pData[i]);
    }
}

/* Float Matrix Addition */
static void arm_mat_add_f32Example(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_status status;
    arm_matrix_instance_f32 addMatrixM1;
    arm_matrix_instance_f32 addMatrixM2;
    arm_matrix_instance_f32 addMatrixR;

    float32_t M1[4]        = {11.3, 22.0, 33.0, 44.0};
    float32_t M2[4]        = {1.4, 2.0, 3.0, 4.0};
    float32_t addResult[4] = {0.0};
    float32_t addRef[4]    = {12.7, 24.0, 36.0, 48.0};

    /* Initialise Matrix Instance addMatrixM1 with numRows, numCols and data array(M1) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_f32(&addMatrixM1, srcRows, srcColumns, M1);

    /* Initialise Matrix Instance addMatrixM1 with numRows, numCols and data array(M1) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_f32(&addMatrixM2, srcRows, srcColumns, M2);

    /* Initialise Matrix Instance addMatrixM1 with numRows, numCols and data array(M1) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_f32(&addMatrixR, srcRows, srcColumns, addResult);

    status = arm_mat_add_f32(&addMatrixM1, &addMatrixM2, &addMatrixR);
    EXAMPLE_ASSERT_TRUE(status == ARM_MATH_SUCCESS);

    for (uint32_t i = 0; i < 4; i++)
    {
        EXAMPLE_ASSERT_TRUE(addRef[i] == addMatrixR.pData[i]);
    }
}

/* Q15 Matrix Subtraction */
static void arm_mat_sub_q15Example(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_status status;
    arm_matrix_instance_q15 subMatrixA15;
    arm_matrix_instance_q15 subMatrixB15;
    arm_matrix_instance_q15 subMatrixR;

    q15_t A15[4]       = {2, 4, 6, 8};
    q15_t B15[4]       = {1, 2, 3, 4};
    q15_t subResult[4] = {0};
    q15_t subRef[4]    = {1, 2, 3, 4};

    /* Initialise Matrix Instance subMatrixA15 with numRows, numCols and data array(A15) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q15(&subMatrixA15, srcRows, srcColumns, A15);

    /* Initialise Matrix Instance subMatrixB15 with numRows, numCols and data array(B15) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q15(&subMatrixB15, srcRows, srcColumns, B15);

    /* Initialise Matrix Instance subMatrixR with numRows, numCols and data array(subResult) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q15(&subMatrixR, srcRows, srcColumns, subResult);

    status = arm_mat_sub_q15(&subMatrixA15, &subMatrixB15, &subMatrixR);
    EXAMPLE_ASSERT_TRUE(status == ARM_MATH_SUCCESS);

    for (uint32_t i = 0; i < 4; i++)
    {
        EXAMPLE_ASSERT_TRUE(subRef[i] == subMatrixR.pData[i]);
    }
}

/* Q31 Matrix Subtraction */
static void arm_mat_sub_q31Example(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_status status;
    arm_matrix_instance_q31 subMatrixA31;
    arm_matrix_instance_q31 subMatrixB31;
    arm_matrix_instance_q31 subMatrixR;

    q31_t A31[4]       = {2, 4, 6, 8};
    q31_t B31[4]       = {1, 2, 3, 4};
    q31_t subResult[4] = {0};
    q31_t subRef[4]    = {1, 2, 3, 4};

    /* Initialise Matrix Instance subMatrixA31 with numRows, numCols and data array(A31) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q31(&subMatrixA31, srcRows, srcColumns, A31);

    /* Initialise Matrix Instance subMatrixB31 with numRows, numCols and data array(B31) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q31(&subMatrixB31, srcRows, srcColumns, B31);

    /* Initialise Matrix Instance subMatrixR with numRows, numCols and data array(subResult) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q31(&subMatrixR, srcRows, srcColumns, subResult);

    status = arm_mat_sub_q31(&subMatrixA31, &subMatrixB31, &subMatrixR);
    EXAMPLE_ASSERT_TRUE(status == ARM_MATH_SUCCESS);

    for (uint32_t i = 0; i < 4; i++)
    {
        EXAMPLE_ASSERT_TRUE(subRef[i] == subMatrixR.pData[i]);
    }
}

/* Float Matrix Subtraction */
static void arm_mat_sub_f32Example(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_status status;
    arm_matrix_instance_f32 subMatrixM1;
    arm_matrix_instance_f32 subMatrixM2;
    arm_matrix_instance_f32 subMatrixR;

    float32_t M1[4]        = {11.0, 22.0, 33.0, 44.0};
    float32_t M2[4]        = {1.0, 2.0, 3.0, 4.0};
    float32_t subResult[4] = {0.0};
    float32_t subRef[4]    = {10.0, 20.0, 30.0, 40.0};

    /* Initialise Matrix Instance subMatrixM1 with numRows, numCols and data array(M1) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_f32(&subMatrixM1, srcRows, srcColumns, M1);

    /* Initialise Matrix Instance subMatrixM2 with numRows, numCols and data array(M1) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_f32(&subMatrixM2, srcRows, srcColumns, M2);

    /* Initialise Matrix Instance subMatrixR with numRows, numCols and data array(M1) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_f32(&subMatrixR, srcRows, srcColumns, subResult);

    status = arm_mat_sub_f32(&subMatrixM1, &subMatrixM2, &subMatrixR);
    EXAMPLE_ASSERT_TRUE(status == ARM_MATH_SUCCESS);

    for (uint32_t i = 0; i < 4; i++)
    {
        EXAMPLE_ASSERT_TRUE(subRef[i] == subMatrixR.pData[i]);
    }
}

/* Q15 Matrix Multiplication */
static void arm_mat_mult_q15Example(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_status status;
    arm_matrix_instance_q15 multMatrixA15;
    arm_matrix_instance_q15 multMatrixB15;
    arm_matrix_instance_q15 multMatrixR;

    q15_t A15[4];
    q15_t B15[4];
    q15_t multResult[4] = {0};
    q15_t multRef[4];

    A15[0] = FLOAT_2_Q15(0.01);
    A15[1] = FLOAT_2_Q15(0.02);
    A15[2] = FLOAT_2_Q15(0.03);
    A15[3] = FLOAT_2_Q15(0.04);

    B15[0] = FLOAT_2_Q15(0.01);
    B15[1] = FLOAT_2_Q15(0.02);
    B15[2] = FLOAT_2_Q15(0.03);
    B15[3] = FLOAT_2_Q15(0.04);

    multRef[0] = FLOAT_2_Q15(0.0007);
    multRef[1] = FLOAT_2_Q15(0.0010);
    multRef[2] = FLOAT_2_Q15(0.0015);
    multRef[3] = FLOAT_2_Q15(0.0022);

    /* Initialise Matrix Instance multMatrixA15 with numRows, numCols and data array(A15) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q15(&multMatrixA15, srcRows, srcColumns, A15);

    /* Initialise Matrix Instance multMatrixB15 with numRows, numCols and data array(B15) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q15(&multMatrixB15, srcRows, srcColumns, B15);

    /* Initialise Matrix Instance multMatrixR with numRows, numCols and data array(multResult) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q15(&multMatrixR, srcRows, srcColumns, multResult);

    status = arm_mat_mult_q15(&multMatrixA15, &multMatrixB15, &multMatrixR, NULL);
    EXAMPLE_ASSERT_TRUE(status == ARM_MATH_SUCCESS);

    for (uint32_t i = 0; i < 4; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(multRef[i] - multMatrixR.pData[i]) <= 1);
    }
}

/* Q31 Matrix Multiplication */
static void arm_mat_mult_q31Example(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_status status;
    arm_matrix_instance_q31 multMatrixA31;
    arm_matrix_instance_q31 multMatrixB31;
    arm_matrix_instance_q31 multMatrixR;

    q31_t A31[4];
    q31_t B31[4];
    q31_t multResult[4] = {0};
    q31_t multRef[4];

    A31[0] = FLOAT_2_Q31(0.01);
    A31[1] = FLOAT_2_Q31(0.02);
    A31[2] = FLOAT_2_Q31(0.03);
    A31[3] = FLOAT_2_Q31(0.04);

    B31[0] = FLOAT_2_Q31(0.01);
    B31[1] = FLOAT_2_Q31(0.02);
    B31[2] = FLOAT_2_Q31(0.03);
    B31[3] = FLOAT_2_Q31(0.04);

    multRef[0] = FLOAT_2_Q31(0.0007);
    multRef[1] = FLOAT_2_Q31(0.0010);
    multRef[2] = FLOAT_2_Q31(0.0015);
    multRef[3] = FLOAT_2_Q31(0.0022);

    /* Initialise Matrix Instance multMatrixA31 with numRows, numCols and data array(A31) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q31(&multMatrixA31, srcRows, srcColumns, A31);

    /* Initialise Matrix Instance multMatrixB15 with numRows, numCols and data array(B15) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q31(&multMatrixB31, srcRows, srcColumns, B31);

    /* Initialise Matrix Instance multMatrixR with numRows, numCols and data array(multResult) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q31(&multMatrixR, srcRows, srcColumns, multResult);

    status = arm_mat_mult_q31(&multMatrixA31, &multMatrixB31, &multMatrixR);
    EXAMPLE_ASSERT_TRUE(status == ARM_MATH_SUCCESS);

    for (uint32_t i = 0; i < 4; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(multRef[i] - multMatrixR.pData[i]) <= 1);
    }
}

/* Float Matrix Multiplication */
static void arm_mat_mult_f32Example(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_status status;
    arm_matrix_instance_f32 multMatrixM1;
    arm_matrix_instance_f32 multMatrixM2;
    arm_matrix_instance_f32 multMatrixR;

    float32_t M1[4]         = {1, 2, 3, 4};
    float32_t M2[4]         = {1, 2, 3, 4};
    float32_t multResult[4] = {0};
    float32_t multRef[4]    = {7, 10, 15, 22};

    /* Initialise Matrix Instance multMatrixM1 with numRows, numCols and data array(M1) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_f32(&multMatrixM1, srcRows, srcColumns, M1);

    /* Initialise Matrix Instance multMatrixM2 with numRows, numCols and data array(M2) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_f32(&multMatrixM2, srcRows, srcColumns, M2);

    /* Initialise Matrix Instance multMatrixR with numRows, numCols and data array(multResult) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_f32(&multMatrixR, srcRows, srcColumns, multResult);

    status = arm_mat_mult_f32(&multMatrixM1, &multMatrixM2, &multMatrixR);
    EXAMPLE_ASSERT_TRUE(status == ARM_MATH_SUCCESS);

    for (uint32_t i = 0; i < 4; i++)
    {
        EXAMPLE_ASSERT_TRUE(multRef[i] == multMatrixR.pData[i]);
    }
}

/* Float Matrix Inverse */
static void arm_mat_inverse_f32Example(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_status status;
    arm_matrix_instance_f32 inverseMatrix;
    arm_matrix_instance_f32 inverseMatrixR;

    float32_t M1[4]            = {1.0, 2.0, 3.0, 4.0};
    float32_t inverseResult[4] = {0.0};
    float32_t inverseRef[4]    = {-2.0, 1.0, 1.5, -0.5};

    /* Initialise Matrix Instance inverseMatrix with numRows, numCols and data array(M1) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_f32(&inverseMatrix, srcRows, srcColumns, M1);

    /* Initialise Matrix Instance inverseMatrixR with numRows, numCols and data array(inverseResult) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_f32(&inverseMatrixR, srcRows, srcColumns, inverseResult);

    status = arm_mat_inverse_f32(&inverseMatrix, &inverseMatrixR);
    EXAMPLE_ASSERT_TRUE(status == ARM_MATH_SUCCESS);

    for (uint32_t i = 0; i < 4; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(inverseRef[i] - inverseMatrixR.pData[i])) < 0.00001);
    }
}

/* Float Matrix Transpose */
static void arm_mat_trans_f32Example(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_status status;
    arm_matrix_instance_f32 transMatrix;
    arm_matrix_instance_f32 transMatrixR;

    float32_t transpose[4]       = {1.0, 2.0, 3.0, 4.0};
    float32_t transposeResult[4] = {0.0};
    float32_t transposeRel[4]    = {1.0, 3.0, 2.0, 4.0};

    /* Initialise Matrix Instance transMatrix with numRows, numCols and data array(transpose) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_f32(&transMatrix, srcRows, srcColumns, transpose);

    /* Initialise Matrix Instance transMatrixR with numRows, numCols and data array(transposeResult) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_f32(&transMatrixR, srcRows, srcColumns, transposeResult);

    status = arm_mat_trans_f32(&transMatrix, &transMatrixR);
    EXAMPLE_ASSERT_TRUE(status == ARM_MATH_SUCCESS);

    for (uint32_t i = 0; i < 4; i++)
    {
        EXAMPLE_ASSERT_TRUE(transposeRel[i] == transMatrixR.pData[i]);
    }
}

/* Q31 Matrix Transpose */
static void arm_mat_trans_q31Example(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_status status;
    arm_matrix_instance_q31 transMatrix;
    arm_matrix_instance_q31 transMatrixR;

    q31_t transpose[4]       = {1, 2, 3, 4};
    q31_t transposeResult[4] = {0};
    q31_t transposeRel[4]    = {1, 3, 2, 4};

    /* Initialise Matrix Instance transMatrix with numRows, numCols and data array(transpose) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q31(&transMatrix, srcRows, srcColumns, transpose);

    /* Initialise Matrix Instance transMatrixR with numRows, numCols and data array(transposeResult) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q31(&transMatrixR, srcRows, srcColumns, transposeResult);

    status = arm_mat_trans_q31(&transMatrix, &transMatrixR);
    EXAMPLE_ASSERT_TRUE(status == ARM_MATH_SUCCESS);

    for (uint32_t i = 0; i < 4; i++)
    {
        EXAMPLE_ASSERT_TRUE(transposeRel[i] == transMatrixR.pData[i]);
    }
}

/* Q15 Matrix Transpose */
static void arm_mat_trans_q15Example(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_status status;
    arm_matrix_instance_q15 transMatrix;
    arm_matrix_instance_q15 transMatrixR;

    q15_t transpose[4]       = {1, 2, 3, 4};
    q15_t transposeResult[4] = {0};
    q15_t transposeRel[4]    = {1, 3, 2, 4};

    /* Initialise Matrix Instance transMatrix with numRows, numCols and data array(transpose) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q15(&transMatrix, srcRows, srcColumns, transpose);

    /* Initialise Matrix Instance transMatrixR with numRows, numCols and data array(transposeResult) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q15(&transMatrixR, srcRows, srcColumns, transposeResult);

    status = arm_mat_trans_q15(&transMatrix, &transMatrixR);
    EXAMPLE_ASSERT_TRUE(status == ARM_MATH_SUCCESS);

    for (uint32_t i = 0; i < 4; i++)
    {
        EXAMPLE_ASSERT_TRUE(transposeRel[i] == transMatrixR.pData[i]);
    }
}

/* Float Matrix Scale */
static void arm_mat_scale_f32Example(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_status status;
    arm_matrix_instance_f32 scaleMatrix;
    arm_matrix_instance_f32 scaleMatrixR;

    float32_t scale[4]       = {1.1, 2.2, 3.3, 4.4};
    float32_t scaler         = 2;
    float32_t scaleResult[4] = {0};
    float32_t scaleRef[4]    = {2.2, 4.4, 6.6, 8.8};

    /* Initialise Matrix Instance scaleMatrix with numRows, numCols and data array(scale) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_f32(&scaleMatrix, srcRows, srcColumns, scale);

    /* Initialise Matrix Instance scaleMatrixR with numRows, numCols and data array(scaleResult) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_f32(&scaleMatrixR, srcRows, srcColumns, scaleResult);

    status = arm_mat_scale_f32(&scaleMatrix, scaler, &scaleMatrixR);
    EXAMPLE_ASSERT_TRUE(status == ARM_MATH_SUCCESS);

    for (uint32_t i = 0; i < 4; i++)
    {
        EXAMPLE_ASSERT_TRUE(scaleRef[i] == scaleMatrixR.pData[i]);
    }
}

/* Q31 Matrix Scale */
static void arm_mat_scale_q31Example(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_status status;
    arm_matrix_instance_q31 scaleMatrix;
    arm_matrix_instance_q31 scaleMatrixR;

    int32_t shift = 0;
    q31_t scaler  = FLOAT_2_Q31(0.5);
    q31_t scale[4];
    q31_t scaleResult[4] = {0};
    q31_t scaleRef[4];

    scale[0] = FLOAT_2_Q31(0.2f);
    scale[1] = FLOAT_2_Q31(0.4f);
    scale[2] = FLOAT_2_Q31(0.6f);
    scale[3] = FLOAT_2_Q31(0.8f);

    scaleRef[0] = FLOAT_2_Q31(0.1f);
    scaleRef[1] = FLOAT_2_Q31(0.2f);
    scaleRef[2] = FLOAT_2_Q31(0.3f);
    scaleRef[3] = FLOAT_2_Q31(0.4f);

    /* Initialise Matrix Instance scaleMatrix with numRows, numCols and data array(scale) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q31(&scaleMatrix, srcRows, srcColumns, scale);

    /* Initialise Matrix Instance scaleMatrixR with numRows, numCols and data array(scaleResult) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q31(&scaleMatrixR, srcRows, srcColumns, scaleResult);

    status = arm_mat_scale_q31(&scaleMatrix, scaler, shift, &scaleMatrixR);
    EXAMPLE_ASSERT_TRUE(status == ARM_MATH_SUCCESS);

    for (uint32_t i = 0; i < 4; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(scaleRef[i] - scaleMatrixR.pData[i]) <= 1);
    }
}

/* Q15 Matrix Scale */
static void arm_mat_scale_q15Example(void)
{
    uint32_t srcRows, srcColumns; /* Temporary variables */
    arm_status status;
    arm_matrix_instance_q15 scaleMatrix;
    arm_matrix_instance_q15 scaleMatrixR;

    int32_t shift = 0;
    q15_t scaler  = FLOAT_2_Q15(0.5);
    q15_t scale[4];
    q15_t scaleResult[4] = {0};
    q15_t scaleRef[4];

    scale[0] = FLOAT_2_Q15(0.2f);
    scale[1] = FLOAT_2_Q15(0.4f);
    scale[2] = FLOAT_2_Q15(0.6f);
    scale[3] = FLOAT_2_Q15(0.8f);

    scaleRef[0] = FLOAT_2_Q15(0.1f);
    scaleRef[1] = FLOAT_2_Q15(0.2f);
    scaleRef[2] = FLOAT_2_Q15(0.3f);
    scaleRef[3] = FLOAT_2_Q15(0.4f);

    /* Initialise Matrix Instance scaleMatrix with numRows, numCols and data array(scale) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q15(&scaleMatrix, srcRows, srcColumns, scale);

    /* Initialise Matrix Instance scaleMatrixR with numRows, numCols and data array(scaleResult) */
    srcRows    = 2;
    srcColumns = 2;
    arm_mat_init_q15(&scaleMatrixR, srcRows, srcColumns, scaleResult);

    status = arm_mat_scale_q15(&scaleMatrix, scaler, shift, &scaleMatrixR);

    EXAMPLE_ASSERT_TRUE(status == ARM_MATH_SUCCESS);

    for (uint32_t i = 0; i < 4; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(scaleRef[i] - scaleMatrixR.pData[i]) <= 1);
    }
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
static void arm_fir_q15Example(void)
{
    q15_t dataForFIR[FIR_INPUT_LEN];
    q15_t taps[FIR_TAP_LEN];
    q15_t FIRRef[FIR_INPUT_LEN];
    q15_t FIRResult[FIR_INPUT_LEN] = {0};
    q15_t state[FIR_INPUT_LEN + FIR_TAP_LEN - 1];
    arm_fir_instance_q15 fir;
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

    /* Calculate one time. */
    arm_fir_init_q15(&fir, FIR_TAP_LEN, taps, state, FIR_INPUT_LEN);
    arm_fir_q15(&fir, dataForFIR, FIRResult, FIR_INPUT_LEN);

    for (i = 0; i < ARRAY_SIZE(FIRRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(FIRRef[i] - FIRResult[i]) <= 1);
    }

    /* Incremental method. */
    memset(FIRResult, 0, sizeof(FIRResult));

    arm_fir_init_q15(&fir, FIR_TAP_LEN, taps, state, FIR_INPUT_LEN / 2);
    arm_fir_q15(&fir, dataForFIR, FIRResult, FIR_INPUT_LEN / 2);
    arm_fir_q15(&fir, dataForFIR + (FIR_INPUT_LEN / 2), FIRResult + (FIR_INPUT_LEN / 2), FIR_INPUT_LEN / 2);

    for (i = 0; i < ARRAY_SIZE(FIRRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(FIRRef[i] - FIRResult[i]) <= 1);
    }
}

/* Q31 FIR */
static void arm_fir_q31Example(void)
{
    q31_t dataForFIR[FIR_INPUT_LEN];
    q31_t taps[FIR_TAP_LEN];
    q31_t FIRRef[FIR_INPUT_LEN];
    q31_t FIRResult[FIR_INPUT_LEN] = {0};
    q31_t state[FIR_INPUT_LEN + FIR_TAP_LEN - 1];
    arm_fir_instance_q31 fir;
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

    /* Calculate one time. */
    arm_fir_init_q31(&fir, FIR_TAP_LEN, taps, state, FIR_INPUT_LEN);
    arm_fir_q31(&fir, dataForFIR, FIRResult, FIR_INPUT_LEN);

    for (i = 0; i < ARRAY_SIZE(FIRRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(FIRRef[i] - FIRResult[i]) <= 20);
    }

    /* Incremental method. */
    memset(FIRResult, 0, sizeof(FIRResult));

    arm_fir_init_q31(&fir, FIR_TAP_LEN, taps, state, FIR_INPUT_LEN / 2);
    arm_fir_q31(&fir, dataForFIR, FIRResult, FIR_INPUT_LEN / 2);
    arm_fir_q31(&fir, dataForFIR + (FIR_INPUT_LEN / 2), FIRResult + (FIR_INPUT_LEN / 2), FIR_INPUT_LEN / 2);

    for (i = 0; i < ARRAY_SIZE(FIRRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(FIRRef[i] - FIRResult[i]) <= 20);
    }
}

/* Float FIR */
static void arm_fir_f32Example(void)
{
    float32_t dataForFIR[FIR_INPUT_LEN];
    float32_t taps[FIR_TAP_LEN];
    float32_t FIRRef[FIR_INPUT_LEN];
    float32_t FIRResult[FIR_INPUT_LEN] = {0};
    float32_t state[FIR_INPUT_LEN + FIR_TAP_LEN - 1];
    arm_fir_instance_f32 fir;
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

    /* Calculate one time. */
    arm_fir_init_f32(&fir, FIR_TAP_LEN, taps, state, FIR_INPUT_LEN);
    arm_fir_f32(&fir, dataForFIR, FIRResult, FIR_INPUT_LEN);

    for (i = 0; i < ARRAY_SIZE(FIRRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(FIRRef[i] - FIRResult[i])) < 0.0001);
    }

    /* Incremental method. */
    memset(FIRResult, 0, sizeof(FIRResult));

    arm_fir_init_f32(&fir, FIR_TAP_LEN, taps, state, FIR_INPUT_LEN / 2);
    arm_fir_f32(&fir, dataForFIR, FIRResult, FIR_INPUT_LEN / 2);
    arm_fir_f32(&fir, dataForFIR + (FIR_INPUT_LEN / 2), FIRResult + (FIR_INPUT_LEN / 2), FIR_INPUT_LEN / 2);

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
static void arm_conv_q15Example(void)
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

    arm_conv_q15(inputA, CONV_A_LEN, inputB, CONV_B_LEN, result);

    for (i = 0; i < ARRAY_SIZE(result); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
    }
}

/* Q31 Convolution */
static void arm_conv_q31Example(void)
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

    arm_conv_q31(inputA, CONV_A_LEN, inputB, CONV_B_LEN, result);

    for (uint32_t i = 0; i < ARRAY_SIZE(ref); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 10);
    }
}

/* Float Convolution */
static void arm_conv_f32Example(void)
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

    arm_conv_f32(inputA, CONV_A_LEN, inputB, CONV_B_LEN, result);

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
static void arm_correlate_q15Example(void)
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

    arm_correlate_q15(inputA, CORR_A_LEN, inputB, CORR_B_LEN, result);

    for (uint32_t i = 0; i < ARRAY_SIZE(ref); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
    }
}

/* Q31 Correlation */
static void arm_correlate_q31Example(void)
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

    arm_correlate_q31(inputA, CORR_A_LEN, inputB, CORR_B_LEN, result);

    for (uint32_t i = 0; i < ARRAY_SIZE(ref); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 2);
    }
}

/* Float Correlation */
static void arm_correlate_f32Example(void)
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

    arm_correlate_f32(inputA, CORR_A_LEN, inputB, CORR_B_LEN, result);

    for (uint32_t i = 0; i < ARRAY_SIZE(ref); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(ref[i] - result[i])) < 0.00001);
    }
}

/* Q15 RFFT */
static void arm_rfft_q15Example(void)
{
    q15_t output[RFFT_INPUT_LEN * 2];
    q15_t input[RFFT_INPUT_LEN];
    q15_t ref[RFFT_INPUT_LEN * 2] = {0};
    arm_rfft_instance_q15 instance;

    /* Two full period sin wave. */
    for (uint32_t i = 0; i < RFFT_INPUT_LEN / 2; i++)
    {
        input[i]                      = arm_sin_q15(i * (0x8000 / (RFFT_INPUT_LEN / 2)));
        input[i + RFFT_INPUT_LEN / 2] = input[i];
    }

    /* Reference result. */
    ref[5]   = FLOAT_2_Q15(-0.5f); /* Imag(2) */
    ref[253] = FLOAT_2_Q15(0.5f);  /* Imag(127) */

    arm_rfft_init_q15(&instance, RFFT_INPUT_LEN, 0, 1);
    arm_rfft_q15(&instance, input, output);

    for (uint32_t i = 0; i < ARRAY_SIZE(ref); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(output[i] - ref[i]) <= 10);
    }
}

/* Q31 RFFT */
static void arm_rfft_q31Example(void)
{
    q31_t output[RFFT_INPUT_LEN * 2];
    q31_t input[RFFT_INPUT_LEN];
    q31_t ref[RFFT_INPUT_LEN * 2] = {0};
    arm_rfft_instance_q31 instance;

    /* Two full period sin wave. */
    for (uint32_t i = 0; i < RFFT_INPUT_LEN / 2; i++)
    {
        input[i]                      = arm_sin_q31(i * (0x80000000 / (RFFT_INPUT_LEN / 2))) >> 5;
        input[i + RFFT_INPUT_LEN / 2] = input[i];
    }

    /* Reference result. */
    ref[5]   = FLOAT_2_Q31(-0.5f / 32.0f); /* Imag(2) */
    ref[253] = FLOAT_2_Q31(0.5f / 32.0f);  /* Imag(127) */

    arm_rfft_init_q31(&instance, RFFT_INPUT_LEN, 0, 1);
    arm_rfft_q31(&instance, input, output);

    for (uint32_t i = 0; i < ARRAY_SIZE(ref); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(output[i] - ref[i]) <= 10);
    }
}

/* Q15 CFFT */
static void arm_cfft_q15Example(void)
{
    q15_t inout[CFFT_INPUT_LEN * 2];
    q15_t ref[CFFT_INPUT_LEN * 2] = {0};

    arm_cfft_instance_q15 instance;

    /* Don't need to initiates other items, powerquad driver does not need. */
    memset(&instance, 0, sizeof(instance));
    instance.fftLen = CFFT_INPUT_LEN;

    /* Two full period sin wave. */
    for (uint32_t i = 0; i < CFFT_INPUT_LEN / 2; i++)
    {
        inout[i * 2]     = arm_sin_q15(i * (0x8000 / (CFFT_INPUT_LEN / 2))) / 2;
        inout[i * 2 + 1] = arm_cos_q15(i * (0x8000 / (CFFT_INPUT_LEN / 2))) / 2;
    }
    memcpy(&inout[CFFT_INPUT_LEN], inout, sizeof(inout) / 2);

    /* Reference result. */
    ref[253] = FLOAT_2_Q15(0.5f); /* Imag(62) */

    arm_cfft_q15(&instance, inout, 0, 1);

    for (uint32_t i = 0; i < ARRAY_SIZE(ref); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(inout[i] - ref[i]) <= 10);
    }
}

/* Q31 CFFT */
static void arm_cfft_q31Example(void)
{
    q31_t inout[CFFT_INPUT_LEN * 2];
    q31_t ref[CFFT_INPUT_LEN * 2] = {0};
    arm_cfft_instance_q31 instance;

    /* Don't need to initiates other items, powerquad driver does not need. */
    memset(&instance, 0, sizeof(instance));
    instance.fftLen = CFFT_INPUT_LEN;

    /* Two full period sin wave. */
    for (uint32_t i = 0; i < CFFT_INPUT_LEN / 2; i++)
    {
        inout[i * 2]     = (arm_sin_q31(i * (0x80000000 / (CFFT_INPUT_LEN / 2))) / 2) >> 5;
        inout[i * 2 + 1] = (arm_cos_q31(i * (0x80000000 / (CFFT_INPUT_LEN / 2))) / 2) >> 5;
    }
    memcpy(&inout[CFFT_INPUT_LEN], inout, sizeof(inout) / 2);

    /* Reference result. */
    ref[253] = FLOAT_2_Q31(0.5f / 32.0f); /* Imag(62) */

    arm_cfft_q31(&instance, inout, 0, 1);

    for (uint32_t i = 0; i < ARRAY_SIZE(ref); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(inout[i] - ref[i]) <= 20);
    }
}

/* Q15 IFFT */
static void arm_ifft_q15Example(void)
{
    q15_t inout[IFFT_INPUT_LEN * 2] = {0};
    q15_t ref[IFFT_INPUT_LEN * 2];
    arm_cfft_instance_q15 instance;

    /* Don't need to initiates other items, powerquad driver does not need. */
    memset(&instance, 0, sizeof(instance));
    instance.fftLen = IFFT_INPUT_LEN;

    /* Reference result: Two full period sin wave. */
    /*
     * Number of bits to upscale = log2(input data size) - 1. Here the input
     * data is 2*128 = 256, so upscale bits is 7.
     */

    for (uint32_t i = 0; i < IFFT_INPUT_LEN / 2; i++)
    {
        ref[i * 2]     = (arm_sin_q15(i * (0x8000 / (IFFT_INPUT_LEN / 2))) / 2) >> 7;
        ref[i * 2 + 1] = (arm_cos_q15(i * (0x8000 / (IFFT_INPUT_LEN / 2))) / 2) >> 7;
    }
    memcpy(&ref[IFFT_INPUT_LEN], ref, sizeof(ref) / 2);

    /* Input. */
    inout[253] = FLOAT_2_Q15(0.5f); /* Imag(126) */

    arm_cfft_q15(&instance, inout, 1, 1);

    for (uint32_t i = 0; i < ARRAY_SIZE(ref); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(inout[i] - ref[i]) <= 10);
    }
}

/* Q31 IFFT */
static void arm_ifft_q31Example(void)
{
    q31_t inout[IFFT_INPUT_LEN * 2] = {0};
    q31_t ref[IFFT_INPUT_LEN * 2];
    arm_cfft_instance_q31 instance;

    /* Don't need to initiates other items, powerquad driver does not need. */
    memset(&instance, 0, sizeof(instance));
    instance.fftLen = IFFT_INPUT_LEN;

    /* Reference result: Two full period sin wave. */
    /*
     * Number of bits to upscale = log2(input data size) - 1. Here the input
     * data is 2*128 = 256, so upscale bits is 7.
     */
    /*
     * Because only low 27 bits could be used for FFT, the input data
     * should left shift 5 bits to ensure no saturation.
     */
    for (uint32_t i = 0; i < IFFT_INPUT_LEN / 2; i++)
    {
        ref[i * 2]     = ((arm_sin_q31(i * (0x80000000 / (IFFT_INPUT_LEN / 2))) / 2) >> 7) >> 5;
        ref[i * 2 + 1] = ((arm_cos_q31(i * (0x80000000 / (IFFT_INPUT_LEN / 2))) / 2) >> 7) >> 5;
    }
    memcpy(&ref[IFFT_INPUT_LEN], ref, sizeof(ref) / 2);

    /* Input. */
    inout[253] = FLOAT_2_Q31(0.5f / 32.0f); /* Imag(126) */

    arm_cfft_q31(&instance, inout, 1, 1);

    for (uint32_t i = 0; i < ARRAY_SIZE(ref); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(inout[i] - ref[i]) <= 1);
    }
}

static const float dct4RefResult[DCT_INPUT_LEN] = {
    0.06249882, 0.06248941, 0.06247059, 0.06244236, 0.06240472, 0.06235769, 0.06230127, 0.06223546, 0.06216029,
    0.06207575, 0.06198186, 0.06187864, 0.06176610, 0.06164426, 0.06151313, 0.06137274, 0.06122311, 0.06106426,
    0.06089621, 0.06071899, 0.06053263, 0.06033715, 0.06013259, 0.05991897, 0.05969632, 0.05946469, 0.05922410,
    0.05897459, 0.05871620, 0.05844897, 0.05817293, 0.05788814, 0.05759463, 0.05729244, 0.05698163, 0.05666223,
    0.05633430, 0.05599789, 0.05565305, 0.05529982, 0.05493826, 0.05456844, 0.05419039, 0.05380419, 0.05340987,
    0.05300752, 0.05259719, 0.05217893, 0.05175282, 0.05131891, 0.05087727, 0.05042797, 0.04997108, 0.04950666,
    0.04903479, 0.04855553, 0.04806896, 0.04757515, 0.04707418, 0.04656611, 0.04605104, 0.04552902, 0.04500016,
    0.04446451, 0.04392217, 0.04337322, 0.04281773, 0.04225579, 0.04168750, 0.04111292, 0.04053215, 0.03994528,
    0.03935239, 0.03875358, 0.03814892, 0.03753853, 0.03692248, 0.03630087, 0.03567380, 0.03504135, 0.03440362,
    0.03376072, 0.03311273, 0.03245975, 0.03180188, 0.03113923, 0.03047189, 0.02979995, 0.02912353, 0.02844272,
    0.02775763, 0.02706836, 0.02637502, 0.02567770, 0.02497651, 0.02427156, 0.02356296, 0.02285081, 0.02213522,
    0.02141630, 0.02069414, 0.01996888, 0.01924060, 0.01850943, 0.01777547, 0.01703884, 0.01629963, 0.01555798,
    0.01481398, 0.01406774, 0.01331939, 0.01256904, 0.01181679, 0.01106276, 0.01030707, 0.00954982, 0.00879114,
    0.00803113, 0.00726991, 0.00650760, 0.00574431, 0.00498015, 0.00421525, 0.00344970, 0.00268364, 0.00191718,
    0.00115042, 0.00038349,
};

/* Q15 DCT-IV */
static void arm_dct4_q15Example(void)
{
    /* N = 128. */
    q15_t inout[DCT_INPUT_LEN] = {0};
    q15_t ref[DCT_INPUT_LEN];
    q15_t dctState[DCT_INPUT_LEN * 2] = {0};
    q15_t normalize;
    arm_dct4_instance_q15 dct4instance;
    arm_rfft_instance_q15 rfftinstance;
    arm_cfft_radix4_instance_q15 cfftinstance;

    /* Don't need to initiates other items, powerquad driver does not need. */
    memset(&cfftinstance, 0, sizeof(cfftinstance));
    cfftinstance.fftLen = DCT_INPUT_LEN;

    /* Normalize value is sqrt(2/N) Q15. */
    normalize = 0x1000;

    for (uint32_t i = 0; i < ARRAY_SIZE(dct4RefResult); i++)
    {
        ref[i] = FLOAT_2_Q15((float)dct4RefResult[i] / (float)DCT_INPUT_LEN);
    }

    inout[0] = FLOAT_2_Q15(0.5f);

    arm_dct4_init_q15(&dct4instance, &rfftinstance, &cfftinstance, DCT_INPUT_LEN, DCT_INPUT_LEN / 2, normalize);
    arm_dct4_q15(&dct4instance, dctState, inout);

    for (uint32_t i = 0; i < DCT_INPUT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(inout[i] - ref[i]) <= 5);
    }
}

/* Q31 DCT-IV */
static void arm_dct4_q31Example(void)
{
    /* N = 128. */
    q31_t inout[DCT_INPUT_LEN] = {0};
    q31_t ref[DCT_INPUT_LEN];
    q31_t dctState[DCT_INPUT_LEN * 2] = {0};
    q31_t normalize;
    arm_dct4_instance_q31 dct4instance;
    arm_rfft_instance_q31 rfftinstance;
    arm_cfft_radix4_instance_q31 cfftinstance;

    /* Don't need to initiates other items, powerquad driver does not need. */
    memset(&cfftinstance, 0, sizeof(cfftinstance));
    cfftinstance.fftLen = DCT_INPUT_LEN;

    /* Normalize value is sqrt(2/N) Q31. */
    normalize = 0x10000000;

    /*
     * Because only low 27 bits could be used for FFT, the input data
     * should left shift 5 bits to ensure no saturation.
     */
    for (uint32_t i = 0; i < ARRAY_SIZE(dct4RefResult); i++)
    {
        ref[i] = (FLOAT_2_Q31((float)dct4RefResult[i] / (float)DCT_INPUT_LEN)) >> 5;
    }

    inout[0] = (FLOAT_2_Q31(0.5f)) >> 5;

    arm_dct4_init_q31(&dct4instance, &rfftinstance, &cfftinstance, DCT_INPUT_LEN, DCT_INPUT_LEN / 2, normalize);
    arm_dct4_q31(&dct4instance, dctState, inout);

    for (uint32_t i = 0; i < DCT_INPUT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(inout[i] - ref[i]) <= 5);
    }
}
