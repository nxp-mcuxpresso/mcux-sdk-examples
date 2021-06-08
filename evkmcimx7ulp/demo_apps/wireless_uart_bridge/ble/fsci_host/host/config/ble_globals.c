/*! *********************************************************************************
 * \addtogroup BLE
 * @{
 ********************************************************************************** */
/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/************************************************************************************
*************************************************************************************
* DO NOT MODIFY THIS FILE!
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "ble_general.h"
#include "att_errors.h"
#include "ble_config.h"

/************************************************************************************
*************************************************************************************
* Public memory declarations - external references from Host library
*************************************************************************************
************************************************************************************/
uint8_t gcGapMaximumBondedDevices_d = gMaxBondedDevices_c;
bleBondDataBlob_t gaBondDataBlobs[gMaxBondedDevices_c];

uint8_t gcGattMaxHandleCountForWriteNotifications_c = gMaxWriteNotificationHandles_c;
uint16_t gGattWriteNotificationHandles[gMaxWriteNotificationHandles_c];
uint8_t gcGattMaxHandleCountForReadNotifications_c = gMaxReadNotificationHandles_c;
uint16_t gGattReadNotificationHandles[gMaxReadNotificationHandles_c];

uint8_t gcGattDbMaxPrepareWriteOperationsInQueue_c = gPrepareWriteQueueSize_c;
attPrepareWriteRequestParams_t gPrepareWriteQueues[gcGattDbMaxPrepareWriteClients_c][gPrepareWriteQueueSize_c];

uint16_t gGapDefaultTxOctets = gBleDefaultTxOctets_c;
uint16_t gGapDefaultTxTime   = gBleDefaultTxTime_c;

uint16_t gGapHostPrivacyTimeout       = gBleHostPrivacyTimeout_c;
uint16_t gGapControllerPrivacyTimeout = gBleControllerPrivacyTimeout_c;

bool_t gGapLeSecureConnectionsOnlyMode = gBleLeSecureConnectionsOnlyMode_c;
bool_t gGapLeScOobHasMitmProtection    = gBleLeScOobHasMitmProtection_c;

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
