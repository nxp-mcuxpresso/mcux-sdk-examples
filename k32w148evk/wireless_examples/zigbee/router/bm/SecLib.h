/*! *********************************************************************************
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2022 NXP
 * All rights reserved.
 *
 * \file
 *
 * This is the header file for the security module.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ********************************************************************************** */

#ifndef SEC_LIB_H
#define SEC_LIB_H

/*!
 * @addtogroup SecLib_module
 * The SecLib_module
 *
 * SecLib_module provides APIs a collection of security features.
 * @{
 */
/*!
 * @addtogroup SecLib
 * The SecLib main module
 *
 * SecLib provides APIs a collection of security features.
 * @{
 */

/*! *********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */
#include "EmbeddedTypes.h"
#include <stdint.h>

/*! *********************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
********************************************************************************** */
#define BITLEN2BYTELEN(x) (((x) + 7u) >> 3)

#define AES_128_KEY_BITS     128u
#define AES_128_KEY_BYTE_LEN BITLEN2BYTELEN(AES_128_KEY_BITS)
#define AES_192_KEY_BITS     192u
#define AES_192_KEY_BYTE_LEN BITLEN2BYTELEN(AES_192_KEY_BITS)
#define AES_256_KEY_BITS     256u
#define AES_256_KEY_BYTE_LEN BITLEN2BYTELEN(AES_256_KEY_BITS)

#define AES_128_BLOCK_SIZE 16u /* [bytes] */

#define BLOB_DATA_OVERLAY_BYTE_LEN 24U

/* CCM */
#define gSecLib_CCM_Encrypt_c 0u
#define gSecLib_CCM_Decrypt_c 1u

#define AES_BLOCK_SIZE 16u /* [bytes] */
#define AESSW_BLK_SIZE (AES_BLOCK_SIZE)

/* Hashes */
#define SHA1_HASH_SIZE  20u /* [bytes] */
#define SHA1_BLOCK_SIZE 64u /* [bytes] */

#define SHA256_HASH_SIZE  32u /* [bytes] */
#define SHA256_BLOCK_SIZE 64u /* [bytes] */

#define gHmacIpad_c 0x36u
#define gHmacOpad_c 0x5Cu

/*! Enable or disable AES-OFB functionality in the SecLib module. */
#ifndef gSecLibAesOfbEnable_d
#define gSecLibAesOfbEnable_d 0
#endif

/*! Enable or disable AES-EAX functionality in the SecLib module. */
#ifndef gSecLibAesEaxEnable_d
#define gSecLibAesEaxEnable_d 0
#endif

/*! Enable or disable SHA1 functionality in the SecLib module. */
#ifndef gSecLibSha1Enable_d
#define gSecLibSha1Enable_d 0
#endif

/*! Number of bytes in an S200 blob */
#define gSecLibElkeBlobSize_c 40U

/*! Number of bytes in an EIRK */
#define gSecLibEirkBlobSize_c 16U

/*! *********************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
********************************************************************************** */
typedef enum
{
    gSecSuccess_c          = 0u,
    gSecAllocError_c       = 1u,
    gSecError_c            = 2u,
    gSecInvalidPublicKey_c = 3u,
    gSecResultPending_c    = 4u
} secResultType_t;

/* Security block definition */
typedef union
{
    uint8_t  au8[AES_BLOCK_SIZE];
    uint32_t au32[AES_BLOCK_SIZE / 4];
} tuAES_Block;

/* For backwards compatibility */
typedef tuAES_Block AESSW_Block_u;

typedef struct
{
    uint32_t u32register0;
    uint32_t u32register1;
    uint32_t u32register2;
    uint32_t u32register3;
} tsReg128;

/*! The type of the key used in A2B protocol */
typedef enum
{
    gSecPlainText_c   = 0u, /*<! Input key type is plaintext  */
    gSecElkeBlob_c    = 1u, /*<! Input key type is ELKE Blob type (when S200 is present)  */
    gSecLtkElkeBlob_c = 2u  /*<! Input key type is LTK Blob type (when S200 is present) */
} secInputKeyType_t;

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
 * \brief  This function performs initialization of the cryptographic HW acceleration.
 *
 ********************************************************************************** */
void SecLib_Init(void);

/*! *********************************************************************************
 * \brief  This function performs initialization of the cryptografic HW acceleration.
 *
 ********************************************************************************** */
void SecLib_ReInit(void);

/*! *********************************************************************************
 * \brief  This function will allow reinitizialize the cryptographic HW acceleration
 * next time we need it, typically after lowpower mode.
 *
 ********************************************************************************** */
void SecLib_DeInit(void);

/*! *********************************************************************************
 * \brief  This function performs AES-128 encryption on a 16-byte block.
 *
 * \param[in]  pInput Pointer to the location of the 16-byte plain text block.
 *
 * \param[in]  pKey Pointer to the location of the 128-bit key.
 *
 * \param[out]  pOutput Pointer to the location to store the 16-byte ciphered output.
 *
 * \pre All Input/Output pointers must refer to a memory address aligned to 4 bytes!
 *
 ********************************************************************************** */
void AES_128_Encrypt(const uint8_t *pInput, const uint8_t *pKey, uint8_t *pOutput);

/*! *********************************************************************************
 * \brief  This function performs AES-128 decryption on a 16-byte block.
 *
 * \param[in]  pInput Pointer to the location of the 16-byte plain text block.
 *
 * \param[in]  pKey Pointer to the location of the 128-bit key.
 *
 * \param[out]  pOutput Pointer to the location to store the 16-byte ciphered output.
 *
 * \pre All Input/Output pointers must refer to a memory address aligned to 4 bytes!
 *
 ********************************************************************************** */
void AES_128_Decrypt(const uint8_t *pInput, const uint8_t *pKey, uint8_t *pOutput);

/*! *********************************************************************************
 * \brief  This function performs AES-128-ECB encryption on a message block.
 *         This function only accepts input lengths which are multiple
 *         of 16 bytes (AES 128 block size).
 *
 * \param[in]  pInput Pointer to the location of the input message.
 *
 * \param[in]  inputLen Input message length in bytes.
 *
 * \param[in]  pKey Pointer to the location of the 128-bit key.
 *
 * \param[out]  pOutput Pointer to the location to store the ciphered output.
 *
 ********************************************************************************** */
void AES_128_ECB_Encrypt(const uint8_t *pInput, uint32_t inputLen, const uint8_t *pKey, uint8_t *pOutput);

/*! *********************************************************************************
 * \brief  This function performs AES-128-CBC encryption on a message block.
 *         This function only accepts input lengths which are multiple
 *         of 16 bytes (AES 128 block size).
 *
 * \param[in]  pInput Pointer to the location of the input message.
 *
 * \param[in]  inputLen Input message length in bytes.
 *
 * \param[in]  pInitVector Pointer to the location of the 128-bit initialization vector.
 *
 * \param[in]  pKey Pointer to the location of the 128-bit key.
 *
 * \param[out]  pOutput Pointer to the location to store the ciphered output.
 *
 ********************************************************************************** */
void AES_128_CBC_Encrypt(
    const uint8_t *pInput, uint32_t inputLen, uint8_t *pInitVector, const uint8_t *pKey, uint8_t *pOutput);

/*! *********************************************************************************
 * \brief  This function performs AES-128-CBC encryption on a message block after
 *         padding it with 1 bit of 1 and 0 bits trail.
 *
 * \param[in]  pInput Pointer to the location of the input message.
 *
 * \param[in]  inputLen Input message length in bytes.
 *
 *             IMPORTANT: User must make sure that input and output
 *             buffers have at least inputLen + 16 bytes size
 *
 * \param[in]  pInitVector Pointer to the location of the 128-bit initialization vector.
 *
 * \param[in]  pKey Pointer to the location of the 128-bit key.
 *
 * \param[out] pOutput Pointer to the location to store the ciphered output.
 *
 * \return     uint32_t size of output buffer (after padding)
 *
 ********************************************************************************** */
uint32_t AES_128_CBC_Encrypt_And_Pad(
    uint8_t *pInput, uint32_t inputLen, uint8_t *pInitVector, const uint8_t *pKey, uint8_t *pOutput);

/*! *********************************************************************************
 * \brief  This function performs AES-128-CBC decryption on a message block.
 *
 * \param[in]  pInput Pointer to the location of the input message.
 *
 * \param[in]  inputLen Input message length in bytes.
 *
 * \param[in]  pInitVector Pointer to the location of the 128-bit initialization vector.
 *
 * \param[in]  pKey Pointer to the location of the 128-bit key.
 *
 * \param[out] pOutput Pointer to the location to store the ciphered output.
 *
 * \return     uint32_t size of output buffer (after depadding the 0x80 0x00 ... padding sequence)
 *
 ********************************************************************************** */
uint32_t AES_128_CBC_Decrypt_And_Depad(
    const uint8_t *pInput, uint32_t inputLen, uint8_t *pInitVector, const uint8_t *pKey, uint8_t *pOutput);

/*! *********************************************************************************
 * \brief  This function performs AES-128-CTR encryption on a message block.
 *         This function only accepts input lengths which are multiple
 *         of 16 bytes (AES 128 block size).
 *
 * \param[in]  pInput Pointer to the location of the input message.
 *
 * \param[in]  inputLen Input message length in bytes.
 *
 * \param[in]  pCounter Pointer to the location of the 128-bit counter.
 *
 * \param[in]  pKey Pointer to the location of the 128-bit key.
 *
 * \param[out]  pOutput Pointer to the location to store the ciphered output.
 *
 ********************************************************************************** */
void AES_128_CTR(const uint8_t *pInput, uint32_t inputLen, uint8_t *pCounter, const uint8_t *pKey, uint8_t *pOutput);

#if gSecLibAesOfbEnable_d
/*! *********************************************************************************
 * \brief  This function performs AES-128-OFB encryption on a message block.
 *         This function only accepts input lengths which are multiple
 *         of 16 bytes (AES 128 block size).
 *
 * \param[in]  pInput Pointer to the location of the input message.
 *
 * \param[in]  inputLen Input message length in bytes.
 *
 * \param[in]  pInitVector Pointer to the location of the 128-bit initialization vector.
 *
 * \param[in]  pKey Pointer to the location of the 128-bit key.
 *
 * \param[out]  pOutput Pointer to the location to store the ciphered output.
 *
 ********************************************************************************** */
void AES_128_OFB(
    const uint8_t *pInput, const uint32_t inputLen, uint8_t *pInitVector, const uint8_t *pKey, uint8_t *pOutput);
#endif /* gSecLibAesOfbEnable_d */

/*! *********************************************************************************
 * \brief  This function performs AES-128-CMAC on a message block.
 *
 * \param[in]  pInput Pointer to the location of the input message.
 *
 * \param[in]  inputLen Length of the input message in bytes. The input data must be provided MSB first.
 *
 * \param[in]  pKey Pointer to the location of the 128-bit key. The key must be provided MSB first.
 *
 * \param[out]  pOutput Pointer to the location to store the 16-byte authentication code. The code will be generated MSB
 *first.
 *
 * \remarks This is public open source code! Terms of use must be checked before use!
 *
 ********************************************************************************** */
void AES_128_CMAC(const uint8_t *pInput, const uint32_t inputLen, const uint8_t *pKey, uint8_t *pOutput);

/*! *********************************************************************************
 * \brief  This function performs AES-128-CMAC on a message block accepting input data
 *         which is in LSB first format and computing the authentication code starting from the end of the data.
 *
 * \param[in]  pInput Pointer to the location of the input message.
 *
 * \param[in]  inputLen Length of the input message in bytes. The input data must be provided LSB first.
 *
 * \param[in]  pKey Pointer to the location of the 128-bit key. The key must be provided MSB first.
 *
 * \param[out]  pOutput Pointer to the location to store the 16-byte authentication code. The code will be generated MSB
 *first.
 *
 ********************************************************************************** */
void AES_128_CMAC_LsbFirstInput(const uint8_t *pInput, uint32_t inputLen, const uint8_t *pKey, uint8_t *pOutput);

/*! *********************************************************************************
 * \brief  This function performs AES 128 CMAC Pseudo-Random Function (AES-CMAC-PRF-128),
 *         according to rfc4615, on a message block.
 *
 * \details The AES-CMAC-PRF-128 algorithm behaves similar to the AES CMAC 128 algorithm
 *          but removes 128 bit key size restriction.
 *
 * \param[in]  pInput Pointer to the location of the input message.
 *
 * \param[in]  inputLen Length of the input message in bytes.
 *
 * \param[in]  pVarKey Pointer to the location of the variable length key.
 *
 * \param[in]  varKeyLen Length of the input key in bytes
 *
 * \param[out]  pOutput Pointer to the location to store the 16-byte pseudo random variable.
 *
 ********************************************************************************** */
void AES_CMAC_PRF_128(
    const uint8_t *pInput, uint32_t inputLen, const uint8_t *pVarKey, uint32_t varKeyLen, uint8_t *pOutput);

#if gSecLibAesEaxEnable_d
/*! *********************************************************************************
 * \brief  This function performs AES-128-EAX encryption on a message block.
 *
 * \param[in]  pInput Pointer to the location of the input message.
 *
 * \param[in]  inputLen Length of the input message in bytes.
 *
 * \param[in]  pNonce Pointer to the location of the nonce.
 *
 * \param[in]  nonceLen Nonce length in bytes.
 *
 * \param[in]  pHeader Pointer to the location of header.
 *
 * \param[in]  headerLen Header length in bytes.
 *
 * \param[in]  pKey Pointer to the location of the 128-bit key.
 *
 * \param[out]  pOutput Pointer to the location to store the 16-byte authentication code.
 *
 * \param[out]  pTag Pointer to the location to store the 128-bit tag.
 *
 ********************************************************************************** */
secResultType_t AES_128_EAX_Encrypt(const uint8_t *pInput,
                                    uint32_t       inputLen,
                                    const uint8_t *pNonce,
                                    uint32_t       nonceLen,
                                    const uint8_t *pHeader,
                                    uint8_t        headerLen,
                                    const uint8_t *pKey,
                                    uint8_t *      pOutput,
                                    uint8_t *      pTag);

/*! *********************************************************************************
 * \brief  This function performs AES-128-EAX decryption on a message block.
 *
 * \param[in]  pInput Pointer to the location of the input message.
 *
 * \param[in]  inputLen Length of the input message in bytes.
 *
 * \param[in]  pNonce Pointer to the location of the nonce.
 *
 * \param[in]  nonceLen Nonce length in bytes.
 *
 * \param[in]  pHeader Pointer to the location of header.
 *
 * \param[in]  headerLen Header length in bytes.
 *
 * \param[in]  pKey Pointer to the location of the 128-bit key.
 *
 * \param[out]  pOutput Pointer to the location to store the 16-byte authentication code.
 *
 * \param[out]  pTag Pointer to the location to store the 128-bit tag.
 *
 ********************************************************************************** */
secResultType_t AES_128_EAX_Decrypt(const uint8_t *pInput,
                                    uint32_t       inputLen,
                                    const uint8_t *pNonce,
                                    uint32_t       nonceLen,
                                    const uint8_t *pHeader,
                                    uint8_t        headerLen,
                                    const uint8_t *pKey,
                                    uint8_t *      pOutput,
                                    uint8_t *      pTag);
#endif

/*! *********************************************************************************
 * \brief  This function performs AES-128-CCM on a message block.
 *
 * \param[in]  pInput       Pointer to the location of the input message (plaintext or cyphertext).
 *
 * \param[in]  inputLen     Length of the input plaintext in bytes when encrypting.
 *                          Length of the input cyphertext without the MAC length when decrypting.
 *
 * \param[in]  pAuthData    Pointer to the additional authentication data.
 *
 * \param[in]  authDataLen  Length of additional authentication data.
 *
 * \param[in]  pNonce       Pointer to the Nonce.
 *
 * \param[in]  nonceSize    The size of the nonce (7-13).
 *
 * \param[in]  pKey         Pointer to the location of the 128-bit key.
 *
 * \param[out]  pOutput     Pointer to the location to store the plaintext data when encrypting.
 *                          Pointer to the location to store the cyphertext data when encrypting.
 *
 * \param[out]  pCbcMac     Pointer to the location to store the Message Authentication Code (MAC) when encrypting.
 *                          Pointer to the location where the received MAC can be found when decrypting.
 *
 * \param[out]  macSize     The size of the MAC.
 *
 * \param[out]  flags       Select encrypt/decrypt operations (gSecLib_CCM_Encrypt_c, gSecLib_CCM_Decrypt_c)
 *
 * \return      uint8_t     error status.
 ********************************************************************************** */
uint8_t AES_128_CCM(const uint8_t *pInput,
                    uint16_t       inputLen,
                    const uint8_t *pAuthData,
                    uint16_t       authDataLen,
                    const uint8_t *pNonce,
                    uint8_t        nonceSize,
                    const uint8_t *pKey,
                    uint8_t *      pOutput,
                    uint8_t *      pCbcMac,
                    uint8_t        macSize,
                    uint32_t       flags);

#if gSecLibSha1Enable_d
/*! *********************************************************************************
 * \brief  This function allocates a memory buffer for a SHA1 context structure
 *
 * \return    Address of the SHA1 context buffer
 *            Deallocate using SHA1_FreeCtx()
 *
 ********************************************************************************** */
void *SHA1_AllocCtx(void);

/*! *********************************************************************************
 * \brief  This function deallocates the memory buffer for the SHA1 context structure
 *
 * \param [in]    pContext    Address of the SHA1 context buffer
 *
 ********************************************************************************** */
void SHA1_FreeCtx(void *pContext);

/*! *********************************************************************************
 * \brief  This function clones a SHA1 context.
 *         Make sure the size of the allocated destination context buffer is appropriate.
 *
 * \param [in]    pDestCtx    Address of the destination SHA1 context
 * \param [in]    pSourceCtx  Address of the source SHA1 context
 *
 ********************************************************************************** */
void SHA1_CloneCtx(void *pDestCtx, void *pSourceCtx);

/*! *********************************************************************************
 * \brief  This function initializes the SHA1 context data
 *
 * \param [in]    pContext    Pointer to the SHA1 context data
 *                            Allocated using SHA1_AllocCtx()
 *
 ********************************************************************************** */
void SHA1_Init(void *pContext);

/*! *********************************************************************************
 * \brief  This function performs SHA1 on multiple bytes and updates the context data
 *
 * \param [in]    pContext    Pointer to the SHA1 context data
 *                            Allocated using SHA1_AllocCtx()
 * \param [in]    pData       Pointer to the input data
 * \param [in]    numBytes    Number of bytes to hash
 *
 ********************************************************************************** */
void SHA1_HashUpdate(void *pContext, uint8_t *pData, uint32_t numBytes);

/*! *********************************************************************************
 * \brief  This function finalizes the SHA1 hash computation and clears the context data.
 *         The final hash value is stored at the provided output location.
 *
 * \param [in]       pContext    Pointer to the SHA1 context data
 *                               Allocated using SHA1_AllocCtx()
 * \param [out]      pOutput     Pointer to the output location
 *
 ********************************************************************************** */
void SHA1_HashFinish(void *pContext, uint8_t *pOutput);

/*! *********************************************************************************
 * \brief  This function performs all SHA1 steps on multiple bytes: initialize,
 *         update and finish.
 *         The final hash value is stored at the provided output location.
 *
 * \param [in]       pData       Pointer to the input data
 * \param [in]       numBytes    Number of bytes to hash
 * \param [out]      pOutput     Pointer to the output location
 *
 ********************************************************************************** */
void SHA1_Hash(uint8_t *pData, uint32_t numBytes, uint8_t *pOutput);

#endif

/*! *********************************************************************************
 * \brief  This function allocates a memory buffer for a SHA256 context structure
 *
 * \return    Address of the SHA256 context buffer
 *            Deallocate using SHA256_FreeCtx()
 *
 ********************************************************************************** */
void *SHA256_AllocCtx(void);

/*! *********************************************************************************
 * \brief  This function deallocates the memory buffer for the SHA256 context structure
 *
 * \param [in]    pContext    Address of the SHA256 context buffer
 *
 ********************************************************************************** */
void SHA256_FreeCtx(void *pContext);

/*! *********************************************************************************
 * \brief  This function clones a SHA256 context.
 *         Make sure the size of the allocated destination context buffer is appropriate.
 *
 * \param [in]    pDestCtx    Address of the destination SHA256 context
 * \param [in]    pSourceCtx  Address of the source SHA256 context
 *
 ********************************************************************************** */
void SHA256_CloneCtx(void *pDestCtx, void *pSourceCtx);

/*! *********************************************************************************
 * \brief  This function initializes the SHA256 context data
 *
 * \param [in]    pContext    Pointer to the SHA256 context data
 *                            Allocated using SHA256_AllocCtx()
 *
 ********************************************************************************** */
void SHA256_Init(void *pContext);

/*! *********************************************************************************
 * \brief  This function performs SHA256 on multiple bytes and updates the context data
 *
 * \param [in]    pContext    Pointer to the SHA256 context data
 *                            Allocated using SHA256_AllocCtx()
 * \param [in]    pData       Pointer to the input data
 * \param [in]    numBytes    Number of bytes to hash
 *
 ********************************************************************************** */
void SHA256_HashUpdate(void *pContext, const uint8_t *pData, uint32_t numBytes);

/*! *********************************************************************************
 * \brief  This function finalizes the SHA256 hash computation and clears the context data.
 *         The final hash value is stored at the provided output location.
 *
 * \param [in]       pContext    Pointer to the SHA256 context data
 *                               Allocated using SHA256_AllocCtx()
 * \param [out]      pOutput     Pointer to the output location
 *
 ********************************************************************************** */
void SHA256_HashFinish(void *pContext, uint8_t *pOutput);

/*! *********************************************************************************
 * \brief  This function performs all SHA256 steps on multiple bytes: initialize,
 *         update and finish.
 *         The final hash value is stored at the provided output location.
 *
 * \param [in]       pData       Pointer to the input data
 * \param [in]       numBytes    Number of bytes to hash
 * \param [out]      pOutput     Pointer to the output location
 *
 ********************************************************************************** */
void SHA256_Hash(const uint8_t *pData, uint32_t numBytes, uint8_t *pOutput);

/*! *********************************************************************************
 * \brief  This function allocates a memory buffer for a HMAC SHA256 context structure
 *
 * \return    Address of the HMAC SHA256 context buffer
 *            Deallocate using HMAC_SHA256_FreeCtx()
 *
 ********************************************************************************** */
void *HMAC_SHA256_AllocCtx(void);

/*! *********************************************************************************
 * \brief  This function deallocates the memory buffer for the HMAC SHA256 context structure
 *
 * \param [in]    pContext    Address of the HMAC SHA256 context buffer
 *
 ********************************************************************************** */
void HMAC_SHA256_FreeCtx(void *pContext);

/*! *********************************************************************************
 * \brief  This function performs the initialization of the HMAC SHA256 context data
 *
 * \param [in]    pContext    Pointer to the HMAC SHA256 context data
 *                            Allocated using HMAC_SHA256_AllocCtx()
 * \param [in]    pKey        Pointer to the HMAC key
 * \param [in]    keyLen      Length of the HMAC key in bytes
 *
 ********************************************************************************** */
void HMAC_SHA256_Init(void *pContext, const uint8_t *pKey, uint32_t keyLen);

/*! *********************************************************************************
 * \brief  This function performs HMAC update with the input data.
 *
 * \param [in]    pContext    Pointer to the HMAC SHA256 context data
 *                            Allocated using HMAC_SHA256_AllocCtx()
 * \param [in]    pData       Pointer to the input data
 * \param [in]    numBytes    Number of bytes to hash
 *
 ********************************************************************************** */
void HMAC_SHA256_Update(void *pContext, const uint8_t *pData, uint32_t numBytes);

/*! *********************************************************************************
 * \brief  This function finalizes the HMAC SHA256 computation and clears the context data.
 *         The final hash value is stored at the provided output location.
 *
 * \param [in]       pContext    Pointer to the HMAC SHA256 context data
 *                               Allocated using HMAC_SHA256_AllocCtx()
 * \param [out]      pOutput     Pointer to the output location
 *
 ********************************************************************************** */
void HMAC_SHA256_Finish(void *pContext, uint8_t *pOutput);

/*! *********************************************************************************
 * \brief  This function performs all HMAC SHA256 steps on multiple bytes: initialize,
 *         update, finish, and update context data.
 *         The final HMAC value is stored at the provided output location.
 *
 * \param [in]       pKey        Pointer to the HMAC key
 * \param [in]       keyLen      Length of the HMAC key in bytes
 * \param [in]       pData       Pointer to the input data
 * \param [in]       numBytes    Number of bytes to perform HMAC on
 * \param [out]      pOutput     Pointer to the output location
 *
 ********************************************************************************** */
void HMAC_SHA256(const uint8_t *pKey, uint32_t keyLen, const uint8_t *pData, uint32_t numBytes, uint8_t *pOutput);

/*! *********************************************************************************
 * \brief  This function calculates XOR of individual byte pairs in two uint8_t arrays.
 *         pDst[i] := pDst[i] ^ pSrc[i] for i=0 to n-1
 *
 * \param[in, out]  pDst First byte array operand for XOR and destination byte array
 *
 * \param[in]  pSrc Second byte array operand for XOR (read-only)
 *
 * \param[in]  n  Length of the byte arrays which will be XORed
 *
 ********************************************************************************** */
void SecLib_XorN(uint8_t *      pDst, /* First operand and result of XOR operation */
                 const uint8_t *pSrc, /* Second operand. Not modified. */
                 uint8_t        n);          /* Number of bytes in input arrays. */

/************************************************************************************
 * \brief Function used to create the mac key and LTK using Bluetooth F5 algorithm.
 *        Version using plaintext key.
 *
 * \param  [out] pMacKey 128 bit MacKey output location (pointer)
 * \param  [out] pLtk    128 bit LTK output location (pointer)
 * \param  [in] pW       256 bit W (pointer) (DHKey)
 * \param  [in] pN1      128 bit N1 (pointer) (Na)
 * \param  [in] pN2      128 bit N2 (pointer) (Nb)
 * \param  [in] a1at     8 bit A1 address type, 0 = Public, 1 = Random
 * \param  [in] pA1      48 bit A1 (pointer) (A)
 * \param  [in] a2at     8 bit A2 address type, 0 = Public, 1 = Random
 * \param  [in] pA2      48 bit A2 (pointer) (B)
 *
 * \retval gSecSuccess_c operation succeeded
 * \retval gSecError_c operation failed
 ************************************************************************************/
secResultType_t SecLib_GenerateBluetoothF5Keys(uint8_t *      pMacKey,
                                               uint8_t *      pLtk,
                                               const uint8_t *pW,
                                               const uint8_t *pN1,
                                               const uint8_t *pN2,
                                               const uint8_t  a1at,
                                               const uint8_t *pA1,
                                               const uint8_t  a2at,
                                               const uint8_t *pA2);

/************************************************************************************
 * \brief Function used to create the mac key and LTK using Bluetooth F5 algorithm.
 *        Version using key blobs and secure bus. Available on EdgeLock only.
 *
 * \param  [out] pMacKey 128 bit MacKey output location (pointer)
 * \param  [out] pLtk    128 bit LTK output location (pointer)
 * \param  [in] pW       256 bit W (pointer) (DHKey)
 * \param  [in] pN1      128 bit N1 (pointer) (Na)
 * \param  [in] pN2      128 bit N2 (pointer) (Nb)
 * \param  [in] a1at     8 bit A1 address type, 0 = Public, 1 = Random
 * \param  [in] pA1      48 bit A1 (pointer) (A)
 * \param  [in] a2at     8 bit A2 address type, 0 = Public, 1 = Random
 * \param  [in] pA2      48 bit A2 (pointer) (B)
 *
 * \retval gSecSuccess_c operation succeeded
 * \retval gSecError_c operation failed
 ************************************************************************************/
secResultType_t SecLib_GenerateBluetoothF5KeysSecure(uint8_t *      pMacKey,
                                                     uint8_t *      pLtk,
                                                     const uint8_t *pW,
                                                     const uint8_t *pN1,
                                                     const uint8_t *pN2,
                                                     const uint8_t  a1at,
                                                     const uint8_t *pA1,
                                                     const uint8_t  a2at,
                                                     const uint8_t *pA2);

/************************************************************************************
 * \brief Function used to derive the Bluetooth SKD used in LL encryption
 *        Available on EdgeLock only.
 *
 * \param  [in] pInSKD   pointer to the received SKD (16-byte array)
 * \param  [in] pLtkBlob pointer to the blob (40-byte array)
 * \param  [in] bOpenKey specify if the EdgeLock key shall be regenerated
 * \param  [out] pOutSKD pointer to the resulted SKD (16-byte array)
 *
 * \retval gSecSuccess_c operation succeeded
 * \retval gSecError_c operation failed
 ************************************************************************************/
secResultType_t SecLib_DeriveBluetoothSKDSecure(const uint8_t *pInSKD,
                                                const uint8_t *pLtkBlob,
                                                bool_t         bOpenKey,
                                                uint8_t *      pOutSKD);

/************************************************************************************
 * \brief Converts a plaintext symmetric key into a blob of blobType. Reverses key beforehand.
 *
 * \param[in]  pKey      Pointer to the key.
 *
 * \param[out] pBlob     Pointer to the blob (shall be allocated, 40 or 16, depending on blobType)
 *
 * \param[in]  blobType  Blob type.
 *
 * \return gSecSuccess_c or error
 *
 ************************************************************************************/
secResultType_t SecLib_ObfuscateKeySecure(const uint8_t *pKey, uint8_t *pBlob, const uint8_t blobType);

/************************************************************************************
 * \brief Converts a blob of a symmetric key into the plaintext. Reverses key afterwards.
 *
 * \param[in]  pBlob    Pointer to the blob.
 *
 * \param[out] pKey     Pointer to the key.
 *
 * \return gSecSuccess_c or error
 *
 ************************************************************************************/
secResultType_t SecLib_DeobfuscateKeySecure(const uint8_t *pBlob, uint8_t *pKey);

/*! *********************************************************************************
 * \brief  This function implements the SMP ah cryptographic toolbox function which calculates the
 *         hash part of a Resolvable Private Address.
 *         Key kept as blob.
 *
 * \param[out]  pHash  Pointer where the 24 bit hash value will be written.
 *                     24 bit hash field of a Resolvable Private Address (output)
 *
 * \param[in]  pKey  Pointer to the 128 bit key.
 *
 * \param[in]  pR   Pointer to the 24 bit random value (Prand).
 *                  The most significant bits of this field must be 0b01 for Resolvable Private Addresses.
 *
 * \retval  gSecSuccess_c  All operations were successful.
 * \retval  gSecError_c The call failed.
 *
 ********************************************************************************** */
secResultType_t SecLib_VerifyBluetoothAhSecure(uint8_t *pHash, const uint8_t *pKey, const uint8_t *pR);

/*! *********************************************************************************
 * \brief  This function implements the SMP ah cryptographic toolbox function which calculates the
 *         hash part of a Resolvable Private Address.
 *         Key kept in plain text.
 *
 * \param[out]  pHash  Pointer where the 24 bit hash value will be written.
 *                     24 bit hash field of a Resolvable Private Address (output)
 *
 * \param[in]  pKey  Pointer to the 128 bit key.
 *
 * \param[in]  pR   Pointer to the 24 bit random value (Prand).
 *                  The most significant bits of this field must be 0b01 for Resolvable Private Addresses.
 *
 * \retval  gSecSuccess_c  All operations were successful.
 * \retval  gSecError_c The call failed.
 *
 ********************************************************************************** */
secResultType_t SecLib_VerifyBluetoothAh(uint8_t *pHash, const uint8_t *pKey, const uint8_t *pR);

/************************************************************************************
 * \brief Generates a symmetric key in ELKE blob or plain text form.
 *        Only implemented on EdgeLock.
 *
 * \param[in]  keySize the size of the generated key.
 *
 * \param[in] blobOutput true - blob, false - plaintext output.
 *
 * \param[out] pOut   the address of the buffer to store the key.
 *                    Storage for sss_sscp_object_t key reference
 *
 * \return gSecSuccess_c or error
 *
 ************************************************************************************/
secResultType_t SecLib_GenerateSymmetricKey(const uint32_t keySize, const bool_t blobOutput, void *pOut);

/************************************************************************************
 * \brief Generates an EIRK blob from an ELKE blob or plain text symmetric key.
 *        Only implemented on EdgeLock.
 *
 * \param[in]  pIRK pointer to the input IRK key.
 *
 * \param[in] blobInput true - pIRK points to an ELKE blob, false - pIRK points to a plain text key.
 *
 * \param[in] generateDKeyIRK true - DKeyIRK is slso generated and provided to NBU.
 *
 * \param[out] pOutEIRKblob   the address of the buffer to store the EIRK blob.
 *
 * \return gSecSuccess_c or error
 *
 ************************************************************************************/
secResultType_t SecLib_GenerateBluetoothEIRKBlobSecure(const void * pIRK,
                                                       const bool_t blobInput,
                                                       const bool_t generateDKeyIRK,
                                                       uint8_t *    pOutEIRKblob);

/************************************************************************************
 * \brief Generates an E2E blob from an ELKE blob or plain text symmetric key.
 *        Only implemented on EdgeLock.
 *
 * \param[in]  pKey      pointer to the input key.
 * \param[in]  keyType   input key type.
 * \param[out] pOutKey   pointer to where the output E2E blob will be copied.
 *
 * \return gSecSuccess_c or error
 *
 ************************************************************************************/
secResultType_t SecLib_ExportA2BBlobSecure(const void *pKey, const secInputKeyType_t keyType, uint8_t *pOutKey);

/************************************************************************************
 * \brief Generates a symmetric key in ELKE blob or plain text form from an E2E blob.
 *        Only implemented on EdgeLock.
 *
 * \param[in]  pKey      pointer to the input E2E blob.
 * \param[in]  keyType   output key type.
 * \param[out] pOutKey   pointer to where the output key will be copied.
 *
 * \return gSecSuccess_c or error
 *
 ************************************************************************************/
secResultType_t SecLib_ImportA2BBlobSecure(const uint8_t *pKey, const secInputKeyType_t keyType, uint8_t *pOutKey);

/*!
 * @}  end of SecLib addtogroup
 */
/*!
 * @}  end of SecLib_module addtogroup
 */

#include "SecLib_ecp256.h"

#endif /* SEC_LIB_H */
