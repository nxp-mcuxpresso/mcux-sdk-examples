/*! *********************************************************************************
* \addtogroup ANCS Client
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright 2018-2024 NXP
*
* \file ancs_client.c
*
* This file is the source file for the ANCS Client application
*
* SPDX-License-Identifier: BSD-3-Clause
*/

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
#include "fsl_component_panic.h"
#include "fsl_format.h"
#include "fsl_component_mem_manager.h"
#include "fsl_component_timer_manager.h"
#include "FunctionLib.h"
#include "sensors.h"

/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"
#include "gatt_db_handles.h"

/* Profile / Services */
#include "battery_interface.h"
#include "device_info_interface.h"
#include "current_time_interface.h"
#include "reference_time_update_interface.h"
#include "next_dst_change_interface.h"
#include "ancs_interface.h"

#include "ble_conn_manager.h"
#include "ble_service_discovery.h"
#include "board.h"
#include "app.h"
#include "app_conn.h"
#include "ancs_client.h"

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
SHELL_HANDLE_DEFINE(g_shellHandle);

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define mBatteryLevelReportInterval_c           (10)            /* battery level report interval in seconds  */
#define mReadTimeCharInterval_c                 (3600U)         /* interval between time sync in seconds */
#define mCharReadBufferLength_c                 (13U)           /* length of the buffer */
#define mInitialTime_c                          (1451606400U)   /* initial timestamp - 01/01/2016 00:00:00 GMT */

#define mPhoneAllowNotificationsInterval_c      (3U)            /* time interval until the user presses the Allow Notifications button*/
#define mMaxDisplayNotifications_c              (16U)           /* The maximum number of notifications the application is
                                                                 * capable of displaying and managing. This will impact memory usage. */
#define mMaxNotifAppNameDisplayLength_c         (40U)           /* The maximum number of characters to use to display the name of the associated application obtained from the server. */
#define mMaxNotifCatDisplayLength_c             (20U)           /* The maximum number of characters to use to display a notification category. */
#define mMaxRemoteCommandDisplayLength_c        (20U)           /* The maximum number of characters to use to display a remote command. */

#define mAppEvt_PeerConnected_c                 (0U)
#define mAppEvt_PairingComplete_c               (1U)
#define mAppEvt_ServiceDiscoveryComplete_c      (2U)
#define mAppEvt_ServiceDiscoveryFailed_c        (3U)
#define mAppEvt_GattProcComplete_c              (4U)
#define mAppEvt_GattProcError_c                 (5U)
#define mAppEvt_AncsNsNotificationReceived_c    (6U)
#define mAppEvt_AncsDsNotificationReceived_c    (7U)
#define mAppEvt_AmsRcNotificationReceived_c     (8U)
#define mAppEvt_AmsEuNotificationReceived_c     (9U)
#define mAppIdentifierReservedSpace_c           (40U)           /* 40 bytes reserved space for the app identifier */
#define mAncsNotifUidFieldLengthOffset_c        (1U)            /* notification Uid field length offset*/
#define mAncsNotifAttrIdAppIdentifierOffset_c   (5U)            /* notification attribute id App identifier offset */
#define mAncsAppIdOffset_c                      (1U)            /* app id offset */
#define mNullTerminatorOffset_c                 (1U)
#define mAncsMaxWriteCCCDProcedures_c           (2U)            /* number of write procedures after discovery */

/* Shell */
static shell_status_t ShellPlay_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellPause_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellTogglePlayPause_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellNextTrack_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellPreviousTrack_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellVolumeUp_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellVolumeDown_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellAdvanceRepeatMode_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellAdvanceShuffleMode_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellSkipForward_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellSkipBackward_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellLikeTrack_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellDislikeTrack_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellBookmarkTrack_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
typedef union
{
    uint8_t        *pNsDataTemp;
    ancsNsPacket_t *pNsPacketTemp;
} pPayloadPacketTemp_t;

typedef union
{
    const char * pConstPrompt;
    char * pPrompt;
} shellPrompt_t;

typedef enum appState_tag
{
    mAppIdle_c,
    mAppExchangeMtu_c,
    mAppPrimaryServiceDisc_c,
    mAppCharServiceDisc_c,
    mAppNsDescriptorSetup_c,
    mAppDsDescriptorSetup_c,
    mAppRcDescriptorSetup_c,
    mAppEuDescriptorSetup_c,
    mAppEuConfigTrackSetup_c,
    mAppRunning_c,
} appState_t;

typedef enum
{
#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U)) &&\
    (defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1U)) &&\
    (defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0))
    fastFilterAcceptListAdvState_c,
#endif /* gAppUseBonding_d && gAppUsePrivacy_d && gBleEnableControllerPrivacy_d */
    fastAdvState_c,
    slowAdvState_c
} advType_t;

typedef struct advState_tag
{
    bool_t      advOn;
    advType_t   advType;
} advState_t;

typedef struct appCustomInfo_tag
{
    ancsClientConfig_t   ancsClientConfig;
    amsClientConfig_t    amsClientConfig;
    /* Add persistent information here */
} appCustomInfo_t;

typedef struct appPeerInfo_tag
{
    deviceId_t      deviceId;
    appCustomInfo_t customInfo;
    bool_t          isBonded;
    appState_t      appState;
} appPeerInfo_t;

/*! Structure type holding information about the notifications displayed on the ANCS Client. */
typedef struct notifInfo_tag
{
    bool_t          slotUsed;                                   /*!< Boolean signalling if the slot is in use or not. */
    ancsEventFlag_t notifFlags;                                 /*!< 0x01 = Silent = S
                                                                 *   0x02 = Important = I
                                                                 *   0x04 = Pre Existing = E
                                                                 *   0x08 = Positive Action = P
                                                                 *   0x10 = Negative Action = N
                                                                 *   Only the set flags are displayed. */
    ancsCatId_t     notifCat;                                   /*!< Notification category. Should be displayed as text. */
    uint32_t        notifUid;                                   /*!< Notification unique identifier. Should be displayed as is. */
    bool_t          appNameValid;                               /*!< Application name has been obtained from the ANCS Server and is valid. */
    uint8_t         appName[mMaxNotifAppNameDisplayLength_c];   /*!< Application displayed name. */
    bool_t          needGetNotifAttribute;                      /*!< This is a new notification for which Notification Attribute should be obtained */
    bool_t          needGetAppAttribute;                        /*!< This is a new notification for which App Attribute should be obtained */
    uint16_t        appIdLength;                                /* Needed for getting App Attribute from server */
    uint8_t         *pAppId;                                    /* Needed for getting App Attribute from server */

} notifInfo_t;

/*! Structures containing the ANCS/AMS functional data. */
typedef struct ancsClientAppData_tag
{
    notifInfo_t     notifications[mMaxDisplayNotifications_c];
    ancsComdId_t    lastCommandSentToAncsProvider;
    bool_t          notificationDataChanged;
} ancsClientAppData_t;

typedef struct amsClientAppData_tag
{
    uint16_t        amsAvailableCommandsBitmask;
    bool_t          amsInvalidCommand;
    uint8_t         euTrackArtist[gAttMaxDataSize_d(gAttMaxMtu_c)];
    uint8_t         euTrackAlbum[gAttMaxDataSize_d(gAttMaxMtu_c)];
    uint8_t         euTrackTitle[gAttMaxDataSize_d(gAttMaxMtu_c)];
    uint8_t         euTrackDuration[gAttMaxDataSize_d(gAttMaxMtu_c)];
} amsClientAppData_t;

/* APP -  pointer to function for Shell events*/
typedef void (*pfBleCallback_t)(void* pData);

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
/* shell callback */
static pfBleCallback_t mpfShellEventHandler = NULL;

static shellPrompt_t shellPrompt;

static shell_command_t mPlayCmd =
{
    .pcCommand = "Play",
    .pcHelpString = "\r\n\"Play\": Send Play command.\r\n",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellPlay_Command,
};

static shell_command_t mPauseCmd =
{
    .pcCommand = "Pause",
    .pcHelpString = "\r\n\"Pause\": Send Pause command.\r\n",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellPause_Command,
};

static shell_command_t mTogglePlayPauseCmd =
{
    .pcCommand = "TogglePlayPause",
    .pcHelpString = "\r\n\"TogglePlayPause\": Send TogglePlayPause command.\r\n",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellTogglePlayPause_Command,
};

static shell_command_t mNextTrackCmd =
{
    .pcCommand = "NextTrack",
    .pcHelpString = "\r\n\"NextTrack\": Send NextTrack command.\r\n",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellNextTrack_Command,
};

static shell_command_t mPreviousTrackCmd =
{
    .pcCommand = "PreviousTrack",
    .pcHelpString = "\r\n\"PreviousTrack\": Send PreviousTrack command.\r\n",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellPreviousTrack_Command,
};

static shell_command_t mVolumeUpCmd =
{
    .pcCommand = "VolumeUp",
    .pcHelpString = "\r\n\"VolumeUp\": Send VolumeUp command.\r\n",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellVolumeUp_Command,
};

static shell_command_t mVolumeDownCmd =
{
    .pcCommand = "VolumeDown",
    .pcHelpString = "\r\n\"VolumeDown\": Send VolumeDown command.\r\n",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellVolumeDown_Command,
};

static shell_command_t mAdvanceRepeatModeCmd =
{
    .pcCommand = "AdvanceRepeatMode",
    .pcHelpString = "\r\n\"AdvanceRepeatMode\": Send AdvanceRepeatMode command.\r\n",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellAdvanceRepeatMode_Command,
};

static shell_command_t mAdvanceShuffleModeCmd =
{
    .pcCommand = "AdvanceShuffleMode",
    .pcHelpString = "\r\n\"AdvanceShuffleMode\": Send AdvanceShuffleMode command.\r\n",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellAdvanceShuffleMode_Command,
};

static shell_command_t mSkipForwardCmd =
{
    .pcCommand = "SkipForward",
    .pcHelpString = "\r\n\"SkipForward\": Send SkipForward command.\r\n",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellSkipForward_Command,
};

static shell_command_t mSkipBackwardCmd =
{
    .pcCommand = "SkipBackward",
    .pcHelpString = "\r\n\"SkipBackward\": Send SkipBackward command.\r\n",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellSkipBackward_Command,
};

static shell_command_t mLikeTrackCmd =
{
    .pcCommand = "LikeTrack",
    .pcHelpString = "\r\n\"LikeTrack\": Send LikeTrack command.\r\n",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellLikeTrack_Command,
};

static shell_command_t mDislikeTrackCmd =
{
    .pcCommand = "DislikeTrack",
    .pcHelpString = "\r\n\"DislikeTrack\": Send DislikeTrack command.\r\n",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellDislikeTrack_Command,
};

static shell_command_t mBookmarkTrackCmd =
{
    .pcCommand = "BookmarkTrack",
    .pcHelpString = "\r\n\"BookmarkTrack\": Send BookmarkTrack command.\r\n",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellBookmarkTrack_Command,
};

static appPeerInfo_t mPeerInformation =
{
    .deviceId = gInvalidDeviceId_c,
    .customInfo =
    {
        .ancsClientConfig =
        {
            .hService = gGattDbInvalidHandle_d,
            .hNotificationSource = gGattDbInvalidHandle_d,
            .hNotificationSourceCccd = gGattDbInvalidHandle_d,
            .hControlPoint = gGattDbInvalidHandle_d,
            .hDataSource = gGattDbInvalidHandle_d,
            .hDataSourceCccd = gGattDbInvalidHandle_d,
        },
        .amsClientConfig =
        {
            .hService = gGattDbInvalidHandle_d,
            .hRemoteCommand = gGattDbInvalidHandle_d,
            .hRemoteCommandCccd = gGattDbInvalidHandle_d,
            .hEntityUpdate = gGattDbInvalidHandle_d,
            .hEntityUpdateCccd = gGattDbInvalidHandle_d,
            .hEntityAttribute = gGattDbInvalidHandle_d,
        },
    },
    .isBonded = FALSE,
    .appState = mAppIdle_c,
};

static char ancsFlagsToLetterTable[] =
{
    'S', /*!< 0 - Silent */
    'I', /*!< 1 - Important */
    'E', /*!< 2 - pre Existing */
    'P', /*!< 3 - Positive action */
    'N', /*!< 4 - Negative action */
};

static uint8_t ancsNotifCatToStringTable[][mMaxNotifCatDisplayLength_c + 1] = /* Extra byte for the 0 terminated string */
{
    "Other               ", /*  0 */
    "IncomingCall        ", /*  1 */
    "MissedCall          ", /*  2 */
    "Voicemail           ", /*  3 */
    "Social              ", /*  4 */
    "Schedule            ", /*  5 */
    "Email               ", /*  6 */
    "News                ", /*  7 */
    "Health And Fitness  ", /*  8 */
    "Business And Finance", /*  9 */
    "Location            ", /* 10 */
    "Entertainment       ", /* 11 */
    "Invalid             ", /* 12 */
};

static uint8_t amsRemoteCommandToStringTable[][mMaxRemoteCommandDisplayLength_c + 1] = /* Extra byte for the 0 terminated string */
{
    "Play",                 /*  0 */
    "Pause",                /*  1 */
    "TogglePlayPause",      /*  2 */
    "NextTrack",            /*  3 */
    "PreviousTrack",        /*  4 */
    "VolumeUp",             /*  5 */
    "VolumeDown",           /*  6 */
    "AdvanceRepeatMode",    /*  7 */
    "AdvanceShuffleMode",   /*  8 */
    "SkipForward",          /*  9 */
    "SkipBackward",         /* 10 */
    "LikeTrack",            /* 11 */
    "DislikeTrack",         /* 12 */
    "BookmarkTrack",        /* 13 */
};

static char mAncsFalgNotSetSymbol = '_';
static char mAncsAppNameCharPlaceholder = '_';
static char mAncsNotifCatCharPlaceholder = '_';

#if gAppUseBonding_d
    static bool_t mSendDataAfterEncStart = FALSE;
#endif

/* Adv State */
static advState_t       mAdvState;

/* Service Data */
static bool_t           basValidClientList[gAppMaxConnections_c] = { FALSE };
static basConfig_t      basServiceConfig = {(uint16_t)service_battery, 0, basValidClientList, gAppMaxConnections_c};
static disConfig_t      disServiceConfig = {(uint16_t)service_device_info};
static ctsConfig_t      ctsServiceConfig = {(uint16_t)service_current_time, gCts_InitTime, gCts_InitTime, 0U, gCts_InitLocalTimeInfo, gCts_InitReferenceTimeInfo, FALSE};
static rtusConfig_t     rtusServiceConfig = {(uint16_t)service_reference_time, {gRtusIdle_c, gRtusSuccessful_c}};
static ndcsConfig_t     ndcsServiceConfig = {(uint16_t)service_next_dst, {{2016, 1, 1, 0, 0, 0}, 0U}};
static uint16_t         cpHandles[] = { (uint16_t)value_current_time, (uint16_t)value_time_update_cp };

/* Application specific data*/
static TIMER_MANAGER_HANDLE_DEFINE(mAdvTimerId);
static TIMER_MANAGER_HANDLE_DEFINE(mCTSTickTimerId);
static TIMER_MANAGER_HANDLE_DEFINE(mRTUSReferenceUpdateTimerId);
static TIMER_MANAGER_HANDLE_DEFINE(mBatteryMeasurementTimerId);
static TIMER_MANAGER_HANDLE_DEFINE(mAllowNotificationsTimerId);

/* Buffer used for Service Discovery */
static gattService_t            *mpServiceDiscoveryBuffer = NULL;
static uint8_t                  mcPrimaryServices = 0;

/* Buffer used for Characteristic Discovery */
static gattCharacteristic_t     *mpCharDiscoveryBuffer = NULL;
static uint8_t                  mCurrentServiceInDiscoveryIndex;
static uint8_t                  mCurrentCharInDiscoveryIndex;

/* Buffer used for Characteristic Descriptor Discovery */
static gattAttribute_t          *mpCharDescriptorBuffer = NULL;

/* Buffer used for Characteristic related procedures */
static gattAttribute_t          *mpDescProcBuffer = NULL;

/* Application Data */

/*! ANCS/AMS Client data structure.
 *  Contains functional information about the notifications displayed
 *  and manipulated by the ANCS/AMS Client */
static ancsClientAppData_t      ancsClientData;
static amsClientAppData_t       amsClientData =
{
    .amsAvailableCommandsBitmask        = 0x0000U,
    .amsInvalidCommand                  = FALSE,
    .euTrackArtist                      = {0},
    .euTrackAlbum                       = {0},
    .euTrackTitle                       = {0},
    .euTrackDuration                    = {0},
};

#if gAppUseTimeService_d
    static uint8_t mOutCharReadBuffer[mCharReadBufferLength_c];
    static uint16_t mOutCharReadByteCount;
    static bool_t isTimeSynchronized = FALSE;

    static uint32_t localTime = mInitialTime_c;
#endif

static uint8_t mFirstRunningBeforeReconnect;

static uint8_t mReceivedNotificationsAndNeedToPrint;
static uint8_t mGetNotifOrAppAttribute;
static bool_t mCanSendToServer;

/* Application advertising parameters */
static appAdvertisingParams_t mAppAdvParams = {
    &gAdvParams,
    &gAppAdvertisingData,
    &gAppScanRspData
};

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
button_status_t BleApp_HandleKeys0(void *pButtonHandle, button_callback_message_t *pMessage,void *pCallbackParam);
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1))
button_status_t BleApp_HandleKeys1(void *pButtonHandle, button_callback_message_t *pMessage,void *pCallbackParam);
#endif /*gAppButtonCnt_c > 1*/
#endif /*gAppButtonCnt_c > 0*/
/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/
/* Host Stack callbacks */
static void BleApp_AdvertisingCallback
(
    gapAdvertisingEvent_t *pAdvertisingEvent
);

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

static void BleApp_GattNotificationCallback
(
    deviceId_t  serverDeviceId,
    uint16_t    characteristicValueHandle,
    uint8_t    *aValue,
    uint16_t    valueLength
);

static void BleApp_HandleValueWriteConfirmations
(
    deviceId_t  deviceId
);

static void BleApp_AttributeNotified
(
    deviceId_t  deviceId,
    uint16_t    handle,
    uint8_t    *pValue,
    uint16_t    length
);

static void BluetoothLEHost_Initialized(void);
static void BluetoothLEHost_GenericCallback(gapGenericEvent_t *pGenericEvent);

static void BleApp_StateMachineHandlerExchangeMtu(deviceId_t peerDeviceId, uint8_t event);
static void BleApp_StateMachineHandlerPrimaryServiceDisc(deviceId_t peerDeviceId, uint8_t event);
static void BleApp_StateMachineHandlerCharServiceDisc(deviceId_t peerDeviceId, uint8_t event);
static void BleApp_StateMachineHandlerNsDescriptorSetup(deviceId_t peerDeviceId, uint8_t event);

void BleApp_StateMachineHandler
(
    deviceId_t peerDeviceId,
    uint8_t event
);

static void BleApp_HandleConnection
(
    deviceId_t peerDeviceId
);

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

static void BleApp_ServiceDiscoveryReset(void);
static void BleApp_ServiceDiscoveryErrorHandler(void);

/* Timer Callbacks */
static void AdvertisingTimerCallback(void *pParam);
static void BatteryMeasurementTimerCallback(void *pParam);
static void CTSTickTimerCallback(void *pParam);
static void RTUSReferenceUpdateTimerCallback(void *pParam);
static void AllowNotificationsTimerCallback(void *pParam);

static void BleApp_Advertise(void);

/* Application Functions */
static void AncsClient_HandleDisconnectionEvent(deviceId_t deviceId);
static void AncsClient_ResetNotificationData(void);
static void AncsClient_ProcessNsNotification(deviceId_t deviceId, uint8_t *pNsData, uint16_t nsDataLength);
static void AncsClient_ProcessDsNotifAttributes(deviceId_t deviceId, uint8_t *pDsData, uint16_t dsDataLength);
static void AncsClient_ProcessDsNotifAppInfo(uint8_t *pDsData, uint16_t dsDataLength);
static void AncsClient_ProcessDsNotification(deviceId_t deviceId, uint8_t *pDsData, uint16_t dsDataLength);
static void AmsClient_ProcessRcNotification(deviceId_t deviceId, uint8_t *pRcData, uint16_t RcDataLength);
static void AmsClient_ProcessEuNotification(deviceId_t deviceId, uint8_t *pEuData, uint16_t EuDataLength);
static void AncsClient_DisplayNotifications(void);
static void AncsClient_SendCommandToAncsServer(deviceId_t ancsServerDevId, void *pCommand, uint16_t cmdLength);
static void AmsClient_SendCommandToAmsServer(deviceId_t amsServerDevId, uint16_t amsCharHandle, void *pCommand, uint16_t cmdLength);

static uint8_t AncsClient_CheckNeedGetNotifOrAppAttribute(void);
static void AncsClient_SendGetNotificationOrApplicationAttribute(void);

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief  Initializes the SHELL module .
*
* \param[in]  prompt the string which will be used for command prompt
*
* \remarks
*
********************************************************************************** */
void AppShellInit(char* prompt)
{
    shell_status_t status = kStatus_SHELL_Error;

    /* Avoid compiler warning in release mode. */
    (void)status;
    status = SHELL_Init((shell_handle_t)g_shellHandle, (serial_handle_t)gSerMgrIf, prompt);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mPlayCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mPauseCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mTogglePlayPauseCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mNextTrackCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mPreviousTrackCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mVolumeUpCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mVolumeDownCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mAdvanceRepeatModeCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mAdvanceShuffleModeCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mSkipForwardCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mSkipBackwardCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mLikeTrackCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mDislikeTrackCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mBookmarkTrackCmd);
    assert(kStatus_SHELL_Success == status);
}

/*! *********************************************************************************
 * \brief        Register function to handle commands from shell
 *
 * \param[in]    pCallback       event handler
 ********************************************************************************** */
void AppShell_RegisterCmdHandler(pfBleCallback_t pfShellEventHandler)
{
    mpfShellEventHandler = pfShellEventHandler;
}

/*! *********************************************************************************
* \brief  This is the initialization function for each application. This function
*         should contain all the initialization code required by the bluetooth demo
********************************************************************************** */
void BluetoothLEHost_AppInit(void)
{
    /* UI */
    shellPrompt.pConstPrompt = "BLE ANCS/AMS Client>";
    AppShellInit(shellPrompt.pPrompt);

    /* Register the function handler for the shell commands events  */
    AppShell_RegisterCmdHandler(App_HandleShellCmds);

    /* Set generic callback */
    BluetoothLEHost_SetGenericCallback(BluetoothLEHost_GenericCallback);

    /* Initialize Bluetooth Host Stack */
    BluetoothLEHost_Init(BluetoothLEHost_Initialized);
    
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[0], BleApp_HandleKeys0, NULL);
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[1], BleApp_HandleKeys1, NULL);
#endif /* (gAppButtonCnt_c > 1) */
#endif /* (gAppButtonCnt_c > 0) */

    /* Start flashing all leds */
    LedStartFlashingAllLeds();
}

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(void)
{
    Led1On();

    if (mPeerInformation.deviceId == gInvalidDeviceId_c)
    {
        /* Device is not connected and not advertising */
        if (!mAdvState.advOn)
        {
#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U)) &&\
    (defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1U)) &&\
    (defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0))
            if (gcBondedDevices > 0U)
            {
                mAdvState.advType = fastFilterAcceptListAdvState_c;
            }
            else
            {
#endif /* gAppUseBonding_d && gAppUsePrivacy_d && gBleEnableControllerPrivacy_d */
                mAdvState.advType = fastAdvState_c;
#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U)) &&\
    (defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1U)) &&\
    (defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0))
            }

#endif /* gAppUseBonding_d && gAppUsePrivacy_d && gBleEnableControllerPrivacy_d */
            BleApp_Advertise();
        }
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
button_status_t BleApp_HandleKeys0(void *pButtonHandle, button_callback_message_t *pMessage,void *pCallbackParam)
{
    switch (pMessage->event)
    {
        case kBUTTON_EventOneClick:
        case kBUTTON_EventShortPress:
        {
            BleApp_Start();
            break;
        }

        case kBUTTON_EventLongPress:
        {
            if (mPeerInformation.deviceId != gInvalidDeviceId_c)
            {
                (void)Gap_Disconnect(mPeerInformation.deviceId);
            }

            break;
        }
        
        default:
            ; /* For MISRA compliance */
            break;
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
button_status_t BleApp_HandleKeys1(void *pButtonHandle, button_callback_message_t *pMessage,void *pCallbackParam)
{
  switch (pMessage->event)
  {
      case kBUTTON_EventOneClick:
      case kBUTTON_EventShortPress:
      {
          ctsServiceConfig.localTime += 3600U;
          ctsServiceConfig.adjustReason = gCts_ManualUpdate;
          (void)Cts_RecordCurrentTime(&ctsServiceConfig);
          break;
      }

      default:
          ; /* For MISRA compliance */
          break;
  }
  return kStatus_BUTTON_Success; 
}
#endif /*gAppButtonCnt_c > 1*/
#endif /*gAppButtonCnt_c > 0*/

/*! *********************************************************************************
* \brief        Handles Shell Commands events.
*
* \param[in]    pData    value containing amsRemoteCommandId_t.
********************************************************************************** */
void App_HandleShellCmds(void *pData)
{
    union
    {
        amsRemoteCommandId_t amsRemoteCommandId;
        void *pAmsRemoteCommandId;
    } remoteCommandId;
    uint8_t amsRemoteCommandConstruct[gAmsRemoteCommandIdFieldLength_c] = {0};

    remoteCommandId.pAmsRemoteCommandId = pData;

    /* Prepare a Remote Command for media control */
    amsRemoteCommandConstruct[0] = remoteCommandId.amsRemoteCommandId;

    if ((amsClientData.amsAvailableCommandsBitmask & (1 << remoteCommandId.amsRemoteCommandId)) != 0)
    {
        AmsClient_SendCommandToAmsServer(mPeerInformation.deviceId,
                                         mPeerInformation.customInfo.amsClientConfig.hRemoteCommand,
                                         amsRemoteCommandConstruct,
                                         (uint16_t)sizeof(amsRemoteCommandConstruct));
    }
    else
    {
        amsClientData.amsInvalidCommand = TRUE;
    }

    /* Use flag in ancs to have the same consistent print */
    ancsClientData.notificationDataChanged = TRUE;

    AncsClient_DisplayNotifications();
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief        Send Play command to application task.
*
* \param[in]    shellHandle    Shell handle
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellPlay_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    if(mpfShellEventHandler != NULL)
    {
        App_PostCallbackMessage(mpfShellEventHandler, (void *)gAmsRemoteCommandIdPlay_c);
    }

    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Send Pause command to application task.
*
* \param[in]    shellHandle    Shell handle
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellPause_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    if(mpfShellEventHandler != NULL)
    {
        App_PostCallbackMessage(mpfShellEventHandler, (void *)gAmsRemoteCommandIdPause_c);
    }

    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Send TogglePlayPause command to application task.
*
* \param[in]    shellHandle    Shell handle
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellTogglePlayPause_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    if(mpfShellEventHandler != NULL)
    {
        App_PostCallbackMessage(mpfShellEventHandler, (void *)gAmsRemoteCommandIdTogglePlayPause_c);
    }

    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Send NextTrack command to application task.
*
* \param[in]    shellHandle    Shell handle
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellNextTrack_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    if(mpfShellEventHandler != NULL)
    {
        App_PostCallbackMessage(mpfShellEventHandler, (void *)gAmsRemoteCommandIdNextTrack_c);
    }

    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Send PreviousTrack command to application task.
*
* \param[in]    shellHandle    Shell handle
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellPreviousTrack_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    if(mpfShellEventHandler != NULL)
    {
        App_PostCallbackMessage(mpfShellEventHandler, (void *)gAmsRemoteCommandIdPreviousTrack_c);
    }

    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Send VolumeUp command to application task.
*
* \param[in]    shellHandle    Shell handle
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellVolumeUp_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    if(mpfShellEventHandler != NULL)
    {
        App_PostCallbackMessage(mpfShellEventHandler, (void *)gAmsRemoteCommandIdVolumeUp_c);
    }

    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Send VolumeDown command to application task.
*
* \param[in]    shellHandle    Shell handle
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellVolumeDown_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    if(mpfShellEventHandler != NULL)
    {
        App_PostCallbackMessage(mpfShellEventHandler, (void *)gAmsRemoteCommandIdVolumeDown_c);
    }

    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Send AdvanceRepeatMode command to application task.
*
* \param[in]    shellHandle    Shell handle
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellAdvanceRepeatMode_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    if(mpfShellEventHandler != NULL)
    {
        App_PostCallbackMessage(mpfShellEventHandler, (void *)gAmsRemoteCommandIdAdvanceRepeatMode_c);
    }

    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Send AdvanceShuffleMode command to application task.
*
* \param[in]    shellHandle    Shell handle
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellAdvanceShuffleMode_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    if(mpfShellEventHandler != NULL)
    {
        App_PostCallbackMessage(mpfShellEventHandler, (void *)gAmsRemoteCommandIdAdvanceShuffleMode_c);
    }

    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Send SkipForward command to application task.
*
* \param[in]    shellHandle    Shell handle
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellSkipForward_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    if(mpfShellEventHandler != NULL)
    {
        App_PostCallbackMessage(mpfShellEventHandler, (void *)gAmsRemoteCommandIdSkipForward_c);
    }

    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Send SkipBackward command to application task.
*
* \param[in]    shellHandle    Shell handle
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellSkipBackward_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    if(mpfShellEventHandler != NULL)
    {
        App_PostCallbackMessage(mpfShellEventHandler, (void *)gAmsRemoteCommandIdSkipBackward_c);
    }

    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Send LikeTrack command to application task.
*
* \param[in]    shellHandle    Shell handle
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellLikeTrack_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    if(mpfShellEventHandler != NULL)
    {
        App_PostCallbackMessage(mpfShellEventHandler, (void *)gAmsRemoteCommandIdLikeTrack_c);
    }

    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Send DislikeTrack command to application task.
*
* \param[in]    shellHandle    Shell handle
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellDislikeTrack_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    if(mpfShellEventHandler != NULL)
    {
        App_PostCallbackMessage(mpfShellEventHandler, (void *)gAmsRemoteCommandIdDislikeTrack_c);
    }

    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Send BookmarkTrack command to application task.
*
* \param[in]    shellHandle    Shell handle
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellBookmarkTrack_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    if(mpfShellEventHandler != NULL)
    {
        App_PostCallbackMessage(mpfShellEventHandler, (void *)gAmsRemoteCommandIdBookmarkTrack_c);
    }

    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
static void BluetoothLEHost_GenericCallback(gapGenericEvent_t *pGenericEvent)
{
    /* Call BLE Conn Manager */
    BleConnManager_GenericEvent(pGenericEvent);
}

/*! *********************************************************************************
* \brief        Configures BLE Stack after initialization. Usually used for
*               configuring advertising, scanning, filter accept list, services, et al.
*
********************************************************************************** */
static void BluetoothLEHost_Initialized(void)
{
    /* Common GAP configuration */
    BleConnManager_GapCommonConfig();

    /* Register for callbacks*/
    (void)App_RegisterGattServerCallback(BleApp_GattServerCallback);
    (void)App_RegisterGattClientProcedureCallback(BleApp_GattClientCallback);
    (void)GattServer_RegisterHandlesForWriteNotifications(NumberOfElements(cpHandles), cpHandles);
    (void)App_RegisterGattClientNotificationCallback(BleApp_GattNotificationCallback);

    mAdvState.advOn = FALSE;

    /* Start services */
    basServiceConfig.batteryLevel = SENSORS_GetBatteryLevel();
    (void)Bas_Start(&basServiceConfig);
    (void)Dis_Start(&disServiceConfig);
    (void)Cts_Start(&ctsServiceConfig);
    (void)Ndcs_Start(&ndcsServiceConfig);
    (void)Rtus_Start(&rtusServiceConfig);

    /* Allocate application timers */
    (void)TM_Open(mAdvTimerId);
    (void)TM_Open(mBatteryMeasurementTimerId);
    (void)TM_Open(mCTSTickTimerId);
    (void)TM_Open(mRTUSReferenceUpdateTimerId);
    (void)TM_Open(mAllowNotificationsTimerId);

    /* Start local time tick timer */
    (void)TM_InstallCallback((timer_handle_t)mCTSTickTimerId, CTSTickTimerCallback, NULL);
    (void)TM_Start((timer_handle_t)mCTSTickTimerId, (uint16_t)kTimerModeLowPowerTimer | (uint16_t)kTimerModeSetSecondTimer, 1);

    /* Start reference update timer */
    (void)TM_InstallCallback((timer_handle_t)mRTUSReferenceUpdateTimerId, RTUSReferenceUpdateTimerCallback, NULL);
    (void)TM_Start((timer_handle_t)mRTUSReferenceUpdateTimerId, (uint16_t)kTimerModeLowPowerTimer | (uint16_t)kTimerModeSetSecondTimer, 60);

    /* Reset application data. */
    AncsClient_ResetNotificationData();

    /* UI */
    shell_write("\r\nPress ADVSW to start advertising to find a device with the ANCS Service!\r\n");
}

/*! *********************************************************************************
* \brief        Configures GAP Advertise parameters. Advertise will start after
*               the parameters are set.
*
********************************************************************************** */
static void BleApp_Advertise(void)
{
    uint32_t timeout = 0;

    switch (mAdvState.advType)
    {
#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U)) &&\
    (defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1U)) &&\
    (defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0))
        case fastFilterAcceptListAdvState_c:
        {
            gAdvParams.minInterval = gFastConnMinAdvInterval_c;
            gAdvParams.maxInterval = gFastConnMaxAdvInterval_c;
            gAdvParams.filterPolicy = gProcessFilterAcceptListOnly_c;
            timeout = gFastConnFilterAcceptListAdvTime_c;
        }
        break;
#endif /* gAppUseBonding_d && gAppUsePrivacy_d && gBleEnableControllerPrivacy_d */
        case fastAdvState_c:
        {
            gAdvParams.minInterval = gFastConnMinAdvInterval_c;
            gAdvParams.maxInterval = gFastConnMaxAdvInterval_c;
            gAdvParams.filterPolicy = gProcessAll_c;
            timeout = gFastConnAdvTime_c - gFastConnFilterAcceptListAdvTime_c;
        }
        break;

        case slowAdvState_c:
        {
            gAdvParams.minInterval = gReducedPowerMinAdvInterval_c;
            gAdvParams.maxInterval = gReducedPowerMinAdvInterval_c;
            gAdvParams.filterPolicy = gProcessAll_c;
            timeout = gReducedPowerAdvTime_c;
        }
        break;

        default:
            ; /* For MISRA compliance */
            break;
    }

    /* Start advertising*/
    (void)BluetoothLEHost_StartAdvertising(&mAppAdvParams, BleApp_AdvertisingCallback, BleApp_ConnectionCallback);

    /* Start advertising timer */
    (void)TM_InstallCallback((timer_handle_t)mAdvTimerId, AdvertisingTimerCallback, NULL);
    (void)TM_Start((timer_handle_t)mAdvTimerId, (uint16_t)kTimerModeLowPowerTimer | (uint16_t)kTimerModeSetSecondTimer, timeout);
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
            
            LedStopFlashingAllLeds();

            if (mAdvState.advOn)
            {
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c ==1))
                LedSetColor(0, kLED_Blue);    
#endif /* gAppLedCnt_c == 1 */ 
                Led1Flashing();
                shell_write("\r\nAdvertising...\r\n");
            }
            else
            {
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c ==1))
                LedSetColor(0, kLED_White);    
#endif /* gAppLedCnt_c == 1 */ 
                LedStartFlashingAllLeds();
                shell_write("\r\nStopped advertising...\r\n");
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
            shell_write("\r\nWarning: Unhandled Advertising Event:");
            shell_write(" 0x");
            shell_writeHex((uint8_t)pAdvertisingEvent->eventType);
            shell_write("\r\n");
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
    /* Connection Manager to handle Host Stack interactions */
    BleConnManager_GapPeripheralEvent(peerDeviceId, pConnectionEvent);

    switch (pConnectionEvent->eventType)
    {
        case gConnEvtConnected_c:
        {
            BleApp_HandleConnection(peerDeviceId);
        }
        break;

        case gConnEvtDisconnected_c:
        {
            mPeerInformation.deviceId = gInvalidDeviceId_c;
            mPeerInformation.appState = mAppIdle_c;
            mPeerInformation.isBonded = FALSE;
            mPeerInformation.customInfo.ancsClientConfig.hService = gGattDbInvalidHandle_d;
            mPeerInformation.customInfo.ancsClientConfig.hNotificationSource = gGattDbInvalidHandle_d;
            mPeerInformation.customInfo.ancsClientConfig.hNotificationSourceCccd = gGattDbInvalidHandle_d;
            mPeerInformation.customInfo.ancsClientConfig.hControlPoint = gGattDbInvalidHandle_d;
            mPeerInformation.customInfo.ancsClientConfig.hDataSource = gGattDbInvalidHandle_d;
            mPeerInformation.customInfo.ancsClientConfig.hDataSourceCccd = gGattDbInvalidHandle_d;
            mPeerInformation.customInfo.amsClientConfig.hService = gGattDbInvalidHandle_d;
            mPeerInformation.customInfo.amsClientConfig.hRemoteCommand = gGattDbInvalidHandle_d;
            mPeerInformation.customInfo.amsClientConfig.hRemoteCommandCccd = gGattDbInvalidHandle_d;
            mPeerInformation.customInfo.amsClientConfig.hEntityUpdate = gGattDbInvalidHandle_d;
            mPeerInformation.customInfo.amsClientConfig.hEntityUpdateCccd = gGattDbInvalidHandle_d;
            mPeerInformation.customInfo.amsClientConfig.hEntityAttribute = gGattDbInvalidHandle_d;

            /* Reset Service Discovery to be sure */
            BleServDisc_Stop(peerDeviceId);

            /* If peer device disconnects the link during Service Discovery, free the allocated buffers */
            BleApp_ServiceDiscoveryReset();

            /* Stop battery measurements */
            (void)TM_Stop((timer_handle_t)mBatteryMeasurementTimerId);

            /* Stop the timer needed to wait for the user to press the Allow Notifications button */
            (void)TM_Stop((timer_handle_t)mAllowNotificationsTimerId);

            /* Notify application */
            AncsClient_HandleDisconnectionEvent(peerDeviceId);

            /* UI */
            LedStopFlashingAllLeds();
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c ==1))
            LedSetColor(0, kLED_White);    
#endif /* gAppLedCnt_c == 1 */ 
            LedStartFlashingAllLeds();
            shell_write("\r\nDisconnected!\r\n");

            /* Unsubscribe client*/
            (void)Bas_Unsubscribe(&basServiceConfig, peerDeviceId);
            (void)Cts_Unsubscribe();
            (void)Ndcs_Unsubscribe();
            (void)Rtus_Unsubscribe();

            /* Restart advertising */
            BleApp_Start();
        }
        break;

        case gConnEvtPairingRequest_c:
        {
#if gAppUsePairing_d
            shell_write("\r\nReceived Pairing Request. Response sent by Connection Manager.\r\n");
#else
            shell_write("\r\nReceived Pairing Request. Response sent by Connection Manager. Pairing not supported.\r\n");
#endif
        }
        break;

#if gAppUsePairing_d

        case gConnEvtPasskeyRequest_c:
            (void)Gap_EnterPasskey(peerDeviceId, gPasskeyValue_c);
            shell_write("\r\nReceived Passkey Request. Please enter passkey...\r\n");
            break;

        case gConnEvtPasskeyDisplay_c:
            shell_write("\r\nReceived Passkey Display Request. The passkey is: ");
            shell_writeDec(pConnectionEvent->eventData.passkeyForDisplay);
            shell_write("\r\n");
            break;

        case gConnEvtLeScDisplayNumericValue_c:
            shell_write("\r\nReceived Numeric Comparison Value: ");
            shell_writeDec(pConnectionEvent->eventData.numericValueForDisplay);
            shell_write("\r\n");
            break;

        case gConnEvtPairingResponse_c:
        {
            shell_write("\r\nReceived Pairing Response.\r\n");
            break;
        }

        case gConnEvtKeyExchangeRequest_c:
        {
            /* For GAP Peripherals the keys are automatically sent by the Connection Manager */
            shell_write("\r\nReceived Key Exchange Request. Sending keys...\r\n");
            break;
        }

        case gConnEvtLongTermKeyRequest_c:
        {
            shell_write("\r\nReceived LTK Request. Handled in Connection Manager.\r\n");
        }
        break;

        case gConnEvtEncryptionChanged_c:
        {
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
            mPeerInformation.isBonded = TRUE;
#endif
#if gAppUseTimeService_d

            if (isTimeSynchronized == FALSE)
            {
                bleUuid_t uuid = { .uuid16 = gBleSig_CurrentTime_d };

                /* Read CTS Characteristic. If the device doesn't have time services
                 gAttErrCodeAttributeNotFound_c will be received. */
                GattClient_ReadUsingCharacteristicUuid
                (
                    peerDeviceId,
                    gBleUuidType16_c,
                    &uuid,
                    NULL,
                    mOutCharReadBuffer,
                    13,
                    &mOutCharReadByteCount
                );
            }

#endif

#if gAppUseBonding_d
            if (mSendDataAfterEncStart)
            {
                /* Application handles encryption changes here. */
            }
#endif

            shell_write("\r\nReceived encryption changed event with link encryption status: ");
            shell_writeBool(pConnectionEvent->eventData.encryptionChangedEvent.newEncryptionState);
            shell_write("\r\n");
        }
        break;

        case gConnEvtPairingComplete_c:
        {
            shell_write("\r\nPairing completed!\r\n");

#if gAppUseTimeService_d

            if (isTimeSynchronized == FALSE)
            {
                bleUuid_t uuid = { .uuid16 = gBleSig_CurrentTime_d };

                /* Read CTS Characteristic. If the device doesn't have time services
                 gAttErrCodeAttributeNotFound_c will be received. */
                GattClient_ReadUsingCharacteristicUuid
                (
                    peerDeviceId,
                    gBleUuidType16_c,
                    &uuid,
                    NULL,
                    mOutCharReadBuffer,
                    13,
                    &mOutCharReadByteCount
                );
            }

#endif

            if (pConnectionEvent->eventData.pairingCompleteEvent.pairingSuccessful)
            {
                shell_write("\r\nPairing was successful!\r\n");
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
                mPeerInformation.isBonded = TRUE;
#endif
                BleApp_StateMachineHandler(peerDeviceId, mAppEvt_PairingComplete_c);
            }
            else
            {
                shell_write("\r\nPairing was NOT successful with status:");
                shell_write(" 0x");
                uint8_t *p = (uint8_t *)&(pConnectionEvent->eventData.pairingCompleteEvent.pairingCompleteData.failReason);
                shell_writeHex(p[1]);
                shell_writeHex(p[0]);
                shell_write("\r\n");
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
                 mPeerInformation.isBonded = FALSE;
#endif
            }
        }
        break;

        case gConnEvtAuthenticationRejected_c:
        {
            shell_write("\r\nReceived authentication rejected event with status:");
            shell_write(" 0x");
            shell_writeHex(pConnectionEvent->eventData.authenticationRejectedEvent.rejectReason);
            shell_write("\r\n");
        }
        break;

        case gConnEvtKeysReceived_c:
        {
            shell_write("\r\nPairing Info: Received keys from peer device.\r\n");
        }
        break;
#endif /* gAppUsePairing_d */

        case gConnEvtParameterUpdateComplete_c:
        {
            if (pConnectionEvent->eventData.connectionUpdateComplete.status == gBleSuccess_c)
            {
                shell_write("\r\nConnection parameters successfully updated!\r\n");
            }
            else
            {
                shell_write("\r\nAn error occurred while updating the connection parameters!\r\n");
            }
        }
        break;

        case gConnEvtLeDataLengthChanged_c:
        {
            shell_write("\r\nData length changed:");
            shell_write("\r\n MaxTxOctets: ");
            shell_writeDec((uint32_t)pConnectionEvent->eventData.leDataLengthChanged.maxTxOctets);
            shell_write("\r\n MaxTxTime: ");
            shell_writeDec((uint32_t)pConnectionEvent->eventData.leDataLengthChanged.maxTxTime);
            shell_write("\r\n MaxRxOctets: ");
            shell_writeDec((uint32_t)pConnectionEvent->eventData.leDataLengthChanged.maxRxOctets);
            shell_write("\r\n MaxRxTime: ");
            shell_writeDec((uint32_t)pConnectionEvent->eventData.leDataLengthChanged.maxRxTime);
            shell_write("\r\n");
        }
        break;

        default:
        {
            shell_write("\r\nWarning: Unhandled GAP Connection Event:");
            shell_write(" 0x");
            shell_writeHex((uint8_t)pConnectionEvent->eventType);
            shell_write("\r\n");
        }
        break;
    }
}

static void BleApp_HandleAttMtuChange
(
    deviceId_t peerDeviceId
)
{
    uint16_t negotiatedAttMtu = 0U;

    /* Get the new negotiated ATT MTU */
    (void)Gatt_GetMtu(peerDeviceId, &negotiatedAttMtu);
    shell_write("\r\nNegotiated MTU: ");
    shell_writeDec(negotiatedAttMtu);
    shell_write("\r\n");
}

static void BleApp_StoreServiceHandles
(
    gattService_t   *pService
)
{
    uint8_t i;

    if ((pService->uuidType == gBleUuidType128_c) &&
        FLib_MemCmp(pService->uuid.uuid128, uuid128_ancs_service, 16))
    {
        /* Found ANCS Service */
        mPeerInformation.customInfo.ancsClientConfig.hService = pService->startHandle;
        shell_write("\r\nFound and stored ANCS Service handle!\r\n");

        /* Look for the ANCS Characteristics */
        for (i = 0; i < pService->cNumCharacteristics; i++)
        {
            if (pService->aCharacteristics[i].value.uuidType == gBleUuidType128_c)
            {
                if (TRUE == FLib_MemCmp(pService->aCharacteristics[i].value.uuid.uuid128,
                                        uuid128_ancs_ns_char,
                                        sizeof(pService->aCharacteristics[i].value.uuid.uuid128)))
                {
                    /* Found ANCS Notification Source Char */
                    mPeerInformation.customInfo.ancsClientConfig.hNotificationSource = pService->aCharacteristics[i].value.handle;
                    shell_write("\r\nFound and stored ANCS Notification Source Characteristic handle!\r\n");
                }
                else if (TRUE == FLib_MemCmp(pService->aCharacteristics[i].value.uuid.uuid128,
                                             uuid128_ancs_cp_char,
                                             sizeof(pService->aCharacteristics[i].value.uuid.uuid128)))
                {
                    /* Found ANCS Control Point Char */
                    mPeerInformation.customInfo.ancsClientConfig.hControlPoint = pService->aCharacteristics[i].value.handle;
                    shell_write("\r\nFound and stored ANCS Control Point Characteristic handle!\r\n");
                }
                else if (TRUE == FLib_MemCmp(pService->aCharacteristics[i].value.uuid.uuid128,
                                             uuid128_ancs_ds_char,
                                             sizeof(pService->aCharacteristics[i].value.uuid.uuid128)))
                {
                    /* Found ANCS Data Source Char */
                    mPeerInformation.customInfo.ancsClientConfig.hDataSource = pService->aCharacteristics[i].value.handle;
                    shell_write("\r\nFound and stored ANCS Data Source Characteristic handle!\r\n");
                }
                else
                {
                    ; /* For MISRA compliance */
                }
            }
        }
    }
    else if ((pService->uuidType == gBleUuidType128_c) &&
        FLib_MemCmp(pService->uuid.uuid128, uuid128_ams_service, 16))
    {
        /* Found AMS Service */
        mPeerInformation.customInfo.amsClientConfig.hService = pService->startHandle;
        shell_write("\r\nFound and stored AMS Service handle!\r\n");

        /* Look for the AMS Characteristics */
        for (i = 0; i < pService->cNumCharacteristics; i++)
        {
            if (pService->aCharacteristics[i].value.uuidType == gBleUuidType128_c)
            {
                if (TRUE == FLib_MemCmp(pService->aCharacteristics[i].value.uuid.uuid128,
                                        uuid128_ams_rc_char,
                                        sizeof(pService->aCharacteristics[i].value.uuid.uuid128)))
                {
                    /* Found AMS Remote Command Char */
                    mPeerInformation.customInfo.amsClientConfig.hRemoteCommand = pService->aCharacteristics[i].value.handle;
                    shell_write("\r\nFound and stored AMS Remote Command Characteristic handle!\r\n");
                }
                else if (TRUE == FLib_MemCmp(pService->aCharacteristics[i].value.uuid.uuid128,
                                             uuid128_ams_eu_char,
                                             sizeof(pService->aCharacteristics[i].value.uuid.uuid128)))
                {
                    /* Found AMS Entity Update Char */
                    mPeerInformation.customInfo.amsClientConfig.hEntityUpdate = pService->aCharacteristics[i].value.handle;
                    shell_write("\r\nFound and stored AMS Entity Update Characteristic handle!\r\n");
                }
                else if (TRUE == FLib_MemCmp(pService->aCharacteristics[i].value.uuid.uuid128,
                                             uuid128_ams_ea_char,
                                             sizeof(pService->aCharacteristics[i].value.uuid.uuid128)))
                {
                    /* Found AMS Entity Attribute Char */
                    mPeerInformation.customInfo.amsClientConfig.hEntityAttribute = pService->aCharacteristics[i].value.handle;
                    shell_write("\r\nFound and stored AMS Entity Attribute Characteristic handle!\r\n");
                }
                else
                {
                    ; /* For MISRA compliance */
                }
            }
        }
    }
    else
    {
        ; /* For MISRA compliance */
    }
}

static void BleApp_StoreCharHandles
(
    gattCharacteristic_t   *pChar
)
{
    uint8_t i;

    if (pChar->value.uuidType == gBleUuidType128_c)
    {
        if (TRUE == FLib_MemCmp(pChar->value.uuid.uuid128,
                                uuid128_ancs_ns_char,
                                sizeof(pChar->value.uuid.uuid128)))
        {
            for (i = 0; i < pChar->cNumDescriptors; i++)
            {
                if (pChar->aDescriptors[i].uuidType == gBleUuidType16_c)
                {
                    switch (pChar->aDescriptors[i].uuid.uuid16)
                    {
                        case gBleSig_CCCD_d:
                        {
                            mPeerInformation.customInfo.ancsClientConfig.hNotificationSourceCccd = pChar->aDescriptors[i].handle;
                            shell_write("\r\nFound and stored ANCS Notification Source CCCD handle!\r\n");
                            break;
                        }

                        default:
                            ; /* For MISRA compliance */
                            break;
                    }
                }
            }
        }
        else if (TRUE == FLib_MemCmp(pChar->value.uuid.uuid128,
                                     uuid128_ancs_ds_char,
                                     sizeof(pChar->value.uuid.uuid128)))
        {
            for (i = 0; i < pChar->cNumDescriptors; i++)
            {
                if (pChar->aDescriptors[i].uuidType == gBleUuidType16_c)
                {
                    switch (pChar->aDescriptors[i].uuid.uuid16)
                    {
                        case gBleSig_CCCD_d:
                        {
                            mPeerInformation.customInfo.ancsClientConfig.hDataSourceCccd = pChar->aDescriptors[i].handle;
                            shell_write("\r\nFound and stored ANCS Data Source CCCD handle!\r\n");
                            break;
                        }

                        default:
                            ; /* For MISRA compliance */
                            break;
                    }
                }
            }
        }
        else if (TRUE == FLib_MemCmp(pChar->value.uuid.uuid128,
                                     uuid128_ams_rc_char,
                                     sizeof(pChar->value.uuid.uuid128)))
        {
            for (i = 0; i < pChar->cNumDescriptors; i++)
            {
                if (pChar->aDescriptors[i].uuidType == gBleUuidType16_c)
                {
                    switch (pChar->aDescriptors[i].uuid.uuid16)
                    {
                        case gBleSig_CCCD_d:
                        {
                            mPeerInformation.customInfo.amsClientConfig.hRemoteCommandCccd = pChar->aDescriptors[i].handle;
                            shell_write("\r\nFound and stored AMS Remote Control CCCD handle!\r\n");
                            break;
                        }

                        default:
                            ; /* For MISRA compliance */
                            break;
                    }
                }
            }
        }
        else if (TRUE == FLib_MemCmp(pChar->value.uuid.uuid128,
                                     uuid128_ams_eu_char,
                                     sizeof(pChar->value.uuid.uuid128)))
        {
            for (i = 0; i < pChar->cNumDescriptors; i++)
            {
                if (pChar->aDescriptors[i].uuidType == gBleUuidType16_c)
                {
                    switch (pChar->aDescriptors[i].uuid.uuid16)
                    {
                        case gBleSig_CCCD_d:
                        {
                            mPeerInformation.customInfo.amsClientConfig.hEntityUpdateCccd = pChar->aDescriptors[i].handle;
                            shell_write("\r\nFound and stored AMS Entity Update CCCD handle!\r\n");
                            break;
                        }

                        default:
                            ; /* For MISRA compliance */
                            break;
                    }
                }
            }
        }
        else
        {
            ; /* For MISRA compliance */
        }
    }
}

/*! *********************************************************************************
* \brief        Handles GATT server callback from host stack.
*
* \param[in]    deviceId        Peer device ID.
* \param[in]    pServerEvent    Pointer to gattServerEvent_t.
********************************************************************************** */
static void BleApp_GattServerCallback(deviceId_t deviceId, gattServerEvent_t *pServerEvent)
{
    uint16_t handle;
    uint8_t status;

    switch (pServerEvent->eventType)
    {
        case gEvtAttributeWritten_c:
        {
            handle = pServerEvent->eventData.attributeWrittenEvent.handle;
            status = (uint8_t)gAttErrCodeNoError_c;

            if (handle == (uint8_t)value_current_time)
            {
                (void)Cts_CurrentTimeWrittenHandler(&ctsServiceConfig, &pServerEvent->eventData.attributeWrittenEvent);
            }
            else
            {
                (void)GattServer_SendAttributeWrittenStatus(deviceId, handle, status);
            }

            break;
        }

        case gEvtAttributeWrittenWithoutResponse_c:
        {
            handle = pServerEvent->eventData.attributeWrittenEvent.handle;

            if (handle == (uint16_t)value_time_update_cp)
            {
                Rtus_ControlPointHandler(&rtusServiceConfig, &pServerEvent->eventData.attributeWrittenEvent);
            }

            break;
        }

        default:
            ; /* For MISRA compliance */
            break;
    }
}

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
    if (procedureResult == gGattProcError_c)
    {
        attErrorCode_t attError = (attErrorCode_t)((uint8_t)error);
        uint16_t mask = 0xFF00U;
        uint16_t receivedStatusBase = (uint16_t)error;
        receivedStatusBase &= mask;

        if(receivedStatusBase == (uint16_t)gAttStatusBase_c)
        {
            if (attError == gAttErrCodeInsufficientEncryption_c         ||
                attError == gAttErrCodeInsufficientEncryptionKeySize_c  ||
                attError == gAttErrCodeInsufficientAuthorization_c      ||
                attError == gAttErrCodeInsufficientAuthentication_c)
            {
#if gAppUsePairing_d && gAppUseBonding_d
            bool_t isBonded = FALSE;
#endif /* gAppUsePairing_d && gAppUseBonding_d */

                shell_write("\r\nGATT Procedure Security Error:");
                shell_write(" 0x");
                shell_writeHex((uint8_t)attError);
                shell_write("\r\n");

#if gAppUsePairing_d
#if gAppUseBonding_d

                /* Check if the devices are bonded and if this is true than the bond may have
                 * been lost on the peer device or the security properties may not be sufficient.
                 * In this case try to restart pairing and bonding. */
                if ((gBleSuccess_c == Gap_CheckIfBonded(serverDeviceId, &isBonded, NULL)) &&
                    (TRUE == isBonded))
#endif /* gAppUseBonding_d */
                {
                    (void)Gap_SendPeripheralSecurityRequest(serverDeviceId, &gPairingParameters);
                }

#endif /* gAppUsePairing_d */
            }
        }
        else
        {
            if(receivedStatusBase == (uint16_t)gGattStatusBase_c)
            {
                shell_write("\r\nGATT Procedure Error:");
                shell_write(" 0x");
                shell_writeHex((uint8_t)attError);
                shell_write("\r\n");
            }
        }

        BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcError_c);
    }
    else if (procedureResult == gGattProcSuccess_c)
    {
        switch (procedureType)
        {
            case gGattProcExchangeMtu_c:
            {
                BleApp_HandleAttMtuChange(serverDeviceId);
            }
            break;

            case gGattProcDiscoverAllPrimaryServices_c:         /* Fall-through */
            case gGattProcWriteCharacteristicDescriptor_c:
                break;


            case gGattProcDiscoverAllCharacteristics_c:
            {
                BleApp_StoreServiceHandles(&mpServiceDiscoveryBuffer[mCurrentServiceInDiscoveryIndex]);
            }
            break;

            case gGattProcDiscoverAllCharacteristicDescriptors_c:
            {
                BleApp_StoreCharHandles(&mpCharDiscoveryBuffer[mCurrentCharInDiscoveryIndex]);

                /* Move on to the next characteristic */
                mCurrentCharInDiscoveryIndex++;
            }
            break;

            case gGattProcReadCharacteristicDescriptor_c:
            {
                if (mpDescProcBuffer != NULL)
                {
                    BleApp_StoreDescValues(mpDescProcBuffer);
                }
            }
            break;

            case gGattProcWriteCharacteristicValue_c:
            {
                BleApp_HandleValueWriteConfirmations(serverDeviceId);
            }
            break;

            case gGattProcReadUsingCharacteristicUuid_c:
            {
#if gAppUseTimeService_d

                if (mOutCharReadByteCount > 2)
                {
                    ctsDayDateTime_t time;
                    uint8_t *pValue = &mOutCharReadBuffer[3];

                    time.dateTime.year          = Utils_ExtractTwoByteValue(&pValue[0]);
                    time.dateTime.month         = pValue[2];
                    time.dateTime.day           = pValue[3];
                    time.dateTime.hours         = pValue[4];
                    time.dateTime.minutes       = pValue[5];
                    time.dateTime.seconds       = pValue[6];
                    time.dayOfWeek              = pValue[7];

                    localTime = Cts_DayDateTimeToEpochTime(time);

                    isTimeSynchronized = TRUE;
                }

#endif /* gAppUseTimeService_d */
            }
            break;

            default:
            {
                shell_write("\r\nWarning: Unhandled GATT Client Procedure:");
                shell_write(" 0x");
                shell_writeHex((uint8_t)procedureType);
                shell_write("\r\n");
            }
            break;
        }

        BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcComplete_c);
    }
    else
    {
        ; /* For MISRA compliance */
    }
}

static void BleApp_HandleValueWriteConfirmations(deviceId_t  deviceId)
{
    /* Handle all command confirmations here - only for commands written
     * to the ANCS Control Point Characteristic. */
    switch (ancsClientData.lastCommandSentToAncsProvider)
    {
        case gAncsCmdIdNoCommand_c:
            /* MISRA compliance */
            break;

        default:
            ; /* Do nothing here. */
            break;
    };
}

static void BleApp_GattNotificationCallback
(
    deviceId_t  serverDeviceId,
    uint16_t    characteristicValueHandle,
    uint8_t    *aValue,
    uint16_t    valueLength
)
{
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
    if (mPeerInformation.isBonded)
    {
#endif
        BleApp_AttributeNotified(serverDeviceId,
                                 characteristicValueHandle,
                                 aValue,
                                 valueLength);
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
    }
#endif
}

static void BleApp_AttributeNotified
(
    deviceId_t  deviceId,
    uint16_t    handle,
    uint8_t    *pValue,
    uint16_t    length
)
{
    if (mPeerInformation.appState == mAppRunning_c)
    {
        if (handle == mPeerInformation.customInfo.ancsClientConfig.hNotificationSource)
        {
            AncsClient_ProcessNsNotification(deviceId, pValue, length);
            BleApp_StateMachineHandler(deviceId, mAppEvt_AncsNsNotificationReceived_c);
        }
        else if (handle == mPeerInformation.customInfo.ancsClientConfig.hDataSource)
        {
            AncsClient_ProcessDsNotification(deviceId, pValue, length);
            BleApp_StateMachineHandler(deviceId, mAppEvt_AncsDsNotificationReceived_c);
        }
        else if (handle == mPeerInformation.customInfo.amsClientConfig.hRemoteCommand)
        {
            AmsClient_ProcessRcNotification(deviceId, pValue, length);
            BleApp_StateMachineHandler(deviceId, mAppEvt_AmsRcNotificationReceived_c);
        }
        else if (handle == mPeerInformation.customInfo.amsClientConfig.hEntityUpdate)
        {
            AmsClient_ProcessEuNotification(deviceId, pValue, length);
            BleApp_StateMachineHandler(deviceId, mAppEvt_AmsEuNotificationReceived_c);
        }
        else
        {
            ; /* For MISRA compliance */
        }
    }
    else
    {
        /* Process the update of the capabilities/information anyway */
        if (handle == mPeerInformation.customInfo.ancsClientConfig.hNotificationSource)
        {
            AncsClient_ProcessNsNotification(deviceId, pValue, length);
        }
        else if (handle == mPeerInformation.customInfo.ancsClientConfig.hDataSource)
        {
            AncsClient_ProcessDsNotification(deviceId, pValue, length);
        }
        else if (handle == mPeerInformation.customInfo.amsClientConfig.hRemoteCommand)
        {
            AmsClient_ProcessRcNotification(deviceId, pValue, length);
        }
        else if (handle == mPeerInformation.customInfo.amsClientConfig.hEntityUpdate)
        {
            AmsClient_ProcessEuNotification(deviceId, pValue, length);
        }
        else
        {
            ; /* For MISRA compliance */
        }
    }
}

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

static void BleApp_ServiceDiscoveryErrorHandler(void)
{
    mPeerInformation.appState = mAppIdle_c;
    BleApp_ServiceDiscoveryReset();
}

static void BleApp_ServiceDiscoveryCompleted(void)
{
    BleApp_ServiceDiscoveryReset();

    if (0U == mPeerInformation.customInfo.ancsClientConfig.hService)
    {
        shell_write("\r\nWarning: ANCS Service not found!\r\n");
    }

    if (0U == mPeerInformation.customInfo.ancsClientConfig.hNotificationSource)
    {
        shell_write("\r\nWarning: ANCS Notification Source not found!\r\n");
    }

    if (0U != mPeerInformation.customInfo.ancsClientConfig.hNotificationSourceCccd)
    {
        mPeerInformation.appState = mAppNsDescriptorSetup_c;

        if (NULL == mpDescProcBuffer)
        {
            mpDescProcBuffer = MEM_BufferAlloc(sizeof(gattAttribute_t) + gAttDefaultMtu_c);
        }
        /* Set notification bit for a CCCD descriptor. */
        uint16_t value = gCccdNotification_c;

        if (NULL == mpDescProcBuffer)
        {
            panic(0, 0, 0, 0);
        }
        else
        {
            /* Moving to the ANCS Notification Source Descriptor Setup State */
            mPeerInformation.appState = mAppNsDescriptorSetup_c;
            /* Enable notifications for the ANCS Notification Source characteristic. */
            mpDescProcBuffer->handle = mPeerInformation.customInfo.ancsClientConfig.hNotificationSourceCccd;
            mpDescProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
            mpDescProcBuffer->valueLength = (uint16_t)sizeof(uint16_t);
            mpDescProcBuffer->paValue = (uint8_t *)&value;
            (void)GattClient_WriteCharacteristicDescriptor(mPeerInformation.deviceId, mpDescProcBuffer, (uint16_t)sizeof(uint16_t),
                                                          (uint8_t *)&value);
        }
        shell_write("\r\nWriting ANCS Notification Source CCCD...\r\n");
    }
    else
    {
        shell_write("\r\nWarning: ANCS Notification Source CCCD not found!\r\n");
    }

    if (0U == mPeerInformation.customInfo.ancsClientConfig.hControlPoint)
    {
        shell_write("\r\nWarning: ANCS Control Point not found!\r\n");
    }

    if (0U == mPeerInformation.customInfo.ancsClientConfig.hDataSource)
    {
        shell_write("\r\nWarning: ANCS Data Source not found!\r\n");
    }

    if (0U == mPeerInformation.customInfo.ancsClientConfig.hDataSourceCccd)
    {
        shell_write("\r\nWarning: ANCS Data Source CCCD not found!\r\n");
    }

    if (0U == mPeerInformation.customInfo.amsClientConfig.hService)
    {
        shell_write("\r\nWarning: AMS Service not found!\r\n");
    }

    if (0U == mPeerInformation.customInfo.amsClientConfig.hRemoteCommand)
    {
        shell_write("\r\nWarning: AMS Remote Command not found!\r\n");
    }

    if (0U == mPeerInformation.customInfo.amsClientConfig.hRemoteCommandCccd)
    {
        shell_write("\r\nWarning: AMS Remote Command CCCD not found!\r\n");
    }

    if (0U == mPeerInformation.customInfo.amsClientConfig.hEntityUpdate)
    {
        shell_write("\r\nWarning: AMS Entity Update not found!\r\n");
    }

    if (0U == mPeerInformation.customInfo.amsClientConfig.hEntityUpdateCccd)
    {
        shell_write("\r\nWarning: AMS Entity Update CCCD not found!\r\n");
    }

    if (0U == mPeerInformation.customInfo.amsClientConfig.hEntityAttribute)
    {
        shell_write("\r\nWarning: AMS Entity Attribute not found!\r\n");
    }
}

static void BleApp_StateMachineHandlerExchangeMtu(deviceId_t peerDeviceId, uint8_t event)
{
    bool_t reconnected = !((mPeerInformation.customInfo.ancsClientConfig.hNotificationSource == gGattDbInvalidHandle_d)        ||
                           (mPeerInformation.customInfo.ancsClientConfig.hNotificationSourceCccd == gGattDbInvalidHandle_d)    ||
                           (mPeerInformation.customInfo.ancsClientConfig.hControlPoint == gGattDbInvalidHandle_d)              ||
                           (mPeerInformation.customInfo.ancsClientConfig.hDataSource == gGattDbInvalidHandle_d)                ||
                           (mPeerInformation.customInfo.ancsClientConfig.hDataSourceCccd == gGattDbInvalidHandle_d)            ||
                           (mPeerInformation.customInfo.amsClientConfig.hRemoteCommand == gGattDbInvalidHandle_d)              ||
                           (mPeerInformation.customInfo.amsClientConfig.hRemoteCommandCccd == gGattDbInvalidHandle_d)          ||
                           (mPeerInformation.customInfo.amsClientConfig.hEntityUpdate == gGattDbInvalidHandle_d)               ||
                           (mPeerInformation.customInfo.amsClientConfig.hEntityUpdateCccd == gGattDbInvalidHandle_d)           ||
                           (mPeerInformation.customInfo.amsClientConfig.hEntityAttribute == gGattDbInvalidHandle_d) );
    if ((event == mAppEvt_PairingComplete_c && !reconnected) ||
        (event == mAppEvt_GattProcComplete_c && reconnected))
    {
        /* Check if required service characteristic discoveries by the client app have been done
         * and change the client application state accordingly. */
        if ( !reconnected )
        {
            /* Allocate memory for Service Discovery */
            mpServiceDiscoveryBuffer = MEM_BufferAlloc(sizeof(gattService_t) * gMaxServicesCount_d);
            mpCharDiscoveryBuffer = MEM_BufferAlloc(sizeof(gattCharacteristic_t) * gMaxServiceCharCount_d);
            mpCharDescriptorBuffer = MEM_BufferAlloc(sizeof(gattAttribute_t) * gMaxCharDescriptorsCount_d);

            if ((NULL == mpServiceDiscoveryBuffer) || (NULL == mpCharDiscoveryBuffer) || (NULL == mpCharDescriptorBuffer))
            {
                panic(0, 0, 0, 0);
            }
            else
            {
                /* Moving to Primary Service Discovery State */
                mPeerInformation.appState = mAppPrimaryServiceDisc_c;

                /* Start Service Discovery*/
                (void)GattClient_DiscoverAllPrimaryServices(
                    peerDeviceId,
                    mpServiceDiscoveryBuffer,
                    gMaxServicesCount_d,
                    &mcPrimaryServices);
            }
        }
        else
        {
            /* Set notification bit for a CCCD descriptor. */
            uint16_t value = gCccdNotification_c;

            if (NULL == mpDescProcBuffer)
            {
                mpDescProcBuffer = MEM_BufferAlloc(sizeof(gattAttribute_t) + gAttDefaultMtu_c);
            }
            if (NULL != mpDescProcBuffer)
            {
                /* Moving to the ANCS Notification Source Descriptor Setup State */
                mPeerInformation.appState = mAppNsDescriptorSetup_c;
                /* Enable notifications for the ANCS Notification Source characteristic. */
                mpDescProcBuffer->handle = mPeerInformation.customInfo.ancsClientConfig.hNotificationSourceCccd;
                mpDescProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
                (void)GattClient_WriteCharacteristicDescriptor(peerDeviceId, mpDescProcBuffer, (uint16_t)sizeof(uint16_t),
                        (uint8_t *)&value);
                shell_write("\r\nWriting ANCS Notification Source CCCD...\r\n");
            }
        }
    }
    else if (event == mAppEvt_GattProcError_c)
    {
        shell_write("\r\nWarning: ATT MTU Exchange failed!\r\n");
        BleApp_ServiceDiscoveryErrorHandler();
    }
    else
    {
        ; /* Other events are not handled in this state */
    }
}

static void BleApp_StateMachineHandlerCharServiceDisc(deviceId_t peerDeviceId, uint8_t event)
{
    if (event == mAppEvt_GattProcComplete_c)
    {
        bool_t               earlyReturn = FALSE;
        gattService_t        *pCurrentService = &mpServiceDiscoveryBuffer[mCurrentServiceInDiscoveryIndex];
        gattCharacteristic_t *pCurrentChar = &pCurrentService->aCharacteristics[mCurrentCharInDiscoveryIndex];

        if (mCurrentCharInDiscoveryIndex < pCurrentService->cNumCharacteristics)
        {
            /* Find next characteristic with descriptors*/
            while (mCurrentCharInDiscoveryIndex < pCurrentService->cNumCharacteristics - 1U)
            {
                /* Check if we have handles available between adjacent characteristics */
                if (pCurrentChar->value.handle + 2U < pCurrentChar[1].value.handle)
                {
                    FLib_MemSet(mpCharDescriptorBuffer, 0, sizeof(gattAttribute_t));
                    pCurrentChar->aDescriptors = mpCharDescriptorBuffer;
                    (void)GattClient_DiscoverAllCharacteristicDescriptors(peerDeviceId,
                                                                          pCurrentChar,
                                                                          pCurrentChar[1].value.handle,
                                                                          gMaxCharDescriptorsCount_d);
                    earlyReturn = TRUE;
                    break;
                }
                else
                {
                    mCurrentCharInDiscoveryIndex++;
                    pCurrentChar = &pCurrentService->aCharacteristics[mCurrentCharInDiscoveryIndex];
                }
            }

            /* Made it to the last characteristic. Check against service end handle. */
            if (!earlyReturn && (pCurrentChar->value.handle < pCurrentService->endHandle))
            {
                FLib_MemSet(mpCharDescriptorBuffer, 0, sizeof(gattAttribute_t));
                pCurrentChar->aDescriptors = mpCharDescriptorBuffer;
               (void)GattClient_DiscoverAllCharacteristicDescriptors(peerDeviceId,
                                                                      pCurrentChar,
                                                                      pCurrentService->endHandle,
                                                                      gMaxCharDescriptorsCount_d);
                earlyReturn = TRUE;
            }
        }

        if (!earlyReturn)
        {
            /* Move on to the next service */
            mCurrentServiceInDiscoveryIndex++;

            /* Reset characteristic discovery */
            mCurrentCharInDiscoveryIndex = 0;

            if (mCurrentServiceInDiscoveryIndex < mcPrimaryServices)
            {
                /* Allocate memory for Char Discovery */
                mpServiceDiscoveryBuffer[mCurrentServiceInDiscoveryIndex].aCharacteristics = mpCharDiscoveryBuffer;

                /* Start Characteristic Discovery for current service */
                (void)GattClient_DiscoverAllCharacteristicsOfService(peerDeviceId,
                                                                     &mpServiceDiscoveryBuffer[mCurrentServiceInDiscoveryIndex],
                                                                     gMaxServiceCharCount_d);
            }
            else
            {
                shell_write("\r\nService discovery completed.\r\n");
                BleApp_ServiceDiscoveryCompleted();
            }
        }
    }
    else if (event == mAppEvt_GattProcError_c)
    {
        BleApp_ServiceDiscoveryErrorHandler();
        shell_write("\r\nService discovery error!!!\r\n");
    }
    else
    {
        ; /* Other events are not handled in this state */
    }
}

static void BleApp_StateMachineHandlerPrimaryServiceDisc(deviceId_t peerDeviceId, uint8_t event)
{
    if (event == mAppEvt_GattProcComplete_c)
    {
        /* We found at least one service */
        if (mcPrimaryServices > 0U)
        {
            /* Moving to Characteristic Discovery State*/
            mPeerInformation.appState = mAppCharServiceDisc_c;

            /* Start characteristic discovery with first service*/
            mCurrentServiceInDiscoveryIndex = 0;
            mCurrentCharInDiscoveryIndex = 0;

            mpServiceDiscoveryBuffer->aCharacteristics = mpCharDiscoveryBuffer;

            /* Start Characteristic Discovery for current service */
            (void)GattClient_DiscoverAllCharacteristicsOfService(peerDeviceId,
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
        ; /* Other events are not handled in this state */
    }
}

static void BleApp_StateMachineHandlerNsDescriptorSetup(deviceId_t peerDeviceId, uint8_t event)
{
    if (event == mAppEvt_GattProcComplete_c)
    {
        if (mPeerInformation.customInfo.ancsClientConfig.hNotificationSourceCccd > 0U)
        {
            /* Set notification bit for a CCCD descriptor.  */
            uint16_t value = gCccdNotification_c;

            shell_write("\r\nANCS Notification Source CCCD written successfully.\r\n");

            if (NULL == mpDescProcBuffer)
            {
                panic(0, 0, 0, 0);
            }
            else
            {
                /* Moving to the ANCS Data Source Descriptor Setup State */
                mPeerInformation.appState = mAppDsDescriptorSetup_c;
                /* Enable notifications for the ANCS Data Source characteristic. */
                mpDescProcBuffer->handle = mPeerInformation.customInfo.ancsClientConfig.hDataSourceCccd;
                mpDescProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
                (void)GattClient_WriteCharacteristicDescriptor(peerDeviceId, mpDescProcBuffer, (uint16_t)sizeof(uint16_t),
                                                              (uint8_t *)&value);
                shell_write("\r\nWriting ANCS Data Source CCCD...\r\n");
            }
        }
    }
    else if (event == mAppEvt_GattProcError_c)
    {
        shell_write("\r\nWarning: Could not write ANCS Notification Source CCCD.\r\n");
    }
    else if (event == mAppEvt_PairingComplete_c)
    {
        /* Continue after pairing is complete */

        /* Set notification bit for a CCCD descriptor.  */
        uint16_t value = gCccdNotification_c;

        if (NULL == mpDescProcBuffer)
        {
            panic(0, 0, 0, 0);
        }
        else
        {
            /* Enable notifications for the ANCS Notification Source characteristic. */
            mpDescProcBuffer->handle = mPeerInformation.customInfo.ancsClientConfig.hNotificationSourceCccd;
            mpDescProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
            (void)GattClient_WriteCharacteristicDescriptor(peerDeviceId, mpDescProcBuffer, (uint16_t)sizeof(uint16_t),
                                                          (uint8_t *)&value);
            shell_write("\r\nWriting ANCS Notification Source CCCD...\r\n");
        }
    }
    else
    {
        ; /* Other events are not handled in this state */
    }
}

static void BleApp_StateMachineHandlerDsDescriptorSetup(deviceId_t peerDeviceId, uint8_t event)
{
    if (event == mAppEvt_GattProcComplete_c)
    {
        if (mPeerInformation.customInfo.ancsClientConfig.hDataSourceCccd > 0U)
        {
            /* Set notification bit for a CCCD descriptor.  */
            uint16_t value = gCccdNotification_c;

            shell_write("\r\nANCS Data Source CCCD written successfully.\r\n");

            if (NULL == mpDescProcBuffer)
            {
                panic(0, 0, 0, 0);
            }
            else
            {
                /* Moving to the AMS Remote Command Descriptor Setup State */
                mPeerInformation.appState = mAppRcDescriptorSetup_c;
                /* Enable notifications for the AMS Remote Command characteristic. */
                mpDescProcBuffer->handle = mPeerInformation.customInfo.amsClientConfig.hRemoteCommandCccd;
                mpDescProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
                (void)GattClient_WriteCharacteristicDescriptor(peerDeviceId, mpDescProcBuffer, (uint16_t)sizeof(uint16_t),
                                                              (uint8_t *)&value);
                shell_write("\r\nWriting AMS Remote Command CCCD...\r\n");
            }
        }
    }
    else if (event == mAppEvt_GattProcError_c)
    {
        shell_write("\r\nWarning: Could not write ANCS Data Source CCCD.\r\n");
    }
    else if (event == mAppEvt_PairingComplete_c)
    {
        /* Continue after pairing is complete */

        /* Set notification bit for a CCCD descriptor.  */
        uint16_t value = gCccdNotification_c;

        if (NULL == mpDescProcBuffer)
        {
            panic(0, 0, 0, 0);
        }
        else
        {
            /* Enable notifications for the ANCS Notification Source characteristic. */
            mpDescProcBuffer->handle = mPeerInformation.customInfo.ancsClientConfig.hNotificationSourceCccd;
            mpDescProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
            (void)GattClient_WriteCharacteristicDescriptor(peerDeviceId, mpDescProcBuffer, (uint16_t)sizeof(uint16_t),
                                                          (uint8_t *)&value);
            shell_write("\r\nWriting ANCS Notification Source CCCD...\r\n");
        }
    }
    else
    {
        ; /* Other events are not handled in this state */
    }
}

static void BleApp_StateMachineHandlerRcDescriptorSetup(deviceId_t peerDeviceId, uint8_t event)
{
    if (event == mAppEvt_GattProcComplete_c)
    {
        if (mPeerInformation.customInfo.amsClientConfig.hRemoteCommandCccd > 0U)
        {
            /* Set notification bit for a CCCD descriptor.  */
            uint16_t value = gCccdNotification_c;

            shell_write("\r\nAMS Remote Command CCCD written successfully.\r\n");

            if (NULL == mpDescProcBuffer)
            {
                panic(0, 0, 0, 0);
            }
            else
            {   
                /* Moving to the AMS Entity Update Descriptor Setup State */
                mPeerInformation.appState = mAppEuDescriptorSetup_c;
                /* Enable notifications for the AMS Entity Update characteristic. */
                mpDescProcBuffer->handle = mPeerInformation.customInfo.amsClientConfig.hEntityUpdateCccd;
                mpDescProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
                (void)GattClient_WriteCharacteristicDescriptor(peerDeviceId, mpDescProcBuffer, (uint16_t)sizeof(uint16_t),
                                                              (uint8_t *)&value);
                shell_write("\r\nWriting AMS Entity Update CCCD...\r\n");
            }
        }
    }
    else if (event == mAppEvt_GattProcError_c)
    {
        shell_write("\r\nWarning: Could not write AMS Remote Command CCCD.\r\n");
    }
    else if (event == mAppEvt_PairingComplete_c)
    {
        /* Continue after pairing is complete */

        /* Set notification bit for a CCCD descriptor.  */
        uint16_t value = gCccdNotification_c;

        if (NULL == mpDescProcBuffer)
        {
            panic(0, 0, 0, 0);
        }
        else
        {
            /* Enable notifications for the ANCS Notification Source characteristic. */
            mpDescProcBuffer->handle = mPeerInformation.customInfo.ancsClientConfig.hNotificationSourceCccd;
            mpDescProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
            (void)GattClient_WriteCharacteristicDescriptor(peerDeviceId, mpDescProcBuffer, (uint16_t)sizeof(uint16_t),
                                                          (uint8_t *)&value);
            shell_write("\r\nWriting ANCS Notification Source CCCD...\r\n");
        }
    }
    else
    {
        ; /* Other events are not handled in this state */
    }
}

static void BleApp_StateMachineHandlerEuDescriptorSetup(deviceId_t peerDeviceId, uint8_t event)
{
    if (event == mAppEvt_GattProcComplete_c)
    {
        if (mPeerInformation.customInfo.amsClientConfig.hEntityUpdateCccd > 0U)
        {
            shell_write("\r\nAMS Entity Update CCCD written successfully.\r\n");

            if (NULL == mpDescProcBuffer)
            {
                panic(0, 0, 0, 0);
            }
            else
            {
                /* Moving to the AMS Entity Update Descriptor Setup State */
                mPeerInformation.appState = mAppEuConfigTrackSetup_c;

                /* Prepare and send a Get Notification Attributes command requesting the Application ID */
                uint8_t amsTrackAttributeSubscriptions[gAmsEntityUpdateIdFieldLength_c + gAmsTrackNumAttributeId_c * gAmsTrackAttributeIdFieldLength_c] = {0};

                /* EntityID + AttributeID 1 + AttributeID N*/
                amsTrackAttributeSubscriptions[0] = (uint8_t)gAmsEntityIdTrack_c;
                amsTrackAttributeSubscriptions[1] = (uint8_t)gAmsTrackAttributeIdArtist_c;
                amsTrackAttributeSubscriptions[2] = (uint8_t)gAmsTrackAttributeIdAlbum_c;
                amsTrackAttributeSubscriptions[3] = (uint8_t)gAmsTrackAttributeIdTitle_c;
                amsTrackAttributeSubscriptions[4] = (uint8_t)gAmsTrackAttributeIdDuration_c;

                AmsClient_SendCommandToAmsServer(mPeerInformation.deviceId,
                                                 mPeerInformation.customInfo.amsClientConfig.hEntityUpdate,
                                                 amsTrackAttributeSubscriptions,
                                                 (uint16_t)sizeof(amsTrackAttributeSubscriptions));
                shell_write("\r\nWriting AMS Entity Update Track Subscription...\r\n");
            }
        }
    }
    else if (event == mAppEvt_GattProcError_c)
    {
        shell_write("\r\nWarning: Could not write AMS Entity Update CCCD.\r\n");
    }
    else if (event == mAppEvt_PairingComplete_c)
    {
        /* Continue after pairing is complete */

        /* Set notification bit for a CCCD descriptor.  */
        uint16_t value = gCccdNotification_c;

        if (NULL == mpDescProcBuffer)
        {
            panic(0, 0, 0, 0);
        }
        else
        {
            /* Enable notifications for the ANCS Notification Source characteristic. */
            mpDescProcBuffer->handle = mPeerInformation.customInfo.ancsClientConfig.hNotificationSourceCccd;
            mpDescProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
            (void)GattClient_WriteCharacteristicDescriptor(peerDeviceId, mpDescProcBuffer, (uint16_t)sizeof(uint16_t),
                                                          (uint8_t *)&value);
            shell_write("\r\nWriting ANCS Notification Source CCCD...\r\n");
        }
    }
    else
    {
        ; /* Other events are not handled in this state */
    }
}

void BleApp_StateMachineHandler(deviceId_t peerDeviceId, uint8_t event)
{
    switch (mPeerInformation.appState)
    {
        case mAppIdle_c:
        {
            if (event == mAppEvt_PeerConnected_c)
            {
                /* Moving to Exchange MTU State */
                mCanSendToServer = FALSE; /* After reconnect the client can not send to server before both NS and DS descriptors are written */
                mPeerInformation.appState = mAppExchangeMtu_c;
               (void)GattClient_ExchangeMtu(peerDeviceId, gAttMaxMtu_c);
            }
        }
        break;

        case mAppExchangeMtu_c:
        {
            BleApp_StateMachineHandlerExchangeMtu(peerDeviceId, event);
        }
        break;

        case mAppPrimaryServiceDisc_c:
        {
            BleApp_StateMachineHandlerPrimaryServiceDisc(peerDeviceId, event);
        }
        break;

        case mAppCharServiceDisc_c:
        {
            BleApp_StateMachineHandlerCharServiceDisc(peerDeviceId, event);
        }
        break;

        case mAppNsDescriptorSetup_c:
        {
            BleApp_StateMachineHandlerNsDescriptorSetup(peerDeviceId, event);
        }
        break;

        case mAppDsDescriptorSetup_c:
        {
            BleApp_StateMachineHandlerDsDescriptorSetup(peerDeviceId, event);
        }
        break;

        case mAppRcDescriptorSetup_c:
        {
            BleApp_StateMachineHandlerRcDescriptorSetup(peerDeviceId, event);
        }
        break;

        case mAppEuDescriptorSetup_c:
        {
            BleApp_StateMachineHandlerEuDescriptorSetup(peerDeviceId, event);
        }
        break;

        case mAppEuConfigTrackSetup_c:
        {
            if (event == mAppEvt_GattProcComplete_c)
            {
                if (mPeerInformation.customInfo.amsClientConfig.hEntityUpdateCccd > 0U)
                {
                    /* Moving to Running State*/
                    mPeerInformation.appState = mAppRunning_c;

                    shell_write("\r\nAMS Entity Update Track Subscription written successfully.\r\n");
                    mCanSendToServer = TRUE;

                    AncsClient_SendGetNotificationOrApplicationAttribute();
                    if(mFirstRunningBeforeReconnect == 0U)
                    {
                        mFirstRunningBeforeReconnect = 1U;
                        (void)TM_InstallCallback((timer_handle_t)mAllowNotificationsTimerId, AllowNotificationsTimerCallback, NULL);
                        (void)TM_Start((timer_handle_t)mAllowNotificationsTimerId, (uint16_t)kTimerModeLowPowerTimer | (uint16_t)kTimerModeSetSecondTimer, mPhoneAllowNotificationsInterval_c);
                    }
                }
            }
            else if (event == mAppEvt_GattProcError_c)
            {
                shell_write("\r\nWarning: Could not write AMS Track Subscription CCCD.\r\n");
            }
            else
            {
                ; /* Other events are not handled in this state */
            }
            break;
        }

        case mAppRunning_c:
        {
            if (event == mAppEvt_GattProcComplete_c)
            {
                if(mFirstRunningBeforeReconnect < mAncsMaxWriteCCCDProcedures_c)
                {
                    mFirstRunningBeforeReconnect++;
                    /* After the unsubscribe there is the need to subscribe to the NS characteristic again */
                    uint16_t value = gCccdNotification_c;
                    if (NULL == mpDescProcBuffer)
                    {
                        panic(0, 0, 0, 0);
                    }
                    else
                    {
                        /* Enable notifications for the ANCS Notification Source characteristic. */
                        mpDescProcBuffer->handle = mPeerInformation.customInfo.ancsClientConfig.hNotificationSourceCccd;
                        mpDescProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
                       (void)GattClient_WriteCharacteristicDescriptor(peerDeviceId, mpDescProcBuffer, (uint16_t)sizeof(uint16_t), (uint8_t *)&value);
                    }

                    (void)TM_Stop((timer_handle_t)mAllowNotificationsTimerId);
                }
                else
                {
                    /* Write data in NVM */
                    (void)Gap_SaveCustomPeerInformation(mPeerInformation.deviceId,
                                                       (void *) &mPeerInformation.customInfo, 0,
                                                        (uint16_t)sizeof(appCustomInfo_t));
                }
            }
            else if (event == mAppEvt_AncsNsNotificationReceived_c)
            {
                if(mReceivedNotificationsAndNeedToPrint == 0U)
                {
                    AncsClient_DisplayNotifications();
                }
            }
            else if (event == mAppEvt_AncsDsNotificationReceived_c)
            {
                if(mReceivedNotificationsAndNeedToPrint == 0U)
                {
                    AncsClient_DisplayNotifications();
                }
            }
            else if (event == mAppEvt_AmsRcNotificationReceived_c)
            {
                if(mReceivedNotificationsAndNeedToPrint == 0U)
                {
                    AncsClient_DisplayNotifications();
                }
            }
            else if (event == mAppEvt_AmsEuNotificationReceived_c)
            {
                if(mReceivedNotificationsAndNeedToPrint == 0U)
                {
                    AncsClient_DisplayNotifications();
                }
            }
            else
            {
                ; /* Other events are not handled in this state */
            }
        }
        break;

        default:
            ; /* For MISRA compliance */
            break;
    }
}

/*! *********************************************************************************
* \brief    Handles disconnections from the ANCS Server.
*
* \param[in]    deviceId    Device which was disconnected.
********************************************************************************** */
static void AncsClient_HandleDisconnectionEvent(deviceId_t deviceId)
{
    (void) deviceId;

    AncsClient_ResetNotificationData();
}

/*! *********************************************************************************
* \brief    Resets the application notification data.
*
********************************************************************************** */
static void AncsClient_ResetNotificationData(void)
{
    uint32_t i;

    ancsClientData.lastCommandSentToAncsProvider = gAncsCmdIdNoCommand_c;
    ancsClientData.notificationDataChanged = FALSE;

    for (i = 0; i < mMaxDisplayNotifications_c; i++)
    {
        /* Invalidate all notification entries and the associated application names.
         * If the information is sensitive all other fields need to be cleared. */
        ancsClientData.notifications[i].slotUsed = FALSE;
        ancsClientData.notifications[i].appNameValid = FALSE;
        ancsClientData.notifications[i].appName[0] = 0; /* Invalidate app name information */
    }

    amsClientData.amsAvailableCommandsBitmask = 0x0000U;
    amsClientData.amsInvalidCommand = FALSE;
    FLib_MemSet(amsClientData.euTrackArtist, 0, gAttMaxDataSize_d(gAttMaxMtu_c));
    FLib_MemSet(amsClientData.euTrackAlbum, 0, gAttMaxDataSize_d(gAttMaxMtu_c));
    FLib_MemSet(amsClientData.euTrackTitle, 0, gAttMaxDataSize_d(gAttMaxMtu_c));
    FLib_MemSet(amsClientData.euTrackDuration, 0, gAttMaxDataSize_d(gAttMaxMtu_c));
}

/*! *********************************************************************************
* \brief    Handles BLE notifications from the ANCS Notification Source Characteristic
*           on the ANCS Server.
*
* \param[in]    deviceId        Id of the device from which the notification was received
* \param[in]    pNsData         Pointer to the ANCS Notification Source BLE notification payload
* \param[in]    nsDataLength    Length of the ANCS Notification Source BLE notification payload
********************************************************************************** */
static void AncsClient_ProcessNsNotification
(
    deviceId_t   deviceId,
    uint8_t     *pNsData,
    uint16_t     nsDataLength
)
{
    /* Check if this is the first notification after a previous print and keep this in memory */
    if(mReceivedNotificationsAndNeedToPrint == 0U)
    {
        mReceivedNotificationsAndNeedToPrint = 1U;
    }

    pPayloadPacketTemp_t pPayloadPacketTemp;
    pPayloadPacketTemp.pNsDataTemp = pNsData;
    ancsNsPacket_t     *pNsPacket = pPayloadPacketTemp.pNsPacketTemp;
    ancsNsEvent_t       nsEvent;

    if (nsDataLength <= gAncsNsPacketMaxLength)
    {
        uint32_t    i;
        uint32_t     notifUidIndex               = mMaxDisplayNotifications_c;
        uint32_t     firstFreeSlotIndex          = mMaxDisplayNotifications_c;
        uint32_t     lowestNotifUidSlotIndex     = mMaxDisplayNotifications_c;

        bool_t      getNotificationAttributes   = FALSE;

        nsEvent.eventId = (ancsEventId_t)(pNsPacket->eventId);
        nsEvent.eventFlags = pNsPacket->eventFlags;
        nsEvent.categoryId = (ancsCatId_t)(pNsPacket->categoryId);
        nsEvent.categoryCount = pNsPacket->categoryCount;
        (void)nsEvent.categoryCount;
        FLib_MemCpy((uint8_t *)(&(nsEvent.notifUid)), (uint8_t *)(&(pNsPacket->notifUid)), sizeof(nsEvent.notifUid));

        for (i = 0; i < mMaxDisplayNotifications_c; i++)
        {
            /* Walk through the existing notifications table and determine the index of the first free slot if available,
             * the index of the oldest notification and the index of a notification matching the received UID. */
            if (ancsClientData.notifications[i].slotUsed == TRUE)
            {
                if (nsEvent.notifUid == ancsClientData.notifications[i].notifUid)
                {
                    notifUidIndex = i;
                }

                if (lowestNotifUidSlotIndex < mMaxDisplayNotifications_c)
                {
                    if (ancsClientData.notifications[i].notifUid < ancsClientData.notifications[lowestNotifUidSlotIndex].notifUid)
                    {
                        lowestNotifUidSlotIndex = i;
                    }
                }
                else
                {
                    lowestNotifUidSlotIndex = i;
                }
            }
            else
            {
                if (firstFreeSlotIndex == mMaxDisplayNotifications_c)
                {
                    firstFreeSlotIndex = i;
                }
            }
        }

        /* Check if this is and existing notification which needs to be updated or it is a
         * new notification going to a free slot or overwriting an old notification
         * and act accordingly. */
        if (notifUidIndex < mMaxDisplayNotifications_c)
        {
            /* Update existing notification */
            if ((nsEvent.eventId == gAncsEventIdNotificationAdded_c) || (nsEvent.eventId == gAncsEventIdNotificationModified_c))
            {
                ancsClientData.notifications[notifUidIndex].notifCat = nsEvent.categoryId;
                ancsClientData.notifications[notifUidIndex].notifFlags = (ancsEventFlag_t)nsEvent.eventFlags;
                ancsClientData.notifications[notifUidIndex].appNameValid = FALSE;
                ancsClientData.notifications[notifUidIndex].appName[0] = 0; /* Invalidate the application display name */

                ancsClientData.notificationDataChanged = TRUE;

                /* Read more notification data from the server for this Uid. */
                getNotificationAttributes = TRUE;
            }
            else if (nsEvent.eventId == gAncsEventIdNotificationRemoved_c)
            {
                ancsClientData.notifications[notifUidIndex].slotUsed = FALSE;
                ancsClientData.notifications[notifUidIndex].appNameValid = FALSE;
                ancsClientData.notifications[notifUidIndex].appName[0] = 0; /* Invalidate the application display name */

                ancsClientData.notificationDataChanged = TRUE;
            }
            else
            {
                ; /* For MISRA compliance */
            }
        }
        else
        {
            /* Only update the notifications table if this notification exists on the ANCS Client
             * and it is not an old notification being removed from the ANCS Server. */
            if (nsEvent.eventId != gAncsEventIdNotificationRemoved_c)
            {
                if (firstFreeSlotIndex < mMaxDisplayNotifications_c)
                {
                    /* Fill the free notification slot. */
                    notifUidIndex = firstFreeSlotIndex;
                }
                else
                {
                    /* Put the in the place of the oldest notification. */
                    notifUidIndex = lowestNotifUidSlotIndex;
                }

                ancsClientData.notifications[notifUidIndex].slotUsed = TRUE;
                ancsClientData.notifications[notifUidIndex].notifCat = nsEvent.categoryId;
                ancsClientData.notifications[notifUidIndex].notifFlags = (ancsEventFlag_t)nsEvent.eventFlags;
                ancsClientData.notifications[notifUidIndex].notifUid = nsEvent.notifUid;
                ancsClientData.notifications[notifUidIndex].appNameValid = FALSE;
                ancsClientData.notifications[notifUidIndex].appName[0] = 0; /* Invalidate the application display name */

                ancsClientData.notificationDataChanged = TRUE;

                /* Read more notification data from the server for this Uid. */
                getNotificationAttributes = TRUE;
            }
        }

         if (getNotificationAttributes == TRUE)
         {
            /* Mark that this is a new notification and there is the need to get more info for it */
            ancsClientData.notifications[notifUidIndex].needGetNotifAttribute = TRUE;
            if(mCanSendToServer == TRUE)
            {
                AncsClient_SendGetNotificationOrApplicationAttribute();
            }
        }
        else
        {
            /* There is just an update of an existing notification */
            AncsClient_DisplayNotifications();
        }
    }
    else
    {
        shell_write("\r\nWarning: Received ANCS NS notification with unexpected length!\r\n");
    }
}

/*! *********************************************************************************
* \brief    Handles disconnections from the ANCS Server.
*
* \param[in]    ancsServerDevId     ANCS Server device to send the command to.
* \param[in]    pCommand            Pointer to the command payload.
* \param[in]    cmdLength           Command payload length.
********************************************************************************** */
static void AncsClient_SendCommandToAncsServer
(
    deviceId_t  ancsServerDevId,
    void       *pCommand,
    uint16_t    cmdLength
)
{
    /* GATT Characteristic to be written - ANCS Control Point */
    gattCharacteristic_t    ancsCtrlPointChar;
    bleResult_t             bleResult;

    /* Only the value handle element of this structure is relevant for this operation. */
    ancsCtrlPointChar.value.handle = mPeerInformation.customInfo.ancsClientConfig.hControlPoint;
    ancsCtrlPointChar.value.valueLength = 0;
    ancsCtrlPointChar.cNumDescriptors = 0;
    ancsCtrlPointChar.aDescriptors = NULL;

    bleResult = GattClient_SimpleCharacteristicWrite(ancsServerDevId,
                                                     &ancsCtrlPointChar,
                                                     cmdLength,
                                                     pCommand);

    if (gBleSuccess_c == bleResult)
    {
        ancsClientData.lastCommandSentToAncsProvider = (ancsComdId_t)(*((uint8_t *)pCommand));
    }
    else
    {
        /*! A BLE error has occurred - Disconnect */
        (void)Gap_Disconnect(ancsServerDevId);
    }
}

/*! *********************************************************************************
* \brief    Handles sending commands to AMS Server.
*
* \param[in]    amsServerDevId      AMS Server device to send the command to.
* \param[in]    amsCharHandle       AMS Characteristic Handle to be used.
* \param[in]    pCommand            Pointer to the command payload.
* \param[in]    cmdLength           Command payload length.
********************************************************************************** */
static void AmsClient_SendCommandToAmsServer
(
    deviceId_t  amsServerDevId,
    uint16_t    amsCharHandle,
    void        *pCommand,
    uint16_t    cmdLength
)
{
    /* GATT Characteristic to be written - AMS*/
    gattCharacteristic_t    amsChar;
    bleResult_t             bleResult;

    /* Only the value handle element of this structure is relevant for this operation. */
    amsChar.value.handle = amsCharHandle;
    amsChar.value.valueLength = 0;
    amsChar.cNumDescriptors = 0;
    amsChar.aDescriptors = NULL;

    bleResult = GattClient_SimpleCharacteristicWrite(amsServerDevId,
                                                     &amsChar,
                                                     cmdLength,
                                                     pCommand);

    if (gBleSuccess_c == bleResult)
    {
        ancsClientData.lastCommandSentToAncsProvider = (ancsComdId_t)(*((uint8_t *)pCommand));
    }
    else
    {
        /*! A BLE error has occurred - Disconnect */
        (void)Gap_Disconnect(amsServerDevId);
    }
}

/*! *********************************************************************************
* \brief    Handles BLE notification attributes from the ANCS Data Source Characteristic
*           on the ANCS Server.
*
* \param[in]    deviceId        Id of the device from which the notification was received
* \param[in]    pDsData         Pointer to the ANCS Data Source BLE notification payload
* \param[in]    dsDataLength    Length of the ANCS Data Source BLE notification payload
********************************************************************************** */
static void AncsClient_ProcessDsNotifAttributes
(
    deviceId_t   deviceId,
    uint8_t     *pDsData,
    uint16_t     dsDataLength
)
{
    uint32_t    i;
    uint32_t    notifUid            = 0xFFFF; /* Initialize to an unlikely notification Uid */
    uint8_t     notifUidIndex       = mMaxDisplayNotifications_c;
    bool_t      getAppAttributes    = FALSE;

    if (dsDataLength >= gAncsCmdIdFieldLength_c + gAncsNotifUidFieldLength_c)
    {
        /* Get the Notification Uid */
        FLib_MemCpy((uint8_t *)(&(notifUid)), &(pDsData[1]), sizeof(notifUid));

        /* Search through the notifications table to find the entry corresponding to the received notification Uid */
        for (i = 0; i < mMaxDisplayNotifications_c; i++)
        {
            if (ancsClientData.notifications[i].slotUsed == TRUE)
            {
                if (notifUid == ancsClientData.notifications[i].notifUid)
                {
                    notifUidIndex = (uint8_t)i;
                    break;
                }
            }
        }

        /* If a corresponding notification Uid was found in the table, perform required actions.
         * Otherwise just ignore the message, the corresponding notification must have been deleted. */
        if (notifUidIndex < mMaxDisplayNotifications_c)
        {
            /* Mark that this notification does not need to send get notification attribute */
            ancsClientData.notifications[notifUidIndex].needGetNotifAttribute = FALSE;

            uint16_t    appIdLength = 0;
            uint8_t    *pAppId      = NULL;

            uint32_t    dsDataIdx   = gAncsCmdIdFieldLength_c + gAncsNotifUidFieldLength_c; /* Initialize the DS data index at the start of the attributes */

            while (dsDataIdx < dsDataLength)
            {
                ancsNotifAttrId_t   notifAttrId         = gAncsNotifAttrIdInvalid;
                uint16_t            notifAttrLength     = 0;
                uint8_t            *pNotifAttrPayload   = NULL;

                notifAttrId = (ancsNotifAttrId_t)(pDsData[dsDataIdx]); /* At the current index should be an attribute ID */
                dsDataIdx = dsDataIdx + gAncsAttrIdFieldLength_c;

                /* Check if the payload contains the Attribute length field. */
                if (dsDataIdx + gAncsAttrLengthFieldLength_c <= dsDataLength)
                {
                    FLib_MemCpy((uint8_t *)(&(notifAttrLength)), &(pDsData[dsDataIdx]), sizeof(notifAttrLength));
                    dsDataIdx = dsDataIdx + gAncsAttrLengthFieldLength_c;
                }
                else
                {
                    shell_write("\r\nWarning: Received ANCS DS notification (Notif Attr) with unexpected length (Notif Attr Length)!\r\n");
                    break; /* Exit the while loop, the packet is malformed */
                }

                /* Check if the payload contains the Attribute payload field. */
                if (dsDataIdx + notifAttrLength <= dsDataLength)
                {
                    pNotifAttrPayload = &(pDsData[dsDataIdx]);
                    dsDataIdx = dsDataIdx + notifAttrLength;
                }
                else
                {
                    shell_write("\r\nWarning: Received ANCS DS notification (Notif Attr) with unexpected length (Notif Attr Payload)!\r\n");
                    break; /* Exit the while loop, the packet is malformed */
                }

                /* Process the notification attribute if it is expected. */
                if (notifAttrId == gAncsNotifAttrIdAppIdentifier_c) /* Application identifier */
                {
                    /* Save the application identifier into the application name buffer, at least the part that fits, zero terminated. */
                    if (notifAttrLength < mMaxNotifAppNameDisplayLength_c)
                    {
                        FLib_MemCpy((uint8_t *)(ancsClientData.notifications[notifUidIndex].appName),
                                    pNotifAttrPayload,
                                    notifAttrLength);
                        ancsClientData.notifications[notifUidIndex].appName[notifAttrLength] = 0x00;
                    }
                    else
                    {
                        FLib_MemCpy((uint8_t *)(ancsClientData.notifications[notifUidIndex].appName),
                                    pNotifAttrPayload,
                                    mMaxNotifAppNameDisplayLength_c - 1U);
                        ancsClientData.notifications[notifUidIndex].appName[mMaxNotifAppNameDisplayLength_c - 1U] = 0x00;
                    }

                    appIdLength = notifAttrLength;
                    pAppId      = pNotifAttrPayload;

                    /* Read more app data from the server. */
                    getAppAttributes = TRUE;
                }
                else
                {
                    shell_write("\r\nWarning: Unhandled Notification Attribute:");
                    shell_write(" 0x");
                    shell_writeHex((uint8_t)notifAttrId);
                    shell_write("\r\n");
                }
            }

            /* Read more app data from the server. */
            if (getAppAttributes == TRUE)
            {
                /* Prepare and send a Get Application Attributes command requesting the Application ID */
                if (appIdLength > 0U)
                {
                    ancsClientData.notifications[notifUidIndex].needGetAppAttribute = TRUE;
                    ancsClientData.notifications[notifUidIndex].appIdLength = appIdLength;
                    ancsClientData.notifications[notifUidIndex].pAppId = pAppId;
                    AncsClient_SendGetNotificationOrApplicationAttribute();
                }
            }
        }
    }
    else
    {
        shell_write("\r\nWarning: Received ANCS DS notification (Notif Attr) with unexpected length (CmdID + Notif UId)!\r\n");
    }
}

/*! *********************************************************************************
* \brief    Handles BLE notification application information from the ANCS Data Source
*           Characteristic on the ANCS Server.
*
* \param[in]    pDsData         Pointer to the ANCS Data Source BLE notification payload
* \param[in]    dsDataLength    Length of the ANCS Data Source BLE notification payload
********************************************************************************** */
static void AncsClient_ProcessDsNotifAppInfo(uint8_t     *pDsData,
        uint16_t     dsDataLength)
{
    uint32_t    i;
    uint16_t    rcvdAppIdLength = 0U;
    uint8_t    *pRcvdAppId      = NULL;
    mCanSendToServer = TRUE;

    if (dsDataLength < gAncsCmdIdFieldLength_c)
    {
        shell_write("\r\nWarning: Received ANCS DS notification (App Attr) with unexpected length (CmdID)!\r\n");
    }
    else
    {
        /* Get the App Id NULL terminated string in the received notification if it exists. */
        i = gAncsCmdIdFieldLength_c;

        while (i < dsDataLength)
        {
            if (pDsData[i] == 0x00U)
            {
                rcvdAppIdLength = (uint16_t)(i - gAncsCmdIdFieldLength_c); /* Do not count the just found NULL string terminator. */
                pRcvdAppId = (uint8_t *)(&(pDsData[gAncsCmdIdFieldLength_c]));
                break; /* Break the while loop, the App Id length may have been identified. */
            }

            i = i + 1U;
        }

        if (rcvdAppIdLength == 0U)
        {
            shell_write("\r\nWarning: Received ANCS DS notification (App Attr) with invalid App Id!\r\n");
        }
        else
        {
            /* Look for the App Id in the notifications table - only entries with an invalid application name field */
            for (i = 0; i < mMaxDisplayNotifications_c; i++)
            {
                if ((ancsClientData.notifications[i].slotUsed == TRUE) && (ancsClientData.notifications[i].appNameValid == FALSE))
                {
                    bool_t      cmpMatch    = FALSE;
                    uint32_t    dsDataIdx   = 0;

                    if (rcvdAppIdLength < mMaxNotifAppNameDisplayLength_c)
                    {
                        cmpMatch = FLib_MemCmp((uint8_t *)(ancsClientData.notifications[i].appName),
                                               pRcvdAppId,
                                               rcvdAppIdLength);
                    }
                    else
                    {
                        cmpMatch = FLib_MemCmp((uint8_t *)(ancsClientData.notifications[i].appName),
                                               pRcvdAppId,
                                               mMaxNotifAppNameDisplayLength_c - 1U);
                    }

                    if (cmpMatch == FALSE)
                    {
                        continue; /* Continue the for loop over the notifications table, the entry does not match the received App Id */
                    }

                    /* Initialize the DS data index at the start of the attributes.
                     * +1 for the NULL string terminator which is not counted in the App Id length */
                    dsDataIdx = gAncsCmdIdFieldLength_c + (uint32_t)rcvdAppIdLength + 1U;

                    while (dsDataIdx < dsDataLength)
                    {
                        ancsAppAttrId_t appAttrId       = gAncsAppAttrIdInvalid_c;
                        uint16_t        appAttrLength   = 0;
                        uint8_t        *pAppAttrPayload = NULL;

                        appAttrId = (ancsAppAttrId_t)(pDsData[dsDataIdx]); /* At the current index should be an app ID */
                        dsDataIdx = dsDataIdx + gAncsAttrIdFieldLength_c;

                        /* Check if the payload contains the Attribute length field. */
                        if (dsDataIdx + gAncsAttrLengthFieldLength_c <= dsDataLength)
                        {
                            FLib_MemCpy((uint8_t *)(&(appAttrLength)), &(pDsData[dsDataIdx]), sizeof(appAttrLength));
                            dsDataIdx = dsDataIdx + gAncsAttrLengthFieldLength_c;
                        }
                        else
                        {
                            shell_write("\r\nWarning: Received ANCS DS notification (App Attr) with unexpected length (App Attr Length)!\r\n");
                            break; /* Exit the while loop, the packet is malformed */
                        }

                        /* Check if the payload contains the Attribute payload field. */
                        if (dsDataIdx + appAttrLength <= dsDataLength)
                        {
                            pAppAttrPayload = &(pDsData[dsDataIdx]);
                            dsDataIdx = dsDataIdx + appAttrLength;
                        }
                        else
                        {
                            shell_write("\r\nWarning: Received ANCS DS notification (App Attr) with unexpected length (App Attr Payload)!\r\n");
                            break; /* Exit the while loop, the packet is malformed */
                        }

                        /* Process the notification attribute if it is expected. */
                        if (appAttrId == gAncsAppAttrIdDisplayName_c) /* Application display name */
                        {
                            /* Save the application display name into the application name buffer, at least the part that fits, zero terminated. */
                            if (appAttrLength < mMaxNotifAppNameDisplayLength_c)
                            {
                                FLib_MemCpy((uint8_t *)(ancsClientData.notifications[i].appName),
                                            pAppAttrPayload,
                                            appAttrLength);
                                ancsClientData.notifications[i].appName[appAttrLength] = 0x00;
                            }
                            else
                            {
                                FLib_MemCpy((uint8_t *)(ancsClientData.notifications[i].appName),
                                            pAppAttrPayload,
                                            mMaxNotifAppNameDisplayLength_c - 1U);
                                ancsClientData.notifications[i].appName[mMaxNotifAppNameDisplayLength_c - 1U] = 0x00;
                            }

                            ancsClientData.notifications[i].appNameValid = TRUE;
                            ancsClientData.notificationDataChanged = TRUE;

                            /* Mark that there is not the need to get App Attribute for this index any more */
                            ancsClientData.notifications[i].needGetAppAttribute = FALSE;
                        }
                        else
                        {
                            shell_write("\r\nWarning: Unhandled Application Attribute:");
                            shell_write(" 0x");
                            shell_writeHex((uint8_t)appAttrId);
                            shell_write("\r\n");
                        }

                    } /* while the end of the attributes was not reached */

                    if (dsDataIdx < dsDataLength)
                    {
                        /* The parsing of the packet ended prematurely. It must be malformed. Break the for loop. */
                        break;
                    }

                } /* if notification slot used and app name is not valid */
            }

            /* index of the first notification for which there should be sent a get Notif/App Attribute to the server */
            uint8_t index = AncsClient_CheckNeedGetNotifOrAppAttribute();
            if(index < mMaxDisplayNotifications_c)
            {
                AncsClient_SendGetNotificationOrApplicationAttribute();
            }
            else
            {
                /* If there is no other notification for which there should be
                   Notif/App Attributes received, all the notifications should be printed*/
                AncsClient_DisplayNotifications();
            }
        }
    }
}

/*! *********************************************************************************
* \brief    Handles BLE notification application information from the AMS Entity Update
*           Characteristic on the AMS Server.
*
* \param[in]    pEuData         Pointer to the AMS Entity Update BLE notification payload
* \param[in]    euDataLength    Length of the AMS Entity Update BLE notification payload
********************************************************************************** */
static void AmsClient_ProcessEuTrackInfo(
    uint8_t     *pEuData,
    uint16_t     euDataLength)
{
    switch ((amsTrackAttributeId_t)pEuData[1])
    {
        case gAmsTrackAttributeIdArtist_c:
        {
            FLib_MemCpy((uint8_t *)(amsClientData.euTrackArtist),
                        &(pEuData[3]),
                        euDataLength - 3);
            amsClientData.euTrackArtist[euDataLength - 3] = 0; /* add null terminator */
        }
        break;

        case gAmsTrackAttributeIdAlbum_c:
        {
            FLib_MemCpy((uint8_t *)(amsClientData.euTrackAlbum),
                        &(pEuData[3]),
                        euDataLength - 3);
            amsClientData.euTrackAlbum[euDataLength - 3] = 0; /* add null terminator */
        }
        break;

        case gAmsTrackAttributeIdTitle_c:
        {
            FLib_MemCpy((uint8_t *)(amsClientData.euTrackTitle),
                        &(pEuData[3]),
                        euDataLength - 3);
            amsClientData.euTrackTitle[euDataLength - 3] = 0; /* add null terminator */
        }
        break;

        case gAmsTrackAttributeIdDuration_c:
        {
            FLib_MemCpy((uint8_t *)(amsClientData.euTrackDuration),
                        &(pEuData[3]),
                        euDataLength - 3);
            amsClientData.euTrackDuration[euDataLength - 3] = 0; /* add null terminator */
        }
        break;

        default:
        {
            shell_write("\r\nWarning: Unhandled AMS Entity Update Track Attribute Id:");
            shell_write(" 0x");
            shell_writeHex(pEuData[1]);
            shell_write("\r\n");
        }
        break;
    }

    /* Use flag in ancs to have the same consistent print */
    ancsClientData.notificationDataChanged = TRUE;
}

/*! *********************************************************************************
* \brief    Handles BLE notifications from the ANCS Data Source Characteristic
*           on the ANCS Server.
*
* \param[in]    deviceId        Id of the device from which the notification was received
* \param[in]    pDsData         Pointer to the ANCS Data Source BLE notification payload
* \param[in]    dsDataLength    Length of the ANCS Data Source BLE notification payload
********************************************************************************** */
static void AncsClient_ProcessDsNotification(deviceId_t   deviceId,
                                             uint8_t     *pDsData,
                                             uint16_t     dsDataLength)
{
    switch ((ancsComdId_t)pDsData[0])
    {
        case gAncsCmdIdGetNotificationAttributes_c:
        {
            AncsClient_ProcessDsNotifAttributes(deviceId, pDsData, dsDataLength);
        }
        break;

        case gAncsCmdIdGetAppAttributes_c:
        {
            AncsClient_ProcessDsNotifAppInfo(pDsData, dsDataLength);
        }
        break;

        case gAncsCmdIdPerformNotificationAction_c:
        {
        }
        break;

        default:
        {
            shell_write("\r\nWarning: Unhandled ANCS Data Source Command Id:");
            shell_write(" 0x");
            shell_writeHex(pDsData[0]);
            shell_write("\r\n");
        }
        break;
    }
}

/*! *********************************************************************************
* \brief    Handles BLE notifications from the AMS Remote Command Characteristic
*           on the ANCS Server.
*
* \param[in]    deviceId        Id of the device from which the notification was received
* \param[in]    pRcData         Pointer to the AMS Remote Command BLE notification payload
* \param[in]    rcDataLength    Length of the AMS Remote Command BLE notification payload
********************************************************************************** */
static void AmsClient_ProcessRcNotification(deviceId_t   deviceId,
                                            uint8_t     *pRcData,
                                            uint16_t     rcDataLength)
{
    uint16_t idx;
    uint16_t commandBitmask;

    commandBitmask = 0x0000U;

    for (idx = 0x00U; idx < rcDataLength; idx++)
    {
        commandBitmask |= (0x01U << pRcData[idx]);
    }

    amsClientData.amsAvailableCommandsBitmask = commandBitmask;

    /* Use flag in ancs to have the same consistent print */
    ancsClientData.notificationDataChanged = TRUE;
}

/*! *********************************************************************************
* \brief    Handles BLE notifications from the AMS Entity Update Characteristic
*           on the AMS Server.
*
* \param[in]    deviceId        Id of the device from which the notification was received
* \param[in]    pEuData         Pointer to the AMS Entity Update BLE notification payload
* \param[in]    euDataLength    Length of the AMS Entity Update BLE notification payload
********************************************************************************** */
static void AmsClient_ProcessEuNotification(deviceId_t   deviceId,
                                             uint8_t     *pEuData,
                                             uint16_t     euDataLength)
{
    switch ((amsEntityUpdateId_t)pEuData[0])
    {
        case gAmsEntityIdPlayer_c:
        {
            /* Not Configured in Demo */
        }
        break;

        case gAmsEntityIdQueue_c:
        {
            /* Not Configured in Demo */
        }
        break;

        case gAmsEntityIdTrack_c:
        {
            AmsClient_ProcessEuTrackInfo(pEuData, euDataLength);
        }
        break;

        default:
        {
            shell_write("\r\nWarning: Unhandled AMS Entity Update Id:");
            shell_write(" 0x");
            shell_writeHex(pEuData[0]);
            shell_write("\r\n");
        }
        break;
    }
}

/*! *********************************************************************************
* \brief        Formats and displays information from the notifications
*               table if the flag signalling changes is set.
*
********************************************************************************** */
static void AncsClient_DisplayNotifications(void)
{
    /* There is no other notification for which there should be more notification attribute requested from the server */
    mReceivedNotificationsAndNeedToPrint = 0U;
    mCanSendToServer = TRUE;
    uint16_t b_idx; /* Index used for iterating the Remote Commands available */

    if (ancsClientData.notificationDataChanged == TRUE)
    {
        shell_write("\033");
        shell_write("[2J");
        shell_write("\033");
        shell_write("[H");
        uint32_t n_idx;
        uint32_t c_idx;

        shell_write("\r\n");
        shell_write("Notif_UID "); /* Notification UID header - 10 chars reserved for value */
        shell_write("  ");
        shell_write("Flags"); /* Notification flags - 5 chars reserved for value */
        shell_write("  ");
        shell_write("Category            "); /* Notification category - 20 chars reserved for value */
        shell_write("  ");
        shell_write("Application"); /* Notification application - if known - mMaxNotifAppNameDisplayLength_c reserved */
        shell_write("\r\n");

        for (n_idx = 0; n_idx < mMaxDisplayNotifications_c; n_idx++)
        {
            if (ancsClientData.notifications[n_idx].slotUsed == TRUE)
            {
                /* Write Notification UID - 10 chars reserved for value */
                shell_write("0x");
                uint8_t *p = (uint8_t *)&(ancsClientData.notifications[n_idx].notifUid);
                shell_writeHex(p[3]);
                shell_writeHex(p[2]);
                shell_writeHex(p[1]);
                shell_writeHex(p[0]);
                shell_write("  ");

                /* Write Notification Flags - 5 chars reserved for value */
                for (c_idx = 0; c_idx < sizeof(ancsFlagsToLetterTable); c_idx++)
                {
                    if (((uint8_t)(ancsClientData.notifications[n_idx].notifFlags) & (uint8_t)(0x01U << c_idx)) > 0U)
                    {
                        shell_writeN((char *)(&(ancsFlagsToLetterTable[c_idx])), 1);
                    }
                    else
                    {
                        shell_writeN((char *)(&mAncsFalgNotSetSymbol), 1);
                    }
                }

                shell_write("  ");

                /* Write Notification category - 20 chars reserved for value */
                if (ancsClientData.notifications[n_idx].notifCat < gAncsCatIdInvalid_c)
                {
                    shell_writeN((char *)(&(ancsNotifCatToStringTable[ancsClientData.notifications[n_idx].notifCat])),
                                 mMaxNotifCatDisplayLength_c);
                }
                else
                {
                    for (c_idx = 0; c_idx < mMaxNotifCatDisplayLength_c; c_idx++)
                    {
                        shell_writeN((char *)(&mAncsNotifCatCharPlaceholder), 1);
                    }
                }

                shell_write("  ");

                /* Write application name if it has been obtained otherwise write mMaxNotifAppNameDisplayLength_c _ characters */
                if (ancsClientData.notifications[n_idx].appNameValid == TRUE)
                {
                    shell_write((char *)(ancsClientData.notifications[n_idx].appName)); /* NULL terminated string. */
                }
                else
                {
                    for (c_idx = 0; c_idx < mMaxNotifAppNameDisplayLength_c; c_idx++)
                    {
                        shell_writeN((char *)(&mAncsAppNameCharPlaceholder), 1);
                    }
                }

                shell_write("\r\n");
            }
        }

        ancsClientData.notificationDataChanged = FALSE;

        /* Display the AMS Track Info */
        shell_write("\n\r\n\rAMS");
        shell_write("\n\r  Artist  : ");
        shell_write((char *)(amsClientData.euTrackArtist));
        shell_write("\n\r  Album   : ");
        shell_write((char *)(amsClientData.euTrackAlbum));
        shell_write("\n\r  Title   : ");
        shell_write((char *)(amsClientData.euTrackTitle));
        shell_write("\n\r  Duration: ");
        shell_write((char *)(amsClientData.euTrackDuration));

        /* Display the possible AMS actions that can be executed at the current state */
        shell_write("\n\r\n\rAvailable commands: ");

        for (b_idx = 0x00U; b_idx < gAmsTotalRemoteCommandIds_c; b_idx++)
        {
            if ((amsClientData.amsAvailableCommandsBitmask & (1 << b_idx)) != 0)
            {
                shell_writeN((char *)(&(amsRemoteCommandToStringTable[b_idx])), mMaxRemoteCommandDisplayLength_c);
                shell_write(" ");
            }
        }

        if (amsClientData.amsInvalidCommand == TRUE)
        {
            shell_write("\r\n\r\n");
            shell_write("Command Invalid in current Remote Command state");
            amsClientData.amsInvalidCommand = FALSE;
        }

        shell_write("\r\n\r\n");
        shell_write(shellPrompt.pConstPrompt);
    }
}

/*! *********************************************************************************
* \brief        Handles advertising timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void AdvertisingTimerCallback(void *pParam)
{
    /* Stop and restart advertising with new parameters */
    (void)Gap_StopAdvertising();

    switch (mAdvState.advType)
    {
#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U)) &&\
    (defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1U)) &&\
    (defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0))
        case fastFilterAcceptListAdvState_c:
        {
            mAdvState.advType = fastAdvState_c;
        }
        break;
#endif /* gAppUseBonding_d && gAppUsePrivacy_d && gBleEnableControllerPrivacy_d */
        case fastAdvState_c:
        {
            mAdvState.advType = slowAdvState_c;
        }
        break;

        default:
            ; /* For MISRA compliance */
            break;
    }

    BleApp_Advertise();
}

/*! *********************************************************************************
* \brief        Handles current time tick callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void CTSTickTimerCallback(void *pParam)
{
    ctsServiceConfig.localTime++;
    ctsServiceConfig.adjustReason = gCts_UnknownReason;
   (void)Cts_RecordCurrentTime(&ctsServiceConfig);
}

/*! *********************************************************************************
* \brief        Handles time update callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void RTUSReferenceUpdateTimerCallback(void *pParam)
{
    if (rtusServiceConfig.timeUpdateState.currentState == gRtusUpdatePending_c)
    {
        rtusResult_t result = gRtusSuccessful_c;

        /* We simulate an update just for demo purposes */
        rtusServiceConfig.timeUpdateState.currentState = gRtusIdle_c;
        rtusServiceConfig.timeUpdateState.result = result;
       (void)Rtus_RecordTimeUpdateState(&rtusServiceConfig);
        ctsServiceConfig.adjustReason = gCts_ExternalRefUpdate;
       (void)Cts_RecordCurrentTime(&ctsServiceConfig);
    }
}

/*! *********************************************************************************
* \brief        Handles battery measurement timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void BatteryMeasurementTimerCallback(void *pParam)
{
    basServiceConfig.batteryLevel = SENSORS_GetBatteryLevel();
   (void)Bas_RecordBatteryMeasurement(&basServiceConfig);
}

/*! *********************************************************************************
* \brief        Handles the timer used to allow the user press the Allow
*               Notifications button callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void AllowNotificationsTimerCallback(void *pParam)
{
    /*Unsubscribe from the NS characteristic */
    uint16_t value = 0;
    if (NULL == mpDescProcBuffer)
    {
        panic(0, 0, 0, 0);
    }
    else
    {
        /* Enable notifications for the ANCS Notification Source characteristic. */
        mpDescProcBuffer->handle = mPeerInformation.customInfo.ancsClientConfig.hNotificationSourceCccd;
        mpDescProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
       (void)GattClient_WriteCharacteristicDescriptor(mPeerInformation.deviceId, mpDescProcBuffer, (uint16_t)sizeof(uint16_t),
                                                     (uint8_t *)&value);
    }
}

/*! *********************************************************************************
* \brief        Checks if there are notifications for which there should be sent
*               a get app attribute to the server and returns the first notification
*               index with this need or mMaxDisplayNotifications_c if there is none
********************************************************************************** */
static uint8_t AncsClient_CheckNeedGetNotifOrAppAttribute(void)
{
  uint8_t notifUidIndex = 0U;
  while(notifUidIndex < mMaxDisplayNotifications_c)
  {
    if(ancsClientData.notifications[notifUidIndex].needGetNotifAttribute == TRUE)
    {
        mGetNotifOrAppAttribute = 0U;
        break;
    }
    else
    {
        if(ancsClientData.notifications[notifUidIndex].needGetAppAttribute == TRUE)
        {
            mGetNotifOrAppAttribute = 1U;
            break;
        }
    }
    notifUidIndex++;
  }
  return notifUidIndex;
}

/*! *********************************************************************************
* \brief        Searches for the first Notification that there is the need to send Get
                Notification Attribute for and sends the request to the server
********************************************************************************** */
static void AncsClient_SendGetNotificationOrApplicationAttribute(void)
{
    uint8_t index = AncsClient_CheckNeedGetNotifOrAppAttribute();
    if(index < mMaxDisplayNotifications_c)
    {
        mCanSendToServer = FALSE;
        if(mGetNotifOrAppAttribute == 0U)
        {
            /* Need to send Get Notif Attribute */
            uint32_t notifUid = ancsClientData.notifications[index].notifUid;

            /* Prepare and send a Get Notification Attributes command requesting the Application ID */
            uint8_t ancsGetNotifAttrCmd[gAncsCmdIdFieldLength_c + gAncsNotifUidFieldLength_c + gAncsAttrIdFieldLength_c] = {0};

            ancsGetNotifAttrCmd[0] = (uint8_t)gAncsCmdIdGetNotificationAttributes_c;
            FLib_MemCpy(&(ancsGetNotifAttrCmd[mAncsNotifUidFieldLengthOffset_c]),
                       (uint8_t *)(&(notifUid)),
                        gAncsNotifUidFieldLength_c);
            ancsGetNotifAttrCmd[mAncsNotifAttrIdAppIdentifierOffset_c] = (uint8_t)gAncsNotifAttrIdAppIdentifier_c;
            AncsClient_SendCommandToAncsServer(mPeerInformation.deviceId, ancsGetNotifAttrCmd, (uint16_t)sizeof(ancsGetNotifAttrCmd));
        }
        else
        {
            /* Need to send Get App Attribute */
            uint8_t     ancsGetAppAttrCmd[gAncsCmdIdFieldLength_c + gAncsAttrIdFieldLength_c + mAppIdentifierReservedSpace_c] = {0};
            uint16_t    cmdLength = (uint16_t)(gAncsCmdIdFieldLength_c + ancsClientData.notifications[index].appIdLength + mNullTerminatorOffset_c + gAncsAttrIdFieldLength_c);

            if ((cmdLength) <= sizeof(ancsGetAppAttrCmd))
            {
                ancsGetAppAttrCmd[0] = (uint8_t)gAncsCmdIdGetAppAttributes_c;
                FLib_MemCpy(&(ancsGetAppAttrCmd[mAncsAppIdOffset_c]),
                            ancsClientData.notifications[index].pAppId,
                            ancsClientData.notifications[index].appIdLength);
                ancsGetAppAttrCmd[gAncsCmdIdFieldLength_c + ancsClientData.notifications[index].appIdLength] = 0x00; /* NULL Terminate the App Id string */
                ancsGetAppAttrCmd[gAncsCmdIdFieldLength_c + ancsClientData.notifications[index].appIdLength + mNullTerminatorOffset_c] = (uint8_t)gAncsAppAttrIdDisplayName_c;
                AncsClient_SendCommandToAncsServer(mPeerInformation.deviceId, ancsGetAppAttrCmd, cmdLength);
            }
            else
            {
                shell_write("\r\nWarning: Could not send a Get App Attributes Command, the command buffer is not large enough!\r\n");
            }
        }
    }
}

static void BleApp_HandleConnection(deviceId_t peerDeviceId)
{
    /* UI */
    LedStopFlashingAllLeds();
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c ==1))
        LedSetColor(0, kLED_Blue);    
#endif /* gAppLedCnt_c == 1 */ 
    Led1On();
    shell_write("\r\nConnected!\r\n");
    
    mPeerInformation.deviceId = peerDeviceId;
    mPeerInformation.isBonded = FALSE;
    
#if gAppUseBonding_d
    mSendDataAfterEncStart = FALSE;
    
    if ((gBleSuccess_c == Gap_CheckIfBonded(peerDeviceId, &mPeerInformation.isBonded, NULL)) &&
        (TRUE == mPeerInformation.isBonded))
    {
        mSendDataAfterEncStart = TRUE;
        (void)Gap_LoadCustomPeerInformation(peerDeviceId, (void*) &mPeerInformation.customInfo, 0, (uint16_t)sizeof(appCustomInfo_t));
        /* Reconnect after reset */
        mFirstRunningBeforeReconnect = mAncsMaxWriteCCCDProcedures_c;
    }
#endif
    
    /* Advertising stops when connected */
    mAdvState.advOn = FALSE;
    
    BleApp_StateMachineHandler(mPeerInformation.deviceId, mAppEvt_PeerConnected_c);
    
    /* Subscribe client*/
    (void)Bas_Subscribe(&basServiceConfig, peerDeviceId);
    (void)Cts_Subscribe(peerDeviceId);
    (void)Ndcs_Subscribe(peerDeviceId);
    (void)Rtus_Subscribe(peerDeviceId);
    
    /* Stop Advertising Timer*/
    (void)TM_Stop((timer_handle_t)mAdvTimerId);
    
    /* Start battery measurements */
    (void)TM_InstallCallback((timer_handle_t)mBatteryMeasurementTimerId, BatteryMeasurementTimerCallback, NULL);
    (void)TM_Start((timer_handle_t)mBatteryMeasurementTimerId, (uint16_t)kTimerModeLowPowerTimer | (uint16_t)kTimerModeSetSecondTimer, mBatteryLevelReportInterval_c);
}

/*! *********************************************************************************
* @}
********************************************************************************** */
