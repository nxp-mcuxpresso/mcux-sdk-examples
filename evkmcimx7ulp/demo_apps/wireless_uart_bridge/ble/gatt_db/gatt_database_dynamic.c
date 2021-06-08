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

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "MemManager.h"
#include "board.h"

#include "gap_types.h"
#include "gatt_db_dynamic.h"

#include "gatt_database_dynamic.h"

/************************************************************************************
*************************************************************************************
* Private constants & macros
*************************************************************************************
************************************************************************************/

#define mGattDbDynamic_GattServiceChangedInitValue \
    {                                              \
        0x00, 0x00, 0x00, 0x00                     \
    }

#define mGattDbDynamic_GapServiceDeviceNameInitValue "FSL_WIRLESS_UART"
#define mGattDbDynamic_GapServiceAppearanceInitValue \
    {                                                \
        0x00, 0x00                                   \
    }

#define mGattDbDynamic_BServiceLevelInitValue \
    {                                         \
        0x5A                                  \
    }
#define mGattDbDynamic_BServiceCharPresFormatDescriptorInitValue \
    {                                                            \
        0x04, 0x00, 0xAD, 0x27, 0x01, 0x01, 0x00                 \
    }

#define mGattDbDynamic_DIServiceManufNameInitValue MANUFACTURER_NAME
#define mGattDbDynamic_DIServiceModelNbInitValue   "Kinetis BLE"
#define mGattDbDynamic_DIServiceSerialNoInitValue  "BLESN01"
#define mGattDbDynamic_DIServiceHwRevInitValue     BOARD_NAME
#define mGattDbDynamic_DIServiceFwRevInitValue     "1.1.1"
#define mGattDbDynamic_DIServiceSwRevInitValue     "1.1.4"
#define mGattDbDynamic_DIServiceSysIdInitValue         \
    {                                                  \
        0x00, 0x00, 0x00, 0xFE, 0xFF, 0x9F, 0x04, 0x00 \
    }
#define mGattDbDynamic_DIServiceIeeeRcdlInitValue \
    {                                             \
        0x00, 0x00, 0x00, 0x00                    \
    }

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
const uint8_t uuid_service_wireless_uart[16] = {0xE0, 0x1C, 0x4B, 0x5E, 0x1E, 0xEB, 0xA1, 0x5C,
                                                0xEE, 0xF4, 0x5E, 0xBA, 0x00, 0x01, 0xFF, 0x01};

const uint8_t uuid_uart_stream[16] = {0xE0, 0x1C, 0x4B, 0x5E, 0x1E, 0xEB, 0xA1, 0x5C,
                                      0xEE, 0xF4, 0x5E, 0xBA, 0x01, 0x01, 0xFF, 0x01};

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

bleResult_t GattDbDynamic_AddServiceInDatabase(serviceInfo_t *pServiceInfo)
{
    characteristicInfo_t *pCharacteristicInfo = NULL;
    descriptorInfo_t *pDescriptorInfo         = NULL;
    uint32_t i                                = 0;
    uint32_t j                                = 0;
    bleResult_t result                        = gBleSuccess_c;

    /* Add service */
    result = GattDbDynamic_AddPrimaryServiceDeclaration(pServiceInfo->uuidType, (bleUuid_t *)&pServiceInfo->uuid,
                                                        &pServiceInfo->handle);

    if (gBleSuccess_c == result)
    {
        for (i = 0; i < pServiceInfo->nbOfCharacteristics; i++)
        {
            /* Add characteristic */
            pCharacteristicInfo = &pServiceInfo->pCharacteristicInfo[i];
            result              = GattDbDynamic_AddCharacteristicDeclarationAndValue(
                pCharacteristicInfo->uuidType, (bleUuid_t *)&pCharacteristicInfo->uuid, pCharacteristicInfo->properties,
                pCharacteristicInfo->maxValueLength, pCharacteristicInfo->valueLength, pCharacteristicInfo->pValue,
                pCharacteristicInfo->accessPermissions, &pCharacteristicInfo->handle);

            if (gBleSuccess_c != result)
            {
                break;
            }
            else
            {
                for (j = 0; j < pCharacteristicInfo->nbOfDescriptors; j++)
                {
                    /* Add descriptor */
                    pDescriptorInfo = &pCharacteristicInfo->pDescriptorInfo[j];
                    result          = GattDbDynamic_AddCharacteristicDescriptor(
                        pDescriptorInfo->uuidType, (bleUuid_t *)&pDescriptorInfo->uuid, pDescriptorInfo->valueLength,
                        pDescriptorInfo->pValue, pDescriptorInfo->accessPermissions, &pDescriptorInfo->handle);

                    if (gBleSuccess_c != result)
                    {
                        break;
                    }
                }
            }

            if (pCharacteristicInfo->bAddCccd && (gBleSuccess_c == result))
            {
                /* Add Cccd */
                result = GattDbDynamic_AddCccd(&pCharacteristicInfo->cccdHandle);
            }

            if (gBleSuccess_c != result)
            {
                break;
            }
        }
    }

    return result;
}

bleResult_t GattDbDynamic_AddGattService(gattServiceHandles_t *pOutServiceHandles)
{
    uint8_t gattServiceChangedValue[] = mGattDbDynamic_GattServiceChangedInitValue;
    serviceInfo_t *pServiceInfo =
        (serviceInfo_t *)MEM_BufferAlloc(sizeof(serviceInfo_t) + 1 * sizeof(characteristicInfo_t) +
                                         0 * sizeof(descriptorInfo_t) + sizeof(gattServiceChangedValue));
    bleResult_t result = gBleSuccess_c;

    if (NULL != pServiceInfo)
    {
        pServiceInfo->uuidType            = gBleUuidType16_c;
        pServiceInfo->uuid.uuid16         = gBleSig_GenericAttributeProfile_d;
        pServiceInfo->nbOfCharacteristics = 1;
        pServiceInfo->pCharacteristicInfo = (characteristicInfo_t *)((uint8_t *)pServiceInfo + sizeof(serviceInfo_t));

        pServiceInfo->pCharacteristicInfo[0].uuidType    = gBleUuidType16_c;
        pServiceInfo->pCharacteristicInfo[0].uuid.uuid16 = gBleSig_GattServiceChanged_d;
        pServiceInfo->pCharacteristicInfo[0].properties =
            (gattCharacteristicPropertiesBitFields_t)(gGattCharPropRead_c | gGattCharPropNotify_c);
        pServiceInfo->pCharacteristicInfo[0].maxValueLength = 0;
        pServiceInfo->pCharacteristicInfo[0].valueLength    = sizeof(gattServiceChangedValue);
        pServiceInfo->pCharacteristicInfo[0].pValue =
            (uint8_t *)pServiceInfo->pCharacteristicInfo + sizeof(characteristicInfo_t);
        FLib_MemCpy(pServiceInfo->pCharacteristicInfo[0].pValue, &gattServiceChangedValue,
                    sizeof(gattServiceChangedValue));
        pServiceInfo->pCharacteristicInfo[0].accessPermissions = gPermissionNone_c;
        pServiceInfo->pCharacteristicInfo[0].bAddCccd          = TRUE;
        pServiceInfo->pCharacteristicInfo[0].nbOfDescriptors   = 0;
        pServiceInfo->pCharacteristicInfo[0].pDescriptorInfo   = NULL;

        result = GattDbDynamic_AddServiceInDatabase(pServiceInfo);

        if ((gBleSuccess_c == result) && (NULL != pOutServiceHandles))
        {
            /* Fill service handles */
            pOutServiceHandles->serviceHandle                = pServiceInfo->handle;
            pOutServiceHandles->charServiceChangedHandle     = pServiceInfo->pCharacteristicInfo->handle;
            pOutServiceHandles->charServiceChangedCccdHandle = pServiceInfo->pCharacteristicInfo->cccdHandle;
        }

        /* Free allocated service */
        MEM_BufferFree(pServiceInfo);
    }
    else
    {
        result = gBleOutOfMemory_c;
    }

    return result;
}

bleResult_t GattDbDynamic_AddGapService(gapServiceHandles_t *pOutServiceHandles)
{
    uint8_t gapDeviceNameValue[] = mGattDbDynamic_GapServiceDeviceNameInitValue;
    uint8_t gapAppearanceValue[] = mGattDbDynamic_GapServiceAppearanceInitValue;
    serviceInfo_t *pServiceInfo  = (serviceInfo_t *)MEM_BufferAlloc(
        sizeof(serviceInfo_t) + 2 * sizeof(characteristicInfo_t) + 0 * sizeof(descriptorInfo_t) +
        sizeof(gapDeviceNameValue) + sizeof(gapAppearanceValue));
    bleResult_t result = gBleSuccess_c;

    if (NULL != pServiceInfo)
    {
        pServiceInfo->uuidType            = gBleUuidType16_c;
        pServiceInfo->uuid.uuid16         = gBleSig_GenericAccessProfile_d;
        pServiceInfo->nbOfCharacteristics = 2;
        pServiceInfo->pCharacteristicInfo = (characteristicInfo_t *)((uint8_t *)pServiceInfo + sizeof(serviceInfo_t));

        pServiceInfo->pCharacteristicInfo[0].uuidType       = gBleUuidType16_c;
        pServiceInfo->pCharacteristicInfo[0].uuid.uuid16    = gBleSig_GapDeviceName_d;
        pServiceInfo->pCharacteristicInfo[0].properties     = gGattCharPropRead_c;
        pServiceInfo->pCharacteristicInfo[0].maxValueLength = 20;
        pServiceInfo->pCharacteristicInfo[0].valueLength    = sizeof(gapDeviceNameValue);
        pServiceInfo->pCharacteristicInfo[0].pValue =
            (uint8_t *)pServiceInfo->pCharacteristicInfo + 2 * sizeof(characteristicInfo_t);
        FLib_MemCpy(pServiceInfo->pCharacteristicInfo[0].pValue, &gapDeviceNameValue, sizeof(gapDeviceNameValue));
        pServiceInfo->pCharacteristicInfo[0].accessPermissions = gPermissionFlagReadable_c;
        pServiceInfo->pCharacteristicInfo[0].bAddCccd          = FALSE;
        pServiceInfo->pCharacteristicInfo[0].nbOfDescriptors   = 0;
        pServiceInfo->pCharacteristicInfo[0].pDescriptorInfo   = NULL;

        pServiceInfo->pCharacteristicInfo[1].uuidType       = gBleUuidType16_c;
        pServiceInfo->pCharacteristicInfo[1].uuid.uuid16    = gBleSig_GapAppearance_d;
        pServiceInfo->pCharacteristicInfo[1].properties     = gGattCharPropRead_c;
        pServiceInfo->pCharacteristicInfo[1].maxValueLength = 0;
        pServiceInfo->pCharacteristicInfo[1].valueLength    = sizeof(gapAppearanceValue);
        pServiceInfo->pCharacteristicInfo[1].pValue =
            pServiceInfo->pCharacteristicInfo[0].pValue + sizeof(gapDeviceNameValue);
        FLib_MemCpy(pServiceInfo->pCharacteristicInfo[1].pValue, &gapAppearanceValue, sizeof(gapAppearanceValue));
        pServiceInfo->pCharacteristicInfo[1].accessPermissions = gPermissionFlagReadable_c;
        pServiceInfo->pCharacteristicInfo[1].bAddCccd          = FALSE;
        pServiceInfo->pCharacteristicInfo[1].nbOfDescriptors   = 0;
        pServiceInfo->pCharacteristicInfo[1].pDescriptorInfo   = NULL;

        result = GattDbDynamic_AddServiceInDatabase(pServiceInfo);

        if ((gBleSuccess_c == result) && (NULL != pOutServiceHandles))
        {
            /* Fill service handles */
            pOutServiceHandles->serviceHandle        = pServiceInfo->handle;
            pOutServiceHandles->charDeviceNameHandle = pServiceInfo->pCharacteristicInfo[0].handle;
            pOutServiceHandles->charAppearanceHandle = pServiceInfo->pCharacteristicInfo[1].handle;
        }

        /* Free allocated service */
        MEM_BufferFree(pServiceInfo);
    }
    else
    {
        result = gBleOutOfMemory_c;
    }

    return result;
}

bleResult_t GattDbDynamic_AddWirelessUartService(wirelessUartServiceHandles_t *pOutServiceHandles)
{
    serviceInfo_t *pServiceInfo = GattDbDynamic_GetWirelessUartService();
    bleResult_t result          = gBleSuccess_c;

    if (NULL != pServiceInfo)
    {
        result = GattDbDynamic_AddServiceInDatabase(pServiceInfo);

        if ((gBleSuccess_c == result) && (NULL != pOutServiceHandles))
        {
            /* Fill service handles */
            pOutServiceHandles->serviceHandle        = pServiceInfo->handle;
            pOutServiceHandles->charUartStreamHandle = pServiceInfo->pCharacteristicInfo->handle;
        }

        /* Free allocated service */
        MEM_BufferFree(pServiceInfo);
    }
    else
    {
        result = gBleOutOfMemory_c;
    }

    return result;
}

bleResult_t GattDbDynamic_AddBatteryService(batteryServiceHandles_t *pOutServiceHandles)
{
    serviceInfo_t *pServiceInfo = GattDbDynamic_GetBatteryService();
    bleResult_t result          = gBleSuccess_c;

    if (NULL != pServiceInfo)
    {
        result = GattDbDynamic_AddServiceInDatabase(pServiceInfo);

        if ((gBleSuccess_c == result) && (NULL != pOutServiceHandles))
        {
            /* Fill service handles */
            pOutServiceHandles->serviceHandle              = pServiceInfo->handle;
            pOutServiceHandles->charBatteryLevelHandle     = pServiceInfo->pCharacteristicInfo[0].handle;
            pOutServiceHandles->charBatteryLevelCccdHandle = pServiceInfo->pCharacteristicInfo[0].cccdHandle;
        }

        /* Free allocated service */
        MEM_BufferFree(pServiceInfo);
    }
    else
    {
        result = gBleOutOfMemory_c;
    }

    return result;
}

bleResult_t GattDbDynamic_AddDeviceInformationService(deviceInfoServiceHandles_t *pOutServiceHandles)
{
    serviceInfo_t *pServiceInfo = GattDbDynamic_GetDeviceInformationService();
    bleResult_t result          = gBleSuccess_c;

    if (NULL != pServiceInfo)
    {
        result = GattDbDynamic_AddServiceInDatabase(pServiceInfo);

        if ((gBleSuccess_c == result) && (NULL != pOutServiceHandles))
        {
            /* Fill service handles */
            pOutServiceHandles->serviceHandle       = pServiceInfo->handle;
            pOutServiceHandles->charManufNameHandle = pServiceInfo->pCharacteristicInfo[0].handle;
            pOutServiceHandles->charModelNoHandle   = pServiceInfo->pCharacteristicInfo[1].handle;
            pOutServiceHandles->charSerialNoHandle  = pServiceInfo->pCharacteristicInfo[2].handle;
            pOutServiceHandles->charHwRevHandle     = pServiceInfo->pCharacteristicInfo[3].handle;
            pOutServiceHandles->charFwRevHandle     = pServiceInfo->pCharacteristicInfo[4].handle;
            pOutServiceHandles->charSwRevHandle     = pServiceInfo->pCharacteristicInfo[5].handle;
            pOutServiceHandles->charSystemIdHandle  = pServiceInfo->pCharacteristicInfo[6].handle;
            pOutServiceHandles->charRcdlHandle      = pServiceInfo->pCharacteristicInfo[7].handle;
        }

        /* Free allocated service */
        MEM_BufferFree(pServiceInfo);
    }
    else
    {
        result = gBleOutOfMemory_c;
    }

    return result;
}

serviceInfo_t *GattDbDynamic_GetWirelessUartService()
{
    serviceInfo_t *pServiceInfo = (serviceInfo_t *)MEM_BufferAlloc(
        sizeof(serviceInfo_t) + 1 * sizeof(characteristicInfo_t) + 0 * sizeof(descriptorInfo_t) + 1);

    if (NULL != pServiceInfo)
    {
        pServiceInfo->uuidType = gBleUuidType128_c;
        FLib_MemCpy(pServiceInfo->uuid.uuid128, (void *)uuid_service_wireless_uart, sizeof(uuid_service_wireless_uart));
        pServiceInfo->nbOfCharacteristics = 1;
        pServiceInfo->pCharacteristicInfo = (characteristicInfo_t *)((uint8_t *)pServiceInfo + sizeof(serviceInfo_t));

        pServiceInfo->pCharacteristicInfo[0].uuidType = gBleUuidType128_c;
        FLib_MemCpy(pServiceInfo->pCharacteristicInfo[0].uuid.uuid128, (void *)uuid_uart_stream,
                    sizeof(uuid_uart_stream));
        pServiceInfo->pCharacteristicInfo[0].properties     = gGattCharPropWriteWithoutRsp_c;
        pServiceInfo->pCharacteristicInfo[0].maxValueLength = gAttMaxWriteDataSize_d(gAttMaxMtu_c);
        pServiceInfo->pCharacteristicInfo[0].valueLength    = 1;
        pServiceInfo->pCharacteristicInfo[0].pValue =
            (uint8_t *)pServiceInfo->pCharacteristicInfo + sizeof(characteristicInfo_t);
        pServiceInfo->pCharacteristicInfo[0].pValue[0]         = 0;
        pServiceInfo->pCharacteristicInfo[0].accessPermissions = gPermissionFlagWritable_c;
        pServiceInfo->pCharacteristicInfo[0].bAddCccd          = FALSE;
        pServiceInfo->pCharacteristicInfo[0].nbOfDescriptors   = 0;
        pServiceInfo->pCharacteristicInfo[0].pDescriptorInfo   = NULL;
    }

    return pServiceInfo;
}

serviceInfo_t *GattDbDynamic_GetBatteryService()
{
    uint8_t bBatteryLevelValue[]             = mGattDbDynamic_BServiceLevelInitValue;
    uint8_t bCharPresFormatDescriptorValue[] = mGattDbDynamic_BServiceCharPresFormatDescriptorInitValue;
    serviceInfo_t *pServiceInfo              = (serviceInfo_t *)MEM_BufferAlloc(
        sizeof(serviceInfo_t) + 1 * sizeof(characteristicInfo_t) + 1 * sizeof(descriptorInfo_t) +
        sizeof(bBatteryLevelValue) + sizeof(bCharPresFormatDescriptorValue));

    if (NULL != pServiceInfo)
    {
        pServiceInfo->uuidType            = gBleUuidType16_c;
        pServiceInfo->uuid.uuid16         = gBleSig_BatteryService_d;
        pServiceInfo->nbOfCharacteristics = 1;
        pServiceInfo->pCharacteristicInfo = (characteristicInfo_t *)((uint8_t *)pServiceInfo + sizeof(serviceInfo_t));

        pServiceInfo->pCharacteristicInfo[0].uuidType    = gBleUuidType16_c;
        pServiceInfo->pCharacteristicInfo[0].uuid.uuid16 = gBleSig_BatteryLevel_d;
        pServiceInfo->pCharacteristicInfo[0].properties =
            (gattCharacteristicPropertiesBitFields_t)(gGattCharPropNotify_c | gGattCharPropRead_c);
        pServiceInfo->pCharacteristicInfo[0].maxValueLength = 0;
        pServiceInfo->pCharacteristicInfo[0].valueLength    = sizeof(bBatteryLevelValue);
        pServiceInfo->pCharacteristicInfo[0].pValue         = (uint8_t *)pServiceInfo->pCharacteristicInfo +
                                                      1 * sizeof(characteristicInfo_t) + 1 * sizeof(descriptorInfo_t);
        FLib_MemCpy(pServiceInfo->pCharacteristicInfo[0].pValue, &bBatteryLevelValue, sizeof(bBatteryLevelValue));
        pServiceInfo->pCharacteristicInfo[0].accessPermissions = gPermissionFlagReadable_c;
        pServiceInfo->pCharacteristicInfo[0].bAddCccd          = TRUE;
        pServiceInfo->pCharacteristicInfo[0].nbOfDescriptors   = 1;
        pServiceInfo->pCharacteristicInfo[0].pDescriptorInfo =
            (descriptorInfo_t *)((uint8_t *)pServiceInfo->pCharacteristicInfo + 1 * sizeof(characteristicInfo_t));

        pServiceInfo->pCharacteristicInfo[0].pDescriptorInfo[0].uuidType    = gBleUuidType16_c;
        pServiceInfo->pCharacteristicInfo[0].pDescriptorInfo[0].uuid.uuid16 = gBleSig_CharPresFormatDescriptor_d;
        pServiceInfo->pCharacteristicInfo[0].pDescriptorInfo[0].valueLength = sizeof(bCharPresFormatDescriptorValue);
        pServiceInfo->pCharacteristicInfo[0].pDescriptorInfo[0].pValue =
            (uint8_t *)pServiceInfo->pCharacteristicInfo[0].pValue + sizeof(bBatteryLevelValue);
        FLib_MemCpy(pServiceInfo->pCharacteristicInfo[0].pDescriptorInfo[0].pValue, &bCharPresFormatDescriptorValue,
                    sizeof(bCharPresFormatDescriptorValue));
        pServiceInfo->pCharacteristicInfo[0].pDescriptorInfo[0].accessPermissions = gPermissionFlagReadable_c;
    }

    return pServiceInfo;
}

serviceInfo_t *GattDbDynamic_GetDeviceInformationService()
{
    uint8_t diManufacturerNameValue[] = mGattDbDynamic_DIServiceManufNameInitValue;
    uint8_t diModelNumberValue[]      = mGattDbDynamic_DIServiceModelNbInitValue;
    uint8_t diSerialNumberValue[]     = mGattDbDynamic_DIServiceSerialNoInitValue;
    uint8_t diHardwareRevisionValue[] = mGattDbDynamic_DIServiceHwRevInitValue;
    uint8_t diFirmwareRevisionValue[] = mGattDbDynamic_DIServiceFwRevInitValue;
    uint8_t diSoftwareRevisionValue[] = mGattDbDynamic_DIServiceSwRevInitValue;
    uint8_t diSystemIdValue[]         = mGattDbDynamic_DIServiceSysIdInitValue;
    uint8_t diIeeeRcdlValue[]         = mGattDbDynamic_DIServiceIeeeRcdlInitValue;
    serviceInfo_t *pServiceInfo       = (serviceInfo_t *)MEM_BufferAlloc(
        sizeof(serviceInfo_t) + 8 * sizeof(characteristicInfo_t) + 0 * sizeof(descriptorInfo_t) +
        sizeof(diManufacturerNameValue) + sizeof(diModelNumberValue) + sizeof(diSerialNumberValue) +
        sizeof(diHardwareRevisionValue) + sizeof(diFirmwareRevisionValue) + sizeof(diSoftwareRevisionValue) +
        sizeof(diSystemIdValue) + sizeof(diIeeeRcdlValue));

    if (NULL != pServiceInfo)
    {
        pServiceInfo->uuidType            = gBleUuidType16_c;
        pServiceInfo->uuid.uuid16         = gBleSig_DeviceInformationService_d;
        pServiceInfo->nbOfCharacteristics = 8;
        pServiceInfo->pCharacteristicInfo = (characteristicInfo_t *)((uint8_t *)pServiceInfo + sizeof(serviceInfo_t));

        pServiceInfo->pCharacteristicInfo[0].uuidType       = gBleUuidType16_c;
        pServiceInfo->pCharacteristicInfo[0].uuid.uuid16    = gBleSig_ManufacturerNameString_d;
        pServiceInfo->pCharacteristicInfo[0].properties     = gGattCharPropRead_c;
        pServiceInfo->pCharacteristicInfo[0].maxValueLength = 0;
        pServiceInfo->pCharacteristicInfo[0].valueLength    = sizeof(diManufacturerNameValue);
        pServiceInfo->pCharacteristicInfo[0].pValue =
            (uint8_t *)pServiceInfo->pCharacteristicInfo + 8 * sizeof(characteristicInfo_t);
        FLib_MemCpy(pServiceInfo->pCharacteristicInfo[0].pValue, &diManufacturerNameValue,
                    sizeof(diManufacturerNameValue));
        pServiceInfo->pCharacteristicInfo[0].accessPermissions = gPermissionFlagReadable_c;
        pServiceInfo->pCharacteristicInfo[0].bAddCccd          = FALSE;
        pServiceInfo->pCharacteristicInfo[0].nbOfDescriptors   = 0;
        pServiceInfo->pCharacteristicInfo[0].pDescriptorInfo   = NULL;

        pServiceInfo->pCharacteristicInfo[1].uuidType       = gBleUuidType16_c;
        pServiceInfo->pCharacteristicInfo[1].uuid.uuid16    = gBleSig_ModelNumberString_d;
        pServiceInfo->pCharacteristicInfo[1].properties     = gGattCharPropRead_c;
        pServiceInfo->pCharacteristicInfo[1].maxValueLength = 0;
        pServiceInfo->pCharacteristicInfo[1].valueLength    = sizeof(diModelNumberValue);
        pServiceInfo->pCharacteristicInfo[1].pValue =
            pServiceInfo->pCharacteristicInfo[0].pValue + sizeof(diManufacturerNameValue);
        FLib_MemCpy(pServiceInfo->pCharacteristicInfo[1].pValue, &diModelNumberValue, sizeof(diModelNumberValue));
        pServiceInfo->pCharacteristicInfo[1].accessPermissions = gPermissionFlagReadable_c;
        pServiceInfo->pCharacteristicInfo[1].bAddCccd          = FALSE;
        pServiceInfo->pCharacteristicInfo[1].nbOfDescriptors   = 0;
        pServiceInfo->pCharacteristicInfo[1].pDescriptorInfo   = NULL;

        pServiceInfo->pCharacteristicInfo[2].uuidType       = gBleUuidType16_c;
        pServiceInfo->pCharacteristicInfo[2].uuid.uuid16    = gBleSig_SerialNumberString_d;
        pServiceInfo->pCharacteristicInfo[2].properties     = gGattCharPropRead_c;
        pServiceInfo->pCharacteristicInfo[2].maxValueLength = 0;
        pServiceInfo->pCharacteristicInfo[2].valueLength    = sizeof(diSerialNumberValue);
        pServiceInfo->pCharacteristicInfo[2].pValue =
            pServiceInfo->pCharacteristicInfo[1].pValue + sizeof(diModelNumberValue);
        FLib_MemCpy(pServiceInfo->pCharacteristicInfo[2].pValue, &diSerialNumberValue, sizeof(diSerialNumberValue));
        pServiceInfo->pCharacteristicInfo[2].accessPermissions = gPermissionFlagReadable_c;
        pServiceInfo->pCharacteristicInfo[2].bAddCccd          = FALSE;
        pServiceInfo->pCharacteristicInfo[2].nbOfDescriptors   = 0;
        pServiceInfo->pCharacteristicInfo[2].pDescriptorInfo   = NULL;

        pServiceInfo->pCharacteristicInfo[3].uuidType       = gBleUuidType16_c;
        pServiceInfo->pCharacteristicInfo[3].uuid.uuid16    = gBleSig_HardwareRevisionString_d;
        pServiceInfo->pCharacteristicInfo[3].properties     = gGattCharPropRead_c;
        pServiceInfo->pCharacteristicInfo[3].maxValueLength = 0;
        pServiceInfo->pCharacteristicInfo[3].valueLength    = sizeof(diHardwareRevisionValue);
        pServiceInfo->pCharacteristicInfo[3].pValue =
            pServiceInfo->pCharacteristicInfo[2].pValue + sizeof(diSerialNumberValue);
        FLib_MemCpy(pServiceInfo->pCharacteristicInfo[3].pValue, &diHardwareRevisionValue,
                    sizeof(diHardwareRevisionValue));
        pServiceInfo->pCharacteristicInfo[3].accessPermissions = gPermissionFlagReadable_c;
        pServiceInfo->pCharacteristicInfo[3].bAddCccd          = FALSE;
        pServiceInfo->pCharacteristicInfo[3].nbOfDescriptors   = 0;
        pServiceInfo->pCharacteristicInfo[3].pDescriptorInfo   = NULL;

        pServiceInfo->pCharacteristicInfo[4].uuidType       = gBleUuidType16_c;
        pServiceInfo->pCharacteristicInfo[4].uuid.uuid16    = gBleSig_FirmwareRevisionString_d;
        pServiceInfo->pCharacteristicInfo[4].properties     = gGattCharPropRead_c;
        pServiceInfo->pCharacteristicInfo[4].maxValueLength = 0;
        pServiceInfo->pCharacteristicInfo[4].valueLength    = sizeof(diFirmwareRevisionValue);
        pServiceInfo->pCharacteristicInfo[4].pValue =
            pServiceInfo->pCharacteristicInfo[3].pValue + sizeof(diHardwareRevisionValue);
        FLib_MemCpy(pServiceInfo->pCharacteristicInfo[4].pValue, &diFirmwareRevisionValue,
                    sizeof(diFirmwareRevisionValue));
        pServiceInfo->pCharacteristicInfo[4].accessPermissions = gPermissionFlagReadable_c;
        pServiceInfo->pCharacteristicInfo[4].bAddCccd          = FALSE;
        pServiceInfo->pCharacteristicInfo[4].nbOfDescriptors   = 0;
        pServiceInfo->pCharacteristicInfo[4].pDescriptorInfo   = NULL;

        pServiceInfo->pCharacteristicInfo[5].uuidType       = gBleUuidType16_c;
        pServiceInfo->pCharacteristicInfo[5].uuid.uuid16    = gBleSig_SoftwareRevisionString_d;
        pServiceInfo->pCharacteristicInfo[5].properties     = gGattCharPropRead_c;
        pServiceInfo->pCharacteristicInfo[5].maxValueLength = 0;
        pServiceInfo->pCharacteristicInfo[5].valueLength    = sizeof(diSoftwareRevisionValue);
        pServiceInfo->pCharacteristicInfo[5].pValue =
            pServiceInfo->pCharacteristicInfo[4].pValue + sizeof(diFirmwareRevisionValue);
        FLib_MemCpy(pServiceInfo->pCharacteristicInfo[5].pValue, &diSoftwareRevisionValue,
                    sizeof(diSoftwareRevisionValue));
        pServiceInfo->pCharacteristicInfo[5].accessPermissions = gPermissionFlagReadable_c;
        pServiceInfo->pCharacteristicInfo[5].bAddCccd          = FALSE;
        pServiceInfo->pCharacteristicInfo[5].nbOfDescriptors   = 0;
        pServiceInfo->pCharacteristicInfo[5].pDescriptorInfo   = NULL;

        pServiceInfo->pCharacteristicInfo[6].uuidType       = gBleUuidType16_c;
        pServiceInfo->pCharacteristicInfo[6].uuid.uuid16    = gBleSig_SystemId_d;
        pServiceInfo->pCharacteristicInfo[6].properties     = gGattCharPropRead_c;
        pServiceInfo->pCharacteristicInfo[6].maxValueLength = 0;
        pServiceInfo->pCharacteristicInfo[6].valueLength    = sizeof(diSystemIdValue);
        pServiceInfo->pCharacteristicInfo[6].pValue =
            pServiceInfo->pCharacteristicInfo[5].pValue + sizeof(diSoftwareRevisionValue);
        FLib_MemCpy(pServiceInfo->pCharacteristicInfo[6].pValue, &diSystemIdValue, sizeof(diSystemIdValue));
        pServiceInfo->pCharacteristicInfo[6].accessPermissions = gPermissionFlagReadable_c;
        pServiceInfo->pCharacteristicInfo[6].bAddCccd          = FALSE;
        pServiceInfo->pCharacteristicInfo[6].nbOfDescriptors   = 0;
        pServiceInfo->pCharacteristicInfo[6].pDescriptorInfo   = NULL;

        pServiceInfo->pCharacteristicInfo[7].uuidType       = gBleUuidType16_c;
        pServiceInfo->pCharacteristicInfo[7].uuid.uuid16    = gBleSig_IeeeRcdl_d;
        pServiceInfo->pCharacteristicInfo[7].properties     = gGattCharPropRead_c;
        pServiceInfo->pCharacteristicInfo[7].maxValueLength = 0;
        pServiceInfo->pCharacteristicInfo[7].valueLength    = sizeof(diIeeeRcdlValue);
        pServiceInfo->pCharacteristicInfo[7].pValue =
            pServiceInfo->pCharacteristicInfo[6].pValue + sizeof(diSystemIdValue);
        FLib_MemCpy(pServiceInfo->pCharacteristicInfo[7].pValue, &diIeeeRcdlValue, sizeof(diIeeeRcdlValue));
        pServiceInfo->pCharacteristicInfo[7].accessPermissions = gPermissionFlagReadable_c;
        pServiceInfo->pCharacteristicInfo[7].bAddCccd          = FALSE;
        pServiceInfo->pCharacteristicInfo[7].nbOfDescriptors   = 0;
        pServiceInfo->pCharacteristicInfo[7].pDescriptorInfo   = NULL;
    }

    return pServiceInfo;
}

/******************************************************;******************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
