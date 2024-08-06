/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include "zb_platform.h"
#include "app_crypto.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: CRYPTO_Init
 *
 * DESCRIPTION:
 * Initialize the cryptographic HW acceleration
 *
 * PARAMETERS:      Name            RW  Usage
 *
 * RETURNS: void
 *
 ****************************************************************************/
void CRYPTO_Init(void)
{
    return zbPlatCryptoInit();
}

/****************************************************************************
 *
 * NAME: CRYPTO_DeInit
 *
 * DESCRIPTION:
 *   Release allocated resources
 *
 * PARAMETERS:      Name            RW  Usage
 *
 * RETURNS: void
 *
 ****************************************************************************/
void CRYPTO_DeInit(void)
{
    return zbPlatCryptoDeInit();
}

/****************************************************************************
 *
 * NAME: CRYPTO_u8RandomInit
 *
 * DESCRIPTION:
 * Initialize the random number generator unit
 *
 * PARAMETERS:      Name            RW  Usage
 *
 * RETURNS:
 * status of initialization
 *
 ****************************************************************************/
uint8_t CRYPTO_u8RandomInit(void)
{
    return zbPlatCryptoRandomInit();
}

/****************************************************************************
 *
 * NAME: CRYPTO_AesMmoBlockUpdate
 *
 * DESCRIPTION:
 * Perform an AES MMO Block Update on the hash
 *
 * PARAMETERS:      Name            RW  Usage
 *                  hash            Pointer to the location of hash to use as AES key
 *                  block           Pointer to the location of the block on which to perform hash
 *
 * RETURNS: void
 *
 ****************************************************************************/
void CRYPTO_AesMmoBlockUpdate(void *hash, void *block)
{
    return zbPlatCryptoAesMmoBlockUpdate(hash, block);
}

/****************************************************************************
 *
 * NAME: CRYPTO_AesMmoFinalUpdate
 *
 * DESCRIPTION:
 * Finalize the AES MMO hash computation
 *
 * PARAMETERS:      Name            RW  Usage
 *                  hash            Pointer to the location of hash to use as MMO object
 *                  pu8Data         Pointer to the location of the text to run through
 *                  iDataLen        Length of data stream
 *                  iOutputLen      Desired length of data buffer
 *
 * RETURNS: void
 *
 ****************************************************************************/
void CRYPTO_AesMmoFinalUpdate(void *hash, uint8_t *pu8Data, int iDataLen, int iOutputLen)
{
    return zbPlatCryptoAesMmoFinalUpdate(hash, pu8Data, iDataLen, iOutputLen);
}

/****************************************************************************
 *
 * NAME: CRYPTO_AesHmacMmo
 *
 * DESCRIPTION:
 * Perform the HMAC-MMO Keyed Hash Function for Message Authentication
 * Specified in B.1.4 in ZigBee specification (053474r17)
 *
 * PARAMETERS:      Name            RW  Usage
 *                  pu8Data             Pointer to data stream
 *                  iDataLen            Length of data stream
 *                  key                 Pointer to the location of the 128-bit key
 *                  hash                Pointer to the location to store the output hash
 *
 * RETURNS: void
 *
 ****************************************************************************/
void CRYPTO_AesHmacMmo(uint8_t *pu8Data, int iDataLen, void *key, void *hash)
{
    return zbPlatCryptoAesHmacMmo(pu8Data, iDataLen, key, hash);
}

/****************************************************************************
 *
 * NAME: CRYPTO_u32RandomGet
 *
 * DESCRIPTION:
 * Returns a 32-bit statistically random generated number, between the
 * specified mininum and maximum values
 *
 * PARAMETERS:      Name            RW  Usage
 *                  u32Min              minimum value
 *                  u32Max              maximum value
 *
 * RETURNS:
 * random generated number
 *
 ****************************************************************************/
uint32_t CRYPTO_u32RandomGet(uint32_t u32Min, uint32_t u32Max)
{
    return zbPlatCryptoRandomGet(u32Min, u32Max);
}

/****************************************************************************
 *
 * NAME: CRYPTO_u32Random256Get
 *
 * DESCRIPTION:
 * Returns a 32-bit statistically random generated number
 *
 * PARAMETERS:      Name            RW  Usage
 *
 * RETURNS:
 * random generated number
 *
 ****************************************************************************/
uint32_t CRYPTO_u32Random256Get(void)
{
    return zbPlatCryptoRandom256Get();
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
