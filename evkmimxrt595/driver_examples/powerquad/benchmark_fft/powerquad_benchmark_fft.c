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
/* CMSIS DSP compatible FFT serials*/
static void arm_rfft_q15Test(void);
static void arm_rfft_q31Test(void);
static void arm_cfft_q15Test(void);
static void arm_cfft_q31Test(void);
static void arm_ifft_q15Test(void);
static void arm_ifft_q31Test(void);
static void arm_dct4_q15Test(void);
static void arm_dct4_q31Test(void);

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

    PRINTF("\r\nCMSIS DSP benchmark fft test start.\r\n");

    PQ_Init(POWERQUAD);

    TEST_InitTime();

    /* CMSIS DSP compatible FFT serials*/
    arm_rfft_q15Test();
    arm_rfft_q31Test();

    arm_cfft_q15Test();
    arm_cfft_q31Test();

    arm_ifft_q15Test();
    arm_ifft_q31Test();
    arm_dct4_q15Test();
    arm_dct4_q31Test();

    PRINTF("\r\nCMSIS DSP benchmark fft test succeeded.\r\n");

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

/* Q15 RFFT */
static void arm_rfft_q15Test(void)
{
    q15_t output[RFFT_INPUT_LEN * 2];
    q15_t input[RFFT_INPUT_LEN];
    q15_t inputSave[RFFT_INPUT_LEN];
    q15_t ref[RFFT_INPUT_LEN * 2] = {0};
    arm_rfft_instance_q15 instance;

    uint32_t allTime;
    uint32_t memCopyTime;
    uint32_t oldTime;

    /* Two full period sin wave. */
    for (uint32_t i = 0; i < RFFT_INPUT_LEN / 2; i++)
    {
        inputSave[i]                      = arm_sin_q15(i * (0x8000 / (RFFT_INPUT_LEN / 2)));
        inputSave[i + RFFT_INPUT_LEN / 2] = inputSave[i];
    }

    /* Reference result. */
    ref[5]    = FLOAT_2_Q15(-0.5f); /* Imag(2) */
    ref[1021] = FLOAT_2_Q15(0.5f);  /* Imag(510) */

    oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < FFT_TEST_LOOP; i++)
    {
        /* The CMSIS function changes the input data, so the input data is initialized every loop. */
        arm_rfft_init_q15(&instance, RFFT_INPUT_LEN, 0, 1);
        memcpy(input, inputSave, sizeof(inputSave));
        arm_rfft_q15(&instance, input, output);
    }
    allTime = TEST_GetTime() - oldTime;

    /* Eliminate the time to copy memory. */
    oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < FFT_TEST_LOOP; i++)
    {
        memcpy(input, inputSave, sizeof(inputSave));
    }
    memCopyTime = TEST_GetTime() - oldTime;

    PRINTF("%s: %d ms\r\n", __func__, allTime - memCopyTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(ref); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(output[i] - ref[i]) <= 10);
    }
}

/* Q31 RFFT */
static void arm_rfft_q31Test(void)
{
    q31_t output[RFFT_INPUT_LEN * 2];
    q31_t input[RFFT_INPUT_LEN];
    q31_t inputSave[RFFT_INPUT_LEN];
    q31_t ref[RFFT_INPUT_LEN * 2] = {0};
    arm_rfft_instance_q31 instance;

    uint32_t allTime;
    uint32_t memCopyTime;
    uint32_t oldTime;

    /* Two full period sin wave. */
    for (uint32_t i = 0; i < RFFT_INPUT_LEN / 2; i++)
    {
        inputSave[i]                      = arm_sin_q31(i * (0x80000000 / (RFFT_INPUT_LEN / 2))) >> 5;
        inputSave[i + RFFT_INPUT_LEN / 2] = inputSave[i];
    }

    /* Reference result. */
    ref[5]    = FLOAT_2_Q31(-0.5f / 32.0f); /* Imag(2) */
    ref[1021] = FLOAT_2_Q31(0.5f / 32.0f);  /* Imag(510) */

    oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < FFT_TEST_LOOP; i++)
    {
        /* The CMSIS function changes the input data, so the input data is initialized every loop. */
        arm_rfft_init_q31(&instance, RFFT_INPUT_LEN, 0, 1);
        memcpy(input, inputSave, sizeof(inputSave));
        arm_rfft_q31(&instance, input, output);
    }
    allTime = TEST_GetTime() - oldTime;

    /* Eliminate the time to copy memory. */
    oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < FFT_TEST_LOOP; i++)
    {
        memcpy(input, inputSave, sizeof(inputSave));
    }
    memCopyTime = TEST_GetTime() - oldTime;

    PRINTF("%s: %d ms\r\n", __func__, allTime - memCopyTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(ref); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(output[i] - ref[i]) <= 10);
    }
}

/* Q15 CFFT */
static void arm_cfft_q15Test(void)
{
    q15_t inout[CFFT_INPUT_LEN * 2];
    q15_t inputSave[CFFT_INPUT_LEN * 2];
    q15_t ref[CFFT_INPUT_LEN * 2] = {0};

    uint32_t allTime;
    uint32_t memCopyTime;
    uint32_t oldTime;
    arm_cfft_instance_q15 instance;

    /* Don't need to initiates other items, powerquad driver does not need. */
    memset(&instance, 0, sizeof(instance));
    instance.fftLen = CFFT_INPUT_LEN;

    /* Two full period sin wave. */
    for (uint32_t i = 0; i < CFFT_INPUT_LEN / 2; i++)
    {
        inputSave[i * 2]     = arm_sin_q15(i * (0x8000 / (CFFT_INPUT_LEN / 2))) / 2;
        inputSave[i * 2 + 1] = arm_cos_q15(i * (0x8000 / (CFFT_INPUT_LEN / 2))) / 2;
    }
    memcpy(&inputSave[CFFT_INPUT_LEN], inputSave, sizeof(inputSave) / 2);

    /* Reference result. */
    ref[1021] = FLOAT_2_Q15(0.5f); /* Imag(510) */

    /* Eliminate the time to copy memory. */
    oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < FFT_TEST_LOOP; i++)
    {
        memcpy(inout, inputSave, sizeof(inputSave));
    }
    memCopyTime = TEST_GetTime() - oldTime;

    oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < FFT_TEST_LOOP; i++)
    {
        /* The CMSIS function changes the input data, so the input data is initialized every loop. */
        memcpy(inout, inputSave, sizeof(inputSave));
        arm_cfft_q15(&instance, inout, 0, 1);
    }
    allTime = TEST_GetTime() - oldTime;

    PRINTF("%s: %d ms\r\n", __func__, allTime - memCopyTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(ref); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(inout[i] - ref[i]) <= 10);
    }
}

/* Q31 CFFT */
static void arm_cfft_q31Test(void)
{
    q31_t inout[CFFT_INPUT_LEN * 2];
    q31_t inputSave[CFFT_INPUT_LEN * 2];
    q31_t ref[CFFT_INPUT_LEN * 2] = {0};
    arm_cfft_instance_q31 instance;

    /* Don't need to initiates other items, powerquad driver does not need. */
    memset(&instance, 0, sizeof(instance));
    instance.fftLen = CFFT_INPUT_LEN;

    uint32_t allTime;
    uint32_t memCopyTime;
    uint32_t oldTime;

    /* Two full period sin wave. */
    for (uint32_t i = 0; i < CFFT_INPUT_LEN / 2; i++)
    {
        inputSave[i * 2]     = (arm_sin_q31(i * (0x80000000 / (CFFT_INPUT_LEN / 2))) / 2) >> 5;
        inputSave[i * 2 + 1] = (arm_cos_q31(i * (0x80000000 / (CFFT_INPUT_LEN / 2))) / 2) >> 5;
    }
    memcpy(&inputSave[CFFT_INPUT_LEN], inputSave, sizeof(inputSave) / 2);

    /* Reference result. */
    ref[1021] = FLOAT_2_Q31(0.5f / 32.0f); /* Imag(510) */

    /* Eliminate the time to copy memory. */
    oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < FFT_TEST_LOOP; i++)
    {
        memcpy(inout, inputSave, sizeof(inputSave));
    }
    memCopyTime = TEST_GetTime() - oldTime;

    oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < FFT_TEST_LOOP; i++)
    {
        /* The CMSIS function changes the input data, so the input data is initialized every loop. */
        memcpy(inout, inputSave, sizeof(inputSave));
        arm_cfft_q31(&instance, inout, 0, 1);
    }
    allTime = TEST_GetTime() - oldTime;

    PRINTF("%s: %d ms\r\n", __func__, allTime - memCopyTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(ref); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(inout[i] - ref[i]) <= 20);
    }
}

/* Q15 IFFT */
static void arm_ifft_q15Test(void)
{
    q15_t inout[IFFT_INPUT_LEN * 2]     = {0};
    q15_t inputSave[IFFT_INPUT_LEN * 2] = {0};
    q15_t ref[IFFT_INPUT_LEN * 2];
    arm_cfft_instance_q15 instance;

    /* Don't need to initiates other items, powerquad driver does not need. */
    memset(&instance, 0, sizeof(instance));
    instance.fftLen = IFFT_INPUT_LEN;

    uint32_t allTime;
    uint32_t memCopyTime;
    uint32_t oldTime;

    /* Reference result: Two full period sin wave. */
    /*
     * Number of bits to upscale = log2(input data size) - 1. Here the input
     * data is 2*512 = 1024, so upscale bits is 9.
     */

    for (uint32_t i = 0; i < IFFT_INPUT_LEN / 2; i++)
    {
        ref[i * 2]     = (arm_sin_q15(i * (0x8000 / (IFFT_INPUT_LEN / 2))) / 2) >> 9;
        ref[i * 2 + 1] = (arm_cos_q15(i * (0x8000 / (IFFT_INPUT_LEN / 2))) / 2) >> 9;
    }
    memcpy(&ref[IFFT_INPUT_LEN], ref, sizeof(ref) / 2);

    /* Input. */
    inputSave[1021] = FLOAT_2_Q15(0.5f); /* Imag(510) */

    /* Eliminate the time to copy memory. */
    oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < FFT_TEST_LOOP; i++)
    {
        memcpy(inout, inputSave, sizeof(inputSave));
    }
    memCopyTime = TEST_GetTime() - oldTime;

    oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < FFT_TEST_LOOP; i++)
    {
        /* The CMSIS function changes the input data, so the input data is initialized every loop. */
        memcpy(inout, inputSave, sizeof(inputSave));
        arm_cfft_q15(&instance, inout, 1, 1);
    }
    allTime = TEST_GetTime() - oldTime;

    PRINTF("%s: %d ms\r\n", __func__, allTime - memCopyTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(ref); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(inout[i] - ref[i]) <= 10);
    }
}

/* Q31 IFFT */
static void arm_ifft_q31Test(void)
{
    q31_t inout[IFFT_INPUT_LEN * 2]     = {0};
    q31_t inputSave[IFFT_INPUT_LEN * 2] = {0};
    q31_t ref[IFFT_INPUT_LEN * 2];
    arm_cfft_instance_q31 instance;

    /* Don't need to initiates other items, powerquad driver does not need. */
    memset(&instance, 0, sizeof(instance));
    instance.fftLen = IFFT_INPUT_LEN;

    uint32_t allTime;
    uint32_t memCopyTime;
    uint32_t oldTime;

    /* Reference result: Two full period sin wave. */
    /*
     * Number of bits to upscale = log2(input data size) - 1. Here the input
     * data is 2*512 = 1024, so upscale bits is 9.
     */

    for (uint32_t i = 0; i < IFFT_INPUT_LEN / 2; i++)
    {
        ref[i * 2]     = ((arm_sin_q31(i * (0x80000000 / (IFFT_INPUT_LEN / 2))) / 2) >> 9) >> 5;
        ref[i * 2 + 1] = ((arm_cos_q31(i * (0x80000000 / (IFFT_INPUT_LEN / 2))) / 2) >> 9) >> 5;
    }
    memcpy(&ref[IFFT_INPUT_LEN], ref, sizeof(ref) / 2);

    /* Input. */
    inputSave[1021] = FLOAT_2_Q31(0.5f / 32.0f); /* Imag(510) */

    /* Eliminate the time to copy memory. */
    oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < FFT_TEST_LOOP; i++)
    {
        memcpy(inout, inputSave, sizeof(inputSave));
    }
    memCopyTime = TEST_GetTime() - oldTime;

    oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < FFT_TEST_LOOP; i++)
    {
        /* The CMSIS function changes the input data, so the input data is initialized every loop. */
        memcpy(inout, inputSave, sizeof(inputSave));
        arm_cfft_q31(&instance, inout, 1, 1);
    }
    allTime = TEST_GetTime() - oldTime;

    PRINTF("%s: %d ms\r\n", __func__, allTime - memCopyTime);

    for (uint32_t i = 0; i < ARRAY_SIZE(ref); i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(inout[i] - ref[i]) <= 1);
    }
}

static const float dct4RefResult[DCT_INPUT_LEN] = {
    3.69141984,  -4.11250544, 1.77583408,  -1.58607316, 1.05064547,  -0.97872084, 0.74445111,  -0.70727551, 0.57621890,
    -0.55361336, 0.46995211,  -0.45478085, 0.39676580,  -0.38588813, 0.34330326,  -0.33512598, 0.30254239,  -0.29617268,
    0.27043974,  -0.26533881, 0.24450305,  -0.24032666, 0.22311263,  -0.21963063, 0.20517029,  -0.20222303, 0.18990515,
    -0.18737841, 0.17676014,  -0.17457002, 0.16532272,  -0.16340627, 0.15528083,  -0.15358984, 0.14639407,  -0.14489107,
    0.13847430,  -0.13712962, 0.13137209,  -0.13016202, 0.12496730,  -0.12387264, 0.11916222,  -0.11816726, 0.11387660,
    -0.11296833, 0.10904387,  -0.10821149, 0.10460839,  -0.10384279, 0.10052326,  -0.09981674, 0.09674870,  -0.09609470,
    0.09325071,  -0.09264362, 0.09000011,  -0.08943506, 0.08697165,  -0.08644445, 0.08414343,  -0.08365042, 0.08149632,
    -0.08103430, 0.07901356,  -0.07857972, 0.07668041,  -0.07627224, 0.07448381,  -0.07409913, 0.07241221,  -0.07204906,
    0.07045529,  -0.07011193, 0.06860388,  -0.06827874, 0.06684973,  -0.06654141, 0.06518544,  -0.06489269, 0.06360434,
    -0.06332601, 0.06210040,  -0.06183547, 0.06066817,  -0.06041570, 0.05930270,  -0.05906184, 0.05799948,  -0.05776946,
    0.05675440,  -0.05653450, 0.05556369,  -0.05535328, 0.05442392,  -0.05422241, 0.05333195,  -0.05313878, 0.05228486,
    -0.05209954, 0.05127998,  -0.05110205, 0.05031487,  -0.05014390, 0.04938723,  -0.04922283, 0.04849497,  -0.04833677,
    0.04763614,  -0.04748381, 0.04680894,  -0.04666216, 0.04601167,  -0.04587015, 0.04524278,  -0.04510625, 0.04450082,
    -0.04436902, 0.04378441,  -0.04365712, 0.04309231,  -0.04296929, 0.04242332,  -0.04230437, 0.04177633,  -0.04166126,
    0.04115031,  -0.04103893, 0.04054427,  -0.04043642, 0.03995731,  -0.03985282, 0.03938856,  -0.03928729, 0.03883722,
    -0.03873902, 0.03830252,  -0.03820727, 0.03778375,  -0.03769131, 0.03728022,  -0.03719048, 0.03679131,  -0.03670415,
    0.03631641,  -0.03623173, 0.03585494,  -0.03577264, 0.03540637,  -0.03532635, 0.03497018,  -0.03489236, 0.03454590,
    -0.03447018, 0.03413305,  -0.03405936, 0.03373121,  -0.03365948, 0.03333997,  -0.03327011, 0.03295892,  -0.03289087,
    0.03258770,  -0.03252139, 0.03222595,  -0.03216132, 0.03187332,  -0.03181032, 0.03152951,  -0.03146807, 0.03119419,
    -0.03113426, 0.03086708,  -0.03080861, 0.03054790,  -0.03049084, 0.03023638,  -0.03018068, 0.02993227,  -0.02987788,
    0.02963531,  -0.02958220, 0.02934528,  -0.02929340, 0.02906196,  -0.02901128, 0.02878513,  -0.02873560, 0.02851459,
    -0.02846618, 0.02825014,  -0.02820281, 0.02799159,  -0.02794532, 0.02773877,  -0.02769352, 0.02749150,  -0.02744723,
    0.02724962,  -0.02720631, 0.02701296,  -0.02697059, 0.02678138,  -0.02673991, 0.02655473,  -0.02651414, 0.02633286,
    -0.02629312, 0.02611565,  -0.02607674, 0.02590296,  -0.02586485, 0.02569466,  -0.02565734, 0.02549064,  -0.02545408,
    0.02529078,  -0.02525496, 0.02509496,  -0.02505987, 0.02490309,  -0.02486870, 0.02471505,  -0.02468134, 0.02453074,
    -0.02449770, 0.02435007,  -0.02431769, 0.02417295,  -0.02414120, 0.02399928,  -0.02396815, 0.02382899,  -0.02379845,
    0.02366197,  -0.02363203, 0.02349816,  -0.02346879, 0.02333748,  -0.02330866, 0.02317984,  -0.02315157, 0.02302518,
    -0.02299744, 0.02287343,  -0.02284621, 0.02272451,  -0.02269780, 0.02257836,  -0.02255215, 0.02243493,  -0.02240920,
    0.02229413,  -0.02226887, 0.02215592,  -0.02213113, 0.02202024,  -0.02199590, 0.02188703,  -0.02186313, 0.02175624,
    -0.02173277, 0.02162780,  -0.02160476, 0.02150169,  -0.02147906, 0.02137783,  -0.02135560, 0.02125619,  -0.02123436,
    0.02113671,  -0.02111527, 0.02101936,  -0.02099830, 0.02090409,  -0.02088340, 0.02079085,  -0.02077053, 0.02067961,
    -0.02065964, 0.02057032,  -0.02055070, 0.02046295,  -0.02044367, 0.02035745,  -0.02033851, 0.02025380,  -0.02023519,
    0.02015195,  -0.02013366, 0.02005187,  -0.02003390, 0.01995352,  -0.01993587, 0.01985689,  -0.01983954, 0.01976192,
    -0.01974487, 0.01966859,  -0.01965184, 0.01957688,  -0.01956042, 0.01948675,  -0.01947057, 0.01939817,  -0.01938226,
    0.01931111,  -0.01929548, 0.01922556,  -0.01921020, 0.01914147,  -0.01912638, 0.01905883,  -0.01904400, 0.01897762,
    -0.01896304, 0.01889780,  -0.01888347, 0.01881936,  -0.01880528, 0.01874227,  -0.01872843, 0.01866650,  -0.01865290,
    0.01859205,  -0.01857868, 0.01851888,  -0.01850574, 0.01844697,  -0.01843406, 0.01837631,  -0.01836363, 0.01830688,
    -0.01829442, 0.01823865,  -0.01822641, 0.01817162,  -0.01815958, 0.01810575,  -0.01809392, 0.01804103,  -0.01802942,
    0.01797745,  -0.01796604, 0.01791499,  -0.01790378, 0.01785363,  -0.01784262, 0.01779336,  -0.01778254, 0.01773416,
    -0.01772353, 0.01767601,  -0.01766558, 0.01761891,  -0.01760866, 0.01756283,  -0.01755277, 0.01750777,  -0.01749789,
    0.01745370,  -0.01744400, 0.01740062,  -0.01739110, 0.01734852,  -0.01733917, 0.01729737,  -0.01728819, 0.01724717,
    -0.01723817, 0.01719791,  -0.01718908, 0.01714958,  -0.01714090, 0.01710215,  -0.01709365, 0.01705563,  -0.01704729,
    0.01701000,  -0.01700182, 0.01696525,  -0.01695722, 0.01692137,  -0.01691350, 0.01687835,  -0.01687063, 0.01683618,
    -0.01682862, 0.01679485,  -0.01678744, 0.01675435,  -0.01674709, 0.01671468,  -0.01670757, 0.01667581,  -0.01666885,
    0.01663776,  -0.01663094, 0.01660050,  -0.01659383, 0.01656403,  -0.01655750, 0.01652834,  -0.01652195, 0.01649343,
    -0.01648718, 0.01645928,  -0.01645316, 0.01642589,  -0.01641991, 0.01639324,  -0.01638740, 0.01636135,  -0.01635564,
    0.01633019,  -0.01632462, 0.01629976,  -0.01629432, 0.01627006,  -0.01626475, 0.01624107,  -0.01623589, 0.01621280,
    -0.01620775, 0.01618523,  -0.01618031, 0.01615837,  -0.01615357, 0.01613220,  -0.01612752, 0.01610672,  -0.01610217,
    0.01608192,  -0.01607750, 0.01605781,  -0.01605351, 0.01603437,  -0.01603019, 0.01601160,  -0.01600754, 0.01598949,
    -0.01598556, 0.01596805,  -0.01596423, 0.01594727,  -0.01594357, 0.01592713,  -0.01592355, 0.01590765,  -0.01590418,
    0.01588881,  -0.01588546, 0.01587061,  -0.01586738, 0.01585305,  -0.01584994, 0.01583613,  -0.01583313, 0.01581983,
    -0.01581695, 0.01580417,  -0.01580139, 0.01578913,  -0.01578647, 0.01577471,  -0.01577216, 0.01576091,  -0.01575847,
    0.01574772,  -0.01574540, 0.01573515,  -0.01573294, 0.01572319,  -0.01572109, 0.01571185,  -0.01570985, 0.01570110,
    -0.01569922, 0.01569097,  -0.01568919, 0.01568143,  -0.01567977, 0.01567250,  -0.01567094, 0.01566417,  -0.01566272,
    0.01565643,  -0.01565509, 0.01564929,  -0.01564806, 0.01564275,  -0.01564163, 0.01563680,  -0.01563579, 0.01563144,
    -0.01563054, 0.01562668,  -0.01562588, 0.01562251,  -0.01562181, 0.01561892,  -0.01561834, 0.01561593,  -0.01561545,
    0.01561352,  -0.01561315, 0.01561171,  -0.01561144, 0.01561048,  -0.01561032, 0.01560984,  -0.01560978,
};

/* Q15 DCT-IV */
static void arm_dct4_q15Test(void)
{
    /* N = 128. */
    q15_t inout[DCT_INPUT_LEN] = {0};
    q15_t input[DCT_INPUT_LEN] = {0};
    q15_t ref[DCT_INPUT_LEN];
    q15_t dctState[DCT_INPUT_LEN * 2] = {0};
    q15_t normalize;
    arm_dct4_instance_q15 dct4instance;
    arm_rfft_instance_q15 rfftinstance;
    arm_cfft_radix4_instance_q15 cfftinstance;

    /* Don't need to initiates other items, powerquad driver does not need. */
    memset(&cfftinstance, 0, sizeof(cfftinstance));
    cfftinstance.fftLen = DCT_INPUT_LEN;

    uint32_t allTime;
    uint32_t memCopyTime;
    uint32_t oldTime;

    /* Normalize value is sqrt(2/N) Q15. */
    normalize = 0x0800;

    for (uint32_t i = 0; i < ARRAY_SIZE(dct4RefResult); i++)
    {
        ref[i] = FLOAT_2_Q15(dct4RefResult[i] / (float)DCT_INPUT_LEN);
    }

    /* Input is 0.5 * (0...N-1) / N) */
    for (uint32_t i = 0; i < ARRAY_SIZE(dct4RefResult); i++)
    {
        input[i] = i * (0x4000 / DCT_INPUT_LEN);
    }

    /* Eliminate the time to copy memory. */
    oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < FFT_TEST_LOOP; i++)
    {
        memcpy(inout, input, sizeof(input));
    }
    memCopyTime = TEST_GetTime() - oldTime;

    oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < FFT_TEST_LOOP; i++)
    {
        memcpy(inout, input, sizeof(input));
        arm_dct4_init_q15(&dct4instance, &rfftinstance, &cfftinstance, DCT_INPUT_LEN, DCT_INPUT_LEN / 2, normalize);
        arm_dct4_q15(&dct4instance, dctState, inout);
    }
    allTime = TEST_GetTime() - oldTime;

    PRINTF("%s: %d ms\r\n", __func__, allTime - memCopyTime);

    for (uint32_t i = 0; i < DCT_INPUT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(inout[i] - ref[i]) <= 150);
    }
}

/* Q31 DCT-IV */
static void arm_dct4_q31Test(void)
{
    q31_t inout[DCT_INPUT_LEN] = {0};
    q31_t input[DCT_INPUT_LEN] = {0};
    q31_t ref[DCT_INPUT_LEN];
    q31_t dctState[DCT_INPUT_LEN * 2] = {0};
    q31_t normalize;
    arm_dct4_instance_q31 dct4instance;
    arm_rfft_instance_q31 rfftinstance;
    arm_cfft_radix4_instance_q31 cfftinstance;

    /* Don't need to initiates other items, powerquad driver does not need. */
    memset(&cfftinstance, 0, sizeof(cfftinstance));
    cfftinstance.fftLen = DCT_INPUT_LEN;

    uint32_t allTime;
    uint32_t memCopyTime;
    uint32_t oldTime;

    /* Normalize value is sqrt(2/N) Q31. */
    normalize = 0x08000000;

    for (uint32_t i = 0; i < ARRAY_SIZE(dct4RefResult); i++)
    {
        ref[i] = FLOAT_2_Q31(dct4RefResult[i] / (float)DCT_INPUT_LEN) >> 5;
    }

    /* Input is 0.5 * (0...N-1) / N) */
    for (uint32_t i = 0; i < ARRAY_SIZE(dct4RefResult); i++)
    {
        input[i] = i * (0x40000000 / DCT_INPUT_LEN) >> 5;
    }

    /* Eliminate the time to copy memory. */
    oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < FFT_TEST_LOOP; i++)
    {
        memcpy(inout, input, sizeof(input));
    }
    memCopyTime = TEST_GetTime() - oldTime;

    oldTime = TEST_GetTime();
    for (uint32_t i = 0; i < FFT_TEST_LOOP; i++)
    {
        memcpy(inout, input, sizeof(input));
        arm_dct4_init_q31(&dct4instance, &rfftinstance, &cfftinstance, DCT_INPUT_LEN, DCT_INPUT_LEN / 2, normalize);
        arm_dct4_q31(&dct4instance, dctState, inout);
    }
    allTime = TEST_GetTime() - oldTime;

    PRINTF("%s: %d ms\r\n", __func__, allTime - memCopyTime);

    for (uint32_t i = 0; i < DCT_INPUT_LEN; i++)
    {
        EXAMPLE_ASSERT_TRUE(abs(inout[i] - ref[i]) <= 30);
    }
}
