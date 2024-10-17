/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */
#include "pin_mux.h"
#include "board.h"
#include "ncp_lpm.h"
#include "host_sleep.h"
#include "fsl_pm_core.h"
#include "fsl_pm_device.h"
#include "fsl_power.h"
#include "fsl_rtc.h"
#include "osa.h"
#include "wlan.h"
#include "cli.h"
#include "board.h"
#include "ncp_intf_pm.h"
#include "clock_config.h"
#include "mflash_common.h"
#if CONFIG_WPA_SUPP
#include "els_pkc_mbedtls.h"
#endif

#include "fwk_platform.h"
#include "fwk_platform_lowpower.h"
#include "PWR_Interface.h"

#include "FreeRTOS.h"
#include "timers.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
power_init_config_t initCfg = {
    /* VCORE AVDD18 supplied from iBuck on RD board. */
    .iBuck = true,
    /* Keep CAU_SOC_SLP_REF_CLK for LPOSC. */
    .gateCauRefClk = false,
};
extern int wakeup_reason;
#if CONFIG_POWER_MANAGER
/* Global power manager handle */
AT_ALWAYS_ON_DATA(pm_handle_t pm_handle);
AT_ALWAYS_ON_DATA(pm_wakeup_source_t rtcWakeupSource);
#if CONFIG_NCP_WIFI
AT_ALWAYS_ON_DATA(pm_wakeup_source_t wlanWakeupSource);
extern pm_notify_element_t wlan_notify;
#endif
#if CONFIG_NCP_BLE
AT_ALWAYS_ON_DATA(pm_wakeup_source_t bleWakeUpSource);
#endif
status_t powerManager_PmNotify(pm_event_type_t eventType, uint8_t powerState, void *data);
AT_ALWAYS_ON_DATA_INIT(pm_notify_element_t ncp_pm_notify) = {
    .notifyCallback = powerManager_PmNotify,
    .data           = NULL,
};
status_t powerManager_BoardNotify(pm_event_type_t eventType, uint8_t powerState, void *data);
AT_ALWAYS_ON_DATA_INIT(pm_notify_element_t board_notify) = {
    .notifyCallback = powerManager_BoardNotify,
    .data           = NULL,
};
#if CONFIG_NCP_WIFI
extern int is_hs_handshake_done;
#endif
#endif
#if CONFIG_CRC32_HW_ACCELERATE
extern void ncp_tlv_chksum_init();
#endif

#if CONFIG_NCP_SPI
int ncp_spi_txrx_is_finish(void);
#endif
/* Default NCP host <-> NCP device low power handshake state */
static uint8_t ncpLowPowerHandshake = NCP_LMP_HANDSHAKE_NOT_START;

#if defined(configUSE_TICKLESS_IDLE) && (configUSE_TICKLESS_IDLE == 1)
/* Tickless idle is allowed by default but can be disabled runtime with APP_SetTicklessIdle */
static int ticklessIdleAllowed = 1;
#endif /* configUSE_TICKLESS_IDLE && configUSE_TICKLESS_IDLE==1 */

#define LOWPOWER_DEFAULT_ENABLE_DURATION 5
static TimerHandle_t      lpTimer = NULL;
static PWR_LowpowerMode_t currentMode;

/* Register a Platform RTC callback for handle RTC IRQ information in ncp device */
static void LPM_RtcCallback(struct _platform_rtc_handle *handle);
static platform_rtc_handle_t lpmRtcHandler = {
    .index = 0,
    .callback = LPM_RtcCallback,
    .userData = NULL,
};
/*******************************************************************************
 * APIs
 ******************************************************************************/

uint8_t lpm_getHandshakeState(void)
{
    return ncpLowPowerHandshake;
}
void lpm_setHandshakeState(uint8_t state)
{
    ncpLowPowerHandshake = state;
}

void lpm_pm3_exit_hw_reinit()
{
    BOARD_InitBootPins();
    if (BOARD_IS_XIP())
    {
        BOARD_BootClockLPR();
        CLOCK_EnableClock(kCLOCK_Otp);
        CLOCK_EnableClock(kCLOCK_Els);
        CLOCK_EnableClock(kCLOCK_ElsApb);
        RESET_PeripheralReset(kOTP_RST_SHIFT_RSTn);
        RESET_PeripheralReset(kELS_APB_RST_SHIFT_RSTn);
    }
    else
    {
        BOARD_InitBootClocks();
    }
    BOARD_InitDebugConsole();
    POWER_InitPowerConfig(&initCfg);
#if CONFIG_WPA_SUPP
    CRYPTO_ReInitHardware();
#endif /*CONFIG_WPA_SUPP */
#if CONFIG_CRC32_HW_ACCELERATE
    ncp_tlv_chksum_init();
#endif /* CONFIG_CRC32_HW_ACCELERATE */
     ncp_intf_pm_exit((int32_t)PM_LP_STATE_PM3);
#if CONFIG_NCP_WIFI
#if CONFIG_NCP_DEBUG
#if CONFIG_UART_INTERRUPT
    cli_uart_reinit();
#endif /* CONFIG_NCP_WIFI */
#endif /* CONFIG_NCP_DEBUG */
#endif /* CONFIG_UART_INTERRUPT */
    ncp_gpio_init();
    mflash_drv_init();
}

#if CONFIG_POWER_MANAGER
status_t powerManager_PmNotify(pm_event_type_t eventType, uint8_t powerState, void *data)
{
#if CONFIG_NCP_WIFI
    if (is_hs_handshake_done != WLAN_HOSTSLEEP_SUCCESS)
        return kStatus_PMPowerStateNotAllowed;
#endif
    /* is ncp host <-> ncp device handshake done? */
    if(lpm_getHandshakeState() == NCP_LMP_HANDSHAKE_IN_PROCESS)
    {
        return kStatus_PMPowerStateNotAllowed;
    }

#if CONFIG_NCP_SPI
    if(ncp_spi_txrx_is_finish() == 0)
    {
        return kStatus_PMPowerStateNotAllowed;
    }
#endif /* CONFIG_NCP_SPI */
    if (eventType == kPM_EventEnteringSleep)
    {
        if (powerState >= 2)
        {
            host_sleep_pre_cfg(powerState);
        }
    }
    else if (eventType == kPM_EventExitingSleep)
    {
        if (powerState >= 2)
        {
            host_sleep_post_cfg(powerState);
        }
    }
    return kStatus_PMSuccess;
}

status_t powerManager_BoardNotify(pm_event_type_t eventType, uint8_t powerState, void *data)
{
#if CONFIG_NCP_WIFI
    if (is_hs_handshake_done != WLAN_HOSTSLEEP_SUCCESS)
        return kStatus_PMPowerStateNotAllowed;
#endif /* CONFIG_NCP_WIFI */

    /* is ncp host <-> ncp device handshake done? */
    if(lpm_getHandshakeState() == NCP_LMP_HANDSHAKE_IN_PROCESS)
    {
        return kStatus_PMPowerStateNotAllowed;
    }
#if CONFIG_NCP_SPI
    if(ncp_spi_txrx_is_finish() == 0)
    {
        return kStatus_PMPowerStateNotAllowed;
    }
#endif /* CONFIG_NCP_SPI */
    if (eventType == kPM_EventEnteringSleep)
    {
        while(ncp_intf_pm_enter((int32_t)powerState) == NCP_PM_STATUS_NOT_READY)
            ;
        if (powerState == PM_LP_STATE_PM3)
        {
#if CONFIG_NCP_WIFI
#if CONFIG_NCP_DEBUG
#if CONFIG_UART_INTERRUPT
            cli_uart_notify();
            cli_uart_deinit();
#endif /* CONFIG_UART_INTERRUPT */
#endif /* CONFIG_NCP_DEBUG */
#endif /* CONFIG_NCP_WIFI */
            DbgConsole_Deinit();
        }
        else
        {
            /* Do Nothing */
        }
    }
    else if (eventType == kPM_EventExitingSleep)
    {
        if (powerState == PM_LP_STATE_PM3)
        {
            lpm_pm3_exit_hw_reinit();
        }
        else if (powerState == PM_LP_STATE_PM2)
            ncp_intf_pm_exit((int32_t)PM_LP_STATE_PM2);
        else
        {
            /* Do Nothing */
        }
    }
    return kStatus_PMSuccess;
}

#if defined(configUSE_TICKLESS_IDLE) && (configUSE_TICKLESS_IDLE == 1)
void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{

    bool abortIdle = false;
    uint64_t expectedIdleTimeUs, actualIdleTimeUs;

    if(ticklessIdleAllowed > 0)
    {
        uint32_t irqMask = DisableGlobalIRQ();

        /* Disable and prepare systicks for low power */
        abortIdle = PWR_SysticksPreProcess((uint32_t)xExpectedIdleTime, &expectedIdleTimeUs);

        if (abortIdle == false)
        {
            /* Enter low power with a maximal timeout */
            actualIdleTimeUs = PWR_EnterLowPower(expectedIdleTimeUs);

            /* Re enable systicks and compensate systick timebase */
            PWR_SysticksPostProcess(expectedIdleTimeUs, actualIdleTimeUs);
        }

        /* Exit from critical section */
        EnableGlobalIRQ(irqMask);
    }
    else
    {
        /* Tickless idle is not allowed, wait for next tick interrupt */
        __WFI();
    }
}

void APP_SetTicklessIdle(bool enable)
{
    if(enable == true)
    {
        ticklessIdleAllowed++;
    }
    else
    {
        ticklessIdleAllowed--;
    }
}
#endif /* configUSE_TICKLESS_IDLE */

static void LPM_RtcCallback(struct _platform_rtc_handle *handle)
{
    if (RTC_GetStatusFlags(RTC) & kRTC_AlarmFlag)
    {
        RTC_ClearStatusFlags(RTC, kRTC_AlarmFlag);
        DisableIRQ(RTC_IRQn);
        POWER_ClearWakeupStatus(RTC_IRQn);
        POWER_DisableWakeup(RTC_IRQn);
        wakeup_reason = WAKEUP_BY_RTC;
    }
}

void powerManager_StartRtcTimer(uint64_t timeOutUs)
{
    uint32_t currSeconds;

    /* Read the RTC seconds register to get current time in seconds */
    currSeconds = RTC_GetSecondsTimerCount(RTC);
    /* Add alarm seconds to current time */
    currSeconds += (timeOutUs + 999999U) / 1000000U;
    /* Set alarm time in seconds */
    RTC_SetSecondsTimerMatch(RTC, currSeconds);
    PM_EnableWakeupSource(&rtcWakeupSource);
    RTC_EnableTimer(RTC, true);
}

void powerManager_StopRtcTimer()
{
    /* Disable RTC timer and clear value of MATCH and COUNT */
    RTC_EnableTimer(RTC, false);
    RTC_SetSecondsTimerMatch(RTC, 0);
    RTC_SetSecondsTimerCount(RTC, 0);
    PM_DisableWakeupSource(&rtcWakeupSource);
    RTC_ClearStatusFlags(RTC, kRTC_AlarmFlag);
}

void powerManager_RTC_Init()
{
    DisableIRQ(RTC_IRQn);
    POWER_ClearWakeupStatus(RTC_IRQn);
    POWER_DisableWakeup(RTC_IRQn);

    RTC_Init(RTC);
    RTC_Reset(RTC);
    RTC_StartTimer(RTC);

    /* Enable wakeup in PD mode */
    RTC_EnableAlarmTimerInterruptFromDPD(RTC, true);

    PLATFORM_RegisterRtcHandle(&lpmRtcHandler);
    /* Start RTC */
    RTC_ClearStatusFlags(RTC, kRTC_AlarmFlag);
    /* Register RTC timer callbacks in power manager */
    PM_RegisterTimerController(&pm_handle, powerManager_StartRtcTimer, powerManager_StopRtcTimer, NULL, NULL);
}

void powerManager_Wakeupsource_Init()
{
    memset(&rtcWakeupSource, 0x0, sizeof(pm_wakeup_source_t));
    PM_InitWakeupSource(&rtcWakeupSource, RTC_IRQn, NULL, false);
#if CONFIG_NCP_WIFI
	memset(&wlanWakeupSource, 0x0, sizeof(pm_wakeup_source_t));
	PM_InitWakeupSource(&wlanWakeupSource, WL_MCI_WAKEUP0_IRQn, NULL, true);
#endif
#if CONFIG_NCP_BLE
    memset(&bleWakeUpSource, 0x0, sizeof(pm_wakeup_source_t));
    PM_InitWakeupSource(&bleWakeUpSource, BLE_MCI_WAKEUP0_IRQn, NULL, true);
#endif
}

void LPM_ConfigureNextLowPowerMode(uint8_t nextMode, uint32_t timeS)
{
    (void)PWR_ReleaseLowPowerModeConstraint(currentMode); /* MISRA CID 26556646 */
    (void)PWR_SetLowPowerModeConstraint((PWR_LowpowerMode_t)nextMode);

#if CONFIG_NCP_USB
    if (nextMode == 2)
    {
        /* Set specific constraints for USB PM2 */
        (void)PM_SetConstraints(PM_LP_STATE_PM2, 1, PM_RESC_USB_ANA_ACTIVE);
    }
    if (currentMode == 2)
    {
        /* Release specific constraints for USB PM2 */
        (void)PM_ReleaseConstraints(PM_LP_STATE_PM2, 1, PM_RESC_USB_ANA_ACTIVE);
    }
#endif

    currentMode = (PWR_LowpowerMode_t)nextMode;

    if (timeS > 0U)
    {
        (void)xTimerChangePeriod(lpTimer, timeS * 1000 / portTICK_PERIOD_MS, 0);

        /* Start the timer, during this time, the configured low power mode will be used as much as possible by
         * the system, when the timer expires, the low power mode will be limited to WFI until next command
         * This is to make sure the serial interface becomes available again to the user */
        if (xTimerStart(lpTimer, 0) != pdPASS)
        {
            assert(0);
        }
    }
}

static void LPM_TimerCallback(TimerHandle_t timer_h)
{
    (void)timer_h;

    lpm_setHandshakeState(NCP_LMP_HANDSHAKE_NOT_START);
    LPM_ConfigureNextLowPowerMode(PWR_WFI, 0U);
}

void powerManager_Init()
{
#if defined(configUSE_TICKLESS_IDLE) && (configUSE_TICKLESS_IDLE == 1)
    PWR_Init(); // need call this to enable
    lpTimer = xTimerCreate("LP timer", (TickType_t)(LOWPOWER_DEFAULT_ENABLE_DURATION * 1000 / portTICK_PERIOD_MS),
                        pdFALSE, NULL, &LPM_TimerCallback);
#endif /* configUSE_TICKLESS_IDLE */

#if defined(configUSE_IDLE_HOOK) && (configUSE_IDLE_HOOK == 1)
    PM_CreateHandle(&pm_handle);
    /* Init and start RTC time counter */
    powerManager_RTC_Init();
    powerManager_Wakeupsource_Init();
    PM_EnablePowerManager(true);

    OSA_SetupIdleFunction(powerManager_EnterLowPower);
#endif /* configUSE_IDLE_HOOK */

#if CONFIG_NCP_WIFI
    /* Register WLAN notifier */
    PM_RegisterNotify(kPM_NotifyGroup0, &wlan_notify);
#endif
    PM_RegisterNotify(kPM_NotifyGroup1, &ncp_pm_notify);
    /* Register board notifier */
    PM_RegisterNotify(kPM_NotifyGroup2, &board_notify);

    wakeup_reason = 0;
}
#endif

int LPM_Init(void)
{
    uint32_t resetSrc;

    POWER_InitPowerConfig(&initCfg);
    resetSrc = POWER_GetResetCause();
    PRINTF("\r\nMCU wakeup source 0x%x...\r\n", resetSrc);
    /* In case PM3/PM4 wakeup, the wakeup config and status need to be cleared */
    POWER_ClearResetCause(resetSrc);

    NVIC_SetPriority(RTC_IRQn, LPM_RTC_PIN1_PRIORITY);
    if(strcmp(BOARD_NAME, "FRDM-RW612") == 0)
    {
        NVIC_SetPriority(GPIO_INTA_IRQn, LPM_RTC_PIN1_PRIORITY);
    }
    else
    {
        NVIC_SetPriority(PIN1_INT_IRQn, LPM_RTC_PIN1_PRIORITY);
    }
#if CONFIG_POWER_MANAGER
    powerManager_Init();
#endif

#if defined(configUSE_TICKLESS_IDLE) && (configUSE_TICKLESS_IDLE == 1)
    /* Set WFI constraint by default (works for All application)
     * Application will be allowed to release the WFI constraint and set a deepest lowpower mode constraint such as
     * DeepSleep or PowerDown if it needs more optimization */
    (void)PWR_SetLowPowerModeConstraint(PWR_WFI);
#endif /* configUSE_TICKLESS_IDLE */

    return kStatus_PMSuccess;
}
