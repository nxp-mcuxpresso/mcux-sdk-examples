/*! *********************************************************************************
 * \addtogroup Wireless UART Application
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2020 NXP
* All rights reserved.
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
#include "Keyboard.h"
#include "LED.h"
#include "TimersManager.h"
#include "FunctionLib.h"
#include "Panic.h"
#include "SerialManager.h"
#include "MemManager.h"
#include "Messaging.h"

#if (cPWR_UsePowerDownMode)
#include "PWR_Interface.h"
#include "PWR_Configuration.h"
#endif

#include "board.h"

/* BLE Host Stack */
#include "gatt_interface.h"
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gatt_database.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"
#include "gatt_db_handles.h"


/* Profile / Services */
#include "wireless_uart_interface.h"
#include "battery_interface.h"
/* Wrappers */
#include "ble_conn_manager.h"
#include "ble_service_discovery.h"

/* Concatenate macros */
#ifndef QU
#define QU(x) #x
#endif
#ifndef QUH
#define QUH(x) QU(x)
#endif

#include "ApplMain.h"
#include "wireless_uart_peripheral.h"

#include "fsl_xcvr.h"

#if (cPWR_UsePowerDownMode)
#include "PWR_Interface.h"
#endif
#if (cPWR_FullPowerDownMode)
#include "fsl_xcvr.h"
#endif

#ifdef DUAL_MODE_APP
#include "app_dual_mode_switch.h"
#endif

/************************************************************************************
 *************************************************************************************
 * Private macros
 *************************************************************************************
 ************************************************************************************/

#ifndef ENABLE_DYNAMIC_ADVERTISING_DATA   
#define ENABLE_DYNAMIC_ADVERTISING_DATA          0
#endif

#ifndef ENABLE_UART_SERIAL_IF
#define ENABLE_UART_SERIAL_IF                    0
#endif

#define UART_READ_BUFFER_SIZE                    32

#if defined APP_SERIAL_LOGS_ENABLED && (ENABLE_UART_SERIAL_IF!=0)
#define APP_SERIAL_PRINT(string) Serial_Print(gAppSerMgrIf, string, gAllowToBlock_d);
#define APP_SERIAL_PRINT_HEX(value, nbBytes) Serial_PrintHex(gAppSerMgrIf, value , nbBytes, gPrtHexNoFormat_c);
#define APP_SERIAL_PRINT_DEC(value) Serial_PrintDec(gAppSerMgrIf, value)
#else
#define APP_SERIAL_PRINT(...)
#define APP_SERIAL_PRINT_HEX(...)
#define APP_SERIAL_PRINT_DEC(...)
#endif

#define gAppMaxServiceDiscoveryRetry 5
#define gAppMaxQueueSize             6

#define mAppUartBufferSize_c            gAttMaxWriteDataSize_d(gAttMaxMtu_c) /* Local Buffer Size */

#define mAppUartFlushIntervalInMs_c       (100)     /* Flush Timeout in Ms */

#define mBatteryLevelReportInterval_c   (10)    /* battery level report interval in seconds  */

#define mTemperatureReportInterval_c    (10)    /* temperature measurement interval in seconds */

#define mMeasurementTimerInterval       (2)     /* Timer measurement interval in seconds */

#define mBatteryMeasurementIntervalAfterConnectEvent (2)

#define mMaxBatteryMeasuremetToDoAfterConnect        (2)

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
    uint8_t nbDiscoveryRetry;
    bool_t mtuNegotiated;
} appPeerInfo_t;

typedef struct advState_tag
{
    bool_t advOn;
} advState_t;

/*! Bitfield for Adc measurements */
typedef enum {
    gAdcMeasureNone_c                   = 0,
    gAdcMeasureTemperature_c            = BIT0,  /* Temperature measurement to do */
    gAdcMeasureBatteryLevelFirst_c      = BIT1,  /* First battery level measurement for BLE config */
    gAdcMeasureBatteryLevel_c           = BIT2,  /* Battery level measurement */
} gAdcMeasure_t;

typedef struct
{
    uint32_t size;
    uint8_t *pBuffer;
} msgBuffer_t;

typedef struct
{
    msgBuffer_t *pMsgCurrent;
    anchor_t msgQueue; /* Message queue */
    uint32_t msgQueueSize;
} fullPacket_t;

/************************************************************************************
 *************************************************************************************
 * Private memory declarations
 *************************************************************************************
 ************************************************************************************/

static appPeerInfo_t maPeerInformation;

/* Adv Parameters */
static advState_t mAdvState;

#if gAppUsePrivacy_d
static bool_t startAppAfterContrPrivacyStateChange = FALSE;
#endif

static uint16_t mCharMonitoredHandles[1] = { (uint16_t)value_uart_stream };

/* Service Data*/
static wusConfig_t mWuServiceConfig;
static bool_t      mBasValidClientList[gAppMaxConnections_c] = {FALSE};
static basConfig_t mBasServiceConfig = {(uint16_t)service_battery, 0, mBasValidClientList, gAppMaxConnections_c};

static tmrTimerID_t mUartStreamFlushTimerId = gTmrInvalidTimerID_c;
static tmrTimerID_t mAppMeasurementTimerId = gTmrInvalidTimerID_c;
static uint32_t appBatteryMeasurementInterval = mBatteryLevelReportInterval_c;
static uint32_t appNbBatteryMeasurementToDoAfterConnect = 0;

static uint16_t mAppUartBufferSize = mAppUartBufferSize_c;
/* A packet to be sent after a command response is composed of multiple messages */
static fullPacket_t packetToSend;

static uint32_t mAdcMeasureToDo = 0;
static uint32_t mAppTemperatureTime = 0; /* time of last temperature measurement */
static uint32_t mAppBatteryLevelTime = 0; /* time of last battery level measurement */

#if ENABLE_UART_SERIAL_IF
uint8_t gAppSerMgrIf;
#endif

#if ENABLE_DYNAMIC_ADVERTISING_DATA
extern uint8_t *App_SetAddvertisingDataCb(uint32_t *advDataSize);
#endif

extern uint8_t gShellSerMgrIf;

#if gAppUsePrivacy_d
extern bool_t mbControllerPrivacyEnabled;
#endif
/************************************************************************************
 *************************************************************************************
 * Private functions prototypes
 *************************************************************************************
 ************************************************************************************/

/* Gatt and Att callbacks */
static void BleApp_AdvertisingCallback(gapAdvertisingEvent_t *pAdvertisingEvent);
static void BleApp_ConnectionCallback
(
    deviceId_t peerDeviceId,
    gapConnectionEvent_t *pConnectionEvent
);
static void BleApp_GattServerCallback
(
    deviceId_t deviceId,
    gattServerEvent_t *pServerEvent
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
    deviceId_t      peerDeviceId,
    servDiscEvent_t *pEvent
);

static void BleApp_Config(void);
static void BleApp_Advertise(void);

void BleApp_StateMachineHandler
(
    deviceId_t peerDeviceId,
    appEvent_t event
);

static void BleApp_StoreServiceHandles
(
    deviceId_t       peerDeviceId,
    gattService_t   *pService
);

static void AdcMeasurementCallback(void *pParam);
static bool_t AdcMeasurement_TimeCheck(uint32_t measureTodo, uint32_t* lastMeasureTime, uint32_t measureInterval);
static void MeasurementTimerCallback(void *pParam);

static bleResult_t BleApp_SendUartStream(uint8_t *pRecvStream, uint16_t streamSize);
static void UartStreamFlushTimerCallback(void *pData);

static void BleApp_SendStreamCb(void *pParam);
static void BleApp_ReceivedUartStream(deviceId_t peerDeviceId, uint8_t *pStream, uint16_t streamLength);
static bool_t BleApp_MsgQueue(msgBuffer_t *pMsg);

#if (cPWR_UsePowerDownMode)
static void BleAppDrv_InitCB(void);
static void BleAppDrv_DeInit(void);
#endif

#if ENABLE_UART_SERIAL_IF
/* Uart Tx/Rx Callbacks*/
static void Uart_RxCallBack(void *pData);
static void Uart_TxCallBack(void *pBuffer);
#endif

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
    BleAppDrv_Init(false);

  #if (cPWR_UsePowerDownMode)
    PWR_RegisterLowPowerExitCallback(BleAppDrv_InitCB);
    PWR_RegisterLowPowerEnterCallback(BleAppDrv_DeInit);
  #endif

}
/*! *********************************************************************************
 * \brief    Starts the BLE application.
 *
 ********************************************************************************** */
void BleApp_Start()
{
    APP_DEBUG_TRACE("%s \r\n", __FUNCTION__);

#ifdef LNT_MODE_APP
    int lnt_BleAdvDisabled();
    if (lnt_BleAdvDisabled())
        return;
#endif

    APP_SERIAL_PRINT("\n\rAdvertising...\n\r");
    gPairingParameters.localIoCapabilities = gIoDisplayOnly_c;
    BleApp_Advertise();
}

void BleApp_Stop(void)
{
    bool_t isBleRunning = FALSE;
#if (cPWR_UsePowerDownMode)
    PWR_DisallowDeviceToSleep();
#endif
    if (maPeerInformation.deviceId == gInvalidDeviceId_c)
    {
        /* Stop advertising if it is in progress */
        if (mAdvState.advOn)
        {
            isBleRunning = TRUE;
            Gap_StopAdvertising();
            APP_DEBUG_TRACE("Advertising will be stopped");
        }
    }
    else
    {
        isBleRunning = TRUE;
        APP_DEBUG_TRACE("Connection will be dropped");
        /* Send a disconnect to the connected peer */
        Gap_Disconnect(maPeerInformation.deviceId);

    }

#ifdef DUAL_MODE_APP
    if (!isBleRunning)
    {
        /* Notify the dual mode app */
        (void)App_PostCallbackMessage(dm_switch_processEvent, (void *) eBleNotRunningEvent);
    }
#endif
}

/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    events    Key event structure.
********************************************************************************** */
void BleApp_HandleKeys(key_event_t events)
{
    APP_DEBUG_TRACE("%s Evt=%d\r\n", __FUNCTION__, events);
    switch (events)
    {
        case gKBD_EventPressPB1_c:
        {
#ifdef APP_LED_INDICATION_ENABLED
            LED_StopFlashingAllLeds();
            Led1Flashing();
#endif
            BleApp_Start();
            break;
        }

        case gKBD_EventLongPB1_c:
        {

            if (maPeerInformation.deviceId != gInvalidDeviceId_c)
            {
                (void)Gap_Disconnect(maPeerInformation.deviceId);
            }

            break;
        }

        case gKBD_EventPressPB2_c:
            break;

        case gKBD_EventLongPB2_c:
            break;

        default:
        {
            ; /* No action required */
            break;
        }
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
    APP_DEBUG_TRACE("%s Evt=0x%x\r\n", __FUNCTION__, pGenericEvent->eventType);
    BleConnManager_GenericEvent(pGenericEvent);

    switch (pGenericEvent->eventType)
    {
        case gInitializationComplete_c:
        {
            BleApp_Config();
#if gAppUsePrivacy_d
            /* Start auto advertising only if the controller privacy is not enabled
               Otherwise wait the gControllerPrivacyStateChanged_c event before starting adv
             */
            if (!mbControllerPrivacyEnabled)
#endif
            {
                BleApp_Start();
            }
#if gAppUsePrivacy_d
            else
            {
                startAppAfterContrPrivacyStateChange = TRUE;
            }
#endif
        }
        break;
#if gAppUsePrivacy_d
        case gControllerPrivacyStateChanged_c:
        {
            if (startAppAfterContrPrivacyStateChange)
            {
                startAppAfterContrPrivacyStateChange = FALSE;
                BleApp_Start();
            }
        }
        break;
#endif
        case gAdvertisingParametersSetupComplete_c:
        {

#if ENABLE_DYNAMIC_ADVERTISING_DATA
            uint32_t advDataSize = 0;
            gAppAdvertisingData.aAdStructures[2].aData = App_SetAddvertisingDataCb(&advDataSize);
            gAppAdvertisingData.aAdStructures[2].length = advDataSize;
#endif
            (void)Gap_SetAdvertisingData(&gAppAdvertisingData, &gAppScanRspData);
        }
        break;

        case gAdvertisingDataSetupComplete_c:
        {
            (void)App_StartAdvertising(BleApp_AdvertisingCallback, BleApp_ConnectionCallback);
        }
        break;

        case gAdvertisingSetupFailed_c:
        {
            panic(0,0,0,0);
        }

        case gTxEntryAvailable_c:
        {
            if (maPeerInformation.appState == mAppRunning_c)
                App_PostCallbackMessage(BleApp_SendStreamCb, NULL);
            break;
        }

        default:
        {
            ; /* No action required */
        }
        break;
    }
}
/*! *********************************************************************************
* \brief        Send through Bluetooth data from serial manager.
*
* \param[in]    pData    Pointer to data buffer.
* \param[in]    size     size in bytes of the data buffer.
********************************************************************************** */
uint32_t Serial_CustomSendData(uint8_t *pData, uint32_t size)
{
    APP_DEBUG_TRACE("%s size = %d, st = %d\r\n", __FUNCTION__, size, maPeerInformation.appState);
    msgBuffer_t *pMsg = NULL;
    uint32_t NoOfBytes = size;
    uint32_t remainingSpace = 0;
    uint32_t nbBytesToCopy = 0;
    uint8_t status = 0;
    uint8_t * pWrite; /* write pointer to buffer */
    uint8_t *pRead = pData;

    TMR_StopTimer(mUartStreamFlushTimerId);

#if ENABLE_UART_SERIAL_IF

    uint8_t* pBuffer = NULL;

    /* Allocate buffer for asynchronous write */
    pBuffer = MEM_BufferAlloc(size);
    if (pBuffer != NULL)
    {
        FLib_MemCpy(pBuffer,pData,size);
        Serial_AsyncWrite(gAppSerMgrIf, pBuffer, size, Uart_TxCallBack, pBuffer);
    }
#endif

    if (maPeerInformation.appState == mAppRunning_c)
    {
        while (NoOfBytes > 0)
        {
            if (packetToSend.pMsgCurrent != NULL)
            {
                APP_DEBUG_TRACE("packetToSend.pMsgCurrent is not NULL\r\n");
                pMsg = packetToSend.pMsgCurrent;
                remainingSpace = mAppUartBufferSize - pMsg->size;
            }
            else
            {
                pMsg = MEM_BufferAlloc(sizeof(msgBuffer_t));
                if (pMsg == NULL)
                {
                    status = 1;
                    APP_DEBUG_TRACE("Alloc FAILURE\r\n");
                    break;
                }
                pMsg->pBuffer = MEM_BufferAlloc(mAppUartBufferSize);
                if (pMsg->pBuffer == NULL)
                {
                    status = 1;
                    APP_DEBUG_TRACE("Alloc FAILURE\r\n");
                    break;
                }
                pMsg->size = 0;
                remainingSpace = mAppUartBufferSize;
            }
            pWrite = &pMsg->pBuffer[pMsg->size];
            nbBytesToCopy = MIN(remainingSpace, NoOfBytes);
            FLib_MemCpy(pWrite, pRead, nbBytesToCopy);
            pRead += nbBytesToCopy;
            pMsg->size += nbBytesToCopy;
            if (pMsg->size == mAppUartBufferSize)
            {
                packetToSend.pMsgCurrent = NULL;
                /* Queue the msg */
                if (BleApp_MsgQueue(pMsg))
                    (void)App_PostCallbackMessage(BleApp_SendStreamCb, NULL);
            }
            else
            {
                packetToSend.pMsgCurrent = pMsg;
            }
            NoOfBytes -= nbBytesToCopy;
        }
        if (packetToSend.pMsgCurrent != NULL)
        {
            /* Start the flush timer */
            TMR_StartLowPowerTimer(mUartStreamFlushTimerId,
                                   gTmrLowPowerSingleShotMillisTimer_c,
                                   mAppUartFlushIntervalInMs_c,
                                   UartStreamFlushTimerCallback, NULL);
        }

    }
    else
    {
        if (packetToSend.pMsgCurrent != NULL)
        {
            MEM_BufferFree(packetToSend.pMsgCurrent->pBuffer);
            MEM_BufferFree(packetToSend.pMsgCurrent);
            packetToSend.pMsgCurrent = NULL;
        }
    }

    Serial_CustomSendCompleted(gShellSerMgrIf);

    return status;
}

/*! *********************************************************************************
* \brief        Configures BLE Stack after initialization. Usually used for
*               configuring advertising, scanning, white list, services, et al.
*
********************************************************************************** */
void BleAppDrv_Init(bool reinit)
{
#if (cPWR_FullPowerDownMode)
    if ( reinit )
    {
        /* check if any ADC measurement need to be done  */
        AdcMeasurement_TimeCheck(gAdcMeasureTemperature_c, &mAppTemperatureTime, mTemperatureReportInterval_c);

        if(maPeerInformation.deviceId != gInvalidDeviceId_c)
            AdcMeasurement_TimeCheck(gAdcMeasureBatteryLevel_c, &mAppBatteryLevelTime, mBatteryLevelReportInterval_c);
#if ENABLE_UART_SERIAL_IF
        SerialInterface_Reinit(gAppSerMgrIf);

        Serial_SetBaudRate(gAppSerMgrIf, gUARTBaudRate115200_c);
#endif

#if gKeyBoardSupported_d
        KBD_PrepareExitLowPower();
#endif
#if gLEDSupported_d
        LED_PrepareExitLowPower();
#endif
    }
    else
#endif
    {
        (void) reinit;

#if ENABLE_UART_SERIAL_IF

        /* UI */
        Serial_InitManager();

        /* Register Serial Manager interface */
        Serial_InitInterface(&gAppSerMgrIf, gSerialMgrUsart_c, 0);
        Serial_SetBaudRate(gAppSerMgrIf, gUARTBaudRate115200_c);

        /* Install Controller Events Callback handler */
        Serial_SetRxCallBack(gAppSerMgrIf, Uart_RxCallBack, NULL);
#endif

        packetToSend.pMsgCurrent=NULL;
        packetToSend.msgQueueSize = 0;
        MSG_InitQueue(&packetToSend.msgQueue);
    }
}

bool_t BleApp_CanGotoSleep()
{
    return (appNbBatteryMeasurementToDoAfterConnect == 0) && !TMR_IsTimerActive(mUartStreamFlushTimerId);
}

void BleApp_StopRunningTimers()
{
    TMR_StopTimer(mUartStreamFlushTimerId);
    TMR_StopTimer(mAppMeasurementTimerId);
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

#if (cPWR_UsePowerDownMode)
static void BleAppDrv_InitCB(void)
{
    BleAppDrv_Init(true);
}

static void BleAppDrv_DeInit(void)
{
    /* DeInitialize application support for drivers */
    /* configure pins for power down mode */
    BOARD_SetPinsForPowerMode();
}
#endif  /*cPWR_UsePowerDownMode */

static void BleApp_Config(void)
{
    APP_DEBUG_TRACE("%s\r\n", __FUNCTION__);

    /* Common GAP configuration */
    BleConnManager_GapCommonConfig();

    /* Register for callbacks */
    (void)App_RegisterGattServerCallback(BleApp_GattServerCallback);
    (void)App_RegisterGattClientProcedureCallback(BleApp_GattClientCallback);
    (void)GattServer_RegisterHandlesForWriteNotifications(NumberOfElements(mCharMonitoredHandles), mCharMonitoredHandles);
    BleServDisc_RegisterCallback(BleApp_ServiceDiscoveryCallback);
    Gap_SetTxPowerLevel(gAdvertisingPowerLeveldBm_c, gTxPowerAdvChannel_c);
    Gap_SetTxPowerLevel(gConnectPowerLeveldBm_c, gTxPowerConnChannel_c);

    maPeerInformation.appState = mAppIdle_c;
    maPeerInformation.deviceId = gInvalidDeviceId_c;
    maPeerInformation.clientInfo.hService = gGattDbInvalidHandleIndex_d;
    maPeerInformation.clientInfo.hUartStream = gGattDbInvalidHandleIndex_d;
    maPeerInformation.mtuNegotiated = FALSE;
    maPeerInformation.nbDiscoveryRetry = 0;

    mAdvState.advOn = FALSE;

    /* Start services */
    (void)Wus_Start(&mWuServiceConfig);

    /* do battery level measurement */
    /* init ADC */
    BOARD_InitAdc();
    mAdcMeasureToDo |= gAdcMeasureBatteryLevelFirst_c;
    App_PostCallbackMessage(AdcMeasurementCallback, NULL);
    mAppMeasurementTimerId = TMR_AllocateTimer();
    TMR_StartLowPowerTimer(mAppMeasurementTimerId, gTmrLowPowerIntervalMillisTimer_c,
                                       TmrSeconds(mMeasurementTimerInterval), MeasurementTimerCallback, &appNbBatteryMeasurementToDoAfterConnect);

    /* Allocate application timer */
    mUartStreamFlushTimerId = TMR_AllocateTimer();

}

/*! *********************************************************************************
* \brief        Configures GAP Advertise parameters. Advertise will start after
*               the parameters are set.
*
********************************************************************************** */
static void BleApp_Advertise(void)
{
    APP_DEBUG_TRACE("%s\r\n", __FUNCTION__);
    /* Set advertising parameters*/
    (void)Gap_SetAdvertisingParameters(&gAdvParams);

#if (cPWR_UsePowerDownMode)
    PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);
    PWR_AllowDeviceToSleep();
#endif
}


/*! *********************************************************************************
* \brief        Handles BLE Advertising callback from host stack.
*
* \param[in]    pAdvertisingEvent    Pointer to gapAdvertisingEvent_t.
********************************************************************************** */
static void BleApp_AdvertisingCallback(gapAdvertisingEvent_t *pAdvertisingEvent)
{
    APP_DEBUG_TRACE("%s Evt=%d\r\n", __FUNCTION__, pAdvertisingEvent->eventType);
    switch (pAdvertisingEvent->eventType)
    {
        case gAdvertisingStateChanged_c:
        {
            mAdvState.advOn = !mAdvState.advOn;
#ifdef APP_LED_INDICATION_ENABLED
            LED_StopFlashingAllLeds();
            Led1Flashing();
#endif

            if (!mAdvState.advOn)
            {
#ifdef APP_LED_INDICATION_ENABLED
                Led2Flashing();
                Led3Flashing();
                Led4Flashing();
#endif
#ifdef DUAL_MODE_APP
                /* notify the dual mode task */
                (void)App_PostCallbackMessage(dm_switch_processEvent, (void *) eBleAdvStopEvent);
#endif
            }
        }
        break;

        case gAdvertisingCommandFailed_c:
        {
            APP_SERIAL_PRINT("\n\rAdvertising Command Failed.\n\r");
            APP_WARNING_TRACE("Advertising Command Failed\r\n");
            //panic(0,0,0,0);
#if (cPWR_UsePowerDownMode)
            PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);
            PWR_AllowDeviceToSleep();
#endif
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
static void BleApp_ConnectionCallback(deviceId_t peerDeviceId, gapConnectionEvent_t *pConnectionEvent)
{
    APP_DEBUG_TRACE("%s Evt=0x%x\r\n", __FUNCTION__, pConnectionEvent->eventType);
    switch (pConnectionEvent->eventType)
    {
        case gConnEvtConnected_c:
        {
            /* Save peer device ID */
            maPeerInformation.deviceId = peerDeviceId;

            /* Advertising stops when connected */
            mAdvState.advOn = FALSE;

            /* Subscribe client*/
            (void)Wus_Subscribe(peerDeviceId);
            (void)Bas_Subscribe(&mBasServiceConfig, peerDeviceId);
#ifdef APP_LED_INDICATION_ENABLED
            /* UI */
            LED_StopFlashingAllLeds();
            Led1On();
#endif

            appBatteryMeasurementInterval = mBatteryMeasurementIntervalAfterConnectEvent;
            appNbBatteryMeasurementToDoAfterConnect = mMaxBatteryMeasuremetToDoAfterConnect;
            if(!TMR_IsTimerActive(mAppMeasurementTimerId))
            {
                (void)TMR_StartLowPowerTimer(mAppMeasurementTimerId, gTmrLowPowerIntervalMillisTimer_c,
                                             TmrSeconds(mMeasurementTimerInterval), MeasurementTimerCallback, &appNbBatteryMeasurementToDoAfterConnect);
            }

            APP_SERIAL_PRINT("Connected to device ");
            APP_SERIAL_PRINT_DEC(peerDeviceId);

#if (cPWR_UsePowerDownMode)
            PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);
            PWR_AllowDeviceToSleep();
#endif
        }

#if gAppUsePairing_d
            /* In case of Pairing/bonding enabled wait the end of the pairing process */
#else
            /* run the state machine */
            BleApp_StateMachineHandler(peerDeviceId, mAppEvt_PeerConnected_c);
#endif
        break;
        case gConnEvtAuthenticationRejected_c:
        {
            APP_DEBUG_TRACE("Authentication Rejected raison = 0x%x\r\n", pConnectionEvent->eventData.authenticationRejectedEvent.rejectReason);
        }
        break;
        case gConnEvtDisconnected_c:
        {
            APP_DEBUG_TRACE("Disconnect event received");
            APP_SERIAL_PRINT("Disconnected from device ");
            APP_SERIAL_PRINT_DEC(peerDeviceId);
            APP_SERIAL_PRINT(".\n\r");

            maPeerInformation.appState = mAppIdle_c;
            maPeerInformation.deviceId = gInvalidDeviceId_c;
            maPeerInformation.clientInfo.hService = gGattDbInvalidHandleIndex_d;
            maPeerInformation.clientInfo.hUartStream = gGattDbInvalidHandleIndex_d;
            maPeerInformation.mtuNegotiated = FALSE;
            maPeerInformation.nbDiscoveryRetry = 0;

            appBatteryMeasurementInterval = mBatteryLevelReportInterval_c;
            appNbBatteryMeasurementToDoAfterConnect = 0;

            /* Unsubscribe client */
            (void)Wus_Unsubscribe();
            (void)Bas_Unsubscribe(&mBasServiceConfig, peerDeviceId);

            /* Reset Service Discovery to be sure*/
            BleServDisc_Stop(peerDeviceId);

#ifdef APP_LED_INDICATION_ENABLED
            /* UI */
            LED_TurnOffAllLeds();
#if (!cPWR_UsePowerDownMode)
            LED_StartFlash(LED_ALL);
#endif
#endif

            /* recalculate minimum of maximum MTU's of all connected devices */
            mAppUartBufferSize                       = mAppUartBufferSize_c;

            while (packetToSend.msgQueueSize > 0)
            {
                msgBuffer_t *pMsg = MSG_DeQueue(&packetToSend.msgQueue);
                if (pMsg != NULL)
                {
                    MEM_BufferFree(pMsg->pBuffer);
                    MEM_BufferFree(pMsg);
                    packetToSend.msgQueueSize--;
                }                
            }
            if (packetToSend.pMsgCurrent != NULL)
            {
                MEM_BufferFree(packetToSend.pMsgCurrent->pBuffer);
                MEM_BufferFree(packetToSend.pMsgCurrent);
                packetToSend.pMsgCurrent = NULL;
                Serial_CustomSendCompleted(gShellSerMgrIf);
            }

#ifndef DUAL_MODE_APP
            BleApp_Start();
#else
            (void)App_PostCallbackMessage(dm_switch_processEvent, (void *) eBleDisconnectionEvent);
#endif
        }
        break;

#if gAppUsePairing_d
        case gConnEvtEncryptionChanged_c:
        {
            if (pConnectionEvent->eventData.encryptionChangedEvent.newEncryptionState)
            {
                BleApp_StateMachineHandler(peerDeviceId,
                                           mAppEvt_PairingComplete_c);
            }
            else
            {
                APP_DEBUG_TRACE("gConnEvtEncryptionChanged_c newEncryptionState error\r\n");
                (void)Gap_Disconnect(peerDeviceId);
            }
        }
        break;
        case gConnEvtPairingComplete_c:
        {
            if (pConnectionEvent->eventData.pairingCompleteEvent.pairingSuccessful)
            {
                BleApp_StateMachineHandler(peerDeviceId,
                                           mAppEvt_PairingComplete_c);
            }
            else
            {
                APP_DEBUG_TRACE("Fail raison = 0x%x\r\n", __FUNCTION__, pConnectionEvent->eventData.pairingCompleteEvent.pairingCompleteData.failReason);
                (void)Gap_Disconnect(peerDeviceId);
            }
        }
        break;
#endif /* gAppUsePairing_d */
        case gConnEvtParameterUpdateComplete_c:
        {
            APP_DEBUG_TRACE("Connection interval changed (st=%d) connInterval = %d connLatency= %d, supervisionTimeout=%d\r\n", pConnectionEvent->eventData.connectionUpdateComplete.status,  pConnectionEvent->eventData.connectionUpdateComplete.connInterval, pConnectionEvent->eventData.connectionUpdateComplete.connLatency, pConnectionEvent->eventData.connectionUpdateComplete.supervisionTimeout);
        };

        default:
        {
            ; /* No action required */
        }
        break;
    }

    BleConnManager_GapPeripheralEvent(peerDeviceId, pConnectionEvent);
}

static void BleApp_ServiceDiscoveryCallback(deviceId_t peerDeviceId, servDiscEvent_t *pEvent)
{
    APP_DEBUG_TRACE("%s Evt=%d\r\n", __FUNCTION__, pEvent->eventType);
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
                if (gGattDbInvalidHandleIndex_d != maPeerInformation.clientInfo.hService)
                {
                    BleApp_StateMachineHandler(peerDeviceId,
                                               mAppEvt_ServiceDiscoveryComplete_c);
                }
                else
                {
                    APP_DEBUG_TRACE("Service Discovery not found\r\n", __FUNCTION__);
                    BleApp_StateMachineHandler(peerDeviceId,
                                               mAppEvt_ServiceDiscoveryNotFound_c);
                }
            }
            else
            {
                APP_DEBUG_TRACE("%s Evt=gDiscoveryFinished_c failure\r\n", __FUNCTION__);
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
static void BleApp_GattClientCallback(
    deviceId_t              serverDeviceId,
    gattProcedureType_t     procedureType,
    gattProcedureResult_t   procedureResult,
    bleResult_t             error
)
{
    APP_DEBUG_TRACE("%s, procedureType = %d, procedureResult=%d, error = 0x%x\r\n", __FUNCTION__, procedureType, procedureResult, error);
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
 * \param[in]    deviceId        Client peer device ID.
 * \param[in]    pServerEvent    Pointer to gattServerEvent_t.
 ********************************************************************************** */
static void BleApp_GattServerCallback(
    deviceId_t deviceId,
    gattServerEvent_t *pServerEvent)
{
    uint16_t tempMtu = 0;
    APP_DEBUG_TRACE("%s Evt=%d\r\n", __FUNCTION__, pServerEvent->eventType);
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

            APP_DEBUG_TRACE("mAppUartBufferSize = %d tempMtu = %d\n", mAppUartBufferSize, tempMtu);
            maPeerInformation.mtuNegotiated = TRUE;
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
static void BleApp_StoreServiceHandles(deviceId_t peerDeviceId, gattService_t *pService)
{
    APP_DEBUG_TRACE("%s\r\n", __FUNCTION__);
    /* Found Wireless UART Service */
    maPeerInformation.clientInfo.hService = pService->startHandle;

    if (pService->cNumCharacteristics > 0U && pService->aCharacteristics != NULL)
    {
        /* Found Uart Characteristic */
        maPeerInformation.clientInfo.hUartStream = pService->aCharacteristics[0].value.handle;
    }
}

static bleResult_t BleApp_SendUartStream(uint8_t *pRecvStream, uint16_t streamSize)
{
    APP_DEBUG_TRACE("%s streamSize = %d\r\n", __FUNCTION__, streamSize);
    gattCharacteristic_t characteristic = {gGattCharPropNone_c, {0}, 0, 0};
    bleResult_t result = gBleUnavailable_c;

    if (gInvalidDeviceId_c != maPeerInformation.deviceId &&
        mAppRunning_c == maPeerInformation.appState)
    {
        characteristic.value.handle = maPeerInformation.clientInfo.hUartStream;
        result = GattClient_WriteCharacteristicValue(maPeerInformation.deviceId, &characteristic,
                                     streamSize, pRecvStream, TRUE, FALSE, FALSE, NULL);
        APP_DEBUG_TRACE("%s GattClient_WriteCharacteristicValue = %d\r\n", __FUNCTION__, result);
    }
    return result;
}

void BleApp_StateMachineHandler(deviceId_t peerDeviceId, appEvent_t event)
{
    uint16_t tempMtu = 0;
    bleResult_t result;

    union
    {
        uint8_t     *pUuidArray;
        bleUuid_t   *pUuidObj;
    } temp; /* MISRA rule 11.3 */

    temp.pUuidArray = uuid_service_wireless_uart;

    /* invalid client information */
    if (gInvalidDeviceId_c == maPeerInformation.deviceId)
    {
        return;
    }

    switch (maPeerInformation.appState)
    {
        case mAppIdle_c:
        {
            if (event == mAppEvt_PeerConnected_c || event == mAppEvt_PairingComplete_c)
            {
                /* Moving to Service Discovery State*/
                maPeerInformation.appState = mAppServiceDisc_c;

                /* Start Service Discovery*/
                result = BleServDisc_FindService(peerDeviceId,
                                              gBleUuidType128_c,
                                              temp.pUuidObj);
                APP_DEBUG_TRACE("BleServDisc_FindService result = %d\r\n", result);
                (void) result;
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

                APP_DEBUG_TRACE("%s mAppUartBufferSize=%d, tempMtu=%d\r\n", __FUNCTION__,mAppUartBufferSize,tempMtu);
                maPeerInformation.mtuNegotiated = TRUE;
                maPeerInformation.appState = mAppRunning_c;
            }
            else
            {
                if (event == mAppEvt_GattProcError_c)
                {
                    APP_DEBUG_TRACE("%s mAppExchangeMtu_c error\r\n", __FUNCTION__,mAppUartBufferSize,tempMtu);
                    (void)Gap_Disconnect(peerDeviceId);
                }
            }
        }
        break;

        case mAppServiceDisc_c:
        {
            if (event == mAppEvt_ServiceDiscoveryComplete_c)
            {
#if gAppUseBonding_d
                /* Write data in NVM */
                (void)Gap_SaveCustomPeerInformation(maPeerInformation.deviceId,
                                                    (void *) &maPeerInformation.clientInfo, 0,
                                                    sizeof(wucConfig_t));
#endif
                if (!maPeerInformation.mtuNegotiated)
                {
                    maPeerInformation.appState = mAppExchangeMtu_c;
                    (void)GattClient_ExchangeMtu(peerDeviceId, gAttMaxMtu_c);
                }
                else
                {
                    /* Moving to Running State*/
                    maPeerInformation.appState = mAppRunning_c;
                }
            }
            else if (event == mAppEvt_ServiceDiscoveryNotFound_c
                        || event == mAppEvt_ServiceDiscoveryFailed_c)
            {
                BleServDisc_Stop(peerDeviceId);
                if (maPeerInformation.nbDiscoveryRetry < gAppMaxServiceDiscoveryRetry)
                {
                    maPeerInformation.nbDiscoveryRetry++;
                    /* Re-start Service Discovery*/
                    result = BleServDisc_FindService(peerDeviceId,
                                                gBleUuidType128_c,
                                                temp.pUuidObj);
                    APP_DEBUG_TRACE("BleServDisc_FindService result = %d\r\n", result);
                }
                else
                {
                    APP_DEBUG_TRACE("gAppMaxServiceDiscoveryRetry (%d)reach\r\n", gAppMaxServiceDiscoveryRetry);
                    (void)Gap_Disconnect(peerDeviceId);
                }
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


static void BleApp_SendStreamCb(void *pParam)
{
    APP_DEBUG_TRACE("%s\r\n", __FUNCTION__);
    if (packetToSend.msgQueueSize>0)
    {
        msgBuffer_t *pMsg = MSG_GetHead(&packetToSend.msgQueue);
        if (BleApp_SendUartStream(pMsg->pBuffer, pMsg->size) == gBleOverflow_c)
        {
            /* Wait for retry that would be triggered by an event from gTxEntryAvailable_c */
            return;
        }
        pMsg = MSG_DeQueue(&packetToSend.msgQueue);
        MEM_BufferFree(pMsg->pBuffer);
        MEM_BufferFree(pMsg);
        packetToSend.msgQueueSize--;
#if (cPWR_UsePowerDownMode)
        if (packetToSend.msgQueueSize == 0)
        {
            /* Allow device to sleep */
            PWR_AllowDeviceToSleep();
        }
#endif
    }
}

static void BleApp_ReceivedUartStream(deviceId_t peerDeviceId, uint8_t *pStream, uint16_t streamLength)
{
    APP_DEBUG_TRACE("%s streamLength = %d\r\n", __FUNCTION__, streamLength);
     int i=0;
    for (i=0;i<streamLength;i++)
    {
        APP_DEBUG_TRACE("%c", pStream[i]);
    }
#if ENABLE_UART_SERIAL_IF

    uint8_t *pBuffer = NULL;
    /* Allocate buffer for asynchronous write */
    pBuffer = MEM_BufferAlloc(streamLength);
    if (pBuffer != NULL)
    {
        FLib_MemCpy(pBuffer,pStream,streamLength);
        Serial_AsyncWrite(gAppSerMgrIf, pBuffer, streamLength, Uart_TxCallBack, pBuffer);
    }
#endif
    Serial_CustomReceiveData(gShellSerMgrIf, pStream, streamLength);
}

static void UartStreamFlushTimerCallback(void *pData)
{
    APP_DEBUG_TRACE("%s\r\n", __FUNCTION__);
    /* Queue the msg */
    if (BleApp_MsgQueue(packetToSend.pMsgCurrent))
        (void)App_PostCallbackMessage(BleApp_SendStreamCb, NULL);
    packetToSend.pMsgCurrent = NULL;
    TMR_StopTimer(mUartStreamFlushTimerId);
}

static bool_t BleApp_MsgQueue(msgBuffer_t *pMsg)
{
    bool_t msgHasBeenQueued = FALSE;
    if (packetToSend.msgQueueSize < gAppMaxQueueSize)
    {
        MSG_Queue(&packetToSend.msgQueue, pMsg);
        packetToSend.msgQueueSize++;
        msgHasBeenQueued = TRUE;
#if (cPWR_UsePowerDownMode)
        if (packetToSend.msgQueueSize == 1)
        {
            /* Disable sleep */
            PWR_DisallowDeviceToSleep();
            APP_DEBUG_TRACE("PWR_DisallowDeviceToSleep\r\n");
        }
#endif
    }
    else
    {
        /* Queue is full, free the msg */
        if (pMsg->pBuffer != NULL)
            MEM_BufferFree(pMsg->pBuffer);
        if (pMsg != NULL)
            MEM_BufferFree(pMsg);
    }
    return msgHasBeenQueued;
}

#if ENABLE_UART_SERIAL_IF
/*! *********************************************************************************
* \brief        Handles UART Receive callback.
*
* \param[in]    pData        Parameters.
********************************************************************************** */
static void Uart_RxCallBack(void *pData)
{

    uint16_t bytesCount = 0;
    uint16_t bytesRead = 0;
    uint8_t  read_buffer[UART_READ_BUFFER_SIZE];
    
    (void)Serial_RxBufferByteCount(gAppSerMgrIf, &bytesCount);

    do
    {
        Serial_Read(gAppSerMgrIf, (uint8_t*)read_buffer, UART_READ_BUFFER_SIZE, &bytesRead );
        Serial_CustomReceiveData(gShellSerMgrIf, read_buffer, bytesRead);
        bytesCount -= bytesCount;

    }while(bytesCount > UART_READ_BUFFER_SIZE);
}

/*! *********************************************************************************
* \brief        Handles UART Transmit callback.
*
* \param[in]    pData        Parameters.
********************************************************************************** */
static void Uart_TxCallBack(void *pBuffer)
{
    (void)MEM_BufferFree(pBuffer);
}
#endif

/*! *********************************************************************************
* \brief        Check if it is time for a specific ADC measurement
*
* \param[in]    measureTodo         the ADC measurement to check if it is time to do it
* \param[in]    lastMeasureTime     Last the ADC measurement was done
* \param[in]    measureInterval     Measurement time interval
********************************************************************************** */

static bool_t AdcMeasurement_TimeCheck(uint32_t measureTodo, uint32_t* lastMeasureTime, uint32_t measureInterval)
{
    uint32_t        current_time;
    bool_t          do_measurement = FALSE;

    APP_DBG_LOG("");
    current_time = TMR_RTCGetTimestampSeconds();
    /* check if it is time for  measurement */
    if(current_time > *lastMeasureTime)
    {
        if((current_time - *lastMeasureTime) >= measureInterval)
            do_measurement = TRUE;
    }
    else if(current_time >= (*lastMeasureTime + measureInterval))
    {
        do_measurement = TRUE;
    }

    if(do_measurement)
    {
        *lastMeasureTime = current_time;
        if(!mAdcMeasureToDo)
        {
            /* init ADC if still no measurement scheduled*/
            BOARD_InitAdc();
            App_PostCallbackMessage(AdcMeasurementCallback, NULL);
        }
        mAdcMeasureToDo |= measureTodo;
    }
    return do_measurement;
}

/*! *********************************************************************************
* \brief        Handles Adc measurement callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void AdcMeasurementCallback(void * pParam)
{
    BOARD_EnableAdc();

    APP_DBG_LOG("");
    /* do the measurements required */
    if(mAdcMeasureToDo & gAdcMeasureBatteryLevelFirst_c)
    {
        /* Assume that when reaching this point , at least 230us needed for ADC init has elapsed */
        mBasServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
        Bas_Start(&mBasServiceConfig);
    }

    if ( mAdcMeasureToDo & gAdcMeasureTemperature_c)
    {
        int32_t temperature = BOARD_GetTemperature();
        /* Update Temperature in XCVR for calibration purpose */
        XCVR_TemperatureUpdate(temperature);
    }

    if(maPeerInformation.deviceId != gInvalidDeviceId_c)
    {
        if ( mAdcMeasureToDo & gAdcMeasureBatteryLevel_c)
        {
            mBasServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
            Bas_RecordBatteryMeasurement(&mBasServiceConfig);
        }
    }
    /* all pending measurements are done */
    mAdcMeasureToDo  = 0;
    BOARD_DeInitAdc();
}

/*! *********************************************************************************
* \brief        Handles measurement timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void MeasurementTimerCallback(void * pParam)
{
    uint32_t * pValueFound = (uint32_t *)pParam;
    bool_t batteryMeasurmentDone = FALSE;

    /* check if any ADC measurement need to be done  */
    AdcMeasurement_TimeCheck(gAdcMeasureTemperature_c, &mAppTemperatureTime, mTemperatureReportInterval_c);

    if(maPeerInformation.deviceId != gInvalidDeviceId_c)
        batteryMeasurmentDone = AdcMeasurement_TimeCheck(gAdcMeasureBatteryLevel_c, &mAppBatteryLevelTime, appBatteryMeasurementInterval);
    
    if (batteryMeasurmentDone && pValueFound != NULL && *pValueFound != 0)
    {
        *pValueFound = *pValueFound -1;
        if (*pValueFound == 0)
           appBatteryMeasurementInterval =  mBatteryLevelReportInterval_c;
    }
}
/*! *********************************************************************************
 * @}
 ********************************************************************************** */
