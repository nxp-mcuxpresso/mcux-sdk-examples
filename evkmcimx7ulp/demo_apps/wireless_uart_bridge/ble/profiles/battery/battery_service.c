/*! *********************************************************************************
 * \addtogroup Battery Service
 * @{
 ********************************************************************************** */
/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "ble_general.h"
#include "gatt_db_app_interface.h"
#include "gatt_server_interface.h"
#include "gap_interface.h"
#include "battery_interface.h"

/************************************************************************************
*************************************************************************************
* Private constants & macros
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/*! Battery Service - Subscribed Client*/
deviceId_t mBas_SubscribedClientId;

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

static void Bas_SendNotifications(uint16_t handle);

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

bleResult_t Bas_Start(basConfig_t *pServiceConfig)
{
    /* Record initial battery level measurement */
    Bas_RecordBatteryMeasurement(pServiceConfig->serviceHandle, pServiceConfig->batteryLevel);

    return gBleSuccess_c;
}

bleResult_t Bas_Stop(basConfig_t *pServiceConfig)
{
    mBas_SubscribedClientId = gInvalidDeviceId_c;
    return gBleSuccess_c;
}

bleResult_t Bas_Subscribe(deviceId_t clientDeviceId)
{
    mBas_SubscribedClientId = clientDeviceId;
    return gBleSuccess_c;
}

bleResult_t Bas_Unsubscribe()
{
    mBas_SubscribedClientId = gInvalidDeviceId_c;
    return gBleSuccess_c;
}

bleResult_t Bas_RecordBatteryMeasurement(uint16_t serviceHandle, uint8_t batteryLevel)
{
    uint16_t handle;
    bleResult_t result;
    bleUuid_t uuid = Uuid16(gBleSig_BatteryLevel_d);

    /* Get handle of  characteristic */
    result = GattDb_FindCharValueHandleInService(serviceHandle, gBleUuidType16_c, &uuid, &handle);

    if (result != gBleSuccess_c)
        return result;

    /* Update characteristic value and send notification */
    result = GattDb_WriteAttribute(handle, sizeof(uint8_t), &batteryLevel);

    if (result != gBleSuccess_c)
        return result;

    Bas_SendNotifications(handle);

    return gBleSuccess_c;
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/
static void Bas_SendNotifications(uint16_t handle)
{
    uint16_t handleCccd;
    bool_t isNotifActive;

    /* Get handle of CCCD */
    if (GattDb_FindCccdHandleForCharValueHandle(handle, &handleCccd) != gBleSuccess_c)
        return;

    if (gBleSuccess_c == Gap_CheckNotificationStatus(mBas_SubscribedClientId, handleCccd, &isNotifActive) &&
        TRUE == isNotifActive)
    {
        GattServer_SendNotification(mBas_SubscribedClientId, handle);
    }
}
/*! *********************************************************************************
 * @}
 ********************************************************************************** */
