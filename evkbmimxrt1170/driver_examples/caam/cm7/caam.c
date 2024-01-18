/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2022 NXP
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
#include "fsl_caam.h"

#include <string.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Define PRINT_BLOBS_CONTENT to print blobs content*/
//#define PRINT_BLOBS_CONTENT


/*******************************************************************************
 * Variables
 ******************************************************************************/

#if defined(FSL_FEATURE_HAS_L1CACHE) || defined(__DCACHE_PRESENT)
/* Note: If JR interface is cached and multiple jobs are being scheduled at the same time, */
/* it is recommended to move JR interfaces to non-cahed memory, rather than invalidate */
/* them before retrieving output ring data  */
/*! @brief CAAM job ring interface 0 in system memory. */
AT_NONCACHEABLE_SECTION(static caam_job_ring_interface_t s_jrif0);
/*! @brief CAAM job ring interface 1 in system memory. */
AT_NONCACHEABLE_SECTION(static caam_job_ring_interface_t s_jrif1);
/*! @brief CAAM job ring interface 2 in system memory. */
AT_NONCACHEABLE_SECTION(static caam_job_ring_interface_t s_jrif2);
/*! @brief CAAM job ring interface 3 in system memory. */
AT_NONCACHEABLE_SECTION(static caam_job_ring_interface_t s_jrif3);
#else
/*! @brief CAAM job ring interface 0 in system memory. */
static caam_job_ring_interface_t s_jrif0;
/*! @brief CAAM job ring interface 1 in system memory. */
static caam_job_ring_interface_t s_jrif1;
/*! @brief CAAM job ring interface 2 in system memory. */
static caam_job_ring_interface_t s_jrif2;
/*! @brief CAAM job ring interface 3 in system memory. */
static caam_job_ring_interface_t s_jrif3;
#endif /* __DCACHE_PRESENT || FSL_FEATURE_HAS_L1CACHE */

/*! @brief 16 bytes key for CBC method: "ultrapassword123". */
static uint8_t s_CbcKey128[] = {0x75, 0x6c, 0x74, 0x72, 0x61, 0x70, 0x61, 0x73,
                                0x73, 0x77, 0x6f, 0x72, 0x64, 0x31, 0x32, 0x33};

/*! @brief 24 bytes key for CBC method: "UltraMegaSecretPassword1". */
static uint8_t s_CbcKey192[] = {0x55, 0x6c, 0x74, 0x72, 0x61, 0x4d, 0x65, 0x67, 0x61, 0x53, 0x65, 0x63,
                                0x72, 0x65, 0x74, 0x50, 0x61, 0x73, 0x73, 0x77, 0x6f, 0x72, 0x64, 0x31};

/*! @brief 32 bytes key for CBC method: "Thispasswordisveryuncommonforher". */
static uint8_t s_CbcKey256[] = {0x54, 0x68, 0x69, 0x73, 0x70, 0x61, 0x73, 0x73, 0x77, 0x6f, 0x72,
                                0x64, 0x69, 0x73, 0x76, 0x65, 0x72, 0x79, 0x75, 0x6e, 0x63, 0x6f,
                                0x6d, 0x6d, 0x6f, 0x6e, 0x66, 0x6f, 0x72, 0x68, 0x65, 0x72};

/*!
 * @brief Plaintext for CBC method.
 * 16-byte multiple, last '\0' is not used.
 */
static uint8_t s_CbcPlain[] =
    "Be that word our sign of parting, bird or fiend! I shrieked upstarting"
    "Get thee back into the tempest and the Nights Plutonian shore!"
    "Leave no black plume as a token of that lie thy soul hath spoken!"
    "Leave my loneliness unbroken! quit the bust above my door!"
    "Take thy beak from out my heart, and take thy form from off my door!"
    "Quoth the raven, Nevermore.  ";

/*! @brief Decrypted plaintext from CBC method goes here. */
static uint8_t s_CbcPlainDecrypted[sizeof(s_CbcPlain) - 1U];

/*! @brief Encrypted ciphertext from CBC method goes here. */
static uint8_t s_CbcCipher[sizeof(s_CbcPlain) - 1U];

/*! @brief Expected ciphertext from CBC method using s_CbcKey128 key. */
static const uint8_t s_Cbc128CipherExpected[] = {
    0xeb, 0x69, 0xb5, 0xae, 0x7a, 0xbb, 0xb8, 0xee, 0x4d, 0xe5, 0x28, 0x97, 0xca, 0xab, 0x60, 0x65, 0x63, 0xf9, 0xe8,
    0x4c, 0x7f, 0xda, 0x0a, 0x02, 0x3a, 0x93, 0x16, 0x0d, 0x64, 0x56, 0x5a, 0x86, 0xf2, 0xe8, 0x5b, 0x38, 0x1d, 0x31,
    0xd7, 0x65, 0x7e, 0x8f, 0x8d, 0x53, 0xc5, 0xa6, 0x0c, 0x5d, 0xc5, 0x43, 0x98, 0x3b, 0x49, 0x3a, 0xce, 0x7d, 0xf9,
    0xb5, 0xf7, 0x95, 0x47, 0x89, 0xaf, 0xd8, 0x2f, 0xbd, 0xa4, 0xd8, 0x7f, 0xb9, 0x13, 0x3a, 0xcd, 0x17, 0xc8, 0xc4,
    0xb0, 0x5d, 0xe8, 0xf5, 0x19, 0x39, 0x6a, 0x14, 0x1b, 0x1b, 0x78, 0x5e, 0xe0, 0xd6, 0x67, 0x9a, 0x36, 0x17, 0x9c,
    0x7a, 0x88, 0x26, 0xfd, 0x8f, 0x3d, 0x82, 0xc9, 0xb1, 0x2a, 0x9c, 0xc0, 0xdd, 0xdb, 0x78, 0x61, 0x3b, 0x22, 0x5d,
    0x48, 0x3c, 0xab, 0x10, 0xd3, 0x5d, 0x0d, 0xa1, 0x25, 0x3e, 0x4d, 0xd6, 0x8e, 0xc4, 0x1b, 0x68, 0xbb, 0xa4, 0x2d,
    0x97, 0x2b, 0xd6, 0x23, 0xa0, 0xf2, 0x90, 0x8e, 0x07, 0x75, 0x44, 0xb3, 0xe2, 0x5a, 0x35, 0x38, 0x4c, 0x5d, 0x35,
    0xa9, 0x7c, 0xa3, 0xb6, 0x38, 0xe7, 0xf5, 0x20, 0xdc, 0x0e, 0x6c, 0x7c, 0x4b, 0x4f, 0x93, 0xc1, 0x81, 0x69, 0x02,
    0xb7, 0x66, 0x37, 0x24, 0x0d, 0xb8, 0x9a, 0xa8, 0xd4, 0x42, 0x75, 0x28, 0xe8, 0x33, 0x89, 0x1e, 0x60, 0x82, 0xe9,
    0xf6, 0x45, 0x72, 0x64, 0x65, 0xd2, 0xcd, 0x19, 0xd9, 0x5e, 0xa2, 0x59, 0x31, 0x82, 0x53, 0x20, 0x35, 0x13, 0x76,
    0x7f, 0xeb, 0xc3, 0xbe, 0xfa, 0x4a, 0x10, 0x83, 0x81, 0x0f, 0x24, 0x6d, 0xca, 0x53, 0x07, 0xb9, 0xe0, 0xb9, 0x5d,
    0x91, 0x2d, 0x90, 0x86, 0x5b, 0x9d, 0xaa, 0xcd, 0x28, 0xea, 0x11, 0xfb, 0x83, 0x39, 0x9c, 0xf5, 0x3b, 0xd9, 0xef,
    0x38, 0xc7, 0xa4, 0xad, 0x47, 0xf2, 0x2d, 0xd6, 0x6b, 0x26, 0x28, 0x59, 0xaa, 0x33, 0x01, 0x73, 0xc9, 0x46, 0x97,
    0xa3, 0xe5, 0x11, 0x71, 0x66, 0xef, 0x1f, 0x0b, 0xbc, 0xe7, 0x4f, 0x8c, 0x79, 0xe2, 0x39, 0x14, 0x85, 0xcd, 0xa9,
    0x59, 0xed, 0x78, 0x9d, 0x37, 0xf5, 0x46, 0xfc, 0xa9, 0x8a, 0x16, 0x0a, 0x76, 0x58, 0x6d, 0x59, 0x9e, 0x65, 0xbe,
    0x1b, 0xc2, 0x09, 0xa1, 0xf9, 0x40, 0xab, 0xdb, 0x2e, 0x11, 0x30, 0x29, 0x49, 0x75, 0xf7, 0x74, 0xe1, 0xf3, 0x78,
    0x97, 0x69, 0x2c, 0x6a, 0x0e, 0x0d, 0xbd, 0x72, 0x3d, 0x75, 0xd6, 0x0a, 0x8c, 0xc2, 0x92, 0xd9, 0xb6, 0x46, 0x91,
    0xa7, 0xe4, 0x74, 0x71, 0xf5, 0xb4, 0x21, 0x86, 0x18, 0xa8};

/*! @brief Expected ciphertext from CBC method using s_CbcKey192 key. */
static const uint8_t s_Cbc192CipherExpected[] = {
    0xb5, 0xb8, 0xe5, 0x87, 0x40, 0x71, 0xdf, 0x48, 0x17, 0xf1, 0xe0, 0xa4, 0x92, 0xf1, 0xcf, 0x78, 0xb4, 0xb3, 0x92,
    0x42, 0xd6, 0x3b, 0x23, 0x3c, 0xa7, 0x82, 0xcc, 0x6a, 0xa4, 0xf5, 0x52, 0x8e, 0xdf, 0x02, 0x14, 0x2d, 0x1d, 0xae,
    0x3e, 0x86, 0x87, 0x41, 0x8d, 0xe9, 0x5b, 0x12, 0x38, 0x24, 0x7e, 0x46, 0xa7, 0xb1, 0x5f, 0x8a, 0x8f, 0x69, 0xdc,
    0x56, 0x8f, 0x37, 0x80, 0x53, 0xff, 0x67, 0x67, 0x54, 0xa7, 0x79, 0x2b, 0x7b, 0x66, 0x21, 0x78, 0x80, 0x34, 0x02,
    0x18, 0xd7, 0xc0, 0xef, 0x05, 0xdb, 0x25, 0x4d, 0x42, 0x05, 0xbb, 0x69, 0x35, 0x63, 0xc1, 0x31, 0xe3, 0x47, 0xc2,
    0xde, 0x67, 0xfe, 0x9f, 0x60, 0xf6, 0x6c, 0xb5, 0x41, 0x5e, 0x25, 0xa6, 0xec, 0xfe, 0xb0, 0x3e, 0x87, 0x61, 0x8e,
    0x5c, 0x03, 0x8e, 0x8b, 0x20, 0x74, 0xcd, 0x49, 0xa8, 0x04, 0xb0, 0xca, 0x10, 0xaa, 0x27, 0x5d, 0xe7, 0xfe, 0x90,
    0x3e, 0x50, 0xe4, 0x3e, 0x94, 0x68, 0xd1, 0xcc, 0x54, 0x28, 0xba, 0x2d, 0x2a, 0x88, 0x0d, 0xfa, 0xb2, 0x0a, 0x15,
    0x8d, 0x0a, 0xdc, 0xbc, 0x16, 0xd8, 0xaf, 0x1d, 0xce, 0x9a, 0xfa, 0x90, 0x96, 0x62, 0xbd, 0x11, 0x62, 0x09, 0x80,
    0xfe, 0xbd, 0x6d, 0xca, 0xbc, 0x6a, 0x07, 0xf9, 0x5e, 0x63, 0xe2, 0x6d, 0xfe, 0x7d, 0x88, 0xa2, 0xb6, 0x8e, 0xaf,
    0x1a, 0x80, 0x62, 0x19, 0x4c, 0x68, 0xfc, 0x61, 0x18, 0x58, 0x33, 0x76, 0x20, 0x84, 0x5d, 0xd6, 0x49, 0x97, 0xb7,
    0x79, 0x83, 0xf0, 0x69, 0x2f, 0xce, 0x73, 0x86, 0x5a, 0x6f, 0xfa, 0x96, 0x66, 0x97, 0xf3, 0xa0, 0xb3, 0xed, 0x67,
    0x36, 0x64, 0x08, 0x28, 0x75, 0xb5, 0x58, 0x19, 0x85, 0x01, 0x28, 0x3e, 0xb1, 0x8e, 0x68, 0x4e, 0x9f, 0x95, 0x86,
    0xae, 0xe0, 0x6e, 0x60, 0xbe, 0xa0, 0xfc, 0x5e, 0x8b, 0x5e, 0xe8, 0x96, 0xe9, 0xfa, 0xcb, 0x3d, 0xce, 0x9d, 0x70,
    0xbe, 0xa2, 0x05, 0x52, 0xbb, 0xa2, 0x79, 0xc9, 0xac, 0xf5, 0x91, 0xa2, 0xe4, 0xda, 0xa4, 0x5f, 0x89, 0x75, 0x45,
    0x7b, 0x58, 0xe3, 0xdb, 0x0f, 0xef, 0xd6, 0xa7, 0x88, 0x9c, 0x0d, 0xf3, 0x5b, 0x49, 0xb1, 0x27, 0xe3, 0x81, 0x92,
    0x93, 0x91, 0xaf, 0x27, 0x6b, 0x5a, 0x2e, 0x1a, 0x0c, 0xb6, 0xc5, 0x50, 0xc4, 0xb3, 0xf8, 0xfd, 0x0a, 0xff, 0xc7,
    0x8c, 0x55, 0xde, 0xde, 0x6f, 0x7c, 0xb9, 0xaa, 0x8d, 0x18, 0x17, 0xc5, 0x55, 0x95, 0x59, 0xd0, 0x00, 0x53, 0x63,
    0xaf, 0xe9, 0xf9, 0xde, 0x93, 0xe2, 0xa6, 0x90, 0xe5, 0xa9};

/*! @brief Expected ciphertext from CBC method using s_CbcKey256 key. */
static const uint8_t s_Cbc256CipherExpected[] = {
    0x09, 0x9b, 0xf5, 0xb3, 0xaf, 0x11, 0xa9, 0xd1, 0xa1, 0x81, 0x78, 0x6c, 0x6e, 0x74, 0xf3, 0xb8, 0x70, 0xee, 0x31,
    0x4d, 0x6d, 0x54, 0xab, 0x37, 0xcb, 0xeb, 0x58, 0x6f, 0x09, 0x5f, 0x72, 0xc4, 0x5a, 0xd0, 0x56, 0xc8, 0x3d, 0x93,
    0x45, 0xe2, 0x7e, 0x97, 0xaa, 0xc3, 0xc9, 0xf5, 0xde, 0x74, 0x73, 0x45, 0x35, 0xea, 0x1f, 0x5e, 0x81, 0xbf, 0x9d,
    0xb5, 0xc9, 0x77, 0x77, 0x1c, 0x00, 0xde, 0x67, 0x34, 0xff, 0x62, 0x48, 0x89, 0xd9, 0xbe, 0x92, 0xd4, 0x7e, 0xaf,
    0x9d, 0x8a, 0x65, 0x14, 0x1f, 0x62, 0xaa, 0x0a, 0xe4, 0x37, 0x8e, 0x18, 0x3c, 0x75, 0x5e, 0x38, 0x6a, 0xa9, 0x5d,
    0x26, 0x54, 0x0a, 0xd8, 0xeb, 0x7a, 0x25, 0xa6, 0xd4, 0x18, 0x13, 0x1f, 0x30, 0xfc, 0x37, 0x09, 0x77, 0x90, 0x26,
    0x88, 0x0e, 0x53, 0x67, 0xba, 0xe2, 0xfa, 0x38, 0xb9, 0x74, 0xa9, 0x5b, 0xda, 0x6a, 0xe0, 0xb3, 0x39, 0xed, 0x07,
    0xae, 0xe6, 0x86, 0x44, 0x2d, 0xf2, 0xd8, 0x1f, 0x86, 0x2c, 0xac, 0x01, 0x4c, 0x9b, 0xce, 0x65, 0x6a, 0x8a, 0x3a,
    0xf0, 0xf9, 0xfd, 0x15, 0x65, 0xb6, 0xaf, 0xdc, 0x90, 0xc5, 0x47, 0x96, 0x28, 0xb0, 0x1c, 0x56, 0x2e, 0xc6, 0xdd,
    0x4e, 0x71, 0xd3, 0x73, 0xf5, 0x7c, 0xa6, 0x66, 0x8b, 0x44, 0xaf, 0x53, 0x61, 0x16, 0xe3, 0x41, 0x94, 0xe7, 0x6d,
    0x3d, 0xdb, 0xe1, 0x92, 0x52, 0x39, 0x05, 0x97, 0xf4, 0x41, 0xc8, 0xbe, 0x54, 0xec, 0x9a, 0x52, 0xf1, 0x79, 0x0c,
    0x71, 0x05, 0x14, 0xc8, 0x16, 0x86, 0xdb, 0xa3, 0x8e, 0x1c, 0x41, 0x5b, 0x7a, 0x3b, 0x77, 0xa9, 0x27, 0x7a, 0xde,
    0xcd, 0xaa, 0x86, 0x2e, 0x52, 0x87, 0x54, 0x1c, 0x88, 0x4c, 0xdb, 0x3e, 0xab, 0x48, 0xaa, 0x51, 0x5a, 0xcd, 0xb0,
    0xe7, 0x68, 0x91, 0x33, 0x9e, 0xfd, 0x07, 0x9d, 0xdf, 0x18, 0x51, 0xa5, 0xc0, 0xa6, 0x68, 0xbc, 0xd2, 0x6b, 0x1f,
    0x03, 0xfc, 0xf3, 0x71, 0xed, 0x5b, 0x28, 0x35, 0xa8, 0x56, 0x93, 0x4c, 0xdc, 0x1f, 0xa1, 0x88, 0xe8, 0xbe, 0x08,
    0x48, 0xe8, 0x28, 0x1d, 0x16, 0xb4, 0x1e, 0xeb, 0xca, 0xdd, 0x43, 0x18, 0xfe, 0x49, 0x24, 0xfd, 0x23, 0x83, 0x44,
    0x2b, 0xc3, 0x33, 0x80, 0x62, 0xb9, 0xa6, 0xb8, 0x48, 0x1e, 0x72, 0x52, 0xef, 0xee, 0x56, 0xd8, 0x05, 0x08, 0xad,
    0xc2, 0xe9, 0xb7, 0x46, 0x12, 0xbc, 0xc8, 0x7d, 0xe2, 0x87, 0x9d, 0x57, 0xf7, 0x6f, 0x10, 0x6e, 0x8c, 0x32, 0x8e,
    0x2f, 0x78, 0x20, 0xf0, 0x23, 0x29, 0x54, 0xef, 0x66, 0x8d};

/*! @brief Initialization vector for CBC method: 16 bytes: "mysecretpassword". */
static uint8_t s_CbcIv[CAAM_AES_BLOCK_SIZE] = {0x6d, 0x79, 0x73, 0x65, 0x63, 0x72, 0x65, 0x74,
                                               0x70, 0x61, 0x73, 0x73, 0x77, 0x6f, 0x72, 0x64};

/*! @brief 16 bytes key for GCM method. */
static uint8_t s_GcmKey[16] = {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
                               0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08};

/*! @brief Plaintext for GCM method. */
static uint8_t s_GcmPlain[] = {0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5, 0xa5, 0x59, 0x09, 0xc5,
                               0xaf, 0xf5, 0x26, 0x9a, 0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
                               0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72, 0x1c, 0x3c, 0x0c, 0x95,
                               0x95, 0x68, 0x09, 0x53, 0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
                               0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57, 0xba, 0x63, 0x7b, 0x39};

/*! @brief Decrypted plaintext from GCM method goes here. */
static uint8_t s_GcmPlainDecrypted[sizeof(s_GcmPlain)];

/*! @brief Expected ciphertext from GCM method. */
static const uint8_t s_GcmCipherExpected[] = {0x42, 0x83, 0x1e, 0xc2, 0x21, 0x77, 0x74, 0x24, 0x4b, 0x72, 0x21, 0xb7,
                                              0x84, 0xd0, 0xd4, 0x9c, 0xe3, 0xaa, 0x21, 0x2f, 0x2c, 0x02, 0xa4, 0xe0,
                                              0x35, 0xc1, 0x7e, 0x23, 0x29, 0xac, 0xa1, 0x2e, 0x21, 0xd5, 0x14, 0xb2,
                                              0x54, 0x66, 0x93, 0x1c, 0x7d, 0x8f, 0x6a, 0x5a, 0xac, 0x84, 0xaa, 0x05,
                                              0x1b, 0xa3, 0x0b, 0x39, 0x6a, 0x0a, 0xac, 0x97, 0x3d, 0x58, 0xe0, 0x91};

/*! @brief Encrypted ciphertext from GCM method goes here. */
static uint8_t s_GcmCipher[sizeof(s_GcmCipherExpected)];

/*! @brief Initialization vector for GCM method. */
static uint8_t s_GcmIv[12] = {0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88};

/*! @brief Additional authenticated data for GCM method. */
static uint8_t s_GcmAad[] = {0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed,
                             0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef, 0xab, 0xad, 0xda, 0xd2};

/*! @brief Expected tag from GCM method. */
static const uint8_t s_GcmTagExpected[] = {0x5b, 0xc9, 0x4f, 0xbc, 0x32, 0x21, 0xa5, 0xdb,
                                           0x94, 0xfa, 0xe9, 0x5a, 0xe7, 0x12, 0x1a, 0x47};

/*! @brief Encrypted tag from GCM method goes here. */
static uint8_t s_GcmTag[sizeof(s_GcmTagExpected)];

/*! @brief 128 byte key for HMAC consisting of a 32-byte repeating pattern. */
static uint8_t s_HmacKey[] = {0xa3, 0x02, 0x5b, 0xd2, 0x2a, 0x7a, 0x06, 0x1d,
                              0x4c, 0xf9, 0x0f, 0xde, 0xf6, 0x43, 0x25, 0x0f,
                              0x6a, 0xe7, 0x0a, 0xf8, 0xc0, 0x37, 0x27, 0x68,
                              0xc5, 0x80, 0xde, 0xd0, 0x79, 0x35, 0x93, 0x27,
                              0xa3, 0x02, 0x5b, 0xd2, 0x2a, 0x7a, 0x06, 0x1d,
                              0x4c, 0xf9, 0x0f, 0xde, 0xf6, 0x43, 0x25, 0x0f,
                              0x6a, 0xe7, 0x0a, 0xf8, 0xc0, 0x37, 0x27, 0x68,
                              0xc5, 0x80, 0xde, 0xd0, 0x79, 0x35, 0x93, 0x27,
                              0xa3, 0x02, 0x5b, 0xd2, 0x2a, 0x7a, 0x06, 0x1d,
                              0x4c, 0xf9, 0x0f, 0xde, 0xf6, 0x43, 0x25, 0x0f,
                              0x6a, 0xe7, 0x0a, 0xf8, 0xc0, 0x37, 0x27, 0x68,
                              0xc5, 0x80, 0xde, 0xd0, 0x79, 0x35, 0x93, 0x27,
                              0xa3, 0x02, 0x5b, 0xd2, 0x2a, 0x7a, 0x06, 0x1d,
                              0x4c, 0xf9, 0x0f, 0xde, 0xf6, 0x43, 0x25, 0x0f,
                              0x6a, 0xe7, 0x0a, 0xf8, 0xc0, 0x37, 0x27, 0x68,
                              0xc5, 0x80, 0xde, 0xd0, 0x79, 0x35, 0x93, 0x27};

/*! @brief Plaintext for HMAC. */
static uint8_t s_HmacPlain[] =
    "Be that word our sign of parting, bird or fiend! I shrieked upstarting"
    "Get thee back into the tempest and the Nights Plutonian shore!"
    "Leave no black plume as a token of that lie thy soul hath spoken!"
    "Leave my loneliness unbroken! quit the bust above my door!"
    "Take thy beak from out my heart, and take thy form from off my door!"
    "Quoth the raven, Nevermore.  ";

/*! @brief Expected SHA1 HMAC. */
static uint8_t s_HmacSha1Expected[] = {0xac, 0xd1, 0x4f, 0x92, 0x60, 0x15, 0xd1,
                                       0x80, 0xd6, 0x4a, 0x6a, 0x99, 0xb6, 0x6e,
                                       0xfe, 0xac, 0xc5, 0xa0, 0x4c, 0x13};

/*! @brief Expected SHA224 HMAC. */
static uint8_t s_HmacSha224Expected[] = {0xfc, 0xdc, 0xc6, 0x52, 0xbd, 0x5d, 0x5c, 0x23, 0xc6, 0x30, 0xd5,
                                         0x0b, 0x7c, 0x68, 0x85, 0x06, 0x17, 0x6a, 0x3c, 0x5f, 0x48, 0x46,
                                         0xdc, 0xa9, 0xb7, 0xf6, 0x79, 0xe9};

/*! @brief Expected SHA256 HMAC. */
static uint8_t s_HmacSha256Expected[] = {0x1d, 0x82, 0x8e, 0x3e, 0xc7, 0xd4, 0x61, 0x78, 0x34, 0xa3, 0x2c,
                                         0x91, 0xad, 0x39, 0x10, 0xbc, 0xe7, 0x3c, 0xb1, 0x29, 0xad, 0xcb,
                                         0x19, 0x24, 0x3d, 0xf4, 0xcb, 0x77, 0xb4, 0x88, 0x6b, 0x2a};

/*! @brief Expected SHA384 HMAC. */
static uint8_t s_HmacSha384Expected[] = {0x5a, 0xa0, 0xe2, 0xd7, 0x45, 0x8c, 0x04, 0xc0, 0x94, 0xd2, 0x68,
                                         0x0a, 0x7c, 0x20, 0x95, 0x1d, 0x09, 0x3f, 0xce, 0x2a, 0xfd, 0x6a,
                                         0x70, 0xe7, 0xab, 0xf0, 0xb7, 0x18, 0x0b, 0x30, 0x6a, 0xcf, 0x7b,
                                         0x91, 0x8e, 0x89, 0x82, 0xc0, 0xeb, 0xa0, 0xc1, 0xb1, 0x74, 0xbd,
                                         0xae, 0x34, 0xa0, 0x90};

/*! @brief Expected SHA512 HMAC. */
static uint8_t s_HmacSha512Expected[] = {0xf8, 0x15, 0xa9, 0xc8, 0x6f, 0x2f, 0x09, 0x67, 0x12, 0x91, 0x19,
                                         0x45, 0xfb, 0xee, 0xf6, 0x8d, 0xce, 0xc6, 0x9e, 0x0e, 0x5a, 0x46,
                                         0xb6, 0xd9, 0xca, 0x9f, 0x3d, 0x5e, 0x32, 0x74, 0xc9, 0x2d, 0x28,
                                         0x83, 0xe1, 0x75, 0x4a, 0x07, 0x63, 0x66, 0xed, 0x76, 0x38, 0x05,
                                         0x23, 0x49, 0x27, 0x28, 0x18, 0xe0, 0xba, 0x05, 0x52, 0x4d, 0xdd,
                                         0x3e, 0x5a, 0x95, 0xfa, 0x64, 0xe7, 0xec, 0xe3, 0x4c};

/*! @brief Plaintext for SHA. */
static uint8_t s_ShaPlain[] =
    "Be that word our sign of parting, bird or fiend! I shrieked upstarting"
    "Get thee back into the tempest and the Nights Plutonian shore!"
    "Leave no black plume as a token of that lie thy soul hath spoken!"
    "Leave my loneliness unbroken! quit the bust above my door!"
    "Take thy beak from out my heart, and take thy form from off my door!"
    "Quoth the raven, Nevermore.  ";

/*! @brief Expected SHA-256 for the message s_ShaPlain. */
static const unsigned char s_ShaExpected[] = {0x63, 0x76, 0xea, 0xcc, 0xc9, 0xa2, 0xc0, 0x43, 0xf4, 0xfb, 0x01,
                                              0x34, 0x69, 0xb3, 0x0c, 0xf5, 0x28, 0x63, 0x5c, 0xfa, 0xa5, 0x65,
                                              0x60, 0xef, 0x59, 0x7b, 0xd9, 0x1c, 0xac, 0xaa, 0x31, 0xf7};

/*! @brief Output buffer for SHA. */
uint8_t sha_output[32U];

/* Blob examples variable */
uint8_t s_KeyGenerate[32]; // Buffer for generated key from RNG
uint8_t s_KeyBlacken[32];  // Buffer for Blackened key, used with AES-ECB is 32bytes, in CCM mode should be 44bytes long

uint8_t s_blob_Oridata[32]; // Buffer for data which is encrypted in blob
uint8_t s_KeyModifier[16];  //for normal memory key modifier is 128-bits long -> 16 bytes , for secure memory is 64-bits long -> 8 bytes
uint8_t s_blob_data[80];    // 32byte blob key + 32 bytes of data + 16bytes of MAC for this example = 80bytes in total
uint8_t s_blob_DecapData[32]; // 32bytes in AES-ECB mode, in CCM mode 44bytes

/*! @brief Plaintext for CRC. */
static uint8_t s_CrcPlain[] =
    "Be that word our sign of parting, bird or fiend! I shrieked upstarting"
    "Get thee back into the tempest and the Nights Plutonian shore!"
    "Leave no black plume as a token of that lie thy soul hath spoken!"
    "Leave my loneliness unbroken! quit the bust above my door!"
    "Take thy beak from out my heart, and take thy form from off my door!"
    "Quoth the raven, Nevermore.  ";

/*! @brief Expected CRC for the message s_CrcPlain. */
static const unsigned char s_CrciSCSIExpected[] = {0x63, 0xF1, 0x73, 0x60}; /*CRC-32C Poly: 0x1EDC6F41 Init: 0xFFFFFFFF RefIn: true Refout:  true XORout: 0xFFFFFFFF*/

static uint8_t s_Crc16CCITZeroPoly[2] = {0x10, 0x21}; /* Polynom used for CRC16 CCIT ZERO */
static const unsigned char s_Crc16CCITZeroExpected[] = {0x4D, 0xC9}; /*CRC-16C Poly: 0x1021 Init: 0x0000 RefIn: false Refout:  false XORout: 0x0000*/

/*! @brief Output buffer for CRC. */
uint8_t AT_NONCACHEABLE_SECTION(crc_output[4U]);
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void RunAesCbcExamples(CAAM_Type *base, caam_handle_t *handle);

static void EncryptDecryptCbc(
    CAAM_Type *base, caam_handle_t *handle, const uint8_t *key, size_t keySize, const uint8_t *cipherExpected);

static void RunAesGcmExamples(CAAM_Type *base, caam_handle_t *handle);

static void EncryptDecryptGcm(CAAM_Type *base, caam_handle_t *handle);

static void HmacSha(CAAM_Type *base, caam_handle_t *handle, const uint8_t *key,
                    size_t keySize, uint32_t shaBitlen, caam_hash_algo_t algo,
                    const uint8_t *hmacExpected);

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Executes examples for AES encryption and decryption in CBC mode.
 */
static void RunAesCbcExamples(CAAM_Type *base, caam_handle_t *handle)
{
    EncryptDecryptCbc(base, handle, s_CbcKey128, sizeof(s_CbcKey128), s_Cbc128CipherExpected);
    EncryptDecryptCbc(base, handle, s_CbcKey192, sizeof(s_CbcKey192), s_Cbc192CipherExpected);
    EncryptDecryptCbc(base, handle, s_CbcKey256, sizeof(s_CbcKey256), s_Cbc256CipherExpected);
}

/*!
 * @brief Encrypts and decrypts in AES CBC mode.
 *
 * @param key encryption key
 * @param keySize size of key in bytes
 * @param cipherExpected expected output of encryption
 */
static void EncryptDecryptCbc(
    CAAM_Type *base, caam_handle_t *handle, const uint8_t *key, size_t keySize, const uint8_t *cipherExpected)
{
    status_t status;

    PRINTF("AES CBC: encrypting using %u bit key ", 8U * keySize);

    status = CAAM_AES_EncryptCbc(base, handle, s_CbcPlain, s_CbcCipher, sizeof(s_CbcCipher), s_CbcIv, key, keySize);
    if (status != kStatus_Success)
    {
        PRINTF("- failed to encrypt!\r\n\r\n");
        return;
    }

    if (memcmp(s_CbcCipher, cipherExpected, sizeof(s_CbcCipher)) == 0)
    {
        PRINTF("done successfully.\r\n");
    }
    else
    {
        PRINTF("- encrypted text mismatch!\r\n\r\n");
        return;
    }

    PRINTF("AES CBC: decrypting back ");

    status =
        CAAM_AES_DecryptCbc(base, handle, s_CbcCipher, s_CbcPlainDecrypted, sizeof(s_CbcCipher), s_CbcIv, key, keySize);
    if (status != kStatus_Success)
    {
        PRINTF("- failed to decrypt!\r\n\r\n");
        return;
    }

    if (memcmp(s_CbcPlainDecrypted, s_CbcPlain, sizeof(s_CbcPlainDecrypted)) == 0)
    {
        PRINTF("done successfully.\r\n\r\n");
    }
    else
    {
        PRINTF("- decrypted text mismatch!\r\n\r\n");
    }
}

/*!
 * @brief Executes examples for AES encryption and decryption in GCM mode.
 */
static void RunAesGcmExamples(CAAM_Type *base, caam_handle_t *handle)
{
    EncryptDecryptGcm(base, handle);
}

/*!
 * @brief Encrypts and decrypts in AES GCM mode.
 */
static void EncryptDecryptGcm(CAAM_Type *base, caam_handle_t *handle)
{
    status_t status;

    PRINTF("AES GCM: encrypt ");

    status = CAAM_AES_EncryptTagGcm(base, handle, s_GcmPlain, s_GcmCipher, sizeof(s_GcmPlain), s_GcmIv, sizeof(s_GcmIv),
                                    s_GcmAad, sizeof(s_GcmAad), s_GcmKey, sizeof(s_GcmKey), s_GcmTag, sizeof(s_GcmTag));
    if (status != kStatus_Success)
    {
        PRINTF("- failed to encrypt!\r\n\r\n");
        return;
    }

    if (memcmp(s_GcmTag, s_GcmTagExpected, sizeof(s_GcmTagExpected)) != 0)
    {
        PRINTF("- tag mismatch!\r\n\r\n");
        return;
    }

    if (memcmp(s_GcmCipher, s_GcmCipherExpected, sizeof(s_GcmCipherExpected)) != 0)
    {
        PRINTF("- encrypted text mismatch!\r\n\r\n");
        return;
    }

    PRINTF("done successfully.\r\n");

    PRINTF("AES GCM: decrypt ");

    status = CAAM_AES_DecryptTagGcm(base, handle, s_GcmCipher, s_GcmPlainDecrypted, sizeof(s_GcmCipher), s_GcmIv,
                                    sizeof(s_GcmIv), s_GcmAad, sizeof(s_GcmAad), s_GcmKey, sizeof(s_GcmKey), s_GcmTag,
                                    sizeof(s_GcmTag));
    if (status != kStatus_Success)
    {
        PRINTF("- failed to decrypt!\r\n\r\n");
        return;
    }

    if (memcmp(s_GcmPlainDecrypted, s_GcmPlain, sizeof(s_GcmPlain)) != 0)
    {
        PRINTF("- decrypted text mismatch!\r\n\r\n");
        return;
    }

    PRINTF("done successfully.\r\n\r\n");
}

/*!
 * @brief Executes example for SHA.
 */
static void RunShaExamples(CAAM_Type *base, caam_handle_t *handle)
{
    status_t status;
    caam_hash_ctx_t ctx;

    PRINTF("SHA:");

    status = CAAM_HASH_Init(base, handle, &ctx, kCAAM_Sha256, NULL, 0u);

    if (status != kStatus_Success)
    {
        PRINTF("- failed to Hash init!\r\n\r\n");
        return;
    }

    size_t length = sizeof(s_ShaPlain) - 1u;

    status = CAAM_HASH_Update(&ctx, s_ShaPlain, length);

    if (status != kStatus_Success)
    {
        PRINTF("- failed to Hash update!\r\n\r\n");
        return;
    }

    status = CAAM_HASH_Finish(&ctx, sha_output, NULL);

    if (status != kStatus_Success)
    {
        PRINTF("- failed to Hash finish!\r\n\r\n");
        return;
    }

    if (memcmp(sha_output, s_ShaExpected, 32u) != 0)
    {
        PRINTF("- failed: unexpected Hash output!\r\n\r\n");
        return;
    }

    PRINTF("done successfully.\r\n\r\n");
}

/*!
 * @brief Executes examples for SHA-HMAC.
 */
static void RunHmacExamples(CAAM_Type *base, caam_handle_t *handle)
{
    const size_t keySize64  = 64u;
    const size_t keySize128 = 128u;

    // Do one with an oversized key
    HmacSha(base, handle, s_HmacKey, keySize128, 1, kCAAM_HmacSha1, s_HmacSha1Expected);

    // Some with block-sized keys
    HmacSha(base, handle, s_HmacKey, keySize64, 224, kCAAM_HmacSha224, s_HmacSha224Expected);
    HmacSha(base, handle, s_HmacKey, keySize64, 256, kCAAM_HmacSha256, s_HmacSha256Expected);

    // Some with under-sized keys
    HmacSha(base, handle, s_HmacKey, keySize64, 384, kCAAM_HmacSha384, s_HmacSha384Expected);
    HmacSha(base, handle, s_HmacKey, keySize64, 512, kCAAM_HmacSha512, s_HmacSha512Expected);
}

/*!
 * @brief Run an HMAC SHA variant.
 */
static void HmacSha(CAAM_Type *base, caam_handle_t *handle, const uint8_t *key,
                    size_t keySize, uint32_t shaVariant, caam_hash_algo_t algo,
                    const uint8_t *hmacExpected)
{
    caam_hash_ctx_t ctx;
    size_t outputSize;
    status_t status         = kStatus_Fail;
    uint8_t hmacOutput[64]  = {0};
    size_t plainLength      = sizeof(s_HmacPlain) - 1u;

    PRINTF("HMAC-SHA%u:", shaVariant);

    status = CAAM_HMAC_Init(base, handle, &ctx, algo, key, keySize);
    if (status != kStatus_Success)
    {
        PRINTF("- failed to HMAC-SHA%u init!\r\n\r\n", shaVariant);
        return;
    }

    status = CAAM_HMAC(base, handle, algo,
                       s_HmacPlain, plainLength,
                       key, keySize,
                       hmacOutput, &outputSize);

    if (status != kStatus_Success)
    {
        PRINTF("- failed to get HMAC-SHA%u!\r\n\r\n", shaVariant);
        return;
    }

    if (memcmp(hmacOutput, hmacExpected, outputSize) != 0)
    {
        PRINTF("- failed: unexpected HMAC-SHA%u output!\r\n\r\n", shaVariant);
        return;
    }

    PRINTF("done successfully.\r\n\r\n");
}

/*!
 * @brief Executes example for RNG.
 */
static void RunRngExample(CAAM_Type *base, caam_handle_t *handle)
{
#define RNG_EXAMPLE_RANDOM_NUMBERS     (4U)
#define RNG_EXAMPLE_RANDOM_BYTES       (16U)
#define RNG_EXAMPLE_RANDOM_NUMBER_BITS (RNG_EXAMPLE_RANDOM_NUMBERS * 8U * sizeof(uint32_t))

    status_t status = kStatus_Fail;
    uint32_t number;
    uint32_t data[RNG_EXAMPLE_RANDOM_NUMBERS] = {0};

    PRINTF("RNG : ");

    PRINTF("Generate %u-bit random number: ", RNG_EXAMPLE_RANDOM_NUMBER_BITS);
    status = CAAM_RNG_GetRandomData(base, handle, kCAAM_RngStateHandle0, (uint8_t *)data, RNG_EXAMPLE_RANDOM_BYTES,
                                    kCAAM_RngDataAny, NULL);

    if (status != kStatus_Success)
    {
        PRINTF("- failed to get random data !\r\n\r\n");
        return;
    }

    /* Print data */
    PRINTF("0x");
    for (number = 0; number < RNG_EXAMPLE_RANDOM_NUMBERS; number++)
    {
        PRINTF("%08X", data[number]);
    }
    PRINTF("\r\n");

    PRINTF("RNG : Random number generated successfully.\r\n\r\n");
}

/*!
 * @brief Executes example for red KEY blob encryption and decryption.
 */
static void RedBlobExample(CAAM_Type *base, caam_handle_t *handle)
{
    status_t status;

    PRINTF("CAAM Red Blob Example\r\n\r\n");

    /* Generate key modifier */
    status = CAAM_RNG_GetRandomData(base, handle, kCAAM_RngStateHandle0, s_KeyModifier, sizeof(s_KeyModifier), kCAAM_RngDataAny, NULL);
    if (status != kStatus_Success)
    {
        PRINTF("- failed to get random blob key!\r\n\r\n");
        return;
    }

    /* Generate data */
    status = CAAM_RNG_GetRandomData(base, handle, kCAAM_RngStateHandle0, s_blob_Oridata, sizeof(s_blob_Oridata), kCAAM_RngDataAny, NULL);
    if (status != kStatus_Success)
    {
        PRINTF("- failed to get random data!\r\n\r\n");
        return;
    }
#if defined(PRINT_BLOBS_CONTENT)

    PRINTF("The original  data is: \r\n\r\n");
    for(uint32_t i=0;i<sizeof(s_blob_Oridata);i++)
    {
        PRINTF("%x ",s_blob_Oridata[i]);
    }
    PRINTF(" \r\n\r\n");
#endif /* PRINT_BLOBS_CONTENT */

    /* Encrypt a original data to blob */
    status = CAAM_RedBlob_Encapsule(base, handle, s_KeyModifier,sizeof(s_KeyModifier), s_blob_Oridata, sizeof(s_blob_Oridata), s_blob_data);

    if (status != kStatus_Success)
    {
        PRINTF("- failed to gen blob data!\r\n\r\n");
        return;
    }
#if defined(PRINT_BLOBS_CONTENT)
    PRINTF("The encrypted blob data is: \r\n\r\n");
    for(uint32_t i=0;i<sizeof(s_blob_data);i++)
    {
        PRINTF("%x ",s_blob_data[i]);
    }
    PRINTF(" \r\n\r\n");
#endif /* PRINT_BLOBS_CONTENT */

    /* Decrypt a blob and compare with original input data */
    status = CAAM_RedBlob_Decapsule(base, handle, s_KeyModifier, sizeof(s_KeyModifier), s_blob_data, s_blob_DecapData, sizeof(s_blob_DecapData));

    if (status != kStatus_Success)
    {
        PRINTF("- failed to decrypt blob data, error status is %d !\r\n\r\n", status);
        return;
    }
#if defined(PRINT_BLOBS_CONTENT)
    PRINTF("The key blob decrypted value is: \r\n\r\n");
    for(uint32_t i=0;i<sizeof(s_blob_DecapData);i++)
    {
        PRINTF("%x ",s_blob_DecapData[i]);
    }
    PRINTF(" \r\n\r\n");
#endif /* PRINT_BLOBS_CONTENT */
    if (memcmp(s_blob_DecapData, s_blob_Oridata, sizeof(s_blob_Oridata)) != 0)
    {
        PRINTF("- data mismatch!\r\n\r\n");
        return;
    }

    PRINTF("Generate Red Blob successfully.\r\n\r\n");
}

/*!
 * @brief Executes example for black KEY generation.
 */
static void GenBlackKey(CAAM_Type *base, caam_handle_t *handle)
{
    status_t status;

    memset(s_KeyGenerate, 0, sizeof(s_KeyGenerate));
    memset(s_KeyBlacken, 0, sizeof(s_KeyBlacken));

    PRINTF("Generate AES Key and blacken it.\r\n\r\n");
    status = CAAM_RNG_GetRandomData(base, handle, kCAAM_RngStateHandle0, s_KeyGenerate, sizeof(s_KeyGenerate), kCAAM_RngDataAny, NULL);

    if (status != kStatus_Success)
    {
        PRINTF("- failed to get random data!\r\n\r\n");
        return;
    }
#if defined(PRINT_BLOBS_CONTENT)
    PRINTF("The key value is: \r\n");
    for(uint32_t i = 0; i < sizeof(s_KeyGenerate); i++)
    {
        PRINTF("%02x ", s_KeyGenerate[i]);
    }
    PRINTF("\r\n");
#endif /* PRINT_BLOBS_CONTENT */
    status = CAAM_BLACK_GetKeyBlacken(base, handle, s_KeyGenerate, sizeof(s_KeyGenerate), kCAAM_FIFOST_Type_Kek_Kek, s_KeyBlacken);

    if (status != kStatus_Success)
    {
        PRINTF("- failed to blacken key data!\r\n\r\n");
        return;
    }
#if defined(PRINT_BLOBS_CONTENT)
    PRINTF("The blacken key value is: \r\n");
    for(uint32_t i = 0; i < sizeof(s_KeyBlacken); i++)
    {
        PRINTF("%02x ", s_KeyBlacken[i]);
    }
    PRINTF("\r\n");
#endif /* PRINT_BLOBS_CONTENT */
    PRINTF("Blacken key successfully.\r\n\r\n");

}

/*!
 * @brief Executes example for black KEY blob encryption and decryption.
 */
static void BlackBlobExample(CAAM_Type *base, caam_handle_t *handle)
{
    status_t status;

    memset(s_KeyModifier, 0, sizeof(s_KeyModifier));
    memset(s_blob_data, 0, sizeof(s_blob_data));
    memset(s_blob_DecapData, 0, sizeof(s_blob_DecapData));

    PRINTF("CAAM Black blob Example \r\n\r\n");
#if defined(PRINT_BLOBS_CONTENT)
    PRINTF("The blacken key value is: \r\n ");
    for(uint32_t i = 0; i < sizeof(s_KeyBlacken); i++)
    {
        PRINTF("%02x ", s_KeyBlacken[i]);
    }
    PRINTF("\r\n");
#endif /* PRINT_BLOBS_CONTENT */

    /* Generate random Key as key modifier */
    status = CAAM_RNG_GetRandomData(base, handle, kCAAM_RngStateHandle0, s_KeyModifier, sizeof(s_KeyModifier), kCAAM_RngDataAny, NULL);
    if (status != kStatus_Success)
    {
        PRINTF("- failed to get random key modifier!\r\n\r\n");
        return;
    }

    status = CAAM_BlackBlob_Encapsule(base, handle,s_KeyModifier, sizeof(s_KeyModifier), s_KeyBlacken, sizeof(s_KeyBlacken), s_blob_data, kCAAM_Descriptor_Type_Kek_Kek);

    if (status != kStatus_Success)
    {
        PRINTF("- failed to generate black blob!\r\n\r\n");
        return;
    }
#if defined(PRINT_BLOBS_CONTENT)
    PRINTF("The encrypted black blob data is: \r\n ");
    for(uint32_t i=0; i < sizeof(s_blob_data); i++)
    {
        PRINTF("%02x ", s_blob_data[i]);
    }
    PRINTF("\r\n");
#endif /* PRINT_BLOBS_CONTENT */
    status = CAAM_BlackBlob_Decapsule(base, handle, s_KeyModifier, sizeof(s_KeyModifier), s_blob_data,  s_blob_DecapData, sizeof(s_blob_DecapData),kCAAM_Descriptor_Type_Kek_Kek);

    if (status != kStatus_Success)
    {
        PRINTF("- failed to decapsulate black blob!\r\n\r\n");
        return;
    }
#if defined(PRINT_BLOBS_CONTENT)
    PRINTF("The decrypted black blob data is: \r\n ");
    for(uint32_t i = 0; i < sizeof(s_blob_DecapData); i++)
    {
        PRINTF("%02x ", s_blob_DecapData[i]);
    }
    PRINTF("\r\n");
#endif /* PRINT_BLOBS_CONTENT */
    if (memcmp(s_blob_DecapData, s_KeyBlacken, sizeof(s_KeyBlacken)) != 0)
    {
        PRINTF("- data mismatch!\r\n\r\n");
        return;
    }

    PRINTF("Generate black blob successfully.\r\n\r\n");

}

/*!
 * @brief Executes example for crc.
 */
static void RunCrcExamples(CAAM_Type *base, caam_handle_t *handle)
{
    status_t status;
    caam_crc_ctx_t ctx;

    PRINTF("CRC:");
    memset(crc_output, 0x00, 4u);

    status = CAAM_CRC_Init(base, handle, &ctx, kCAAM_CrciSCSI, NULL, 0u, kCAAM_CRC_ModeDefault);

    if (status != kStatus_Success)
    {
        PRINTF("- failed to Hash init!\r\n\r\n");
        return;
    }


    status = CAAM_CRC_Update(&ctx, s_CrcPlain, sizeof(s_CrcPlain) -1u);

    if (status != kStatus_Success)
    {
        PRINTF("- failed to CRC update!\r\n\r\n");
        return;
    }

    status = CAAM_CRC_Finish(&ctx, crc_output, NULL);

    if (status != kStatus_Success)
    {
        PRINTF("- failed to CRC finish!\r\n\r\n");
        return;
    }

   if (memcmp(crc_output, s_CrciSCSIExpected, 4u) != 0)
    {
        PRINTF("- failed: unexpected CRC output!\r\n\r\n");
        return;
    }

    /* Compute with CRC-16/CCITT-FALSE protocol*/
    /* CRC mode with custom polynomial and mode values to conform CRC-16/CCITT-FALSE protocol */
    /* Polynomial: 0x1021 = set this value into polynomial and set coresponding size into polynomialsize
       Init values: 0x0000 = set kCAAM_ModeIVZ into AAI (Additional Algorithm Information)
       RefIn: false = Input data is NOT reflected so set kCAAM_ModeDIS
       Refout:  false = Output CRC is NOT reflected so set kCAAM_ModeDOS
       XORout: 0x0000 = No XOR is performed on the output CRC so set kCAAM_ModeDOC
    */

    caam_aai_crc_alg_t mode = (caam_aai_crc_alg_t)((uint32_t)kCAAM_CRC_ModeDIS | (uint32_t)kCAAM_CRC_ModeDOS | (uint32_t)kCAAM_CRC_ModeDOC | (uint32_t)kCAAM_CRC_ModeIVZ);
    status = CAAM_CRC(base, handle, kCAAM_CrcCUSTPOLY, mode, s_CrcPlain, sizeof(s_CrcPlain), s_Crc16CCITZeroPoly, 2u, crc_output, NULL);
    if (memcmp(crc_output, s_Crc16CCITZeroExpected, 2u) != 0)
    {
        PRINTF("- failed: unexpected CRC output!\r\n\r\n");
        return;
    }

    PRINTF(" done successfully.\r\n\r\n");
}

/*!
 * @brief Main function.
 */
int main(void)
{
    CAAM_Type *base = CAAM;
    caam_handle_t caamHandle;
    caam_config_t caamConfig;

    /* Init hardware */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Get default configuration. */
    CAAM_GetDefaultConfig(&caamConfig);

    /* setup memory for job ring interfaces. Can be in system memory or CAAM's secure memory.
     * Although this driver example only uses job ring interface 0, example setup for job ring interface 1 is also
     * shown.
     */
    caamConfig.jobRingInterface[0] = &s_jrif0;
    caamConfig.jobRingInterface[1] = &s_jrif1;
    caamConfig.jobRingInterface[2] = &s_jrif2;
    caamConfig.jobRingInterface[3] = &s_jrif3;

    /* Init CAAM driver, including CAAM's internal RNG */
    if (CAAM_Init(base, &caamConfig) != kStatus_Success)
    {
        /* Make sure that RNG is not already instantiated (reset otherwise) */
        PRINTF("- failed to init CAAM&RNG!\r\n\r\n");
    }

    PRINTF("CAAM Peripheral Driver Example\r\n\r\n");

    /* in this driver example, requests for CAAM jobs use job ring 0 */
    PRINTF("*CAAM Job Ring 0* :\r\n\r\n");
    caamHandle.jobRing = kCAAM_JobRing0;

    /* Example of SHA */
    RunShaExamples(base, &caamHandle);

    /* Example of HMAC */
    RunHmacExamples(base, &caamHandle);

    /* Example of AES CBC */
    RunAesCbcExamples(base, &caamHandle);

    /* in this driver example, requests for CAAM jobs use job ring 1 */
    PRINTF("*CAAM Job Ring 1* :\r\n\r\n");
    caamHandle.jobRing = kCAAM_JobRing1;

    /* Example of AES GCM */
    RunAesGcmExamples(base, &caamHandle);

    /* in this driver example, requests for CAAM jobs use job ring 2 */
    PRINTF("*CAAM Job Ring 2* :\r\n\r\n");
    caamHandle.jobRing = kCAAM_JobRing2;

    /* Example of AES CBC */
    RunAesCbcExamples(base, &caamHandle);

    /* in this driver example, requests for CAAM jobs use job ring 3 */
    PRINTF("*CAAM Job Ring 3* :\r\n\r\n");
    caamHandle.jobRing = kCAAM_JobRing3;

    /* Example of AES GCM */
    RunAesGcmExamples(base, &caamHandle);

    /* Example of RNG */
    RunRngExample(base, &caamHandle);

    /* Example of Blob */
    RedBlobExample(base, &caamHandle);

    GenBlackKey(base, &caamHandle);
    BlackBlobExample(base, &caamHandle);

    /* Example of CRC */
    RunCrcExamples(base, &caamHandle);

    /* Deinit RNG */
    caamHandle.jobRing = kCAAM_JobRing0;
    if (CAAM_RNG_Deinit(CAAM, &caamHandle, kCAAM_RngStateHandle0) != kStatus_Success)
    {
        PRINTF("- failed to deinit RNG!\r\n\r\n");
    }

    while (1)
    {
    }
}
