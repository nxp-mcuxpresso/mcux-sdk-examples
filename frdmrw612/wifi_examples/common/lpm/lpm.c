/*
 * Copyright 2020-2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#if CONFIG_HOST_SLEEP
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "lpm.h"
#include "host_sleep.h"
#include "fsl_pm_core.h"
#include "fsl_pm_device.h"
#include "fsl_power.h"
#include "fsl_rtc.h"
#include "fsl_gpio.h"
#include "osa.h"
#include "wlan.h"
#include "cli.h"
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
#if CONFIG_POWER_MANAGER
/* Global power manager handle */
AT_ALWAYS_ON_DATA(pm_handle_t pm_handle);
AT_ALWAYS_ON_DATA(pm_wakeup_source_t wlanWakeupSource);
AT_ALWAYS_ON_DATA(pm_wakeup_source_t rtcWakeupSource);
extern pm_notify_element_t wlan_notify;
status_t powerManager_BoardNotify(pm_event_type_t eventType, uint8_t powerState, void *data);
AT_ALWAYS_ON_DATA_INIT(pm_notify_element_t board_notify) = {
    .notifyCallback = powerManager_BoardNotify,
    .data           = NULL,
};
#if CONFIG_UART_INTERRUPT
extern bool usart_suspend_flag;
#endif
#endif
extern int wakeup_by;
extern int is_hs_handshake_done;

/*******************************************************************************
 * APIs
 ******************************************************************************/
void lpm_gpio_wake_pin_init(void)
{
    /* Enables the clock for the GPIO0 module */
    GPIO_PortInit(GPIO, 0);

    gpio_pin_config_t gpio0_pinM2_config = {
        .pinDirection = kGPIO_DigitalInput,
        .outputLogic = 0U
    };
    gpio_interrupt_config_t gpio0_pinM2_int_config = {
        .mode = kGPIO_PinIntEnableEdge,
        .polarity = kGPIO_PinIntEnableLowOrFall
    };
    /* Initialize GPIO functionality on pin PIO0_11 (pin M2)  */
    GPIO_PinInit(GPIO, BOARD_SW2_GPIO_PORT, BOARD_SW2_GPIO_PIN, &gpio0_pinM2_config);
    GPIO_SetPinInterruptConfig(GPIO, BOARD_SW2_GPIO_PORT, BOARD_SW2_GPIO_PIN, &gpio0_pinM2_int_config);
    GPIO_PinEnableInterrupt(GPIO, BOARD_SW2_GPIO_PORT, BOARD_SW2_GPIO_PIN, (uint32_t)kGPIO_InterruptA);
}

void lpm_pm3_exit_hw_reinit()
{
    BOARD_InitBootPins();
    lpm_gpio_wake_pin_init();
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
#if CONFIG_UART_INTERRUPT
    cli_uart_reinit();
#endif
    RTC_Init(RTC);
}

#if CONFIG_POWER_MANAGER
status_t powerManager_BoardNotify(pm_event_type_t eventType, uint8_t powerState, void *data)
{
    if (is_hs_handshake_done != WLAN_HOSTSLEEP_SUCCESS)
        return kStatus_PMPowerStateNotAllowed;
    if (eventType == kPM_EventEnteringSleep)
    {
        if (powerState == PM_LP_STATE_PM3)
        {
#if CONFIG_UART_INTERRUPT
            cli_uart_deinit();
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
#if CONFIG_UART_INTERRUPT
            usart_suspend_flag = false;
#endif
        }
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
    memset(&wlanWakeupSource, 0x0, sizeof(pm_wakeup_source_t));
    memset(&rtcWakeupSource, 0x0, sizeof(pm_wakeup_source_t));
    /* Init wakeup sources. Corresponding IRQ numbers act as wsId here. */
    PM_InitWakeupSource(&wlanWakeupSource, WL_MCI_WAKEUP0_IRQn, NULL, true);
    PM_InitWakeupSource(&rtcWakeupSource, RTC_IRQn, NULL, false);
}

void powerManager_Init()
{
    PM_CreateHandle(&pm_handle);
    /* Init and start RTC time counter */
    powerManager_RTC_Init();
    /* Set priority of RTC and PIN1 interrupt */
    NVIC_SetPriority(RTC_IRQn, LPM_RTC_PIN1_PRIORITY);
    NVIC_SetPriority(PIN1_INT_IRQn, LPM_RTC_PIN1_PRIORITY);
    /* Register WLAN notifier */
    PM_RegisterNotify(kPM_NotifyGroup0, &wlan_notify);
    /* Register board notifier */
    PM_RegisterNotify(kPM_NotifyGroup2, &board_notify);
    /* Init WLAN wakeup source */
    powerManager_Wakeupsource_Init();
    PM_EnablePowerManager(true);
    OSA_SetupIdleFunction(powerManager_EnterLowPower);
    wakeup_by = 0;
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

#if CONFIG_POWER_MANAGER
    powerManager_Init();
#endif

    lpm_gpio_wake_pin_init();
    NVIC_SetPriority(GPIO_INTA_IRQn, LPM_RTC_PIN1_PRIORITY);
    return kStatus_PMSuccess;
}
#endif /* CONFIG_HOST_SLEEP */
