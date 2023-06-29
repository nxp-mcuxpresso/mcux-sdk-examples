/*! *********************************************************************************
* \addtogroup Beacon
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2019 NXP
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

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
#include "PWR_Interface.h"
#include "PWR_Configuration.h"
#endif
#include "GPIO_Adapter.h"
/* BLE Host Stack */
#include "gatt_interface.h"
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gatt_database.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
#include "dynamic_gatt_database.h"
#else
#include "gatt_db_handles.h"
#endif /* MULTICORE_APPLICATION_CORE */

/* Application */
#include "ApplMain.h"
#include "beacon.h"
#include "ble_conn_manager.h"

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
#include "erpc_host.h"
#include "dynamic_gatt_database.h"
#endif /* MULTICORE_APPLICATION_CORE */

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

/* Adv Parmeters */
static bool_t mAdvertisingOn = FALSE;

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

/* Gatt and Att callbacks */
static void BleApp_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent);
static void BleApp_Config(void);
static void BleApp_Advertise(void);
#if defined (CPU_JN518X)
static void BleAppDrv_Init(bool reinit);
#if (cPWR_UsePowerDownMode)
static void BleAppDrv_InitCB(void);
static void BleAppDrv_DeInit(void);
#endif
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
void BleApp_Init(void)
{
    APP_DBG_LOG("");
#if ! defined CPU_JN518X
    uint8_t buffer[16] = {0};
    uint8_t hash[SHA256_HASH_SIZE];
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
    /* Init eRPC host */
    init_erpc_host();
#endif /* MULTICORE_APPLICATION_CORE */
#ifdef FSL_FEATURE_FLASH_ADDR_OF_VENDOR_BD_ADDR
    FLib_MemCpy(&buffer[0], (uint8_t*)FSL_FEATURE_FLASH_ADDR_OF_VENDOR_BD_ADDR, 6);
    FLib_MemCopy32Unaligned(&buffer[7], 0);
    FLib_MemCopy16Unaligned(&buffer[11], 0);
#else /* FSL_FEATURE_FLASH_ADDR_OF_VENDOR_BD_ADDR */
    /* Initialize sha buffer with values from SIM_UID */
    uint8_t len;
    BOARD_GetMCUUid(buffer, &len);
#endif /* FSL_FEATURE_FLASH_ADDR_OF_VENDOR_BD_ADDR */

    SHA256_Hash(buffer, 16, hash);

    /* Updated UUID value from advertising data with the hashed value */
    FLib_MemCpy(&gAppAdvertisingData.aAdStructures[1].aData[gAdvUuidOffset_c], hash, 16);
#else /* defined CPU_JN518X */
    BleAppDrv_Init(false);
  #if (cPWR_UsePowerDownMode)
    PWR_RegisterLowPowerExitCallback(BleAppDrv_InitCB);
    PWR_RegisterLowPowerEnterCallback(BleAppDrv_DeInit);
  #endif
#endif
}

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(void)
{
    APP_DBG_LOG("");
    if (!mAdvertisingOn)
    {
        BleApp_Advertise();
    }
}

/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    events    Key event structure.
********************************************************************************** */
void BleApp_HandleKeys(key_event_t events)
{
    APP_DBG_LOG("event=%x", events);

    switch (events)
    {
        case gKBD_EventPressPB1_c:
        {
            if (!mAdvertisingOn)
            {
                BleApp_Start();
            }
            else
            {
                (void)Gap_StopAdvertising();
            }
        }
        break;
      case gKBD_EventLongPB1_c:
      {
          if (mAdvertisingOn)
          {
              (void)Gap_StopAdvertising();
          }
      }
      break;
#if gLoggingActive_d
      case gKBD_EventPressPB2_c:
      {
          DbgLogDump(true);
          break;
      }
#endif
      default:
      {
          ; /* No action required */
          break;
      }
  }
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
#if defined (CPU_JN518X)
static void BleAppDrv_Init(bool reinit)
{
    APP_DBG_LOG("");

    /* Initialize application support for drivers */
    if ( reinit )
    {
  #if gKeyBoardSupported_d
        KBD_PrepareExitLowPower();
  #endif
    }
    else
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
}
#if (cPWR_UsePowerDownMode)
static void BleAppDrv_InitCB(void)
{
    APP_DBG_LOG("");
    BleAppDrv_Init(true);
}

static void BleAppDrv_DeInit()
{
    APP_DBG_LOG("");
    /* Deinitialize debug console */
    BOARD_DeinitDebugConsole();

    /* configure pins for power down mode */
    BOARD_SetPinsForPowerMode();
}
#endif /* cPWR_UsePowerDownMode */
#endif /* CPU_JN518X */
/*! *********************************************************************************
* \brief        Configures BLE Stack after initialization. Usually used for
*               configuring advertising, scanning, white list, services, et al.
*
********************************************************************************** */
static void BleApp_Config(void)
{
    APP_DBG_LOG("");
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
    if (GattDbDynamic_CreateDatabase() != gBleSuccess_c)
    {
        panic(0,0,0,0);
        return;
    }
#endif /* MULTICORE_APPLICATION_CORE */

    /* Read public address from controller */
    (void)Gap_ReadPublicDeviceAddress();

    /* Set Tx power level */
    Gap_SetTxPowerLevel(gAdvertisingPowerLeveldBm_c, gTxPowerAdvChannel_c);
    Gap_SetTxPowerLevel(gConnectPowerLeveldBm_c, gTxPowerConnChannel_c);

    mAdvertisingOn = FALSE;
    /* Set low power mode */
  #if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
    #if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE) &&\
        defined(gErpcLowPowerApiServiceIncluded_c) && (gErpcLowPowerApiServiceIncluded_c)
    (void)PWR_ChangeBlackBoxDeepSleepMode(cPWR_DeepSleepMode);
    PWR_AllowBlackBoxToSleep();
    #endif
        (void)PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);
        PWR_AllowDeviceToSleep();
  #endif    /* Set low power mode */
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
    PWR_DisallowDeviceToSleep();
    /* Set advertising parameters*/
    (void)Gap_SetAdvertisingParameters(&gAppAdvParams);
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
            mAdvertisingOn = !mAdvertisingOn;
            APP_DBG_LOG("AdvOn=%d", mAdvertisingOn);
            PWR_AllowDeviceToSleep();
            LED_StopFlashingAllLeds();
            Led1Flashing();

            if(!mAdvertisingOn)
            {
                Led2Flashing();
                Led3Flashing();
                Led4Flashing();
            }
            else
            {
#if defined (CPU_JN518X) && (cPWR_UsePowerDownMode)
                PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);
                PWR_AllowDeviceToSleep();
#endif
            }
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
