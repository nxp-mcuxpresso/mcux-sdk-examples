/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include <stdlib.h>
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_puf.h"
#include "fsl_hashcrypt.h"

#include "fsl_power.h"
#include "fsl_rng.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CORE_CLK_FREQ CLOCK_GetFreq(kCLOCK_CoreSysClk)
/* Worst-case time in ms to fully discharge PUF SRAM */
#define PUF_DISCHARGE_TIME 400
#define BOARD_RandomInit() RNG_Init(RNG)
#define GetRandomData32(x) RNG_GetRandomData(RNG, x, 4U);
#define PUF_INTRINSIC_KEY_SIZE 16

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* User key in little-endian format. */
/* 32 bytes key for ECB method: "Thispasswordisveryuncommonforher". */
static const uint8_t s_userKey256[] __attribute__((aligned)) = {
    0x72, 0x65, 0x68, 0x72, 0x6f, 0x66, 0x6e, 0x6f, 0x6d, 0x6d, 0x6f, 0x63, 0x6e, 0x75, 0x79, 0x72,
    0x65, 0x76, 0x73, 0x69, 0x64, 0x72, 0x6f, 0x77, 0x73, 0x73, 0x61, 0x70, 0x73, 0x69, 0x68, 0x54};

static uint8_t s_OutputUserKey256[32] __attribute__((aligned)) = {0};

static const uint8_t s_EcbPlain[] =
    "Be that word our sign of parting, bird or fiend! I shrieked upstarting"
    "Get thee back into the tempest and the Nights Plutonian shore!"
    "Leave no black plume as a token of that lie thy soul hath spoken!"
    "Leave my loneliness unbroken! quit the bust above my door!"
    "Take thy beak from out my heart, and take thy form from off my door!"
    "Quoth the raven, Nevermore. ";

static const uint8_t s_EcbExpected[] = {
    0x51, 0xcc, 0x90, 0xbf, 0xb5, 0x98, 0x3c, 0xcc, 0xb8, 0x6f, 0xb3, 0x6b, 0x12, 0x55, 0x2d, 0xc3, 0xe6, 0xcd, 0xd2,
    0x99, 0xd8, 0x58, 0xa9, 0xfb, 0x0c, 0xca, 0x59, 0x29, 0x82, 0xb8, 0x0a, 0xcf, 0x4f, 0x44, 0x8a, 0xeb, 0x5f, 0x3c,
    0x29, 0xbb, 0xf6, 0xbc, 0xfe, 0x9f, 0x99, 0xfe, 0xe6, 0xc9, 0x92, 0x6e, 0x19, 0x98, 0x22, 0x68, 0x65, 0x64, 0xad,
    0xa9, 0x7d, 0x1c, 0xd5, 0x66, 0xd5, 0xb2, 0xc2, 0xb5, 0xe0, 0x24, 0x5b, 0x8d, 0xea, 0x2a, 0x02, 0x24, 0x3e, 0xe6,
    0x7b, 0x84, 0xb0, 0x42, 0xb8, 0x05, 0x91, 0xea, 0xbf, 0xd2, 0x60, 0x3a, 0xf9, 0xbe, 0xa7, 0x0d, 0x61, 0x29, 0xdf,
    0xd9, 0xd9, 0x26, 0x67, 0x30, 0xe9, 0xee, 0x0d, 0x00, 0x77, 0x5e, 0xcd, 0xef, 0xb5, 0x6f, 0x16, 0x10, 0x9f, 0xdf,
    0xa1, 0x49, 0x96, 0xf9, 0xaa, 0xb1, 0xa7, 0xbc, 0xb2, 0xcd, 0x09, 0xaf, 0xc1, 0xd8, 0x43, 0xd9, 0x2c, 0xd1, 0xd0,
    0xb6, 0x94, 0x93, 0x0b, 0x8f, 0x5a, 0xb7, 0x0c, 0xc9, 0x8c, 0xe2, 0x06, 0x1f, 0x81, 0xe4, 0xf3, 0x08, 0x29, 0x6b,
    0x13, 0x77, 0x81, 0xe5, 0x9c, 0x1f, 0xe7, 0x0a, 0x05, 0xfa, 0x47, 0x17, 0x72, 0xcf, 0x76, 0x71, 0x65, 0x38, 0x49,
    0xc2, 0xe9, 0x17, 0x5b, 0xe0, 0x50, 0x2e, 0x98, 0x1f, 0x61, 0x08, 0x29, 0x12, 0x0e, 0x5e, 0x24, 0xfc, 0x91, 0x0d,
    0x9c, 0x5d, 0x81, 0x38, 0x2b, 0xd3, 0x2a, 0x73, 0x73, 0x53, 0x30, 0x28, 0x1e, 0xca, 0xf9, 0xaa, 0x7f, 0x7b, 0x4c,
    0x22, 0x18, 0x0b, 0x9a, 0x13, 0xef, 0x9c, 0xa2, 0x6c, 0xd0, 0x25, 0xa3, 0xc0, 0xab, 0x49, 0x7e, 0x66, 0xb8, 0x29,
    0x17, 0xca, 0xd3, 0x02, 0xbc, 0x84, 0x8a, 0x2d, 0x49, 0x78, 0x90, 0x14, 0x23, 0x99, 0xc2, 0xed, 0xc5, 0x8c, 0xad,
    0x4f, 0x9a, 0xdb, 0xe8, 0x41, 0x3a, 0xaf, 0x6c, 0x43, 0xbc, 0x5d, 0x24, 0xba, 0xc0, 0x71, 0x91, 0xe0, 0x1e, 0x10,
    0xc9, 0x7b, 0xb8, 0xe7, 0x16, 0x7f, 0x6a, 0x9c, 0x6d, 0x4a, 0xda, 0xaa, 0x8e, 0x49, 0x88, 0x55, 0xcf, 0xac, 0x39,
    0x04, 0xf9, 0x65, 0x26, 0x8f, 0xd5, 0x9d, 0x87, 0xc8, 0xb4, 0xdf, 0xfd, 0x94, 0x3b, 0x81, 0xe6, 0x4d, 0xdb, 0xb6,
    0xf1, 0xd3, 0x54, 0x83, 0xff, 0x5b, 0x26, 0x14, 0x51, 0x6a, 0xb2, 0x02, 0x2e, 0x67, 0x6c, 0x76, 0x2d, 0xc6, 0x38,
    0xe6, 0x2d, 0xd0, 0x68, 0xab, 0xed, 0xfc, 0x2b, 0x57, 0x9e, 0xc3, 0x7e, 0xe3, 0x14, 0x71, 0xce, 0xce, 0x9a, 0x76,
    0xb9, 0x1c, 0x95, 0xb4, 0x9a, 0xf1, 0xbb, 0xbd, 0x9f, 0x4d};

uint8_t cipher1[sizeof(s_EcbPlain)] = {0};
uint8_t cipher2[sizeof(s_EcbPlain)] = {0};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function.
 */
int main(void)
{
    status_t result;
    uint8_t activationCode[PUF_ACTIVATION_CODE_SIZE];
    uint8_t keyCode0[PUF_GET_KEY_CODE_SIZE_FOR_KEY_SIZE(32)];
    uint8_t keyCode1[PUF_GET_KEY_CODE_SIZE_FOR_KEY_SIZE(32)];
    uint32_t random;

    /* Init hardware */
    /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_RandomInit();

    PRINTF("PUF and HACHCRYPT Peripheral Driver Example\r\n\r\n");

    /* PUF SRAM Configuration*/
    puf_config_t conf;
    PUF_GetDefaultConfig(&conf);

    /* Initialize PUF peripheral */
    result = PUF_Init(PUF, &conf);
    if (result != kStatus_Success)
    {
        PRINTF("Error Initializing PUF!\r\n");
    }

    /* Perform enroll to get device specific PUF activation code */
    /* Note: Enroll operation is usually performed only once for each device. */
    /* Activation code is stored and used in Start operation */
    result = PUF_Enroll(PUF, activationCode, sizeof(activationCode));
    if (result == kStatus_EnrollNotAllowed)
    {
        PRINTF("Enroll is not allowed!\r\n");
        PRINTF("Will make powercycle and enroll again!\r\n");
        (void)PUF_PowerCycle(PUF, &conf);
        result = PUF_Enroll(PUF, activationCode, sizeof(activationCode));
    }

    if (result != kStatus_Success)
    {
        PRINTF("Error during Enroll!\r\n");
    }
    else
    {
        PRINTF("PUF Enroll success\r\n");
    }
    PUF_Deinit(PUF, &conf);

    /* Reinitialize PUF after enroll */
    result = PUF_Init(PUF, &conf);
    if (result != kStatus_Success)
    {
        PRINTF("Error Initializing PUF!\r\n");
    }

    /* Start PUF by loading generated activation code */
    result = PUF_Start(PUF, activationCode, sizeof(activationCode));
    if (result == kStatus_StartNotAllowed)
    {
        PRINTF("Start is not allowed!\r\n");
        PRINTF("Will make powercycle and start again!\r\n");
        (void)PUF_PowerCycle(PUF, &conf);
        result = PUF_Start(PUF, activationCode, sizeof(activationCode));
    }

    if (result != kStatus_Success)
    {
        PRINTF("Error during Start !\r\n");
    }
    else
    {
        PRINTF("PUF Start success\r\n\r\n");
    }

    PRINTF("User key:\r\n");
    for (int i = 0; i < sizeof(s_userKey256); i++)
    {
        PRINTF("%x ", s_userKey256[i]);

        if (i == 15U)
        {
            PRINTF("\r\n");
        }
    }
    PRINTF("\r\n\r\n");

    /* Create keycode for user key with index 0 */
    /* Index 0 selects that the key shall be ouptut (by PUF_GetHwKey()) to a SoC specific private hardware bus. */
    result = PUF_SetUserKey(PUF, kPUF_KeyIndex_00, s_userKey256, 32, keyCode0, sizeof(keyCode0));
    if (result != kStatus_Success)
    {
        PRINTF("Error setting user key on index 0!\r\n");
    }
    else
    {
        PRINTF("User key successfully set for HW bus crypto module\r\n\r\n");
    }

    /* Store user key on index 1 */
    result = PUF_SetUserKey(PUF, kPUF_KeyIndex_01, s_userKey256, 32, keyCode1, sizeof(keyCode1));
    if (result != kStatus_Success)
    {
        PRINTF("Error setting user key!\r\n");
    }
    else
    {
        PRINTF("User key successfully set on PUF index 1\r\n\r\n");
    }

    /* Prepare 32 bit random data used in PUF_GetHwKey() */
    GetRandomData32(&random);
    /* Reconstruct key from keyCode0 to HW bus for crypto module */
    result = PUF_GetHwKey(PUF, keyCode0, sizeof(keyCode0), kPUF_KeySlot0, random);
    if (result != kStatus_Success)
    {
        PRINTF("Error reconstructing key to HW bus!\r\n");
    }
    else
    {
        PRINTF("Successfully reconstructed secret key to HW bus for crypto module\r\n\r\n");
    }

    /* Reconstruct key from keyCode1 to output buffer s_OutputUserKey256 */
    result = PUF_GetKey(PUF, keyCode1, sizeof(keyCode1), s_OutputUserKey256, sizeof(s_OutputUserKey256));
    if (result != kStatus_Success)
    {
        PRINTF("Error reconstructing key to output buffer!\r\n");
    }

    /* Check output key */
    if (memcmp(s_userKey256, s_OutputUserKey256, sizeof(s_userKey256)))
    {
        PRINTF("Error reconstructed user key is invalid");
    }
    else
    {
        PRINTF("Successfully reconstructed user key:\r\n");
        for (int i = 0; i < sizeof(s_OutputUserKey256); i++)
        {
            PRINTF("%x ", s_OutputUserKey256[i]);

            if (i == 15U)
            {
                PRINTF("\r\n");
            }
        }
        PRINTF("\r\n\r\n");
    }

    /* Encrypt plaintext via Hascrypt using User key */
    HASHCRYPT_Init(HASHCRYPT);
    hashcrypt_handle_t m_handle;
    m_handle.keyType = kHASHCRYPT_UserKey;
    result           = HASHCRYPT_AES_SetKey(HASHCRYPT, &m_handle, s_userKey256, 32U);
    if (result != kStatus_Success)
    {
        PRINTF("Error setting user key!\r\n");
    }
    else
    {
        PRINTF("Setting user key for HASHCRYPT encryption\r\n\r\n");
    }
    result = HASHCRYPT_AES_EncryptEcb(HASHCRYPT, &m_handle, s_EcbPlain, cipher1, sizeof(s_EcbPlain));
    if (result != kStatus_Success)
    {
        PRINTF("Error encrypting using user key!\r\n");
    }
    else
    {
        PRINTF("Encryption success! Printing first 16 bytes:\r\n");
        for (int i = 0; i < 16; i++)
        {
            PRINTF("%x ", cipher1[i]);
        }
        PRINTF("\r\n\r\n");
    }

    /* Encrypt plaintext via Hascrypt using HW secret key */
    m_handle.keyType = kHASHCRYPT_SecretKey;
    result           = HASHCRYPT_AES_SetKey(HASHCRYPT, &m_handle, NULL, 32);
    if (result != kStatus_Success)
    {
        PRINTF("Error setting secret key!\r\n");
    }
    else
    {
        PRINTF("Setting HW bus secret key for HASHCRYPT encryption\r\n\r\n");
    }
    result = HASHCRYPT_AES_EncryptEcb(HASHCRYPT, &m_handle, s_EcbPlain, cipher2, sizeof(s_EcbPlain));

    if (result != kStatus_Success)
    {
        PRINTF("Error encrypting using secret HW bus key!\r\n");
    }
    else
    {
        PRINTF("Encryption success! Printing first 16 bytes:\r\n");
        for (int i = 0; i < 16; i++)
        {
            PRINTF("%x ", cipher2[i]);
        }
        PRINTF("\r\n\r\n");
    }

    HASHCRYPT_Deinit(HASHCRYPT);
    PUF_Deinit(PUF, &conf);

    /* Check both cipher texts are correct */
    if (memcmp(s_EcbExpected, cipher1, sizeof(s_EcbExpected)) || memcmp(s_EcbExpected, cipher2, sizeof(s_EcbExpected)))
    {
        PRINTF("Error encrypted outputs are incorrect!\r\n");
    }
    else
    {
        PRINTF("Success: encrypted outputs are correct\r\n");
    }

    PRINTF("\r\n\nExample end.\r\n");

    while (1)
    {
    }
}
