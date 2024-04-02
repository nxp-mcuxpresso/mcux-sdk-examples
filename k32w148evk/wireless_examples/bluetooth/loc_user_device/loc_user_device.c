/*! *********************************************************************************
* \addtogroup Localization User Device application
* @{
********************************************************************************** */
/*! *********************************************************************************
* \file loc_user_device.c
*
* Copyright 2023-2024 NXP
*
* NXP Confidential Proprietary
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from NXP.
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
/* Framework / Drivers */
#include "EmbeddedTypes.h"
#include "fsl_component_button.h"
#include "fsl_component_timer_manager.h"
#include "fsl_component_panic.h"
#include "FunctionLib.h"
#include "fsl_component_mem_manager.h"
#include "fsl_adapter_reset.h"
#include "fsl_format.h"
#include "fsl_debug_console.h"
#include "app.h"
#include "board.h"
#include "fwk_platform_ble.h"
#include "NVM_Interface.h"
#include "fsl_shell.h"

/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"
#include "gatt_db_handles.h"

/* Profile / Services */
#include "ranging_interface.h"

#include "ble_conn_manager.h"
#include "ble_service_discovery.h"
#include "app_conn.h"
#include "loc_user_device.h"
#include "shell_loc_user_device.h"
#include "app_localization.h"
#include "channel_sounding.h"


/************************************************************************************
*************************************************************************************
* Extern functions
*************************************************************************************
************************************************************************************/
/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
typedef enum appEvent_tag
{
    mAppEvt_PeerConnected_c,
    mAppEvt_EncryptionChanged_c,
    mAppEvt_PairingComplete_c,
    mAppEvt_GattProcComplete_c,
    mAppEvt_GattProcError_c,
    mAppEvt_GattServerCallback_CCCDWrittenComplete_c,
    mAppEvt_PeerDisconnected_c
} appEvent_t;

typedef enum appState_tag
{
    mAppIdle_c,
    mAppExchangeMtu_c,
    mAppPairing,
    mAppEncryptLink_c,
    mAppLocalizationSetup_c,
    mAppRunning_c
} appState_t;

typedef struct appPeerInfo_tag
{
    deviceId_t          deviceId;
    bool_t              isBonded;
    appState_t          appState;
} appPeerInfo_t;

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
static appPeerInfo_t maPeerInformation[gAppMaxConnections_c];
static appScanningParams_t mAppScanParams = {
    &gScanParams,
    gGapDuplicateFilteringEnable_c,
    gGapScanContinuously_d,
    gGapScanPeriodicDisabled_d
};
static bool_t   mScanningOn = FALSE;
static bool_t   mFoundDeviceToConnect = FALSE;
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
static TIMER_MANAGER_HANDLE_DEFINE(mAppTimerId);
#endif
static uint16_t mCharMonitoredHandles[1] = { (uint16_t)value_ras_ctrl_point };
static bool_t mRestoringBondedLink = FALSE;
/* Number of the current procedure */
static uint16_t mProcedureCount = 0U;

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
/* Local RAS service configuration */
rasConfig_t mRasServiceConfig;

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
static button_status_t BleApp_HandleKeys0(void *pButtonHandle,
                                          button_callback_message_t *pMessage,
                                          void *pCallbackParam);
#endif /* (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0)) */
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1))
static button_status_t BleApp_HandleKeys1(void *pButtonHandle,
                                          button_callback_message_t *pMessage,
                                          void *pCallbackParam);
#endif /* (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1)) */
static void BluetoothLEHost_Initialized(void);
static void BleApp_GenericCallback (gapGenericEvent_t* pGenericEvent);
static void BleApp_StateMachineHandler(deviceId_t peerDeviceId, appEvent_t event);
static void BleApp_GattClientCallback(deviceId_t serverDeviceId,
                                      gattProcedureType_t procedureType,
                                      gattProcedureResult_t procedureResult,
                                      bleResult_t error);
static void BleApp_GattServerCallback(deviceId_t deviceId,
                                      gattServerEvent_t *pServerEvent);
static void BleApp_ScanningCallback(gapScanningEvent_t* pScanningEvent);
static void BleApp_ConnectionCallback(deviceId_t peerDeviceId,
                                      gapConnectionEvent_t* pConnectionEvent);
static bool_t CheckScanEventExtended(gapExtScannedDevice_t* pData);
static bool_t CheckScanEventLegacy(gapScannedDevice_t* pData);
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
static void ScanningTimeoutTimerCallback(void* pParam);
#endif
static void BleApp_CsEventHandler(void *pData, appCsEventType_t eventType);
#if defined(gAppCsTimeInfo_d) && (gAppCsTimeInfo_d == 1)
static void BleApp_PrintMeasurementResults(deviceId_t deviceId, localizationAlgoResult_t *pResult);
#endif /* defined(gAppCsTimeInfo_d) && (gAppCsTimeInfo_d == 1) */
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
    union Prompt_tag
    {
        const char * constPrompt;
        char * prompt;
    } shellPrompt;

    uint8_t mPeerId = 0;

    for (mPeerId = 0; mPeerId < (uint8_t)gAppMaxConnections_c; mPeerId++)
    {
        maPeerInformation[mPeerId].deviceId = gInvalidDeviceId_c;
    }

    LedStartFlashingAllLeds();
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[0], BleApp_HandleKeys0, NULL);

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[1], BleApp_HandleKeys1, NULL);
#endif /* (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1)) */
#endif /* (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0)) */

    /* Initialize Bluetooth Host Stack */
    BluetoothLEHost_SetGenericCallback(BleApp_GenericCallback);
    BluetoothLEHost_Init(BluetoothLEHost_Initialized);

    /* UI */
    shellPrompt.constPrompt = "Device>";
    AppShellInit(shellPrompt.prompt);

    /* Temporarily use minimum interval to speed up connection */
    gConnReqParams.connIntervalMin = gGapConnIntervalMin_d;
    gConnReqParams.connIntervalMax = gGapConnIntervalMin_d;

    /* Register CS callback and initialize localization */
    (void)AppLocalization_Init((csRoleType)gCsRole_c, BleApp_CsEventHandler, NULL);
}

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(void)
{
    if (!mScanningOn)
    {
        /* Start scanning */
        (void)BluetoothLEHost_StartScanning(&mAppScanParams, BleApp_ScanningCallback);
    }
}

/*! *********************************************************************************
* \brief    Disconnect from each device in the peer table on the BLE application.
*
********************************************************************************** */
void BleApp_Disconnect(void)
{
    uint8_t peerId;

    for (peerId = 0; peerId < (uint8_t)gAppMaxConnections_c; peerId++)
    {
        if (maPeerInformation[peerId].deviceId != gInvalidDeviceId_c)
        {
            (void)Gap_Disconnect(maPeerInformation[peerId].deviceId);
        }
    }
}

/*! *********************************************************************************
* \brief        Handles Shell_Factory Reset Command event.
********************************************************************************** */
void BleApp_FactoryReset(void)
{
    /* Erase NVM Datasets */
    NVM_Status_t status = NvFormat();
    if (status != gNVM_OK_c)
    {
         /* NvFormat exited with an error status */
         panic(0, (uint32_t)BleApp_FactoryReset, 0, 0);
    }

    /* Reset MCU */
    HAL_ResetMCU();
}

/*! *********************************************************************************
* \brief        Trigger CS distance measurement.
*
********************************************************************************** */
bleResult_t BleApp_TriggerCsDistanceMeasurement(deviceId_t deviceId)
{
    bleResult_t result = gBleSuccess_c;

    /* Reset data before starting a new procedure */
    AppLocalization_ResetPeer(deviceId, FALSE);

    if (maPeerInformation[deviceId].deviceId != gInvalidDeviceId_c)
    {
        result = AppLocalization_SetProcedureParameters(deviceId);
    }
    else
    {
        result = gBleInvalidParameter_c;
    }

    return result;
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/
/*! *********************************************************************************
 * \brief        Handle the main application state machine
 *
 * \param[in]    peerDeviceId       The remote device ID.
 * \param[in]    event              The application event.
 ********************************************************************************** */
static void BleApp_StateMachineHandler
(
    deviceId_t peerDeviceId,
    appEvent_t event
)
{
    if (maPeerInformation[peerDeviceId].deviceId != gInvalidDeviceId_c)
    {
        switch (maPeerInformation[peerDeviceId].appState)
        {
        case mAppIdle_c:
        {
            if (event == mAppEvt_PeerConnected_c)
            {
                shell_write("Connected\r\n");

                if (maPeerInformation[peerDeviceId].isBonded == TRUE)
                {
                    maPeerInformation[peerDeviceId].appState = mAppEncryptLink_c;
                }
                else
                {
                    maPeerInformation[peerDeviceId].appState = mAppPairing;
                }
            }
        }
        break;

        case mAppPairing:
        {
            if (event == mAppEvt_PairingComplete_c)
            {
                shell_write("Pairing complete\n\r");
                /* Moving to Exchange MTU State */
                maPeerInformation[peerDeviceId].appState = mAppExchangeMtu_c;
                (void)GattClient_ExchangeMtu(peerDeviceId, gAttMaxMtu_c);
            }
        }
        break;

        case mAppEncryptLink_c:
        {
            if (event == mAppEvt_EncryptionChanged_c)
            {
                shell_write("Link encrypted\n\r");
                /* Moving to Exchange MTU State */
                maPeerInformation[peerDeviceId].appState = mAppExchangeMtu_c;
                (void)GattClient_ExchangeMtu(peerDeviceId, gAttMaxMtu_c);
            }
        }
        break;

        case mAppExchangeMtu_c:
        {
            if (event == mAppEvt_GattProcComplete_c)
            {
                uint16_t attMtu = gAttDefaultMtu_c;
                /* Get new MTU value to register with RAS */
                shell_write("MTU Exchange complete\n\r");
                (void)Gatt_GetMtu(peerDeviceId, &attMtu);
                Ras_SetMtuValue(peerDeviceId, attMtu);
                /* Moving to Service Discovery State*/
                maPeerInformation[peerDeviceId].appState = mAppLocalizationSetup_c;
            }
            else
            {
                if (event == mAppEvt_GattProcError_c)
                {
                    (void)Gap_Disconnect(peerDeviceId);
                }
            }
        }
        break;

        case mAppLocalizationSetup_c:
        {
            /* RAS client subscribed */
            if (event == mAppEvt_GattServerCallback_CCCDWrittenComplete_c)
            {
#if defined(gCsRole_c) && (gCsRole_c == gCsInitiatorRole_c)
                bleResult_t status = gBleSuccess_c;
#endif /* gCsRole_c */

                /* Start CS localization */
                shell_write("Starting channel sounding configuration\r\n");
                maPeerInformation[peerDeviceId].appState = mAppRunning_c;
#if defined(gCsRole_c) && (gCsRole_c == gCsInitiatorRole_c)
                status = AppLocalization_Config(peerDeviceId);

                if (status != gBleSuccess_c)
                {
                    shell_write("Localization configuration failed\r\n");
                }
#endif /* gCsRole_c */
            }
        }
        break;

        case mAppRunning_c:
        {

        }
        break;

        default:
        {
            ; /* No action required */
        }
        break;
        }
    }
    /* Handle disconnect event in all application states. */
    if (event == mAppEvt_PeerDisconnected_c)
    {
        shell_write("Disconnected\n\r");
        shell_cmd_finished();
        maPeerInformation[peerDeviceId].appState = mAppIdle_c;
        maPeerInformation[peerDeviceId].deviceId = gInvalidDeviceId_c;
        AppLocalization_ResetPeer(peerDeviceId, TRUE);
    }
}

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
/*! *********************************************************************************
* \brief        Handler for the first key.
*
* \param[in]    pButtonHandle       Pointer to the button handle.
* \param[in]    pMessage            Pointer to the message.
* \param[in]    pCallbackParam      Pointer to the callback parameters.
********************************************************************************** */
static button_status_t BleApp_HandleKeys0
(
    void *pButtonHandle,
    button_callback_message_t *pMessage,
    void *pCallbackParam
)
{
    uint8_t mPeerId = 0;

    switch (pMessage->event)
    {
        case kBUTTON_EventOneClick:
        case kBUTTON_EventShortPress:
        {

            LedStopFlashingAllLeds();
            Led1Flashing();
            BleApp_Start();
            break;
        }

        case kBUTTON_EventLongPress:
        {
            for (mPeerId = 0; mPeerId < (uint8_t)gAppMaxConnections_c; mPeerId++)
            {
                if (maPeerInformation[mPeerId].deviceId != gInvalidDeviceId_c)
                {
                    (void)Gap_Disconnect(maPeerInformation[mPeerId].deviceId);
                }
            }

            break;
        }

        default:
        {
            ; /* No action required */
            break;
        }
    }
    return kStatus_BUTTON_Success;
}

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1))
/*! *********************************************************************************
* \brief        Handler for the second key.
*
* \param[in]    pButtonHandle       Pointer to the button handle.
* \param[in]    pMessage            Pointer to the message.
* \param[in]    pCallbackParam      Pointer to the callback parameters.
********************************************************************************** */
static button_status_t BleApp_HandleKeys1
(
    void *pButtonHandle,
    button_callback_message_t *pMessage,
    void *pCallbackParam
)
{
    switch (pMessage->event)
    {
        case kBUTTON_EventOneClick:
        case kBUTTON_EventShortPress:
        {
            uint8_t mPeerId = 0U;

            for (mPeerId = 0; mPeerId < (uint8_t)gAppMaxConnections_c; mPeerId++)
            {
                if (maPeerInformation[mPeerId].deviceId != gInvalidDeviceId_c)
                {
                    (void)BleApp_TriggerCsDistanceMeasurement(maPeerInformation[mPeerId].deviceId);
                    break;
                }
            }

            if (mPeerId == (uint8_t)gAppMaxConnections_c)
            {
                shell_write("No peer device available\r\n");
            }
            break;
        }

        case kBUTTON_EventLongPress:
            /* No action required */
            break;

        default:
        {
            /* No action required */
            break;
        }
    }

    return kStatus_BUTTON_Success;
}
#endif /* (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1)) */
#endif /* (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0)) */

/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
static void BleApp_GenericCallback(gapGenericEvent_t* pGenericEvent)
{
    /* Call BLE Conn Manager */
    BleConnManager_GenericEvent(pGenericEvent);
}

/*! *********************************************************************************
* \brief  This is the callback for Bluetooth LE Host stack initialization.
********************************************************************************** */
static void BluetoothLEHost_Initialized(void)
{
    uint8_t mPeerId = 0;
    bleResult_t status = gBleSuccess_c;

    /* Common GAP configuration */
    BleConnManager_GapCommonConfig();

    /* Register for callbacks*/
    status = App_RegisterGattClientProcedureCallback(BleApp_GattClientCallback);

    if (status == gBleSuccess_c)
    {
        status = App_RegisterGattServerCallback(BleApp_GattServerCallback);
    }

    if (status == gBleSuccess_c)
    {
        status = GattServer_RegisterHandlesForWriteNotifications(NumberOfElements(mCharMonitoredHandles), mCharMonitoredHandles);
    }

    if (status == gBleSuccess_c)
    {
        /* Initialize private variables */
        for (mPeerId = 0; mPeerId < (uint8_t)gAppMaxConnections_c; mPeerId++)
        {
            maPeerInformation[mPeerId].appState = mAppIdle_c;
            maPeerInformation[mPeerId].deviceId = gInvalidDeviceId_c;
        }
        mScanningOn = FALSE;
        mFoundDeviceToConnect = FALSE;

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
        /* Allocate scan timeout timer */
        TM_Open((timer_handle_t)mAppTimerId);
#endif

        /* Initialize RAS */
        mRasServiceConfig.serviceHandle = (uint16_t)service_ranging;
        mRasServiceConfig.controlPointHandle = (uint16_t)cccd_ras_ctrl_point;
        mRasServiceConfig.storedDataHandle = (uint16_t)value_ras_stored_data;
        mRasServiceConfig.realTimeDataHandle = (uint16_t)value_ras_real_time_data;
        mRasServiceConfig.dataReadyHandle = (uint16_t)value_ras_ranging_data_ready;
        mRasServiceConfig.dataOverwrittenHandle = (uint16_t)value_ras_ranging_data_overwritten;
        mRasServiceConfig.featuresHandle = (uint16_t)value_ras_feature;
        status = Ras_Start(&mRasServiceConfig);
    }

    if (status == gBleSuccess_c)
    {
        /* Continue CS initialization */
        status = AppLocalization_HostInitHandler();
    }

    if (status == gBleSuccess_c)
    {
        shell_write("\r\nLocalization User Device");
    }
    else
    {
        shell_write("Init error\r\n");
    }

    shell_cmd_finished();
}

/*! *********************************************************************************
* \brief        Process scanning events to search for the Ranging Service.
*               This function is called from the scanning callback.
*
* \param[in]    pData                   Pointer to gapScannedDevice_t.
*
* \return       TRUE if the scanned device implements the Ranging Service,
                FALSE otherwise
********************************************************************************** */
static bool_t CheckScanEventLegacy(gapScannedDevice_t* pData)
{
    uint32_t index = 0;
    bool_t foundMatch = FALSE;
    uint8_t deviceName[7] = "NXP_LOC";
    while (index < pData->dataLength)
    {
        gapAdStructure_t adElement;

        adElement.length = pData->data[index];
        adElement.adType = (gapAdType_t)pData->data[index + 1U];
        adElement.aData = &pData->data[index + 2U];

         /* Search for localization peripheral */
        if (adElement.adType == gAdShortenedLocalName_c)
        {
            foundMatch = BluetoothLEHost_MatchDataInAdvElementList(&adElement, &deviceName, 7);
        }

        /* Move on to the next AD element type */
        index += (uint32_t)adElement.length + sizeof(uint8_t);
    }

    if (foundMatch)
    {
        /* Update UI */
        shell_write("Legacy ADV: ");
        shell_writeHexLe(pData->aAddress, gcBleDeviceAddressSize_c);
        shell_write("\r\n");
    }
    return foundMatch;
}

/*! *********************************************************************************
* \brief        Process scanning events to search for the Ranging Service.
*               This function is called from the scanning callback.
*
* \param[in]    pData                   Pointer to gapExtScannedDevice_t.
*
* \return       TRUE if the scanned device implements the Ranging Service,
                FALSE otherwise
********************************************************************************** */
static bool_t CheckScanEventExtended(gapExtScannedDevice_t* pData)
{
    uint32_t index = 0;
    bool_t foundMatch = FALSE;
    uint8_t deviceName[7] = "NXP_LOC";
    while (index < pData->dataLength)
    {
        gapAdStructure_t adElement;

        adElement.length = pData->pData[index];
        adElement.adType = (gapAdType_t)pData->pData[index + 1U];
        adElement.aData = &pData->pData[index + 2U];

         /* Search for localization peripheral */
        if (adElement.adType == gAdShortenedLocalName_c)
        {
            foundMatch = BluetoothLEHost_MatchDataInAdvElementList(&adElement, &deviceName, 7);
        }

        /* Move on to the next AD element type */
        index += (uint32_t)adElement.length + sizeof(uint8_t);
    }

    if (foundMatch)
    {
        /* Update UI */
        shell_write("Extended LR ADV: ");
        shell_writeHexLe(pData->aAddress, gcBleDeviceAddressSize_c);
        shell_write("\r\n");
    }
    return foundMatch;
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
            /* Check if the scanned device implements the Ranging Service */
            if( FALSE == mFoundDeviceToConnect )
            {
                /* Only check the payload if we do not have a bond */
                if (pScanningEvent->eventData.scannedDevice.advertisingAddressResolved == FALSE)
                {
                    mFoundDeviceToConnect = CheckScanEventLegacy(&pScanningEvent->eventData.scannedDevice);
                }

                if (mFoundDeviceToConnect || (pScanningEvent->eventData.scannedDevice.advertisingAddressResolved == TRUE))
                {
                    mFoundDeviceToConnect = TRUE;
                    /* Set connection parameters and stop scanning. Connect on gScanStateChanged_c. */
                    gConnReqParams.peerAddressType = pScanningEvent->eventData.scannedDevice.addressType;
                    FLib_MemCpy(gConnReqParams.peerAddress,
                                pScanningEvent->eventData.scannedDevice.aAddress,
                                sizeof(bleDeviceAddress_t));

                    (void)Gap_StopScanning();
#if defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d)
                    gConnReqParams.usePeerIdentityAddress = pScanningEvent->eventData.scannedDevice.advertisingAddressResolved;
#endif
                }
            }
        }
        break;

        case gExtDeviceScanned_c:
        {
            /* Check if the scanned device implements the Ranging Service */
            if( FALSE == mFoundDeviceToConnect )
            {
                /* Only check the payload if we do not have a bond */
                if (pScanningEvent->eventData.extScannedDevice.advertisingAddressResolved == FALSE)
                {
                    mFoundDeviceToConnect = CheckScanEventExtended(&pScanningEvent->eventData.extScannedDevice);
                }

                if (mFoundDeviceToConnect || (pScanningEvent->eventData.extScannedDevice.advertisingAddressResolved == TRUE))
                {
                    mFoundDeviceToConnect = TRUE;
                    /* Set connection parameters and stop scanning. Connect on gScanStateChanged_c. */
                    gConnReqParams.peerAddressType = pScanningEvent->eventData.extScannedDevice.addressType;
                    FLib_MemCpy(gConnReqParams.peerAddress,
                                pScanningEvent->eventData.extScannedDevice.aAddress,
                                sizeof(bleDeviceAddress_t));

                    (void)Gap_StopScanning();
#if defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d)
                    gConnReqParams.usePeerIdentityAddress = pScanningEvent->eventData.extScannedDevice.advertisingAddressResolved;
#endif
                }
            }
        }
        break;

        case gScanStateChanged_c:
        {
            mScanningOn = !mScanningOn;

            /* Node starts scanning */
            if (mScanningOn)
            {
                mFoundDeviceToConnect = FALSE;

                shell_write("Scanning\r\n");

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
                /* Start scanning timer */
                (void)TM_InstallCallback((timer_handle_t)mAppTimerId, ScanningTimeoutTimerCallback, NULL);
                (void)TM_Start((timer_handle_t)mAppTimerId, (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSetSecondTimer, gScanningTime_c);
                Led1On();
#else
                LedStopFlashingAllLeds();
                Led1Flashing();
#endif /* #if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode) */
            }
            /* Node is not scanning */
            else
            {
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
                timer_status_t status = TM_Stop((timer_handle_t)mAppTimerId);
                if(status != kStatus_TimerSuccess)
                {
                    panic(0, (uint32_t)BleApp_ScanningCallback, 0, 0);
                }
#endif

                shell_write("Scan stopped\r\n");

                /* Connect with the previously scanned peer device */
                if (mFoundDeviceToConnect)
                {
                    shell_write("Connecting\r\n");
                    /* Temporarily use minimum interval to speed up connection */
                    gConnReqParams.connIntervalMin = gcConnectionInterval_c;
                    gConnReqParams.connIntervalMax = gcConnectionInterval_c;
                    (void)BluetoothLEHost_Connect(&gConnReqParams, BleApp_ConnectionCallback);
                }
                else
                {
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
                    Led1Off();
#else
                    LedStopFlashingAllLeds();
                    Led1Flashing();
                    Led2Flashing();
#endif
                    shell_cmd_finished();
                }
            }
        }
        break;

        case gScanCommandFailed_c:
        {
            ; /* No action required */
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
* \brief        Stop scanning after a given time (gScanningTime_c).
                Called on timer task.
*
* \param[in]    pParam              not used
********************************************************************************** */
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
static void ScanningTimeoutTimerCallback(void* pParam)
{
    /* Stop scanning */
    if (mScanningOn)
    {
        (void)Gap_StopScanning();
    }
}
#endif


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
            appLocalization_rangeCfg_t locConfig;
            /* Update UI */
            LedStopFlashingAllLeds();
            Led1On();
            maPeerInformation[peerDeviceId].deviceId = peerDeviceId;
            maPeerInformation[peerDeviceId].isBonded = FALSE;

            /* Read current CS config and update procedure repeat interval */
            AppLocalization_ReadConfig(&locConfig);

            /* Estimated algo durations (4 AP, 80 channels) for procedure repeat frequency calculation */
            uint32_t algoDurationMs = 0U;
#if defined(gAppUseRADEAlgorithm_d) && (gAppUseRADEAlgorithm_d == 1)
            algoDurationMs += 45U;
#endif /* gAppUseRADEAlgorithm_d */
#if defined(gAppUseCDEAlgorithm_d) && (gAppUseCDEAlgorithm_d == 1)
            algoDurationMs += 20U;
#endif /* gAppUseCDEAlgorithm_d */

            uint32_t procInterval = CS_PROC_DURATION_MS_MAX + POSTPROC_VERB_DURATION_MS_MIN + APPLICATION_OFFSET_DURATION_MS + algoDurationMs;
#if defined (gKW45B41ZLOC_d) && (gKW45B41ZLOC_d == 1U)
            procInterval +=  LOC_BOARD_PROC_REPEAT_DELAY;
#endif
            /* Convert ms to connection intervals */
            procInterval = 1 + (procInterval * 1000)/(pConnectionEvent->eventData.connectedEvent.connParameters.connInterval * 1250);
            locConfig.minPeriodBetweenProcedures = procInterval;
            locConfig.maxPeriodBetweenProcedures = procInterval;

            AppLocalization_WriteConfig(&locConfig);
            /* Set low power mode */
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
            (void)PWR_ChangeDeepSleepMode(gAppDeepSleepMode_c);
            PWR_AllowDeviceToSleep();
#endif
            (void)Gap_CheckIfBonded(peerDeviceId, &maPeerInformation[peerDeviceId].isBonded, NULL);

            if (maPeerInformation[peerDeviceId].isBonded)
            {
                mRestoringBondedLink = TRUE;
                /* Restored custom connection information. Encrypt link */
                (void)Gap_EncryptLink(peerDeviceId);
            }

            BleApp_StateMachineHandler(peerDeviceId, mAppEvt_PeerConnected_c);
        }
        break;

        case gConnEvtDisconnected_c:
        {
            Ras_Unsubscribe(peerDeviceId);
            BleApp_StateMachineHandler(peerDeviceId, mAppEvt_PeerDisconnected_c);
            /* UI */
            LedStartFlashingAllLeds();
        }
        break;

        case gConnEvtEncryptionChanged_c:
        {
            if( pConnectionEvent->eventData.encryptionChangedEvent.newEncryptionState )
            {
                if( mRestoringBondedLink )
                {
                    mRestoringBondedLink = FALSE;
                    BleApp_StateMachineHandler(peerDeviceId, mAppEvt_EncryptionChanged_c);
                }
            }
        }
        break;

        case gConnEvtPairingComplete_c:
        {
            if (pConnectionEvent->eventData.pairingCompleteEvent.pairingSuccessful)
            {
                BleApp_StateMachineHandler(peerDeviceId, mAppEvt_PairingComplete_c);
            }
        }
        break;

        default:
            ; /* No action required */
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
    if (procedureResult == gGattProcError_c)
    {
        attErrorCode_t attError = (attErrorCode_t)(uint8_t)(error);

        if (attError == gAttErrCodeInsufficientEncryption_c     ||
            attError == gAttErrCodeInsufficientAuthorization_c  ||
            attError == gAttErrCodeInsufficientAuthentication_c)
        {
            /* Start Pairing Procedure */
            (void)Gap_Pair(serverDeviceId, &gPairingParameters);
        }

        BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcError_c);
    }
    else
    {
        if (procedureResult == gGattProcSuccess_c)
        {
            BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcComplete_c);
        }
    }
}

/*! *********************************************************************************
 * \brief        Handles GATT server callback from host stack.
 *
 * \param[in]    deviceId           Client peer device ID.
 * \param[in]    pServerEvent       Pointer to gattServerEvent_t.
 ********************************************************************************** */
static void BleApp_GattServerCallback
(
    deviceId_t deviceId,
    gattServerEvent_t *pServerEvent
)
{
    switch (pServerEvent->eventType)
    {
        case gEvtCharacteristicCccdWritten_c:
        {
            uint8_t rasPreferenceValue = 0U;

            switch (pServerEvent->eventData.charCccdWrittenEvent.handle)
            {
                case cccd_ras_data_ready:
                {
                    shell_write("Ranging Client Subscribed\r\n");
                    BleApp_StateMachineHandler(deviceId, mAppEvt_GattServerCallback_CCCDWrittenComplete_c);
                    
                    if (pServerEvent->eventData.charCccdWrittenEvent.newCccd == gCccdIndication_c)
                    {
                        /* Signal preference for indications for Data Ready */
                        rasPreferenceValue |= BIT2;
                    }
                }
                break;
                
                case cccd_ras_ctrl_point:
                {
                    if (pServerEvent->eventData.charCccdWrittenEvent.newCccd == gCccdIndication_c)
                    {
                        /* Signal preference for indications for the RAS CP */
                        rasPreferenceValue |= BIT1;
                    }
                }
                break;
                
                case cccd_ras_stored_data:
                {
                    if (pServerEvent->eventData.charCccdWrittenEvent.newCccd == gCccdIndication_c)
                    {
                        /* Signal preference for indications for RAS On-Demand Data */
                        rasPreferenceValue |= BIT0;
                    }
                    
                    (void)Ras_Subscribe(deviceId, FALSE);
                }
                break;
                
                case cccd_ras_real_time_data:
                {
                    if (pServerEvent->eventData.charCccdWrittenEvent.newCccd == gCccdIndication_c)
                    {
                        /* Signal preference for indications for RAS On-Demand Data */
                        rasPreferenceValue |= BIT4;
                    }

                    (void)Ras_Subscribe(deviceId, TRUE);
                    shell_write("Ranging Client Subscribed\r\n");
                    BleApp_StateMachineHandler(deviceId, mAppEvt_GattServerCallback_CCCDWrittenComplete_c);
                }
                break;
                
                case cccd_ras_data_overwritten:
                {
                    if (pServerEvent->eventData.charCccdWrittenEvent.newCccd == gCccdIndication_c)
                    {
                        /* Signal preference for indications for Data Overwritten */
                        rasPreferenceValue |= BIT3;
                    }
                }
                break;
                
                default:
                {
                    /* Should not get here */
                    rasPreferenceValue = 0xFF;
                }
                break;
            }
            
            (void)Ras_SetDataSendPreference(deviceId, rasPreferenceValue);
        }
        break;

        case gEvtAttributeWrittenWithoutResponse_c:
        case gEvtAttributeWritten_c:
        {
            if (pServerEvent->eventData.attributeWrittenEvent.handle == (uint16_t)value_ras_ctrl_point)
            {
                rasControlPointReq_t *rasCtrlPointCmd = (rasControlPointReq_t*)pServerEvent->eventData.attributeWrittenEvent.aValue;
                (void)GattServer_SendAttributeWrittenStatus(deviceId,
                                                      pServerEvent->eventData.attributeWrittenEvent.handle,
                                                      (uint8_t)gAttErrCodeNoError_c);
                /* RAS control point characteristic was written */
                bleResult_t result = Ras_ControlPointHandler(deviceId,
                                                             &mRasServiceConfig,
                                                             pServerEvent->eventType,
                                                             &pServerEvent->eventData.attributeWrittenEvent);
#if defined(gAppCsTimeInfo_d) && (gAppCsTimeInfo_d == 1)
                if (rasCtrlPointCmd->cmdOpCode == combinedReportOpCode_c)
                {
                    localizationAlgoResult_t algoResult = {0};;

                    gCsTimeInfo.rasTransferEnd = TM_GetTimestamp();
                    algoResult.algorithm = 0U;
                    algoResult.csConfigDuration = gCsTimeInfo.csConfigEndTs - gCsTimeInfo.csConfigStartTs;
                    algoResult.csProcedureDuration = gCsTimeInfo.csDistMeasDuration;
                    algoResult.rasTransferDuration = gCsTimeInfo.rasTransferEnd - gCsTimeInfo.rasTransferStart;

                    BleApp_PrintMeasurementResults(deviceId,  &algoResult);
                }
#endif /* defined(gAppCsTimeInfo_d) && (gAppCsTimeInfo_d == 1) */

                if (result != gBleSuccess_c)
                {
                    shell_write("[");
                    shell_writeDec(deviceId);
                    shell_write("] RAS Error Received! Error code: ");
                    shell_writeDec(result);
                    shell_write(".\r\n");
                }

                if (rasCtrlPointCmd->cmdOpCode == ackProcDataOpCode_c)
                {
                    shell_write("RAS transfer completed for procedure index ");
                    shell_writeDec((mProcedureCount - 1U)); /* It was previously incremented at CS procedure completion */
                    shell_write(".\r\n");

                    if (mProcedureCount == mRangeSettings.maxNumProcedures)
                    {
                        shell_cmd_finished();
                    }
                }
            }
        }
        break;

        case gEvtHandleValueConfirmation_c:
        {
            /* Confirm indication received */
            AppLocalization_SetRealTimePreference(deviceId, FALSE);
            if (Ras_CheckTransferInProgress(deviceId))
            {
                if ((Ras_GetDataSendPreference(deviceId) & BIT0) != 0U)
                {
                    /* On-Demand data transfer in progress through indications */
                    Ras_SendStoredRangingDataIndication(deviceId, (uint16_t)value_ras_stored_data);
                }
            }
            
            if (Ras_CheckSegmentTransmInProgress(deviceId))
            {
                /* Segment retransmission in progress through indications */
                Ras_HandleGetRecordSegmentsIndications(deviceId);
            }
            
            if ((Ras_CheckRealTimeData(deviceId)) && ((Ras_GetDataSendPreference(deviceId) & BIT4) != 0U))
            {
                /* REal-Time data transfer in progress through indications */
                Ras_SendStoredRangingDataIndication(deviceId, (uint16_t)value_ras_stored_data);
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
* \brief  This is the callback for Bluetooth LE CS events
********************************************************************************** */
static void BleApp_CsEventHandler(void *pData, appCsEventType_t eventType)
{
    switch (eventType)
    {
        case gCsMetaEvent_c:
        {

        }
        break;

        case gCsCcEvent_c:
        {

        }
        break;

        case gCsSecurityEnabled_c:
        {
#if defined(gCsRole_c) && (gCsRole_c == gCsInitiatorRole_c)
            bleResult_t result = gBleSuccess_c;
#endif /* gCsRole_c */
            shell_write("CS security enabled.\r\n");
#if defined(gCsRole_c) && (gCsRole_c == gCsInitiatorRole_c)
            result = AppLocalization_SetProcedureParameters(*(deviceId_t*)pData);

            if (result != gBleSuccess_c)
            {
                shell_write("Set Procedure parameters failed.\r\n");
            }
#endif /* gCsRole_c */
        }
        break;

        case gConfigComplete_c:
        case gLocalConfigWritten_c:
        {
            bleResult_t result = gBleSuccess_c;

            shell_write("Localization config complete.\r\n");

            result = AppLocalization_SecurityEnable(*(deviceId_t*)pData);

            if (result != gBleSuccess_c)
            {
                shell_write("CS Security Enable failed.\r\n");
            }
        }
        break;

        case gSetProcParamsComplete_c:
        {
#if defined(gCsRole_c) && (gCsRole_c == gCsInitiatorRole_c)
            bleResult_t result = gBleSuccess_c;

            shell_write("Set Procedure parameters complete.\r\n");
            shell_write("Start distance measurement.\r\n");
            mProcedureCount = 0U;

            result = AppLocalization_StartMeasurement(*(deviceId_t*)pData);

            if (result != gBleSuccess_c)
            {
                shell_write("Distance measurement start failed.\r\n");
            }
#endif /* gCsRole_c */
        }
        break;

        case gDistanceMeastStarted_c:
        {
 #if defined(gCsRole_c) && (gCsRole_c == gCsReflectorRole_c)
            mProcedureCount = 0U;
            shell_write("Distance measurement started.\r\n");
#endif /* gCsRole_c */
        }
        break;

        case gLocalMeasurementComplete_c:
        {
            mProcedureCount++;
            shell_write("Distance measurement complete. Local data available\r\n");
        }
        break;

        case gDataOverwritten_c:
        {
            shell_write("\r\nSent Data Overwritten Indication!\r\n");
        }
        break;

        case gErrorEvent_c:
        {
            appLocalizationError_t *pError = (appLocalizationError_t*)pData;

            switch (*pError)
            {
                case gAppLclErrorRLSC_c:
                {
                    shell_write("Error occured! Source: csReadLocalSupportedCapabilities!\r\n");
                }
                break;

                case gAppLclUnexpectedCC_c:
                {
                    shell_write("Received an unexpected Config Complete Event!\r\n");
                }
                break;

                case gAppLclUnexpectedRRSCC_c:
                {
                    shell_write("Received an unexpected Read Remote Supported Capabilities Complete Event!\r\n");
                }
                break;

                case gAppLclUnexpectedPEC_c:
                {
                    shell_write("Received an unexpected Procedure Enable Complete Event!\r\n");
                }
                break;

                case gAppLclUnexpectedSRE_c:
                {
                    shell_write("Received an unexpected Subevent Result Event!\r\n");
                }
                break;

                case gAppLclUnexpectedSRCE_c:
                {
                    shell_write("Received an unexpected Subevent Result Continue Event!\r\n");
                }
                break;

                case gAppLclErrorRRSCCC_c:
                {
                    shell_write("Error occured! Source: readRemoteSupportedCapabilitiesComplete!\r\n");
                }
                break;

                case gAppLclErrorSEC_c:
                {
                    shell_write("Error occured! Source: securityEnableComplete!\r\n");
                }
                break;

                case gAppLclUnexpectedSPP_c:
                {
                    shell_write("Received an unexpected Set Procedure Parameters Command Status Event!\r\n");
                }
                break;

                case gAppLclInvalidDeviceId_c:
                {
                    shell_write("Received an invalid device Id!\r\n");
                }
                break;

                case gAppLclStartMeasurementFail_c:
                {
                    shell_write("Start measurement failed!\r\n");
                }
                break;

                case gAppLclSDSConfigError_c:
                {
                    shell_write("CS_SetDefaultSettings command failed!\r\n");
                }
                break;

                case gAppLclCCConfigError_c:
                {
                    shell_write("CS_CreateConfig command failed!\r\n");
                }
                break;

                case gAppLclRRSCError_c:
                {
                    shell_write("Error status received! csReadRemoteSupportedHadmCapabilities command status event!\r\n");
                }
                break;

                case gAppLclSEError_c:
                {
                    shell_write("Error status received! csSecurityEnable command status event!\r\n");
                }
                break;

                case gAppLclCCError_c:
                {
                    shell_write("Error status received! csCreateConfig command status event!\r\n");
                }
                break;

                case gAppLclProcStatusFailed_c:
                {
                    shell_write("Procedure done status error received!\r\n");
                }
                break;

                case gAppLclProcedureAborted_c:
                {
                    shell_write("All subsequent CS procedures aborted!.\r\n");
                }
                break;

                case gAppLclSubeventStatusFailed_c:
                {
                    shell_write("Subevent status failed!\r\n");
                }
                break;

                case gAppLclRasTransferFailed_c:
                {
                    shell_write("Received an error response from RAS server!.\r\n");
                }
                break;

                case gAppLclProcEndSubeventStatusFailed_c:
                {
                    shell_write("Procedure done, subevent status failed was received.\r\n");
                }
                break;

                case gAppLclRasSendIndicationFailed_c:
                {
                    shell_write("Error occured! Ras_SendDataReadyIndication call failed.\r\n");
                }
                break;

                case gAppLclNoSubeventMemoryAvailable_c:
                {
                    shell_write("No more memory available for a local subevent!\r\n");
                }
                break;

                case gAppLclErrorProcessingSubevent_c:
                {
                    shell_write("An error occured in the processing of subevent data!\r\n");
                }
                break;

                default:
                ; /* Do nothing */
                break;
            }
        }
        break;

        case gErrorSubeventAborted_c:
        {
            uint8_t abortReason = *((uint8_t*)pData);
            shell_write("Current CS subevent aborted! Abort Reason: ");
            switch (abortReason)
            {
                case gAppLclNoCsSync_c:
                {
                    shell_write("No CS_SYNC (mode0) received.\r\n");
                }
                break;

                case gAppLclScheduleConflict_c:
                {
                    shell_write("Scheduling conflicts or limited resources.\r\n");
                }
                break;

                case gAppLclTimePassed_c:
                {
                    shell_write("Time passed.\r\n");
                }
                break;

                case gAppLclInvalidArguments_c:
                {
                    shell_write("Invalid arguments.\r\n");
                }
                break;

                case gAppLclAborted_c:
                {
                    shell_write("Aborted.\r\n");
                }
                break;

                case gAppLclUnspecifiedReasons_c:
                {
                    shell_write("Unspecified reasons.\r\n");
                }
                break;

                default:
                {
                    shell_write("Unknown!\r\n");
                }
                break;
            }
        }
        break;

        case gErrorProcedureAborted_c:
        {
            uint8_t abortReason = *((uint8_t*)pData);
            shell_write("All subsequent CS procedures aborted! Abort Reason: ");
            switch (abortReason)
            {
                case gAppLclLocalHost_c:
                {
                    shell_write("Abort because of local Host or remote request.\r\n");
                }
                break;

                case gAppLclRequiredChannelNumber_c:
                {
                    shell_write("Abort because filtered channel map has less than 15 channels.\r\n");
                }
                break;

                case gAppLclChannelMapInstant_c:
                {
                    shell_write("Abort because the channel map update instant has passed.\r\n");
                }
                break;

                case gAppLclUnspecifiedReasons_c:
                {
                    shell_write("Abort because of unspecified reasons.\r\n");
                }
                break;

                default:
                {
                    shell_write("Unknown!\r\n");
                }
                break;
            }
        }
        break;

        default:
        {
        }
        break;
    }
}

#if defined(gAppCsTimeInfo_d) && (gAppCsTimeInfo_d == 1)
/*! *********************************************************************************
* \brief  This is the callback for displaying distance measurement results
********************************************************************************** */
static void BleApp_PrintMeasurementResults(deviceId_t deviceId, localizationAlgoResult_t *pResult)
{
        shell_write("Time information:");
        shell_write("\r\n");
        if (pResult->csConfigDuration != 0)
        {
            shell_write("CS Config: ");
            shell_writeDec(pResult->csConfigDuration/1000);
            shell_write("ms\r\n");
        }
        if (pResult->csProcedureDuration != 0)
        {
            shell_write("CS Procedure: ");
            shell_writeDec(pResult->csProcedureDuration/1000);
            shell_write("ms\r\n");
        }
        if (pResult->rasTransferDuration != 0)
        {
            shell_write("RAS transfer: ");
            shell_writeDec(pResult->rasTransferDuration/1000);
            shell_write("ms\r\n");
        }
        if (pResult->algoDuration != 0)
        {
            shell_write("Localization algorithm: ");
            shell_writeDec(pResult->algoDuration/1000);
            shell_write("ms\r\n");
        }
}
#endif /* defined(gAppCsTimeInfo_d) && (gAppCsTimeInfo_d == 1) */

/*! *********************************************************************************
* @}
********************************************************************************** */
