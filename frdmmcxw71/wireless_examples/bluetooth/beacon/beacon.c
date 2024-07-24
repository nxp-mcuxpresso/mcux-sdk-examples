/*! *********************************************************************************
* \addtogroup Beacon
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright 2015 Freescale Semiconductor, Inc.
* Copyright 2016-2024 NXP
*
*
* \file
*
* This file is the source file for the Beacon application
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
#include "fsl_component_panic.h"
#include "FunctionLib.h"
#include "fsl_os_abstraction.h"
#include "SecLib.h"
#include "fsl_device_registers.h"
#include "fwk_platform.h"

/* BLE Host Stack */
#include "gatt_interface.h"
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gatt_database.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"
#include "gatt_db_handles.h"

/* Application */
#include "app.h"
#include "app_conn.h"
#include "app_advertiser.h"
#include "beacon.h"
#include "ble_conn_manager.h"

/*************************************************************************************
**************************************************************************************
* Private macros
**************************************************************************************
*************************************************************************************/
#define    gKBD_EventPB1_c  1U   /* Pushbutton 1 */

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
#if defined(gBeaconAE_c) && (gBeaconAE_c)
typedef enum
{
  mAppState_NoAdv_c,
  mAppState_LegacyAdv_c,
  mAppState_ExtAdv_c,
  mAppState_PeriodicAdv_c,
  mAppState_Legacy_ExtAdv_c
}mAppTargetState_t;
#endif

typedef uint8_t key_event_t;
/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
static bleDeviceAddress_t maBleDeviceAddress;

/* Adv Parameters */
static bool_t mAdvertisingOn = FALSE;

#if defined(gBeaconAE_c) && (gBeaconAE_c)
static bool_t mExtAdvertisingOn = FALSE;
static bool_t mPeriodicAdvOn = FALSE;
static mAppTargetState_t mAppTargetState = mAppState_NoAdv_c;
#endif /*gBeaconAE_c */

static appAdvertisingParams_t mAppAdvParams = {
    &gAppAdvParams,
    &gAppAdvertisingData,
    NULL
};
static appExtAdvertisingParams_t mAppExtAdvParams = {
    &gExtAdvParams,
    &gAppExtAdvertisingData,
    NULL,
    mBeaconExtHandleId_c,
    gBleExtAdvNoDuration_c,
    gBleExtAdvNoMaxEvents_c
};

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/
/* Gatt and Att callbacks */
static void BleApp_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent);
static void BluetoothLEHost_Initialized(void);

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0U))
static void BleApp_HandleKeys(key_event_t events);
#endif /* (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0)) */

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0U))
button_status_t BleApp_HandleKeys0(void *buttonHandle, button_callback_message_t *message,void *callbackParam);
#endif

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief  This is the initialization function for each application. This function
*         should contain all the initialization code required by the bluetooth demo
********************************************************************************** */
void BluetoothLEHost_AppInit(void)
{
    uint8_t buffer[16] = {0U};
    uint8_t hash[SHA256_HASH_SIZE] = {0U};
    uint8_t len = 0U;

    /* Initialize Bluetooth Host Stack */
    BluetoothLEHost_SetGenericCallback(BleApp_GenericCallback);
    BluetoothLEHost_Init(BluetoothLEHost_Initialized);

    /* Initialize sha buffer with values from SIM_UID */
    PLATFORM_GetMCUUid(buffer, &len);
    SHA256_Hash(buffer, len, hash);

    /* Updated UUID value from advertising data with the hashed value */
    FLib_MemCpy(&gAppAdvertisingData.aAdStructures[1].aData[gAdvUuidOffset_c], hash, 16);

    /* Register callbacks for button press */
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0U))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[0], BleApp_HandleKeys0, NULL);
#endif
}

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(void)
{
    if (!mAdvertisingOn)
    {
        if (gBleSuccess_c != BluetoothLEHost_StartAdvertising(&mAppAdvParams, BleApp_AdvertisingCallback, NULL))
        {
            panic(0, 0, 0, 0);
        }
    }
}

/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    events    Key event structure.
********************************************************************************** */
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0U))
button_status_t BleApp_HandleKeys0(void *buttonHandle, button_callback_message_t *message,void *callbackParam)
{
    (void)buttonHandle;
    (void)callbackParam;
    switch (message->event)
    {
        case kBUTTON_EventOneClick:
        case kBUTTON_EventShortPress:
            {
                BleApp_HandleKeys(gKBD_EventPB1_c);
                break;
            }

        default:
            {
                ; /* No action required */
                break;
            }
    }

    return kStatus_BUTTON_Success;
}
#endif /*gAppButtonCnt_c > 0*/

/*! *********************************************************************************
* \brief        Handles gap generic events.
*
* \param[in]    pGenericEvent    Gap Generic event structure.
********************************************************************************** */
void BleApp_GenericCallback (gapGenericEvent_t* pGenericEvent)
{
    switch (pGenericEvent->eventType)
    {
    case gPublicAddressRead_c:
        {
            /* Use address read from the controller */
            FLib_MemCpyReverseOrder(maBleDeviceAddress, pGenericEvent->eventData.aAddress, sizeof(bleDeviceAddress_t));
        }
        break;

#if defined(gBeaconAE_c) && (gBeaconAE_c)

    case gExtAdvertisingSetRemoveComplete_c:
        {
            if((mAppTargetState == mAppState_PeriodicAdv_c) || (mAppTargetState == mAppState_Legacy_ExtAdv_c))
            {
                if (gBleSuccess_c != BluetoothLEHost_StartExtAdvertising(&mAppExtAdvParams, BleApp_AdvertisingCallback, NULL))
                {
                    panic(0, 0, 0, 0);
                }
            }
            else if((mAppTargetState == mAppState_NoAdv_c) && (mAdvertisingOn == TRUE))
            {
                if (gBleSuccess_c != Gap_StopAdvertising())
                {
                    panic(0, 0, 0, 0);
                }
            }
            else
            {
                ;/* MISRA compliance*/
            }
        }
        break;

    case gPeriodicAdvParamSetupComplete_c:
        {
            if(gBleSuccess_c != Gap_SetPeriodicAdvertisingData(gExtAdvParams.handle, &gAppExtAdvertisingData, FALSE))
            {
                panic(0, 0, 0, 0);
            }
        }
        break;

    case gPeriodicAdvDataSetupComplete_c:
        {
            if(gBleSuccess_c != Gap_StartPeriodicAdvertising(gExtAdvParams.handle, FALSE))
            {
                panic(0, 0, 0, 0);
            }
        }
        break;

    case gPeriodicAdvertisingStateChanged_c:
        {
            mPeriodicAdvOn = !mPeriodicAdvOn;
            if(mPeriodicAdvOn)
            {
              
#if defined(gAppLedCnt_c) && (gAppLedCnt_c == 1)
                Led1Off();
                LedSetColor(0, kLED_Blue);
                Led1On();
#else
                Led2Flashing();
#endif /* gAppLedCnt_c == 1*/           
              
            }
            else
            {
#if defined(gAppLedCnt_c) && (gAppLedCnt_c == 1)
                LedSetColor(0, kLED_Blue);
                Led1Flashing();
#else
                LedStopFlashingAllLeds();
                Led2On();
#endif /* gAppLedCnt_c == 1*/
                Led1On();
                if(mAppTargetState == mAppState_Legacy_ExtAdv_c)
                {
                    if(gBleSuccess_c != Gap_StopExtAdvertising(gExtAdvParams.handle))
                    {
                        panic(0, 0, 0, 0);
                    }
                }
            }
        }
        break;
#endif /*gBeaconAE_c */

    case gAdvertisingSetupFailed_c:
        {
            panic(0,0,0,0);
        }
        break;

        /* Internal error has occurred */
    case gInternalError_c:
        {
            panic(0,0,0,0);
        }
        break;

    default:
        {
            ; /* No action required */
        }
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
*               configuring advertising, scanning, filter accept list, services, et al.
*
********************************************************************************** */
static void BluetoothLEHost_Initialized(void)
{
    /* Read public address from controller */
    (void)Gap_ReadPublicDeviceAddress();
    mAdvertisingOn = FALSE;

    LedStopFlashingAllLeds();
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
        /* Advertising state changed */
    case gAdvertisingStateChanged_c:
        {
            mAdvertisingOn = !mAdvertisingOn;
            if(mAdvertisingOn)
            {
                Led1On();
            }
            else
            {
                Led1Off();
#if defined(gBeaconAE_c) && (gBeaconAE_c)
                if(mAppTargetState == mAppState_ExtAdv_c)
                {
                    if (gBleSuccess_c != BluetoothLEHost_StartExtAdvertising(&mAppExtAdvParams, BleApp_AdvertisingCallback, NULL))
                    {
                        panic(0, 0, 0, 0);
                    }
                }
#endif
            }
        }
        break;

#if defined(gBeaconAE_c) && (gBeaconAE_c)
    case gExtAdvertisingStateChanged_c:
        {
            mExtAdvertisingOn = !mExtAdvertisingOn;
            if(mExtAdvertisingOn)
            {
#if defined(gAppLedCnt_c) && (gAppLedCnt_c == 1)
                Led1Flashing();
#else
                Led2On();
#endif /* gAppLedCnt_c == 1*/
                if(mAppTargetState == mAppState_PeriodicAdv_c)
                {
                    if (gBleSuccess_c != Gap_SetPeriodicAdvParameters(&gPeriodicAdvParams))
                    {
                        panic(0, 0, 0, 0);
                    }
                }
                else if(mAppTargetState == mAppState_Legacy_ExtAdv_c)
                {
#if defined(gAppLedCnt_c) && (gAppLedCnt_c ==1)
                    LedSetColor(0, kLED_Blue);
                    Led1Flashing();
#endif /* gAppLedCnt_c == 1*/
                  
                    if (gBleSuccess_c != BluetoothLEHost_StartAdvertising(&mAppAdvParams, BleApp_AdvertisingCallback, NULL))
                    {
                        panic(0, 0, 0, 0);
                    }
                }
                else
                {
                    ;/*MISRA compliance*/
                }
            }
            else
            {
#if defined(gAppLedCnt_c) && (gAppLedCnt_c == 1)
                Led1Off();
#else
                Led2Off();
#endif /* gAppLedCnt_c == 1*/
                (void)Gap_RemoveAdvSet(gExtAdvParams.handle);
            }
        }
        break;

#endif /*gBeaconAE_c */

        /* Advertising command failed */
    case gAdvertisingCommandFailed_c:
        {
            panic(0,0,0,0);
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
* \brief        Handle the buttons.
*
* \param[in]    events   the button event.
********************************************************************************** */
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
#if defined(gBeaconAE_c) && (gBeaconAE_c)
static void BleApp_HandleKeys(key_event_t events)
{
    switch (events)
    {
    case gKBD_EventPB1_c:
        {
            /* First press starts legacy advertising */
            if ((mAdvertisingOn == FALSE) && (mExtAdvertisingOn == FALSE) && (mPeriodicAdvOn == FALSE))
            {
                mAppTargetState = mAppState_LegacyAdv_c;
                BleApp_Start();
            }
            /* Second press starts extended advertising  and stops legacy advertising */
            else if ((mAdvertisingOn == TRUE) && (mExtAdvertisingOn == FALSE) && (mPeriodicAdvOn == FALSE))
            {
                mAppTargetState = mAppState_ExtAdv_c;
                mAppExtAdvParams.pGapAdvData = &gAppExtAdvertisingData;
                if (gBleSuccess_c != Gap_StopAdvertising())
                {
                    panic(0, 0, 0, 0);
                }
            }
            
#if(defined(gLlUsePeriodicAdvertising_d) && gLlUsePeriodicAdvertising_d)
            /* Third press starts extended advertising without data and periodic advertising */
            else if ((mAdvertisingOn == FALSE) && (mExtAdvertisingOn == TRUE) && (mPeriodicAdvOn == FALSE))
            {
                mAppTargetState = mAppState_PeriodicAdv_c;
                mAppExtAdvParams.pGapAdvData = &gAppExtAdvertisingNoData;
                if(gBleSuccess_c != Gap_StopExtAdvertising(gExtAdvParams.handle))
                {
                    panic(0, 0, 0, 0);
                }
            }
            /* Fourth press starts legacy advertising and extended advertising */
            else if ((mAdvertisingOn == FALSE) && (mExtAdvertisingOn == TRUE) && (mPeriodicAdvOn == TRUE))
            {
                mAppTargetState = mAppState_Legacy_ExtAdv_c;
                (void)Gap_StopPeriodicAdvertising(gExtAdvParams.handle);
            }
#else
            /* Third press starts legacy advertising and extended advertising */
            else if ((mAdvertisingOn == FALSE) && (mExtAdvertisingOn == TRUE))
            {
                mAppTargetState = mAppState_Legacy_ExtAdv_c;
                if(gBleSuccess_c != Gap_StopExtAdvertising(gExtAdvParams.handle))
                {
                    panic(0, 0, 0, 0);
                }
            }
#endif /*gLlUsePeriodicAdvertising_d */
            
            /* Last press stops extended advertising and legacy advertising*/
            else if ((mAdvertisingOn == TRUE) && (mExtAdvertisingOn == TRUE) && (mPeriodicAdvOn == FALSE))
            {
                mAppTargetState = mAppState_NoAdv_c;
                if(gBleSuccess_c != Gap_StopExtAdvertising(gExtAdvParams.handle))
                {
                    panic(0, 0, 0, 0);
                }
            }
            else
            {
                ; /* No action required - MISRA rule 15.7 */
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

#else /* defined(gBeaconAE_c) && (gBeaconAE_c) */
static void BleApp_HandleKeys(key_event_t events)
{
    switch (events)
    {
        case gKBD_EventPB1_c:
        {
            /* First press starts legacy advertising */
            if (mAdvertisingOn == FALSE)
            {
                BleApp_Start();
            }
            else
            {
                if (gBleSuccess_c != Gap_StopAdvertising())
                {
                    panic(0, 0, 0, 0);
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
#endif /* defined(gBeaconAE_c) && (gBeaconAE_c) */
#endif /*(defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))*/

/*! *********************************************************************************
* \brief  This is a dummy callback to allow using app_advertiser.c without app_connection.c
********************************************************************************** */
void App_ConnectionCallback
(
    deviceId_t            peerDeviceId,
    gapConnectionEvent_t* pConnectionEvent
)
{
    return;
}

/*! *********************************************************************************
* @}
********************************************************************************** */
