/*
* Copyright 2019, 2023 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef APP_MAIN_H
#define APP_MAIN_H

#include "ZQueue.h"
#include "ZTimer.h"

#include "bdb_api.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
/* timers */
extern uint8_t u8TimerId;
#ifndef NCP_HOST
extern uint8_t  u8LedTimer;
#endif
/* queue handles */
extern tszQueue APP_msgSerialRx;
extern tszQueue APP_msgAppEvents;
#ifdef NCP_HOST
extern tszQueue appQueueHandle;
extern tszQueue zclQueueHandle;
#endif
extern uint32_t u32Togglems;
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
void APP_vInitResources(void);
void APP_cbTimerId(void *pvParam);
void APP_cbTimerLed(void *pvParam);

/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/



#endif /* APP_MAIN_H */






