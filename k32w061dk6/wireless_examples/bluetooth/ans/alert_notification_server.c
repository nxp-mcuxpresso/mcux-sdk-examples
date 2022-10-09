/*! *********************************************************************************
* \addtogroup Alert Notification Server
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2020 NXP
* All rights reserved.
*
* \file
*
* This file is the source file for the Alert Notification Server application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "EmbeddedTypes.h"
/* Framework / Drivers */
#include "RNG_Interface.h"
#include "Keyboard.h"
#include "LED.h"
#include "TimersManager.h"
#include "FunctionLib.h"
#include "Panic.h"

/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE)
    #include "dynamic_gatt_database.h"
#else
    #include "gatt_db_handles.h"
#endif

/* Profile / Services */
#include "battery_interface.h"
#include "device_info_interface.h"
#include "current_time_interface.h"
#include "reference_time_update_interface.h"
#include "next_dst_change_interface.h"
#include "alert_notification_interface.h"

#include "ble_conn_manager.h"

#include "board.h"
#include "ApplMain.h"
#include "alert_notification_server.h"

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE)
    #include "erpc_host.h"
#endif

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define mBatteryLevelReportInterval_c   (10)        /* battery level report interval in seconds  */

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
typedef struct appPeerInfo_tag
{
    deviceId_t      deviceId;
    bool_t          isBonded;
} appPeerInfo_t;

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
static appPeerInfo_t mPeerInformation;
static bool_t        mScanningOn = FALSE;

/* Service Data*/
static bool_t           basValidClientList[gAppMaxConnections_c] = { FALSE };
static basConfig_t      basServiceConfig  = {(uint8_t)service_battery, 0, basValidClientList, gAppMaxConnections_c};
static ctsConfig_t      ctsServiceConfig  = {(uint16_t)service_current_time, gCts_InitTime, gCts_InitTime, 0U, gCts_InitLocalTimeInfo, gCts_InitReferenceTimeInfo, FALSE};
static rtusConfig_t     rtusServiceConfig = {(uint16_t)service_reference_time, {gRtusIdle_c, gRtusSuccessful_c}};
static ndcsConfig_t     ndcsServiceConfig = {(uint16_t)service_next_dst, {{2016, 1, 1, 0, 0, 0}, 0U}};
static ansConfig_t      ansServiceConfig  = {(uint16_t)service_alert};
static uint16_t         cpHandles[] = { (uint16_t)value_alert_cp, (uint16_t)value_current_time, (uint16_t)value_time_update_cp };

/* Application specific data*/
static tmrTimerID_t mBatteryMeasurementTimerId;
static tmrTimerID_t mAppTimerId;
static tmrTimerID_t mCTSTickTimerId;
static tmrTimerID_t mRTUSReferenceUpdateTimerId;

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/
/* Host Stack callbacks */
static void BleApp_ScanningCallback
(
    gapScanningEvent_t *pScanningEvent
);

static void BleApp_ConnectionCallback
(
    deviceId_t peerDeviceId,
    gapConnectionEvent_t *pConnectionEvent
);

static void ScanningTimeoutTimerCallback(void *pParam);

static void BleApp_Config(void);
static void BleApp_SendUnreadAlertStatus(void);
static void BleApp_SendNewAlert(void);

static void BleApp_GattServerCallback(deviceId_t deviceId, gattServerEvent_t *pServerEvent);
static bool_t CheckScanEvent(gapScannedDevice_t *pData);
static void BatteryMeasurementTimerCallback(void *pParam);
static void CTSTickTimerCallback(void *pParam);
static void RTUSReferenceUpdateTimerCallback(void *pParam);

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief    Initializes application specific functionality before the BLE stack init.
*
********************************************************************************** */
void BleApp_Init(void)
{
    /* Initialize application support for drivers */
    BOARD_InitAdc();

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE)
    /* Init eRPC host */
    init_erpc_host();
#endif
}

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(void)
{
    if (!mScanningOn)
    {
        LED_StopFlashingAllLeds();
        Led1Flashing();
        (void)App_StartScanning(&gScanParams, BleApp_ScanningCallback, gGapDuplicateFilteringEnable_c, gGapScanContinuously_d, gGapScanPeriodicDisabled_d);
    }
}

/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    events    Key event structure.
********************************************************************************** */
void BleApp_HandleKeys(key_event_t events)
{
    switch (events)
    {
        case gKBD_EventPressPB1_c:
        {
            BleApp_Start();
            break;
        }

        case gKBD_EventLongPB1_c:
        {
            if (mPeerInformation.deviceId != gInvalidDeviceId_c)
            {
                (void)Gap_Disconnect(mPeerInformation.deviceId);
            }

            break;
        }

        case gKBD_EventPressPB2_c:
        {
            ctsServiceConfig.localTime += 3600U;
            ctsServiceConfig.adjustReason = gCts_ManualUpdate;
            (void)Cts_RecordCurrentTime(&ctsServiceConfig);
            BleApp_SendNewAlert();
            break;
        }

        case gKBD_EventLongPB2_c:
        {
            BleApp_SendUnreadAlertStatus();
            break;
        }

        default:
            ; /* For MISRA Compliance */
            break;
    }
}

/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
void BleApp_GenericCallback(gapGenericEvent_t *pGenericEvent)
{
    /* Call BLE Conn Manager */
    BleConnManager_GenericEvent(pGenericEvent);

    switch (pGenericEvent->eventType)
    {
        case gInitializationComplete_c:
        {
            BleApp_Config();
        }
        break;

        default:
            ; /* For MISRA Compliance */
            break;
    }
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief        Configures BLE Stack after initialization. Usually used for
*               configuring advertising, scanning, white list, services, et al.
*
********************************************************************************** */
static void BleApp_Config(void)
{
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1U)

    if (GattDbDynamic_CreateDatabase() != gBleSuccess_c)
    {
        panic(0, 0, 0, 0);
        return;
    }

#endif /* MULTICORE_APPLICATION_CORE */

    /* Configure as GAP Central */
    BleConnManager_GapCommonConfig();

    /* Register for callbacks*/
    (void)App_RegisterGattServerCallback(BleApp_GattServerCallback);
    (void)GattServer_RegisterHandlesForWriteNotifications(NumberOfElements(cpHandles), cpHandles);

    /* Initialize private variables */
    mScanningOn = FALSE;

    /* Allocate scan timeout timer */
    mAppTimerId = TMR_AllocateTimer();

    /* Start services */
    (void)Ans_Start(&ansServiceConfig);

    basServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
    (void)Bas_Start(&basServiceConfig);

    (void)Cts_Start(&ctsServiceConfig);
    (void)Ndcs_Start(&ndcsServiceConfig);
    (void)Rtus_Start(&rtusServiceConfig);

    /* Allocate application timers */
    mBatteryMeasurementTimerId = TMR_AllocateTimer();
    mCTSTickTimerId = TMR_AllocateTimer();
    mRTUSReferenceUpdateTimerId = TMR_AllocateTimer();

    /* Start local time tick timer */
    (void)TMR_StartLowPowerTimer(mCTSTickTimerId, gTmrLowPowerIntervalMillisTimer_c,
                                 TmrSeconds(1), CTSTickTimerCallback, NULL);

    /* Start reference update timer */
    (void)TMR_StartLowPowerTimer(mRTUSReferenceUpdateTimerId, gTmrLowPowerIntervalMillisTimer_c,
                                 TmrSeconds(60), RTUSReferenceUpdateTimerCallback, NULL);
}

/*! *********************************************************************************
* \brief        Handles BLE Scanning callback from host stack.
*
* \param[in]    pScanningEvent    Pointer to gapScanningEvent_t.
********************************************************************************** */
static void BleApp_ScanningCallback(gapScanningEvent_t *pScanningEvent)
{
    switch (pScanningEvent->eventType)
    {
        case gDeviceScanned_c:
        {
            if (CheckScanEvent(&pScanningEvent->eventData.scannedDevice))
            {
                gConnReqParams.peerAddressType = pScanningEvent->eventData.scannedDevice.addressType;
                FLib_MemCpy(gConnReqParams.peerAddress,
                            pScanningEvent->eventData.scannedDevice.aAddress,
                            sizeof(bleDeviceAddress_t));

                (void)Gap_StopScanning();
#if gAppUsePrivacy_d
                gConnReqParams.usePeerIdentityAddress = pScanningEvent->eventData.scannedDevice.advertisingAddressResolved;
#endif
                (void)App_Connect(&gConnReqParams, BleApp_ConnectionCallback);
            }
        }
        break;

        case gScanStateChanged_c:
        {
            mScanningOn = !mScanningOn;

            /* Node starts scanning */
            if (mScanningOn)
            {
                /* Start scan timeout timer */
                (void)TMR_StartLowPowerTimer(mAppTimerId,
                                             gTmrLowPowerSecondTimer_c,
                                             TmrSeconds(gScanningTime_c),
                                             ScanningTimeoutTimerCallback, NULL);
                LED_StopFlashingAllLeds();
                Led1Flashing();
            }
            /* Node is not scanning */
            else
            {
                (void)TMR_StopTimer(mAppTimerId);

                Led1Flashing();
                Led2Flashing();
                Led3Flashing();
                Led4Flashing();
            }
        }
        break;

        default:
            ; /* For MISRA Compliance */
            break;
    }
}

/*! *********************************************************************************
* \brief        Handles BLE Connection callback from host stack.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    pConnectionEvent    Pointer to gapConnectionEvent_t.
********************************************************************************** */
static void BleApp_ConnectionCallback(deviceId_t peerDeviceId, gapConnectionEvent_t *pConnectionEvent)
{
    /* Connection Manager to handle Host Stack interactions */
    BleConnManager_GapCentralEvent(peerDeviceId, pConnectionEvent);

    switch (pConnectionEvent->eventType)
    {
        case gConnEvtConnected_c:
        {
            /* UI */
            LED_StopFlashingAllLeds();
            Led1On();

            mPeerInformation.deviceId = peerDeviceId;
            mPeerInformation.isBonded = FALSE;

#if gAppUseBonding_d
            (void)Gap_CheckIfBonded(peerDeviceId, &mPeerInformation.isBonded, NULL);

            if (mPeerInformation.isBonded)
            {
                /* Restored custom connection information. Encrypt link */
                (void)Gap_EncryptLink(peerDeviceId);
            }

#endif

            /* Subscribe client*/
            (void)Bas_Subscribe(&basServiceConfig, peerDeviceId);
            (void)Ans_Subscribe(peerDeviceId);
            (void)Cts_Subscribe(peerDeviceId);
            (void)Ndcs_Subscribe(peerDeviceId);
            (void)Rtus_Subscribe(peerDeviceId);

            /* Start battery measurements */
            (void)TMR_StartLowPowerTimer(mBatteryMeasurementTimerId, gTmrLowPowerIntervalMillisTimer_c,
                                         TmrSeconds(mBatteryLevelReportInterval_c), BatteryMeasurementTimerCallback, NULL);
        }
        break;

        case gConnEvtDisconnected_c:
        {
            mPeerInformation.deviceId = gInvalidDeviceId_c;

            /* Stop battery measurements */
            (void)TMR_StopTimer(mBatteryMeasurementTimerId);

            /* UI */
            LED_TurnOffAllLeds();
            LED_StartFlash(LED_ALL);

            /* Unsubscribe client*/
            (void)Bas_Unsubscribe(&basServiceConfig, peerDeviceId);
            (void)Ans_Unsubscribe();
            (void)Cts_Unsubscribe();
        }
        break;

        case gConnEvtPairingRequest_c:
        {
#if gAppUsePairing_d
            gPairingParameters.peripheralKeys = pConnectionEvent->eventData.pairingEvent.peripheralKeys;
            (void)Gap_AcceptPairingRequest(peerDeviceId, &gPairingParameters);
#else
            (void)Gap_RejectPairing(peerDeviceId, gPairingNotSupported_c);
#endif
        }
        break;

#if gAppUsePairing_d

        case gConnEvtPasskeyRequest_c:
            (void)Gap_EnterPasskey(peerDeviceId, gPasskeyValue_c);
            break;

        case gConnEvtPasskeyDisplay_c:
            /* Display on a screen or simply ignore */
            break;

        case gConnEvtPairingResponse_c:
        {
            if ((pConnectionEvent->eventData.pairingEvent.localIoCapabilities == gIoKeyboardDisplay_c) ||
                (pConnectionEvent->eventData.pairingEvent.localIoCapabilities == gIoKeyboardOnly_c))
            {
                (void)Gap_SetLocalPasskey(gPasskeyValue_c);
            }

            break;
        }

        case gConnEvtKeyExchangeRequest_c:
        {
            gapSmpKeys_t sentSmpKeys = gSmpKeys;

            if ((pConnectionEvent->eventData.keyExchangeRequestEvent.requestedKeys & gLtk_c) == 0U)
            {
                sentSmpKeys.aLtk = NULL;
                /* When the LTK is NULL EDIV and Rand are not sent and will be ignored. */
            }

            if ((pConnectionEvent->eventData.keyExchangeRequestEvent.requestedKeys & gIrk_c) == 0U)
            {
                sentSmpKeys.aIrk = NULL;
                /* When the IRK is NULL the Address and Address Type are not sent and will be ignored. */
            }

            if ((pConnectionEvent->eventData.keyExchangeRequestEvent.requestedKeys & gCsrk_c) == 0U)
            {
                sentSmpKeys.aCsrk = NULL;
            }

            (void)Gap_SendSmpKeys(peerDeviceId, &sentSmpKeys);
            break;
        }

        case gConnEvtLongTermKeyRequest_c:
            if ((pConnectionEvent->eventData.longTermKeyRequestEvent.ediv == gSmpKeys.ediv) &&
                (pConnectionEvent->eventData.longTermKeyRequestEvent.randSize == gSmpKeys.cRandSize))
            {
                (void)Gap_LoadEncryptionInformation(peerDeviceId, gSmpKeys.aLtk, &gSmpKeys.cLtkSize);
                /* EDIV and RAND both matched */
                (void)Gap_ProvideLongTermKey(peerDeviceId, gSmpKeys.aLtk, gSmpKeys.cLtkSize);
            }
            else
                /* EDIV or RAND size did not match */
            {
                (void)Gap_DenyLongTermKey(peerDeviceId);
            }

            break;

        case gConnEvtSlaveSecurityRequest_c:
        {
            (void)Gap_Pair(peerDeviceId, &gPairingParameters);
            break;
        }

#endif /* gAppUsePairing_d */

        default:
            ; /* For MISRA Compliance */
            break;
    }
}


/*! *********************************************************************************
* \brief        Handles GATT server callback from host stack.
*
* \param[in]    deviceId        Peer device ID.
* \param[in]    pServerEvent    Pointer to gattServerEvent_t.
********************************************************************************** */
static void BleApp_GattServerCallback(deviceId_t deviceId, gattServerEvent_t *pServerEvent)
{
    uint16_t handle;
    uint8_t status;

    switch (pServerEvent->eventType)
    {
        case gEvtAttributeWritten_c:
        {
            handle = pServerEvent->eventData.attributeWrittenEvent.handle;
            status = (uint8_t)gAttErrCodeNoError_c;

            if (handle == (uint16_t)value_alert_cp)
            {
                Ans_ControlPointHandler(&ansServiceConfig, &pServerEvent->eventData.attributeWrittenEvent);
            }
            else if (handle == (uint16_t)value_current_time)
            {
                Cts_CurrentTimeWrittenHandler(&ctsServiceConfig, &pServerEvent->eventData.attributeWrittenEvent);
            }
            else
            {
                (void)GattServer_SendAttributeWrittenStatus(deviceId, handle, status);
            }

            break;
        }

        case gEvtAttributeWrittenWithoutResponse_c:
        {
            handle = pServerEvent->eventData.attributeWrittenEvent.handle;

            if (handle == (uint16_t)value_time_update_cp)
            {
                Rtus_ControlPointHandler(&rtusServiceConfig, &pServerEvent->eventData.attributeWrittenEvent);
            }

            break;
        }

        default:
            ; /* For MISRA Compliance */
            break;
    }
}

static bool_t MatchDataInAdvElementList(gapAdStructure_t *pElement, void *pData, uint8_t iDataLen)
{
    uint8_t i;
    bool_t status = FALSE;

    for (i = 0U; i < (pElement->length - 1U); i += iDataLen)
    {
        if (FLib_MemCmp(pData, &pElement->aData[i], iDataLen))
        {
            status = TRUE;
            break;
        }
    }

    return status;
}

static bool_t CheckScanEvent(gapScannedDevice_t *pData)
{
    uint8_t index = 0;
    uint8_t name[10];
    uint8_t nameLength;
    bool_t foundMatch = FALSE;
    uint16_t uuid = 0U;

    while (index < pData->dataLength)
    {
        gapAdStructure_t adElement;

        adElement.length = pData->data[index];
        adElement.adType = (gapAdType_t)pData->data[index + 1U];
        adElement.aData = &pData->data[index + 2U];

        /* Search for Temperature Custom Service */
        if ((adElement.adType == gAdIncomplete16bitServiceList_c) ||
            (adElement.adType == gAdComplete16bitServiceList_c) ||
            (adElement.adType == gAdServiceSolicitationList16bit_c))
        {
            uuid = gBleSig_AlertNotificationService_d;
            foundMatch = MatchDataInAdvElementList(&adElement, &uuid, sizeof(uint16_t));

            /* Connect to a time client for Time Service Demo */
            if (foundMatch != TRUE)
            {
                uuid = gBleSig_CurrentTimeService_d;
                foundMatch = MatchDataInAdvElementList(&adElement, &uuid, sizeof(uint16_t));
            }

            if (foundMatch != TRUE)
            {
                uuid = gBleSig_ReferenceTimeUpdateService_d;
                foundMatch = MatchDataInAdvElementList(&adElement, &uuid, sizeof(uint16_t));
            }

            if (foundMatch != TRUE)
            {
                uuid = gBleSig_NextDSTChangeService_d;
                foundMatch = MatchDataInAdvElementList(&adElement, &uuid, sizeof(uint16_t));
            }

            /* Connect to Blood Pressure Server to update it's time (for Demo purpose)*/
            if (foundMatch != TRUE)
            {
                uuid = gBleSig_BloodPressureService_d;
                foundMatch = MatchDataInAdvElementList(&adElement, &uuid, sizeof(uint16_t));
            }
        }

        if ((adElement.adType == gAdShortenedLocalName_c) ||
            (adElement.adType == gAdCompleteLocalName_c))
        {
            nameLength = MIN(adElement.length, 10U);
            FLib_MemCpy(name, adElement.aData, nameLength);
        }

        /* Move on to the next AD element type */
        index += adElement.length + sizeof(uint8_t);
    }

    return foundMatch;
}

static void ScanningTimeoutTimerCallback(void *pParam)
{
    /* Stop scanning */
    if (mScanningOn)
    {
        (void)Gap_StopScanning();
    }
}

static void BleApp_SendNewAlert(void)
{
    ansNewAlert_t alert = {(ansCategoryId_t)gAns_Email_c, 1U, "New mail"};
    alert.textLength = 8;

    (void)Ans_SendNewAlert(ansServiceConfig.serviceHandle, &alert);
}

static void BleApp_SendUnreadAlertStatus(void)
{
    ansUnreadAlertStatus_t alert = {(uint8_t)gAns_MissedCall_c, 3};
    (void)Ans_SendUnreadAlertStatus(ansServiceConfig.serviceHandle, &alert);
}

/*! *********************************************************************************
* \brief        Handles battery measurement timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void BatteryMeasurementTimerCallback(void *pParam)
{
    basServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
    (void)Bas_RecordBatteryMeasurement(&basServiceConfig);
}

/*! *********************************************************************************
* \brief        Handles current time tick callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void CTSTickTimerCallback(void *pParam)
{
    ctsServiceConfig.localTime++;
    ctsServiceConfig.adjustReason = gCts_UnknownReason;
    (void)Cts_RecordCurrentTime(&ctsServiceConfig);
}

/*! *********************************************************************************
* \brief        Handles time update callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void RTUSReferenceUpdateTimerCallback(void *pParam)
{
    if (rtusServiceConfig.timeUpdateState.currentState == gRtusUpdatePending_c)
    {
        rtusResult_t result = gRtusSuccessful_c;

        /* We simulate an update just for demo purposes */
        rtusServiceConfig.timeUpdateState.currentState = gRtusIdle_c;
        rtusServiceConfig.timeUpdateState.result = result;
        (void)Rtus_RecordTimeUpdateState(&rtusServiceConfig);
        ctsServiceConfig.adjustReason = gCts_ExternalRefUpdate;
        (void)Cts_RecordCurrentTime(&ctsServiceConfig);
    }
}

/*! *********************************************************************************
* @}
********************************************************************************** */
