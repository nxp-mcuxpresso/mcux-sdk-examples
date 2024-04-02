/*! *********************************************************************************
* \addtogroup Localization Reader application
* @{
********************************************************************************** */
/*! *********************************************************************************
* \file loc_reader.c
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
#include "fsl_debug_console.h"
#include "fsl_format.h"
#include "app.h"
#include "board.h"
#include "fwk_platform_ble.h"
#include "NVM_Interface.h"
#include "RNG_Interface.h"

/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"
#include "gatt_db_handles.h"
#include "ble_conn_manager.h"
#include "ble_service_discovery.h"
#include "app_conn.h"
#include "loc_reader.h"
#include "shell_loc_reader.h"
#include "app_localization.h"

/************************************************************************************
*************************************************************************************
* Public memory declarations
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
    mAppEvt_ExchangeMtuComplete_c,
    mAppEvt_ServiceDiscoveryComplete_c,
    mAppEvt_ServiceDiscoveryFailed_c,
    mAppEvt_WriteCharacteristicDescriptorComplete_c,
    mAppEvt_ReadCharacteristicValueComplete_c,
    mAppEvt_GattProcError_c,
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
    rasConfig_t         rasConfigInfo;
}appPeerInfo_t;
/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
static appPeerInfo_t maPeerInformation[gAppMaxConnections_c];
/* Application timer*/
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
static TIMER_MANAGER_HANDLE_DEFINE(mAppTimerId);
#endif
static bool_t mAdvOn = FALSE;

/* Buffer used for Characteristic related procedures */
static gattAttribute_t      *mpCharProcBuffer = NULL;
static uint16_t mProcedureCount = 0x0U;

#if (defined(gAppUseRADEAlgorithm_d) && (gAppUseRADEAlgorithm_d == 1)) || \
    (defined(gAppUseCDEAlgorithm_d) && (gAppUseCDEAlgorithm_d == 1))
static float mPreviousDistance = (float)10.0;
#endif

static gattCharacteristic_t  mpRasCharacteristic;

static uint8_t mVerbosityLevel = 2U; /* default: all prints enabled */

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/
static void BleApp_StateMachineHandler(deviceId_t peerDeviceId, appEvent_t event);
static void BleApp_GenericCallback(gapGenericEvent_t* pGenericEvent);
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
static void BleApp_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent);
void BleApp_ConnectionCallback (deviceId_t peerDeviceId,
                                gapConnectionEvent_t* pConnectionEvent);
static void BleApp_GattServerCallback (deviceId_t deviceId,
                                       gattServerEvent_t* pServerEvent);
static void BleApp_GattClientCallback(deviceId_t serverDeviceId,
                                      gattProcedureType_t procedureType,
                                      gattProcedureResult_t procedureResult,
                                      bleResult_t error);
static void BleApp_GattNotificationCallback(deviceId_t serverDeviceId,
                                            uint16_t characteristicValueHandle,
                                            uint8_t* aValue,
                                            uint16_t valueLength);
static void BleApp_GattIndicationCallback(deviceId_t serverDeviceId,
                                          uint16_t characteristicValueHandle,
                                          uint8_t* aValue,
                                          uint16_t valueLength);
static void BleApp_StoreServiceHandles(deviceId_t peerDeviceId,
                                       gattService_t *pService);
static void BleApp_ServiceDiscoveryCallback(deviceId_t peerDeviceId,
                                            servDiscEvent_t* pEvent);

static void BleApp_CsEventHandler(void *pData, appCsEventType_t eventType);
static void BleApp_PrintMeasurementResults(deviceId_t deviceId, localizationAlgoResult_t *pResult);
/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/
/*! *********************************************************************************
* \brief    Initializes application specific functionality before the BLE stack init.
*
********************************************************************************** */
void BluetoothLEHost_AppInit(void)
{
    union Prompt_tag
    {
        const char * constPrompt;
        char * prompt;
    } shellPrompt;

    uint8_t mPeerId = 0;

    /* Initialize table with peer devices information  */
    for (mPeerId = 0; mPeerId < (uint8_t)gAppMaxConnections_c; mPeerId++)
    {
        maPeerInformation[mPeerId].deviceId = gInvalidDeviceId_c;
        maPeerInformation[mPeerId].appState = mAppIdle_c;
    }
    /* UI */
    LedStartFlashingAllLeds();
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[0],
                                 BleApp_HandleKeys0, NULL);
#endif /* (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0)) */
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[1],
                                 BleApp_HandleKeys1, NULL);
#endif /* (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1)) */
    /* Set generic callback */
    BluetoothLEHost_SetGenericCallback(BleApp_GenericCallback);
    /* Initialize Bluetooth Host Stack */
    BluetoothLEHost_Init(BluetoothLEHost_Initialized);
    /* UI */
    shellPrompt.constPrompt = "Reader>";
    AppShellInit(shellPrompt.prompt);
    /* Register CS callback and initialize localization */
    (void)AppLocalization_Init((csRoleType)gCsRole_c,
                               BleApp_CsEventHandler,
                               BleApp_PrintMeasurementResults);
}

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(void)
{
    /* Device is not already advertising */
    if (!mAdvOn)
    {
        /* UI update */
        Led1On();

        /* Set advertising parameters, advertising to start on gAdvertisingParametersSetupComplete_c */
        (void)BluetoothLEHost_StartExtAdvertising(&gAppAdvParams,
                                                  BleApp_AdvertisingCallback,
                                                  BleApp_ConnectionCallback);
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
    static uint16_t lastWrittenHandle = gGattDbInvalidHandle_d;

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
            }
        }
        break;

        case mAppExchangeMtu_c:
        {
            if (event == mAppEvt_ExchangeMtuComplete_c)
            {
                /* Start service discovery for Ranging Service */
                bleUuid_t rasUuid;
                /* Moving to Localization Setup State*/
                shell_write("MTU Exchange complete\n\r");
                maPeerInformation[peerDeviceId].appState = mAppLocalizationSetup_c;
                rasUuid.uuid16 = gBleSig_RangingService_d;
                (void)BleServDisc_FindService(peerDeviceId, gBleUuidType16_c, &rasUuid);
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
                if (event == mAppEvt_ServiceDiscoveryComplete_c)
                {
                    /* Read RAS features characteristic */
                    mpRasCharacteristic.value.handle = maPeerInformation[peerDeviceId].rasConfigInfo.featuresHandle;
                    mpRasCharacteristic.value.uuidType = gBleUuidType16_c;
                    mpRasCharacteristic.value.uuid.uuid16 = gBleSig_RasFeature_d;
                    mpRasCharacteristic.value.paValue = MEM_BufferAlloc(sizeof(uint32_t));
                    if (mpRasCharacteristic.value.paValue != NULL)
                    {
                        (void)GattClient_ReadCharacteristicValue(peerDeviceId,
                                                                &mpRasCharacteristic,
                                                                sizeof(uint32_t));
                    }
                }
                else if (event == mAppEvt_ReadCharacteristicValueComplete_c)
                {
                    if (mpRasCharacteristic.value.handle == maPeerInformation[peerDeviceId].rasConfigInfo.featuresHandle)
                    {
                        uint16_t value = gCccdIndication_c;
                        uint32_t rasFeatures = Utils_ExtractFourByteValue(mpRasCharacteristic.value.paValue);
                        AppLocalization_SetRasSupportedFeatures(peerDeviceId, rasFeatures);

                        if( mpCharProcBuffer == NULL )
                        {
                            mpCharProcBuffer = MEM_BufferAlloc(sizeof(gattAttribute_t) + gAttDefaultMtu_c);
                        }

                        if( mpCharProcBuffer != NULL )
                        {
#if defined(gAppRealTimeDataTransfer_c) && (gAppRealTimeDataTransfer_c == 1U)
                            /* Enable real-time data transfer */
                            lastWrittenHandle = (uint16_t)(maPeerInformation[peerDeviceId].rasConfigInfo.realTimeDataHandle + 1U);
                            AppLocalization_SetRealTimePreference(peerDeviceId, TRUE);
#else
                            /* Write the RAS-CP */
                            lastWrittenHandle = (uint16_t)(maPeerInformation[peerDeviceId].rasConfigInfo.controlPointHandle + 1U);
#endif

                            mpCharProcBuffer->handle = lastWrittenHandle;
                            mpCharProcBuffer->uuidType = gBleUuidType16_c;
                            mpCharProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
                            mpCharProcBuffer->valueLength = 1U;
                            bleResult_t result = GattClient_WriteCharacteristicDescriptor(peerDeviceId,
                                                                                          mpCharProcBuffer,
                                                                                          (uint16_t)sizeof(value),
                                                                                          (void*)&value);
                        }
                    }
                }
                else if (event == mAppEvt_WriteCharacteristicDescriptorComplete_c)
                {
                    uint16_t value = gCccdNotification_c;
                    bleResult_t result;

                    if( mpCharProcBuffer == NULL )
                    {
                        mpCharProcBuffer = MEM_BufferAlloc(sizeof(gattAttribute_t) + gAttDefaultMtu_c);
                    }

                    if( mpCharProcBuffer != NULL )
                    {
                        if (lastWrittenHandle ==
                            (maPeerInformation[peerDeviceId].rasConfigInfo.controlPointHandle + 1U))
                        {
                            /* Enable notifications for Stored Ranging Data Characteristic */
                            lastWrittenHandle = (uint16_t)(maPeerInformation[peerDeviceId].rasConfigInfo.storedDataHandle + 1U);
                        }
                        else if (lastWrittenHandle ==
                                    (maPeerInformation[peerDeviceId].rasConfigInfo.storedDataHandle + 1U))
                        {
                            /* Enable notifications for Data Ready  Characteristic */
                            lastWrittenHandle = (uint16_t)(maPeerInformation[peerDeviceId].rasConfigInfo.dataReadyHandle + 1U);
                        }
                        else
                        {
                            /* Enable notifications for Data Overwritten Characteristic */
                            lastWrittenHandle = (uint16_t)(maPeerInformation[peerDeviceId].rasConfigInfo.dataOverwrittenHandle + 1U);
                        }
                        mpCharProcBuffer->handle = lastWrittenHandle;
                        mpCharProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
                        mpCharProcBuffer->uuidType = gBleUuidType16_c;
                        mpCharProcBuffer->valueLength = 1U;
                        result = GattClient_WriteCharacteristicDescriptor(peerDeviceId,
                                                                          mpCharProcBuffer,
                                                                          (uint16_t)sizeof(value),
                                                                          (void*)&value);

                        if ((result == gBleSuccess_c) && ((lastWrittenHandle ==
                            (maPeerInformation[peerDeviceId].rasConfigInfo.dataOverwrittenHandle + 1U))
                            || (lastWrittenHandle == (maPeerInformation[peerDeviceId].rasConfigInfo.realTimeDataHandle + 1U))))
                        {
                            /* Begin localization procedure */
                            maPeerInformation[peerDeviceId].appState = mAppRunning_c;
#if defined(gCsRole_c) && (gCsRole_c == gCsInitiatorRole_c)
                            result = AppLocalization_Config(peerDeviceId);

                            if (result != gBleSuccess_c)
                            {
                                shell_write("Localization configuration failed !\r\n");
                            }
#endif /* gCsRole_c */
                        }
                    }
                }
                else if (event == mAppEvt_ServiceDiscoveryFailed_c)
                {
                    (void)Gap_Disconnect(peerDeviceId);
                }
                else
                {
                    /* For MISRA compliance */
                }
            }
            break;

        case mAppRunning_c:
        {
            if (event == mAppEvt_WriteCharacteristicDescriptorComplete_c)
            {
                shell_write("Channel Sounding configuration complete\n\r");
                (void)MEM_BufferFree(mpCharProcBuffer);
                mpCharProcBuffer = NULL;
            }
            else
            {
                /* ignore other event types */
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
    /* Handle disconnect event in all application states. */
    if (event == mAppEvt_PeerDisconnected_c)
    {
        shell_write("Disconnected\n\r");
        shell_cmd_finished();
        maPeerInformation[peerDeviceId].deviceId = gInvalidDeviceId_c;
        maPeerInformation[peerDeviceId].appState = mAppIdle_c;
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
 * \brief        Configures BLE Stack after initialization
 *
 ********************************************************************************** */
static void BluetoothLEHost_Initialized(void)
{
    bleResult_t status = gBleSuccess_c;
    /* Common GAP configuration */
    BleConnManager_GapCommonConfig();

    /* Register for callbacks*/
    status = App_RegisterGattServerCallback(BleApp_GattServerCallback);

    if (status == gBleSuccess_c)
    {
        status = App_RegisterGattClientProcedureCallback(BleApp_GattClientCallback);
    }

    if (status == gBleSuccess_c)
    {
        status = App_RegisterGattClientNotificationCallback(BleApp_GattNotificationCallback);
    }

    if (status == gBleSuccess_c)
    {
        status = App_RegisterGattClientIndicationCallback(BleApp_GattIndicationCallback);
    }

    if (status == gBleSuccess_c)
    {
        BleServDisc_RegisterCallback(BleApp_ServiceDiscoveryCallback);

        /* Allocate application timers */
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
        (void)TM_Open(mAppTimerId);
#endif
        status = AppLocalization_HostInitHandler();
    }

    if (status == gBleSuccess_c)
    {
        shell_write("\r\nLocalization Reader");
    }
    else
    {
        shell_write("\r\nInit error");
    }

    shell_cmd_finished();
}

/*! *********************************************************************************
* \brief        Handles BLE Advertising callback from host stack.
*
* \param[in]    pAdvertisingEvent    Pointer to gapAdvertisingEvent_t.
********************************************************************************** */
static void BleApp_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent)
{
    switch (pAdvertisingEvent->eventType)
    {
        case gExtAdvertisingStateChanged_c:
        {
            if (mAdvOn == FALSE)
            {
                mAdvOn = !mAdvOn;
                shell_write("Advertising started\r\n");
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
                /* Start advertising timer */
                (void)TM_InstallCallback((timer_handle_t)mAppTimerId, AdvertisingTimerCallback, NULL);
                (void)TM_Start((timer_handle_t)mAppTimerId, (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSetSecondTimer, TmSecondsToMilliseconds(gAdvTime_c));
                 Led1On();
#else
                 /* UI */
                 LedStopFlashingAllLeds();
                 Led1Flashing();
#endif /* #if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode) */
            }
            else
            {
                mAdvOn = !mAdvOn;
                shell_write("Advertising stopped\r\n");
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
                timer_status_t status = TM_Stop((timer_handle_t)mAppTimerId);
                if(status != kStatus_TimerSuccess)
                {
                    panic(0, (uint32_t)BleApp_AdvertisingCallback, 0, 0);
                }
                Led1Off();
#else
                /* UI */
                LedStopFlashingAllLeds();
                Led1Flashing();
                Led2Flashing();

#endif /* #if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode) */
            }
        }
        break;

        case gAdvertisingCommandFailed_c:
        {
            /* Panic UI */
            Led2On();
            panic(0,0,0,0);
        }
        break;

        case gAdvertisingSetTerminated_c:
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

/*! *********************************************************************************
* \brief        Handles BLE Connection callback from host stack.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    pConnectionEvent    Pointer to gapConnectionEvent_t.
********************************************************************************** */
void BleApp_ConnectionCallback (deviceId_t peerDeviceId, gapConnectionEvent_t* pConnectionEvent)
{
    /* Connection Manager to handle Host Stack interactions */
    BleConnManager_GapPeripheralEvent(peerDeviceId, pConnectionEvent);

    switch (pConnectionEvent->eventType)
    {
        case gConnEvtConnected_c:
        {
            appLocalization_rangeCfg_t locConfig;

            /* Advertising stops when connected */
            mAdvOn = FALSE;
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
            (void)TM_Stop((timer_handle_t)mAppTimerId);
#endif
            maPeerInformation[peerDeviceId].deviceId = peerDeviceId;
            maPeerInformation[peerDeviceId].isBonded = FALSE;
            (void)Gap_CheckIfBonded(peerDeviceId, &maPeerInformation[peerDeviceId].isBonded, NULL);
            BleApp_StateMachineHandler(peerDeviceId, mAppEvt_PeerConnected_c);
            /* UI */
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
            PWR_AllowDeviceToSleep();
#else
            LedStopFlashingAllLeds();
#endif

            /* UI */
            Led1On();

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
#if defined(gAppCsTimeInfo_d) && (gAppCsTimeInfo_d == 1)
            AppLocalization_TimeInfoSetConnInterval(pConnectionEvent->eventData.connectedEvent.connParameters.connInterval);
#endif /* defined(gAppCsTimeInfo_d) && (gAppCsTimeInfo_d == 1) */

        }
        break;

        case gConnEvtDisconnected_c:
        {
            BleApp_StateMachineHandler(peerDeviceId, mAppEvt_PeerDisconnected_c);

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
            /* UI */
            Led1Off();
#else
            LedStartFlashingAllLeds();
#endif
        }
        break;

        case gConnEvtPairingComplete_c:
        {
            /* Notify state machine handler on pairing complete */
            if (pConnectionEvent->eventData.pairingCompleteEvent.pairingSuccessful)
            {
                BleApp_StateMachineHandler(maPeerInformation[peerDeviceId].deviceId, mAppEvt_PairingComplete_c);
            }
        }
        break;

        case gConnEvtEncryptionChanged_c:
        {
            BleApp_StateMachineHandler(peerDeviceId, mAppEvt_EncryptionChanged_c);
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
* \brief        Handles GATT server callback from host stack.
*
* \param[in]    deviceId        Peer device ID.
* \param[in]    pServerEvent    Pointer to gattServerEvent_t.
********************************************************************************** */
static void BleApp_GattServerCallback (deviceId_t deviceId, gattServerEvent_t* pServerEvent)
{
    switch (pServerEvent->eventType)
    {
        case gEvtMtuChanged_c:
        {
            BleApp_StateMachineHandler(deviceId, mAppEvt_ExchangeMtuComplete_c);
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
static void BleApp_GattClientCallback
(
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
            switch(procedureType)
            {
                case gGattProcWriteCharacteristicDescriptor_c:
                {
                    BleApp_StateMachineHandler(serverDeviceId, mAppEvt_WriteCharacteristicDescriptorComplete_c);
                }
                break;

                case gGattProcReadCharacteristicValue_c:
                case gGattProcReadUsingCharacteristicUuid_c:
                {
                    BleApp_StateMachineHandler(serverDeviceId, mAppEvt_ReadCharacteristicValueComplete_c);
                }
                break;

                default:
                {
                    ; /* No action required */
                }
                break;
            }
        }
    }

    /* Signal Service Discovery Module */
    BleServDisc_SignalGattClientEvent(serverDeviceId, procedureType, procedureResult, error);
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
    if ((characteristicValueHandle ==  maPeerInformation[serverDeviceId].rasConfigInfo.storedDataHandle) ||
        (characteristicValueHandle ==  maPeerInformation[serverDeviceId].rasConfigInfo.realTimeDataHandle))
    {
        /* Received localization data from peer */
        (void)AppLocalization_StorePeerMeasurementData(serverDeviceId, aValue, valueLength);
    }

    if (characteristicValueHandle == maPeerInformation[serverDeviceId].rasConfigInfo.controlPointHandle + 1U)
    {
        /* Received a command response from peer */
        (void)AppLocalization_ProcessRasCPRsp(serverDeviceId, aValue, valueLength);
    }

    if (characteristicValueHandle == maPeerInformation[serverDeviceId].rasConfigInfo.dataReadyHandle)
    {
        /* Received an indication from peer for the RAS control point characteristic */
        (void)AppLocalization_ProcessRasDataReadyIndications(serverDeviceId, aValue, valueLength);
    }

    if (characteristicValueHandle == maPeerInformation[serverDeviceId].rasConfigInfo.dataOverwrittenHandle)
    {
        /* Received an indication from peer for the RAS Data Overwritten characteristic */
        (void)AppLocalization_ProcessRasDataOverwrittenIndications(serverDeviceId, aValue, valueLength);
    }
}

/*! *********************************************************************************
* \brief        Handles GATT client indication callback from host stack.
*
* \param[in]    serverDeviceId              GATT Server device ID.
* \param[in]    characteristicValueHandle   Handle.
* \param[in]    aValue                      Pointer to value.
* \param[in]    valueLength                 Value length.
********************************************************************************** */
static void BleApp_GattIndicationCallback
(
    deviceId_t  serverDeviceId,
    uint16_t    characteristicValueHandle,
    uint8_t*    aValue,
    uint16_t    valueLength
)
{
    if (characteristicValueHandle == maPeerInformation[serverDeviceId].rasConfigInfo.controlPointHandle + 1U)
    {
        /* Received an indication from peer for the RAS control point characteristic */
        (void)AppLocalization_ProcessRasCPRsp(serverDeviceId, aValue, valueLength);
    }

    if (characteristicValueHandle == maPeerInformation[serverDeviceId].rasConfigInfo.dataReadyHandle)
    {
        /* Received an indication from peer for the RAS control point characteristic */
        (void)AppLocalization_ProcessRasDataReadyIndications(serverDeviceId, aValue, valueLength);
    }

    if (characteristicValueHandle == maPeerInformation[serverDeviceId].rasConfigInfo.dataOverwrittenHandle)
    {
        /* Received an indication from peer for the RAS Data Overwritten characteristic */
        (void)AppLocalization_ProcessRasDataOverwrittenIndications(serverDeviceId, aValue, valueLength);
    }

    if ((characteristicValueHandle ==  maPeerInformation[serverDeviceId].rasConfigInfo.storedDataHandle) ||
        (characteristicValueHandle ==  maPeerInformation[serverDeviceId].rasConfigInfo.realTimeDataHandle))
    {
        /* Received localization data from peer */
        (void)AppLocalization_StorePeerMeasurementData(serverDeviceId, aValue, valueLength);
    }
}

/*! *********************************************************************************
* \brief        Handles discovered services.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    pEvent              Pointer to servDiscEvent_t.
********************************************************************************** */
static void BleApp_ServiceDiscoveryCallback(deviceId_t peerDeviceId, servDiscEvent_t* pEvent)
{
    switch(pEvent->eventType)
    {
        /* Store the discovered handles for later use. */
        case gServiceDiscovered_c:
        {
            BleApp_StoreServiceHandles(peerDeviceId, pEvent->eventData.pService);
        }
        break;

        /* Service discovery has finished, run the state machine. */
        case gDiscoveryFinished_c:
        {
            if (pEvent->eventData.success)
            {
                BleApp_StateMachineHandler(peerDeviceId, mAppEvt_ServiceDiscoveryComplete_c);
            }
            else
            {
                BleApp_StateMachineHandler(peerDeviceId, mAppEvt_ServiceDiscoveryFailed_c);
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
 * \brief        Stores handles used by the application.
 *
 * \param[in]    pService    Pointer to gattService_t.
 ********************************************************************************** */
static void BleApp_StoreServiceHandles
(
    deviceId_t peerDeviceId,
    gattService_t *pService
)
{
   if ((pService->uuidType == gBleUuidType16_c) &&
       (pService->uuid.uuid16 == gBleSig_RangingService_d))
   {
        /* Found Wireless Ranging Service */
        maPeerInformation[peerDeviceId].rasConfigInfo.serviceHandle = pService->startHandle;

        for (uint8_t i = 0; i < pService->cNumCharacteristics; i++)
        {
            if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType16_c) &&
                (pService->aCharacteristics[i].value.uuid.uuid16 == gBleSig_RasControlPoint_d))
            {
                /* Found RAS Control Point Characteristic */
                maPeerInformation[peerDeviceId].rasConfigInfo.controlPointHandle =
                pService->aCharacteristics[i].value.handle;
                /* Register value with localization component */
                AppLocalization_SetRasControlPointHandle(pService->aCharacteristics[i].value.handle);
            }

            if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType16_c) &&
                (pService->aCharacteristics[i].value.uuid.uuid16 == gBleSig_RasProcDataReady_d))
            {
                /* Found RAS Ranging Data Ready Characteristic */
                maPeerInformation[peerDeviceId].rasConfigInfo.dataReadyHandle =
                pService->aCharacteristics[i].value.handle;
            }

            if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType16_c) &&
                (pService->aCharacteristics[i].value.uuid.uuid16 == gBleSig_RasOnDemandProcData_d))
            {
                /* Found RAS Stored Ranging Data Characteristic */
                maPeerInformation[peerDeviceId].rasConfigInfo.storedDataHandle =
                pService->aCharacteristics[i].value.handle;
            }

            if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType16_c) &&
                (pService->aCharacteristics[i].value.uuid.uuid16 == gBleSig_RasRealTimeProcData_d))
            {
                /* Found RAS Stored Ranging Data Characteristic */
                maPeerInformation[peerDeviceId].rasConfigInfo.realTimeDataHandle =
                pService->aCharacteristics[i].value.handle;
            }

            if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType16_c) &&
                (pService->aCharacteristics[i].value.uuid.uuid16 == gBleSig_RasprocDataOverwritten_d))
            {
                /* Found RAS Procedure Data Overwritten Characteristic */
                maPeerInformation[peerDeviceId].rasConfigInfo.dataOverwrittenHandle =
                pService->aCharacteristics[i].value.handle;
            }

            if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType16_c) &&
                (pService->aCharacteristics[i].value.uuid.uuid16 == gBleSig_RasFeature_d))
            {
                /* Found RAS Features Characteristic */
                maPeerInformation[peerDeviceId].rasConfigInfo.featuresHandle =
                pService->aCharacteristics[i].value.handle;
            }
        }
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

        case gCsStatusEvent_c:
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
        {
            shell_write("Localization config complete.\r\n");
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
            shell_write("Distance measurement complete. Local data available.\r\n");
        }
        break;

        case gDataOverwritten_c:
        {
            shell_write("Received Data Overwritten Indication! Clearing local data....\r\n");
        }
        break;

        case gCombinedReportsFailed_c:
        {
            shell_write("\r\nCombined Reports command failed!\r\n");
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

                case gAppLclUnexpectedSDS_c:
                {
                    shell_write("Received an unexpected Set Default Settings Event!\r\n");
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

                case gAppLclInvalidDeviceId_c:
                {
                    shell_write("Received an invalid device Id!\r\n");
                }

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

                case gAppLclAlgoNotRun_c:
                {
                    shell_write("\r\nAlgorithm did not run, procedure likely failed on peer.\r\n");
                }
                break;

                case gAppLclStartMeasurementFail_c:
                {
                    shell_write("Start measurement failed!\r\n");
                }
                break;

                case gAppLclProcStatusFailed_c:
                {
                    shell_write("Procedure done status error received!\r\n");
                }
                break;

                case gAppLclProcedureAborted_c:
                {
                    shell_write("All subsequent CS procedures aborted!\r\n");
                }
                break;

                case gAppLclRasTransferFailed_c:
                {
                    shell_write("RAS - Received an error response from RAS server!\r\n");
                }
                break;

                case gAppLclInvalidProcCounter_c:
                {
                    shell_write("RAS - Received an invalid procedure index!\r\n");
                }
                break;

		        case gAppLclInvalidProcIndex_c:
                {
                    shell_write("RAS - Received a data ready indication for a procedure index different from the local one!\r\n");
                }
                break;

                case gAppLclInvalidSegmentCounter_c:
                {
                    shell_write("RAS - Received an invalid segment counter in data notification!\r\n");
                }
                break;

                case gAppLclSubeventStatusFailed_c:
                {
                    shell_write("Subevent status failed!\r\n");
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
                {
                    shell_write("Unknown error!\r\n");
                }
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
            ; /* No action required */
        }
        break;
    }
}

/*! *********************************************************************************
* \brief  This is the callback for displaying distance measurement results
********************************************************************************** */
static void BleApp_PrintMeasurementResults(deviceId_t deviceId, localizationAlgoResult_t *pResult)
{
#if ((defined(gAppUseRADEAlgorithm_d) && (gAppUseRADEAlgorithm_d == 1)) || \
    (defined(gAppUseCDEAlgorithm_d) && (gAppUseCDEAlgorithm_d == 1)))
    bool_t bUIUpdated = FALSE;
#endif /* ((defined(gAppUseRADEAlgorithm_d) && (gAppUseRADEAlgorithm_d == 1)) || \
            (defined(gAppUseCDEAlgorithm_d) && (gAppUseCDEAlgorithm_d == 1))) */

    if ((mVerbosityLevel != 0U) || (mProcedureCount == mRangeSettings.maxNumProcedures))
    {
#if defined(gAppUseRADEAlgorithm_d) && (gAppUseRADEAlgorithm_d == 1)
        if ((pResult->algorithm & eMciqAlgoEmbedRADE) != 0U)
        {
            shell_write("\r\n[");
            shell_writeDec((uint8_t)deviceId);

            if (pResult->radeError != 0U)
            {
                 shell_write("] RADE Error: ");
                 shell_writeDec(pResult->radeError);
                 shell_write("!\r\n");
            }
            else if (pResult->resultRADE.dqiIntegerPart == 0U)
            {
               shell_write("] Low Quality data for RADE! Quality indicator is 0! \n\r");
            }
            else
            {
                shell_write("] Distance (RADE): ");
                /* Display the integer part of the distance in meters. */
                shell_writeDec(pResult->resultRADE.distanceIntegerPart);
                shell_write(".");
                /* Display the decimal part of the distance in meters. */
                shell_writeDec(pResult->resultRADE.distanceDecimalPart);
                shell_write(" m   ");

                shell_write("Quality: ");
                shell_writeDec(pResult->resultRADE.dqiIntegerPart);
                shell_write(".");
                shell_writeDec(pResult->resultRADE.dqiDecimalPart);
                shell_write("%%\r\n");
                /* Flash LEDs if distance is less than 1m. */
                if (bUIUpdated == FALSE)
                {
                    if ((mPreviousDistance <= (float)1) && (pResult->resultRADE.distanceInMeters <= (float)1))
                    {
                        LedStartFlashingAllLeds();
                    }
                    else if ((mPreviousDistance > (float)1) && (pResult->resultRADE.distanceInMeters > (float)1))
                    {
                        LedStopFlashingAllLeds();
                        Led1On();
                    }
                    mPreviousDistance = pResult->resultRADE.distanceInMeters;
                    bUIUpdated = TRUE;
                }
            }
        }
#endif /* gAppUseRADEAlgorithm_d */

#if defined(gAppUseCDEAlgorithm_d) && (gAppUseCDEAlgorithm_d == 1)
        if ((pResult->algorithm & eMciqAlgoEmbedCDE) != 0U)
        {
            shell_write("\r\n[");
            shell_writeDec((uint8_t)deviceId);
            shell_write("] Distance (CDE): ");
            /* Display the integer part of the distance in meters. */
            shell_writeDec(pResult->resultCDE.distanceIntegerPart);
            shell_write(".");
            /* Display the decimal part of the distance in meters. */
            shell_writeDec(pResult->resultCDE.distanceDecimalPart);
            shell_write(" m   ");

            shell_write("Quality: ");
            shell_writeDec(pResult->resultCDE.dqiIntegerPart);
            shell_write(".");
            shell_writeDec(pResult->resultCDE.dqiDecimalPart);
            shell_write("%%\r\n");

            /* Flash LEDs if distance is less than 1m. */
            if (bUIUpdated == FALSE)
            {
                if ((mPreviousDistance <= (float)1) && (pResult->resultCDE.distanceInMeters <= (float)1))
                {
                    LedStartFlashingAllLeds();
                }
                else if ((mPreviousDistance > (float)1) && (pResult->resultCDE.distanceInMeters > (float)1))
                {
                    LedStopFlashingAllLeds();
                    Led1On();
                }
                mPreviousDistance = pResult->resultCDE.distanceInMeters;
                bUIUpdated = TRUE;
            }
        }
#endif /* gAppUseCDEAlgorithm_d */

#if defined(gAppCsTimeInfo_d) && (gAppCsTimeInfo_d == 1)
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
#endif /* defined(gAppCsTimeInfo_d) && (gAppCsTimeInfo_d == 1) */

#if defined(gAppParseQualityInfo_d) && (gAppParseQualityInfo_d == 1)
        uint8_t aux;
        shell_write("Local RSSI avg: ");
        if(((uint8_t)pResult->rssiInfo[0] >> 7) != 0U)
        {
            shell_write("-");
            aux = ~((uint8_t)pResult->rssiInfo[0] - 1U);
            pResult->rssiInfo[0] = (int8_t)aux;
        }
        shell_writeDec(pResult->rssiInfo[0]);

        shell_write("\n\rRemote RSSI avg: ");
        if(((uint8_t)pResult->rssiInfo[1] >> 7) != 0U)
        {
            shell_write("-");
            aux = ~((uint8_t)pResult->rssiInfo[1] - 1U);
            pResult->rssiInfo[1] = (int8_t)aux;
        }
        shell_writeDec(pResult->rssiInfo[1]);

        shell_write("\n\rLocal Tone Quality Indicator: ");
        if (pResult->qualityInfo[0] != 0U)
        {
          shell_write("good");
        }
        else
        {
          shell_write("low");
        }

        shell_write("\n\rRemote Tone Quality Indicator: ");
        if (pResult->qualityInfo[1] != 0U)
        {
          shell_write("good");
        }
        else
        {
          shell_write("low");
        }
        shell_write("\r\n");
#endif /* defined(gAppParseQualityInfo_d) && (gAppParseQualityInfo_d == 1) */
    }

    if (mProcedureCount == mRangeSettings.maxNumProcedures)
    {
        shell_cmd_finished();
    }
}
