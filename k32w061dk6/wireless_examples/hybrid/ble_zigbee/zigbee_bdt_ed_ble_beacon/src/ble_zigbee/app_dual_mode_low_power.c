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
#include "MacSched.h"

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
*   - in dm_lp_wakeup() (should be called from the app in the wakeup callback):
*     wake 15.4, LL up (either of them could be up already)
*
*   - in BleAppWakeupEndCallback() wake 15.4 too.
*     The assumption is that BleAppWakeupEndCallback() is alwasys called
*
*   - at the end of any LL callback (radio is in BLE mode)
*     init 15.4 if there are no pending LL events or idle time allows it
*
*   - prevent low power mode until 15.4 is initialized
*
*   - at the end of dm_switch_init15_4AfterWakeUp(), 15.4 should be inactive, BLE active
*/

/************************************************************************************
*************************************************************************************
* Private Macro
*************************************************************************************
************************************************************************************/
#define LL_MAX_LIMIT 0xf0000000u

/************************************************************************************
*************************************************************************************
* Private definitions
*************************************************************************************
************************************************************************************/
typedef struct 
{
    volatile bool_t b15_4_wakeUpEnded;
    volatile bool_t bBle_wakeUpEnded;
    volatile bool_t b15_4Initialized;
} sDualModeLpStates;
/************************************************************************************
*************************************************************************************
* Extern functions/variables
*************************************************************************************
************************************************************************************/

/* BLE Link Layer extern function */
extern int BLE_SetActive(void);

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/
uint32_t dm_lp_reinit_15_4(uint32_t);

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
    /* Wake 15.4 up too */
    dm_lp_processEvent((void *) e15_4WakeUpEnded);

    if (BLE_TimeBeforeNextBleEvent() >= LL_MAX_LIMIT)
    {
        /* No pending LL events */
        dm_lp_reinit_15_4(LL_MAX_LIMIT);

        if (dualModeLpStates.b15_4Initialized)
        {
            /* start 15.4 */
            vMac_DynRadioAvailable(LL_MAX_LIMIT);
        }
    }
}

void BleAppInactivityCallback(uint32_t inactive_time)
{
    /* Check if a radio re-calibration is required */
    if (XCVR_GetRecalDuration() != 0)
    {
        BLE_get_sleep_mode();   /* does radio recalibration */
        inactive_time = BLE_TimeBeforeNextBleEvent();
    }

    uint32_t dt = dm_lp_reinit_15_4(inactive_time);

    if (dualModeLpStates.b15_4Initialized)
    {
        vMac_DynRadioAvailable(dt);
    }
}

int BleAppNewActivityCallback(uint32_t activity_time)
{
    uint32_t dt = dm_lp_reinit_15_4(activity_time);

    if (dualModeLpStates.b15_4Initialized)
    {
        return eMac_DynActivityAdded(dt);
    }
    else
    {
        return E_DYN_OK;
    }
}

void dm_lp_init(void)
{
    dualModeLpStates.b15_4_wakeUpEnded = TRUE;
    dualModeLpStates.bBle_wakeUpEnded = TRUE;
    dualModeLpStates.b15_4Initialized = TRUE;
}

void dm_lp_preSleep(void)
{
    dualModeLpStates.b15_4_wakeUpEnded = FALSE;
    dualModeLpStates.bBle_wakeUpEnded = FALSE;
    dualModeLpStates.b15_4Initialized = FALSE;

    /* Stop the dynamic module */
    vDynStopAll();	
}

void dm_lp_wakeup(void)
{
    /* Wake 15.4 up */
    dm_lp_processEvent((void *) e15_4WakeUpEnded);

    /* Wake LL up */
    BLE_SetActive();
}

void dm_lp_processEvent(void *pParam)
{
    OSA_InterruptDisable();

    uint32_t event = (uint32_t) pParam;

    switch (event)
    {
        case eBleWakeUpEnded:
            dualModeLpStates.bBle_wakeUpEnded = TRUE;
            break;
        case e15_4WakeUpEnded:
            if (!dualModeLpStates.b15_4_wakeUpEnded)
            {
                dualModeLpStates.b15_4_wakeUpEnded = TRUE;
                PWR_DisallowDeviceToSleep();
            }
            break;
        default:
            break;
    }

    OSA_InterruptEnable();
}

/* Must be called when LL is idle.
   Returns remaining time */
uint32_t dm_lp_reinit_15_4(uint32_t inactive_ble_time)
{
    uint32_t dt = inactive_ble_time;

    OSA_InterruptDisable();

    /* Do we need to re-initialize the 15.4 app/stack ? */
    if (dualModeLpStates.b15_4_wakeUpEnded && !dualModeLpStates.b15_4Initialized)
    {
        uint32_t initTime15_4 = dm_switch_get15_4InitWakeUpTime();
        SWITCH_DBG_LOG("nextActivity in %d, initTime15_4 = %d", inactive_ble_time, initTime15_4);

        /* Do we have enough time to init the 15.4 app/stack ? */
        if (inactive_ble_time > initTime15_4)
        {
            dm_switch_init15_4AfterWakeUp();
            /* it updates the 15.4 init time */
            /* 15.4 should be inactive, BLE active */

            dualModeLpStates.b15_4Initialized = TRUE;
            PWR_AllowDeviceToSleep();

            /* Get remaining time */
            initTime15_4 = dm_switch_get15_4InitWakeUpTime();
            if (dt >= initTime15_4)
            {
                dt -= initTime15_4;
            }
            else
            {
                dt = 0;
            }
        }
    }
    OSA_InterruptEnable();

    return dt;
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/
