/*! *********************************************************************************
* Copyright 2020 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */


/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "app_dual_mode_low_power.h"
#include "app_dual_mode_switch.h"
#include "MacDynamic.h"

#include "PWR_Interface.h"
#include "fsl_os_abstraction.h"

/* BLE includes */
#include "gatt_interface.h"
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "ApplMain.h"
#include "controller_interface.h"

/*
*  Low power dual mode state machine:
*   - in dm_lp_wakeup (should be called from the app in the wakeup callback):
*       o Do not init 15.4
*       o Always restart the LL by calling BLE_SetActive
*       o Disable sleep
*   - at the end of LL wake-up, BleAppWakeupEndCallback is called, an event is posted (dm_lp_processEvent) to indicate that the LL wake up ended
*       o dm_lp_processEvent content: wake up by 15.4 ? 
*           o yes: get the time before next ble event
*               1. if enough time or no event schedule
*                   o re-init the 15.4 stack 
*                   o inform dynamic mode with inactivityTime (a new call to BLE_TimeBeforeNextBleEvent should be done to get the latest value)
*               2. not enough time => do nothing (only move to state "retry after next ble event")
*                    o Catch the call to BleAppInactivityCallback (called at the end of the ble event), 
*					 o check the state "retry after next ble event" and if it is the case post a "ble wake up end" event to dm_lp_processEvent  
*           o no: do nothing  
*      
*/

/************************************************************************************
*************************************************************************************
* Private Macro
*************************************************************************************
************************************************************************************/


/************************************************************************************
*************************************************************************************
* Private definitions
*************************************************************************************
************************************************************************************/
typedef struct 
{
    bool_t b15_4_wakeUpEnded;
    bool_t bBle_wakeUpEnded;
    bool_t bRetry15_4InitAfterNextBleEvent;
} sDualModeLpStates;
/************************************************************************************
*************************************************************************************
* Extern functions/variables
*************************************************************************************
************************************************************************************/

/* This function is not implemented in the Mac_sched lib */
void vDynStopAll(void)
{
    vDynRequestState(E_DYN_SLAVE, E_DYN_STATE_OFF);
    vDynRequestState(E_DYN_MASTER, E_DYN_STATE_OFF);
}

/* BLE Link Layer extern function */
extern int BLE_SetActive(void);

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
static sDualModeLpStates dualModeLpStates;

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

void BleAppWakeupEndCallback(void)
{
    /* Notify the dual mode low power module */
    (void)App_PostCallbackMessage(dm_lp_processEvent, (void *) eBleWakeUpEnded);
}

void BleAppInactivityCallback(uint32_t inactive_time)
{
    if (dualModeLpStates.bRetry15_4InitAfterNextBleEvent)
    {
        dualModeLpStates.bRetry15_4InitAfterNextBleEvent = FALSE;
        (void)App_PostCallbackMessage(dm_lp_processEvent, (void *) eBleWakeUpEnded);
    }
    else
    {
        /* Check if a radio re-calibration is required */
        if (XCVR_GetRecalDuration() != 0)
        {
            BLE_get_sleep_mode();
            inactive_time = BLE_TimeBeforeNextBleEvent();
        }
        vMac_DynRadioAvailable(inactive_time);
    }
}

void dm_lp_init(void)
{
    dualModeLpStates.b15_4_wakeUpEnded = TRUE;
    dualModeLpStates.bBle_wakeUpEnded = TRUE;
    dualModeLpStates.bRetry15_4InitAfterNextBleEvent = FALSE;	
}

void dm_lp_preSleep(void)
{
    dualModeLpStates.b15_4_wakeUpEnded = FALSE;
    dualModeLpStates.bBle_wakeUpEnded = FALSE;
    /* Stop the dynamic module */
    vDynStopAll();	
}

void dm_lp_wakeup(void)
{
    /* Force the ble LL to wake up */
    BLE_SetActive();
    /* Disable sleep until ble wake-up ended event */
    PWR_DisallowDeviceToSleep();	
}

void dm_lp_processEvent(void *pParam)
{
    uint32_t event = (uint32_t) pParam;

    switch (event)
    {
        case eBleWakeUpEnded:
            if (!dualModeLpStates.bBle_wakeUpEnded)
            {
                PWR_AllowDeviceToSleep();
                dualModeLpStates.bBle_wakeUpEnded = TRUE;
            }
            break;
        case e15_4WakeUpEnded:
            dualModeLpStates.b15_4_wakeUpEnded = TRUE;
            break;
        default:
            break;
    }

    /* Do we need to re-initialize the 15.4 app/stack ? */
    if (dualModeLpStates.bBle_wakeUpEnded && dualModeLpStates.b15_4_wakeUpEnded)
    {
        OSA_DisableIRQGlobal();
        uint32_t inactive_time = BLE_TimeBeforeNextBleEvent();
        uint32_t initTime15_4 = dm_switch_get15_4InitWakeUpTime();
        SWITCH_DBG_LOG("nextActivity in %d, initTime15_4 = %d", inactive_time, initTime15_4);
        /* Do we have enough time to init the 15.4 app/stack ? */
        if (inactive_time > initTime15_4)
        {
            OSA_EnableIRQGlobal();
            dm_switch_init15_4AfterWakeUp();
            /* Inform the dynamic mode of the next ble event */
            vMac_DynRadioAvailable(BLE_TimeBeforeNextBleEvent());
        }
        else
        {
            dualModeLpStates.bRetry15_4InitAfterNextBleEvent = TRUE;
            OSA_EnableIRQGlobal();
        }
    }
}


/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/