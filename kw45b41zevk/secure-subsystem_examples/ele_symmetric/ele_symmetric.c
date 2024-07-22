/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_sss_mgmt.h"
#include "fsl_sss_sscp.h"
#include "fsl_sscp_mu.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CORE_CLK_FREQ CLOCK_GetFreq(kCLOCK_CoreSysClk)
#define ELE_MAX_SUBSYSTEM_WAIT (0xFFFFFFFFu)
#define ELE_SUBSYSTEM          (kType_SSS_Ele200)
#define KEY_ID                 (0u)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Variables used by example */
static sscp_context_t sscpContext    = {0};
static sss_sscp_session_t sssSession = {0};
static sss_sscp_key_store_t keyStore = {0};

/*******************************************************************************
 * Code
 ******************************************************************************/

status_t test_aes_cbc(void)
{
    /*!
     * @brief Plaintext for examples using the generic AES API.
     * 16-byte multiple, last '\0' is not used.
     */
    SDK_ALIGN(static uint8_t s_GenericAesPlain[], 8u) =
        "Be that word our sign of parting, bird or fiend! I shrieked upstarting"
        "Get thee back into the tempest and the Nights Plutonian shore!"
        "Leave no black plume as a token of that lie thy soul hath spoken!"
        "Leave my loneliness unbroken! quit the bust above my door!"
        "Take thy beak from out my heart, and take thy form from off my door!"
        "Quoth the raven, Nevermore.  ";

    /*! @brief 16 bytes key for CBC method: "ultrapassword123". */
    SDK_ALIGN(static uint8_t s_CbcKey128[], 8u) = {0x75, 0x6c, 0x74, 0x72, 0x61, 0x70, 0x61, 0x73,
                                                   0x73, 0x77, 0x6f, 0x72, 0x64, 0x31, 0x32, 0x33};

    /*! @brief Initialization vector for CBC method: 16 bytes: "mysecretpassword". */
    SDK_ALIGN(static uint8_t s_CbcIv[16u], 8u) = {0x6d, 0x79, 0x73, 0x65, 0x63, 0x72, 0x65, 0x74,
                                                  0x70, 0x61, 0x73, 0x73, 0x77, 0x6f, 0x72, 0x64};

    /*! @brief Expected ciphertext from CBC method using s_CbcKey128 key. */
    static uint8_t s_Cbc128CipherExpected[] = {
        0xeb, 0x69, 0xb5, 0xae, 0x7a, 0xbb, 0xb8, 0xee, 0x4d, 0xe5, 0x28, 0x97, 0xca, 0xab, 0x60, 0x65, 0x63, 0xf9,
        0xe8, 0x4c, 0x7f, 0xda, 0x0a, 0x02, 0x3a, 0x93, 0x16, 0x0d, 0x64, 0x56, 0x5a, 0x86, 0xf2, 0xe8, 0x5b, 0x38,
        0x1d, 0x31, 0xd7, 0x65, 0x7e, 0x8f, 0x8d, 0x53, 0xc5, 0xa6, 0x0c, 0x5d, 0xc5, 0x43, 0x98, 0x3b, 0x49, 0x3a,
        0xce, 0x7d, 0xf9, 0xb5, 0xf7, 0x95, 0x47, 0x89, 0xaf, 0xd8, 0x2f, 0xbd, 0xa4, 0xd8, 0x7f, 0xb9, 0x13, 0x3a,
        0xcd, 0x17, 0xc8, 0xc4, 0xb0, 0x5d, 0xe8, 0xf5, 0x19, 0x39, 0x6a, 0x14, 0x1b, 0x1b, 0x78, 0x5e, 0xe0, 0xd6,
        0x67, 0x9a, 0x36, 0x17, 0x9c, 0x7a, 0x88, 0x26, 0xfd, 0x8f, 0x3d, 0x82, 0xc9, 0xb1, 0x2a, 0x9c, 0xc0, 0xdd,
        0xdb, 0x78, 0x61, 0x3b, 0x22, 0x5d, 0x48, 0x3c, 0xab, 0x10, 0xd3, 0x5d, 0x0d, 0xa1, 0x25, 0x3e, 0x4d, 0xd6,
        0x8e, 0xc4, 0x1b, 0x68, 0xbb, 0xa4, 0x2d, 0x97, 0x2b, 0xd6, 0x23, 0xa0, 0xf2, 0x90, 0x8e, 0x07, 0x75, 0x44,
        0xb3, 0xe2, 0x5a, 0x35, 0x38, 0x4c, 0x5d, 0x35, 0xa9, 0x7c, 0xa3, 0xb6, 0x38, 0xe7, 0xf5, 0x20, 0xdc, 0x0e,
        0x6c, 0x7c, 0x4b, 0x4f, 0x93, 0xc1, 0x81, 0x69, 0x02, 0xb7, 0x66, 0x37, 0x24, 0x0d, 0xb8, 0x9a, 0xa8, 0xd4,
        0x42, 0x75, 0x28, 0xe8, 0x33, 0x89, 0x1e, 0x60, 0x82, 0xe9, 0xf6, 0x45, 0x72, 0x64, 0x65, 0xd2, 0xcd, 0x19,
        0xd9, 0x5e, 0xa2, 0x59, 0x31, 0x82, 0x53, 0x20, 0x35, 0x13, 0x76, 0x7f, 0xeb, 0xc3, 0xbe, 0xfa, 0x4a, 0x10,
        0x83, 0x81, 0x0f, 0x24, 0x6d, 0xca, 0x53, 0x07, 0xb9, 0xe0, 0xb9, 0x5d, 0x91, 0x2d, 0x90, 0x86, 0x5b, 0x9d,
        0xaa, 0xcd, 0x28, 0xea, 0x11, 0xfb, 0x83, 0x39, 0x9c, 0xf5, 0x3b, 0xd9, 0xef, 0x38, 0xc7, 0xa4, 0xad, 0x47,
        0xf2, 0x2d, 0xd6, 0x6b, 0x26, 0x28, 0x59, 0xaa, 0x33, 0x01, 0x73, 0xc9, 0x46, 0x97, 0xa3, 0xe5, 0x11, 0x71,
        0x66, 0xef, 0x1f, 0x0b, 0xbc, 0xe7, 0x4f, 0x8c, 0x79, 0xe2, 0x39, 0x14, 0x85, 0xcd, 0xa9, 0x59, 0xed, 0x78,
        0x9d, 0x37, 0xf5, 0x46, 0xfc, 0xa9, 0x8a, 0x16, 0x0a, 0x76, 0x58, 0x6d, 0x59, 0x9e, 0x65, 0xbe, 0x1b, 0xc2,
        0x09, 0xa1, 0xf9, 0x40, 0xab, 0xdb, 0x2e, 0x11, 0x30, 0x29, 0x49, 0x75, 0xf7, 0x74, 0xe1, 0xf3, 0x78, 0x97,
        0x69, 0x2c, 0x6a, 0x0e, 0x0d, 0xbd, 0x72, 0x3d, 0x75, 0xd6, 0x0a, 0x8c, 0xc2, 0x92, 0xd9, 0xb6, 0x46, 0x91,
        0xa7, 0xe4, 0x74, 0x71, 0xf5, 0xb4, 0x21, 0x86, 0x18, 0xa8};

    /*! @brief Encrypted ciphertext from CBC method goes here. */
    SDK_ALIGN(static uint8_t s_CbcCipher[sizeof(s_GenericAesPlain) - 1U], 8u);

    /*! @brief Decrypted plaintext from CBC method goes here. */
    SDK_ALIGN(static uint8_t s_CbcPlainDecrypted[sizeof(s_GenericAesPlain) - 1U], 8u);

    /****************** Crypto AES-CBC **************************/
    PRINTF("****************** Crypto AES-CBC ******************\r\n");

    status_t status = kStatus_Fail;

    sss_sscp_object_t sssKey = {0};
    sss_sscp_symmetric_t ctx = {0};

    do
    {
        /* Init key object  */
        status = sss_sscp_key_object_init(&sssKey, &keyStore);
        if (status != kStatus_SSS_Success)
        {
            break;
        }
        /* Allocate keystore handle */
        status = sss_sscp_key_object_allocate_handle(&sssKey, KEY_ID, /* key id */
                                                     kSSS_KeyPart_Default, kSSS_CipherType_AES, 16u,
                                                     kSSS_KeyProp_CryptoAlgo_AES);
        if (status != kStatus_SSS_Success)
        {
            (void)sss_sscp_key_object_free(&sssKey, kSSS_keyObjFree_KeysStoreDefragment);
            break;
        }

        /* Set key into key object*/
        status = sss_sscp_key_store_set_key(&keyStore, &sssKey, s_CbcKey128, sizeof(s_CbcKey128),
                                            (sizeof(s_CbcKey128) * 8U), kSSS_KeyPart_Default);
        if (status != kStatus_SSS_Success)
        {
            (void)sss_sscp_key_object_free(&sssKey, kSSS_keyObjFree_KeysStoreDefragment);
            break;
        }

        /* Init symmetric context */
        status = sss_sscp_symmetric_context_init(&ctx, &sssSession, &sssKey, kAlgorithm_SSS_AES_CBC, kMode_SSS_Encrypt);
        if (status != kStatus_SSS_Success)
        {
            (void)sss_sscp_key_object_free(&sssKey, kSSS_keyObjFree_KeysStoreDefragment);
            break;
        }

        /* RUN AES-CBC Encryption */
        status = sss_sscp_cipher_one_go(&ctx, s_CbcIv, sizeof(s_CbcIv), s_GenericAesPlain, s_CbcCipher,
                                        sizeof(s_GenericAesPlain));
        if (status != kStatus_SSS_Success)
        {
            (void)sss_sscp_symmetric_context_free(&ctx);
            (void)sss_sscp_key_object_free(&sssKey, kSSS_keyObjFree_KeysStoreDefragment);
            break;
        }

        if (status == kStatus_SSS_Success)
        {
            if (memcmp(s_CbcCipher, s_Cbc128CipherExpected, sizeof(s_CbcCipher)) == 0)
            {
                PRINTF("AES-CBC crypto encryption success and output matches expected result.\r\n\r\n");
                status = kStatus_Success;
            }
            else
            {
                status = kStatus_Fail;
            }
        }

        /* Close AES context*/
        status = sss_sscp_symmetric_context_free(&ctx);

        /* Init symmetric context */
        status = sss_sscp_symmetric_context_init(&ctx, &sssSession, &sssKey, kAlgorithm_SSS_AES_CBC, kMode_SSS_Decrypt);
        if (status != kStatus_SSS_Success)
        {
            (void)sss_sscp_key_object_free(&sssKey, kSSS_keyObjFree_KeysStoreDefragment);
            break;
        }

        /* RUN AES-CBC Decryption */
        status = sss_sscp_cipher_one_go(&ctx, s_CbcIv, sizeof(s_CbcIv), s_CbcCipher, s_CbcPlainDecrypted,
                                        sizeof(s_GenericAesPlain));
        if (status != kStatus_SSS_Success)
        {
            (void)sss_sscp_symmetric_context_free(&ctx);
            (void)sss_sscp_key_object_free(&sssKey, kSSS_keyObjFree_KeysStoreDefragment);
            break;
        }

        /* Cleanup Close all context, objects and sessions which were opened before */
        /* Close AES context*/
        status = sss_sscp_symmetric_context_free(&ctx);

        /* Free Key object */
        status = sss_sscp_key_object_free(&sssKey, kSSS_keyObjFree_KeysStoreDefragment);

    } while (0);

    if (status == kStatus_SSS_Success)
    {
        if (memcmp(s_CbcPlainDecrypted, s_GenericAesPlain, sizeof(s_CbcCipher)) == 0)
        {
            PRINTF("AES-CBC crypto decryption success and output matches expected result.\r\n\r\n");
            status = kStatus_Success;
        }
        else
        {
            status = kStatus_Fail;
        }
    }

    return status;
}

status_t test_aes_gcm(void)
{
    /* Test data for AES GCM*/

    /*! @brief 16 bytes key for GCM method. */
    SDK_ALIGN(static uint8_t s_GcmKey[16], 8u) = {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
                                                  0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08};

    /*! @brief Plaintext for GCM method. */
    SDK_ALIGN(static uint8_t s_GcmPlain[], 8u) = {
        0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5, 0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26,
        0x9a, 0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda, 0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31,
        0x8a, 0x72, 0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53, 0x2f, 0xcf, 0x0e, 0x24, 0x49,
        0xa6, 0xb5, 0x25, 0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57, 0xba, 0x63, 0x7b, 0x39};

    /*! @brief Expected ciphertext from GCM method. */
    static uint8_t s_GcmCipherExpected[] = {0x42, 0x83, 0x1e, 0xc2, 0x21, 0x77, 0x74, 0x24, 0x4b, 0x72, 0x21, 0xb7,
                                            0x84, 0xd0, 0xd4, 0x9c, 0xe3, 0xaa, 0x21, 0x2f, 0x2c, 0x02, 0xa4, 0xe0,
                                            0x35, 0xc1, 0x7e, 0x23, 0x29, 0xac, 0xa1, 0x2e, 0x21, 0xd5, 0x14, 0xb2,
                                            0x54, 0x66, 0x93, 0x1c, 0x7d, 0x8f, 0x6a, 0x5a, 0xac, 0x84, 0xaa, 0x05,
                                            0x1b, 0xa3, 0x0b, 0x39, 0x6a, 0x0a, 0xac, 0x97, 0x3d, 0x58, 0xe0, 0x91};

    /*! @brief Encrypted ciphertext from GCM method goes here. */
    SDK_ALIGN(static uint8_t s_GcmCipher[sizeof(s_GcmCipherExpected)], 8u);

    /*! @brief Initialization vector for GCM method. */
    SDK_ALIGN(static uint8_t s_GcmIv[12], 8u) = {0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce,
                                                 0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88};

    /*! @brief Additional authenticated data for GCM method. */
    SDK_ALIGN(static uint8_t s_GcmAad[], 8u) = {0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed,
                                                0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef, 0xab, 0xad, 0xda, 0xd2};

    /*! @brief Expected tag from GCM method. */
    static uint8_t s_GcmTagExpected[] = {0x5b, 0xc9, 0x4f, 0xbc, 0x32, 0x21, 0xa5, 0xdb,
                                         0x94, 0xfa, 0xe9, 0x5a, 0xe7, 0x12, 0x1a, 0x47};

    /*! @brief Encrypted tag from GCM method goes here. */
    SDK_ALIGN(static uint8_t s_GcmTag[sizeof(s_GcmTagExpected)], 8u);

    /****************** Crypto AES-GCM **************************/
    PRINTF("****************** Crypto AES-GCM ******************\r\n");

    status_t status = kStatus_Fail;

    sss_sscp_object_t sssKey = {0};
    sss_sscp_aead_t ctx      = {0};

    size_t taglen = sizeof(s_GcmTag);

    do
    {
        /* Init key object  */
        status = sss_sscp_key_object_init(&sssKey, &keyStore);
        if (status != kStatus_SSS_Success)
        {
            break;
        }
        /* Allocate keystore handle */
        status = sss_sscp_key_object_allocate_handle(&sssKey, KEY_ID, /* key id */
                                                     kSSS_KeyPart_Default, kSSS_CipherType_AES, 16u,
                                                     kSSS_KeyProp_CryptoAlgo_AEAD);
        if (status != kStatus_SSS_Success)
        {
            (void)sss_sscp_key_object_free(&sssKey, kSSS_keyObjFree_KeysStoreDefragment);
            break;
        }

        /* Set key into key object*/
        status = sss_sscp_key_store_set_key(&keyStore, &sssKey, s_GcmKey, sizeof(s_GcmKey), (sizeof(s_GcmKey) * 8U),
                                            kSSS_KeyPart_Default);
        if (status != kStatus_SSS_Success)
        {
            (void)sss_sscp_key_object_free(&sssKey, kSSS_keyObjFree_KeysStoreDefragment);
            break;
        }

        /* Init aead context */
        status = sss_sscp_aead_context_init(&ctx, &sssSession, &sssKey, kAlgorithm_SSS_AES_GCM, kMode_SSS_Encrypt);
        if (status != kStatus_SSS_Success)
        {
            (void)sss_sscp_key_object_free(&sssKey, kSSS_keyObjFree_KeysStoreDefragment);
            break;
        }

        /* RUN AES-GCM Encryption */
        status = sss_sscp_aead_one_go(&ctx, s_GcmPlain, s_GcmCipher, sizeof(s_GcmPlain), s_GcmIv, sizeof(s_GcmIv),
                                      s_GcmAad, sizeof(s_GcmAad), s_GcmTag, &taglen);
        if (status != kStatus_SSS_Success)
        {
            (void)sss_sscp_aead_context_free(&ctx);
            (void)sss_sscp_key_object_free(&sssKey, kSSS_keyObjFree_KeysStoreDefragment);
            break;
        }

        /* Close AES AEAD context*/
        status = sss_sscp_aead_context_free(&ctx);

        /* Free Key object */
        status = sss_sscp_key_object_free(&sssKey, kSSS_keyObjFree_KeysStoreDefragment);

        if (status == kStatus_SSS_Success)
        {
            if (memcmp(s_GcmCipher, s_GcmCipherExpected, sizeof(s_GcmCipher)) == 0 &&
                memcmp(s_GcmTag, s_GcmTagExpected, sizeof(s_GcmTag)) == 0)
            {
                PRINTF("AES-GCM crypto encryption success and output matches expected result.\r\n\r\n");
                status = kStatus_Success;
            }
            else
            {
                status = kStatus_Fail;
            }
        }

    } while (0);

    return status;
}

/*!
 * @brief Main function
 */

int main(void)
{
    char ch;
    status_t status = kStatus_Fail;

    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("ELE Symmetric via SSSAPI Example\r\n\r\n");

    /*
     * This code example demonstrates EdgeLock usage for AES CBC and AES GCM operation via SSSAPI.
     * The example is performed in following steps:
     * 1. Open EdgeLock session
     * 2. Create key store
     * 3. Create and allocate key object
     * 4. Initialize AES CBC encrypt operation context
     * 5. Perform AES CBC encryption operation
     * 6. Initialize AES CBC decrypt operation context
     * 7. Perform AES CBC decrypt operation
     * 8. Initialize AES GCM encrypt operation context
     * 9. Perform AES GCM encryption operation
     * 10. Close all opened contexts and created objects
     * Note: This example does not close already opened contexts or objects in case of failed command.
     */

    do
    {
        status = ELEMU_mu_wait_for_ready(ELEMUA, ELE_MAX_SUBSYSTEM_WAIT);
        if (status != kStatus_Success)
        {
            break;
        }

        /****************** Start   ***********************/
        status = sscp_mu_init(&sscpContext, (ELEMU_Type *)(uintptr_t)ELEMUA);
        if (status != kStatus_SSCP_Success)
        {
            break;
        }
        /* open session to specific security subsystem */
        status = sss_sscp_open_session(&sssSession, 0u, ELE_SUBSYSTEM, &sscpContext);
        if (status != kStatus_SSS_Success)
        {
            return status;
        }

        /* Init keystore  */
        status = sss_sscp_key_store_init(&keyStore, &sssSession);
        if (status != kStatus_SSS_Success)
        {
            break;
        }

        status = test_aes_cbc();
        if (status != kStatus_Success)
        {
            break;
        }

        status = test_aes_gcm();
        if (status != kStatus_Success)
        {
            break;
        }

    } while (0);

    if (status == kStatus_Success)
    {
        PRINTF("End of Example with SUCCESS!!\r\n\r\n");
    }
    else
    {
        PRINTF("ERROR: execution of commands on Security Sub-System failed!\r\n\r\n");
    }

    /* Close keystore*/
    status = sss_sscp_key_store_free(&keyStore);
    /* Close session */
    status = sss_sscp_close_session(&sssSession);

    PRINTF("Example end\r\n");

    while (1)
    {
        ch = GETCHAR();
        PUTCHAR(ch);
    }
}
