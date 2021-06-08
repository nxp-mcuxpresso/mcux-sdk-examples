/*! *********************************************************************************
 * \addtogroup BLE
 * @{
 ********************************************************************************** */
/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BLE_CONN_MANAGER_H_
#define _BLE_CONN_MANAGER_H_

/************************************************************************************
*************************************************************************************
* Includes
*************************************************************************************
************************************************************************************/
#include "gap_types.h"

/************************************************************************************
*************************************************************************************
* Public Macros
*************************************************************************************
************************************************************************************/

/*! Enable / Disable Connection Update procedure on Peripheral */
#ifndef gConnUpdateAlwaysAccept_d
#define gConnUpdateAlwaysAccept_d 1
#endif

/*! Minimum connection interval for accepting an update */
#ifndef gConnUpdateIntervalMin_d
#define gConnUpdateIntervalMin_d gGapConnIntervalMin_d
#endif

/*! Maximum connection interval for accepting an update */
#ifndef gConnUpdateIntervalMax_d
#define gConnUpdateIntervalMax_d gGapConnIntervalMax_d
#endif

/*! Minimum latency for accepting an update */
#ifndef gConnUpdateLatencyMin_d
#define gConnUpdateLatencyMin_d gGapConnLatencyMin_d
#endif

/*! Maximum latency for accepting an update */
#ifndef gConnUpdateLatencyMax_d
#define gConnUpdateLatencyMax_d gGapConnLatencyMin_d
#endif

/*! Minimum supervision timeout for accepting an update */
#ifndef gConnUpdateSuperTimeoutMin_d
#define gConnUpdateSuperTimeoutMin_d gGapConnSuperTimeoutMin_d
#endif

/*! Maximum supervision timeout for accepting an update */
#ifndef gConnUpdateSuperTimeoutMax_d
#define gConnUpdateSuperTimeoutMax_d gGapConnSuperTimeoutMax_d
#endif

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */
extern uint8_t gcBondedDevices;

extern gapConnectionRequestParameters_t gConnReqParams;
extern gapScanningParameters_t gScanParams;
extern gapAdvertisingData_t gAppAdvertisingData;
extern gapScanResponseData_t gAppScanRspData;
extern gapAdvertisingParameters_t gAdvParams;
extern gapSmpKeys_t gSmpKeys;
extern gapPairingParameters_t gPairingParameters;
extern gapDeviceSecurityRequirements_t deviceSecurityRequirements;
/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
 * \brief  Performs common operations on the BLE stack on the generic callback.
 *
 * \param[in] pGenericEvent  GAP Generic event from the Host Stack .
 *
 *
 ********************************************************************************** */
void BleConnManager_GenericEvent(gapGenericEvent_t *pGenericEvent);

/*! *********************************************************************************
 * \brief  Performs common GAP Peripheral and Central configurations of the BLE stack.
 *
 *
 ********************************************************************************** */
void BleConnManager_GapDualRoleConfig(void);

/*! *********************************************************************************
 * \brief  Performs common GAP Peripheral configurations of the BLE stack.
 *
 *
 ********************************************************************************** */
void BleConnManager_GapPeripheralConfig(void);

/*! *********************************************************************************
 * \brief  Performs common GAP Central configurations of the BLE stack.
 *
 *
 ********************************************************************************** */
void BleConnManager_GapCentralConfig(void);

/*! *********************************************************************************
 * \brief  Performs GAP Peripheral operations on the BLE stack on the connection callback.
 *
 * \param[in] peerDeviceId  	   The GAP peer Id.
 *
 * \param[in] pConnectionEvent  GAP Connection event from the Host Stack.
 *
 ********************************************************************************** */
void BleConnManager_GapPeripheralEvent(deviceId_t peerDeviceId, gapConnectionEvent_t *pConnectionEvent);

/*! *********************************************************************************
 * \brief  Performs GAP Central operations on the BLE stack on the connection callback.
 *
 * \param[in] peerDeviceId  	   The GAP peer Id.
 *
 * \param[in] pConnectionEvent  GAP Connection event from the Host Stack.
 *
 ********************************************************************************** */
void BleConnManager_GapCentralEvent(deviceId_t peerDeviceId, gapConnectionEvent_t *pConnectionEvent);

#endif /* _BLE_CONN_MANAGER_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
