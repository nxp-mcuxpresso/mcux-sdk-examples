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

#include "fsci_ble_gap.h"
#include "host_ble.h"

/************************************************************************************
*************************************************************************************
* Private constants & macros
*************************************************************************************
************************************************************************************/

/* Macro used for saving the out parameters pointers of the GAP functions */
#define fsciBleGapSaveOutParams(pFirstParam, pSecondParam) \
    fsciBleGapOutParams.pParam1 = (uint8_t *)pFirstParam;  \
    fsciBleGapOutParams.pParam2 = (uint8_t *)pSecondParam

/* Macro used for restoring the out parameters pointers of the GAP functions */
#define fsciBleGapRestoreOutParams() &fsciBleGapOutParams

/* Macro used for setting the out parameters pointers of the GAP
functions to NULL */
#define fsciBleGapCleanOutParams()      \
    fsciBleGapOutParams.pParam1 = NULL; \
    fsciBleGapOutParams.pParam2 = NULL;

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

/*! Structure that keeps callbacks registered by application or FSCI in GAP. */
typedef struct fsciBleGapCallbacks_tag
{
    hciHostToControllerInterface_t hostToControllerInterface; /* LE Controller uplink interface function
                                                              pointer. */
    gapGenericCallback_t genericCallback;                     /* Callback used to propagate GAP controller
                                                              events to the application. */
    gapAdvertisingCallback_t advertisingCallback;             /* Callback used by the application to
                                                              receive advertising events. */
    gapScanningCallback_t scanningCallback;                   /* Callback used by the application to
                                                              receive scanning events. */
    gapConnectionCallback_t connectionCallback;               /* Callback used by the application to
                                                              receive connection events. */
} fsciBleGapCallbacks_t;

/* Structure used for keeping the out parameters pointers of the GAP
 functions */
typedef struct fsciBleGapOutParams_tag
{
    uint8_t *pParam1;
    uint8_t *pParam2;
} fsciBleGapOutParams_t;

/* Structure used for keeping the out parameters pointers of the
Gap_CheckNotificationStatus function */
typedef struct fsciBleGapCheckNotificationStatusOutParams_tag
{
    bool_t *pIsActive;
} fsciBleGapCheckNotificationStatusOutParams_t;

/* Structure used for keeping the out parameters pointers of the
Gap_CheckIndicationStatus function */
typedef struct fsciBleGapCheckIndicationStatusOutParams_tag
{
    bool_t *pIsActive;
} fsciBleGapCheckIndicationStatusOutParams_t;

/* Structure used for keeping the out parameters pointers of the
Gap_GetBondedStaticAddresses function */
typedef struct fsciBleGapGetBondedStaticAddressesOutParams_tag
{
    bleDeviceAddress_t *pDeviceAddresses;
    uint8_t *pActualCount;
} fsciBleGapGetBondedStaticAddressesOutParams_t;

/* Structure used for keeping the out parameters pointers of the
Gap_GetBondedDevicesIdentityInformation function */
typedef struct fsciBleGapGetBondedDevicesIdentityInformationOutParams_tag
{
    gapIdentityInformation_t *pIdentityAddresses;
    uint8_t *pActualCount;
} fsciBleGapGetBondedDevicesIdentityInformationParams_t;

/* Structure used for keeping the out parameters pointers of the
Gap_LoadEncryptionInformation function */
typedef struct fsciBleGapLoadEncryptionInformationOutParams_tag
{
    uint8_t *pLtk;
    uint8_t *pLtkSize;
} fsciBleGapLoadEncryptionInformationOutParams_t;

/* Structure used for keeping the out parameters pointers of the
Gap_LoadCustomPeerInformation function */
typedef struct fsciBleGapLoadCustomPeerInformationOutParams_tag
{
    void *pInfo;
} fsciBleGapLoadCustomPeerInformationOutParams_t;

/* Structure used for keeping the out parameters pointers of the
Gap_CheckIfBonded function */
typedef struct fsciBleGapCheckIfBondedOutParams_tag
{
    bool_t *pIsBonded;
} fsciBleGapCheckIfBondedOutParams_t;

/* Structure used for keeping the out parameters pointers of the
Gap_GetBondedDevicesCount function */
typedef struct fsciBleGapGetBondedDevicesCountOutParams_tag
{
    uint8_t *pBondedDevicesCount;
} fsciBleGapGetBondedDevicesCountOutParams_t;

/* Structure used for keeping the out parameters pointers of the
Gap_GetBondedDeviceName function */
typedef struct fsciBleGapGetBondedDeviceNameOutParams_tag
{
    uchar_t *pName;
} fsciBleGapGetBondedDeviceNameOutParams_t;

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

static void fsciBleGapGenericCallback(gapGenericEvent_t *pGenericEvent);
static bleResult_t fsciBleHciHostToControllerInterface(hciPacketType_t packetType, void *pPacket, uint16_t packetSize);
static void fsciBleGapAdvertisingCallback(gapAdvertisingEvent_t *pAdvertisingEvent);
static void fsciBleGapScanningCallback(gapScanningEvent_t *pScanningEvent);
static void fsciBleGapConnectionCallback(deviceId_t deviceId, gapConnectionEvent_t *pConnectionEvent);

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/* GAP callbacks structure initialized with FSCI empty static functions */
static fsciBleGapCallbacks_t fsciBleGapCallbacks = {fsciBleHciHostToControllerInterface, fsciBleGapGenericCallback,
                                                    fsciBleGapAdvertisingCallback, fsciBleGapScanningCallback,
                                                    fsciBleGapConnectionCallback};

/* Keeps out parameters pointers for Host - BBox functionality */
static fsciBleGapOutParams_t fsciBleGapOutParams = {NULL, NULL};

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

void fsciBleSetHciHostToControllerInterface(hciHostToControllerInterface_t hostToControllerInterface)
{
    /* Set HCI host to controller interface to an application desired function, only
    if not NULL. Otherwise set it to the FSCI empty static function */
    fsciBleGapCallbacks.hostToControllerInterface =
        (NULL != hostToControllerInterface) ? hostToControllerInterface : fsciBleHciHostToControllerInterface;
}

void fsciBleSetGapGenericCallback(gapGenericCallback_t genericCallback)
{
    /* Set GAP controller callback to an application desired function, only
    if not NULL. Otherwise set it to the FSCI empty static function */
    fsciBleGapCallbacks.genericCallback = (NULL != genericCallback) ? genericCallback : fsciBleGapGenericCallback;
}

void fsciBleSetGapAdvertisingCallback(gapAdvertisingCallback_t advertisingCallback)
{
    /* Set GAP advertising callback to an application desired function, only
    if not NULL. Otherwise set it to the FSCI empty static function */
    fsciBleGapCallbacks.advertisingCallback =
        (NULL != advertisingCallback) ? advertisingCallback : fsciBleGapAdvertisingCallback;
}

void fsciBleSetGapConnectionCallback(gapConnectionCallback_t connectionCallback)
{
    /* Set GAP connection callback to an application desired function, only
    if not NULL. Otherwise set it to the FSCI empty static function */
    fsciBleGapCallbacks.connectionCallback =
        (NULL != connectionCallback) ? connectionCallback : fsciBleGapConnectionCallback;
}

void fsciBleSetGapScanningCallback(gapScanningCallback_t scanningCallback)
{
    /* Set GAP scanning callback to an application desired function, only
    if not NULL. Otherwise set it to the FSCI empty static function */
    fsciBleGapCallbacks.scanningCallback = (NULL != scanningCallback) ? scanningCallback : fsciBleGapScanningCallback;
}

void fsciBleGapHandler(void *pData, void *param, uint32_t fsciBleInterfaceId)
{
    clientPacket_t *pClientPacket = (clientPacket_t *)pData;
    uint8_t *pBuffer              = &pClientPacket->structured.payload[0];

    /* Select the GAP function to be called (using the FSCI opcode) */
    switch (pClientPacket->structured.header.opCode)
    {
        case gBleGapStatusOpCode_c:
        {
            bleResult_t status;

            /* Get status from buffer */
            fsciBleGetEnumValueFromBuffer(status, pBuffer, bleResult_t);

            if (gBleSuccess_c != status)
            {
                /* Clean out parameters pointers kept in FSCI */
                fsciBleGapCleanOutParams();
            }
        }
        break;

        case gBleGapEvtCheckNotificationStatusOpCode_c:
        {
            fsciBleGapCheckNotificationStatusOutParams_t *pOutParams =
                (fsciBleGapCheckNotificationStatusOutParams_t *)fsciBleGapRestoreOutParams();

            if (NULL != pOutParams->pIsActive)
            {
                /* Get out parameter of the Gap_CheckNotificationStatus function from buffer */
                fsciBleGetBoolValueFromBuffer(*pOutParams->pIsActive, pBuffer);

                /* Clean out parameters pointers kept in FSCI */
                fsciBleGapCleanOutParams();

                /* Signal out parameters ready */
                Ble_OutParamsReady();
            }
        }
        break;

        case gBleGapEvtCheckIndicationStatusOpCode_c:
        {
            fsciBleGapCheckIndicationStatusOutParams_t *pOutParams =
                (fsciBleGapCheckIndicationStatusOutParams_t *)fsciBleGapRestoreOutParams();

            if (NULL != pOutParams->pIsActive)
            {
                /* Get out parameter of the Gap_CheckIndicationStatus function from buffer */
                fsciBleGetBoolValueFromBuffer(*pOutParams->pIsActive, pBuffer);

                /* Clean out parameters pointers kept in FSCI */
                fsciBleGapCleanOutParams();

                /* Signal out parameters ready */
                Ble_OutParamsReady();
            }
        }
        break;

        case gBleGapEvtGetBondedStaticAddressesOpCode_c:
        {
            fsciBleGapGetBondedStaticAddressesOutParams_t *pOutParams =
                (fsciBleGapGetBondedStaticAddressesOutParams_t *)fsciBleGapRestoreOutParams();
            uint32_t iCount;

            if (NULL != pOutParams->pActualCount)
            {
                /* Get number of device addresses from buffer */
                fsciBleGetUint8ValueFromBuffer(*pOutParams->pActualCount, pBuffer);

                if (NULL != pOutParams->pDeviceAddresses)
                {
                    /* Get device addresses from buffer */
                    for (iCount = 0; iCount < *pOutParams->pActualCount; iCount++)
                    {
                        fsciBleGetArrayFromBuffer(pOutParams->pDeviceAddresses[iCount], pBuffer,
                                                  gcBleDeviceAddressSize_c);
                    }
                }

                /* Clean out parameters pointers kept in FSCI */
                fsciBleGapCleanOutParams();

                /* Signal out parameters ready */
                Ble_OutParamsReady();
            }
        }
        break;

        case gBleGapEvtGetBondedDevicesIdentityInformationOpCode_c:
        {
            fsciBleGapGetBondedDevicesIdentityInformationParams_t *pOutParams;
            uint32_t iCount;

            pOutParams = (fsciBleGapGetBondedDevicesIdentityInformationParams_t *)fsciBleGapRestoreOutParams();

            if (NULL != pOutParams->pActualCount)
            {
                /* Get number of device addresses from buffer */
                fsciBleGetUint8ValueFromBuffer(*pOutParams->pActualCount, pBuffer);

                if (NULL != pOutParams->pIdentityAddresses)
                {
                    /* Get device addresses from buffer */
                    for (iCount = 0; iCount < *pOutParams->pActualCount; iCount++)
                    {
                        fsciBleGapGetIdentityInformationFromBuffer(&pOutParams->pIdentityAddresses[iCount], &pBuffer);
                    }
                }

                /* Clean out parameters pointers kept in FSCI */
                fsciBleGapCleanOutParams();

                /* Signal out parameters ready */
                Ble_OutParamsReady();
            }
        }
        break;

        case gBleGapEvtLoadEncryptionInformationOpCode_c:
        {
            fsciBleGapLoadEncryptionInformationOutParams_t *pOutParams =
                (fsciBleGapLoadEncryptionInformationOutParams_t *)fsciBleGapRestoreOutParams();

            /* Get out parameters of the Gap_LoadEncryptionInformation function from buffer */

            if (NULL != pOutParams->pLtkSize)
            {
                fsciBleGetUint8ValueFromBuffer(*pOutParams->pLtkSize, pBuffer);
            }

            if (NULL != pOutParams->pLtk && NULL != pOutParams->pLtkSize)
            {
                fsciBleGetArrayFromBuffer(pOutParams->pLtk, pBuffer, *pOutParams->pLtkSize);
            }

            if ((NULL != pOutParams->pLtkSize) || (NULL != pOutParams->pLtk))
            {
                /* Clean out parameters pointers kept in FSCI */
                fsciBleGapCleanOutParams();

                /* Signal out parameters ready */
                Ble_OutParamsReady();
            }
        }
        break;

        case gBleGapEvtLoadCustomPeerInformationOpCode_c:
        {
            fsciBleGapLoadCustomPeerInformationOutParams_t *pOutParams =
                (fsciBleGapLoadCustomPeerInformationOutParams_t *)fsciBleGapRestoreOutParams();
            uint16_t infoSize;

            if (NULL != pOutParams->pInfo)
            {
                /* Get info size from buffer */
                fsciBleGetUint16ValueFromBuffer(infoSize, pBuffer);

                /* Get info value from buffer */
                fsciBleGetArrayFromBuffer(pOutParams->pInfo, pBuffer, infoSize);

                /* Clean out parameters pointers kept in FSCI */
                fsciBleGapCleanOutParams();

                /* Signal out parameters ready */
                Ble_OutParamsReady();
            }
        }
        break;

        case gBleGapEvtCheckIfBondedOpCode_c:
        {
            fsciBleGapCheckIfBondedOutParams_t *pOutParams =
                (fsciBleGapCheckIfBondedOutParams_t *)fsciBleGapRestoreOutParams();

            if (NULL != pOutParams->pIsBonded)
            {
                /* Get out parameter of the Gap_CheckIfBonded function from buffer */
                fsciBleGetBoolValueFromBuffer(*pOutParams->pIsBonded, pBuffer);

                /* Clean out parameters pointers kept in FSCI */
                fsciBleGapCleanOutParams();

                /* Signal out parameters ready */
                Ble_OutParamsReady();
            }
        }
        break;

        case gBleGapEvtGetBondedDevicesCountOpCode_c:
        {
            fsciBleGapGetBondedDevicesCountOutParams_t *pOutParams =
                (fsciBleGapGetBondedDevicesCountOutParams_t *)fsciBleGapRestoreOutParams();

            if (NULL != pOutParams->pBondedDevicesCount)
            {
                /* Get out parameter of the Gap_GetBondedDevicesCount function from buffer */
                fsciBleGetUint8ValueFromBuffer(*pOutParams->pBondedDevicesCount, pBuffer);

                /* Clean out parameters pointers kept in FSCI */
                fsciBleGapCleanOutParams();

                /* Signal out parameters ready */
                Ble_OutParamsReady();
            }
        }
        break;

        case gBleGapEvtGetBondedDeviceNameOpCode_c:
        {
            fsciBleGapGetBondedDeviceNameOutParams_t *pOutParams =
                (fsciBleGapGetBondedDeviceNameOutParams_t *)fsciBleGapRestoreOutParams();
            uint16_t nameSize;

            if (NULL != pOutParams->pName)
            {
                /* Get out parameters of the Gap_GetBondedDeviceName function from buffer */
                fsciBleGetUint16ValueFromBuffer(nameSize, pBuffer);
                fsciBleGetArrayFromBuffer(pOutParams->pName, pBuffer, nameSize);

                /* Clean out parameters pointers kept in FSCI */
                fsciBleGapCleanOutParams();

                /* Signal out parameters ready */
                Ble_OutParamsReady();
            }
        }
        break;

        case gBleGapEvtGenericEventInitializationCompleteOpCode_c:
        case gBleGapEvtGenericEventInternalErrorOpCode_c:
        case gBleGapEvtGenericEventAdvertisingSetupFailedOpCode_c:
        case gBleGapEvtGenericEventAdvertisingParametersSetupCompleteOpCode_c:
        case gBleGapEvtGenericEventAdvertisingDataSetupCompleteOpCode_c:
        case gBleGapEvtGenericEventWhiteListSizeReadOpCode_c:
        case gBleGapEvtGenericEventDeviceAddedToWhiteListOpCode_c:
        case gBleGapEvtGenericEventDeviceRemovedFromWhiteListOpCode_c:
        case gBleGapEvtGenericEventWhiteListClearedOpCode_c:
        case gBleGapEvtGenericEventRandomAddressReadyOpCode_c:
        case gBleGapEvtGenericEventCreateConnectionCanceledOpCode_c:
        case gBleGapEvtGenericEventPublicAddressReadOpCode_c:
        case gBleGapEvtGenericEventAdvTxPowerLevelReadOpCode_c:
        case gBleGapEvtGenericEventPrivateResolvableAddressVerifiedOpCode_c:
        case gBleGapEvtGenericEventRandomAddressSetOpCode_c:
        case gBleGapEvtGenericEventControllerResetCompleteOpCode_c:
        case gBleGapEvtGenericEventLeScPublicKeyRegeneratedOpCode_c:
        case gBleGapEvtGenericEventLeScLocalOobDataOpCode_c:
        case gBleGapEvtGenericEventControllerPrivacyStateChangedOpCode_c:
        {
            gapGenericEvent_t genericEvent;

            switch (pClientPacket->structured.header.opCode)
            {
                case gBleGapEvtGenericEventInitializationCompleteOpCode_c:
                {
                    genericEvent.eventType = gInitializationComplete_c;
                }
                break;

                case gBleGapEvtGenericEventInternalErrorOpCode_c:
                {
                    genericEvent.eventType = gInternalError_c;
                }
                break;

                case gBleGapEvtGenericEventAdvertisingSetupFailedOpCode_c:
                {
                    genericEvent.eventType = gAdvertisingSetupFailed_c;
                }
                break;

                case gBleGapEvtGenericEventAdvertisingParametersSetupCompleteOpCode_c:
                {
                    genericEvent.eventType = gAdvertisingParametersSetupComplete_c;
                }
                break;

                case gBleGapEvtGenericEventAdvertisingDataSetupCompleteOpCode_c:
                {
                    genericEvent.eventType = gAdvertisingDataSetupComplete_c;
                }
                break;

                case gBleGapEvtGenericEventWhiteListSizeReadOpCode_c:
                {
                    genericEvent.eventType = gWhiteListSizeRead_c;
                }
                break;

                case gBleGapEvtGenericEventDeviceAddedToWhiteListOpCode_c:
                {
                    genericEvent.eventType = gDeviceAddedToWhiteList_c;
                }
                break;

                case gBleGapEvtGenericEventDeviceRemovedFromWhiteListOpCode_c:
                {
                    genericEvent.eventType = gDeviceRemovedFromWhiteList_c;
                }
                break;

                case gBleGapEvtGenericEventWhiteListClearedOpCode_c:
                {
                    genericEvent.eventType = gWhiteListCleared_c;
                }
                break;

                case gBleGapEvtGenericEventRandomAddressReadyOpCode_c:
                {
                    genericEvent.eventType = gRandomAddressReady_c;
                }
                break;

                case gBleGapEvtGenericEventCreateConnectionCanceledOpCode_c:
                {
                    genericEvent.eventType = gCreateConnectionCanceled_c;
                }
                break;

                case gBleGapEvtGenericEventPublicAddressReadOpCode_c:
                {
                    genericEvent.eventType = gPublicAddressRead_c;
                }
                break;

                case gBleGapEvtGenericEventAdvTxPowerLevelReadOpCode_c:
                {
                    genericEvent.eventType = gAdvTxPowerLevelRead_c;
                }
                break;

                case gBleGapEvtGenericEventPrivateResolvableAddressVerifiedOpCode_c:
                {
                    genericEvent.eventType = gPrivateResolvableAddressVerified_c;
                }
                break;

                case gBleGapEvtGenericEventRandomAddressSetOpCode_c:
                {
                    genericEvent.eventType = gRandomAddressSet_c;
                }
                break;

                case gBleGapEvtGenericEventControllerResetCompleteOpCode_c:
                {
                    genericEvent.eventType = gControllerResetComplete_c;
                }
                break;

                case gBleGapEvtGenericEventLeScPublicKeyRegeneratedOpCode_c:
                {
                    genericEvent.eventType = gLeScPublicKeyRegenerated_c;
                }
                break;

                case gBleGapEvtGenericEventLeScLocalOobDataOpCode_c:
                {
                    genericEvent.eventType = gLeScLocalOobData_c;
                }
                break;

                case gBleGapEvtGenericEventControllerPrivacyStateChangedOpCode_c:
                {
                    genericEvent.eventType = gControllerPrivacyStateChanged_c;
                }
                break;

                default:
                {
                }
                break;
            }

            fsciBleGapGetGenericEventFromBuffer(&genericEvent, &pBuffer);

            fsciBleGapCallbacks.genericCallback(&genericEvent);
        }
        break;

        case gBleGapEvtAdvertisingEventAdvertisingStateChangedOpCode_c:
        case gBleGapEvtAdvertisingEventAdvertisingCommandFailedOpCode_c:
        {
            gapAdvertisingEvent_t advertisingEvent;

            switch (pClientPacket->structured.header.opCode)
            {
                case gBleGapEvtAdvertisingEventAdvertisingStateChangedOpCode_c:
                {
                    advertisingEvent.eventType = gAdvertisingStateChanged_c;
                }
                break;

                case gBleGapEvtAdvertisingEventAdvertisingCommandFailedOpCode_c:
                {
                    advertisingEvent.eventType = gAdvertisingCommandFailed_c;
                }
                break;

                default:
                {
                }
                break;
            }

            fsciBleGapGetAdvertisingEventFromBuffer(&advertisingEvent, &pBuffer);

            fsciBleGapCallbacks.advertisingCallback(&advertisingEvent);
        }
        break;

        case gBleGapEvtScanningEventScanStateChangedOpCode_c:
        case gBleGapEvtScanningEventScanCommandFailedOpCode_c:
        case gBleGapEvtScanningEventDeviceScannedOpCode_c:
        {
            gapScanningEvent_t *pScanningEvent;
            gapScanningEventType_t eventType = gScanStateChanged_c;

            switch (pClientPacket->structured.header.opCode)
            {
                case gBleGapEvtScanningEventScanStateChangedOpCode_c:
                {
                    eventType = gScanStateChanged_c;
                }
                break;

                case gBleGapEvtScanningEventScanCommandFailedOpCode_c:
                {
                    eventType = gScanCommandFailed_c;
                }
                break;

                case gBleGapEvtScanningEventDeviceScannedOpCode_c:
                {
                    eventType = gDeviceScanned_c;
                }
                break;

                default:
                {
                }
                break;
            }

            pScanningEvent = fsciBleGapAllocScanningEventForBuffer(eventType, pBuffer);

            if (NULL == pScanningEvent)
            {
                /* No memory */
                FSCI_Error(gFsciOutOfMessages_c, fsciBleInterfaceId);
            }
            else
            {
                fsciBleGapGetScanningEventFromBuffer(pScanningEvent, &pBuffer);

                fsciBleGapCallbacks.scanningCallback(pScanningEvent);

                fsciBleGapFreeScanningEvent(pScanningEvent);
            }
        }
        break;

        case gBleGapEvtConnectionEventConnectedOpCode_c:
        case gBleGapEvtConnectionEventPairingRequestOpCode_c:
        case gBleGapEvtConnectionEventSlaveSecurityRequestOpCode_c:
        case gBleGapEvtConnectionEventPairingResponseOpCode_c:
        case gBleGapEvtConnectionEventAuthenticationRejectedOpCode_c:
        case gBleGapEvtConnectionEventPasskeyRequestOpCode_c:
        case gBleGapEvtConnectionEventOobRequestOpCode_c:
        case gBleGapEvtConnectionEventPasskeyDisplayOpCode_c:
        case gBleGapEvtConnectionEventKeyExchangeRequestOpCode_c:
        case gBleGapEvtConnectionEventKeysReceivedOpCode_c:
        case gBleGapEvtConnectionEventLongTermKeyRequestOpCode_c:
        case gBleGapEvtConnectionEventEncryptionChangedOpCode_c:
        case gBleGapEvtConnectionEventPairingCompleteOpCode_c:
        case gBleGapEvtConnectionEventDisconnectedOpCode_c:
        case gBleGapEvtConnectionEventRssiReadOpCode_c:
        case gBleGapEvtConnectionEventTxPowerLevelReadOpCode_c:
        case gBleGapEvtConnectionEventPowerReadFailureOpCode_c:
        case gBleGapEvtConnectionEventParameterUpdateRequestOpCode_c:
        case gBleGapEvtConnectionEventParameterUpdateCompleteOpCode_c:
        case gBleGapEvtConnectionEventLeDataLengthChangedOpCode_c:
        case gBleGapEvtConnectionEventLeScOobDataRequestOpCode_c:
        case gBleGapEvtConnectionEventLeScDisplayNumericValueOpCode_c:
        case gBleGapEvtConnectionEventLeScKeypressNotificationOpCode_c:
        {
            gapConnectionEvent_t *gConnectionEvent;
            gapConnectionEventType_t eventType = gConnEvtConnected_c;
            deviceId_t deviceId;

            fsciBleGetDeviceIdFromBuffer(&deviceId, &pBuffer);

            switch (pClientPacket->structured.header.opCode)
            {
                case gBleGapEvtConnectionEventConnectedOpCode_c:
                {
                    eventType = gConnEvtConnected_c;
                }
                break;

                case gBleGapEvtConnectionEventPairingRequestOpCode_c:
                {
                    eventType = gConnEvtPairingRequest_c;
                }
                break;

                case gBleGapEvtConnectionEventSlaveSecurityRequestOpCode_c:
                {
                    eventType = gConnEvtSlaveSecurityRequest_c;
                }
                break;

                case gBleGapEvtConnectionEventPairingResponseOpCode_c:
                {
                    eventType = gConnEvtPairingResponse_c;
                }
                break;

                case gBleGapEvtConnectionEventAuthenticationRejectedOpCode_c:
                {
                    eventType = gConnEvtAuthenticationRejected_c;
                }
                break;

                case gBleGapEvtConnectionEventPasskeyRequestOpCode_c:
                {
                    eventType = gConnEvtPasskeyRequest_c;
                }
                break;

                case gBleGapEvtConnectionEventOobRequestOpCode_c:
                {
                    eventType = gConnEvtOobRequest_c;
                }
                break;

                case gBleGapEvtConnectionEventPasskeyDisplayOpCode_c:
                {
                    eventType = gConnEvtPasskeyDisplay_c;
                }
                break;

                case gBleGapEvtConnectionEventKeyExchangeRequestOpCode_c:
                {
                    eventType = gConnEvtKeyExchangeRequest_c;
                }
                break;

                case gBleGapEvtConnectionEventKeysReceivedOpCode_c:
                {
                    eventType = gConnEvtKeysReceived_c;
                }
                break;

                case gBleGapEvtConnectionEventLongTermKeyRequestOpCode_c:
                {
                    eventType = gConnEvtLongTermKeyRequest_c;
                }
                break;

                case gBleGapEvtConnectionEventEncryptionChangedOpCode_c:
                {
                    eventType = gConnEvtEncryptionChanged_c;
                }
                break;

                case gBleGapEvtConnectionEventPairingCompleteOpCode_c:
                {
                    eventType = gConnEvtPairingComplete_c;
                }
                break;

                case gBleGapEvtConnectionEventDisconnectedOpCode_c:
                {
                    eventType = gConnEvtDisconnected_c;
                }
                break;

                case gBleGapEvtConnectionEventRssiReadOpCode_c:
                {
                    eventType = gConnEvtRssiRead_c;
                }
                break;

                case gBleGapEvtConnectionEventTxPowerLevelReadOpCode_c:
                {
                    eventType = gConnEvtTxPowerLevelRead_c;
                }
                break;

                case gBleGapEvtConnectionEventPowerReadFailureOpCode_c:
                {
                    eventType = gConnEvtPowerReadFailure_c;
                }
                break;

                case gBleGapEvtConnectionEventParameterUpdateRequestOpCode_c:
                {
                    eventType = gConnEvtParameterUpdateRequest_c;
                }
                break;

                case gBleGapEvtConnectionEventParameterUpdateCompleteOpCode_c:
                {
                    eventType = gConnEvtParameterUpdateComplete_c;
                }
                break;

                case gBleGapEvtConnectionEventLeDataLengthChangedOpCode_c:
                {
                    eventType = gConnEvtLeDataLengthChanged_c;
                }
                break;

                case gBleGapEvtConnectionEventLeScOobDataRequestOpCode_c:
                {
                    eventType = gConnEvtLeScOobDataRequest_c;
                }
                break;

                case gBleGapEvtConnectionEventLeScDisplayNumericValueOpCode_c:
                {
                    eventType = gConnEvtLeScDisplayNumericValue_c;
                }
                break;

                case gBleGapEvtConnectionEventLeScKeypressNotificationOpCode_c:
                {
                    eventType = gConnEvtLeScKeypressNotification_c;
                }
                break;

                default:
                {
                }
                break;
            }

            gConnectionEvent = fsciBleGapAllocConnectionEventForBuffer(eventType, pBuffer);

            if (NULL == gConnectionEvent)
            {
                /* No memory */
                FSCI_Error(gFsciOutOfMessages_c, fsciBleInterfaceId);
            }
            else
            {
                fsciBleGapGetConnectionEventFromBuffer(gConnectionEvent, &pBuffer);

                fsciBleGapCallbacks.connectionCallback(deviceId, gConnectionEvent);

                fsciBleGapFreeConnectionEvent(gConnectionEvent);
            }
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

void fsciBleGapNoParamCmdMonitor(fsciBleGapOpCode_t opCode)
{
    /* Call the generic FSCI BLE monitor for commands or events that have no parameters */
    fsciBleNoParamCmdOrEvtMonitor(gFsciBleGapOpcodeGroup_c, opCode);
}

void fsciBleGapUint8ParamCmdMonitor(fsciBleGapOpCode_t opCode, uint8_t param)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(opCode, sizeof(uint8_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromUint8Value(param, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapDeviceIdParamCmdMonitor(fsciBleGapOpCode_t opCode, deviceId_t deviceId)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(opCode, fsciBleGetDeviceIdBufferSize(&deviceId));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapCheckNotificationsAndIndicationsCmdMonitor(fsciBleGapOpCode_t opCode,
                                                          deviceId_t deviceId,
                                                          uint16_t handle,
                                                          bool_t *pOutIsActive)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(opCode, fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(uint16_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromUint16Value(handle, pBuffer);

    /* Save out parameters pointers */
    fsciBleGapSaveOutParams(pOutIsActive, NULL);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapAddressParamsCmdMonitor(fsciBleGapOpCode_t opCode,
                                       bleAddressType_t addressType,
                                       bleDeviceAddress_t address)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(opCode, sizeof(bleAddressType_t) + gcBleDeviceAddressSize_c);

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromEnumValue(addressType, pBuffer, bleAddressType_t);
    fsciBleGetBufferFromAddress(address, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapHostInitializeCmdMonitor(gapGenericCallback_t genericCallback,
                                        hciHostToControllerInterface_t hostToControllerInterface)
{
    /* Save generic callback */
    fsciBleGapCallbacks.genericCallback = genericCallback;

    /* Send FSCI CPU reset command to BlackBox */
    fsciBleNoParamCmdOrEvtMonitor(gFSCI_ReqOpcodeGroup_c, mFsciMsgResetCPUReq_c);
}

void fsciBleGapRegisterDeviceSecurityRequirementsCmdMonitor(gapDeviceSecurityRequirements_t *pSecurity)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;
    bool_t bDeviceSecurityRequirementsIncluded = ((NULL != pSecurity) ? TRUE : FALSE);

    /* Allocate the packet to be sent over UART */
    pClientPacket =
        fsciBleGapAllocFsciPacket(gBleGapCmdRegisterDeviceSecurityRequirementsOpCode_c,
                                  sizeof(bool_t) + ((TRUE == bDeviceSecurityRequirementsIncluded) ?
                                                        fsciBleGapGetDeviceSecurityRequirementsBufferSize(pSecurity) :
                                                        0));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromBoolValue(bDeviceSecurityRequirementsIncluded, pBuffer);
    if (TRUE == bDeviceSecurityRequirementsIncluded)
    {
        fsciBleGapGetBufferFromDeviceSecurityRequirements(pSecurity, &pBuffer);
    }

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapSetAdvertisingParametersCmdMonitor(gapAdvertisingParameters_t *pAdvertisingParameters)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(gBleGapCmdSetAdvertisingParametersOpCode_c,
                                              fsciBleGapGetAdvertisingParametersBufferSize(pAdvertisingParameters));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGapGetBufferFromAdvertisingParameters(pAdvertisingParameters, &pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapSetAdvertisingDataCmdMonitor(gapAdvertisingData_t *pAdvertisingData,
                                            gapScanResponseData_t *pScanResponseData)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;
    bool_t bAdvertisingDataIncluded  = FALSE;
    bool_t bScanResponseDataIncluded = FALSE;

    if (NULL != pAdvertisingData)
    {
        if (pAdvertisingData->cNumAdStructures)
        {
            bAdvertisingDataIncluded = TRUE;
        }
    }

    if (NULL != pScanResponseData)
    {
        if (pScanResponseData->cNumAdStructures)
        {
            bScanResponseDataIncluded = TRUE;
        }
    }

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(
        gBleGapCmdSetAdvertisingDataOpCode_c,
        sizeof(bool_t) +
            ((TRUE == bAdvertisingDataIncluded) ? fsciBleGapGetAdvertisingDataBufferSize(pAdvertisingData) : 0) +
            sizeof(bool_t) +
            ((TRUE == bScanResponseDataIncluded) ? fsciBleGapGetScanResponseDataBufferSize(pScanResponseData) : 0));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromBoolValue(bAdvertisingDataIncluded, pBuffer);
    if (TRUE == bAdvertisingDataIncluded)
    {
        fsciBleGapGetBufferFromAdvertisingData(pAdvertisingData, &pBuffer);
    }

    fsciBleGetBufferFromBoolValue(bScanResponseDataIncluded, pBuffer);
    if (TRUE == bScanResponseDataIncluded)
    {
        fsciBleGapGetBufferFromScanResponseData(pScanResponseData, &pBuffer);
    }

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapStartAdvertisingCmdMonitor(gapAdvertisingCallback_t advertisingCallback,
                                          gapConnectionCallback_t connectionCallback)
{
    fsciBleGapCallbacks.advertisingCallback = advertisingCallback;
    fsciBleGapCallbacks.connectionCallback  = connectionCallback;

    fsciBleGapNoParamCmdMonitor(gBleGapCmdStartAdvertisingOpCode_c);
}

void fsciBleGapAuthorizeCmdMonitor(deviceId_t deviceId, uint16_t handle, gattDbAccessType_t access)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket =
        fsciBleGapAllocFsciPacket(gBleGapCmdAuthorizeOpCode_c, fsciBleGetDeviceIdBufferSize(&deviceId) +
                                                                   sizeof(uint16_t) + sizeof(gattDbAccessType_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromUint16Value(handle, pBuffer);
    fsciBleGetBufferFromEnumValue(access, pBuffer, gattDbAccessType_t);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapSaveCccdCmdMonitor(deviceId_t deviceId, uint16_t handle, gattCccdFlags_t cccd)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket =
        fsciBleGapAllocFsciPacket(gBleGapCmdSaveCccdOpCode_c,
                                  fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(uint16_t) + sizeof(gattCccdFlags_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromUint16Value(handle, pBuffer);
    fsciBleGetBufferFromEnumValue(cccd, pBuffer, gattCccdFlags_t);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapGetBondedStaticAddressesCmdMonitor(bleDeviceAddress_t *aOutDeviceAddresses,
                                                  uint8_t maxDevices,
                                                  uint8_t *pOutActualCount)
{
    /* Save out parameters pointers */
    fsciBleGapSaveOutParams(aOutDeviceAddresses, pOutActualCount);

    fsciBleGapUint8ParamCmdMonitor(gBleGapCmdGetBondedStaticAddressesOpCode_c, maxDevices);
}

void fsciBleGapGetBondedDevicesIdentityInformationCmdMonitor(gapIdentityInformation_t *aOutIdentityAddresses,
                                                             uint8_t maxDevices,
                                                             uint8_t *pOutActualCount)
{
    /* Save out parameters pointers */
    fsciBleGapSaveOutParams(aOutIdentityAddresses, pOutActualCount);

    fsciBleGapUint8ParamCmdMonitor(gBleGapCmdGetBondedDevicesIdentityInformationOpCode_c, maxDevices);
}

void fsciBleGapPairCmdMonitor(deviceId_t deviceId, gapPairingParameters_t *pPairingParameters)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(
        gBleGapCmdPairOpCode_c,
        fsciBleGetDeviceIdBufferSize(&deviceId) + fsciBleGapGetPairingParametersBufferSize(pPairingParameters));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGapGetBufferFromPairingParameters(pPairingParameters, &pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapSendSlaveSecurityRequestCmdMonitor(deviceId_t deviceId,
                                                  bool_t bondAfterPairing,
                                                  gapSecurityModeAndLevel_t securityModeLevel)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(
        gBleGapCmdSendSlaveSecurityRequestOpCode_c,
        fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(bool_t) + sizeof(gapSecurityModeAndLevel_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromBoolValue(bondAfterPairing, pBuffer);
    fsciBleGetBufferFromEnumValue(securityModeLevel, pBuffer, gapSecurityModeAndLevel_t);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapAcceptPairingRequestCmdMonitor(deviceId_t deviceId, gapPairingParameters_t *pPairingParameters)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(
        gBleGapCmdAcceptPairingRequestOpCode_c,
        fsciBleGetDeviceIdBufferSize(&deviceId) + fsciBleGapGetPairingParametersBufferSize(pPairingParameters));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGapGetBufferFromPairingParameters(pPairingParameters, &pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapRejectPairingCmdMonitor(deviceId_t deviceId, gapAuthenticationRejectReason_t reason)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket =
        fsciBleGapAllocFsciPacket(gBleGapCmdRejectPairingOpCode_c,
                                  fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(gapAuthenticationRejectReason_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromEnumValue(reason, pBuffer, gapAuthenticationRejectReason_t);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapEnterPasskeyCmdMonitor(deviceId_t deviceId, uint32_t passkey)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(gBleGapCmdEnterPasskeyOpCode_c,
                                              fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(uint32_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromUint32Value(passkey, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapProvideOobCmdMonitor(deviceId_t deviceId, uint8_t *aOob)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(gBleGapCmdProvideOobOpCode_c,
                                              fsciBleGetDeviceIdBufferSize(&deviceId) + gcSmpOobSize_c);

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromOob(aOob, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapSendSmpKeysCmdMonitor(deviceId_t deviceId, gapSmpKeys_t *pKeys)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(
        gBleGapCmdSendSmpKeysOpCode_c, fsciBleGetDeviceIdBufferSize(&deviceId) + fsciBleGapGetSmpKeysBufferSize(pKeys));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGapGetBufferFromSmpKeys(pKeys, &pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapProvideLongTermKeyCmdMonitor(deviceId_t deviceId, uint8_t *aLtk, uint8_t ltkSize)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(gBleGapCmdProvideLongTermKeyOpCode_c,
                                              fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(uint8_t) + ltkSize);

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromUint8Value(ltkSize, pBuffer);
    fsciBleGetBufferFromArray(aLtk, pBuffer, ltkSize);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapLoadEncryptionInformationCmdMonitor(deviceId_t deviceId, uint8_t *aOutLtk, uint8_t *pOutLtkSize)
{
    /* Save out parameters pointers */
    fsciBleGapSaveOutParams(aOutLtk, pOutLtkSize);
    /* Monitor deviceId parameter */
    fsciBleGapDeviceIdParamCmdMonitor(gBleGapCmdLoadEncryptionInformationOpCode_c, deviceId);
}

void fsciBleGapSetLocalPasskeyCmdMonitor(uint32_t passkey)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(gBleGapCmdSetLocalPasskeyOpCode_c, sizeof(uint32_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromUint32Value(passkey, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapStartScanningCmdMonitor(gapScanningParameters_t *pScanningParameters,
                                       gapScanningCallback_t scanningCallback)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;
    bool_t bScanningParametersIncluded = ((NULL != pScanningParameters) ? TRUE : FALSE);

    fsciBleGapCallbacks.scanningCallback = scanningCallback;

    /* Allocate the packet to be sent over UART */
    pClientPacket =
        fsciBleGapAllocFsciPacket(gBleGapCmdStartScanningOpCode_c,
                                  sizeof(bool_t) + ((TRUE == bScanningParametersIncluded) ?
                                                        fsciBleGapGetScanningParametersBufferSize(pScanningParameters) :
                                                        0));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromBoolValue(bScanningParametersIncluded, pBuffer);
    if (TRUE == bScanningParametersIncluded)
    {
        fsciBleGapGetBufferFromScanningParameters(pScanningParameters, &pBuffer);
    }

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapConnectCmdMonitor(gapConnectionRequestParameters_t *pParameters, gapConnectionCallback_t connCallback)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    fsciBleGapCallbacks.connectionCallback = connCallback;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(gBleGapCmdConnectOpCode_c,
                                              fsciBleGapGetConnectionRequestParametersBufferSize(pParameters));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGapGetBufferFromConnectionRequestParameters(pParameters, &pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapSaveCustomPeerInformationCmdMonitor(deviceId_t deviceId, void *aInfo, uint16_t offset, uint16_t infoSize)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(
        gBleGapCmdSaveCustomPeerInformationOpCode_c,
        fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(uint16_t) + sizeof(uint16_t) + infoSize);

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromUint16Value(offset, pBuffer);
    fsciBleGetBufferFromUint16Value(infoSize, pBuffer);
    fsciBleGetBufferFromArray(aInfo, pBuffer, infoSize);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapLoadCustomPeerInformationCmdMonitor(deviceId_t deviceId,
                                                   void *aOutInfo,
                                                   uint16_t offset,
                                                   uint16_t infoSize)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket =
        fsciBleGapAllocFsciPacket(gBleGapCmdLoadCustomPeerInformationOpCode_c,
                                  fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(uint16_t) + sizeof(uint16_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromUint16Value(offset, pBuffer);
    fsciBleGetBufferFromUint16Value(infoSize, pBuffer);

    /* Save out parameters pointers */
    fsciBleGapSaveOutParams(aOutInfo, NULL);
    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapCheckIfBondedCmdMonitor(deviceId_t deviceId, bool_t *pOutIsBonded)
{
    /* Save out parameters pointers */
    fsciBleGapSaveOutParams(pOutIsBonded, NULL);
    /* Monitor deviceId parameter */
    fsciBleGapDeviceIdParamCmdMonitor(gBleGapCmdCheckIfBondedOpCode_c, deviceId);
}

void fsciBleGapCreateRandomDeviceAddressCmdMonitor(uint8_t *aIrk, uint8_t *aRandomPart)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;
    bool_t bIrkIncluded        = ((NULL != aIrk) ? TRUE : FALSE);
    bool_t bRandomPartIncluded = ((NULL != aRandomPart) ? TRUE : FALSE);

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(
        gBleGapCmdCreateRandomDeviceAddressOpCode_c,
        sizeof(bool_t) +
            ((TRUE == bIrkIncluded) ? (gcSmpIrkSize_c + sizeof(bool_t) + ((TRUE == bRandomPartIncluded) ? 3 : 0)) : 0));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromBoolValue(bIrkIncluded, pBuffer);
    if (TRUE == bIrkIncluded)
    {
        fsciBleGetBufferFromIrk(aIrk, pBuffer);
    }

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapSaveDeviceNameCmdMonitor(deviceId_t deviceId, uchar_t *aName, uint8_t cNameSize)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(gBleGapCmdSaveDeviceNameOpCode_c,
                                              fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(uint8_t) + cNameSize);

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromUint8Value(cNameSize, pBuffer);
    fsciBleGetBufferFromArray(aName, pBuffer, cNameSize);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapGetBondedDevicesCountCmdMonitor(uint8_t *pOutBondedDevicesCount)
{
    /* Save out parameters pointers */
    fsciBleGapSaveOutParams(pOutBondedDevicesCount, NULL);

    fsciBleGapNoParamCmdMonitor(gBleGapCmdGetBondedDevicesCountOpCode_c);
}

void fsciBleGapGetBondedDeviceNameCmdMonitor(uint8_t nvmIndex, uchar_t *aOutName, uint8_t maxNameSize)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(gBleGapCmdGetBondedDeviceNameOpCode_c, sizeof(uint8_t) + sizeof(uint8_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromUint8Value(nvmIndex, pBuffer);
    fsciBleGetBufferFromUint8Value(maxNameSize, pBuffer);

    /* Save out parameters pointers */
    fsciBleGapSaveOutParams(aOutName, NULL);
    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapReadRadioPowerLevelCmdMonitor(gapRadioPowerLevelReadType_t txReadType, deviceId_t deviceId)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket =
        fsciBleGapAllocFsciPacket(gBleGapCmdReadRadioPowerLevelOpCode_c,
                                  sizeof(gapRadioPowerLevelReadType_t) + fsciBleGetDeviceIdBufferSize(&deviceId));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromEnumValue(txReadType, pBuffer, gapRadioPowerLevelReadType_t);
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapVerifyPrivateResolvableAddressCmdMonitor(uint8_t nvmIndex, bleDeviceAddress_t aAddress)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(gBleGapCmdVerifyPrivateResolvableAddressOpCode_c,
                                              sizeof(uint8_t) + gcBleDeviceAddressSize_c);

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromUint8Value(nvmIndex, pBuffer);
    fsciBleGetBufferFromAddress(aAddress, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapSetRandomAddressCmdMonitor(bleDeviceAddress_t aAddress)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(gBleGapCmdSetRandomAddressOpCode_c, gcBleDeviceAddressSize_c);

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromAddress(aAddress, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapSetScanModeCmdMonitor(gapScanMode_t scanMode, gapAutoConnectParams_t *pAutoConnectParams)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(
        gBleGapCmdSetScanModeOpCode_c,
        sizeof(gapScanMode_t) +
            ((gAutoConnect_c == scanMode) ? fsciBleGapGetAutoConnectParamsBufferSize(pAutoConnectParams) : 0));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromEnumValue(scanMode, pBuffer, gapScanMode_t);
    fsciBleGapGetBufferFromAutoConnectParams(pAutoConnectParams, &pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapSetDefaultPairingParametersCmdMonitor(gapPairingParameters_t *pPairingParameters)
{
    bool_t bPairingParametersIncluded = (NULL == pPairingParameters) ? FALSE : TRUE;
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(
        gBleGapCmdSetDefaultPairingParameters_c,
        sizeof(bool_t) +
            ((TRUE == bPairingParametersIncluded) ? fsciBleGapGetPairingParametersBufferSize(pPairingParameters) : 0));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromBoolValue(bPairingParametersIncluded, pBuffer);

    if (TRUE == bPairingParametersIncluded)
    {
        fsciBleGapGetBufferFromPairingParameters(pPairingParameters, &pBuffer);
    }

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapUpdateConnectionParametersCmdMonitor(deviceId_t deviceId,
                                                    uint16_t intervalMin,
                                                    uint16_t intervalMax,
                                                    uint16_t slaveLatency,
                                                    uint16_t timeoutMultiplier,
                                                    uint16_t minCeLength,
                                                    uint16_t maxCeLength)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket =
        fsciBleGapAllocFsciPacket(gBleGapCmdUpdateConnectionParametersOpCode_c,
                                  fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(uint16_t) + sizeof(uint16_t) +
                                      sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromUint16Value(intervalMin, pBuffer);
    fsciBleGetBufferFromUint16Value(intervalMax, pBuffer);
    fsciBleGetBufferFromUint16Value(slaveLatency, pBuffer);
    fsciBleGetBufferFromUint16Value(timeoutMultiplier, pBuffer);
    fsciBleGetBufferFromUint16Value(minCeLength, pBuffer);
    fsciBleGetBufferFromUint16Value(maxCeLength, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapEnableUpdateConnectionParametersCmdMonitor(deviceId_t deviceId, bool_t enable)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(gBleGapCmdEnableUpdateConnectionParametersOpCode_c,
                                              fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(bool_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromBoolValue(enable, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapUpdateLeDataLengthCmdMonitor(deviceId_t deviceId, uint16_t txOctets, uint16_t txTime)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(gBleGapCmdUpdateLeDataLengthOpCode_c,
                                              2 * sizeof(uint16_t) + fsciBleGetDeviceIdBufferSize(&deviceId));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromUint16Value(txOctets, pBuffer);
    fsciBleGetBufferFromUint16Value(txTime, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapEnableHostPrivacyCmdMonitor(bool_t enable, uint8_t *aIrk)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(gBleGapCmdEnableHostPrivacyOpCode_c,
                                              sizeof(bool_t) + ((TRUE == enable) ? gcSmpIrkSize_c : 0));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromBoolValue(enable, pBuffer);
    if (TRUE == enable)
    {
        fsciBleGetBufferFromIrk(aIrk, pBuffer);
    }

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapEnableControllerPrivacyCmdMonitor(bool_t enable,
                                                 uint8_t *aOwnIrk,
                                                 uint8_t peerIdCount,
                                                 gapIdentityInformation_t *aPeerIdentities)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;
    uint32_t i   = 0;
    uint8_t size = sizeof(bool_t);

    if (TRUE == enable)
    {
        size += gcSmpIrkSize_c + peerIdCount * (sizeof(bleAddressType_t) + sizeof(bleDeviceAddress_t) + gcSmpIrkSize_c);
    }

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(gBleGapCmdEnableControllerPrivacyOpCode_c, size);
    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromBoolValue(enable, pBuffer);
    if (TRUE == enable)
    {
        fsciBleGetBufferFromIrk(aOwnIrk, pBuffer);
        fsciBleGetBufferFromUint8Value(peerIdCount, pBuffer);

        for (i = 0; i < peerIdCount; i++)
        {
            fsciBleGapGetBufferFromIdentityInformation(&aPeerIdentities[i], &pBuffer);
        }
    }

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapLeScValidateNumericValueCmdMonitor(deviceId_t deviceId, bool_t valid)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(gBleGapCmdLeScValidateNumericValueOpCode_c,
                                              sizeof(bool_t) + fsciBleGetDeviceIdBufferSize(&deviceId));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromBoolValue(valid, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapLeScSetPeerOobDataCmdMonitor(deviceId_t deviceId, gapLeScOobData_t *pPeerOobData)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGapAllocFsciPacket(
        gBleGapCmdLeScSetPeerOobDataOpCode_c,
        gSmpLeScRandomValueSize_c + gSmpLeScRandomConfirmValueSize_c + fsciBleGetDeviceIdBufferSize(&deviceId));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromArray(pPeerOobData->randomValue, pBuffer, gSmpLeScRandomValueSize_c);
    fsciBleGetBufferFromArray(pPeerOobData->confirmValue, pBuffer, gSmpLeScRandomConfirmValueSize_c);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGapLeScSendKeypressNotificationCmdMonitor(deviceId_t deviceId,
                                                      gapKeypressNotification_t keypressNotification)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket =
        fsciBleGapAllocFsciPacket(gBleGapCmdLeScSendKeypressNotificationPrivacyOpCode_c,
                                  sizeof(gapKeypressNotification_t) + fsciBleGetDeviceIdBufferSize(&deviceId));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromEnumValue(keypressNotification, pBuffer, gapKeypressNotification_t);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}
/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

static void fsciBleGapGenericCallback(gapGenericEvent_t *pGenericEvent)
{
}

static bleResult_t fsciBleHciHostToControllerInterface(hciPacketType_t packetType, void *pPacket, uint16_t packetSize)
{
    return gBleSuccess_c;
}

static void fsciBleGapAdvertisingCallback(gapAdvertisingEvent_t *pAdvertisingEvent)
{
}

static void fsciBleGapScanningCallback(gapScanningEvent_t *pScanningEvent)
{
}

static void fsciBleGapConnectionCallback(deviceId_t deviceId, gapConnectionEvent_t *pConnectionEvent)
{
}

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
