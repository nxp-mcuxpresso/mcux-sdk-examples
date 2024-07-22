/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#if CONFIG_HOST_SLEEP
#include "pin_mux.h"
#include "board.h"
#include "lpm.h"
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
#if CONFIG_NCP_UART
extern bool usart_suspend_flag;
#endif
#endif
#if CONFIG_CRC32_HW_ACCELERATE
extern void ncp_tlv_chksum_init();
#endif

/*******************************************************************************
 * APIs
 ******************************************************************************/
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
#endif
#if CONFIG_CRC32_HW_ACCELERATE
    ncp_tlv_chksum_init();
#endif
#if CONFIG_NCP_UART
    usart_suspend_flag = false;
#endif
     ncp_intf_pm_exit((int32_t)PM_LP_STATE_PM3);
#if CONFIG_NCP_WIFI
#if CONFIG_NCP_DEBUG
#if CONFIG_UART_INTERRUPT
    cli_uart_reinit();
#endif
#endif
#endif
    RTC_Init(RTC);
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
    if (eventType == kPM_EventEnteringSleep)
    {
        host_sleep_pre_cfg(powerState);
    }
    else if (eventType == kPM_EventExitingSleep)
    {
        host_sleep_post_cfg(powerState);
    }
    return kStatus_PMSuccess;
}

status_t powerManager_BoardNotify(pm_event_type_t eventType, uint8_t powerState, void *data)
{
#if CONFIG_NCP_WIFI
    if (is_hs_handshake_done != WLAN_HOSTSLEEP_SUCCESS)
        return kStatus_PMPowerStateNotAllowed;
#endif
    if (eventType == kPM_EventEnteringSleep)
    {
#if CONFIG_NCP_UART
        if (powerState == PM_LP_STATE_PM3)
        {
             usart_suspend_flag = true;
        }
#endif
        while(ncp_intf_pm_enter((int32_t)powerState) == NCP_PM_STATUS_NOT_READY)
            ;
        if (powerState == PM_LP_STATE_PM3)
        {
#if CONFIG_NCP_WIFI
#if CONFIG_NCP_DEBUG
#if CONFIG_UART_INTERRUPT
            cli_uart_notify();
            cli_uart_deinit();
#endif
#endif
#endif
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
#if CONFIG_NCP_UART
            usart_suspend_flag = false;
#endif
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
    /* Enable wakeup in PD mode */
    RTC_EnableAlarmTimerInterruptFromDPD(RTC, true);
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
}

void powerManager_Init()
{
    PM_CreateHandle(&pm_handle);
    /* Init and start RTC time counter */
    powerManager_RTC_Init();
#if CONFIG_NCP_WIFI
    /* Register WLAN notifier */
    PM_RegisterNotify(kPM_NotifyGroup0, &wlan_notify);
#endif
    PM_RegisterNotify(kPM_NotifyGroup1, &ncp_pm_notify);
    /* Register board notifier */
    PM_RegisterNotify(kPM_NotifyGroup2, &board_notify);
    powerManager_Wakeupsource_Init();
    PM_EnablePowerManager(true);
    OSA_SetupIdleFunction(powerManager_EnterLowPower);
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
    NVIC_SetPriority(PIN1_INT_IRQn, LPM_RTC_PIN1_PRIORITY);
#if CONFIG_POWER_MANAGER
    powerManager_Init();
#endif

    return kStatus_PMSuccess;
}
#endif /* CONFIG_HOST_SLEEP */
