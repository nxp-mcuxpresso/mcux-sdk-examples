/*! *********************************************************************************
* \addtogroup BLE OTAP Server
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright 2015 Freescale Semiconductor, Inc.
* Copyright 2016-2024 NXP
*
*
* \file
*
* This file is the source file for the BLE OTAP Server application.
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
#include "fsl_component_panic.h"
#include "FunctionLib.h"
#include "fsl_component_mem_manager.h"
#include "fsl_adapter_reset.h"
#include "fsl_format.h"
#include "fsl_debug_console.h"
#include "app.h"
#include "board.h"

/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"
#include "gatt_db_handles.h"

/* Profile / Services */
#include "otap_interface.h"

#include "ble_conn_manager.h"
#include "ble_service_discovery.h"

#include "app_conn.h"
#include "FsciInterface.h"
#include "FsciCommunication.h"
#include "fsci_ble_otap.h"
#include "otap_server.h"

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

#define mAppLeCbInitialCredits_c    (32768)

/* appEvent_t */
#define mAppEvt_PeerConnected_c                 0x00U
#define mAppEvt_PairingComplete_c               0x01U
#define mAppEvt_GattProcComplete_c              0x02U
#define mAppEvt_GattProcError_c                 0x03U
#define mAppEvt_FsciBleOtapCmdReceived_c        0x04U
#define mAppEvt_CbConnectionComplete_c          0x05U
#define mAppEvt_CbConnectionFailed_c            0x06U
#define mAppEvt_CbDisconnected_c                0x07U


/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
typedef uint8_t appEvent_t;

typedef enum appState_tag{
    mAppIdle_c,
    mAppExchangeMtu_c,
    mAppPrimaryServiceDisc_c,
    mAppCharServiceDisc_c,
    mAppDescriptorSetup_c,
    mAppCbConnecting_c,
    mAppRunning_c,
}appState_t;

typedef struct appCustomInfo_tag
{
    otapServerConfig_t   otapServerConfig;
    /* Add persistent information here */
}appCustomInfo_t;

typedef struct appPeerInfo_tag
{
    deviceId_t      deviceId;
    appCustomInfo_t customInfo;
    bool_t          isBonded;
    appState_t      appState;
}appPeerInfo_t;

/*! Structure type holding basic information about the images available on an OTAP Server. */
typedef struct imgInfo_tag
{
    uint8_t         imgId[gOtap_ImageIdFieldSize_c];        /*!< Image id. */
    uint8_t         imgVer[gOtap_ImageVersionFieldSize_c];  /*!< Image version. */
    uint32_t        imgSize;                                /*!< Image size. */
} imgInfo_t;

/*! Structure containing the OTAP Server functional data. */
typedef struct otapServerAppData_tag
{
    imgInfo_t                       images[1];                      /*!< Array of images available on this OTAP Server. Only 1 image is supported at this time. */
    const otapServerStorageMode_t   storageMode;                    /*!< Storage mode used by the OTAP Server. Depends on the storage support available on the platform. */
    otapTransferMethod_t            transferMethod;                 /*!< OTAP Image File transfer method requested by the OTAP Client. */
    uint16_t                        l2capChannelOrPsm;              /*!< L2CAP Channel or PSM used for the transfer of the image file: channel 0x0004 for ATT, application specific PSM for CoC. */
    bool_t                          sentInitialImgNotification;     /*!< Boolean flag which is set if an Image Notification is sent to the OTAP Client on the first connection. */
    otapCmdIdt_t                    lastCmdSentToOtapClient;        /*!< The last command sent to the OTAP Client for which a Write Response is expected. */
    void*                           pLastFsciCmdId;                 /*!< Pointer to the Id of the last OTAP command received from the FSCI. */
    void*                           pLastFsciCmdPayload;            /*!< Pointer to the payload of the last OTAP command received from the FSCI. */
    uint32_t                        lastFsciCmdPayloadLen;          /*!< Length of the last OTAP command received from the FSCI. */
    uint16_t                        negotiatedMaxAttChunkSize;      /*!< The negotiated maximum ATT chunk size based on the negotiated ATT MTU between the OTAP Server and the OTAP Client. */
    uint16_t                        negotiatedMaxL2CapChunkSize;    /*!< The negotiated maximum L2CAP chunk size based on the negotiated L2CAP MTU between the OTAP Server and the OTAP Client. */
    bool_t                          l2capPsmConnected;              /*!< Flag which is set to true if an L2CAP PSM connection is currently established with a peer device. */
    uint16_t                        l2capPsmChannelId;              /*!< Channel Id for an L2CAP PSM connection currently established with a peer device. */
} otapServerAppData_t;

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/*! OTAP Protocol Command Id to Command Length table.
 *  The length includes the Command Id and the Command Payload. */
static const uint8_t cmdIdToCmdLengthTable[] =
{
    0,
    sizeof(otapCmdNewImgNotification_t) + gOtap_CmdIdFieldSize_c,
    sizeof(otapCmdNewImgInfoReq_t) + gOtap_CmdIdFieldSize_c,
    sizeof(otapCmdNewImgInfoRes_t) + gOtap_CmdIdFieldSize_c,
    sizeof(otapCmdImgBlockReq_t) + gOtap_CmdIdFieldSize_c,
    sizeof(otapCmdImgChunkAtt_t) + gOtap_CmdIdFieldSize_c, /*!< For ATT transfer method only, maximum length. */
    sizeof(otapCmdImgTransferComplete_t) + gOtap_CmdIdFieldSize_c,
    sizeof(otapErrNotification_t) + gOtap_CmdIdFieldSize_c,
    sizeof(otapCmdStopImgTransfer_t) + gOtap_CmdIdFieldSize_c,
};

static appPeerInfo_t mPeerInformation;

/* Scanning variables */
static bool_t   mScanningOn = FALSE;
static bool_t   mFoundDeviceToConnect = FALSE;
static TIMER_MANAGER_HANDLE_DEFINE(mAppTimerId);

static appScanningParams_t mAppScanParams = {
    &gScanParams,
    gGapDuplicateFilteringEnable_c,
    gGapScanContinuously_d,
    gGapScanPeriodicDisabled_d
};

/* Buffer used for Service Discovery */
static gattService_t *mpServiceDiscoveryBuffer = NULL;
static uint8_t  mcPrimaryServices = 0;

/* Buffer used for Characteristic Discovery */
static gattCharacteristic_t *mpCharDiscoveryBuffer = NULL;
static uint8_t mCurrentServiceInDiscoveryIndex;
static uint8_t mCurrentCharInDiscoveryIndex;

/* Buffer used for Characteristic Descriptor Discovery */
static gattAttribute_t *mpCharDescriptorBuffer = NULL;

/* Buffer used for Characteristic related procedures */
static gattAttribute_t      *mpDescProcBuffer = NULL;

/* Application Data */
/*! OTAP Server data structure.
 *  Contains functional information, available images information and state information
 *  regarding the image download procedure to the OTAp Client. */
static otapServerAppData_t      otapServerData =
{
    .images =       {
                        {
                            .imgId      = {0xFF, 0xFF}, /* No Image */
                            .imgVer     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* No image version initialized */
                            .imgSize    = 0,
                        },
                    },
    .storageMode    = gOtapServerStoargeNone_c,
    .transferMethod = gOtapTransferMethodAtt_c, /* The default transfer method is ATT */
    .l2capChannelOrPsm = gL2capCidAtt_c,  /* The default L2CAP channel is the ATT Channel */
    .sentInitialImgNotification = TRUE, /* Disable the transmission of a New Image Notification immediately after the connection is established */
    .lastCmdSentToOtapClient = gOtapCmdIdNoCommand_c,
    .pLastFsciCmdId = NULL,
    .pLastFsciCmdPayload = NULL,
    .lastFsciCmdPayloadLen = 0,
    .negotiatedMaxAttChunkSize = gAttDefaultMtu_c - gOtap_AttCommandMtuDataChunkOverhead_c,
    .negotiatedMaxL2CapChunkSize = gOtap_ImageChunkDataSizeL2capCoc_c,
    .l2capPsmConnected = FALSE,
    .l2capPsmChannelId = 0,
};

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

/* Host Stack callbacks */
static void BleApp_ScanningCallback
(
    gapScanningEvent_t* pScanningEvent
);

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

static void BleApp_HandleValueWriteConfirmations
(
    deviceId_t  deviceId
);

static void BleApp_GattIndicationCallback
(
    deviceId_t  serverDeviceId,
    uint16_t    characteristicValueHandle,
    uint8_t*    aValue,
    uint16_t    valueLength
);

static void BleApp_AttributeIndicated
(
    deviceId_t  deviceId,
    uint16_t    handle,
    uint8_t*    pValue,
    uint16_t    length
);

/* L2CAP LE PSM callbacks */
static void BleApp_L2capPsmDataCallback
(
    deviceId_t  deviceId,
    uint16_t    lePsm,
    uint8_t*    pPacket,
    uint16_t    packetLength
);

static void BleApp_L2capPsmControlCallback
(
    l2capControlMessage_t* pMessage
);

static void BluetoothLEHost_Initialized(void);

void BleApp_StateMachineHandler
(
    deviceId_t peerDeviceId,
    uint8_t event
);

void BleApp_StateMachineHandler_ServiceDiscovery
(
    deviceId_t peerDeviceId,
    uint8_t event
);

void BleApp_StateMachineHandler_AppRunning
(
    deviceId_t peerDeviceId,
    uint8_t event
);

static bool_t CheckScanEvent(gapScannedDevice_t* pData);

static void BleApp_HandleAttMtuChange
(
    deviceId_t peerDeviceId
);

static void BleApp_StoreServiceHandles
(
    gattService_t   *pService
);

static void BleApp_StoreCharHandles
(
    gattCharacteristic_t   *pChar
);

static void BleApp_StoreDescValues
(
    gattAttribute_t     *pDesc
);

static void BleApp_ServiceDiscoveryErrorHandler(void);

static void ScanningTimeoutTimerCallback(void* pParam);

/* OTAP Server functions */
static void OtapServer_SendCommandToOtapClient (deviceId_t  otapClientDevId,
                                                void*       pCommand,
                                                uint16_t    cmdLength);

static void OtapServer_SendCImgChunkToOtapClient (deviceId_t  otapClientDevId,
                                                  void*       pChunk,
                                                  uint16_t    chunkCmdLength);
/* Connection and Disconnection events */
static void OtapServer_HandleDisconnectionEvent (deviceId_t deviceId);

static void BluetoothLEHost_Initialized(void);

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
static button_status_t BleApp_HandleKeys0(void *pButtonHandle, button_callback_message_t *pMessage, void *pCallbackParam);
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
void BluetoothLEHost_AppInit(void)
{
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[0], BleApp_HandleKeys0, NULL);
#endif

    /* Init FSCI */
    FSCI_commInit( g_fsciHandleList );

    /* Register the BLE OTAP Opcode group with a FSCI interface */
    FsciBleOtap_Register (0);

    /* Set generic callback */
    BluetoothLEHost_SetGenericCallback(BleApp_GenericCallback);

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
static button_status_t BleApp_HandleKeys0(void *pButtonHandle, button_callback_message_t *pMessage,void *pCallbackParam)
{
    switch (pMessage->event)
    {
        case kBUTTON_EventOneClick:
        case kBUTTON_EventShortPress:
            for (uint8_t i = 0; i < gAppLedCnt_c; i++)
            {
                LedOff(i);
            }
            Led1Flashing();

            BleApp_Start();
            break;
        case kBUTTON_EventLongPress:
            (void)Gap_Disconnect(mPeerInformation.deviceId);
            break;
        default:
            ; /* For MISRA compliance */
             break;
    }

    return kStatus_BUTTON_Success;
}
#endif

/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
void BleApp_GenericCallback (gapGenericEvent_t* pGenericEvent)
{
    /* Call BLE Conn Manager */
    BleConnManager_GenericEvent(pGenericEvent);
}


/*! *********************************************************************************
* \brief        Handles a FSCI BLE OTAP command.
*
* \param[in]    pPacket         Pointer to the packet received via the FSCI interface.
* \param[in]    fsciInterface   Interface from which the packet was received.
*
********************************************************************************** */
void BleApp_HandleFsciBleOtapCommand (clientPacket_t*   pData,
                                      uint32_t          fsciInterface)
{
    union
    {
        uint8_t*                commandTemp;
        otapCommand_t*          otapCommandTemp;
    }otapCommandVars;

    /*! Initialize the OTAP Cmd pointer to the location of the payload minus the
     *  Command ID field size because the FSCI command payload only contains the
     *  OTAP command payload and not the OTAP Command opcode which is stored
     *  in the FSCI BLE OTAP packet header.
     *  The OTAP CmdId accessed through this pointer is not valid!!! */
    otapCommandVars.commandTemp = (uint8_t*)(pData->structured.payload) - gOtap_CmdIdFieldSize_c;
    otapCommand_t*  pOtapCmd = otapCommandVars.otapCommandTemp;
    bool_t          sendCmdToStateMachine = TRUE;

    switch (pData->structured.header.opCode)
    {
    case gOtapCmdIdNewImageNotification_c:
    {
        uint16_t cmdImgId;

        /* Check command parameters including length and send a FSCI Error
         * and stop immediately any of them is invalid */
        if (pData->structured.header.len != ((uint16_t)cmdIdToCmdLengthTable[gOtapCmdIdNewImageNotification_c] - gOtap_CmdIdFieldSize_c))
        {
            /* If the length is not valid send a FSCI Error. */
            FSCI_Error ((uint8_t)gFsciError_c, fsciInterface);
            sendCmdToStateMachine = FALSE;
            break;
        }
        /* Save image data in the local app buffer if a valid image id is received.
         * This message can be received via the serial interface even if an OTAP Client
         * was not found yet if the app requests image information via the serial interface. */
        FLib_MemCpy (&cmdImgId,
                     pOtapCmd->cmd.newImgInfoRes.imageId,
                     sizeof(cmdImgId));
        if ((cmdImgId != gBleOtaImageIdCurrentRunningImage_c) &&
            (cmdImgId != gBleOtaImageIdNoImageAvailable_c))
        {
            FLib_MemCpy (otapServerData.images[0].imgId,
                         pOtapCmd->cmd.newImgInfoRes.imageId,
                         sizeof(otapServerData.images[0].imgId));
            FLib_MemCpy (otapServerData.images[0].imgVer,
                         pOtapCmd->cmd.newImgInfoRes.imageVersion,
                         sizeof(otapServerData.images[0].imgVer));
            FLib_MemCpy ((uint8_t*)(&otapServerData.images[0].imgSize),
                         (uint8_t*)(&pOtapCmd->cmd.newImgInfoRes.imageFileSize),
                         sizeof(otapServerData.images[0].imgSize));
        }
        /* This command will be sent to the application state machine. */
        break;
    }
    case gOtapCmdIdNewImageInfoRequest_c:
        /* This should never be sent by an OTAP Server - Ignore */
        sendCmdToStateMachine = FALSE;
        break;
    case gOtapCmdIdNewImageInfoResponse_c:
    {
        uint16_t cmdImgId;

        if (pData->structured.header.len != ((uint16_t)cmdIdToCmdLengthTable[gOtapCmdIdNewImageInfoResponse_c] - gOtap_CmdIdFieldSize_c))
        {
            /* If the length is not valid send a FSCI Error. */
            FSCI_Error ((uint8_t)gFsciError_c, fsciInterface);
            sendCmdToStateMachine = FALSE;
            break;
        }
        /* Save image data in the local app buffer if a valid image id is received.
         * This message can be received via the serial interface even if an OTAP Client
         * was not found yet if the app requests image information via the serial interface. */
        FLib_MemCpy (&cmdImgId,
                     pOtapCmd->cmd.newImgNotif.imageId,
                     sizeof(cmdImgId));
        if ((cmdImgId != gBleOtaImageIdCurrentRunningImage_c) &&
            (cmdImgId != gBleOtaImageIdNoImageAvailable_c))
        {
            FLib_MemCpy (otapServerData.images[0].imgId,
                         pOtapCmd->cmd.newImgNotif.imageId,
                         sizeof(otapServerData.images[0].imgId));
            FLib_MemCpy (otapServerData.images[0].imgVer,
                         pOtapCmd->cmd.newImgNotif.imageVersion,
                         sizeof(otapServerData.images[0].imgVer));
            FLib_MemCpy ((uint8_t*)(&otapServerData.images[0].imgSize),
                         (uint8_t*)(&pOtapCmd->cmd.newImgNotif.imageFileSize),
                         sizeof(otapServerData.images[0].imgSize));
        }
        /* This command will be sent to the application state machine. */
        break;
    }
    case gOtapCmdIdImageBlockRequest_c:
        /* This should never be sent by an OTAP Server - Ignore */
        sendCmdToStateMachine = FALSE;
        break;
    case gOtapCmdIdImageChunk_c:
        /* No checks are needed here for this type of command. Any channel and length types will be handle din the state machine. */
        /* This command will be sent to the application state machine. */
        break;
    case gOtapCmdIdImageTransferComplete_c:
        /* This should never be sent by an OTAP Server - Ignore */
        sendCmdToStateMachine = FALSE;
        break;
    case gOtapCmdIdErrorNotification_c:
        /* This command will be sent to the application state machine. */
        if (pData->structured.header.len != ((uint16_t)cmdIdToCmdLengthTable[gOtapCmdIdErrorNotification_c] - gOtap_CmdIdFieldSize_c))
        {
            /* If the length is not valid send a FSCI Error. */
            FSCI_Error ((uint8_t)gFsciError_c, fsciInterface);
            sendCmdToStateMachine = FALSE;
            break;
        }
        break;
    case gOtapCmdIdStopImageTransfer_c:
        /* User stopped OTA transfer */
        sendCmdToStateMachine = TRUE;
        break;
    default:
        /* If the Opcode is not recognized send a FSCI Error. */
        FSCI_Error ((uint8_t)gFsciUnknownOpcode_c, fsciInterface);
        sendCmdToStateMachine = FALSE;
        break;
    }

    if (TRUE == sendCmdToStateMachine)
    {
        /* Notify the application state machine that a command has been received.
         * Depending on it's state it may do something or ignore it. */
        otapServerData.pLastFsciCmdId = &(pData->structured.header.opCode);
        otapServerData.pLastFsciCmdPayload = pData->structured.payload;
        otapServerData.lastFsciCmdPayloadLen = pData->structured.header.len;
        BleApp_StateMachineHandler (mPeerInformation.deviceId, mAppEvt_FsciBleOtapCmdReceived_c);
    }
}


/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief        Configures BLE Stack after initialization. Usually used for
*               configuring advertising, scanning, filter accept list, services, et al.
*
********************************************************************************** */
static void BluetoothLEHost_Initialized(void)
{
    /* Common GAP configuration */
    BleConnManager_GapCommonConfig();

    /* Register OTAP L2CAP PSM */
    (void)L2ca_RegisterLePsm (gOtap_L2capLePsm_c,
                        gOtapCmdImageChunkCocMaxLength_c);  /*!< The negotiated MTU must be higher than the biggest data chunk that will be sent fragmented */

    /* Register stack callbacks */
    (void)App_RegisterGattClientProcedureCallback (BleApp_GattClientCallback);
    (void)App_RegisterLeCbCallbacks(BleApp_L2capPsmDataCallback, BleApp_L2capPsmControlCallback);
    (void)App_RegisterGattClientIndicationCallback (BleApp_GattIndicationCallback);

    /* Initialize private variables */
    mPeerInformation.appState = mAppIdle_c;
    mScanningOn = FALSE;
    mFoundDeviceToConnect = FALSE;

    /* Allocate scan timeout timer */
    (void)TM_Open(mAppTimerId);

    /* UI */
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
            if( FALSE == mFoundDeviceToConnect )
            {
                mFoundDeviceToConnect = CheckScanEvent(&pScanningEvent->eventData.scannedDevice);
                if (mFoundDeviceToConnect)
                {
                    gConnReqParams.peerAddressType = pScanningEvent->eventData.scannedDevice.addressType;
                    FLib_MemCpy(gConnReqParams.peerAddress,
                                pScanningEvent->eventData.scannedDevice.aAddress,
                                sizeof(bleDeviceAddress_t));

                    (void)Gap_StopScanning();
#if gAppUsePrivacy_d
                    gConnReqParams.usePeerIdentityAddress = pScanningEvent->eventData.scannedDevice.advertisingAddressResolved;
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
                /* Start advertising timer */
                (void)TM_InstallCallback((timer_handle_t)mAppTimerId, ScanningTimeoutTimerCallback, NULL);
                (void)TM_Start((timer_handle_t)mAppTimerId, kTimerModeLowPowerTimer | kTimerModeSetSecondTimer, gScanningTime_c);

                for (uint8_t i = 0; i < gAppLedCnt_c; i++)
                {
                   LedOff(i);
                }
                Led1Flashing();

            }
            /* Node is not scanning */
            else
            {
                (void)TM_Stop((timer_handle_t)mAppTimerId);

                if (mFoundDeviceToConnect)
                {
                    (void)BluetoothLEHost_Connect(&gConnReqParams, BleApp_ConnectionCallback);
                }
                else
                {
                    LedStartFlashingAllLeds();
                }
            }
        }
        break;

        case gScanCommandFailed_c:
        {
            panic(0,0,0,0);
            break;
        }
    default:
        ; /* For MISRA compliance */
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
            Led1On();

            mPeerInformation.deviceId = peerDeviceId;
            mPeerInformation.isBonded = FALSE;

#if gAppUseBonding_d
            (void)Gap_CheckIfBonded(peerDeviceId, &mPeerInformation.isBonded, NULL);

            if ((mPeerInformation.isBonded) &&
                (gBleSuccess_c == Gap_LoadCustomPeerInformation(peerDeviceId,
                    (void*) &mPeerInformation.customInfo, 0, (uint16_t)(sizeof (appCustomInfo_t)))))
            {
                /* Restored custom connection information. Encrypt link */
                (void)Gap_EncryptLink(peerDeviceId);
            }
#endif
            BleApp_StateMachineHandler(mPeerInformation.deviceId, mAppEvt_PeerConnected_c);
        }
        break;

        case gConnEvtDisconnected_c:
        {
            mPeerInformation.deviceId = gInvalidDeviceId_c;
            mPeerInformation.customInfo.otapServerConfig.hControlPoint = gGattDbInvalidHandle_d;
            mPeerInformation.customInfo.otapServerConfig.hControlPointCccd = gGattDbInvalidHandle_d;
            mPeerInformation.customInfo.otapServerConfig.hData = gGattDbInvalidHandle_d;

            /* Reset Service Discovery to be sure*/
            BleApp_ServiceDiscoveryErrorHandler();

            /* Notify application */
            OtapServer_HandleDisconnectionEvent (peerDeviceId);

            /* UI */
            for (uint8_t i = 0; i < gAppLedCnt_c; i++)
            {
                LedOff(i);
            }
            Led1Flashing();

            /* Restart application */
            BleApp_Start();
        }
        break;


#if gAppUsePairing_d
        case gConnEvtPairingComplete_c:
        {
            if (pConnectionEvent->eventData.pairingCompleteEvent.pairingSuccessful)
            {
                BleApp_StateMachineHandler(peerDeviceId, mAppEvt_PairingComplete_c);
            }
        }
        break;

        case gConnEvtAuthenticationRejected_c:
        {
            /* Start Pairing Procedure */
            (void)Gap_Pair (peerDeviceId, &gPairingParameters);
        }
        break;

        case gConnEvtEncryptionChanged_c:
        {
              if( pConnectionEvent->eventData.encryptionChangedEvent.newEncryptionState )
              {
                    /* After reset the exchanged MTU is lost */
                    (void)GattClient_ExchangeMtu(peerDeviceId, gAttMaxMtu_c);
                    mPeerInformation.appState = mAppRunning_c;
              }

        }
        break;

#endif /* gAppUsePairing_d */

    default:
        ; /* For MISRA compliance */
    break;
    }
}

/*! *********************************************************************************
* \brief        Handles ATT MTU change.
*
* \param[in]    peerDeviceId    Peer device ID.
********************************************************************************** */
static void BleApp_HandleAttMtuChange
(
    deviceId_t peerDeviceId
)
{
    uint16_t negotiatedAttMtu = 0U;

    /* Get the new negotiated ATT MTU and compute the maximum ATT data chunk length an store
     * it in the application data structure. */
    (void)Gatt_GetMtu (peerDeviceId, &negotiatedAttMtu);
    otapServerData.negotiatedMaxAttChunkSize = negotiatedAttMtu - gOtap_AttCommandMtuDataChunkOverhead_c;
}

/*! *********************************************************************************
* \brief        Stores required GATT service handles for OTAP.
*
* \param[in]    pService    Service.
********************************************************************************** */
static void BleApp_StoreServiceHandles
(
    gattService_t   *pService
)
{
    uint8_t i;

    if ((pService->uuidType == gBleUuidType128_c) &&
        FLib_MemCmp(pService->uuid.uuid128, uuid_service_otap, 16))
    {
        /* Found OTAP Service */
        mPeerInformation.customInfo.otapServerConfig.hService = pService->startHandle;
        for (i = 0; i < pService->cNumCharacteristics; i++)
        {
            if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType128_c) &&
                (TRUE == FLib_MemCmp (pService->aCharacteristics[i].value.uuid.uuid128, uuid_char_otap_control_point, sizeof(pService->aCharacteristics[i].value.uuid.uuid128))))
            {
                /* Found OTAP Control Point Char */
                mPeerInformation.customInfo.otapServerConfig.hControlPoint = pService->aCharacteristics[i].value.handle;
            }

            if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType128_c) &&
                (TRUE == FLib_MemCmp (pService->aCharacteristics[i].value.uuid.uuid128, uuid_char_otap_data, sizeof(pService->aCharacteristics[i].value.uuid.uuid128))))
            {
                /* Found OTAP Data Char */
                mPeerInformation.customInfo.otapServerConfig.hData = pService->aCharacteristics[i].value.handle;
            }
        }
    }
}

/*! *********************************************************************************
* \brief        Stores required characteristic handles for OTA.
*
* \param[in]    pChar    Characteristic.
********************************************************************************** */
static void BleApp_StoreCharHandles
(
    gattCharacteristic_t   *pChar
)
{
    uint8_t i;

    if ((pChar->value.uuidType == gBleUuidType128_c) &&
        (TRUE == FLib_MemCmp (pChar->value.uuid.uuid128, uuid_char_otap_control_point, sizeof(pChar->value.uuid.uuid128)))
       )
    {
        for (i = 0; i < pChar->cNumDescriptors; i++)
        {
            if (pChar->aDescriptors[i].uuidType == gBleUuidType16_c)
            {
                switch (pChar->aDescriptors[i].uuid.uuid16)
                {
                    case gBleSig_CCCD_d:
                    {
                        mPeerInformation.customInfo.otapServerConfig.hControlPointCccd = pChar->aDescriptors[i].handle;
                        break;
                    }
                    default:
                        ; /* For MISRA compliance */
                        break;
                }
            }
        }
    }
}

/*! *********************************************************************************
* \brief        Stores required descriptor handles for OTA.
*
* \param[in]    pDesc    Descriptor.
********************************************************************************** */
static void BleApp_StoreDescValues
(
    gattAttribute_t     *pDesc
)
{
    /* No descriptor values are stored in this application. */
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
    union
    {
        uint8_t                     errorTemp;
        attErrorCode_t              attErrorCodeTemp;
    }attErrorCodeVars;

    if (procedureResult == gGattProcError_c)
    {
        attErrorCodeVars.errorTemp = (uint8_t)error & 0xFFU;
        attErrorCode_t attError = attErrorCodeVars.attErrorCodeTemp;
        if (attError == gAttErrCodeInsufficientEncryption_c     ||
            attError == gAttErrCodeInsufficientAuthorization_c  ||
            attError == gAttErrCodeInsufficientAuthentication_c)
        {
#if gAppUsePairing_d
            /* Start Pairing Procedure */
            (void)Gap_Pair (serverDeviceId, &gPairingParameters);
#endif
        }

        BleApp_StateMachineHandler (serverDeviceId, mAppEvt_GattProcError_c);
    }
    else if (procedureResult == gGattProcSuccess_c)
    {
        switch(procedureType)
        {
            case gGattProcExchangeMtu_c:
            {
                BleApp_HandleAttMtuChange (serverDeviceId);
            }
            break;

            case gGattProcDiscoverAllPrimaryServices_c:         /* Fall-through */
            case gGattProcWriteCharacteristicDescriptor_c:
            break;


            case gGattProcDiscoverAllCharacteristics_c:
            {
                BleApp_StoreServiceHandles (mpServiceDiscoveryBuffer + mCurrentServiceInDiscoveryIndex);
            }
            break;

            case gGattProcDiscoverAllCharacteristicDescriptors_c:
            {
                BleApp_StoreCharHandles (mpCharDiscoveryBuffer + mCurrentCharInDiscoveryIndex);

                /* Move on to the next characteristic */
                mCurrentCharInDiscoveryIndex++;
            }
            break;

            case gGattProcReadCharacteristicDescriptor_c:
            {
                if (mpDescProcBuffer != NULL)
                {
                    BleApp_StoreDescValues (mpDescProcBuffer);
                }
            }
            break;

            case gGattProcWriteCharacteristicValue_c:
            {
                BleApp_HandleValueWriteConfirmations (serverDeviceId);
            }
            break;

            default:
                ; /* For MISRA compliance */
            break;
        }

        BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcComplete_c);
    }
    else
    {
        ; /* For MISRA compliance */
    }
}

/*! *********************************************************************************
* \brief        Handles command confirmations from peer
*
* \param[in]    deviceId    Peer device ID
********************************************************************************** */
static void BleApp_HandleValueWriteConfirmations (deviceId_t  deviceId)
{
    /* Handle all command confirmations here - only for commands sent form
     * the OTAP Server to the OTAP Client. */
    switch (otapServerData.lastCmdSentToOtapClient)
    {
    case gOtapCmdIdNewImageNotification_c:      /* Fall-through */
    case gOtapCmdIdNewImageInfoResponse_c:      /* Fall-through */
    case gOtapCmdIdErrorNotification_c:         /* Fall-through */

        ; /* Do nothing here at the moment.
        * All actions on the OTAP Server are triggered by requests made by the
        * OTAP Client. Also the OTAP Client may choose to not respond to a
        * command received from the OTAP Server. */
        break;
    default:
        ; /* For MISRA compliance */
    break;
    };
}

/*! *********************************************************************************
* \brief        Handles GATT indication callback from host stack.
*
* \param[in]    serverDeviceId                  GATT Server device ID.
* \param[in]    characteristicValueHandle       Characteristic value handle.
* \param[in]    aValue                          Value.
* \param[in]    valueLength                     Value length.
********************************************************************************** */
static void BleApp_GattIndicationCallback
(
    deviceId_t  serverDeviceId,
    uint16_t    characteristicValueHandle,
    uint8_t*    aValue,
    uint16_t    valueLength
)
{
    BleApp_AttributeIndicated (serverDeviceId,
                               characteristicValueHandle,
                               aValue,
                               valueLength);
}

/*! *********************************************************************************
* \brief        Handles attribute indication.
*
* \param[in]    serverDeviceId                  GATT Server device ID.
* \param[in]    characteristicValueHandle       Characteristic value handle.
* \param[in]    aValue                          Value.
* \param[in]    valueLength                     Value length.
********************************************************************************** */
static void BleApp_AttributeIndicated
(
    deviceId_t  deviceId,
    uint16_t    handle,
    uint8_t*    pValue,
    uint16_t    length
)
{
    union
    {
        uint8_t*                pValueTemp;
        otapCommand_t*          otapCommandTemp;
    }otapCommandVars;

    if (handle == mPeerInformation.customInfo.otapServerConfig.hControlPoint)
    {
        if (otapServerData.storageMode == gOtapServerStoargeNone_c)
        {
            otapCommandVars.pValueTemp = pValue;
            otapCommand_t*  pOtaCmd = otapCommandVars.otapCommandTemp;
            /* If this a is an OTAP Block Request then save the transfer method and channel in the
             * application data structure. This information is needed when forwarding data chunks
             * to the device which requested them. */
            if (pOtaCmd->cmdId == gOtapCmdIdImageBlockRequest_c)
            {
                FLib_MemCpy ((uint8_t*)(&otapServerData.transferMethod),
                             (uint8_t*)(&pOtaCmd->cmd.imgBlockReq.transferMethod),
                             sizeof(pOtaCmd->cmd.imgBlockReq.transferMethod));
                FLib_MemCpy ((uint8_t*)(&otapServerData.l2capChannelOrPsm),
                             (uint8_t*)(&pOtaCmd->cmd.imgBlockReq.l2capChannelOrPsm),
                             sizeof(pOtaCmd->cmd.imgBlockReq.l2capChannelOrPsm));

                if (otapServerData.transferMethod == gOtapTransferMethodL2capCoC_c)
                {
                    if (otapServerData.l2capChannelOrPsm != gOtap_L2capLePsm_c)
                    {
                        /* An invalid L2CAP PSM was received in the image block request,
                         * transform the received command into an error notification and send it to the peer.
                         * It will also be forwarded via the serial interface to the image server. */
                        pOtaCmd->cmdId = gOtapCmdIdErrorNotification_c;
                        pOtaCmd->cmd.errNotif.cmdId = gOtapCmdIdImageBlockRequest_c;
                        pOtaCmd->cmd.errNotif.errStatus = gOtapStatusUnexpectedL2capChannelOrPsm_c;

                        OtapServer_SendCommandToOtapClient (deviceId,
                                                            pOtaCmd,
                                                            cmdIdToCmdLengthTable[gOtapCmdIdErrorNotification_c]);
                    }
                    else if (otapServerData.l2capPsmConnected == FALSE)
                    {
                        /* An image block request was made with a CoC transfer method but the
                         * L2CAP Psm is not connected. Transform the received command into an
                         * error notification and send it to the peer.
                         * It will also be forwarded via the serial interface to the image server. */
                        pOtaCmd->cmdId = gOtapCmdIdErrorNotification_c;
                        pOtaCmd->cmd.errNotif.cmdId = gOtapCmdIdImageBlockRequest_c;
                        pOtaCmd->cmd.errNotif.errStatus = gOtapStatusNoL2capPsmConnection_c;

                        OtapServer_SendCommandToOtapClient (deviceId,
                                                            pOtaCmd,
                                                            cmdIdToCmdLengthTable[gOtapCmdIdErrorNotification_c]);

                        length = cmdIdToCmdLengthTable[gOtapCmdIdErrorNotification_c];
                    }
                    else
                    {
                        ; /* For MISRA compliance */
                    }
                }
            }
            /* If the OTAP Server does not have internal storage then all commands must be forwarded
             *  via the serial interface. */
            FsciBleOtap_SendPkt (&(pOtaCmd->cmdId),
                                 (uint8_t*)(&(pOtaCmd->cmd)),
                                 length - gOtap_CmdIdFieldSize_c);
        }
    }
    else
    {
        /*! A GATT Client is trying to GATT Indicate an unknown attribute value.
         *  This should not happen. Disconnect the link. */
        (void)Gap_Disconnect (deviceId);
    }
}

/*! *********************************************************************************
* \brief        Handles L2CAP data callback from host stack.
*
* \param[in]    deviceId      Peer device ID
* \param[in]    lePsm         L2CAP LE PSM.
* \param[in]    pPacket       Received packet.
* \param[in]    packetLength  Received packet length.
********************************************************************************** */
static void BleApp_L2capPsmDataCallback (deviceId_t     deviceId,
                                         uint16_t       lePsm,
                                         uint8_t*       pPacket,
                                         uint16_t       packetLength)
{
    /* Do nothing here. No L2CAP PSM packets are expected by the OTAP server demo application. */
}


static void BleApp_L2capPsmControlCallback(l2capControlMessage_t* pMessage)
{
    switch (pMessage->messageType)
    {
        case gL2ca_LePsmConnectRequest_c:
        {
            l2caLeCbConnectionRequest_t *pConnReq = &pMessage->messageData.connectionRequest;

            /* Respond to the peer L2CAP CB Connection request - send a connection response. */
            (void)L2ca_ConnectLePsm (gOtap_L2capLePsm_c,
                               pConnReq->deviceId,
                               mAppLeCbInitialCredits_c);
            break;
        }
        case gL2ca_LePsmConnectionComplete_c:
        {
            l2caLeCbConnectionComplete_t *pConnComplete = &pMessage->messageData.connectionComplete;

            if (pConnComplete->result == gSuccessful_c)
            {
                /* Set the application L2CAP PSM Connection flag to TRUE because there is no gL2ca_LePsmConnectionComplete_c
                 * event on the responder of the PSM connection. */
                otapServerData.l2capPsmConnected = TRUE;
                otapServerData.l2capPsmChannelId = pConnComplete->cId;

                if (pConnComplete->peerMtu > gOtap_l2capCmdMtuDataChunkOverhead_c)
                {
                    otapServerData.negotiatedMaxL2CapChunkSize = pConnComplete->peerMtu - gOtap_l2capCmdMtuDataChunkOverhead_c;
                }
            }
            break;
        }
        case gL2ca_LePsmDisconnectNotification_c:
        {
            l2caLeCbDisconnection_t *pCbDisconnect = &pMessage->messageData.disconnection;

            /* Call App State Machine */
            BleApp_StateMachineHandler (pCbDisconnect->deviceId, mAppEvt_CbDisconnected_c);

            otapServerData.l2capPsmConnected = FALSE;
            break;
        }
        case gL2ca_NoPeerCredits_c:
        {
            l2caLeCbNoPeerCredits_t *pCbNoPeerCredits = &pMessage->messageData.noPeerCredits;
            (void)L2ca_SendLeCredit (pCbNoPeerCredits->deviceId,
                               otapServerData.l2capPsmChannelId,
                               mAppLeCbInitialCredits_c);
            break;
        }
        case gL2ca_LocalCreditsNotification_c:
        {
            /* l2caLeCbLocalCreditsNotification_t *pMsg = &pMessage->messageData.localCreditsNotification; */

            break;
        }
        case gL2ca_Error_c:
        {
            /* Handle error */
            break;
        }
        default:
            ; /* For MISRA compliance */
            break;
    }
}

/*! *********************************************************************************
* \brief        Searches an AD structure for given data.
*
* \param[in]    pElement    AD element to search in.
* \param[in]    pData       Data to search for.
* \param[in]    iDataLen    Data length.
********************************************************************************** */
static bool_t MatchDataInAdvElementList(gapAdStructure_t *pElement, void *pData, uint8_t iDataLen)
{
    uint8_t i;
    bool_t status = FALSE;

    for (i = 0; i < (pElement->length - 1U); i += iDataLen)
    {
        if (FLib_MemCmp(pData, &pElement->aData[i], iDataLen))
        {
            status = TRUE;
            break;
        }
    }

    return status;
}

/*! *********************************************************************************
* \brief       Checks scan event data for the OTAP Custom Service.
*
* \param[in]    pData      Scan event data.
********************************************************************************** */
static bool_t CheckScanEvent(gapScannedDevice_t* pData)
{
    uint32_t index = 0;
    uint8_t name[10];
    uint8_t nameLength;
    bool_t foundMatch = FALSE;

    while (index < pData->dataLength)
    {
        gapAdStructure_t adElement;

        adElement.length = pData->data[index];
        adElement.adType = (gapAdType_t)pData->data[index + 1U];
        adElement.aData = &pData->data[index + 2U];

         /* Search for OTAP Custom Service */
        if ((adElement.adType == gAdIncomplete128bitServiceList_c) ||
            (adElement.adType == gAdComplete128bitServiceList_c))
        {
            foundMatch = MatchDataInAdvElementList(&adElement, &uuid_service_otap, 16);
        }

        if ((adElement.adType == gAdShortenedLocalName_c) ||
          (adElement.adType == gAdCompleteLocalName_c))
        {
            nameLength = MIN(adElement.length, 10U);
            FLib_MemCpy(name, adElement.aData, nameLength);
        }

        /* Move on to the next AD element type */
        index += adElement.length + sizeof(uint8_t);
    }
    return foundMatch;
}

/*! *********************************************************************************
* \brief       Resets service discovery.
********************************************************************************** */
static void BleApp_ServiceDiscoveryReset(void)
{
    if (mpServiceDiscoveryBuffer != NULL)
    {
        (void)MEM_BufferFree(mpServiceDiscoveryBuffer);
        mpServiceDiscoveryBuffer = NULL;
    }

    if (mpCharDiscoveryBuffer != NULL)
    {
        (void)MEM_BufferFree(mpCharDiscoveryBuffer);
        mpCharDiscoveryBuffer = NULL;
    }

    if (mpCharDescriptorBuffer != NULL)
    {
        (void)MEM_BufferFree(mpCharDescriptorBuffer);
        mpCharDescriptorBuffer = NULL;
    }
}

/*! *********************************************************************************
* \brief       Handles service discovery errors.
********************************************************************************** */
static void BleApp_ServiceDiscoveryErrorHandler(void)
{
   mPeerInformation.appState = mAppIdle_c;
   BleApp_ServiceDiscoveryReset();
}

/*! *********************************************************************************
* \brief       Handles completion of service discovery.
********************************************************************************** */
static void BleApp_ServiceDiscoveryCompleted(void)
{
    mPeerInformation.appState = mAppDescriptorSetup_c;
    BleApp_ServiceDiscoveryReset();

    if (mPeerInformation.customInfo.otapServerConfig.hControlPointCccd != 0U)
    {
        mpDescProcBuffer = MEM_BufferAlloc(sizeof(gattAttribute_t) + 23U);

        if (mpDescProcBuffer == NULL)
        {
            panic(0,0,0,0);
            return;
        }
        mpDescProcBuffer->handle = mPeerInformation.customInfo.otapServerConfig.hControlPointCccd;
        mpDescProcBuffer->paValue = (uint8_t*)(mpDescProcBuffer + 1);
        (void)GattClient_ReadCharacteristicDescriptor(mPeerInformation.deviceId, mpDescProcBuffer ,23);
    }
}

/*! *********************************************************************************
* \brief       State machine handler helper function (application running state).
*
* \param[in]   peerDeviceId     Peer device ID.
* \param[in]   event            Event.
********************************************************************************** */
void BleApp_StateMachineHandler_AppRunning(deviceId_t peerDeviceId, uint8_t event)
{
    union
    {
        uint8_t*                    cmdBufferTemp;
        otapCommand_t*              otapCommandTemp;
    }otapCommandVars;

    if(mPeerInformation.appState == mAppRunning_c)
    {
        {
            if (event == mAppEvt_GattProcComplete_c)
            {
                /* Write data in NVM */
                (void)Gap_SaveCustomPeerInformation(mPeerInformation.deviceId,
                                              (void*) &mPeerInformation.customInfo, 0,
                                              (uint16_t)(sizeof (appCustomInfo_t)));

                if (FALSE == otapServerData.sentInitialImgNotification)
                {
                    uint16_t avaialbleImgId;

                    FLib_MemCpy (&avaialbleImgId,
                                 otapServerData.images[0].imgId,
                                 sizeof(avaialbleImgId));
                    if ((avaialbleImgId != gBleOtaImageIdCurrentRunningImage_c) &&
                        (avaialbleImgId != gBleOtaImageIdNoImageAvailable_c))
                    {
                        otapCommand_t otapCommand;

                        otapCommand.cmdId = gOtapCmdIdNewImageNotification_c;
                        FLib_MemCpy (otapCommand.cmd.newImgNotif.imageId,
                                     otapServerData.images[0].imgId,
                                     gOtap_ImageIdFieldSize_c);
                        FLib_MemCpy (otapCommand.cmd.newImgNotif.imageVersion,
                                     otapServerData.images[0].imgVer,
                                     gOtap_ImageVersionFieldSize_c);
                        FLib_MemCpy ((uint8_t*)(&otapCommand.cmd.newImgNotif.imageFileSize),
                                     (uint8_t*)(&otapServerData.images[0].imgSize),
                                     sizeof(otapCommand.cmd.newImgNotif.imageFileSize));
                        otapServerData.sentInitialImgNotification = TRUE;

                        OtapServer_SendCommandToOtapClient (peerDeviceId,
                                                            &otapCommand,
                                                            cmdIdToCmdLengthTable[gOtapCmdIdNewImageNotification_c]);

                        otapServerData.sentInitialImgNotification = TRUE;
                    }
                }
            }
            else if (event == mAppEvt_FsciBleOtapCmdReceived_c)
            {
                if (*((otapCmdIdt_t*)(otapServerData.pLastFsciCmdId)) == gOtapCmdIdImageChunk_c)
                {
                    if (otapServerData.transferMethod == gOtapTransferMethodAtt_c)
                    {
                        uint8_t         cmdBuffer[gOtapCmdImageChunkAttMaxLength_c];
                        otapCommandVars.cmdBufferTemp = cmdBuffer;
                        otapCommand_t*  pOtapCommand = otapCommandVars.otapCommandTemp;

                        pOtapCommand->cmdId = gOtapCmdIdImageChunk_c;
                        FLib_MemCpy ((uint8_t*)(&pOtapCommand->cmd),
                                     otapServerData.pLastFsciCmdPayload,
                                     otapServerData.lastFsciCmdPayloadLen);

                        OtapServer_SendCImgChunkToOtapClient (peerDeviceId,
                                                              pOtapCommand,
                                                              (uint16_t)(otapServerData.lastFsciCmdPayloadLen + gOtap_CmdIdFieldSize_c));
                    }
                    else if (otapServerData.transferMethod == gOtapTransferMethodL2capCoC_c)
                    {
                        uint8_t         cmdBuffer[gOtapCmdImageChunkCocMaxLength_c];
                        otapCommandVars.cmdBufferTemp = cmdBuffer;
                        otapCommand_t*  pOtapCommand = otapCommandVars.otapCommandTemp;

                        if (otapServerData.l2capPsmConnected == TRUE)
                        {
                            pOtapCommand->cmdId = gOtapCmdIdImageChunk_c;
                            FLib_MemCpy ((uint8_t*)(&pOtapCommand->cmd),
                                         otapServerData.pLastFsciCmdPayload,
                                         otapServerData.lastFsciCmdPayloadLen);

                            OtapServer_SendCImgChunkToOtapClient (peerDeviceId,
                                                                  pOtapCommand,
                                                                  (uint16_t)(otapServerData.lastFsciCmdPayloadLen + gOtap_CmdIdFieldSize_c));
                        }
                        else
                        {
                            /* If the L2CAP PSM channel is not connected then send an error notification to the server
                             * via the serial interface and to the client over the air. */
                            pOtapCommand->cmdId = gOtapCmdIdErrorNotification_c;
                            pOtapCommand->cmd.errNotif.cmdId = gOtapCmdIdNoCommand_c;
                            pOtapCommand->cmd.errNotif.errStatus = gOtapStatusNoL2capPsmConnection_c;

                            OtapServer_SendCommandToOtapClient (peerDeviceId,
                                                                pOtapCommand,
                                                                cmdIdToCmdLengthTable[gOtapCmdIdErrorNotification_c]);

                            FsciBleOtap_SendPkt (&(pOtapCommand->cmdId),
                                                 (uint8_t*)(&(pOtapCommand->cmd)),
                                                 (uint16_t)(sizeof(otapErrNotification_t)));
                        }
                    }
                    else
                    {
                        ; /* For MISRA compliance */
                    }

                }
                else
                {
                    otapCommand_t otapCommand;

                    otapCommand.cmdId = *((uint8_t*)(otapServerData.pLastFsciCmdId));
                    FLib_MemCpy ((uint8_t*)(&otapCommand.cmd),
                                 otapServerData.pLastFsciCmdPayload,
                                 otapServerData.lastFsciCmdPayloadLen);

                    OtapServer_SendCommandToOtapClient (peerDeviceId,
                                                        &otapCommand,
                                                        (uint16_t)(otapServerData.lastFsciCmdPayloadLen + gOtap_CmdIdFieldSize_c));
                }
            }
            else if (event == mAppEvt_PeerConnected_c)
            {
                /* Check if required service characteristic discoveries by the client app have been done
                 * and change the client application state accordingly. */
                if ((mPeerInformation.customInfo.otapServerConfig.hControlPoint != gGattDbInvalidHandle_d) &&
                    (mPeerInformation.customInfo.otapServerConfig.hControlPointCccd != gGattDbInvalidHandle_d) &&
                    (mPeerInformation.customInfo.otapServerConfig.hData != gGattDbInvalidHandle_d) &&
                    (mPeerInformation.isBonded == FALSE))
                {
                    /* Set indication bit for a CCCD descriptor if service discovery was performed and the
                     * devices are not bonded, no security is used. */
                    uint16_t value = gCccdIndication_c;

                    /* Enable indications for the OTAP Control point characteristic. */
                    mpDescProcBuffer->handle = mPeerInformation.customInfo.otapServerConfig.hControlPointCccd;
                    mpDescProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
                    (void)GattClient_WriteCharacteristicDescriptor(peerDeviceId, mpDescProcBuffer, (uint16_t)(sizeof(uint16_t)),
                                                             (uint8_t*)&value);
                }
            }
            else if (event == mAppEvt_PairingComplete_c)
            {
                uint16_t value = gCccdIndication_c;
                if (mpDescProcBuffer != NULL)
                {
                    /* Enable indications for the OTAP Control point characteristic. */
                    mpDescProcBuffer->handle = mPeerInformation.customInfo.otapServerConfig.hControlPointCccd;
                    mpDescProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
                    (void)GattClient_WriteCharacteristicDescriptor(peerDeviceId, mpDescProcBuffer, (uint16_t)(sizeof(uint16_t)),
                                                           (uint8_t*)&value);
                }
                else
                {
                    panic(0,0,0,0);
                }
            }
            else
            {
                ; /* For MISRA compliance */
            }
        }
    }
}

/*! *********************************************************************************
* \brief       State machine handler helper function (service discovery).
*
* \param[in]   peerDeviceId     Peer device ID.
* \param[in]   event            Event.
********************************************************************************** */
void BleApp_StateMachineHandler_ServiceDiscovery(deviceId_t peerDeviceId, uint8_t event)
{

    switch (mPeerInformation.appState)
    {
        case mAppCharServiceDisc_c:
        {
            if (event == mAppEvt_GattProcComplete_c)
            {
                gattService_t        *pCurrentService = mpServiceDiscoveryBuffer + mCurrentServiceInDiscoveryIndex;
                gattCharacteristic_t *pCurrentChar = pCurrentService->aCharacteristics + mCurrentCharInDiscoveryIndex;

                if (mCurrentCharInDiscoveryIndex < pCurrentService->cNumCharacteristics)
                {
                    /* Find next characteristic with descriptors*/
                    while (mCurrentCharInDiscoveryIndex < pCurrentService->cNumCharacteristics - 1U)
                    {
                        /* Check if we have handles available between adjacent characteristics */
                        if (pCurrentChar->value.handle + 2U < (pCurrentChar + 1U)->value.handle)
                        {
                            FLib_MemSet(mpCharDescriptorBuffer, 0, sizeof(gattAttribute_t));
                            pCurrentChar->aDescriptors = mpCharDescriptorBuffer;
                            (void)GattClient_DiscoverAllCharacteristicDescriptors(peerDeviceId,
                                                    pCurrentChar,
                                                    (pCurrentChar + 1)->value.handle,
                                                    gMaxCharDescriptorsCount_d);
                            return;
                        }

                        mCurrentCharInDiscoveryIndex++;
                        pCurrentChar = pCurrentService->aCharacteristics + mCurrentCharInDiscoveryIndex;
                    }

                    /* Made it to the last characteristic. Check against service end handle*/
                    if (pCurrentChar->value.handle < pCurrentService->endHandle)
                    {
                        FLib_MemSet(mpCharDescriptorBuffer, 0, sizeof(gattAttribute_t));
                        pCurrentChar->aDescriptors = mpCharDescriptorBuffer;
                        (void)GattClient_DiscoverAllCharacteristicDescriptors(peerDeviceId,
                                                    pCurrentChar,
                                                    pCurrentService->endHandle,
                                                    gMaxCharDescriptorsCount_d);
                         return;
                    }
                }

                /* Move on to the next service */
                mCurrentServiceInDiscoveryIndex++;

                /* Reset characteristic discovery */
                mCurrentCharInDiscoveryIndex = 0;

                if (mCurrentServiceInDiscoveryIndex < mcPrimaryServices)
                {
                    /* Allocate memory for Char Discovery */
                    (mpServiceDiscoveryBuffer + mCurrentServiceInDiscoveryIndex)->aCharacteristics = mpCharDiscoveryBuffer;

                     /* Start Characteristic Discovery for current service */
                    (void)GattClient_DiscoverAllCharacteristicsOfService(peerDeviceId,
                                                mpServiceDiscoveryBuffer + mCurrentServiceInDiscoveryIndex,
                                                gMaxServiceCharCount_d);
                }
                else
                {
                    BleApp_ServiceDiscoveryCompleted();
                }
            }
            else if (event == mAppEvt_GattProcError_c)
            {
                BleApp_ServiceDiscoveryErrorHandler();
            }
            else
            {
                ; /* For MISRA compliance */
            }
        }
        break;

        case mAppDescriptorSetup_c:
        {
            if (event == mAppEvt_GattProcComplete_c)
            {
                if (mPeerInformation.customInfo.otapServerConfig.hControlPointCccd != 0U)
                {
                    /* Set indication bit for a CCCD descriptor.  */
                    uint16_t value = gCccdIndication_c;

                    if (mpDescProcBuffer == NULL)
                    {
                        panic(0,0,0,0);
                        return;
                    }

                    /* Moving to Running State*/
                    mPeerInformation.appState = mAppRunning_c;
                    /* Enable indications for the OTAP Control point characteristic. */
                    mpDescProcBuffer->handle = mPeerInformation.customInfo.otapServerConfig.hControlPointCccd;
                    mpDescProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
                    (void)GattClient_WriteCharacteristicDescriptor(peerDeviceId, mpDescProcBuffer, (uint16_t)(sizeof(uint16_t)),
                                                             (uint8_t*)&value);
                }
            }
            else if (event == mAppEvt_PairingComplete_c)
            {
                /* Continue after pairing is complete */
                (void)GattClient_ReadCharacteristicDescriptor(mPeerInformation.deviceId, mpDescProcBuffer ,23);
            }
            else
            {
                ; /* For MISRA compliance */
            }
            break;
        }

        default:
        {
            BleApp_StateMachineHandler_AppRunning(peerDeviceId, event);
            break;
        }
    }

}

/*! *********************************************************************************
* \brief       Main state machine handler function.
*
* \param[in]   peerDeviceId     Peer device ID.
* \param[in]   event            Event.
********************************************************************************** */
void BleApp_StateMachineHandler(deviceId_t peerDeviceId, uint8_t event)
{
    switch (mPeerInformation.appState)
    {
        case mAppIdle_c:
        {
            if (event == mAppEvt_PeerConnected_c)
            {
                /* Check if required service characteristic discoveries by the client app have been done
                 * and change the client application state accordingly. */
                if ((mPeerInformation.customInfo.otapServerConfig.hControlPoint == gGattDbInvalidHandle_d) ||
                    (mPeerInformation.customInfo.otapServerConfig.hControlPointCccd == gGattDbInvalidHandle_d) ||
                    (mPeerInformation.customInfo.otapServerConfig.hData == gGattDbInvalidHandle_d))
                {
                    otapCommand_t   otaCmd;

                    /* Moving to Exchange MTU State */
                    mPeerInformation.appState = mAppExchangeMtu_c;
                    (void)GattClient_ExchangeMtu(peerDeviceId, gAttMaxMtu_c);

                    /*! When a new unknown peer connects send a New Image Info request via FSCI
                     *  with the image version set to request info about all available images. */
                    otaCmd.cmdId = gOtapCmdIdNewImageInfoRequest_c;
                    /* Set the image ID to all zeroes representing current running image ID. */
                    FLib_MemSet (otaCmd.cmd.newImgInfoReq.currentImageId,
                                 0x00,
                                 sizeof(otaCmd.cmd.newImgInfoReq.currentImageId));\
                    /* Set the image version to all 0x00 representing a request for all available images. */
                    FLib_MemSet (otaCmd.cmd.newImgInfoReq.currentImageVersion,
                                 0x00,
                                 sizeof(otaCmd.cmd.newImgInfoReq.currentImageVersion));
                    FsciBleOtap_SendPkt (&otaCmd.cmdId,
                                         (uint8_t*)(&otaCmd.cmd.newImgInfoReq),
                                         (uint16_t)(sizeof(otapCmdNewImgInfoReq_t)));
                }
                else
                {
                    /* Set indication bit for a CCCD descriptor.  */
                    uint16_t value = gCccdIndication_c;

                    if (mpDescProcBuffer == NULL)
                    {
                        mpDescProcBuffer = MEM_BufferAlloc(sizeof(gattAttribute_t) + 23U);
                        if (mpDescProcBuffer == NULL)
                        {
                            panic(0,0,0,0);
                            break;
                        }
                    }

                    /* Moving to Running State */
                    mPeerInformation.appState = mAppRunning_c;
                    /* Enable indications for the OTAP Control point characteristic. */
                    mpDescProcBuffer->handle = mPeerInformation.customInfo.otapServerConfig.hControlPointCccd;
                    mpDescProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
                    (void)GattClient_WriteCharacteristicDescriptor(peerDeviceId, mpDescProcBuffer, (uint16_t)(sizeof(uint16_t)),
                                                             (uint8_t*)&value);
                }
            }
        }
        break;

        case mAppExchangeMtu_c:
        {
            if (event == mAppEvt_GattProcComplete_c)
            {

                /* Allocate memory for Service Discovery */
                mpServiceDiscoveryBuffer = MEM_BufferAlloc(sizeof(gattService_t) * gMaxServicesCount_d);
                mpCharDiscoveryBuffer = MEM_BufferAlloc(sizeof(gattCharacteristic_t) * gMaxServiceCharCount_d);
                mpCharDescriptorBuffer = MEM_BufferAlloc(sizeof(gattAttribute_t) * gMaxCharDescriptorsCount_d);

                if ((mpServiceDiscoveryBuffer == NULL) || (mpCharDiscoveryBuffer == NULL) || (mpCharDescriptorBuffer == NULL))
                {
                    BleApp_ServiceDiscoveryErrorHandler();
                    return;
                }

                /* Moving to Primary Service Discovery State*/
                mPeerInformation.appState = mAppPrimaryServiceDisc_c;

                /* Start Service Discovery*/
                (void)GattClient_DiscoverAllPrimaryServices(
                                            peerDeviceId,
                                            mpServiceDiscoveryBuffer,
                                            gMaxServicesCount_d,
                                            &mcPrimaryServices);
            }
            else if (event == mAppEvt_GattProcError_c)
            {
               BleApp_ServiceDiscoveryErrorHandler();
            }
            else
            {
                ; /* For MISRA compliance */
            }
        }
        break;

        case mAppPrimaryServiceDisc_c:
        {
            if (event == mAppEvt_GattProcComplete_c)
            {
                /* We found at least one service */
                if (mcPrimaryServices != 0U)
                {
                    /* Moving to Characteristic Discovery State*/
                    mPeerInformation.appState = mAppCharServiceDisc_c;

                    /* Start characteristic discovery with first service*/
                    mCurrentServiceInDiscoveryIndex = 0;
                    mCurrentCharInDiscoveryIndex = 0;

                    mpServiceDiscoveryBuffer->aCharacteristics = mpCharDiscoveryBuffer;

                    /* Start Characteristic Discovery for current service */
                    (void)GattClient_DiscoverAllCharacteristicsOfService(
                                                peerDeviceId,
                                                mpServiceDiscoveryBuffer,
                                                gMaxServiceCharCount_d);
                }
            }
            else if (event == mAppEvt_GattProcError_c)
            {
                BleApp_ServiceDiscoveryErrorHandler();
            }
            else
            {
                ; /* For MISRA compliance */
            }
        }
        break;

        default:
        {
            BleApp_StateMachineHandler_ServiceDiscovery(peerDeviceId, event);
            break;
        }
    }
}

/*! *********************************************************************************
* \brief       Scanning timeout timer callback.
*
* \param[in]   pParam   Callback parameter.
********************************************************************************** */
static void ScanningTimeoutTimerCallback(void* pParam)
{
    /* Stop scanning */
    if (mScanningOn)
    {
        (void)Gap_StopScanning();
    }
}

/*! *********************************************************************************
* \brief       Sends commands to the OTAP client.
*
* \param[in]   otapClientDevId      Client device ID.
* \param[in]   pCommand             Command to be sent.
* \param[in]   cmdLength            Command length.
********************************************************************************** */
static void OtapServer_SendCommandToOtapClient (deviceId_t  otapClientDevId,
                                                void*       pCommand,
                                                uint16_t    cmdLength)
{
    /* GATT Characteristic to be written - OTAP Client Control Point */
    gattCharacteristic_t    otapCtrlPointChar;
    bleResult_t             bleResult;

    /* Only the value handle element of this structure is relevant for this operation. */
    otapCtrlPointChar.value.handle = mPeerInformation.customInfo.otapServerConfig.hControlPoint;
    otapCtrlPointChar.value.valueLength = 0;
    otapCtrlPointChar.cNumDescriptors = 0;
    otapCtrlPointChar.aDescriptors = NULL;

    bleResult = GattClient_SimpleCharacteristicWrite (mPeerInformation.deviceId,
                                                      &otapCtrlPointChar,
                                                      cmdLength,
                                                      pCommand);

    if (gBleSuccess_c == bleResult)
    {
        otapServerData.lastCmdSentToOtapClient = (otapCmdIdt_t)(((otapCommand_t*)pCommand)->cmdId);
    }
    else
    {
        /*! A BLE error has occurred - Disconnect */
        (void)Gap_Disconnect (otapClientDevId);
    }
}

/*! *********************************************************************************
* \brief       Sends an image chunk to the OTAP client.
*
* \param[in]   otapClientDevId      Client device ID.
* \param[in]   pChunk               Image chunk.
* \param[in]   chunkCmdLength       Image chunk length.
********************************************************************************** */
static void OtapServer_SendCImgChunkToOtapClient (deviceId_t  otapClientDevId,
                                                  void*       pChunk,
                                                  uint16_t    chunkCmdLength)
{
    bleResult_t     bleResult = gBleSuccess_c;

    if (otapServerData.transferMethod == gOtapTransferMethodAtt_c)
    {
        /* GATT Characteristic to be written without response - OTAP Client Data */
        gattCharacteristic_t    otapDataChar;

        /* Only the value handle element of this structure is relevant for this operation. */
        otapDataChar.value.handle = mPeerInformation.customInfo.otapServerConfig.hData;
        otapDataChar.value.valueLength = 0;
        otapDataChar.cNumDescriptors = 0;

        bleResult = GattClient_CharacteristicWriteWithoutResponse (mPeerInformation.deviceId,
                                                                   &otapDataChar,
                                                                   chunkCmdLength,
                                                                   pChunk);
    }
    else if (otapServerData.transferMethod == gOtapTransferMethodL2capCoC_c)
    {
        bleResult =  L2ca_SendLeCbData (mPeerInformation.deviceId,
                                        otapServerData.l2capPsmChannelId,
                                        pChunk,
                                        chunkCmdLength);
    }
    else
    {
        ; /* For MISRA compliance */
    }

    if (gBleSuccess_c != bleResult)
    {
        /*! A BLE error has occurred - Disconnect */
        (void)Gap_Disconnect (otapClientDevId);
    }
}

/*! *********************************************************************************
* \brief       Handles disconnection event.
*
* \param[in]   deviceId      Client device ID.
********************************************************************************** */
static void OtapServer_HandleDisconnectionEvent (deviceId_t deviceId)
{
    otapCommand_t   otaCmd;

    /* On the disconnection of the peer OTAP Client send a Stop Image Transfer
     * command via the FSCI interface. */
    otaCmd.cmdId = gOtapCmdIdStopImageTransfer_c;
    /* Set the image ID to all zeroes. */
    FLib_MemSet (otaCmd.cmd.stopImgTransf.imageId,
                 0x00,
                 sizeof(otaCmd.cmd.stopImgTransf.imageId));
    FsciBleOtap_SendPkt (&otaCmd.cmdId,
                         (uint8_t*)(&otaCmd.cmd.stopImgTransf),
                         (uint16_t)(sizeof(otapCmdStopImgTransfer_t)));
}

/*! *********************************************************************************
* @}
********************************************************************************** */
