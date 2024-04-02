/*! *********************************************************************************
 * \addtogroup Wireless UART Application
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright 2015 Freescale Semiconductor, Inc.
* Copyright 2016-2024 NXP
*
*
* \file
*
* This file is the source file for the Wireless UART Application
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
#include "fsl_component_timer_manager.h"
#include "fsl_component_panic.h"
#include "fsl_component_serial_manager.h"
#include "fsl_component_mem_manager.h"
#include "fsl_format.h"
#include "fsl_debug_console.h"
#include "app.h"
#include "FunctionLib.h"
#include "board.h"
#include "fwk_platform_ble.h"

/* BLE Host Stack */
#include "gatt_interface.h"
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gatt_database.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"
#include "gatt_db_handles.h"
#include "gap_types.h"

/* Profile / Services */
#include "wireless_uart_interface.h"
#include "battery_interface.h"
/* Wrappers */
#include "ble_conn_manager.h"
#include "ble_service_discovery.h"

#include "board.h"
#include "board_comp.h"
#include "app_conn.h"
#include "wireless_uart.h"

#if defined(K32W232H_SERIES) || defined(KW45B41Z82_SERIES) || defined(KW45B41Z83_SERIES) || defined(K32W1480_SERIES) || \
    defined(KW47B42ZB7_cm33_core0_SERIES) || defined(KW47B42ZB6_cm33_core0_SERIES) || defined(KW47B42ZB3_cm33_core0_SERIES) || \
    defined(KW47B42ZB2_cm33_core0_SERIES) || defined(KW47B42Z97_cm33_core0_SERIES) || defined(KW47B42Z96_cm33_core0_SERIES) || \
    defined(KW47B42Z83_cm33_core0_SERIES)
#include "sensors.h"
#endif

#if defined(gUseControllerNotificationsCallback_c) && (gUseControllerNotificationsCallback_c)
    #include "controller_interface.h"
#endif

#include "app_conn.h"
#include "board.h"
#include "app.h"

/************************************************************************************
 *************************************************************************************
 * Private macros
 *************************************************************************************
 ************************************************************************************/

#define mAppUartBufferSize_c            gAttMaxWriteDataSize_d(gAttMaxMtu_c) /* Local Buffer Size */

#define mAppUartFlushIntervalInMs_c     (7)     /* Flush Timeout in Ms */

#define mBatteryLevelReportInterval_c   (10)    /* battery level report interval in seconds  */

#define gAllowToBlock_d                 (TRUE)
#define gNoBlock_d                      (FALSE)
#define Serial_Print(a,b)               do{ \
                                            union \
                                            { \
                                                const char *pStr; \
                                                uint8_t *pUint8; \
                                            } temp; \
                                            temp.pStr = (a); \
                                            if ((b) == gAllowToBlock_d) \
                                            { \
                                                (void)SerialManager_WriteBlocking((serial_write_handle_t)s_writeHandle, (uint8_t *)temp.pUint8, strlen(temp.pStr)); \
                                            } \
                                            else \
                                            { \
                                                (void)SerialManager_WriteNonBlocking((serial_write_handle_t)s_writeHandle, (uint8_t *)temp.pUint8, strlen(temp.pStr)); \
                                            } \
                                        } while(0);
#define Serial_PrintDec(a)              (void)SerialManager_WriteBlocking((serial_write_handle_t)s_writeHandle, FORMAT_Dec2Str(a), strlen((char const *)FORMAT_Dec2Str(a)))
#define Serial_PrintHex(a)              (void)SerialManager_WriteBlocking((serial_write_handle_t)s_writeHandle, FORMAT_Hex2Ascii(a), strlen((const char*)FORMAT_Hex2Ascii(a)))

/************************************************************************************
 *************************************************************************************
 * Private type definitions
 *************************************************************************************
 ************************************************************************************/
typedef enum appEvent_tag
{
    mAppEvt_PeerConnected_c,
    mAppEvt_PairingComplete_c,
    mAppEvt_ServiceDiscoveryComplete_c,
    mAppEvt_ServiceDiscoveryNotFound_c,
    mAppEvt_ServiceDiscoveryFailed_c,
    mAppEvt_GattProcComplete_c,
    mAppEvt_GattProcError_c
} appEvent_t;

typedef enum appState_tag
{
    mAppIdle_c,
    mAppExchangeMtu_c,
    mAppServiceDisc_c,
    mAppServiceDiscRetry_c,
    mAppRunning_c
} appState_t;

typedef struct appPeerInfo_tag
{
    deviceId_t  deviceId;
    bool_t      isBonded;
    wucConfig_t clientInfo;
    appState_t  appState;
    gapRole_t   gapRole;
} appPeerInfo_t;

typedef struct advState_tag
{
    bool_t advOn;
} advState_t;

/************************************************************************************
 *************************************************************************************
 * Private functions prototypes
 *************************************************************************************
 ************************************************************************************/
static void BleApp_Start(gapRole_t gapRole);
#if gWuart_PeripheralRole_c == 1
static void BleApp_AdvertisingCallback(gapAdvertisingEvent_t *pAdvertisingEvent);
#endif /* gWuart_PeripheralRole_c */
#if gWuart_CentralRole_c == 1
static void BleApp_ScanningCallback(gapScanningEvent_t *pScanningEvent);
#endif /* gWuart_CentralRole_c */
static void BleApp_ConnectionCallback(deviceId_t peerDeviceId, gapConnectionEvent_t *pConnectionEvent);
#if gWuart_CentralRole_c == 1
static bool_t BleApp_CheckScanEvent(gapScannedDevice_t *pData);
#endif /* gWuart_CentralRole_c */
/* Gatt and Att callbacks */
static void BleApp_GattServerCallback(deviceId_t deviceId, gattServerEvent_t *pServerEvent);
static void BleApp_GattClientCallback(deviceId_t serverDeviceId, gattProcedureType_t procedureType, gattProcedureResult_t   procedureResult, bleResult_t error);
static void BleApp_ServiceDiscoveryCallback(deviceId_t peerDeviceId, servDiscEvent_t *pEvent);
static void BleApp_StateMachineHandler(deviceId_t peerDeviceId, appEvent_t event);
static void BleApp_StoreServiceHandles(deviceId_t peerDeviceId, gattService_t *pService);

/* Timer Callbacks */
#if gWuart_CentralRole_c == 1
static void ScanningTimerCallback(void *pParam);
#endif /* gWuart_CentralRole_c */
static void UartStreamFlushTimerCallback(void *pData);
static void BatteryMeasurementTimerCallback(void *pParam);
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c == 1))
static void SwitchPressTimerCallback(void *pParam);
#endif

/* Uart Tx/Rx Callbacks*/
static void Uart_RxCallBack(void *pData, serial_manager_callback_message_t *pMessage, serial_manager_status_t status);
static void Uart_TxCallBack(void *pBuffer, serial_manager_callback_message_t *pMessage, serial_manager_status_t status);

static void BleApp_FlushUartStream(void *pParam);
static void BleApp_ReceivedUartStream(deviceId_t peerDeviceId, uint8_t *pStream, uint16_t streamLength);

#if defined(gUseControllerNotifications_c) && (gUseControllerNotifications_c)
static void BleApp_HandleControllerNotification(bleNotificationEvent_t *pNotificationEvent);

#if defined(gUseControllerNotificationsCallback_c) && (gUseControllerNotificationsCallback_c)
static void BleApp_ControllerNotificationCallback(bleCtrlNotificationEvent_t *pNotificationEvent);
#endif
#endif

static void BleApp_SerialInit(void);
static void BluetoothLEHost_Initialized(void);
static void BluetoothLEHost_GenericCallback(gapGenericEvent_t *pGenericEvent);

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
static button_status_t BleApp_HandleKeys0(void *pButtonHandle, button_callback_message_t *pMessage, void *pCallbackParam);
#endif /*gAppButtonCnt_c > 0*/

#if (gWuart_CentralRole_c == 1) && (gWuart_PeripheralRole_c == 1)
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1))
static button_status_t BleApp_HandleKeys1(void *pButtonHandle, button_callback_message_t *pMessage, void *pCallbackParam);
#endif
#endif
/************************************************************************************
 *************************************************************************************
 * Private memory declarations
 *************************************************************************************
 ************************************************************************************/
static appPeerInfo_t maPeerInformation[gAppMaxConnections_c];
static gapRole_t mGapRole;
static deviceId_t mcActiveConnNo;
/* Adv Parameters */
#if gWuart_PeripheralRole_c == 1
static advState_t mAdvState;
#endif /* gWuart_PeripheralRole_c */
#if gWuart_CentralRole_c == 1
static bool_t mScanningOn = FALSE;
#endif /* gWuart_CentralRole_c */

static uint16_t mCharMonitoredHandles[1] = { (uint16_t)value_uart_stream };

/* Service Data*/
static wusConfig_t mWuServiceConfig;
static bool_t      mBasValidClientList[gAppMaxConnections_c] = {FALSE};
static basConfig_t mBasServiceConfig = {(uint16_t)service_battery, 0, mBasValidClientList, gAppMaxConnections_c};

static TIMER_MANAGER_HANDLE_DEFINE(mAppTimerId);
static TIMER_MANAGER_HANDLE_DEFINE(mUartStreamFlushTimerId);
static TIMER_MANAGER_HANDLE_DEFINE(mBatteryMeasurementTimerId);

/* If the board has only one button, multiplex the required functionalities on it using an application timer */
#if (gAppButtonCnt_c == 1)
static TIMER_MANAGER_HANDLE_DEFINE(mSwitchPressTimerId);
#endif


/*wireless uart write handle*/
static SERIAL_MANAGER_WRITE_HANDLE_DEFINE(s_writeHandle);
/*wireless uart read handle*/
static SERIAL_MANAGER_READ_HANDLE_DEFINE(s_readHandle);

static uint16_t mAppUartBufferSize = mAppUartBufferSize_c;
static volatile bool_t mAppUartNewLine = FALSE;
static volatile bool_t mAppDapaPending = FALSE;

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c == 1))
static uint8_t mSwitchPressCnt = 0;
#endif

#if (gWuart_CentralRole_c == 1)
static appScanningParams_t mAppScanParams = {
    &gScanParams,
    gGapDuplicateFilteringEnable_c,
    gGapScanContinuously_d,
    gGapScanPeriodicDisabled_d
};
#endif

#if (gWuart_PeripheralRole_c == 1)
static appAdvertisingParams_t mAppAdvParams = {
    &gAdvParams,
    &gAppAdvertisingData,
    &gAppScanRspData
};
#endif
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
    BleApp_SerialInit();

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[0], BleApp_HandleKeys0, NULL);

#if (gWuart_CentralRole_c == 1) && (gWuart_PeripheralRole_c == 1)
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[1], BleApp_HandleKeys1, NULL);
#endif
#endif
#endif

    /* Initialize Bluetooth Host Stack */
    BluetoothLEHost_SetGenericCallback(BluetoothLEHost_GenericCallback);
    BluetoothLEHost_Init(BluetoothLEHost_Initialized);
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

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
#if (1 == gAppButtonCnt_c)
            /* increment the switch press counter */
            mSwitchPressCnt++;

            if(FALSE == TM_IsTimerActive(mSwitchPressTimerId))
            {
                /* Start the switch press timer */
                (void)TM_InstallCallback((timer_handle_t)mSwitchPressTimerId, SwitchPressTimerCallback, NULL);
                (void)TM_Start((timer_handle_t)mAppTimerId, (uint8_t)kTimerModeSingleShot | (uint8_t)kTimerModeLowPowerTimer, gSwitchPressTimeout_c);
            }
#else  /*1 == gAppButtonCnt_c*/

            BleApp_Start(mGapRole);
#endif /*1 == gAppButtonCnt_c*/
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
#endif /*gAppButtonCnt_c > 0*/

#if (gWuart_CentralRole_c == 1) && (gWuart_PeripheralRole_c == 1)
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
            /* Switch current role */
            if (mGapRole == gGapCentral_c)
            {
                Serial_Print("\n\rSwitched role to GAP Peripheral.\n\r", gAllowToBlock_d);
                mAppUartNewLine = TRUE;
                mGapRole = gGapPeripheral_c;
            }
            else
            {
                Serial_Print("\n\rSwitched role to GAP Central.\n\r", gAllowToBlock_d);
                mAppUartNewLine = TRUE;
                mGapRole = gGapCentral_c;
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
#endif /* (gWuart_CentralRole_c == 1) && (gWuart_PeripheralRole_c == 1) */
#endif /*gAppButtonCnt_c > 0*/

/*! *********************************************************************************
 * \brief    Starts the BLE application.
 *
 * \param[in]    gapRole            GAP Start Role (Central or Peripheral).
 ********************************************************************************** */
static void BleApp_Start
(
    gapRole_t gapRole
)
{
    LedStopFlashingAllLeds();
    Led1Flashing();
    switch (gapRole)
    {
#if gWuart_CentralRole_c == 1
    case gGapCentral_c:
        {
            if (mScanningOn == FALSE)
            {
                mAppUartNewLine = TRUE;
#if defined(gUseControllerNotifications_c) && (gUseControllerNotifications_c)
                (void)Gap_ControllerEnhancedNotification((uint16_t)gNotifScanEventOver_c | (uint16_t)gNotifScanAdvPktRx_c |
                                                   (uint16_t)gNotifScanRspRx_c | (uint16_t)gNotifScanReqTx_c, 0);
#endif
#if defined(gAppUsePairing_d) && (gAppUsePairing_d == 1)
                gPairingParameters.localIoCapabilities = gIoKeyboardDisplay_c;
#endif /* gAppUsePairing_d */
                (void)BluetoothLEHost_StartScanning(&mAppScanParams, BleApp_ScanningCallback);
            }
            break;
        }
#endif /* gWuart_CentralRole_c */
#if gWuart_PeripheralRole_c == 1
    case gGapPeripheral_c:
        {
            mAppUartNewLine = TRUE;
#if defined(gUseControllerNotifications_c) && (gUseControllerNotifications_c)
            (void)Gap_ControllerEnhancedNotification((uint16_t)gNotifAdvEventOver_c | (uint16_t)gNotifAdvTx_c |
                                               (uint16_t)gNotifAdvScanReqRx_c | (uint16_t)gNotifAdvConnReqRx_c, 0);
#endif
#if defined(gAppUsePairing_d) && (gAppUsePairing_d == 1)
            gPairingParameters.localIoCapabilities = gIoDisplayOnly_c;
#endif /* gAppUsePairing_d */

            /* Start ADV only if it's not already started */
            if (!mAdvState.advOn)
            {
                (void)BluetoothLEHost_StartAdvertising(&mAppAdvParams, BleApp_AdvertisingCallback, BleApp_ConnectionCallback);
            }
            break;
        }
#endif /* gWuart_PeripheralRole_c */
    default:
        {
            ; /* No action required */
            break;
        }
    }
}

/*! *********************************************************************************
 * \brief        Handles BLE Scanning callback from host stack.
 *
 * \param[in]    pScanningEvent     Pointer to gapScanningEvent_t.
 ********************************************************************************** */
#if gWuart_CentralRole_c == 1
static void BleApp_ScanningCallback
(
    gapScanningEvent_t *pScanningEvent
)
{
    switch (pScanningEvent->eventType)
    {
        case gDeviceScanned_c:
        {
            if (BleApp_CheckScanEvent(&pScanningEvent->eventData.scannedDevice) && (mcActiveConnNo < gAppMaxConnections_c))
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
            /* Node starts scanning */
            if (mScanningOn)
            {
                /* Start scanning timer */
                (void)TM_InstallCallback((timer_handle_t)mAppTimerId, ScanningTimerCallback, NULL);
                (void)TM_Start((timer_handle_t)mAppTimerId,
                            (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSetSecondTimer, gScanningTime_c);
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c == 1))
                LedSetColor(0, kLED_Blue);
#endif  /*gAppLedCnt_c == 1*/
                Led1Flashing();
                Serial_Print("\n\rScanning...\n\r", gAllowToBlock_d);
            }
            /* Node is not scanning */
            else
            {
                (void)TM_Stop((timer_handle_t)mAppTimerId);
                LedStartFlashingAllLeds();
                Serial_Print("\n\rStop Scanning...\n\r", gAllowToBlock_d);
            }
        }
        break;

        case gScanCommandFailed_c:
        {
            panic(0, 0, 0, 0);
            break;
        }

        default:
        {
            ; /* No action required */
            break;
        }
    }
}
#endif /* gWuart_CentralRole_c == 1 */

#if (gWuart_PeripheralRole_c == 1)
/*! *********************************************************************************
* \brief        Handles BLE Advertising callback from host stack.
*
* \param[in]    pAdvertisingEvent    Pointer to gapAdvertisingEvent_t.
********************************************************************************** */
static void BleApp_AdvertisingCallback
(
    gapAdvertisingEvent_t *pAdvertisingEvent
)
{
    switch (pAdvertisingEvent->eventType)
    {
        case gAdvertisingStateChanged_c:
        {
            mAdvState.advOn = !mAdvState.advOn;
            LedStopFlashingAllLeds();
            if (mAdvState.advOn)
            {
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c ==1))
            LedSetColor(0, kLED_Blue);
#endif /* gAppLedCnt_c == 1 */
            Led1Flashing();
            Serial_Print("\n\rAdvertising...\n\r", gAllowToBlock_d);
            }
            else
            {
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c ==1))
                LedSetColor(0, kLED_White);
#endif /* gAppLedCnt_c == 1 */
                LedStartFlashingAllLeds();
            }
        }
        break;

        case gAdvertisingCommandFailed_c:
        {
            panic(0, 0, 0, 0);
        }
        break;

        default:
        {
            ; /* No action required */
        }
        break;
    }
}

#endif
/*! *********************************************************************************
* \brief        Handles BLE Connection callback from host stack.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    pConnectionEvent    Pointer to gapConnectionEvent_t.
********************************************************************************** */
static void BleApp_ConnectionCallback
(
    deviceId_t peerDeviceId,
    gapConnectionEvent_t *pConnectionEvent
)
{
    switch (pConnectionEvent->eventType)
    {
        case gConnEvtConnected_c:
        {
            /* Save peer device ID */
            maPeerInformation[peerDeviceId].deviceId = peerDeviceId;
            mcActiveConnNo++;
            /* Advertising stops when connected */
#if gWuart_PeripheralRole_c == 1
            if(pConnectionEvent->eventData.connectedEvent.connectionRole == gBleLlConnectionPeripheral_c)
            {
                mAdvState.advOn = FALSE;
            }

#endif

            /* Subscribe client*/
            (void)Wus_Subscribe(peerDeviceId);
            (void)Bas_Subscribe(&mBasServiceConfig, peerDeviceId);

            /* UI */
            LedStopFlashingAllLeds();
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c == 1))
            LedSetColor(0, kLED_White);
#endif /* gAppLedCnt_c == 1 */
            Led1On();

            if (TM_IsTimerActive((timer_handle_t)mBatteryMeasurementTimerId) == 0U)
            {
                /* Start battery measurements */
                (void)TM_InstallCallback((timer_handle_t)mBatteryMeasurementTimerId, BatteryMeasurementTimerCallback, NULL);
                (void)TM_Start((timer_handle_t)mBatteryMeasurementTimerId,
                            (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSetSecondTimer, mBatteryLevelReportInterval_c);
            }

#if gAppUsePairing_d
#if gAppUseBonding_d

            if (mGapRole == gGapCentral_c)
            {
                union
                {
                    uint32_t u32;
                    uint16_t u16;
                } tempCast;

                tempCast.u32 = sizeof(wucConfig_t);
                (void)Gap_CheckIfBonded(peerDeviceId, &maPeerInformation[peerDeviceId].isBonded, NULL);

                if ((maPeerInformation[peerDeviceId].isBonded) &&
                    (gBleSuccess_c == Gap_LoadCustomPeerInformation(peerDeviceId,
                            (uint8_t *) &maPeerInformation[peerDeviceId].clientInfo, 0, tempCast.u16)))
                {
                    /* Restored custom connection information. Encrypt link */
                    (void)Gap_EncryptLink(peerDeviceId);
                }
            }

#endif /* gAppUseBonding_d*/
#endif /* gAppUsePairing_d */

            Serial_Print("Connected to device ", gAllowToBlock_d);
            Serial_PrintDec(peerDeviceId);

            if (pConnectionEvent->eventData.connectedEvent.connectionRole == gBleLlConnectionCentral_c)
            {
                Serial_Print(" as central.\n\r", gAllowToBlock_d);
            }
            else
            {
                Serial_Print(" as peripheral.\n\r", gAllowToBlock_d);
            }

            mAppUartNewLine = TRUE;

#if defined(gUseControllerNotifications_c) && (gUseControllerNotifications_c)
            (void)Gap_ControllerEnhancedNotification((uint16_t)gNotifConnRxPdu_c, peerDeviceId);
#endif

            if  (pConnectionEvent->eventData.connectedEvent.connectionRole == gBleLlConnectionPeripheral_c)
            {
                maPeerInformation[peerDeviceId].gapRole = gGapPeripheral_c;
            }
            else
            {
                maPeerInformation[peerDeviceId].gapRole = gGapCentral_c;
            }

            /* run the state machine */
            BleApp_StateMachineHandler(peerDeviceId, mAppEvt_PeerConnected_c);
        }
        break;

        case gConnEvtDisconnected_c:
        {
            Serial_Print("Disconnected from device ", gAllowToBlock_d);
            Serial_PrintDec(peerDeviceId);
            Serial_Print(".\n\r", gAllowToBlock_d);

            maPeerInformation[peerDeviceId].appState = mAppIdle_c;
            maPeerInformation[peerDeviceId].clientInfo.hService = gGattDbInvalidHandleIndex_d;
            maPeerInformation[peerDeviceId].clientInfo.hUartStream = gGattDbInvalidHandleIndex_d;

            /* Unsubscribe client */
            (void)Wus_Unsubscribe();
            (void)Bas_Unsubscribe(&mBasServiceConfig, peerDeviceId);
            (void)TM_Stop((timer_handle_t)mBatteryMeasurementTimerId);

            /* Reset Service Discovery to be sure*/
            BleServDisc_Stop(peerDeviceId);

            /* UI */
            LedStartFlashingAllLeds();

            /* mark device id as invalid */
            maPeerInformation[peerDeviceId].deviceId = gInvalidDeviceId_c;
            mcActiveConnNo--;
            /* recalculate minimum of maximum MTU's of all connected devices */
            mAppUartBufferSize                       = mAppUartBufferSize_c;

            for (uint8_t mPeerId = 0U; mPeerId < (uint8_t)gAppMaxConnections_c; mPeerId++)
            {
                if (gInvalidDeviceId_c != maPeerInformation[mPeerId].deviceId)
                {
                    uint16_t tempMtu = 0U;

                    (void)Gatt_GetMtu(mPeerId, &tempMtu);
                    tempMtu = gAttMaxWriteDataSize_d(tempMtu);

                    if (tempMtu < mAppUartBufferSize)
                    {
                        mAppUartBufferSize = tempMtu;
                    }
                }
            }

            if (mGapRole == gGapPeripheral_c)
            {
                BleApp_Start(mGapRole);
            }
        }
        break;

#if gAppUsePairing_d

        case gConnEvtPairingComplete_c:
        {
            if (pConnectionEvent->eventData.pairingCompleteEvent.pairingSuccessful)
            {
                BleApp_StateMachineHandler(peerDeviceId,
                                           mAppEvt_PairingComplete_c);
            }
        }
        break;
#endif /* gAppUsePairing_d */

        default:
        {
            ; /* No action required */
        }
        break;
    }

    /* Connection Manager to handle Host Stack interactions */
    switch (maPeerInformation[peerDeviceId].gapRole)
    {
#if gWuart_CentralRole_c == 1
        case gGapCentral_c:
            BleConnManager_GapCentralEvent(peerDeviceId, pConnectionEvent);
            break;
#endif /* gWuart_CentralRole_c == 1 */
#if gWuart_PeripheralRole_c == 1
        case gGapPeripheral_c:
            BleConnManager_GapPeripheralEvent(peerDeviceId, pConnectionEvent);
            break;
#endif /* gWuart_PeripheralRole_c == 1 */
        default:
            ; /* No action required */
            break;
    }
}

/*! *********************************************************************************
* \brief        Handle Service Discovery events
*
* \param[in]    peerDeviceId        Remote device ID.
* \param[in]    pEvent              Pointer to the event revceived.
********************************************************************************** */
static void BleApp_ServiceDiscoveryCallback
(
    deviceId_t peerDeviceId,
    servDiscEvent_t *pEvent
)
{
    switch (pEvent->eventType)
    {
        case gServiceDiscovered_c:
        {
            if (pEvent->eventData.pService->uuidType == gBleUuidType128_c)
            {
                if (FLib_MemCmp((void *)&uuid_service_wireless_uart, (void *)&pEvent->eventData.pService->uuid, sizeof(bleUuid_t)))
                {
                    BleApp_StoreServiceHandles(peerDeviceId, pEvent->eventData.pService);
                }
            }
        }
        break;

        case gDiscoveryFinished_c:
        {
            if (pEvent->eventData.success)
            {
                if (gGattDbInvalidHandleIndex_d != maPeerInformation[peerDeviceId].clientInfo.hService)
                {
                    BleApp_StateMachineHandler(peerDeviceId,
                                               mAppEvt_ServiceDiscoveryComplete_c);
                }
                else
                {
                    BleApp_StateMachineHandler(peerDeviceId,
                                               mAppEvt_ServiceDiscoveryNotFound_c);
                }
            }
            else
            {
                BleApp_StateMachineHandler(peerDeviceId,
                                           mAppEvt_ServiceDiscoveryFailed_c);
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
static void BleApp_GattClientCallback
(
    deviceId_t              serverDeviceId,
    gattProcedureType_t     procedureType,
    gattProcedureResult_t   procedureResult,
    bleResult_t             error
)
{
    switch (procedureResult)
    {
        case gGattProcError_c:
            BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcError_c);
            break;

        case gGattProcSuccess_c:
            BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcComplete_c);
            break;

        default:
            ; /* No action required */
            break;
    }

    /* Signal Service Discovery Module */
    BleServDisc_SignalGattClientEvent(serverDeviceId, procedureType, procedureResult, error);
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
    uint16_t tempMtu = 0;

    switch (pServerEvent->eventType)
    {
        case gEvtAttributeWrittenWithoutResponse_c:
        {
            if (pServerEvent->eventData.attributeWrittenEvent.handle == (uint16_t)value_uart_stream)
            {
                BleApp_ReceivedUartStream(deviceId, pServerEvent->eventData.attributeWrittenEvent.aValue,
                                          pServerEvent->eventData.attributeWrittenEvent.cValueLength);
            }

            break;
        }

        case gEvtMtuChanged_c:
        {
            /* update stream length with minimum of  new MTU */
            (void)Gatt_GetMtu(deviceId, &tempMtu);
            tempMtu = gAttMaxWriteDataSize_d(tempMtu);

            mAppUartBufferSize = mAppUartBufferSize <= tempMtu ? mAppUartBufferSize : tempMtu;
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
 * \param[in]    peerDeviceId       The remote device ID.
 * \param[in]    pService           Pointer to gattService_t.
 ********************************************************************************** */
static void BleApp_StoreServiceHandles
(
    deviceId_t peerDeviceId,
    gattService_t *pService
)
{
    /* Found Wireless UART Service */
    maPeerInformation[peerDeviceId].clientInfo.hService = pService->startHandle;

    if (pService->cNumCharacteristics > 0U &&
        pService->aCharacteristics != NULL)
    {
        /* Found Uart Characteristic */
        maPeerInformation[peerDeviceId].clientInfo.hUartStream =
            pService->aCharacteristics[0].value.handle;
    }
}

/*! *********************************************************************************
 * \brief        Checks Scan data for a device to connect.
 *
 * \param[in]    pData              Pointer to gapScannedDevice_t.
 ********************************************************************************** */
#if gWuart_CentralRole_c == 1
static bool_t BleApp_CheckScanEvent
(
    gapScannedDevice_t *pData
)
{
    uint8_t index = 0;
    bool_t foundMatch = FALSE;

    while (index < pData->dataLength)
    {
        gapAdStructure_t adElement;

        adElement.length = pData->data[index];
        adElement.adType = (gapAdType_t) pData->data[index + 1U];
        adElement.aData = &pData->data[index + 2U];

        /* Search for Wireless UART Service */
        if ((adElement.adType == gAdIncomplete128bitServiceList_c)
            || (adElement.adType == gAdComplete128bitServiceList_c))
        {
            foundMatch = BluetoothLEHost_MatchDataInAdvElementList(
                    &adElement,
                    &uuid_service_wireless_uart,
                    16);
        }

        /* Move on to the next AD element type */
        index += adElement.length + (uint8_t)sizeof(uint8_t);
    }

    return foundMatch;
}
#endif /* gWuart_CentralRole_c == 1 */

/*! *********************************************************************************
 * \brief        Send the received uart stream over GATT
 *
 * \param[in]    pData              Pointer to the received stream.
 * \param[in]    streamSize         The number of bytes in the stream.
 ********************************************************************************** */
static void BleApp_SendUartStream
(
    uint8_t *pRecvStream,
    uint8_t streamSize
)
{
    gattCharacteristic_t characteristic = {(uint8_t)gGattCharPropNone_c, {0}, 0, 0};
    uint8_t              mPeerId = 0;

    /* send UART stream to all peers */
    for (mPeerId = 0; mPeerId < (uint8_t)gAppMaxConnections_c; mPeerId++)
    {
        if (gInvalidDeviceId_c != maPeerInformation[mPeerId].deviceId &&
            mAppRunning_c == maPeerInformation[mPeerId].appState)
        {
            characteristic.value.handle = maPeerInformation[mPeerId].clientInfo.hUartStream;
            (void)GattClient_WriteCharacteristicValue(mPeerId, &characteristic,
                    streamSize, pRecvStream, TRUE,
                    FALSE, FALSE, NULL);
        }
    }
}

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
    uint16_t tempMtu = 0;
    union
    {
        uint8_t     *pUuidArray;
        bleUuid_t   *pUuidObj;
    } temp; /* MISRA rule 11.3 */

    temp.pUuidArray = uuid_service_wireless_uart;

    /* invalid client information */
    if (maPeerInformation[peerDeviceId].deviceId != gInvalidDeviceId_c)
    {
        switch (maPeerInformation[peerDeviceId].appState)
        {
        case mAppIdle_c:
        {
            if (event == mAppEvt_PeerConnected_c)
            {
                /* Let the central device initiate the Exchange MTU procedure*/
                if (mGapRole == gGapCentral_c)
                {
                    /* Moving to Exchange MTU State */
                    maPeerInformation[peerDeviceId].appState = mAppExchangeMtu_c;
                    (void)GattClient_ExchangeMtu(peerDeviceId, gAttMaxMtu_c);
                }
                else
                {
                    /* Moving to Service Discovery State*/
                    maPeerInformation[peerDeviceId].appState = mAppServiceDisc_c;

                    /* Start Service Discovery*/
                    (void)BleServDisc_FindService(peerDeviceId,
                            gBleUuidType128_c,
                            temp.pUuidObj);
                }
            }
        }
        break;

        case mAppExchangeMtu_c:
        {
            if (event == mAppEvt_GattProcComplete_c)
            {
                /* update stream length with minimum of maximum MTU's of connected devices */
                (void)Gatt_GetMtu(peerDeviceId, &tempMtu);
                tempMtu = gAttMaxWriteDataSize_d(tempMtu);

                mAppUartBufferSize = mAppUartBufferSize <= tempMtu ? mAppUartBufferSize : tempMtu;

                /* Moving to Service Discovery State*/
                maPeerInformation[peerDeviceId].appState = mAppServiceDisc_c;

                /* Start Service Discovery*/
                (void)BleServDisc_FindService(peerDeviceId,
                        gBleUuidType128_c,
                        temp.pUuidObj);
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

        case mAppServiceDisc_c:
        {
            if (event == mAppEvt_ServiceDiscoveryComplete_c)
            {
                /* Moving to Running State*/
                maPeerInformation[peerDeviceId].appState = mAppRunning_c;
#if gAppUseBonding_d
                union
                {
                    uint32_t u32;
                    uint16_t u16;
                } tempCast;

                tempCast.u32 = sizeof(wucConfig_t);
                /* Write data in NVM */
                (void)Gap_SaveCustomPeerInformation(maPeerInformation[peerDeviceId].deviceId,
                        (uint8_t *) &maPeerInformation[peerDeviceId].clientInfo, 0,
                        tempCast.u16);
#endif
            }
            else if (event == mAppEvt_ServiceDiscoveryNotFound_c)
            {
                /* Moving to Service discovery Retry State*/
                maPeerInformation[peerDeviceId].appState = mAppServiceDiscRetry_c;
                /* Restart Service Discovery for all services */
                (void)BleServDisc_Start(peerDeviceId);
            }
            else if (event == mAppEvt_ServiceDiscoveryFailed_c)
            {
                (void)Gap_Disconnect(peerDeviceId);
            }
            else
            {
                /* ignore other event types */
            }
        }
        break;

        case mAppServiceDiscRetry_c:
        {
            if (event == mAppEvt_ServiceDiscoveryComplete_c)
            {
                /* Moving to Running State*/
                maPeerInformation[peerDeviceId].appState = mAppRunning_c;
            }
            else if ((event == mAppEvt_ServiceDiscoveryNotFound_c) ||
                    (event == mAppEvt_ServiceDiscoveryFailed_c))
            {
                (void)Gap_Disconnect(peerDeviceId);
            }
            else
            {
                /* ignore other event types */
            }
        }
        break;

        case mAppRunning_c:
            break;

        default:
        {
            ; /* No action required */
        }
        break;
        }
    }
}

#if gWuart_CentralRole_c == 1
/*! *********************************************************************************
 * \brief        Handles scanning timer callback.
 *
 * \param[in]    pParam             Callback parameters.
 ********************************************************************************** */
static void ScanningTimerCallback
(
    void *pParam
)
{
    /* Stop scanning */
    (void)Gap_StopScanning();
}
#endif /* gWuart_CentralRole_c == 1 */

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c == 1))
/*! *********************************************************************************
 * \brief        Handles the switch press timer callback.
 *
 * \param[in]    pParam             Callback parameters.
 ********************************************************************************** */
static void SwitchPressTimerCallback
(
    void *pParam
)
{
  if(mSwitchPressCnt >= gSwitchPressThreshold_c)
  {
      /* Switch the current role */
      if (mGapRole == gGapCentral_c)
      {
          Serial_Print("\n\rSwitched role to GAP Peripheral.\n\r", gAllowToBlock_d);
          mAppUartNewLine = TRUE;
          mGapRole = gGapPeripheral_c;
      }
      else
      {
          Serial_Print("\n\rSwitched role to GAP Central.\n\r", gAllowToBlock_d);
          mAppUartNewLine = TRUE;
          mGapRole = gGapCentral_c;
      }
  }
  else
  {
      /* start the application using the selected role */
      LedStopFlashingAllLeds();
      Led1Flashing();
      BleApp_Start(mGapRole);
  }

  /* reset the switch press counter */
  mSwitchPressCnt = 0;
}
#endif

/*! *********************************************************************************
 * \brief        Get all bytes from the serial interface and send it over GATT.
 *
 * \param[in]    pParam             Callback parameters.
 ********************************************************************************** */
static void BleApp_FlushUartStream
(
    void *pParam
)
{
    uint8_t *pMsg = NULL;
    uint32_t bytesRead = 0;
    uint8_t  mPeerId = 0;
    bool_t   mValidDevices = FALSE;

    /* Valid devices are in Running state */
    for (mPeerId = 0; mPeerId < (uint8_t)gAppMaxConnections_c; mPeerId++)
    {
        if ((gInvalidDeviceId_c != maPeerInformation[mPeerId].deviceId) &&
            (mAppRunning_c == maPeerInformation[mPeerId].appState))
        {
            mValidDevices = TRUE;
            break;
        }
    }

    if (mValidDevices)
    {
        /* Allocate buffer for GATT Write */
        pMsg = MEM_BufferAlloc(mAppUartBufferSize);

        if (pMsg != NULL)
        {
            /* Collect the data from the serial manager buffer */
            if (SerialManager_TryRead((serial_read_handle_t)s_readHandle, pMsg, mAppUartBufferSize, &bytesRead) == kStatus_SerialManager_Success)
            {
                if (bytesRead != 0U)
                {
                    /* Send data over the air */
                    BleApp_SendUartStream(pMsg, (uint8_t)bytesRead);
                }
            }


            /* Free Buffer */
            (void)MEM_BufferFree(pMsg);
        }
    }

    mAppDapaPending = FALSE;
}

/*! *********************************************************************************
 * \brief        Write bytes to serial.
 *
 * \param[in]    peerDeviceId       The remote device ID.
 * \param[in]    pStream            Pointer to the received stream.
 * \param[in]    streamLength       Number of bytes in the strem.
 ********************************************************************************** */
static void BleApp_ReceivedUartStream
(
    deviceId_t peerDeviceId,
    uint8_t *pStream,
    uint16_t streamLength
)
{
    static deviceId_t previousDeviceId = gInvalidDeviceId_c;

    char additionalInfoBuff[10] = { '\r', '\n', '[', '0', '0', '-', 'C', ']', ':', ' '};
    uint8_t *pBuffer = NULL;
    uint32_t messageHeaderSize = 0;

    if (mAppUartNewLine || (previousDeviceId != peerDeviceId))
    {
        streamLength += (uint16_t)sizeof(additionalInfoBuff);
    }

    /* Allocate buffer for asynchronous write */
    pBuffer = MEM_BufferAlloc(streamLength);

    if (pBuffer != NULL)
    {
        /* if this is a message from a previous device, print device ID */
        if (mAppUartNewLine || (previousDeviceId != peerDeviceId))
        {
            messageHeaderSize = sizeof(additionalInfoBuff);

            if (mAppUartNewLine)
            {
                mAppUartNewLine = FALSE;
            }

            additionalInfoBuff[3] = '0' + (peerDeviceId / 10U);
            additionalInfoBuff[4] = '0' + (peerDeviceId % 10U);

            if (gGapCentral_c != maPeerInformation[peerDeviceId].gapRole)
            {
                additionalInfoBuff[6] = 'P';
            }

            FLib_MemCpy(pBuffer, additionalInfoBuff, sizeof(additionalInfoBuff));
        }

        FLib_MemCpy(&pBuffer[messageHeaderSize], pStream, (uint32_t)streamLength - messageHeaderSize);
#if (defined(SERIAL_MANAGER_NON_BLOCKING_MODE) && (SERIAL_MANAGER_NON_BLOCKING_MODE > 0U))
        serial_manager_status_t status = SerialManager_InstallTxCallback((serial_write_handle_t)s_writeHandle, Uart_TxCallBack, pBuffer);
        (void)status;
        assert(kStatus_SerialManager_Success == status);

        (void)SerialManager_WriteNonBlocking((serial_write_handle_t)s_writeHandle, pBuffer, streamLength);
#endif /*SERIAL_MANAGER_NON_BLOCKING_MODE > 0U*/
    }

    /* update the previous device ID */
    previousDeviceId = peerDeviceId;
}

/*! *********************************************************************************
 * \brief        Timer handler for flushing the UART.
 *
 * \param[in]    pData              Pointer to the parameters.
 ********************************************************************************** */
static void UartStreamFlushTimerCallback
(
    void *pData
)
{
    if (!mAppDapaPending)
    {
        mAppDapaPending = TRUE;
        (void)App_PostCallbackMessage(BleApp_FlushUartStream, NULL);
    }
}

/*! *********************************************************************************
* \brief        Handles UART Receive callback.
*
* \param[in]    pData        Unused pointer to data.
* \param[in]    pMessage     Unused pointer to message.
* \param[in]    status       Unused status.
********************************************************************************** */
static void Uart_RxCallBack
(
    void *pData,
    serial_manager_callback_message_t *pMessage,
    serial_manager_status_t status
)
{
    uint16_t byteCount = 0;

    if (byteCount < mAppUartBufferSize)
    {
        /* Restart flush timer */
        (void)TM_InstallCallback((timer_handle_t)mUartStreamFlushTimerId, UartStreamFlushTimerCallback, NULL);
        (void)TM_Start((timer_handle_t)mUartStreamFlushTimerId,
                    (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSingleShot, mAppUartFlushIntervalInMs_c);
    }
    else
    {
        /* Post App Msg only one at a time */
        if (!mAppDapaPending)
        {
            mAppDapaPending = TRUE;
            (void)App_PostCallbackMessage(BleApp_FlushUartStream, NULL);
        }
    }
}

/*! *********************************************************************************
* \brief        Handles UART Transmit callback.
*
* \param[in]    pBuffer      Pointer to the sent data.
* \param[in]    pMessage     Unused pointer to message.
* \param[in]    status       Unused status.
********************************************************************************** */
static void Uart_TxCallBack
(
    void *pBuffer,
    serial_manager_callback_message_t *pMessage,
    serial_manager_status_t status
)
{
    (void)MEM_BufferFree(pBuffer);
}


/*! *********************************************************************************
* \brief        Handles battery measurement timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void BatteryMeasurementTimerCallback
(
    void *pParam
)
{
    mBasServiceConfig.batteryLevel = SENSORS_GetBatteryLevel();
    (void)Bas_RecordBatteryMeasurement(&mBasServiceConfig);
}

#if defined(gUseControllerNotifications_c) && (gUseControllerNotifications_c)
/*! *********************************************************************************
 * \brief        Function handling the Bluetooth Controller notification events.
 *
 * \param[in]    pNotificationEvent Pointer to the notification event.
 ********************************************************************************** */
static void BleApp_HandleControllerNotification
(
    bleNotificationEvent_t *pNotificationEvent
)
{
    switch(pNotificationEvent->eventType)
    {
        case (uint16_t)gNotifEventNone_c:
        {
            Serial_Print("Configured notification status ", gAllowToBlock_d);
            Serial_PrintDec((uint32_t)pNotificationEvent->status);
            Serial_Print("\n\r", gAllowToBlock_d);
            break;
        }

        case (uint16_t)gNotifConnEventOver_c:
        {
            Serial_Print("CONN Event Over device ", gAllowToBlock_d);
            Serial_PrintDec(pNotificationEvent->deviceId);
            Serial_Print(" on channel ", gAllowToBlock_d);
            Serial_PrintDec(pNotificationEvent->channel);
            Serial_Print(" with RSSI ", gAllowToBlock_d);
            Serial_PrintDec((uint8_t)pNotificationEvent->rssi);
            Serial_Print(" and event counter ", gAllowToBlock_d);
            Serial_PrintDec((uint16_t)pNotificationEvent->ce_counter);
            Serial_Print("\n\r", gAllowToBlock_d);
            break;
        }

        case (uint16_t)gNotifConnRxPdu_c:
        {
            Serial_Print("CONN Rx PDU from device ", gAllowToBlock_d);
            Serial_PrintDec(pNotificationEvent->deviceId);
            Serial_Print(" on channel ", gAllowToBlock_d);
            Serial_PrintDec(pNotificationEvent->channel);
            Serial_Print(" with RSSI ", gAllowToBlock_d);
            Serial_PrintDec((uint8_t)pNotificationEvent->rssi);
            Serial_Print(" with event counter ", gAllowToBlock_d);
            Serial_PrintDec((uint16_t)pNotificationEvent->ce_counter);
            Serial_Print(" and timestamp ", gAllowToBlock_d);
            Serial_PrintDec(pNotificationEvent->timestamp);
            Serial_Print("\n\r", gAllowToBlock_d);
            break;
        }

        case (uint16_t)gNotifAdvEventOver_c:
        {
            Serial_Print("ADV Event Over.\n\r", gAllowToBlock_d);
            break;
        }

        case (uint16_t)gNotifAdvTx_c:
        {
            Serial_Print("ADV Tx on channel ", gAllowToBlock_d);
            Serial_PrintDec(pNotificationEvent->channel);
            Serial_Print("\n\r", gAllowToBlock_d);
            break;
        }

        case (uint16_t)gNotifAdvScanReqRx_c:
        {
            Serial_Print("ADV Rx Scan Req on channel ", gAllowToBlock_d);
            Serial_PrintDec(pNotificationEvent->channel);
            Serial_Print(" with RSSI ", gAllowToBlock_d);
            Serial_PrintDec((uint8_t)pNotificationEvent->rssi);
            Serial_Print("\n\r", gAllowToBlock_d);
            break;
        }

        case (uint16_t)gNotifAdvConnReqRx_c:
        {
            Serial_Print("ADV Rx Conn Req on channel ", gAllowToBlock_d);
            Serial_PrintDec(pNotificationEvent->channel);
            Serial_Print(" with RSSI ", gAllowToBlock_d);
            Serial_PrintDec((uint8_t)pNotificationEvent->rssi);
            Serial_Print("\n\r", gAllowToBlock_d);
            break;
        }

        case (uint16_t)gNotifScanEventOver_c:
        {
            Serial_Print("SCAN Event Over on channel ", gAllowToBlock_d);
            Serial_PrintDec(pNotificationEvent->channel);
            Serial_Print("\n\r", gAllowToBlock_d);
            break;
        }

        case (uint16_t)gNotifScanAdvPktRx_c:
        {
            Serial_Print("SCAN Rx Adv Pkt on channel ", gAllowToBlock_d);
            Serial_PrintDec(pNotificationEvent->channel);
            Serial_Print(" with RSSI ", gAllowToBlock_d);
            Serial_PrintDec((uint8_t)pNotificationEvent->rssi);
            Serial_Print("\n\r", gAllowToBlock_d);
            break;
        }

        case (uint16_t)gNotifScanRspRx_c:
        {
            Serial_Print("SCAN Rx Scan Rsp on channel ", gAllowToBlock_d);
            Serial_PrintDec(pNotificationEvent->channel);
            Serial_Print(" with RSSI ", gAllowToBlock_d);
            Serial_PrintDec((uint8_t)pNotificationEvent->rssi);
            Serial_Print("\n\r", gAllowToBlock_d);
            break;
        }

        case (uint16_t)gNotifScanReqTx_c:
        {
            Serial_Print("SCAN Tx Scan Req on channel ", gAllowToBlock_d);
            Serial_PrintDec(pNotificationEvent->channel);
            Serial_Print("\n\r", gAllowToBlock_d);
            break;
        }

        case (uint16_t)gNotifConnCreated_c:
        {
            Serial_Print("CONN Created with device ", gAllowToBlock_d);
            Serial_PrintDec(pNotificationEvent->deviceId);
            Serial_Print(" with timestamp ", gAllowToBlock_d);
            Serial_PrintDec(pNotificationEvent->timestamp);
            Serial_Print("\n\r", gAllowToBlock_d);
            break;
        }
        
        case (uint16_t)gNotifConnChannelMapUpdate_c:
        {
            Serial_Print("Map update with device ", gAllowToBlock_d);
            Serial_PrintDec(pNotificationEvent->deviceId);
            Serial_Print("\n\r", gAllowToBlock_d);
            break;
        }

        default:
        {
            ; /* No action required */
            break;
        }
    }
}

#if defined(gUseControllerNotificationsCallback_c) && (gUseControllerNotificationsCallback_c)
/*! *********************************************************************************
 * \brief        Function handling the controller notifications.
 *
 * \param[in]    pNotificationEvent Pointer to the notification event.
 ********************************************************************************** */
static void BleApp_ControllerNotificationCallback
(
    bleCtrlNotificationEvent_t *pNotificationEvent
)
{
    switch(pNotificationEvent->event_type)
    {
        case gNotifConnEventOver_c:
        {
            Serial_Print("CONN Ev Over\n\r", gNoBlock_d);
            break;
        }

        case gNotifConnRxPdu_c:
        {
            Serial_Print("CONN Rx PDU\n\r", gNoBlock_d);
            break;
        }

        case gNotifAdvEventOver_c:
        {
            Serial_Print("ADV Ev Over\n\r", gNoBlock_d);
            break;
        }

        case gNotifAdvTx_c:
        {
            Serial_Print("ADV Tx\n\r", gNoBlock_d);
            break;
        }

        case gNotifAdvScanReqRx_c:
        {
            Serial_Print("ADV Rx Scan Req\n\r", gNoBlock_d);
            break;
        }

        case gNotifAdvConnReqRx_c:
        {
            Serial_Print("ADV Rx Conn Req\n\r", gNoBlock_d);
            break;
        }

        case gNotifScanEventOver_c:
        {
            Serial_Print("SCAN Ev Over\n\r", gNoBlock_d);
            break;
        }

        case gNotifScanAdvPktRx_c:
        {
            Serial_Print("SCAN Rx Adv\n\r", gNoBlock_d);
            break;
        }

        case gNotifScanRspRx_c:
        {
            Serial_Print("SCAN Rx Scan Rsp\n\r", gNoBlock_d);
            break;
        }

        case gNotifScanReqTx_c:
        {
            Serial_Print("SCAN Tx Scan Req\n\r", gNoBlock_d);
            break;
        }

        case gNotifConnCreated_c:
        {
            Serial_Print("CONN Created\n\r", gNoBlock_d);
            break;
        }

        default:
        {
            ; /* No action required */
            break;
        }
    }
}
#endif

#endif

/*! *********************************************************************************
 * \brief        Function used to setup the serial interface.
 *
 ********************************************************************************** */
static void BleApp_SerialInit(void)
{
    serial_manager_status_t status;
    /*wireless uart serial manager handle*/
    static serial_handle_t appSerMgrIf;

    /* UI */
    appSerMgrIf = (serial_handle_t)&gSerMgrIf[0];

    /*open wireless uart read/write handle*/
    status = SerialManager_OpenWriteHandle((serial_handle_t)appSerMgrIf, (serial_write_handle_t)s_writeHandle);
    assert(kStatus_SerialManager_Success == status);
    (void)status;

    status = SerialManager_OpenReadHandle((serial_handle_t)appSerMgrIf, (serial_read_handle_t)s_readHandle);
    assert(kStatus_SerialManager_Success == status);
    status = SerialManager_InstallRxCallback((serial_read_handle_t)s_readHandle, Uart_RxCallBack, NULL);
    assert(kStatus_SerialManager_Success == status);
}

/*! *********************************************************************************
 * \brief        Function handling the initialization complete event of the Bluetooth LE Host stack.
 *
 ********************************************************************************** */
static void BluetoothLEHost_Initialized(void)
{
    uint8_t peerId = 0;

    /* Common GAP configuration */
    BleConnManager_GapCommonConfig();

    /* Register for callbacks */
    (void)App_RegisterGattServerCallback(BleApp_GattServerCallback);
    (void)App_RegisterGattClientProcedureCallback(BleApp_GattClientCallback);
    (void)GattServer_RegisterHandlesForWriteNotifications(NumberOfElements(mCharMonitoredHandles), mCharMonitoredHandles);
    BleServDisc_RegisterCallback(BleApp_ServiceDiscoveryCallback);
    mcActiveConnNo = 0U;
    for (peerId = 0; peerId < (uint8_t)gAppMaxConnections_c; peerId++)
    {
        maPeerInformation[peerId].deviceId = gInvalidDeviceId_c;
        maPeerInformation[peerId].appState = mAppIdle_c;
        maPeerInformation[peerId].clientInfo.hService = gGattDbInvalidHandleIndex_d;
        maPeerInformation[peerId].clientInfo.hUartStream = gGattDbInvalidHandleIndex_d;
    }

#if (gWuart_AutoStart_c == 1)
    mGapRole = gWuart_AutoStartGapRole_c;
#else
    /* By default, always start node as GAP central */
#if gWuart_CentralRole_c == 1
    mGapRole = gGapCentral_c;
#else
    mGapRole = gGapPeripheral_c;
#endif /* gWuart_CentralRole_c */
#endif /* gWuart_AutoStart_c */

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c == 1))
    Serial_Print("\n\rWireless UART starting as GAP Central.\n\r", gAllowToBlock_d);
    Serial_Print("\n\rWithin one second, either:\n\r", gAllowToBlock_d);
    Serial_Print(" - double press the switch to change the role or\n\r", gAllowToBlock_d);
    Serial_Print(" - single press to start the application with the selected role.\n\r", gAllowToBlock_d);
#else
    if (mGapRole == gGapCentral_c)
    {
        Serial_Print("\n\rWireless UART starting as GAP Central.\n\r", gAllowToBlock_d);
    }
    else
    {
        Serial_Print("\n\rWireless UART starting as GAP Peripheral.\n\r", gAllowToBlock_d);
    }
#endif
#if gWuart_PeripheralRole_c == 1
    mAdvState.advOn = FALSE;
#endif /* gWuart_PeripheralRole_c */

#if gWuart_CentralRole_c == 1
    mScanningOn = FALSE;
#endif /* gWuart_CentralRole_c */

    /* Start services */
    (void)Wus_Start(&mWuServiceConfig);

    mBasServiceConfig.batteryLevel = SENSORS_GetBatteryLevel();
    (void)Bas_Start(&mBasServiceConfig);

    /* Allocate application timer */
    (void)TM_Open(mAppTimerId);
    (void)TM_Open(mUartStreamFlushTimerId);
    (void)TM_Open(mBatteryMeasurementTimerId);

#if (gAppButtonCnt_c == 1)
    (void)TM_Open(mSwitchPressTimerId);
#endif

#if !(defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode))
    LedStartFlashingAllLeds();
#endif
#if (gWuart_AutoStart_c == 1)
    BleApp_Start(mGapRole);
#endif
}

/*! *********************************************************************************
 * \brief        Function handling the geeneric events from the host stack.
 *
 ********************************************************************************** */
static void BluetoothLEHost_GenericCallback(gapGenericEvent_t *pGenericEvent)
{
    /* Call BLE Conn Manager */
    BleConnManager_GenericEvent(pGenericEvent);

#if defined(gUseControllerNotifications_c) && (gUseControllerNotifications_c)
    if (pGenericEvent->eventType == gControllerNotificationEvent_c)
    {
        BleApp_HandleControllerNotification(&pGenericEvent->eventData.notifEvent);
    }
#endif /* gUseControllerNotifications_c */
    if(pGenericEvent->eventType == gInternalError_c)
    {
        if((pGenericEvent->eventData.internalError.errorCode == gBleOverflow_c) && (pGenericEvent->eventData.internalError.errorSource == gAddNewConnection_c))
        {
            Serial_Print("Connection failed ", gAllowToBlock_d);
#if gWuart_PeripheralRole_c == 1
            if(mAdvState.advOn)
            {
                mAdvState.advOn = FALSE;
                Serial_Print(" as peripheral.\n\r", gAllowToBlock_d);
            }
            else
#endif /* gWuart_PeripheralRole_c == 1 */
            {
#if gWuart_CentralRole_c == 1
                Serial_Print(" as central.\n\r", gAllowToBlock_d);
#endif /* gWuart_CentralRole_c == 1 */

            }
        }
    }
}

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
