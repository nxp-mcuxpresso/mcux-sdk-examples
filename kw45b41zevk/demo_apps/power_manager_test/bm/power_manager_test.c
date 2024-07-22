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

#include "fsl_lptmr.h"
#include "fsl_lpuart.h"
#include "fsl_port.h"
#include "fsl_cmc.h"
#include "fsl_spc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define TEMP_CONSTRIANTS                                                                                              \
    8, PM_RESC_BUS_SYS_CLK_ON, PM_RESC_CTCM0_ACTIVE, PM_RESC_CTCM1_ACTIVE, PM_RESC_STCM0_ACTIVE, PM_RESC_FRO_192M_ON, \
        PM_RESC_FRO_6M_ON, PM_RESC_MAIN_PD_PERI_OPERATIONAL, PM_RESC_WAKE_PD_PERI_ACTIVE

#define APP_POWER_NAME                                         \
    {                                                          \
        "Sleep", "Deep Sleep", "Power Down", "Deep Power Down" \
    }
#define APP_TARGET_POWER_NUM (4U)

#define APP_SLEEP_CONSTRAINTS                                                                                        \
    9U, PM_RESC_CTCM0_DEEPSLEEP, PM_RESC_CTCM1_DEEPSLEEP, PM_RESC_STCM0_DEEPSLEEP, PM_RESC_STCM1_DEEPSLEEP,          \
        PM_RESC_STCM2_DEEPSLEEP, PM_RESC_STCM3_DEEPSLEEP, PM_RESC_STCM4_DEEPSLEEP, PM_RESC_MAIN_PD_PERI_OPERATIONAL, \
        PM_RESC_WAKE_PD_PERI_ACTIVE

#define APP_DEEP_SLEEP_CONSTRAINTS                                                                          \
    9U, PM_RESC_CTCM0_DEEPSLEEP, PM_RESC_CTCM1_DEEPSLEEP, PM_RESC_STCM0_DEEPSLEEP, PM_RESC_STCM1_DEEPSLEEP, \
        PM_RESC_STCM2_DEEPSLEEP, PM_RESC_STCM3_DEEPSLEEP, PM_RESC_STCM4_DEEPSLEEP,                          \
        PM_RESC_MAIN_PD_PERI_STATE_RETENTION, PM_RESC_WAKE_PD_PERI_OPERATIONAL

#define APP_POWER_DOWN_CONSTRAINTS                                                                          \
    8U, PM_RESC_CTCM0_DEEPSLEEP, PM_RESC_CTCM1_DEEPSLEEP, PM_RESC_STCM0_DEEPSLEEP, PM_RESC_STCM1_DEEPSLEEP, \
        PM_RESC_STCM2_DEEPSLEEP, PM_RESC_STCM3_DEEPSLEEP, PM_RESC_STCM4_DEEPSLEEP,                          \
        PM_RESC_WAKE_PD_PERI_STATE_RETENTION

#define APP_DEEP_POWER_DOWN_CONSTRAINTS 0U

#define DEBUG_CONSOLE_RX_PORT   PORTC
#define DEBUG_CONSOLE_RX_GPIO   GPIOC
#define DEBUG_CONSOLE_RX_PIN    2U
#define DEBUG_CONSOLE_RX_PINMUX kPORT_MuxAlt3
/* Debug console TX pin: PORTC3 MUX: 3 */
#define DEBUG_CONSOLE_TX_PORT   PORTC
#define DEBUG_CONSOLE_TX_GPIO   GPIOC
#define DEBUG_CONSOLE_TX_PIN    3U
#define DEBUG_CONSOLE_TX_PINMUX kPORT_MuxAlt3

#define APP_LPTMR LPTMR0


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
status_t APP_EccReInitCallback(pm_event_type_t eventType, uint8_t powerState, void *data);
status_t APP_UartControlCallback(pm_event_type_t eventType, uint8_t powerState, void *data);
void APP_InitWakeupSource(void);
void APP_Lptmr0WakeupService(void);
void APP_StartLptmr(uint64_t timeOutTickes);
void APP_StopLptmr(void);
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

AT_ALWAYS_ON_DATA_INIT(pm_notify_element_t g_notify2) = {
    .notifyCallback = APP_EccReInitCallback,
    .data           = NULL,
};

AT_ALWAYS_ON_DATA(pm_wakeup_source_t g_lptmr0WakeupSource);

void LPTMR0_IRQHandler(void)
{
    PM_TriggerWakeSourceService(&g_lptmr0WakeupSource);
}


status_t APP_EccReInitCallback(pm_event_type_t eventType, uint8_t powerState, void *data)
{
    if ((eventType == kPM_EventExitingSleep) && (powerState == PM_LP_STATE_DEEP_POWER_DOWN))
    {
        uint32_t ramAddress = 0x4000000UL;
        while (ramAddress < 0x4004000UL)
        {
            memset((void *)(uint32_t *)ramAddress, 0UL, sizeof(uint32_t));
            ramAddress = ramAddress + 4UL;
        }
        ramAddress = 0x20000000;
        while (ramAddress < 0x20010000)
        {
            memset((void *)(uint32_t *)ramAddress, 0UL, sizeof(uint32_t));
            ramAddress = ramAddress + 4UL;
        }
    }
    return kStatus_Success;
}

status_t APP_UartControlCallback(pm_event_type_t eventType, uint8_t powerState, void *data)
{
    if (eventType == kPM_EventEnteringSleep)
    {
        /* De-init uart */
        PRINTF("De-init UART.\r\n");
        /* Wait for debug console output finished. */
        while (!(kLPUART_TransmissionCompleteFlag & LPUART_GetStatusFlags((LPUART_Type *)BOARD_DEBUG_UART_BASEADDR)))
        {
        }
        DbgConsole_Deinit();
        /*
         * Set pin for current leakage.
         * Debug console RX pin: Set to pinmux to disable.
         * Debug console TX pin: Don't need to change.
         */
        PORT_SetPinMux(DEBUG_CONSOLE_RX_PORT, DEBUG_CONSOLE_RX_PIN, kPORT_PinDisabledOrAnalog);
    }
    else
    {
        /* Re-init uart. */
        BOARD_InitPins();
        BOARD_BootClockRUN();
        BOARD_InitDebugConsole();
        if ((CMC_GetSystemResetStatus(CMC0) & kCMC_WakeUpReset) != 0UL)
        {
            /* Close ISO. */
            SPC_ClearPeriphIOIsolationFlag(SPC0);
        }
        PRINTF("Re-init UART.\r\n");
    }

    return kStatus_Success;
}

void APP_InitWakeupSource(void)
{
    PM_InitWakeupSource(&g_lptmr0WakeupSource, PM_WSID_LPTMR0, APP_Lptmr0WakeupService, true);
    PM_RegisterTimerController(&g_pmHandle, APP_StartLptmr, APP_StopLptmr, NULL, NULL);
}

void APP_Lptmr0WakeupService(void)
{
    if (kLPTMR_TimerInterruptEnable & LPTMR_GetEnabledInterrupts(LPTMR0))
    {
        LPTMR_DisableInterrupts(LPTMR0, kLPTMR_TimerInterruptEnable);
        LPTMR_ClearStatusFlags(LPTMR0, kLPTMR_TimerCompareFlag);
        LPTMR_StopTimer(LPTMR0);
    }
}

void APP_StartLptmr(uint64_t timeOutTickes)
{
    const lptmr_config_t DEMO_LPTMR_config = {.timerMode            = kLPTMR_TimerModeTimeCounter,
                                              .pinSelect            = kLPTMR_PinSelectInput_0,
                                              .pinPolarity          = kLPTMR_PinPolarityActiveHigh,
                                              .enableFreeRunning    = false,
                                              .bypassPrescaler      = true,
                                              .prescalerClockSource = kLPTMR_PrescalerClock_2,
                                              .value                = kLPTMR_Prescale_Glitch_0};

    LPTMR_Init(APP_LPTMR, &DEMO_LPTMR_config);
    LPTMR_SetTimerPeriod(APP_LPTMR, (uint32_t)timeOutTickes);
    LPTMR_EnableInterrupts(APP_LPTMR, kLPTMR_TimerInterruptEnable);
    LPTMR_StartTimer(APP_LPTMR);
}

void APP_StopLptmr(void)
{
    LPTMR_StopTimer(APP_LPTMR);
}

uint32_t APP_GetWakeupTimeout(void)
{
    uint8_t timeout;
    uint32_t timeoutTicks;

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
            timeoutTicks = (32000UL * timeout) - 1UL;
            return timeoutTicks;
        }
        PRINTF("Wrong value!\r\n");
    }
}

void APP_RegisterNotify(void)
{
    PM_RegisterNotify(kPM_NotifyGroup0, &g_notify1);
    PM_RegisterNotify(kPM_NotifyGroup2, &g_notify2);
}

void APP_SetConstraints(uint8_t powerMode)
{
    switch (powerMode)
    {
        case PM_LP_STATE_SLEEP:
        {
            PM_SetConstraints(PM_LP_STATE_SLEEP, APP_SLEEP_CONSTRAINTS);
            break;
        }

        case PM_LP_STATE_DEEP_SLEEP:
        {
            PM_SetConstraints(PM_LP_STATE_DEEP_SLEEP, APP_DEEP_SLEEP_CONSTRAINTS);
            break;
        }

        case PM_LP_STATE_POWER_DOWN:
        {
            PM_SetConstraints(PM_LP_STATE_POWER_DOWN, APP_POWER_DOWN_CONSTRAINTS);
            break;
        }

        case PM_LP_STATE_DEEP_POWER_DOWN:
        {
            PM_SetConstraints(PM_LP_STATE_DEEP_POWER_DOWN, APP_DEEP_POWER_DOWN_CONSTRAINTS);
            break;
        }

        default:
        {
            /* This branch will never be hit. */
            break;
        }
    }
}

void APP_ReleaseConstraints(uint8_t powerMode)
{
    switch (powerMode)
    {
        case PM_LP_STATE_SLEEP:
        {
            PM_ReleaseConstraints(PM_LP_STATE_SLEEP, APP_SLEEP_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_DEEP_SLEEP:
        {
            PM_ReleaseConstraints(PM_LP_STATE_DEEP_SLEEP, APP_DEEP_SLEEP_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_POWER_DOWN:
        {
            PM_ReleaseConstraints(PM_LP_STATE_POWER_DOWN, APP_POWER_DOWN_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_DEEP_POWER_DOWN:
        {
            PM_ReleaseConstraints(PM_LP_STATE_DEEP_POWER_DOWN, APP_DEEP_POWER_DOWN_CONSTRAINTS);
            break;
        }
        default:
        {
            /* This branch should never be hit. */
            break;
        }
    }
}


int main(void)
{
    uint32_t timeoutUs = 0UL;

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CMC_EnableDebugOperation(CMC0, false);
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
