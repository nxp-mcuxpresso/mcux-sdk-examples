/*
* Copyright 2019, 2023-2024 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/


/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "EmbeddedTypes.h"
#include "PDM.h"
#include "app_end_device_node.h"
#include "app_main.h"
#include "bdb_api.h"
#include "zigbee_config.h"
#ifdef CLD_OTA
#include "app_ota_client.h"
#endif
#include "dbg.h"
#include "app_crypto.h"
#if IS_NOT_MCXW7x_SERIES
#include "MemManager.h"
#include "TimersManager.h"
#endif
#include "app_zcl_task.h"
#include "app_buttons.h"
#include "app_leds.h"
#ifdef ZIGBEE_EVENT_IMPL
#include "app_signals.h"
#endif
#include "PWR_Interface.h"
#include "pwrm.h"
#include "app_reporting.h"
#include "fsl_os_abstraction.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#ifndef TRACE_APP
    #define TRACE_APP   TRUE
#endif

#define HALT_ON_EXCEPTION   FALSE

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

static void APP_vInitialise(bool_t bColdStart);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

extern void *_stack_low_water_mark;
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
#if IS_MCXW7x_SERIES
static uint8_t led_states;
#endif
/**
 * Power manager Callback.
 * Called just before the device is put to sleep
 */
static void vAppPreSleep(void);
/**
 * Power manager Callback.
 * Called just after the device wakes up from sleep
 */
static void vAppWakeup(void);
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
extern void OSA_TimeInit(void);
#ifdef ZIGBEE_EVENT_IMPL
extern PWR_tsWakeTimerEvent sZTimer;
#endif
extern void vWakeCallBackZtimer(void);

#ifdef OT_ZB_SUPPORT
extern void App_ZB_WakeCallBack(void);
#endif
/****************************************************************************
 *
 * NAME: vAppMain
 *
 * DESCRIPTION:
 * Entry point for application from a cold start.
 *
 * RETURNS:
 * Never returns.
 *
 ****************************************************************************/
void vAppMain(bool_t bColdStart)
{
    APP_vInitResources();
    APP_vInitZigbeeResources();
    APP_vInitialise(bColdStart);
    if(bColdStart)
    {
        BDB_vStart();
    }
    else
    {
        BDB_vRestart();
    }

}

/****************************************************************************
 *
 * NAME: vAppRegisterPWRMCallbacks
 *
 * DESCRIPTION:
 * Power manager callback.
 * Called to allow the application to register
 * sleep and wake callbacks.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void vAppRegisterPWRCallbacks(void)
{
    PWR_RegisterLowPowerEnterCallback(vAppPreSleep);
    PWR_RegisterLowPowerExitCallback(vAppWakeup);
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: vAppPreSleep
 *
 * DESCRIPTION:
 *
 * PreSleep call back by the power manager before the controller put into sleep.
 *
 * PARAMETERS:      Name            RW  Usage
 *
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/

void vAppPreSleep(void)
{
    DBG_vPrintf(TRACE_APP, "sleeping \n");
#if IS_NOT_MCXW7x_SERIES

    /* If the power mode is with RAM held do the following
     * else not required as the entry point will init everything*/
#ifdef CLD_OTA
     vSetOTAPersistedDatForMinRetention();
#endif
     /* sleep memory held */
     vAppApiSaveMacSettings();
     /* Disable debug */
     DbgConsole_Deinit();
#else
     /* Save LED state before deinit */
     led_states = APP_u8GetLedStates();

     /*
      * Since we want the lowest current in LP, turn off the LEDs and
      * restore them at wake-up.
      */
     APP_vSetLed(APP_E_LEDS_LED_1, APP_E_LED_OFF);
     APP_vSetLed(APP_E_LEDS_LED_2, APP_E_LED_OFF);
#endif

}

void vAppWakeup(void)
{
#if IS_NOT_MCXW7x_SERIES
    /* If the power status is OK and RAM held while sleeping
     * restore the MAC settings
     * */
    if( (PMC->RESETCAUSE & ( PMC_RESETCAUSE_WAKEUPIORESET_MASK |
                     PMC_RESETCAUSE_WAKEUPPWDNRESET_MASK ) )  )
    {

        vAppApiRestoreMacSettings();
        TMR_Init();
        CRYPTO_u8RandomInit();
        CRYPTO_Init();
        MEM_Init();
        vAppMain(FALSE);
        APP_vAlignStatesAfterSleep();
    }
#else
    /* NOTE: Currently vWakeCallBack callback is called before this callback,
     * this is a known limitation of current low power system.
     */

    /* Only need to update ZCL timer for 1 second, the hardcoded sleep period */
    APP_vUpdateZCLTimer();

    /* Restore LED states */
    APP_vSetLed(APP_E_LEDS_LED_1, !!(led_states & (1 << APP_E_LEDS_LED_1)));
    APP_vSetLed(APP_E_LEDS_LED_2, !!(led_states & (1 << APP_E_LEDS_LED_2)));
#endif
    DBG_vPrintf(TRACE_APP, "woken up\n");
}

/****************************************************************************
 *
 * NAME: APP_vInitialise
 *
 * DESCRIPTION:
 * Initialises Zigbee stack, hardware and application.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
static void APP_vInitialise(bool_t bColdStart)
{
    if(bColdStart)
    {
#if IS_NOT_MCXW7x_SERIES
        PWR_ChangeDeepSleepMode((uint8_t)E_AHI_SLEEP_OSCON_RAMON);
        PWR_Init();
        PWR_vForceRadioRetention(TRUE);
#endif
        /* Initialise the Persistent Data Manager */
        PDM_eInitialise(1200, 63, NULL);
    }
    /* Initialise application */
    APP_vInitialiseEndDevice(bColdStart);
}


/****************************************************************************
 *
 * NAME: vWakeCallBack
 *
 * DESCRIPTION:
 * callback when wake timer expires.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void vWakeCallBack(void)
{
#ifdef OT_ZB_SUPPORT
    App_ZB_WakeCallBack();
#endif

    APP_ZCL_vStartTimers();
    if(ZTIMER_eStart(u8TimerPoll, POLL_TIME_FAST) != E_ZTIMER_OK)
    {
        DBG_vPrintf(TRACE_APP, "\r\nAPP: Failed to start poll");
    }
#ifdef ZIGBEE_EVENT_IMPL
    /* Timer running the ZTIMER task must be restarted and unlock the tasks */
    PWR_eScheduleActivity(&sZTimer, 250, vWakeCallBackZtimer);
    zbTaskletsSignalZps();
    zbTaskletsSignalApp();
#endif
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
