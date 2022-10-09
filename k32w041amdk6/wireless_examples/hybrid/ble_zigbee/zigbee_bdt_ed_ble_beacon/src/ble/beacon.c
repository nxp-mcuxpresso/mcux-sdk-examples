/*! *********************************************************************************
* \addtogroup Beacon
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2020 NXP
* All rights reserved.
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
#include "Keyboard.h"
#include "LED.h"
#include "TimersManager.h"
#include "FunctionLib.h"
#include "fsl_os_abstraction.h"
#include "Panic.h"
#include "SecLib.h"
#include "fsl_device_registers.h"
#include "PWR_Interface.h"
#include "PWR_Configuration.h"


#include "GPIO_Adapter.h"
/* BLE Host Stack */
#include "gatt_interface.h"
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gatt_database.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"

#include "gatt_db_handles.h"

/* Application */
#include "ApplMain.h"
#include "beacon.h"
#include "ble_conn_manager.h"

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
static bleDeviceAddress_t maBleDeviceAddress;

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

/* Gatt and Att callbacks */
static void BleApp_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent);
static void BleApp_Config(void);
static void BleApp_Advertise(void);

/***********************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

void BleAppDrv_Init(bool reinit)
{
    /* Initialize application support for drivers */
    if ( !reinit )
    {
        (void) reinit;
        uint8_t buffer[16] = {0};
        uint8_t hash[SHA256_HASH_SIZE];

        uint8_t len;
        BOARD_GetMCUUid(buffer, &len);

        SHA256_Hash(buffer, 16, hash);

        /* Updated UUID value from advertising data with the hashed value */
        FLib_MemCpy(&gAppAdvertisingData.aAdStructures[1].aData[gAdvUuidOffset_c], hash, 16);
    }
    else
    {
#if gKeyBoardSupported_d
        KBD_PrepareExitLowPower();
#endif
#if gLEDSupported_d
        LED_PrepareExitLowPower();
#endif
    }
}

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(void)
{

    BleApp_Advertise();
}

/*! *********************************************************************************
* \brief        Handles gap generic events.
*
* \param[in]    pGenericEvent    Gap Generic event structure.
********************************************************************************** */
void BleApp_GenericCallback (gapGenericEvent_t* pGenericEvent)
{
    APP_DBG_LOG("eventType=%x", pGenericEvent->eventType);

    switch (pGenericEvent->eventType)
    {
        /* Host Stack initialization is complete */
        case gInitializationComplete_c:
        {
            /* Stack configuration */
            BleApp_Config();
        }
        break;

        case gPublicAddressRead_c:
        {
            /* Use address read from the controller */
            FLib_MemCpyReverseOrder(maBleDeviceAddress, pGenericEvent->eventData.aAddress, sizeof(bleDeviceAddress_t));
        }
        break;

        case gAdvertisingParametersSetupComplete_c:
        {
            (void)Gap_SetAdvertisingData(&gAppAdvertisingData, NULL);
        }
        break;

        case gAdvertisingDataSetupComplete_c:
        {
            (void)App_StartAdvertising(BleApp_AdvertisingCallback, NULL);
        }
        break;

        case gAdvertisingSetupFailed_c:
        {
            APP_INFO_TRACE("Advertising Setup failure\r\n");
            //panic(0,0,0,0);
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
*               configuring advertising, scanning, white list, services, et al.
*
********************************************************************************** */
static void BleApp_Config(void)
{
    /* Read public address from controller */
    (void)Gap_ReadPublicDeviceAddress();
}

/*! *********************************************************************************
* \brief        Configures GAP Advertise parameters. Advertise will start after
*               the parameters are set.
*
********************************************************************************** */
static void BleApp_Advertise(void)
{
    APP_DBG_LOG("");
    APP_DEBUG_TRACE("BleApp_Advertise\r\n");
    /* Set advertising parameters*/
    (void)Gap_SetAdvertisingParameters(&gAppAdvParams);

    PWR_ChangeDeepSleepMode(cPWR_PowerDown_RamRet);
    PWR_AllowDeviceToSleep();
}

/*! *********************************************************************************
* \brief        Handles BLE Advertising callback from host stack.
*
* \param[in]    pAdvertisingEvent    Pointer to gapAdvertisingEvent_t.
********************************************************************************** */
static void BleApp_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent)
{
    APP_DBG_LOG("eventType=%x", pAdvertisingEvent->eventType);

    switch (pAdvertisingEvent->eventType)
    {
        /* Advertising state changed */
        case gAdvertisingStateChanged_c:
        {
            PWR_ChangeDeepSleepMode(cPWR_PowerDown_RamRet);
        }
        break;

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
* @}
********************************************************************************** */
