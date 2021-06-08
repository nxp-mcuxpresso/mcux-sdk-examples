/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Framework / Drivers */
#include "FunctionLib.h"
#include "Panic.h"
#include "SerialManager.h"
#include "MemManager.h"

/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"
#include "gatt_database_dynamic.h"
#include "fsci_ble_interface.h"

/* Profile / Services */
#include "wireless_uart_interface.h"
#include "battery_interface.h"
#include "device_info_interface.h"
/* Wrappers */
#include "ble_conn_manager.h"
#include "ble_service_discovery.h"

#include "fsl_gpio.h"
#include "rpmsg_lite.h"
#include "rpmsg_queue.h"
#include "rpmsg_ns.h"

#include "app_srtm.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "ApplMain.h"
#include "app_config.h"

#include "ble_general.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define APP_BLE_BRIDGE_RPMSG_NS_ANNOUNCE_STRING ("rpmsg-virtual-tty-channel")


#define mAppUartBufferSize_c 20 /* Match App on mobile */

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
    mAppEvt_ServiceDiscoveryFailed_c,
    mAppEvt_GattProcComplete_c,
    mAppEvt_GattProcError_c
} appEvent_t;

typedef enum appState_tag
{
    mAppIdle_c,
    mAppServiceDisc_c,
    mAppRunning_c
} appState_t;

typedef struct appPeerInfo_tag
{
    deviceId_t deviceId;
    bool_t isBonded;
    wucConfig_t clientInfo;
    appState_t appState;
} appPeerInfo_t;

typedef struct advState_tag
{
    bool_t advOn;
} advState_t;

typedef struct _rpmsg_message
{
    uint8_t *buffer;
    uint32_t bufferLen;
} rpmsg_message_t;

/************************************************************************************
 *************************************************************************************
 * Private memory declarations
 *************************************************************************************
 ************************************************************************************/
static struct rpmsg_lite_instance *myRpmsg;
static struct rpmsg_lite_endpoint *myEpt;
static rpmsg_queue_handle myQueue;
static uint32_t a7Addr;
static SemaphoreHandle_t mutex;
static SemaphoreHandle_t startSig;
static SemaphoreHandle_t stopSig;
static volatile bool runRpmsg;

static appPeerInfo_t mPeerInformation;
static volatile bool bleConnectedFlag = false;

#if gAppUseBonding_d
static bool_t mRestoringBondedLink = FALSE;
#endif

/* Adv Parmeters */
static advState_t mAdvState;

/* Service Data*/
static wusConfig_t mWuServiceConfig;
static basConfig_t basServiceConfig = {gGattDbInvalidHandle_d, 0};
static uint16_t cpHandles[1]        = {gGattDbInvalidHandle_d};

static const gFsciSerialConfig_t mFsciSerials[] = {
    /* Baudrate,            interface type,            channel No,                   virtual interface */
    {gAppFSCIHostInterfaceBaud_d, gAppFSCIHostInterfaceType_d, gAppFSCIHostInterfaceId_d, 0}};

static uint8_t mMsg[mAppUartBufferSize_c];

extern gapServiceSecurityRequirements_t serviceSecurity[];
extern const uint8_t uuid_service_wireless_uart[];

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
void BleApp_Init(void);
void BleApp_Start(void);
void BleApp_GenericCallback(gapGenericEvent_t *pGenericEvent);

#ifdef __cplusplus
}
#endif

/* Gatt and Att callbacks */
static void BleApp_AdvertisingCallback(gapAdvertisingEvent_t *pAdvertisingEvent);
static void BleApp_CreateGattDb(void);
static void BleApp_ConnectionCallback(deviceId_t peerDeviceId, gapConnectionEvent_t *pConnectionEvent);

static void BleApp_GattServerCallback(deviceId_t deviceId, gattServerEvent_t *pServerEvent);

static void BleApp_GattClientCallback(deviceId_t serverDeviceId,
                                      gattProcedureType_t procedureType,
                                      gattProcedureResult_t procedureResult,
                                      bleResult_t error);

static void BleApp_ServiceDiscoveryCallback(deviceId_t deviceId, servDiscEvent_t *pEvent);

static void BleApp_Config(void);
static void BleApp_Advertise(void);

static void BleApp_StateMachineHandler(deviceId_t peerDeviceId, uint8_t event);

static void BleApp_StoreService(deviceId_t peerDeviceId, gattService_t *pService);

static void BleApp_ReceivedUartStream(uint8_t *pStream, uint16_t streamLength);

/*******************************************************************************
 * Code
 ******************************************************************************/
extern void main_task(uint32_t param);

static void BleTask(void *param)
{
    main_task((uint32_t)param);
}

static void BleApp_FlushUartStream(void *pParam)
{
    rpmsg_message_t *message = (rpmsg_message_t *)pParam;
    uint32_t length          = 0U;

    if (mPeerInformation.appState != mAppRunning_c)
    {
        return;
    }

    for (uint32_t i = 0U; i < message->bufferLen; i++)
    {
        if (('\r' != message->buffer[i]) && ('\n' != message->buffer[i]))
        {
            mMsg[length++] = message->buffer[i];
        }

        if (((length == (mAppUartBufferSize_c - 2U)) || (i == (message->bufferLen - 1U))) && (length != 0U))
        {
            mMsg[length++] = '\r';
            mMsg[length++] = '\n';

            if (bleConnectedFlag)
            {
                GattClient_WriteCharacteristicValue(mPeerInformation.deviceId, &mPeerInformation.clientInfo.uartStream,
                                                    length, mMsg, TRUE, FALSE, FALSE, NULL);
            }

            length = 0U;
        }
    }

    rpmsg_queue_nocopy_free(myRpmsg, message->buffer);
    vPortFree(message);
}

static void AppRpmsgMonitor(struct rpmsg_lite_instance *rpmsgHandle, bool ready, void *param)
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    a7Addr = RL_ADDR_ANY; /* No matter connect or disconnect, CA7 RPMsg endpoint address need reset. */
    xSemaphoreGive(mutex);

    if (ready)
    {
        assert(rpmsgHandle);
        myRpmsg = rpmsgHandle;
        /* RPMsg channel created */
        myQueue = rpmsg_queue_create(rpmsgHandle);
        assert(myQueue);
        myEpt = rpmsg_lite_create_ept(rpmsgHandle, RL_ADDR_ANY, rpmsg_queue_rx_cb, myQueue);
        assert(myEpt);
        rpmsg_ns_announce(rpmsgHandle, myEpt, APP_BLE_BRIDGE_RPMSG_NS_ANNOUNCE_STRING, RL_NS_CREATE);

        runRpmsg = true;
        xSemaphoreGive(startSig); /* Activate Bridge Task to run. */
    }
    else
    {
        runRpmsg = false;
        xSemaphoreTake(stopSig, portMAX_DELAY); /* Wait Bridge Task to pend on startSig. */
        /* Now safe to destroy. */
        if (myEpt)
        {
            rpmsg_lite_destroy_ept(myRpmsg, myEpt);
            myEpt = NULL;
        }
        if (myQueue)
        {
            rpmsg_queue_destroy(myRpmsg, myQueue);
            myQueue = NULL;
        }
        myRpmsg = NULL;
    }
}

static void BridgeTask(void *param)
{
    uint32_t remoteAddr;
    int32_t result;
    uint8_t *rpmsgBuffer;
    uint32_t rpmsgBufferLen;
    rpmsg_message_t *message;

    while (true)
    {
        xSemaphoreTake(startSig, portMAX_DELAY);
        for (;;)
        {
            if (!runRpmsg)
            {
                xSemaphoreGive(stopSig);
                break;
            }

            result =
                rpmsg_queue_recv_nocopy(myRpmsg, myQueue, &remoteAddr, (char **)&rpmsgBuffer, &rpmsgBufferLen, 50U);
            if (RL_SUCCESS == result)
            {
                if (a7Addr == RL_ADDR_ANY)
                {
                    a7Addr = remoteAddr;
                }
                message            = pvPortMalloc(sizeof(rpmsg_message_t));
                message->buffer    = rpmsgBuffer;
                message->bufferLen = rpmsgBufferLen;

                if (bleConnectedFlag)
                {
                    App_PostCallbackMessage(BleApp_FlushUartStream, (void *)message);
                }
                else
                {
                    rpmsg_queue_nocopy_free(myRpmsg, message->buffer);
                    vPortFree(message);
                }
            }
        }
    }
}

int main(void)
{
    /* Initialize MCU clock */
    CLOCK_EnableClock(kCLOCK_PctlA);
    CLOCK_EnableClock(kCLOCK_PctlB);
    CLOCK_EnableClock(kCLOCK_Rgpio2p0);

    BOARD_InitPins();
    BOARD_BootClockRUN();
    APP_SRTM_I2C_ReleaseBus();
    BOARD_I2C_ConfigurePins();
    BOARD_InitDebugConsole();

    CLOCK_SetIpSrc(kCLOCK_Lpi2c3, kCLOCK_IpSrcSircAsync);
    CLOCK_SetIpSrc(kCLOCK_Lpi2c0, kCLOCK_IpSrcSystem);

    /* Use SIRC async clock to avoid baudrate impact from CPU freq change */
    CLOCK_SetIpSrc(kCLOCK_Lpuart2, kCLOCK_IpSrcSircAsync);

    /* Use AUXPLL main clock source */
    CLOCK_SetIpSrcDiv(kCLOCK_Sai0, kCLOCK_IpSrcRtcAuxPllAsync, 0, 0);

    mutex = xSemaphoreCreateMutex();
    assert(mutex);
    startSig = xSemaphoreCreateBinary();
    assert(startSig);

    APP_SRTM_Init();
    APP_SRTM_SetRpmsgMonitor(AppRpmsgMonitor, NULL);
    APP_SRTM_BootCA7();

    /* Create BLE main task. */
    xTaskCreate(BleTask, "BLE Task", 1024U, NULL, tskIDLE_PRIORITY + 1U, NULL);
    /* Create BLE bridge RPMsg task. */
    xTaskCreate(BridgeTask, "Bridge Task", configMINIMAL_STACK_SIZE * 2U, NULL, tskIDLE_PRIORITY + 2U, NULL);

    vTaskStartScheduler();
    /* Application should never reach this point. */
    for (;;)
    {
    }
}

/*! *********************************************************************************
 * \brief    Initializes application specific functionality before the BLE stack init.
 *
 ********************************************************************************** */
void BleApp_Init(void)
{
    /* UI */
    Serial_Init();

    /* Init FSCI */
    FSCI_Init((void *)mFsciSerials);

    /* Register BLE handlers in FSCI */
    fsciBleRegister(0);
}

/*! *********************************************************************************
 * \brief    Starts the BLE application.
 *
 * \param[in]    mGapRole    GAP Start Role (Central or Peripheral).
 ********************************************************************************** */
void BleApp_Start(void)
{
    BleApp_Advertise();
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
            BleApp_Start();
        }
        break;

        case gAdvertisingParametersSetupComplete_c:
        {
            App_StartAdvertising(BleApp_AdvertisingCallback, BleApp_ConnectionCallback);
        }
        break;

        default:
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
    /* Init GATT DB dynamically */
    BleApp_CreateGattDb();

    /* Configure as GAP dual role */
    BleConnManager_GapDualRoleConfig();

    /* Register for callbacks*/
    GattServer_RegisterHandlesForWriteNotifications(NumberOfElements(cpHandles), cpHandles);
    App_RegisterGattServerCallback(BleApp_GattServerCallback);
    App_RegisterGattClientProcedureCallback(BleApp_GattClientCallback);
    BleServDisc_RegisterCallback(BleApp_ServiceDiscoveryCallback);

    mPeerInformation.appState = mAppIdle_c;
    mAdvState.advOn           = FALSE;

    /* Start services */
    Wus_Start(&mWuServiceConfig);

    basServiceConfig.batteryLevel = 5;
    Bas_Start(&basServiceConfig);
}

/*! *********************************************************************************
 * \brief        Configures GAP Advertise parameters. Advertise will satrt after
 *               the parameters are set.
 *
 ********************************************************************************** */
static void BleApp_Advertise(void)
{
    /* Set advertising parameters*/
    Gap_SetAdvertisingParameters(&gAdvParams);
}

/*! *********************************************************************************
 * \brief        Handles BLE Advertising callback from host stack.
 *
 * \param[in]    pAdvertisingEvent    Pointer to gapAdvertisingEvent_t.
 ********************************************************************************** */
static void BleApp_AdvertisingCallback(gapAdvertisingEvent_t *pAdvertisingEvent)
{
    switch (pAdvertisingEvent->eventType)
    {
        case gAdvertisingStateChanged_c:
        {
            mAdvState.advOn = !mAdvState.advOn;
            PRINTF("gAdvertisingStateChanged_c\r\n");
        }
        break;

        case gAdvertisingCommandFailed_c:
        {
            panic(0, 0, 0, 0);
        }
        break;

        default:
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
    BleConnManager_GapPeripheralEvent(peerDeviceId, pConnectionEvent);

    switch (pConnectionEvent->eventType)
    {
        case gConnEvtConnected_c:
        {
            mPeerInformation.deviceId = peerDeviceId;

            /* Advertising stops when connected */
            mAdvState.advOn = FALSE;

            /* Subscribe client*/
            Wus_Subscribe(peerDeviceId);

            BleApp_StateMachineHandler(peerDeviceId, mAppEvt_PeerConnected_c);

            bleConnectedFlag = true;

            PRINTF("gConnEvtConnected_c\r\n");
        }
        break;

        case gConnEvtDisconnected_c:
        {
            mPeerInformation.appState = mAppIdle_c;
            mPeerInformation.deviceId = gInvalidDeviceId_c;

            /* Unsubscribe client */
            Wus_Unsubscribe();

            /* Reset Service Discovery to be sure*/
            BleServDisc_Stop(peerDeviceId);

            bleConnectedFlag = false;

            PRINTF("gConnEvtDisconnected_c\r\n");

            BleApp_Start();
        }
        break;

        default:
            break;
    }

    /* Connection Manager to handle Host Stack interactions */
    BleConnManager_GapPeripheralEvent(peerDeviceId, pConnectionEvent);
}

static void BleApp_ServiceDiscoveryCallback(deviceId_t peerDeviceId, servDiscEvent_t *pEvent)
{
    switch (pEvent->eventType)
    {
        case gServiceDiscovered_c:
        {
            BleApp_StoreService(peerDeviceId, pEvent->eventData.pService);
        }
        break;

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
            break;
    }
}

/*! *********************************************************************************
 * \brief        Handles GATT client callback from host stack.
 *
 * \param[in]    serverDeviceId      GATT Server device ID.
 * \param[in]    procedureType    	Procedure type.
 * \param[in]    procedureResult    	Procedure result.
 * \param[in]    error    			Callback result.
 ********************************************************************************** */
static void BleApp_GattClientCallback(deviceId_t serverDeviceId,
                                      gattProcedureType_t procedureType,
                                      gattProcedureResult_t procedureResult,
                                      bleResult_t error)
{
    if (procedureResult == gGattProcError_c)
    {
        BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcError_c);
    }
    else if (procedureResult == gGattProcSuccess_c)
    {
        BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcComplete_c);
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
static void BleApp_GattServerCallback(deviceId_t deviceId, gattServerEvent_t *pServerEvent)
{
    uint16_t handle;

    switch (pServerEvent->eventType)
    {
        case gEvtAttributeWrittenWithoutResponse_c:
        {
            handle = pServerEvent->eventData.attributeWrittenEvent.handle;

            if (handle == cpHandles[0])
            {
                BleApp_ReceivedUartStream(pServerEvent->eventData.attributeWrittenEvent.aValue,
                                          pServerEvent->eventData.attributeWrittenEvent.cValueLength);
            }
            break;
        }
        default:
            break;
    }
}

static void BleApp_StateMachineHandler(deviceId_t peerDeviceId, uint8_t event)
{
    switch (mPeerInformation.appState)
    {
        case mAppIdle_c:
        {
            if (event == mAppEvt_PeerConnected_c)
            {
                /* Moving to Service Discovery State*/
                mPeerInformation.appState = mAppServiceDisc_c;

                /* Start Service Discovery*/
                BleServDisc_FindService(peerDeviceId, gBleUuidType128_c, (bleUuid_t *)&uuid_service_wireless_uart);
            }
            break;
        }

        case mAppServiceDisc_c:
        {
            if (event == mAppEvt_ServiceDiscoveryComplete_c)
            {
                /* Moving to Running State*/
                mPeerInformation.appState = mAppRunning_c;
            }
            else if (event == mAppEvt_ServiceDiscoveryFailed_c)
            {
                Gap_Disconnect(peerDeviceId);
            }
            break;
        }

        case mAppRunning_c:
            break;
        default:
            break;
    }
}

/*! *********************************************************************************
 * \brief        Stores handles used by the application.
 *
 * \param[in]    pService    Pointer to gattService_t.
 ********************************************************************************** */
static void BleApp_StoreService(deviceId_t peerDeviceId, gattService_t *pService)
{
    /* Found Wireless UART Service */
    FLib_MemCpy(&mPeerInformation.clientInfo.service, pService, sizeof(gattService_t));

    if (pService->cNumCharacteristics > 0 && pService->aCharacteristics != NULL)
    {
        /* Found Uart Characteristic */
        FLib_MemCpy(&mPeerInformation.clientInfo.uartStream, &pService->aCharacteristics[0],
                    sizeof(gattCharacteristic_t));
    }
}

static void BleApp_ReceivedUartStream(uint8_t *pStream, uint16_t streamLength)
{
    uint8_t *data       = pStream;
    uint16_t dataLen    = 0U;
    uint16_t dataRemain = streamLength;

    for (;;)
    {
        dataLen = (dataRemain > RL_BUFFER_PAYLOAD_SIZE) ? RL_BUFFER_PAYLOAD_SIZE : dataRemain;

        xSemaphoreTake(mutex, portMAX_DELAY);
        if (a7Addr) /* Will not deliver message to CA7 until gets CA7 endpoint address. */
        {
            rpmsg_lite_send(myRpmsg, myEpt, a7Addr, (char *)data, dataLen, RL_BLOCK);
        }
        xSemaphoreGive(mutex);

        if (dataRemain > RL_BUFFER_PAYLOAD_SIZE)
        {
            data += RL_BUFFER_PAYLOAD_SIZE;
            dataRemain -= RL_BUFFER_PAYLOAD_SIZE;
        }
        else
        {
            break;
        }
    }
}

/*! *********************************************************************************
 * \brief        Creates GATT DB dynamically for the BLE BB
 *
 ********************************************************************************** */
static void BleApp_CreateGattDb(void)
{
    wirelessUartServiceHandles_t *pWuHandles = NULL;
    batteryServiceHandles_t *pBsHandles      = NULL;
    deviceInfoServiceHandles_t *pDisHandles  = NULL;

    /* GATT */
    GattDbDynamic_AddGattService(NULL);

    /* GAP */
    GattDbDynamic_AddGapService(NULL);

    /* Wireless UART */
    pWuHandles = (wirelessUartServiceHandles_t *)MEM_BufferAlloc(sizeof(wirelessUartServiceHandles_t));
    GattDbDynamic_AddWirelessUartService(pWuHandles);
    if (NULL != pWuHandles)
    {
        mWuServiceConfig.serviceHandle   = pWuHandles->serviceHandle;
        serviceSecurity[0].serviceHandle = pWuHandles->serviceHandle;
        cpHandles[0]                     = pWuHandles->charUartStreamHandle + 1; /* value_uart_stream */
        MEM_BufferFree(pWuHandles);
    }

    /* BS */
    pBsHandles = (batteryServiceHandles_t *)MEM_BufferAlloc(sizeof(batteryServiceHandles_t));
    GattDbDynamic_AddBatteryService(pBsHandles);
    if (NULL != pBsHandles)
    {
        basServiceConfig.serviceHandle   = pBsHandles->serviceHandle;
        serviceSecurity[1].serviceHandle = pBsHandles->serviceHandle;
        MEM_BufferFree(pBsHandles);
    }

    /* DIS */
    pDisHandles = (deviceInfoServiceHandles_t *)MEM_BufferAlloc(sizeof(deviceInfoServiceHandles_t));
    GattDbDynamic_AddDeviceInformationService(pDisHandles);
    if (NULL != pDisHandles)
    {
        serviceSecurity[2].serviceHandle = pDisHandles->serviceHandle;
        MEM_BufferFree(pDisHandles);
    }
}
