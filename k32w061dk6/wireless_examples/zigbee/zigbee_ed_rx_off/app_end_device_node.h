/*
* Copyright 2019 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/


#ifndef APP_END_DEVICE_NODE_H_
#define APP_END_DEVICE_NODE_H_

#include "app_common.h"
#include "OTA.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifndef ZED_SLEEP_PERIOD
#define ZED_SLEEP_PERIOD (1000)
#endif

#ifndef ZED_MAX_POLLS
#define ZED_MAX_POLLS (20)
#endif

#ifndef ZED_FB_START_TIME
#define ZED_FB_START_TIME (1) /* s */
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum {
    E_STATE_ZB_APP_INIT,
    E_STATE_ZB_APP_INACTIVE,
    E_STATE_ZB_SCAN_NETWORKS,
    E_STATE_ZB_NETWORK_ENTRY,
    E_STATE_ZB_APP_POLL,
    E_STATE_ZB_APP_ACTIVE,
    E_APP_ZB_STATE_INVALID,
} APP_teState;
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

void APP_vInitialiseEndDevice(bool_t bColdStart);
void APP_vFactoryResetRecords(void);
void APP_taskEndDevicNode(void);
teNodeState eGetNodeState(void);
#ifdef CLD_OTA
tsOTA_PersistedData sGetOTACallBackPersistdata(void);
#endif
#ifdef OT_ZB_SUPPORT
extern void APP_vSetSchedPolicy(uint8_t u8ProtoId, APP_teState eState);
extern void vNsTryNwkJoinAppCb(ZPS_tsNwkNetworkDescr  *pNwkDescr);
#endif
/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/
extern tsDeviceDesc sDeviceDesc;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#endif /*APP_END_DEVICE_NODE_H_*/
