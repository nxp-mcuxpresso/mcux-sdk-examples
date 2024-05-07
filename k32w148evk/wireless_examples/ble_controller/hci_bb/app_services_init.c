/*
 * Copyright 2020 - 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*${header:start}*/
#include "app.h"
#include "board.h"
#include "fwk_platform.h"

#if (defined(gAppForceLvdResetOnResetPinDet_d) && (gAppForceLvdResetOnResetPinDet_d > 0)) || \
    (defined(gAppForceDeepPowerDownResetOnResetPinDet_d) && (gAppForceDeepPowerDownResetOnResetPinDet_d > 0))
#include "fwk_platform_reset.h"
#endif

#if ((defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0)) ||               \
     (defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0)) || \
     (defined(gAppLedCnt_c) && (gAppLedCnt_c > 0)))
/* board_comp.h only required for above services   */
#include "board_comp.h"
#endif

#if defined(gAppUseSensors_d) && (gAppUseSensors_d > 0)
#include "sensors.h"
#endif

#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d > 0)
#include "fsl_pm_device.h"
#include "fsl_pm_core.h"
#include "PWR_Interface.h"
#include "board_lp.h"
#endif

#if defined(BOARD_LOCALIZATION_REVISION_SUPPORT) && (BOARD_LOCALIZATION_REVISION_SUPPORT > 0)
#include "pin_mux.h"
#endif

/*${header:end}*/

/* -------------------------------------------------------------------------- */
/*                               Private macros                               */
/* -------------------------------------------------------------------------- */

/* Define constraints for LPUART0 wake up so we can set/release the constraints at runtime based on application call
 * Wake up on serial can be done only on LPUART0 and FRO6M which are located in WAKE domain
 * We need to use the FRO6M as clock source for LPUART0
 * Regarding constraints, we need to keep the FRO6M running and keep the WAKE domain operational (SLEEP mode)
 */
#define APP_LPUART0_WAKEUP_CONSTRAINTS 2, PM_RESC_FRO_6M_ON, PM_RESC_WAKE_PD_PERI_OPERATIONAL

/* -------------------------------------------------------------------------- */
/*                              Public Variables                              */
/* -------------------------------------------------------------------------- */
#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0)
/* Define serial manager handle*/
SERIAL_MANAGER_HANDLE_DEFINE(gSerMgrIf);

#if (gAppUseSerialManager_c > 1)
/* Define second instance of serial manager handle*/
SERIAL_MANAGER_HANDLE_DEFINE(gSerMgrIf2);
#endif /*gAppUseSerialManager_c*/

/*Define fsci serial manager handle*/
#if defined(gFsciIncluded_c) && (gFsciIncluded_c > 0)
serial_handle_t g_fsciHandleList[gFsciIncluded_c];
#endif /*gFsciIncluded_c > 0*/
#endif /*gAppUseSerialManager_c*/

/*Define led handle*/
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c > 0))
LED_HANDLE_ARRAY_DEFINE(g_ledHandle, gAppLedCnt_c);
#endif /*gAppLedCnt_c > 0*/

/*Define led flash configuration*/
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c > 0))
LED_FLASH_CONFIG_ARRAY_DEFINE(g_ledFlashConfig, gAppLedCnt_c) = {
    {
        .times     = LED_FLASH_CYCLE_FOREVER,
        .period    = 200,
        .flashType = kLED_FlashOneColor,
        .duty      = 50,
    },
#if (gAppLedCnt_c > 1)
    {
        .times     = LED_FLASH_CYCLE_FOREVER,
        .period    = 200,
        .flashType = kLED_FlashOneColor,
        .duty      = 50,
    },
#endif
};
#endif /*gAppLedCnt_c > 0*/

/*Define button handle*/
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
BUTTON_HANDLE_ARRAY_DEFINE(g_buttonHandle, gAppButtonCnt_c);
#endif /*gAppButtonCnt_c > 0*/

#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d > 0)
#if (defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0))
static const serial_manager_lowpower_critical_CBs_t gSerMgr_LowpowerCriticalCBs = {
    .serialEnterLowpowerCriticalFunc = &PWR_LowPowerEnterCritical,
    .serialExitLowpowerCriticalFunc  = &PWR_LowPowerExitCritical,
};
#endif
#if (defined(gAppUseSensors_d) && (gAppUseSensors_d > 0))
static const Sensors_LowpowerCriticalCBs_t app_LowpowerSensorsCriticalCBs = {
    .SensorsEnterLowpowerCriticalFunc = &PWR_LowPowerEnterCritical,
    .SensorsExitLowpowerCriticalFunc  = &PWR_LowPowerExitCritical,
};
#endif

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
static pm_wakeup_source_t button0WakeUpSource;
#if (gAppButtonCnt_c > 1)
static pm_wakeup_source_t button1WakeUpSource;
#endif /* gAppButtonCnt_c > 1 */
#endif /*gAppButtonCnt_c > 0*/
#endif
/*${function:start}*/
/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */
#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d > 0)
static void APP_ServiceInitLowpower(void)
{
    PWR_ReturnStatus_t status = PWR_Success;

    /* It is required to initialize PWR module so the application
     * can call PWR API during its init (wake up sources...) */
    PWR_Init();

    /* Initialize board_lp module, likely to register the enter/exit
     * low power callback to Power Manager */
    BOARD_LowPowerInit();

    /* Set Deep Sleep constraint by default (works for All application)
     *   Application will be allowed to release the Deep Sleep constraint
     *   and set a deepest lowpower mode constraint such as Power down if it needs
     *   more optimization */
    status = PWR_SetLowPowerModeConstraint(PWR_DeepSleep);
    assert(status == PWR_Success);

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
    /* Init and enable button0 as wake up source
     * BOARD_WAKEUP_SOURCE_BUTTON0 can be customized based on board configuration
     * On EVK we use the SW2 mapped to GPIOD */
    PM_InitWakeupSource(&button0WakeUpSource, BOARD_WAKEUP_SOURCE_BUTTON0, NULL, true);

#if (gAppButtonCnt_c > 1)
    /* Init and enable button1 as wake up source
     * BOARD_WAKEUP_SOURCE_BUTTON1 can be customized based on board configuration
     * On EVK we use the SW3 mapped to PTC6 */
    PM_InitWakeupSource(&button1WakeUpSource, BOARD_WAKEUP_SOURCE_BUTTON1, NULL, true);
#endif
#endif

#if (defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0))

#if defined(gAppLpuart0WakeUpSourceEnable_d) && (gAppLpuart0WakeUpSourceEnable_d > 0)
    /* To be able to wake up from LPUART0, we need to keep the FRO6M running
     * also, we need to keep the WAKE domain is SLEEP.
     * We can't put the WAKE domain in DEEP SLEEP because the LPUART0 is not mapped
     * to the WUU as wake up source */
    (void)PM_SetConstraints(PM_LP_STATE_NO_CONSTRAINT, APP_LPUART0_WAKEUP_CONSTRAINTS);
#endif

    /* Register PWR functions into SerialManager module in order to disable device lowpower
        during SerialManager processing. Typically, allow only WFI instruction when
        uart data are processed by serail manager  */
    SerialManager_SetLowpowerCriticalCb(&gSerMgr_LowpowerCriticalCBs);
#endif

#if defined(gAppUseSensors_d) && (gAppUseSensors_d > 0)
    Sensors_SetLowpowerCriticalCb(&app_LowpowerSensorsCriticalCBs);
#endif

    (void)status;
}
#endif /* gAppLowpowerEnabled_d */

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */
/*${function:start}*/
void APP_InitServices(void)
{
#if defined(gAppForceLvdResetOnResetPinDet_d) && (gAppForceLvdResetOnResetPinDet_d > 0)
    PLATFORM_ForceLvdResetFromResetPin();
#endif
#if defined(gAppForceDeepPowerDownResetOnResetPinDet_d) && (gAppForceDeepPowerDownResetOnResetPinDet_d > 0)
    PLATFORM_CheckAndForceDeepPowerDownResetOnResetPin();
#endif
    /* Initialize Timers for Application, you can comment this out if no timer is used on Application */
    (void)PLATFORM_InitTimerManager();

#if defined(gAppUseSensors_d) && (gAppUseSensors_d > 0)
    /* for battery measurement */
    SENSORS_InitAdc();

    /* Trig the ADC on the battery voltage during initialization for BLE battery service profile */
    SENSORS_TriggerBatteryMeasurement();
#endif

#if (defined(gBoardLedRed_d) && (gBoardLedRed_d == 1))
    BOARD_InitRedLed(gBoardLedRedHdl);
#endif

#if (defined(gBoardLedYellow_d) && (gBoardLedYellow_d == 1))
    BOARD_InitYellowLed(gBoardLedYellowHdl);
#endif
#if defined(gBoardLedMonochromeHdl)
    /* Bind monochrome LED to handle 0 : arbitrary could be the other way round */
    BOARD_InitMonochromeLed(gBoardLedMonochromeHdl);
#endif /* gBoardLedMonochromeHdl */

#if defined(gBoardLedRgbHdl)
    /* Bind RGB LED to handle */
    BOARD_InitRgbLed(gBoardLedRgbHdl);
#endif /* gBoardLedRgbHdl */

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
    BOARD_InitButton0((button_handle_t)g_buttonHandle[0]);
#if (gAppButtonCnt_c > 1)
    BOARD_InitButton1((button_handle_t)g_buttonHandle[1]);
#endif /* gAppButtonCnt_c > 1*/
#endif /* gAppButtonCnt_c > 0*/

#if (defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0))
    /* init application serial manager*/
    BOARD_InitSerialManager((serial_handle_t)gSerMgrIf);

#if (gAppUseSerialManager_c > 1)
    /* init second instance of serial manager*/
    BOARD_InitSerialManager2((serial_handle_t)gSerMgrIf2);
#endif

    /* Set fsci handler */
#if defined(gFsciIncluded_c) && (gFsciIncluded_c > 0)
    g_fsciHandleList[0] = gSerMgrIf;
#endif
#endif

#if defined(gAppUseSensors_d) && (gAppUseSensors_d > 0)
    /* Measure battery voltage during initialization for BLE battery service profile */
    SENSORS_RefreshBatteryLevel();
#endif

#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d > 0)
    APP_ServiceInitLowpower();
#endif
#if defined(BOARD_LOCALIZATION_REVISION_SUPPORT) && (BOARD_LOCALIZATION_REVISION_SUPPORT > 0)
    BOARD_InitRFSwitchControlPins();
#endif
}

#if (defined(gAppLedCnt_c) && (gAppLedCnt_c > 0))
void LedStartFlashingAllLeds(void)
{
    for (uint8_t ledNo = 0U; ledNo < (uint8_t)gAppLedCnt_c; ledNo++)
    {
        LedFlashing(ledNo);
    }
}
#endif /* gAppLedCnt_c > 0*/

#if (defined(gAppLedCnt_c) && (gAppLedCnt_c > 0))
void LedStopFlashingAllLeds(void)
{
    for (uint8_t ledNo = 0U; ledNo < (uint8_t)gAppLedCnt_c; ledNo++)
    {
        (void)LED_TurnOnOff((led_handle_t)g_ledHandle[ledNo], 0U);
    }
}

#endif /* gAppLedCnt_c > 0*/

/*${function:end}*/
