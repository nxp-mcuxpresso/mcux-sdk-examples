/*
* Copyright 2019, 2023 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/


#ifndef ZIGBEE_CONFIG_H
#define ZIGBEE_CONFIG_H

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include "EmbeddedTypes.h"
#include "ZQueue.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define GREENPOWER_ENABLE            (1)
#define ZIGBEE_TIMER_STORAGE         (6)
    
#define BDB_QUEUE_SIZE               (3)
#define TIMER_QUEUE_SIZE             (8)
#define MLME_QUEQUE_SIZE             (9)
#define MCPS_QUEUE_SIZE              (16)
#define MCPS_DCFM_QUEUE_SIZE         (8)

#define ZPS_EVENT_QUEUE_SIZE        3
#define SERIAL_MSG_QUEUE_SIZE    10

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
void APP_vRunZigbee(void);
void APP_vInitZigbeeResources(void);

/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/
extern tszQueue sAPP_msgZpsEvents;
extern tszQueue zps_msgMlmeDcfmInd;
extern tszQueue zps_msgMcpsDcfmInd;
extern tszQueue zps_msgMcpsDcfm;
extern tszQueue zps_TimeEvents;

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
#endif /* ZIGBEE_CONFIG_H */
