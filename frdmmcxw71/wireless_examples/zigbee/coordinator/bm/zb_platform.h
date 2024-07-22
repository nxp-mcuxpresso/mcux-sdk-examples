/*
* Copyright 2023-2024 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/* ZB Platform and board dependent functions */

#ifndef ZB_PLATFORM_H
#define ZB_PLATFORM_H

#include <stdint.h>
#include <stdbool.h>
#include "jendefs.h"
#include "EmbeddedTypes.h"

/* Select the Elliptic curve: P-256 or Curve25519 */
#define CRYPTO_ECDH_P256
#undef  CRYPTO_ECDH_CURVE25519

/* Crypto related macro & type defines */
#define CRYPTO_AES_BLK_SIZE 16u             /* [bytes] */
#ifdef CRYPTO_ECDH_P256
#define SEC_ECP256_COORDINATE_BITLEN 256u
#define SEC_ECP256_COORDINATE_LEN    (SEC_ECP256_COORDINATE_BITLEN / 8u)
#define SEC_ECP256_COORDINATE_WLEN   (SEC_ECP256_COORDINATE_LEN / 4u)
#define SEC_ECP256_SCALAR_LEN        32u
#define SEC_ECP256_SCALAR_WLEN       (SEC_ECP256_SCALAR_LEN / 4u)
#endif

/* Security block definition */
typedef union
{
    uint8_t  au8[CRYPTO_AES_BLK_SIZE];
    uint32_t au32[CRYPTO_AES_BLK_SIZE / 4];
} CRYPTO_tsAesBlock;

typedef struct
{
    uint32_t u32register0;
    uint32_t u32register1;
    uint32_t u32register2;
    uint32_t u32register3;
} CRYPTO_tsReg128;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpacked"
#pragma GCC diagnostic ignored "-Wattributes"
#ifdef CRYPTO_ECDH_P256
typedef union {
    uint8_t raw[2*SEC_ECP256_COORDINATE_LEN];
    PACKED_STRUCT {
        uint8_t x[SEC_ECP256_COORDINATE_LEN];
        uint8_t y[SEC_ECP256_COORDINATE_LEN];
    } components_8bit;
    PACKED_STRUCT {
        uint32_t x[SEC_ECP256_COORDINATE_WLEN];
        uint32_t y[SEC_ECP256_COORDINATE_WLEN];
    } components_32bit;
    PACKED_STRUCT {
        uint8_t X[SEC_ECP256_SCALAR_LEN];
        uint8_t Y[SEC_ECP256_SCALAR_LEN];
    } coord;
#elif defined(CRYPTO_ECDH_CURVE25519)
typedef union {
#endif
} CRYPTO_ecdhPublicKey_t;

typedef CRYPTO_ecdhPublicKey_t CRYPTO_ecdhDhKey_t;

#ifdef CRYPTO_ECDH_P256
typedef union {
    uint8_t  raw_8bit[SEC_ECP256_SCALAR_LEN];
    uint32_t raw_32bit[SEC_ECP256_SCALAR_WLEN];
#elif defined(CRYPTO_ECDH_CURVE25519)
typedef union {
#endif
} CRYPTO_ecdhPrivateKey_t;
#pragma GCC diagnostic pop

/* Button related functions */
typedef void (*button_cb)(uint8_t button);

bool zbPlatButtonInit(uint8_t num_buttons, button_cb cb);
uint32_t zbPlatButtonGetState(void);

/* Led related functions */
bool zbPlatLedInit(uint8_t num_leds);
void zbPlatLedSetState(uint8_t led, uint8_t state);
uint8_t zbPlatLedGetStates();

/* Console related functions */
bool zbPlatConsoleInit(void);
bool zbPlatConsoleReceiveChar(uint8_t *ch);
bool zbPlatConsoleCanTransmit(void);
bool zbPlatConsoleTransmit(uint8_t pu8Data);
void zbPlatConsoleSetBaudRate(uint32_t baud);

/* Uart related functions */
bool zbPlatUartInit(void *device);
bool zbPlatUartSetBaudRate(uint32_t baud);
bool zbPlatUartCanTransmit(void);
bool zbPlatUartTransmit(uint8_t ch);
bool zbPlatUartReceiveChar(uint8_t *ch);
bool zbPlatUartReceiveBuffer(uint8_t* buffer, uint32_t *length);
void zbPlatUartFree(void);

/* Time related functions */
uint32_t zbPlatGetTime(void);

/* Watchdog related functions */
/* Registration for callbacks */
void zbPlatWdogIntRegisterPrologue(int (*fp)(void *));
void zbPlatWdogIntRegisterEpilogue(int (*fp)(void *));
void zbPlatWdogRegisterResetCheckCallback(int (*fp)(void *));

/* General management functions */
void zbPlatWdogInit(void);
void zbPlatWdogKick(void);
void zbPlatWdogDeInit(void);

/* Crypto related functions */
void zbPlatCryptoInit(void);
void zbPlatCryptoAesHmacMmo(uint8_t *pu8Data, int iDataLen, void *key, void *hash);
void zbPlatCryptoAesMmoBlockUpdate(void *hash, void *block);
void zbPlatCryptoAesMmoFinalUpdate(void *hash, uint8_t *pu8Data, int iDataLen, int iFinalLen);
bool_t zbPlatCryptoAesSetKey(CRYPTO_tsReg128 *psKeyData);
void zbPlatCryptoAes128EcbEncrypt(const uint8_t* pu8Input, uint32_t u32InputLen, const uint8_t* pu8Key, uint8_t* pu8Output);
void zbPlatCryptoAesDecrypt(const uint8_t* pu8Input, const uint8_t* pu8Key, uint8_t* pu8Output);
void zbPlatCryptoAesCcmStar(bool_t bEncrypt, uint8_t u8M, uint8_t  u8AuthLen, uint8_t u8InputLen, CRYPTO_tsAesBlock *puNonce,
        uint8_t *pu8AuthData, uint8_t *pu8Input, uint8_t *pu8ChecksumData, bool_t *pbChecksumVerify);
uint8_t zbPlatCryptoRandomInit(void);
uint32_t zbPlatCryptoRandomGet(uint32_t u32Min, uint32_t u32Max);
uint32_t zbPlatCryptoRandom256Get(void);
void zbPlatCryptoDeInit(void);
bool_t zbPlatCryptoEcdhGenerateKeys(CRYPTO_ecdhPublicKey_t *psPublicKey, CRYPTO_ecdhPrivateKey_t *psSecretKey);
bool_t zbPlatCryptoEcdhComputeDhKey(CRYPTO_ecdhPrivateKey_t *psSecretKey, CRYPTO_ecdhPublicKey_t *psPeerPublicKey, CRYPTO_ecdhDhKey_t *psOutEcdhKey);

/* Boot-time functions */
void zbPlatWdogResetCheckSource(void);

#endif
