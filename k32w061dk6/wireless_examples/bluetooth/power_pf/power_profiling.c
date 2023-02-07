/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2020 NXP
* All rights reserved.
*
* \file
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
/* Framework / Drivers */
#include "RNG_Interface.h"
#include "Keyboard.h"
#include "LED.h"
#include "TimersManager.h"
#include "FunctionLib.h"
#include "MemManager.h"
#include "Panic.h"
#if (defined(CPU_QN908X)&&(CPU_QN908X > 0)) || (defined(CPU_QN909X)&&(CPU_QN909X > 0)) || defined (CPU_JN518X)
#include "fsl_power.h"
#endif
#include "fsl_iocon.h"
#include "fsl_gpio.h"
#include "fsl_wtimer.h"
#include "board.h"
#include "pin_mux.h"

/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "gatt_db_handles.h"

/* Profile / Services */
#include "battery_interface.h"
#include "device_info_interface.h"
#include "blood_pressure_interface.h"
#include "current_time_interface.h"

/* Connection Manager */
#include "ble_conn_manager.h"

#include "board.h"
#define APP_DBG_LVL DBG_LEVEL_DEBUG
#include "ApplMain.h"
#include "power_profiling.h"
#include "PWR_Interface.h"
#include "PWR_Configuration.h"
#include "controller_interface.h"
#include "GPIO_Adapter.h"

/************************************************************************************
*************************************************************************************
* Private constants & macros
*************************************************************************************
************************************************************************************/

#define mBatteryLevelReportInterval_c   (10U)           /* battery level report interval in seconds  */
#define mReadTimeCharInterval_c         (3600U)         /* interval between time sync in seconds */
#define mCharReadBufferLength_c         (13U)           /* length of the buffer */
#define mInitialTime_c                  (1451606400U)   /* initial timestamp - 01/01/2016 00:00:00 GMT */

#define LAST_POWER_STATE 4
//#define ADV_NON_CONNECTABLE

//#define PWR_PERIODIC_WAKE_TIMER_IN_MODE_2
#define PWR_WAKE_PERIOD_SECONDS        60

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

/* Adv State */
static advState_t  mAdvState;
static deviceId_t  mPeerDeviceId = gInvalidDeviceId_c;

/* Service Data*/
#if gBps_EnableMultiUserSupport_d || gBps_EnableStoredMeasurements_d
static bpsUserData_t        mUserData;
#endif
static basConfig_t      basServiceConfig = {service_battery, 0};
static bpsConfig_t      bpsServiceConfig = {service_blood_pressure, gBps_InitDefaultConfig,
                                        &mUserData};

/* Application specific data*/
static tmrTimerID_t mAdvTimerId;
static tmrTimerID_t mMeasurementTimerId;
#if gAppUseTimeService_d
static tmrTimerID_t mReadTimeCharTimerId;
#endif

static uint8_t mMeasurementSelect = 0;
static uint8_t mCurrentUserId = 1;

#if gAppUseTimeService_d
static uint8_t mOutCharReadBuffer[mCharReadBufferLength_c];
static uint16_t mOutCharReadByteCount;
#endif

static uint32_t localTime = mInitialTime_c;
#if gAppUseTimeService_d
static bool_t isTimeSynchronized = FALSE;
#endif

static uint8_t mPowerState = 0;
static bool mButtonDetected = false;
static bool mBleInitDone = false;
#ifdef ADV_NON_CONNECTABLE
static bool not_connectable = true;
#else
static bool not_connectable = false;
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
#if gAppUseTimeService_d
static void BleApp_GattClientCallback (deviceId_t serverDeviceId, gattProcedureType_t procedureType, gattProcedureResult_t procedureResult, bleResult_t error);
#endif
static void BleApp_Config(void);

/* Timer Callbacks */
static void AdvertisingTimerCallback (void *);
static void TimerMeasurementCallback (void *);
#if gAppUseTimeService_d
static void ReadTimeCharTimerCallback (void *);
#endif

static void BleApp_Advertise(void);

#define GET_POWER_PROFILING_MODE()    ((PMC->AOREG2 >> 29u) & 0x7u)
#define SET_POWER_PROFILING_MODE(val) { PMC->AOREG2 &= ~(7u << 29u); PMC->AOREG2 |= (((val) & 0x7u)<<29u);}

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/


/*!***************************************************************************
 *
 * \brief      Wtimer call back function
 *
 ****************************************************************************/
#ifdef PWR_PERIODIC_WAKE_TIMER_IN_MODE_2
static void TimerWakeCallBack(void)
{
#ifdef PWR_PERIODIC_WAKE_TIMER_IN_MODE_2
    WTIMER_StartTimer(WTIMER_TIMER1_ID, 32768*PWR_WAKE_PERIOD_SECONDS);
#endif
    APP_INFO_TRACE("Wake Timer expiration in Power State %u\r\n", mPowerState);
}

void WAKE_UP_TIMER1_IRQHandler(void)
{
    /* clear the interrupt */
    SYSCON->WKT_INTENCLR = SYSCON_WKT_INTENCLR_WKT1_TIMEOUT_MASK;
    /* clear status */
    SYSCON->WKT_STAT = SYSCON_WKT_STAT_WKT1_TIMEOUT_MASK;

    TimerWakeCallBack();
}
#endif
/*! *********************************************************************************
* \brief    Initializes application specific functionality before the BLE stack init.
*
********************************************************************************** */
static void BleAppDrv_Init(bool reinit)
{

    /* init ADC, done periodically */
    BOARD_ADCWakeupInit();
    
    WTIMER_EnableInterrupts(WTIMER_TIMER1_ID);
    if ( reinit )
    {
        APP_DBG_LOG("reinit");
#if gKeyBoardSupported_d
        KBD_PrepareExitLowPower();
#endif
#if gLEDSupported_d
        LED_PrepareExitLowPower();
#endif
    }
    mPowerState = GET_POWER_PROFILING_MODE();
}

static void BleAppDrv_InitCB(void)
{
    APP_DBG_LOG("");
    BleAppDrv_Init(true);
}

static uint8_t change_step(uint8_t step)
{
    step++;
    if (step > LAST_POWER_STATE) step = 0;
    return step;
}

static void BleAppGotoPowerMode(uint8_t power_state)
{
    uint8_t next_state;
    APP_INFO_TRACE("Power State %u\r\n", mPowerState);
    const char * pwr_state_desc[] = {
            "Power Down RAM off-Osc 32k On\r\n",
            "Power Down RAM off- Osc32k Off\r\n",
            "BLE LL Active - MCU Sleep\r\n",
            "BLE sleep enabled - Device allowed to sleep\r\n",
#ifdef ADV_NON_CONNECTABLE
            "PowerDown - Advertising non connectable\r\n",
#else
            "PowerDown - Advertising Connection\r\n"
#endif

    };

    next_state = change_step(mPowerState);

    APP_DBG_LOG("power_state=%d next will be=%d", mPowerState, next_state);

    SET_POWER_PROFILING_MODE(next_state);

    switch (mPowerState)
    {
    case 0:
    {
      Led3On(); // blue
      WTIMER_Init();
#ifdef PWR_PERIODIC_WAKE_TIMER_IN_MODE_2
      WTIMER_EnableInterrupts(WTIMER_TIMER1_ID);
      WTIMER_StartTimer(WTIMER_TIMER1_ID, 32768*PWR_WAKE_PERIOD_SECONDS);
#else
      WTIMER_StartTimer(WTIMER_TIMER1_ID, ((1<<28u)-1)); /* almost infinite timer */
#endif
      PWR_ChangeDeepSleepMode(cPWR_DeepSleep_RamOffOsc32kOn);
      PWR_AllowDeviceToSleep();
      PWR_PreventEnterLowPower(false);

      break;
    }

    case 1:
    {
        //WTIMER_StopTimer(WTIMER_TIMER1_ID);
        SET_POWER_PROFILING_MODE(next_state);
        PWR_ChangeDeepSleepMode(cPWR_DeepSleep_RamOffOsc32kOff);
        LED_TurnOffAllLeds();
        PWR_AllowDeviceToSleep();
        PWR_PreventEnterLowPower(false);
        break;
    }

    case 2:
    {
        SET_POWER_PROFILING_MODE(next_state);
        Led3Off();
        Led2On();  // green
        PWR_ChangeDeepSleepMode(cPWR_PowerDown_RamRet); 
        BLE_disable_sleep();
        PWR_AllowDeviceToSleep();
        PWR_PreventEnterLowPower(false);
        break;
     }
    case 3:
    {
        SET_POWER_PROFILING_MODE(next_state);
        Led2Off();
        Led1On();  // red
        PWR_DisallowDeviceToSleep();
        PWR_PreventEnterLowPower(true);
        break;
    }

    case 4:
    {
        SET_POWER_PROFILING_MODE(next_state);
        Led1Off();
        Led2On();  // green
        Led3On();  // blue
        PWR_ChangeDeepSleepMode(cPWR_PowerDown_RamRet);
        BLE_enable_sleep();
        PWR_AllowDeviceToSleep();
        BleApp_Start();
        PWR_PreventEnterLowPower(false);
        break;
    }
    default:
        return;
    }
    APP_DEBUG_TRACE(pwr_state_desc[mPowerState]);
    //DbgLogDump(false);
#if 0
    uint8_t dsmode = PWR_GetDeepSleepMode();

    if (   (cPWR_DeepSleep_RamOffOsc32kOff == dsmode)
      ||   (cPWR_DeepSleep_RamOffOsc32kOn == dsmode))
      {
        CLOCK_uDelay(500000);
      }
#endif
}

static void BleAppDrv_DeInit(void)
{
    APP_DBG_LOG("");
    /* DeInitialize application support for drivers */
    BOARD_DeInitAdc();

    /* configure pins for power down mode */
    BOARD_SetPinsForPowerMode();

    /* The pin mux settings below are there to compensate for a bug in the IO output configurations */
    /* Force LED outputs to become pulled-up inputs */
    IOCON_PinMuxSet(IOCON, 0, 0, IOCON_PIO_FUNC(0)| IOCON_PIO_MODE(3) | IOCON_PIO_DIGIMODE(0) );
    IOCON_PinMuxSet(IOCON, 0, 3, IOCON_PIO_FUNC(0)| IOCON_PIO_MODE(3) | IOCON_PIO_DIGIMODE(0) );
    /* Force UART1 Tx line to be set as input and pulled up */
    IOCON_PinMuxSet(IOCON, 0, 10, IOCON_PIO_FUNC(0)| IOCON_PIO_MODE(3) | IOCON_PIO_DIGIMODE(0) );
}

void BleApp_Init(void)
{
    BleAppDrv_Init(false);

#if (cPWR_UsePowerDownMode)
    PWR_RegisterLowPowerExitCallback(BleAppDrv_InitCB);
    PWR_RegisterLowPowerEnterCallback(BleAppDrv_DeInit);
#endif


}

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(void)
{
    APP_DBG_LOG("");
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
        mAdvState.advType = slowAdvState_c;
#if gAppUseBonding_d
    }
#endif

    BleApp_Advertise();
}

/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    events    Key event structure.
********************************************************************************** */
void BleApp_HandleKeys(key_event_t events)
{
    APP_DBG_LOG("");
    switch (events)
    {
        case gKBD_EventPressPB1_c:
        {   
            /* BP1 times
             * key pressed    Mode               Comments
                  0         Power down 1       RAM off 32kOsc Off - Default state after boot up
                  1         Power down 0       RAM off 32kOsc On - Allows wake up by wake timer
                  2         sleep MCU sleep,   BLE sleep
                  3         Active MCU active, BLE idle
                  4         Advertising non connectable type
               or 4         Advertising connectable  Dynamic states
            */
            mPowerState = GET_POWER_PROFILING_MODE();
            APP_INFO_TRACE("Key PB1 pressed in state=%x\r\n", mPowerState);
            if (!mBleInitDone)
            {
                mButtonDetected = true;
                /* not ready yet to go on */
                break;
            }
            if (mPowerState == 0)
            {
                if (mAdvState.advOn)
                {
                    Gap_StopAdvertising();
                    TMR_StopTimer(mAdvTimerId);
                }
                else if (mPeerDeviceId != gInvalidDeviceId_c)
                {
                    Gap_Disconnect(mPeerDeviceId);
                }
                BleAppGotoPowerMode(0);
            }
            else if (mPowerState <= LAST_POWER_STATE)
            {
                BleAppGotoPowerMode(mPowerState);
            }
            break;
        }
        default:
            break;
    }
}

/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
void BleApp_GenericCallback (gapGenericEvent_t* pGenericEvent)
{
    APP_DBG_LOG("Ev=%x", pGenericEvent->eventType);
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
            APP_INFO_TRACE("Advertising Setup failure\r\n");
        }
        break;

        /* Internal error has occurred */
        case gInternalError_c:
        {
            panic(0,0,0,0);
        }
        break;

        default:
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
    APP_DBG_LOG("");
    /* Common GAP configuration */
    BleConnManager_GapCommonConfig();

    /* Register for callbacks*/
    App_RegisterGattServerCallback(BleApp_GattServerCallback);

#if gAppUseTimeService_d
    App_RegisterGattClientProcedureCallback(BleApp_GattClientCallback);
#endif

    /* Set Tx power level */
    Gap_SetTxPowerLevel(gAdvertisingPowerLeveldBm_c, gTxPowerAdvChannel_c);
    Gap_SetTxPowerLevel(gConnectPowerLeveldBm_c, gTxPowerConnChannel_c);

    mAdvState.advOn = FALSE;

    /* Start services */
#if gBps_EnableMultiUserSupport_d
    mUserData.userId = mCurrentUserId;
#endif
#if gBps_EnableStoredMeasurements_d
    mUserData.cMeasurements = 0;
    mUserData.pStoredMeasurements = MEM_BufferAlloc(10);
#endif
    Bps_Start(&bpsServiceConfig);

    basServiceConfig.batteryLevel = 99U;
    Bas_Start(&basServiceConfig);

    /* Allocate application timers */
    mAdvTimerId = TMR_AllocateTimer();
    mMeasurementTimerId = TMR_AllocateTimer();
    uint8_t next_state = GET_POWER_PROFILING_MODE();
    mBleInitDone = true;

#ifdef GPIO_IDENTIFICATION
    if (next_state == 0)
    {
        mPowerState = 0;
        BleAppGotoPowerMode(next_state);
    }
    else if (mButtonDetected)
    {
        if (next_state > 0)
        {
            mPowerState = next_state - 1;
            BleAppGotoPowerMode(next_state);
        }
    }
#else
    if (next_state <= 2)
    {
        reset_cause_t reset_cause;

        reset_cause = POWER_GetResetCause();
        APP_INFO_TRACE("Reset Cause is %x\r\n", reset_cause);
        if (reset_cause != RESET_WAKE_PD &&
            reset_cause != RESET_WAKE_DEEP_PD)
        {
            /* On Cold Rest do the transition as if button had been pressed */
            BleAppGotoPowerMode(0);
        }
        else
        {
            const gpioInputPinConfig_t * switchPin;
            switchPin = (const gpioInputPinConfig_t *)kbdSwButtons[0].config_struct.pSwGpio;
            if (GpioIsPinCauseOfWakeup(switchPin))
            {
                while (1)
                {
                    uint32_t val = GPIO_PinRead(GPIO, switchPin->gpioPort, switchPin->gpioPin);
                    if (val) break;
                    /* spin until button is released */
                }
                BleAppGotoPowerMode(next_state);
            }
        }
    }
    else
    {
        if (mButtonDetected)
        {
            BleAppGotoPowerMode(next_state);
        }
    }
#endif

}

/*! *********************************************************************************
* \brief        Configures GAP Advertise parameters. Advertise will start after
*               the parameters are set.
*
********************************************************************************** */
static void BleApp_Advertise(void)
{
    APP_DBG_LOG("");
    uint32_t timeout = 0;

    switch (mAdvState.advType)
    {
#if gAppUseBonding_d
#if defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0)
        case fastWhiteListAdvState_c:
        {
            gAdvParams.minInterval = gFastConnMinAdvInterval_c;
            gAdvParams.maxInterval = gFastConnMaxAdvInterval_c;
            gAdvParams.filterPolicy = gProcessWhiteListOnly_c;
            timeout = gFastConnWhiteListAdvTime_c;
        }
        break;
#endif
#endif
        case fastAdvState_c:
        {
            gAdvParams.minInterval = gFastConnMinAdvInterval_c;
            gAdvParams.maxInterval = gFastConnMaxAdvInterval_c;
            gAdvParams.filterPolicy = gProcessAll_c;
            timeout = gFastConnAdvTime_c - gFastConnWhiteListAdvTime_c;
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
    }
    if (not_connectable)
    {
        gAdvParams.advertisingType = gAdvNonConnectable_c;
    }
    else
    {
        gAdvParams.advertisingType = gAdvConnectableUndirected_c;
    }
    /* Set advertising parameters*/
    Gap_SetAdvertisingParameters(&gAdvParams);
    if (timeout != 0)
    {
    /* Start advertising timer */
        APP_DBG_LOG("Start Adv Lp Timer=%x, duration=%d", mAdvTimerId, timeout);

        TMR_StartLowPowerTimer(mAdvTimerId,
                               gTmrLowPowerSecondTimer_c,
                               TmrSeconds(timeout),
                               AdvertisingTimerCallback, NULL);
    }
}

/*! *********************************************************************************
* \brief        Handles BLE Advertising callback from host stack.
*
* \param[in]    pAdvertisingEvent    Pointer to gapAdvertisingEvent_t.
********************************************************************************** */
static void BleApp_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent)
{
    APP_DBG_LOG("Ev=%x", pAdvertisingEvent->eventType);
    switch (pAdvertisingEvent->eventType)
    {
        case gAdvertisingStateChanged_c:
        {
            mAdvState.advOn = !mAdvState.advOn;
        }
        break;

        case gAdvertisingCommandFailed_c:
        {
            panic(0,0,0,0);
        }
        break;

        default:
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
    APP_DBG_LOG("Ev=%x", pConnectionEvent->eventType);

    /* Connection Manager to handle Host Stack interactions */
    BleConnManager_GapPeripheralEvent(peerDeviceId, pConnectionEvent);

    switch (pConnectionEvent->eventType)
    {
        case gConnEvtConnected_c:
        {
            mPeerDeviceId = peerDeviceId;
#if gAppUseTimeService_d
            bool_t isBonded = FALSE;

            Gap_CheckIfBonded(mPeerDeviceId, &isBonded, NULL);
            if(isBonded == FALSE)
            {
                if(isTimeSynchronized == FALSE)
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
            }
#endif
            /* Advertising stops when connected */
            mAdvState.advOn = FALSE;
            APP_INFO_TRACE("Connected\r\n");

            Bas_Subscribe(&basServiceConfig, peerDeviceId);
            Bps_Subscribe(peerDeviceId);

            Led3Off(); //blue  - off
            Led1On();  //red   - on
            Led2On();  //green - on

            /* Stop Advertising Timer*/
            TMR_StopTimer(mAdvTimerId);

            /* Start measurements */
            mMeasurementSelect = 0;
            TMR_StartLowPowerTimer(mMeasurementTimerId, gTmrLowPowerIntervalMillisTimer_c,
                       TmrSeconds(2), TimerMeasurementCallback, NULL);
            PWR_AllowDeviceToSleep();
        }
        break;

        case gConnEvtDisconnected_c:
        {
            /* Unsubscribe client */
            Bas_Unsubscribe(&basServiceConfig, peerDeviceId);
            Bps_Unsubscribe();

            mPeerDeviceId = gInvalidDeviceId_c;

            if (pConnectionEvent->eventData.disconnectedEvent.reason == gHciRemoteUserTerminatedConnection_c)
            {
              /* Connection was terminated by peer or application */
                // go to PD1 state
                LED_TurnOffAllLeds();
                PWR_ChangeDeepSleepMode(cPWR_DeepSleep_RamOffOsc32kOff); //Allow PD1
                PWR_AllowDeviceToSleep();
                mPowerState = 0;
            }
#if gAppUseTimeService_d
            TMR_StopTimer(mReadTimeCharTimerId);
            isTimeSynchronized = FALSE;
#endif
            /* Stop Measurement Timer*/
            TMR_StopTimer(mMeasurementTimerId);
        }
        break;

        case gConnEvtEncryptionChanged_c:
        {
#if gAppUseTimeService_d
            if(isTimeSynchronized == FALSE)
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
        }
        break;

        case gConnEvtPairingComplete_c:
        {
#if gAppUseTimeService_d
            if(isTimeSynchronized == FALSE)
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
        }
        break;
    default:
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
    APP_DBG_LOG("Ev=%x", pServerEvent->eventType);
    switch (pServerEvent->eventType)
    {
        case gEvtHandleValueConfirmation_c:
        {
            /* BPM received by client. Look for pending stored BPMs */
#if gBps_EnableStoredMeasurements_d
            if (mUserData.cMeasurements > 0)
            {
                Bps_SendStoredBloodPressureMeasurement(&bpsServiceConfig);
            }
#endif
        }
        break;
        default:
        break;
    }
}

/*! *********************************************************************************
* \brief        Handles GATT client callback from host stack.
*
* \param[in]    serverDeviceId          GATT Server device ID.
* \param[in]    procedureType        Procedure type.
* \param[in]    procedureResult      Procedure result.
* \param[in]    error            Callback result.
********************************************************************************** */
#if gAppUseTimeService_d
static void BleApp_GattClientCallback(
    deviceId_t              serverDeviceId,
    gattProcedureType_t     procedureType,
    gattProcedureResult_t   procedureResult,
    bleResult_t             error
)
{

    if (procedureResult == gGattProcError_c)
    {
        attErrorCode_t attError = (attErrorCode_t) (error & 0xFF);
        if (attError == gAttErrCodeInsufficientEncryption_c     ||
            attError == gAttErrCodeInsufficientAuthorization_c  ||
            attError == gAttErrCodeInsufficientAuthentication_c)
        {
            //handle auth. errors
        }

        else
        {
            //characteristic not found
        }
    }
    else if (procedureResult == gGattProcSuccess_c)
    {
        switch(procedureType)
        {
            case gGattProcReadUsingCharacteristicUuid_c:
            {
                if (mOutCharReadByteCount > 2)
                {
                    ctsDayDateTime_t time;
                    uint8_t* pValue = &mOutCharReadBuffer[3];

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
            }
            break;

            default:
                break;
            }
    }
}
#endif

/*! *********************************************************************************
* \brief        Handles advertising timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void AdvertisingTimerCallback(void * pParam)
{
    /* Stop and restart advertising with new parameters */
    APP_DBG_LOG("");
    Gap_StopAdvertising();

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
        break;
    }
    BleApp_Advertise();
}

/*! *********************************************************************************
* \brief        Handles measurement timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void TimerMeasurementCallback(void * pParam)
{
    APP_DBG_LOG("");
    uint32_t random = 0x01234567;
    bpsMeasurement_t bp;
    cuffPressureMeasurement_t cp;
    ctsDayDateTime_t time;

    RNG_GetRandomNo(&random);

    if (mMeasurementSelect < 5)
    {
        cp.unit = gBps_UnitInkPa_c;
        cp.cuffPressure = 110 + (random & 0x07);

#if gBps_EnableMultiUserSupport_d
        cp.userIdPresent = TRUE;
        cp.userId = mUserData.userId;
#else
        cp.userIdPresent = FALSE;
#endif
        cp.measurementStatusPresent = TRUE;
        cp.measurementStatus = gBps_BodyMoveDuringDetection_c;
        cp.measurementStatus |= gBps_CuffFitsProperly_c;

        Bps_RecordCuffPressureMeasurement(service_blood_pressure, &cp);
    }
    else
    {
        bp.unit = gBps_UnitInkPa_c;
        bp.systolicValue = 110 + (random & 0x07);
        bp.diastolicValue = 70 + (random & 0x07);
        bp.meanArterialPressure = (bp.systolicValue + bp.diastolicValue * 2)/3;

#if gBps_EnableMultiUserSupport_d
        bp.userIdPresent = TRUE;
        bp.userId = mUserData.userId;
#else
        bp.userIdPresent = FALSE;
#endif

        bp.pulseRatePresent = TRUE;
        bp.pulseRate = 60 + (random & 0x07) ;

        bp.timeStampPresent = TRUE;
        time = Cts_EpochToDayDateTime(localTime);
        FLib_MemCpy(&bp.timeStamp, &time.dateTime, sizeof(ctsDateTime_t));

        bp.measurementStatusPresent = TRUE;
        bp.measurementStatus = gBps_BodyMoveDuringDetection_c;
        bp.measurementStatus |= gBps_CuffFitsProperly_c;

        Bps_RecordBloodPressureMeasurement(service_blood_pressure, &bp);
    }

    mMeasurementSelect += 1;
    mMeasurementSelect = mMeasurementSelect % 6;

}

#if gAppUseTimeService_d
static void ReadTimeCharTimerCallback (void * pParam)
{
    bleUuid_t uuid = { .uuid16 = gBleSig_CurrentTime_d };

    /* Read CTS Characteristic. If the device doesn't have time services
     gAttErrCodeAttributeNotFound_c will be received. */
    GattClient_ReadUsingCharacteristicUuid
    (
        mPeerDeviceId,
        gBleUuidType16_c,
        &uuid,
        NULL,
        mOutCharReadBuffer,
        13,
        &mOutCharReadByteCount
    );
}
#endif

/*! *********************************************************************************
* @}
********************************************************************************** */
