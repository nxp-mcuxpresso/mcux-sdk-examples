/*! *********************************************************************************
 * Copyright 2022-2023 NXP
 * All rights reserved.
 *
 * \file
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ********************************************************************************** */

#ifndef __SECLIB_ECP256_H__
#define __SECLIB_ECP256_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "SecLib.h"

/*!
 * @addtogroup SecLib_module
 * The SecLib_module
 *
 * SecLib_module provides APIs a collection of security features.
 * @{
 */
/*!
 * @addtogroup SecLib_ecp256
 * The SecLib ecp256 module
 *
 * SecLib provides APIs a collection of ecp256 security features.
 * @{
 */

/*! *********************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
********************************************************************************** */

/*! How many steps to use for the EC multiplication procedure */
#ifndef gSecLibEcStepsAtATime
#define gSecLibEcStepsAtATime 16U
#endif

#define SEC_ECP256_COORDINATE_BITLEN 256u
#define SEC_ECP256_COORDINATE_LEN    (SEC_ECP256_COORDINATE_BITLEN / 8u)
#define SEC_ECP256_COORDINATE_WLEN   (SEC_ECP256_COORDINATE_LEN / 4u)
#define SEC_ECP256_SCALAR_LEN        32u
#define SEC_ECP256_SCALAR_WLEN       (SEC_ECP256_SCALAR_LEN / 4u)
/* The point representation requires 1 additional header byte equal to 0x04u
 * in the case of the uncompressed form */
/* Compressed representation of a ppoint P(x,y) uses the following convention:
 * 0x00: point at infinity
 * 2u:  y is even, P is represented by x and y is computed knowing y parity.
 * 3u: y is odd*/

#define SEC_ECP256_POINT_LEN ((SEC_ECP256_COORDINATE_LEN << 1) + 1)

/*! *********************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
********************************************************************************** */
typedef enum
{
    gSecEcdsaSuccess_c,
    gSecEcdsaBadParameters_c,
    gSecEcdsaInvalidState_c,
    gSecEcdsaOutOfMemory_c,
    gSecEcdsaRngError_c,
    gSecEcdsaNeutralPointError_c,
    gSecEcdsaInvalidPoint_c,
    gSecEcdsaInvalidEphemeralKey_c,
    gSecEcdsaInvalidSignature_c,
    gSecEcdsaDigestError_c,
    gSecEcdsaFailure_c
} secEcdsaStatus_t;

typedef enum
{
    gSecEcdhSuccess_c,
    gSecEcdhBadParameters_c,
    gSecEcdhOutOfMemory_c,
    gSecEcdhRngError_c,
    gSecEcdhInvalidState_c,
    gSecEcdhInvalidPublicKey_c
} secEcdhStatus_t;

typedef enum
{
    gSecEcp256Success_c,
    gSecEcp256BadParameters_c,
    gSecEcp256OutOfMemory_c,
    gSecEcp256RngError_c,
    gSecEcp256InvalidState_c,
    gSecEcp256InvalidPublicKey_c,
    gSecEcp256InvalidPrivateKey_c,
    gSecEcp256InvalidPoint_c,
    gSecEcp256InvalidScalar_c,
    gSecEcp256NeutralPoint_c
} secEcp256Status_t;

typedef union
{
    uint8_t  raw_8bit[SEC_ECP256_SCALAR_LEN];
    uint32_t raw_32bit[SEC_ECP256_SCALAR_WLEN];
} big_int256_t;

typedef big_int256_t ecp256Coordinate_t;

typedef union
{
    uint8_t raw[2 * SEC_ECP256_COORDINATE_LEN];
    struct
    {
        uint8_t x[SEC_ECP256_COORDINATE_LEN];
        uint8_t y[SEC_ECP256_COORDINATE_LEN];
    } components_8bit;
    struct
    {
        uint32_t x[SEC_ECP256_COORDINATE_WLEN];
        uint32_t y[SEC_ECP256_COORDINATE_WLEN];
    } components_32bit;
    struct
    {
        ecp256Coordinate_t X;
        ecp256Coordinate_t Y;
    } coord;
} ecp256Point_t;

typedef struct
{
    ecp256Point_t public_key;
    big_int256_t  private_key;
} ecp256KeyPair_t;

typedef big_int256_t ecdhPrivateKey_t;

typedef big_int256_t ec_p256_coordinate;

typedef ecp256Point_t ecdhPublicKey_t;
typedef ecp256Point_t ecdhDhKey_t;
typedef ecp256Point_t ecdhPoint_t;

typedef void (*pfDhKeyCallback_t)(void *pParam);

typedef struct computeDhKeyParams_tag
{
    ecdhPrivateKey_t  privateKey;    /*!< Secret */
    ecdhPublicKey_t   peerPublicKey; /*!< Peer public key */
    ecdhPoint_t       outPoint;      /*!< The resulting point */
    void *            pWorkBuffer;   /*!< Pointer to the buffer used for computation */
    uint8_t           procStep;      /*!< The step used for segmented computation */
    uint8_t           result;        /*!< The asynchronous computation result */
    pfDhKeyCallback_t pFCallback;    /*!< The function to be called when asynchronous computation completes */
    void *pMsg; /*!< Pointer to a pre-allocated message to be injected in the SM state machine when asynchronous
                   computation completes */
    bool_t   keepInternalBlob; /*!< Keep internal object foir later usage? */
    uint32_t aUserData[1];     /*!< Hold upper layer private data */
} computeDhKeyParam_t;

typedef void (*secLibCallback_t)(computeDhKeyParam_t *pData);

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
 * \brief  This function performs initialization of the callback used to offload
 * elliptic curve multiplication.
 *
 * \param[in]  pfCallback Pointer to the function used to handle multiplication.
 *
 ********************************************************************************** */
void SecLib_SetExternalMultiplicationCb(secLibCallback_t pfCallback);

/*! *********************************************************************************
 * \brief  This function performs calls the multiplication Callback.
 *
 * \param[in]  pMsg Pointer to the data used in multiplication.
 *
 ********************************************************************************** */
void SecLib_ExecMultiplicationCb(computeDhKeyParam_t *pMsg);

/************************************************************************************
 * \brief Generates a new ECDH P256 Private/Public key pair
 *
 * \return gSecSuccess_c or error
 *
 ************************************************************************************/
secResultType_t ECDH_P256_GenerateKeys(ecdhPublicKey_t *pOutPublicKey, ecdhPrivateKey_t *pOutPrivateKey);

/************************************************************************************
 * \brief Generates a new ECDH P256 Private/Public key pair. This function starts the
 *        ECDH generate procedure. The pDhKeyData must be allocated and kept
 *        allocated for the time of the computation procedure.
 *        When the result is gSecResultPending_c the memory should be kept until the
 *        last step.
 *        In any other result messages the data shall be cleared after this call.
 *
 * \param[in]  pDhKeyData Pointer to the structure holding information about the
 *                        multiplication
 *
 * \return gSecSuccess_c, gSecResultPending_c or error
 *
 ************************************************************************************/
secResultType_t ECDH_P256_GenerateKeysSeg(computeDhKeyParam_t *pDhKeyData);

/************************************************************************************
 * \brief Computes the Diffie-Hellman Key for an ECDH P256 key pair.
 *
 * \param[in]  pInPrivateKey    Pointer to the structure holding the private key
 * \param[in]  pInPeerPublicKey Pointer to the structure holding the public key
 * \param[out] pOutDhKey        Pointer to the structure holding the DH key computed
 * \param[in]  keepBlobDhKey    Specify if the shared key object is stored after this call. This
 *                              parameter is used only if gSecLibUseSecureSubSystem_d is enabled.
 *
 * \return gSecSuccess_c or error
 *
 ************************************************************************************/
secResultType_t ECDH_P256_ComputeDhKey(const ecdhPrivateKey_t *pInPrivateKey,
                                       const ecdhPublicKey_t * pInPeerPublicKey,
                                       ecdhDhKey_t *           pOutDhKey,
                                       const bool_t            keepBlobDhKey);

/************************************************************************************
 * \brief Computes the Diffie-Hellman Key for an ECDH P256 key pair. This function
 *        starts the ECDH key pair generate procedure. The pDhKeyData must be
 *        allocated and kept allocated for the time of the computation procedure.
 *        When the result is gSecResultPending_c the memory should be kept until the
 *        last step, when it can be safely freed.
 *        In any other result messages the data shall be cleared after this call.
 *
 * \param[in]  pDhKeyData Pointer to the structure holding information about the
 *                        multiplication
 *
 * \return gSecSuccess_c or error
 *
 ************************************************************************************/
secResultType_t ECDH_P256_ComputeDhKeySeg(computeDhKeyParam_t *pDhKeyData);

/************************************************************************************
 * \brief Free any data allocated in the input structure.
 *
 * \param[in]  pDhKeyData Pointer to the structure holding information about the
 *                        multiplication
 *
 * \return gSecSuccess_c or error
 *
 ************************************************************************************/
void ECDH_P256_FreeDhKeyDataSecure(computeDhKeyParam_t *pDhKeyData);

/************************************************************************************
 * \brief Computes the Edgelock to Edgelock key for an ECDH P256 key pair.
 *
 * \param[in]   pInPeerPublicKey     pointer to the public key.
 * \param[out]  pOutE2EKey           pointer where the E2E key object is stored
 *
 * \return gSecSuccess_c or error
 *
 ************************************************************************************/
secResultType_t ECDH_P256_ComputeA2BKeySecure(const ecdhPublicKey_t *pInPeerPublicKey, ecdhDhKey_t *pOutE2EKey);

/************************************************************************************
 * \brief Free E2E key object
 *
 * \param[in]  pE2EKeyData   Pointer to the E2E key data to be freed.
 *
 * \return gSecSuccess_c or error
 *
 ************************************************************************************/
secResultType_t ECDH_P256_FreeE2EKeyDataSecure(ecdhDhKey_t *pE2EKeyData);

/*! *********************************************************************************
 * \brief ECP256R1 generate public from private key passed as argument
 *
 *  See : ECP256_GenerateKeyPair
 *
 * \param[out]  pOutPublicKey   pointer on ECP256R1 public key.
 *              64 byte Storage provided by caller.
 * \param[in] pInPrivateKey   pointer on ECP256R1 private key.
 *              32 byte storage provided by caller.
 * \param[in] pMultiplicationBuffer   pointer on work buffer for multiplication buffer.
 *
 * \return  gSecEcp256Success_c if success.
 * Note:    Does not retry if private key is not suitable
 *
 ********************************************************************************** */
secEcp256Status_t ECP256_GeneratePublicKey(uint8_t *      pOutPublicKey,
                                           const uint8_t *pInPrivateKey,
                                           void *         pMultiplicationBuffer);

/****************************************************************************
 *                          ECDSA
 ***************************************************************************/

/**
 * @brief ECDSA signature generation
 *
 * Given a random 32 byte ephemeral key candidate k, a message digest e and a private key, this function
 * verifies if the ephemeral key is in range [1,n-1] and generates an ECDSA signature (r,s) with
 *
 *    r = (k*G).x mod n
 *    s = k^(-1)*(e'+r*d) mod n
 *
 * of the message digest. Here e' is derived from e by either truncating it or padding it with zeros to 32 bytes.
 *
 * NOTE: The ECDSA signature generation function expects a valid private key, no private key validation is performed.
 *       An ephemeral key is generated internally
 *
 *  @param [out] pSignature Pointer to a 64 byte buffer to where the generated signature (r,s) will be stored.
 *                          The signature is stored as the concatenation r||s with the integers r and s being in BE
 format.
 *
 * @param [in]  pDigest         Pointer to a buffer of byte length digestLen containing the message digest in BE format.
 * @param [in]  DigestLen       Byte length of the passed message digest.
 * @param [in]  pPrivKey        Pointer to a 32 byte buffer containing the private key d in range [1,n-1] in BE format.

 * @retval gSecEcdsaSuccess_c               The operation finished successfully.
 * @retval gSecEcdsaInvalidEphemeralKey_c   The passed ephemeral key is out of range.
 * @retval gSecEcdsaFailure_c                The signature generation failed and shall be repeated with a fresh
 ephemeral key.
 */
secEcdsaStatus_t ECDSA_SignFromHash(uint8_t *      pSignature,
                                    const uint8_t *pDigest,
                                    uint16_t       DigestLen,
                                    const uint8_t *pPrivKey);

/**
 * @brief ECDSA message signature generation
 *
 * For a given message computes the SHA256 digest and then computes signature calling ECDSA_SignFromHash.
 *
 * @param [out] pSignature Pointer to a 64 byte buffer to where the generated signature (r,s) will be stored.
 *                         The signature is stored concatenating r||s with the integers r and s being in BE format.
 *
 * @param [in]  msg         Pointer to a constant message buffer
 * @param [in]  msglen      number of bytes in message.
 * @param [in]  pPrivKey     Pointer to a 32 byte buffer containing the private key d in range [1,n-1] in BE format.

 * @retval gSecEcdsaSuccess_c             The operation finished successfully.
 * @retval gSecEcdsaInvalidEphemeralKey_c The passed ephemeral key is out of range.
 * @retval gSecEcdsaFailure_c    The signature generation failed and shall be repeated with a fresh ephemeral key.
 * @retval gSecEcdsaBadParameter_c           if msg is NULL or msglen is 0.
 *
 */
secEcdsaStatus_t ECDSA_SignMessage(uint8_t        pSignature[SEC_ECP256_COORDINATE_LEN << 1],
                                   const uint8_t *msg,
                                   const size_t   msglen,
                                   const uint8_t  pPrivKey[SEC_ECP256_SCALAR_LEN]);

/**
 * @brief ECDSA signature verification
 *
 * This function verifies an ECDSA signature of a message digest e by verifying the equation
 *
 *    (u1*G + u2*Q).x mod n = r
 *
 * where u1 = e'*s^(-1) mod n and u2 = r*s^(-1) mod n. Here, Q is a valid public key and e' is derived from e by either
 * truncating it or padding it with zeros to 32 bytes.
 *
 * @param [in] pPubKey    Pointer to a 64 byte buffer containing the public key (x,y).
 *                        The public key is stored as the concatenation x||y with x and y being in BE format.
 * @param [in] pDigest    Pointer to a buffer of byte length digestLen containing the message digest in BE format.
 * @param [in] DigestLen  Byte length of the passed message digest.
 * @param [in] pSignature Pointer to a 64 byte buffer containing the generated signature (r,s).
 *                        The signature is stored as the concatenation r||s with the integers r and s being in BE
 * format.
 *
 * @retval gSecEcdsaSuccess_c            The operation finished successfully.
 * @retval gSecEcdsaInvalidSignature_c   The passed signature is invalid.
 * @retval gSecEcdsaInvalidPoint_c       The passed public key is invalid.
 *
 */
secEcdsaStatus_t ECDSA_VerifySignature(const uint8_t *pPubKey,
                                       const uint8_t *pDigest,
                                       uint16_t       DigestLen,
                                       const uint8_t *pSignature);

/**
 * @brief ECDSA message signature verification
 *
 * This function computes the SHA256 digest over the message and then verifies an ECDSA signature
 * calling ECDSA_VerifySignature
 *
 * @param [in] pSignature  Pointer to a 64 byte buffer containing the public key (x,y).
 *                         The public key is stored as the concatenation x||y with x and y being in BE format.
 * @param [in] msg         Pointer to a message buffer.
 * @param [in] msglen      Byte length of the passed message.
 * @param [in] pPublicKey  Pointer to a 64 byte buffer containing the generated signature (r,s).
 *                         The signature is stored concatenating r||s with the integers r and s being in BE format.
 *
 * @retval gSecEcdsaSuccess_c            The operation finished successfully.
 * @retval gSecEcdsaInvalidSignature_c   The passed signature is invalid.
 * @retval gSecEcdsaInvalidPoint_c       The passed public key is invalid.
 *
 */
secEcdsaStatus_t ECDSA_VerifyMessageSignature(const uint8_t  pSignature[SEC_ECP256_COORDINATE_LEN << 1],
                                              const uint8_t *msg,
                                              const size_t   msglen,
                                              const uint8_t  pPublicKey[SEC_ECP256_COORDINATE_LEN << 1]);

/****************************************************************************
 *                          ECP256
 ***************************************************************************/

/*! *********************************************************************************
 * \brief Generate a key pair :
 *          private key (random 32 byte number)
 *          and compute matching public key
 *
 * \param[in] pOutPublicKey pointer on 64 byte storage to receive generate public key
 * \param[in] pOutPrivateKey pointer on 32 byte storage to receive the scalar private key
 * \param[in] pMultiplicationBuffer  pointer on work buffer (can be NULL if using NCCL optimization)
 *
 * \return    gSecEcp256Success_c (0) if success,
 *
 * \pre       RNG initialization must have been run
 *
 * \remarks
 *
 ********************************************************************************** */
secEcp256Status_t ECP256_GenerateKeyPair(ecp256Point_t *pOutPublicKey,
                                         big_int256_t * pOutPrivateKey,
                                         void *         pMultiplicationBuffer);

/*! *********************************************************************************
 * \brief Concatenate ECP256 key pair to an octet string affine point first followed by
 *        private key scalar
 *
 * \param[out] serialized_key pointer on 64 + 32 byte storage in RAM location, preferably aligned
 * \param[in]  keypair pointer on keypair structure (might be read-only)
 *
 *
 ********************************************************************************** */
void ECP256_KeyPairSerialize(uint8_t          serialized_key[SEC_ECP256_POINT_LEN + SEC_ECP256_SCALAR_LEN],
                             ecp256KeyPair_t *keypair);

/*! *********************************************************************************
 * \brief Scatter content of serialized key to keypair structure
 *
 * \param[out]  keypair pointer on keypair structure
 * \param[in] serialized_key pointer on 64 + 32 byte octet string  (might be read-only)
 *
 *
 ********************************************************************************** */
void ECP256_KeyPairDeserialize(ecp256KeyPair_t *keypair,
                               uint8_t          serialized_key[SEC_ECP256_POINT_LEN + SEC_ECP256_SCALAR_LEN]);

/*! *********************************************************************************
 * \brief Load point structure from octet string
 *
 * \param[out] R pointer on 64 byte storage (ecp256Point_t size)
 * \param[in]  in pointer on point array (65 byte) containing affine coordinates in
 *             uncompressed format must start with in[0] == 0x04
 * \param[in]  change_endianness byte reverse coordinates if true
 *
 * \return
 *
 ********************************************************************************** */
void ECP256_PointLoad(ecp256Point_t *R, const uint8_t *in, bool change_endianness);

/*! *********************************************************************************
 * \brief Store point structure from octet string to uncompressed format
 *
 * \param[out] out  pointer on 65 byte storage out[0] will contain 0x04 on exit
 * \param[in]  P in pointer on point array (64 byte) containing affine coordinates
 * \param[in]  change_endianness byte reverse coordinates if true
 *
 * \return
 *
 ********************************************************************************** */
void ECP256_PointWrite(uint8_t *out, const ecp256Point_t *P, bool change_endianness);

/*! *********************************************************************************
 * \brief Load  scalar and forcing it to belong to field
 *        R = fe mod N
 *
 * \param[out] out_fe     pointer on resulting scalar field element (BE)
 * \param[in]  fe_in      pointer on input scalar octet string   (BE)
 * \param[in]  fe_in_len  length of input scalar octet string   (BE)
 *
 * \return gSecEcp256Success_c if successful operation
 *         gSecEcp256BadParameters_c if fe_in_len exceeds 512 bit capacity
 *
 ********************************************************************************** */
secEcp256Status_t ECP256_FieldLoad(uint32_t R[SEC_ECP256_SCALAR_WLEN], const uint8_t *fe_in, uint8_t fe_in_len);

/*! *********************************************************************************
 * \brief Write scalar / Field element to storage
 *
 * \param[out] fe_out     Scalar field element (BE) must be of sufficient size to receive
 *             input scalar so 32 bytes
 * \param[in]  fe_in     pointer on input scalar octet string (BE) presumed to have been
 *             reduced mod N so length is 32 bytes *
 * \return gSecEcp256Success_c if successful operation
 *         gSecEcp256BadParameters_c if either pointer is NULL
 *
 ********************************************************************************** */
secEcp256Status_t ECP256_FieldWrite(uint8_t *fe_out, uint8_t *fe_in);

/*! *********************************************************************************
 * \brief Perform modular reduction on number
 *
 * \param[out]  result   pointer on 8 word array containing reduced value. R = bn mod N
 * \param[in]   bn       pointer on big number to reduce
 * \param[in]   bn_in_len size of bn in number of bytes bn is assumed to be less than 64
 *
 * \return  gSecEcp256Success_c if success
 *          gSecEcp256BadParameters_c if bn is larger than 512 bits (64 bytes)
 *
 ********************************************************************************** */
secEcp256Status_t ECP256_ModularReductionN(uint32_t       R[SEC_ECP256_COORDINATE_WLEN],
                                           const uint8_t *bn_in,
                                           uint8_t        bn_in_len);

/*! *********************************************************************************
 * \brief Multiply 2 scalars and perform the the modulo reduction.
 *        R = ( fe1 * fe2 ) mod N
 *
 * \param[out] R        pointer on result - 256 bit array BE
 * \param[in]  fe1      pointer on scalar field element 256 bits (BE)
 * \param[in]  fe2      pointer on scalar field element 256 bits (BE)
 * \return gSecEcp256Success_c if success
 *
 ********************************************************************************** */
secEcp256Status_t ECP256_ScalarMultiplicationModN(uint32_t       R[SEC_ECP256_SCALAR_WLEN],
                                                  const uint32_t fe1[SEC_ECP256_SCALAR_WLEN],
                                                  const uint32_t fe2[SEC_ECP256_SCALAR_WLEN]);

/*! *********************************************************************************
 * \brief Load coordinate forcing it to belong to field
 *
 * This function performs a double scalar multiplication ec_R = ec_fe1*ec_P1 + ec_fe2*ec_P2
 * for scalars ec_fe1,ec_fe2 in range [1,n-1]
 * and affine points P1 and P2. For double scalar multiplications of the form k1*G + k2*P2,
 * i.e. with the first point being the base point G, one can alternatively leave P1 unspecified
 * (see below) in which case a performance improved algorithm will be used for the operation.
 *
 * \param[out] ec_R   Pointer to 16 CPU words buffer to where the scalar multiplication result
 *                    shall be stored in affine coordinates in BE format in range [0,p-1].
 * \param[in]  ec_P1  Pointer to a 64 byte buffer containing the point P1 = (x1,y1).
 *                    The point is stored as the concatenation x1||y1 with x1 and y1 being in BE format.
 *                    If NULL is passed for pPoint1, then the function will take ec_P1 = G.
 * \param[in]  ec_P2  Pointer to a 64 byte buffer containing the point P2 = (x2,y2).
 *                    The point is stored as the concatenation x2||y2 with x2 and y2 being in BE format.
 * \param[in]  ec_fe1  Pointer to 8 CPU words buffer containing the scalar in range [1,n-1] in BE format.
 * \param[in]  ec_fe2  Pointer to 8 CPU words buffer containing the scalar in range [1,n-1] in BE format.
 *
 * \return gSecEcp256Success_c if success
 *
 ********************************************************************************** */
secEcp256Status_t ECP256_DoublePointMulAdd(
    void *ec_R, const void *ec_P1, const void *ec_fe1, const void *ec_P2, const void *ec_fe2);

/*! *********************************************************************************
 * \brief Return inverted point P(x,y) -> P(x,p-y)
 *
 * \param[out] R  result vector
 * \param[in]  P  pointer on ECP256R1 point input octet string
 *
 * \return  gSecEcp256Success_c if success
 *
 ********************************************************************************** */
secEcp256Status_t ECP256_PointInvert(uint32_t *R, uint32_t const *P);

/*! *********************************************************************************
 * \brief Return whether point is valid or not, i.e it belongs to the curve
 *
 * \param[in]  P   pointer on ECP256R1 point input octet string BE format
 *
 * \return  true if ECP256R1 point is valid false otherwise
 *
 ********************************************************************************** */
bool ECP256_PointValid(const ecp256Point_t *P);

/*! *********************************************************************************
 * \brief Multiply ECP256R1 point by scalar i.e R = fe * P
 *
 * \param[out]  R  pointer on resulting point R = fe * P
 * \param[in]   P  pointer on ECP256R1 affine point
 * \param[in]   fe pointer on scalar

 * \return  gSecEcp256Success_c if success
 *
 ********************************************************************************** */
secEcp256Status_t ECP256_PointMult(ecp256Point_t *R, const uint8_t *P, const uint8_t *fe);

/*! *********************************************************************************
 * \brief Generate a random 32 byte long scalar using RNG, without ensuring it belongs
 *        to [1..p-1] valid range.
 *
 * \param[out] pOutPrivateKey pointer on big int256 (32 byte) storage to receive private key
 *
 * \return
 *
 * \pre       RNG initialization must have been run
 *
 * \remarks   In the unlikely case where the procedure generating the public key rejects
 *            the private key as invalid, it is assumed that we reattempt until a valid
 *            key if generated. After 5 errors it bails out.
 *
 ********************************************************************************** */
secEcp256Status_t ECP256_GeneratePrivateKey(big_int256_t *pOutPrivateKey);

/*! *********************************************************************************
 * \brief Generate an ECP256 Key pair using Ultra fast library
 *
 * \param[out] pOutPublicKey pointer ecp256Point_t storage to receive public key.
 * \param[out] pOutPrivateKey pointer on big_int256_t storage to receive private key
 * \param[in]  pMultiplicationBuffer not used
 *
 * \pre       RNG initialization must have been run
 *
 ********************************************************************************** */
secEcp256Status_t ECP256_GenerateKeyPairUltraFast(ecp256Point_t *pOutPublicKey,
                                                  big_int256_t * pOutPrivateKey,
                                                  void *         pMultiplicationBuffer);

#ifdef __cplusplus
}
/*!
 * @}  end of SecLib_module addtogroup
 */
#endif

#endif /* __SECLIB_ECP256_H__ */
