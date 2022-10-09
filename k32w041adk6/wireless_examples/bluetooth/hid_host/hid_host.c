/*! *********************************************************************************
* \addtogroup HID Host
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2014, Freescale Semiconductor, Inc.
* Copyright 2016-2020 NXP
* All rights reserved.
*
* \file
*
* This file is the source file for the HID Host application
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
#include "shell.h"
#include "Panic.h"

/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1U)
#include "dynamic_gatt_database.h"
#else
#include "gatt_db_handles.h"
#endif

/* Profile / Services */
#include "hid_interface.h"

#include "ble_conn_manager.h"
#include "ble_service_discovery.h"

#include "ApplMain.h"
#include "hid_host.h"

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1U)
#include "erpc_host.h"
#include "dynamic_gatt_database.h"
#endif

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
/*  appEvent_t  */
#define mAppEvt_PeerConnected_c             0x00U
#define mAppEvt_PairingComplete_c           0x01U
#define mAppEvt_ServiceDiscoveryComplete_c  0x02U
#define mAppEvt_ServiceDiscoveryFailed_c    0x03U
#define mAppEvt_GattProcComplete_c          0x04U
#define mAppEvt_GattProcError_c             0x05U

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
typedef uint8_t appEvent_t;

typedef enum appState_tag{
    mAppIdle_c,
    mAppExchangeMtu_c,
    mAppServiceDisc_c,
    mAppReadDescriptor_c,
    mAppRunning_c,
}appState_t;

typedef struct appCustomInfo_tag
{
    hidcConfig_t     hidClientConfig;
    /* Add persistent information here */
}appCustomInfo_t;

typedef struct appPeerInfo_tag
{
    deviceId_t      deviceId;
    appCustomInfo_t customInfo;
    bool_t          isBonded;
    appState_t      appState;
}appPeerInfo_t;

typedef struct mouseHidReport_tag{
  uint8_t buttonStatus;
  uint8_t xAxis;
  uint8_t yAxis;
}mouseHidReport_t;
/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
static appPeerInfo_t mPeerInformation[gAppMaxConnections_c];

#if gAppUseBonding_d
static bool_t mRestoringBondedLink = FALSE;
static bool_t mAuthRejected = FALSE;
#if gAppUsePrivacy_d
static bool_t mAttemptRpaResolvingAtConnect = FALSE;
#endif
#endif

static bool_t   mScanningOn = FALSE;
static bool_t   mFoundDeviceToConnect = FALSE;

/* Buffer used for Characteristic related procedures */
static gattAttribute_t      *mpCharProcBuffer = NULL;

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

/* Host Stack callbacks */
static void BleApp_ScanningCallback
(
    gapScanningEvent_t* pScanningEvent
);

static void BleApp_ConnectionCallback
(
    deviceId_t peerDeviceId,
    gapConnectionEvent_t* pConnectionEvent
);

static void BleApp_GattClientCallback
(
    deviceId_t              serverDeviceId,
    gattProcedureType_t     procedureType,
    gattProcedureResult_t   procedureResult,
    bleResult_t             error
);

static void BleApp_GattNotificationCallback
(
    deviceId_t          serverDeviceId,
    uint16_t characteristicValueHandle,
    uint8_t* aValue,
    uint16_t valueLength
);

static void BleApp_ServiceDiscoveryCallback
(
    deviceId_t deviceId,
    servDiscEvent_t* pEvent
);

static void BleApp_Config(void);

void BleApp_StateMachineHandler
(
    deviceId_t peerDeviceId,
    uint8_t event
);

static bool_t CheckScanEvent(gapScannedDevice_t* pData);

static void BleApp_StoreServiceHandles
(
    deviceId_t      peerDeviceId,
    gattService_t   *pService
);

static bleResult_t BleApp_ConfigureNotifications
(
    deviceId_t peerDeviceId
);

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
    /* UI */
    shell_init("BLE HID Host>");

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1U)
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
            for (uint8_t i = 0U; i < gAppMaxConnections_c; i++)
            {
                if (mPeerInformation[i].deviceId != gInvalidDeviceId_c)
                {
                    (void)Gap_Disconnect(mPeerInformation[i].deviceId);
                }
            }
            break;
        }
        case gKBD_EventPressPB2_c:
        case gKBD_EventLongPB2_c:
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
void BleApp_GenericCallback (gapGenericEvent_t* pGenericEvent)
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
        panic(0,0,0,0);
        return;
    }
#endif /* MULTICORE_APPLICATION_CORE */

    /* Configure as GAP Central */
    BleConnManager_GapCommonConfig();

    /* Register for callbacks*/
    (void)App_RegisterGattClientProcedureCallback(BleApp_GattClientCallback);
    (void)App_RegisterGattClientNotificationCallback(BleApp_GattNotificationCallback);
    BleServDisc_RegisterCallback(BleApp_ServiceDiscoveryCallback);

    for (uint8_t i = 0U; i < gAppMaxConnections_c; i++)
    {
        mPeerInformation[i].deviceId = gInvalidDeviceId_c;
        mPeerInformation[i].appState = mAppIdle_c;
    }

    mScanningOn = FALSE;
    mFoundDeviceToConnect = FALSE;

    /* UI */
    shell_write("\r\nPress SCANSW to connect to a HID Device!\r\n");
}

/*! *********************************************************************************
* \brief        Handles BLE Scanning callback from host stack.
*
* \param[in]    pScanningEvent    Pointer to gapScanningEvent_t.
********************************************************************************** */
static void BleApp_ScanningCallback (gapScanningEvent_t* pScanningEvent)
{
#if gAppUsePrivacy_d && gAppUseBonding_d
    uint8_t bondedDevicesCnt = 0;
#endif

    switch (pScanningEvent->eventType)
    {
        case gDeviceScanned_c:
        {
            if(mFoundDeviceToConnect == FALSE)
            {
                mFoundDeviceToConnect = CheckScanEvent(&pScanningEvent->eventData.scannedDevice);

                if (mFoundDeviceToConnect == TRUE)
                {
                    gConnReqParams.peerAddressType = pScanningEvent->eventData.scannedDevice.addressType;
                    FLib_MemCpy(gConnReqParams.peerAddress,
                                pScanningEvent->eventData.scannedDevice.aAddress,
                                sizeof(bleDeviceAddress_t));

                    (void)Gap_StopScanning();
#if gAppUsePrivacy_d
                    gConnReqParams.usePeerIdentityAddress = pScanningEvent->eventData.scannedDevice.advertisingAddressResolved;
#endif
                }
            }
        }
        break;

        case gScanStateChanged_c:
        {
            mScanningOn = !mScanningOn;
            if (mScanningOn)
            {
                mFoundDeviceToConnect = FALSE;
                LED_StopFlashingAllLeds();
                Led1Flashing();
                shell_write("\r\nScanning...\r\n");
            }
            else /* Scanning is turned OFF */
            {
                if(mFoundDeviceToConnect == TRUE)
                {
#if gAppUsePrivacy_d
                    if(gConnReqParams.peerAddressType == gBleAddrTypeRandom_c)
                    {
#if gAppUseBonding_d
                        /* Check if there are any bonded devices */
                        Gap_GetBondedDevicesCount(&bondedDevicesCnt);

                        if(bondedDevicesCnt == 0)
                        {
                            /* display the unresolved RPA address */
                            shell_writeHexLe(gConnReqParams.peerAddress, gcBleDeviceAddressSize_c);
                        }
                        else
                        {
                            mAttemptRpaResolvingAtConnect = TRUE;
                        }
#else
                        /* If bonding is disabled and we receive an RPA address there is nothing to do but display it */
                        shell_writeHexLe(gConnReqParams.peerAddress, gcBleDeviceAddressSize_c);
#endif /* gAppUseBonding_d */
                    }
#else
                    /* Display the peer address */
                    shell_writeHexLe(gConnReqParams.peerAddress, gcBleDeviceAddressSize_c);
#endif /* gAppUsePrivacy_d */

                    (void)App_Connect(&gConnReqParams, BleApp_ConnectionCallback);
                }
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
static void BleApp_ConnectionCallback (deviceId_t peerDeviceId, gapConnectionEvent_t* pConnectionEvent)
{
    /* Connection Manager to handle Host Stack interactions */
    BleConnManager_GapCentralEvent(peerDeviceId, pConnectionEvent);

    switch (pConnectionEvent->eventType)
    {
        case gConnEvtConnected_c:
        {
#if gAppUsePrivacy_d && gAppUseBonding_d
            if(mAttemptRpaResolvingAtConnect == TRUE)
            {
                /* If the peer RPA was resolved, the IA is displayed, otherwise the peer RPA address is displayed */
                shell_writeHexLe(pConnectionEvent->eventData.connectedEvent.peerAddress, gcBleDeviceAddressSize_c);
                /* clear the flag */
                mAttemptRpaResolvingAtConnect = FALSE;
            }
#endif
            /* UI */
            LED_StopFlashingAllLeds();
            Led1On();
            shell_write("\r\nConnected to device ");
            shell_writeDec(peerDeviceId);
            shell_write("!\r\n");

            mPeerInformation[peerDeviceId].deviceId = peerDeviceId;
            mPeerInformation[peerDeviceId].isBonded = FALSE;

#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
            bool_t isBonded = FALSE;
            (void)Gap_CheckIfBonded(peerDeviceId, &isBonded, NULL);

            if ((isBonded) &&
                (gBleSuccess_c == Gap_LoadCustomPeerInformation(peerDeviceId,
                    (void*) &mPeerInformation[peerDeviceId].customInfo, 0, sizeof (appCustomInfo_t))))
            {
                mRestoringBondedLink = TRUE;
                /* Restored custom connection information. Encrypt link */
                (void)Gap_EncryptLink(peerDeviceId);
            }
            else
            {
                mRestoringBondedLink = FALSE;
                mAuthRejected = FALSE;
            }
#endif
            BleApp_StateMachineHandler(mPeerInformation[peerDeviceId].deviceId, mAppEvt_PeerConnected_c);
        }
        break;

        case gConnEvtDisconnected_c:
        {
            mPeerInformation[peerDeviceId].deviceId = gInvalidDeviceId_c;
            mPeerInformation[peerDeviceId].appState = mAppIdle_c;

            /* Reset the custom information for the current peer device */
            FLib_MemSet(&(mPeerInformation[peerDeviceId].customInfo), 0x00, sizeof(appCustomInfo_t));

            /* Reset Service Discovery to be sure*/
            BleServDisc_Stop(peerDeviceId);

            /* UI */
            LED_TurnOffAllLeds();
            LED_StartFlash(LED_ALL);
            shell_write("\r\nDisconnected from device ");
            shell_writeDec(peerDeviceId);
            shell_write("!\r\n");

            /* Check if the last device was disconnected and if so re-enter GAP Limited Discovery Mode */
            uint8_t connectedDevicesNumber = 0U;
            for (uint8_t i = 0U; i < gAppMaxConnections_c; i++)
            {
                if (mPeerInformation[i].deviceId != gInvalidDeviceId_c)
                {
                    connectedDevicesNumber++;
                }
            }
            if(connectedDevicesNumber == 0U)
            {
                /* Restart advertising */
                BleApp_Start();
            }

        }
        break;

#if gAppUsePairing_d
        case gConnEvtPairingComplete_c:
        {
            if (pConnectionEvent->eventData.pairingCompleteEvent.pairingSuccessful)
            {
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
                mPeerInformation[peerDeviceId].isBonded = TRUE;
#endif
                BleApp_StateMachineHandler(mPeerInformation[peerDeviceId].deviceId, mAppEvt_PairingComplete_c);
                shell_write("\r\n-->  GAP Event: Device Paired.\r\n");
            }
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
            else
            {
                mPeerInformation[peerDeviceId].isBonded = FALSE;
                shell_write("\r\n-->  GAP Event: Pairing Unsuccessful.\r\n");
            }
#endif
        }
        break;

#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
        case gConnEvtEncryptionChanged_c:
        {
            if( pConnectionEvent->eventData.encryptionChangedEvent.newEncryptionState )
            {
                mPeerInformation[peerDeviceId].isBonded = TRUE;
                if ( (TRUE == mRestoringBondedLink) &&
                     (FALSE == mAuthRejected) )
                {
                    if( gBleSuccess_c != BleApp_ConfigureNotifications(peerDeviceId) )
                    {
                        Gap_Disconnect(peerDeviceId);
                    }
                }
                else
                {
                    mRestoringBondedLink = FALSE;
                }
            }
        }
        break;

        case gConnEvtAuthenticationRejected_c:
        {
            /* Start Pairing Procedure */
            mAuthRejected = TRUE;
            Gap_Pair(peerDeviceId, &gPairingParameters);
        }
        break;
#endif /* gAppUseBonding_d */
#endif /* gAppUsePairing_d */

    default:
        ; /* For MISRA Compliance */
        break;
    }
}

static void BleApp_ServiceDiscoveryCallback(deviceId_t deviceId, servDiscEvent_t* pEvent)
{
    switch(pEvent->eventType)
    {
        case gServiceDiscovered_c:
        {
            BleApp_StoreServiceHandles(deviceId, pEvent->eventData.pService);
        }
        break;

        case gDiscoveryFinished_c:
        {
            if (pEvent->eventData.success)
            {
                BleApp_StateMachineHandler(deviceId, mAppEvt_ServiceDiscoveryComplete_c);
            }
            else
            {
                BleApp_StateMachineHandler(deviceId, mAppEvt_ServiceDiscoveryFailed_c);
            }
        }
        break;

        default:
            ; /* For MISRA Compliance */
        break;
    }
}


/*! *********************************************************************************
* \brief        Handles GATT client callback from host stack.
*
* \param[in]    serverDeviceId      GATT Server device ID.
* \param[in]    procedureType       Procedure type.
* \param[in]    procedureResult     Procedure result.
* \param[in]    error               Callback result.
********************************************************************************** */
static void BleApp_GattClientCallback(
    deviceId_t              serverDeviceId,
    gattProcedureType_t     procedureType,
    gattProcedureResult_t   procedureResult,
    bleResult_t             error
)
{
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
    if ((mPeerInformation[serverDeviceId].isBonded) || (mPeerInformation[serverDeviceId].appState != mAppRunning_c))
    {
#endif
        if (procedureResult == gGattProcError_c)
        {
            attErrorCode_t attError = (attErrorCode_t)((uint8_t)error);

            if (attError == gAttErrCodeInsufficientEncryption_c     ||
                attError == gAttErrCodeInsufficientAuthorization_c  ||
                attError == gAttErrCodeInsufficientAuthentication_c)
            {
                /* Start Pairing Procedure */
                (void)Gap_Pair(serverDeviceId, &gPairingParameters);
            }

            BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcError_c);
        }
        else if (procedureResult == gGattProcSuccess_c)
        {
            BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcComplete_c);
        }
        else
        {
            /* For MISRA Compliance */
        }

        /* Signal Service Discovery Module */
        BleServDisc_SignalGattClientEvent(serverDeviceId, procedureType, procedureResult, error);
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
    }
#endif
}

/*! *********************************************************************************
* \brief        Handles GATT client notification callback from host stack.
*
* \param[in]    serverDeviceId              GATT Server device ID.
* \param[in]    characteristicValueHandle   Handle.
* \param[in]    aValue                      Pointer to value.
* \param[in]    valueLength                 Value length.
********************************************************************************** */
static void BleApp_GattNotificationCallback
(
    deviceId_t  serverDeviceId,
    uint16_t    characteristicValueHandle,
    uint8_t*    aValue,
    uint16_t    valueLength
)
{
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
    if (mPeerInformation[serverDeviceId].isBonded)
    {
#endif
      union {
          uint8_t* aValueTemp;
          mouseHidReport_t* pReportTemp;
      }mouseValues;
      mouseValues.aValueTemp = aValue;

      if (characteristicValueHandle == mPeerInformation[serverDeviceId].customInfo.hidClientConfig.hHidReport)
      {
          mouseHidReport_t *pReport = mouseValues.pReportTemp;
          shell_write("\r\nReceived HID Report from device ");
          shell_writeDec(serverDeviceId);
          shell_write(": X: ");
          shell_writeHex(&pReport->xAxis, sizeof(uint8_t));
          shell_write(" Y: ");
          shell_writeHex(&pReport->yAxis, sizeof(uint8_t));
      }
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
    }
#endif
}

/*! *********************************************************************************
* \brief        Stores handles for the specified service.
*
* \param[in]    pService    Pointer to gattService_t.
********************************************************************************** */
static void BleApp_StoreServiceHandles
(
    deviceId_t      peerDeviceId,
    gattService_t   *pService
)
{
    uint8_t i, j;

    if ((pService->uuidType == gBleUuidType16_c) &&
        (pService->uuid.uuid16 == gBleSig_HidService_d))
    {
        /* Found HID Service */
        mPeerInformation[peerDeviceId].customInfo.hidClientConfig.hService = pService->startHandle;

        for (i = 0; i < pService->cNumCharacteristics; i++)
        {
            if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType16_c) &&
                (pService->aCharacteristics[i].value.uuid.uuid16 == gBleSig_Report_d))
            {
                /* Found HID Report Char */
                mPeerInformation[peerDeviceId].customInfo.hidClientConfig.hHidReport = pService->aCharacteristics[i].value.handle;

                for (j = 0; j < pService->aCharacteristics[i].cNumDescriptors; j++)
                {
                    if (( pService->aCharacteristics[i].aDescriptors[j].uuidType == gBleUuidType16_c) &&
                    ( pService->aCharacteristics[i].aDescriptors[j].uuid.uuid16 == gBleSig_CCCD_d))
                    {
                        mPeerInformation[peerDeviceId].customInfo.hidClientConfig.hHidReportCccd =  pService->aCharacteristics[i].aDescriptors[j].handle;
                    }
                }
            }
        }
    }
}

static bool_t MatchDataInAdvElementList(gapAdStructure_t *pElement, void *pData, uint8_t iDataLen)
{
    uint8_t i;
    bool_t status = FALSE;

    for (i = 0; i < (pElement->length - 1U); i += iDataLen)
    {
        if (FLib_MemCmp(pData, &pElement->aData[i], iDataLen))
        {
            status = TRUE;
            break;
        }
    }
    return status;
}

static bool_t CheckScanEvent(gapScannedDevice_t* pData)
{
    uint8_t index = 0;
    uint8_t name[10];
    uint8_t nameLength = 0U;
    bool_t foundMatch = FALSE;

    while (index < pData->dataLength)
    {
        gapAdStructure_t adElement;

        adElement.length = pData->data[index];
        adElement.adType = (gapAdType_t)pData->data[index + 1U];
        adElement.aData = &pData->data[index + 2U];

         /* Search for HID Service */
        if ((adElement.adType == gAdIncomplete16bitServiceList_c) ||
          (adElement.adType == gAdComplete16bitServiceList_c))
        {
            uint16_t uuid = gBleSig_HidService_d;
            foundMatch = MatchDataInAdvElementList(&adElement, &uuid, sizeof(uint16_t));
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

    if (foundMatch)
    {
        /* UI */
        shell_write("\r\nFound device: \r\n");
        if (nameLength != 0U)
        {
            shell_writeN((char*)name, (uint16_t)nameLength-1U);
            SHELL_NEWLINE();
        }
    }
    return foundMatch;
}

void BleApp_StateMachineHandler(deviceId_t peerDeviceId, uint8_t event)
{
    switch (mPeerInformation[peerDeviceId].appState)
    {
        case mAppIdle_c:
        {
            if (event == mAppEvt_PeerConnected_c)
            {
#if gAppUseBonding_d
                if ( mRestoringBondedLink &&
                    (mPeerInformation[peerDeviceId].customInfo.hidClientConfig.hHidReportCccd != gGattDbInvalidHandle_d) )
                {
                    /* Moving to Running State and wait for Link encryption result */
                    mPeerInformation[peerDeviceId].appState = mAppRunning_c;
                }
                else
#endif
                {
                    /* Moving to Exchange MTU State */
                    mPeerInformation[peerDeviceId].appState = mAppExchangeMtu_c;
                   (void)GattClient_ExchangeMtu(peerDeviceId, gAttMaxMtu_c);
                }
            }
        }
        break;

        case mAppExchangeMtu_c:
        {
            if (event == mAppEvt_GattProcComplete_c)
            {
                /* Moving to Primary Service Discovery State*/
                mPeerInformation[peerDeviceId].appState = mAppServiceDisc_c;

                /* Start Service Discovery*/
                if( gBleSuccess_c != BleServDisc_Start(peerDeviceId) )
                {
                    Gap_Disconnect(peerDeviceId);
                }
            }
            else if (event == mAppEvt_GattProcError_c)
            {
                (void)Gap_Disconnect(peerDeviceId);
            }
        }
        break;

        case mAppServiceDisc_c:
        {
            if (event == mAppEvt_ServiceDiscoveryComplete_c)
            {
                /* Moving to Primary Service Discovery State*/
                mPeerInformation[peerDeviceId].appState = mAppReadDescriptor_c;

                if (mPeerInformation[peerDeviceId].customInfo.hidClientConfig.hHidReportCccd != gGattDbInvalidHandle_d)
                {
                    mpCharProcBuffer = MEM_BufferAlloc(sizeof(gattAttribute_t) + gAttDefaultMtu_c);
                    if (mpCharProcBuffer == NULL)
                    {
                        Gap_Disconnect(peerDeviceId);
                    }
                    else
                    {
                        mpCharProcBuffer->handle = mPeerInformation[peerDeviceId].customInfo.hidClientConfig.hHidReportCccd;
                        mpCharProcBuffer->paValue = (uint8_t*)(mpCharProcBuffer + 1);
                    GattClient_ReadCharacteristicDescriptor(mPeerInformation[peerDeviceId].deviceId, mpCharProcBuffer ,23);
                    }
                }
            }
            else if (event == mAppEvt_ServiceDiscoveryFailed_c)
            {
                (void)Gap_Disconnect(peerDeviceId);
            }
        }
        break;

        case mAppReadDescriptor_c:
        {
            if (event == mAppEvt_GattProcComplete_c)
            {
                if (mPeerInformation[peerDeviceId].customInfo.hidClientConfig.hHidReportCccd != gGattDbInvalidHandle_d)
                {
                    if( gBleSuccess_c != BleApp_ConfigureNotifications(peerDeviceId) )
                    {
                        Gap_Disconnect(peerDeviceId);
                    }
                    else
                    {
                        /* Moving to Running State*/
                        mPeerInformation[peerDeviceId].appState = mAppRunning_c;
                    }
                }
            }
            else if (event == mAppEvt_PairingComplete_c)
            {
                /* Continue after pairing is complete */
                (void)GattClient_ReadCharacteristicDescriptor(peerDeviceId, mpCharProcBuffer ,23);
            }
            else
            {
                /* For MISRA Compliance */
            }
        }
        break;

        case mAppRunning_c:
        {
            if (event == mAppEvt_GattProcComplete_c)
            {
                if (mpCharProcBuffer != NULL)
                {
                    (void)MEM_BufferFree(mpCharProcBuffer);
                    mpCharProcBuffer = NULL;
                }

#if gAppUseBonding_d
                /* Write data in NVM */
                (void)Gap_SaveCustomPeerInformation(peerDeviceId,
                                              (void*) &mPeerInformation[peerDeviceId].customInfo, 0,
                                              sizeof (appCustomInfo_t));
#endif
            }
            else if (event == mAppEvt_PairingComplete_c)
            {
                if( gBleSuccess_c != BleApp_ConfigureNotifications(peerDeviceId) )
                {
                    Gap_Disconnect(peerDeviceId);
                }
            }
        }
        break;
    default:
        ; /* For MISRA Compliance */
        break;
    }
}

static bleResult_t BleApp_ConfigureNotifications(deviceId_t peerDeviceId)
{
    bleResult_t result = gBleSuccess_c;
    uint16_t value = gCccdNotification_c;

    if( mpCharProcBuffer == NULL )
    {
        mpCharProcBuffer = MEM_BufferAlloc(sizeof(gattAttribute_t) + gAttDefaultMtu_c);
    }

    if( mpCharProcBuffer != NULL )
    {
        mpCharProcBuffer->handle = mPeerInformation[peerDeviceId].customInfo.hidClientConfig.hHidReportCccd;
        mpCharProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
        mpCharProcBuffer->valueLength = 0;
        GattClient_WriteCharacteristicDescriptor(peerDeviceId,
                                                 mpCharProcBuffer,
                                                 sizeof(value), (void*)&value);
    }
    else
    {
        result = gBleOutOfMemory_c;
    }

    return result;
}

/*! *********************************************************************************
* @}
********************************************************************************** */
