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
#define MATH_EXAMPLE_LEN 32
#define DATA_SIZE        4
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

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void PQ_MatrixAdditionFixed16Example(void);
static void PQ_MatrixAdditionFixed32Example(void);
static void PQ_MatrixAdditionFloatExample(void);
static void PQ_MatrixSubstractionFixed16Example(void);
static void PQ_MatrixSubstractionFixed32Example(void);
static void PQ_MatrixSubstractionFloatExample(void);
static void PQ_MatrixMultiplicationFixed16Example(void);
static void PQ_MatrixMultiplicationFixed32Example(void);
static void PQ_MatrixMultiplicationFloatExample(void);
static void PQ_MatrixProductFixed16Example(void);
static void PQ_MatrixProductFixed32Example(void);
static void PQ_MatrixProductFloatExample(void);
static void PQ_VectDotProdFixed16Example(void);
static void PQ_VectDotProdFixed32Example(void);
static void PQ_VectDotProdFloatExample(void);
static void PQ_MatrixInversionFixed16Example(void);
static void PQ_MatrixInversionFixed32Example(void);
static void PQ_MatrixInversionFloatExample(void);
static void PQ_MatrixTransposeFixed16Example(void);
static void PQ_MatrixTransposeFixed32Example(void);
static void PQ_MatrixTransposeFloatExample(void);
static void PQ_MatrixScaleFixed16Example(void);
static void PQ_MatrixScaleFixed32Example(void);
static void PQ_MatrixScaleFloatExample(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t s_matrixInvTmp[1024];

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{
    /* Board pin, clock, debug console init */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Power up PQ RAM. */
    POWER_DisablePD(kPDRUNCFG_PPD_PQ_SRAM);
    /* Apply power setting. */
    POWER_ApplyPD();

    PRINTF("POWERQUAD matrix example started\r\n");

    PQ_Init(DEMO_POWERQUAD);

    PQ_MatrixAdditionFixed16Example();
    PQ_MatrixAdditionFixed32Example();
    PQ_MatrixAdditionFloatExample();
    PQ_MatrixSubstractionFixed16Example();
    PQ_MatrixSubstractionFixed32Example();
    PQ_MatrixSubstractionFloatExample();
    PQ_MatrixMultiplicationFixed16Example();
    PQ_MatrixMultiplicationFixed32Example();
    PQ_MatrixMultiplicationFloatExample();
    PQ_MatrixProductFixed16Example();
    PQ_MatrixProductFixed32Example();
    PQ_MatrixProductFloatExample();
    PQ_VectDotProdFixed16Example();
    PQ_VectDotProdFixed32Example();
    PQ_VectDotProdFloatExample();
    PQ_MatrixInversionFixed16Example();
    PQ_MatrixInversionFixed32Example();
    PQ_MatrixInversionFloatExample();
    PQ_MatrixTransposeFixed16Example();
    PQ_MatrixTransposeFixed32Example();
    PQ_MatrixTransposeFloatExample();
    PQ_MatrixScaleFixed16Example();
    PQ_MatrixScaleFixed32Example();
    PQ_MatrixScaleFloatExample();

    PRINTF("POWERQUAD matrix example successed\r\n");

    while (1)
    {
    }
}

static void PQ_MatrixAdditionFixed16Example(void)
{
    uint32_t length   = POWERQUAD_MAKE_MATRIX_LEN(2, 2, 2);
    int16_t A15[4]    = {1, 2, 3, 4};
    int16_t B15[4]    = {1, 2, 3, 4};
    int16_t result[4] = {0};
    int16_t ref[4]    = {2, 4, 6, 8};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_MTX, kPQ_16Bit);

    /* Matrix Addition */
    PQ_MatrixAddition(DEMO_POWERQUAD, length, (void *)A15, (void *)B15, (void *)result);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < DATA_SIZE; i++)
    {
        EXAMPLE_ASSERT_TRUE(ref[i] == result[i]);
    }
}

static void PQ_MatrixAdditionFixed32Example(void)
{
    uint32_t length   = POWERQUAD_MAKE_MATRIX_LEN(2, 2, 2);
    int32_t A31[4]    = {1, 2, 3, 4};
    int32_t B31[4]    = {1, 2, 3, 4};
    int32_t result[4] = {0};
    int32_t ref[4]    = {2, 4, 6, 8};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_MTX, kPQ_32Bit);

    /* Matrix Addition */
    PQ_MatrixAddition(DEMO_POWERQUAD, length, (void *)A31, (void *)B31, (void *)result);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < DATA_SIZE; i++)
    {
        EXAMPLE_ASSERT_TRUE(ref[i] == result[i]);
    }
}

/* Float Matrix Addition */
static void PQ_MatrixAdditionFloatExample(void)
{
    uint32_t length = POWERQUAD_MAKE_MATRIX_LEN(2, 3, 3);
    float M1[6]     = {11.0, 22.0, 33.0, 44.0, 55.0, 66.0};
    float M2[6]     = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    float result[6] = {0};
    float ref[6]    = {12, 24, 36, 48, 60, 72};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_MTX, kPQ_Float);

    /* Matrix Addition */
    PQ_MatrixAddition(DEMO_POWERQUAD, length, (float *)M1, (float *)M2, (float *)result);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < DATA_SIZE; i++)
    {
        EXAMPLE_ASSERT_TRUE(ref[i] == result[i]);
    }
}

static void PQ_MatrixSubstractionFixed16Example(void)
{
    uint32_t length   = POWERQUAD_MAKE_MATRIX_LEN(2, 2, 2);
    int16_t A15[4]    = {1, 2, 3, 4};
    int16_t B15[4]    = {1, 2, 3, 4};
    int16_t result[4] = {5, 5, 5, 5};
    int16_t ref[4]    = {0, 0, 0, 0};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_MTX, kPQ_16Bit);

    /* Matrix Substraction */
    PQ_MatrixSubtraction(DEMO_POWERQUAD, length, (void *)A15, (void *)B15, (void *)result);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < DATA_SIZE; i++)
    {
        EXAMPLE_ASSERT_TRUE(ref[i] == result[i]);
    }
}

static void PQ_MatrixSubstractionFixed32Example(void)
{
    uint32_t length   = POWERQUAD_MAKE_MATRIX_LEN(2, 2, 2);
    int32_t A31[4]    = {1, 2, 3, 4};
    int32_t B31[4]    = {1, 2, 3, 4};
    int32_t result[4] = {5, 5, 5, 5};
    int32_t ref[4]    = {0, 0, 0, 0};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_MTX, kPQ_32Bit);

    /* Matrix Substraction */
    PQ_MatrixSubtraction(DEMO_POWERQUAD, length, (void *)A31, (void *)B31, (void *)result);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < DATA_SIZE; i++)
    {
        EXAMPLE_ASSERT_TRUE(ref[i] == result[i]);
    }
}

/* Float Matrix Substraction */
static void PQ_MatrixSubstractionFloatExample(void)
{
    uint32_t length = POWERQUAD_MAKE_MATRIX_LEN(2, 2, 2);
    float M1[4]     = {11.0, 22.0, 33.0, 44.0};
    float M2[4]     = {1.0, 2.0, 3.0, 4.0};
    float result[4] = {0};
    float ref[4]    = {10, 20, 30, 40};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_MTX, kPQ_Float);

    /* Matrix Subtraction */
    PQ_MatrixSubtraction(DEMO_POWERQUAD, length, (void *)M1, (void *)M2, (void *)result);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < DATA_SIZE; i++)
    {
        EXAMPLE_ASSERT_TRUE(ref[i] == result[i]);
    }
}

static void PQ_MatrixMultiplicationFixed16Example(void)
{
    uint32_t length   = POWERQUAD_MAKE_MATRIX_LEN(2, 2, 2);
    int16_t A[4]      = {1, 2, 3, 4};
    int16_t B[4]      = {1, 2, 3, 4};
    int16_t result[4] = {0};
    int16_t ref[4]    = {7, 10, 15, 22};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_MTX, kPQ_16Bit);

    /* Matrix Multiplication */
    PQ_MatrixMultiplication(DEMO_POWERQUAD, length, (void *)A, (void *)B, (void *)result);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < DATA_SIZE; i++)
    {
        EXAMPLE_ASSERT_TRUE(ref[i] == result[i]);
    }
}

static void PQ_MatrixMultiplicationFixed32Example(void)
{
    uint32_t length   = POWERQUAD_MAKE_MATRIX_LEN(2, 2, 2);
    int32_t A[4]      = {1, 2, 3, 4};
    int32_t B[4]      = {1, 2, 3, 4};
    int32_t result[4] = {0};
    int32_t ref[4]    = {7, 10, 15, 22};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_MTX, kPQ_32Bit);

    /* Matrix Multiplication */
    PQ_MatrixMultiplication(DEMO_POWERQUAD, length, (void *)A, (void *)B, (void *)result);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < DATA_SIZE; i++)
    {
        EXAMPLE_ASSERT_TRUE(ref[i] == result[i]);
    }
}

/* Float Matrix Multiplication */
static void PQ_MatrixMultiplicationFloatExample(void)
{
    uint32_t length = POWERQUAD_MAKE_MATRIX_LEN(2, 2, 2);
    float A[4]      = {1.0, 2.0, 3.0, 4.0};
    float B[4]      = {1.0, 2.0, 3.0, 4.0};
    float result[4] = {0};
    float ref[4]    = {7.0, 10.0, 15.0, 22.0};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_MTX, kPQ_Float);

    /* Matrix Multiplication */
    PQ_MatrixMultiplication(DEMO_POWERQUAD, length, (void *)A, (void *)B, (void *)result);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < DATA_SIZE; i++)
    {
        EXAMPLE_ASSERT_TRUE(ref[i] == result[i]);
    }
}

static void PQ_MatrixProductFixed16Example(void)
{
    uint32_t length      = POWERQUAD_MAKE_MATRIX_LEN(2, 2, 2);
    int16_t A[4]         = {1, 2, 3, 4};
    int16_t B[4]         = {1, 2, 3, 4};
    int16_t ABProduct[4] = {0};
    int16_t ref[4]       = {1, 4, 9, 16};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_MTX, kPQ_16Bit);

    /* Matrix Product */
    PQ_MatrixProduct(DEMO_POWERQUAD, length, (void *)A, (void *)B, (void *)ABProduct);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < DATA_SIZE; i++)
    {
        EXAMPLE_ASSERT_TRUE(ref[i] == ABProduct[i]);
    }
}

static void PQ_MatrixProductFixed32Example(void)
{
    uint32_t length      = (2 << 16) | (2 << 8) | (2 << 0);
    int32_t A[4]         = {1, 2, 3, 4};
    int32_t B[4]         = {1, 2, 3, 4};
    int32_t ABProduct[4] = {0};
    int32_t ref[4]       = {1, 4, 9, 16};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_MTX, kPQ_32Bit);

    /* Matrix Product */
    PQ_MatrixProduct(DEMO_POWERQUAD, length, (void *)A, (void *)B, (void *)ABProduct);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < DATA_SIZE; i++)
    {
        EXAMPLE_ASSERT_TRUE(ref[i] == ABProduct[i]);
    }
}

/* Float Matrix Product */
static void PQ_MatrixProductFloatExample(void)
{
    uint32_t length    = POWERQUAD_MAKE_MATRIX_LEN(2, 2, 2);
    float A[4]         = {1, 2, 3, 4};
    float B[4]         = {1, 2, 3, 4};
    float ABProduct[4] = {0};
    float ref[4]       = {1, 4, 9, 16};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_MTX, kPQ_Float);

    /* Matrix Product */
    PQ_MatrixProduct(DEMO_POWERQUAD, length, (void *)A, (void *)B, (void *)ABProduct);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < DATA_SIZE; i++)
    {
        EXAMPLE_ASSERT_TRUE(ref[i] == ABProduct[i]);
    }
}

static void PQ_VectDotProdFixed16Example(void)
{
    uint32_t length   = 4;
    int16_t A[4]      = {1, 2, 3, 4};
    int16_t B[4]      = {1, 2, 3, 4};
    int16_t result[1] = {0};
    int16_t ref[1]    = {30};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_MTX, kPQ_16Bit);

    /* Vector Dot Product */
    PQ_VectorDotProduct(DEMO_POWERQUAD, length, (void *)A, (void *)B, (void *)result);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < ARRAY_SIZE(result); i++)
    {
        EXAMPLE_ASSERT_TRUE(ref[i] == result[i]);
    }
}

static void PQ_VectDotProdFixed32Example(void)
{
    uint32_t length     = 4;
    int32_t A[4]        = {1, 2, 3, 4};
    int32_t B[4]        = {1, 2, 3, 4};
    int32_t ABResult[1] = {0};
    int32_t ABRel[1]    = {30};

    PQ_SetFormat(POWERQUAD, kPQ_CP_MTX, kPQ_32Bit);

    /* Vector Dot Product */
    PQ_VectorDotProduct(POWERQUAD, length, (void *)A, (void *)B, (void *)ABResult);
    PQ_WaitDone(POWERQUAD);

    for (uint32_t i = 0; i < ARRAY_SIZE(ABResult); i++)
    {
        EXAMPLE_ASSERT_TRUE(ABRel[i] == ABResult[i]);
    }
}

/* Float Vector Dot Product */
static void PQ_VectDotProdFloatExample(void)
{
    uint32_t length   = 4;
    float A[4]        = {1.0, 2.0, 3.0, 4.0};
    float B[4]        = {1.0, 2.0, 3.0, 4.0};
    float ABResult[1] = {0};
    float ABRel[1]    = {30.0};

    PQ_SetFormat(POWERQUAD, kPQ_CP_MTX, kPQ_Float);

    /* Vector Dot Product */
    PQ_VectorDotProduct(POWERQUAD, length, (void *)A, (void *)B, (void *)ABResult);
    PQ_WaitDone(POWERQUAD);

    for (uint32_t i = 0; i < ARRAY_SIZE(ABResult); i++)
    {
        EXAMPLE_ASSERT_TRUE(ABRel[i] == ABResult[i]);
    }
}

static void PQ_MatrixInversionFixed16Example(void)
{
    uint32_t length    = POWERQUAD_MAKE_MATRIX_LEN(2, 2, 2);
    int16_t inverse[4] = {3, 4, 5, 6};
    int16_t result[4]  = {0};
    int16_t ref[4]     = {-3, 2, 2, -1}; /* Round down */

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_MTX, kPQ_16Bit);

    /* Matrix Inversion */
    PQ_MatrixInversion(DEMO_POWERQUAD, length, (void *)inverse, (void *)s_matrixInvTmp, (void *)result);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < DATA_SIZE; i++)
    {
        EXAMPLE_ASSERT_TRUE(ref[i] == result[i]);
    }
}

static void PQ_MatrixInversionFixed32Example(void)
{
    uint32_t length    = POWERQUAD_MAKE_MATRIX_LEN(2, 2, 2);
    int32_t inverse[4] = {3, 4, 5, 6};
    int32_t result[4]  = {0};
    int32_t ref[4]     = {-3, 2, 2, -1}; /* Round down */

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_MTX, kPQ_32Bit);

    /* Matrix Inversion */
    PQ_MatrixInversion(DEMO_POWERQUAD, length, (void *)inverse, (void *)s_matrixInvTmp, (void *)result);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < DATA_SIZE; i++)
    {
        EXAMPLE_ASSERT_TRUE(ref[i] == result[i]);
    }
}

/* Float Matrix Inversion */
static void PQ_MatrixInversionFloatExample(void)
{
    uint32_t length  = POWERQUAD_MAKE_MATRIX_LEN(2, 2, 2);
    float inverse[4] = {1.0, 2.0, 3.0, 4.0};
    float result[4]  = {0};
    float ref[4]     = {-2.0, 1.0, 1.5, -0.5};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_MTX, kPQ_Float);

    /* Matrix Inversion */
    PQ_MatrixInversion(DEMO_POWERQUAD, length, (void *)inverse, (void *)s_matrixInvTmp, (void *)result);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < 4; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(ref[i] - result[i])) < 0.00001);
    }
}

static void PQ_MatrixTransposeFixed16Example(void)
{
    uint32_t length      = POWERQUAD_MAKE_MATRIX_LEN(2, 2, 0);
    int16_t transpose[4] = {1, 2, 3, 4};
    int16_t result[4]    = {0};
    int16_t ref[4]       = {1, 3, 2, 4};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_MTX, kPQ_16Bit);

    /* Matrix Transpose */
    PQ_MatrixTranspose(DEMO_POWERQUAD, length, (void *)transpose, (void *)result);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < 4; i++)
    {
        EXAMPLE_ASSERT_TRUE(ref[i] == result[i]);
    }
}

static void PQ_MatrixTransposeFixed32Example(void)
{
    uint32_t length      = POWERQUAD_MAKE_MATRIX_LEN(2, 2, 0);
    int32_t transpose[4] = {1, 2, 3, 4};
    int32_t result[4]    = {0};
    int32_t ref[4]       = {1, 3, 2, 4};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_MTX, kPQ_32Bit);

    /* Matrix Transpose */
    PQ_MatrixTranspose(DEMO_POWERQUAD, length, (void *)transpose, (void *)result);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < 4; i++)
    {
        EXAMPLE_ASSERT_TRUE(ref[i] == result[i]);
    }
}

/* Float Matrix Transpose */
static void PQ_MatrixTransposeFloatExample(void)
{
    uint32_t length    = POWERQUAD_MAKE_MATRIX_LEN(2, 2, 0);
    float transpose[4] = {1, 2, 3, 4};
    float result[4]    = {0};
    float ref[4]       = {1, 3, 2, 4};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_MTX, kPQ_Float);

    /* Matrix Transpose */
    PQ_MatrixTranspose(DEMO_POWERQUAD, length, (void *)transpose, (void *)result);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < 4; i++)
    {
        EXAMPLE_ASSERT_TRUE(ref[i] == result[i]);
    }
}

static void PQ_MatrixScaleFixed16Example(void)
{
    uint32_t length   = POWERQUAD_MAKE_MATRIX_LEN(2, 2, 0);
    int16_t scale[4]  = {1, 2, 3, 4};
    int16_t scaler    = 2;
    int16_t result[4] = {0};
    int16_t ref[4]    = {2, 4, 6, 8};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_MTX, kPQ_16Bit);

    /* Matrix Scale */
    PQ_MatrixScale(DEMO_POWERQUAD, length, scaler, (void *)scale, (void *)result);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < DATA_SIZE; i++)
    {
        EXAMPLE_ASSERT_TRUE(ref[i] == result[i]);
    }
}

static void PQ_MatrixScaleFixed32Example(void)
{
    uint32_t length   = POWERQUAD_MAKE_MATRIX_LEN(2, 2, 0);
    int32_t scale[4]  = {1, 2, 3, 4};
    int32_t scaler    = 2;
    int32_t result[4] = {0};
    int32_t ref[4]    = {2, 4, 6, 8};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_MTX, kPQ_32Bit);

    /* Matrix Scale */
    PQ_MatrixScale(DEMO_POWERQUAD, length, scaler, (void *)scale, (void *)result);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < DATA_SIZE; i++)
    {
        EXAMPLE_ASSERT_TRUE(ref[i] == result[i]);
    }
}

/* Float Matrix Scale */
static void PQ_MatrixScaleFloatExample(void)
{
    uint32_t length = POWERQUAD_MAKE_MATRIX_LEN(2, 2, 0);
    float scale[4]  = {1.0, 2.0, 3.0, 4.0};
    float scaler    = 2;
    float result[4] = {0};
    float ref[4]    = {2.0, 4.0, 6.0, 8.0};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_MTX, kPQ_Float);

    /* Matrix Scale */
    PQ_MatrixScale(DEMO_POWERQUAD, length, scaler, (void *)scale, (void *)result);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < DATA_SIZE; i++)
    {
        EXAMPLE_ASSERT_TRUE(ref[i] == result[i]);
    }
}
