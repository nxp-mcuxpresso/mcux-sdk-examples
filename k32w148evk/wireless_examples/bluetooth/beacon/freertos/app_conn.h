/*! *********************************************************************************
* Copyright 2020-2024 NXP
*
*
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef APP_CONN_H
#define APP_CONN_H

/*! *********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */
#include "EmbeddedTypes.h"
#include "fsl_component_button.h"
#include "fsl_component_led.h"
#include "fsl_os_abstraction.h"
#include "l2ca_cb_interface.h"
#include "ble_config.h"

/* Bluetooth Low Energy */
#include "gatt_interface.h"
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "ble_init.h"
#include "ble_config.h"
#include "l2ca_cb_interface.h"
#include "ble_constants.h"

#if !defined(gUseHciTransportDownward_d) || (!gUseHciTransportDownward_d)
#include "controller_interface.h"
#endif /* gUseHciTransportDownward_d */

#include "fsl_component_messaging.h"

/*! *********************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
********************************************************************************** */
typedef void* appCallbackParam_t;
typedef void (*appCallbackHandler_t)(appCallbackParam_t param);
typedef void (*gapConnManagerHandler_t)
(
    deviceId_t            peerDeviceId,
    gapConnectionEvent_t* pConnectionEvent
);
typedef void (*appAdvertiserHandler_t)(gapGenericEventType_t  eventType);

/* Host to Application Messages Types */
typedef enum {
    gAppGapGenericMsg_c = 0,
    gAppGapConnectionMsg_c,
    gAppGapAdvertisementMsg_c,
    gAppGapScanMsg_c,
    gAppGattServerMsg_c,
    gAppGattClientProcedureMsg_c,
    gAppGattClientNotificationMsg_c,
    gAppGattClientIndicationMsg_c,
    gAppL2caLeDataMsg_c,
    gAppL2caLeControlMsg_c,
    gAppSecLibMultiplyMsg_c,
} appHostMsgType_t;

/* Host to Application GATT Server Message */
typedef struct gattServerMsg_tag
{
    deviceId_t          deviceId;
    gattServerEvent_t   serverEvent;
} gattServerMsg_t;

/* Host to Application GATT Client Procedure Message */
typedef struct gattClientProcMsg_tag
{
    deviceId_t              deviceId;
    gattProcedureType_t     procedureType;
    gattProcedureResult_t   procedureResult;
    bleResult_t             error;
} gattClientProcMsg_t;

/* Host to Application GATT Client Notification/Indication Message */
typedef struct gattClientNotifIndMsg_tag
{
    uint8_t*    aValue;
    uint16_t    characteristicValueHandle;
    uint16_t    valueLength;
    deviceId_t  deviceId;
} gattClientNotifIndMsg_t;

/* L2ca to Application Data Message */
typedef struct l2caLeCbDataMsg_tag
{
    deviceId_t  deviceId;
    uint16_t    channelId;
    uint16_t    packetLength;
    uint8_t     aPacket[1];
} l2caLeCbDataMsg_t;

/* Host to Application Connection Message */
typedef struct connectionMsg_tag
{
    deviceId_t              deviceId;
    gapConnectionEvent_t    connEvent;
} connectionMsg_t;

/* Sec Lib to Application Data Message */
typedef struct secLibMsgData_tag
{
    computeDhKeyParam_t *pData;
} secLibMsgData_t;

typedef struct appMsgFromHost_tag
{
    uint32_t    msgType;
    union {
        gapGenericEvent_t       genericMsg;
        gapAdvertisingEvent_t   advMsg;
        connectionMsg_t         connMsg;
        gapScanningEvent_t      scanMsg;
        gattServerMsg_t         gattServerMsg;
        gattClientProcMsg_t     gattClientProcMsg;
        gattClientNotifIndMsg_t gattClientNotifIndMsg;
        l2caLeCbDataMsg_t       l2caLeCbDataMsg;
        l2capControlMessage_t   l2caLeCbControlMsg;
        secLibMsgData_t         secLibMsgData;
    } msgData;
} appMsgFromHost_t;

/*! Callback for notifying application upon Bluetooth LE stack initialization */
typedef void (*appBluetoothLEInitCompleteCallback_t)(void);

/*! *********************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
********************************************************************************** */

/*! Enable/disable use NV flash procedures for operations triggered by the host stack
    Do not modify directly. Redefine it in the app_preinclude.h file*/
#ifndef gAppUseNvm_d
#define gAppUseNvm_d                    (FALSE)
#endif /* gAppUseNvm_d */

/* Application Events */
#define gAppEvtMsgFromHostStack_c       (1U << 0U)
#define gAppEvtAppCallback_c            (1U << 1U)

#if (defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U))
#define App_NvmWriteLocalIRK(pLocalKey)   App_NvmWriteLocalKeys(0U, pLocalKey)
#define App_NvmWriteLocalCSRK(pLocalKey)  App_NvmWriteLocalKeys(1U, pLocalKey)
#define App_NvmReadLocalIRK(pLocalKey)    App_NvmReadLocalKeys(0U, pLocalKey)
#define App_NvmReadLocalCSRK(pLocalKey)   App_NvmReadLocalKeys(1U, pLocalKey)
#endif /* (defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U)) */
/*! *********************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */
extern gapConnectionCallback_t  pfConnCallback;
extern OSA_EVENT_HANDLE_DEFINE(mAppEvent);
extern messaging_t mHostAppInputQueue;

/*! *********************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
********************************************************************************** */
#if defined(gUseHciTransportUpward_d) && (gUseHciTransportUpward_d)
#define App_GenericCallback(param)
#else /* gUseHciTransportUpward_d */
/*! *********************************************************************************
*\fn           bleResult_t App_GenericCallback(gapGenericEvent_t* pGenericEvent)
*\brief        Callback used by the Host Stack to propagate GAP generic
*              events to the application.
*
*\param  [in]  pGenericEvent    Pointer to the GAP generic event.
*
*\retval       void.
********************************************************************************** */
void App_GenericCallback
(
    gapGenericEvent_t *pGenericEvent
);
#endif /* gUseHciTransportUpward_d */

/*! *********************************************************************************
*\fn           void BluetoothLEHost_AppInit(void)
*\brief        This is the initialization function for each application. This
*              function should contain all the initialization code required by the
*              bluetooth demo.
*
*\param  [in]  none.
*
*\retval       void.
********************************************************************************** */
void BluetoothLEHost_AppInit(void);

/*! *********************************************************************************
*\fn           void BluetoothLEHost_Init(
*                  appBluetoothLEInitCompleteCallback_t pCallback
*              )
*\brief        This is the host initialization function (initialize the Bluetooth
*              LE stack and also all framework and controller required components).
*
*\param  [in]  pCallback    Callback triggered when the initialization is
*                           complete.
*
*\retval       void.
********************************************************************************** */
void BluetoothLEHost_Init
(
    appBluetoothLEInitCompleteCallback_t pCallback
);

/*! *********************************************************************************
*\fn           void BluetoothLEHost_HandleMessages(void)
*\brief        This function is responsible for consuming all events coming from the
*              Bluetooth LE stack.
*
*\param  [in]  none.
*
*\retval       void.
********************************************************************************** */
void BluetoothLEHost_HandleMessages(void);

/*! *********************************************************************************
*\fn           void BluetoothLEHost_IsMessagePending(void)
*\brief        This function checks whether Messages are pending to be processed.
*
*\param  [in]  none.
*
*\retval       TRUE if pending messages.
********************************************************************************** */
bool BluetoothLEHost_IsMessagePending(void);

/*! *********************************************************************************
*\fn           void BluetoothLEHost_SetGenericCallback(
*                  gapGenericCallback_t pfGenericCallback
*              )
*\brief        Set advanced generic callback. Set a callback to receive all Bluetooth
*              LE stack generic events.
*
*\param  [in]  pfGenericCallback      Callback used by the application to receive
*                                     all stack generic events.
*
*\retval       void.
********************************************************************************** */
void BluetoothLEHost_SetGenericCallback
(
    gapGenericCallback_t pfGenericCallback
);

/*! *********************************************************************************
*\fn           bleResult_t BluetoothLEHost_Connect(
*                  gapConnectionRequestParameters_t*   pParameters,
*                  gapConnectionCallback_t             connCallback
*              )
*\brief        Start connection using the connection parameters specified.
*
*\param  [in]  pParameters     Pointer to the connection parameters.
*\param  [in]  connCallback    Callback used to receive connection events.
*
*\return       bleResult_t     Result of the operation.
*
*\remarks      This function should be used by the application if the callback
*              should be executed in the context of the Application Task.
********************************************************************************** */
bleResult_t BluetoothLEHost_Connect
(
    gapConnectionRequestParameters_t*   pParameters,
    gapConnectionCallback_t             connCallback
);

/*! *********************************************************************************
*\fn           void App_ConnectionCallback(deviceId_t            peerDeviceId,
*                                          gapConnectionEvent_t* pConnectionEvent);
*\brief        Sends the GAP Connection Event triggered by the Host Stack to the
*              application.
*
*\param  [in]  peerDeviceId        The id of the peer device.
*\param  [in]  pConnectionEvent    Pointer to the connection event.
*
*\retval       void.
********************************************************************************** */
void App_ConnectionCallback
(
    deviceId_t            peerDeviceId,
    gapConnectionEvent_t* pConnectionEvent
);

/*! *********************************************************************************
*\fn           bleResult_t App_RegisterGattClientNotificationCallback(
*                  gattClientNotificationCallback_t  callback
*              )
*\brief        Application wrapper function for GattClient_RegisterNotificationCallback.
*
*\param  [in]  callback       Application defined callback to be triggered by this
*                             module.
*
*\return       bleResult_t    Result of the operation.
*
*\remarks      This function should be used by the application if the callback should
*              be executed in the context of the Application Task.
********************************************************************************** */
bleResult_t App_RegisterGattClientNotificationCallback
(
    gattClientNotificationCallback_t  callback
);

/*! *********************************************************************************
*\fn           bleResult_t App_RegisterGattClientIndicationCallback(
*                  gattClientIndicationCallback_t  callback
*              )
*\brief        Application wrapper function for GattClient_RegisterIndicationCallback.
*
*\param  [in]  callback       Application defined callback to be triggered by this
*                             module.
*
*\return       bleResult_t    Result of the operation.
*
*\remarks      This function should be used by the application if the callback should
*              be executed in the context of the Application Task.
********************************************************************************** */
bleResult_t App_RegisterGattClientIndicationCallback
(
    gattClientIndicationCallback_t  callback
);

/*! *********************************************************************************
*\fn           bleResult_t App_RegisterGattServerCallback(
*                  gattServerCallback_t  serverCallback
*              )
*\brief        Application wrapper function for GattServer_RegisterCallback.
*
*\param  [in]  callback       Application-defined callback to be triggered by this
*                             module.
*
*\return       bleResult_t    Result of the operation.
*
*\remarks      This function should be used by the application if the callback should
*              be executed in the context of the Application Task.
********************************************************************************** */
bleResult_t App_RegisterGattServerCallback
(
    gattServerCallback_t  serverCallback
);

/*! *********************************************************************************
*\fn           bleResult_t App_RegisterGattClientProcedureCallback(
*                  gattClientProcedureCallback_t  callback
*              )
*\brief        Application wrapper function for App_RegisterGattClientProcedureCallback.
*
*\param  [in]  callback       Application-defined callback to be triggered by this
*                             module.
*
*\return       bleResult_t    Result of the operation.
*
*\remarks      This function should be used by the application if the callback should
*              be executed in the context of the Application Task.
********************************************************************************** */
bleResult_t App_RegisterGattClientProcedureCallback
(
    gattClientProcedureCallback_t  callback
);

/*! *********************************************************************************
*\fn           bleResult_t App_RegisterLeCbCallbacks(
*                  l2caLeCbDataCallback_t      pCallback,
*                  l2caLeCbControlCallback_t   pCtrlCallback
*              )
*\brief        Application wrapper function for L2ca_RegisterLeCbCallbacks.
*
*\param  [in]  callback       Application-defined callback to be triggered by this
*                             module.
*
*\return       bleResult_t    Result of the operation.
*
*\remarks      This function should be used by the application if the callback should
*              be executed in the context of the Application Task.
********************************************************************************** */
bleResult_t App_RegisterLeCbCallbacks
(
    l2caLeCbDataCallback_t      pCallback,
    l2caLeCbControlCallback_t   pCtrlCallback
);

/*! *********************************************************************************
*\fn           bleResult_t App_PostCallbackMessage(
*                  appCallbackHandler_t   handler
*                  appCallbackParam_t     param
*              )
*\brief        Posts an application event containing a callback handler and parameter.
*
*\param  [in]  handler         Handler function, to be executed when the event is
*                              processed.
*\param  [in]  param           Parameter for the handler function.
*
*\return       bleResult_t     Result of the operation.
*
*\remarks      This function should be used by the application if a callback must
*              be executed in the context of the Application Task.
********************************************************************************** */
bleResult_t App_PostCallbackMessage
(
    appCallbackHandler_t   handler,
    appCallbackParam_t     param
);

/*! *********************************************************************************
*\fn           bleResult_t App_NvmErase(uint8_t mEntryIdx)
*\brief        This function erases the data corresponding to an entry.
*
*\param  [in]  mEntryIdx              NVM entry index.
*
\return       bleResult_t    Result of the operation.
********************************************************************************** */
bleResult_t App_NvmErase(uint8_t mEntryIdx);

/*! *********************************************************************************
*\fn           bleResult_t App_NvmWrite(
*                  uint8_t  mEntryIdx,
*                  void*    pBondHeader,
*                  void*    pBondDataDynamic,
*                  void*    pBondDataStatic,
*                  void*    pBondDataLegacy,
*                  void*    pBondDataDeviceInfo,
*                  void*    pBondDataDescriptor,
*                  uint8_t  mDescriptorIndex
*               )
*\brief        Write data to NVM.
*
*\param  [in]  mEntryIdx              NVM entry index.
*\param  [in]  pBondHeader            Pointer to the bonding header.
*                                     Can be NULL.
*\param  [in]  pBondDataDynamic       Pointer to the dynamic bonding data
*                                     structure. Can be NULL.
*\param  [in]  pBondDataStatic        Pointer to the static bonding data
*                                     structure. Can be NULL.
*\param  [in]  pBondDataLegacy        Pointer to the legacy bonding data
*                                     structure. Can be NULL.
*\param  [in]  pBondDataDeviceInfo    Pointer to the bonding data device info
*                                     structure. Can be NULL.
*\param  [in]  pBondDataDescriptor    Pointer to the bonding data descriptor.
*                                     Can be NULL.
*\param  [in]  mDescriptorIndex       Bonding data descriptor index.
*
\return       bleResult_t    Result of the operation.
********************************************************************************** */
bleResult_t App_NvmWrite
(
    uint8_t  mEntryIdx,
    void*    pBondHeader,
    void*    pBondDataDynamic,
    void*    pBondDataStatic,
    void*    pBondDataLegacy,
    void*    pBondDataDeviceInfo,
    void*    pBondDataDescriptor,
    uint8_t  mDescriptorIndex
);

/*! *********************************************************************************
*\fn           bleResult_t App_NvmRead(
*                  uint8_t  mEntryIdx,
*                  void*    pBondHeader,
*                  void*    pBondDataDynamic,
*                  void*    pBondDataStatic,
*                  void*    pBondDataLegacy,
*                  void*    pBondDataDeviceInfo,
*                  void*    pBondDataDescriptor,
*                  uint8_t  mDescriptorIndex
*              )
*\brief        Read data from NVM.
*
*\param  [in]  mEntryIdx              NVM entry index.
*\param  [in]  pBondHeader            Pointer to the place where the the bonding
*                                     header will be read. Can be NULL.
*\param  [in]  pBondDataDynamic       Pointer to the place where the the dynamic
*                                     bonding data structure will be read.
*                                     Can be NULL.
*\param  [in]  pBondDataStatic        Pointer to the place where the static bonding
*                                     data structure will be read. Can be NULL.
*\param  [in]  pBondDataLegacy        Pointer to the place where the legacy bonding
*                                     data structure will be read. Can be NULL.
*\param  [in]  pBondDataDeviceInfo    Pointer to the place where the bonding data
*                                     device info structure will be read.
*                                     Can be NULL.
*\param  [in]  pBondDataDescriptor    Pointer to the place where the bonding data
*                                     descriptor will be read. Can be NULL.
*\param  [in]  mDescriptorIndex       Bonding data descriptor index.
*
\return       bleResult_t    Result of the operation.
********************************************************************************** */
bleResult_t App_NvmRead
(
    uint8_t  mEntryIdx,
    void*    pBondHeader,
    void*    pBondDataDynamic,
    void*    pBondDataStatic,
    void*    pBondDataLegacy,
    void*    pBondDataDeviceInfo,
    void*    pBondDataDescriptor,
    uint8_t  mDescriptorIndex
);

#if (defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U))
/*! *********************************************************************************
*\fn           bleResult_t App_NvmWriteLocalKeys(uint8_t  mEntryIdx,
*                                                 void*   pLocalKey )
*\brief        Write local key to NVM.
*
*\param  [in]  mEntryIdx              NVM entry index 0/1 - local IRK/CSRK.
*\param  [in]  pLocalKey              Pointer to a structure of type bleLocalKeysBlob_t containing the key blob .
* \return    bleResult_t
*
********************************************************************************** */
bleResult_t App_NvmWriteLocalKeys
(
    uint8_t  mEntryIdx,
    void*    pLocalKey
);


/*! *********************************************************************************
*\fn        bleResult_t App_NvmReadLocalKeys(uint8_t  mEntryIdx,
*                                            void*    pLocalKey)
*\brief      Read local key from NVM.
*
*\param[in]  mEntryIdx              NVM entry index 0/1 - local IRK/CSRK.
*\param[in]  pLocalKey              Pointer to a structure of type bleLocalKeysBlob_t 
*                                   where the key blob will be read.
*
* \return  bleResult_t
********************************************************************************** */
bleResult_t App_NvmReadLocalKeys
(
    uint8_t  mEntryIdx,
    void*    pLocalKey
);

#endif /* (defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U)) */
/*! *********************************************************************************
*\private
*\fn           void BluetoothLEHost_ProcessIdleTask(void)
*\brief        Handles Connectivity background task, usually executed from Idle task.
*
*\param  [in]  none.
*
*\retval       void.
********************************************************************************** */
void BluetoothLEHost_ProcessIdleTask(void);

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
bool_t BluetoothLEHost_IsConnectivityTaskToProcess(void);

#if defined(SDK_OS_FREE_RTOS)
/*! *********************************************************************************
*\fn           void vApplicationIdleHook(void)
*\brief        Idle hook function which places the microcontroller into a power
*              saving mode.
*
*\param  [in]  none.
*
*\retval       void.
********************************************************************************** */
void vApplicationIdleHook(void);

#endif /* defined(SDK_OS_FREE_RTOS) */

#endif /* APP_CONN_H */
