/*! *********************************************************************************
* \addtogroup Digital Key Car Anchor
* @{
********************************************************************************** */
/*! *********************************************************************************
* \file digital_key_car_anchor.c
*
* Copyright 2020-2024 NXP
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
#include "fsl_component_button.h"
#include "fsl_component_timer_manager.h"
#include "fsl_component_panic.h"
#include "FunctionLib.h"
#include "fsl_component_mem_manager.h"
#include "fsl_adapter_reset.h"
#include "fsl_debug_console.h"
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

/* Profile / Services */
#include "digital_key_interface.h"

#include "ble_conn_manager.h"
#include "ble_service_discovery.h"

#include "app_conn.h"
#include "digital_key_car_anchor.h"
#include "shell_digital_key_car_anchor.h"
#include "app_digital_key_car_anchor.h"

#include "gap_handover_types.h"
#include "gap_handover_interface.h"
#include "app_a2a_interface.h"
#include "app_handover.h"
/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
/* Time Sync UWB Device Time. Demo value. */
uint64_t mTsUwbDeviceTime = 0U;

/* Table with peer devices information */
appPeerInfo_t maPeerInformation[gAppMaxConnections_c];

/* This global will be TRUE if the user adds or removes a bond */
bool_t gPrivacyStateChangedByUser = FALSE;

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
typedef struct advState_tag{
    bool_t      advOn;
}advState_t;
/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
#if (defined(gAppScanNonCCC_d) && (gAppScanNonCCC_d == 1U))

static appScanningParams_t appScanParams = {
    &gScanParams,
    gGapDuplicateFilteringEnable_c,
    gGapScanContinuously_d,
    gGapScanPeriodicDisabled_d
};

/* Scan state */
static bool_t mScanningOn = FALSE;
static bool_t mFoundPeripheralToConnect = FALSE;

#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
static bool_t mRestoringBondedLink = FALSE;
#endif /* gAppUseBonding_d */

#endif /* gAppScanNonCCC_d */

/* Own address used during discovery. Included in First Approach Response. */
static uint8_t gaAppOwnDiscAddress[gcBleDeviceAddressSize_c];

/* Application timer*/
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
static TIMER_MANAGER_HANDLE_DEFINE(mAppTimerId);
#endif
static TIMER_MANAGER_HANDLE_DEFINE(mL2caTimerId);
static bool_t mL2caTimerValid = FALSE;
/* Owner Pairing Mode enabled */
static bool_t mOwnerPairingMode = TRUE;

/* Current Advertising Set */
static uint8_t gCurrentAdvHandle = (uint8_t)gLegacyAdvSetHandle_c;
/* Holds the identification information about the device are we doing OOB pairing with */
static deviceId_t mCurrentPeerId = gInvalidDeviceId_c;
/* Application specific data*/
static deviceId_t mDeviceIdToDisconnect;

/* application callback */
static pfBleCallback_t mpfBleEventHandler = NULL;
static pfBleCallback_t mpfBleUserInterfaceEventHandler = NULL;
static bool_t gStopExtAdvSetAfterConnect = FALSE;

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/
/* Host, Gatt and Att callbacks */
#if (defined(gAppScanNonCCC_d) && (gAppScanNonCCC_d == 1U))
static void BleApp_ScanningCallback(gapScanningEvent_t* pScanningEvent);
static void BleApp_ServiceDiscoveryCallback(deviceId_t peerDeviceId, servDiscEvent_t* pEvent);

static void BleApp_GattClientCallback(
    deviceId_t              serverDeviceId,
    gattProcedureType_t     procedureType,
    gattProcedureResult_t   procedureResult,
    bleResult_t             error
);
#endif /* (defined(gAppScanNonCCC_d) && (gAppScanNonCCC_d == 1U)) */

static void BleApp_GattServerCallback (deviceId_t deviceId, gattServerEvent_t* pServerEvent);

static void L2caTimerCallback(void *param);

/* PSM callbacks */
static void BleApp_L2capPsmDataCallback (deviceId_t deviceId, uint16_t lePsm, uint8_t* pPacket, uint16_t packetLength);
static void BleApp_L2capPsmControlCallback (l2capControlMessage_t* pMessage);

/* Timer Callbacks */
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
static void AdvertisingTimerCallback(void *pParam);
#endif

static void BleApp_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent);
static void BleApp_Advertise(void);

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
static void SleepTimeoutSequence(void);
#endif

static uint8_t BleApp_GetNoOfActiveConnections(void);

static void BleApp_StateMachineHandler_AppPair(deviceId_t peerDeviceId, appEvent_t event);
static void BleApp_StateMachineHandler_AppIdle(deviceId_t peerDeviceId, appEvent_t event);
static void BleApp_StateMachineHandler_AppCCCWaitingForOwnerPairingRequest(deviceId_t peerDeviceId, appEvent_t event);
static void BleApp_StateMachineHandler_AppCCCPhase2WaitingForResponse(deviceId_t peerDeviceId, appEvent_t event);
#if (defined(gAppScanNonCCC_d) && (gAppScanNonCCC_d == 1U))
static void BleApp_StateMachineHandler_AppExchangeMtu(deviceId_t peerDeviceId, appEvent_t event);
#endif
static void BleApp_GenericCallback_ControllerNotificationEvent(gapGenericEvent_t* pGenericEvent);
static void BleApp_GenericCallback_HandlePrivacyEvents(gapGenericEvent_t* pGenericEvent);
/* Get a simulated UWB clock. */
static uint64_t GetUwbClock(void);

#if defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U)
static void BleApp_A2BCommHandler(uint8_t opGroup, uint8_t cmdId, uint16_t len, uint8_t *pData);
#endif /* defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U) */
/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
 * \brief        Register function to handle events from BLE to APP
 *
 * \param[in]    pfBleEventHandler   event handler
 ********************************************************************************** */
void BleApp_RegisterEventHandler(pfBleCallback_t pfBleEventHandler)
{
    mpfBleEventHandler = pfBleEventHandler;
}

/*! *********************************************************************************
 * \brief        Register function to handle User Interface events from BLE to APP
 *
 * \param[in]    pfBleEventHandler   event handler
 ********************************************************************************** */
void BleApp_RegisterUserInterfaceEventHandler(pfBleCallback_t pfUserInterfaceEventHandler)
{
    mpfBleUserInterfaceEventHandler = pfUserInterfaceEventHandler;
}

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(void)
{
    Led1On();

#if (defined(gAppScanNonCCC_d) && (gAppScanNonCCC_d == 1U))
    if(!mScanningOn)
    {
        /* Start scanning for non-CCC key fobs */
        (void)BluetoothLEHost_StartScanning(&appScanParams, BleApp_ScanningCallback);
    }
#endif /* (defined(gAppScanNonCCC_d) && (gAppScanNonCCC_d == 1U)) */

    /* Set advertising parameters, advertising to start on gAdvertisingParametersSetupComplete_c */
    gCurrentAdvHandle = (uint8_t)gLegacyAdvSetHandle_c;
    BleApp_Advertise();
}

#if defined(gHandoverDemo_d) && (gHandoverDemo_d == 1)
/*! *********************************************************************************
* \brief        Get next active peer for handover.
*
********************************************************************************** */
deviceId_t BleApp_SelectDeviceIdForHandover(void)
{
    static uint8_t devIdIdx = 0U;
    deviceId_t deviceId = gInvalidDeviceId_c;

    for (uint32_t i = 0U; i < gAppMaxConnections_c; i++)
    {
        devIdIdx++;
        
        if (devIdIdx == gAppMaxConnections_c)
        {
            devIdIdx = 0U;
        }
        
        if (maPeerInformation[devIdIdx].deviceId != gInvalidDeviceId_c)
        {
            deviceId = maPeerInformation[devIdIdx].deviceId;
            break;
        }
        
    }
    
    return deviceId;
}
#endif /* gHandoverDemo_d */

/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
void BleApp_GenericCallback (gapGenericEvent_t* pGenericEvent)
{
#if defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U)
#if defined(gA2BInitiator_d) && (gA2BInitiator_d == 0)
    if ((TRUE == gA2ALocalIrkSet) ||
        ((FALSE == gA2ALocalIrkSet) && (pGenericEvent->eventType != gInitializationComplete_c)))
    {
#endif /* defined(gA2BInitiator_d) && (gA2BInitiator_d == 0) */
#endif /* defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U) */
    /* Call BLE Conn Manager */
    BleConnManager_GenericEvent(pGenericEvent);
#if defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U)
#if defined(gA2BInitiator_d) && (gA2BInitiator_d == 0)
    }
    else
    {
        /* Postpone processing of the gInitializationComplete_c generic event until the local IRK is set. */
        if (NULL == gpBleHostInitComplete)
        {
            gpBleHostInitComplete = MEM_BufferAlloc(sizeof(gapGenericEvent_t));
        }
        
        if (NULL != gpBleHostInitComplete)
        {
            FLib_MemCpy(gpBleHostInitComplete, pGenericEvent, sizeof(gapGenericEvent_t));
        }
        else
        {
            panic(0,0,0,0);
        }
    }
#endif /* defined(gA2BInitiator_d) && (gA2BInitiator_d == 0) */
#endif /* defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U) */
    switch (pGenericEvent->eventType)
    {
#if defined(gHandoverDemo_d) && (gHandoverDemo_d == 1)
        case gHandoverAnchorSearchStarted_c:
        case gHandoverAnchorSearchStopped_c:
        case gHandoverAnchorMonitorEvent_c:
        case gHandoverSuspendTransmitComplete_c:
        case gHandoverGetComplete_c:
        case gGetConnParamsComplete_c:
        case gHandoverTimeSyncEvent_c:
        case gHandoverTimeSyncTransmitStateChanged_c:
        case gHandoverTimeSyncReceiveComplete_c:
        case gLlSkdReportEvent_c:
        case gInternalError_c:
        case gHandoverAnchorMonitorPacketEvent_c:
        case gHandoverAnchorMonitorPacketContinueEvent_c:
        case gHandoverFreeComplete_c:
        case gHandoverLlPendingData_c:
        {
            AppHandover_GenericCallback(pGenericEvent);
        }
        break;
#endif /* gHandoverDemo_d */

        case gLePhyEvent_c:
        {
            if(pGenericEvent->eventData.phyEvent.phyEventType == gPhyUpdateComplete_c )
            {
                /* Inform application about Le Phy user interface event */
                if(mpfBleUserInterfaceEventHandler != NULL)
                {
                    appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t) + sizeof(gapPhyEvent_t));
                    if(pEventData != NULL)
                    {
                        pEventData->appEvent = mAppEvt_LePhyEvent_c;
                        pEventData->eventData.pData = &pEventData[1];
                        FLib_MemCpy(pEventData->eventData.pData, &pGenericEvent->eventData.phyEvent, sizeof(gapPhyEvent_t));  
                        if (gBleSuccess_c != App_PostCallbackMessage(mpfBleUserInterfaceEventHandler, pEventData))
                        {
                           (void)MEM_BufferFree(pEventData);
                        }
                    }
                }
            }
        }
        break;

        case gLeScLocalOobData_c:
        {
            /* Inform application about LE SC Local OOB Data event */
            if(mpfBleEventHandler != NULL)
            {
                appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t) + sizeof(gapLeScOobData_t));
                if(pEventData != NULL)
                {
                    pEventData->appEvent = mAppEvt_GenericCallback_LeScLocalOobData_c;
                    pEventData->eventData.pData = &pEventData[1];
                    FLib_MemCpy(pEventData->eventData.pData, &pGenericEvent->eventData.localOobData, sizeof(gapLeScOobData_t));  
                    if (gBleSuccess_c != App_PostCallbackMessage(mpfBleEventHandler, pEventData))
                    {
                       (void)MEM_BufferFree(pEventData);
                    }
                }
            }
        }
        break;

        case gBondCreatedEvent_c:
        {
            /* Inform application about bond created event */
            if(mpfBleEventHandler != NULL)
            {
                appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t) + sizeof(bleBondCreatedEvent_t));
                if(pEventData != NULL)
                {
                    pEventData->appEvent = mAppEvt_GenericCallback_BondCreatedEvent_c;
                    pEventData->eventData.pData = &pEventData[1];
                    FLib_MemCpy(pEventData->eventData.pData, &pGenericEvent->eventData.bondCreatedEvent, sizeof(bleBondCreatedEvent_t));  
                    if (gBleSuccess_c != App_PostCallbackMessage(mpfBleEventHandler, pEventData))
                    {
                       (void)MEM_BufferFree(pEventData);
                    }
                }
            }
        }
        break;

        case gRandomAddressReady_c:
        {
            if (pGenericEvent->eventData.addrReady.advHandle == gLegacyAdvSetHandle_c)
            {
                FLib_MemCpy(gaAppOwnDiscAddress, pGenericEvent->eventData.addrReady.aAddress, gcBleDeviceAddressSize_c);
            }
        }
        break;
        
        case gControllerNotificationEvent_c:
        {   
            BleApp_GenericCallback_ControllerNotificationEvent(pGenericEvent);
        }
        break;
        
        case gHostPrivacyStateChanged_c:
        case gControllerPrivacyStateChanged_c:
        {
            BleApp_GenericCallback_HandlePrivacyEvents(pGenericEvent);
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
    if (pConnectionEvent->eventType == gConnEvtConnected_c)
    {
        if (pConnectionEvent->eventData.connectedEvent.connectionRole == gBleLlConnectionCentral_c)
        {
            maPeerInformation[peerDeviceId].gapRole = gGapCentral_c;
        }
        else
        {
            maPeerInformation[peerDeviceId].gapRole = gGapPeripheral_c;
        }
    }

    /* Connection Manager to handle Host Stack interactions */
    if (maPeerInformation[peerDeviceId].gapRole == gGapCentral_c)
    {
        BleConnManager_GapCentralEvent(peerDeviceId, pConnectionEvent);
    }
    else
    {
        BleConnManager_GapPeripheralEvent(peerDeviceId, pConnectionEvent);
    }

    switch (pConnectionEvent->eventType)
    {
#if defined(gHandoverDemo_d) && (gHandoverDemo_d)
        case gConnEvtHandoverConnected_c:
        case gHandoverDisconnected_c:
        {
            AppHandover_ConnectionCallback(peerDeviceId, pConnectionEvent);
        }
        break;
#endif /* defined(gHandoverDemo_d) && (gHandoverDemo_d) */
        case gConnEvtConnected_c:
        {
            bleResult_t result = gBleSuccess_c;
            /* Save address used during discovery if controller privacy was used. */
            if (pConnectionEvent->eventData.connectedEvent.localRpaUsed)
            {
                FLib_MemCpy(gaAppOwnDiscAddress, pConnectionEvent->eventData.connectedEvent.localRpa, gcBleDeviceAddressSize_c);
            }

            /* Advertising stops when connected - on all PHYs */
            result = Gap_StopExtAdvertising(0xFF);
            
            if ((result == gBleInvalidState_c) && (gCurrentAdvHandle == gExtendedAdvSetHandle_c))
            {
                /* Connection was establised before the extended advertising set was started */
                gStopExtAdvSetAfterConnect = TRUE;
                gCurrentAdvHandle = (uint8_t)gNoAdvSetHandle_c;
            }
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
            (void)TM_Stop((timer_handle_t)mAppTimerId);
#endif

            maPeerInformation[peerDeviceId].deviceId = peerDeviceId;

#if (defined(gAppScanNonCCC_d) && (gAppScanNonCCC_d == 1U))
            (void)Gap_StopScanning();

            if (pConnectionEvent->eventData.connectedEvent.connectionRole == gBleLlConnectionCentral_c)
            {
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
                (void)Gap_CheckIfBonded(peerDeviceId, &maPeerInformation[peerDeviceId].isBonded, NULL);

                if (maPeerInformation[peerDeviceId].isBonded == TRUE)
                {
                    mRestoringBondedLink = TRUE;
                    /* Restored custom connection information. Encrypt link */
                    (void)Gap_EncryptLink(peerDeviceId);
                }
#endif  /* defined(gAppUseBonding_d) && (gAppUseBonding_d) */
            }
#endif /* (defined(gAppScanNonCCC_d) && (gAppScanNonCCC_d == 1U)) */

            BleApp_StateMachineHandler(maPeerInformation[peerDeviceId].deviceId, mAppEvt_PeerConnected_c);
            /* UI */
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
            PWR_AllowDeviceToSleep();
#else
            LedStopFlashingAllLeds();
#endif

            /* UI */
            Led1On();
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

#if gAppUsePairing_d
        case gConnEvtPairingRequest_c:
        {
            BleApp_StateMachineHandler(maPeerInformation[peerDeviceId].deviceId, mAppEvt_PairingReqRcv_c);
        }
        break;
        
        case gConnEvtLeScOobDataRequest_c:
        {
            BleApp_StateMachineHandler(peerDeviceId, mAppEvt_PairingPeerOobDataReq_c);
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
        
#if defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U)
        case gConnEvtEncryptionChanged_c:
        {            
#if (defined(gAppScanNonCCC_d) && (gAppScanNonCCC_d == 1U))
            if( mRestoringBondedLink )
            {
                mRestoringBondedLink = FALSE;
            }
#endif
            BleApp_StateMachineHandler(peerDeviceId, mAppEvt_EncryptionChanged_c);
        }
        break;

#if (defined(gAppScanNonCCC_d) && (gAppScanNonCCC_d == 1U))
        case gConnEvtAuthenticationRejected_c:
        {
            /* Start Pairing Procedure */
            (void)Gap_Pair(peerDeviceId, &gPairingParameters);
        }
        break;
#endif /* (defined(gAppScanNonCCC_d) && (gAppScanNonCCC_d == 1U)) */
#endif /* gAppUseBonding_d */
#endif /* gAppUsePairing_d */

        default:
        {
            ; /* No action required */
        }
        break;
    }
}

/*! *********************************************************************************
* \brief        Handles LeScLocalOobDataCallback events.
*
* \param[in]    pEventData    pointer to appEventData_t.
********************************************************************************** */
void App_HandleLeScLocalOobDataCallback(appEventData_t *pEventData)
{
    if (mCurrentPeerId != gInvalidDeviceId_c)
    {
        FLib_MemCpy(&maPeerInformation[mCurrentPeerId].oobData, pEventData->eventData.pData, sizeof(gapLeScOobData_t));
        BleApp_StateMachineHandler(mCurrentPeerId, mAppEvt_PairingLocalOobData_c);
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
* \brief    Starts the Passive Entry Scenario on the BLE application.
*
********************************************************************************** */
void BleApp_PE_Start(void)
{
    mOwnerPairingMode = FALSE;
    gAppAdvParams.pGapAdvData = &gAppAdvertisingDataEmpty;
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
    gLegacyAdvParams.extAdvProperties = (bleAdvRequestProperties_t)(gAdvReqConnectable_c | gAdvUseDecisionPDU_c | gAdvIncludeAdvAinDecisionPDU_c);
    gExtAdvParams.extAdvProperties = (bleAdvRequestProperties_t)(gAdvReqConnectable_c | gAdvIncludeTxPower_c | gAdvUseDecisionPDU_c | gAdvIncludeAdvAinDecisionPDU_c);
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
    BleApp_Start();
}

/*! *********************************************************************************
* \brief    Starts the Owner Pairing Scenario on the BLE application.
*
********************************************************************************** */
void BleApp_OP_Start(void)
{
    FLib_MemSet(gaAppOwnDiscAddress, 0x00, gcBleDeviceAddressSize_c);
    mOwnerPairingMode = TRUE;
    gAppAdvParams.pGapAdvData = &gAppAdvertisingData;
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
    gLegacyAdvParams.extAdvProperties = (bleAdvRequestProperties_t)(gAdvReqConnectable_c | gAdvReqScannable_c | gAdvReqLegacy_c);
    gExtAdvParams.extAdvProperties = (bleAdvRequestProperties_t)(gAdvReqConnectable_c | gAdvIncludeTxPower_c);
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
    BleApp_Start();
}

/*! *********************************************************************************
* \brief    Stop Discovery on the BLE application.
*
********************************************************************************** */
void BleApp_StopDiscovery(void)
{
    gCurrentAdvHandle = (uint8_t)gNoAdvSetHandle_c;
    (void)Gap_StopExtAdvertising(0xFF);
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
    (void)TM_Stop((timer_handle_t)mAppTimerId);
#endif
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
* \brief        State machine handler of the Digital Key Car Anchor application.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    event               Event type.
********************************************************************************** */
void BleApp_StateMachineHandler(deviceId_t peerDeviceId, appEvent_t event)
{
    switch (maPeerInformation[peerDeviceId].appState)
    {
        case mAppIdle_c:
        {
            BleApp_StateMachineHandler_AppIdle(peerDeviceId, event);
        }
        break;

 #if (defined(gAppScanNonCCC_d) && (gAppScanNonCCC_d == 1U))
        case mAppExchangeMtu_c:
        {
            BleApp_StateMachineHandler_AppExchangeMtu(peerDeviceId, event);
        }
        break;
#endif /* (defined(gAppScanNonCCC_d) && (gAppScanNonCCC_d == 1U)) */

        case mAppCCCWaitingForOwnerPairingRequest_c:
        {
            BleApp_StateMachineHandler_AppCCCWaitingForOwnerPairingRequest(peerDeviceId, event);
        }
        break;

        case mAppCCCPhase2WaitingForResponse_c:
        {
            BleApp_StateMachineHandler_AppCCCPhase2WaitingForResponse(peerDeviceId, event);
        }
        break;

        case mAppCCCPhase2WaitingForVerify_c:
        {
            if (event == mAppEvt_BlePairingReady_c)
            {
                maPeerInformation[peerDeviceId].appState = mAppCCCReadyForPairing_c;
            }
        }
        break;

        case mAppCCCReadyForPairing_c:
        {
            if ( event == mAppEvt_PairingPeerOobDataRcv_c )
            {
                mCurrentPeerId = peerDeviceId;
                (void)Gap_LeScGetLocalOobData();
                maPeerInformation[peerDeviceId].appState = mAppPair_c;
            }
        }
        break;
        
        case mAppPair_c:
        {
            BleApp_StateMachineHandler_AppPair(peerDeviceId,event);
        }
        break;

        case mAppServiceDisc_c:
        {
            if (event == mAppEvt_ServiceDiscoveryComplete_c)
            {
                /* Moving to Running State*/
                maPeerInformation[peerDeviceId].appState = mAppRunning_c;
#if gAppUseBonding_d
                /* Write data in NVM */
                (void)Gap_SaveCustomPeerInformation(maPeerInformation[peerDeviceId].deviceId,
                                                    (void *)&maPeerInformation[peerDeviceId].customInfo, 0,
                                                    (uint16_t)sizeof(appCustomInfo_t));
#endif
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
        
        default:
        {
            ; /* No action required */
        }
        break;
    }

    /* Handle disconnect event in all application states */
    if ( event == mAppEvt_PeerDisconnected_c )
    {
        maPeerInformation[peerDeviceId].deviceId = gInvalidDeviceId_c;
        maPeerInformation[peerDeviceId].appState = mAppIdle_c;
        maPeerInformation[peerDeviceId].isLinkEncrypted = FALSE;
    }

    /* Inform the user interface handler about events received in the BleApp_StateMachineHandler */
    if(mpfBleUserInterfaceEventHandler != NULL)
    {
         appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
         if(pEventData != NULL)
         {
             pEventData->appEvent = event;
             if (gBleSuccess_c != App_PostCallbackMessage(mpfBleUserInterfaceEventHandler, pEventData))
             {
                (void)MEM_BufferFree(pEventData);
             }
         } 
    }
}

/*! *********************************************************************************
 * \brief        Owner Pairing Certificate Exchange - step 1, SPAKE2+ Request Command
 *
 ********************************************************************************** */
bleResult_t CCCPhase2_SendSPAKERequest(deviceId_t deviceId, uint8_t *pData, uint16_t dataLen)
{
    bleResult_t result = gBleSuccess_c;

    result = DK_SendMessage(deviceId,
                            maPeerInformation[deviceId].customInfo.psmChannelId,
                            gDKMessageTypeFrameworkMessage_c,
                            gDkApduRQ_c,
                            dataLen,
                            pData);
    
    /* Inform the user interface handler that SPAKE Request has been sent */
    if(mpfBleUserInterfaceEventHandler != NULL)
    {
         appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
         if(pEventData != NULL)
         {
             pEventData->appEvent = mAppEvt_SPAKERequestSent_c;
             if (gBleSuccess_c != App_PostCallbackMessage(mpfBleUserInterfaceEventHandler, pEventData))
             {
                (void)MEM_BufferFree(pEventData);
             }
         } 
    }
    
    return result;
}

/*! *********************************************************************************
 * \brief        Owner Pairing Certificate Exchange - step 2, SPAKE2+ Verify Command
 *
 ********************************************************************************** */
bleResult_t CCCPhase2_SendSPAKEVerify(deviceId_t deviceId, uint8_t *pData, uint16_t dataLen)
{
    bleResult_t result = gBleSuccess_c;
    
    result = DK_SendMessage(deviceId,
                            maPeerInformation[deviceId].customInfo.psmChannelId,
                            gDKMessageTypeFrameworkMessage_c,
                            gDkApduRQ_c,
                            dataLen,
                            pData);
    
    /* Inform the user interface handler that SPAKE Verify has been sent */
    if(mpfBleUserInterfaceEventHandler != NULL)
    {
         appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
         if(pEventData != NULL)
         {
             pEventData->appEvent = mAppEvt_SPAKEVerifySent_c;
             if (gBleSuccess_c != App_PostCallbackMessage(mpfBleUserInterfaceEventHandler, pEventData))
             {
                (void)MEM_BufferFree(pEventData);
             }
         } 
    }
    return result;
}

/*! *********************************************************************************
 * \brief        First Approach messages enables BLE OOB Secure LE pairing for both owner and friend devices
 *
 ********************************************************************************** */
bleResult_t CCC_FirstApproachRsp(deviceId_t deviceId, uint8_t* pBdAddr, gapLeScOobData_t* pOobData)
{
    bleResult_t result = gBleSuccess_c;
    
    uint8_t aPayload[gFirstApproachReqRspPayloadLength] = {0}; 
    uint8_t *pData = aPayload;
    
    if (FLib_MemCmpToVal(pOobData, 0x00, sizeof(gapLeScOobData_t)))
    {
        result = gBleInvalidParameter_c;
    }
    else
    {
        FLib_MemCpy(pData, pBdAddr, gcBleDeviceAddressSize_c);
        pData = &pData[gcBleDeviceAddressSize_c];
        FLib_MemCpy(pData, pOobData->confirmValue, gSmpLeScRandomConfirmValueSize_c);
        pData = &pData[gSmpLeScRandomConfirmValueSize_c];
        FLib_MemCpy(pData, pOobData->randomValue, gSmpLeScRandomValueSize_c);
        
        result = DK_SendMessage(deviceId,
                                maPeerInformation[deviceId].customInfo.psmChannelId,
                                gDKMessageTypeSupplementaryServiceMessage_c,
                                gFirstApproachRS_c,
                                gFirstApproachReqRspPayloadLength,
                                aPayload);
    }
    
    return result;
}

/*! *********************************************************************************
 * \brief        Sends DK SubEvents to Device.
 *
 ********************************************************************************** */
bleResult_t CCC_SendSubEvent(deviceId_t deviceId,
                             dkSubEventCategory_t category,
                             dkSubEventCommandCompleteType_t type)
{
    bleResult_t result = gBleSuccess_c;

    uint8_t payload[gCommandCompleteSubEventPayloadLength_c] = {0}; /* SubEvent Category + SubEvent Type */
    payload[0] = (uint8_t)category;
    payload[1] = (uint8_t)type;

    result = DK_SendMessage(deviceId,
                            maPeerInformation[deviceId].customInfo.psmChannelId,
                            gDKMessageTypeDKEventNotification_c,
                            gDkEventNotification_c,
                            gCommandCompleteSubEventPayloadLength_c,
                            payload);

    return result;
}

/*! *********************************************************************************
 * \brief        Configures BLE Stack after initialization
 *
 ********************************************************************************** */
void BluetoothLEHost_Initialized(void)
{
#if defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U)
#if defined(gA2BInitiator_d) && (gA2BInitiator_d == 0)
    if (FALSE == gA2ALocalIrkSet)
    {
        /* do nothing */
    }
    else
#endif /* defined(gA2BInitiator_d) && (gA2BInitiator_d == 0) */
#endif /* defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U) */
    {
        /* Common GAP configuration */
        BleConnManager_GapCommonConfig();
    }
    
    /* Register for callbacks*/
    (void)App_RegisterGattServerCallback(BleApp_GattServerCallback);
#if (defined(gAppScanNonCCC_d) && (gAppScanNonCCC_d == 1U))
    (void)App_RegisterGattClientProcedureCallback(BleApp_GattClientCallback);
    BleServDisc_RegisterCallback(BleApp_ServiceDiscoveryCallback);
    mScanningOn = FALSE;
#endif /* (defined(gAppScanNonCCC_d) && (gAppScanNonCCC_d == 1U)) */

    /* Register DK L2CAP PSM */
    (void)L2ca_RegisterLePsm(gDK_DefaultVehiclePsm_c, gDKMessageMaxLength_c);

    /* Register stack callbacks */
    (void)App_RegisterLeCbCallbacks(BleApp_L2capPsmDataCallback, BleApp_L2capPsmControlCallback);

    /* Allocate application timers */
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
    (void)TM_Open(mAppTimerId);
#endif
    if (TM_Open(mL2caTimerId) == kStatus_TimerSuccess)
    {
        mL2caTimerValid = TRUE;
    }

    /* Inform the user interface handler that Bluetooth application configuration done */
    if(mpfBleUserInterfaceEventHandler != NULL)
    {
         appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
         if(pEventData != NULL)
         {
             pEventData->appEvent = mAppEvt_BleConfigDone_c;
             if (gBleSuccess_c != App_PostCallbackMessage(mpfBleUserInterfaceEventHandler, pEventData))
             {
                (void)MEM_BufferFree(pEventData);
             }
         } 
    }
    
    (void)Gap_ControllerEnhancedNotification(((uint32_t)gNotifConnCreated_c | (uint32_t)gNotifPhyUpdateInd_c), 0U);
#if (defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U))
    (void)A2B_Init(BleApp_A2BEventHandler, BleApp_A2BCommHandler);
#endif /* (defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U)) */
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief        Handler of gHostPrivacyStateChanged_c and 
*               gControllerPrivacyStateChanged_c from BleApp_GenericCallback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
static void BleApp_GenericCallback_HandlePrivacyEvents(gapGenericEvent_t* pGenericEvent)
{
    switch (pGenericEvent->eventType)
    {   
        case gHostPrivacyStateChanged_c:
        {
            if (!pGenericEvent->eventData.newHostPrivacyState)
            {
                if(gPrivacyStateChangedByUser)
                {
                    gPrivacyStateChangedByUser = FALSE;
                    /* Host privacy disabled because a bond was removed
                       or added. Enable privacy. */
                    (void)BleConnManager_EnablePrivacy();
                }
            }
        }
        break;
        
        case gControllerPrivacyStateChanged_c:
        {
            if (!pGenericEvent->eventData.newControllerPrivacyState)
            {
                if(gPrivacyStateChangedByUser)
                {
                    gPrivacyStateChangedByUser = FALSE;
                    /* Controller privacy disabled because a bond was removed
                       or added. Enable privacy. */
                    (void)BleConnManager_EnablePrivacy();
                }
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
* \brief        Handler of gControllerNotificationEvent_c from BleApp_GenericCallback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
static void BleApp_GenericCallback_ControllerNotificationEvent(gapGenericEvent_t* pGenericEvent)
{
    if ((pGenericEvent->eventData.notifEvent.eventType & ((uint16_t)gNotifConnCreated_c | (uint16_t)gNotifPhyUpdateInd_c)) > 0U )
    {
        uint64_t bleTime = TM_GetTimestamp();
        /* Get current timestamp */
        mTsUwbDeviceTime = GetUwbClock();
        /* Subtract event delay */       
        if (bleTime >= pGenericEvent->eventData.notifEvent.timestamp)
        {
            mTsUwbDeviceTime = mTsUwbDeviceTime - ((uint64_t)bleTime - (uint64_t)pGenericEvent->eventData.notifEvent.timestamp);
        }
        else
        {
            mTsUwbDeviceTime = mTsUwbDeviceTime - ((0x00000000FFFFFFFFU - (uint64_t)pGenericEvent->eventData.notifEvent.timestamp) + bleTime);
        }
    }
}

/*! *********************************************************************************
* \brief        Handler of the mAppCCCPhase2WaitingForResponse_c state for
*               BleApp_StateMachineHandler.            
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    event               Event type.
************************************************************************************/
static void BleApp_StateMachineHandler_AppCCCPhase2WaitingForResponse(deviceId_t peerDeviceId, appEvent_t event)
{
    if (event == mAppEvt_ReceivedSPAKEResponse_c)
    {
        /* SPAKE2+ Flow: Send Verify Command */
        /* Dummy data - replace with calls to get actual data */
        uint16_t payloadLen = gDummyPayloadLength_c;
        uint8_t payload[gDummyPayloadLength_c] = gDummyPayload_c;
        bleResult_t status = CCCPhase2_SendSPAKEVerify(peerDeviceId, payload, payloadLen);
        if (status == gBleSuccess_c)
        {
            maPeerInformation[peerDeviceId].appState = mAppCCCPhase2WaitingForVerify_c;
        }
    }
    else if (event == mAppEvt_EncryptionChanged_c)
    {
        maPeerInformation[peerDeviceId].isLinkEncrypted = TRUE;
        maPeerInformation[peerDeviceId].appState = mAppRunning_c;
    }
    else
    {
        /* For MISRA compliance */
    }
}

/*! *********************************************************************************
* \brief        Handler of the mAppExchangeMtu_c state for BleApp_StateMachineHandler.            
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    event               Event type.
************************************************************************************/
#if (defined(gAppScanNonCCC_d) && (gAppScanNonCCC_d == 1U))
static void BleApp_StateMachineHandler_AppExchangeMtu(deviceId_t peerDeviceId, appEvent_t event)
{
    if (event == mAppEvt_GattProcComplete_c)
    {
        /* Moving to Service Discovery State*/
        maPeerInformation[peerDeviceId].appState = mAppServiceDisc_c;

        /* Start Service Discovery*/
        (void)BleServDisc_Start(peerDeviceId);
    }
    else
    {
        if (event == mAppEvt_GattProcError_c)
        {
            (void)Gap_Disconnect(peerDeviceId);
        }
    }
}
#endif /* (defined(gAppScanNonCCC_d) && (gAppScanNonCCC_d == 1U)) */

/*! *********************************************************************************
* \brief        Handler of the mAppCCCWaitingForOwnerPairingRequest_c state for
*               BleApp_StateMachineHandler.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    event               Event type.
************************************************************************************/
static void BleApp_StateMachineHandler_AppPair(deviceId_t peerDeviceId, appEvent_t event)
{
    if ( event == mAppEvt_PairingLocalOobData_c )
    {
        (void)CCC_FirstApproachRsp(peerDeviceId, gaAppOwnDiscAddress, &maPeerInformation[peerDeviceId].oobData);
    }
    else if ( event == mAppEvt_PairingPeerOobDataRcv_c )
    {
        mCurrentPeerId = peerDeviceId;
        (void)Gap_LeScGetLocalOobData();
    }
    else if (event == mAppEvt_PairingReqRcv_c)
    {
        /* No action required */
    }
    else if ( event == mAppEvt_PairingPeerOobDataReq_c )
    {
        (void)Gap_LeScSetPeerOobData(peerDeviceId, &maPeerInformation[peerDeviceId].peerOobData);
    }
    else if ( event == mAppEvt_PairingComplete_c )
    {
        FLib_MemSet(&maPeerInformation[peerDeviceId].oobData, 0x00, sizeof(gapLeScOobData_t));
        FLib_MemSet(&maPeerInformation[peerDeviceId].peerOobData, 0x00, sizeof(gapLeScOobData_t));
        maPeerInformation[peerDeviceId].appState = mAppRunning_c;
    }
    else
    {
        /* For MISRA compliance */
    }
}

/*! *********************************************************************************
* \brief        Handler of the mAppIdle_c state for BleApp_StateMachineHandler.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    event               Event type.
************************************************************************************/
static void BleApp_StateMachineHandler_AppIdle(deviceId_t peerDeviceId, appEvent_t event)
{
    if (event == mAppEvt_PeerConnected_c)
    {
        if (maPeerInformation[peerDeviceId].gapRole == gGapPeripheral_c)
        {
            maPeerInformation[peerDeviceId].appState = mAppCCCWaitingForOwnerPairingRequest_c;
        }
        else
        {
 #if (defined(gAppScanNonCCC_d) && (gAppScanNonCCC_d == 1U))
 #if defined(gAppUseBonding_d) && (gAppUseBonding_d)
            if (maPeerInformation[peerDeviceId].isBonded == TRUE)
            {
                /* Moving to Running State and wait for Link encryption result */
                maPeerInformation[peerDeviceId].appState = mAppRunning_c;
            }
            else
#endif
            {
              /* Moving to Exchange MTU State */
              maPeerInformation[peerDeviceId].appState = mAppExchangeMtu_c;
              (void)GattClient_ExchangeMtu(peerDeviceId, gAttMaxMtu_c);
            }
#endif /* (defined(gAppScanNonCCC_d) && (gAppScanNonCCC_d == 1U)) */
        }
    }
}

/*! *********************************************************************************
* \brief        Handler of the mAppIdle_c state for BleApp_StateMachineHandler.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    event               Event type.
************************************************************************************/
static void BleApp_StateMachineHandler_AppCCCWaitingForOwnerPairingRequest
(
    deviceId_t peerDeviceId,
    appEvent_t event
)
{
    if (event == mAppEvt_OwnerPairingRequestReceived_c)
    {
        bleResult_t status;

        /* Move to CCC Phase 2 */
        /* Dummy data - replace with calls to get actual data */
        uint16_t payloadLen = gDummyPayloadLength_c;
        uint8_t payload[gDummyPayloadLength_c] = gDummyPayload_c;
        status = CCCPhase2_SendSPAKERequest(peerDeviceId, payload, payloadLen);
        
        if (status == gBleSuccess_c)
        {
            maPeerInformation[peerDeviceId].appState = mAppCCCPhase2WaitingForResponse_c;
        }
    }
    else if (event == mAppEvt_EncryptionChanged_c)
    {
        maPeerInformation[peerDeviceId].isLinkEncrypted = TRUE;
        maPeerInformation[peerDeviceId].appState = mAppRunning_c;
    }
    else
    {
        /* For MISRA compliance */
    }
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
			appEvent_t   appEvent;
            if (gCurrentAdvHandle == (uint8_t)gLegacyAdvSetHandle_c)
            {
                /* Inform the user interface handler that legacy advertising started */
                appEvent = mAppEvt_AdvertisingStartedLegacy_c;

                if (FALSE == mOwnerPairingMode)
                {
                    gCurrentAdvHandle = gExtendedAdvSetHandle_c;
                    gStopExtAdvSetAfterConnect = FALSE;
                    BleApp_Advertise();
                }

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
                /* Start advertising timer */
                (void)TM_InstallCallback((timer_handle_t)mAppTimerId, AdvertisingTimerCallback, NULL);
                (void)TM_Start((timer_handle_t)mAppTimerId, (uint8_t)kTimerModeSetSecondTimer | (uint8_t)kTimerModeLowPowerTimer, TmSecondsToMilliseconds(gAdvTime_c));
                Led1On();
#else
                /* UI */
                LedStopFlashingAllLeds();
                Led1Flashing();
#endif /* #if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode) */
            }
            else if (gCurrentAdvHandle == (uint8_t)gExtendedAdvSetHandle_c)
            {
                /* Inform the user interface handler that extended advertising started */
                appEvent = mAppEvt_AdvertisingStartedExtendedLR_c;
            }
            else
            {
                if (gStopExtAdvSetAfterConnect == TRUE)
                {
                    /* Connection established based on the legacy advertising set
                    before the extended advertising set was started. Advertising
                    should be stopped */
                    gStopExtAdvSetAfterConnect = FALSE;
                    (void)Gap_StopExtAdvertising(0xFF);
                }
                else
                {
                    /* Inform the user interface handler that advertising has stopped */
                    appEvent = mAppEvt_AdvertisingStopped_c;
                    
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
                    timer_status_t status = TM_Stop((timer_handle_t)mAppTimerId);
                    if (status != kStatus_TimerSuccess)
                    {
                        panic(0, (uint32_t)BleApp_AdvertisingCallback, 0, 0);
                    }
                    Led1Off();
#else
                    if (0U == BleApp_GetNoOfActiveConnections())
                    {
                        /* UI */
                        LedStopFlashingAllLeds();
                        Led1Flashing();
                        Led2Flashing();
                    }
#endif /* #if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode) */
                }
            }

            if(mpfBleUserInterfaceEventHandler != NULL)
            {
                 appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
                 if(pEventData != NULL)
                 {
                     pEventData->appEvent = appEvent;
                     if (gBleSuccess_c != App_PostCallbackMessage(mpfBleUserInterfaceEventHandler, pEventData))
                     {
                        (void)MEM_BufferFree(pEventData);
                     }
                 } 
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
* \brief        Configures GAP Advertise parameters. Advertise will start after
*               the parameters are set.
*
********************************************************************************** */
static void BleApp_Advertise(void)
{
    /* Set advertising parameters*/
    if (gCurrentAdvHandle == (uint8_t)gLegacyAdvSetHandle_c)
    {
        gAppAdvParams.pGapExtAdvParams = &gLegacyAdvParams;
        gAppAdvParams.handle = gLegacyAdvSetHandle_c;
    }
    else if (gCurrentAdvHandle == (uint8_t)gExtendedAdvSetHandle_c)
    {
        gAppAdvParams.pGapExtAdvParams = &gExtAdvParams;
        gAppAdvParams.handle = gExtendedAdvSetHandle_c;
    }
    else
    {
        /* For MISRA compliance */
    }
    
    
    (void)BluetoothLEHost_StartExtAdvertising(&gAppAdvParams, BleApp_AdvertisingCallback, BleApp_ConnectionCallback);
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
        case gEvtCharacteristicCccdWritten_c:
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

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
/*! *********************************************************************************
* \brief        Stops advertising when the application timeout has expired.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void AdvertisingTimerCallback(void* pParam)
{
    (void)Gap_StopAdvertising();
}
#endif

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
static void SleepTimeoutSequence(void)
{
    (void)PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);
}
#endif

/*! *********************************************************************************
* \brief        Callback for incoming PSM data.
*
* \param[in]    deviceId        The device ID of the connected peer that sent the data
* \param[in]    lePsm           Channel ID
* \param[in]    pPacket         Pointer to incoming data
* \param[in]    packetLength    Length of incoming data
********************************************************************************** */
static void BleApp_L2capPsmDataCallback (deviceId_t     deviceId,
                                         uint16_t       lePsm,
                                         uint8_t*       pPacket,
                                         uint16_t       packetLength)
{
    /* The vehicle shall trigger a disconnect 5 seconds after an unencrypted L2CAP connection establishment
     * if no First_Approach_RQ or Request_owner_pairing Command Complete SubEvent notification has been
     * received during the first approach and owner pairing, respectively.*/
    if(TM_IsTimerActive(mL2caTimerId) == 1U)
    {
        if (packetLength > (gMessageHeaderSize_c + gPayloadHeaderSize_c + gLengthFieldSize_c))
        {
            dkMessageType_t messageType = (dkMessageType_t)pPacket[0];
            rangingMsgId_t msgId = (rangingMsgId_t)pPacket[1];
            
            switch (messageType)
            {
                case gDKMessageTypeDKEventNotification_c:
                {
                    if ( msgId == gDkEventNotification_c )
                    {
                        if ( packetLength == (gMessageHeaderSize_c + gPayloadHeaderSize_c + gLengthFieldSize_c + gCommandCompleteSubEventPayloadLength_c) )
                        {
                            dkSubEventCategory_t category = (dkSubEventCategory_t)pPacket[gMessageHeaderSize_c + gPayloadHeaderSize_c + gLengthFieldSize_c];
                            if ( category == gCommandComplete_c)
                            {
                                dkSubEventCommandCompleteType_t type = (dkSubEventCommandCompleteType_t)pPacket[gMessageHeaderSize_c + gPayloadHeaderSize_c + gLengthFieldSize_c + sizeof(category)];
                                if ( type == gRequestOwnerPairing_c )
                                {
                                    (void)TM_Stop((timer_handle_t)mL2caTimerId);
                                }
                            }
                        }
                    }
                }
                break;
                
                case gDKMessageTypeSupplementaryServiceMessage_c:
                {
                    if ( msgId == gFirstApproachRQ_c )
                    {
                        if ( packetLength == (gMessageHeaderSize_c + gPayloadHeaderSize_c + gLengthFieldSize_c + gFirstApproachReqRspPayloadLength) )
                        {
                            (void)TM_Stop((timer_handle_t)mL2caTimerId);
                        }
                    }
                }
				break;
                
                default:
                    ; /* For MISRA compliance */
                    break;
            }
        }
    }
    
    
    /* Inform the application events handler that L2capPsmData package received */
    if(mpfBleEventHandler != NULL)
    {
        appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t) + sizeof(appEventL2capPsmData_t) + (uint32_t)packetLength);
        if(pEventData != NULL)
        {
            pEventData->appEvent = mAppEvt_L2capPsmDataCallback_c;
            pEventData->eventData.pData = &pEventData[1];
            appEventL2capPsmData_t *pL2capPsmDataEvent = pEventData->eventData.pData;
            pL2capPsmDataEvent->deviceId = deviceId;
            pL2capPsmDataEvent->lePsm = lePsm;
            pL2capPsmDataEvent->packetLength = packetLength;
            pL2capPsmDataEvent->pPacket = (uint8_t*)(&pL2capPsmDataEvent[1]);
            FLib_MemCpy(pL2capPsmDataEvent->pPacket, pPacket, packetLength);
            pL2capPsmDataEvent = NULL;
            if (gBleSuccess_c != App_PostCallbackMessage(mpfBleEventHandler, pEventData))
            {
               (void)MEM_BufferFree(pEventData);
            }
        }
    }
}

/*! *********************************************************************************
* \brief        Callback for control messages.
*
* \param[in]    pMessage    Pointer to control message
********************************************************************************** */
static void BleApp_L2capPsmControlCallback(l2capControlMessage_t* pMessage)
{
    switch (pMessage->messageType)
    {
#if defined(gHandoverDemo_d) && (gHandoverDemo_d == 1)
        case gL2ca_HandoverConnectionComplete_c:
        {
            /* Save L2CAP channel ID */
            maPeerInformation[pMessage->messageData.handoverConnectionComplete.deviceId].customInfo.psmChannelId =
                pMessage->messageData.handoverConnectionComplete.cId;
        }
        break;
#endif
        case gL2ca_LePsmConnectRequest_c:
        {
            deviceId_t deviceId = pMessage->messageData.connectionRequest.deviceId;

            /* For Passive Entry the link must be encrypted before opening the L2CAP CB channel */
            if ((maPeerInformation[deviceId].isBonded == TRUE) && (maPeerInformation[deviceId].isLinkEncrypted == FALSE))
            {
                (void)L2ca_CancelConnection(gDK_DefaultVehiclePsm_c, deviceId, gInsufficientEncryption_c);
            }
            else
            {
                (void)L2ca_ConnectLePsm(gDK_DefaultVehiclePsm_c, deviceId, mAppLeCbInitialCredits_c);
            }
        }
        break;
        
        case gL2ca_LePsmConnectionComplete_c:
        {
            if (pMessage->messageData.connectionComplete.result == gSuccessful_c)
            {
                /* Handle Conn Complete */
                maPeerInformation[pMessage->messageData.connectionComplete.deviceId].customInfo.psmChannelId = pMessage->messageData.connectionComplete.cId;
                if (maPeerInformation[pMessage->messageData.connectionComplete.deviceId].isLinkEncrypted == FALSE)
                {
                    if (mL2caTimerValid == TRUE)
                    {
                        mDeviceIdToDisconnect = pMessage->messageData.connectionComplete.deviceId;
                        (void)TM_InstallCallback((timer_handle_t)mL2caTimerId, L2caTimerCallback, &mDeviceIdToDisconnect);
                        (void)TM_Start((timer_handle_t)mL2caTimerId, (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSetSecondTimer, gCCCL2caTimeout_c);
                    }
                }
                BleApp_StateMachineHandler(maPeerInformation[pMessage->messageData.connectionComplete.deviceId].deviceId, mAppEvt_PsmChannelCreated_c);
            }
        }
        break;
        
        case gL2ca_LePsmDisconnectNotification_c:
        {
            
        }
        break;
        
        case gL2ca_NoPeerCredits_c:
        {
            (void)L2ca_SendLeCredit (pMessage->messageData.noPeerCredits.deviceId,
                                    pMessage->messageData.noPeerCredits.cId,
                                    mAppLeCbInitialCredits_c);
        }
        break;
        
        case gL2ca_Error_c:
        {
            /* Handle error */    
        }
        break;
        
        default:
        {
            ; /* For MISRA compliance */
            break;
        }
    }
}

#if (defined(gAppScanNonCCC_d) && (gAppScanNonCCC_d == 1U))
/*! *********************************************************************************
* \brief        Handles BLE Scanning callback from host stack.
*
* \param[in]    pScanningEvent    Pointer to gapScanningEvent_t.
********************************************************************************** */
static void BleApp_ScanningCallback (gapScanningEvent_t* pScanningEvent)
{
    switch (pScanningEvent->eventType)
    {
        case gExtDeviceScanned_c:
        {
            /* For now just connect to anyone. Non-CCC Key FOB data is not defined */
            /* Set connection parameters and stop scanning. Connect on gScanStateChanged_c. */
            gConnReqParams.peerAddressType = pScanningEvent->eventData.extScannedDevice.addressType;
            FLib_MemCpy(gConnReqParams.peerAddress,
                        pScanningEvent->eventData.extScannedDevice.aAddress,
                        sizeof(bleDeviceAddress_t));

            mFoundPeripheralToConnect = TRUE;
            (void)Gap_StopScanning();
#if defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d)
            gConnReqParams.usePeerIdentityAddress = pScanningEvent->eventData.extScannedDevice.advertisingAddressResolved;
#endif
        }
        break;

        case gScanStateChanged_c:
        {
            mScanningOn = !mScanningOn;

            /* Node starts scanning */
            if (mScanningOn)
            {
                /* Inform the user interface handler that the Bluetooth scan has started */
                if(mpfBleUserInterfaceEventHandler != NULL)
                {
                     appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
                     if(pEventData != NULL)
                     {
                         pEventData->appEvent = mAppEvt_BleScanning_c;
                         if (gBleSuccess_c != App_PostCallbackMessage(mpfBleUserInterfaceEventHandler, pEventData))
                         {
                            (void)MEM_BufferFree(pEventData);
                         }
                     } 
                }

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
                Led1On();
#else
                LedStopFlashingAllLeds();
                Led1Flashing();
#endif
            }
            /* Node is not scanning */
            else
            {
                /* Inform the user interface handler that the Bluetooth scan has stopped */
                if(mpfBleUserInterfaceEventHandler != NULL)
                {
                     appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
                     if(pEventData != NULL)
                     {
                         pEventData->appEvent = mAppEvt_BleScanStopped_c;
                         if (gBleSuccess_c != App_PostCallbackMessage(mpfBleUserInterfaceEventHandler, pEventData))
                         {
                            (void)MEM_BufferFree(pEventData);
                         }
                     } 
                }                

                if (TRUE == mFoundPeripheralToConnect)
                {
                    /* Inform the user interface handler that a device to connect to has been found */
                    if(mpfBleUserInterfaceEventHandler != NULL)
                    {
                         appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
                         if(pEventData != NULL)
                         {
                             pEventData->appEvent = mAppEvt_BleConnectingToDevice_c;
                             if (gBleSuccess_c != App_PostCallbackMessage(mpfBleUserInterfaceEventHandler, pEventData))
                             {
                                (void)MEM_BufferFree(pEventData);
                             }
                         } 
                    }
                    (void)BluetoothLEHost_Connect(&gConnReqParams, BleApp_ConnectionCallback);
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
            appEvent_t appEvt = mAppEvt_GattProcComplete_c;

            switch(procedureType)
            {

                case gGattProcReadCharacteristicValue_c:
                {
                    appEvt = mAppEvt_ReadCharacteristicValueComplete_c;
                }
                break;

                default:
                {
                    ; /* No action required */
                }
                break;
            }

            BleApp_StateMachineHandler(serverDeviceId, appEvt);
        }
    }

    /* Signal Service Discovery Module */
    BleServDisc_SignalGattClientEvent(serverDeviceId, procedureType, procedureResult, error);
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
#endif /* (defined(gAppScanNonCCC_d) && (gAppScanNonCCC_d == 1U)) */

/*! *********************************************************************************
 * \brief        Returns number of active connections.
 ********************************************************************************** */
static uint8_t BleApp_GetNoOfActiveConnections(void)
{
    uint8_t activeConnections = 0;
    uint8_t mPeerId = 0;

    for (mPeerId = 0; mPeerId < (uint8_t)gAppMaxConnections_c; mPeerId++)
    {
        if (maPeerInformation[mPeerId].deviceId != gInvalidDeviceId_c)
        {
            ++activeConnections;
        }
    }

    return activeConnections;
}

/*! *********************************************************************************
 * \brief        Timer Callback
 ********************************************************************************** */
static void L2caTimerCallback(void *param)
{
    deviceId_t deviceId = *(deviceId_t*)param;

    if (maPeerInformation[deviceId].deviceId != gInvalidDeviceId_c)
    {
        (void)Gap_Disconnect(maPeerInformation[deviceId].deviceId);
    }
}

/*! *********************************************************************************
 * \brief        Returns UWB clock. 
 ********************************************************************************** */
static uint64_t GetUwbClock(void)
{
    uint32_t randomNo;
    (void)RNG_GetTrueRandomNumber(&randomNo);
    /* Get a simulated UWB clock */
    return TM_GetTimestamp() + (uint8_t)randomNo;
}

#if defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U)
/*! *********************************************************************************
* \brief        Handler function for APP A2B communication interface.
*
********************************************************************************** */
static void BleApp_A2BCommHandler(uint8_t opGroup, uint8_t cmdId, uint16_t len, uint8_t *pData)
{
    A2A_SendCommand(opGroup, cmdId, pData, len);
}
#endif /* defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U) */
