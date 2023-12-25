/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_common.h"
#include "fsl_pm_core.h"
#include "fsl_pm_device.h"

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_rtc.h"
#include "fsl_power.h"
#include "fsl_usart.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_POWER_NAME                                                                   \
    {                                                                                    \
        "PM0 - Active", "PM1 - Idle", "PM2 - Standby", "PM3 - Sleep", "PM4 - Deep Sleep" \
    }
#define APP_TARGET_POWER_NUM (PM_LP_STATE_COUNT)

#define APP_PM2_CONSTRAINTS                                                                           \
    6U, PM_RESC_SRAM_0K_384K_STANDBY, PM_RESC_SRAM_384K_448K_STANDBY, PM_RESC_SRAM_448K_512K_STANDBY, \
        PM_RESC_SRAM_512K_640K_STANDBY, PM_RESC_SRAM_640K_896K_STANDBY, PM_RESC_SRAM_896K_1216K_STANDBY

#define APP_PM3_CONSTRAINTS                                                                                 \
    6U, PM_RESC_SRAM_0K_384K_RETENTION, PM_RESC_SRAM_384K_448K_RETENTION, PM_RESC_SRAM_448K_512K_RETENTION, \
        PM_RESC_SRAM_512K_640K_RETENTION, PM_RESC_SRAM_640K_896K_RETENTION, PM_RESC_SRAM_896K_1216K_RETENTION

#define APP_PM4_CONSTRAINTS 0U


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
status_t APP_UartControlCallback(pm_event_type_t eventType, uint8_t powerState, void *data);
void APP_InitWakeupSource(void);
void APP_StartRtcTimer(uint64_t timeOutTickes);
void APP_StopRtcTimer(void);
uint32_t APP_GetWakeupTimeout(void);
void APP_RegisterNotify(void);
void APP_SetConstraints(uint8_t powerMode);
void APP_ReleaseConstraints(uint8_t powerMode);

static uint8_t APP_GetTargetPowerMode(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
AT_ALWAYS_ON_DATA(pm_handle_t g_pmHandle);
AT_ALWAYS_ON_DATA(uint8_t g_targetPowerMode);
AT_ALWAYS_ON_DATA(uint32_t g_irqMask);
static const char *const g_targetPowerNameArray[APP_TARGET_POWER_NUM] = APP_POWER_NAME;

/*******************************************************************************
 * Code
 ******************************************************************************/

AT_ALWAYS_ON_DATA_INIT(pm_notify_element_t g_notify1) = {
    .notifyCallback = APP_UartControlCallback,
    .data           = NULL,
};

AT_ALWAYS_ON_DATA(pm_wakeup_source_t g_OstimerWakeupSource);

AT_ALWAYS_ON_DATA_INIT(power_init_config_t g_initCfg) = {
    .iBuck         = true, /* VCORE AVDD18 supplied from iBuck on RD board. */
    .gateCauRefClk = true, /* CAU_SOC_SLP_REF_CLK not needed. */
};

extern void RTC_IRQHandler(void);
void RTC_IRQHandler()
{
    if (RTC_GetStatusFlags(RTC) & kRTC_AlarmFlag)
    {
        /* Clear alarm flag */
        RTC_ClearStatusFlags(RTC, kRTC_AlarmFlag);
        POWER_ClearWakeupStatus(RTC_IRQn);
        PRINTF("Woken up by RTC\r\n");
    }
}


status_t APP_UartControlCallback(pm_event_type_t eventType, uint8_t powerState, void *data)
{
    if (powerState >= PM_LP_STATE_PM2 && powerState <= PM_LP_STATE_PM3)
    {
        if (eventType == kPM_EventEnteringSleep)
        {
            /* De-init uart */
            PRINTF("De-init UART.\r\n");
            /* Wait for debug console output finished. */
            while (((uint32_t)kUSART_TxFifoEmptyFlag & USART_GetStatusFlags(BOARD_DEBUG_UART)) == 0U)
            {
            }
            DbgConsole_Deinit();
        }
        else
        {
            if (powerState == PM_LP_STATE_PM3)
            {
                /* Perihperal state lost, need reinitialize in exit from PM3 */
                BOARD_InitBootPins();
                BOARD_InitBootClocks();
                POWER_InitPowerConfig(&g_initCfg);
            }
            /* Re-init uart. */
            BOARD_InitDebugConsole();
            PRINTF("Re-init UART.\r\n");
        }
    }

    return kStatus_Success;
}

void APP_InitWakeupSource(void)
{
    PM_InitWakeupSource(&g_OstimerWakeupSource, (uint32_t)RTC_IRQn, NULL, true);
    PM_RegisterTimerController(&g_pmHandle, APP_StartRtcTimer, APP_StopRtcTimer, NULL, NULL);
}

void APP_StartRtcTimer(uint64_t timeOutUs)
{
    uint32_t currSeconds;

    /* Read the RTC seconds register to get current time in seconds */
    currSeconds = RTC_GetSecondsTimerCount(RTC);
    /* Add alarm seconds to current time */
    currSeconds += (uint32_t)((timeOutUs + 999999U) / 1000000U);
    /* Set alarm time in seconds */
    RTC_SetSecondsTimerMatch(RTC, currSeconds);
}

void APP_StopRtcTimer(void)
{
    /* Do nothing */
}

uint32_t APP_GetWakeupTimeout(void)
{
    uint8_t timeout;

    while (1)
    {
        PRINTF("Select the wake up timeout in seconds.\r\n");
        PRINTF("The allowed range is 1s ~ 9s.\r\n");
        PRINTF("Eg. enter 5 to wake up in 5 seconds.\r\n");
        PRINTF("\r\nWaiting for input timeout value...\r\n\r\n");

        timeout = GETCHAR();
        PRINTF("%c\r\n", timeout);
        if ((timeout > '0') && (timeout <= '9'))
        {
            timeout -= '0';
            PRINTF("Will wakeup in %d seconds.\r\n", timeout);
            return (uint32_t)timeout * 1000000UL;
        }
        PRINTF("Wrong value!\r\n");
    }
}

void APP_RegisterNotify(void)
{
    PM_RegisterNotify(kPM_NotifyGroup0, &g_notify1);
}

void APP_SetConstraints(uint8_t powerMode)
{
    switch (powerMode)
    {
        case PM_LP_STATE_PM2:
        {
            PM_SetConstraints(powerMode, APP_PM2_CONSTRAINTS);
            break;
        }

        case PM_LP_STATE_PM3:
        {
            PM_SetConstraints(powerMode, APP_PM3_CONSTRAINTS);
            break;
        }

        case PM_LP_STATE_PM4:
        {
            PM_SetConstraints(powerMode, APP_PM4_CONSTRAINTS);
            break;
        }

        default:
        {
            /* PM0/PM1 has no reousrce constraints. */
            PM_SetConstraints(powerMode, 0U);
            break;
        }
    }
}

void APP_ReleaseConstraints(uint8_t powerMode)
{
    switch (powerMode)
    {
        case PM_LP_STATE_PM2:
        {
            PM_ReleaseConstraints(powerMode, APP_PM2_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_PM3:
        {
            PM_ReleaseConstraints(powerMode, APP_PM3_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_PM4:
        {
            PM_ReleaseConstraints(powerMode, APP_PM4_CONSTRAINTS);
            break;
        }
        default:
        {
            /* PM0/PM1 has no reousrce constraints. */
            PM_ReleaseConstraints(powerMode, 0U);
            break;
        }
    }
}


int main(void)
{
    uint32_t timeoutUs = 0UL;

    uint32_t resetSrc;

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitSleepPinConfig();
    POWER_InitPowerConfig(&g_initCfg);

    resetSrc = POWER_GetResetCause();
    PRINTF("\r\nMCU wakeup source 0x%x...\r\n", resetSrc);
    /* In case PM4 wakeup, the wakeup config and status need to be cleared */
    POWER_ClearResetCause(resetSrc);
    DisableIRQ(RTC_IRQn);
    POWER_ClearWakeupStatus(RTC_IRQn);
    POWER_DisableWakeup(RTC_IRQn);

    RTC_Init(RTC);
    /* Enable wakeup in PD mode */
    RTC_EnableAlarmTimerInterruptFromDPD(RTC, true);
    /* Start RTC */
    RTC_ClearStatusFlags(RTC, kRTC_AlarmFlag);
    RTC_StartTimer(RTC);
    PRINTF("\r\nPower Manager Test.\r\n");
    PRINTF("\r\nNormal Boot.\r\n");
    PM_CreateHandle(&g_pmHandle);

    APP_RegisterNotify();
    APP_InitWakeupSource();
    while (1)
    {
        g_targetPowerMode = APP_GetTargetPowerMode();
        if (g_targetPowerMode >= APP_TARGET_POWER_NUM)
        {
            PRINTF("\r\nWrong Input! Please reselect.\r\n");
            continue;
        }
        PRINTF("Selected to enter %s.\r\n", g_targetPowerNameArray[(uint8_t)g_targetPowerMode]);
        timeoutUs = APP_GetWakeupTimeout();
        APP_SetConstraints(g_targetPowerMode);
        g_irqMask = DisableGlobalIRQ();
        PM_EnablePowerManager(true);
        PM_EnterLowPower(timeoutUs);
        PM_EnablePowerManager(false);
        EnableGlobalIRQ(g_irqMask);
        APP_ReleaseConstraints(g_targetPowerMode);
        PRINTF("\r\nNext Loop\r\n");
    }
}

static uint8_t APP_GetTargetPowerMode(void)
{
    uint32_t i;
    uint8_t ch;
    uint8_t g_targetPowerModeIndex;

    PRINTF("\r\nPlease select the desired power mode:\r\n");
    for (i = 0UL; i < APP_TARGET_POWER_NUM; i++)
    {
        PRINTF("\tPress %c to enter: %s\r\n", ('A' + i), g_targetPowerNameArray[i]);
    }

    PRINTF("\r\nWaiting for power mode select...\r\n\r\n");

    ch = GETCHAR();

    if ((ch >= 'a') && (ch <= 'z'))
    {
        ch -= 'a' - 'A';
    }

    g_targetPowerModeIndex = ch - 'A';

    return g_targetPowerModeIndex;
}
