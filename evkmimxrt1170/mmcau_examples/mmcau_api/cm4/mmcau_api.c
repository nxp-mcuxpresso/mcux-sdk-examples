/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
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

#include "fsl_mmcau.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define CORE_CLK_FREQ CLOCK_GetFreqFromObs(CCM_OBS_M4_CLK_ROOT)
/* Number of cycles for througput measurement. One data buffer of certain size if processed this times. */
#define CYCLES_FOR_THROUGHPUT 128
#define CYCLES_FOR_PASSRATE   1024

/*AES specific*/
#define AES128           128
#define AES128_KEY_SIZE  16
#define AES_BLOCK_LENGTH 16

#define AES192          192
#define AES192_KEY_SIZE 24

#define AES256          256
#define AES256_KEY_SIZE 32

/*DES specific*/
#define DES3_BLOCK_LENGTH 8
#define DES3_KEY_LENGTH   24

/*length of AES & DES encrypted data array*/
#define OUTPUT_ARRAY_LEN 512
/*length of result hash in bytes*/
#define SHA1_RESULT_LENGTH   20
#define SHA256_RESULT_LENGTH 32
#define MD5_RESULT_LENGTH    16

/*size of one crypto block*/
#define CRYPTO_BLOCK_LENGTH 64
/*size of padded hash input string*/
#define TEST_LENGTH 64

/*MMCAU result codes*/
#define MMCAU_OK    0
#define MMCAU_ERROR -1

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*16 bytes key: "ultrapassword123"*/
const uint8_t g_aesKey128[AES128_KEY_SIZE] = {0x75, 0x6c, 0x74, 0x72, 0x61, 0x70, 0x61, 0x73,
                                              0x73, 0x77, 0x6f, 0x72, 0x64, 0x31, 0x32, 0x33};
/*16 bytes key: "ultrapassword123ultrapas"*/
const uint8_t g_aesKey192[AES192_KEY_SIZE] = {0x75, 0x6c, 0x74, 0x72, 0x61, 0x70, 0x61, 0x73, 0x73, 0x77, 0x6f, 0x72,
                                              0x64, 0x31, 0x32, 0x33, 0x75, 0x6c, 0x74, 0x72, 0x61, 0x70, 0x61, 0x73};

/*32 bytes key: "ultrapassword123ultrapassword123"*/
const uint8_t g_aesKey256[AES256_KEY_SIZE] = {0x75, 0x6c, 0x74, 0x72, 0x61, 0x70, 0x61, 0x73, 0x73, 0x77, 0x6f,
                                              0x72, 0x64, 0x31, 0x32, 0x33, 0x75, 0x6c, 0x74, 0x72, 0x61, 0x70,
                                              0x61, 0x73, 0x73, 0x77, 0x6f, 0x72, 0x64, 0x31, 0x32, 0x33};

/*initialization vector: 16 bytes: "mysecretpassword"*/
const uint8_t g_aesIV[AES_BLOCK_LENGTH] = {0x6d, 0x79, 0x73, 0x65, 0x63, 0x72, 0x65, 0x74,
                                           0x70, 0x61, 0x73, 0x73, 0x77, 0x6f, 0x72, 0x64};

/*24 bytes key: "verynicepassword12345678"
  Note: parity is fixed inside DES crypto function*/
static uint8_t g_des3Key[DES3_KEY_LENGTH] = {0x76, 0x65, 0x72, 0x79, 0x6e, 0x69, 0x63, 0x65, 0x70, 0x61, 0x73, 0x73,
                                             0x77, 0x6f, 0x72, 0x64, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38};

/*initialization vector: 8 bytes: "mysecret"*/
static const uint8_t g_des3IV[DES3_BLOCK_LENGTH] = {0x6d, 0x79, 0x73, 0x65, 0x63, 0x72, 0x65, 0x74};

/* SHA test string: "The quick brown fox jumps over the lazy dog"
 * with padding bits included
 */
static uint8_t g_testSha[TEST_LENGTH] = {0x54, 0x68, 0x65, 0x20, 0x71, 0x75, 0x69, 0x63, 0x6b, 0x20, 0x62, 0x72, 0x6f,
                                         0x77, 0x6e, 0x20, 0x66, 0x6f, 0x78, 0x20, 0x6a, 0x75, 0x6d, 0x70, 0x73, 0x20,
                                         0x6f, 0x76, 0x65, 0x72, 0x20, 0x74, 0x68, 0x65, 0x20, 0x6c, 0x61, 0x7a, 0x79,
                                         0x20, 0x64, 0x6f, 0x67, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x58};

static const uint8_t g_parityBits[128] = {1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0,
                                          0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1,
                                          0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1,
                                          1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
                                          0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

/* MD5 test string: "The quick brown fox jumps over the lazy dog"
 * with padding bits included
 */
static uint8_t g_testMd5[TEST_LENGTH] = {0x54, 0x68, 0x65, 0x20, 0x71, 0x75, 0x69, 0x63, 0x6b, 0x20, 0x62, 0x72, 0x6f,
                                         0x77, 0x6e, 0x20, 0x66, 0x6f, 0x78, 0x20, 0x6a, 0x75, 0x6d, 0x70, 0x73, 0x20,
                                         0x6f, 0x76, 0x65, 0x72, 0x20, 0x74, 0x68, 0x65, 0x20, 0x6c, 0x61, 0x7a, 0x79,
                                         0x20, 0x64, 0x6f, 0x67, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x58, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/*8-byte multiple*/
static const uint8_t g_testFull[] =
    "Once upon a midnight dreary, while I pondered weak and weary,"
    "Over many a quaint and curious volume of forgotten lore,"
    "While I nodded, nearly napping, suddenly there came a tapping,"
    "As of some one gently rapping, rapping at my chamber door"
    "Its some visitor, I muttered, tapping at my chamber door"
    "Only this, and nothing more.";

#if defined(DEMO_MMCAU_PASS_RATE)
/* AES-128 expected cipher
 */
static const uint8_t g_aes128_cipher[] = {
    0x80, 0x0f, 0x23, 0x95, 0xfd, 0x33, 0x35, 0x61, 0x53, 0x6e, 0x2c, 0x70, 0x0e, 0x5a, 0x86, 0xef, 0x24, 0x60, 0x5f,
    0x62, 0xdd, 0x8e, 0xcf, 0x84, 0x61, 0x5b, 0x5c, 0x53, 0xfa, 0x28, 0xfe, 0x7e, 0x81, 0x2d, 0xe7, 0x85, 0x6d, 0xfb,
    0x52, 0xa6, 0xae, 0xec, 0x75, 0x16, 0xdb, 0xf7, 0xc4, 0x36, 0xf6, 0xad, 0x3e, 0x01, 0xcd, 0x6d, 0x89, 0xe2, 0x79,
    0x48, 0x6b, 0xb2, 0x89, 0x12, 0xba, 0xa9, 0x07, 0x34, 0x3b, 0xe0, 0x4f, 0xd5, 0x18, 0x8e, 0x91, 0xb4, 0x22, 0x63,
    0x20, 0xac, 0xe5, 0x22, 0xe0, 0xbf, 0x3a, 0xaa, 0x7f, 0x49, 0xe1, 0x11, 0x9c, 0x41, 0x04, 0x0b, 0xad, 0x29, 0x17,
    0x77, 0x6f, 0x69, 0x0d, 0xd5, 0xf5, 0x3f, 0x9b, 0x90, 0x48, 0x46, 0xca, 0x9b, 0xb2, 0x40, 0x9b, 0x45, 0xbf, 0x0b,
    0xb4, 0xb5, 0xcc, 0xc5, 0xd7, 0x97, 0xe4, 0x99, 0x43, 0x9d, 0x27, 0x10, 0x74, 0x66, 0xec, 0xc0, 0xd9, 0x6e, 0xe8,
    0x17, 0xdd, 0xae, 0x49, 0x96, 0xd0, 0x09, 0x93, 0x3a, 0xad, 0x9a, 0x25, 0x06, 0x44, 0xda, 0x24, 0xff, 0xed, 0x25,
    0x15, 0xaa, 0x68, 0x1a, 0x2e, 0x86, 0xdc, 0x5b, 0x07, 0x7b, 0x0c, 0xfe, 0xa3, 0x3c, 0x6b, 0x56, 0x13, 0xa2, 0x94,
    0x16, 0x73, 0xb6, 0x83, 0xdd, 0x59, 0xf5, 0x98, 0x72, 0xc8, 0xb1, 0xaa, 0x8f, 0x48, 0x7f, 0x82, 0x51, 0x1e, 0x53,
    0x47, 0x6a, 0xed, 0x73, 0x53, 0x5f, 0x19, 0x52, 0x41, 0xd0, 0x64, 0xd3, 0xd3, 0x32, 0x46, 0x95, 0x22, 0x59, 0x98,
    0x0a, 0xde, 0x17, 0x4a, 0x68, 0x2c, 0x94, 0xa2, 0x72, 0x22, 0x9b, 0x74, 0x31, 0x1e, 0x83, 0x61, 0x6f, 0x38, 0x02,
    0xb5, 0xb8, 0xad, 0x7f, 0xad, 0x6d, 0xbe, 0x57, 0xa9, 0x19, 0xfa, 0x95, 0xc8, 0xd3, 0x05, 0x0e, 0x9d, 0xda, 0xf4,
    0x18, 0xea, 0xcd, 0x20, 0xd5, 0x6b, 0x4f, 0x70, 0xd9, 0xef, 0xb3, 0x25, 0xad, 0x27, 0x7e, 0x55, 0x7b, 0xda, 0x9b,
    0x13, 0xc2, 0xb1, 0x53, 0xd1, 0x35, 0x6d, 0xf7, 0x8f, 0x12, 0xe7, 0x8c, 0x28, 0x39, 0x32, 0x84, 0x33, 0x76, 0x53,
    0x22, 0xd3, 0x8c, 0xa5, 0x2d, 0xf1, 0xb3, 0x3c, 0xd8, 0xc3, 0x4b, 0xc6, 0x5e, 0xf6, 0x4d, 0x52, 0xc2, 0xed, 0x2a,
    0xc9, 0x4b, 0x46, 0x53, 0x77, 0x11, 0x26, 0xe4, 0x35, 0xc6, 0xaa, 0x3b, 0x2e, 0x6a, 0xd7, 0x36};

/* AES-128 expected cipher
 */
static const uint8_t g_aes192_cipher[] = {
    0x20, 0xf9, 0x7e, 0x58, 0xcd, 0x19, 0xbe, 0xb6, 0x03, 0xab, 0xc0, 0xd5, 0xfc, 0x68, 0xeb, 0x0f, 0x5f, 0x05, 0x64,
    0x57, 0x91, 0xbe, 0xef, 0xbf, 0x17, 0x71, 0xad, 0x71, 0xd7, 0xd7, 0xe0, 0xb7, 0xe1, 0x0e, 0x60, 0x0d, 0x9e, 0xa0,
    0x6e, 0x21, 0xf7, 0xef, 0xd5, 0xd6, 0xab, 0xc6, 0x20, 0xa3, 0x5d, 0xcb, 0x71, 0xa3, 0x6f, 0xd3, 0x74, 0xe5, 0x84,
    0x91, 0x1b, 0x3c, 0x28, 0x36, 0xa1, 0xa2, 0x4f, 0x7c, 0x55, 0x59, 0x26, 0xd7, 0x2a, 0x99, 0x9d, 0x66, 0xee, 0x39,
    0xca, 0x93, 0xd1, 0x53, 0xba, 0xa7, 0xb0, 0xaf, 0xe3, 0xde, 0x90, 0x3d, 0x3c, 0x9e, 0x42, 0xb2, 0x8b, 0x22, 0xe7,
    0x4e, 0x6c, 0x44, 0x6e, 0x3a, 0x6b, 0xe1, 0x86, 0xb5, 0xea, 0xb6, 0x3b, 0x5e, 0x89, 0xed, 0x33, 0x1f, 0xec, 0x59,
    0x3e, 0x69, 0x28, 0x1b, 0x33, 0x3d, 0xc6, 0xc1, 0xb0, 0x25, 0xd0, 0x74, 0xfd, 0x6d, 0x0f, 0x6f, 0x70, 0x87, 0x46,
    0x85, 0xf8, 0x39, 0xf4, 0x31, 0xd5, 0xe1, 0xc5, 0x6d, 0x01, 0x2b, 0x22, 0xc0, 0x68, 0x70, 0x10, 0xe6, 0x8d, 0xe4,
    0xa0, 0x72, 0x94, 0x4a, 0x4b, 0x53, 0x0d, 0x76, 0x89, 0x0d, 0xf2, 0xd8, 0x0b, 0x91, 0x95, 0x36, 0x3a, 0xb0, 0xfa,
    0x9e, 0xa5, 0x20, 0x7f, 0x4c, 0x8e, 0x7f, 0x59, 0x46, 0xf2, 0x12, 0xc6, 0xb6, 0xbb, 0x29, 0x76, 0x35, 0x56, 0x61,
    0x11, 0x7e, 0xc8, 0xd6, 0x50, 0xf0, 0x21, 0x68, 0x59, 0xcd, 0x0b, 0x3f, 0xa2, 0xcb, 0xf5, 0xeb, 0xb3, 0x13, 0x6a,
    0x2a, 0xa0, 0x78, 0x60, 0xe7, 0x8f, 0x7c, 0x02, 0x92, 0x91, 0x1b, 0xe7, 0x04, 0x7e, 0x49, 0x5e, 0xac, 0x3d, 0x8f,
    0xa5, 0xf0, 0x04, 0xf5, 0x36, 0xdf, 0x41, 0xae, 0x77, 0x08, 0x22, 0x19, 0x37, 0x07, 0x67, 0x4e, 0x31, 0x4b, 0xdf,
    0x5a, 0xc4, 0x31, 0xce, 0x20, 0x54, 0x16, 0x79, 0x6b, 0x91, 0x5e, 0x7b, 0xa3, 0x56, 0x2f, 0xe8, 0xfb, 0x3d, 0xad,
    0xbd, 0x58, 0x7b, 0x94, 0x83, 0x7b, 0x01, 0x59, 0x11, 0x73, 0xb9, 0x05, 0x99, 0xda, 0x4d, 0xcf, 0xa7, 0xa4, 0xd0,
    0xd3, 0xc5, 0x9f, 0x48, 0x7e, 0xef, 0x95, 0x12, 0x01, 0x6c, 0x95, 0x0a, 0x2a, 0x7d, 0xcb, 0x43, 0x00, 0xcb, 0x96,
    0x34, 0xe4, 0x72, 0x31, 0xd4, 0x90, 0xf2, 0xb4, 0xa7, 0x2a, 0x20, 0x13, 0x96, 0x34, 0xbe, 0x70};

/* AES-256 expected cipher
 */
static const uint8_t g_aes256_cipher[] = {
    0xc2, 0x3b, 0xc4, 0x7e, 0xc5, 0x27, 0x71, 0x3d, 0xee, 0xd6, 0x06, 0x68, 0x24, 0xe6, 0x2b, 0x50, 0x8c, 0x83, 0xee,
    0x4d, 0xa2, 0x83, 0x6f, 0xc4, 0x7d, 0x0f, 0x79, 0xe9, 0x43, 0x4d, 0x9b, 0xe4, 0x5c, 0xcd, 0x92, 0xdc, 0xfb, 0x09,
    0xd0, 0x7d, 0xb9, 0x0a, 0x1d, 0xf8, 0xef, 0x81, 0xc4, 0x09, 0xf2, 0xbf, 0x9c, 0xb7, 0x5f, 0xaf, 0x1c, 0x0b, 0x6b,
    0xbb, 0xcf, 0x8e, 0xf4, 0x84, 0x7a, 0xbe, 0xbe, 0xc4, 0x8d, 0x79, 0x4d, 0x09, 0x9b, 0xf1, 0x60, 0xaa, 0xe3, 0x75,
    0x57, 0xce, 0x92, 0x59, 0x06, 0xb2, 0x56, 0xaf, 0x58, 0xb0, 0x73, 0x25, 0xd2, 0x24, 0x86, 0x57, 0xb8, 0xbc, 0x2e,
    0x6c, 0xd2, 0x97, 0x70, 0xe9, 0x42, 0x6f, 0x7c, 0x99, 0x60, 0xe3, 0xf0, 0xd3, 0xa8, 0xf6, 0x8f, 0x79, 0xb1, 0x90,
    0x31, 0x17, 0x03, 0x85, 0xc9, 0x14, 0x53, 0x49, 0xe9, 0x71, 0xc4, 0x04, 0xcb, 0x22, 0x82, 0xb0, 0x86, 0xc5, 0xc3,
    0x58, 0xd3, 0xb5, 0xad, 0xc1, 0x09, 0x1f, 0x0d, 0x85, 0x04, 0xf8, 0xbc, 0x05, 0x3f, 0xd2, 0x23, 0x86, 0x8d, 0xec,
    0x24, 0x85, 0x10, 0x81, 0x48, 0x49, 0x73, 0x4a, 0x5d, 0xee, 0x30, 0xbd, 0xba, 0x9d, 0x8c, 0x98, 0x43, 0xce, 0x72,
    0xd3, 0x01, 0x50, 0x16, 0x3e, 0x44, 0xb6, 0x8b, 0xd2, 0xc5, 0x9f, 0xbe, 0x7d, 0x24, 0x3d, 0x45, 0x8d, 0xf5, 0x3e,
    0x84, 0x7f, 0xcf, 0x8d, 0xcb, 0x94, 0x11, 0x98, 0xc9, 0xf7, 0x46, 0xe0, 0x6f, 0xf6, 0x7c, 0x38, 0xb9, 0xcd, 0x10,
    0xb3, 0x9c, 0x44, 0x7d, 0x48, 0x23, 0x51, 0x71, 0x33, 0xea, 0xad, 0xea, 0x4c, 0x13, 0x7a, 0xf4, 0x2c, 0x87, 0xf6,
    0x64, 0x00, 0xa4, 0xa4, 0x51, 0x4f, 0x2e, 0x2d, 0x45, 0x17, 0xa9, 0x36, 0xd5, 0xf1, 0x63, 0xd2, 0x56, 0x37, 0xb6,
    0x50, 0xf5, 0x29, 0x04, 0x68, 0x39, 0x17, 0x3f, 0xef, 0xcf, 0x9a, 0x53, 0x3a, 0xc0, 0xa7, 0x75, 0x33, 0x82, 0x5b,
    0x46, 0x91, 0xcc, 0x63, 0xa3, 0xe8, 0xde, 0xaf, 0x98, 0x53, 0x4f, 0xd1, 0x03, 0xd8, 0x82, 0x2c, 0x37, 0x8c, 0xbf,
    0xdd, 0xee, 0x6f, 0x21, 0x04, 0xb2, 0xa7, 0xb8, 0xec, 0x8f, 0xad, 0xc2, 0xd6, 0xc5, 0xfb, 0xe5, 0x7a, 0x03, 0x46,
    0xb7, 0xf9, 0x9d, 0x28, 0xba, 0x54, 0xb6, 0xd4, 0x34, 0xf5, 0x35, 0xa8, 0x6c, 0x61, 0x15, 0xf1};

/* DES3 expected cipher
 */
static const uint8_t g_des3Expected[] = {
    0xdc, 0x1b, 0x63, 0xa9, 0x7d, 0xfc, 0xdd, 0x12, 0x86, 0xf3, 0xa9, 0x0d, 0x9f, 0xa5, 0x68, 0xab, 0x57, 0x92, 0x25,
    0xcd, 0x2d, 0xe3, 0x44, 0x8c, 0xcf, 0x55, 0x17, 0xb8, 0xbc, 0xb7, 0x5a, 0xe4, 0x8b, 0x9f, 0x7b, 0xa5, 0x71, 0xe3,
    0x76, 0x6d, 0x61, 0x06, 0x4c, 0xe3, 0xd8, 0xcd, 0x37, 0x2c, 0x31, 0x83, 0x45, 0xda, 0xed, 0xbe, 0x22, 0x17, 0x56,
    0x98, 0xe5, 0xda, 0x6d, 0xc2, 0x24, 0x7e, 0x9e, 0xa9, 0x25, 0x2b, 0x90, 0x1a, 0x81, 0xb9, 0x42, 0xeb, 0xd0, 0x95,
    0x9e, 0xa8, 0x59, 0x8c, 0xed, 0xdc, 0x64, 0x9f, 0xcc, 0x11, 0x76, 0x2c, 0xd6, 0x5e, 0x77, 0xa0, 0xe9, 0x70, 0x0f,
    0xf9, 0xfe, 0x57, 0x45, 0x06, 0x0c, 0xc1, 0xcf, 0xb9, 0xf5, 0x9c, 0x0b, 0x05, 0xaf, 0xe0, 0xae, 0x91, 0xb3, 0x0c,
    0x4b, 0x52, 0x6f, 0x70, 0xa0, 0x99, 0x29, 0xb6, 0x13, 0xf3, 0x1a, 0x43, 0xf1, 0x70, 0x97, 0x42, 0x69, 0x72, 0xaf,
    0x69, 0x6c, 0xf4, 0x33, 0xac, 0x22, 0x22, 0x41, 0x42, 0x25, 0xf0, 0x2b, 0xbe, 0xed, 0xcf, 0x23, 0x24, 0xcd, 0x28,
    0x24, 0x61, 0xf2, 0x24, 0x5c, 0xb4, 0x8f, 0x2c, 0x77, 0x25, 0xed, 0x58, 0x06, 0x1e, 0xcc, 0xd7, 0xda, 0xbc, 0x72,
    0x60, 0xc1, 0xba, 0xf8, 0x35, 0x47, 0xcd, 0xa3, 0x77, 0x97, 0xfa, 0xa8, 0xae, 0x48, 0x49, 0x20, 0x39, 0x3a, 0x91,
    0xf7, 0x52, 0x8f, 0xa7, 0xaf, 0x0a, 0x71, 0x98, 0x61, 0xdf, 0xe7, 0x12, 0x7b, 0x78, 0x7d, 0x17, 0x4d, 0x48, 0x6a,
    0x4f, 0x88, 0x07, 0xf6, 0xef, 0x9e, 0x59, 0xab, 0x53, 0xd0, 0xaa, 0x84, 0x2b, 0xaa, 0x8b, 0xcf, 0x9e, 0x71, 0x8f,
    0xc4, 0xeb, 0x14, 0xc1, 0xfb, 0x28, 0x51, 0x87, 0xed, 0xf5, 0xdf, 0xe1, 0xcc, 0xcd, 0xf4, 0x56, 0xaf, 0x34, 0x1e,
    0x3e, 0xaa, 0xb7, 0xd2, 0x65, 0xbb, 0x47, 0xef, 0x37, 0x1a, 0xce, 0x90, 0x68, 0x52, 0xfd, 0xa3, 0xfe, 0x62, 0xd7,
    0x00, 0x22, 0xda, 0xd4, 0x4b, 0xf5, 0xe7, 0x04, 0xd5, 0xdc, 0x80, 0x6e, 0x76, 0x8e, 0x37, 0x13, 0x8a, 0xf7, 0x63,
    0x53, 0x7c, 0x8f, 0xe0, 0x85, 0x34, 0x57, 0x93, 0xfb, 0x3d, 0x1b, 0xfc, 0x12, 0x04, 0xd2, 0x0f, 0xc9, 0x3f, 0x80,
    0x5d, 0xf7, 0xbc, 0xc9, 0x5b, 0xd5, 0x88, 0x57, 0xcd, 0xa9, 0xb0, 0xb0, 0x3d, 0x08, 0x37, 0xd2};

/* SHA1 expected result in little-endiness
 */
static uint8_t g_Sha1_expected[] = {0xc6, 0xe1, 0xd4, 0x2f, 0xfc, 0x28, 0x2d, 0x7a, 0xe1, 0x9e,
                                    0x84, 0xed, 0x39, 0xe7, 0x76, 0xbb, 0x12, 0xeb, 0x93, 0x1b};

/* SHA256 expected result in little-endiness
 */
static uint8_t g_Sha256_expected[] = {0xb3, 0xfb, 0xa8, 0xd7, 0x94, 0x80, 0xd7, 0x07, 0xbc, 0x9a, 0xca,
                                      0x69, 0x4f, 0x2e, 0x08, 0xb0, 0xe4, 0x51, 0x56, 0x8d, 0x76, 0xdb,
                                      0x3c, 0x6d, 0xbf, 0xd0, 0x02, 0x2d, 0x92, 0xe5, 0xc9, 0x37};
/* MD5 expected result in little-endiness
 */
static uint8_t g_testMd5Expected[] = {0x9e, 0x10, 0x7d, 0x9d, 0x37, 0x2b, 0xb6, 0x82,
                                      0x6b, 0xd8, 0x1d, 0x35, 0x42, 0xa4, 0x19, 0xd6};
#endif /* DEMO_MMCAU_PASS_RATE */
static uint8_t g_output[OUTPUT_ARRAY_LEN];
static uint8_t g_result[OUTPUT_ARRAY_LEN];

static volatile uint32_t g_msCount = 0;
static volatile bool g_irq_random  = false;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void mmcau_print_msg(const uint8_t *data, uint32_t length);
static void mmcau_example_task(void);

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief counter since last POR/reset.
 */
void SysTick_Handler(void)
{
    g_msCount++;
#if defined(DEMO_MMCAU_PASS_RATE)
    if (g_irq_random)
    {
        uint32_t randomDelay;
        GetRandomData32(&randomDelay);
        randomDelay = randomDelay % 1000;

        /* call CMSIS SysTick function. It enables the SysTick interrupt at low priority */
        SysTick_Config(CORE_CLK_FREQ / randomDelay); /* 0-1 ms period */
    }
#endif /* DEMO_MMCAU_PASS_RATE */
}

/*!
 * @brief SysTick period configuration and interrupt enable.
 */
static uint32_t time_config(bool random)
{
#if defined(DEMO_MMCAU_PASS_RATE)
    if (random)
    {
        uint32_t randomDelay;
        GetRandomData32(&randomDelay);
        randomDelay  = randomDelay % 1000;
        g_irq_random = true;

        /* call CMSIS SysTick function. It enables the SysTick interrupt at low priority */
        return SysTick_Config(CORE_CLK_FREQ / randomDelay); /* 0-1 ms period */
    }
    else
#endif /* DEMO_MMCAU_PASS_RATE */
    {
        g_irq_random = false;
        /* call CMSIS SysTick function. It enables the SysTick interrupt at low priority */
        return SysTick_Config(CORE_CLK_FREQ / 1000); /* 1 ms period */
    }
}

/*!
 * @brief Get milliseconds since last POR/reset.
 */
static float time_get_ms(void)
{
    uint32_t currMsCount;
    uint32_t currTick;
    uint32_t loadTick;

    do
    {
        currMsCount = g_msCount;
        currTick    = SysTick->VAL;
    } while (currMsCount != g_msCount);

    loadTick = CORE_CLK_FREQ / 1000;
    return (float)currMsCount + ((float)loadTick - (float)currTick) / (float)loadTick;
}

/*!
 * @brief Get througput in MB/s
 * @param elapsedMs Time interval in milliseconds.
 * @param numBytes Number of bytes processed within the given interval.
 * @return Throughput in MB/s.
 */
static float mmcau_get_throughput(float elapsedMs, size_t numBytes)
{
    return ((float)(1000 * numBytes / 1024 / 1024) / elapsedMs);
}

/*!
 * @brief Function mmcau_print_msg prints data bytes into console.
 */
static void mmcau_print_msg(const uint8_t *data, uint32_t length)
{
    uint32_t i;

    PRINTF("          ");
    for (i = 0; i < length; i++)
    {
        PUTCHAR(data[i]);
        if (data[i] == ',')
        {
            PRINTF("\r\n          ");
        }
    }
    PRINTF("\r\n");
}

/*
 * mmcau_encrypt_aes_cbc: AES encryption function
 *
 * Parameters:
 *   [in] key: key of 8 bytes
 *   [in] mode: 128, 192 or 256 AES mode
 *   [in] inputData: pointer to in data
 *   [out] outputData: pointer to out data
 *   [in] dataLength: number of bytes of input data. Must be divisible by 8 (DES block)
 *   [in] initVector: initVector to use during xor
 * Return:
 *   0 if OK, otherwise error
 *
 */
static int mmcau_encrypt_aes_cbc(const uint8_t *key,
                                 uint32_t mode,
                                 const uint8_t *inputData,
                                 uint8_t *outputData,
                                 uint16_t dataLength,
                                 const uint8_t *initVector)
{
    uint8_t i;
    uint16_t blocks;
    uint16_t rounds;
    uint8_t tempBlock[AES_BLOCK_LENGTH];
    uint8_t tempIV[AES_BLOCK_LENGTH];
    /*
     * AES128 needs 44 longwords for expansion
     * AES192 needs 52 longwords for expansion
     * AES256 needs 60 longwords for expansion
     *    Reserving 60 longwords as the max space
     */
    uint32_t keyLen;
    uint8_t keyExpansion[60 * 4];

    /*validate NULL for key, inputData, outputData or initVector*/
    if ((key == NULL) || (inputData == NULL) || (outputData == NULL) || (initVector == NULL))
    {
        return MMCAU_ERROR; /*wrong pointer*/
    }

    /*validate AES mode*/
    if ((mode != 128u) && (mode != 192u) && (mode != 256u))
    {
        return MMCAU_ERROR; /*wrong AES mode*/
    }

    /*validate data length*/
    if (dataLength % AES_BLOCK_LENGTH)
    {
        return MMCAU_ERROR; /*wrong length*/
    }

    /*calculate number of AES rounds*/
    if (mode == 128u)
    {
        rounds = 10u;
        keyLen = 16u;
    }
    else if (mode == 192u)
    {
        rounds = 12u;
        keyLen = 24u;
    }
    else /*AES256*/
    {
        rounds = 14u;
        keyLen = 32u;
    }

    /*expand AES key*/
    MMCAU_AES_SetKey(key, keyLen, keyExpansion);

    /*execute AES in CBC mode*/
    /*http://en.wikipedia.org/wiki/Block_cipher_modes_of_operation*/

    /*get number of blocks*/
    blocks = dataLength / AES_BLOCK_LENGTH;

    /*copy init vector to temp storage*/
    memcpy(tempIV, initVector, AES_BLOCK_LENGTH);

    do
    {
        /*copy to temp storage*/
        memcpy(tempBlock, inputData, AES_BLOCK_LENGTH);

        /*xor for CBC*/
        for (i = 0; i < AES_BLOCK_LENGTH; i++)
        {
            tempBlock[i] ^= tempIV[i];
        }

        /*FSL: single AES round*/
        MMCAU_AES_EncryptEcb(tempBlock, keyExpansion, rounds, outputData);

        /*update initVector for next 3DES round*/
        memcpy((void *)tempIV, (void *)outputData, AES_BLOCK_LENGTH);

        /*adjust pointers for next 3DES round*/
        inputData += AES_BLOCK_LENGTH;
        outputData += AES_BLOCK_LENGTH;

    } while (--blocks);

    return MMCAU_OK;
}

/*
 * mmcau_decrypt_aes_cbc: AES decryption function
 *
 * Parameters:
 *   [in] key: key of 8 bytes
 *   [in] mode: 128, 192 or 256 AES mode
 *   [in] inputData: pointer to in data
 *   [out] outputData: pointer to out data
 *   [in] dataLength: number of bytes of input data. Must be divisible by 8 (DES block)
 *   [in] initVector: initVector to use during xor
 * Return:
 *   0 if OK, otherwise error
 *
 */
static int mmcau_decrypt_aes_cbc(const uint8_t *key,
                                 uint32_t mode,
                                 const uint8_t *inputData,
                                 uint8_t *outputData,
                                 uint16_t dataLength,
                                 const uint8_t *initVector)
{
    uint8_t i;
    uint16_t blocks;
    uint16_t rounds;
    uint8_t tempBlock[AES_BLOCK_LENGTH];
    uint8_t tempIV[AES_BLOCK_LENGTH];
    /*
     * AES128 needs 44 longwords for expansion
     * AES192 needs 52 longwords for expansion
     * AES256 needs 60 longwords for expansion
     *    Reserving 60 longwords as the max space
     */
    uint8_t keyExpansion[60 * 4];
    uint32_t keyLen;

    /*validate NULL for key, inputData, outputData or initVector*/
    if ((key == NULL) || (inputData == NULL) || (outputData == NULL) || (initVector == NULL))
    {
        return MMCAU_ERROR; /*wrong pointer*/
    }

    /*validate AES mode*/
    if ((mode != 128u) && (mode != 192u) && (mode != 256u))
    {
        return MMCAU_ERROR; /*wrong AES mode*/
    }

    /*validate data length*/
    if (dataLength % AES_BLOCK_LENGTH)
    {
        return MMCAU_ERROR; /*wrong length*/
    }

    /*calculate number of AES rounds*/
    if (mode == 128u)
    {
        rounds = 10u;
        keyLen = 16u;
    }
    else if (mode == 192u)
    {
        rounds = 12u;
        keyLen = 24u;
    }
    else /*AES256*/
    {
        rounds = 14u;
        keyLen = 32u;
    }

    /*expand AES key*/
    MMCAU_AES_SetKey(key, keyLen, keyExpansion);

    /*execute AES in CBC mode*/
    /*http://en.wikipedia.org/wiki/Block_cipher_modes_of_operation*/

    /*get number of blocks*/
    blocks = dataLength / AES_BLOCK_LENGTH;

    /*copy init vector to temp storage*/
    memcpy(tempIV, initVector, AES_BLOCK_LENGTH);

    do
    {
        /*copy to temp storage*/
        memcpy(tempBlock, inputData, AES_BLOCK_LENGTH);

        /*FSL: single AES round*/
        MMCAU_AES_DecryptEcb(inputData, keyExpansion, rounds, outputData);

        /*xor for CBC*/
        for (i = 0; i < AES_BLOCK_LENGTH; i++)
        {
            outputData[i] ^= tempIV[i];
        }

        /*update initVector for next AES round*/
        memcpy(tempIV, tempBlock, AES_BLOCK_LENGTH);

        /*adjust pointers for next 3DES round*/
        inputData += AES_BLOCK_LENGTH;
        outputData += AES_BLOCK_LENGTH;

    } while (--blocks);

    return MMCAU_OK;
}

/*
 * mmcau_encrypt_3des_cbc: 3DES encryption function
 *
 * Parameters:
 *   [in] key: key of 24 bytes
 *   [in] inputData: pointer to in data
 *   [out] outputData: pointer to out data
 *   [in] dataLength: number of bytes of input data. Must be divisible by 8 (3DES block)
 *   [in] initVector: initVector to use during xor
 * Return:
 *   0 if OK, otherwise error
 *
 */
int mmcau_encrypt_3des_cbc(
    uint8_t *key, const uint8_t *inputData, uint8_t *outputData, uint16_t dataLength, const uint8_t *initVector)
{
    uint8_t i;
    uint16_t blocks;
    uint8_t tempBlock[DES3_BLOCK_LENGTH];
    uint8_t tempIV[DES3_BLOCK_LENGTH];

    /*validate NULL for key, inputData, outputData or initVector*/
    if ((key == NULL) || (inputData == NULL) || (outputData == NULL) || (initVector == NULL))
    {
        return MMCAU_ERROR; /*wrong pointer*/
    }

    /*validate data length*/
    if (dataLength % DES3_BLOCK_LENGTH)
    {
        return MMCAU_ERROR; /*wrong length*/
    }

    /*fix parity key if needed: keep in mind LSB of each byte is only used for parity*/
    for (i = 0; i < DES3_KEY_LENGTH; i++)
    {
        key[i] = ((key[i] & 0xFEu) | g_parityBits[key[i] >> 1]);
    }

    /* optional parity check */
    if ((MMCAU_DES_ChkParity(key) != kStatus_Success) ||
        (MMCAU_DES_ChkParity(key + DES3_BLOCK_LENGTH) != kStatus_Success) ||
        (MMCAU_DES_ChkParity(key + (2 * DES3_BLOCK_LENGTH)) != kStatus_Success))
    {
        return MMCAU_ERROR; /*wrong parity*/
    }

    /*execute 3DES in CBC mode*/
    /*http://en.wikipedia.org/wiki/Block_cipher_modes_of_operation*/

    /*get number of blocks*/
    blocks = dataLength / DES3_BLOCK_LENGTH;

    /*copy init vector to temp storage*/
    memcpy(tempIV, initVector, DES3_BLOCK_LENGTH);

    do
    {
        /*copy to temp storage*/
        memcpy(tempBlock, inputData, DES3_BLOCK_LENGTH);

        /*xor for CBC*/
        for (i = 0; i < DES3_BLOCK_LENGTH; i++)
        {
            tempBlock[i] ^= tempIV[i];
        }

        /*FSL: 1st DES round*/
        MMCAU_DES_EncryptEcb(tempBlock, key, outputData);

        /*FSL: 2nd DES round*/
        /*adjust key to 2nd part*/
        MMCAU_DES_DecryptEcb(outputData, key + DES3_BLOCK_LENGTH, outputData);

        /*FSL: 3rd DES round*/
        /*adjust key to 3rd part*/
        MMCAU_DES_EncryptEcb(outputData, key + (2 * DES3_BLOCK_LENGTH), outputData);

        /*update initVector for next 3DES round*/
        memcpy(tempIV, outputData, DES3_BLOCK_LENGTH);

        /*adjust pointers for next 3DES round*/
        inputData += DES3_BLOCK_LENGTH;
        outputData += DES3_BLOCK_LENGTH;

    } while (--blocks);

    return MMCAU_OK;
}

/*
 * mmcau_decrypt_3des_cbc: 3DES decryption function
 *
 * Parameters:
 *   [in] key: key of 24 bytes
 *   [in] inputData: pointer to in data
 *   [out] outputData: pointer to out data
 *   [in] dataLength: number of bytes of input data. Must be divisible by 8 (3DES block)
 *   [in] initVector: initVector to use during xor
 * Return:
 *   0 if OK, otherwise error
 *
 */
int mmcau_decrypt_3des_cbc(
    uint8_t *key, const uint8_t *inputData, uint8_t *outputData, uint16_t dataLength, const uint8_t *initVector)
{
    uint8_t i;
    uint16_t blocks;
    uint8_t tempBlock[DES3_BLOCK_LENGTH];
    uint8_t tempIV[DES3_BLOCK_LENGTH];

    /*validate NULL for key, inputData, outputData or initVector*/
    if ((key == NULL) || (inputData == NULL) || (outputData == NULL) || (initVector == NULL))
    {
        return MMCAU_ERROR; /*wrong pointer*/
    }

    /*validate data length*/
    if (dataLength % DES3_BLOCK_LENGTH)
    {
        return MMCAU_ERROR; /*wrong length*/
    }

    /*fix parity key if needed: keep in mind LSB of each byte is only used for parity*/
    for (i = 0; i < DES3_KEY_LENGTH; i++)
    {
        key[i] = ((key[i] & 0xFEu) | g_parityBits[key[i] >> 1]);
    }

    /* optional -- check parity*/

    if ((MMCAU_DES_ChkParity(key) != kStatus_Success) ||
        (MMCAU_DES_ChkParity(key + DES3_BLOCK_LENGTH) != kStatus_Success) ||
        (MMCAU_DES_ChkParity(key + (2 * DES3_BLOCK_LENGTH)) != kStatus_Success))
    {
        return MMCAU_ERROR; /*wrong parity*/
    }

    /*execute 3DES in CBC mode*/
    /*http://en.wikipedia.org/wiki/Block_cipher_modes_of_operation*/

    /*get number of blocks*/
    blocks = dataLength / DES3_BLOCK_LENGTH;

    /*copy init vector to temp storage*/
    memcpy(tempIV, initVector, DES3_BLOCK_LENGTH);

    do
    {
        /*copy to temp storage*/
        memcpy(tempBlock, inputData, DES3_BLOCK_LENGTH);

        /*FSL: 1st DES round*/
        MMCAU_DES_DecryptEcb(inputData, key + (2 * DES3_BLOCK_LENGTH), outputData);

        /*FSL: 2nd DES round*/
        /*adjust key to 2nd part*/
        MMCAU_DES_EncryptEcb(outputData, key + DES3_BLOCK_LENGTH, outputData);

        /*FSL: 3rd DES round*/
        /*adjust key to 3rd part*/
        MMCAU_DES_DecryptEcb(outputData, key, outputData);

        /*xor for CBC*/
        for (i = 0; i < DES3_BLOCK_LENGTH; i++)
        {
            outputData[i] ^= tempIV[i];
        }

        /*update initVector for next 3DES round*/
        memcpy(tempIV, tempBlock, DES3_BLOCK_LENGTH);

        /*adjust pointers for next 3DES round*/
        inputData += DES3_BLOCK_LENGTH;
        outputData += DES3_BLOCK_LENGTH;

    } while (--blocks);

    return MMCAU_OK;
}

/*!
 * @brief Function mmcau_example_task demonstrates use of mmcau
 *        encryption/decryption functions on short sample text.
 */
static void mmcau_example_task(void)
{
    uint32_t length;
    uint32_t blocks;
    uint32_t resultSha1[SHA1_RESULT_LENGTH / sizeof(uint32_t)];
    uint32_t resultSha256[SHA256_RESULT_LENGTH / sizeof(uint32_t)];
    uint32_t resultMd5[MD5_RESULT_LENGTH / sizeof(uint32_t)];
    uint8_t status;
    float timeBefore;
    float timeAfter;
    int cycles;

    /* Print welcome string */
    PRINTF("............................. MMCAU  DRIVER  EXAMPLE ............................. \r\n\r\n");
    memset(&g_output[0], 0, OUTPUT_ARRAY_LEN);
    memset(&g_result[0], 0, OUTPUT_ARRAY_LEN);
    length = sizeof(g_testFull) - 1u;

    PRINTF("Testing input string: \r\n");
    mmcau_print_msg(&g_testFull[0], length);
    /* Format console output */
    PRINTF("\r\n");

    /***************************************************/
    /******* FIRST PART USING AES-CBC method *********/
    /***************************************************/
    PRINTF("----------------------------------- AES-128-CBC method --------------------------------------\r\n");

    /*   ENCRYPTION   */
    PRINTF("AES-128 CBC Encryption of %d bytes.\r\n", length);

    /* Call AES_cbc encryption */
    cycles     = CYCLES_FOR_THROUGHPUT;
    timeBefore = time_get_ms();
    while (cycles)
    {
        status = mmcau_encrypt_aes_cbc(g_aesKey128, AES128, g_testFull, g_output, length, g_aesIV);
        if (status != MMCAU_OK)
        {
            PRINTF("AES-128 CBC encryption failed !\r\n");
            return;
        }
        cycles--;
    }
    timeAfter = time_get_ms();

    /* Result message */
    PRINTF("AES-128 CBC encryption finished. Speed %f MB/s.\r\n\r\n",
           mmcau_get_throughput(timeAfter - timeBefore, CYCLES_FOR_THROUGHPUT * length));

    /*   DECRYPTION   */
    PRINTF("AES-128 CBC Decryption of %d bytes.\r\n", length);

    /* Call AES_cbc decryption */
    cycles     = CYCLES_FOR_THROUGHPUT;
    timeBefore = time_get_ms();
    while (cycles)
    {
        status = mmcau_decrypt_aes_cbc(g_aesKey128, AES128, g_output, g_result, length, g_aesIV);
        if (status != MMCAU_OK)
        {
            PRINTF("AES-128 CBC decryption failed !\r\n");
            return;
        }
        cycles--;
    }
    timeAfter = time_get_ms();

    /* Result message */
    PRINTF("AES-128 CBC decryption finished. Speed %f MB/s.\r\n",
           mmcau_get_throughput(timeAfter - timeBefore, CYCLES_FOR_THROUGHPUT * length));
    /* Print decrypted string */
    PRINTF("Decrypted string :\r\n");
    mmcau_print_msg(g_result, length);
    PRINTF("\r\n");

#if defined(DEMO_MMCAU_PASS_RATE)
    time_config(true);
    uint32_t CountPass = 0U;
    uint32_t CountFail = 0U;
    float PassRate     = 0.0;

    /* Call AES_cbc encryption */
    cycles = CYCLES_FOR_PASSRATE;

    while (cycles)
    {
        status = mmcau_encrypt_aes_cbc(g_aesKey128, AES128, g_testFull, g_output, length, g_aesIV);
        if (status != MMCAU_OK)
        {
            PRINTF("AES-128 CBC encryption failed !\r\n");
            return;
        }

        if (memcmp(g_aes128_cipher, g_output, length) == 0)
        {
            CountPass++;
        }
        else
        {
            CountFail++;
        }

        status = mmcau_decrypt_aes_cbc(g_aesKey128, AES128, g_output, g_result, length, g_aesIV);

        if (status != MMCAU_OK)
        {
            PRINTF("AES-128 CBC decryption failed !\r\n");
            return;
        }

        if (memcmp(g_testFull, g_result, length) == 0)
        {
            CountPass++;
        }
        else
        {
            CountFail++;
        }

        PassRate = (float)(CountPass * 100.0f) / ((float)(CountPass + CountFail));
        cycles--;
    }
    time_config(false);
    PRINTF("AES-128 CBC encryption & decryption finished. Sucess rate %f % \r\n", PassRate);
#endif /* DEMO_MMCAU_PASS_RATE */
    PRINTF("----------------------------------- AES-192-CBC method --------------------------------------\r\n");

    /*   ENCRYPTION   */
    PRINTF("AES-192 CBC Encryption of %d bytes.\r\n", length);

    /* Call AES_cbc encryption */
    cycles     = CYCLES_FOR_THROUGHPUT;
    timeBefore = time_get_ms();
    while (cycles)
    {
        status = mmcau_encrypt_aes_cbc(g_aesKey192, AES192, g_testFull, g_output, length, g_aesIV);
        if (status != MMCAU_OK)
        {
            PRINTF("AES-192 CBC encryption failed !\r\n");
            return;
        }
        cycles--;
    }
    timeAfter = time_get_ms();

    /* Result message */
    PRINTF("AES-192 CBC encryption finished. Speed %f MB/s.\r\n\r\n",
           mmcau_get_throughput(timeAfter - timeBefore, CYCLES_FOR_THROUGHPUT * length));

    /*   DECRYPTION   */
    PRINTF("AES-192 CBC Decryption of %d bytes.\r\n", length);

    /* Call AES_cbc decryption */
    cycles     = CYCLES_FOR_THROUGHPUT;
    timeBefore = time_get_ms();
    while (cycles)
    {
        status = mmcau_decrypt_aes_cbc(g_aesKey192, AES192, g_output, g_result, length, g_aesIV);
        if (status != MMCAU_OK)
        {
            PRINTF("AES-192 CBC decryption failed !\r\n");
            return;
        }
        cycles--;
    }
    timeAfter = time_get_ms();

    /* Result message */
    PRINTF("AES-192 CBC decryption finished. Speed %f MB/s.\r\n",
           mmcau_get_throughput(timeAfter - timeBefore, CYCLES_FOR_THROUGHPUT * length));
    /* Print decrypted string */
    PRINTF("Decrypted string :\r\n");
    mmcau_print_msg(g_result, length);
    PRINTF("\r\n");

#if defined(DEMO_MMCAU_PASS_RATE)
    time_config(true);
    CountPass = 0U;
    CountFail = 0U;
    PassRate  = 0.0;

    /* Call AES_cbc encryption */
    cycles = CYCLES_FOR_PASSRATE;

    while (cycles)
    {
        status = mmcau_encrypt_aes_cbc(g_aesKey192, AES192, g_testFull, g_output, length, g_aesIV);
        if (status != MMCAU_OK)
        {
            PRINTF("AES-192 CBC encryption failed !\r\n");
            return;
        }

        if (memcmp(g_aes192_cipher, g_output, length) == 0)
        {
            CountPass++;
        }
        else
        {
            CountFail++;
        }

        status = mmcau_decrypt_aes_cbc(g_aesKey192, AES192, g_output, g_result, length, g_aesIV);

        if (status != MMCAU_OK)
        {
            PRINTF("AES-192 CBC decryption failed !\r\n");
            return;
        }

        if (memcmp(g_testFull, g_result, length) == 0)
        {
            CountPass++;
        }
        else
        {
            CountFail++;
        }

        PassRate = (float)(CountPass * 100.0f) / ((float)(CountPass + CountFail));
        cycles--;
    }
    time_config(false);
    PRINTF("AES-192 CBC encryption & decryption finished. Sucess rate %f % \r\n", PassRate);
#endif /* DEMO_MMCAU_PASS_RATE */
    PRINTF("----------------------------------- AES-256-CBC method --------------------------------------\r\n");

    /*   ENCRYPTION   */
    PRINTF("AES-256 CBC Encryption of %d bytes.\r\n", length);

    /* Call AES_cbc encryption */
    cycles     = CYCLES_FOR_THROUGHPUT;
    timeBefore = time_get_ms();
    while (cycles)
    {
        status = mmcau_encrypt_aes_cbc(g_aesKey256, AES256, g_testFull, g_output, length, g_aesIV);
        if (status != MMCAU_OK)
        {
            PRINTF("AES-256 CBC encryption failed !\r\n");
            return;
        }
        cycles--;
    }
    timeAfter = time_get_ms();

    /* Result message */
    PRINTF("AES-256 CBC encryption finished. Speed %f MB/s.\r\n\r\n",
           mmcau_get_throughput(timeAfter - timeBefore, CYCLES_FOR_THROUGHPUT * length));

    /*   DECRYPTION   */
    PRINTF("AES-256 CBC Decryption of %d bytes.\r\n", length);

    /* Call AES_cbc decryption */
    cycles     = CYCLES_FOR_THROUGHPUT;
    timeBefore = time_get_ms();
    while (cycles)
    {
        status = mmcau_decrypt_aes_cbc(g_aesKey256, AES256, g_output, g_result, length, g_aesIV);
        if (status != MMCAU_OK)
        {
            PRINTF("AES-256 CBC decryption failed !\r\n");
            return;
        }
        cycles--;
    }
    timeAfter = time_get_ms();

    /* Result message */
    PRINTF("AES-256 CBC decryption finished. Speed %f MB/s.\r\n",
           mmcau_get_throughput(timeAfter - timeBefore, CYCLES_FOR_THROUGHPUT * length));
    /* Print decrypted string */
    PRINTF("Decrypted string :\r\n");
    mmcau_print_msg(g_result, length);
    PRINTF("\r\n");

#if defined(DEMO_MMCAU_PASS_RATE)
    /* Call AES_cbc encryption */
    time_config(true);
    CountPass = 0U;
    CountFail = 0U;
    PassRate  = 0.0;

    cycles = CYCLES_FOR_PASSRATE;
    while (cycles)
    {
        /* AES-CBC 256 */
        status = mmcau_encrypt_aes_cbc(g_aesKey256, AES256, g_testFull, g_output, length, g_aesIV);
        if (status != MMCAU_OK)
        {
            PRINTF("AES CBC encryption failed !\r\n");
            return;
        }

        if (memcmp(g_aes256_cipher, g_output, length) == 0)
        {
            CountPass++;
        }
        else
        {
            CountFail++;
        }

        status = mmcau_decrypt_aes_cbc(g_aesKey256, AES256, g_output, g_result, length, g_aesIV);
        if (status != MMCAU_OK)
        {
            PRINTF("AES CBC decryption failed !\r\n");
            return;
        }
        if (memcmp(g_testFull, g_result, length) == 0)
        {
            CountPass++;
        }
        else
        {
            CountFail++;
        }

        PassRate = (float)(CountPass * 100.0f) / ((float)(CountPass + CountFail));
        cycles--;
    }
    time_config(false);
    PRINTF("AES-256 CBC encryption & decryption finished. Sucess rate %f % \r\n", PassRate);
    PRINTF("\r\n");
#endif /* DEMO_MMCAU_PASS_RATE */

    /***************************************************/
    /******* SECOND PART USING DES3-CBC method ********/
    /***************************************************/
    PRINTF("----------------------------------- DES3-CBC method --------------------------------------\r\n");
    length = sizeof(g_testFull) - 1u;

    /*   ENCRYPTION   */
    PRINTF("DES3 CBC Encryption of %d bytes.\r\n", length);
    /* Call DES3_cbc encryption */
    cycles     = CYCLES_FOR_THROUGHPUT;
    timeBefore = time_get_ms();
    while (cycles)
    {
        status = mmcau_encrypt_3des_cbc(g_des3Key, g_testFull, g_output, length, g_des3IV);
        if (status != MMCAU_OK)
        {
            PRINTF("DES3 CBC encryption failed !\r\n");
            return;
        }
        cycles--;
    }
    timeAfter = time_get_ms();

    /* Result message */
    PRINTF("DES3 CBC encryption finished. Speed %f MB/s.\r\n\r\n",
           mmcau_get_throughput(timeAfter - timeBefore, CYCLES_FOR_THROUGHPUT * length));

    /*   DECRYPTION   */
    PRINTF("DES3 CBC decryption of %d bytes.\r\n", length);
    /* Call DES3_cbc decryption */
    cycles     = CYCLES_FOR_THROUGHPUT;
    timeBefore = time_get_ms();
    while (cycles)
    {
        status = mmcau_decrypt_3des_cbc(g_des3Key, g_output, g_result, length, g_des3IV);
        if (status != MMCAU_OK)
        {
            PRINTF("DES3 CBC decryption failed !\r\n");
            return;
        }
        cycles--;
    }
    timeAfter = time_get_ms();
    /* Result message */
    PRINTF("DES3 CBC decryption finished. Speed %f MB/s.\r\n",
           mmcau_get_throughput(timeAfter - timeBefore, CYCLES_FOR_THROUGHPUT * length));
    /* Print decrypted string */
    PRINTF("Decrypted string :\r\n");
    mmcau_print_msg(g_result, length);
    PRINTF("\r\n");

#if defined(DEMO_MMCAU_PASS_RATE)
    /* Call DES3  */
    time_config(true);
    CountPass = 0U;
    CountFail = 0U;
    PassRate  = 0.0;

    cycles = CYCLES_FOR_PASSRATE;
    while (cycles)
    {
        status = mmcau_encrypt_3des_cbc(g_des3Key, g_testFull, g_output, length, g_des3IV);
        if (status != kStatus_Success)
        {
            PRINTF("DES3 CBC encryption failed !\r\n");
            return;
        }

        if (memcmp(g_output, g_des3Expected, sizeof(g_des3Expected)) == 0)
        {
            CountPass++;
        }
        else
        {
            CountFail++;
        }

        status = mmcau_decrypt_3des_cbc(g_des3Key, g_output, g_result, length, g_des3IV);
        if (status != MMCAU_OK)
        {
            PRINTF("DES3 CBC decryption failed !\r\n");
            return;
        }
        if (memcmp(g_testFull, g_result, length) == 0)
        {
            CountPass++;
        }
        else
        {
            CountFail++;
        }

        PassRate = (float)(CountPass * 100.0f) / ((float)(CountPass + CountFail));
        cycles--;
    }

    time_config(false);
    PRINTF("DES3 encryption & decryption finished. Sucess rate %f % \r\n", PassRate);
    PRINTF("\r\n");
#endif /* DEMO_MMCAU_PASS_RATE */

    /***************************************************/
    /******* THIRD PART USES HASH FUNCTIONALITY ******/
    /***************************************************/
    /*get string length*/
    length = sizeof(g_testSha);
    PRINTF("--------------------------------------- HASH ------------------------------------------\r\n");
    PRINTF("Computing hash of %d bytes. \r\n", length);
    PRINTF("Input string: \r\n");
    mmcau_print_msg((uint8_t *)&g_testSha[0], 43u /* length of the test string (without padding bits) */);
    PRINTF("\r\n");

    /*calculate number of 512-bit blocks present in the message*/
    /*multiple of CRYPTO_BLOCK_LENGTH bytes alway because of padding*/
    blocks = length / CRYPTO_BLOCK_LENGTH;

    /*Compute SHA1 */
    cycles     = CYCLES_FOR_THROUGHPUT;
    timeBefore = time_get_ms();
    while (cycles)
    {
        MMCAU_SHA1_Update(g_testSha, blocks, resultSha1);
        cycles--;
    }
    timeAfter = time_get_ms();
    PRINTF("Computed SHA1 at speed %f MB/s:\r\n",
           mmcau_get_throughput(timeAfter - timeBefore, CYCLES_FOR_THROUGHPUT * length));
    for (int i = 0; i < SHA1_RESULT_LENGTH / sizeof(uint32_t); i++)
    {
        PRINTF("%08x", resultSha1[i]);
    }
    PRINTF("\r\n\r\n");

#if defined(DEMO_MMCAU_PASS_RATE)
    /* Call SHA1  */
    time_config(true);
    CountPass = 0U;
    CountFail = 0U;
    PassRate  = 0.0;

    cycles = CYCLES_FOR_PASSRATE;
    while (cycles)
    {
        MMCAU_SHA1_Update(g_testSha, blocks, resultSha1);
        if (status != kStatus_Success)
        {
            PRINTF("SHA1 update failed !\r\n");
            return;
        }

        if (memcmp(resultSha1, g_Sha1_expected, sizeof(g_Sha1_expected)) == 0)
        {
            CountPass++;
        }
        else
        {
            CountFail++;
        }

        PassRate = (float)(CountPass * 100.0f) / ((float)(CountPass + CountFail));
        cycles--;
    }
    time_config(false);
    PRINTF("SHA1 finished. Sucess rate %f % \r\n", PassRate);
    PRINTF("\r\n");
#endif /* DEMO_MMCAU_PASS_RATE */

    /*Compute SHA256 */
    cycles     = CYCLES_FOR_THROUGHPUT;
    timeBefore = time_get_ms();
    while (cycles)
    {
        MMCAU_SHA256_Update(g_testSha, blocks, resultSha256);
        cycles--;
    }
    timeAfter = time_get_ms();
    PRINTF("Computed SHA256 at speed %f MB/s:\r\n",
           mmcau_get_throughput(timeAfter - timeBefore, CYCLES_FOR_THROUGHPUT * length));
    for (int i = 0; i < SHA256_RESULT_LENGTH / sizeof(uint32_t); i++)
    {
        PRINTF("%08x", resultSha256[i]);
    }
    PRINTF("\r\n\r\n");

#if defined(DEMO_MMCAU_PASS_RATE)
    /* Call SHA256 */
    time_config(true);
    CountPass = 0U;
    CountFail = 0U;
    PassRate  = 0.0;

    cycles = CYCLES_FOR_PASSRATE;
    while (cycles)
    {
        MMCAU_SHA256_Update(g_testSha, blocks, resultSha256);
        if (status != kStatus_Success)
        {
            PRINTF("SHA256 update failed !\r\n");
            return;
        }

        if (memcmp(resultSha256, g_Sha256_expected, sizeof(g_Sha1_expected)) == 0)
        {
            CountPass++;
        }
        else
        {
            CountFail++;
        }

        PassRate = (float)(CountPass * 100.0f) / ((float)(CountPass + CountFail));
        cycles--;
    }
    time_config(false);
    PRINTF("SHA256 finished. Sucess rate %f % \r\n", PassRate);
    PRINTF("\r\n");
#endif /* DEMO_MMCAU_PASS_RATE */

    /*Compute MD5 */
    cycles     = CYCLES_FOR_THROUGHPUT;
    timeBefore = time_get_ms();
    while (cycles)
    {
        MMCAU_MD5_Update(g_testMd5, blocks, resultMd5);
        cycles--;
    }
    timeAfter = time_get_ms();
    /* NOTE: CAU lib provides resultMd5[] in most significant byte first order */
    PRINTF("Computed MD5 at speed %f MB/s:\r\n",
           mmcau_get_throughput(timeAfter - timeBefore, CYCLES_FOR_THROUGHPUT * length));
    for (int i = 0; i < MD5_RESULT_LENGTH / sizeof(uint32_t); i++)
    {
        uint8_t *pByte = (uint8_t *)&resultMd5[i];
        for (int j = 0; j < 4; j++)
        {
            PRINTF("%02x", pByte[j]);
        }
    }

#if defined(DEMO_MMCAU_PASS_RATE)
    /* Call MD5 */
    time_config(true);
    CountPass = 0U;
    CountFail = 0U;
    PassRate  = 0.0;

    cycles = CYCLES_FOR_PASSRATE;
    while (cycles)
    {
        status = MMCAU_MD5_Update(g_testMd5, blocks, resultMd5);
        if (status != kStatus_Success)
        {
            PRINTF("MD5 update failed !\r\n");
            return;
        }

        if (memcmp(resultMd5, g_testMd5Expected, sizeof(g_testMd5Expected)) == 0)
        {
            CountPass++;
        }
        else
        {
            CountFail++;
        }

        PassRate = (float)(CountPass * 100.0f) / ((float)(CountPass + CountFail));
        cycles--;
    }
    time_config(false);
    PRINTF("MD5 finished. Sucess rate %f % \r\n", PassRate);
    PRINTF("\r\n");
#endif /* DEMO_MMCAU_PASS_RATE */

    /* Format console output */
    PRINTF("\r\n\r\n");

    /* Goodbye message */
    PRINTF(".............. THE  END  OF  THE  MMCAU  DRIVER  EXAMPLE ................................\r\n");
}

/*!
 * @brief MMCAU example.
 */
int main(void)
{
    /* Init hardware*/
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

#if defined(DEMO_MMCAU_PASS_RATE)
    RNGA_Init(RNG);
    BOARD_RandomInit();
#endif

    /* Init time measurement. SysTick method deployed. */
    if (time_config(false))
    {
        PRINTF("ERROR in SysTick Configuration\r\n");
    }
    else
    {
        /* Call example task */
        mmcau_example_task();
    }

    while (1)
    {
    }
}
