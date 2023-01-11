/*
 * Copyright 2018-2022 NXP
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
#define MATH_PI        3.1415926535898
#define FLOAT_2_Q31(x) ((int32_t)((x)*2147483648.0f))
#define FLOAT_2_Q15(x) (int16_t) __SSAT(((int32_t)((x)*32768.0f)), 16)
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
static void PQ_Vector16BiquadDf2FPExample(void);
static void PQ_Vector16BiquadDf2FXExample(void);
static void PQ_16ByteBiquadCascadedf2FPExample(void);
static void PQ_16ByteBiquadCascadedf2FXExample(void);

static void PQ_VectorBiquadDf2FloatExample(void);
static void PQ_VectorBiquadDf2Fixed32Example(void);
static void PQ_VectorBiquadDf2Fixed16Example(void);
static void PQ_BiquadCascadedf2FloatExample(void);
static void PQ_BiquadCascadedf2Fixed32Example(void);
static void PQ_BiquadCascadedf2Fixed16Example(void);
static void PQ_FIRFixed16Example(void);
static void PQ_FIRFixed32Example(void);
static void PQ_FIRFloatExample(void);
static void PQ_ConvolutionFixed16Example(void);
static void PQ_ConvolutionFixed32Example(void);
static void PQ_ConvolutionFloatExample(void);
static void PQ_CorrelationFixed16Example(void);
static void PQ_CorrelationFixed32Example(void);
static void PQ_CorrelationFloatExample(void);

void PQ_16ByteBiquadCascadeDf2FP(const pq_biquad_cascade_df2_instance *S, float *pSrc, float *pDst, uint32_t blockSize);
void PQ_16ByteBiquadCascadeDf2FX(const pq_biquad_cascade_df2_instance *S,
                                 int32_t *pSrc,
                                 int32_t *pDst,
                                 uint32_t blockSize);
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

    PRINTF("POWERQUAD filter example started\r\n");

    PQ_Init(DEMO_POWERQUAD);

    PQ_Vector16BiquadDf2FPExample();
    PQ_Vector16BiquadDf2FXExample();
    PQ_16ByteBiquadCascadedf2FPExample();
    PQ_16ByteBiquadCascadedf2FXExample();

    PQ_VectorBiquadDf2FloatExample();
    PQ_VectorBiquadDf2Fixed32Example();
    PQ_VectorBiquadDf2Fixed16Example();
    PQ_BiquadCascadedf2FloatExample();
    PQ_BiquadCascadedf2Fixed32Example();
    PQ_BiquadCascadedf2Fixed16Example();
    PQ_FIRFixed16Example();
    PQ_FIRFixed32Example();
    PQ_FIRFloatExample();
    PQ_ConvolutionFixed16Example();
    PQ_ConvolutionFixed32Example();
    PQ_ConvolutionFloatExample();
    PQ_CorrelationFixed16Example();
    PQ_CorrelationFixed32Example();
    PQ_CorrelationFloatExample();

    PRINTF("POWERQUAD filter example successed\r\n");

    while (1)
    {
    }
}

void PQ_16ByteBiquadCascadeDf2FP(const pq_biquad_cascade_df2_instance *S, float *pSrc, float *pDst, uint32_t blockSize)
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

void PQ_16ByteBiquadCascadeDf2FX(const pq_biquad_cascade_df2_instance *S,
                                 int32_t *pSrc,
                                 int32_t *pDst,
                                 uint32_t blockSize)
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

static void PQ_Vector16BiquadDf2FPExample(void)
{
    pq_biquad_state_t state = {
        .compreg = 0,
        .param =
            {
                .v_n_1 = 0.0f,
                .v_n   = 0.0f,
                .a_1   = 0.2514f,
                .a_2   = 0.5028f,
                .b_0   = 0.2514f,
                .b_1   = -0.1712f,
                .b_2   = 0.1768f,
            },
    };

    float dataForBiquad[16] = {1024.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    float biquadResult[16]  = {0};
    float biquadRef[16]     = {257.433594, -240.027618, 111.948532, 92.542015, -79.552780, -26.530556,
                           46.668919,  1.606997,    -23.869129, 5.192701,  10.695952,  -5.299852,
                           -4.045542,  3.681815,    1.108490,   -2.129891};

    // Init the biquad0 filter
    PQ_BiquadRestoreInternalState(DEMO_POWERQUAD, 0, &state);

    PQ_Vector16BiquadDf2FP(dataForBiquad, biquadResult);

    for (uint32_t i = 0; i < ARRAY_SIZE(dataForBiquad); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(biquadRef[i] - biquadResult[i])) < 0.00001);
    }
}

static void PQ_Vector16BiquadDf2FXExample(void)
{
    pq_biquad_state_t state = {
        .compreg = 0,
        .param =
            {
                .v_n_1 = 0.0f,
                .v_n   = 0.0f,
                .a_1   = 0.2514f,
                .a_2   = 0.5028f,
                .b_0   = 0.2514f,
                .b_1   = -0.1712f,
                .b_2   = 0.1768f,
            },
    };

    int32_t dataForBiquad[16] = {1024, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int32_t biquadResult[16]  = {0};
    int32_t biquadRef[16]     = {257, -240, 112, 93, -80, -27, 47, 2, -24, 5, 11, -5, -4, 4, 1, -2};

    pq_prescale_t prescale;
    prescale.inputPrescale  = 0;
    prescale.outputPrescale = 0;
    prescale.outputSaturate = 0;

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    // Init the biquad0 filter
    PQ_BiquadRestoreInternalState(DEMO_POWERQUAD, 0, &state);

    PQ_Vector16BiquadDf2FX(dataForBiquad, biquadResult);

    for (uint32_t i = 0; i < ARRAY_SIZE(biquadRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(biquadRef[i] == biquadResult[i]);
    }
}

/* Float biquad cascade IIR */
static void PQ_16ByteBiquadCascadedf2FPExample(void)
{
    uint8_t stage = 3;

    pq_biquad_state_t state[3] = {
        {
            .compreg = 0,
            .param =
                {
                    .v_n_1 = 0.0f,
                    .v_n   = 0.0f,
                    .a_1   = 0.2514f,
                    .a_2   = 0.5028f,
                    .b_0   = 0.2514f,
                    .b_1   = -0.1712f,
                    .b_2   = 0.1768f,
                },
        },
        {
            .compreg = 0,
            .param =
                {
                    .v_n_1 = 0.0f,
                    .v_n   = 0.0f,
                    .a_1   = 0.2514f,
                    .a_2   = 0.5028f,
                    .b_0   = 0.2514f,
                    .b_1   = -0.1712f,
                    .b_2   = 0.1768f,
                },
        },
        {
            .compreg = 0,
            .param =
                {
                    .v_n_1 = 0.0f,
                    .v_n   = 0.0f,
                    .a_1   = 0.2514f,
                    .a_2   = 0.5028f,
                    .b_0   = 0.2514f,
                    .b_1   = -0.1712f,
                    .b_2   = 0.1768f,
                },
        },
    };

    float dataForBiquad[16] = {1024.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    float biquadResult[16]  = {0};
    float biquadRef[16] = {16.270306, -45.510643, 63.659611,  -35.223492, -20.120678, 45.005569, -14.585688, -26.120077,
                           25.420242, 6.363828,   -21.309111, 4.942792,   12.659100,  -8.400287, -5.238772,  7.446815};

    pq_biquad_cascade_df2_instance instance;

    /* Calculate all result one time. */
    PQ_BiquadCascadeDf2Init(&instance, stage, state);

    PQ_16ByteBiquadCascadeDf2FP(&instance, dataForBiquad, biquadResult, 15);
    for (uint32_t i = 0; i < 15; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(biquadRef[i] - biquadResult[i])) < 0.00001);
    }
}

static void PQ_16ByteBiquadCascadedf2FXExample(void)
{
    uint8_t stage = 3;

    pq_biquad_state_t state[3] = {
        {
            .compreg = 0,
            .param =
                {
                    .v_n_1 = 0.0f,
                    .v_n   = 0.0f,
                    .a_1   = 0.2514f,
                    .a_2   = 0.5028f,
                    .b_0   = 0.2514f,
                    .b_1   = -0.1712f,
                    .b_2   = 0.1768f,
                },
        },
        {
            .compreg = 0,
            .param =
                {
                    .v_n_1 = 0.0f,
                    .v_n   = 0.0f,
                    .a_1   = 0.2514f,
                    .a_2   = 0.5028f,
                    .b_0   = 0.2514f,
                    .b_1   = -0.1712f,
                    .b_2   = 0.1768f,
                },
        },
        {
            .compreg = 0,
            .param =
                {
                    .v_n_1 = 0.0f,
                    .v_n   = 0.0f,
                    .a_1   = 0.2514f,
                    .a_2   = 0.5028f,
                    .b_0   = 0.2514f,
                    .b_1   = -0.1712f,
                    .b_2   = 0.1768f,
                },
        },
    };

    int32_t dataForBiquad[16] = {1024, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int32_t biquadResult[16]  = {0};
    int32_t biquadRef[16]     = {16, -46, 64, -35, -20, 45, -15, -26, 25, 6, -21, 5, 13, -8, -5, 7};

    pq_biquad_cascade_df2_instance instance;

    /* Calculate all result one time. */
    PQ_BiquadCascadeDf2Init(&instance, stage, state);
    PQ_16ByteBiquadCascadeDf2FX(&instance, dataForBiquad, biquadResult, 15);

    for (uint32_t i = 0; i < 15; i++)
    {
        EXAMPLE_ASSERT_TRUE(biquadRef[i] == biquadResult[i]);
    }
}

static void PQ_VectorBiquadDf2FloatExample(void)
{
    pq_biquad_state_t state = {
        .compreg = 0,
        .param =
            {
                .v_n_1 = 0.0f,
                .v_n   = 0.0f,
                .a_1   = 0.2514f,
                .a_2   = 0.5028f,
                .b_0   = 0.2514f,
                .b_1   = -0.1712f,
                .b_2   = 0.1768f,
            },
    };

    float dataForBiquad[16] = {1024.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    float biquadResult[16]  = {0};
    float biquadRef[16]     = {257.433594, -240.027618, 111.948532, 92.542015, -79.552780, -26.530556,
                           46.668919,  1.606997,    -23.869129, 5.192701,  10.695952,  -5.299852,
                           -4.045542,  3.681815,    1.108490,   -2.129891};

    // Init the biquad0 filter
    PQ_BiquadRestoreInternalState(DEMO_POWERQUAD, 0, &state);

    PQ_VectorBiquadDf2F32(dataForBiquad, biquadResult, ARRAY_SIZE(dataForBiquad));

    for (uint32_t i = 0; i < ARRAY_SIZE(dataForBiquad); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(biquadRef[i] - biquadResult[i])) < 0.00001);
    }
}

static void PQ_VectorBiquadDf2Fixed32Example(void)
{
    pq_biquad_state_t state = {
        .compreg = 0,
        .param =
            {
                .v_n_1 = 0.0f,
                .v_n   = 0.0f,
                .a_1   = 0.2514f,
                .a_2   = 0.5028f,
                .b_0   = 0.2514f,
                .b_1   = -0.1712f,
                .b_2   = 0.1768f,
            },
    };

    int32_t dataForBiquad[16] = {1024, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int32_t biquadResult[16]  = {0};
    int32_t biquadRef[16]     = {257, -240, 112, 93, -80, -27, 47, 2, -24, 5, 11, -5, -4, 4, 1, -2};

    pq_prescale_t prescale;
    prescale.inputPrescale  = 0;
    prescale.outputPrescale = 0;
    prescale.outputSaturate = 0;

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    // Init the biquad0 filter
    PQ_BiquadRestoreInternalState(DEMO_POWERQUAD, 0, &state);

    PQ_VectorBiquadDf2Fixed32(dataForBiquad, biquadResult, ARRAY_SIZE(biquadRef));

    for (uint32_t i = 0; i < ARRAY_SIZE(biquadRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(biquadRef[i] == biquadResult[i]);
    }
}

static void PQ_VectorBiquadDf2Fixed16Example(void)
{
    pq_biquad_state_t state = {
        .compreg = 0,
        .param =
            {
                .v_n_1 = 0.0f,
                .v_n   = 0.0f,
                .a_1   = 0.2514f,
                .a_2   = 0.5028f,
                .b_0   = 0.2514f,
                .b_1   = -0.1712f,
                .b_2   = 0.1768f,
            },
    };

    int16_t dataForBiquad[16] = {1024, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int16_t biquadResult[16]  = {0};
    int16_t biquadRef[16]     = {257, -240, 112, 93, -80, -27, 47, 2, -24, 5, 11, -5, -4, 4, 1, -2};

    pq_prescale_t prescale;
    prescale.inputPrescale  = 0;
    prescale.outputPrescale = 0;
    prescale.outputSaturate = 0;

    PQ_SetCoprocessorScaler(DEMO_POWERQUAD, &prescale);

    // Init the biquad0 filter
    PQ_BiquadRestoreInternalState(DEMO_POWERQUAD, 0, &state);

    PQ_VectorBiquadDf2Fixed16(dataForBiquad, biquadResult, ARRAY_SIZE(biquadRef));

    for (uint32_t i = 0; i < ARRAY_SIZE(biquadRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(biquadRef[i] == biquadResult[i]);
    }
}

/* Float biquad cascade IIR */
static void PQ_BiquadCascadedf2FloatExample(void)
{
    uint8_t stage = 3;

    pq_biquad_state_t state[3] = {
        {
            .compreg = 0,
            .param =
                {
                    .v_n_1 = 0.0f,
                    .v_n   = 0.0f,
                    .a_1   = 0.2514f,
                    .a_2   = 0.5028f,
                    .b_0   = 0.2514f,
                    .b_1   = -0.1712f,
                    .b_2   = 0.1768f,
                },
        },
        {
            .compreg = 0,
            .param =
                {
                    .v_n_1 = 0.0f,
                    .v_n   = 0.0f,
                    .a_1   = 0.2514f,
                    .a_2   = 0.5028f,
                    .b_0   = 0.2514f,
                    .b_1   = -0.1712f,
                    .b_2   = 0.1768f,
                },
        },
        {
            .compreg = 0,
            .param =
                {
                    .v_n_1 = 0.0f,
                    .v_n   = 0.0f,
                    .a_1   = 0.2514f,
                    .a_2   = 0.5028f,
                    .b_0   = 0.2514f,
                    .b_1   = -0.1712f,
                    .b_2   = 0.1768f,
                },
        },
    };

    float dataForBiquad[16] = {1024.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    float biquadResult[16]  = {0};
    float biquadRef[16] = {16.270306, -45.510643, 63.659611,  -35.223492, -20.120678, 45.005569, -14.585688, -26.120077,
                           25.420242, 6.363828,   -21.309111, 4.942792,   12.659100,  -8.400287, -5.238772,  7.446815};

    pq_biquad_cascade_df2_instance instance;

    /* Calculate all result one time. */
    PQ_BiquadCascadeDf2Init(&instance, stage, state);

    PQ_BiquadCascadeDf2F32(&instance, dataForBiquad, biquadResult, 15);
    for (uint32_t i = 0; i < 15; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(biquadRef[i] - biquadResult[i])) < 0.00001);
    }

    /* Incremental calculation. */
    for (uint32_t i = 0; i < 3; i++)
    {
        state[i].compreg     = 0;
        state[i].param.v_n_1 = 0.0f;
        state[i].param.v_n   = 0.0f;
    }
    memset(biquadResult, 0, sizeof(biquadResult));

    PQ_BiquadCascadeDf2Init(&instance, stage, state);

    PQ_BiquadCascadeDf2F32(&instance, dataForBiquad, biquadResult, 8);
    PQ_BiquadCascadeDf2F32(&instance, &dataForBiquad[8], &biquadResult[8], 7);

    for (uint32_t i = 0; i < 15; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(biquadRef[i] - biquadResult[i])) < 0.00001);
    }
}

static void PQ_BiquadCascadedf2Fixed32Example(void)
{
    uint8_t stage = 3;

    pq_biquad_state_t state[3] = {
        {
            .compreg = 0,
            .param =
                {
                    .v_n_1 = 0.0f,
                    .v_n   = 0.0f,
                    .a_1   = 0.2514f,
                    .a_2   = 0.5028f,
                    .b_0   = 0.2514f,
                    .b_1   = -0.1712f,
                    .b_2   = 0.1768f,
                },
        },
        {
            .compreg = 0,
            .param =
                {
                    .v_n_1 = 0.0f,
                    .v_n   = 0.0f,
                    .a_1   = 0.2514f,
                    .a_2   = 0.5028f,
                    .b_0   = 0.2514f,
                    .b_1   = -0.1712f,
                    .b_2   = 0.1768f,
                },
        },
        {
            .compreg = 0,
            .param =
                {
                    .v_n_1 = 0.0f,
                    .v_n   = 0.0f,
                    .a_1   = 0.2514f,
                    .a_2   = 0.5028f,
                    .b_0   = 0.2514f,
                    .b_1   = -0.1712f,
                    .b_2   = 0.1768f,
                },
        },
    };

    int32_t dataForBiquad[16] = {1024, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int32_t biquadResult[16]  = {0};
    int32_t biquadRef[16]     = {16, -46, 64, -35, -20, 45, -15, -26, 25, 6, -21, 5, 13, -8, -5, 7};

    pq_biquad_cascade_df2_instance instance;

    /* Calculate all result one time. */
    PQ_BiquadCascadeDf2Init(&instance, stage, state);
    PQ_BiquadCascadeDf2Fixed32(&instance, dataForBiquad, biquadResult, 15);

    for (uint32_t i = 0; i < 15; i++)
    {
        EXAMPLE_ASSERT_TRUE(biquadRef[i] == biquadResult[i]);
    }

    /* Incremental calculation. */
    for (uint32_t i = 0; i < 3; i++)
    {
        state[i].compreg     = 0;
        state[i].param.v_n_1 = 0.0f;
        state[i].param.v_n   = 0.0f;
    }
    memset(biquadResult, 0, sizeof(biquadResult));

    PQ_BiquadCascadeDf2Init(&instance, stage, state);
    PQ_BiquadCascadeDf2Fixed32(&instance, dataForBiquad, biquadResult, 8);
    PQ_BiquadCascadeDf2Fixed32(&instance, &dataForBiquad[8], &biquadResult[8], 7);

    for (uint32_t i = 0; i < 15; i++)
    {
        EXAMPLE_ASSERT_TRUE(biquadRef[i] == biquadResult[i]);
    }
}

static void PQ_BiquadCascadedf2Fixed16Example(void)
{
    uint8_t stage              = 3;
    pq_biquad_state_t state[3] = {
        {
            .compreg = 0,
            .param =
                {
                    .v_n_1 = 0.0f,
                    .v_n   = 0.0f,
                    .a_1   = 0.2514f,
                    .a_2   = 0.5028f,
                    .b_0   = 0.2514f,
                    .b_1   = -0.1712f,
                    .b_2   = 0.1768f,
                },
        },
        {
            .compreg = 0,
            .param =
                {
                    .v_n_1 = 0.0f,
                    .v_n   = 0.0f,
                    .a_1   = 0.2514f,
                    .a_2   = 0.5028f,
                    .b_0   = 0.2514f,
                    .b_1   = -0.1712f,
                    .b_2   = 0.1768f,
                },
        },
        {
            .compreg = 0,
            .param =
                {
                    .v_n_1 = 0.0f,
                    .v_n   = 0.0f,
                    .a_1   = 0.2514f,
                    .a_2   = 0.5028f,
                    .b_0   = 0.2514f,
                    .b_1   = -0.1712f,
                    .b_2   = 0.1768f,
                },
        },
    };
    int16_t dataForBiquad[16] = {1024, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int16_t biquadResult[16]  = {0};
    int16_t biquadRef[16]     = {16, -46, 64, -35, -20, 45, -15, -26, 25, 6, -21, 5, 13, -8, -5, 7};

    pq_biquad_cascade_df2_instance instance;

    /* Calculate all result one time. */
    PQ_BiquadCascadeDf2Init(&instance, stage, state);
    PQ_BiquadCascadeDf2Fixed16(&instance, dataForBiquad, biquadResult, 15);

    for (uint32_t i = 0; i < 15; i++)
    {
        EXAMPLE_ASSERT_TRUE(biquadRef[i] == biquadResult[i]);
    }

    /* Incremental calculation. */
    for (uint32_t i = 0; i < 3; i++)
    {
        state[i].compreg     = 0;
        state[i].param.v_n_1 = 0.0f;
        state[i].param.v_n   = 0.0f;
    }
    memset(biquadResult, 0, sizeof(biquadResult));

    PQ_BiquadCascadeDf2Init(&instance, stage, state);
    PQ_BiquadCascadeDf2Fixed16(&instance, dataForBiquad, biquadResult, 8);
    PQ_BiquadCascadeDf2Fixed16(&instance, &dataForBiquad[8], &biquadResult[8], 7);

    for (uint32_t i = 0; i < 15; i++)
    {
        EXAMPLE_ASSERT_TRUE(biquadRef[i] == biquadResult[i]);
    }
}

static void PQ_FIRFixed16Example(void)
{
    int16_t dataForFIR[16] = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    int16_t taps[12]       = {7, 9, 11, 6, 8, 9, 3, 6, 3, 7, 3, 5};
    int16_t FIRRef[16]     = {112, 249, 409, 478, 573, 676, 674, 717, 706, 756, 735, 743, 666, 589, 512, 435};
    int16_t FIRResult[16]  = {0};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_FIR, kPQ_16Bit);

    /* Calculation one time. */
    PQ_FIR(DEMO_POWERQUAD, dataForFIR, 16, taps, 12, FIRResult, PQ_FIR_FIR);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < 16; i++)
    {
        EXAMPLE_ASSERT_TRUE(FIRRef[i] == FIRResult[i]);
    }

    /* Incremental calculation. */
    memset(FIRResult, 0, sizeof(FIRResult));

    /* Process the first 10 data. */
    PQ_FIR(DEMO_POWERQUAD, dataForFIR, 10, taps, 12, FIRResult, PQ_FIR_FIR);
    PQ_WaitDone(DEMO_POWERQUAD);

    /* Process the remaining 6 data. */
    PQ_FIRIncrement(DEMO_POWERQUAD, 6, 12, 10);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < 16; i++)
    {
        EXAMPLE_ASSERT_TRUE(FIRRef[i] == FIRResult[i]);
    }
}

static void PQ_FIRFixed32Example(void)
{
    int32_t dataForFIR[16] = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    int32_t taps[12]       = {7, 9, 11, 6, 8, 9, 3, 6, 3, 7, 3, 5};
    int32_t FIRRef[16]     = {112, 249, 409, 478, 573, 676, 674, 717, 706, 756, 735, 743, 666, 589, 512, 435};
    int32_t FIRResult[16]  = {0};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_FIR, kPQ_32Bit);

    /* Calculation one time. */
    PQ_FIR(DEMO_POWERQUAD, dataForFIR, 16, taps, 12, FIRResult, PQ_FIR_FIR);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < 16; i++)
    {
        EXAMPLE_ASSERT_TRUE(FIRRef[i] == FIRResult[i]);
    }

    /* Incremental calculation. */
    memset(FIRResult, 0, sizeof(FIRResult));

    /* Process the first 10 data. */
    PQ_FIR(DEMO_POWERQUAD, dataForFIR, 10, taps, 12, FIRResult, PQ_FIR_FIR);
    PQ_WaitDone(DEMO_POWERQUAD);

    /* Process the remaining 6 data. */
    PQ_FIRIncrement(DEMO_POWERQUAD, 6, 12, 10);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < 16; i++)
    {
        EXAMPLE_ASSERT_TRUE(FIRRef[i] == FIRResult[i]);
    }
}

/* Float FIR */
static void PQ_FIRFloatExample(void)
{
    float dataForFIR[16] = {16.0, 15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0};
    float taps[12]       = {7, 9, 11, 6, 8, 9, 3, 6, 3, 7, 3, 5};
    float FIRRef[16]     = {112.0, 249.0, 409.0, 478.0, 573.0, 676.0, 674.0, 717.0,
                        706.0, 756.0, 735.0, 743.0, 666.0, 589.0, 512.0, 435.0};
    float FIRResult[16]  = {0};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_FIR, kPQ_Float);

    /* Calculation one time. */
    PQ_FIR(DEMO_POWERQUAD, dataForFIR, 16, taps, 12, FIRResult, PQ_FIR_FIR);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < 16; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(FIRRef[i] - FIRResult[i])) < 0.00001);
    }

    /* Incremental calculation. */
    memset(FIRResult, 0, sizeof(FIRResult));

    /* Process the first 10 data. */
    PQ_FIR(DEMO_POWERQUAD, dataForFIR, 10, taps, 12, FIRResult, PQ_FIR_FIR);
    PQ_WaitDone(DEMO_POWERQUAD);

    /* Process the remaining 6 data. */
    PQ_FIRIncrement(DEMO_POWERQUAD, 6, 12, 10);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < 16; i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(FIRRef[i] - FIRResult[i])) < 0.00001);
    }
}

static void PQ_ConvolutionFixed16Example(void)
{
    int16_t dataForConvA[5] = {1, 2, 3, 4, 5};
    int16_t dataForConvB[5] = {2, 4, 6, 8, 10};
    int16_t convRef[9]      = {2, 8, 20, 40, 70, 88, 92, 80, 50};
    int16_t convResult[9]   = {0};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_FIR, kPQ_16Bit);

    PQ_FIR(DEMO_POWERQUAD, dataForConvA, 5, dataForConvB, 5, convResult, PQ_FIR_CONVOLUTION);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < ARRAY_SIZE(convRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(convRef[i] == convResult[i]);
    }
}

static void PQ_ConvolutionFixed32Example(void)
{
    int32_t dataForConvA[5] = {1, 2, 3, 4, 5};
    int32_t dataForConvB[5] = {2, 4, 6, 8, 10};
    int32_t convRef[9]      = {2, 8, 20, 40, 70, 88, 92, 80, 50};
    int32_t convResult[9]   = {0};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_FIR, kPQ_32Bit);

    PQ_FIR(DEMO_POWERQUAD, dataForConvA, 5, dataForConvB, 5, convResult, PQ_FIR_CONVOLUTION);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < ARRAY_SIZE(convRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(convRef[i] == convResult[i]);
    }
}

/* Float Convolution */
static void PQ_ConvolutionFloatExample(void)
{
    float dataForConvA[5] = {1.1, 2.2, 3.3, 4.4, 5.5};
    float dataForConvB[5] = {0.1, 0.2, 0.3, 0.4, 0.5};
    float convRef[9]      = {0.11, 0.44, 1.1, 2.2, 3.85, 4.84, 5.06, 4.4, 2.75};
    float convResult[9]   = {0};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_FIR, kPQ_Float);

    PQ_FIR(DEMO_POWERQUAD, dataForConvA, 5, dataForConvB, 5, convResult, PQ_FIR_CONVOLUTION);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < ARRAY_SIZE(convRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(convRef[i] - convResult[i])) < 0.00001);
    }
}

static void PQ_CorrelationFixed16Example(void)
{
    int16_t dataForCorrelA[5] = {1, 2, 3, 4, 5};
    int16_t dataForCorrelB[5] = {2, 4, 6, 8, 10};
    int16_t correlRef[9]      = {10, 28, 52, 80, 110, 80, 52, 28, 10};
    int16_t correlResult[9]   = {0};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_FIR, kPQ_16Bit);

    PQ_FIR(DEMO_POWERQUAD, dataForCorrelA, 5, dataForCorrelB, 5, correlResult, PQ_FIR_CORRELATION);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < ARRAY_SIZE(correlRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(correlRef[i] == correlResult[i]);
    }
}

static void PQ_CorrelationFixed32Example(void)
{
    int32_t dataForCorrelA[5] = {1, 2, 3, 4, 5};
    int32_t dataForCorrelB[5] = {2, 4, 6, 8, 10};
    int32_t correlRef[9]      = {10, 28, 52, 80, 110, 80, 52, 28, 10};
    int32_t correlResult[9]   = {0};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_FIR, kPQ_32Bit);

    PQ_FIR(DEMO_POWERQUAD, dataForCorrelA, 5, dataForCorrelB, 5, correlResult, PQ_FIR_CORRELATION);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < ARRAY_SIZE(correlRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(correlRef[i] == correlResult[i]);
    }
}

/* Float Correlation */
static void PQ_CorrelationFloatExample(void)
{
    float dataForCorrelA[5] = {1.1, 2.2, 3.3, 4.4, 5.5};
    float dataForCorrelB[5] = {0.1, 0.2, 0.3, 0.4, 0.5};
    float correlRef[9]      = {0.55, 1.54, 2.86, 4.4, 6.05, 4.4, 2.86, 1.54, 0.55};
    float correlResult[9]   = {0};

    PQ_SetFormat(DEMO_POWERQUAD, kPQ_CP_FIR, kPQ_Float);

    PQ_FIR(DEMO_POWERQUAD, dataForCorrelA, 5, dataForCorrelB, 5, correlResult, PQ_FIR_CORRELATION);
    PQ_WaitDone(DEMO_POWERQUAD);

    for (uint32_t i = 0; i < ARRAY_SIZE(correlRef); i++)
    {
        EXAMPLE_ASSERT_TRUE(fabs((double)(correlRef[i] - correlResult[i])) < 0.00001);
    }
}
