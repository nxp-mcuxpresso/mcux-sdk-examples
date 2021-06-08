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

#include "fsci_ble_gatt_db_app.h"
#include "host_ble.h"

/************************************************************************************
*************************************************************************************
* Private constants & macros
*************************************************************************************
************************************************************************************/

/* Macro used for saving the out parameters pointers of the GATT Database
(application) functions */
#define fsciBleGattDbAppSaveOutParams(pFirstParam, pSecondParam) \
    fsciBleGattDbAppOutParams.pParam1 = (uint8_t *)pFirstParam;  \
    fsciBleGattDbAppOutParams.pParam2 = (uint8_t *)pSecondParam

/* Macro used for restoring the out parameters pointers of the GATT Database
(application) functions */
#define fsciBleGattDbAppRestoreOutParams() &fsciBleGattDbAppOutParams;

/* Macro used for cleaning the out parameters pointers of the GATT Database
(application) functions to NULL */
#define fsciBleGattDbAppCleanOutParams()      \
    fsciBleGattDbAppOutParams.pParam1 = NULL; \
    fsciBleGattDbAppOutParams.pParam2 = NULL

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

/* Structure used for keeping the out parameters pointers of the GATT Database
(application) functions */
typedef struct fsciBleGattDbAppOutParams_tag
{
    uint8_t *pParam1;
    uint8_t *pParam2;
} fsciBleGattDbAppOutParams_t;

/* Structure used for keeping the out parameters pointers of the
GattDb_ReadAttribute function */
typedef struct fsciBleGattDbAppReadAttributeOutParams_tag
{
    uint8_t *pValue;
    uint16_t *pValueLength;
} fsciBleGattDbAppReadAttributeOutParams_t;

/* Structure used for keeping the out parameter pointer of the
GattDb_FindServiceHandle function */
typedef struct fsciBleGattDbAppFindServiceHandleOutParams_tag
{
    uint16_t *pOutServiceHandle;
} fsciBleGattDbAppFindServiceHandleOutParams_t;

/* Structure used for keeping the out parameter pointer of the
GattDb_FindCharValueHandleInService function */
typedef struct fsciBleGattDbAppFindCharValueHandleInServiceOutParams_tag
{
    uint16_t *pCharValueHandle;
} fsciBleGattDbAppFindCharValueHandleInServiceOutParams_t;

/* Structure used for keeping the out parameter pointer of the
GattDb_FindCccdHandleForCharValueHandle function */
typedef struct fsciBleGattDbAppFindCccdHandleForCharValueHandleOutParams_tag
{
    uint16_t *pCccdHandle;
} fsciBleGattDbAppFindCccdHandleForCharValueHandleOutParams_t;

/* Structure used for keeping the out parameter pointer of the
GattDb_FindDescriptorHandleForCharValueHandle function */
typedef struct fsciBleGattDbAppFindDescriptorHandleForCharValueHandleOutParams_tag
{
    uint16_t *pDescriptorHandle;
} fsciBleGattDbAppFindDescriptorHandleForCharValueHandleOutParams_t;

/* Structure used for keeping the out parameters pointers of the
GattDbDynamic_AddPrimaryServiceDeclaration function */
typedef struct fsciBleGattDbAppAddPrimaryServiceDeclarationOutParams_tag
{
    uint16_t *pOutHandle;
} fsciBleGattDbAppAddPrimaryServiceDeclarationOutParams_t;

/* Structure used for keeping the out parameters pointers of the
GattDbDynamic_AddSecondaryServiceDeclaration function */
typedef struct fsciBleGattDbAppAddSecondaryServiceDeclarationOutParams_tag
{
    uint16_t *pOutHandle;
} fsciBleGattDbAppAddSecondaryServiceDeclarationOutParams_t;

/* Structure used for keeping the out parameters pointers of the
GattDbDynamic_AddIncludeDeclaration function */
typedef struct fsciBleGattDbAppAddIncludeDeclarationOutParams_tag
{
    uint16_t *pOutHandle;
} fsciBleGattDbAppAddIncludeDeclarationOutParams_t;

/* Structure used for keeping the out parameters pointers of the
GattDbDynamic_AddCharacteristicDeclarationAndValue function */
typedef struct fsciBleGattDbAppAddCharacteristicDeclarationAndValueOutParams_tag
{
    uint16_t *pOutHandle;
} fsciBleGattDbAppAddCharacteristicDeclarationAndValueOutParams_t;

/* Structure used for keeping the out parameters pointers of the
GattDbDynamic_AddCharacteristicDescriptor function */
typedef struct fsciBleGattDbAppAddCharacteristicDescriptorOutParams_tag
{
    uint16_t *pOutHandle;
} fsciBleGattDbAppAddCharacteristicDescriptorOutParams_t;

/* Structure used for keeping the out parameters pointers of the
GattDbDynamic_AddCccd function */
typedef struct fsciBleGattDbAppAddCccdOutParams_tag
{
    uint16_t *pOutHandle;
} fsciBleGattDbAppAddCccdOutParams_t;

/* Structure used for keeping the out parameters pointers of the
GattDbDynamic_AddCharacteristicDeclarationWithUniqueValue function */
typedef struct fsciBleGattDbAppAddCharacteristicDeclarationWithUniqueValueOutParams_tag
{
    uint16_t *pOutHandle;
} fsciBleGattDbAppAddCharacteristicDeclarationWithUniqueValueOutParams_t;

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/* Keeps out parameters pointers for Host - BBox functionality */
static fsciBleGattDbAppOutParams_t fsciBleGattDbAppOutParams = {NULL, NULL};

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

void fsciBleGattDbAppHandler(void *pData, void *pParam, uint32_t fsciBleInterfaceId)
{
    clientPacket_t *pClientPacket = (clientPacket_t *)pData;
    uint8_t *pBuffer              = &pClientPacket->structured.payload[0];

    /* Select the GATT Database (application) function to be called (using the FSCI opcode) */
    switch (pClientPacket->structured.header.opCode)
    {
        case gBleGattDbAppStatusOpCode_c:
        {
            bleResult_t status;

            fsciBleGetEnumValueFromBuffer(status, pBuffer, bleResult_t);

            if (gBleSuccess_c != status)
            {
                /* Clean out parameters pointers kept in FSCI */
                fsciBleGattDbAppCleanOutParams();
            }
        }
        break;

        case gBleGattDbAppEvtReadAttributeValueOpCode_c:
        {
            fsciBleGattDbAppReadAttributeOutParams_t *pOutParams =
                (fsciBleGattDbAppReadAttributeOutParams_t *)fsciBleGattDbAppRestoreOutParams();

            /* Get out parameters of the GattDb_ReadAttribute function from buffer */
            if (NULL != pOutParams->pValueLength)
            {
                fsciBleGetUint16ValueFromBuffer(*pOutParams->pValueLength, pBuffer);
            }

            if (NULL != pOutParams->pValue && NULL != pOutParams->pValueLength)
            {
                fsciBleGetArrayFromBuffer(pOutParams->pValue, pBuffer, *pOutParams->pValueLength);
            }

            if ((NULL != pOutParams->pValueLength) || (NULL != pOutParams->pValue))
            {
                /* Clean out parameters pointers kept in FSCI */
                fsciBleGattDbAppCleanOutParams();

                /* Signal out parameters ready */
                Ble_OutParamsReady();
            }
        }
        break;

        case gBleGattDbAppEvtFindServiceHandleOpCode_c:
        {
            fsciBleGattDbAppFindServiceHandleOutParams_t *pOutParams =
                (fsciBleGattDbAppFindServiceHandleOutParams_t *)fsciBleGattDbAppRestoreOutParams();

            if (NULL != pOutParams->pOutServiceHandle)
            {
                /* Get out parameters of the GattDb_FindServiceHandle function from buffer */
                fsciBleGetUint16ValueFromBuffer(*pOutParams->pOutServiceHandle, pBuffer);

                /* Clean out parameters pointers kept in FSCI */
                fsciBleGattDbAppCleanOutParams();

                /* Signal out parameter ready */
                Ble_OutParamsReady();
            }
        }
        break;

        case gBleGattDbAppEvtFindCharValueHandleInServiceOpCode_c:
        {
            fsciBleGattDbAppFindCharValueHandleInServiceOutParams_t *pOutParams =
                (fsciBleGattDbAppFindCharValueHandleInServiceOutParams_t *)fsciBleGattDbAppRestoreOutParams();

            if (NULL != pOutParams->pCharValueHandle)
            {
                /* Get out parameters of the GattDb_FindCharValueHandleInService function from buffer */
                fsciBleGetUint16ValueFromBuffer(*pOutParams->pCharValueHandle, pBuffer);

                /* Clean out parameters pointers kept in FSCI */
                fsciBleGattDbAppCleanOutParams();

                /* Signal out parameter ready */
                Ble_OutParamsReady();
            }
        }
        break;

        case gBleGattDbAppEvtFindCccdHandleForCharValueHandleOpCode_c:
        {
            fsciBleGattDbAppFindCccdHandleForCharValueHandleOutParams_t *pOutParams =
                (fsciBleGattDbAppFindCccdHandleForCharValueHandleOutParams_t *)fsciBleGattDbAppRestoreOutParams();

            if (NULL != pOutParams->pCccdHandle)
            {
                /* Get out parameters of the GattDb_FindCccdHandleForCharValueHandle function from buffer */
                fsciBleGetUint16ValueFromBuffer(*pOutParams->pCccdHandle, pBuffer);

                /* Clean out parameters pointers kept in FSCI */
                fsciBleGattDbAppCleanOutParams();

                /* Signal out parameter ready */
                Ble_OutParamsReady();
            }
        }
        break;

        case gBleGattDbAppEvtFindDescriptorHandleForCharValueHandleOpCode_c:
        {
            fsciBleGattDbAppFindDescriptorHandleForCharValueHandleOutParams_t *pOutParams =
                (fsciBleGattDbAppFindDescriptorHandleForCharValueHandleOutParams_t *)fsciBleGattDbAppRestoreOutParams();

            if (NULL != pOutParams->pDescriptorHandle)
            {
                /* Get out parameters of the GattDb_FindDescriptorHandleForCharValueHandle function from buffer */
                fsciBleGetUint16ValueFromBuffer(*pOutParams->pDescriptorHandle, pBuffer);

                /* Clean out parameters pointers kept in FSCI */
                fsciBleGattDbAppCleanOutParams();

                /* Signal out parameter ready */
                Ble_OutParamsReady();
            }
        }
        break;

        case gBleGattDbAppEvtAddPrimaryServiceDeclarationOpCode_c:
        {
            fsciBleGattDbAppAddPrimaryServiceDeclarationOutParams_t *pOutParams =
                (fsciBleGattDbAppAddPrimaryServiceDeclarationOutParams_t *)fsciBleGattDbAppRestoreOutParams();

            if (NULL != pOutParams->pOutHandle)
            {
                /* Get out parameters of the GattDbDynamic_AddPrimaryServiceDeclaration function from buffer */
                fsciBleGetUint16ValueFromBuffer(*pOutParams->pOutHandle, pBuffer);

                /* Clean out parameters pointers kept in FSCI */
                fsciBleGattDbAppCleanOutParams();
            }

            /* Signal out parameter ready */
            Ble_OutParamsReady();
        }
        break;

        case gBleGattDbAppEvtAddSecondaryServiceDeclarationOpCode_c:
        {
            fsciBleGattDbAppAddSecondaryServiceDeclarationOutParams_t *pOutParams =
                (fsciBleGattDbAppAddSecondaryServiceDeclarationOutParams_t *)fsciBleGattDbAppRestoreOutParams();

            if (NULL != pOutParams->pOutHandle)
            {
                /* Get out parameters of the GattDbDynamic_AddSecondaryServiceDeclaration function from buffer */
                fsciBleGetUint16ValueFromBuffer(*pOutParams->pOutHandle, pBuffer);

                /* Clean out parameters pointers kept in FSCI */
                fsciBleGattDbAppCleanOutParams();
            }

            /* Signal out parameter ready */
            Ble_OutParamsReady();
        }
        break;

        case gBleGattDbAppEvtAddIncludeDeclarationOpCode_c:
        {
            fsciBleGattDbAppAddIncludeDeclarationOutParams_t *pOutParams =
                (fsciBleGattDbAppAddIncludeDeclarationOutParams_t *)fsciBleGattDbAppRestoreOutParams();

            if (NULL != pOutParams->pOutHandle)
            {
                /* Get out parameters of the GattDbDynamic_AddIncludeDeclaration function from buffer */
                fsciBleGetUint16ValueFromBuffer(*pOutParams->pOutHandle, pBuffer);

                /* Clean out parameters pointers kept in FSCI */
                fsciBleGattDbAppCleanOutParams();
            }

            /* Signal out parameter ready */
            Ble_OutParamsReady();
        }
        break;

        case gBleGattDbAppEvtAddCharacteristicDeclarationAndValueOpCode_c:
        {
            fsciBleGattDbAppAddCharacteristicDeclarationAndValueOutParams_t *pOutParams =
                (fsciBleGattDbAppAddCharacteristicDeclarationAndValueOutParams_t *)fsciBleGattDbAppRestoreOutParams();

            if (NULL != pOutParams->pOutHandle)
            {
                /* Get out parameters of the GattDbDynamic_AddCharacteristicDeclarationAndValue function from buffer */
                fsciBleGetUint16ValueFromBuffer(*pOutParams->pOutHandle, pBuffer);

                /* Clean out parameters pointers kept in FSCI */
                fsciBleGattDbAppCleanOutParams();
            }

            /* Signal out parameter ready */
            Ble_OutParamsReady();
        }
        break;

        case gBleGattDbAppEvtAddCharacteristicDescriptorOpCode_c:
        {
            fsciBleGattDbAppAddCharacteristicDescriptorOutParams_t *pOutParams =
                (fsciBleGattDbAppAddCharacteristicDescriptorOutParams_t *)fsciBleGattDbAppRestoreOutParams();

            if (NULL != pOutParams->pOutHandle)
            {
                /* Get out parameters of the GattDbDynamic_AddCharacteristicDescriptor function from buffer */
                fsciBleGetUint16ValueFromBuffer(*pOutParams->pOutHandle, pBuffer);

                /* Clean out parameters pointers kept in FSCI */
                fsciBleGattDbAppCleanOutParams();
            }

            /* Signal out parameter ready */
            Ble_OutParamsReady();
        }
        break;

        case gBleGattDbAppEvtAddCccdOpCode_c:
        {
            fsciBleGattDbAppAddCccdOutParams_t *pOutParams =
                (fsciBleGattDbAppAddCccdOutParams_t *)fsciBleGattDbAppRestoreOutParams();

            if (NULL != pOutParams->pOutHandle)
            {
                /* Get out parameters of the GattDbDynamic_AddCccd function from buffer */
                fsciBleGetUint16ValueFromBuffer(*pOutParams->pOutHandle, pBuffer);

                /* Clean out parameters pointers kept in FSCI */
                fsciBleGattDbAppCleanOutParams();
            }

            /* Signal out parameter ready */
            Ble_OutParamsReady();
        }
        break;

        case gBleGattDbAppEvtAddCharacteristicDeclarationWithUniqueValueOpCode_c:
        {
            fsciBleGattDbAppAddCharacteristicDeclarationWithUniqueValueOutParams_t *pOutParams =
                (fsciBleGattDbAppAddCharacteristicDeclarationWithUniqueValueOutParams_t *)
                    fsciBleGattDbAppRestoreOutParams();

            if (NULL != pOutParams->pOutHandle)
            {
                /* Get out parameters of the GattDbDynamic_AddCharacteristicDeclarationWithUniqueValue function from
                 * buffer */
                fsciBleGetUint16ValueFromBuffer(*pOutParams->pOutHandle, pBuffer);

                /* Clean out parameters pointers kept in FSCI */
                fsciBleGattDbAppCleanOutParams();
            }

            /* Signal out parameter ready */
            Ble_OutParamsReady();
        }
        break;

        default:
        {
            /* Unknown FSCI opcode */
            FSCI_Error(gFsciUnknownOpcode_c, fsciBleInterfaceId);
        }
        break;
    }

    MEM_BufferFree(pData);
}

void fsciBleGattDbAppNoParamCmdMonitor(fsciBleGattDbAppOpCode_t opCode)
{
    /* Call the generic FSCI BLE monitor for commands or events that have no parameters */
    fsciBleNoParamCmdOrEvtMonitor(gFsciBleGattDbAppOpcodeGroup_c, opCode);
}

void fsciBleGattDbAppUint16ParamCmdMonitor(fsciBleGattDbAppOpCode_t opCode, uint16_t value)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGattDbAppAllocFsciPacket(opCode, sizeof(uint16_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromUint16Value(value, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattDbAppUuidTypeAndUuidParamCmdMonitor(fsciBleGattDbAppOpCode_t opCode,
                                                    bleUuidType_t uuidType,
                                                    bleUuid_t *pUuid,
                                                    uint16_t *pOutHandle)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGattDbAppAllocFsciPacket(opCode, sizeof(bleUuidType_t) + fsciBleGetUuidBufferSize(uuidType));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromEnumValue(uuidType, pBuffer, bleUuidType_t);
    fsciBleGetBufferFromUuid(pUuid, &pBuffer, uuidType);

    /* Save out parameters pointers */
    fsciBleGattDbAppSaveOutParams(pOutHandle, NULL);
    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattDbAppWriteAttributeCmdMonitor(uint16_t handle, uint16_t valueLength, uint8_t *aValue)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGattDbAppAllocFsciPacket(gBleGattDbAppCmdWriteAttributeOpCode_c,
                                                    sizeof(uint16_t) + sizeof(uint16_t) + valueLength);

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromUint16Value(handle, pBuffer);
    fsciBleGetBufferFromUint16Value(valueLength, pBuffer);
    fsciBleGetBufferFromArray(aValue, pBuffer, valueLength);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattDbAppReadAttributeCmdMonitor(uint16_t handle,
                                             uint16_t maxBytes,
                                             uint8_t *aOutValue,
                                             uint16_t *pOutValueLength)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket =
        fsciBleGattDbAppAllocFsciPacket(gBleGattDbAppCmdReadAttributeOpCode_c, sizeof(uint16_t) + sizeof(uint16_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromUint16Value(handle, pBuffer);
    fsciBleGetBufferFromUint16Value(maxBytes, pBuffer);

    /* Save out parameters pointers */
    fsciBleGattDbAppSaveOutParams(aOutValue, pOutValueLength);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattDbAppFindCccdHandleForCharValueHandleCmdMonitor(uint16_t charValueHandle, uint16_t *pOutCccdHandle)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket =
        fsciBleGattDbAppAllocFsciPacket(gBleGattDbAppCmdFindCccdHandleForCharValueHandleOpCode_c, sizeof(uint16_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromUint16Value(charValueHandle, pBuffer);

    /* Save out parameters pointers */
    fsciBleGattDbAppSaveOutParams(pOutCccdHandle, NULL);
    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattDbAppFindServiceCharValueOrDescriptorHandleCmdMonitor(
    fsciBleGattDbAppOpCode_t opCode, uint16_t handle, bleUuidType_t uuidType, bleUuid_t *pUuid, uint16_t *pOutHandle)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGattDbAppAllocFsciPacket(
        opCode, sizeof(uint16_t) + sizeof(bleUuidType_t) + fsciBleGetUuidBufferSize(uuidType));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromUint16Value(handle, pBuffer);
    fsciBleGetBufferFromEnumValue(uuidType, pBuffer, bleUuidType_t);
    fsciBleGetBufferFromUuid(pUuid, &pBuffer, uuidType);

    /* Save out parameters pointers */
    fsciBleGattDbAppSaveOutParams(pOutHandle, NULL);
    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattDbAppAddIncludeDeclarationCmdMonitor(uint16_t includedServiceHandle,
                                                     uint16_t endGroupHandle,
                                                     bleUuidType_t serviceUuidType,
                                                     bleUuid_t *pServiceUuid,
                                                     uint16_t *pOutHandle)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGattDbAppAllocFsciPacket(
        gBleGattDbAppCmdAddIncludeDeclarationOpCode_c,
        sizeof(uint16_t) + sizeof(uint16_t) + sizeof(bleUuidType_t) + fsciBleGetUuidBufferSize(serviceUuidType));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromUint16Value(includedServiceHandle, pBuffer);
    fsciBleGetBufferFromUint16Value(endGroupHandle, pBuffer);
    fsciBleGetBufferFromEnumValue(serviceUuidType, pBuffer, bleUuidType_t);
    fsciBleGetBufferFromUuid(pServiceUuid, &pBuffer, serviceUuidType);

    /* Save out parameters pointers */
    fsciBleGattDbAppSaveOutParams(pOutHandle, NULL);
    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattDbAppAddCharacteristicDeclarationAndValueCmdMonitor(
    bleUuidType_t characteristicUuidType,
    bleUuid_t *pCharacteristicUuid,
    gattCharacteristicPropertiesBitFields_t characteristicProperties,
    uint16_t maxValueLength,
    uint16_t initialValueLength,
    uint8_t *aInitialValue,
    gattAttributePermissionsBitFields_t valueAccessPermissions,
    uint16_t *pOutHandle)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGattDbAppAllocFsciPacket(
        gBleGattDbAppCmdAddCharacteristicDeclarationAndValueOpCode_c,
        sizeof(bleUuidType_t) + fsciBleGetUuidBufferSize(characteristicUuidType) +
            sizeof(gattCharacteristicPropertiesBitFields_t) + sizeof(uint16_t) + sizeof(uint16_t) + initialValueLength +
            sizeof(gattAttributePermissionsBitFields_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromEnumValue(characteristicUuidType, pBuffer, bleUuidType_t);
    fsciBleGetBufferFromUuid(pCharacteristicUuid, &pBuffer, characteristicUuidType);
    fsciBleGetBufferFromEnumValue(characteristicProperties, pBuffer, gattCharacteristicPropertiesBitFields_t);
    fsciBleGetBufferFromUint16Value(maxValueLength, pBuffer);
    fsciBleGetBufferFromUint16Value(initialValueLength, pBuffer);
    fsciBleGetBufferFromArray(aInitialValue, pBuffer, initialValueLength);
    fsciBleGetBufferFromEnumValue(valueAccessPermissions, pBuffer, gattAttributePermissionsBitFields_t);

    /* Save out parameters pointers */
    fsciBleGattDbAppSaveOutParams(pOutHandle, NULL);
    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattDbAppAddCharacteristicDeclarationWithUniqueValueCmdMonitor(
    bleUuidType_t characteristicUuidType,
    bleUuid_t *pCharacteristicUuid,
    gattCharacteristicPropertiesBitFields_t characteristicProperties,
    gattAttributePermissionsBitFields_t valueAccessPermissions,
    uint16_t *pOutHandle)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGattDbAppAllocFsciPacket(
        gBleGattDbAppCmdAddCharacteristicDeclarationWithUniqueValueOpCode_c,
        sizeof(bleUuidType_t) + fsciBleGetUuidBufferSize(characteristicUuidType) +
            sizeof(gattCharacteristicPropertiesBitFields_t) + sizeof(gattAttributePermissionsBitFields_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromEnumValue(characteristicUuidType, pBuffer, bleUuidType_t);
    fsciBleGetBufferFromUuid(pCharacteristicUuid, &pBuffer, characteristicUuidType);
    fsciBleGetBufferFromEnumValue(characteristicProperties, pBuffer, gattCharacteristicPropertiesBitFields_t);
    fsciBleGetBufferFromEnumValue(valueAccessPermissions, pBuffer, gattAttributePermissionsBitFields_t);

    /* Save out parameters pointers */
    fsciBleGattDbAppSaveOutParams(pOutHandle, NULL);
    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattDbAppAddCharacteristicDescriptorCmdMonitor(
    bleUuidType_t descriptorUuidType,
    bleUuid_t *pDescriptorUuid,
    uint16_t descriptorValueLength,
    uint8_t *aInitialValue,
    gattAttributePermissionsBitFields_t descriptorAccessPermissions,
    uint16_t *pOutHandle)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGattDbAppAllocFsciPacket(
        gBleGattDbAppCmdAddCharacteristicDescriptorOpCode_c,
        sizeof(bleUuidType_t) + fsciBleGetUuidBufferSize(descriptorUuidType) + sizeof(uint16_t) +
            descriptorValueLength + sizeof(gattAttributePermissionsBitFields_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromEnumValue(descriptorUuidType, pBuffer, bleUuidType_t);
    fsciBleGetBufferFromUuid(pDescriptorUuid, &pBuffer, descriptorUuidType);
    fsciBleGetBufferFromUint16Value(descriptorValueLength, pBuffer);
    fsciBleGetBufferFromArray(aInitialValue, pBuffer, descriptorValueLength);
    fsciBleGetBufferFromEnumValue(descriptorAccessPermissions, pBuffer, gattAttributePermissionsBitFields_t);

    /* Save out parameters pointers */
    fsciBleGattDbAppSaveOutParams(pOutHandle, NULL);
    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattDbAppAddCccdCmdMonitor(uint16_t *pOutHandle)
{
    /* Save out parameters pointers */
    fsciBleGattDbAppSaveOutParams(pOutHandle, NULL);

    fsciBleGattDbAppNoParamCmdMonitor(gBleGattDbAppCmdAddCccdOpCode_c);
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
