/*
* Copyright 2019, 2023 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/


#ifndef  CONSOLE_H_INCLUDED
#define  CONSOLE_H_INCLUDED

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
#ifndef TRACE_CONSOLE
#define TRACE_CONSOLE   FALSE
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
void APP_vConsoleInitialise(void);
bool_t APP_bConsoleReceiveChar(uint8_t* pu8Data);
bool_t APP_bConsoleCanTransmit(void);
bool_t APP_bConsoleTransmit(uint8_t pu8Data);
void APP_vConsoleSetBaudRate(uint32_t baud);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* CONSOLE_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/


