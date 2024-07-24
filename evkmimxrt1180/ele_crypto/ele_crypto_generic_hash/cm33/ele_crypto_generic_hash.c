/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "ele_crypto.h" /* ELE Crypto SW */
#include "ele_fw.h"     /* ELE FW, to be placed in bootable container in real world app */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define S3MU MU_RT_S3MUA

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t const message[] =
    "Be that word our sign of parting, bird or fiend! I shrieked upstarting"
    "Get thee back into the tempest and the Nights Plutonian shore!"
    "Leave no black plume as a token of that lie thy soul hath spoken!"
    "Leave my loneliness unbroken! quit the bust above my door!"
    "Take thy beak from out my heart, and take thy form from off my door!"
    "Quoth the raven, Nevermore.  ";
/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{
    /*
     * This code example demonstrates EdgeLock usage in following steps:
     * 1.  load and authenticate EdgeLock Enclave FW (to be done in secure boot flow in real world app)
     * 2.  Compute hash one go and verify result with expected value
     * 3.  Compute hash streaming and verify result with expected value
     * 4.  Compute HMACs of messages and verify result with expected value
     */

    status_t result = kStatus_Fail;

    do
    {
        /* HW init */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

        PRINTF("EdgeLock Enclave Sub-System crypto example:\r\n\r\n");

        /****************** Load EdgeLock FW message ***************************/
        PRINTF("****************** Load EdgeLock FW ***********************\r\n");
        if (ELE_LoadFw(S3MU, ele_fw) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("EdgeLock FW loaded and authenticated successfully.\r\n\r\n");
        }

        /****************** Hash SHA256  ***************************************/
        PRINTF("****************** Compute Hash (SHA256) of message *******\r\n");
        /* Message to be hashed */

        /* Output for HASH */
        AT_NONCACHEABLE_SECTION_INIT(SDK_ALIGN(static uint8_t output[32], 8u));
        /* Expected SHA-256 for the message. */
        static const uint8_t sha256[] = {0x63, 0x76, 0xea, 0xcc, 0xc9, 0xa2, 0xc0, 0x43, 0xf4, 0xfb, 0x01,
                                         0x34, 0x69, 0xb3, 0x0c, 0xf5, 0x28, 0x63, 0x5c, 0xfa, 0xa5, 0x65,
                                         0x60, 0xef, 0x59, 0x7b, 0xd9, 0x1c, 0xac, 0xaa, 0x31, 0xf7};
        uint32_t length               = sizeof(message) - 1; /* no null string terminating */
        uint32_t outLength            = 0;

        /* Compute HASH ONE Go */
        if (ELE_Hash(S3MU, (const uint8_t *)message, length, output, sizeof(output), &outLength, kELE_Sha256) ==
            kStatus_Success)
        {
            /* Check output Hash digest data */
            if (memcmp(output, sha256, outLength) == 0)
            {
                PRINTF("*SUCCESS* Computed HASH matches the expected value.\r\n\r\n");
            }
            else
            {
                result = kStatus_Fail;
                break;
            }
        }
        else
        {
            result = kStatus_Fail;
            break;
        }

        /* Compute HASH Init */
        ele_hash_ctx_t hashCtx;

        if (ELE_Hash_Init(S3MU, &hashCtx, kELE_Sha256) == kStatus_Success)
        {
            PRINTF("*SUCCESS* HASH Init done.\r\n\r\n");
        }
        else
        {
            result = kStatus_Fail;
            break;
        }

        if (ELE_Hash_Update(S3MU, &hashCtx, kELE_Sha256, (const uint8_t *)message, length) == kStatus_Success)
        {
            PRINTF("*SUCCESS* HASH Update done.\r\n\r\n");
        }
        else
        {
            result = kStatus_Fail;
            break;
        }

        memset(output, 0u, sizeof(output));

        if (ELE_Hash_Finish(S3MU, &hashCtx, kELE_Sha256, output, sizeof(output), &outLength, NULL, 0u) ==
            kStatus_Success)
        {
            /* Check output Hash digest data */
            if (memcmp(output, sha256, outLength) == 0)
            {
                PRINTF("*SUCCESS* Computed HASH (Finish) matches the expected value.\r\n\r\n");
            }
            else
            {
                result = kStatus_Fail;
                break;
            }
        }
        else
        {
            result = kStatus_Fail;
            break;
        }

        /****************** HMAC ***********************************************/
        PRINTF("****************** Compute HMAC (SHA256) of massage *******\r\n");
        /* HMAC plaintext key */
        SDK_ALIGN(uint8_t MACkey0[32], 8u) = {0u};
        /* Message for HMAC */
        SDK_ALIGN(uint8_t MACmessage0[2], 8u) = {0xbe, 0xef};
        /* Message size */
        length = sizeof(MACmessage0);
        /* Output buffer */
        memset(output, 0u, sizeof(output));
        /* Expected HMAC for the message. */
        static const uint8_t hmac256_0[32u] = {0x54, 0x75, 0x4d, 0x18, 0xab, 0x19, 0xed, 0x1a, 0xb1, 0xc7, 0x99,
                                               0x25, 0xb7, 0xbc, 0x3e, 0x4f, 0x64, 0xf7, 0x0a, 0x0a, 0x9c, 0x49,
                                               0xff, 0xea, 0x7a, 0x4f, 0x90, 0xff, 0x94, 0x86, 0x47, 0xb9};

        if (ELE_GenericHmac(S3MU, (const uint8_t *)MACmessage0, length, output, &outLength, MACkey0, sizeof(MACkey0),
                            kELE_Hmac256) == kStatus_Success)
        {
            /* Check output Hash digest data */
            if (memcmp(output, hmac256_0, outLength) == 0)
            {
                PRINTF("*SUCCESS* Computed HMAC (#1) matches the expected value.\r\n\r\n");
            }
            else
            {
                result = kStatus_Fail;
                break;
            }
        }
        else
        {
            result = kStatus_Fail;
            break;
        }

        /* HMAC plaintext key */
        SDK_ALIGN(uint8_t MACkey1[32], 8u) = {0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61,
                                              0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61,
                                              0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61};

        /* Message for HMAC */
        SDK_ALIGN(uint8_t MACmessage1[50], 8u) = {
            0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
            0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
            0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64};
        /* Message size */
        length = sizeof(MACmessage1);
        /* Output buffer */
        memset(output, 0u, sizeof(output));
        /* Expected HMAC for the message. */
        static const uint8_t hmac256_1[32u] = {0xaf, 0xf9, 0x9f, 0xe6, 0xb6, 0x3d, 0x4b, 0xf4, 0x9b, 0x57, 0xf9,
                                               0x8a, 0x5b, 0xb8, 0x8d, 0xa6, 0x60, 0x95, 0xd0, 0x65, 0xc8, 0x94,
                                               0x6c, 0x7f, 0x33, 0xb8, 0xa6, 0xbb, 0xce, 0x31, 0x01, 0xe0};

        if (ELE_GenericHmac(S3MU, (const uint8_t *)MACmessage1, length, output, &outLength, MACkey1, sizeof(MACkey1),
                            kELE_Hmac256) == kStatus_Success)
        {
            /* Check output Hash digest data */
            if (memcmp(output, hmac256_1, outLength) == 0)
            {
                PRINTF("*SUCCESS* Computed HMAC (#2) matches the expected value.\r\n\r\n");
            }
            else
            {
                result = kStatus_Fail;
                break;
            }
        }
        else
        {
            result = kStatus_Fail;
            break;
        }

        /****************** ONESHOT Fast HMAC **********************************/
        PRINTF("****************** Compute ONESHOT Fast HMAC **************\r\n");
        SDK_ALIGN(uint8_t fastMACkeys[64], 8u) = {0u};
        uint16_t flags                         = 0u;
        uint32_t verificationStatus            = 0u;

        /* FastMAC always loads 64 Bytes of key data - in this example both
         * keys are used, so prepare 2 keys' worth of key data.
         */
        memcpy(fastMACkeys, MACkey0, 32u);
        memcpy(fastMACkeys + 32u, MACkey1, 32u);

        /* Start FastMAC */
        if (ELE_FastMacStart(S3MU, fastMACkeys) == kStatus_Success)
        {
            PRINTF("*SUCCESS* Fast HMAC Start done.\r\n\r\n");
        }
        else
        {
            result = kStatus_Fail;
            break;
        }

        /* First showing a oneshot operation with key 0 and no truncation.
         */
        flags = FAST_MAC_ONE_SHOT | FAST_MAC_USE_KEY_0;
        if (ELE_FastMacProceed(S3MU, (const uint8_t *)MACmessage0, output, sizeof(MACmessage0), flags, NULL) ==
            kStatus_Success)
        {
            /* Check output HMAC data */
            if (memcmp(output, hmac256_0, outLength) == 0)
            {
                PRINTF("*SUCCESS* Computed OneShot Fast HMAC matches the expected value.\r\n\r\n");
            }
            else
            {
                result = kStatus_Fail;
                break;
            }
        }
        else
        {
            result = kStatus_Fail;
            break;
        }

        /* A oneshot operation with key 0, no truncation,
         * and with internal verification.
         */
        flags = FAST_MAC_ONE_SHOT | FAST_MAC_USE_KEY_0 | FAST_MAC_VERIFY_INTERNALLY;
        if (ELE_FastMacProceed(S3MU, (const uint8_t *)MACmessage0, (uint8_t *)hmac256_0, sizeof(MACmessage0), flags,
                               &verificationStatus) == kStatus_Success)
        {
            if (FAST_MAC_CHECK_VERIFICATION_SUCCESS_ONESHOT(verificationStatus) == 1)
            {
                PRINTF("*SUCCESS* Internally verified OneShot Fast HMAC matches expected value.\r\n\r\n");
            }
        }
        else
        {
            result = kStatus_Fail;
            break;
        }

        /****************** PRELOADED Fast HMAC ********************************/
        PRINTF("****************** Compute PRELOADED Fast HMAC ************\r\n");
        /* Next to show preloading.
         * To utilize the preload feature, start with preloading a buffer and
         * setting the key to be used. Using buffer 0 and key 0.
         */
        flags = FAST_MAC_PRELOAD_BUFF_0 | FAST_MAC_USE_KEY_0;
        if (ELE_FastMacProceed(S3MU, (const uint8_t *)MACmessage0, output, sizeof(MACmessage0), flags, NULL) !=
            kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        PRINTF("*SUCCESS* Buffer 0 preload completed.\r\n\r\n");

        /* Now another buffer can preloaded and a different key can be used.
         * At the same time, the previously preloaded buffer can be processed.
         * Preloading buffer 1 with different input and setting key 1.
         * Proceed with HMAC generation from buffer 0.
         */
        flags = FAST_MAC_PRELOAD_BUFF_1 | FAST_MAC_USE_KEY_1 | FAST_MAC_PROCEED_BUFF_0;
        if (ELE_FastMacProceed(S3MU, (const uint8_t *)MACmessage1, output, sizeof(MACmessage1), flags, NULL) !=
            kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        if (memcmp(hmac256_0, output, sizeof(hmac256_0)) == 0)
        {
            PRINTF("*SUCCESS* Buffer 0 HMAC matches expected value AND buffer 1 preload completed.\r\n\r\n");
        }

        /* Proceed with HMAC generation from buffer 1. The input, output and
         * length parameters are no longer needed, as they were set
         * during preloading.
         */
        flags = FAST_MAC_PROCEED_BUFF_1;
        if (ELE_FastMacProceed(S3MU, NULL, NULL, 0u, flags, NULL) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        if (memcmp(hmac256_1, output, sizeof(hmac256_1)) == 0)
        {
            PRINTF("*SUCCESS* Buffer 1 HMAC matches expected value.\r\n\r\n");
        }

        /* Next, showing preloading with internal MAC verification and with
         * truncation to 8 Bytes.
         * For the internal verification process, when utilizing preloading,
         * the input needs to be set up by the caller - concatenate expected
         * (if needed, truncated) MAC to original message.
         */
        const size_t truncatedMACLength               = 8u;
        SDK_ALIGN(uint8_t MACmessage1WithMAC[58], 8u) = {0u};

        memcpy(MACmessage1WithMAC, MACmessage1, sizeof(MACmessage1));
        memcpy(MACmessage1WithMAC + sizeof(MACmessage1), hmac256_1, truncatedMACLength);

        flags = FAST_MAC_PRELOAD_BUFF_1 | FAST_MAC_USE_KEY_1 | FAST_MAC_TRUNCATE_08B | FAST_MAC_VERIFY_INTERNALLY;
        if (ELE_FastMacProceed(S3MU, (const uint8_t *)MACmessage1WithMAC, NULL, sizeof(MACmessage1WithMAC), flags,
                               NULL) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        PRINTF("*SUCCESS* Buffer 1 preload for internal verification completed.\r\n\r\n");

        /* Proceed to compute and verify HMAC and check verification status.
         */
        flags = FAST_MAC_PROCEED_BUFF_1;
        if (ELE_FastMacProceed(S3MU, NULL, NULL, 0u, flags, &verificationStatus) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            if (FAST_MAC_CHECK_VERIFICATION_SUCCESS_BUF_1(verificationStatus) == 1)
            {
                PRINTF("*SUCCESS* Buffer 1 internally verified HMAC matches expected value.\r\n\r\n");
            }
            else
            {
                result = kStatus_Fail;
                break;
            }
        }

        /* Exit FastMAC mode */
        if (ELE_FastMacEnd(S3MU) == kStatus_Success)
        {
            PRINTF("*SUCCESS* Fast HMAC End done.\r\n\r\n");
        }
        else
        {
            result = kStatus_Fail;
            break;
        }

        /****************** END of Example *************************************/

        result = kStatus_Success;

    } while (0);

    if (result == kStatus_Success)
    {
        PRINTF("End of Example with SUCCESS!!\r\n\r\n");
    }
    else
    {
        PRINTF("ERROR: execution of commands on Security Sub-System failed!\r\n\r\n");
    }
    while (1)
    {
    }
}
