/*! *********************************************************************************
* \addtogroup BLE OTAP Client ATT
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2020 NXP
* All rights reserved.
*
* \file
*
* This file is the source file for the BLE OTAP Client ATT application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "EmbeddedTypes.h"

/* Framework / Drivers */
#include "RNG_Interface.h"
#include "Keyboard.h"
#include "LED.h"
#include "TimersManager.h"
#include "FunctionLib.h"
#include "Panic.h"
#if (cPWR_UsePowerDownMode)
#include "PWR_Interface.h"
#endif
#include "OtaUtils.h"
#include "OtaSupport.h"

/* BLE Host Stack */
#include "gatt_interface.h"
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gatt_database.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"
#if !defined(MULTICORE_APPLICATION_CORE) || (!MULTICORE_APPLICATION_CORE)
#include "gatt_db_handles.h"
#endif

/* Profile / Services */
#include "battery_interface.h"
#include "device_info_interface.h"
#include "otap_interface.h"

/* Connection Manager */
#include "ble_conn_manager.h"

#include "board.h"

#define APP_DBG_LVL    DBG_LEVEL_WARNING
#include "ApplMain.h"
#include "otap_client_att.h"
#include "otap_client.h"
#include "Flash_Adapter.h"
#include "fsl_reset.h"

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
#include "erpc_host.h"
#include "dynamic_gatt_database.h"
#endif

#ifdef CPU_JN518X
#include "rom_api.h"
#include "rom_psector.h"
#endif

#if gLoggingActive_d
#include "dbg_logging.h"
#endif

#if defined (SOTA_ENABLED)
#include "blob_utils.h"
#endif

#ifdef DUAL_MODE_APP
#include "app_dual_mode_switch.h"
#endif


/************************************************************************************
*************************************************************************************
* Extern functions
*************************************************************************************
************************************************************************************/
extern void ResetMCU(void);


/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define mBatteryLevelReportInterval_c   (10)        /* battery level report interval in seconds  */

#define IMG_TYPE_ZB_FULL_APP 0x01

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
typedef enum
{
#if gAppUseBonding_d
#if defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0)
    fastWhiteListAdvState_c,
#endif
#endif
    fastAdvState_c,
    slowAdvState_c
}advType_t;

typedef struct advState_tag{
    bool_t      advOn;
    advType_t   advType;
}advState_t;

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

static deviceId_t  mPeerDeviceId = gInvalidDeviceId_c;

/* Advertising related local variables */
static advState_t   mAdvState;
static tmrTimerID_t mAdvTimerId;
static uint32_t     mAdvTimerTimeout = 0;

/* Service Data */
static bool_t      basValidClientList[gAppMaxConnections_c] = { FALSE };
static basConfig_t basServiceConfig = {(uint16_t)service_battery, 0, basValidClientList, gAppMaxConnections_c};
static disConfig_t disServiceConfig = {(uint16_t)service_device_info};

/* Application Data */
static tmrTimerID_t mBatteryMeasurementTimerId;

#ifdef gOtaImageSwitch

/* Statically allocate space for ZB OTA header within image header */
extern const uint8_t s_au8Nonce[16];
const uint8_t s_au8Nonce[16] __attribute__((used, section (".ro_nonce"))) =
    {0x00, 0x00, 0x00, 0x10, 0x11, 0x12, 0x13, 0x14,
     0x15, 0x16, 0x17, 0x18, 0x00, 0x00, 0x00, 0x00};

extern const uint8_t s_au8LnkKeyArray[16];
const uint8_t s_au8LnkKeyArray[16] __attribute__ ((used, section (".ro_se_lnkKey"))) =
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
     0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

extern const uint8_t au8OtaHeader[69];
const uint8_t au8OtaHeader[69] __attribute__ ((used, section (".ro_ota_header"))) =
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
     0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
     0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
     0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
     0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
     0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
     0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
#endif


/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

/* Gatt and Att callbacks */
static void BleApp_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent);
static void BleApp_ConnectionCallback (deviceId_t peerDeviceId, gapConnectionEvent_t* pConnectionEvent);
static void BleApp_GattServerCallback (deviceId_t deviceId, gattServerEvent_t* pServerEvent);

static void BleApp_Config(void);
static void BleApp_Advertise (void);
static void AdvertisingTimerCallback (void *pParam);
static void BatteryMeasurementTimerCallback (void *pParam);
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
    PRINTF("------------\r\n" BOARD_NAME " - OTAP CLIENT - Debug Console\r\n");
    OTA_DBG_LOG("");

#ifndef DUAL_MODE_APP
    NV_Init();
    /* Initialize application support for drivers */
    BOARD_InitAdc();
    (void) OTA_ClientInit();

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
    /* Init eRPC host */
    init_erpc_host();
#endif
#endif

    /* Initialize application specific peripheral drivers here. */
}

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(void)
{
    APP_DEBUG_TRACE("BleApp_Start\r\n");
    Led1On();

    if (mPeerDeviceId == gInvalidDeviceId_c)
    {
#if gAppUseBonding_d
#if defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0)
        if (gcBondedDevices > 0)
        {
            mAdvState.advType = fastWhiteListAdvState_c;
        }
        else
#endif
        {
#endif
            mAdvState.advType = fastAdvState_c;
#if gAppUseBonding_d
        }
#endif

        BleApp_Advertise();
    }
}

#ifdef DUAL_MODE_APP
/*! *********************************************************************************
* \brief        Stop the BLE application.

* \return TRUE if the BLE app is stopped at the end of the call, FALSE otherwise
********************************************************************************** */
bool_t BleApp_Stop(void)
{
    bool_t isBleRunning = FALSE;
    if (mPeerDeviceId == gInvalidDeviceId_c)
    {
        /* Stop advertising if it is in progress */
        if (mAdvState.advOn)
        {
            isBleRunning = TRUE;
            Gap_StopAdvertising();
        }
    }
    else
    {
        isBleRunning = TRUE;
        /* Send a disconnect */
        Gap_Disconnect(mPeerDeviceId);
    }

    if (!isBleRunning)
    {
        /* Notify the dual mode app */
        (void)App_PostCallbackMessage(dm_switch_processEvent, (void *) eBleNotRunningEvent);
    }
    return !isBleRunning;
}
#endif

#ifdef gOtaImageSwitch_d

typedef struct
{
    const  uint32_t min_valid_addr;
    const void*  root_cert;
    bool in_ota_check;
} ImageValidationArg_t;

static otaUtilsResult_t BleApp_EEPROM_ReadData(uint16_t nbBytes, uint32_t address, uint8_t *pInbuf)
{
    FLib_MemCpy(pInbuf, (void*)(address), nbBytes);
    return gOtaUtilsSuccess_c;
}

static uint32_t BleApp_ValidateImageCallback(uint32_t image_addr, void *imgValidationArgs)
{
    ImageValidationArg_t *args = (ImageValidationArg_t*)imgValidationArgs;

    return OtaUtils_ValidateImage(OtaUtils_ReadFromInternalFlash,
                                      NULL,
                                      BleApp_EEPROM_ReadData,
                                      image_addr,
                                      args->min_valid_addr,
                                      NULL,
                                      FALSE,
                                      FALSE);
}

/*! *********************************************************************************
* \brief        image switch.
*
********************************************************************************** */
void BleApp_ImageSwitch(void)
{
#ifdef CPU_JN518X
    #define LOWER_TEXT_LIMIT 0x04000

    const ImageValidationArg_t img_validation_params = {
        .min_valid_addr = LOWER_TEXT_LIMIT,
        .root_cert = NULL, /* skip authentication */
        .in_ota_check = false,
    };


     if (psector_SetPreferredApp(IMG_TYPE_ZB_FULL_APP, true,
                                BleApp_ValidateImageCallback,
                                (void*)&img_validation_params))
    {
        PRINTF("Switching to other app\r\n");
        /* Only reset if setting changed and write did happen */
        ResetMCU ();
    }
    else
    {
        PRINTF("App Switching failed\r\n");
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
    switch (events)
    {
        case gKBD_EventPressPB1_c:
        {
            BleApp_Start();
            break;
        }
	    case gKBD_EventPressPB2_c:
	    {
	        BleApp_ImageSwitch();
	        break;
	    }
        default:
            ; /* For MISRA compliance */
            break;
    }
}

#else

/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    events    Key event structure.
********************************************************************************** */
void BleApp_HandleKeys(key_event_t events)
{
    BleApp_Start();
}
#endif

/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
void BleApp_GenericCallback (gapGenericEvent_t* pGenericEvent)
{
    APP_DEBUG_TRACE("BleApp_GenericCallback pGenericEvent=0x%x type=%d\r\n", pGenericEvent, pGenericEvent->eventType);
    OTA_DBG_LOG("type=%x", pGenericEvent->eventType);
    /* Call BLE Conn Manager */
    BleConnManager_GenericEvent(pGenericEvent);

    switch (pGenericEvent->eventType)
    {
        case gInitializationComplete_c:
        {
            BleApp_Config();
        }
        break;

        case gAdvertisingParametersSetupComplete_c:
        {
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
        break;

        default:
            ; /* For MISRA compliance */
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
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
    if (GattDbDynamic_CreateDatabase() != gBleSuccess_c)
    {
        panic(0,0,0,0);
        return;
    }
#endif /* MULTICORE_APPLICATION_CORE */

    /* Common GAP configuration */
    BleConnManager_GapCommonConfig();

    /* Register stack callbacks */
    (void)App_RegisterGattServerCallback (BleApp_GattServerCallback);


    mAdvState.advOn = FALSE;

    /* Start services */
    basServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
    (void)Bas_Start(&basServiceConfig);
    (void)Dis_Start(&disServiceConfig);

    if (OtapClient_Config() == FALSE)
    {
        /* An error occurred in configuring the OTAP Client */
        panic(0,0,0,0);
    }

    /* Allocate application timer */
    mAdvTimerId = TMR_AllocateTimer();
    mBatteryMeasurementTimerId = TMR_AllocateTimer();
}

/*! *********************************************************************************
* \brief        Configures GAP Advertise parameters. Advertise will start after
*               the parameters are set.
*
********************************************************************************** */
static void BleApp_Advertise(void)
{
    APP_DEBUG_TRACE("%s\r\n", __FUNCTION__);
    OTA_DBG_LOG("");
    switch (mAdvState.advType)
    {
#if gAppUseBonding_d
#if defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0)
        case fastWhiteListAdvState_c:
        {
            gAdvParams.minInterval = gFastConnMinAdvInterval_c;
            gAdvParams.maxInterval = gFastConnMaxAdvInterval_c;
            gAdvParams.filterPolicy = gProcessWhiteListOnly_c;
            mAdvTimerTimeout = gFastConnWhiteListAdvTime_c;
        }
        break;
#endif
#endif
        case fastAdvState_c:
        {
            gAdvParams.minInterval = gFastConnMinAdvInterval_c;
            gAdvParams.maxInterval = gFastConnMaxAdvInterval_c;
            gAdvParams.filterPolicy = gProcessAll_c;
            mAdvTimerTimeout = gFastConnAdvTime_c - gFastConnWhiteListAdvTime_c;
        }
        break;

        case slowAdvState_c:
        {
            gAdvParams.minInterval = gReducedPowerMinAdvInterval_c;
            gAdvParams.maxInterval = gReducedPowerMinAdvInterval_c;
            gAdvParams.filterPolicy = gProcessAll_c;
            mAdvTimerTimeout = gReducedPowerAdvTime_c;
        }
        break;

        default:
            ; /* For MISRA compliance */
        break;
    }

    /* Set advertising parameters*/
    (void)Gap_SetAdvertisingParameters(&gAdvParams);
}

/*! *********************************************************************************
* \brief        Handles BLE Advertising callback from host stack.
*
* \param[in]    pAdvertisingEvent    Pointer to gapAdvertisingEvent_t.
********************************************************************************** */
static void BleApp_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent)
{
    APP_DEBUG_TRACE("%s pAdvertisingEvent=0x%x\r\n", __FUNCTION__, pAdvertisingEvent->eventType);
    OTA_DBG_LOG("pAdvertisingEvent=%x",pAdvertisingEvent->eventType);

    switch (pAdvertisingEvent->eventType)
    {
        case gAdvertisingStateChanged_c:
        {
            mAdvState.advOn = !mAdvState.advOn;

            if(mAdvState.advOn)
            {
                /* UI */
                LED_StopFlashingAllLeds();
                Led1Flashing();
                /* Start advertising timer */
                (void)TMR_StartLowPowerTimer(mAdvTimerId,gTmrLowPowerSecondTimer_c,
                      TmrSeconds(mAdvTimerTimeout), AdvertisingTimerCallback, NULL);
            }
#ifdef DUAL_MODE_APP
            else
            {
                /* notify the dual mode task */
                (void)App_PostCallbackMessage(dm_switch_processEvent, (void *) eBleAdvStopEvent);
            }
#endif
        }
        break;

        case gAdvertisingCommandFailed_c:
        {
            Led2On();
            panic(0,0,0,0);
        }
        break;

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
    BleConnManager_GapPeripheralEvent(peerDeviceId, pConnectionEvent);
    APP_DEBUG_TRACE("%s ConnectionEvent=0x%x\r\n", __FUNCTION__, pConnectionEvent->eventType);
    OTA_DBG_LOG("EventType=%x", pConnectionEvent->eventType);

    switch (pConnectionEvent->eventType)
    {
        case gConnEvtConnected_c:
        {
            /* Advertising stops when connected */
            mAdvState.advOn = FALSE;
            (void)TMR_StopTimer(mAdvTimerId);

            /* Subscribe client*/
            mPeerDeviceId = peerDeviceId;
            (void)Bas_Subscribe(&basServiceConfig, peerDeviceId);
            (void)OtapCS_Subscribe(peerDeviceId);

            /* UI */
            LED_StopFlashingAllLeds();
            Led1On();

#if (cPWR_UsePowerDownMode)
            /* Device does not need to sleep until some information is exchanged with the peer. */
            PWR_DisallowDeviceToSleep();
#endif

            OtapClient_HandleConnectionEvent (peerDeviceId);

            /* Start battery measurements */
            (void)TMR_StartLowPowerTimer(mBatteryMeasurementTimerId, gTmrLowPowerIntervalMillisTimer_c,
                       TmrSeconds(mBatteryLevelReportInterval_c), BatteryMeasurementTimerCallback, NULL);
        }
        break;

        case gConnEvtDisconnected_c:
        {
            /* Unsubscribe client */
            mPeerDeviceId = gInvalidDeviceId_c;
            (void)Bas_Unsubscribe(&basServiceConfig, peerDeviceId);
            (void)OtapCS_Unsubscribe();

            /* UI */
            LED_StopFlashingAllLeds();
            Led1Flashing();
            Led2Flashing();
            Led3Flashing();
            Led4Flashing();

            OtapClient_HandleDisconnectionEvent (peerDeviceId);
#ifndef DUAL_MODE_APP
            /* Restart advertising*/
            BleApp_Start();
#else
            (void)App_PostCallbackMessage(dm_switch_processEvent, (void *) eBleDisconnectionEvent);
#endif
        }
        break;

#if gAppUsePairing_d
        case gConnEvtEncryptionChanged_c:
        {

#if gAppUseBonding_d
            OtapClient_HandleEncryptionChangedEvent(peerDeviceId);
#endif
        }
        break;
#endif /* gAppUsePairing_d */

    default:
        ; /* For MISRA compliance */
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
    OTA_DBG_LOG("eventType=%x", pServerEvent->eventType);

    switch (pServerEvent->eventType)
    {
        case gEvtMtuChanged_c:
        {
            OtapClient_AttMtuChanged (deviceId,
                                  pServerEvent->eventData.mtuChangedEvent.newMtu);
        }
        break;
        case gEvtCharacteristicCccdWritten_c:
        {
            OtapClient_CccdWritten (deviceId,
                                pServerEvent->eventData.charCccdWrittenEvent.handle,
                                pServerEvent->eventData.charCccdWrittenEvent.newCccd);
        }
        break;

        case gEvtAttributeWritten_c:
        {
            OtapClient_AttributeWritten (deviceId,
                                     pServerEvent->eventData.attributeWrittenEvent.handle,
                                     pServerEvent->eventData.attributeWrittenEvent.cValueLength,
                                     pServerEvent->eventData.attributeWrittenEvent.aValue);
        }
        break;

        case gEvtAttributeWrittenWithoutResponse_c:
        {
            OtapClient_AttributeWrittenWithoutResponse (deviceId,
                                                    pServerEvent->eventData.attributeWrittenEvent.handle,
                                                    pServerEvent->eventData.attributeWrittenEvent.cValueLength,
                                                    pServerEvent->eventData.attributeWrittenEvent.aValue);
        }
        break;

        case gEvtHandleValueConfirmation_c:
        {
            OtapClient_HandleValueConfirmation (deviceId);
        }
        break;

        case gEvtError_c:
        {
            attErrorCode_t attError = (attErrorCode_t) (pServerEvent->eventData.procedureError.error & 0xFF);
            if (attError == gAttErrCodeInsufficientEncryption_c     ||
                attError == gAttErrCodeInsufficientAuthorization_c  ||
                attError == gAttErrCodeInsufficientAuthentication_c)
            {
#if gAppUsePairing_d
#if gAppUseBonding_d
                bool_t isBonded = FALSE;

                /* Check if the devices are bonded and if this is true than the bond may have
                 * been lost on the peer device or the security properties may not be sufficient.
                 * In this case try to restart pairing and bonding. */
                if (gBleSuccess_c == Gap_CheckIfBonded(deviceId, &isBonded, NULL) &&
                    TRUE == isBonded)
#endif /* gAppUseBonding_d */
                {
                    (void)Gap_SendSlaveSecurityRequest(deviceId, &gPairingParameters);
                }
#endif /* gAppUsePairing_d */
            }
        }
    break;
        default:
            ; /* For MISRA compliance */
        break;
    }
}

/*! *********************************************************************************
* \brief        Handles advertising timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void AdvertisingTimerCallback(void * pParam)
{
    /* Stop and restart advertising with new parameters */
    (void)Gap_StopAdvertising();

    switch (mAdvState.advType)
    {
#if gAppUseBonding_d
#if defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0)
        case fastWhiteListAdvState_c:
        {
            mAdvState.advType = fastAdvState_c;
        }
        break;
#endif
#endif
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
* \brief        Reads the battery level at mBatteryLevelReportInterval_c time interval.
*
********************************************************************************** */
static void BatteryMeasurementTimerCallback(void * pParam)
{
    basServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
    (void)Bas_RecordBatteryMeasurement(&basServiceConfig);
}
/*! *********************************************************************************
* @}
********************************************************************************** */
