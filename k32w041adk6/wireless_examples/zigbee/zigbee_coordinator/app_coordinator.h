/*
* Copyright 2019,2024 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/


#ifndef APP_COORD_H_
#define APP_COORD_H_

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include "app_common.h"
#include "bdb_api.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifndef TRACE_APP
#define TRACE_APP FALSE
#endif

#ifndef TRACE_APP_INIT
#define TRACE_APP_INIT FALSE
#endif

#ifndef MAX_HOST_TO_COPROCESSOR_COMMS_ATTEMPS
#define MAX_HOST_TO_COPROCESSOR_COMMS_ATTEMPS (5)
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

void APP_vInitialiseCoordinator(void);
void APP_taskCoordinator(void);
void APP_cbTimerTick1Sec(void *pvParam);
void APP_cbTimerBlinkLED(void *pvParam);
void APP_vFactoryResetRecords(void);
void vAppHandleZdoEvents( BDB_tsZpsAfEvent *psZpsAfEvent);

/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#endif /*APP_COORD_H_*/
