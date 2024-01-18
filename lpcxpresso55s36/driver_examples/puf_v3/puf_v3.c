/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "unity.h"
#include "fsl_puf_v3.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CORE_CLK_FREQ CLOCK_GetFreq(kCLOCK_CoreSysClk)
/* Using PUF_ALIAS_0 in this example */
#ifdef PUF_ALIAS_0
#define PUF PUF_ALIAS_0
#endif
/* Activation Code FFR offset */
#define APP_KEYSTORE_OFFSET (0x3e600)
/* Offset in FFR Memory to access Key Store */
#define FFR_KEYSTORE_OFFSET FSL_FEATURE_PUF_ACTIVATION_CODE_ADDRESS
/* PUF header Valid constant */
#define PUF_FFR_HEADER_VALID 0x95959595u

/* Devices have provisioned PUF from NXP factory. This means that PUF activation code is already present in NXP
 * programmed Key Store Area in FFR. Also device certificate and device private keys derived from activation key have
 * been programmed in NMPA area. In most cases, user can reuse PUF activation code stored in FFR to start the PUF, so it
 * is not necessary to perform PUF enroll again. In case that user application requires to have a separate PUF key store
 * which is independent on the NXP programmed key store, it is possible to enroll PUF again and store the new AC at user
 * defined location in non-volatile memory. It is strongly not recommended to update the PUF activation code stored in
 * NXP programmed key store area in FFR, since device certificate and device private keys will become unusable. */
#define DO_ENROLL 0

/*******************************************************************************
 * Variables
 ******************************************************************************/

#if defined(DO_ENROLL) && (DO_ENROLL > 0)
uint8_t SDK_ALIGN(g_AC[PUF_ACTIVATION_CODE_SIZE], 8u) = {0};
#endif

puf_config_t g_pufConfig;

/* User key in little-endian format. */
/* "Thispasswordisveryuncommonforher". */
static const uint8_t s_userKey[] __attribute__((aligned)) = {
    0x72, 0x65, 0x68, 0x72, 0x6f, 0x66, 0x6e, 0x6f, 0x6d, 0x6d, 0x6f, 0x63, 0x6e, 0x75, 0x79, 0x72,
    0x65, 0x76, 0x73, 0x69, 0x64, 0x72, 0x6f, 0x77, 0x73, 0x73, 0x61, 0x70, 0x73, 0x69, 0x68, 0x54};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

typedef struct
{
    uint32_t headerValid[1];
    uint32_t dischargeTime[1]; /* Legacy - not used on LPC55S3x */
    uint8_t activationCode[PUF_ACTIVATION_CODE_SIZE];
} ffr_key_store_t;

/*******************************************************************************
 * Code
 ******************************************************************************/

static void TestSelftest(void)
{
    uint8_t score = 0;
    status_t result;

    PUF_GetDefaultConfig(&g_pufConfig);

    /* Initialize PUF */
    result = PUF_Init(PUF, &g_pufConfig);
    TEST_ASSERT_EQUAL(result, kStatus_Success);

    /* Run internal PUF self test */
    result = PUF_Test(PUF, &score);
    TEST_ASSERT_EQUAL(result, kStatus_Success);
    TEST_ASSERT_TRUE(score <= 7u);
}

static void TestGenerateRandomData(void)
{
    SDK_ALIGN(uint8_t data1[256], 8u) = {0};
    SDK_ALIGN(uint8_t data2[256], 8u) = {0};
    uint8_t zeroData[256]             = {0};
    status_t result;

    /* Generate random */
    result = PUF_GenerateRandom(PUF, data1, sizeof(data1));
    TEST_ASSERT_EQUAL(result, kStatus_Success);
    result = PUF_GenerateRandom(PUF, data2, sizeof(data2));
    TEST_ASSERT_EQUAL(result, kStatus_Success);

    /* Check that data1 are not zero */
    TEST_ASSERT_FALSE(memcmp(data1, zeroData, sizeof(data1)) == 0);
    /* Check that data2 are not zero */
    TEST_ASSERT_FALSE(memcmp(data2, zeroData, sizeof(data2)) == 0);
    /* Check that data1 are not the same as data2 */
    TEST_ASSERT_FALSE(memcmp(data1, data2, sizeof(data1)) == 0);
}

#if defined(DO_ENROLL) && (DO_ENROLL > 0)
static void TestEnroll(void)
{
    uint8_t pufScore                         = 0;
    uint8_t zeroAC[PUF_ACTIVATION_CODE_SIZE] = {0};
    SDK_ALIGN(uint8_t keyCode[PUF_GET_KEY_CODE_SIZE_FOR_KEY_SIZE(32)], 8u);
    SDK_ALIGN(uint8_t key[32], 8u);
    puf_key_ctx_t keyCtx;
    status_t result;

    /* Enroll */
    result = PUF_Enroll(PUF, g_AC, sizeof(g_AC), &pufScore);
    TEST_ASSERT_EQUAL(result, kStatus_Success);
    TEST_ASSERT_TRUE(pufScore <= 7u);

    TEST_ASSERT_FALSE(memcmp(g_AC, zeroAC, PUF_ACTIVATION_CODE_SIZE) == 0);

    /* Test enrolled state key scope */
    keyCtx.keyScopeEnrolled = kPUF_KeyAllowAll;
    keyCtx.keyScopeStarted  = kPUF_KeyAllowAll;
    keyCtx.userCtx0         = 0u;
    keyCtx.userCtx1         = 0u;

    /* Test kPUF_KeyAllowAll */
    result = PUF_WrapGeneratedRandom(PUF, &keyCtx, 32u, keyCode, sizeof(keyCode));
    TEST_ASSERT_EQUAL(result, kStatus_Success);
    result = PUF_Unwrap(PUF, kPUF_KeyDestRegister, keyCode, sizeof(keyCode), key, sizeof(key));
    TEST_ASSERT_EQUAL(result, kStatus_Success);

    /* Test kPUF_KeyAllowRegister */
    keyCtx.keyScopeEnrolled = kPUF_KeyAllowRegister;
    result                  = PUF_WrapGeneratedRandom(PUF, &keyCtx, 32u, keyCode, sizeof(keyCode));
    TEST_ASSERT_EQUAL(result, kStatus_Success);
    result = PUF_Unwrap(PUF, kPUF_KeyDestRegister, keyCode, sizeof(keyCode), key, sizeof(key));
    TEST_ASSERT_EQUAL(result, kStatus_Success);

    /* Test kPUF_KeyAllowKeyBus */
    keyCtx.keyScopeEnrolled = kPUF_KeyAllowKeyBus;
    result                  = PUF_WrapGeneratedRandom(PUF, &keyCtx, 32u, keyCode, sizeof(keyCode));
    TEST_ASSERT_EQUAL(result, kStatus_Success);
    result = PUF_Unwrap(PUF, kPUF_KeyDestRegister, keyCode, sizeof(keyCode), key, sizeof(key));
    TEST_ASSERT_EQUAL(result, kStatus_PUF_DestinationNotAllowed);
}

static void TestStartAfterEnroll()
{
    uint8_t pufScore = 0;
    SDK_ALIGN(uint8_t keyCode[PUF_GET_KEY_CODE_SIZE_FOR_KEY_SIZE(32)], 8U);
    SDK_ALIGN(uint8_t key[32], 8u);
    puf_key_ctx_t keyCtx;
    status_t result;

    PUF_Deinit(PUF, &g_pufConfig);
    result = PUF_Init(PUF, &g_pufConfig);
    TEST_ASSERT_EQUAL(result, kStatus_Success);

    /* Use global variable as AC*/
    result = PUF_Start(PUF, g_AC, sizeof(g_AC), &pufScore);
    TEST_ASSERT_EQUAL(result, kStatus_Success);
    TEST_ASSERT_TRUE(pufScore <= 7u);

    /* Test started state key scope */
    keyCtx.keyScopeEnrolled = kPUF_KeyAllowAll;
    keyCtx.keyScopeStarted  = kPUF_KeyAllowAll;
    keyCtx.userCtx0         = 0u;
    keyCtx.userCtx1         = 0u;

    /* Test kPUF_KeyAllowAll */
    result = PUF_WrapGeneratedRandom(PUF, &keyCtx, 32u, keyCode, sizeof(keyCode));
    TEST_ASSERT_EQUAL(result, kStatus_Success);
    result = PUF_Unwrap(PUF, kPUF_KeyDestRegister, keyCode, sizeof(keyCode), key, sizeof(key));
    TEST_ASSERT_EQUAL(result, kStatus_Success);

    /* Test kPUF_KeyAllowRegister */
    keyCtx.keyScopeStarted = kPUF_KeyAllowRegister;
    result                 = PUF_WrapGeneratedRandom(PUF, &keyCtx, 32u, keyCode, sizeof(keyCode));
    TEST_ASSERT_EQUAL(result, kStatus_Success);
    result = PUF_Unwrap(PUF, kPUF_KeyDestRegister, keyCode, sizeof(keyCode), key, sizeof(key));
    TEST_ASSERT_EQUAL(result, kStatus_Success);

    /* Test kPUF_KeyAllowKeyBus */
    keyCtx.keyScopeStarted = kPUF_KeyAllowKeyBus;
    result                 = PUF_WrapGeneratedRandom(PUF, &keyCtx, 32u, keyCode, sizeof(keyCode));
    TEST_ASSERT_EQUAL(result, kStatus_Success);
    result = PUF_Unwrap(PUF, kPUF_KeyDestRegister, keyCode, sizeof(keyCode), key, sizeof(key));
    TEST_ASSERT_EQUAL(result, kStatus_PUF_DestinationNotAllowed);
}
#endif /* (DO_ENROLL) && (DO_ENROLL > 0) */

static void TestStart()
{
    uint8_t pufScore = 0;
    SDK_ALIGN(uint8_t keyCode[PUF_GET_KEY_CODE_SIZE_FOR_KEY_SIZE(32)], 8U);
    SDK_ALIGN(uint8_t key[32], 8u);
    puf_key_ctx_t keyCtx;
    status_t result;

    PUF_Deinit(PUF, &g_pufConfig);
    result = PUF_Init(PUF, &g_pufConfig);
    TEST_ASSERT_EQUAL(result, kStatus_Success);

    ffr_key_store_t *ffrKeyStore = (ffr_key_store_t *)FFR_KEYSTORE_OFFSET;

    /* Check if Key Store Header in FFR is valid */
    TEST_ASSERT_EQUAL(*ffrKeyStore->headerValid, PUF_FFR_HEADER_VALID);

    /* Use FFR Key Store as AC */
    result = PUF_Start(PUF, ffrKeyStore->activationCode, PUF_ACTIVATION_CODE_SIZE, &pufScore);
    TEST_ASSERT_EQUAL(result, kStatus_Success);
    TEST_ASSERT_TRUE(pufScore <= 7u);

    /* Test started state key scope */
    keyCtx.keyScopeEnrolled = kPUF_KeyAllowAll;
    keyCtx.keyScopeStarted  = kPUF_KeyAllowAll;
    keyCtx.userCtx0         = 0u;
    keyCtx.userCtx1         = 0u;

    /* Test kPUF_KeyAllowAll */
    result = PUF_WrapGeneratedRandom(PUF, &keyCtx, 32u, keyCode, sizeof(keyCode));
    TEST_ASSERT_EQUAL(result, kStatus_Success);
    result = PUF_Unwrap(PUF, kPUF_KeyDestRegister, keyCode, sizeof(keyCode), key, sizeof(key));
    TEST_ASSERT_EQUAL(result, kStatus_Success);

    /* Test kPUF_KeyAllowRegister */
    keyCtx.keyScopeStarted = kPUF_KeyAllowRegister;
    result                 = PUF_WrapGeneratedRandom(PUF, &keyCtx, 32u, keyCode, sizeof(keyCode));
    TEST_ASSERT_EQUAL(result, kStatus_Success);
    result = PUF_Unwrap(PUF, kPUF_KeyDestRegister, keyCode, sizeof(keyCode), key, sizeof(key));
    TEST_ASSERT_EQUAL(result, kStatus_Success);

    /* Test kPUF_KeyAllowKeyBus */
    keyCtx.keyScopeStarted = kPUF_KeyAllowKeyBus;
    result                 = PUF_WrapGeneratedRandom(PUF, &keyCtx, 32u, keyCode, sizeof(keyCode));
    TEST_ASSERT_EQUAL(result, kStatus_Success);
    result = PUF_Unwrap(PUF, kPUF_KeyDestRegister, keyCode, sizeof(keyCode), key, sizeof(key));
    TEST_ASSERT_EQUAL(result, kStatus_PUF_DestinationNotAllowed);
}

static void TestDeriveDeviceKey(void)
{
    puf_key_ctx_t keyCtx;
    status_t result;
    SDK_ALIGN(uint8_t key1[512], 8u) = {0};
    SDK_ALIGN(uint8_t key2[512], 8u) = {0};
    uint8_t keyZero[512]             = {0};

    /* Fill in key context */
    keyCtx.keyScopeEnrolled = kPUF_KeyAllowAll;
    keyCtx.keyScopeStarted  = kPUF_KeyAllowAll;
    keyCtx.userCtx0         = 0u;
    keyCtx.userCtx1         = 0u;

    /* Derive first device key */
    result = PUF_GetKey(PUF, &keyCtx, kPUF_KeyDestRegister, key1, sizeof(key1));
    TEST_ASSERT_EQUAL(result, kStatus_Success);

    /* Derive second device key - need different context */
    keyCtx.userCtx1 = 2u; /* possible values here is limited by qk_restrict_user_context_0 */
    result          = PUF_GetKey(PUF, &keyCtx, kPUF_KeyDestRegister, key2, sizeof(key2));
    TEST_ASSERT_EQUAL(result, kStatus_Success);

    /* Check that key1 is not zero */
    TEST_ASSERT_FALSE(memcmp(keyZero, key1, sizeof(key1)) == 0);
    /* Check that key2 is not zero */
    TEST_ASSERT_FALSE(memcmp(keyZero, key2, sizeof(key1)) == 0);
    /* Check that key1 is not the same as key2 */
    TEST_ASSERT_FALSE(memcmp(key1, key2, sizeof(key1)) == 0);
}

static void TestRandomWrapUnwrap(void)
{
    puf_key_ctx_t keyCtx;
    SDK_ALIGN(uint8_t randKey[128], 8u);
    uint8_t zeroKey[sizeof(randKey)] = {0};
    SDK_ALIGN(uint8_t keyCode[PUF_GET_KEY_CODE_SIZE_FOR_KEY_SIZE(sizeof(randKey))], 8u);
    status_t result;

    /* Fill in key context */
    keyCtx.keyScopeEnrolled = kPUF_KeyAllowRegister;
    keyCtx.keyScopeStarted  = kPUF_KeyAllowRegister;
    keyCtx.userCtx0         = 0u;
    keyCtx.userCtx1         = 0u;

    /* Wrap */
    result = PUF_WrapGeneratedRandom(PUF, &keyCtx, sizeof(randKey), keyCode, sizeof(keyCode));
    TEST_ASSERT_EQUAL(result, kStatus_Success);

    /* Unwrap */
    result = PUF_Unwrap(PUF, kPUF_KeyDestRegister, keyCode, sizeof(keyCode), randKey, sizeof(randKey));
    TEST_ASSERT_EQUAL(result, kStatus_Success);

    /* randKey must contain non-zero data */
    TEST_ASSERT_FALSE(memcmp(randKey, zeroKey, sizeof(randKey)) == 0);
}

static void TestUserKeyWrapUnwrap(void)
{
    puf_key_ctx_t keyCtx;
    SDK_ALIGN(uint8_t keyCode[PUF_GET_KEY_CODE_SIZE_FOR_KEY_SIZE(sizeof(s_userKey))], 8u);
    SDK_ALIGN(uint8_t key[sizeof(s_userKey)], 8u);
    status_t result;

    /* Fill in key context */
    keyCtx.keyScopeEnrolled = kPUF_KeyAllowRegister;
    keyCtx.keyScopeStarted  = kPUF_KeyAllowRegister;
    keyCtx.userCtx0         = 0u;
    keyCtx.userCtx1         = 0u;

    /* Wrap */
    result = PUF_Wrap(PUF, &keyCtx, (uint8_t *)s_userKey, sizeof(s_userKey), keyCode, sizeof(keyCode));
    TEST_ASSERT_EQUAL(result, kStatus_Success);

    /* Unwrap */
    result = PUF_Unwrap(PUF, kPUF_KeyDestRegister, keyCode, sizeof(keyCode), key, sizeof(key));
    TEST_ASSERT_EQUAL(result, kStatus_Success);

    /* Check that unwrapped key is equal to s_userKey */
    TEST_ASSERT_TRUE(memcmp(key, s_userKey, sizeof(key)) == 0);
}

static void TestStop(void)
{
    SDK_ALIGN(uint8_t data[128], 8u);
    puf_key_ctx_t keyCtx;
    status_t result;

    /* Stop PUF */
    result = PUF_Stop(PUF);
    TEST_ASSERT_EQUAL(result, kStatus_Success);

    /* Generate random data operation should be alowed */
    result = PUF_GenerateRandom(PUF, data, sizeof(data));
    TEST_ASSERT_EQUAL(result, kStatus_Success);

    /* Key operations must be rejected in stopped state */
    result = PUF_GetKey(PUF, &keyCtx, kPUF_KeyDestRegister, data, sizeof(data));
    TEST_ASSERT_EQUAL(result, kStatus_PUF_OperationNotAllowed);
}

static void TestZeroize(void)
{
    status_t result;

    /* Zeroize PUF */
    result = PUF_Zeroize(PUF);
    TEST_ASSERT_EQUAL(result, kStatus_Success);

    /* Allow register must be set to zero when zeroized */
    TEST_ASSERT_EQUAL(PUF->AR, 0u);
}

void setUp(void)
{
}

void tearDown(void)
{
}

/*!
 * @brief Main function.
 */
int main(void)
{
    uint8_t testCount = 0;

    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    PRINTF("PUFv3 driver example\n\r");

    UnityBegin();

    /* Run selftest */
    RUN_EXAMPLE(TestSelftest, MAKE_UNITY_NUM(k_unity_puf, testCount++));
    /* Generate random data */
    RUN_EXAMPLE(TestGenerateRandomData, MAKE_UNITY_NUM(k_unity_puf, testCount++));
#if defined(DO_ENROLL) && (DO_ENROLL > 0)
    PRINTF(
        "///////////////////////////////////////////  NOTE  //////////////////////////////////////////////////////\r\n"
        "PUF activation code is already present in NXP programmed Key Store Area in FFR.\r\n"
        "Also device certificate and device private keys derived from activation key have been programmed in NMPA "
        "area\r\n"
        "In most cases, Enroll in not wanted/needed. Calling PUF_Enroll() will not override data in CMPA/NMPA.\r\n"
        "//////////////////////////////////////////////////////////////////////////////////////////////////////////"
        "\r\n");
    /* Enroll */
    RUN_EXAMPLE(TestEnroll, MAKE_UNITY_NUM(k_unity_puf, testCount++));
    /* Start with newly created Activation code in g_AC buffer */
    /* Note: this AC will not override factory prvisioned data in FFR */
    /* To override it, user need to do FFR write into NMPA */
    RUN_EXAMPLE(TestStartAfterEnroll, MAKE_UNITY_NUM(k_unity_puf, testCount++));
#endif /* (DO_ENROLL) && (DO_ENROLL > 0) */
    /* Start with factory privsioned Activation code in FFR */
    RUN_EXAMPLE(TestStart, MAKE_UNITY_NUM(k_unity_puf, testCount++));
    /* Derive Device Key */
    RUN_EXAMPLE(TestDeriveDeviceKey, MAKE_UNITY_NUM(k_unity_puf, testCount++));
    /* Wrap and Unwrap random data */
    RUN_EXAMPLE(TestRandomWrapUnwrap, MAKE_UNITY_NUM(k_unity_puf, testCount++));
    /* Wrap and Unwrap user data */
    RUN_EXAMPLE(TestUserKeyWrapUnwrap, MAKE_UNITY_NUM(k_unity_puf, testCount++));
    /* Stop */
    RUN_EXAMPLE(TestStop, MAKE_UNITY_NUM(k_unity_puf, testCount++));
    /* Zeroize */
    RUN_EXAMPLE(TestZeroize, MAKE_UNITY_NUM(k_unity_puf, testCount++));

    UnityEnd();

    while (1)
    {
    }
}
