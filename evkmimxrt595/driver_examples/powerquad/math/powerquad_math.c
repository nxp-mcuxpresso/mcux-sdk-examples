/*
 * Copyright 2018-2019 NXP
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

/*!
 * @brief primitive 32-byte data vector calculation.
 *
 * @param format  Pointer to the source data format.
 * @param inf     Pointer to the vector function type.

 * Primitive data vector calculation, the input data should be float. The parameter
 * could be (FP, PQ_LN_INF).
 *
 */
#define PQ_Primitive_Vector32(format, inf)  \
    PQ_Initiate_Vector_Func(pSrc, pDst);    \
    PQ_Vector8_##format(false, false, inf); \
    PQ_Vector8_##format(true, false, inf);  \
    PQ_Vector8_##format(true, false, inf);  \
    PQ_Vector8_##format(true, true, inf);   \
    PQ_End_Vector_Func();

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void PQ_MathLnFixedExample(void);
static void PQ_MathLnFloatExample(void);
static void PQ_MathInvFixedExample(void);
static void PQ_MathInvFloatExample(void);
static void PQ_MathSqrtFixedExample(void);
static void PQ_MathSqrtFloatExample(void);
static void PQ_MathInvSqrtFixedExample(void);
static void PQ_MathInvSqrtFloatExample(void);
static void PQ_MathEtoxFixedExample(void);
static void PQ_MathEtoxFloatExample(void);
static void PQ_MathEtonxFixedExample(void);
static void PQ_MathEtonxFloatExample(void);
static void PQ_MathSinQ31Example(void);
static void PQ_MathSinFloatExample(void);
static void PQ_MathCosQ31Example(void);
static void PQ_MathCosFloatExample(void);
static void PQ_MathDivFloatExample(void);
static void PQ_MathArctanFixedExample(void);
static void PQ_MathArctanhFixedExample(void);

/* Vector functions. */
static void PQ_VectorLnFloatExample(void);
static void PQ_VectorLnFixed32Example(void);
static void PQ_VectorLnFixed16Example(void);
static void PQ_VectorInvFloatExample(void);
static void PQ_VectorInvFixed32Example(void);
static void PQ_VectorInvFixed16Example(void);
static void PQ_VectorSqrtFloatExample(void);
static void PQ_VectorSqrtFixed32Example(void);
static void PQ_VectorSqrtFixed16Example(void);
static void PQ_VectorInvSqrtFloatExample(void);
static void PQ_VectorInvSqrtFixed32Example(void);
static void PQ_VectorInvSqrtFixed16Example(void);
static void PQ_VectorEtoxFloatExample(void);
static void PQ_VectorEtoxFixed32Example(void);
static void PQ_VectorEtoxFixed16Example(void);
static void PQ_VectorEtonxFloatExample(void);
static void PQ_VectorEtonxFixed32Example(void);
static void PQ_VectorEtonxFixed16Example(void);
static void PQ_VectorSinFloatExample(void);
static void PQ_VectorSinQ31Example(void);
static void PQ_VectorSinQ15Example(void);
static void PQ_VectorCosFloatExample(void);
static void PQ_VectorCosQ31Example(void);
static void PQ_VectorCosQ15Example(void);

static void PQ_VectorLnFPExample(void);
static void PQ_VectorLnFXExample(void);
static void PQ_VectorInvFPExample(void);
static void PQ_VectorInvFXExample(void);
static void PQ_VectorSqrtFPExample(void);
static void PQ_VectorSqrtFXExample(void);
static void PQ_VectorInvSqrtFPExample(void);
static void PQ_VectorInvSqrtFXExample(void);
static void PQ_VectorEtoxFPExample(void);
static void PQ_VectorEtoxFXExample(void);
static void PQ_VectorEtonxFPExample(void);
static void PQ_VectorEtonxFXExample(void);
static void PQ_VectorSinFPExample(void);
static void PQ_VectorSinFXExample(void);
static void PQ_VectorCosFPExample(void);
static void PQ_VectorCosFXExample(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32InvFP(float *pSrc, float *pDst) __attribute__((noinline))
#else
static void PQ_Vector32InvFP(float *pSrc, float *pDst)
#endif
{PQ_Primitive_Vector32(FP, PQ_INV_INF)};

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32LnFP(float *pSrc, float *pDst) __attribute__((noinline))
#else
static void PQ_Vector32LnFP(float *pSrc, float *pDst)
#endif
{PQ_Primitive_Vector32(FP, PQ_LN_INF)};

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32SqrtFP(float *pSrc, float *pDst) __attribute__((noinline))
#else
static void PQ_Vector32SqrtFP(float *pSrc, float *pDst)
#endif
{PQ_Primitive_Vector32(FP, PQ_SQRT_INF)};

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32InvSqrtFP(float *pSrc, float *pDst) __attribute__((noinline))
#else
static void PQ_Vector32InvSqrtFP(float *pSrc, float *pDst)
#endif
{PQ_Primitive_Vector32(FP, PQ_ISQRT_INF)};

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32EtoxFP(float *pSrc, float *pDst) __attribute__((noinline))
#else
static void PQ_Vector32EtoxFP(float *pSrc, float *pDst)
#endif
{PQ_Primitive_Vector32(FP, PQ_ETOX_INF)};

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32EtonxFP(float *pSrc, float *pDst) __attribute__((noinline))
#else
static void PQ_Vector32EtonxFP(float *pSrc, float *pDst)
#endif
{PQ_Primitive_Vector32(FP, PQ_ETONX_INF)};

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32SinFP(float *pSrc, float *pDst) __attribute__((noinline))
#else
static void PQ_Vector32SinFP(float *pSrc, float *pDst)
#endif
{PQ_Primitive_Vector32(FP, PQ_SIN_INF)};

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32CosFP(float *pSrc, float *pDst) __attribute__((noinline))
#else
static void PQ_Vector32CosFP(float *pSrc, float *pDst)
#endif

{PQ_Primitive_Vector32(FP, PQ_COS_INF)};

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32InvFX(int32_t *pSrc, int32_t *pDst) __attribute__((noinline))
#else
static void PQ_Vector32InvFX(int32_t *pSrc, int32_t *pDst)
#endif
{PQ_Primitive_Vector32(FX, PQ_INV_INF)};

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32LnFX(int32_t *pSrc, int32_t *pDst) __attribute__((noinline))
#else
static void PQ_Vector32LnFX(int32_t *pSrc, int32_t *pDst)
#endif
{PQ_Primitive_Vector32(FX, PQ_LN_INF)};

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32SqrtFX(int32_t *pSrc, int32_t *pDst) __attribute__((noinline))
#else
static void PQ_Vector32SqrtFX(int32_t *pSrc, int32_t *pDst)
#endif
{PQ_Primitive_Vector32(FX, PQ_SQRT_INF)};

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32InvSqrtFX(int32_t *pSrc, int32_t *pDst) __attribute__((noinline))
#else
static void PQ_Vector32InvSqrtFX(int32_t *pSrc, int32_t *pDst)
#endif
{PQ_Primitive_Vector32(FX, PQ_ISQRT_INF)};

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32EtoxFX(int32_t *pSrc, int32_t *pDst) __attribute__((noinline))
#else
static void PQ_Vector32EtoxFX(int32_t *pSrc, int32_t *pDst)
#endif
{PQ_Primitive_Vector32(FX, PQ_ETOX_INF)};

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32EtonxFX(int32_t *pSrc, int32_t *pDst) __attribute__((noinline))
#else
static void PQ_Vector32EtonxFX(int32_t *pSrc, int32_t *pDst)
#endif
{PQ_Primitive_Vector32(FX, PQ_ETONX_INF)};

#if !(defined(FSL_FEATURE_POWERQUAD_SIN_COS_FIX_ERRATA) && FSL_FEATURE_POWERQUAD_SIN_COS_FIX_ERRATA)
/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32SinFX(int32_t *pSrc, int32_t *pDst) __attribute__((noinline))
#else
static void PQ_Vector32SinFX(int32_t *pSrc, int32_t *pDst)
#endif
{PQ_Primitive_Vector32(FX, PQ_SIN_INF)};

/* The armclang compiler will optimize this API to force the assembly code to be inline,
   resulting in disruption of the execution order of the original code in MDK release mode with -O3.
   Resolution: add noinline attribute to assembly code API to prevent inline optimization by compiler. */
#if (defined(__ARMCC_VERSION))
static void PQ_Vector32CosFX(int32_t *pSrc, int32_t *pDst) __attribute__((noinline))
#else
static void PQ_Vector32CosFX(int32_t *pSrc, int32_t *pDst)
#endif
{PQ_Primitive_Vector32(FX, PQ_COS_INF)};
#endif

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

    PRINTF("POWERQUAD math example started\r\n");

    PQ_Init(DEMO_POWERQUAD);

    PQ_MathLnFixedExample();
    PQ_MathLnFloatExample();
    PQ_MathInvFixedExample();
    PQ_MathInvFloatExample();
    PQ_MathSqrtFixedExample();
    PQ_MathSqrtFloatExample();
    PQ_MathInvSqrtFixedExample();
    PQ_MathInvSqrtFloatExample();
    PQ_MathEtoxFixedExample();
    PQ_MathEtoxFloatExample();
    PQ_MathEtonxFixedExample();
    PQ_MathEtonxFloatExample();
    PQ_MathSinQ31Example();
    PQ_MathSinFloatExample();
    PQ_MathCosQ31Example();
    PQ_MathCosFloatExample();
    PQ_MathDivFloatExample();
    PQ_MathArctanFixedExample();
    PQ_MathArctanhFixedExample();

    PQ_VectorLnFloatExample();
    PQ_VectorLnFixed32Example();
    PQ_VectorLnFixed16Example();
    PQ_VectorInvFloatExample();
    PQ_VectorInvFixed32Example();
    PQ_VectorInvFixed16Example();
    PQ_VectorSqrtFloatExample();
    PQ_VectorSqrtFixed32Example();
    PQ_VectorSqrtFixed16Example();
    PQ_VectorInvSqrtFloatExample();
    PQ_VectorInvSqrtFixed32Example();
    PQ_VectorInvSqrtFixed16Example();
    PQ_VectorEtoxFloatExample();
    PQ_VectorEtoxFixed32Example();
    PQ_VectorEtoxFixed16Example();
    PQ_VectorEtonxFloatExample();
    PQ_VectorEtonxFixed32Example();
    PQ_VectorEtonxFixed16Example();
    PQ_VectorSinFloatExample();
    PQ_VectorSinQ31Example();
    PQ_VectorSinQ15Example();
    PQ_VectorCosFloatExample();
    PQ_VectorCosQ31Example();
    PQ_VectorCosQ15Example();

    PQ_VectorLnFPExample();
    PQ_VectorLnFXExample();
    PQ_VectorInvFPExample();
    PQ_VectorInvFXExample();
    PQ_VectorSqrtFPExample();
    PQ_VectorSqrtFXExample();
    PQ_VectorInvSqrtFPExample();
    PQ_VectorInvSqrtFXExample();
    PQ_VectorEtoxFPExample();
    PQ_VectorEtoxFXExample();
    PQ_VectorEtonxFPExample();
    PQ_VectorEtonxFXExample();
    PQ_VectorSinFPExample();
    PQ_VectorSinFXExample();
    PQ_VectorCosFPExample();
    PQ_VectorCosFXExample();

    PRINTF("POWERQUAD math example successed\r\n");

    while (1)
    {
    }
}

/* Fixed Natural Log */
static void PQ_MathLnFixedExample(void)
{
    int32_t input;
    int32_t result;
    int32_t ref;
    uint32_t i;

    /*
     * The output scale is set to 16, so the output result is multiplied by 2^^16
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = 0,
        .outputPrescale = 16,
        .outputSaturate = 0,
    };

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input  = i;
        ref    = (int32_t)(log(input) * (float)(1 << 16));
        result = PQ_LnFixed(input);

        EXAMPLE_ASSERT_TRUE(abs(ref - result) <= 1);
    }
}

/* Float Natural Log */
static void PQ_MathLnFloatExample(void)
{
    float input;
    float result;
    float ref;
    uint32_t i;

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input = i;
        ref   = (float)(log((double)input));
        PQ_LnF32(&input, &result);

        EXAMPLE_ASSERT_TRUE(fabs((double)(ref - result)) <= 0.000001);
    }
}

/* Fixed Reciprocal */
static void PQ_MathInvFixedExample(void)
{
    int32_t input;
    int32_t result;
    int32_t ref;
    uint32_t i;

    /*
     * The output scale is set to 16, so the output result is multiplied by 2^^16
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = 0,
        .outputPrescale = 16,
        .outputSaturate = 0,
    };

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input  = i;
        ref    = (int32_t)(1.0f / (float)input * (float)(1 << 16));
        result = PQ_InvFixed(input);

        EXAMPLE_ASSERT_TRUE(abs(ref - result) <= 1);
    }
}

/* Float Reciprocal */
static void PQ_MathInvFloatExample(void)
{
    float input;
    float result;
    float ref;
    uint32_t i;

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input = i;
        ref   = 1.0f / (float)input;
        PQ_InvF32(&input, &result);

        EXAMPLE_ASSERT_TRUE(fabs((double)(ref - result)) <= 0.000001);
    }
}

/* Fixed Square-root */
static void PQ_MathSqrtFixedExample(void)
{
    int32_t input;
    int32_t result;
    int32_t ref;
    uint32_t i;

    /*
     * The output scale is set to 16, so the output result is multiplied by 2^^16
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = 0,
        .outputPrescale = 16,
        .outputSaturate = 0,
    };

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input  = i;
        ref    = (int32_t)(sqrt(input) * (float)(1 << 16));
        result = PQ_SqrtFixed(input);

        EXAMPLE_ASSERT_TRUE(abs(ref - result) <= 1);
    }
}

/* Float Square-root */
static void PQ_MathSqrtFloatExample(void)
{
    float input;
    float result;
    float ref;
    uint32_t i;

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input = i;
        ref   = sqrt((double)input);
        PQ_SqrtF32(&input, &result);

        EXAMPLE_ASSERT_TRUE(fabs((double)(ref - result)) <= 0.000001);
    }
}

/* Fixed Inverse Square-root */
static void PQ_MathInvSqrtFixedExample(void)
{
    int32_t input;
    int32_t result;
    int32_t ref;
    uint32_t i;

    /*
     * The output scale is set to 16, so the output result is multiplied by 2^^16
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = 0,
        .outputPrescale = 16,
        .outputSaturate = 0,
    };

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input  = i;
        ref    = (int32_t)(1.0f / sqrt(input) * (float)(1 << 16));
        result = PQ_InvSqrtFixed(input);

        EXAMPLE_ASSERT_TRUE(abs(ref - result) <= 1);
    }
}

/* Float Inverse Square-root */
static void PQ_MathInvSqrtFloatExample(void)
{
    float input;
    float result;
    float ref;
    uint32_t i;

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input = i;
        ref   = 1.0f / sqrt((double)input);
        PQ_InvSqrtF32(&input, &result);

        EXAMPLE_ASSERT_TRUE(fabs((double)(ref - result)) <= 0.000001);
    }
}

/* Fixed Natural Exponent */
static void PQ_MathEtoxFixedExample(void)
{
    int32_t input;
    int32_t result;
    int32_t ref;
    uint32_t i;

    /*
     * The output scale is set to 16, so the output result is multiplied by 2^^16
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = -3,
        .outputPrescale = 16,
        .outputSaturate = 0,
    };

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input  = i;
        ref    = (int32_t)(exp((double)input / 8.0) * (double)(1 << 16));
        result = PQ_EtoxFixed(input);

        EXAMPLE_ASSERT_TRUE(abs(ref - result) <= 1);
    }
}

/* F32 Natural Exponent */
static void PQ_MathEtoxFloatExample(void)
{
    float input;
    float result;
    float ref;
    uint32_t i;

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input = i / 8.0f;
        ref   = exp((double)input);
        PQ_EtoxF32(&input, &result);

        EXAMPLE_ASSERT_TRUE(fabs((double)(ref - result)) <= 0.0001);
    }
}

/* Fixed Natural Exponent with negative parameter*/
static void PQ_MathEtonxFixedExample(void)
{
    int32_t input;
    int32_t result;
    int32_t ref;
    uint32_t i;

    /*
     * The output scale is set to 16, so the output result is multiplied by 2^^16
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = -3,
        .outputPrescale = 16,
        .outputSaturate = 0,
    };

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    for (i = 0; i <= MATH_EXAMPLE_LEN; i++)
    {
        input  = i;
        ref    = (int32_t)(1.0f / exp((double)i / 8.0) * (double)(1 << 16));
        result = PQ_EtonxFixed(input);

        EXAMPLE_ASSERT_TRUE(abs(ref - result) <= 1);
    }
}

/* F32 Natural Exponent with negative parameter*/
static void PQ_MathEtonxFloatExample(void)
{
    float input;
    float result;
    float ref;
    uint32_t i;

    for (i = 0; i <= MATH_EXAMPLE_LEN; i++)
    {
        input = i / 8.0f;
        ref   = 1.0f / exp((double)input);
        PQ_EtonxF32(&input, &result);

        EXAMPLE_ASSERT_TRUE(fabs((double)(ref - result)) <= 0.0001);
    }
}

/* Fixed sine */
static void PQ_MathSinQ31Example(void)
{
    float input;
    int32_t result;
    int32_t ref;
    uint32_t i;

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        /*
         * The input value is in radians, input range -1 to 1 (Fixed) means -pi to pi.
         */
        input  = ((float)(i * 2) / (float)MATH_EXAMPLE_LEN) - 1.0f;
        ref    = FLOAT_2_Q31(sin((double)(input)*MATH_PI));
        result = PQ_SinQ31(FLOAT_2_Q31(input));

        EXAMPLE_ASSERT_TRUE(abs(ref - result) <= 400);
    }
}

/* Fixed sine */
static void PQ_MathSinFloatExample(void)
{
    float input;
    float result;
    float ref;
    uint32_t i;

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        /*
         * The input value is in radians. This function calculates -pi to pi.
         */
        input = (double)((((float)(i * 2) / (float)MATH_EXAMPLE_LEN) - 1.0f)) * MATH_PI;
        ref   = sin((double)input);
        PQ_SinF32(&input, &result);

        EXAMPLE_ASSERT_TRUE(fabs((double)(ref - result)) <= 0.000001);
    }
}

/* Fixed Cosine */
static void PQ_MathCosQ31Example(void)
{
    float input;
    int32_t result;
    int32_t ref;
    uint32_t i;

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        /*
         * The input value is in radians, input range -1 to 1 (Fixed) means -pi to pi.
         */
        input  = ((float)(i * 2) / (float)MATH_EXAMPLE_LEN) - 1.0f;
        ref    = FLOAT_2_Q31(cos((double)(input)*MATH_PI));
        result = PQ_CosQ31(FLOAT_2_Q31(input));

        EXAMPLE_ASSERT_TRUE(abs(ref - result) <= 200);
    }
}

/* Fixed Cosine */
static void PQ_MathCosFloatExample(void)
{
    float input;
    float result;
    float ref;
    uint32_t i;

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        /*
         * The input value is in radians. This function calculates -pi to pi.
         */
        input = (double)((((float)(i * 2) / (float)MATH_EXAMPLE_LEN) - 1.0f)) * MATH_PI;
        ref   = cos((double)input);
        PQ_CosF32(&input, &result);

        EXAMPLE_ASSERT_TRUE(fabs((double)(ref - result)) <= 0.000001);
    }
}

/* Float Division */
static void PQ_MathDivFloatExample(void)
{
    float result;
    float ref;
    float x1, x2;
    uint32_t i, j;

    for (i = 0; i <= MATH_EXAMPLE_LEN; i++)
    {
        for (j = 1; j <= MATH_EXAMPLE_LEN; j++)
        {
            x1  = (float)i;
            x2  = (float)j;
            ref = x1 / x2;
            PQ_DivF32(&x1, &x2, &result);

            EXAMPLE_ASSERT_TRUE(fabs((double)(ref - result)) <= 0.00002);
        }
    }
}

/* Fixed Trigonometric */
static void PQ_MathArctanFixedExample(void)
{
    int32_t myres      = 0;
    float arctanResult = 0;
    float arctanRef    = atan(0.5); /* 0.4636476 */

    /* 2^27 means pi. */
    myres        = PQ_ArctanFixed(DEMO_POWERQUAD, 20000000, 10000000, kPQ_Iteration_24);
    arctanResult = (float)(2 * MATH_PI * (myres) / (134217728.0 * 2));

    EXAMPLE_ASSERT_TRUE(fabs((double)(arctanRef - arctanResult)) < 0.001);
}

/* Fixed Inverse Trigonometric */
static void PQ_MathArctanhFixedExample(void)
{
    int32_t myres       = 0;
    float arctanhResult = 0;
    float arctanhRef    = atanh(0.5); /* 0.549306 */

    /* 2^27 means pi. */
    myres         = PQ_ArctanhFixed(DEMO_POWERQUAD, 20000000, 10000000, kPQ_Iteration_24);
    arctanhResult = (float)(2 * (myres) / (134217728.0 * 2));

    EXAMPLE_ASSERT_TRUE(fabs((double)(arctanhRef - arctanhResult)) < 0.00001);
}

static void PQ_VectorLnFloatExample(void)
{
    float input[MATH_EXAMPLE_LEN];
    float result[MATH_EXAMPLE_LEN];
    float ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = log(i);
    }

    PQ_VectorLnF32(input, result, ARRAY_SIZE(result));

    for (i = 0; i < ARRAY_SIZE(result); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(ref[i] - result[i])) <= 0.000001);
    }
}

static void PQ_VectorLnFixed32Example(void)
{
    int32_t input[MATH_EXAMPLE_LEN];
    int32_t result[MATH_EXAMPLE_LEN];
    int32_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    /*
     * The output scale is set to 16, so the output result is multiplied by 2^^16
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = 0,
        .outputPrescale = 16,
        .outputSaturate = 0,
    };

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = (int32_t)(log(i) * (float)(1 << 16));
    }

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    PQ_VectorLnFixed32(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
    }
}

static void PQ_VectorLnFixed16Example(void)
{
    int16_t input[MATH_EXAMPLE_LEN];
    int16_t result[MATH_EXAMPLE_LEN];
    int16_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    /*
     * The output scale is set to 8, so the output result is multiplied by 2^^8
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = 0,
        .outputPrescale = 8,
        .outputSaturate = 0,
    };

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = (int16_t)(log(i) * (float)(1 << 8));
    }

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    PQ_VectorLnFixed16(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
    }
}

static void PQ_VectorInvFloatExample(void)
{
    float input[MATH_EXAMPLE_LEN];
    float result[MATH_EXAMPLE_LEN];
    float ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = 1.0f / (float)i;
    }

    PQ_VectorInvF32(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(ref[i] - result[i])) <= 0.000001);
    }
}

static void PQ_VectorInvFixed32Example(void)
{
    int32_t input[MATH_EXAMPLE_LEN];
    int32_t result[MATH_EXAMPLE_LEN];
    int32_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    /*
     * The output scale is set to 16, so the output result is multiplied by 2^^16
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = 0,
        .outputPrescale = 16,
        .outputSaturate = 0,
    };

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = (int32_t)(1.0f / (float)i * (float)(1 << 16));
    }

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    PQ_VectorInvFixed32(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
    }
}

static void PQ_VectorInvFixed16Example(void)
{
    int16_t input[MATH_EXAMPLE_LEN];
    int16_t result[MATH_EXAMPLE_LEN];
    int16_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    /*
     * The output scale is set to 8, so the output result is multiplied by 2^^8
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = 0,
        .outputPrescale = 8,
        .outputSaturate = 0,
    };

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = (int16_t)(1.0f / (float)i * (float)(1 << 8));
    }

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    PQ_VectorInvFixed16(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
    }
}

/* Float Vector Sqrt */
static void PQ_VectorSqrtFloatExample(void)
{
    float input[MATH_EXAMPLE_LEN];
    float result[MATH_EXAMPLE_LEN];
    float ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = sqrt((double)i);
    }

    PQ_VectorSqrtF32(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(ref[i] - result[i])) <= 0.000001);
    }
}

static void PQ_VectorSqrtFixed32Example(void)
{
    int32_t input[MATH_EXAMPLE_LEN];
    int32_t result[MATH_EXAMPLE_LEN];
    int32_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    /*
     * The output scale is set to 16, so the output result is multiplied by 2^^16
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = 0,
        .outputPrescale = 16,
        .outputSaturate = 0,
    };

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = (int32_t)(sqrt((double)i) * (float)(1 << 16));
    }

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    PQ_VectorSqrtFixed32(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
    }
}

static void PQ_VectorSqrtFixed16Example(void)
{
    int16_t input[MATH_EXAMPLE_LEN];
    int16_t result[MATH_EXAMPLE_LEN];
    int16_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    /*
     * The output scale is set to 8, so the output result is multiplied by 2^^8
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = 0,
        .outputPrescale = 8,
        .outputSaturate = 0,
    };

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = (int16_t)(sqrt((double)i) * (float)(1 << 8));
    }

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    PQ_VectorSqrtFixed16(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
    }
}

static void PQ_VectorInvSqrtFloatExample(void)
{
    float input[MATH_EXAMPLE_LEN];
    float result[MATH_EXAMPLE_LEN];
    float ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = 1.0f / sqrt((double)i);
    }

    PQ_VectorInvSqrtF32(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(ref[i] - result[i])) <= 0.000001);
    }
}

static void PQ_VectorInvSqrtFixed32Example(void)
{
    int32_t input[MATH_EXAMPLE_LEN];
    int32_t result[MATH_EXAMPLE_LEN];
    int32_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    /*
     * The output scale is set to 16, so the output result is multiplied by 2^^16
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = 0,
        .outputPrescale = 16,
        .outputSaturate = 0,
    };

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = (int32_t)(1.0f / sqrt((double)i) * (float)(1 << 16));
    }

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    PQ_VectorInvSqrtFixed32(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
    }
}

static void PQ_VectorInvSqrtFixed16Example(void)
{
    int16_t input[MATH_EXAMPLE_LEN];
    int16_t result[MATH_EXAMPLE_LEN];
    int16_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    /*
     * The output scale is set to 8, so the output result is multiplied by 2^^8
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = 0,
        .outputPrescale = 8,
        .outputSaturate = 0,
    };

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = (int16_t)(1.0f / sqrt((double)i) * (float)(1 << 8));
    }

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    PQ_VectorInvSqrtFixed16(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
    }
}

static void PQ_VectorEtoxFloatExample(void)
{
    float input[MATH_EXAMPLE_LEN];
    float result[MATH_EXAMPLE_LEN];
    float ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i / 8.0f;
        ref[i - 1]   = exp((double)(i) / 8.0);
    }

    PQ_VectorEtoxF32(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(ref[i] - result[i])) <= 0.0001);
    }
}

static void PQ_VectorEtoxFixed32Example(void)
{
    int32_t input[MATH_EXAMPLE_LEN];
    int32_t result[MATH_EXAMPLE_LEN];
    int32_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    /*
     * The output scale is set to 16, so the output result is multiplied by 2^^16
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = -3,
        .outputPrescale = 16,
        .outputSaturate = 0,
    };

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = (int32_t)(exp(((double)i / 8.0)) * (double)(1 << 16));
    }

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    PQ_VectorEtoxFixed32(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
    }
}

static void PQ_VectorEtoxFixed16Example(void)
{
    int16_t input[MATH_EXAMPLE_LEN];
    int16_t result[MATH_EXAMPLE_LEN];
    int16_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    /*
     * The output scale is set to 8, so the output result is multiplied by 2^^8
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = -3,
        .outputPrescale = 8,
        .outputSaturate = 0,
    };

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = (int16_t)(exp((double)i / 8.0) * (double)(1 << 8));
    }

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    PQ_VectorEtoxFixed16(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
    }
}

static void PQ_VectorEtonxFloatExample(void)
{
    float input[MATH_EXAMPLE_LEN];
    float result[MATH_EXAMPLE_LEN];
    float ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i / 8.0f;
        ref[i - 1]   = 1.0f / exp((double)(i) / 8.0);
    }

    PQ_VectorEtonxF32(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(ref[i] - result[i])) <= 0.0001);
    }
}

static void PQ_VectorEtonxFixed32Example(void)
{
    int32_t input[MATH_EXAMPLE_LEN];
    int32_t result[MATH_EXAMPLE_LEN];
    int32_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    /*
     * The output scale is set to 16, so the output result is multiplied by 2^^16
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = -3,
        .outputPrescale = 16,
        .outputSaturate = 0,
    };

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = (int32_t)(1.0f / exp((double)i / 8.0) * (double)(1 << 16));
    }

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    PQ_VectorEtonxFixed32(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
    }
}

static void PQ_VectorEtonxFixed16Example(void)
{
    int16_t input[MATH_EXAMPLE_LEN];
    int16_t result[MATH_EXAMPLE_LEN];
    int16_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    /*
     * The output scale is set to 8, so the output result is multiplied by 2^^8
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = -3,
        .outputPrescale = 8,
        .outputSaturate = 0,
    };

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = (int16_t)(1.0f / exp((double)i / 8.0) * (double)(1 << 8));
    }

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    PQ_VectorEtonxFixed16(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
    }
}

static void PQ_VectorSinFloatExample(void)
{
    float input[MATH_EXAMPLE_LEN];
    float result[MATH_EXAMPLE_LEN];
    float ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    for (i = 0; i < MATH_EXAMPLE_LEN; i++)
    {
        /*
         * The input value is in radians. This function calculates -pi to pi.
         */
        input[i] = (double)((((float)(i * 2) / (float)MATH_EXAMPLE_LEN) - 1.0f)) * MATH_PI;
        ref[i]   = sin((double)(input[i]));
    }

    PQ_VectorSinF32(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(ref[i] - result[i])) <= 0.000001);
    }

    for (; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(result[i] == 0.0f);
    }
}

static void PQ_VectorSinQ31Example(void)
{
    float inputFloat;
    int32_t input[MATH_EXAMPLE_LEN];
    int32_t result[MATH_EXAMPLE_LEN];
    int32_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    for (i = 0; i < MATH_EXAMPLE_LEN; i++)
    {
        /*
         * The input value is in radians, input range -1 to 1 (Fixed) means -pi to pi.
         */
        inputFloat = ((float)(i * 2) / (float)MATH_EXAMPLE_LEN) - 1.0f;
        input[i]   = FLOAT_2_Q31(inputFloat);
        ref[i]     = FLOAT_2_Q31(sin((double)(inputFloat)*MATH_PI));
    }

    PQ_VectorSinQ31(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 400);
    }
}

static void PQ_VectorSinQ15Example(void)
{
    float inputFloat;
    int16_t input[MATH_EXAMPLE_LEN];
    int16_t result[MATH_EXAMPLE_LEN];
    int16_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    for (i = 0; i < MATH_EXAMPLE_LEN; i++)
    {
        /*
         * The input value is in radians, input range -1 to 1 (Q15) means -pi to pi.
         */
        inputFloat = ((float)(i * 2) / (float)MATH_EXAMPLE_LEN) - 1.0f;
        input[i]   = FLOAT_2_Q15(inputFloat);
        ref[i]     = FLOAT_2_Q15(sin((double)(inputFloat)*MATH_PI));
    }

    PQ_VectorSinQ15(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 2);
    }
}

static void PQ_VectorCosFloatExample(void)
{
    float input[MATH_EXAMPLE_LEN];
    float result[MATH_EXAMPLE_LEN];
    float ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    for (i = 0; i < MATH_EXAMPLE_LEN; i++)
    {
        /*
         * The input value is in radians. This function calculates -pi to pi.
         */
        input[i] = (double)((((float)(i * 2) / (float)MATH_EXAMPLE_LEN) - 1.0f)) * MATH_PI;
        ref[i]   = cos((double)(input[i]));
    }

    PQ_VectorCosF32(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(ref[i] - result[i])) <= 0.000001);
    }
}

static void PQ_VectorCosQ31Example(void)
{
    float inputFloat;
    int32_t input[MATH_EXAMPLE_LEN];
    int32_t result[MATH_EXAMPLE_LEN];
    int32_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    for (i = 0; i < MATH_EXAMPLE_LEN; i++)
    {
        /*
         * The input value is in radians, input range -1 to 1 (Fixed) means -pi to pi.
         */
        inputFloat = ((float)(i * 2) / (float)MATH_EXAMPLE_LEN) - 1.0f;
        input[i]   = FLOAT_2_Q31(inputFloat);
        ref[i]     = FLOAT_2_Q31(cos((double)(inputFloat)*MATH_PI));
    }

    PQ_VectorCosQ31(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 200);
    }
}

static void PQ_VectorCosQ15Example(void)
{
    float inputFloat;
    int16_t input[MATH_EXAMPLE_LEN];
    int16_t result[MATH_EXAMPLE_LEN];
    int16_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    for (i = 0; i < MATH_EXAMPLE_LEN; i++)
    {
        /*
         * The input value is in radians, input range -1 to 1 (Q15) means -pi to pi.
         */
        inputFloat = ((float)(i * 2) / (float)MATH_EXAMPLE_LEN) - 1.0f;
        input[i]   = FLOAT_2_Q15(inputFloat);
        ref[i]     = FLOAT_2_Q15(cos((double)(inputFloat)*MATH_PI));
    }

    PQ_VectorCosQ15(input, result, ARRAY_SIZE(input));

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 2);
    }
}

static void PQ_VectorLnFPExample(void)
{
    float input[MATH_EXAMPLE_LEN];
    float result[MATH_EXAMPLE_LEN];
    float ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = log(i);
    }

    PQ_Vector32LnFP(input, result);

    for (i = 0; i < ARRAY_SIZE(result); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(ref[i] - result[i])) <= 0.000001);
    }
}

static void PQ_VectorLnFXExample(void)
{
    int32_t input[MATH_EXAMPLE_LEN];
    int32_t result[MATH_EXAMPLE_LEN];
    int32_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    /*
     * The output scale is set to 16, so the output result is multiplied by 2^^16
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = 0,
        .outputPrescale = 16,
        .outputSaturate = 0,
    };

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = (int32_t)(log(i) * (float)(1 << 16));
    }

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    PQ_Vector32LnFX(input, result);

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
    }
}

static void PQ_VectorInvFPExample(void)
{
    float input[MATH_EXAMPLE_LEN];
    float result[MATH_EXAMPLE_LEN];
    float ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = 1.0f / (float)i;
    }

    PQ_Vector32InvFP(input, result);

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(ref[i] - result[i])) <= 0.000001);
    }
}

static void PQ_VectorInvFXExample(void)
{
    int32_t input[MATH_EXAMPLE_LEN];
    int32_t result[MATH_EXAMPLE_LEN];
    int32_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    /*
     * The output scale is set to 16, so the output result is multiplied by 2^^16
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = 0,
        .outputPrescale = 16,
        .outputSaturate = 0,
    };

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = (int32_t)(1.0f / (float)i * (float)(1 << 16));
    }

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    PQ_Vector32InvFX(input, result);

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
    }
}

/* Float Vector Sqrt */
static void PQ_VectorSqrtFPExample(void)
{
    float input[MATH_EXAMPLE_LEN];
    float result[MATH_EXAMPLE_LEN];
    float ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = sqrt((double)i);
    }

    PQ_Vector32SqrtFP(input, result);

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(ref[i] - result[i])) <= 0.000001);
    }
}

static void PQ_VectorSqrtFXExample(void)
{
    int32_t input[MATH_EXAMPLE_LEN];
    int32_t result[MATH_EXAMPLE_LEN];
    int32_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    /*
     * The output scale is set to 16, so the output result is multiplied by 2^^16
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = 0,
        .outputPrescale = 16,
        .outputSaturate = 0,
    };

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = (int32_t)(sqrt((double)i) * (float)(1 << 16));
    }

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    PQ_Vector32SqrtFX(input, result);

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
    }
}

static void PQ_VectorInvSqrtFPExample(void)
{
    float input[MATH_EXAMPLE_LEN];
    float result[MATH_EXAMPLE_LEN];
    float ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = 1.0f / sqrt((double)i);
    }

    PQ_Vector32InvSqrtFP(input, result);

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(ref[i] - result[i])) <= 0.000001);
    }
}

static void PQ_VectorInvSqrtFXExample(void)
{
    int32_t input[MATH_EXAMPLE_LEN];
    int32_t result[MATH_EXAMPLE_LEN];
    int32_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    /*
     * The output scale is set to 16, so the output result is multiplied by 2^^16
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = 0,
        .outputPrescale = 16,
        .outputSaturate = 0,
    };

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = (int32_t)(1.0f / sqrt((double)(i)) * (double)(1 << 16));
    }

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    PQ_Vector32InvSqrtFX(input, result);

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
    }
}

static void PQ_VectorEtoxFPExample(void)
{
    float input[MATH_EXAMPLE_LEN];
    float result[MATH_EXAMPLE_LEN];
    float ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i / 8.0f;
        ref[i - 1]   = exp((double)(i) / 8.0);
    }

    PQ_Vector32EtoxFP(input, result);

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(ref[i] - result[i])) <= 0.0001);
    }
}

static void PQ_VectorEtoxFXExample(void)
{
    int32_t input[MATH_EXAMPLE_LEN];
    int32_t result[MATH_EXAMPLE_LEN];
    int32_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    /*
     * The output scale is set to 16, so the output result is multiplied by 2^^16
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = -3,
        .outputPrescale = 16,
        .outputSaturate = 0,
    };

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = (int32_t)(exp(((double)(i) / 8.0)) * (double)(1 << 16));
    }

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    PQ_Vector32EtoxFX(input, result);

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
    }
}

static void PQ_VectorEtonxFPExample(void)
{
    float input[MATH_EXAMPLE_LEN];
    float result[MATH_EXAMPLE_LEN];
    float ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i / 8.0f;
        ref[i - 1]   = 1.0f / exp((double)(i) / 8.0);
    }

    PQ_Vector32EtonxFP(input, result);

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(ref[i] - result[i])) <= 0.0001);
    }
}

static void PQ_VectorEtonxFXExample(void)
{
    int32_t input[MATH_EXAMPLE_LEN];
    int32_t result[MATH_EXAMPLE_LEN];
    int32_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    /*
     * The output scale is set to 16, so the output result is multiplied by 2^^16
     */
    const pq_prescale_t prescale = {
        .inputPrescale  = -3,
        .outputPrescale = 16,
        .outputSaturate = 0,
    };

    for (i = 1; i <= MATH_EXAMPLE_LEN; i++)
    {
        input[i - 1] = i;
        ref[i - 1]   = (int32_t)(1.0f / exp((double)(i) / 8.0) * (double)(1 << 16));
    }

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    PQ_Vector32EtonxFX(input, result);

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 1);
    }
}

static void PQ_VectorSinFPExample(void)
{
    float input[MATH_EXAMPLE_LEN];
    float result[MATH_EXAMPLE_LEN];
    float ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    for (i = 0; i < MATH_EXAMPLE_LEN; i++)
    {
        /*
         * The input value is in radians. This function calculates -pi to pi.
         */
        input[i] = (double)((((float)(i * 2) / (float)MATH_EXAMPLE_LEN) - 1.0f)) * MATH_PI;
        ref[i]   = sin((double)input[i]);
    }

    PQ_Vector32SinFP(input, result);

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(ref[i] - result[i])) <= 0.000001);
    }

    for (; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(result[i] == 0.0f);
    }
}

static void PQ_VectorSinFXExample(void)
{
    float inputFloat;
    int32_t input[MATH_EXAMPLE_LEN];
    int32_t result[MATH_EXAMPLE_LEN];
    int32_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    for (i = 0; i < MATH_EXAMPLE_LEN; i++)
    {
        /*
         * The input value is in radians, input range -1 to 1 (Fixed) means -pi to pi.
         */
        inputFloat = ((float)((i + 1) * 2) / (float)MATH_EXAMPLE_LEN) - 1.0f;
        input[i]   = FLOAT_2_Q31(inputFloat);
        ref[i]     = FLOAT_2_Q31(sin((double)(inputFloat)*MATH_PI));
    }

#if defined(FSL_FEATURE_POWERQUAD_SIN_COS_FIX_ERRATA) && FSL_FEATURE_POWERQUAD_SIN_COS_FIX_ERRATA
    PQ_VectorSinQ31(input, result, ARRAY_SIZE(input));
#else
    const pq_prescale_t prescale = {.inputPrescale = 0, .outputPrescale = 31, .outputSaturate = 0};

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);
    PQ_Vector32SinFX(input, result);
#endif

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 400);
    }
}

static void PQ_VectorCosFPExample(void)
{
    float input[MATH_EXAMPLE_LEN];
    float result[MATH_EXAMPLE_LEN];
    float ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    for (i = 0; i < MATH_EXAMPLE_LEN; i++)
    {
        /*
         * The input value is in radians. This function calculates -pi to pi.
         */
        input[i] = (double)((((float)(i * 2) / (float)MATH_EXAMPLE_LEN) - 1.0f)) * MATH_PI;
        ref[i]   = cos((double)input[i]);
    }

    PQ_Vector32CosFP(input, result);

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(ref[i] - result[i])) <= 0.000001);
    }
}

static void PQ_VectorCosFXExample(void)
{
    float inputFloat;
    int32_t input[MATH_EXAMPLE_LEN];
    int32_t result[MATH_EXAMPLE_LEN];
    int32_t ref[MATH_EXAMPLE_LEN];
    uint32_t i;

    for (i = 0; i < MATH_EXAMPLE_LEN; i++)
    {
        /*
         * The input value is in radians, input range -1 to 1 (Fixed) means -pi to pi.
         */
        inputFloat = ((float)((i + 1) * 2) / (float)MATH_EXAMPLE_LEN) - 1.0f;
        input[i]   = FLOAT_2_Q31(inputFloat);
        ref[i]     = FLOAT_2_Q31(cos((double)(inputFloat)*MATH_PI));
    }

#if defined(FSL_FEATURE_POWERQUAD_SIN_COS_FIX_ERRATA) && FSL_FEATURE_POWERQUAD_SIN_COS_FIX_ERRATA
    PQ_VectorCosQ31(input, result, ARRAY_SIZE(input));
#else
    const pq_prescale_t prescale = {.inputPrescale = 0, .outputPrescale = 31, .outputSaturate = 0};

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);
    PQ_Vector32CosFX(input, result);
#endif

    for (i = 0; i < ARRAY_SIZE(input); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(ref[i] - result[i]) <= 200);
    }
}
