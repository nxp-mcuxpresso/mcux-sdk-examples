/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
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

#include "fsl_hashcrypt.h"

#include <string.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define TEST_ASSERT(a)       \
    if (!(a))                \
    {                        \
        PRINTF("error\r\n"); \
        do                   \
        {                    \
        } while (1);         \
    }

#define CRYPTO_TEST_OUT_ARRAY_LEN 0x100
#define CRYPTO_SHA1_OUT_LEN       20
#define CRYPTO_SHA256_OUT_LEN     32

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

void TestAesEcb(void)
{
    static const uint8_t keyAes128[] __attribute__((aligned)) = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                                                                 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    static const uint8_t plainAes128[]                        = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
                                          0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a};
    static const uint8_t cipherAes128[]                       = {0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60,
                                           0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97};
    uint8_t cipher[16];
    uint8_t output[16];
    status_t status;

    hashcrypt_handle_t m_handle;

    m_handle.keyType = kHASHCRYPT_UserKey;

    status = HASHCRYPT_AES_SetKey(HASHCRYPT, &m_handle, keyAes128, 16);

    TEST_ASSERT(kStatus_Success == status);

    status = HASHCRYPT_AES_EncryptEcb(HASHCRYPT, &m_handle, plainAes128, cipher, 16);
    TEST_ASSERT(kStatus_Success == status);
    TEST_ASSERT(memcmp(cipher, cipherAes128, 16) == 0);

    status = HASHCRYPT_AES_DecryptEcb(HASHCRYPT, &m_handle, cipher, output, 16);
    TEST_ASSERT(kStatus_Success == status);
    TEST_ASSERT(memcmp(output, plainAes128, 16) == 0);

    PRINTF("AES ECB Test pass\r\n");
}

void TestAesCbc(void)
{
    static const uint8_t keyAes128[] __attribute__((aligned)) = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                                                                 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    static const uint8_t plainAes128[]                        = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
                                          0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a};
    static const uint8_t ive[]                                = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

    static const uint8_t cipherAes128[] = {0x76, 0x49, 0xab, 0xac, 0x81, 0x19, 0xb2, 0x46,
                                           0xce, 0xe9, 0x8e, 0x9b, 0x12, 0xe9, 0x19, 0x7d};

    uint8_t cipher[16];
    uint8_t output[16];
    status_t status;

    hashcrypt_handle_t m_handle;

    m_handle.keySize = kHASHCRYPT_Aes128;
    m_handle.keyType = kHASHCRYPT_UserKey;

    status = HASHCRYPT_AES_SetKey(HASHCRYPT, &m_handle, keyAes128, 16);
    TEST_ASSERT(kStatus_Success == status);

    status = HASHCRYPT_AES_EncryptCbc(HASHCRYPT, &m_handle, plainAes128, cipher, 16, ive);
    TEST_ASSERT(kStatus_Success == status);
    TEST_ASSERT(memcmp(cipher, cipherAes128, 16) == 0);

    status = HASHCRYPT_AES_DecryptCbc(HASHCRYPT, &m_handle, cipher, output, 16, ive);
    TEST_ASSERT(kStatus_Success == status);
    TEST_ASSERT(memcmp(output, plainAes128, 16) == 0);

    PRINTF("AES CBC Test pass\r\n");
}

void TestAesCtr(void)
{
    static const uint8_t aes_ctr_test01_key[] __attribute__((aligned)) = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    static uint8_t aes_ctr_test01_counter_1[16]      = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
                                                   0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};
    static uint8_t aes_ctr_test01_counter_2[16]      = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
                                                   0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};
    static uint8_t aes_ctr_test01_plaintext[]        = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
                                                 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a};
    static const uint8_t aes_ctr_test01_ciphertext[] = {0x87, 0x4d, 0x61, 0x91, 0xb6, 0x20, 0xe3, 0x26,
                                                        0x1b, 0xef, 0x68, 0x64, 0x99, 0x0d, 0xb6, 0xce};
    uint8_t cipher[16]                               = {0};
    uint8_t output[16]                               = {0};
    status_t status;

    hashcrypt_handle_t m_handle;

    m_handle.keySize = kHASHCRYPT_Aes128;
    m_handle.keyType = kHASHCRYPT_UserKey;

    status = HASHCRYPT_AES_SetKey(HASHCRYPT, &m_handle, aes_ctr_test01_key, 16);
    TEST_ASSERT(kStatus_Success == status);

    /* Encrypt */
    status = HASHCRYPT_AES_CryptCtr(HASHCRYPT, &m_handle, aes_ctr_test01_plaintext, cipher, 16,
                                    aes_ctr_test01_counter_1, NULL, NULL);
    TEST_ASSERT(kStatus_Success == status);
    TEST_ASSERT(memcmp(cipher, aes_ctr_test01_ciphertext, 16) == 0);

    /* Decrypt */
    status = HASHCRYPT_AES_CryptCtr(HASHCRYPT, &m_handle, cipher, output, 16, aes_ctr_test01_counter_2, NULL, NULL);
    TEST_ASSERT(kStatus_Success == status);
    TEST_ASSERT(memcmp(output, aes_ctr_test01_plaintext, sizeof(aes_ctr_test01_plaintext)) == 0);

    PRINTF("AES CTR Test pass\r\n");
}

void TestAesOfb(void)
{
    static const uint8_t aes_test_ofb_iv[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                                0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    static const uint8_t aes_test_ofb_key[] = {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
                                               0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C};

    static const uint8_t aes_ofb_test01_plaintext[] = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
                                                       0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a};

    static const uint8_t aes_ofb_test01_ciphertext[] = {0x3b, 0x3f, 0xd9, 0x2e, 0xb7, 0x2d, 0xad, 0x20,
                                                        0x33, 0x34, 0x49, 0xf8, 0xe8, 0x3c, 0xfb, 0x4a};

    /* Long Data with non-block multiple size */
    static const uint8_t aes_ofb_test01_plaintext_long[] = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d,
                                                            0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a, 0xae, 0x2d, 0x8a, 0x57,
                                                            0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf,
                                                            0x8e, 0x51, 0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11};

    static const uint8_t aes_ofb_test01_ciphertext_long[] = {
        0x3b, 0x3f, 0xd9, 0x2e, 0xb7, 0x2d, 0xad, 0x20, 0x33, 0x34, 0x49, 0xf8, 0xe8, 0x3c,
        0xfb, 0x4a, 0x77, 0x89, 0x50, 0x8d, 0x16, 0x91, 0x8f, 0x03, 0xf5, 0x3c, 0x52, 0xda,
        0xc5, 0x4e, 0xd8, 0x25, 0x97, 0x40, 0x05, 0x1e, 0x9c, 0x5f, 0xec, 0xf6};

    uint8_t cipher[16] = {0};
    uint8_t output[16] = {0};

    uint8_t cipher_long[40] = {0};
    uint8_t output_long[40] = {0};
    size_t length_long      = sizeof(aes_ofb_test01_plaintext_long);
    status_t status;

    hashcrypt_handle_t m_handle;

    m_handle.keySize = kHASHCRYPT_Aes128;
    m_handle.keyType = kHASHCRYPT_UserKey;

    status = HASHCRYPT_AES_SetKey(HASHCRYPT, &m_handle, aes_test_ofb_key, 16);
    TEST_ASSERT(kStatus_Success == status);

    /* Encrypt */
    status = HASHCRYPT_AES_CryptOfb(HASHCRYPT, &m_handle, aes_ofb_test01_plaintext, cipher, 16, aes_test_ofb_iv);
    TEST_ASSERT(kStatus_Success == status);
    TEST_ASSERT(memcmp(cipher, aes_ofb_test01_ciphertext, 16) == 0);

    /* Data with non-block multiple size */
    status = HASHCRYPT_AES_CryptOfb(HASHCRYPT, &m_handle, aes_ofb_test01_plaintext_long, cipher_long, length_long,
                                    aes_test_ofb_iv);
    TEST_ASSERT(kStatus_Success == status);
    TEST_ASSERT(memcmp(cipher_long, aes_ofb_test01_ciphertext_long, length_long) == 0);

    /* Decrypt */
    status = HASHCRYPT_AES_CryptOfb(HASHCRYPT, &m_handle, aes_ofb_test01_ciphertext, output, 16, aes_test_ofb_iv);
    TEST_ASSERT(kStatus_Success == status);
    TEST_ASSERT(memcmp(output, aes_ofb_test01_plaintext, sizeof(aes_ofb_test01_ciphertext)) == 0);

    /* Data with non-block multiple size */
    status = HASHCRYPT_AES_CryptOfb(HASHCRYPT, &m_handle, aes_ofb_test01_ciphertext_long, output_long, length_long,
                                    aes_test_ofb_iv);
    TEST_ASSERT(kStatus_Success == status);
    TEST_ASSERT(memcmp(output_long, aes_ofb_test01_plaintext_long, length_long) == 0);

    PRINTF("AES OFB Test pass\r\n");
}

void TestAesCfb(void)
{
    static const uint8_t aes_test_cfb_iv[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                                0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    static const uint8_t aes_test_cfb_key[] = {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
                                               0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C};

    static const uint8_t aes_cfb_test01_plaintext[] = {
        0x6B, 0xC1, 0xBE, 0xE2, 0x2E, 0x40, 0x9F, 0x96, 0xE9, 0x3D, 0x7E, 0x11, 0x73, 0x93, 0x17, 0x2A,
        0xAE, 0x2D, 0x8A, 0x57, 0x1E, 0x03, 0xAC, 0x9C, 0x9E, 0xB7, 0x6F, 0xAC, 0x45, 0xAF, 0x8E, 0x51,
        0x30, 0xC8, 0x1C, 0x46, 0xA3, 0x5C, 0xE4, 0x11, 0xE5, 0xFB, 0xC1, 0x19, 0x1A, 0x0A, 0x52, 0xEF,
        0xF6, 0x9F, 0x24, 0x45, 0xDF, 0x4F, 0x9B, 0x17, 0xAD, 0x2B, 0x41, 0x7B, 0xE6, 0x6C, 0x37, 0x10};

    static const uint8_t aes_cfb_test01_ciphertext[] = {
        0x3B, 0x3F, 0xD9, 0x2E, 0xB7, 0x2D, 0xAD, 0x20, 0x33, 0x34, 0x49, 0xF8, 0xE8, 0x3C, 0xFB, 0x4A,
        0xC8, 0xA6, 0x45, 0x37, 0xA0, 0xB3, 0xA9, 0x3F, 0xCD, 0xE3, 0xCD, 0xAD, 0x9F, 0x1C, 0xE5, 0x8B,
        0x26, 0x75, 0x1F, 0x67, 0xA3, 0xCB, 0xB1, 0x40, 0xB1, 0x80, 0x8C, 0xF1, 0x87, 0xA4, 0xF4, 0xDF,
        0xC0, 0x4B, 0x05, 0x35, 0x7C, 0x5D, 0x1C, 0x0E, 0xEA, 0xC4, 0xC6, 0x6F, 0x9F, 0xF7, 0xF2, 0xE6};

    uint8_t cipher[64] = {0};
    uint8_t output[64] = {0};

    status_t status;

    hashcrypt_handle_t m_handle;

    m_handle.keySize = kHASHCRYPT_Aes128;
    m_handle.keyType = kHASHCRYPT_UserKey;

    status = HASHCRYPT_AES_SetKey(HASHCRYPT, &m_handle, aes_test_cfb_key, 16);
    TEST_ASSERT(kStatus_Success == status);

    /* Encrypt */
    status = HASHCRYPT_AES_EncryptCfb(HASHCRYPT, &m_handle, aes_cfb_test01_plaintext, cipher, 64, aes_test_cfb_iv);
    TEST_ASSERT(kStatus_Success == status);
    TEST_ASSERT(memcmp(cipher, aes_cfb_test01_ciphertext, 64) == 0);

    /* Decrypt */
    status = HASHCRYPT_AES_DecryptCfb(HASHCRYPT, &m_handle, aes_cfb_test01_ciphertext, output, 64, aes_test_cfb_iv);
    TEST_ASSERT(kStatus_Success == status);
    TEST_ASSERT(memcmp(output, aes_cfb_test01_plaintext, sizeof(aes_cfb_test01_ciphertext)) == 0);

    PRINTF("AES CFB Test pass\r\n");
}

void TestSha1(void)
{
    status_t status;
    size_t outLength;
    unsigned int length;
    unsigned char output[20];

    static const uint8_t message[] = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";

    /* Expected SHA-1 for the message. */
    static const unsigned char sha1[] = {0x84, 0x98, 0x3e, 0x44, 0x1c, 0x3b, 0xd2, 0x6e, 0xba, 0xae,
                                         0x4a, 0xa1, 0xf9, 0x51, 0x29, 0xe5, 0xe5, 0x46, 0x70, 0xf1};

    length    = sizeof(message) - 1;
    outLength = sizeof(output);
    memset(&output, 0, outLength);

    /************************ SHA-1 **************************/
    status = HASHCRYPT_SHA(HASHCRYPT, kHASHCRYPT_Sha1, message, length, output, &outLength);
    TEST_ASSERT(kStatus_Success == status);
    TEST_ASSERT(outLength == 20u);
    TEST_ASSERT(memcmp(output, sha1, outLength) == 0);

    PRINTF("SHA-1 Test pass\r\n");
}

void TestSha256(void)
{
    status_t status;
    size_t outLength;
    unsigned int length;
    unsigned char output[32];

    static const uint8_t message[] =
        "Be that word our sign of parting, bird or fiend! I shrieked upstarting"
        "Get thee back into the tempest and the Nights Plutonian shore!"
        "Leave no black plume as a token of that lie thy soul hath spoken!"
        "Leave my loneliness unbroken! quit the bust above my door!"
        "Take thy beak from out my heart, and take thy form from off my door!"
        "Quoth the raven, Nevermore.  ";

    /* Expected SHA-256 for the message. */
    static const unsigned char sha256[] = {0x63, 0x76, 0xea, 0xcc, 0xc9, 0xa2, 0xc0, 0x43, 0xf4, 0xfb, 0x01,
                                           0x34, 0x69, 0xb3, 0x0c, 0xf5, 0x28, 0x63, 0x5c, 0xfa, 0xa5, 0x65,
                                           0x60, 0xef, 0x59, 0x7b, 0xd9, 0x1c, 0xac, 0xaa, 0x31, 0xf7};

    length    = sizeof(message) - 1;
    outLength = sizeof(output);
    memset(&output, 0, outLength);

    /************************ SHA-256 **************************/
    status = HASHCRYPT_SHA(HASHCRYPT, kHASHCRYPT_Sha256, message, length, output, &outLength);
    TEST_ASSERT(kStatus_Success == status);
    TEST_ASSERT(outLength == 32u);
    TEST_ASSERT(memcmp(output, sha256, outLength) == 0);

    PRINTF("SHA-256 Test pass\r\n");
}

void TestReloadHashcryptFeature(void)
{
#if defined(FSL_FEATURE_HASHCRYPT_HAS_RELOAD_FEATURE) && (FSL_FEATURE_HASHCRYPT_HAS_RELOAD_FEATURE > 0)
    /* Hashcrypt without context switch is not able to calculate SHA in parallel with AES. */

    static const uint8_t keyAes128[] __attribute__((aligned)) = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                                                                 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    static const uint8_t plainAes128[]                        = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
                                          0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a};
    static const uint8_t cipherAes128[]                       = {0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60,
                                           0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97};

    static const uint8_t message1[] =
        "Once upon a midnight dreary, while I pondered weak and weary,"
        "Over many a quaint and curious volume of forgotten lore,"
        "While I nodded, nearly napping, suddenly there came a tapping,"
        "As of some one gently rapping, rapping at my chamber door"
        "Tis some visitor, I muttered, tapping at my chamber door"
        "Only this, and nothing more.";

    /* Expected SHA-256 for the message1. */
    static const unsigned char sha1[] = {0xf7, 0x2f, 0xa9, 0xc3, 0x2a, 0x9d, 0xc2, 0x3a, 0x23, 0x23, 0x78,
                                         0xbd, 0xb3, 0xf2, 0x59, 0x86, 0x42, 0xc0, 0xca, 0xff, 0x17, 0x6d,
                                         0x9c, 0x2c, 0x1a, 0xcd, 0xcc, 0xb9, 0xe1, 0x06, 0x14, 0x1e};

    static const uint8_t message2[] =
        "Be that word our sign of parting, bird or fiend! I shrieked upstarting"
        "Get thee back into the tempest and the Nights Plutonian shore!"
        "Leave no black plume as a token of that lie thy soul hath spoken!"
        "Leave my loneliness unbroken! quit the bust above my door!"
        "Take thy beak from out my heart, and take thy form from off my door!"
        "Quoth the raven, Nevermore.  ";

    /* Expected SHA-256 for the message2. */
    static const unsigned char sha2[] = {0x63, 0x76, 0xea, 0xcc, 0xc9, 0xa2, 0xc0, 0x43, 0xf4, 0xfb, 0x01,
                                         0x34, 0x69, 0xb3, 0x0c, 0xf5, 0x28, 0x63, 0x5c, 0xfa, 0xa5, 0x65,
                                         0x60, 0xef, 0x59, 0x7b, 0xd9, 0x1c, 0xac, 0xaa, 0x31, 0xf7};
    uint8_t cipher[16]; /*  For AES ECB */
    uint8_t output[16]; /*  For AES ECB */
    status_t status;

    size_t outLength1, outLength2;
    unsigned int length1, length2;
    unsigned char outputSHA1[32], outputSHA2[32];

    hashcrypt_hash_ctx_t hashCtx1;
    hashcrypt_hash_ctx_t hashCtx2;

    length1    = sizeof(message1) - 1;
    outLength1 = sizeof(outputSHA1);
    memset(&outputSHA1, 0, outLength1);

    length2    = sizeof(message2) - 1;
    outLength2 = sizeof(outputSHA2);
    memset(&outputSHA2, 0, outLength2);

    hashcrypt_handle_t m_handle;

    m_handle.keyType = kHASHCRYPT_UserKey;

    status = HASHCRYPT_SHA_Init(HASHCRYPT, &hashCtx1, kHASHCRYPT_Sha256);
    TEST_ASSERT(kStatus_Success == status);

    status = HASHCRYPT_SHA_Init(HASHCRYPT, &hashCtx2, kHASHCRYPT_Sha256);
    TEST_ASSERT(kStatus_Success == status);

    status = HASHCRYPT_AES_SetKey(HASHCRYPT, &m_handle, keyAes128, 16);
    TEST_ASSERT(kStatus_Success == status);

    status = HASHCRYPT_AES_EncryptEcb(HASHCRYPT, &m_handle, plainAes128, cipher, 16);
    TEST_ASSERT(kStatus_Success == status);
    TEST_ASSERT(memcmp(cipher, cipherAes128, 16) == 0);

    status = HASHCRYPT_SHA_Update(HASHCRYPT, &hashCtx1, message1, length1);
    TEST_ASSERT(kStatus_Success == status);

    status = HASHCRYPT_SHA_Update(HASHCRYPT, &hashCtx2, message2, length2);
    TEST_ASSERT(kStatus_Success == status);

    status = HASHCRYPT_AES_DecryptEcb(HASHCRYPT, &m_handle, cipher, output, 16);
    TEST_ASSERT(kStatus_Success == status);
    TEST_ASSERT(memcmp(output, plainAes128, 16) == 0);

    status = HASHCRYPT_SHA_Finish(HASHCRYPT, &hashCtx1, outputSHA1, &outLength1);
    TEST_ASSERT(kStatus_Success == status);
    TEST_ASSERT(outLength1 == 32u);
    TEST_ASSERT(memcmp(outputSHA1, sha1, outLength1) == 0);

    status = HASHCRYPT_SHA_Finish(HASHCRYPT, &hashCtx2, outputSHA2, &outLength2);
    TEST_ASSERT(kStatus_Success == status);
    TEST_ASSERT(outLength2 == 32u);
    TEST_ASSERT(memcmp(outputSHA2, sha2, outLength2) == 0);

    PRINTF("Hashcrypt reload feature pass\r\n");

#endif
}

/*!
 * @brief Main function.
 */
int main(void)
{
    /* Init hardware */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Initialize Hashcrypt */
    HASHCRYPT_Init(HASHCRYPT);

    /* Call HASH APIs */
    TestAesEcb();
    TestAesCbc();
    TestAesCtr();
    TestAesOfb();
    TestAesCfb();
    TestSha1();
    TestSha256();

    TestReloadHashcryptFeature();

    HASHCRYPT_Deinit(HASHCRYPT);

    while (1)
    {
    }
}
