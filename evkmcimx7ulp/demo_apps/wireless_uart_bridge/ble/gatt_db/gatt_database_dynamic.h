/*! *********************************************************************************
 * \addtogroup GATT_DB
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

#ifndef _GATT_DATABASE_DYNAMIC_H_
#define _GATT_DATABASE_DYNAMIC_H_

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/

#include "EmbeddedTypes.h"

/*************************************************************************************
**************************************************************************************
* Public macros
**************************************************************************************
*************************************************************************************/

/*************************************************************************************
**************************************************************************************
* Public types
**************************************************************************************
*************************************************************************************/

/* Service, Characteristics and Descriptors used to add a service */

typedef struct descriptorInfo_tag
{
    bleUuidType_t uuidType;
    bleUuid_t uuid;
    uint16_t handle;
    uint16_t valueLength;
    uint8_t *pValue;
    gattAttributePermissionsBitFields_t accessPermissions;
} descriptorInfo_t;

typedef struct characteristicInfo_tag
{
    bleUuidType_t uuidType;
    bleUuid_t uuid;
    uint16_t handle;
    gattCharacteristicPropertiesBitFields_t properties;
    uint16_t maxValueLength;
    uint16_t valueLength;
    uint8_t *pValue;
    gattAttributePermissionsBitFields_t accessPermissions;
    bool_t bAddCccd;
    uint16_t cccdHandle;
    uint8_t nbOfDescriptors;
    descriptorInfo_t *pDescriptorInfo;
} characteristicInfo_t;

typedef struct serviceInfo_tag
{
    bleUuidType_t uuidType;
    bleUuid_t uuid;
    uint16_t handle;
    uint8_t nbOfCharacteristics;
    characteristicInfo_t *pCharacteristicInfo;
} serviceInfo_t;

/* Output handles returned by the service add functions obtained from Ble Host */

/* GATT service */
typedef struct gattServiceHandles_tag
{
    uint16_t serviceHandle;
    uint16_t charServiceChangedHandle;
    uint16_t charServiceChangedCccdHandle;
} gattServiceHandles_t;

/* GAP service */
typedef struct gapServiceHandles_tag
{
    uint16_t serviceHandle;
    uint16_t charDeviceNameHandle;
    uint16_t charAppearanceHandle;
    uint16_t charPpcpHandle;
} gapServiceHandles_t;

/* Wireless Uart service */
typedef struct wirelessUartServiceHandles_tag
{
    uint16_t serviceHandle;
    uint16_t charUartStreamHandle;
} wirelessUartServiceHandles_t;

/* Battery service */
typedef struct batteryServiceHandles_tag
{
    uint16_t serviceHandle;
    uint16_t charBatteryLevelHandle;
    uint16_t charBatteryLevelCccdHandle;
} batteryServiceHandles_t;

/* Device info service */
typedef struct deviceInfoServiceHandles_tag
{
    uint16_t serviceHandle;
    uint16_t charManufNameHandle;
    uint16_t charModelNoHandle;
    uint16_t charSerialNoHandle;
    uint16_t charHwRevHandle;
    uint16_t charFwRevHandle;
    uint16_t charSwRevHandle;
    uint16_t charSystemIdHandle;
    uint16_t charRcdlHandle;
} deviceInfoServiceHandles_t;

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

bleResult_t GattDbDynamic_AddServiceInDatabase(serviceInfo_t *pServiceInfo);

bleResult_t GattDbDynamic_AddGattService(gattServiceHandles_t *pOutServiceHandles);
bleResult_t GattDbDynamic_AddGapService(gapServiceHandles_t *pOutServiceHandles);
bleResult_t GattDbDynamic_AddWirelessUartService(wirelessUartServiceHandles_t *pOutServiceHandles);
bleResult_t GattDbDynamic_AddBatteryService(batteryServiceHandles_t *pOutServiceHandles);
bleResult_t GattDbDynamic_AddDeviceInformationService(deviceInfoServiceHandles_t *pOutServiceHandles);

serviceInfo_t *GattDbDynamic_GetWirelessUartService(void);
serviceInfo_t *GattDbDynamic_GetBatteryService(void);
serviceInfo_t *GattDbDynamic_GetDeviceInformationService(void);

#ifdef __cplusplus
}
#endif

#endif /* _GATT_DATABASE_DYNAMIC_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
