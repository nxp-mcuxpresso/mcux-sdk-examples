/*! *********************************************************************************
* Copyright 2020-2024 NXP
*
*
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "EmbeddedTypes.h"

/* Components */
#include "fsl_os_abstraction.h"
#include "fsl_component_mem_manager.h"
#include "fsl_component_timer_manager.h"
#include "fsl_component_messaging.h"
#include "fsl_adapter_flash.h"
#include "fsl_component_panic.h"
#include "fsl_component_led.h"

/* Fwk */
#include "app.h"
#include "RNG_Interface.h"
#include "SecLib.h"
#include "board.h"
#include "board_platform.h"
#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d>0)
#include "PWR_Interface.h"
#endif /* gAppLowpowerEnabled_d */

#if defined (gAppOtaASyncFlashTransactions_c ) && (gAppOtaASyncFlashTransactions_c > 0)
#include "OtaSupport.h"
#endif

#if defined(gFsciIncluded_c) && (gFsciIncluded_c == 1)
#include "FsciInterface.h"
#if gFSCI_IncludeLpmCommands_c
#include "FsciCommands.h"
#endif /* gFSCI_IncludeLpmCommands_c */
#endif /* gFsciIncluded_c */

#if defined(gAppUseNvm_d) && (gAppUseNvm_d > 0)
#include "NVM_Interface.h"
#if defined(gFsciIncluded_c) && (gFsciIncluded_c == 1)
#include "NV_FsciCommands.h"
#endif /* gFsciIncluded_c */
#endif /* gAppUseNvm_d */

#include "fwk_platform_ble.h"

/* SDK */
#ifdef SDK_OS_FREE_RTOS
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#endif /* SDK_OS_FREE_RTOS */

/* Application common */
#include "app_conn.h"
#include "fsl_os_abstraction.h"
#include "ble_conn_manager.h"
#include "controller_api.h"

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
typedef struct appMsgCallback_tag
{
    appCallbackHandler_t   handler;
    appCallbackParam_t     param;
} appMsgCallback_t;

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/

static void App_HandleHostMessageInput
(
    appMsgFromHost_t* pMsg
);
static bool_t App_HandleHostMessageInput_Gap
(
    appMsgFromHost_t* pMsg
);
static bool_t App_HandleHostMessageInput_Gatt
(
    appMsgFromHost_t* pMsg
);
static bool_t App_HandleHostMessageInput_L2ca
(
    appMsgFromHost_t* pMsg
);
static void App_GenericHandler
(
    gapGenericEvent_t *pGenericEvent
);
STATIC void App_GattServerCallback
(
    deviceId_t         peerDeviceId,
    gattServerEvent_t* pServerEvent
);
STATIC void App_GattClientProcedureCallback
(
    deviceId_t              deviceId,
    gattProcedureType_t     procedureType,
    gattProcedureResult_t   procedureResult,
    bleResult_t             error
);
STATIC void App_GattClientNotificationCallback
(
    deviceId_t      deviceId,
    uint16_t        characteristicValueHandle,
    uint8_t*        aValue,
    uint16_t        valueLength
);
STATIC void App_GattClientIndicationCallback
(
    deviceId_t      deviceId,
    uint16_t        characteristicValueHandle,
    uint8_t*        aValue,
    uint16_t        valueLength
);
STATIC void App_L2caLeDataCallback
(
    deviceId_t deviceId,
    uint16_t channelId,
    uint8_t* pPacket,
    uint16_t packetLength
);
STATIC void App_L2caLeControlCallback
(
    l2capControlMessage_t* pMessage
);

/*! *********************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
static appBluetoothLEInitCompleteCallback_t mpfInitDoneCallback       = NULL;
static gapGenericCallback_t                 mpfAppGenericCallback     = NULL;
static gapGenericCallback_t                 mpfGenericHandler         = NULL;
static gattServerCallback_t                 pfGattServerCallback      = NULL;
static gattClientProcedureCallback_t        pfGattClientProcCallback  = NULL;
static gattClientNotificationCallback_t     pfGattClientNotifCallback = NULL;
static gattClientNotificationCallback_t     pfGattClientIndCallback   = NULL;
static l2caLeCbDataCallback_t               pfL2caLeCbDataCallback    = NULL;
static l2caLeCbControlCallback_t            pfL2caLeCbControlCallback = NULL;

/* Application input queues */
static messaging_t mAppCbInputQueue;

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

OSA_EVENT_HANDLE_DEFINE(mAppEvent);
appAdvertiserHandler_t   pfAdvertiserHandler = NULL;

gapAdvertisingCallback_t pfAdvCallback       = NULL;     /* Set only if advertising is used */
gapScanningCallback_t    pfScanCallback      = NULL;     /* Set only if scanning is used */
gapConnectionCallback_t  pfConnCallback      = NULL;     /* Set only if connection is used */

/* Application input queues */
messaging_t mHostAppInputQueue;

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
*\fn           void BluetoothLEHost_Init(
*                  appBluetoothLEInitCompleteCallback_t pCallback
*              )
*\brief        This is the host initialization function (initialize the Bluetooth LE
*              stack and also all framework and controller required components).
*
*\param  [in]  pCallback    Callback triggered when the initialization is
*                           complete.
*
*\retval       void.
********************************************************************************** */
void BluetoothLEHost_Init
(
    appBluetoothLEInitCompleteCallback_t pCallback
)
{
        (void)MEM_Init();

        /* Framework init */
#if defined(gRngSeedStorageAddr_d) || defined(gXcvrDacTrimValueSorageAddr_d)
        NV_Init();
#endif /* gRngSeedStorageAddr_d || gXcvrDacTrimValueSorageAddr_d */

        /* Cryptographic hardware initialization */
        SecLib_Init();

        /* RNG software initialization and PRNG initial seeding (from hardware) */
        (void)RNG_Init();

        /*Has to be called after RNG_Init(), once seed is generated.*/
        (void)Controller_SetRandomSeed();

#if defined(BOARD_32KHZ_SRC_CLK_ACCURACY)
        /* Set sleep clock accuracy value in the controller, 
         * set to 0 by LL if Controller_SetSleepClockAccuracy() not called */
        Controller_SetSleepClockAccuracy(BOARD_32KHZ_SRC_CLK_ACCURACY);
#endif

#if (defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U))        
        (void)PLATFORM_EnableBleSecureKeyManagement();
#endif
#if defined(gAppUseNvm_d) && (gAppUseNvm_d>0)
        /* Initialize NV module */
        (void)NvModuleInit();
#endif /* gAppUseNvm_d */

        /* Create application event */
        (void)OSA_EventCreate(mAppEvent, (uint8_t)TRUE);

        /* Prepare application input queue.*/
        MSG_QueueInit(&mHostAppInputQueue);

        /* Prepare callback input queue.*/
        MSG_QueueInit(&mAppCbInputQueue);

        /* BLE common part */
        mpfInitDoneCallback = pCallback;
        mpfGenericHandler = App_GenericHandler;

        /* BLE Host Stack Init */
        if (Ble_Initialize(App_GenericCallback) != gBleSuccess_c)
        {
            panic(0,0,0,0);
            return;
        }

#if (gAppUseNvm_d && defined(gFsciIncluded_c) && (gFsciIncluded_c))
#if gNvmEnableFSCIMonitoring_c
        NV_SetFSCIMonitoringState(TRUE);
#endif /* gNvmEnableFSCIMonitoring_c */
#if gNvmEnableFSCIRequests_c
        NV_RegisterToFSCI();
#endif /* gNvmEnableFSCIRequests_c */
#endif /* gAppUseNvm_d && gFsciIncluded_c */

    /* configure tx power to use in NBU specfic to BLE */
    (void)Controller_SetTxPowerLevelDbm(mAdvertisingDefaultTxPower_c, gAdvTxChannel_c);
    (void)Controller_SetTxPowerLevelDbm(mConnectionDefaultTxPower_c, gConnTxChannel_c);
    (void)Controller_ConfigureInvalidPduHandling(gLlInvalidPduHandlingType_c);
}

/*! *********************************************************************************
*\fn           void BluetoothLEHost_HandleMessages(void)
*\brief        This function is responsible for consuming all events coming from the
*              Bluetooth LE stack.
*
*\param  [in]  none.
*
*\retval       void.
********************************************************************************** */
void BluetoothLEHost_HandleMessages(void)
{
#ifdef SDK_OS_FREE_RTOS
    osa_event_flags_t event = 0U;
    (void)OSA_EventWait((osa_event_handle_t)mAppEvent,
                        osaEventFlagsAll_c,
                        (uint8_t)FALSE,
                        gAppTaskWaitTimeout_ms_c ,
                        &event);
#endif /* SDK_OS_FREE_RTOS */

    /* Check for existing messages in queue */
    if (MSG_QueueGetHead(&mHostAppInputQueue) != NULL)
    {
        /* Pointer for storing the messages from host. */
        appMsgFromHost_t *pMsgIn = MSG_QueueRemoveHead(&mHostAppInputQueue);

        if (pMsgIn != NULL)
        {
            /* Process it */
            App_HandleHostMessageInput(pMsgIn);

            /* Messages must always be freed. */
            (void)MSG_Free(pMsgIn);
        }
    }

    /* Check for existing messages in queue */
    if (MSG_QueueGetHead(&mAppCbInputQueue) != NULL)
    {
        /* Pointer for storing the callback messages. */
        appMsgCallback_t *pMsgIn = MSG_QueueGetHead(&mAppCbInputQueue);

        if (pMsgIn != NULL)
        {
            /* Execute callback handler */
            if (pMsgIn->handler != NULL)
            {
                pMsgIn->handler(pMsgIn->param);
            }

            /* Messages must always be freed. */
            (void)MSG_Free(pMsgIn);
        }
    }

#ifdef SDK_OS_FREE_RTOS
    /* Signal the main_thread again if there are more messages pending */
    event = (MSG_QueueGetHead(&mHostAppInputQueue) != NULL) ? gAppEvtMsgFromHostStack_c : 0U;
    event |= (MSG_QueueGetHead(&mAppCbInputQueue) != NULL) ? gAppEvtAppCallback_c : 0U;
    if (event != 0U)
    {
    	(void)OSA_EventSet((osa_event_handle_t)mAppEvent, gAppEvtAppCallback_c);
    }
#endif /* SDK_OS_FREE_RTOS */
}


/*! *********************************************************************************
*\fn           void BluetoothLEHost_IsMessagePending(void)
*\brief        This function checks whether Messages are pending to be processed.
*
*\param  [in]  none.
*
*\retval       TRUE if pending messages.
********************************************************************************** */
bool BluetoothLEHost_IsMessagePending(void)
{
    bool ret = FALSE;
     /* Check for existing messages in queue */
    if ( (MSG_QueueGetHead(&mHostAppInputQueue) != NULL) || (MSG_QueueGetHead(&mAppCbInputQueue) != NULL) )
    {
        ret = TRUE;
    }
    return ret;
}

/*! *********************************************************************************
\fn            void BluetoothLEHost_SetGenericCallback(
*                  gapGenericCallback_t pfGenericCallback
*              )
*\brief        Set advanced generic callback. Set a callback to receive all Bluetooth
*              LE stack generic events.
*
*\param  [in]  pfGenericCallback    Callback used by the application to receive all
*                                   stack generic events.
*
*\retval       void.
********************************************************************************** */
void BluetoothLEHost_SetGenericCallback
(
    gapGenericCallback_t pfGenericCallback
)
{
    mpfAppGenericCallback = pfGenericCallback;
}

/*! *********************************************************************************
\fn            bleResult_t App_RegisterGattServerCallback(
*                  gattServerCallback_t serverCallback)
*\brief        Set application gatt server callback and install static callback for
*              the GATT Server module.
*
*\param  [in]  serverCallback    Callback used by the application to receive gatt
*                                server events.
*
*\return       bleResult_t        Result of the operation.
********************************************************************************** */
bleResult_t App_RegisterGattServerCallback
(
    gattServerCallback_t serverCallback
)
{
    pfGattServerCallback = serverCallback;

    return GattServer_RegisterCallback(App_GattServerCallback);
}

/*! *********************************************************************************
\fn            bleResult_t App_RegisterGattClientProcedureCallback(
*                  gattClientProcedureCallback_t  callback)
*\brief        Set application gatt client callback and install static callback for
*              the GATT Client module procedures.
*
*\param  [in]  callback       Callback used by the application to receive gatt
*                             client events.
*
*\return       bleResult_t    Result of the operation.
********************************************************************************** */
bleResult_t App_RegisterGattClientProcedureCallback
(
    gattClientProcedureCallback_t  callback
)
{
    pfGattClientProcCallback = callback;

    return GattClient_RegisterProcedureCallback(App_GattClientProcedureCallback);
}

/*! *********************************************************************************
\fn            bleResult_t App_RegisterGattClientNotificationCallback(
*                  gattClientNotificationCallback_t  callback)
*\brief        Set application gatt client notification callback and install static
*              callback for Gatt server notifications.
*
*\param  [in]  callback       Callback used by the application to receive gatt
*                             server notifications.
*
*\return       bleResult_t    Result of the operation.
********************************************************************************** */
bleResult_t App_RegisterGattClientNotificationCallback
(
    gattClientNotificationCallback_t  callback
)
{
    pfGattClientNotifCallback = callback;

    return GattClient_RegisterNotificationCallback(App_GattClientNotificationCallback);
}

/*! *********************************************************************************
\fn            bleResult_t App_RegisterGattClientIndicationCallback(
*                  gattClientIndicationCallback_t  callback)
*\brief        Set application gatt client indication callback and install static
*              callback for Gatt server indications.
*
*\param  [in]  callback       Callback used by the application to receive gatt
*                             server indications.
*
*\return       bleResult_t    Result of the operation.
********************************************************************************** */
bleResult_t App_RegisterGattClientIndicationCallback
(
    gattClientIndicationCallback_t  callback
)
{
    pfGattClientIndCallback = callback;

    return GattClient_RegisterIndicationCallback(App_GattClientIndicationCallback);
}

/*! *********************************************************************************
\fn            bleResult_t App_RegisterLeCbCallbacks(
*                  l2caLeCbDataCallback_t      pCallback,
*                  l2caLeCbControlCallback_t   pCtrlCallback
*              )
*\brief        Set application L2CAP callbacks for credit based data and control
*              events and register static callbacks on L2CAP.
*
*\param  [in]  pCallback      Callback function for data plane messages.
*\param  [in]  pCtrlCallback  Callback function for control plane messages.
*
*\return       bleResult_t    Result of the operation.
********************************************************************************** */
bleResult_t App_RegisterLeCbCallbacks
(
    l2caLeCbDataCallback_t      pCallback,
    l2caLeCbControlCallback_t   pCtrlCallback
)
{
    pfL2caLeCbDataCallback = pCallback;
    pfL2caLeCbControlCallback = pCtrlCallback;

    return L2ca_RegisterLeCbCallbacks(App_L2caLeDataCallback, App_L2caLeControlCallback);
}

/*! *********************************************************************************
\fn            bleResult_t App_PostCallbackMessage(
*                  appCallbackHandler_t   handler,
*                  appCallbackParam_t     param
               )
*\brief        Store a callback message in the Cb App queue and signal application.
*
*\param  [in]  handler              Callback handler.
*\param  [in]  param                Callback parameter.
*
*\retval       gBleOutOfMemory_c    Message allocation fail.
*\retval       gBleSuccess_c        Successful addition to the Cb App queue.
********************************************************************************** */
bleResult_t App_PostCallbackMessage
(
    appCallbackHandler_t   handler,
    appCallbackParam_t     param
)
{
    appMsgCallback_t *pMsgIn = NULL;

    /* Allocate a buffer with enough space to store the packet */
    pMsgIn = MSG_Alloc(sizeof (appMsgCallback_t));

    if (pMsgIn == NULL)
    {
        return gBleOutOfMemory_c;
    }

    pMsgIn->handler = handler;
    pMsgIn->param = param;

    /* Put message in the Cb App queue */
    (void)MSG_QueueAddTail(&mAppCbInputQueue, pMsgIn);

    /* Signal application */
    (void)OSA_EventSet(mAppEvent, gAppEvtAppCallback_c);

    return gBleSuccess_c;
}

/*! *********************************************************************************
\fn            bleResult_t App_GenericCallback(gapGenericEvent_t* pGenericEvent)
*\brief        Callback used by the Host Stack to propagate GAP generic
*              events to the application.
*
*\param  [in]  pGenericEvent    Pointer to the GAP generic event.
*
*\retval       void.
********************************************************************************** */
void App_GenericCallback
(
    gapGenericEvent_t* pGenericEvent
)
{
    appMsgFromHost_t *pMsgIn = NULL;

    pMsgIn = MSG_Alloc((uint32_t)&(pMsgIn->msgData) + sizeof(gapGenericEvent_t));

    if (pMsgIn == NULL)
    {
        return;
    }

    pMsgIn->msgType = (uint32_t)gAppGapGenericMsg_c;
    FLib_MemCpy(&pMsgIn->msgData.genericMsg,
                pGenericEvent,
                sizeof(gapGenericEvent_t));

    /* Put message in the Host Stack to App queue */
    (void)MSG_QueueAddTail(&mHostAppInputQueue, pMsgIn);

    /* Signal application */
    (void)OSA_EventSet(mAppEvent, gAppEvtMsgFromHostStack_c);
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
*\private
*\fn           void App_HandleHostMessageInput(appMsgFromHost_t* pMsg)
*\brief        Handles all messages received from the host task.
*
*\param  [in]  pMsg    Pointer to the mesage received from the host task.
*
*\retval       void.
********************************************************************************** */
static void App_HandleHostMessageInput
(
    appMsgFromHost_t* pMsg
)
{
    bool_t matchFound = App_HandleHostMessageInput_Gap(pMsg);

    if (!matchFound)
    {
        matchFound = App_HandleHostMessageInput_Gatt(pMsg);
    }

    if (!matchFound)
    {
        matchFound = App_HandleHostMessageInput_L2ca(pMsg);
    }
}

/*! *********************************************************************************
*\private
*\fn           bool_t App_HandleHostMessageInput_Gap(appMsgFromHost_t* pMsg)
*\brief        Handles all messages received from the host task if the message
*              addresses GAP functionality. Called by App_HandleHostMessageInput().
*
*\param  [in]  pMsg    Pointer to the mesage received from the host task.
*
*\retval       TRUE     If the message type matched and a handler function was called
*\retval       FALSE    Message was not GAP layer related
********************************************************************************** */
static bool_t App_HandleHostMessageInput_Gap
(
    appMsgFromHost_t* pMsg
)
{
    bool_t matchFound = TRUE;
    switch ( pMsg->msgType )
    {
        case (uint32_t)gAppGapGenericMsg_c:
        {
            if (mpfGenericHandler != NULL)
            {
                mpfGenericHandler(&pMsg->msgData.genericMsg);
            }
            break;
        }
        case (uint32_t)gAppGapAdvertisementMsg_c:
        {
            if (pfAdvCallback != NULL)
            {
                pfAdvCallback(&pMsg->msgData.advMsg);
            }
            break;
        }
        case (uint32_t)gAppGapScanMsg_c:
        {
            if (pfScanCallback != NULL)
            {
                pfScanCallback(&pMsg->msgData.scanMsg);
            }
            break;
        }
        case (uint32_t)gAppGapConnectionMsg_c:
        {
            if (pfConnCallback != NULL)
            {
                pfConnCallback(pMsg->msgData.connMsg.deviceId,
                               &pMsg->msgData.connMsg.connEvent);
            }
            break;
        }
        default:
        {
            matchFound = FALSE;
                        break;
        }
    }

    return matchFound;
}

/*! *********************************************************************************
*\private
*\fn           bool_t App_HandleHostMessageInput_Gatt(appMsgFromHost_t* pMsg)
*\brief        Handles all messages received from the host task if the message
*              addresses GATT functionality. Called by App_HandleHostMessageInput().
*
*\param  [in]  pMsg    Pointer to the mesage received from the host task.
*
*\retval       TRUE     If the message type matched and a handler function was called
*\retval       FALSE    Message was not GATT layer related
********************************************************************************** */
static bool_t App_HandleHostMessageInput_Gatt
(
    appMsgFromHost_t* pMsg
)
{
    bool_t matchFound = TRUE;
    switch ( pMsg->msgType )
    {
        case (uint32_t)gAppGattServerMsg_c:
        {
            if (pfGattServerCallback != NULL)
            {
                pfGattServerCallback(pMsg->msgData.gattServerMsg.deviceId,
                                     &pMsg->msgData.gattServerMsg.serverEvent);
            }
            break;
        }
        case (uint32_t)gAppGattClientProcedureMsg_c:
        {
            if (pfGattClientProcCallback != NULL)
            {
                pfGattClientProcCallback(
                    pMsg->msgData.gattClientProcMsg.deviceId,
                    pMsg->msgData.gattClientProcMsg.procedureType,
                    pMsg->msgData.gattClientProcMsg.procedureResult,
                    pMsg->msgData.gattClientProcMsg.error);
            }
            break;
        }
        case (uint32_t)gAppGattClientNotificationMsg_c:
        {
            if (pfGattClientNotifCallback != NULL)
            {
                pfGattClientNotifCallback(
                    pMsg->msgData.gattClientNotifIndMsg.deviceId,
                    pMsg->msgData.gattClientNotifIndMsg.characteristicValueHandle,
                    pMsg->msgData.gattClientNotifIndMsg.aValue,
                    pMsg->msgData.gattClientNotifIndMsg.valueLength);
            }
            break;
        }
        case (uint32_t)gAppGattClientIndicationMsg_c:
        {
            if (pfGattClientIndCallback != NULL)
            {
                pfGattClientIndCallback(
                    pMsg->msgData.gattClientNotifIndMsg.deviceId,
                    pMsg->msgData.gattClientNotifIndMsg.characteristicValueHandle,
                    pMsg->msgData.gattClientNotifIndMsg.aValue,
                    pMsg->msgData.gattClientNotifIndMsg.valueLength);
            }
            break;
        }
        default:
        {
            matchFound = FALSE;
            break;
        }
    }
    return matchFound;
}

/*! *********************************************************************************
*\private
*\fn           bool_t App_HandleHostMessageInput_L2ca(appMsgFromHost_t* pMsg)
*\brief        Handles all messages received from the host task if the message
*              addresses L2CAP functionality. Called by App_HandleHostMessageInput().
*
*\param  [in]  pMsg    Pointer to the mesage received from the host task.
*
*\retval       TRUE     If the message type matched and a handler function was called
*\retval       FALSE    Message was not L2CAP layer related
********************************************************************************** */
static bool_t App_HandleHostMessageInput_L2ca
(
    appMsgFromHost_t* pMsg
)
{
    bool_t matchFound = TRUE;
    switch ( pMsg->msgType )
    {
        case (uint32_t)gAppL2caLeDataMsg_c:
        {
            if (pfL2caLeCbDataCallback != NULL)
            {
                pfL2caLeCbDataCallback(
                    pMsg->msgData.l2caLeCbDataMsg.deviceId,
                    pMsg->msgData.l2caLeCbDataMsg.channelId,
                    pMsg->msgData.l2caLeCbDataMsg.aPacket,
                    pMsg->msgData.l2caLeCbDataMsg.packetLength);
            }
            break;
        }
        case (uint32_t)gAppL2caLeControlMsg_c:
        {
            if (pfL2caLeCbControlCallback != NULL)
            {
                pfL2caLeCbControlCallback(&pMsg->msgData.l2caLeCbControlMsg);
            }
            break;
        }
        default:
        {
            matchFound = FALSE;
            break;
        }
    }
    return matchFound;
}

/*! *********************************************************************************
*\private
*\fn           void App_GattServerCallback(deviceId_t          peerDeviceId,
                                           gattServerEvent_t*  pServerEvent)
*\brief        Callback used by the Host Stack to propagate Gatt Server events
*              to the application.
*
*\param  [in]  peerDeviceId    Peer device id.
*\param  [in]  pServerEvent    Pointer to the Gatt Server event.
*
*\retval       void.
********************************************************************************** */
STATIC void App_GattServerCallback
(
    deviceId_t          peerDeviceId,
    gattServerEvent_t*  pServerEvent
)
{
    appMsgFromHost_t *pMsgIn = NULL;
    uint32_t msgLen = (uint32_t)&(pMsgIn->msgData) + sizeof(gattServerMsg_t);

    if (pServerEvent->eventType == gEvtAttributeWritten_c ||
        pServerEvent->eventType == gEvtAttributeWrittenWithoutResponse_c)
    {
        msgLen += pServerEvent->eventData.attributeWrittenEvent.cValueLength;
    }

    pMsgIn = MSG_Alloc(msgLen);

    if (pMsgIn == NULL)
    {
        return;
    }

    pMsgIn->msgType = (uint32_t)gAppGattServerMsg_c;
    pMsgIn->msgData.gattServerMsg.deviceId = peerDeviceId;
    FLib_MemCpy(&pMsgIn->msgData.gattServerMsg.serverEvent,
                pServerEvent,
                sizeof(gattServerEvent_t));

    if ((pMsgIn->msgData.gattServerMsg.serverEvent.eventType ==
            gEvtAttributeWritten_c) ||
        (pMsgIn->msgData.gattServerMsg.serverEvent.eventType ==
            gEvtAttributeWrittenWithoutResponse_c))
    {
        /* Copy value after the gattServerEvent_t structure and update the aValue pointer*/
        pMsgIn->msgData.gattServerMsg.serverEvent.eventData.attributeWrittenEvent.aValue =
          (uint8_t *)&pMsgIn->msgData.gattServerMsg.serverEvent.eventData.attributeWrittenEvent.aValue +
          sizeof(uint8_t*) + sizeof(bearerId_t);
        pMsgIn->msgData.gattServerMsg.serverEvent.eventData.attributeWrittenEvent.bearerId =
            pServerEvent->eventData.attributeWrittenEvent.bearerId;
        FLib_MemCpy(pMsgIn->msgData.gattServerMsg.serverEvent.eventData.attributeWrittenEvent.aValue,
                    pServerEvent->eventData.attributeWrittenEvent.aValue,
                    pServerEvent->eventData.attributeWrittenEvent.cValueLength);

    }

    /* Put message in the Host Stack to App queue */
    (void)MSG_QueueAddTail(&mHostAppInputQueue, pMsgIn);

    /* Signal application */
    (void)OSA_EventSet(mAppEvent, gAppEvtMsgFromHostStack_c);
}

/*! *********************************************************************************
*\private
*\fn           void App_GattClientProcedureCallback(
*                  deviceId_t            deviceId,
*                  gattProcedureType_t   procedureType,
*                  gattProcedureResult_t procedureResult,
*                  bleResult_t           error
*              )
*\brief        Callback used by the Host Stack to propagate Gatt Client procedure
*              events to the application.
*
*\param  [in]  deviceId           Peer device id.
*\param  [in]  procedureType      Procedure type.
*\param  [in]  procedureResult    Procedure result.
*\param  [in]  error              Error result.
*
*\retval       void.
********************************************************************************** */
STATIC void App_GattClientProcedureCallback
(
    deviceId_t              deviceId,
    gattProcedureType_t     procedureType,
    gattProcedureResult_t   procedureResult,
    bleResult_t             error)
{
    appMsgFromHost_t *pMsgIn = NULL;

    pMsgIn = MSG_Alloc((uint32_t)&(pMsgIn->msgData) + sizeof(gattClientProcMsg_t));

    if (pMsgIn == NULL)
    {
        return;
    }

    pMsgIn->msgType = (uint32_t)gAppGattClientProcedureMsg_c;
    pMsgIn->msgData.gattClientProcMsg.deviceId = deviceId;
    pMsgIn->msgData.gattClientProcMsg.procedureType = procedureType;
    pMsgIn->msgData.gattClientProcMsg.error = error;
    pMsgIn->msgData.gattClientProcMsg.procedureResult = procedureResult;

    /* Put message in the Host Stack to App queue */
    (void)MSG_QueueAddTail(&mHostAppInputQueue, pMsgIn);

    /* Signal application */
    (void)OSA_EventSet(mAppEvent, gAppEvtMsgFromHostStack_c);
}

/*! *********************************************************************************
*\private
*\fn           void App_GattClientNotificationCallback(
*                  deviceId_t deviceId,
*                  uint16_t   characteristicValueHandle,
*                  uint8_t*   aValue,
*                  uint16_t   valueLength
*              )
*\brief        Callback used by the Host Stack to propagate Gatt Client notifications
*              to the application.
*
*\param  [in]  deviceId                     Peer device id.
*\param  [in]  characteristicValueHandle    Handle of the notified Gatt characteristic
*                                           value.
*\param  [in]  aValue                       Pointer to the characteristic value.
*\param  [in]  valueLength                  Value length.
*
*\retval       void.
********************************************************************************** */
STATIC void App_GattClientNotificationCallback
(
    deviceId_t      deviceId,
    uint16_t        characteristicValueHandle,
    uint8_t*        aValue,
    uint16_t        valueLength
)
{
    appMsgFromHost_t *pMsgIn = NULL;

    /* Allocate a buffer with enough space to store also the notified value*/
    pMsgIn = MSG_Alloc((uint32_t)&(pMsgIn->msgData) +
                       sizeof(gattClientNotifIndMsg_t) +
                       (uint32_t)valueLength);

    if (pMsgIn == NULL)
    {
        return;
    }

    pMsgIn->msgType = (uint32_t)gAppGattClientNotificationMsg_c;
    pMsgIn->msgData.gattClientNotifIndMsg.deviceId = deviceId;
    pMsgIn->msgData.gattClientNotifIndMsg.characteristicValueHandle =
                                characteristicValueHandle;
    pMsgIn->msgData.gattClientNotifIndMsg.valueLength = valueLength;

    /*
     * Copy value after the gattClientNotifIndMsg_t structure and update
     * the aValue pointer
     */
    pMsgIn->msgData.gattClientNotifIndMsg.aValue =
                                (uint8_t*)&pMsgIn->msgData +
                                sizeof(gattClientNotifIndMsg_t);
    FLib_MemCpy(pMsgIn->msgData.gattClientNotifIndMsg.aValue,
                aValue,
                valueLength);

    /* Put message in the Host Stack to App queue */
    (void)MSG_QueueAddTail(&mHostAppInputQueue, pMsgIn);

    /* Signal application */
    (void)OSA_EventSet(mAppEvent, gAppEvtMsgFromHostStack_c);
}

/*! *********************************************************************************
*\private
*\fn           void App_GattClientIndicationCallback(
*                  deviceId_t deviceId,
*                  uint16_t   characteristicValueHandle,
*                  uint8_t*   aValue,
*                  uint16_t   valueLength
*              )
*\brief        Callback used by the Host Stack to propagate Gatt Client indications
*              to the application.
*
*\param  [in]  deviceId                    Peer device id.
*\param  [in]  characteristicValueHandle   Handle of the indicated Gatt characteristic
*                                          value.
*\param  [in]  aValue                      Pointer to the characteristic value.
*\param  [in]  valueLength                 Value length.
*
*\retval       void.
********************************************************************************** */
STATIC void App_GattClientIndicationCallback
(
    deviceId_t      deviceId,
    uint16_t        characteristicValueHandle,
    uint8_t*        aValue,
    uint16_t        valueLength
)
{
    appMsgFromHost_t *pMsgIn = NULL;

    /* Allocate a buffer with enough space to store also the notified value*/
    pMsgIn = MSG_Alloc((uint32_t)&(pMsgIn->msgData) +
                       sizeof(gattClientNotifIndMsg_t) +
                       (uint32_t)valueLength);

    if (pMsgIn == NULL)
    {
        return;
    }

    pMsgIn->msgType = (uint32_t)gAppGattClientIndicationMsg_c;
    pMsgIn->msgData.gattClientNotifIndMsg.deviceId = deviceId;
    pMsgIn->msgData.gattClientNotifIndMsg.characteristicValueHandle =
                characteristicValueHandle;
    pMsgIn->msgData.gattClientNotifIndMsg.valueLength = valueLength;

    /*
     * Copy value after the gattClientIndIndMsg_t structure and update
     * the aValue pointer
     */
    pMsgIn->msgData.gattClientNotifIndMsg.aValue =
                (uint8_t*)&pMsgIn->msgData +
                sizeof(gattClientNotifIndMsg_t);
    FLib_MemCpy(pMsgIn->msgData.gattClientNotifIndMsg.aValue,
                aValue,
                valueLength);

    /* Put message in the Host Stack to App queue */
    (void)MSG_QueueAddTail(&mHostAppInputQueue, pMsgIn);

    /* Signal application */
    (void)OSA_EventSet(mAppEvent, gAppEvtMsgFromHostStack_c);
}

/*! *********************************************************************************
*\private
*\fn           void App_L2caLeDataCallback(deviceId_t deviceId,
                                           uint16_t   channelId,
                                           uint8_t*   pPacket,
                                           uint16_t   packetLength)
*\brief        Callback used by the Host Stack to send L2CAP credit based data to
*              the application.
*
*\param  [in]  deviceId        Peer device id.
*\param  [in]  channelId       Identifier of the L2CAP channel used for credit based
*                              data transmission.
*\param  [in]  pPacket         Pointer to the L2CAP packet.
*\param  [in]  packetLength    Packet length.
*
*\retval       void.
********************************************************************************** */
STATIC void App_L2caLeDataCallback
(
    deviceId_t deviceId,
    uint16_t channelId,
    uint8_t* pPacket,
    uint16_t packetLength
)
{
    appMsgFromHost_t *pMsgIn = NULL;

    /* Allocate a buffer with enough space to store the packet */
    pMsgIn = MSG_Alloc((uint32_t)&(pMsgIn->msgData) +
                       (sizeof(l2caLeCbDataMsg_t) - 1U) +
                       (uint32_t)packetLength);

    if (pMsgIn == NULL)
    {
        return;
    }

    pMsgIn->msgType = (uint32_t)gAppL2caLeDataMsg_c;
    pMsgIn->msgData.l2caLeCbDataMsg.deviceId = deviceId;
    pMsgIn->msgData.l2caLeCbDataMsg.channelId = channelId;
    pMsgIn->msgData.l2caLeCbDataMsg.packetLength = packetLength;

    FLib_MemCpy(pMsgIn->msgData.l2caLeCbDataMsg.aPacket,
                pPacket,
                packetLength);

    /* Put message in the Host Stack to App queue */
    (void)MSG_QueueAddTail(&mHostAppInputQueue, pMsgIn);

    /* Signal application */
    (void)OSA_EventSet(mAppEvent, gAppEvtMsgFromHostStack_c);
}

/*! *********************************************************************************
*\private
*\fn           void App_L2caLeControlCallback(l2capControlMessage_t* pMessage)
*\brief        Callback used by the Host Stack to propagate L2CAP control events to
*              the application.
*
*\param  [in]  pMessage    Pointer to the L2CAP control message.
*
*\retval       void.
********************************************************************************** */
STATIC void App_L2caLeControlCallback
(
    l2capControlMessage_t* pMessage
)
{
    appMsgFromHost_t *pMsgIn = NULL;
    uint8_t messageLength = 0U;

    switch (pMessage->messageType) {
        case gL2ca_LePsmConnectRequest_c:
        {
            messageLength = (uint8_t)sizeof(l2caLeCbConnectionRequest_t);
            break;
        }
        case gL2ca_LePsmConnectionComplete_c:
        {
            messageLength = (uint8_t)sizeof(l2caLeCbConnectionComplete_t);
            break;
        }
        case gL2ca_LePsmDisconnectNotification_c:
        {
            messageLength = (uint8_t)sizeof(l2caLeCbDisconnection_t);
            break;
        }
        case gL2ca_NoPeerCredits_c:
        {
            messageLength = (uint8_t)sizeof(l2caLeCbNoPeerCredits_t);
            break;
        }
        case gL2ca_LocalCreditsNotification_c:
        {
            messageLength = (uint8_t)sizeof(l2caLeCbLocalCreditsNotification_t);
            break;
        }
        case gL2ca_LowPeerCredits_c:
        {
            messageLength = (uint8_t)sizeof(l2caLeCbLowPeerCredits_t);
            break;
        }
        case gL2ca_Error_c:
        {
            messageLength = (uint8_t)sizeof(l2caLeCbError_t);
            break;
        }

        case gL2ca_HandoverConnectionComplete_c:
        {
            messageLength = (uint8_t)sizeof(l2caHandoverConnectionComplete_t);
        }
        break;

        default:
        {
            messageLength = 0U;
            break;
        }
    }

    if (messageLength == 0U)
    {
        return;
    }

    /* Allocate a buffer with enough space to store the biggest packet */
    pMsgIn = MSG_Alloc((uint32_t)&(pMsgIn->msgData) + sizeof(l2capControlMessage_t));

    if (pMsgIn == NULL)
    {
          return;
    }

    pMsgIn->msgType = (uint32_t)gAppL2caLeControlMsg_c;
    pMsgIn->msgData.l2caLeCbControlMsg.messageType = pMessage->messageType;

    FLib_MemCpy(&pMsgIn->msgData.l2caLeCbControlMsg.messageData,
                &pMessage->messageData,
                messageLength);

    /* Put message in the Host Stack to App queue */
    (void)MSG_QueueAddTail(&mHostAppInputQueue, pMsgIn);

    /* Signal application */
    (void)OSA_EventSet(mAppEvent, gAppEvtMsgFromHostStack_c);
}


/*! *********************************************************************************
*\private
*\fn           void App_GenericHandler(gapGenericEvent_t *pGenericEvent)
*\brief        Handles BLE generic callback.
*
*\param  [in]  pGenericEvent    Pointer to gapGenericEvent_t.
*
*\retval       void.
********************************************************************************** */
static void App_GenericHandler
(
    gapGenericEvent_t *pGenericEvent
)
{
    /* Call application handler for advanced use */
    if (mpfAppGenericCallback != NULL)
    {
        mpfAppGenericCallback(pGenericEvent);
    }

    switch (pGenericEvent->eventType)
    {
        case gInitializationComplete_c:
        {
            /* Call application handler */
            if (mpfInitDoneCallback != NULL)
            {
                mpfInitDoneCallback();
            }
        }
        break;
        case gDeInitializationComplete_c:
        {
            (void)Ble_DeInitialize();
        }
        break;
        /* Direct advertising events to the advertising handler */
        case gAdvertisingParametersSetupComplete_c:
                            /* Fall Through */
        case gAdvertisingDataSetupComplete_c:
                            /* Fall Through */
        case gAdvertisingSetupFailed_c:
                            /* Fall Through */
        case gExtAdvertisingParametersSetupComplete_c:
                            /* Fall Through */
        case gExtAdvertisingDataSetupComplete_c:
                            /* Fall Through */
        case gExtAdvertisingDecisionDataSetupComplete_c:
        {
            if (pfAdvertiserHandler != NULL)
            {
                pfAdvertiserHandler(pGenericEvent->eventType);
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
*\private
*\fn           void BluetoothLEHost_ProcessIdleTask(void)
*\brief        Handles Connectivity background task, usually executed from Idle task.
*
*\param  [in]  none.
*
*\retval       void.
********************************************************************************** */
void BluetoothLEHost_ProcessIdleTask(void)
{
#if defined(gAppUseNvm_d) && (gAppUseNvm_d > 0)
    if(NvIdle() == 0)
#endif /* gAppUseNvm_d */
    {
#if defined (gAppOtaASyncFlashTransactions_c) && (gAppOtaASyncFlashTransactions_c > 0)
        (void)OTA_TransactionResume();
#endif
    }
}

/*! *********************************************************************************
*\private
*\fn           void BluetoothLEHost_IsConnectivityTaskToProcess(void)
*\brief        Returns if there is Connectivity background task to process.
*
*\param  [in]  none.
*
*\retval       TRUE     If there is a connectivity task to process
*\retval       FALSE    If there is no connectivity task to process
********************************************************************************** */
bool_t BluetoothLEHost_IsConnectivityTaskToProcess(void)
{
    bool_t isConnectivityTaskToProcess = FALSE;
    do
    {
#if defined(gAppUseNvm_d) && (gAppUseNvm_d > 0)
        if(NvIsPendingOperation())
        {
            isConnectivityTaskToProcess = TRUE;
            break;
        }
#endif /* gAppUseNvm_d */

#if defined (gAppOtaASyncFlashTransactions_c) && (gAppOtaASyncFlashTransactions_c > 0)
        if(OTA_IsTransactionPending())
        {
            isConnectivityTaskToProcess = TRUE;
            break;
        }
#endif
    }while(false);

    return isConnectivityTaskToProcess;
}