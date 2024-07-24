/*
* Copyright 2019, 2023 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/


#ifndef  UART_H_INCLUDED
#define  UART_H_INCLUDED

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
#ifndef TRACE_UART
#define TRACE_UART	FALSE
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

void UART_vInit(void *device);
void UART_vTxChar(uint8_t u8TxChar);
bool_t UART_bTxReady(void);
void UART_vSetBaudRate(uint32_t u32BaudRate);
bool_t UART_bReceiveChar ( uint8_t* u8Data );
void UART_vSendString(char* sMessage);
bool_t UART_bReceiveBuffer(uint8_t* u8Buffer, uint32_t *pu32BufferLen);
void UART_vFree(void);
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* UART_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/


