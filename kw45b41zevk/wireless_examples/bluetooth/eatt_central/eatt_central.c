/*! *********************************************************************************
* \addtogroup EATT Central
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright 2021-2024 NXP
*
*
* \file
*
* This file is the source file for the EATT central application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
/* Framework / Drivers */
#include "EmbeddedTypes.h"
#include "RNG_Interface.h"
#include "fsl_component_button.h"
#include "fsl_component_led.h"
#include "fsl_component_panic.h"
#include "fsl_component_serial_manager.h"
#include "fsl_component_mem_manager.h"
#include "FunctionLib.h"

#include "fwk_platform_ble.h"

/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"
#include "gatt_db_handles.h"

#include "ble_conn_manager.h"
#include "ble_service_discovery.h"
#include "app.h"
#include "app_conn.h"
#include "app_scanner.h"
#include "eatt_central.h"

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************/
#define mAppEvt_PeerConnected_c             0x00U
#define mAppEvt_PairingComplete_c           0x01U
#define mAppEvt_ServiceDiscoveryComplete_c  0x02U
#define mAppEvt_ServiceDiscoveryFailed_c    0x03U
#define mAppEvt_GattProcComplete_c          0x04U
#define mAppEvt_GattProcError_c             0x05U
#define mAppEvt_PeerConnectedEATT_c         0x06U
#define mAppEvt_DescriptorWritten_c         0x07U

#define mMaxCbCredits_c                 (65535U)    /* maximum number of credits for an L2CAP channel */

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
typedef uint8_t appEvent_t;

typedef enum appState_tag {
    mAppIdle_c,
    mAppExchangeMtu_c,
    mAppServiceDisc_c,
    mAppWaitForPairingComplete_c,
    mAppReadDescriptorA_c,
    mAppReadDescriptorB_c,
    mAppEATTConn_c,
    mAppRunning_c,
}appState_t;

/*! ServiceA Client - Configuration */
typedef struct serviceAConfig_tag
{
    uint16_t    hService;
    uint16_t    hServAReport;
    uint16_t    hServAReportCccd;
} serviceAConfig_t;

/*! ServiceB Client - Configuration */
typedef struct serviceBConfig_tag
{
    uint16_t    hService;
    uint16_t    hServBReport;
    uint16_t    hServBReportCccd;
} serviceBConfig_t;

typedef struct appPeerInfo_tag
{
    deviceId_t       deviceId;
    bool_t           isBonded;
    serviceAConfig_t servAcfg;
    serviceBConfig_t servBcfg;
    appState_t       appState;
} appPeerInfo_t;

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
/* Buffers used for Characteristic related procedures */
static gattAttribute_t *mpCharProcBuffer = NULL;

#if gAppUseBonding_d
static bool_t mRestoringBondedLink = FALSE;
static bool_t mAuthRejected = FALSE;
#endif

/* Application data */
static bool_t mScanningOn = FALSE;
static appPeerInfo_t mPeerInformation;
static uint16_t mClientSupportedFeaturesCharHandle = gGattDbInvalidHandle_d;
static char initStr[] = "BLE EATT Central>";

static uint16_t mLocalMtu = gEattMaxMtu_c;
static uint16_t mInitialCredits = mMaxCbCredits_c;
static uint8_t  mCBearers = gAppEattMaxNoOfBearers_c;

static uint8_t  mPeerCBearers;
static bearerId_t mABearerIds[gGapEattMaxBearers] = {gUnenhancedBearerId_c};

/* Application scanning parameters */
static appScanningParams_t mAppScanParams = {
    &gScanParams,
    gGapDuplicateFilteringEnable_c,
    gGapScanContinuously_d,
    gGapScanPeriodicDisabled_d
};

static bool_t mbIsPairingComplete = FALSE;

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/
static void BluetoothLEHost_Initialized(void);

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
static button_status_t BleApp_HandleKeys0(void *pButtonHandle, button_callback_message_t *pMessage, void *pCallbackParam);
#endif

/* Scanning helper functions */
static bool_t BleApp_CheckScanEvent(gapScannedDevice_t* pData);

static void BleApp_StoreServiceHandles
(
    deviceId_t      peerDeviceId,
    gattService_t   *pService
);

/* Application state machine */
static bleResult_t BleApp_ConfigureNotifications(deviceId_t peerDeviceId, uint16_t handle);

static void BleApp_StateMachineHandler
(
    deviceId_t peerDeviceId,
    uint8_t event
);

static void BleApp_StateMachineHandler_IdleState
(
    deviceId_t peerDeviceId,
    uint8_t event
);

static void BleApp_StateMachineHandler_ExchangeMtuState
(
    deviceId_t peerDeviceId,
    uint8_t event
);

static void BleApp_StateMachineHandler_ServiceDiscState
(
    deviceId_t peerDeviceId,
    uint8_t event
);

static void BleApp_StateMachineHandler_WaitForPairingCompleteState
(
    deviceId_t peerDeviceId, 
    uint8_t    event
);

static void BleApp_StateMachineHandler_ReadDescriptorAState
(
    deviceId_t peerDeviceId,
    uint8_t event
);

static void BleApp_StateMachineHandler_ReadDescriptorBState
(
    deviceId_t peerDeviceId,
    uint8_t event
);

static void BleApp_StateMachineHandler_RunningState
(
    deviceId_t peerDeviceId,
    uint8_t event
);

static void BleApp_StateMachineHandler_EATTConnState
(
    deviceId_t peerDeviceId,
    uint8_t event
);

/* State machine helper functions */
static void BleApp_MoveToReadDescriptorAState(deviceId_t peerDeviceId);

/* Host Stack callbacks */
static void BleApp_ScanningCallback(gapScanningEvent_t* pScanningEvent);

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

static void BleApp_ServiceDiscoveryCallback
(
    deviceId_t deviceId,
    servDiscEvent_t* pEvent
);

static void BleApp_GattEnhancedNotificationCallback
(
    deviceId_t serverDeviceId,
    bearerId_t bearerId,
    uint16_t   characteristicValueHandle,
    uint8_t*   aValue,
    uint16_t   valueLength
);

static void BluetoothLEHost_GenericCallback(gapGenericEvent_t *pGenericEvent);

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
SHELL_HANDLE_DEFINE(g_shellHandle);

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/
/*! *********************************************************************************
* \brief  This is the initialization function for each application. This function
*         should contain all the initialization code required by the bluetooth demo
********************************************************************************** */
void BluetoothLEHost_AppInit(void)
{
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[0], BleApp_HandleKeys0, NULL);
#endif

    /* Set generic callback */
    BluetoothLEHost_SetGenericCallback(BluetoothLEHost_GenericCallback);

    /* Initialize Bluetooth Host Stack */
    BluetoothLEHost_Init(BluetoothLEHost_Initialized);
}

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(void)
{
    if (!mScanningOn)
    {
        (void)BluetoothLEHost_StartScanning(&mAppScanParams, BleApp_ScanningCallback);
    }
}

/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    pButtonHandle    button handle
* \param[in]    pMessage         Button press event
* \param[in]    pCallbackParam   parameter
********************************************************************************** */
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
static button_status_t BleApp_HandleKeys0(void *pButtonHandle, button_callback_message_t *pMessage, void *pCallbackParam)
{
    switch (pMessage->event)
    {
        case kBUTTON_EventOneClick:
        case kBUTTON_EventShortPress:
        {
            BleApp_Start();
            break;
        }

        case kBUTTON_EventLongPress:
        {
            (void)Gap_Disconnect(mPeerInformation.deviceId);
            break;
        }

        default:
        {
            ; /* For MISRA Compliance */
            break;
        }
    }

    return kStatus_BUTTON_Success;
}
#endif

/*! *********************************************************************************
* \brief        Prints string of hex values
*
* \param[in]    pHex    pointer to hex value.
* \param[in]    len     hex value length.
********************************************************************************** */
void BleApp_PrintHex(uint8_t *pHex, uint8_t len)
{
    for (uint32_t i = 0; i<len; i++)
    {
        (void)SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, (char const*)FORMAT_Hex2Ascii(pHex[i]));
    }
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/
/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
static void BluetoothLEHost_GenericCallback(gapGenericEvent_t *pGenericEvent)
{
    /* Call BLE Conn Manager */
    BleConnManager_GenericEvent(pGenericEvent);
}

/*! *********************************************************************************
 * \brief        Configures BLE Stack after initialization
 *
 ********************************************************************************** */
static void BluetoothLEHost_Initialized(void)
{
    /* Common GAP configuration */
    BleConnManager_GapCommonConfig();

     /* Register for callbacks*/
    (void)App_RegisterGattClientProcedureCallback(BleApp_GattClientCallback);
    (void)GattClient_RegisterEnhancedNotificationCallback(BleApp_GattEnhancedNotificationCallback);
    BleServDisc_RegisterCallback(BleApp_ServiceDiscoveryCallback);

    mPeerInformation.deviceId = gInvalidDeviceId_c;
    mPeerInformation.appState = mAppIdle_c;
    mScanningOn = FALSE;

    /* UI */
    (void)SHELL_Init((shell_handle_t)g_shellHandle, (serial_handle_t)gSerMgrIf, initStr);
    (void)shell_write("\r\nPress SCANSW to connect to an EATT Peripheral!\r\n");

    LedStartFlashingAllLeds();
}

/*! *********************************************************************************
* \brief        Handles BLE Scanning callback from host stack.
*
* \param[in]    pScanningEvent    Pointer to gapScanningEvent_t.
********************************************************************************** */
static void BleApp_ScanningCallback (gapScanningEvent_t* pScanningEvent)
{
    switch (pScanningEvent->eventType)
    {
        case gDeviceScanned_c:
        {
            if (BleApp_CheckScanEvent(&pScanningEvent->eventData.scannedDevice))
            {
                gConnReqParams.peerAddressType = pScanningEvent->eventData.scannedDevice.addressType;
                FLib_MemCpy(gConnReqParams.peerAddress,
                            pScanningEvent->eventData.scannedDevice.aAddress,
                            sizeof(bleDeviceAddress_t));

                (void)Gap_StopScanning();
#if gAppUsePrivacy_d
                gConnReqParams.usePeerIdentityAddress = pScanningEvent->eventData.scannedDevice.advertisingAddressResolved;
#endif
                (void)BluetoothLEHost_Connect(&gConnReqParams, BleApp_ConnectionCallback);
            }
        }
        break;

        case gScanStateChanged_c:
        {
            mScanningOn = !mScanningOn;
            if (mScanningOn)
            {
                /* Stop flashing LEDs */
                LedStopFlashingAllLeds();
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c == 1))
                LedSetColor(0, kLED_Blue);
#endif /*gAppLedCnt_c == 1*/
                /* Flash LED1 */
                Led1Flashing();
                (void)shell_write("\r\nScanning...\r\n");
            }
        }
        break;

        default:
        {
            ; /* For MISRA Compliance */
        }
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
            /* UI */
            LedStopFlashingAllLeds();
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c == 1))
            LedSetColor(0, kLED_White);
#endif /*gAppLedCnt_c == 1*/
            Led1On();

            (void)shell_write("\r\nConnected to device ");
            (void)shell_writeDec(peerDeviceId);
            (void)shell_write("!\r\n");

            mPeerInformation.deviceId = peerDeviceId;
            mPeerInformation.isBonded = FALSE;

#if gAppUseBonding_d
            (void)Gap_CheckIfBonded(peerDeviceId, &mPeerInformation.isBonded, NULL);

            if ((mPeerInformation.isBonded) &&
                (gBleSuccess_c == Gap_LoadCustomPeerInformation(peerDeviceId,
                    (void*) &mPeerInformation.servAcfg, 0, (uint16_t)sizeof(serviceAConfig_t))) &&
                  (gBleSuccess_c == Gap_LoadCustomPeerInformation(peerDeviceId,
                    (void*) &mPeerInformation.servBcfg, (uint16_t)sizeof(serviceAConfig_t), (uint16_t)sizeof(serviceBConfig_t))))
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
            BleApp_StateMachineHandler(mPeerInformation.deviceId, mAppEvt_PeerConnected_c);
        }
        break;

        case gConnEvtDisconnected_c:
        {
            mPeerInformation.deviceId = gInvalidDeviceId_c;
            mPeerInformation.appState = mAppIdle_c;
            mbIsPairingComplete = FALSE;

            /* Reset the custom information for the current peer device */
            FLib_MemSet(&(mPeerInformation.servAcfg), 0x00, sizeof(serviceAConfig_t));
            FLib_MemSet(&(mPeerInformation.servBcfg), 0x00, sizeof(serviceBConfig_t));

            /* Reset Service Discovery to be sure*/
            BleServDisc_Stop(peerDeviceId);

            /* UI */
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c > 0))
            LedStopFlashingAllLeds();
            LedStartFlashingAllLeds();
#endif /*gAppLedCnt_c > 0*/
            (void)shell_write("\r\nDisconnected from device ");
            (void)shell_writeDec(peerDeviceId);
            (void)shell_write("!\r\n");

            /* Restart scanning */
            BleApp_Start();
        }
        break;

#if gAppUsePairing_d
        case gConnEvtPairingComplete_c:
        {
            (void)shell_write("\r\n Pairing Complete \r\n");

            if (pConnectionEvent->eventData.pairingCompleteEvent.pairingSuccessful)
            {
                mbIsPairingComplete = TRUE;
                BleApp_StateMachineHandler(mPeerInformation.deviceId, mAppEvt_PairingComplete_c);
            }
        }
        break;

#if gAppUseBonding_d
        case gConnEvtEncryptionChanged_c:
        {
            if( pConnectionEvent->eventData.encryptionChangedEvent.newEncryptionState )
            {
                if ( (TRUE == mRestoringBondedLink) &&
                     (FALSE == mAuthRejected) )
                {
                    if( gBleSuccess_c != BleApp_ConfigureNotifications(peerDeviceId, mPeerInformation.servAcfg.hServAReportCccd) )
                    {
                        (void)Gap_Disconnect(peerDeviceId);
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
            (void)Gap_Pair(peerDeviceId, &gPairingParameters);
        }
        break;
#endif /* gAppUseBonding_d */
#endif /* gAppUsePairing_d */

        case gConnEvtEattConnectionComplete_c:
        {
            if (gSuccessful_c == pConnectionEvent->eventData.eattConnectionComplete.status)
            {
                /* save the data from the peer if the EATT connection was successful */
                mPeerCBearers = pConnectionEvent->eventData.eattConnectionComplete.cBearers;
                FLib_MemCpy(mABearerIds, pConnectionEvent->eventData.eattConnectionComplete.aBearerIds, mPeerCBearers);
                (void)shell_write("\r\n EATT Connection Complete\r\n");
                BleApp_StateMachineHandler(mPeerInformation.deviceId, mAppEvt_PeerConnectedEATT_c);
            }
            else
            {
                (void)Gap_Disconnect(peerDeviceId);
            }
        }
        break;

        default:
        {
          ; /* For MISRA Compliance */
        }
        break;
    }
}

/*! *********************************************************************************
* \brief        Handles discovered services.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    pEvent              Pointer to servDiscEvent_t.
********************************************************************************** */
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
        {
            ; /* No action required */
        }
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
    if( procedureResult == gGattProcError_c )
    {
        attErrorCode_t attError = (attErrorCode_t)((uint8_t)error);

        if (attError == gAttErrCodeInsufficientEncryption_c     ||
            attError == gAttErrCodeInsufficientAuthorization_c  ||
            attError == gAttErrCodeInsufficientAuthentication_c)
        {
#if gAppUsePairing_d
            /* Start Pairing Procedure */
            (void)Gap_Pair(serverDeviceId, &gPairingParameters);
#endif
        }
        BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcError_c);
    }
    else if( procedureResult == gGattProcSuccess_c )
    {
        if( procedureType == gGattProcWriteCharacteristicDescriptor_c )
        {
            BleApp_StateMachineHandler(serverDeviceId, mAppEvt_DescriptorWritten_c);
        }
        else
        {
            BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcComplete_c);
        }

    }
    else
    {
        /* For MISRA Compliance */
    }

    /* Signal Service Discovery Module */
    BleServDisc_SignalGattClientEvent(serverDeviceId, procedureType, procedureResult, error);
}

/*! *********************************************************************************
* \brief        Handles GATT client notification callback from host stack.
*
* \param[in]    serverDeviceId              GATT Server device ID.
* \param[in]    bearerId                    Bearer on which the notification was received
* \param[in]    characteristicValueHandle   Handle.
* \param[in]    aValue                      Pointer to value.
* \param[in]    valueLength                 Value length.
********************************************************************************** */
static void BleApp_GattEnhancedNotificationCallback
(
    deviceId_t  serverDeviceId,
    bearerId_t  bearerId,
    uint16_t    characteristicValueHandle,
    uint8_t*    aValue,
    uint16_t    valueLength
)
{
    if (characteristicValueHandle == mPeerInformation.servAcfg.hServAReport)
    {
        (void)shell_write("\r\nReceived value of Service A: ");
        shell_writeHex(aValue, (uint8_t)valueLength);
        (void)shell_write(" On bearer: ");
        shell_writeHex(&bearerId, (uint8_t)sizeof(uint8_t));
    }
    else if (characteristicValueHandle == mPeerInformation.servBcfg.hServBReport)
    {
        (void)shell_write("\r\nReceived value of Service B: ");
        shell_writeHex(aValue, (uint8_t)valueLength);
        (void)shell_write(" On bearer: ");
        shell_writeHex(&bearerId, (uint8_t)sizeof(uint8_t));
    }
    else
    {
        ; /* For MISRA Compliance */
    }
}

/*! *********************************************************************************
* \brief        Stores handles for the specified service.
*
* \param[in]    peerDeviceId    Peer identifier
* \param[in]    pService        Pointer to gattService_t.
********************************************************************************** */
static void BleApp_StoreServiceHandles
(
    deviceId_t      peerDeviceId,
    gattService_t   *pService
)
{
    uint8_t i, j;

    if ((pService->uuidType == gBleUuidType16_c) &&
        (pService->uuid.uuid16 == (uint16_t)gBleSig_AService_d))
    {
        /* Found Service A */
        mPeerInformation.servAcfg.hService = pService->startHandle;

        for (i = 0; i < pService->cNumCharacteristics; i++)
        {
            if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType16_c) &&
                (pService->aCharacteristics[i].value.uuid.uuid16 == gBleSig_Report_d))
            {
                /* Found Service A Report Char */
                mPeerInformation.servAcfg.hServAReport = pService->aCharacteristics[i].value.handle;

                for (j = 0; j < pService->aCharacteristics[i].cNumDescriptors; j++)
                {
                    if (( pService->aCharacteristics[i].aDescriptors[j].uuidType == gBleUuidType16_c) &&
                    ( pService->aCharacteristics[i].aDescriptors[j].uuid.uuid16 == gBleSig_CCCD_d))
                    {
                        mPeerInformation.servAcfg.hServAReportCccd =  pService->aCharacteristics[i].aDescriptors[j].handle;
                    }
                }
            }
        }
    }

    if ((pService->uuidType == gBleUuidType16_c) &&
        (pService->uuid.uuid16 == (uint16_t)gBleSig_BService_d))
    {
        /* Found Service B */
        mPeerInformation.servBcfg.hService = pService->startHandle;

        for (i = 0; i < pService->cNumCharacteristics; i++)
        {
            if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType16_c) &&
                (pService->aCharacteristics[i].value.uuid.uuid16 == gBleSig_Report_d))
            {
                /* Found Service B Report Char */
                mPeerInformation.servBcfg.hServBReport = pService->aCharacteristics[i].value.handle;

                for (j = 0; j < pService->aCharacteristics[i].cNumDescriptors; j++)
                {
                    if (( pService->aCharacteristics[i].aDescriptors[j].uuidType == gBleUuidType16_c) &&
                    ( pService->aCharacteristics[i].aDescriptors[j].uuid.uuid16 == gBleSig_CCCD_d))
                    {
                        mPeerInformation.servBcfg.hServBReportCccd =  pService->aCharacteristics[i].aDescriptors[j].handle;
                    }
                }
            }
        }
    }

    if ((pService->uuidType == gBleUuidType16_c) &&
        (pService->uuid.uuid16 == gBleSig_GenericAttributeProfile_d))
    {
        for (i = 0; i < pService->cNumCharacteristics; i++)
        {
            if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType16_c) &&
                (pService->aCharacteristics[i].value.uuid.uuid16 == gBleSig_GattClientSupportedFeatures_d))
            {
                /* Save the handle for the Client Supported Features characteristic */
                mClientSupportedFeaturesCharHandle = pService->aCharacteristics[i].value.handle;
            }
        }
    }
}

/*! *********************************************************************************
* \brief       Checks the advertising data looking for the UUIDs of the desired service.
*
* \param[in]   pData    Pointer to gapScannedDevice_t.
********************************************************************************** */
static bool_t BleApp_CheckScanEvent(gapScannedDevice_t* pData)
{
    uint32_t index = 0;
    uint8_t name[10];
    uint32_t nameLength = 0U;
    bool_t foundMatch = FALSE;

    while (index < pData->dataLength)
    {
        gapAdStructure_t adElement;

        adElement.length = pData->data[index];
        adElement.adType = (gapAdType_t)pData->data[index + 1U];
        adElement.aData = &pData->data[index + 2U];

        /* Search for Service A */
        if ((adElement.adType == gAdIncomplete16bitServiceList_c) ||
          (adElement.adType == gAdComplete16bitServiceList_c))
        {
            uint16_t uuid = gBleSig_AService_d;
            foundMatch = BluetoothLEHost_MatchDataInAdvElementList(&adElement, &uuid, (uint8_t)sizeof(uint16_t));
        }

        /* Search for Service B */
        if ((adElement.adType == gAdIncomplete16bitServiceList_c) ||
          (adElement.adType == gAdComplete16bitServiceList_c))
        {
            uint16_t uuid = gBleSig_BService_d;
            foundMatch = BluetoothLEHost_MatchDataInAdvElementList(&adElement, &uuid, (uint8_t)sizeof(uint16_t));
        }

        if ((adElement.adType == gAdShortenedLocalName_c) ||
          (adElement.adType == gAdCompleteLocalName_c))
        {
            nameLength = (uint32_t)MIN(adElement.length, 10U);
            FLib_MemCpy(name, adElement.aData, nameLength);
        }

        /* Move on to the next AD element type */
        index += (uint32_t)adElement.length + (uint8_t)sizeof(uint8_t);
    }

    if (foundMatch)
    {
        /* UI */
        (void)shell_write("\r\nFound device: \r\n");
        if (nameLength != 0U)
        {
            (void)shell_writeN((char*)name, nameLength - 1U);
            (void)SHELL_NEWLINE();
        }
        shell_writeHex(pData->aAddress, 6);
    }
    return foundMatch;
}

/*! *********************************************************************************
* \brief        State machine handler of the EATT Central application.
*
* \param[in]    peerDeviceId    peer identifier
* \param[in]    event           event to be handled
********************************************************************************** */
static void BleApp_StateMachineHandler(deviceId_t peerDeviceId, uint8_t event)
{
    switch (mPeerInformation.appState)
    {
        case mAppIdle_c:
        {
            BleApp_StateMachineHandler_IdleState(peerDeviceId, event);
        }
        break;

        case mAppExchangeMtu_c:
        {
            BleApp_StateMachineHandler_ExchangeMtuState(peerDeviceId, event);
        }
        break;

        case mAppServiceDisc_c:
        {
            BleApp_StateMachineHandler_ServiceDiscState(peerDeviceId, event);
        }
        break;
        
        case mAppWaitForPairingComplete_c:
        {
            BleApp_StateMachineHandler_WaitForPairingCompleteState(peerDeviceId, event);
        }
        break;

        case mAppReadDescriptorA_c:
        {
            BleApp_StateMachineHandler_ReadDescriptorAState(peerDeviceId, event);
        }
        break;

        case mAppReadDescriptorB_c:
        {
            BleApp_StateMachineHandler_ReadDescriptorBState(peerDeviceId, event);
        }
        break;

        case mAppEATTConn_c:
        {
            BleApp_StateMachineHandler_EATTConnState(peerDeviceId, event);
        }
        break;

        case mAppRunning_c:
        {
            BleApp_StateMachineHandler_RunningState(peerDeviceId, event);
        }
        break;

        default:
        {
            ; /* For MISRA Compliance */
        }
        break;
    }
}

/*! *********************************************************************************
* \brief        State machine handler of the Idle state.
*
* \param[in]    peerDeviceId    peer identifier
* \param[in]    event           event to be handled
********************************************************************************** */
static void BleApp_StateMachineHandler_IdleState(deviceId_t peerDeviceId, uint8_t event)
{
    if (event == mAppEvt_PeerConnected_c)
    {
#if gAppUseBonding_d
        if (mRestoringBondedLink &&
            (mPeerInformation.servAcfg.hServAReportCccd != gGattDbInvalidHandle_d) &&
            (mPeerInformation.servBcfg.hServBReportCccd != gGattDbInvalidHandle_d) )
        {
            /* Moving to mAppReadDescriptorA_c state */
            mPeerInformation.appState = mAppReadDescriptorA_c;
        }
        else
#endif
        {
            /* Moving to Exchange MTU State */
            mPeerInformation.appState = mAppExchangeMtu_c;
            (void)GattClient_ExchangeMtu(peerDeviceId, gAttMaxMtu_c);
        }
    }
}

/*! *********************************************************************************
* \brief        State machine handler of the Exchange MTU state.
*
* \param[in]    peerDeviceId    peer identifier
* \param[in]    event           event to be handled
********************************************************************************** */
static void BleApp_StateMachineHandler_ExchangeMtuState(deviceId_t peerDeviceId, uint8_t event)
{
    if (event == mAppEvt_GattProcComplete_c)
    {
        /* Moving to Primary Service Discovery State*/
        mPeerInformation.appState = mAppServiceDisc_c;

        /* Start Service Discovery*/
        if (gBleSuccess_c != BleServDisc_Start(peerDeviceId) )
        {
            (void)Gap_Disconnect(peerDeviceId);
        }
    }
    else if (event == mAppEvt_GattProcError_c)
    {
        (void)Gap_Disconnect(peerDeviceId);
    }
    else
    {
        ; /* For MISRA rule 15.7 compliance */
    }
}

/*! *********************************************************************************
* \brief        State machine handler of the Service Discovery state.
*
* \param[in]    peerDeviceId    peer identifier
* \param[in]    event           event to be handled
********************************************************************************** */
static void BleApp_StateMachineHandler_ServiceDiscState(deviceId_t peerDeviceId, uint8_t event)
{
    if (event == mAppEvt_ServiceDiscoveryComplete_c)
    {
        if (mbIsPairingComplete == TRUE)
        {
            /* Moving to mAppReadDescriptorA_c State*/
            BleApp_MoveToReadDescriptorAState(peerDeviceId);
        }
        else
        {
            /* Wait for Pairing Complete before moving to mAppReadDescriptorA_c state */
            mPeerInformation.appState = mAppWaitForPairingComplete_c;
        }
    }
    else if (event == mAppEvt_ServiceDiscoveryFailed_c)
    {
        (void)Gap_Disconnect(peerDeviceId);
    }
    else
    {
        ; /* For MISRA rule 15.7 compliance */
    }
}

/*! *********************************************************************************
* \brief        State machine handler of the Wait For Pairing Complete state.
*
* \param[in]    peerDeviceId    peer identifier
* \param[in]    event           event to be handled
********************************************************************************** */
static void BleApp_StateMachineHandler_WaitForPairingCompleteState
(
    deviceId_t peerDeviceId,
    uint8_t    event
)
{
    if (event == mAppEvt_PairingComplete_c)
    {
        /* Moving to mAppReadDescriptorA_c State*/
        BleApp_MoveToReadDescriptorAState(peerDeviceId);
    }
}

/*! *********************************************************************************
* \brief        Change peer app state to mAppReadDescriptorA_c and read
*               characteristic descriptor.
*
* \param[in]    peerDeviceId    peer identifier
* \param[in]    event           event to be handled
********************************************************************************** */
static void BleApp_MoveToReadDescriptorAState(deviceId_t peerDeviceId)
{
    mPeerInformation.appState = mAppReadDescriptorA_c;

    if (mPeerInformation.servAcfg.hServAReport != gGattDbInvalidHandle_d)
    {
        mpCharProcBuffer = MSG_Alloc(sizeof(gattAttribute_t) + gAttDefaultMtu_c);
        if (mpCharProcBuffer == NULL)
        {
            (void)Gap_Disconnect(peerDeviceId);
        }
        else
        {
            mpCharProcBuffer->handle = mPeerInformation.servAcfg.hServAReport;
            mpCharProcBuffer->paValue = (uint8_t*)(mpCharProcBuffer + 1);
            (void)shell_write("\r\n Read descriptor A ");
            (void)GattClient_ReadCharacteristicDescriptor(peerDeviceId, mpCharProcBuffer, 23);
        }
    }
}

/*! *********************************************************************************
* \brief        State machine handler of the Read Descriptor A state.
*
* \param[in]    peerDeviceId    peer identifier
* \param[in]    event           event to be handled
********************************************************************************** */
static void BleApp_StateMachineHandler_ReadDescriptorAState(deviceId_t peerDeviceId, uint8_t event)
{
    bool_t bEarlyReturn = FALSE;

    if( (event == mAppEvt_GattProcComplete_c) || (event == mAppEvt_PairingComplete_c))
    {
        if (mPeerInformation.servAcfg.hServAReportCccd != gGattDbInvalidHandle_d)
        {
            (void)shell_write("\r\n Write descriptor A ");
            /* Will move to the next state after the descriptor has been written */
            if( gBleSuccess_c != BleApp_ConfigureNotifications(peerDeviceId, mPeerInformation.servAcfg.hServAReportCccd) )
            {
                (void)Gap_Disconnect(peerDeviceId);
            }
        }
    }
    else if (event == mAppEvt_DescriptorWritten_c)
    {
        /* Moving to mAppReadDescriptorB_c State*/
        mPeerInformation.appState = mAppReadDescriptorB_c;

        if (mPeerInformation.servBcfg.hServBReport != gGattDbInvalidHandle_d)
        {
            if (mpCharProcBuffer == NULL)
            {
                mpCharProcBuffer = MSG_Alloc(sizeof(gattAttribute_t) + gAttDefaultMtu_c);
                if (mpCharProcBuffer == NULL)
                {
                    (void)Gap_Disconnect(peerDeviceId);
                    bEarlyReturn = TRUE;
                }
            }

            if( bEarlyReturn == FALSE )
            {
                mpCharProcBuffer->handle = mPeerInformation.servBcfg.hServBReport;
                mpCharProcBuffer->paValue = (uint8_t*)(mpCharProcBuffer + 1);
                (void)shell_write("\r\n Read descriptor B ");
                (void)GattClient_ReadCharacteristicDescriptor(peerDeviceId, mpCharProcBuffer, 23);
            }
        }
    }
    else
    {
        /* For MISRA Compliance */
    }
}

/*! *********************************************************************************
* \brief        State machine handler of the Read Descriptor B state.
*
* \param[in]    peerDeviceId    peer identifier
* \param[in]    event           event to be handled
********************************************************************************** */
static void BleApp_StateMachineHandler_ReadDescriptorBState(deviceId_t peerDeviceId, uint8_t event)
{
    if( (event == mAppEvt_GattProcComplete_c) || (event == mAppEvt_PairingComplete_c))
    {
        if (mPeerInformation.servBcfg.hServBReportCccd != gGattDbInvalidHandle_d)
        {
            (void)shell_write("\r\n Write descriptor B ");
            if( gBleSuccess_c != BleApp_ConfigureNotifications(peerDeviceId, mPeerInformation.servBcfg.hServBReportCccd) )
            {
                (void)Gap_Disconnect(peerDeviceId);
            }
        }
    }
    else if (event == mAppEvt_DescriptorWritten_c)
    {
        /* Moving to the mAppEATTConn_c State*/
        mPeerInformation.appState = mAppEATTConn_c;
        /* Signal own EATT support to the server by writing the Client Supported Features characteristic */
        uint8_t value = 2;
        uint16_t valueLength = (uint16_t)sizeof(value);
        gattCharacteristic_t pChar;
        pChar.cNumDescriptors = 0;
        pChar.aDescriptors = NULL;
        pChar.value.handle = mClientSupportedFeaturesCharHandle;
        pChar.value.uuidType = gBleUuidType16_c;
        pChar.value.uuid.uuid16 = gBleSig_GattClientSupportedFeatures_d;
        pChar.value.valueLength = 0;

        (void)GattClient_SimpleCharacteristicWrite(peerDeviceId, &pChar, valueLength, &value);
    }
    else
    {
        /* For MISRA Compliance */
    }
}

/*! *********************************************************************************
* \brief        State machine handler of the EATT Connection state.
*
* \param[in]    peerDeviceId    peer identifier
* \param[in]    event           event to be handled
********************************************************************************** */
static void BleApp_StateMachineHandler_EATTConnState(deviceId_t peerDeviceId, uint8_t event)
{
    /* Initiate EATT connection */
    (void)Gap_EattConnectionRequest(peerDeviceId, mLocalMtu, mCBearers, mInitialCredits, TRUE);

    /* Moving to the mAppRunning_c State*/
    mPeerInformation.appState = mAppRunning_c;
}

/*! *********************************************************************************
* \brief        State machine handler of the Running state.
*
* \param[in]    peerDeviceId    peer identifier
* \param[in]    event           event to be handled
********************************************************************************** */
static void BleApp_StateMachineHandler_RunningState(deviceId_t peerDeviceId, uint8_t event)
{
    if( event == mAppEvt_PeerConnectedEATT_c )
    {
        if( mpCharProcBuffer != NULL )
        {
            (void)MSG_Free(mpCharProcBuffer);
            mpCharProcBuffer = NULL;
        }

#if gAppUseBonding_d
        /* Write data in NVM */
        (void)Gap_SaveCustomPeerInformation(peerDeviceId, (void*) &mPeerInformation.servAcfg, 0, (uint16_t)sizeof(serviceAConfig_t));
        (void)Gap_SaveCustomPeerInformation(peerDeviceId, (void*) &mPeerInformation.servBcfg, (uint16_t)sizeof(serviceAConfig_t), (uint16_t)sizeof (serviceBConfig_t));
#endif
    }
}

/*! *********************************************************************************
* \brief        Write the CCCD of a characteristic.
*
* \param[in]    peerDeviceId  Peer identifier
* \param[in]    handle        Handle of the CCCD to be written
********************************************************************************** */
static bleResult_t BleApp_ConfigureNotifications(deviceId_t peerDeviceId, uint16_t handle)
{
    bleResult_t result = gBleSuccess_c;
    uint16_t value = gCccdNotification_c;

    if( mpCharProcBuffer == NULL )
    {
        mpCharProcBuffer = MSG_Alloc(sizeof(gattAttribute_t) + gAttDefaultMtu_c);
    }

    if( (mpCharProcBuffer != NULL) )
    {
        mpCharProcBuffer->handle = handle;
        mpCharProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
        mpCharProcBuffer->valueLength = 0;
        (void)GattClient_WriteCharacteristicDescriptor(peerDeviceId,
                                                       mpCharProcBuffer,
                                                       (uint16_t)sizeof(value), (void*)&value);
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
