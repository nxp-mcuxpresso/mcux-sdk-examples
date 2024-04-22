/*
 * Copyright 2022 NXP
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
#include "fsl_s3mu.h"   /* Messaging unit driver */
#include "ele_fw.h"     /* ELE FW, to be placed in bootable container in real world app */
#include "fsl_cache.h"

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

/*! @brief RSA PKCS#1 Ver 1.5 test vector data.                                                       *
 *  (https://csrc.nist.gov/Projects/cryptographic-algorithm-validation-program/digital-signatures)    *
 *   File: SigVer15_186-3.rsp                                                                         *
 * e =   49d2a1                                                                                       *
 * Msg = 95123c8d1b236540b86976a11cea31f8bd4e6c54c235147d20ce722b03a6ad756fbd918c27df8ea9ce3104       *
 *       444c0bbe877305bc02e35535a02a58dcda306e632ad30b3dc3ce0ba97fdf46ec192965dd9cd7f4a71b02b8       *
 *       cba3d442646eeec4af590824ca98d74fbca934d0b6867aa1991f3040b707e806de6e66b5934f05509bea         *
 *                                                                                                    *
 * S  =  51265d96f11ab338762891cb29bf3f1d2b3305107063f5f3245af376dfcc7027d39365de70a31db05e9e10       *
 *       eb6148cb7f6425f0c93c4fb0e2291adbd22c77656afc196858a11e1c670d9eeb592613e69eb4f3aa501730       *
 *       743ac4464486c7ae68fd509e896f63884e9424f69c1c5397959f1e52a368667a598a1fc90125273d934129       *
 *       5d2f8e1cc4969bf228c860e07a3546be2eeda1cde48ee94d062801fe666e4a7ae8cb9cd79262c017b081af       *
 *       874ff00453ca43e34efdb43fffb0bb42a4e2d32a5e5cc9e8546a221fe930250e5f5333e0efe58ffebf1936       *
 *       9a3b8ae5a67f6a048bc9ef915bda25160729b508667ada84a0c27e7e26cf2abca413e5e4693f4a9405           *
 *                                                                                                    *
 *       Result = P                                                                                   */

SDK_ALIGN(uint8_t rsa_modulo_2048_test[256], 8u) = {
    0xC4, 0x7A, 0xBA, 0xCC, 0x2A, 0x84, 0xD5, 0x6F, 0x36, 0x14, 0xD9, 0x2F, 0xD6, 0x2E, 0xD3, 0x6D, 0xDD, 0xE4, 0x59,
    0x66, 0x4B, 0x93, 0x01, 0xDC, 0xD1, 0xD6, 0x17, 0x81, 0xCF, 0xCC, 0x02, 0x6B, 0xCB, 0x23, 0x99, 0xBE, 0xE7, 0xE7,
    0x56, 0x81, 0xA8, 0x0B, 0x7B, 0xF5, 0x00, 0xE2, 0xD0, 0x8C, 0xEA, 0xE1, 0xC4, 0x2E, 0xC0, 0xB7, 0x07, 0x92, 0x7F,
    0x2B, 0x2F, 0xE9, 0x2A, 0xE8, 0x52, 0x08, 0x7D, 0x25, 0xF1, 0xD2, 0x60, 0xCC, 0x74, 0x90, 0x5E, 0xE5, 0xF9, 0xB2,
    0x54, 0xED, 0x05, 0x49, 0x4A, 0x9F, 0xE0, 0x67, 0x32, 0xC3, 0x68, 0x09, 0x92, 0xDD, 0x6F, 0x0D, 0xC6, 0x34, 0x56,
    0x8D, 0x11, 0x54, 0x2A, 0x70, 0x5F, 0x83, 0xAE, 0x96, 0xD2, 0xA4, 0x97, 0x63, 0xD5, 0xFB, 0xB2, 0x43, 0x98, 0xED,
    0xF3, 0x70, 0x2B, 0xC9, 0x4B, 0xC1, 0x68, 0x19, 0x01, 0x66, 0x49, 0x2B, 0x86, 0x71, 0xDE, 0x87, 0x4B, 0xB9, 0xCE,
    0xCB, 0x05, 0x8C, 0x6C, 0x83, 0x44, 0xAA, 0x8C, 0x93, 0x75, 0x4D, 0x6E, 0xFF, 0xCD, 0x44, 0xA4, 0x1E, 0xD7, 0xDE,
    0x0A, 0x9D, 0xCD, 0x91, 0x44, 0x43, 0x7F, 0x21, 0x2B, 0x18, 0x88, 0x1D, 0x04, 0x2D, 0x33, 0x1A, 0x46, 0x18, 0xA9,
    0xE6, 0x30, 0xEF, 0x9B, 0xB6, 0x63, 0x05, 0xE4, 0xFD, 0xF8, 0xF0, 0x39, 0x1B, 0x3B, 0x23, 0x13, 0xFE, 0x54, 0x9F,
    0x01, 0x89, 0xFF, 0x96, 0x8B, 0x92, 0xF3, 0x3C, 0x26, 0x6A, 0x4B, 0xC2, 0xCF, 0xFC, 0x89, 0x7D, 0x19, 0x37, 0xEE,
    0xB9, 0xE4, 0x06, 0xF5, 0xD0, 0xEA, 0xA7, 0xA1, 0x47, 0x82, 0xE7, 0x6A, 0xF3, 0xFC, 0xE9, 0x8F, 0x54, 0xED, 0x23,
    0x7B, 0x4A, 0x04, 0xA4, 0x15, 0x9A, 0x5F, 0x62, 0x50, 0xA2, 0x96, 0xA9, 0x02, 0x88, 0x02, 0x04, 0xE6, 0x1D, 0x89,
    0x1C, 0x4D, 0xA2, 0x9F, 0x2D, 0x65, 0xF3, 0x4C, 0xBB};

SDK_ALIGN(uint8_t rsa_pub_exp_2048_test[3], 8u) = {0x49, 0xd2, 0xa1};

SDK_ALIGN(uint8_t digest_msg_test[32], 8u) = {0x56, 0x5f, 0xf4, 0xf3, 0x6e, 0x8b, 0xd4, 0xa9, 0x60, 0x07, 0xed,
                                              0x57, 0x7f, 0x32, 0x48, 0xb4, 0xf9, 0x94, 0x38, 0x26, 0xd7, 0x21,
                                              0xb4, 0x17, 0xa2, 0x0b, 0xb1, 0x2c, 0x3a, 0xe6, 0xe8, 0x74};

SDK_ALIGN(uint8_t rsa_sig_2048_test[256], 8u) = {
    0x51, 0x26, 0x5D, 0x96, 0xF1, 0x1A, 0xB3, 0x38, 0x76, 0x28, 0x91, 0xCB, 0x29, 0xBF, 0x3F, 0x1D, 0x2B, 0x33, 0x05,
    0x10, 0x70, 0x63, 0xF5, 0xF3, 0x24, 0x5A, 0xF3, 0x76, 0xDF, 0xCC, 0x70, 0x27, 0xD3, 0x93, 0x65, 0xDE, 0x70, 0xA3,
    0x1D, 0xB0, 0x5E, 0x9E, 0x10, 0xEB, 0x61, 0x48, 0xCB, 0x7F, 0x64, 0x25, 0xF0, 0xC9, 0x3C, 0x4F, 0xB0, 0xE2, 0x29,
    0x1A, 0xDB, 0xD2, 0x2C, 0x77, 0x65, 0x6A, 0xFC, 0x19, 0x68, 0x58, 0xA1, 0x1E, 0x1C, 0x67, 0x0D, 0x9E, 0xEB, 0x59,
    0x26, 0x13, 0xE6, 0x9E, 0xB4, 0xF3, 0xAA, 0x50, 0x17, 0x30, 0x74, 0x3A, 0xC4, 0x46, 0x44, 0x86, 0xC7, 0xAE, 0x68,
    0xFD, 0x50, 0x9E, 0x89, 0x6F, 0x63, 0x88, 0x4E, 0x94, 0x24, 0xF6, 0x9C, 0x1C, 0x53, 0x97, 0x95, 0x9F, 0x1E, 0x52,
    0xA3, 0x68, 0x66, 0x7A, 0x59, 0x8A, 0x1F, 0xC9, 0x01, 0x25, 0x27, 0x3D, 0x93, 0x41, 0x29, 0x5D, 0x2F, 0x8E, 0x1C,
    0xC4, 0x96, 0x9B, 0xF2, 0x28, 0xC8, 0x60, 0xE0, 0x7A, 0x35, 0x46, 0xBE, 0x2E, 0xED, 0xA1, 0xCD, 0xE4, 0x8E, 0xE9,
    0x4D, 0x06, 0x28, 0x01, 0xFE, 0x66, 0x6E, 0x4A, 0x7A, 0xE8, 0xCB, 0x9C, 0xD7, 0x92, 0x62, 0xC0, 0x17, 0xB0, 0x81,
    0xAF, 0x87, 0x4F, 0xF0, 0x04, 0x53, 0xCA, 0x43, 0xE3, 0x4E, 0xFD, 0xB4, 0x3F, 0xFF, 0xB0, 0xBB, 0x42, 0xA4, 0xE2,
    0xD3, 0x2A, 0x5E, 0x5C, 0xC9, 0xE8, 0x54, 0x6A, 0x22, 0x1F, 0xE9, 0x30, 0x25, 0x0E, 0x5F, 0x53, 0x33, 0xE0, 0xEF,
    0xE5, 0x8F, 0xFE, 0xBF, 0x19, 0x36, 0x9A, 0x3B, 0x8A, 0xE5, 0xA6, 0x7F, 0x6A, 0x04, 0x8B, 0xC9, 0xEF, 0x91, 0x5B,
    0xDA, 0x25, 0x16, 0x07, 0x29, 0xB5, 0x08, 0x66, 0x7A, 0xDA, 0x84, 0xA0, 0xC2, 0x7E, 0x7E, 0x26, 0xCF, 0x2A, 0xBC,
    0xA4, 0x13, 0xE5, 0xE4, 0x69, 0x3F, 0x4A, 0x94, 0x05};

/*! @brief RSA PSS test vector data.                                                                  *
 *  (https://csrc.nist.gov/Projects/cryptographic-algorithm-validation-program/digital-signatures)    *
 *   File: SigVerPSS_186-3.rsp                                                                        *
 * e =   73b193                                                                                       *
 * Msg = 0897d40e7c0f2dfc07b0c7fddaf5fd8fcc6af9c1fdc17bebb923d59c9fc43bd402ba39738f0f85f23015f7       *
 *       5131f9d650a29b55e2fc9d5ddf07bb8df9fa5a80f1e4634e0b4c5155bf148939b1a4ea29e344a66429c850       *
 *       fcde7336dad616f0039378391abcfafe25ca7bb594057af07faf7a322f7fab01e051c63cc51b39af4d23         *
 *                                                                                                    *
 * S  =  8ebed002d4f54de5898a5f2e69d770ed5a5ce1d45ad6dd9ce5f1179d1c46daa4d0394e21a99d803358d9ab       *
 *       fd23bb53166394f997b909e675662066324ca1f2b731deba170525c4ee8fa752d2d7f201b10219489f5784       *
 *       e399d916302fd4b7adf88490df876501c46742a93cfb3aaab9602e65d7e60d7c4ceadb7eb67e421d180323       *
 *       a6d38f38b9f999213ebfccc7e04f060fbdb7c210206522b494e199e98c6c24e457f8696644fdcaebc1b903       *
 *       1c818322c29d135e1172fa0fdf7be1007dabcaab4966332e7ea1456b6ce879cd910c9110104fc7d3dcab07       *
 *       6f2bd182bb8327a863254570cdf2ab38e0cda31779deaad616e3437ed659d74e5a4e045a70133890b81bc4       *
 *       f24ab6da67a2ee0ce15baba337d091cb5a1c44da690f81145b0252a6549bbb20cd5cc47afec755eb37fed5       *
 *       5a9a33d36557424503d805a0a120b76941f4150d89342d7a7fa3a2b08c515e6f68429cf7afd1a3fce0f428       *
 *       351a6f9eda3ab24a7ef591994c21fbf1001f99239e88340f9b359ec72e8a212a1920e6cf993ff848             *
 *                                                                                                    *
 *       SaltVal = 00                                                                                 *
 *       Result = P                                                                                   */
SDK_ALIGN(uint8_t rsa_pss_modulo_3072_test[384], 8u) = {
    0xCE, 0x49, 0x24, 0xFF, 0x47, 0x0F, 0xB9, 0x9D, 0x17, 0xF6, 0x65, 0x95, 0x56, 0x1A, 0x74, 0xDE, 0xD2, 0x20, 0x92,
    0xD1, 0xDC, 0x27, 0x12, 0x2A, 0xE1, 0x5C, 0xA8, 0xCA, 0xC4, 0xBF, 0xAE, 0x11, 0xDA, 0xA9, 0xE3, 0x7A, 0x94, 0x14,
    0x30, 0xDD, 0x1B, 0x81, 0xAA, 0xF4, 0x72, 0xF3, 0x20, 0x83, 0x5E, 0xE2, 0xFE, 0x74, 0x4C, 0x83, 0xF1, 0x32, 0x08,
    0x82, 0xA8, 0xA0, 0x23, 0x16, 0xCE, 0xB3, 0x75, 0xF5, 0xC4, 0x90, 0x92, 0x32, 0xBB, 0x2C, 0x65, 0x20, 0xB2, 0x49,
    0xC8, 0x8B, 0xE4, 0xF4, 0x7B, 0x8B, 0x86, 0xFD, 0xD9, 0x36, 0x78, 0xC6, 0x9E, 0x64, 0xF5, 0x00, 0x89, 0xE9, 0x07,
    0xA5, 0x50, 0x4F, 0xDD, 0x43, 0xF0, 0xCA, 0xD2, 0x4A, 0xAA, 0x9E, 0x31, 0x7E, 0xF2, 0xEC, 0xAD, 0xE3, 0xB5, 0xC1,
    0xFD, 0x31, 0xF3, 0xC3, 0x27, 0xD7, 0x0A, 0x0E, 0x2D, 0x48, 0x67, 0xE6, 0xFE, 0x3F, 0x26, 0x27, 0x2E, 0x8B, 0x6A,
    0x3C, 0xCE, 0x17, 0x84, 0x3E, 0x35, 0x9B, 0x82, 0xEB, 0x7A, 0x4C, 0xAD, 0x8C, 0x42, 0x46, 0x01, 0x79, 0xCB, 0x6C,
    0x07, 0xFA, 0x25, 0x2E, 0xFA, 0xEC, 0x42, 0x8F, 0xD5, 0xCA, 0xE5, 0x20, 0x8B, 0x29, 0x8B, 0x25, 0x51, 0x09, 0x02,
    0x6E, 0x21, 0x27, 0x24, 0x24, 0xEC, 0x0C, 0x52, 0xE1, 0xE5, 0xF7, 0x2C, 0x5A, 0xB0, 0x6F, 0x5D, 0x2A, 0x05, 0xE7,
    0x7C, 0x19, 0x3B, 0x64, 0x7E, 0xC9, 0x48, 0xBB, 0x84, 0x4E, 0x0C, 0x2E, 0xF1, 0x30, 0x7F, 0x53, 0xCB, 0x80, 0x0D,
    0x4F, 0x55, 0x52, 0x3D, 0x86, 0x03, 0x8B, 0xB9, 0xE2, 0x10, 0x99, 0xA8, 0x61, 0xB6, 0xB9, 0xBC, 0xC9, 0x69, 0xE5,
    0xDD, 0xDB, 0xDF, 0x71, 0x71, 0xB3, 0x7D, 0x61, 0x63, 0x81, 0xB7, 0x8C, 0x3B, 0x22, 0xEF, 0x66, 0x51, 0x0B, 0x27,
    0x65, 0xD9, 0x61, 0x75, 0x56, 0xB1, 0x75, 0x59, 0x98, 0x79, 0xD8, 0x55, 0x81, 0x00, 0xAD, 0x90, 0xB8, 0x30, 0xE8,
    0x7A, 0xD4, 0x60, 0xA2, 0x21, 0x08, 0xBA, 0xA5, 0xED, 0x0F, 0x2B, 0xA9, 0xDF, 0xC0, 0x51, 0x67, 0xF8, 0xAB, 0x61,
    0xFC, 0x9F, 0x8A, 0xE0, 0x16, 0x03, 0xF9, 0xDD, 0x5E, 0x66, 0xCE, 0x1E, 0x64, 0x2B, 0x60, 0x4B, 0xCA, 0x92, 0x94,
    0xB5, 0x7F, 0xB7, 0xC0, 0xD8, 0x3F, 0x05, 0x4B, 0xAC, 0xF4, 0x45, 0x4C, 0x29, 0x8A, 0x27, 0x2C, 0x44, 0xBC, 0x71,
    0x8F, 0x54, 0x60, 0x5B, 0x91, 0xE0, 0xBF, 0xAF, 0xD7, 0x72, 0xAE, 0xBA, 0xF3, 0x82, 0x88, 0x46, 0xC9, 0x30, 0x18,
    0xF9, 0x8E, 0x31, 0x57, 0x08, 0xD5, 0x0B, 0xE8, 0x40, 0x1E, 0xB9, 0xA8, 0x77, 0x8D, 0xCB, 0xD0, 0xD6, 0xDB, 0x93,
    0x70, 0x86, 0x04, 0x11, 0xB0, 0x04, 0xCD, 0x37, 0xFB, 0xB8, 0xB5, 0xDF, 0x87, 0xED, 0xEE, 0x7A, 0xAE, 0x94, 0x9F,
    0xFF, 0x34, 0x60, 0x7B};

SDK_ALIGN(uint8_t rsa_pss_pub_exp_3072_test[3], 8u) = {0x73, 0xb1, 0x93};

SDK_ALIGN(uint8_t digest_pss_msg_test[32], 8u) = {0x07, 0xa6, 0x12, 0x4a, 0xe7, 0x51, 0x28, 0x37, 0x95, 0xc4, 0x4d,
                                                  0x64, 0xd8, 0x1e, 0xc9, 0x89, 0xbd, 0x00, 0xb1, 0x87, 0x99, 0xb0,
                                                  0xc9, 0x26, 0x21, 0xaa, 0xc2, 0xdb, 0x7d, 0x36, 0xc9, 0x6f};

SDK_ALIGN(uint8_t rsa_pss_sig_3072_test[384], 8u) = {
    0x8E, 0xBE, 0xD0, 0x02, 0xD4, 0xF5, 0x4D, 0xE5, 0x89, 0x8A, 0x5F, 0x2E, 0x69, 0xD7, 0x70, 0xED, 0x5A, 0x5C, 0xE1,
    0xD4, 0x5A, 0xD6, 0xDD, 0x9C, 0xE5, 0xF1, 0x17, 0x9D, 0x1C, 0x46, 0xDA, 0xA4, 0xD0, 0x39, 0x4E, 0x21, 0xA9, 0x9D,
    0x80, 0x33, 0x58, 0xD9, 0xAB, 0xFD, 0x23, 0xBB, 0x53, 0x16, 0x63, 0x94, 0xF9, 0x97, 0xB9, 0x09, 0xE6, 0x75, 0x66,
    0x20, 0x66, 0x32, 0x4C, 0xA1, 0xF2, 0xB7, 0x31, 0xDE, 0xBA, 0x17, 0x05, 0x25, 0xC4, 0xEE, 0x8F, 0xA7, 0x52, 0xD2,
    0xD7, 0xF2, 0x01, 0xB1, 0x02, 0x19, 0x48, 0x9F, 0x57, 0x84, 0xE3, 0x99, 0xD9, 0x16, 0x30, 0x2F, 0xD4, 0xB7, 0xAD,
    0xF8, 0x84, 0x90, 0xDF, 0x87, 0x65, 0x01, 0xC4, 0x67, 0x42, 0xA9, 0x3C, 0xFB, 0x3A, 0xAA, 0xB9, 0x60, 0x2E, 0x65,
    0xD7, 0xE6, 0x0D, 0x7C, 0x4C, 0xEA, 0xDB, 0x7E, 0xB6, 0x7E, 0x42, 0x1D, 0x18, 0x03, 0x23, 0xA6, 0xD3, 0x8F, 0x38,
    0xB9, 0xF9, 0x99, 0x21, 0x3E, 0xBF, 0xCC, 0xC7, 0xE0, 0x4F, 0x06, 0x0F, 0xBD, 0xB7, 0xC2, 0x10, 0x20, 0x65, 0x22,
    0xB4, 0x94, 0xE1, 0x99, 0xE9, 0x8C, 0x6C, 0x24, 0xE4, 0x57, 0xF8, 0x69, 0x66, 0x44, 0xFD, 0xCA, 0xEB, 0xC1, 0xB9,
    0x03, 0x1C, 0x81, 0x83, 0x22, 0xC2, 0x9D, 0x13, 0x5E, 0x11, 0x72, 0xFA, 0x0F, 0xDF, 0x7B, 0xE1, 0x00, 0x7D, 0xAB,
    0xCA, 0xAB, 0x49, 0x66, 0x33, 0x2E, 0x7E, 0xA1, 0x45, 0x6B, 0x6C, 0xE8, 0x79, 0xCD, 0x91, 0x0C, 0x91, 0x10, 0x10,
    0x4F, 0xC7, 0xD3, 0xDC, 0xAB, 0x07, 0x6F, 0x2B, 0xD1, 0x82, 0xBB, 0x83, 0x27, 0xA8, 0x63, 0x25, 0x45, 0x70, 0xCD,
    0xF2, 0xAB, 0x38, 0xE0, 0xCD, 0xA3, 0x17, 0x79, 0xDE, 0xAA, 0xD6, 0x16, 0xE3, 0x43, 0x7E, 0xD6, 0x59, 0xD7, 0x4E,
    0x5A, 0x4E, 0x04, 0x5A, 0x70, 0x13, 0x38, 0x90, 0xB8, 0x1B, 0xC4, 0xF2, 0x4A, 0xB6, 0xDA, 0x67, 0xA2, 0xEE, 0x0C,
    0xE1, 0x5B, 0xAB, 0xA3, 0x37, 0xD0, 0x91, 0xCB, 0x5A, 0x1C, 0x44, 0xDA, 0x69, 0x0F, 0x81, 0x14, 0x5B, 0x02, 0x52,
    0xA6, 0x54, 0x9B, 0xBB, 0x20, 0xCD, 0x5C, 0xC4, 0x7A, 0xFE, 0xC7, 0x55, 0xEB, 0x37, 0xFE, 0xD5, 0x5A, 0x9A, 0x33,
    0xD3, 0x65, 0x57, 0x42, 0x45, 0x03, 0xD8, 0x05, 0xA0, 0xA1, 0x20, 0xB7, 0x69, 0x41, 0xF4, 0x15, 0x0D, 0x89, 0x34,
    0x2D, 0x7A, 0x7F, 0xA3, 0xA2, 0xB0, 0x8C, 0x51, 0x5E, 0x6F, 0x68, 0x42, 0x9C, 0xF7, 0xAF, 0xD1, 0xA3, 0xFC, 0xE0,
    0xF4, 0x28, 0x35, 0x1A, 0x6F, 0x9E, 0xDA, 0x3A, 0xB2, 0x4A, 0x7E, 0xF5, 0x91, 0x99, 0x4C, 0x21, 0xFB, 0xF1, 0x00,
    0x1F, 0x99, 0x23, 0x9E, 0x88, 0x34, 0x0F, 0x9B, 0x35, 0x9E, 0xC7, 0x2E, 0x8A, 0x21, 0x2A, 0x19, 0x20, 0xE6, 0xCF,
    0x99, 0x3F, 0xF8, 0x48};

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
     * 0.  load and authenticate EdgeLock Enclave FW (to be done in secure boot flow in real world app)
     * 1.  Start RNG
     * 2.  Generate RSA key-pair
     * 3.  Generate RSA PKCS#1.5 signature
     * 4.  Generate RSA PSS signature
     * 5.  Verify RSA PKCS#1.5 signature (NIST vector)
     * 6.  Verify RSA PSS signature (NIST vector)
     * 7.  RSA PKCS#1.5 encrypt and decrypt
     * 8.  RSA OAEP encrypt and decrypt
     */

    status_t result = kStatus_Fail;

    do
    {
        /* HW init */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

        PRINTF("EdgeLock Enclave secure Sub-System Driver Example:\r\n\r\n");

        /****************** Load EdgeLock FW message ***************************/
        PRINTF("*********** Load EdgeLock FW ******************************\r\n");
        if (ELE_LoadFw(S3MU, ele_fw) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("EdgeLock FW loaded and authenticated successfully.\r\n\r\n");
        }

        /****************** Start RNG ***********************/
        PRINTF("****************** Start RNG ******************\r\n");
        if (ELE_StartRng(S3MU) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("EdgeLock RNG Start success.\r\n");
        }

        uint32_t trng_state = 0u;
        do
        {
            result = ELE_GetTrngState(S3MU, &trng_state);
        } while ((trng_state & 0xFF) != kELE_TRNG_ready && result == kStatus_Success);

        PRINTF("EdgeLock RNG ready to use.\r\n\r\n");

        /****************** Generic Crypto RSA keygen **************************/
        PRINTF("*********** Generic Crypto RSA keygen *********************\r\n");

        SDK_ALIGN(uint8_t modulus[2048u / 8u], 8u)  = {0u};
        SDK_ALIGN(uint8_t priv_exp[2048u / 8u], 8u) = {0u};
        SDK_ALIGN(uint8_t pub_exp[3], 8u)           = {0x1, 0x0, 0x1}; /* public exponent: 65537 */

        ele_generic_rsa_t GenericRsaKeygen  = {0u};
        GenericRsaKeygen.modulus            = (uint32_t)modulus;
        GenericRsaKeygen.priv_exponent      = (uint32_t)priv_exp;
        GenericRsaKeygen.priv_exponent_size = sizeof(priv_exp);
        GenericRsaKeygen.modulus_size       = sizeof(modulus);
        GenericRsaKeygen.pub_exponent       = (uint32_t)pub_exp;
        GenericRsaKeygen.pub_exponent_size  = sizeof(pub_exp);
        GenericRsaKeygen.key_size           = 2048u;

        /* Generate key-pair */
        PRINTF("RSA Key-Pair generation. \r\n");

        if (ELE_GenericRsaKeygen(S3MU, &GenericRsaKeygen) != kStatus_Success)
        {
            PRINTF("RSA Key-pair generation failed\r\n\r\n");
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("RSA Key-pair generated successfully.\r\n\r\n");
        }

        /****************** Generic Crypto RSA PKCS#1.5 Signature gen *********/
        PRINTF("*********** Generic Crypto RSA PKCS#1.5 Signature gen *****\r\n");
        SDK_ALIGN(uint8_t rsa_sig_2048[256], 8u) = {0u};

        SDK_ALIGN(uint8_t msg[3], 8u)         = {0xAA, 0xAA, 0xAA};
        SDK_ALIGN(uint8_t digest_msg[32], 8u) = {0x9B, 0x68, 0x42, 0xCB, 0xC4, 0x8D, 0x02, 0x52, 0x4C, 0x05, 0x66,
                                                 0xCF, 0xF1, 0xED, 0x43, 0x73, 0xC4, 0x47, 0x13, 0x24, 0xB9, 0xA6,
                                                 0xDB, 0x7D, 0x20, 0x00, 0xF1, 0xCF, 0xFF, 0x7B, 0x03, 0xFE};

        ele_generic_rsa_t GenericRsaSign = {0u};
        GenericRsaSign.mode              = kSignGen;
        GenericRsaSign.algo              = RSA_PKCS1_V1_5_SHA256_SIGN;
        GenericRsaSign.key_size          = 2048u;
        /* Private exponent */
        GenericRsaSign.priv_exponent      = (uint32_t)priv_exp;
        GenericRsaSign.priv_exponent_size = sizeof(priv_exp);
        /* Modulus */
        GenericRsaSign.modulus      = (uint32_t)modulus;
        GenericRsaSign.modulus_size = sizeof(modulus);
        /* Digest */
        GenericRsaSign.digest      = (uint32_t)digest_msg;
        GenericRsaSign.digest_size = sizeof(digest_msg);
        /* Signature destination */
        GenericRsaSign.signature      = (uint32_t)rsa_sig_2048;
        GenericRsaSign.signature_size = sizeof(rsa_sig_2048);
        /* Flags */
        GenericRsaSign.flags = kFlagDigest;

        if (ELE_GenericRsa(S3MU, &GenericRsaSign) != kStatus_Success)
        {
            PRINTF("RSA sign generation with input as digest failed\r\n\r\n");
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("RSA signature with input as digest generated successfully.\r\n");
        }

        /* Try it also with input as message */
        /* Message */
        GenericRsaSign.digest      = (uint32_t)msg;
        GenericRsaSign.digest_size = sizeof(msg);
        /* Flags */
        GenericRsaSign.flags = kFlagActualMsg;

        if (ELE_GenericRsa(S3MU, &GenericRsaSign) != kStatus_Success)
        {
            PRINTF("RSA sign generation with input as message failed\r\n\r\n");
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("RSA signature with input as message generated successfully.\r\n\r\n");
        }

        /****************** Generic Crypto RSA PSS Signature gen **************/
        PRINTF("*********** Generic Crypto RSA PSS Signature gen **********\r\n");
        SDK_ALIGN(uint8_t rsa_pss_sig_2048[256], 8u) = {0u};

        ele_generic_rsa_t GenericRsaPssSign = {0u};
        GenericRsaPssSign.mode              = kSignGen;
        GenericRsaPssSign.algo              = RSA_PKCS1_PSS_MGF1_SHA256;
        GenericRsaPssSign.key_size          = 2048u;
        /* Private exponent */
        GenericRsaPssSign.priv_exponent      = (uint32_t)priv_exp;
        GenericRsaPssSign.priv_exponent_size = sizeof(priv_exp);
        /* Modulus */
        GenericRsaPssSign.modulus      = (uint32_t)modulus;
        GenericRsaPssSign.modulus_size = sizeof(modulus);
        /* Digest */
        GenericRsaPssSign.digest      = (uint32_t)digest_msg;
        GenericRsaPssSign.digest_size = sizeof(digest_msg);
        /* Signature destination */
        GenericRsaPssSign.signature      = (uint32_t)rsa_pss_sig_2048;
        GenericRsaPssSign.signature_size = sizeof(rsa_pss_sig_2048);
        /* Flags */
        GenericRsaPssSign.flags = kFlagDigest;

        if (ELE_GenericRsa(S3MU, &GenericRsaPssSign) != kStatus_Success)
        {
            PRINTF("RSA sign generation failed\r\n\r\n");
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("RSA signature generated successfully.\r\n\r\n");
        }

        /****************** Generic Crypto RSA 2048 PKCS#1.5 Verify ***********/
        PRINTF("*********** Generic Crypto RSA 2048 PKCS#1.5 Verify *******\r\n");

        ele_generic_rsa_t GenericRsaVerif = {0u};
        GenericRsaVerif.mode              = kVerification;
        GenericRsaVerif.algo              = RSA_PKCS1_V1_5_SHA256_SIGN;
        GenericRsaVerif.key_size          = 2048u;
        /* Public exponent */
        GenericRsaVerif.pub_exponent      = (uint32_t)rsa_pub_exp_2048_test;
        GenericRsaVerif.pub_exponent_size = sizeof(rsa_pub_exp_2048_test);
        /* Modulus */
        GenericRsaVerif.modulus      = (uint32_t)rsa_modulo_2048_test;
        GenericRsaVerif.modulus_size = sizeof(modulus);
        /* Digest */
        GenericRsaVerif.digest      = (uint32_t)digest_msg_test;
        GenericRsaVerif.digest_size = sizeof(digest_msg_test);
        /* Signature destination */
        GenericRsaVerif.signature      = (uint32_t)rsa_sig_2048_test;
        GenericRsaVerif.signature_size = sizeof(rsa_sig_2048_test);
        /* Flags */
        GenericRsaVerif.flags = kFlagDigest;

        if ((ELE_GenericRsa(S3MU, &GenericRsaVerif) == kStatus_Success) &&
            (GenericRsaVerif.verify_status == kVerifySuccess))
        {
            PRINTF("RSA verify success\r\n\r\n");
        }
        else
        {
            PRINTF("RSA verify failed!.\r\n\r\n");
        }

        /****************** Generic Crypto RSA 3072 PSS Verify ****************/
        PRINTF("*********** Generic Crypto RSA 3072 PSS Verify ************\r\n");

        ele_generic_rsa_t GenericRsaPssVerif = {0u};
        GenericRsaPssVerif.mode              = kVerification;
        GenericRsaPssVerif.algo              = RSA_PKCS1_PSS_MGF1_SHA256;
        GenericRsaPssVerif.key_size          = 3072u;
        /* Public exponent */
        GenericRsaPssVerif.pub_exponent      = (uint32_t)rsa_pss_pub_exp_3072_test;
        GenericRsaPssVerif.pub_exponent_size = sizeof(rsa_pss_pub_exp_3072_test);
        /* Modulus */
        GenericRsaPssVerif.modulus      = (uint32_t)rsa_pss_modulo_3072_test;
        GenericRsaPssVerif.modulus_size = sizeof(rsa_pss_modulo_3072_test);
        /* Digest */
        GenericRsaPssVerif.digest      = (uint32_t)digest_pss_msg_test;
        GenericRsaPssVerif.digest_size = sizeof(digest_pss_msg_test);
        /* Signature destination */
        GenericRsaPssVerif.signature      = (uint32_t)rsa_pss_sig_3072_test;
        GenericRsaPssVerif.signature_size = sizeof(rsa_pss_sig_3072_test);
        /* Salt size */
        GenericRsaPssVerif.salt_size = 0u;
        /* Flags */
        GenericRsaPssVerif.flags = kFlagDigest;

        if ((ELE_GenericRsa(S3MU, &GenericRsaPssVerif) == kStatus_Success) &&
            (GenericRsaPssVerif.verify_status == kVerifySuccess))
        {
            PRINTF("RSA verify success\r\n\r\n");
        }
        else
        {
            PRINTF("RSA verify failed!.\r\n\r\n");
            result = kStatus_Fail;
            break;
        }

        /****************** Generic Crypto RSA PKCS#1.5 encrypt ***************/
        PRINTF("*********** Generic Crypto RSA PKCS#1.5 encrypt ***********\r\n");
        const char plaintext[]   = "hello world !";
        uint8_t ciphertext[256u] = {0u};

        ele_generic_rsa_t GenericRsaEnc = {0u};
        GenericRsaEnc.mode              = kEncryption;
        GenericRsaEnc.algo              = RSA_PKCS1_V1_5_CRYPT;
        GenericRsaEnc.key_size          = 2048u;
        /* Public exponent */
        GenericRsaEnc.pub_exponent      = (uint32_t)pub_exp;
        GenericRsaEnc.pub_exponent_size = sizeof(pub_exp);
        /* Modulus */
        GenericRsaEnc.modulus      = (uint32_t)modulus;
        GenericRsaEnc.modulus_size = sizeof(modulus);
        /* Plaintext */
        GenericRsaEnc.plaintext      = (uint32_t)plaintext;
        GenericRsaEnc.plaintext_size = sizeof(plaintext);
        /* Ciphertext */
        GenericRsaEnc.ciphertext      = (uint32_t)ciphertext;
        GenericRsaEnc.ciphertext_size = sizeof(ciphertext);
        /* Flags */
        GenericRsaEnc.flags = kFlagDigest;

        if (ELE_GenericRsa(S3MU, &GenericRsaEnc) != kStatus_Success)
        {
            PRINTF("RSA encryption failed\r\n\r\n");
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("RSA encryption success.\r\n\r\n");
        }

        /****************** Generic Crypto RSA PKCS#1.5 decrypt ***************/
        PRINTF("*********** Generic Crypto RSA PKCS#1.5 decrypt ***********\r\n");
        uint8_t retrieve_plaintext[256u] = {0u};

        ele_generic_rsa_t GenericRsaDec = {0u};
        GenericRsaDec.mode              = kDecryption;
        GenericRsaDec.algo              = RSA_PKCS1_V1_5_CRYPT;
        GenericRsaDec.key_size          = 2048u;
        /* Public exponent */
        GenericRsaDec.priv_exponent      = (uint32_t)priv_exp;
        GenericRsaDec.priv_exponent_size = sizeof(priv_exp);
        /* Modulus */
        GenericRsaDec.modulus      = (uint32_t)modulus;
        GenericRsaDec.modulus_size = sizeof(modulus);
        /* Plaintext */
        GenericRsaDec.plaintext      = (uint32_t)retrieve_plaintext;
        GenericRsaDec.plaintext_size = sizeof(plaintext);
        /* Ciphertext */
        GenericRsaDec.ciphertext      = (uint32_t)ciphertext;
        GenericRsaDec.ciphertext_size = sizeof(ciphertext);
        /* Flags */
        GenericRsaDec.flags = kFlagDigest;

        if (ELE_GenericRsa(S3MU, &GenericRsaDec) != kStatus_Success)
        {
            PRINTF("RSA decryption failed\r\n\r\n");
            result = kStatus_Fail;
            break;
        }
        else
        {
            if (memcmp(retrieve_plaintext, plaintext, sizeof(plaintext)) == 0u)
            {
                PRINTF("RSA decryption success, plaintext match original one.\r\n\r\n");
            }
        }

        /****************** Generic Crypto RSA OAEP encrypt *******************/
        PRINTF("*********** Generic Crypto RSA OAEP encrypt ***************\r\n");
        SDK_ALIGN(uint8_t plaintext_oaep[], 8u)      = "hello world and OAEP !";
        SDK_ALIGN(uint8_t ciphertext_oaep[256u], 8u) = {0u};

        /* label for OAEP encryption */
        SDK_ALIGN(uint8_t label[], 8u) = {0xA7u, 0x0Bu, 0x3Fu, 0x9Cu, 0x97u, 0x57u, 0xD1u, 0xE3u};

        ele_generic_rsa_t GenericRsaOaepEnc = {0U};
        GenericRsaOaepEnc.mode              = kEncryption;
        GenericRsaOaepEnc.algo              = RSA_PKCS1_OAEP_SHA256;
        ;
        GenericRsaOaepEnc.key_size = 2048u;
        /* Public exponent */
        GenericRsaOaepEnc.pub_exponent      = (uint32_t)pub_exp;
        GenericRsaOaepEnc.pub_exponent_size = sizeof(pub_exp);
        /* Modulus */
        GenericRsaOaepEnc.modulus      = (uint32_t)modulus;
        GenericRsaOaepEnc.modulus_size = sizeof(modulus);
        /* Plaintext */
        GenericRsaOaepEnc.plaintext      = (uint32_t)plaintext_oaep;
        GenericRsaOaepEnc.plaintext_size = sizeof(plaintext_oaep);
        /* Ciphertext */
        GenericRsaOaepEnc.ciphertext      = (uint32_t)ciphertext_oaep;
        GenericRsaOaepEnc.ciphertext_size = sizeof(ciphertext_oaep);
        /* Label */
        GenericRsaOaepEnc.label      = (uint32_t)label;
        GenericRsaOaepEnc.label_size = sizeof(label);
        /* Flags */
        GenericRsaOaepEnc.flags = kFlagDigest;

        if (ELE_GenericRsa(S3MU, &GenericRsaOaepEnc) != kStatus_Success)
        {
            PRINTF("RSA encryption failed\r\n\r\n");
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("RSA encryption success.\r\n\r\n");
        }

        /****************** Generic Crypto RSA OAEP decrypt *******************/
        PRINTF("*********** Generic Crypto RSA OAEP decrypt ***************\r\n");
        uint8_t retrieve_plaintext_oaep[256u] = {0u};

        ele_generic_rsa_t GenericRsaOaepDec = {0u};
        GenericRsaOaepDec.mode              = kDecryption;
        GenericRsaOaepDec.algo              = RSA_PKCS1_OAEP_SHA256;
        GenericRsaOaepDec.key_size          = 2048u;
        /* Public exponent */
        GenericRsaOaepDec.priv_exponent      = (uint32_t)priv_exp;
        GenericRsaOaepDec.priv_exponent_size = sizeof(priv_exp);
        /* Modulus */
        GenericRsaOaepDec.modulus      = (uint32_t)modulus;
        GenericRsaOaepDec.modulus_size = sizeof(modulus);
        /* Plaintext */
        GenericRsaOaepDec.plaintext      = (uint32_t)retrieve_plaintext_oaep;
        GenericRsaOaepDec.plaintext_size = sizeof(retrieve_plaintext_oaep);
        /* Ciphertext */
        GenericRsaOaepDec.ciphertext      = (uint32_t)ciphertext_oaep;
        GenericRsaOaepDec.ciphertext_size = sizeof(ciphertext_oaep);
        /* Label */
        GenericRsaOaepDec.label      = (uint32_t)label;
        GenericRsaOaepDec.label_size = sizeof(label);
        /* Flags */
        GenericRsaOaepDec.flags = kFlagDigest;

        if (ELE_GenericRsa(S3MU, &GenericRsaOaepDec) != kStatus_Success)
        {
            PRINTF("RSA decryption failed\r\n\r\n");
            result = kStatus_Fail;
            break;
        }
        else
        {
            if (memcmp(retrieve_plaintext_oaep, plaintext_oaep, sizeof(plaintext_oaep)) == 0u)
            {
                PRINTF("RSA decryption success, plaintext match original one.\r\n\r\n");
            }
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
