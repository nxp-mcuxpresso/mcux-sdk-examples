/*! *********************************************************************************
* \addtogroup Digital Key Car Anchor Application
* @{
********************************************************************************** */
/*! *********************************************************************************
* \file app_digital_key_car_anchor.c
*
* Copyright 2021-2024 NXP
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
#include "fsl_component_led.h"
#include "fsl_component_timer_manager.h"
#include "fsl_os_abstraction.h"
#include "FunctionLib.h"
#include "fsl_component_mem_manager.h"
#include "fsl_adapter_reset.h"
#if defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
#include "fsl_shell.h"
#endif
#include "HWParameter.h"
#include "app.h"
#include "fwk_seclib.h"
/* BLE Host Stack */
#include "gatt_server_interface.h"

/* Connection Manager */
#include "ble_conn_manager.h"

#include "board.h"
#include "app_conn.h"
#include "digital_key_car_anchor.h"

#include "shell_digital_key_car_anchor.h"
#include "app_digital_key_car_anchor.h"

#include "gap_handover_types.h"
#include "gap_handover_interface.h"
#include "app_a2a_interface.h"
#include "app_handover.h"
#include "app_a2b.h"

#include "controller_api.h"

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define mcEncryptionKeySize_c   16
/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
#if defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
#if (defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U))
/* LTK size */
#define gAppLtkSize_c gcSmpMaxBlobSize_c
/* LTK */
static uint8_t gaAppSmpLtk[gcSmpMaxBlobSize_c];
/* IRK */
static uint8_t gaAppSmpIrk[gcSmpMaxBlobSize_c];
#else /* (defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U)) */
/* LTK */
static uint8_t gaAppSmpLtk[gcSmpMaxLtkSize_c];
/* LTK size */
#define gAppLtkSize_c mcEncryptionKeySize_c
/* IRK */
static uint8_t gaAppSmpIrk[gcSmpIrkSize_c];
#endif /* (defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U)) */
/* RAND*/
static uint8_t gaAppSmpRand[gcSmpMaxRandSize_c];
/* Address */
static uint8_t gaAppAddress[gcBleDeviceAddressSize_c];
static gapSmpKeys_t gAppOutKeys = {
    .cLtkSize = gAppLtkSize_c,
    .aLtk = (void *)gaAppSmpLtk,
    .aIrk = (void *)gaAppSmpIrk,
    .aCsrk = NULL,
    .aRand = (void *)gaAppSmpRand,
    .cRandSize = gcSmpMaxRandSize_c,
    .ediv = 0,
    .addressType = 0,
    .aAddress = gaAppAddress
};
static gapSmpKeyFlags_t gAppOutKeyFlags;
static bool_t gAppOutLeSc;
static bool_t gAppOutAuth;
#if (defined (gAppSecureMode_d) && (gAppSecureMode_d == 0U) || (defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U)))
/* 
Global used to identify if a bond was added by
shell command or connection with the device 
mBondAddedFromShell = FALSE bond created by connection
mBondAddedFromShell = TRUE  bond added by shell command
*/
static bool_t mBondAddedFromShell = FALSE;
#endif
#endif /* defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1) */

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/
#if defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
#if defined(gAppSecureMode_d) && (gAppSecureMode_d == 0U)
static bleResult_t SetBondingData
(
    uint8_t nvmIndex,
    bleAddressType_t addressType,
    uint8_t* ltk,
    uint8_t* irk,
    uint8_t* address
);
#endif /* defined(gAppSecureMode_d) && (gAppSecureMode_d == 0U) */
/* CCC Time Sync */
static bleResult_t CCC_TriggerTimeSync(deviceId_t deviceId);
static void AppPrintLePhyEvent(gapPhyEvent_t* pPhyEvent);
static void PrintLePhyEvent(gapPhyEvent_t* pPhyEvent);
static void App_HandleBondShellCmds(void *pData);
static void App_HandleL2capPsmDataCallback(appEventData_t *pEventData);
#if (defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U))
static void A2A_HandleGenericCallbackBondCreatedEvent(appEventData_t *pEventData);
#else
static void App_HandleGenericCallbackBondCreatedEvent(appEventData_t *pEventData);
#endif /* (defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U)) */
#endif /* defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1) */
static void AppSetBD_ADDR(void);

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
button_status_t BleApp_HandleKeys0(void *buttonHandle, button_callback_message_t *message, void *callbackParam);
#endif /*gAppButtonCnt_c > 0*/

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1))
button_status_t BleApp_HandleKeys1(void *buttonHandle, button_callback_message_t *message, void *callbackParam);
#endif /*gAppButtonCnt_c > 1*/

#if defined(gA2ASerialInterface_d) && (gA2ASerialInterface_d == 1)
static void A2A_ProcessCommand(void *pMsg);
#endif /* defined(gA2ASerialInterface_d) && (gA2ASerialInterface_d == 1) */

#if defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U)
#if defined(gA2BInitiator_d) && (gA2BInitiator_d == 0)
static void A2A_CheckLocalIrk(void);
#endif /* defined(gA2BInitiator_d) && (gA2BInitiator_d == 0) */
#endif /* defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U) */
#if defined(gHandoverDemo_d) && (gHandoverDemo_d == 1U)
static void BleApp_HandoverEventHandler(appHandoverEvent_t eventType, void *pData);
static void BleApp_HandoverCommHandler(uint8_t opGroup, uint8_t cmdId, uint16_t len, uint8_t *pData);
#endif /* defined(gHandoverDemo_d) && (gHandoverDemo_d == 1U) */
static void BleApp_OP_StartCaller(appCallbackParam_t param);
static void BleApp_PE_StartCaller(appCallbackParam_t param);
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
        maPeerInformation[mPeerId].isLinkEncrypted = FALSE;
        maPeerInformation[mPeerId].appState = mAppIdle_c;
        FLib_MemSet(&maPeerInformation[mPeerId].oobData, 0x00, sizeof(gapLeScOobData_t));
        FLib_MemSet(&maPeerInformation[mPeerId].peerOobData, 0x00, sizeof(gapLeScOobData_t));
    }
    
    LedStartFlashingAllLeds();
    
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[0], BleApp_HandleKeys0, NULL);
#endif /* (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0)) */
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[1], BleApp_HandleKeys1, NULL);
#endif /* (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1)) */

    /* Configures Bluetooth address on the anchor form APP_BD_ADDR  */
    AppSetBD_ADDR();
    
    /* Add/modify init code starting from here */
    
    /* Register the function handler for the user interface events  */
    BleApp_RegisterUserInterfaceEventHandler(APP_UserInterfaceEventHandler);
    
#if defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
    /* Register the function handler for the shell commands events  */
    AppShell_RegisterCmdHandler(App_HandleShellCmds);
#endif
    
    /* Register the function handler for the Bluetooth application events  */
    BleApp_RegisterEventHandler(APP_BleEventHandler);

#if defined(gHandoverDemo_d) && (gHandoverDemo_d == 1U)
    /* Initialize Handover. */
    (void)AppHandover_Init(BleApp_HandoverEventHandler, BleApp_ConnectionCallback, BleApp_HandoverCommHandler);
#endif

    /* Set coding scheme for passive entry */
    (void)Controller_ConfigureAdvCodingScheme(mLongRangeAdvCodingScheme_c, gExtendedAdvSetHandle_c);

    /* Set generic callback */
    BluetoothLEHost_SetGenericCallback(BleApp_GenericCallback);

    /* Initialize Bluetooth Host Stack */
    BluetoothLEHost_Init(BluetoothLEHost_Initialized);

#if defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U)
#if defined(gA2BInitiator_d) && (gA2BInitiator_d == 0)
    /* Check if local IRK is set. Otherwise postpone BLE Host initialization. */
    A2A_CheckLocalIrk();
#endif /* defined(gA2BInitiator_d) && (gA2BInitiator_d == 0) */
#endif /* defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U) */
    
    /* UI */
    shellPrompt.constPrompt = "Anchor>";
    AppShellInit(shellPrompt.prompt);

#if defined(gA2ASerialInterface_d) && (gA2ASerialInterface_d == 1)
    (void)A2A_Init(gSerMgrIf2, A2A_ProcessCommand);
#endif /* defined(gA2ASerialInterface_d) && (gA2ASerialInterface_d == 1) */
}

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    buttonHandle    button handle
* \param[in]    message         Button press event
* \param[in]    callbackParam   parameter
********************************************************************************** */
button_status_t BleApp_HandleKeys0(void *buttonHandle, button_callback_message_t *message,void *callbackParam)
{
    switch (message->event)
    {
        case kBUTTON_EventOneClick:
        case kBUTTON_EventShortPress:
        {
            (void)App_PostCallbackMessage(BleApp_OP_StartCaller, NULL);
        }
        break;

        /* Disconnect on long button press */
        case kBUTTON_EventLongPress:
        {
            uint8_t mPeerId = 0;
            for (mPeerId = 0; mPeerId < (uint8_t)gAppMaxConnections_c; mPeerId++)
            {
                if (maPeerInformation[mPeerId].deviceId != gInvalidDeviceId_c)
                {
                    (void)Gap_Disconnect(maPeerInformation[mPeerId].deviceId);
                }
            }
        }
        break;
        
        /* Very Long Press not available - use Double Click */
        case kBUTTON_EventDoubleClick:
        {
            /* Factory reset*/
            BleApp_FactoryReset();
        }
        break;
        
        default:
        {
            ; /* No action required */
            break;
        }
    }
    
    return kStatus_BUTTON_Success;
}
#endif /* (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0)) */

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1))
/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    buttonHandle    button handle
* \param[in]    message         Button press event
* \param[in]    callbackParam   parameter
********************************************************************************** */
button_status_t BleApp_HandleKeys1(void *buttonHandle, button_callback_message_t *message,void *callbackParam)
{
    switch (message->event)
    {
        case kBUTTON_EventOneClick:
        case kBUTTON_EventShortPress:
        {
#if defined(gHandoverDemo_d) && (gHandoverDemo_d == 1)
            bleResult_t status = gBleSuccess_c;
            deviceId_t handoverDeviceId = gInvalidDeviceId_c;
            shell_write("\r\nHandover started.\r\n");
            
            handoverDeviceId = BleApp_SelectDeviceIdForHandover();
            
            if (handoverDeviceId == gInvalidDeviceId_c)
            {
                shell_write("\r\n Handover device id error.\r\n");
                status = gBleInvalidState_c;
            }
            else
            {
                AppHandover_SetPeerDevice(handoverDeviceId);
                status = AppHandover_StartTimeSync(TRUE);
            }
            
            if (status != gBleSuccess_c)
            {
                shell_write("\r\nHandover time synchronization error.\r\n");
                shell_cmd_finished();
            }
#endif
        }
        break;
        case kBUTTON_EventLongPress:
        {
            (void)App_PostCallbackMessage(BleApp_PE_StartCaller, NULL);
        }
        break;
        default:
        {
            ; /* No action required */
            break;
        }
    }

    return kStatus_BUTTON_Success;
}
#endif /* (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1)) */

/*!*************************************************************************************************
* \fn     uint8_t APP_UserInterfaceEventHandler(uint8_t *pData)
* \brief  This function is used to handle User Interface events from BLE
*         To register another function use BleApp_RegisterUserInterfaceEventHandler
*
* \param  [in]   pData - pointer to appEventData_t data;
 ***************************************************************************************************/
void APP_UserInterfaceEventHandler(void *pData)
{
    /* Here custom code can be added to handle user interface update based on application events */
    appEventData_t *pEventData = (appEventData_t *)pData;
    switch(pEventData->appEvent)
    {
        case mAppEvt_PeerConnected_c:
        {
            shell_write("\r\nConnected!\r\n");
        }
        break;
        
        case mAppEvt_PsmChannelCreated_c:
        {
            shell_write("\r\nL2CAP PSM Connection Complete.\r\n");
        }
        break;
        
        case mAppEvt_PairingPeerOobDataRcv_c:
        {
            shell_write("\r\nReceived First_Approach_RQ.\r\n");
        }
        break;
          
        case mAppEvt_PeerDisconnected_c:
        {
            shell_write("Disconnected!\r\n");
            shell_cmd_finished();
        }
        break;
        
        case mAppEvt_PairingLocalOobData_c:
        {
            shell_write("\r\nSending First_Approach_RS\r\n");
        }
        break;
        
        case mAppEvt_PairingComplete_c:
        {
            shell_write("\r\nPairing successful.\r\n");
            shell_cmd_finished();
        }
        break;
        
        case mAppEvt_PairingReqRcv_c:
        {
            shell_write("\r\nPairing...\r\n");
        }
        break;
        
        case mAppEvt_SPAKERequestSent_c:
        {
            shell_write("\r\nSPAKE Request sent.\r\n");
        }
        break;
        
        case mAppEvt_SPAKEVerifySent_c:
        {
            shell_write("\r\nSPAKE Verify sent.\r\n");
        }
        break;
        
        case mAppEvt_AdvertisingStartedLegacy_c:
        {
            shell_write("Advertising started - Legacy.\r\n");
        }
        break;
        
        case mAppEvt_AdvertisingStartedExtendedLR_c:
        {
            shell_write("Advertising started - Extended LR.\r\n");
        }
        break;
        
        case mAppEvt_AdvertisingStopped_c:
        {
            shell_write("Advertising stopped - All PHYs.\r\n");
        }
        break;
        
        case mAppEvt_BleConfigDone_c:
        {
            shell_write("\r\nDigital Key Car Anchor.\r\n");
            shell_cmd_finished();
        }
        break;
        
        case mAppEvt_BleScanning_c:
        {
            shell_write("Scanning...\r\n");
        }
        break;
        
        case mAppEvt_BleScanStopped_c:
        {
            shell_write("Scan stopped.\r\n");
        }
        break;
        
        case mAppEvt_BleConnectingToDevice_c:
        {
            shell_write("Connecting...\r\n");
        }
        break;
        
        case mAppEvt_LePhyEvent_c:
        {
#if defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
            AppPrintLePhyEvent((gapPhyEvent_t *)pEventData->eventData.pData);
#endif
        }
        break;
#if defined(gHandoverDemo_d) && (gHandoverDemo_d == 1)
        case mAppEvt_Shell_HandoverError_c:
        {
            switch (pEventData->eventData.handoverError)
            {
                case mAppHandover_NoActiveConnection_c:
                {
                    shell_write("\r\nNo active connection to transfer.\r\n");
                }
                break;
                
                case mAppHandover_TimeSyncTx_c:
                {
                    shell_write("\r\nTime synchronization transmit error.\r\n");
                }
                break;
                
                case mAppHandover_AnchorSearchStartFailed_c:
                {
                    shell_write("\r\nAnchor search start failed.\r\n");
                }
                break;
                
                case mAppHandover_PeerBondingDataInvalid_c:
                {
                    shell_write("\r\nInvalid bonding data.\r\n");
                }
                break;

                case mAppHandover_AnchorSearchFailedToSync_c:
                {
                    shell_write("\r\nHandover failed - Anchor search unsuccessful.\r\n");
                }
                break;

                default:
                {
                    shell_write("\r\nHandover error.\r\n");
                    shell_cmd_finished();
                }
                break;
            }
            
            shell_cmd_finished();
        }
        break;
        
        case mAppEvt_Shell_HandoverCompleteConnected_c:
        {
            shell_write("\r\nHandover complete, connected.\r\n");
            shell_cmd_finished();
        }
        break;
        
        case mAppEvt_Shell_HandoverCompleteDisconnected_c:
        {
            shell_write("\r\nHandover complete, disconnected.\r\n");
            shell_cmd_finished();
        }
        break;
        
        case mAppEvt_Shell_HandoverStarted_c:
        {
            if (pEventData->eventData.handoverTimeSync == TRUE)
            {
                shell_write("\r\nHandover started.\r\n");
            }
            else
            {
                shell_write("\r\nAnchor Monitor started.\r\n");
            }
            shell_cmd_finished();
        }
        break;

        case mAppEvt_Shell_AnchorMonitorEventReceived_c:
        {
            shell_write("\r\nRSSI event received for device id: ");
            shell_writeDec(pEventData->eventData.anchorMonitorEvent.deviceId);
            shell_write("\r\n");
        }
        break;

        case mAppEvt_Shell_PacketMonitorEventReceived_c:
        {
            shell_write("\r\nPacket monitor event received for device id ");
            shell_writeDec(pEventData->eventData.anchorPacketEvent.deviceId);
            shell_write(", ");

            /* Status bit1: packet transmitter central (1) or peripheral (0) */
            if ((pEventData->eventData.anchorPacketEvent.pktMntEvt.statusPacket & BIT1) != 0U)
            {
                shell_write("from central");
            }
            else
            {
                shell_write("from peripheral");
            }
            
            shell_write(" with RSSI: ");
            if(((uint8_t)pEventData->eventData.anchorPacketEvent.pktMntEvt.rssiPacket >> 7) != 0U)
            {
                shell_write("-");
                uint8_t aux = ~((uint8_t)pEventData->eventData.anchorPacketEvent.pktMntEvt.rssiPacket - 1U);
                pEventData->eventData.anchorPacketEvent.pktMntEvt.rssiPacket = (int8_t)aux;
            }

            shell_writeDec(pEventData->eventData.anchorPacketEvent.pktMntEvt.rssiPacket);
            shell_write("\r\n");
            /* Free pdu memory */
            (void)MEM_BufferFree(pEventData->eventData.anchorPacketEvent.pktMntEvt.pPdu);
        }
        break;

        case mAppEvt_Shell_PacketMonitorContinueEventReceived_c:
        {
            shell_write("\r\nPacket continue monitor event received for device id ");
            shell_writeDec(pEventData->eventData.anchorPacketContinueEvent.deviceId);
            shell_write("\r\n");
            /* Free pdu memory */
            (void)MEM_BufferFree(pEventData->eventData.anchorPacketContinueEvent.pktMntCntEvt.pPdu);
        }
        break;
#endif /* defined(gHandoverDemo_d) && (gHandoverDemo_d == 1) */
#if defined(gA2BEnabled_d) && (gA2BEnabled_d == 1)
        case mAppEvt_Shell_A2BKeyDerivationComplete_c:
        {
            shell_write("\r\nE2E key derivation successful.\r\n");
            shell_cmd_finished();
        }
        break;
        
        case mAppEvt_Shell_A2BLocalIrkSyncComplete_c:
        {
            shell_write("\r\nE2E local IRK sync successful.\r\n");
            shell_cmd_finished();
        }
        break;

        case mAppEvt_Shell_A2BError_c:
        {
            switch (pEventData->eventData.a2bError)
            {
                case mAppA2B_E2EKeyDerivationFailiure_c:
                {
                    shell_write("\r\nE2E key derivation failed.\r\n");
                    shell_cmd_finished();
                }
                break;
                
                case mAppA2B_E2ELocalIrkSyncFailiure_c:
                {
                    shell_write("\r\nE2E local IRK sync failed.\r\n");
                    shell_cmd_finished();
                }
                break;
                
                default:
                {
                    shell_write("\r\nA2B error.\r\n");
                    shell_cmd_finished();
                }
                break;
            }
        }
        break;
#endif /* defined(gA2BEnabled_d) && (gA2BEnabled_d == 1) */
        default:
        {
            ; /* No action required */
        }
        break;
    }
    
    (void)MEM_BufferFree(pData);
    pData = NULL;
}

/*!*************************************************************************************************
 *\fn     uint8_t APP_BleEventHandler(uint8_t *pData)
 *\brief  This function is used to handle events from BLE
 *        To register another function use BleApp_RegisterEventHandler
 *
 *\param  [in]   pData - pointer to appEventData_t data;
 ***************************************************************************************************/
void APP_BleEventHandler(void *pData)
{
    appEventData_t *pEventData = (appEventData_t *)pData;
    
    switch(pEventData->appEvent)
    {
        case mAppEvt_GenericCallback_LeScLocalOobData_c:
        {
#if defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
            /* Code for CCC demo application */
            (void)App_HandleLeScLocalOobDataCallback(pEventData);
#else
            /* 
             * To custom handle OOB data, add your code here and set gAppUseShellInApplication_d to 0 in app_preinclude.h  
             * The shell will be disabled too. 
             * The OOB data is received in pEventData->eventData.pData as gapLeScOobData_t
             */
#endif
        }
        break;
          
        case mAppEvt_GenericCallback_BondCreatedEvent_c:
        {
#if defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
            
#if (defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U))
            A2A_HandleGenericCallbackBondCreatedEvent(pEventData);
#else
            App_HandleGenericCallbackBondCreatedEvent(pEventData);
#endif /* (defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U)) */
#else
            /* 
             * To custom handle bond created event, add your code here and set gAppUseShellInApplication_d to 0 in app_preinclude.h  
             * The shell will be disabled too.
             * The bond created event is received in pEventData->eventData.pData as bleBondCreatedEvent_t
             */
#endif
        }
        break;
        
        case mAppEvt_L2capPsmDataCallback_c:
        {
#if defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
            App_HandleL2capPsmDataCallback(pEventData);
#else
             /* 
             * To custom handle L2CAP data packet, add your code here and set gAppUseShellInApplication_d to 0 in app_preinclude.h  
             * The shell will be disabled too. 
             * The L2CAP data packet is received in pEventData->eventData.pData as appEventL2capPsmData_t
             */
#endif
        }
        break;
        
        case mAppEvt_FactoryReset_c:
        {
#if defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
            BleApp_FactoryReset();
#else
            /* 
             * To custom handle factory reset data packet, add your code here and set gAppUseShellInApplication_d to 0 in app_preinclude.h
             * The shell will be disabled too. 
             */
#endif
        }
        break;
      
        default:
        {
            ; /* No action required */
        }
        break;
    }

    (void)MEM_BufferFree(pData);
    pData = NULL;
}

#if defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
/*! *********************************************************************************
* \brief        Handles Shell Commands events.
*
* \param[in]    pEventData    pointer to appEventData_t.
********************************************************************************** */
void App_HandleShellCmds(void *pData)
{
    appEventData_t *pEventData = (appEventData_t *)pData;
    
    switch(pEventData->appEvent)
    {
        case mAppEvt_Shell_Reset_Command_c:
        {
            HAL_ResetMCU();
        }
        break;
        
        case mAppEvt_Shell_FactoryReset_Command_c:
        {
            /* Factory Reset for the CCC demo application*/
            BleApp_FactoryReset();
        }
        break;

        case mAppEvt_Shell_ShellStartDiscoveryOP_Command_c:
        {
            /* start owner pairing scenario */
            BleApp_OP_Start();
        }
        break;

        case mAppEvt_Shell_ShellStartDiscoveryPE_Command_c:
        {
            /* start passive entry scenario */
            BleApp_PE_Start();
        }
        break;

        case mAppEvt_Shell_StopDiscovery_Command_c:
        {
             /* stop discovery */
            BleApp_StopDiscovery();
        }
        break;

        case mAppEvt_Shell_Disconnect_Command_c:
        {
            BleApp_Disconnect();
        }
        break;

        case mAppEvt_Shell_TriggerTimeSync_Command_c:
        {
            (void)CCC_TriggerTimeSync(pEventData->eventData.peerDeviceId);
        }
        break;

        case mAppEvt_Shell_SetBondingData_Command_c:
        case mAppEvt_Shell_ListBondedDev_Command_c:
        case mAppEvt_Shell_RemoveBondedDev_Command_c:
        {
            App_HandleBondShellCmds(pData);
        }
        break;

#if defined(gHandoverDemo_d) && (gHandoverDemo_d == 1)
        case mAppEvt_Shell_HandoverSendL2cap_Command_c:
        {
            deviceId_t handoverDeviceId = gInvalidDeviceId_c;
            
            handoverDeviceId = BleApp_SelectDeviceIdForHandover();
            
            if (handoverDeviceId != gInvalidDeviceId_c)
            {
                char msg[] = L2CAP_SAMPLE_MESSAGE;
                (void)L2ca_SendLeCbData(handoverDeviceId, maPeerInformation[handoverDeviceId].customInfo.psmChannelId,
                                        (const uint8_t *)msg, (uint16_t)FLib_StrLen(L2CAP_SAMPLE_MESSAGE));
            }
        }
        break;

        case mAppEvt_Shell_HandoverStartAnchorMonitor_Command_c:
        {
            if (pEventData->eventData.monitorStart.deviceId != gInvalidDeviceId_c)
            {
                bleResult_t result = gBleSuccess_c;
                result = AppHandover_SetMonitorMode(pEventData->eventData.monitorStart.deviceId, pEventData->eventData.monitorStart.monitorMode);
                
                if (result == gBleSuccess_c)
                {
                    AppHandover_SetPeerDevice(pEventData->eventData.monitorStart.deviceId);
                    result = AppHandover_StartTimeSync(FALSE);
                }
                
                if (result != gBleSuccess_c)
                {
                    shell_write("\r\nAnchor monitor start failed");
                }
            }
        }
        break;

        case mAppEvt_Shell_HandoverStopAnchorMonitor_Command_c:
        {
            bleResult_t result = gBleInvalidParameter_c;
            
            result = AppHandover_AnchorMonitorStop(pEventData->eventData.peerDeviceId);
            
            if (result != gBleSuccess_c)
            {
                shell_write("\r\nAnchor monitor stop failed");
            }
        }
        break;
#endif
        
        default:
        {
            ; /* No action required */
        }
        break;
    }
    
    (void)MEM_BufferFree(pData);
    pData = NULL;
}
#endif

#if defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U)
/*! *********************************************************************************
* \brief        Handler function for APP Handover events.
*
********************************************************************************** */
void BleApp_A2BEventHandler(appA2BEvent_t eventType, void *pData)
{
    
    switch (eventType)
    {
        case mAppA2B_E2EKeyDerivationComplete_c:
        {
            appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
            
            if(pEventData != NULL)
            {
                pEventData->appEvent = mAppEvt_Shell_A2BKeyDerivationComplete_c;
                pEventData->eventData.pData = NULL;
                if (gBleSuccess_c != App_PostCallbackMessage(APP_UserInterfaceEventHandler, pEventData))
                {
                    (void)MEM_BufferFree(pEventData);
                }
            }
        }
        break;
        
        case mAppA2B_E2ELocalIrkSyncComplete_c:
        {
            appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
            
            if(pEventData != NULL)
            {
                pEventData->appEvent = mAppEvt_Shell_A2BLocalIrkSyncComplete_c;
                pEventData->eventData.pData = NULL;
                if (gBleSuccess_c != App_PostCallbackMessage(APP_UserInterfaceEventHandler, pEventData))
                {
                    (void)MEM_BufferFree(pEventData);
                }
            }
        }
        break;
        
        case mAppA2B_Error_c:
        {
            appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
            
            if(pEventData != NULL)
            {
                appA2BError_t error = *(appA2BError_t *)pData;
                pEventData->appEvent = mAppEvt_Shell_A2BError_c;
                pEventData->eventData.a2bError = error;
                if (gBleSuccess_c != App_PostCallbackMessage(APP_UserInterfaceEventHandler, pEventData))
                {
                    (void)MEM_BufferFree(pEventData);
                }
            }
        }
        break;
        
        default:
        ; /* For MISRA compliance */
        break;
    }
}
#endif /* defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U) */

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

#if defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
/*! *********************************************************************************
* \brief        Handles mAppEvt_Shell_SetBondingData_Command_c,
*               mAppEvt_Shell_ListBondedDev_Command_c,
*               mAppEvt_Shell_RemoveBondedDev_Command_c events.
*
* \param[in]    pEventData    pointer to appEventData_t.
********************************************************************************** */
static void App_HandleBondShellCmds(void *pData)
{
    appEventData_t *pEventData = (appEventData_t *)pData;
    
    switch(pEventData->appEvent)
    {
        case mAppEvt_Shell_SetBondingData_Command_c:
        {
#if defined (gAppSecureMode_d) && (gAppSecureMode_d == 1U)
            shell_write("\r\nsetbd command not available in Secure Mode.\r\n");
            shell_cmd_finished();
#else
            /* Set address type, LTK, IRK and pear device address  */
            appBondingData_t *pAppBondingData = (appBondingData_t *)pEventData->eventData.pData;
            
            bleResult_t status = gBleSuccess_c;
            status = SetBondingData(pAppBondingData->nvmIndex, pAppBondingData->addrType, pAppBondingData->aLtk,
                                    pAppBondingData->aIrk, pAppBondingData->deviceAddr);
            if ( status != gBleSuccess_c )
            {
                shell_write("\r\nsetbd failed with status: ");
                shell_writeHex((uint8_t*)&status, 2);
                shell_write("\r\n");
                shell_cmd_finished();
            }
            else
            {
#if defined(gAppUseBonding_d) && (gAppUseBonding_d == 1)
                (void)Gap_AddDeviceToFilterAcceptList(pAppBondingData->addrType, pAppBondingData->deviceAddr);
#endif
                mBondAddedFromShell = TRUE;
            }
#endif /* defined (gAppSecureMode_d) && (gAppSecureMode_d == 0U) */
        }
        break;
        
        case mAppEvt_Shell_ListBondedDev_Command_c:
        {
#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U))
            gapIdentityInformation_t aIdentity[gMaxBondedDevices_c];
            uint8_t nrBondedDevices = 0;
            uint8_t foundBondedDevices = 0;
            bleResult_t result = Gap_GetBondedDevicesIdentityInformation(aIdentity, gMaxBondedDevices_c, &nrBondedDevices);
            if (gBleSuccess_c == result && nrBondedDevices > 0U)
            {
                for (int8_t i = 0; i < gMaxBondedDevices_c; i++)
                {
                    result = Gap_LoadKeys((uint8_t)i, &gAppOutKeys, &gAppOutKeyFlags, &gAppOutLeSc, &gAppOutAuth);
                    if (gBleSuccess_c == result && nrBondedDevices > 0U)
                    {
                        /* address type, address, ltk, irk */
                        shell_write("\r\nNVMIndex: ");
                        shell_writeHex((uint8_t*)&i, 1);
                        shell_write(" ");
                        shell_write(" BondingData: ");
                        shell_writeHex((uint8_t*)&gAppOutKeys.addressType, 1);
                        shell_write(" ");
                        shell_writeHex((uint8_t*)gAppOutKeys.aAddress, 6);
                        shell_write(" ");
                        shell_writeHex((uint8_t*)gAppOutKeys.aLtk, 16);
                        shell_write(" ");
                        shell_writeHex((uint8_t*)gAppOutKeys.aIrk, 16);
                        foundBondedDevices++;
                    }

                    if(foundBondedDevices == nrBondedDevices)
                    {
                        shell_write("\r\n");
                        shell_cmd_finished();
                        break;
                    }
                }
            }
#endif /* gAppUseBonding_d */
        }
        break;
        
        case mAppEvt_Shell_RemoveBondedDev_Command_c:
        {
#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U))
            bleResult_t result = gGapSuccess_c;
            result = Gap_LoadKeys(pEventData->eventData.peerDeviceId, &gAppOutKeys, &gAppOutKeyFlags, &gAppOutLeSc, &gAppOutAuth);
            if(result == gBleSuccess_c)
            {
                if(gBleSuccess_c == Gap_RemoveDeviceFromFilterAcceptList(gAppOutKeys.addressType, gAppOutKeys.aAddress))
                {
                    /* Remove bond based on nvm index stored in eventData.peerDeviceId */
                    if ((Gap_RemoveBond(pEventData->eventData.peerDeviceId) == gBleSuccess_c))
                    {
                        gcBondedDevices--;
                        gPrivacyStateChangedByUser = TRUE;
                        (void)BleConnManager_DisablePrivacy();
                        shell_write("\r\nBond removed!\r\n");
                        shell_cmd_finished();
                    }
                    else
                    {
                        shell_write("\r\nOperation failed!\r\n");
                        shell_cmd_finished();
                    }
                }
                else
                {
                     shell_write("\r\nOperation failed!\r\n");
                     shell_cmd_finished();
                }
            }
            else
            {
                shell_write("\r\nRemoved bond failed because unable to load the keys from the bond.\r\n");
                shell_cmd_finished();
            }
#endif /* gAppUseBonding_d */
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
* \brief        Prints phy event.
*
********************************************************************************** */
static void AppPrintLePhyEvent(gapPhyEvent_t* pPhyEvent)
{
    PrintLePhyEvent(pPhyEvent);
}

/*! *********************************************************************************
* \brief        Prints phy event.
*
********************************************************************************** */
static void PrintLePhyEvent(gapPhyEvent_t* pPhyEvent)
{
    /* String dictionary corresponding to gapLePhyMode_t */
    static const char* mLePhyModeStrings[] =
    {
        "Invalid\r\n",
        "1M\r\n",
        "2M\r\n",
        "Coded\r\n",
    };    
    uint8_t txPhy = ((gapLePhyMode_tag)(pPhyEvent->txPhy) <= gLePhyCoded_c) ? pPhyEvent->txPhy : 0U;
    uint8_t rxPhy = ((gapLePhyMode_tag)(pPhyEvent->rxPhy) <= gLePhyCoded_c) ? pPhyEvent->rxPhy : 0U;
    shell_write("Phy Update Complete.\r\n");
    shell_write("TxPhy ");
    shell_write(mLePhyModeStrings[txPhy]);
    shell_write("RxPhy ");
    shell_write(mLePhyModeStrings[rxPhy]);
    shell_cmd_finished();
}

/*! *********************************************************************************
* \brief        Handles L2capPsmDataCallback events.
*
* \param[in]    pEventData    pointer to appEventData_t.
********************************************************************************** */
static void App_HandleL2capPsmDataCallback(appEventData_t *pEventData)
{
   
    appEventL2capPsmData_t *l2capDataEvent = (appEventL2capPsmData_t *)pEventData->eventData.pData;
    deviceId_t deviceId = l2capDataEvent->deviceId;
    uint16_t packetLength = l2capDataEvent->packetLength;
    uint8_t* pPacket = l2capDataEvent->pPacket;
    
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
                                shell_write("\r\nReceived Command Complete SubEvent: Request_owner_pairing.\r\n");
                                BleApp_StateMachineHandler(deviceId, mAppEvt_OwnerPairingRequestReceived_c);
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
                        uint8_t *pData = &pPacket[gMessageHeaderSize_c + gPayloadHeaderSize_c + gLengthFieldSize_c];
                        
                        /* BD address not used. */
                        pData = &pData[gcBleDeviceAddressSize_c];
                        /* Confirm Value */
                        FLib_MemCpy(&maPeerInformation[deviceId].peerOobData.confirmValue, pData, gSmpLeScRandomConfirmValueSize_c);
                        pData = &pData[gSmpLeScRandomConfirmValueSize_c];
                        /* Random Value */
                        FLib_MemCpy(&maPeerInformation[deviceId].peerOobData.randomValue, pData, gSmpLeScRandomValueSize_c);
                        
                        /* Send event to the application state machine. */
                        BleApp_StateMachineHandler(deviceId, mAppEvt_PairingPeerOobDataRcv_c);
                    }
                    else
                    {
                        shell_write("\r\nERROR: Invalid length for FirstApproachRQ.\r\n");
                    }
                }
                else if (msgId == gTimeSync_c)
                {
                    shell_write("\r\nTime Sync received.\r\n");
                    shell_cmd_finished();
                    /* Handle Time Sync data */
                    (void)mTsUwbDeviceTime;
                }
                else
                {
                    /* For MISRA compliance */
                }
            }
            break;
            
            case gDKMessageTypeFrameworkMessage_c:
            {
                switch (maPeerInformation[deviceId].appState)
                {
                    case mAppCCCPhase2WaitingForResponse_c:
                    {
                         /* Process the data in pPacket here */
                         shell_write("\r\nSPAKE Response received.\r\n");
                         BleApp_StateMachineHandler(deviceId, mAppEvt_ReceivedSPAKEResponse_c);
                    }
                    break;

                    case mAppCCCPhase2WaitingForVerify_c:
                    {
                        /* Process the data in pPacket here */
                        shell_write("\r\nSPAKE Verify received.\r\n");
                        shell_write("\r\nSending Command Complete SubEvent: BLE_pairing_ready\r\n");
                        (void)CCC_SendSubEvent(deviceId, gCommandComplete_c, gBlePairingReady_c);
                        BleApp_StateMachineHandler(deviceId, mAppEvt_BlePairingReady_c);
                    }
                    break;

                    default:
                    {
                        ; /* For MISRA compliance */
                    }
                    break;
                }
            }
            break;
            
            default:
                ; /* For MISRA compliance */
                break;
        }
    }
}
#if (defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U))
/*! *********************************************************************************
* \brief        Handles GenericCallback events.
*
* \param[in]    pEventData    pointer to appEventData_t.
********************************************************************************** */
static void A2A_HandleGenericCallbackBondCreatedEvent(appEventData_t *pEventData)
{
    bleBondCreatedEvent_t *pBondEventData = (bleBondCreatedEvent_t *)pEventData->eventData.pData;
    bleResult_t status = gBleSuccess_c;
#if (defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U))
    secResultType_t secStatus = gSecSuccess_c;
#endif /* (defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U)) */
    
    status = Gap_LoadKeys(pBondEventData->nvmIndex,
                          &gAppOutKeys, &gAppOutKeyFlags, &gAppOutLeSc,
                          &gAppOutAuth);
    
    if ( status == gBleSuccess_c)
    {
        /* address type, address, ltk, irk */
        shell_write("\r\nBondingData: ");
        shell_writeHex((uint8_t*)&gAppOutKeys.addressType, 1);
        shell_write(" ");
        shell_writeHex(gAppOutKeys.aAddress, gcBleDeviceAddressSize_c);
        shell_write(" ");
        shell_writeHex((uint8_t*)gAppOutKeys.aLtk, gcSmpMaxBlobSize_c);
        shell_write(" ");
        shell_writeHex((uint8_t*)gAppOutKeys.aIrk, gcSmpIrkSize_c);
        shell_write("\r\n");
#if (defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U))
        if (mBondAddedFromShell == FALSE)
        {
            /* Bonding data containing E2E blobs received from the other Anchor. */
            /* get LTK E2E blob */
            uint8_t aTempKeyData[gcSmpMaxBlobSize_c];
            FLib_MemCpy(aTempKeyData, gAppOutKeys.aLtk, gcSmpMaxBlobSize_c);
            if (gSecLibFunctions.pfSecLib_ExportA2BBlob != NULL)
            {
                secStatus = gSecLibFunctions.pfSecLib_ExportA2BBlob(aTempKeyData, gSecLtkElkeBlob_c, gAppOutKeys.aLtk);
            }
            else
            {
                secStatus = gSecError_c;
            }
            
            if (gSecSuccess_c == secStatus)
            {
                /* get IRK E2E blob */
                FLib_MemCpy(aTempKeyData, gAppOutKeys.aIrk, gcSmpIrkSize_c);
                if (gSecLibFunctions.pfSecLib_ExportA2BBlob != NULL)
                {
                    secStatus = gSecLibFunctions.pfSecLib_ExportA2BBlob(aTempKeyData, gSecPlainText_c, gAppOutKeys.aIrk);
                }
            }
        }
#endif /* (defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U)) */
    }

#if (defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U))
    if (gSecSuccess_c == secStatus)
    {
        if (mBondAddedFromShell == TRUE)
        {
            shell_cmd_finished();
            mBondAddedFromShell = FALSE;
            gPrivacyStateChangedByUser = TRUE;
            (void)BleConnManager_DisablePrivacy();
        }
        else
        {
            uint8_t buf[gHandoverSetBdCommandLen_c] = {0};
            buf[0] = pBondEventData->nvmIndex;
            buf[1] = gAppOutKeys.addressType;
            FLib_MemCpy(&buf[2], gAppOutKeys.aAddress, gcBleDeviceAddressSize_c);
            FLib_MemCpy(&buf[8], gAppOutKeys.aLtk, gcSmpMaxBlobSize_c);
            FLib_MemCpy(&buf[48], gAppOutKeys.aIrk, gcSmpMaxIrkBlobSize_c);
#if defined(gHandoverDemo_d) && (gHandoverDemo_d > 0U)
            (void)AppHandover_GetPeerSkd(pBondEventData->nvmIndex, &buf[88]);
#endif /* defined(gHandoverDemo_d) && (gHandoverDemo_d > 0U) */
            A2A_SendSetBondingDataCommand(buf, gHandoverSetBdCommandLen_c);
        }
    }
    else
    {
        shell_write("\r\nE2E BondingData load failed ");
    }
#endif /* (defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U)) */
}
#else
/*! *********************************************************************************
* \brief        Handles GenericCallback events.
*
* \param[in]    pEventData    pointer to appEventData_t.
********************************************************************************** */
static void App_HandleGenericCallbackBondCreatedEvent(appEventData_t *pEventData)
{
    bleBondCreatedEvent_t *pBondEventData = (bleBondCreatedEvent_t *)pEventData->eventData.pData;
    bleResult_t status;
    
    status = Gap_LoadKeys(pBondEventData->nvmIndex,
                          &gAppOutKeys, &gAppOutKeyFlags, &gAppOutLeSc,
                          &gAppOutAuth);
    
    if ( status == gBleSuccess_c)
    {
        /* address type, address, ltk, irk */
        shell_write("\r\nBondingData: ");
        shell_writeHex((uint8_t*)&gAppOutKeys.addressType, 1);
        shell_write(" ");
        shell_writeHex(gAppOutKeys.aAddress, 6);
        shell_write(" ");
        shell_writeHex((uint8_t*)gAppOutKeys.aLtk, 16);
        shell_write(" ");
        shell_writeHex((uint8_t*)gAppOutKeys.aIrk, 16);
        shell_write("\r\n");
    }

    if (mBondAddedFromShell == TRUE)
    {
        shell_cmd_finished();
        mBondAddedFromShell = FALSE;
        gPrivacyStateChangedByUser = TRUE;
        (void)BleConnManager_DisablePrivacy();
    }
    else
    {
#if defined(gHandoverDemo_d) && (gHandoverDemo_d == 1U)
        uint8_t buf[gHandoverSetBdCommandLen_c] = {0};
        buf[0] = pBondEventData->nvmIndex;
        buf[1] = gAppOutKeys.addressType;
        FLib_MemCpy(&buf[2], gAppOutKeys.aAddress, gcBleDeviceAddressSize_c);
        FLib_MemCpy(&buf[8], gAppOutKeys.aLtk, gcSmpMaxLtkSize_c);
        FLib_MemCpy(&buf[24], gAppOutKeys.aIrk, gcSmpIrkSize_c);
        A2A_SendSetBondingDataCommand(buf, gHandoverSetBdCommandLen_c);
#endif
    }
}
#endif /* (defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U)) */

#if defined(gAppSecureMode_d) && (gAppSecureMode_d == 0U)
/*! *********************************************************************************
* \brief    Set Bonding Data on the BLE application.
*
********************************************************************************** */
static bleResult_t SetBondingData(uint8_t nvmIndex, bleAddressType_t addressType,
                                  uint8_t* ltk, uint8_t* irk, uint8_t* address)
{
    bleResult_t status;
    
    gAppOutKeys.addressType = addressType;
    FLib_MemCpy(gAppOutKeys.aAddress, address, gcBleDeviceAddressSize_c);
    FLib_MemCpy(gAppOutKeys.aLtk, ltk, gcSmpMaxLtkSize_c);
    FLib_MemCpy(gAppOutKeys.aIrk, irk, gcSmpIrkSize_c);

    status = Gap_SaveKeys(nvmIndex, &gAppOutKeys, TRUE, FALSE);
    
    return status;
}
#endif /* defined(gAppSecureMode_d) && (gAppSecureMode_d == 0U) */

/*! *********************************************************************************
 * \brief        Trigger TimeSync from device - send LE Data Length Changed message
 *
 ********************************************************************************** */
static bleResult_t CCC_TriggerTimeSync(deviceId_t deviceId)
{
    bleResult_t result = gBleSuccess_c;

    if (deviceId < (uint8_t)gAppMaxConnections_c)
    {
        result = Gap_LeSetPhy(FALSE, deviceId, 0, gConnDefaultTxPhySettings_c, gConnDefaultRxPhySettings_c, 0);
    }
    else
    {
        result = gBleInvalidParameter_c;
    }

    return result;
}
#endif /* defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1) */

/*! *********************************************************************************
* \brief        Configures Bluetooth Address
*
********************************************************************************** */
static void AppSetBD_ADDR(void)
{
    hardwareParameters_t *pHWParams = NULL;
    uint8_t appBdAddr[gcBleDeviceAddressSize_c] = APP_BD_ADDR;

    (void)NV_ReadHWParameters(&pHWParams);
    
    if ( !FLib_MemCmp(appBdAddr, pHWParams->bluetooth_address, gcBleDeviceAddressSize_c) )
    {
        OSA_DisableIRQGlobal();
        FLib_MemCpy(pHWParams->bluetooth_address, appBdAddr, gcBleDeviceAddressSize_c);
        OSA_EnableIRQGlobal();
        (void)NV_WriteHWParameters();
    }
}

#if defined(gA2ASerialInterface_d) && (gA2ASerialInterface_d == 1)
/*! *********************************************************************************
* \brief        Processes received Handover commands.
*
********************************************************************************** */
static void A2A_ProcessCommand(void *pMsg)
{
    clientPacketStructured_t* pPacket = (clientPacketStructured_t*)pMsg;
    switch (pPacket->header.opGroup)
    {
        case gHandoverCommandsOpGroup_c:
        {
#if defined(gHandoverDemo_d) && (gHandoverDemo_d > 0U)
            AppHandover_ProcessA2ACommand(pPacket->header.opCode,
                                          pPacket->header.len,
                                          pPacket->payload);
#endif /* defined(gHandoverDemo_d) && (gHandoverDemo_d > 0U) */
        }
        break;

        case gA2ACommandsOpGroup_c:
        {
            switch (pPacket->header.opCode)
            {
                case gSetBdCommandOpCode_c:
                {
 #if defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U)
                    uint8_t *pIndex = pPacket->payload;
                    uint8_t nvmIndex = gInvalidNvmIndex_c;
                    secResultType_t secStatus = gSecError_c;
                    bleResult_t status = gBleUnexpectedError_c;
                    
                    nvmIndex = *pIndex;
                    pIndex++;
                    gAppOutKeys.addressType = *pIndex;
                    pIndex++;
                    FLib_MemCpy(gAppOutKeys.aAddress, pIndex, gcBleDeviceAddressSize_c);
                    pIndex = &pIndex[gcBleDeviceAddressSize_c];
                    
                    if (gSecLibFunctions.pfSecLib_ImportA2BBlob != NULL)
                    {
                        secStatus = gSecLibFunctions.pfSecLib_ImportA2BBlob(pIndex, gSecLtkElkeBlob_c, gAppOutKeys.aLtk);
                    }

                    if (gSecSuccess_c == secStatus)
                    {
                        pIndex = &pIndex[gcSmpMaxIrkBlobSize_c];
                        if (gSecLibFunctions.pfSecLib_ImportA2BBlob != NULL)
                        {
                            secStatus = gSecLibFunctions.pfSecLib_ImportA2BBlob(pIndex, gSecPlainText_c, gAppOutKeys.aIrk);
                        }
                    }
                    
                    if (gSecSuccess_c == secStatus)
                    {
                        pIndex = &pIndex[gcSmpMaxIrkBlobSize_c];
                        mBondAddedFromShell = TRUE;
                        
                        status = Gap_SaveKeys(nvmIndex, &gAppOutKeys, TRUE, FALSE);
                    }
#if defined(gHandoverDemo_d) && (gHandoverDemo_d > 0U)
                    (void)AppHandover_SetPeerSkd(nvmIndex, pIndex);
                    
                    shell_write("\r\nReceived peer SKD: ");
                    shell_writeHex(pIndex, gSkdSize_c);
                    shell_write("\r\n");
#endif /* defined(gHandoverDemo_d) && (gHandoverDemo_d > 0U) */
                    /* If Handover is not enabled the EdgeLock to EdgeLock key is no longer needed. */
                    if (gBleSuccess_c == status)
                    {
                        (void)A2B_FreeE2EKey();
                    }
                    if ((gSecSuccess_c != secStatus) || (gBleSuccess_c != status))
                    {
                        shell_write("\r\nA2A SetBD fail.\r\n");
                    }
#else
                    uint8_t *pIndex = pPacket->payload;
                    uint8_t nvmIndex = gInvalidNvmIndex_c;
                    
                    nvmIndex = *pIndex;
                    pIndex++;
                    gAppOutKeys.addressType = *pIndex;
                    pIndex++;
                    FLib_MemCpy(gAppOutKeys.aAddress, pIndex, gcBleDeviceAddressSize_c);
                    pIndex = &pIndex[gcBleDeviceAddressSize_c];
                    FLib_MemCpy(gAppOutKeys.aLtk, pIndex, gcSmpMaxLtkSize_c);
                    pIndex = &pIndex[gcSmpMaxLtkSize_c];
                    FLib_MemCpy(gAppOutKeys.aIrk, pIndex, gcSmpIrkSize_c);
                    /* Save to first index in handover demo */
                    mBondAddedFromShell = TRUE;
                    (void)Gap_SaveKeys(nvmIndex, &gAppOutKeys, TRUE, FALSE);
#endif /* defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U) */
                }
                break;
                
                default:
                {
                    /* */
                }
                break;
            }
        }
        break;

        case gA2BCommandsOpGroup_c:
        {
#if defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U)
            A2B_ProcessA2ACommand(pPacket->header.opCode,
                                  pPacket->header.len,
                                  pPacket->payload);
#endif /* defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U) */
        }
        break;

        default:
        {
            /* */
        }
        break;   
    }

    MEM_BufferFree(pMsg);
    return;
}
#endif /* defined(gA2ASerialInterface_d) && (gA2ASerialInterface_d == 1) */

#if defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U)
#if defined(gA2BInitiator_d) && (gA2BInitiator_d == 0)
/*! *********************************************************************************
* \brief        Check if the local IRK is set.
*
********************************************************************************** */
static void A2A_CheckLocalIrk(void)
{
    bleResult_t status = gBleSuccess_c;
    bleLocalKeysBlob_t localIrk = {0};
    
    status = App_NvmReadLocalIRK(&localIrk);
    
    if ((gBleSuccess_c == status) && (localIrk.keyGenerated == TRUE))
    {
        gA2ALocalIrkSet = TRUE;
    }
    else
    {
        gA2ALocalIrkSet = FALSE;
    }
}
#endif /* defined(gA2BInitiator_d) && (gA2BInitiator_d == 0) */
#endif /* defined(gA2BEnabled_d) && (gA2BEnabled_d > 0U) */

#if defined(gHandoverDemo_d) && (gHandoverDemo_d == 1U)
/*! *********************************************************************************
* \brief        Handler function for APP Handover events.
*
********************************************************************************** */
static void BleApp_HandoverEventHandler(appHandoverEvent_t eventType, void *pData)
{
    switch (eventType)
    {
        case mAppHandover_ConnectComplete_c:
        {
            deviceId_t peerDeviceId = *(deviceId_t *)pData;
            LedStopFlashingAllLeds();
            Led1On();
            appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
            
            if(pEventData != NULL)
            {
                pEventData->appEvent = mAppEvt_Shell_HandoverCompleteConnected_c;
                pEventData->eventData.pData = NULL;
                if (gBleSuccess_c != App_PostCallbackMessage(APP_UserInterfaceEventHandler, pEventData))
                {
                    (void)MEM_BufferFree(pEventData);
                }
            }
            
            /* Save peer device ID */
            maPeerInformation[peerDeviceId].deviceId = peerDeviceId;
        }
        break;
        
        case mAppHandover_Disconnected_c:
        {
            deviceId_t peerDeviceId = *(deviceId_t *)pData;
            uint8_t peerId = 0U;
            
            appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
            
            if(pEventData != NULL)
            {
                pEventData->appEvent = mAppEvt_Shell_HandoverCompleteDisconnected_c;
                pEventData->eventData.pData = NULL;
                if (gBleSuccess_c != App_PostCallbackMessage(APP_UserInterfaceEventHandler, pEventData))
                {
                    (void)MEM_BufferFree(pEventData);
                }
            }
            
            /* Mark device ID as invalid */
            maPeerInformation[peerDeviceId].deviceId = gInvalidDeviceId_c;
            /* UI */
            
            /* Check to see if there are other devices connected. */
            for (peerId = 0; peerId < (uint8_t)gAppMaxConnections_c; peerId++)
            {
                if (maPeerInformation[peerId].deviceId != gInvalidDeviceId_c)
                {
                    break;
                }
            }
            
            /* Update the UI if no other devices are connected. */
            if (peerId == (uint8_t)gAppMaxConnections_c)
            {
                LedStartFlashingAllLeds();
            }
        }
        break;
        
        case mAppHandover_TimeSyncStarted_c:
        {
            appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
            
            if(pEventData != NULL)
            {
                pEventData->appEvent = mAppEvt_Shell_HandoverStarted_c;
                pEventData->eventData.handoverTimeSync = *(bool_t *)pData;
                if (gBleSuccess_c != App_PostCallbackMessage(APP_UserInterfaceEventHandler, pEventData))
                {
                    (void)MEM_BufferFree(pEventData);
                }
            }
        }
        break;

        case mAppHandoverAnchorMonitor_c:
        {
            appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));

            if(pEventData != NULL)
            {
                pEventData->appEvent = mAppEvt_Shell_AnchorMonitorEventReceived_c;
                FLib_MemCpy(&pEventData->eventData.anchorMonitorEvent, pData, sizeof(appHandoverAnchorMonitorEvent_t));
                if (gBleSuccess_c != App_PostCallbackMessage(APP_UserInterfaceEventHandler, pEventData))
                {
                    (void)MEM_BufferFree(pEventData);
                }
            }
        }
        break;

        case mAppHandoverPacketMonitor_c:
        {
            appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
            appHandoverAnchorMonitorPacketEvent_t *pAppPacketMonitorEvent = pData;

            if(pEventData != NULL)
            {
                pEventData->appEvent = mAppEvt_Shell_PacketMonitorEventReceived_c;
                FLib_MemCpy(&pEventData->eventData.anchorMonitorEvent, pData, sizeof(appHandoverAnchorMonitorPacketEvent_t));
                if (gBleSuccess_c != App_PostCallbackMessage(APP_UserInterfaceEventHandler, pEventData))
                {
                    (void)MEM_BufferFree(pAppPacketMonitorEvent->pktMntEvt.pPdu);
                    (void)MEM_BufferFree(pEventData);
                }
            }
            else
            {
                (void)MEM_BufferFree(pAppPacketMonitorEvent->pktMntEvt.pPdu);
            }
        }
        break;

        case mAppHandoverPacketContinueMonitor_c:
        {
            appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
            appHandoverAnchorMonitorPacketContinueEvent_t *pAppPacketContinueMonitorEvent = pData;

            if(pEventData != NULL)
            {
                pEventData->appEvent = mAppEvt_Shell_PacketMonitorContinueEventReceived_c;
                FLib_MemCpy(&pEventData->eventData.anchorPacketContinueEvent, pData, sizeof(appHandoverAnchorMonitorPacketContinueEvent_t));
                if (gBleSuccess_c != App_PostCallbackMessage(APP_UserInterfaceEventHandler, pEventData))
                {
                    (void)MEM_BufferFree(pAppPacketContinueMonitorEvent->pktMntCntEvt.pPdu);
                    (void)MEM_BufferFree(pEventData);
                }
            }
            else
            {
                (void)MEM_BufferFree(pAppPacketContinueMonitorEvent->pktMntCntEvt.pPdu);
            }
        }
        break;

        case mAppHandover_Error_c:
        {
            appHandoverError_t error = *(appHandoverError_t *)pData;
            appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
            
            if(pEventData != NULL)
            {
                pEventData->appEvent = mAppEvt_Shell_HandoverError_c;
                pEventData->eventData.handoverError = error;
                if (gBleSuccess_c != App_PostCallbackMessage(APP_UserInterfaceEventHandler, pEventData))
                {
                    (void)MEM_BufferFree(pEventData);
                }
            }
        }
        break;
            
        default:
        ; /* For MISRA compliance */
        break;
    }
}

/*! *********************************************************************************
* \brief        Handler function for APP Handover communication interface.
*
********************************************************************************** */
static void BleApp_HandoverCommHandler(uint8_t opGroup, uint8_t cmdId, uint16_t len, uint8_t *pData)
{
    A2A_SendCommand(opGroup, cmdId, pData, len);
}
#endif /* defined(gHandoverDemo_d) && (gHandoverDemo_d == 1U) */

/*! *********************************************************************************
* \brief        Calls BleApp_OP_Start on application task
*
********************************************************************************** */
static void BleApp_OP_StartCaller(appCallbackParam_t param)
{
    (void)param;
    BleApp_OP_Start();
}

/*! *********************************************************************************
* \brief        Calls BleApp_PE_Start on application task
*
********************************************************************************** */
static void BleApp_PE_StartCaller(appCallbackParam_t param)
{
    (void)param;
    BleApp_PE_Start();
}
/*! *********************************************************************************
* @}
********************************************************************************** */
