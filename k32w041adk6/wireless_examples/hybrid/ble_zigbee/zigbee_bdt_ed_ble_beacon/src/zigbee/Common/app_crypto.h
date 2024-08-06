/*
 * Copyright 2019, 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef APP_CRYPTO_H_INCLUDED
#define APP_CRYPTO_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include "EmbeddedTypes.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifndef TRACE_CRYPTO
#define TRACE_CRYPTO FALSE
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
void CRYPTO_Init(void);
void CRYPTO_DeInit(void);
void CRYPTO_AesHmacMmo(uint8_t *pu8Data, int iDataLen, void *key, void *hash);
void CRYPTO_AesMmoBlockUpdate(void *key, void *hash);
void CRYPTO_AesMmoFinalUpdate(void *hash, uint8_t *pu8Data, int iDataLen, int iOutputLen);
uint8_t CRYPTO_u8RandomInit(void);
uint32_t CRYPTO_u32RandomGet(uint32_t u32Min, uint32_t u32Max);
uint32_t CRYPTO_u32Random256Get(void);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif /* APP_CRYPTO_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
