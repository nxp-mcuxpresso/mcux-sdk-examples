/*! *********************************************************************************
* \addtogroup BLE OTAP Client State Machine
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright 2019-2023 NXP
*
*
* \file
*
* This file is the source file for the BLE OTAP Client State Machine application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "EmbeddedTypes.h"

#include "RNG_Interface.h"
#include "fsl_component_button.h"
#include "fsl_component_led.h"
#include "fsl_component_timer_manager.h"
#include "fsl_component_panic.h"
#include "fsl_adapter_reset.h"

#include "OtaSupport.h"

/* Framework / Drivers */
#include "RNG_Interface.h"
#include "FunctionLib.h"

/* BLE Host Stack */
#include "gatt_interface.h"
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gatt_database.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"
#include "gatt_db_handles.h"

#if defined(gOtapClientL2Cap_d) && (gOtapClientL2Cap_d == 1)
#include "l2ca_cb_interface.h"
#include "l2ca_types.h"
#endif

/* Profile / Services */
#include "battery_interface.h"
#include "device_info_interface.h"
#include "otap_interface.h"

/* Connection Manager */
#include "ble_conn_manager.h"
#include "board.h"
#include "app_conn.h"
#include "otap_client.h"
/************************************************************************************
*************************************************************************************
* Extern functions
*************************************************************************************
************************************************************************************/
#define ResetMCU HAL_ResetMCU

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
/* OTAP Timing Parameters */
/* Time interval for checking the OTA transfer progress */
#ifndef gOTAPTransferCheckIntervalMs
#define gOTAPTransferCheckIntervalMs     10000 /* milliseconds */
#endif

/* Time interval until clearing the OTA progress after disconnection, if transfer is not re-started */
#ifndef gOTAPCancelIntervalMs
#define gOTAPCancelIntervalMs            20000 /* milliseconds */
#endif

/* Number of OTA Image Requests sent to Server until cancelling the OTA transfer */
#ifndef gOTAPMaxNbRetries
#define gOTAPMaxNbRetries                5U
#endif

#if defined(gOtapClientL2Cap_d) && (gOtapClientL2Cap_d == 1)
#define mAppLeCbInitialCredits_c        (32768)
#endif

#ifndef gBootData_SectorsBitmap_Size_c
#define gBootData_SectorsBitmap_Size_c  (32)
#endif

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
typedef enum otapClientState_tag
{
    mOtapClientStateIdle_c                  = 0x00,
    mOtapClientStateDownloadingImage_c      = 0x01,
    mOtapClientStateImageDownloadComplete_c = 0x02,
} otapClientState_t;

/*! Structure containing the OTAP Client functional data. */
typedef struct otapClientAppData_tag
{
    otapClientState_t           state;
    const uint8_t               currentImgId[gOtap_ImageIdFieldSize_c];         /*!< Id of the currently running image on the OTAP Client */
    const uint8_t               currentImgVer[gOtap_ImageVersionFieldSize_c];   /*!< Version of the currently running image on the OTAP Client */
    deviceId_t                  peerOtapServer;                                 /*!< Device id of the OTAP Server a new image is being downloaded from. */
    uint8_t                     imgId[gOtap_ImageIdFieldSize_c];                /*!< Id of the image being downloaded from the OTAP Server */
    uint8_t                     imgVer[gOtap_ImageVersionFieldSize_c];          /*!< Version of the image being downloaded from the OTAP Server */
    uint32_t                    imgSize;                                        /*!< Size of the image file being downloaded from the OTAP Server */
    uint16_t                    imgComputedCrc;                                 /*!< Computed 16 bit CRC of the image file used in this implementation. */
    uint16_t                    imgReceivedCrc;                                 /*!< Received 16 bit CRC of the image file used in this implementation. */
    uint8_t                     imgSectorBitmap[gBootData_SectorsBitmap_Size_c];    /*!< Flash sector bitmap for the received image for the current implementation. */
    uint32_t                    currentPos;                                     /*!< Current position of the file being downloaded. */
    uint16_t                    chunkSize;                                      /*!< Current chunk size for the image file transfer. */
    uint16_t                    chunkSeqNum;                                    /*!< Current chunk sequence number for the block being transferred. */
    uint16_t                    totalBlockChunks;                               /*!< Total number of chunks for the block being transferred. */
    uint32_t                    totalBlockSize;                                 /*!< Total size of the block which was requested. may be smaller than totalBlockChunks * chunkSize. */
    const otapTransferMethod_t  transferMethod;                                 /*!< Currently used transfer method for the OTAP Image File */
    uint16_t                    l2capChannelOrPsm;                              /*!< L2CAP Channel or PSM used for the transfer of the image file: channel 0x0004 for ATT, application specific PSM for CoC. */
    bool_t                      serverWrittenCccd;                              /*!< The OTAP Server has written the CCCD to receive commands from the OTAp Client. */
    otapCmdIdt_t                lastCmdSentToOtapServer;                        /*!< The last command sent to the OTAP Server for which an Indication is expected. */
    uint16_t                    negotiatedMaxAttChunkSize;                      /*!< The negotiated maximum ATT chunk size based on the negotiated ATT MTU between the OTAP Server and the OTAP Client. */
    uint16_t                    negotiatedMaxL2CapChunkSize;                    /*!< The negotiated maximum L2CAP chunk size based on the negotiated L2CAP MTU between the OTAP Server and the OTAP Client. */
#if defined(gOtapClientL2Cap_d) && (gOtapClientL2Cap_d == 1)
    bool_t                      l2capPsmConnected;                              /*!< Flag which is set to true if an L2CAP PSM connection is currently established with a peer device. */
    uint16_t                    l2capPsmChannelId;                              /*!< Channel Id for an L2CAP PSM connection currently established with a peer device. */
#endif
} otapClientAppData_t;


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

/* OTAP Service Data */
static otapClientConfig_t otapServiceConfig = {(uint16_t)service_otap};
static uint16_t otapWriteNotifHandles[] = {(uint16_t)value_otap_control_point,
                                           (uint16_t)value_otap_data};

/*! OTAP Client data structure.
 *  Contains current image information and state informations
 *  regarding the image download procedure. */
static otapClientAppData_t     otapClientData =
{
    .state = mOtapClientStateIdle_c,
    .currentImgId = {0x00, 0x00},     // Current Running Image Id - should be 0x0000
    .currentImgVer = {0x01, 0x00, 0x00,    // Build Version
                      0x41,                // Stack Version
                      0x11, 0x11, 0x11,    // Hardware Id
                      0x01                 // Manufacturer Id
                     },               // Current Image Version
    .peerOtapServer = gInvalidDeviceId_c,
    .imgId = {0x00, 0x00},
    .imgVer = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    .imgSize = 0,
    .imgComputedCrc = 0,
    .imgReceivedCrc = 0,
    .imgSectorBitmap = {0x00},
    .currentPos = 0,
    .chunkSize = 0,
    .chunkSeqNum = 0,
    .totalBlockChunks = 0,
    .totalBlockSize = 0,
#if defined(gOtapClientL2Cap_d) && (gOtapClientL2Cap_d == 1)
    .transferMethod = gOtapTransferMethodL2capCoC_c,   // Transfer method L2CAP Credit based
    .l2capChannelOrPsm = gOtap_L2capLePsm_c,   // The L2CAP Otap PSM
#elif defined(gOtapClientAtt_d) && (gOtapClientAtt_d == 1)
    .transferMethod = gOtapTransferMethodAtt_c,   // The default transfer method is ATT
    .l2capChannelOrPsm = gL2capCidAtt_c,   // The default L2CAP channel is the ATT Channel
#endif
    .lastCmdSentToOtapServer = gOtapCmdIdNoCommand_c,
    .negotiatedMaxAttChunkSize = gAttDefaultMtu_c - gOtap_AttCommandMtuDataChunkOverhead_c,
    .negotiatedMaxL2CapChunkSize = gOtap_ImageChunkDataSizeL2capCoc_c,
#if defined(gOtapClientL2Cap_d) && (gOtapClientL2Cap_d == 1)
    .l2capPsmConnected = FALSE,
    .l2capPsmChannelId = 0,
#endif
};

/* Timer that periodically checks if OTA transfer is progressing */
static TIMER_MANAGER_HANDLE_DEFINE(mTransferStatusCheckTmrId);

#if (defined gAppOtaASyncFlashTransactions_c && (gAppOtaASyncFlashTransactions_c >0))
#define gAppOtaPostedOpsBufferSize_c (gAppOtaNumberOfTransactions_c*gOtaTransactionSz_d)
static uint32_t mPostedTransactionsBuffer[(gAppOtaPostedOpsBufferSize_c +3)/4];
#endif

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/
#if defined(gOtapClientL2Cap_d) && (gOtapClientL2Cap_d == 1)
/* L2CAP LE PSM callbacks */
static void BleApp_L2capPsmDataCallback (deviceId_t deviceId, uint16_t lePsm, uint8_t* pPacket, uint16_t packetLength);
static void BleApp_L2capPsmControlCallback (l2capControlMessage_t* pMessage);
static void OtapClient_HandlePsmConnectionComplete (l2caLeCbConnectionComplete_t *pConnComplete);
static void OtapClient_HandlePsmDisconnection (l2caLeCbDisconnection_t * pCbDisconnect);
#endif

/* OTAP Client functions */
/* Commands received from the OTAP Server */
static void OtapClient_HandleDataChunk (deviceId_t deviceId, uint16_t length, uint8_t* pData);
static void OtapClient_HandleNewImageNotification (deviceId_t deviceId, uint16_t length, uint8_t* pValue);
static void OtapClient_HandleNewImageInfoResponse (deviceId_t deviceId, uint16_t length, uint8_t* pValue);
static void OtapClient_HandleErrorNotification (deviceId_t deviceId, uint16_t length, uint8_t* pValue);
/* Confirmations of commands sent to the OTAP Server */
static void OtapClient_HandleNewImageInfoRequestConfirmation (deviceId_t deviceId);
static void OtapClient_HandleImageBlockRequestConfirmation (deviceId_t deviceId);
static void OtapClient_HandleImageTransferCompleteConfirmation (deviceId_t deviceId);
static void OtapClient_HandleErrorNotificationConfirmation (deviceId_t deviceId);
static void OtapClient_HandleStopImageTransferConfirmation (deviceId_t deviceId);
/* Otap Client operations */
static void OtapClient_ContinueImageDownload (deviceId_t deviceId);
static void OtapClient_ResetBlockTransferParameters (deviceId_t deviceId);
static void OtapClient_HandleServerCommError (deviceId_t deviceId);
static bool_t OtapClient_IsRemoteImageNewer (uint8_t* pRemoteImgId, uint8_t* pRemoteImgVer);
static otapStatus_t OtapClient_IsImageFileHeaderValid (bleOtaImageFileHeader_t* imgFileHeader);
static bleResult_t OtapClient_SendImageRequest(void);
static void OtapClient_TimerCb(void * param);
static void OtapClient_CancelOTA(void *param);
static void OtapClient_NewImageInfoRequest(deviceId_t deviceId);

static otapStatus_t OtapClient_ValidateOtapCommand
(
    deviceId_t                  deviceId,
    uint16_t                    commandLen,
    otapCmdImgChunkCoc_t        *pDataChunk,
    uint16_t                    dataLen
);

static otapStatus_t OtapClient_ReceiveOtaImageFileHeader
(
    uint16_t                    *pDataLen,
    uint8_t                     **pData,
    bleOtaImageFileHeader_t     *pImageFileHeader,
    uint32_t                    *pCurrentImgElemRcvdLen
);

static void OtapClient_ReceiveSubElementHeader
(
    uint16_t                    *pDataLen,
    uint8_t                     **pData,
    subElementHeader_t          *pSubElemHdr,
    uint32_t                    *pCurrentImgElemRcvdLen,
    uint32_t                    *pElementEnd
);

static otapStatus_t OtapClient_HandleSubElementPayload
(
    uint8_t             *pData,
    uint32_t            elementChunkLength,
    subElementHeader_t  *pSubElemHdr,
    uint32_t            currentImgElemRcvdLen
);

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief        Configures the OTAP service.
*
*\return        TRUE - Successfully configured, FALSE - an error occurred
********************************************************************************** */
bool_t OtapClient_Config(void)
{
    bool_t retStatus = TRUE;
    otaResult_t ota_status;
    bleResult_t ble_status = gBleUnexpectedError_c;
    timer_status_t timerStatus = kStatus_TimerSuccess;

    do {
 #if (defined gAppOtaASyncFlashTransactions_c && (gAppOtaASyncFlashTransactions_c >0))
        ota_config_t otaConfig;

        ota_status = OTA_ServiceInit(&mPostedTransactionsBuffer[0], gAppOtaPostedOpsBufferSize_c);
        if (ota_status != gOtaSuccess_c)
        {
            retStatus = FALSE;
            break;
        }

        OTA_GetDefaultConfig(&otaConfig);
        /* Configure the max consecutive OTA transactions processed in OTA_TransactionResume and enable
         * PostedOpInIdleTask to make sure the queue is processed only during idle task.
         * This ensures the system is blocked when the system is in idle, and not during high priority tasks.
         * gAppOtaMaxConsecutiveTransactions_c can be tuned to reduce or increase the maximum blocking time during idle,
         * which relies on different factors such as: internal or external flash, flash performances, real time
         * constraints... */
        otaConfig.maxConsecutiveTransactions = (int)gAppOtaMaxConsecutiveTransactions_c;
        otaConfig.PostedOpInIdleTask = TRUE;
        OTA_SetConfig(&otaConfig);
#endif
#if (defined gAppOtaExternalStorage_c && (gAppOtaExternalStorage_c == 1))
        ota_status = OTA_SelectExternalStoragePartition();
#else
        ota_status = OTA_SelectInternalStoragePartition();
#endif
        if (ota_status != gOtaSuccess_c)
        {
            retStatus = FALSE;
            break;
        }

        ble_status = GattServer_RegisterHandlesForWriteNotifications (NumberOfElements(otapWriteNotifHandles),
                                                                      otapWriteNotifHandles);

        if (ble_status != gBleSuccess_c)
        {
            retStatus = FALSE;
            break;
        }

 #if defined(gOtapClientL2Cap_d) && (gOtapClientL2Cap_d == 1)
        /* Register OTAP L2CAP PSM */
        ble_status = L2ca_RegisterLePsm (gOtap_L2capLePsm_c,
                            gOtapCmdImageChunkCocMaxLength_c);  /*!< The negotiated MTU must be higher than the biggest data chunk that will be sent fragmented */

        if (ble_status != gBleSuccess_c)
        {
            retStatus = FALSE;
            break;
        }

        /* Register stack callbacks */
        ble_status = App_RegisterLeCbCallbacks(BleApp_L2capPsmDataCallback, BleApp_L2capPsmControlCallback);

        if (ble_status != gBleSuccess_c)
        {
            retStatus = FALSE;
            break;
        }
#endif

        /* Start OTA service */
        ble_status = OtapCS_Start(&otapServiceConfig);

        if (ble_status != gBleSuccess_c)
        {
            retStatus = FALSE;
            break;
        }

        /* Allocate timer that checks periodically the transfer status */
        timerStatus = TM_Open(mTransferStatusCheckTmrId);
        if (timerStatus != kStatus_TimerSuccess)
        {
            retStatus = FALSE;
            break;
        }
    } while (FALSE);
    return retStatus;
}

/*! *********************************************************************************
* \brief        Callback for ATT MTU changed event.
*
* \param[in]    deviceId        The device ID of the connected peer.
* \param[in]    negotiatedMtu   Negotiated MTU length
********************************************************************************** */
void OtapClient_AttMtuChanged (deviceId_t deviceId, uint16_t negotiatedMtu)
{
    otapClientData.negotiatedMaxAttChunkSize = negotiatedMtu - gOtap_AttCommandMtuDataChunkOverhead_c;
}

#if defined(gOtapClientL2Cap_d) && (gOtapClientL2Cap_d == 1)
/*! *********************************************************************************
* \brief        Callback for incoming credit based data.
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
    OtapClient_HandleDataChunk (deviceId,
                                packetLength,
                                pPacket);
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
        case gL2ca_LePsmConnectRequest_c:
        {
            l2caLeCbConnectionRequest_t *pConnReq = &pMessage->messageData.connectionRequest;

            /* This message is unexpected on the OTAP Client, the OTAP Client sends L2CAP PSM connection
             * requests and expects L2CAP PSM connection responses.
             * Disconnect the peer. */
            (void)Gap_Disconnect (pConnReq->deviceId);

            break;
        }
        case gL2ca_LePsmConnectionComplete_c:
        {
            l2caLeCbConnectionComplete_t *pConnComplete = &pMessage->messageData.connectionComplete;

            /* Call the application PSM connection complete handler. */
            OtapClient_HandlePsmConnectionComplete (pConnComplete);

            break;
        }
        case gL2ca_LePsmDisconnectNotification_c:
        {
            l2caLeCbDisconnection_t *pCbDisconnect = &pMessage->messageData.disconnection;

            /* Call the application PSM disconnection handler. */
            OtapClient_HandlePsmDisconnection (pCbDisconnect);

            break;
        }
        case gL2ca_NoPeerCredits_c:
        {
            l2caLeCbNoPeerCredits_t *pCbNoPeerCredits = &pMessage->messageData.noPeerCredits;
            (void)L2ca_SendLeCredit (pCbNoPeerCredits->deviceId,
                               otapClientData.l2capPsmChannelId,
                               mAppLeCbInitialCredits_c);
            break;
        }
        case gL2ca_LocalCreditsNotification_c:
        {
            //l2caLeCbLocalCreditsNotification_t *pMsg = &pMessage->messageData.localCreditsNotification;

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
#endif

/*! *********************************************************************************
* \brief        Handles the CCCD Written GATT event
*
* \param[in]    deviceId    The device ID of the connected peer
* \param[in]    handle      The handle of the written characteristic
* \param[in]    cccd        Flags for the value of the CCCD
********************************************************************************** */
void OtapClient_CccdWritten (deviceId_t deviceId, uint16_t handle, gattCccdFlags_t cccd)
{

    /*! Check if the OTAP control point CCCD was written. */
    if (handle == (uint16_t)cccd_otap_control_point)
    {
        otapClientData.serverWrittenCccd = TRUE;
        switch (otapClientData.state)
        {
        case mOtapClientStateDownloadingImage_c:
        case mOtapClientStateIdle_c:
            /*! If the state is Idle try to send a New Image Info Request Command to the OTAP Server. */
            OtapClient_NewImageInfoRequest(deviceId);
            break;

        case mOtapClientStateImageDownloadComplete_c:
            /*! Simply ignore the situation if the image download is complete. */
            break;

        default:
            /*! Ignore. */
            break;
        }
    }
}

/*! *********************************************************************************
* \brief        Handles the Attribute Written GATT event
*
* \param[in]    deviceId    The device ID of the connected peer
* \param[in]    handle      The handle of the written attribute
* \param[in]    length      Length of the value to be written
* \param[in]    pValue      Pointer to attribute's value
********************************************************************************** */
void OtapClient_AttributeWritten(deviceId_t  deviceId,
                                    uint16_t    handle,
                                    uint16_t    length,
                                    uint8_t*    pValue)
{
    union
    {
        uint8_t*                    pValue;
        otapCommand_t*              otapCommandTemp;
    }otapCommandVars;

    bleResult_t bleResult;
    otapCommand_t otapCommand;

    /* Only the OTAP Control Point attribute is expected to be written using the
     * ATT Write Command. */
    if (handle == (uint16_t)value_otap_control_point)
    {
        otapCommandVars.pValue = pValue;
        /*! Handle all OTAP Server to Client Commands Here. */
        switch((otapCommandVars.otapCommandTemp)->cmdId)
        {
        case gOtapCmdIdNewImageNotification_c:
            bleResult = GattServer_SendAttributeWrittenStatus (deviceId,
                                                               (uint16_t)value_otap_control_point,
                                                               (uint8_t)gAttErrCodeNoError_c);
            if (gBleSuccess_c == bleResult)
            {
                OtapClient_HandleNewImageNotification (deviceId,
                                                       length,
                                                       pValue);
            }
            else
            {
                /*! A BLE error has occurred - Disconnect */
                (void)Gap_Disconnect (deviceId);
            }
            break;
        case gOtapCmdIdNewImageInfoResponse_c:
            bleResult = GattServer_SendAttributeWrittenStatus (deviceId,
                                                               (uint16_t)value_otap_control_point,
                                                               (uint8_t)gAttErrCodeNoError_c);
            if (gBleSuccess_c == bleResult)
            {
                OtapClient_HandleNewImageInfoResponse (deviceId,
                                                       length,
                                                       pValue);
            }
            else
            {
                /*! A BLE error has occurred - Disconnect */
                (void)Gap_Disconnect (deviceId);
            }
            break;
        case gOtapCmdIdErrorNotification_c:
            bleResult = GattServer_SendAttributeWrittenStatus (deviceId,
                                                               (uint16_t)value_otap_control_point,
                                                               (uint8_t)gAttErrCodeNoError_c);
            if (gBleSuccess_c == bleResult)
            {
                OtapClient_HandleErrorNotification (deviceId,
                                                    length,
                                                    pValue);
            }
            else
            {
                /*! A BLE error has occurred - Disconnect */
                (void)Gap_Disconnect (deviceId);
            }
            break;

        case gOtapCmdIdStopImageTransfer_c:
            /* Server stopped the transfer */
            (void)GattServer_SendAttributeWrittenStatus (deviceId,
                                                               (uint16_t)value_otap_control_point,
                                                               (uint8_t)gAttErrCodeNoError_c);
            /* Stop timer. It will be started again when a new transfer starts. */
            (void)TM_Stop((timer_handle_t)mTransferStatusCheckTmrId);
            /* Cancel OTA transfer */
            OtapClient_CancelOTA(NULL);
            break;

        default:
            otapCommand.cmdId = gOtapCmdIdErrorNotification_c;
            otapCommand.cmd.errNotif.cmdId = pValue[0];
            otapCommand.cmd.errNotif.errStatus = gOtapStatusUnexpectedCommand_c;

            bleResult = OtapCS_SendCommandToOtapServer ((uint16_t)service_otap,
                                                        (void*)(&otapCommand),
                                                        cmdIdToCmdLengthTable[gOtapCmdIdErrorNotification_c]);
            if (gBleSuccess_c == bleResult)
            {
                otapClientData.lastCmdSentToOtapServer = gOtapCmdIdErrorNotification_c;
            }
            else
            {
                OtapClient_HandleServerCommError (deviceId);
            }
            break;
        };
    }
    else
    {
        /*! A GATT Server is trying to GATT Write an unknown attribute value.
         *  This should not happen. Disconnect the link. */
        (void)Gap_Disconnect (deviceId);
    }
}

/*! *********************************************************************************
* \brief        Handles the Attribute Written Without Response GATT event
*
* \param[in]    deviceId    The device ID of the connected peer
* \param[in]    handle      The handle of the written attribute
* \param[in]    length      Length of the value to be written
* \param[in]    pValue      Pointer to attribute's value
********************************************************************************** */
void OtapClient_AttributeWrittenWithoutResponse (deviceId_t deviceId,
                                                    uint16_t handle,
                                                    uint16_t length,
                                                    uint8_t* pValue)
{
    union
    {
        uint8_t*                    pValueTemp;
        otapCommand_t*              otapCommandTemp;
    }otapCommandVars;

    otapCommand_t otapCommand;
    otapStatus_t otapStatus = gOtapStatusSuccess_c;
    bleResult_t bleResult;

    /* Only the OTAP Data attribute is expected to be written using the
     * ATT Write Without Response Command. */
    if (handle == (uint16_t)value_otap_data)
    {
        if (otapClientData.state == mOtapClientStateDownloadingImage_c)
        {
            if (otapClientData.transferMethod == gOtapTransferMethodAtt_c)
            {
                otapCommandVars.pValueTemp = pValue;
                if ((otapCommandVars.otapCommandTemp)->cmdId == gOtapCmdIdImageChunk_c)
                {
                    OtapClient_HandleDataChunk (deviceId,
                                                length,
                                                pValue);
                }
                else
                {
                    /* If the OTAP Client received an unexpected command on the data channel send an error to the OTAP Server. */
                    otapStatus = gOtapStatusUnexpectedCmdOnDataChannel_c;
                }
            }
            else
            {
                /* If the OTAP Client is not expecting image file chunks via ATT send an error to the OTAP Server. */
                otapStatus = gOtapStatusUnexpectedTransferMethod_c;
            }
        }
        else
        {
            /* If the OTAP Client is not expecting image file chunks send an error to the OTAP Server. */
            otapStatus = gOtapStatusImageDataNotExpected_c;
        }

        if (otapStatus != gOtapStatusSuccess_c)
        {
            otapCommand.cmdId = gOtapCmdIdErrorNotification_c;
            otapCommand.cmd.errNotif.cmdId = pValue[0];
            otapCommand.cmd.errNotif.errStatus = otapStatus;

            bleResult = OtapCS_SendCommandToOtapServer ((uint16_t)service_otap,
                                                        (void*)(&otapCommand),
                                                        cmdIdToCmdLengthTable[gOtapCmdIdErrorNotification_c]);
            if (gBleSuccess_c == bleResult)
            {
                otapClientData.lastCmdSentToOtapServer = gOtapCmdIdErrorNotification_c;
            }
            else
            {
                OtapClient_HandleServerCommError (deviceId);
            }
        }
    }
}

/*! *********************************************************************************
* \brief        Handles the Value Confirmation GATT event
*
* \param[in]    deviceId    The device ID of the connected peer
********************************************************************************** */
void OtapClient_HandleValueConfirmation (deviceId_t deviceId)
{
    otapCommand_t otapCommand;
    bleResult_t   bleResult;

    /*! Check for which command sent to the OTAP Server the confirmation has been received. */
    switch (otapClientData.lastCmdSentToOtapServer)
    {
    case gOtapCmdIdNewImageInfoRequest_c:
        OtapClient_HandleNewImageInfoRequestConfirmation (deviceId);
        break;

    case gOtapCmdIdImageBlockRequest_c:
        OtapClient_HandleImageBlockRequestConfirmation (deviceId);
        break;

    case gOtapCmdIdImageTransferComplete_c:
        OtapClient_HandleImageTransferCompleteConfirmation (deviceId);
        break;

    case gOtapCmdIdErrorNotification_c:
        OtapClient_HandleErrorNotificationConfirmation (deviceId);
        break;

    case gOtapCmdIdStopImageTransfer_c:
        OtapClient_HandleStopImageTransferConfirmation (deviceId);
        break;

    default:
        otapCommand.cmdId = gOtapCmdIdErrorNotification_c;
        otapCommand.cmd.errNotif.cmdId = gOtapCmdIdNoCommand_c;
        otapCommand.cmd.errNotif.errStatus = gOtapStatusUnexpectedCommand_c;

        bleResult = OtapCS_SendCommandToOtapServer ((uint16_t)service_otap,
                                                    (void*)(&otapCommand),
                                                    cmdIdToCmdLengthTable[gOtapCmdIdErrorNotification_c]);
        if (gBleSuccess_c == bleResult)
        {
            otapClientData.lastCmdSentToOtapServer = gOtapCmdIdErrorNotification_c;
        }
        else
        {
            OtapClient_HandleServerCommError (deviceId);
        }
        break;
    };
}

#if defined(gOtapClientL2Cap_d) && (gOtapClientL2Cap_d == 1)
/*! *********************************************************************************
* \brief        Handles the LE PSM Connection Complete event
*
* \param[in]    pConnComplete   Pointer to Connection Complete event parameters
********************************************************************************** */
static void OtapClient_HandlePsmConnectionComplete (l2caLeCbConnectionComplete_t *pConnComplete)
{
    if (pConnComplete->result == gSuccessful_c)
    {
        otapClientData.l2capPsmConnected = TRUE;
        otapClientData.l2capPsmChannelId = pConnComplete->cId;

        if (pConnComplete->peerMtu > gOtap_l2capCmdMtuDataChunkOverhead_c)
        {
            otapClientData.negotiatedMaxL2CapChunkSize = pConnComplete->peerMtu - gOtap_l2capCmdMtuDataChunkOverhead_c;
        }

        /* If the connection is successful reset the image download parameters to safe values and
         * try to continue the image download. */
        otapClientData.chunkSize = 0;
        otapClientData.chunkSeqNum = 0;
        otapClientData.totalBlockChunks = 0;
        otapClientData.totalBlockSize = 0;
        OtapClient_ContinueImageDownload (pConnComplete->deviceId);
    }
    else
    {
        otapClientData.l2capPsmConnected = FALSE;
        /* If the connection failed try to reconnect. */
        (uint16_t)L2ca_ConnectLePsm (gOtap_L2capLePsm_c,
                           pConnComplete->deviceId,
                           mAppLeCbInitialCredits_c);
    }
}

/*! *********************************************************************************
* \brief        Handles the LE PSM Disconnection  event
*
* \param[in]    pConnComplete   Pointer to Disconnection event parameters
********************************************************************************** */
static void OtapClient_HandlePsmDisconnection (l2caLeCbDisconnection_t * pCbDisconnect)
{
    otapCommand_t otapCommand;
    bleResult_t   bleResult = gBleSuccess_c;

    otapClientData.l2capPsmConnected = FALSE;

    /* Stop the image transfer if a download is in progress
     * and try to reconnect. */
    if ((otapClientData.state == mOtapClientStateDownloadingImage_c) &&
        (otapClientData.transferMethod == gOtapTransferMethodL2capCoC_c))
    {
        otapCommand.cmdId = gOtapCmdIdStopImageTransfer_c;
        FLib_MemCpy (otapCommand.cmd.stopImgTransf.imageId,
                     (uint8_t*)otapClientData.imgId,
                     gOtap_ImageIdFieldSize_c);

        bleResult = OtapCS_SendCommandToOtapServer ((uint16_t)service_otap,
                                                    (void*)(&otapCommand),
                                                    cmdIdToCmdLengthTable[gOtapCmdIdStopImageTransfer_c]);
        if (gBleSuccess_c == bleResult)
        {
            otapClientData.lastCmdSentToOtapServer = gOtapCmdIdStopImageTransfer_c;
        }
        else
        {
            /*! A BLE error has occurred - Disconnect */
            (void)Gap_Disconnect (pCbDisconnect->deviceId);
        }
    }

    if (gBleSuccess_c == bleResult)
    {
        (void)L2ca_ConnectLePsm (gOtap_L2capLePsm_c,
                           pCbDisconnect->deviceId,
                           mAppLeCbInitialCredits_c);
    }
}
#endif

/*! *********************************************************************************
* \brief        Handles the Connection Complete event
*
* \param[in]    deviceId   Device ID of the connected peer
********************************************************************************** */
void OtapClient_HandleConnectionEvent (deviceId_t deviceId)
{
    switch (otapClientData.state)
    {
        case mOtapClientStateIdle_c:
        {
            /* If the OTAP Server has written the CCCD to receive commands from the OTAP Client then send a new image info request. */
            if (otapClientData.serverWrittenCccd == TRUE)
            {
                OtapClient_NewImageInfoRequest (deviceId);
            }
        }
        break;

        case  mOtapClientStateDownloadingImage_c:
        {
            /*! If the state is Downloading try to continue the download from where it was left off.
             *  Check if the appropriate server is connected first. */
            if (otapClientData.peerOtapServer == deviceId)
            {
                /* Reset block download parameters to safe values. */
                OtapClient_ResetBlockTransferParameters (deviceId);

                if (otapClientData.serverWrittenCccd == TRUE)
                {
                    OtapClient_ContinueImageDownload (deviceId);
                }
            }
        }
        break;

        case mOtapClientStateImageDownloadComplete_c:
        {
            /*! If the image download is complete try to set the new image flag
             *  and reset the MCU for the bootloader to kick in. */
            (void)Gap_Disconnect (deviceId);

            OTA_SetNewImageFlag ();
            ResetMCU ();
        }
        break;

        default:
        {
            /* Some kind of internal error has occurred. Reset the
             * client state to Idle and act as if the state was Idle. */
            otapClientData.state = mOtapClientStateIdle_c;
        }
        break;
    }
}

/*! *********************************************************************************
* \brief        Handles the Disconnection event
*
* \param[in]    deviceId   Device ID of the disconnected peer
********************************************************************************** */
void OtapClient_HandleDisconnectionEvent (deviceId_t deviceId)
{
    /* Check if the peer OTAP server was disconnected and if so reset block download
     * parameters to safe values. */
    if (otapClientData.peerOtapServer == deviceId)
    {
        OtapClient_ResetBlockTransferParameters (deviceId);
        otapClientData.peerOtapServer = gInvalidDeviceId_c;
    }

    otapClientData.serverWrittenCccd = FALSE;
#if defined(gOtapClientL2Cap_d) && (gOtapClientL2Cap_d == 1)
    otapClientData.l2capPsmConnected = FALSE;
#endif
    
    if( otapClientData.state != mOtapClientStateIdle_c)
    {  
        /* Stop timer. It will be started again when a new transfer starts. */
        (void)TM_Stop((timer_handle_t)mTransferStatusCheckTmrId);
        
        /* Start the timer that will clear the progress if no transfer starts in the next gOTAPCancelIntervalMs interval */
        (void)TM_InstallCallback((timer_handle_t)mTransferStatusCheckTmrId, OtapClient_CancelOTA, NULL);
        (void)TM_Start((timer_handle_t)mTransferStatusCheckTmrId, (uint8_t)kTimerModeSingleShot | (uint8_t)kTimerModeLowPowerTimer, gOTAPCancelIntervalMs);
    }
}

/*! *********************************************************************************
* \brief        Handles the Encryption Changed event
*
* \param[in]    deviceId   Device ID of the connected peer
********************************************************************************** */
void OtapClient_HandleEncryptionChangedEvent (deviceId_t deviceId)
{
    if ((otapClientData.state == mOtapClientStateIdle_c) &&
        (otapClientData.serverWrittenCccd == TRUE))
    {
         OtapClient_NewImageInfoRequest (deviceId);
    }
}
/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/
static void OtapClient_HandleDataChunk (deviceId_t deviceId, uint16_t length, uint8_t* pData)
{
    union
    {
        uint8_t*                    pDataTemp;
        otapCommand_t*              otapCommandTemp;
    }otapCommandVars;

    otapCommand_t   otapCommand;
    bleResult_t     bleResult;
    otapStatus_t    otapStatus = gOtapStatusSuccess_c;

    otapCommandVars.pDataTemp = pData;
    otapCmdImgChunkCoc_t* pDataChunk = &(otapCommandVars.otapCommandTemp->cmd.imgChunkCoc); //use the CoC Data Chunk type but observe the length
    uint16_t dataLen = length - gOtap_CmdIdFieldSize_c - gOtap_ChunkSeqNumberSize_c; // len

    /* Variables for the local image file parsing state machine. */
    static uint32_t currentImgElemRcvdLen = 0; /*!< Contains the number of received bytes for the current image element (header or other sub element).
                                                         *   This is needed because the */
    static bleOtaImageFileHeader_t imgFileHeader;   /*!< Saved image file header. */
    static uint32_t elementEnd = 0;                 /*!< Current image file element expected end. */
    static subElementHeader_t subElemHdr;

    otapStatus = OtapClient_ValidateOtapCommand(deviceId, length, pDataChunk, dataLen);

    /*! If all checks were successful then parse the current data chunk, else send an error notification. */
    if (otapStatus == gOtapStatusSuccess_c)
    {
        pData = (uint8_t*)(&pDataChunk->data);

        /* If the Current position is 0 then reset the received length for the current image element
         * and the current image CRC to the initialization value which is 0.
         * The current position should be 0 only at the start of the image file transfer. */
        if (otapClientData.currentPos == 0U)
        {
            currentImgElemRcvdLen = 0;
            otapClientData.imgComputedCrc = 0;
        }

        /* Parse all the bytes in the data payload. */
        while (dataLen > 0U)
        {
            /* Wait for the header to arrive and check it's contents
             * then handle the elements of the image. */
            if (otapClientData.currentPos < sizeof(bleOtaImageFileHeader_t))
            {
                otapStatus = OtapClient_ReceiveOtaImageFileHeader(&dataLen,
                                                                  &pData,
                                                                  &imgFileHeader,
                                                                  &currentImgElemRcvdLen);

                if(otapStatus != gOtapStatusSuccess_c)
                {
                    break;
                }
            }
            else
            {
                /* The parsing has reached the sub-elements portion of the image.
                 * Wait for each sub-element tag to arrive or parse it if it is known. */
                if (currentImgElemRcvdLen < sizeof(subElementHeader_t))
                {
                    OtapClient_ReceiveSubElementHeader(&dataLen,
                                                       &pData,
                                                       &subElemHdr,
                                                       &currentImgElemRcvdLen,
                                                       &elementEnd);
                }
                else
                {
                    uint32_t    elementChunkLength = 0;

                    /* Make sure we do not pass the current element boundary. */
                    if ((otapClientData.currentPos + dataLen) >= elementEnd)
                    {
                        elementChunkLength = elementEnd - otapClientData.currentPos;
                    }
                    else
                    {
                        elementChunkLength = dataLen;
                    }

                    /* Handle sub-element payload. */
                    otapStatus = OtapClient_HandleSubElementPayload(pData,
                                                                    elementChunkLength,
                                                                    &subElemHdr,
                                                                    currentImgElemRcvdLen);

                    if (otapStatus != gOtapStatusSuccess_c)
                    {
                        /* If an error has occurred then break the loop. */
                        break;
                    }

                    otapClientData.currentPos += elementChunkLength;
                    currentImgElemRcvdLen += elementChunkLength;
                    pData += elementChunkLength;
                    dataLen -= (uint16_t)elementChunkLength;

                    /* If this element has been completely received then reset the current element
                     * received length to trigger the reception of the next sub-element. */
                    if (otapClientData.currentPos >= elementEnd)
                    {
                        currentImgElemRcvdLen = 0;
                    }
                }
            }
        } /* while (dataLen) */
    }

    if (otapStatus == gOtapStatusSuccess_c)
    {
        /* If the chunk has been successfully processed increase the expected sequence number. */
        otapClientData.chunkSeqNum += 1U;

        /* Check if the block and/or image transfer is complete */
        if (otapClientData.chunkSeqNum >= otapClientData.totalBlockChunks)
        {
            /* If the image transfer is complete check the image CRC then
             * commit the image and set the bootloader flags. */
            if (otapClientData.currentPos >= otapClientData.imgSize)
            {
                if (otapClientData.imgComputedCrc != otapClientData.imgReceivedCrc)
                {
                    otapStatus = gOtapStatusInvalidImageCrc_c;
                    otapClientData.currentPos = 0;
                    OTA_CancelImage();
                }

                if (gOtaSuccess_c != OTA_CommitImage(otapClientData.imgSectorBitmap))
                {
                    otapStatus = gOtapStatusImageStorageError_c;
                    otapClientData.currentPos = 0;
                    OTA_CancelImage();
                }
                else
                {
                    /* The new image was successfully committed. Send an image transfer complete
                     * message to the peer. */
                    otapClientData.state = mOtapClientStateImageDownloadComplete_c;

                    otapCommand.cmdId = gOtapCmdIdImageTransferComplete_c;
                    FLib_MemCpy((uint8_t*)otapCommand.cmd.imgTransComplete.imageId, otapClientData.imgId, sizeof(otapCommand.cmd.imgTransComplete.imageId));
                    otapCommand.cmd.imgTransComplete.status = gOtapStatusSuccess_c;

                    bleResult = OtapCS_SendCommandToOtapServer ((uint16_t)service_otap,
                                                                (void*)(&otapCommand),
                                                                cmdIdToCmdLengthTable[gOtapCmdIdImageTransferComplete_c]);
                    if (gBleSuccess_c == bleResult)
                    {
                        otapClientData.lastCmdSentToOtapServer = gOtapCmdIdImageTransferComplete_c;
                    }
                    else
                    {
                        /*! A BLE error has occurred - Trigger the bootloader and reset now.
                         *  Do not wait for the Image Transfer Complete Confirmation. */
                        OtapClient_HandleServerCommError (deviceId);
                        OTA_SetNewImageFlag ();
                        ResetMCU ();
                    }
                }
            }
            else
            {
                /* If just the current block is complete ask for another block. */
                OtapClient_ContinueImageDownload (deviceId);
            }
        }
    }

    if (otapStatus != gOtapStatusSuccess_c)
    {
        otapCommand.cmdId = gOtapCmdIdErrorNotification_c;
        otapCommand.cmd.errNotif.cmdId = gOtapCmdIdImageChunk_c;
        otapCommand.cmd.errNotif.errStatus = otapStatus;

        bleResult = OtapCS_SendCommandToOtapServer ((uint16_t)service_otap,
                                                    (void*)(&otapCommand),
                                                    cmdIdToCmdLengthTable[gOtapCmdIdErrorNotification_c]);

        if (gBleSuccess_c == bleResult)
        {
            otapClientData.lastCmdSentToOtapServer = gOtapCmdIdErrorNotification_c;
        }
        else
        {
            OtapClient_HandleServerCommError (deviceId);
        }
    }
}

static void OtapClient_HandleNewImageNotification (deviceId_t deviceId, uint16_t length, uint8_t* pValue)
{
    union
    {
        uint8_t*                            pValueTemp;
        otapCommand_t*                      otapCommandTemp;
    }otapCommandVars;

    otapCommand_t otapCommand;
    bleResult_t   bleResult;
    otapStatus_t otapStatus = gOtapStatusSuccess_c;
    otapCommandVars.pValueTemp = pValue;
    otapCommand_t*  pRemoteCmd = otapCommandVars.otapCommandTemp;

    /* Check the command length and parameters. */
    if (length != cmdIdToCmdLengthTable[gOtapCmdIdNewImageNotification_c])
    {
        otapStatus = gOtapStatusInvalidCommandLength_c;
    }
    else if (pRemoteCmd->cmd.newImgNotif.imageFileSize <= (sizeof(bleOtaImageFileHeader_t) + sizeof(subElementHeader_t)))
    {
        otapStatus = gOtapStatusInvalidImageFileSize_c;
    }
    else
    {
        switch (otapClientData.state)
        {
        case mOtapClientStateIdle_c:
            if (OtapClient_IsRemoteImageNewer(pRemoteCmd->cmd.newImgNotif.imageId, pRemoteCmd->cmd.newImgNotif.imageVersion))
            {
                /* If a response for a New Image Info Request is expected from the OTAP Server simply ignore the
                 * New Image Notification. */
                if (otapClientData.lastCmdSentToOtapServer != gOtapCmdIdNewImageInfoRequest_c)
                {
                    /* Set up the Client to receive the image file. */
                    if (otapClientData.peerOtapServer == gInvalidDeviceId_c)
                    {
                       otapClientData.peerOtapServer = deviceId;
                    }
                    FLib_MemCpy(otapClientData.imgId, pRemoteCmd->cmd.newImgNotif.imageId, gOtap_ImageIdFieldSize_c);
                    FLib_MemCpy(otapClientData.imgVer, pRemoteCmd->cmd.newImgNotif.imageVersion, gOtap_ImageVersionFieldSize_c);
                    otapClientData.imgSize = pRemoteCmd->cmd.newImgNotif.imageFileSize;
                    otapClientData.currentPos = 0;
                    OtapClient_ResetBlockTransferParameters (deviceId);

                    /* Change the Client state to Downloading and trigger the download. */
                    otapClientData.state = mOtapClientStateDownloadingImage_c;
                    OtapClient_ContinueImageDownload (deviceId);
                }
            }
            /* If the remote image is not newer than the current image simply ignore the New Image Notification */
            break;

        case mOtapClientStateDownloadingImage_c:
            /*! Check if the image is the one currently being downloaded and if it is continue the download,
             *  else if the image is newer than the current one being downloaded then restart the whole download process. */
            if ((FLib_MemCmp(otapClientData.imgId, pRemoteCmd->cmd.newImgNotif.imageId, gOtap_ImageIdFieldSize_c)) &&
                (FLib_MemCmp(otapClientData.imgVer, pRemoteCmd->cmd.newImgNotif.imageVersion, gOtap_ImageVersionFieldSize_c))
               )
            {
                OtapClient_ResetBlockTransferParameters (deviceId);
                OtapClient_ContinueImageDownload (deviceId);
            }
            else if (OtapClient_IsRemoteImageNewer(pRemoteCmd->cmd.newImgNotif.imageId, pRemoteCmd->cmd.newImgNotif.imageVersion))
            {
                /*! A newer image than the one being downloaded is available, restart the download with the new image. */
                if (otapClientData.peerOtapServer == gInvalidDeviceId_c)
                {
                   otapClientData.peerOtapServer = deviceId;
                }
                FLib_MemCpy(otapClientData.imgId, pRemoteCmd->cmd.newImgNotif.imageId, gOtap_ImageIdFieldSize_c);
                FLib_MemCpy(otapClientData.imgVer, pRemoteCmd->cmd.newImgNotif.imageVersion, gOtap_ImageVersionFieldSize_c);
                otapClientData.imgSize = pRemoteCmd->cmd.newImgNotif.imageFileSize;
                otapClientData.currentPos = 0;
                OtapClient_ResetBlockTransferParameters (deviceId);

                OtapClient_ContinueImageDownload (deviceId);
            }
            else
            {
                ; /* For MISRA compliance */
            }
            break;
        case mOtapClientStateImageDownloadComplete_c:
            /* Simply ignore the message if an image is being downloaded or
             * an image download is complete. */
            break;

        default:
            /* Some kind of internal error has occurred. Reset the
             * client state to Idle and act as if the state was Idle. */
            otapClientData.state = mOtapClientStateIdle_c;
            if (OtapClient_IsRemoteImageNewer(pRemoteCmd->cmd.newImgNotif.imageId, pRemoteCmd->cmd.newImgNotif.imageVersion))
            {
                /* If a response for a New Image Info Request is expected from the OTAp Server simply ignore the
                 * New Image Notification. */
                if (otapClientData.lastCmdSentToOtapServer != gOtapCmdIdNewImageInfoRequest_c)
                {
                    /* Set up the Client to receive the image file. */
                    if (otapClientData.peerOtapServer == gInvalidDeviceId_c)
                    {
                       otapClientData.peerOtapServer = deviceId;
                    }
                    FLib_MemCpy(otapClientData.imgId, pRemoteCmd->cmd.newImgNotif.imageId, gOtap_ImageIdFieldSize_c);
                    FLib_MemCpy(otapClientData.imgVer, pRemoteCmd->cmd.newImgNotif.imageVersion, gOtap_ImageVersionFieldSize_c);
                    otapClientData.imgSize = pRemoteCmd->cmd.newImgNotif.imageFileSize;
                    otapClientData.currentPos = 0;
                    OtapClient_ResetBlockTransferParameters (deviceId);

                    /* Change the Client state to Downloading and trigger the download. */
                    otapClientData.state = mOtapClientStateDownloadingImage_c;
                    OtapClient_ContinueImageDownload (deviceId);
                }
            }
            /* If the remote image is not newer than the current image simply ignore the New Image Notification */
            break;
        };
    }

    if (otapStatus != gOtapStatusSuccess_c)
    {
        otapCommand.cmdId = gOtapCmdIdErrorNotification_c;
        otapCommand.cmd.errNotif.cmdId = pValue[0];
        otapCommand.cmd.errNotif.errStatus = otapStatus;

        bleResult = OtapCS_SendCommandToOtapServer ((uint16_t)service_otap,
                                                    (void*)(&otapCommand),
                                                    cmdIdToCmdLengthTable[gOtapCmdIdErrorNotification_c]);
        if (gBleSuccess_c == bleResult)
        {
            otapClientData.lastCmdSentToOtapServer = gOtapCmdIdErrorNotification_c;
        }
        else
        {
            OtapClient_HandleServerCommError (deviceId);
        }
    }
}

static void OtapClient_HandleNewImageInfoResponse (deviceId_t deviceId, uint16_t length, uint8_t* pValue)
{
    union
    {
        uint8_t*                        pValueTemp;
        otapCommand_t*                  otapCommandTemp;
    }otapCommandVars;

    otapCommand_t otapCommand;
    bleResult_t   bleResult;
    otapStatus_t otapStatus = gOtapStatusSuccess_c;
    otapCommandVars.pValueTemp = pValue;
    otapCommand_t*  pRemoteCmd = otapCommandVars.otapCommandTemp;

    /* Check the command length and parameters. */
    if (length != cmdIdToCmdLengthTable[gOtapCmdIdNewImageInfoResponse_c])
    {
        otapStatus = gOtapStatusInvalidCommandLength_c;
    }
    else if (pRemoteCmd->cmd.newImgInfoRes.imageFileSize <= (sizeof(bleOtaImageFileHeader_t) + sizeof(subElementHeader_t)))
    {
        otapStatus = gOtapStatusInvalidImageFileSize_c;
    }
    else
    {
        switch (otapClientData.state)
        {
        case mOtapClientStateIdle_c:
            if (OtapClient_IsRemoteImageNewer(pRemoteCmd->cmd.newImgInfoRes.imageId, pRemoteCmd->cmd.newImgInfoRes.imageVersion))
            {
                /* Set up the Client to receive the image file. */
                if (otapClientData.peerOtapServer == gInvalidDeviceId_c)
                {
                   otapClientData.peerOtapServer = deviceId;
                }
                FLib_MemCpy(otapClientData.imgId, pRemoteCmd->cmd.newImgInfoRes.imageId, gOtap_ImageIdFieldSize_c);
                FLib_MemCpy(otapClientData.imgVer, pRemoteCmd->cmd.newImgInfoRes.imageVersion, gOtap_ImageVersionFieldSize_c);
                otapClientData.imgSize = pRemoteCmd->cmd.newImgInfoRes.imageFileSize;
                otapClientData.currentPos = 0;
                OtapClient_ResetBlockTransferParameters (deviceId);

                /* Change the Client state to Downloading and trigger the download. */
                otapClientData.state = mOtapClientStateDownloadingImage_c;
                OtapClient_ContinueImageDownload (deviceId);
            }
            /* If the remote image is not newer than the current image simply ignore the New Image Info Response */
            break;

        case mOtapClientStateDownloadingImage_c:
            /*! Check if the image is the one currently being downloaded and if it is continue the download,
             *  else if the image is newer than the current one being downloaded then restart the whole download process. */
            if (otapClientData.peerOtapServer == gInvalidDeviceId_c)
            {
               otapClientData.peerOtapServer = deviceId;
            }

            if ((FLib_MemCmp(otapClientData.imgId, pRemoteCmd->cmd.newImgNotif.imageId, gOtap_ImageIdFieldSize_c)) &&
                (FLib_MemCmp(otapClientData.imgVer, pRemoteCmd->cmd.newImgNotif.imageVersion, gOtap_ImageVersionFieldSize_c))
               )
            {
                OtapClient_ResetBlockTransferParameters (deviceId);
                OtapClient_ContinueImageDownload (deviceId);
            }
            else if (OtapClient_IsRemoteImageNewer(pRemoteCmd->cmd.newImgNotif.imageId, pRemoteCmd->cmd.newImgNotif.imageVersion))
            {
                /*! A newer image than the one being downloaded is available, restart the download with the new image. */

                FLib_MemCpy(otapClientData.imgId, pRemoteCmd->cmd.newImgNotif.imageId, gOtap_ImageIdFieldSize_c);
                FLib_MemCpy(otapClientData.imgVer, pRemoteCmd->cmd.newImgNotif.imageVersion, gOtap_ImageVersionFieldSize_c);
                otapClientData.imgSize = pRemoteCmd->cmd.newImgNotif.imageFileSize;
                otapClientData.currentPos = 0;
                OtapClient_ResetBlockTransferParameters (deviceId);

                OtapClient_ContinueImageDownload (deviceId);
            }
            else
            {
                ; /* For MISRA compliance */
            }
            break;

        case mOtapClientStateImageDownloadComplete_c:
            /* Simply ignore the message if an image is being downloaded or
             * an image download is complete. */
            break;

        default:
            /* Some kind of internal error has occurred. Reset the
             * client state to Idle and act as if the state was Idle. */
            otapClientData.state = mOtapClientStateIdle_c;
            if (OtapClient_IsRemoteImageNewer(pRemoteCmd->cmd.newImgInfoRes.imageId, pRemoteCmd->cmd.newImgInfoRes.imageVersion))
            {
                /* Set up the Client to receive the image file. */
                if (otapClientData.peerOtapServer == gInvalidDeviceId_c)
                {
                   otapClientData.peerOtapServer = deviceId;
                }
                FLib_MemCpy(otapClientData.imgId, pRemoteCmd->cmd.newImgInfoRes.imageId, gOtap_ImageIdFieldSize_c);
                FLib_MemCpy(otapClientData.imgVer, pRemoteCmd->cmd.newImgInfoRes.imageVersion, gOtap_ImageVersionFieldSize_c);
                otapClientData.imgSize = pRemoteCmd->cmd.newImgInfoRes.imageFileSize;
                otapClientData.currentPos = 0;
                OtapClient_ResetBlockTransferParameters (deviceId);

                /* Change the Client state to Downloading and trigger the download. */
                otapClientData.state = mOtapClientStateDownloadingImage_c;
                OtapClient_ContinueImageDownload (deviceId);
            }
            /* If the remote image is not newer than the current image simply ignore the New Image Info Response */
            break;
        };
    }

    if (otapStatus != gOtapStatusSuccess_c)
    {
        otapCommand.cmdId = gOtapCmdIdErrorNotification_c;
        otapCommand.cmd.errNotif.cmdId = gOtapCmdIdNewImageInfoResponse_c;
        otapCommand.cmd.errNotif.errStatus = otapStatus;

        bleResult = OtapCS_SendCommandToOtapServer ((uint16_t)service_otap,
                                                    (void*)(&otapCommand),
                                                    cmdIdToCmdLengthTable[gOtapCmdIdErrorNotification_c]);
        if (gBleSuccess_c == bleResult)
        {
            otapClientData.lastCmdSentToOtapServer = gOtapCmdIdErrorNotification_c;
        }
        else
        {
            OtapClient_HandleServerCommError (deviceId);
        }
    }
}

static void OtapClient_HandleErrorNotification (deviceId_t deviceId, uint16_t length, uint8_t* pValue)
{
    union
    {
        uint8_t*                    pValueTemp;
        otapCommand_t*              otapCommandTemp;
    }otapCommandVars;

    otapCommand_t otapCommand;
    bleResult_t   bleResult;
    otapStatus_t otapStatus = gOtapStatusSuccess_c;
    otapCommandVars.pValueTemp = pValue;
    otapCommand_t*  pRemoteCmd = otapCommandVars.otapCommandTemp;

    /* Check the command length and parameters. */
    if (length == cmdIdToCmdLengthTable[gOtapCmdIdErrorNotification_c])
    {
        /*! Handle remote error statuses here. */
        if (pRemoteCmd->cmd.errNotif.errStatus < gOtapNumberOfStatuses_c)
        {
            /* Handle all errors in the same way, disconnect to restart the download process. */
            (void)Gap_Disconnect (deviceId);
        }
        else
        {
            otapStatus = gOtapStatusInvalidCommandParameter_c;
        }
    }
    else
    {
        otapStatus = gOtapStatusInvalidCommandLength_c;
    }

    if (otapStatus != gOtapStatusSuccess_c)
    {
        otapCommand.cmdId = gOtapCmdIdErrorNotification_c;
        otapCommand.cmd.errNotif.cmdId = gOtapCmdIdNewImageInfoResponse_c;
        otapCommand.cmd.errNotif.errStatus = otapStatus;

        bleResult = OtapCS_SendCommandToOtapServer ((uint16_t)service_otap,
                                                    (void*)(&otapCommand),
                                                    cmdIdToCmdLengthTable[gOtapCmdIdErrorNotification_c]);
        if (gBleSuccess_c == bleResult)
        {
            otapClientData.lastCmdSentToOtapServer = gOtapCmdIdErrorNotification_c;
        }
        else
        {
            OtapClient_HandleServerCommError (deviceId);
        }
    }
}

static void OtapClient_HandleNewImageInfoRequestConfirmation (deviceId_t deviceId)
{
    /* Clear the last command sent to the OTAP Server for which a Confirmation is expected. */
    otapClientData.lastCmdSentToOtapServer = gOtapCmdIdNoCommand_c;

    /* Nothing more to do here. If the New Image Info Request Command has reached
     * the OTAP Server then the OTAP Client expects a New Image Info Response */
}

static void OtapClient_HandleImageBlockRequestConfirmation (deviceId_t deviceId)
{
    /* Clear the last command sent to the OTAP Server for which a Confirmation is expected. */
    otapClientData.lastCmdSentToOtapServer = gOtapCmdIdNoCommand_c;

    /* Nothing more to do here. If the Image Block Request Command has reached
     * the OTAP Server then the OTAP Client expects the requested image chunks
     * or an Error Notification. */
}

static void OtapClient_HandleImageTransferCompleteConfirmation (deviceId_t deviceId)
{
    /* Clear the last command sent to the OTAP Server for which a Confirmation is expected. */
    otapClientData.lastCmdSentToOtapServer = gOtapCmdIdNoCommand_c;

    /* If the image transfer was not successful then the image download state should be Idle.
     * If it is, try to trigger a new download.
     * If the Image Transfer Complete Command has reached the OTAP Server and the transfer was successful
     * then the OTAP Client will just wait for the restart and the
     * bootloader to flash the new image. */
    if (otapClientData.state == mOtapClientStateIdle_c)
    {
        OtapClient_NewImageInfoRequest(deviceId);
    }
    else if (otapClientData.state == mOtapClientStateImageDownloadComplete_c)
    {
        /* If the image transfer is complete trigger the bootloader and reset the device. */
        (void)Gap_Disconnect (deviceId);
        OTA_SetNewImageFlag ();
        ResetMCU();
    }
    else
    {
        ; /* For MISRA compliance */
    }
}

static void OtapClient_HandleErrorNotificationConfirmation (deviceId_t deviceId)
{
    /* Clear the last command sent to the OTAP Server for which a Confirmation is expected. */
    otapClientData.lastCmdSentToOtapServer = gOtapCmdIdNoCommand_c;

    /* Reset block download parameters to safe values. */
    OtapClient_ResetBlockTransferParameters (deviceId);

    /* If an error has occurred try to continue the image download from a "safe" point. */
    OtapClient_ContinueImageDownload (deviceId);
}

static void OtapClient_HandleStopImageTransferConfirmation (deviceId_t deviceId)
{
    /* Clear the last command sent to the OTAP Server for which a Confirmation is expected. */
    otapClientData.lastCmdSentToOtapServer = gOtapCmdIdNoCommand_c;

    /* Reset block download parameters to safe values. */
    OtapClient_ResetBlockTransferParameters (deviceId);

    /* If an error has occurred try to continue the image download from a "safe" point. */
    OtapClient_ContinueImageDownload (deviceId);
}

static void OtapClient_ContinueImageDownload (deviceId_t deviceId)
{
    bool_t bValidState = TRUE;

    /* If the Server did not write the CCCD and the image is being downloaded exit immediately.
     * No commands can be exchanged before the CCCD is written. */
    if ((otapClientData.serverWrittenCccd == FALSE) &&
        (otapClientData.state == mOtapClientStateDownloadingImage_c))
    {
        bValidState = FALSE;
    }
#if defined(gOtapClientL2Cap_d) && (gOtapClientL2Cap_d == 1)
    /* Check if the L2CAP OTAP PSM is not connected and the image download is not complete.
     * If yes try to connect and exit immediately. */
    if ((otapClientData.l2capPsmConnected == FALSE) &&
        (otapClientData.state != mOtapClientStateImageDownloadComplete_c))
    {
        (void)L2ca_ConnectLePsm (gOtap_L2capLePsm_c,
                           deviceId,
                           mAppLeCbInitialCredits_c);
        bValidState = FALSE;
    }
#endif

    if (bValidState)
    {
        switch (otapClientData.state)
        {
        case mOtapClientStateIdle_c:
            /* If the state is Idle do nothing. No need to continue the transfer of an image. */
            break;
        case mOtapClientStateDownloadingImage_c:
            /* If the last received chunk sequence number is equal to the total block
             * chunks or they are both zero then ask for a new block from the server. */
            if (otapClientData.chunkSeqNum == otapClientData.totalBlockChunks)
            {
                (void)OtapClient_SendImageRequest();
            }
            break;
        case mOtapClientStateImageDownloadComplete_c:
            /* Stop timer */
            (void)TM_Stop((timer_handle_t)mTransferStatusCheckTmrId);

            /*! If the image download is complete try to set the new image flag
             *  and reset the MCU for the bootloader to kick in. */
            (void)Gap_Disconnect (deviceId);
            OTA_SetNewImageFlag ();
            ResetMCU ();
            break;
        default:
            /* This code should never be reached in normal running conditions.
            Do nothing here, no need to continue the transfer of an image. */
            break;
        };
    }
}

static void OtapClient_ResetBlockTransferParameters (deviceId_t deviceId)
{
    otapClientData.chunkSize = 0;
    otapClientData.chunkSeqNum = 0;
    otapClientData.totalBlockChunks = 0;
    otapClientData.totalBlockSize = 0;
}

static void OtapClient_HandleServerCommError (deviceId_t deviceId)
{
    /* Disconnect to trigger communication reset. */
    (void)Gap_Disconnect (deviceId);
}

static bool_t OtapClient_IsRemoteImageNewer (uint8_t* pRemoteImgId, uint8_t* pRemoteImgVer)
{
    uint32_t    remoteBuildVer;
    uint32_t    localeBuildVer;
    bool_t      bIsNewer = TRUE;

    /* Ignore the Image Id for the moment. */
    /* Check the Manufacturer Id */
    if (pRemoteImgVer[7] != otapClientData.currentImgVer[7])
    {
        bIsNewer = FALSE;
    }

    /* Check Hardware Id */
    else if (!FLib_MemCmp((void*)(&(pRemoteImgVer[4])), (const void*)(&(otapClientData.currentImgVer[4])), 3))
    {
        bIsNewer = FALSE;
    }

    /* Check Stack Version */
    else if (pRemoteImgVer[3] < otapClientData.currentImgVer[3])
    {
        bIsNewer = FALSE;
    }
    else
    {
        /* Check Build Version */
        remoteBuildVer = (uint32_t)pRemoteImgVer[0] + ((uint32_t)(pRemoteImgVer[1]) << 8) + ((uint32_t)(pRemoteImgVer[2]) << 16);
        localeBuildVer = (uint32_t)otapClientData.currentImgVer[0] + ((uint32_t)(otapClientData.currentImgVer[1]) << 8) + ((uint32_t)(otapClientData.currentImgVer[2]) << 16);
        if (remoteBuildVer <= localeBuildVer)
        {
            bIsNewer = FALSE;
        }
    }

    return bIsNewer;
}

static otapStatus_t OtapClient_IsImageFileHeaderValid (bleOtaImageFileHeader_t* imgFileHeader)
{
    otapStatus_t headerStatus;

    if (imgFileHeader->fileIdentifier != gBleOtaFileHeaderIdentifier_c)
    {
        headerStatus = gOtapStatusUnknownFileIdentifier_c;
    }

    else if (imgFileHeader->headerVersion != (uint16_t)gbleOtapHeaderVersion0100_c)
    {
        headerStatus = gOtapStatusUnknownHeaderVersion_c;
    }

    else if (imgFileHeader->headerLength != sizeof(bleOtaImageFileHeader_t))
    {
        headerStatus = gOtapStatusUnexpectedHeaderLength_c;
    }

    else if (imgFileHeader->fieldControl != gBleOtaFileHeaderDefaultFieldControl_c)
    {
        headerStatus = gOtapStatusUnexpectedHeaderFieldControl_c;
    }

    else if (imgFileHeader->companyId != gBleOtaCompanyIdentifier_c)
    {
        headerStatus = gOtapStatusUnknownCompanyId_c;
    }

    else if (FALSE == FLib_MemCmp (imgFileHeader->imageId, otapClientData.imgId, sizeof(imgFileHeader->imageId)))
    {
        headerStatus = gOtapStatusUnexpectedImageId_c;
    }

    else if (FALSE == FLib_MemCmp (imgFileHeader->imageVersion, otapClientData.imgVer, sizeof(imgFileHeader->imageVersion)))
    {
        headerStatus = gOtapStatusUnexpectedImageVersion_c;
    }

    else if (imgFileHeader->totalImageFileSize != otapClientData.imgSize)
    {
        headerStatus = gOtapStatusUnexpectedImageFileSize_c;
    }
    else
    {
        headerStatus = gOtapStatusSuccess_c;
    }

    return headerStatus;
}

static void OtapClient_SendImageBlockReq_cb(uint32_t param)
{
    otapCommand_t   otapCommand;
    bleResult_t     bleResult;
    NOT_USED(param);

    /* Send the Block request Command with the determined parameters. */
    otapCommand.cmdId = gOtapCmdIdImageBlockRequest_c;

    FLib_MemCpy(otapCommand.cmd.imgBlockReq.imageId, otapClientData.imgId, gOtap_ImageIdFieldSize_c);
    otapCommand.cmd.imgBlockReq.startPosition = otapClientData.currentPos;
    otapCommand.cmd.imgBlockReq.blockSize = otapClientData.totalBlockSize;
    otapCommand.cmd.imgBlockReq.chunkSize = otapClientData.chunkSize;
    otapCommand.cmd.imgBlockReq.transferMethod = (uint8_t)otapClientData.transferMethod;
    otapCommand.cmd.imgBlockReq.l2capChannelOrPsm = otapClientData.l2capChannelOrPsm;

    bleResult = OtapCS_SendCommandToOtapServer ((uint16_t)service_otap,
                                                (void*)(&otapCommand),
                                                cmdIdToCmdLengthTable[gOtapCmdIdImageBlockRequest_c]);
    if (gBleSuccess_c == bleResult)
    {
        otapClientData.lastCmdSentToOtapServer = gOtapCmdIdImageBlockRequest_c;
        (void)TM_InstallCallback((timer_handle_t)mTransferStatusCheckTmrId, OtapClient_TimerCb, NULL);
        (void)TM_Start((timer_handle_t)mTransferStatusCheckTmrId, kTimerModeLowPowerTimer, gOTAPTransferCheckIntervalMs);
    }
    else
    {
        OtapClient_HandleServerCommError (otapClientData.peerOtapServer);
    }
}

static bleResult_t OtapClient_SendImageRequest(void)
{
    otapCommand_t   otapCommand;
    bleResult_t     bleResult = gBleSuccess_c;
    uint32_t        bytesToDownload;
    uint32_t        maxBlockSize;

    do {
        /* Ask for another block only if the image transfer was not completed. */
        if (otapClientData.currentPos >= otapClientData.imgSize)
        {
            break;
        }
        bytesToDownload = otapClientData.imgSize - otapClientData.currentPos;

        if (otapClientData.transferMethod == gOtapTransferMethodAtt_c)
        {
            maxBlockSize = (uint32_t)otapClientData.negotiatedMaxAttChunkSize * gOtap_MaxChunksPerBlock_c;
            otapClientData.l2capChannelOrPsm = gL2capCidAtt_c;
            otapClientData.chunkSize = otapClientData.negotiatedMaxAttChunkSize;
        }
        else if (otapClientData.transferMethod == gOtapTransferMethodL2capCoC_c)
        {
            if (otapClientData.l2capChannelOrPsm == gOtap_L2capLePsm_c)
            {
                maxBlockSize = (uint32_t)otapClientData.negotiatedMaxL2CapChunkSize * gOtap_MaxChunksPerBlock_c;
                otapClientData.chunkSize = otapClientData.negotiatedMaxL2CapChunkSize;
            }
            else
            {
                /* If the L2CAP CoC is not valid then some kind of error or misconfiguration has
                 * occurred. Send a proper error notification to the peer and
                 * reset the download state machine to Idle. */
                otapClientData.state = mOtapClientStateIdle_c;

                otapCommand.cmdId = gOtapCmdIdErrorNotification_c;
                otapCommand.cmd.errNotif.cmdId = gOtapCmdIdNoCommand_c;
                otapCommand.cmd.errNotif.errStatus = gOtapStatusUnexpectedL2capChannelOrPsm_c;

                bleResult = OtapCS_SendCommandToOtapServer ((uint16_t)service_otap,
                                                            (void*)(&otapCommand),
                                                            cmdIdToCmdLengthTable[gOtapCmdIdErrorNotification_c]);
                if (gBleSuccess_c == bleResult)
                {
                    otapClientData.lastCmdSentToOtapServer = gOtapCmdIdErrorNotification_c;
                }
                else
                {
                    OtapClient_HandleServerCommError (otapClientData.peerOtapServer);
                }

                break;
            }
        }
        else
        {
            /* If the transfer method is not recognized then this image has been misconfigured
             * or a critical error has occurred. Send a proper error notification to the peer and
             * reset the download state machine to Idle. */
            otapClientData.state = mOtapClientStateIdle_c;

            otapCommand.cmdId = gOtapCmdIdErrorNotification_c;
            otapCommand.cmd.errNotif.cmdId = gOtapCmdIdNoCommand_c;
            otapCommand.cmd.errNotif.errStatus = gOtapStatusUnexpectedTransferMethod_c;

            bleResult = OtapCS_SendCommandToOtapServer ((uint16_t)service_otap,
                                                        (void*)(&otapCommand),
                                                        cmdIdToCmdLengthTable[gOtapCmdIdErrorNotification_c]);
            if (gBleSuccess_c == bleResult)
            {
                otapClientData.lastCmdSentToOtapServer = gOtapCmdIdErrorNotification_c;
            }
            else
            {
                OtapClient_HandleServerCommError (otapClientData.peerOtapServer);
            }

            break;
        }

        if (bytesToDownload >= maxBlockSize)
        {
            /* If there are more bytes to download than the maximum block size then
             * ask a full block from the server on the selected transfer method and set up
             * the client to receive the chunks.*/
            otapClientData.chunkSeqNum = 0;
            otapClientData.totalBlockChunks = gOtap_MaxChunksPerBlock_c;
            otapClientData.totalBlockSize = maxBlockSize;
        }
        else
        {
            /* If there are fewer bytes to download than the maximum block size then compute the
             *  number of chunks expected and set the expected block size to the number of
             *  bytes to download. */
            otapClientData.chunkSeqNum = 0;
            /* Compute number of full chunks. Integer division. */
            otapClientData.totalBlockChunks = (uint16_t)(bytesToDownload / otapClientData.chunkSize);
            /* Add an extra chunk if the chunk size is not a divisor of the number of bytes to download. */
            otapClientData.totalBlockChunks += (bytesToDownload % otapClientData.chunkSize) != 0U ? 1U : 0U;
            otapClientData.totalBlockSize = bytesToDownload;
        }
#if (gOtaErasePolicy_c == gOtaEraseBeforeImageBlockReq_c)
        /* In order to prevent interference of the erasure during the BLE OTA transfer
         * erase before sending the gOtapCmdIdImageBlockRequest_c to the server
         * Doing so provides a 'safe' flow control while waiting for the erasure to complete.
         * */
        if (bytesToDownload > 0U)
        {
            (void)OTA_MakeHeadRoomForNextBlock(otapClientData.totalBlockSize,
                                               OtapClient_SendImageBlockReq_cb,
                                               otapClientData.peerOtapServer);
        }
#else
        OtapClient_SendImageBlockReq_cb(otapClientData.peerOtapServer);
#endif

    } while (FALSE);

    return bleResult;
}

/*! *********************************************************************************
* \brief        Callback for periodic timer that checks the OTA transfer progress.
*
********************************************************************************** */
static void OtapClient_TimerCb(void * param)
{
    static uint32_t prevOffset = 0;
    static uint8_t mNbRetriesSent = 0U;

    if (otapClientData.currentPos != prevOffset)
    {
        /* OTA transfer is in progress. Update offset */
        prevOffset = otapClientData.currentPos;

        /* Reset retry number */
        if (mNbRetriesSent > 0U)
        {
            mNbRetriesSent = 0U;
        }
    }
    else
    {
        /* OTA transfer is stuck. Retry sending Image Info Request to Server */
        if (mNbRetriesSent < gOTAPMaxNbRetries)
        {
            bleResult_t bleResult = OtapClient_SendImageRequest();
            if (gBleSuccess_c == bleResult)
            {
                mNbRetriesSent++;
            }
        }
        else
        {
            /* Reset retries */
            mNbRetriesSent = 0U;

            /* Cancel OTA transfer */
            OtapClient_CancelOTA(NULL);

            /* Disconnect. Transfer parameters will be reset on disconnect event */
            (void)Gap_Disconnect(otapClientData.peerOtapServer);
        }
    }
}

/*! *********************************************************************************
* \brief        Resets the OTA transfer and cancels the writing of a new image
*
********************************************************************************** */
static void OtapClient_CancelOTA(void *param)
{
    /* Cancel OTA transfer */
    otapClientData.state = mOtapClientStateIdle_c;
    otapClientData.currentPos = 0;
    OTA_CancelImage();
}

static void OtapClient_NewImageInfoRequest(deviceId_t deviceId)
{
    otapCommand_t otapCommand;
    bleResult_t   bleResult;

    otapCommand.cmdId = gOtapCmdIdNewImageInfoRequest_c;
    FLib_MemCpy (otapCommand.cmd.newImgInfoReq.currentImageId,
                 (const uint8_t*)otapClientData.currentImgId,
                 gOtap_ImageIdFieldSize_c);
    FLib_MemCpy (otapCommand.cmd.newImgInfoReq.currentImageVersion,
                 (const uint8_t*)otapClientData.currentImgVer,
                 gOtap_ImageVersionFieldSize_c);

    bleResult = OtapCS_SendCommandToOtapServer ((uint16_t)service_otap,
                                                (void*)(&otapCommand),
                                                cmdIdToCmdLengthTable[gOtapCmdIdNewImageInfoRequest_c]);
    if (gBleSuccess_c == bleResult)
    {
        otapClientData.lastCmdSentToOtapServer = gOtapCmdIdNewImageInfoRequest_c;
    }
    else
    {
        OtapClient_HandleServerCommError (deviceId);
    }
}

static otapStatus_t OtapClient_ValidateOtapCommand
(
    deviceId_t                  deviceId,
    uint16_t                    commandLen,
    otapCmdImgChunkCoc_t        *pDataChunk,
    uint16_t                    dataLen
)
{
    otapStatus_t otapStatus = gOtapStatusSuccess_c;

    if (deviceId == otapClientData.peerOtapServer)
    {
        /* Check if the command length is as expected. */
        if ((commandLen > (gOtap_CmdIdFieldSize_c + gOtap_ChunkSeqNumberSize_c)) &&
            (((otapClientData.transferMethod == gOtapTransferMethodAtt_c) &&
              (commandLen <= gOtapCmdImageChunkAttMaxLength_c)) ||
             ((otapClientData.transferMethod == gOtapTransferMethodL2capCoC_c) &&
              (commandLen <= gOtapCmdImageChunkCocMaxLength_c))))
        {
            /* Check if the chunk (sequence number) is as expected */
            if ((pDataChunk->seqNumber == otapClientData.chunkSeqNum) &&
                (pDataChunk->seqNumber < otapClientData.totalBlockChunks))
            {
                /*  Check if the data length is as expected. */
                if (((dataLen == otapClientData.chunkSize) &&
                    ((pDataChunk->seqNumber < (otapClientData.totalBlockChunks - 1U)) ||
                     (otapClientData.totalBlockSize % otapClientData.chunkSize == 0U))) ||
                    ((dataLen < otapClientData.chunkSize) &&
                     (pDataChunk->seqNumber == (otapClientData.totalBlockChunks - 1U)) &&
                     (dataLen == otapClientData.totalBlockSize % otapClientData.chunkSize)))
                {
                    /* Do more checks here if necessary. */
                }
                else
                {
                    otapStatus = gOtapStatusUnexpectedDataLength_c;
                }
            }
            else
            {
                otapStatus = gOtapStatusUnexpectedSequenceNumber_c;
            }
        }
        else
        {
            otapStatus = gOtapStatusInvalidCommandLength_c;
        }
    }
    else
    {
        otapStatus = gOtapStatusUnexpectedOtapPeer_c;
    }

    return otapStatus;
}

static otapStatus_t OtapClient_ReceiveOtaImageFileHeader
(
    uint16_t                    *pDataLen,
    uint8_t                     **pData,
    bleOtaImageFileHeader_t     *pImageFileHeader,
    uint32_t                    *pCurrentImgElemRcvdLen
)
{
    otapStatus_t otapStatus = gOtapStatusSuccess_c;

    if ((otapClientData.currentPos + *pDataLen) >= sizeof(bleOtaImageFileHeader_t))
    {
        uint16_t residualHeaderLen = (uint16_t)(sizeof(bleOtaImageFileHeader_t) -
                                                otapClientData.currentPos);

        /* There is enough information in the data
         * payload to complete the header. */
        FLib_MemCpy ((uint8_t*)pImageFileHeader + otapClientData.currentPos,
                    *pData,
                    residualHeaderLen);
        otapClientData.currentPos += residualHeaderLen;
        *pData += residualHeaderLen;
        *pDataLen -= residualHeaderLen;

        /* Check header contents, and if it is not valid
         * return and error and reset the image download position. */
        otapStatus = OtapClient_IsImageFileHeaderValid (pImageFileHeader);
        if (otapStatus != gOtapStatusSuccess_c)
        {
            otapClientData.currentPos = 0;
        }
        else
        {
            /* If the header is valid then update the CRC
             * over the header part of the image. */
            otapClientData.imgComputedCrc = OTA_CrcCompute ((uint8_t*)pImageFileHeader,
                                                            (uint16_t)sizeof(bleOtaImageFileHeader_t),
                                                            otapClientData.imgComputedCrc);
            *pCurrentImgElemRcvdLen = 0;

            /* If the remaining data length is not 0 then the
             * loop will continue with the parsing of the next element. */
        }
    }
    else
    {
        /* Not enough data to complete the header.
         * Copy all the data into the temporary header and
         * increment the current image position. */
        FLib_MemCpy((uint8_t*)pImageFileHeader + otapClientData.currentPos,
                    *pData,
                    *pDataLen);
        otapClientData.currentPos += *pDataLen;
        *pDataLen = 0;
    }

    return otapStatus;
}

static void OtapClient_ReceiveSubElementHeader
(
    uint16_t                    *pDataLen,
    uint8_t                     **pData,
    subElementHeader_t          *pSubElemHdr,
    uint32_t                    *pCurrentImgElemRcvdLen,
    uint32_t                    *pElementEnd
)
{
    if ((*pCurrentImgElemRcvdLen + *pDataLen) >= sizeof(subElementHeader_t))
    {
        uint16_t residualSubElemHdrLen = (uint16_t)(sizeof(subElementHeader_t) -
                                                    *pCurrentImgElemRcvdLen);

        /* There is enough information in the data payload to
         * complete the sub-element header. */
        FLib_MemCpy ((uint8_t*)pSubElemHdr + *pCurrentImgElemRcvdLen,
                     *pData,
                     residualSubElemHdrLen);
        otapClientData.currentPos += residualSubElemHdrLen;
        *pCurrentImgElemRcvdLen += residualSubElemHdrLen;
        *pData += residualSubElemHdrLen;
        *pDataLen -= residualSubElemHdrLen;

        /* Update the CRC over the sub-element header only
         * if it is not the CRC Sub-Element header. */
        if (pSubElemHdr->tagId != (uint16_t)gBleOtaSubElemTagIdImageFileCrc_c)
        {
            otapClientData.imgComputedCrc = OTA_CrcCompute ((uint8_t*)pSubElemHdr,
                                                            (uint16_t)sizeof(subElementHeader_t),
                                                            otapClientData.imgComputedCrc);
        }

        *pElementEnd = otapClientData.currentPos + pSubElemHdr->dataLen;

        /* If the remaining data length is not 0 then the loop will
        continue with the parsing of the sub-element. */
    }
    else
    {
        /* Not enough data to complete the sub-element header.
         * Copy all the data into the temporary sub-element header
         * and increment the current image position. */
        FLib_MemCpy ((uint8_t*)pSubElemHdr + *pCurrentImgElemRcvdLen,
                     *pData,
                    *pDataLen);
        otapClientData.currentPos += *pDataLen;
        *pCurrentImgElemRcvdLen += *pDataLen;
        *pDataLen = 0;
    }
}

static otapStatus_t OtapClient_HandleSubElementPayload
(
    uint8_t             *pData,
    uint32_t            elementChunkLength,
    subElementHeader_t  *pSubElemHdr,
    uint32_t            currentImgElemRcvdLen
)
{
    otapStatus_t otapStatus = gOtapStatusSuccess_c;

    switch (pSubElemHdr->tagId)
    {
        case (uint16_t)gBleOtaSubElemTagIdUpgradeImage_c:
        {
            /* Immediately after receiving the header check
             * if the image sub-element length is valid
             * by trying to start the image upgrade procedure. */
            if (currentImgElemRcvdLen == sizeof(subElementHeader_t))
            {
                otaResult_t otaStartImageResult;
                otaStartImageResult = OTA_StartImage(pSubElemHdr->dataLen);
                if (gOtaSuccess_c != otaStartImageResult)
                {
                    if (otaStartImageResult == gOtaImageTooLarge_c)
                    {
                        /* The image is too large and does not fit into
                         * the non-volatile storage. */
                        otapStatus = gOtapStatusImageSizeTooLarge_c;
                    }
                    else
                    {
                        /* There was an error when trying to initiate the
                         * image storage in non-volatile memory */
                        otapStatus = gOtapStatusImageStorageError_c;
                    }
                    otapClientData.currentPos = 0;
                    break;
                }
            }
            /* Upgrade Image Tag - compute the CRC and try to push the
             * chunk to the storage. */
            otapClientData.imgComputedCrc = OTA_CrcCompute (pData,
                                                            (uint16_t)elementChunkLength,
                                                            otapClientData.imgComputedCrc);

            if (gOtaSuccess_c != OTA_PushImageChunk (pData,
                                                     (uint16_t)elementChunkLength,
                                                     NULL,
                                                     NULL))
            {
                otapStatus = gOtapStatusImageStorageError_c;
                otapClientData.currentPos = 0;
                OTA_CancelImage();
                break;
            }
            break;
        }

        case (uint16_t)gBleOtaSubElemTagIdSectorBitmap_c:
        {
            /* Immediately after receiving the header check if
             * the sub-element length is valid. */
            if (currentImgElemRcvdLen == sizeof(subElementHeader_t))
            {
                if (pSubElemHdr->dataLen != sizeof(otapClientData.imgSectorBitmap))
                {
                    /* The sub-element length is invalid, set an error
                     * status and reset the image file download process. */
                    otapStatus = gOtapStatusInvalidSubElementLength_c;
                    otapClientData.currentPos = 0;
                    OTA_CancelImage();
                    break;
                }
            }

            /* Sector Bitmap Tag - Compute the CRC and copy the
             * received bitmap to the buffer. */
            otapClientData.imgComputedCrc = OTA_CrcCompute (pData,
                                                            (uint16_t)elementChunkLength,
                                                            otapClientData.imgComputedCrc);

            FLib_MemCpy ((uint8_t*)otapClientData.imgSectorBitmap +
                         (currentImgElemRcvdLen - sizeof(subElementHeader_t)),
                         pData,
                         elementChunkLength);
            break;
        }

        case (uint16_t)gBleOtaSubElemTagIdImageFileCrc_c:
        {
            /* Immediately after receiving the header check
             * if the sub-element length is valid. */
            if (currentImgElemRcvdLen == sizeof(subElementHeader_t))
            {
                if (pSubElemHdr->dataLen != sizeof(otapClientData.imgReceivedCrc))
                {
                    /* The sub-element length is invalid, set an error
                     * status and reset the image file download process. */
                    otapStatus = gOtapStatusInvalidSubElementLength_c;
                    otapClientData.currentPos = 0;
                    OTA_CancelImage();
                    break;
                }
            }

            /* CRC Tag - Just copy the received CRC to the buffer. */
            FLib_MemCpy ((uint8_t*)(&otapClientData.imgReceivedCrc) +
                         (currentImgElemRcvdLen - sizeof(subElementHeader_t)),
                         pData,
                         elementChunkLength);
            break;
        }

        default:
        {
            /* Unknown sub-element type, just compute the CRC over it. */
            otapClientData.imgComputedCrc = OTA_CrcCompute (pData,
                                                            (uint16_t)elementChunkLength,
                                                            otapClientData.imgComputedCrc);
            break;
        }
    };

    return otapStatus;
}

/*! *********************************************************************************
* @}
********************************************************************************** */
