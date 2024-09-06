/*! *********************************************************************************
* \addtogroup Extended Advertising Peripheral
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright 2015 Freescale Semiconductor, Inc.
* Copyright 2020 - 2024 NXP
*
*
* \file
*
* This file is the source file for the extended advertising peripheral application
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
#include "fsl_component_mem_manager.h"
#include "fsl_component_panic.h"
#include "fsl_component_serial_manager.h"
#include "fsl_format.h"
#include "FunctionLib.h"
#include "sensors.h"


/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "gatt_db_handles.h"

/* Profile / Services */
#include "battery_interface.h"
#include "device_info_interface.h"
#include "temperature_interface.h"

/* Connection Manager */
#include "ble_conn_manager.h"

#include "board.h"
#include "app_conn.h"
#include "app_advertiser.h"
#include "app.h"
#include "adv_ext_peripheral.h"

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define mNumberOfAdvHandles_c    (2U)
#define mInvalidAdvHandle      (0xffU)

#ifndef mAE_PeripheralDebug_c
#define mAE_PeripheralDebug_c   (0)
#endif

#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
#define mChangePerAdvDataOption_c                   (10U)
#define mChangeDbafNonConnNonScannAdvDataOption_c   (9U)
#define mChangeNonConnNonScannAdvDataOption_c       (8U)
#else
#define mChangePerAdvDataOption_c              (6U)
#define mChangeNonConnNonScannAdvDataOption_c  (5U)
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */

#define Serial_Print(a,b,c)     (void)SerialManager_WriteBlocking((serial_write_handle_t)s_writeHandle, (uint8_t *)&(b)[0], strlen(b))
#define Serial_PrintDec(a,b)    (void)SerialManager_WriteBlocking((serial_write_handle_t)s_writeHandle, FORMAT_Dec2Str(b), strlen((char const *)FORMAT_Dec2Str(b)))
#define AppPrintString(b)       (void)SerialManager_WriteBlocking((serial_write_handle_t)s_writeHandle, (uint8_t *)&(b)[0], strlen(b))

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

typedef enum
{
    madvHandle_Available_c,
    madvHandle_InUse_c,
}advHandleStatus_t;

typedef enum
{
    mAdvStatus_Off_c,
    mAdvStatus_On_c
}advStatus_t;

typedef enum
{
    mLegacyAdvIndex_c = 0,
    mExtAdvScannIndex_c,
    mExtAdvConnIndex_c,
    mExtAdvNonConnNonScanIndex_c,
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
    mDbafAdvScannIndex_c,
    mDbafAdvConnIndex_c,
    mDbafAdvNonConnNonScanIndex_c,
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
    mPeriodicAdvIndex_c,
    mAdvIndexMax_c
}advIndex_t;

typedef enum
{
    mExtAdvSeq_Idle_c,
    mExtAdvSeq_Start_c,
    mExtAdvSeq_Stop_c,
    mExtAdvSeq_ChangeExtAdvData_c,
    mExtAdvSeq_ChangePeriodicData_c
}extAdvSequence_t;

typedef enum {gApiReq_Success_c, gApiReq_Denied_c }apiReqStatus_t;
typedef enum {mSS_PrintMenu_c, mSS_WaitOption_c, mSS_HandleOption_c}switch2Status_t;

typedef struct appPeerDevice_tag
{
    deviceId_t deviceId;
    TIMER_MANAGER_HANDLE_DEFINE(timerId);
}appPeerDevice_t;
/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/* Adv State */
static extAdvSequence_t mExtAdvSequence;
static advStatus_t maAdvStatus[mAdvIndexMax_c];

static advIndex_t mExtAdvAPIOwner = mAdvIndexMax_c;

/* Service Data*/
static bool_t           basValidClientList[gAppMaxConnections_c] = {FALSE};
static basConfig_t      basServiceConfig = {(uint16_t)service_battery, 0, basValidClientList, gAppMaxConnections_c};
static disConfig_t      disServiceConfig = {(uint16_t)service_device_info};

static tmsConfig_t tmsServiceConfig = {(uint16_t)service_temperature, 0};

/* Application specific data*/
static appPeerDevice_t maPeerDeviceId[gAppMaxConnections_c];

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
static switch2Status_t switch2Status = mSS_PrintMenu_c;
static uint8_t menuOption = 0;
static char* legacyAdvStrings[] = {"\n\r 1. Start Legacy Advertising",\
                                         "\n\r 1. Stop Legacy Advertising",
                                         "\n\rConnected"};
static char* extAdvScannStrings[] = {"\n\r 2. Start Extended Scannable Advertising",\
                                           "\n\r 2. Stop Extended Scannable Advertising"};
static char* extAdvConnStrings[] = {"\n\r 3. Start Extended Connectable Advertising",\
                                          "\n\r 3. Stop Extended Connectable Advertising",
                                          "\n\rConnected"};
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
static char* extAdvNonConnNonScanStrings[] = {"\n\r 4. Start Extended Non Connectable Non Scannable Advertising",\
        "\n\r 4. Stop Extended Non Connectable Non Scannable Advertising \n\r 9. Change Data for Non Connectable Non Scannable Advertising"};

static char* dbafAdvScannStrings[] = {"\n\r 5. Start DBAF Scannable Advertising",\
                                           "\n\r 5. Stop DBAF Scannable Advertising"};

static char* dbafAdvConnStrings[] = {"\n\r 6. Start DBAF Connectable Advertising",\
                                          "\n\r 6. Stop DBAF Connectable Advertising",
                                          "\n\rConnected"};

static char* dbafAdvNonConnNonScanStrings[] = {"\n\r 7. Start DBAF Non Connectable Non Scannable Advertising",\
        "\n\r 7. Stop DBAF Non Connectable Non Scannable Advertising \n\r 10. Change Data for DBAF Non Connectable Non Scannable Advertising"};

static char* periodicAdvStrings[] = {"\n\r 8. Start Periodic Advertising",\
        "\n\r 8. Stop Periodic Advertising \n\r 11. Change Data for Periodic Advertising"};

static char** maMenu[]= {legacyAdvStrings, extAdvScannStrings, extAdvConnStrings, extAdvNonConnNonScanStrings, dbafAdvScannStrings, dbafAdvConnStrings, dbafAdvNonConnNonScanStrings, periodicAdvStrings};
static char* menuOptions[] ={"\r 1 ","\r 2 ", "\r 3 ", "\r 4 ", "\r 5 ", "\r 6 ", "\r 7 ", "\r 8 ", "\r 9 ", "\r 10", "\r 11"};
#else
static char* extAdvNonConnNonScanStrings[] = {"\n\r 4. Start Extended Non Connectable Non Scannable Advertising",\
        "\n\r 4. Stop Extended Non Connectable Non Scannable Advertising \n\r 6. Change Data for Non Connectable Non Scannable Advertising"};

static char* periodicAdvStrings[] = {"\n\r 5. Start Periodic Advertising",\
        "\n\r 5. Stop Periodic Advertising \n\r 7. Change Data for Periodic Advertising"};

static char** maMenu[]= {legacyAdvStrings, extAdvScannStrings, extAdvConnStrings, extAdvNonConnNonScanStrings, periodicAdvStrings};
static char* menuOptions[] ={"\r 1","\r 2", "\r 3", "\r 4", "\r 5", "\r 6", "\r 7"};
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
#endif /*gAppButtonCnt_c > 0*/
static advHandleStatus_t maAdvHandle[mNumberOfAdvHandles_c] = {madvHandle_Available_c, madvHandle_Available_c};
static advIndex_t maLastAdvIndexForThisHandle[mNumberOfAdvHandles_c]={ mAdvIndexMax_c, mAdvIndexMax_c};
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
static gapExtAdvertisingParameters_t* maPExtAdvParam[] = {&gExtAdvParamsLegacy, &gExtAdvParamsScannable, &gExtAdvParamsConnectable, &gExtAdvParamsNonConnNonScann, &gDbafParamsScannable, &gDbafParamsConnectable, &gDbafParamsNonConnNonScann };

static gapAdvertisingData_t* maPExtAdvData[] = {&gAppAdvertisingData , NULL, &gAppExtAdvDataConnectable, &gAppExtAdvDataId1NonConnNonScan, NULL, &gAppExtAdvDataConnectable, &gAppExtAdvDataId1NonConnNonScan };
static gapAdvertisingData_t* maPExtScanData[] = {&gAppScanRspData , &gAppExtAdvDataScannable, NULL, &gAppScanRspData, &gAppExtAdvDataScannable, NULL, &gAppScanRspData };
#else
static gapExtAdvertisingParameters_t* maPExtAdvParam[] = {&gExtAdvParamsLegacy, &gExtAdvParamsScannable, &gExtAdvParamsConnectable, &gExtAdvParamsNonConnNonScann };

static gapAdvertisingData_t* maPExtAdvData[] = {&gAppAdvertisingData , NULL, &gAppExtAdvDataConnectable, &gAppExtAdvDataId1NonConnNonScan };
static gapAdvertisingData_t* maPExtScanData[] = {&gAppScanRspData , &gAppExtAdvDataScannable, NULL, &gAppScanRspData };
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
static gapAdvertisingData_t* maPPeriodicAdvData[] = {&gAppExtAdvDataId1Periodic, &gAppExtAdvDataId2Periodic};
static gapAdvertisingData_t* maPExtAdvDataNonConnNonScan[] = {&gAppExtAdvDataId1NonConnNonScan, &gAppExtAdvDataId2NonConnNonScan};
static uint8_t mPeriodicAdvDataIndex = 0;
static uint8_t mExtAdvDataNCNSIndex = 0;
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
static uint8_t mDbafAdvDataNCNSIndex = 0;
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */

static char* advTypeStrings[] = {"\n\rLegacy Advertising",\
                                       "\n\rExtended Scanable Advertising",\
                                       "\n\rExtended Connectable Advertising",\
                                       "\n\rExtended Non Connectable Non Scanable Advertising",
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
                                       "\n\rDBAF Scanable Advertising",\
                                       "\n\rDBAF Connectable Advertising",\
                                       "\n\rDBAF Non Connectable Non Scanable Advertising",
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
                                       "\n\rExtended Periodic Advertising"};

#if mAE_PeripheralDebug_c
static char* genericCBCKStrings[] = {\
    "gInitializationComplete_c",\
    "gInternalError_c",\
    "gAdvertisingSetupFailed_c",\
    "gAdvertisingParametersSetupComplete_c",\
    "gAdvertisingDataSetupComplete_c",\
    "gFilterAcceptListSizeRead_c",\
    "gDeviceAddedToFilterAcceptList_c",
    "gDeviceRemovedFromFilterAcceptList_c",\
    "gFilterAcceptListCleared_c",\
    "gRandomAddressReady_c",\
    "gCreateConnectionCanceled_c",\
    "gPublicAddressRead_c",\
    "gAdvTxPowerLevelRead_c",\
    "gPrivateResolvableAddressVerified_c",\
    "gRandomAddressSet_c",\
    "gLeScPublicKeyRegenerated_c",\
    "gLeScLocalOobData_c",\
    "gHostPrivacyStateChanged_c",\
    "gControllerPrivacyStateChanged_c",\
    "gControllerTestEvent_c",\
    "gTxPowerLevelSetComplete_c",\
    "gLePhyEvent_c",\
    "gControllerNotificationEvent_c",\
    "gBondCreatedEvent_c",\
    "gChannelMapSet_c",\
    "gExtAdvertisingParametersSetupComplete_c",\
    "gExtAdvertisingDataSetupComplete_c",\
    "gExtAdvertisingSetRemoveComplete_c",\
    "gPeriodicAdvParamSetupComplete_c",\
    "gPeriodicAdvDataSetupComplete_c",\
    "gPeriodicAdvListUpdateComplete_c",\
    "gPeriodicAdvCreateSyncCancelled_c",\
    "gTxEntryAvailable_c",\
    "gPeriodicAdvertisingStateChanged_c"};

static char* advertisingCBCKStrings[] = {\
    "gAdvertisingStateChanged_c",\
    "gAdvertisingCommandFailed_c",\
    "gExtAdvertisingStateChanged_c",\
    "gAdvertisingSetTerminated_c",\
    "gExtScanNotification_c"};
#endif

/*serial manager handle*/
static serial_handle_t gAppSerMgrIf;
/*write handle*/
static SERIAL_MANAGER_WRITE_HANDLE_DEFINE(s_writeHandle);
/*read handle*/
static SERIAL_MANAGER_READ_HANDLE_DEFINE(s_readHandle);

static appExtAdvertisingParams_t mAppExtAdvParams;

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

/* Gatt and Att callbacks */
static void BleApp_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent);
static void BleApp_ConnectionCallback (deviceId_t peerDeviceId, gapConnectionEvent_t* pConnectionEvent);
static void BleApp_GattServerCallback (deviceId_t deviceId, gattServerEvent_t* pServerEvent);
static void BluetoothLEHost_Initialized(void);
static void BluetoothLEHost_GenericCallback(gapGenericEvent_t *pGenericEvent);

/* Timer Callbacks */
static void DisconnectTimerCallback(void *pParam);

static void BleApp_SendTemperature(deviceId_t deviceId);

static void AppPrintLePhyEvent(gapPhyEvent_t* pPhyEvent);
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
static void Uart_PrintMenu(void *pData);
static void Key_HandleOption(void *pData);
#endif /*gAppButtonCnt_c > 0*/
static void AppPrintDec(uint32_t nb);
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
static void AppPrintStringCallback(void* pData);
#endif /*gAppButtonCnt_c > 0*/
static uint8_t Alloc_AdvHandler(void);
static void Free_AdvHandler(uint8_t advHandle);
static bool_t AllAdvertisingsAreOff(void);
static apiReqStatus_t ExtAdvAPIRequest(advIndex_t advIndex);
static void  FreeExtAdvAPI(void);
static void  EndSequence(void);
static bool_t Add_PeerDevice(deviceId_t deviceId);
static void Remove_PeerDevice(deviceId_t deviceId);
static uint8_t Get_PeerDeviceIndex(deviceId_t deviceId);
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
static void Disconnect_AllPeerDevices(void);
#endif /*gAppButtonCnt_c > 0*/
static uint8_t NumberOf_PeerDevices(void);
static void BleApp_SerialInit(void);

static void BleApp_HandleExtAdvNonConnNonScannMode(uint8_t mode);
static void BleApp_HandlePeriodicAdvMode(uint8_t mode);
static void BleApp_HandleChangePerAdvDataMode(void);
static void BleApp_HandleChangeNonConnNonScannAdvDataMode(void);
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
static void BleApp_HandleChangeDbafNonConnNonScannAdvDataMode(void);
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
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
    BleApp_SerialInit();
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[0], BleApp_HandleKeys0, NULL);
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[1], BleApp_HandleKeys1, NULL);
#endif
#endif
    /* Set generic callback */
    BluetoothLEHost_SetGenericCallback(BluetoothLEHost_GenericCallback);

    /* Initialize Bluetooth Host Stack */
    BluetoothLEHost_Init(BluetoothLEHost_Initialized);
}

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(uint8_t mode)
{
    switch(mode)
    {
    case (uint8_t)mLegacyAdvIndex_c:
    case (uint8_t)mExtAdvScannIndex_c:
    case (uint8_t)mExtAdvConnIndex_c:
    case (uint8_t)mExtAdvNonConnNonScanIndex_c:
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
    case (uint8_t)mDbafAdvScannIndex_c:
    case (uint8_t)mDbafAdvConnIndex_c:
    case (uint8_t)mDbafAdvNonConnNonScanIndex_c:
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
        BleApp_HandleExtAdvNonConnNonScannMode(mode);
        break;

    case (uint8_t)mPeriodicAdvIndex_c:
        BleApp_HandlePeriodicAdvMode(mode);
        break;

    case mChangePerAdvDataOption_c:
        BleApp_HandleChangePerAdvDataMode();
        break;

    case mChangeNonConnNonScannAdvDataOption_c:
        BleApp_HandleChangeNonConnNonScannAdvDataMode();
        break;
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
    case mChangeDbafNonConnNonScannAdvDataOption_c:
        BleApp_HandleChangeDbafNonConnNonScannAdvDataMode();
        break;
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
    default:
        {
            ; /* No action required */
        }
        break;
    }
}

/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    events    Key event structure.
********************************************************************************** */
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
button_status_t BleApp_HandleKeys0(void *buttonHandle, button_callback_message_t *message,void *callbackParam)
{
    switch (message->event)
    {
    case kBUTTON_EventOneClick:
    case kBUTTON_EventShortPress:
    case kBUTTON_EventLongPress:
        {
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
            uint16_t bmValidOption = (uint16_t)0x00ffU |
                (((maAdvStatus[mExtAdvNonConnNonScanIndex_c] == mAdvStatus_On_c)? (uint8_t)1:(uint8_t)0)<<8) |
                    (((maAdvStatus[mDbafAdvNonConnNonScanIndex_c] == mAdvStatus_On_c)? (uint8_t)1:(uint8_t)0)<<9) |
                    (((maAdvStatus[mPeriodicAdvIndex_c] == mAdvStatus_On_c)? (uint8_t)1:(uint8_t)0)<<10);
#else
            uint16_t bmValidOption = (uint16_t)0x001fU |
                (((maAdvStatus[mExtAdvNonConnNonScanIndex_c] == mAdvStatus_On_c)? (uint8_t)1:(uint8_t)0)<<5) |
                    (((maAdvStatus[mPeriodicAdvIndex_c] == mAdvStatus_On_c)? (uint8_t)1:(uint8_t)0)<<6);
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
            do
            {
                menuOption = (menuOption + 1U)%NumberOfElements(menuOptions);
            }
            while( ((((uint16_t)1)<<menuOption) & bmValidOption) == 0U);
        }
        (void)App_PostCallbackMessage(AppPrintStringCallback, (void*)menuOptions[menuOption]);
        switch2Status = mSS_HandleOption_c;
        break;

    default:
        {
            ; /* No action required */
        }
        break;
    }
    return kStatus_BUTTON_Success;
}
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1))
button_status_t BleApp_HandleKeys1(void *buttonHandle, button_callback_message_t *message,void *callbackParam)
{
    switch (message->event)
    {
        /* Start the application */
        case kBUTTON_EventOneClick:
        case kBUTTON_EventShortPress:
            {
                if(switch2Status == mSS_PrintMenu_c )
                {
                    switch2Status = mSS_WaitOption_c;
                    menuOption = NumberOfElements(menuOptions) - 1U;
                    (void)App_PostCallbackMessage(Uart_PrintMenu, NULL);
                }
                else if(switch2Status == mSS_HandleOption_c )
                {
                    switch2Status = mSS_PrintMenu_c;
                    (void)App_PostCallbackMessage(Key_HandleOption, &menuOption);
                }
                else
                {
                    ;/*misra compliance purposes*/
                }
            }
            break;

        /* Disconnect  */
        case kBUTTON_EventLongPress:
            {
                Disconnect_AllPeerDevices();
            }
            break;

        default:
            {
                ; /* No action required */
            }
            break;
    }
    return kStatus_BUTTON_Success;
}
#endif /*gAppButtonCnt_c > 1*/
#endif /*gAppButtonCnt_c > 0*/

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief        Configures BLE Stack after initialization.
*
********************************************************************************** */
static void BluetoothLEHost_Initialized(void)
{
    /* Common GAP configuration */
    BleConnManager_GapCommonConfig();

    /* Register for callbacks*/
    (void)App_RegisterGattServerCallback(BleApp_GattServerCallback);

    FLib_MemSet (maAdvStatus, (uint8_t)mAdvStatus_Off_c, sizeof(maAdvStatus));

    /* Start services */
    SENSORS_TriggerTemperatureMeasurement();
    (void)SENSORS_RefreshTemperatureValue();
    /* Multiply temperature value by 10. SENSORS_GetTemperature() reports temperature
    value in tenths of degrees Celsius. Temperature characteristic value is degrees
    Celsius with a resolution of 0.01 degrees Celsius (GATT Specification
    Supplement v6). */
    tmsServiceConfig.initialTemperature = (int16_t)(10 * SENSORS_GetTemperature());
    (void)Tms_Start(&tmsServiceConfig);

    basServiceConfig.batteryLevel = SENSORS_GetBatteryLevel();
    (void)Bas_Start(&basServiceConfig);
    (void)Dis_Start(&disServiceConfig);

    /* Allocate application timer */
    {
        uint8_t i;
        for( i = 0; i < gAppMaxConnections_c; i++ )
        {
            timer_status_t timerStatus;
            maPeerDeviceId[i].deviceId = gInvalidDeviceId_c;
            timerStatus = TM_Open(maPeerDeviceId[i].timerId);
            if(timerStatus != kStatus_TimerSuccess)
            {
                panic(0,0,0,0);
            }
        }
    }
    AppPrintString("\n\rExtended Advertising Application - Peripheral");
    AppPrintString("\n\rPress WAKESW to see the menu");

    LedStopFlashingAllLeds();
    Led1On();
    Led2On();
}

/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
static void BluetoothLEHost_GenericCallback (gapGenericEvent_t* pGenericEvent)
{
    bleResult_t bleResult;

#if mAE_PeripheralDebug_c
    AppPrintString("\n\rGeneric Callback - ");
    AppPrintString(genericCBCKStrings[pGenericEvent->eventType]);
#endif
    /* Call BLE Conn Manager */
    BleConnManager_GenericEvent(pGenericEvent);
    switch (pGenericEvent->eventType)
    {
    case gExtAdvertisingParametersSetupComplete_c:
    case gExtAdvertisingDataSetupComplete_c:
        {
            maLastAdvIndexForThisHandle[maPExtAdvParam[mExtAdvAPIOwner]->handle] = mExtAdvAPIOwner;
        }
      break;
    case gPeriodicAdvParamSetupComplete_c:
        {
            bleResult = Gap_SetPeriodicAdvertisingData(gPeriodicAdvParams.handle, maPPeriodicAdvData[mPeriodicAdvDataIndex], FALSE);
            if(gBleSuccess_c != bleResult)
            {
                AppPrintString("\n\r Gap_SetPeriodicAdvertisingData Failed");
                EndSequence();
            }
        }
        break;
    case gPeriodicAdvDataSetupComplete_c:
        {
            if(gBleSuccess_c != Gap_StartPeriodicAdvertising(gPeriodicAdvParams.handle, FALSE))
            {
                AppPrintString("\n\r Gap_StartPeriodicAdvertising Failed");
                EndSequence();
            }
        }
        break;
    case gAdvertisingSetupFailed_c:
        EndSequence();
        break;
    case gExtAdvertisingSetRemoveComplete_c:
        AppPrintString(advTypeStrings[maLastAdvIndexForThisHandle[maPExtAdvParam[mExtAdvAPIOwner]->handle]]);
        AppPrintString(" Removed" );
        maLastAdvIndexForThisHandle[maPExtAdvParam[mExtAdvAPIOwner]->handle] = mAdvIndexMax_c;
        if(mExtAdvSequence == mExtAdvSeq_Start_c)
        {
            if(gBleSuccess_c != BluetoothLEHost_StartExtAdvertising(&mAppExtAdvParams, BleApp_AdvertisingCallback, BleApp_ConnectionCallback))
            {
                AppPrintString("\n\rGap_SetExtAdvertisingParameters failed");
                EndSequence();
            }
        }
        else
        {
            maAdvStatus[mExtAdvAPIOwner] = mAdvStatus_Off_c;
            EndSequence();
        }
        break;
    case gLePhyEvent_c:
        if(pGenericEvent->eventData.phyEvent.phyEventType == gPhyUpdateComplete_c )
        {
            AppPrintLePhyEvent(&pGenericEvent->eventData.phyEvent);
        }
        break;
    case gPeriodicAdvertisingStateChanged_c:

        if(maAdvStatus[mExtAdvAPIOwner] == mAdvStatus_On_c)
        {
            maAdvStatus[mExtAdvAPIOwner] = mAdvStatus_Off_c;
            AppPrintString(advTypeStrings[mExtAdvAPIOwner]);
            AppPrintString(" Stopped" );
            if(mExtAdvSequence == mExtAdvSeq_ChangePeriodicData_c)
            {
                if(gBleSuccess_c !=  Gap_SetPeriodicAdvertisingData(gPeriodicAdvParams.handle, maPPeriodicAdvData[mPeriodicAdvDataIndex], FALSE))
                {
                    AppPrintString("\n\rGap_SetPeriodicAdvertisingData failed");
                    EndSequence();
                }
            }
            else
            {
                EndSequence();
            }
        }
        else
        {
            maAdvStatus[mExtAdvAPIOwner] = mAdvStatus_On_c;
            AppPrintString(advTypeStrings[mExtAdvAPIOwner]);
            AppPrintString(" Started" );
            if(mExtAdvSequence == mExtAdvSeq_ChangePeriodicData_c)
            {
                AppPrintString(advTypeStrings[mExtAdvAPIOwner]);
                AppPrintString(" Data Changed" );
            }
            EndSequence();
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
* \brief        Handles BLE Advertising callback from host stack.
*
* \param[in]    pAdvertisingEvent    Pointer to gapAdvertisingEvent_t.
********************************************************************************** */
static void BleApp_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent)
{
#if mAE_PeripheralDebug_c
    AppPrintString("\n\rAdvertising Callback - ");
    AppPrintString(advertisingCBCKStrings[pAdvertisingEvent->eventType]);
#endif
    switch (pAdvertisingEvent->eventType)
    {

    case gAdvertisingStateChanged_c:

        break;

    case gAdvertisingCommandFailed_c:
        {
            /* Panic UI */
            EndSequence();
            panic(0,0,0,0);
        }
        break;

    case gExtAdvertisingStateChanged_c:

        if(maAdvStatus[mExtAdvAPIOwner] == mAdvStatus_On_c)
        {
            maAdvStatus[mExtAdvAPIOwner] = mAdvStatus_Off_c;
            AppPrintString(advTypeStrings[mExtAdvAPIOwner]);
            AppPrintString(" Stopped" );
            if(mExtAdvSequence == mExtAdvSeq_ChangeExtAdvData_c)
            {
                if(gBleSuccess_c != Gap_SetExtAdvertisingData(maPExtAdvParam[mExtAdvAPIOwner]->handle, maPExtAdvData[mExtAdvAPIOwner], maPExtScanData[mExtAdvAPIOwner]))
                {
                    AppPrintString("\n\rGap_SetExtAdvertisingData failed");
                    EndSequence();
                }
            }
            if(mExtAdvSequence == mExtAdvSeq_Stop_c)
            {
                maAdvStatus[mExtAdvAPIOwner] = mAdvStatus_Off_c;
                EndSequence();
            }
        }
        else
        {
            Led1Flashing();
            maAdvStatus[mExtAdvAPIOwner] = mAdvStatus_On_c;
            maLastAdvIndexForThisHandle[maPExtAdvParam[mExtAdvAPIOwner]->handle] = mExtAdvAPIOwner;
            AppPrintString(advTypeStrings[mExtAdvAPIOwner]);
            AppPrintString(" Started on handle " );
            AppPrintDec((uint32_t)maPExtAdvParam[mExtAdvAPIOwner]->handle);
            if(mExtAdvSequence == mExtAdvSeq_ChangeExtAdvData_c)
            {
                AppPrintString(advTypeStrings[mExtAdvAPIOwner]);
                AppPrintString(" Data Changed" );
            }
            EndSequence();
        }
        break;
    case gAdvertisingSetTerminated_c:
        {
            uint8_t i;
            for( i=0; i < (uint8_t)mPeriodicAdvIndex_c; i++)
            {
                if((maAdvStatus[i] ==  mAdvStatus_On_c) && (maPExtAdvParam[i]->handle == pAdvertisingEvent->eventData.advSetTerminated.handle ))
                {
                    AppPrintString(advTypeStrings[i]);
                    AppPrintString(" Terminated on handle " );
                    AppPrintDec((uint32_t)maPExtAdvParam[i]->handle);
                    maAdvStatus[i] =  mAdvStatus_Off_c;
                    Free_AdvHandler(maPExtAdvParam[i]->handle);
                    if(AllAdvertisingsAreOff())
                    {
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c == 1))
                        if(NumberOf_PeerDevices() == 0U)
                        {
#endif /* gAppLedCnt_c */
                            Led1Off();
                            Led1On();
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c == 1))
                        }
#endif /* gAppLedCnt_c */
                    }
                    break;
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
* \brief        Handles BLE Connection callback from host stack.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    pConnectionEvent    Pointer to gapConnectionEvent_t.
********************************************************************************** */
static void BleApp_ConnectionCallback (deviceId_t peerDeviceId, gapConnectionEvent_t* pConnectionEvent)
{
    /* Connection Manager to handle Host Stack interactions */
    BleConnManager_GapPeripheralEvent(peerDeviceId, pConnectionEvent);

    switch (pConnectionEvent->eventType)
    {
    case gConnEvtConnected_c:
        {
            /* Advertising stops when connected */
            /* Subscribe client*/
            (void)Add_PeerDevice(peerDeviceId);
            (void)Bas_Subscribe(&basServiceConfig, peerDeviceId);
            (void)Tms_Subscribe(peerDeviceId);
            (void)Serial_Print(gAppSerMgrIf, "\n\rConnected!\n\r", gAllowToBlock_d);
            /* UI */
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c == 1))
            LedSetColor(0, kLED_Blue);
            Led1Flashing();
#else /* gAppLedCnt_c */
            Led2Flashing();
#endif /* gAppLedCnt_c */
        }
        break;

    case gConnEvtDisconnected_c:
        {
            /* Unsubscribe client */
            Remove_PeerDevice(peerDeviceId);
            (void)Bas_Unsubscribe(&basServiceConfig, peerDeviceId);
            (void)Tms_Unsubscribe();
            (void)Serial_Print(gAppSerMgrIf, "\n\rDisconnected!\n\r", gAllowToBlock_d);
            if(NumberOf_PeerDevices() == 0U)
            {
                /* UI */
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c == 1))
                LedSetColor(0, kLED_White);
                Led1On();
#else /* gAppLedCnt_c */
                Led2Off();
                Led2On();
#endif /* gAppLedCnt_c */
            }
        }
        break;

    case gConnEvtEncryptionChanged_c:   /* Fall-through */
    default:
        {
            ; /* No action required */
        }
        break;
    }
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
            /* Notify the temperature value when CCCD is written */
            BleApp_SendTemperature(deviceId);
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
* \brief        Handles disconnect timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void DisconnectTimerCallback(void* pParam)
{
    /* Terminate connection */
    appPeerDevice_t* pAppPeerDevice = (appPeerDevice_t*)pParam;
    if (pAppPeerDevice->deviceId != gInvalidDeviceId_c)
    {
        (void)Gap_Disconnect(pAppPeerDevice->deviceId);
    }
}

/*! *********************************************************************************
* \brief        Sends temperature value over-the-air.
*
********************************************************************************** */
static void BleApp_SendTemperature(deviceId_t deviceId)
{
    uint8_t peerIndex;
    peerIndex = Get_PeerDeviceIndex(deviceId);
    if(peerIndex < gAppMaxConnections_c)
    {
        (void)TM_Stop((timer_handle_t)maPeerDeviceId[peerIndex].timerId);

        /* Update with initial temperature */
        SENSORS_TriggerTemperatureMeasurement();
        (void)SENSORS_RefreshTemperatureValue();
        
        /* Multiply temperature value by 10. SENSORS_GetTemperature() reports temperature
        value in tenths of degrees Celsius. Temperature characteristic value is degrees
        Celsius with a resolution of 0.01 degrees Celsius (GATT Specification
        Supplement v6). */
        (void)Tms_RecordTemperatureMeasurement((uint16_t)service_temperature,
                                               (int16_t)(SENSORS_GetTemperature() * 10));

         /* Start Sleep After Data timer */
         (void)TM_InstallCallback((timer_handle_t)maPeerDeviceId[peerIndex].timerId, DisconnectTimerCallback, &maPeerDeviceId[peerIndex]);
         (void)TM_Start((timer_handle_t)maPeerDeviceId[peerIndex].timerId, (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSetSecondTimer | (uint8_t)kTimerModeSingleShot, gGoToSleepAfterDataTime_c);
    }
}

/*! *********************************************************************************
* \brief        Prints a uint32_t value in decimal.
*
********************************************************************************** */
static void AppPrintDec(uint32_t nb)
{
    (void)Serial_PrintDec(gAppSerMgrIf, nb);
}
/*! *********************************************************************************
* \brief        Prints a string from the app task.
*
********************************************************************************** */
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
static void AppPrintStringCallback(void* pData)
{
    char* pBuff = (char*)pData;
    (void)Serial_Print(gAppSerMgrIf, pBuff, gAllowToBlock_d);
}
#endif /*gAppButtonCnt_c > 0*/

/*! *********************************************************************************
* \brief        Prints phy event.
*
********************************************************************************** */
static void AppPrintLePhyEvent(gapPhyEvent_t* pPhyEvent)
{
    /* String dictionary corresponding to gapLePhyMode_t */
    static char* mLePhyModeStrings[] =
    {
        "Invalid\n\r",
        "1M\n\r",
        "2M\n\r",
        "Coded\n\r",
    };
    uint8_t txPhy = (pPhyEvent->txPhy <= (uint8_t)gLePhyCoded_c) ? pPhyEvent->txPhy : (0U);
    uint8_t rxPhy = (pPhyEvent->rxPhy <= (uint8_t)gLePhyCoded_c) ? pPhyEvent->rxPhy : (0U);
    AppPrintString("Phy Update Complete.\n\r");
    AppPrintString("TxPhy ");
    AppPrintString(mLePhyModeStrings[txPhy]);
    AppPrintString("RxPhy ");
    AppPrintString(mLePhyModeStrings[rxPhy]);
}

/*! *********************************************************************************
* \brief        prints application menu.
*
* \param[in]    pData        Parameters.
********************************************************************************** */
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
static void Uart_PrintMenu(void *pData)
{
    uint8_t i;
    AppPrintString("\n\r Menu");
    for( i = 0 ; i < (uint8_t)mAdvIndexMax_c; i++)
    {
        AppPrintString((maMenu[i])[(uint8_t)maAdvStatus[i]]);
    }
    AppPrintString("\n\rPress OPTSW to choose an option\n\rThen confirm it with the WAKESW\n\r");
}

/*! *********************************************************************************
* \brief        Handles the keyboard option.
*
* \param[in]    pData        Parameters.
********************************************************************************** */
static void Key_HandleOption(void *pData)
{
    uint8_t keyOption = *((uint8_t*)pData);
    BleApp_Start(keyOption);
}
#endif /*gAppButtonCnt_c > 0*/

/*! *********************************************************************************
* \brief        Allocates the first available advertising handler .
*
* \param[in]    none
********************************************************************************** */
static uint8_t Alloc_AdvHandler(void)
{
    uint8_t i, advHandle;
    advHandle = mInvalidAdvHandle;
    OSA_InterruptDisable();
    for( i=0; i < mNumberOfAdvHandles_c; i++)
    {
        if(maAdvHandle[i] == madvHandle_Available_c)
        {
            maAdvHandle[i] = madvHandle_InUse_c;
            advHandle = i;
            break;
        }
    }
    OSA_InterruptEnable();
    return advHandle;
}

/*! *********************************************************************************
* \brief        Frees the advertising handle received as parameter.
*
* \param[in]    advHandle to free
********************************************************************************** */
static void Free_AdvHandler(uint8_t advHandle)
{
    OSA_InterruptDisable();
    if( advHandle <  mNumberOfAdvHandles_c)
    {
        if(maAdvHandle[advHandle] == madvHandle_InUse_c )
        {
            maAdvHandle[advHandle] = madvHandle_Available_c;
        }
    }
    OSA_InterruptEnable();
}

/*! *********************************************************************************
* \brief        Returns whether there is an advertising started or not.
*
* \param[in]    none
********************************************************************************** */
static bool_t AllAdvertisingsAreOff(void)
{
    bool_t result = TRUE;
    uint8_t i;
    for(i = 0; i< (uint8_t)mAdvIndexMax_c; i++)
    {
        if(maAdvStatus[i] != mAdvStatus_Off_c)
        {
            result = FALSE;
            break;
        }
    }
    return result;
}

/*! *********************************************************************************
* \brief        Function used to get access to the ext adv API.
*
* \param[in]    none
********************************************************************************** */
static apiReqStatus_t ExtAdvAPIRequest(advIndex_t advIndex)
{
    apiReqStatus_t   apiReqStatus = gApiReq_Denied_c;
    if(advIndex < mAdvIndexMax_c)
    {
        OSA_InterruptDisable();
        if(mExtAdvAPIOwner == mAdvIndexMax_c)
        {
            mExtAdvAPIOwner = advIndex;
        }
        if(mExtAdvAPIOwner == advIndex)
        {
            apiReqStatus = gApiReq_Success_c;
        }
        OSA_InterruptEnable();
    }
    return apiReqStatus;
}

/*! *********************************************************************************
* \brief        Function used to release the ext adv API.
*
* \param[in]    none
********************************************************************************** */
static void  FreeExtAdvAPI(void)
{
    mExtAdvAPIOwner = mAdvIndexMax_c;
}

/*! *********************************************************************************
* \brief        Function used to manage the end of a sequence.
*
* \param[in]    none
********************************************************************************** */
static void  EndSequence(void)
{
    mExtAdvSequence = mExtAdvSeq_Idle_c;
    if((mExtAdvAPIOwner < mPeriodicAdvIndex_c) && (maAdvStatus[mExtAdvAPIOwner] == mAdvStatus_Off_c))
    {
        Free_AdvHandler(maPExtAdvParam[mExtAdvAPIOwner]->handle);
        if(AllAdvertisingsAreOff())
        {
            Led1Off();
            Led1On();
        }
    }
    FreeExtAdvAPI();
}

/*! *********************************************************************************
* \brief        Add peer device in the peer array. Return FALSE if there is no room in the array
*               or the device is already there and TRUE otherwise.
*
* \param[in]    deviceId peer device to add
********************************************************************************** */
static bool_t Add_PeerDevice(deviceId_t deviceId)
{
    bool_t result = FALSE;
    uint8_t i;
    uint8_t firstFreeIndex = gAppMaxConnections_c;
    if( gInvalidDeviceId_c != deviceId )
    {
        result = TRUE;
        for( i = 0; i < gAppMaxConnections_c; i++ )
        {
            if(maPeerDeviceId[i].deviceId == gInvalidDeviceId_c)
            {
                if(firstFreeIndex == gAppMaxConnections_c)
                {
                    firstFreeIndex = i;
                }
            }
            else if(maPeerDeviceId[i].deviceId == deviceId)
            {
                result = FALSE;
                break;
            }
            else
            {
                ;
            }
        }
    }
    if(result == TRUE)
    {
        if(firstFreeIndex < gAppMaxConnections_c)
        {
            maPeerDeviceId[firstFreeIndex].deviceId = deviceId;
        }
        else
        {
            result = FALSE;
        }
    }
    return result;
}

/*! *********************************************************************************
* \brief        Removes peer device from the peer array.
*
* \param[in]    deviceId peer device to remove
********************************************************************************** */
static void Remove_PeerDevice(deviceId_t deviceId)
{
    uint8_t i;
    if( gInvalidDeviceId_c != deviceId )
    {
        for( i = 0; i < gAppMaxConnections_c; i++ )
        {
            if(maPeerDeviceId[i].deviceId == deviceId)
            {
                maPeerDeviceId[i].deviceId = gInvalidDeviceId_c;
                break;
            }
        }
    }
}
/*! *********************************************************************************
* \brief        Returns the peer array index of the peer device.
*
* \param[in]    deviceId peer device to search for
********************************************************************************** */
static uint8_t Get_PeerDeviceIndex(deviceId_t deviceId)
{
    uint8_t i = gAppMaxConnections_c;
    if( gInvalidDeviceId_c != deviceId )
    {
        for( i = 0; i < gAppMaxConnections_c; i++ )
        {
            if(maPeerDeviceId[i].deviceId == deviceId)
            {
                break;
            }
        }
    }
    return i;
}

/*! *********************************************************************************
* \brief        Disconnects all peer devices in the array
*
* \param[in]    none
********************************************************************************** */
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
static void Disconnect_AllPeerDevices(void)
{
    uint8_t i;
    for( i = 0; i < gAppMaxConnections_c; i++ )
    {
        if(maPeerDeviceId[i].deviceId != gInvalidDeviceId_c)
        {
            (void)Gap_Disconnect(maPeerDeviceId[i].deviceId);
        }
    }
}
#endif /*gAppButtonCnt_c > 0*/

/*! *********************************************************************************
* \brief        Returns the number of peer devices in the array
*
* \param[in]    none
********************************************************************************** */
static uint8_t NumberOf_PeerDevices(void)
{
    uint8_t i;
    uint8_t peerNo = 0;
    for( i = 0; i < gAppMaxConnections_c; i++ )
    {
        if(maPeerDeviceId[i].deviceId != gInvalidDeviceId_c)
        {
            peerNo++;
        }
    }
 return peerNo;
}

static void BleApp_SerialInit(void)
{
    serial_manager_status_t status;

    /* UI */
    gAppSerMgrIf = (serial_handle_t)&gSerMgrIf[0];

    /*open wireless uart read/write handle*/
    status = SerialManager_OpenWriteHandle((serial_handle_t)gAppSerMgrIf, (serial_write_handle_t)s_writeHandle);
    assert(kStatus_SerialManager_Success == status);
    (void)status;

    status = SerialManager_OpenReadHandle((serial_handle_t)gAppSerMgrIf, (serial_read_handle_t)s_readHandle);
    assert(kStatus_SerialManager_Success == status);
}

static void BleApp_HandleExtAdvNonConnNonScannMode(uint8_t mode)
{
    mAppExtAdvParams.duration = gBleExtAdvNoDuration_c;
    mAppExtAdvParams.maxExtAdvEvents = gBleExtAdvNoMaxEvents_c;

    if(ExtAdvAPIRequest((advIndex_t)mode) == gApiReq_Denied_c)
    {
        AppPrintString("\n\rAnother Advertising Operation in Progress. Try later...");
    }
    else
    {
        if (maAdvStatus[mode] == mAdvStatus_Off_c)
        {
            maPExtAdvParam[mode]->handle = Alloc_AdvHandler();
            if(maPExtAdvParam[mode]->handle == mInvalidAdvHandle)
            {
                AppPrintString("\n\r No Advertising Handle Available. Stop Another Advertising");
                FreeExtAdvAPI();
            }
            else
            {
                mAppExtAdvParams.pGapExtAdvParams = maPExtAdvParam[mode];
                mAppExtAdvParams.handle = maPExtAdvParam[mode]->handle;
                mAppExtAdvParams.pGapAdvData = maPExtAdvData[mode];
                mAppExtAdvParams.pScanResponseData = maPExtScanData[mode];
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
                mAppExtAdvParams.pGapDecisionData = &gAdvDecisionData;
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */

                if(((uint8_t)maLastAdvIndexForThisHandle[maPExtAdvParam[mode]->handle] == mode) || (maLastAdvIndexForThisHandle[maPExtAdvParam[mode]->handle] == mAdvIndexMax_c))
                {
                    if(gBleSuccess_c == BluetoothLEHost_StartExtAdvertising(&mAppExtAdvParams, BleApp_AdvertisingCallback, BleApp_ConnectionCallback))
                    {
                        mExtAdvSequence = mExtAdvSeq_Start_c;
                    }
                    else
                    {
                        AppPrintString("\n\rGap_SetExtAdvertisingParameters failed");
                        Free_AdvHandler(maPExtAdvParam[mode]->handle);
                        FreeExtAdvAPI();
                    }
                }
                else
                {
                    if(gBleSuccess_c == Gap_RemoveAdvSet(maPExtAdvParam[mode]->handle))
                    {
                        mExtAdvSequence = mExtAdvSeq_Start_c;
                    }
                    else
                    {
                        AppPrintString("\n\rGap_RemoveAdvSet failed");
                        Free_AdvHandler(maPExtAdvParam[mode]->handle);
                        FreeExtAdvAPI();
                    }
                }
            }
        }
        else if(maAdvStatus[mode] ==  mAdvStatus_On_c)
        {
            if((mode == (uint8_t)mExtAdvNonConnNonScanIndex_c) && (maAdvStatus[mPeriodicAdvIndex_c] == mAdvStatus_On_c) )
            {
                AppPrintString("\n\rPeriodic Advertising is ON. Stop the Periodic Advertising First");
                FreeExtAdvAPI();
            }
            else
            {
                if(gBleSuccess_c == Gap_StopExtAdvertising(maPExtAdvParam[mode]->handle))
                {
                    mExtAdvSequence = mExtAdvSeq_Stop_c;
                }
                else
                {
                    AppPrintString("\n\rGap_StopExtAdvertising failed");
                    FreeExtAdvAPI();
                }
            }
        }
        else
        {
            FreeExtAdvAPI();
        }
    }
}

static void BleApp_HandlePeriodicAdvMode(uint8_t mode)
{
    if(maAdvStatus[mExtAdvNonConnNonScanIndex_c] !=  mAdvStatus_On_c)
    {
        AppPrintString("\n\rThis Option Requires Extended Non Connectable Non Scanable Advertising to be On   ");
    }
    else
    {
        if(ExtAdvAPIRequest((advIndex_t)mode) == gApiReq_Denied_c)
        {
            AppPrintString("\n\rAnother Advertising Operation in Progress. Try later...");
        }
        else
        {
            if(maAdvStatus[mode] ==  mAdvStatus_Off_c)
            {
                gPeriodicAdvParams.handle = gExtAdvParamsNonConnNonScann.handle;
                if(Gap_SetPeriodicAdvParameters(&gPeriodicAdvParams) == gBleSuccess_c )
                {
                    mExtAdvSequence = mExtAdvSeq_Start_c;
                }
                else
                {
                    AppPrintString("\n\rGap_SetPeriodicAdvParameters failed");
                    FreeExtAdvAPI();
                }
            }
            else if(maAdvStatus[mode] ==  mAdvStatus_On_c)
            {
                if(Gap_StopPeriodicAdvertising(gPeriodicAdvParams.handle) != gBleSuccess_c )
                {
                    AppPrintString("\n\rGap_StopPeriodicAdvertising failed");
                    FreeExtAdvAPI();
                }
                else
                {
                    mExtAdvSequence = mExtAdvSeq_Stop_c;
                }
            }
            else
            {
                FreeExtAdvAPI();
            }
        }
    }
}

static void BleApp_HandleChangePerAdvDataMode(void)
{
    if(ExtAdvAPIRequest(mPeriodicAdvIndex_c) == gApiReq_Denied_c)
    {
        AppPrintString("\n\rAnother Advertising Operation in Progress. Try later...");
    }
    else
    {
        mPeriodicAdvDataIndex ^= 1U;
        if(Gap_StopPeriodicAdvertising(gPeriodicAdvParams.handle) != gBleSuccess_c )
        {
            mPeriodicAdvDataIndex ^= 1U;
            AppPrintString("\n\rGap_StopPeriodicAdvertising Failed");
            FreeExtAdvAPI();
        }
        else
        {
            mExtAdvSequence = mExtAdvSeq_ChangePeriodicData_c;
        }
    }
}

static void BleApp_HandleChangeNonConnNonScannAdvDataMode(void)
{
    if(ExtAdvAPIRequest(mExtAdvNonConnNonScanIndex_c) == gApiReq_Denied_c)
    {
        AppPrintString("\n\rAnother Advertising Operation in Progress. Try later...");
    }
    else
    {
        mExtAdvDataNCNSIndex ^= 1U;
        maPExtAdvData[mExtAdvAPIOwner] = maPExtAdvDataNonConnNonScan[mExtAdvDataNCNSIndex];

        if(gBleSuccess_c != Gap_StopExtAdvertising(maPExtAdvParam[mExtAdvAPIOwner]->handle))
        {
            AppPrintString("\n\rGap_StopExtAdvertising Failed");
            mExtAdvDataNCNSIndex ^= 1U;
            maPExtAdvData[mExtAdvAPIOwner] = maPExtAdvDataNonConnNonScan[mExtAdvDataNCNSIndex];
            FreeExtAdvAPI();
        }
        else
        {
            mExtAdvSequence = mExtAdvSeq_ChangeExtAdvData_c;
        }
    }
}
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
static void BleApp_HandleChangeDbafNonConnNonScannAdvDataMode(void)
{
    if(ExtAdvAPIRequest(mDbafAdvNonConnNonScanIndex_c) == gApiReq_Denied_c)
    {
        AppPrintString("\n\rAnother Advertising Operation in Progress. Try later...");
    }
    else
    {
        mDbafAdvDataNCNSIndex ^= 1U;
        maPExtAdvData[mExtAdvAPIOwner] = maPExtAdvDataNonConnNonScan[mDbafAdvDataNCNSIndex];

        if(gBleSuccess_c != Gap_StopExtAdvertising(maPExtAdvParam[mExtAdvAPIOwner]->handle))
        {
            AppPrintString("\n\rGap_StopExtAdvertising Failed");
            mDbafAdvDataNCNSIndex ^= 1U;
            maPExtAdvData[mExtAdvAPIOwner] = maPExtAdvDataNonConnNonScan[mDbafAdvDataNCNSIndex];
            FreeExtAdvAPI();
        }
        else
        {
            mExtAdvSequence = mExtAdvSeq_ChangeExtAdvData_c;
        }
    }
}
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
/*! *********************************************************************************
* @}
********************************************************************************** */
