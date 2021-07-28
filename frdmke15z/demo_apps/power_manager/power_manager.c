/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_smc.h"
#include "fsl_rcm.h"
#include "fsl_port.h"
#include "fsl_debug_console.h"
#include "power_manager.h"
#include "fsl_notifier.h"

#include "pin_mux.h"
#include "peripherals.h"
#include "board.h"
#include "fsl_pmc.h"
#include "fsl_lpuart.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_DEBUG_UART_BAUDRATE 9600U /* Debug console baud rate.           */

/* Default debug console clock source. */
#define APP_DEBUG_UART_DEFAULT_CLKSRC_NAME kCLOCK_ScgSircClk /* SCG SIRC clock. */

#define APP_LPTMR            DEMO_LPTMR_PERIPHERAL
#define APP_LPTMR_IRQHANDLER DEMO_LPTMR_IRQHANDLER

#define APP_WAKEUP_BUTTON_GPIO        BOARD_SW2_GPIO
#define APP_WAKEUP_BUTTON_PORT        BOARD_SW2_PORT
#define APP_WAKEUP_BUTTON_GPIO_PIN    BOARD_SW2_GPIO_PIN
#define APP_WAKEUP_BUTTON_IRQ         BOARD_SW2_IRQ
#define APP_WAKEUP_BUTTON_IRQ_HANDLER BOARD_SW2_IRQ_HANDLER
#define APP_WAKEUP_BUTTON_NAME        BOARD_SW2_NAME
#define APP_WAKEUP_BUTTON_IRQ_TYPE    kPORT_InterruptFallingEdge

/* Debug console RX pin: PORTC MUX: 2 */
#define DEBUG_CONSOLE_RX_PORT   PORTC
#define DEBUG_CONSOLE_RX_GPIO   GPIOC
#define DEBUG_CONSOLE_RX_PIN    6U
#define DEBUG_CONSOLE_RX_PINMUX kPORT_MuxAlt2
/* Debug console TX pin: PORTC MUX: 2 */
#define DEBUG_CONSOLE_TX_PORT   PORTC
#define DEBUG_CONSOLE_TX_GPIO   GPIOC
#define DEBUG_CONSOLE_TX_PIN    7U
#define DEBUG_CONSOLE_TX_PINMUX kPORT_MuxAlt2
#define CORE_CLK_FREQ           CLOCK_GetFreq(kCLOCK_CoreSysClk)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern void APP_InitClock(void); /* SCG init function defined in clock_scg.c */

/*
 * Set the clock configuration for RUN mode from VLPR mode.
 */
extern void APP_SetClockRunFromVlpr(void);

/*
 * Set the clock configuration for VLPR mode.
 */
extern void APP_SetClockVlpr(void);

/*
 * Power mode switch callback.
 */
status_t callback0(notifier_notification_block_t *notify, void *dataPtr);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint8_t s_wakeupTimeout;            /* Wakeup timeout. (Unit: Second) */
static app_wakeup_source_t s_wakeupSource; /* Wakeup source.                 */

/*******************************************************************************
 * Code
 ******************************************************************************/
static void APP_InitDebugConsole(void)
{
    CLOCK_SetIpSrc(kCLOCK_Lpuart1, kCLOCK_IpSrcSircAsync);
    uint32_t uartClkSrcFreq = CLOCK_GetIpFreq(kCLOCK_Lpuart1);
    DbgConsole_Init(BOARD_DEBUG_UART_INSTANCE, APP_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, uartClkSrcFreq);
}


status_t callback0(notifier_notification_block_t *notify, void *dataPtr)
{
    user_callback_data_t *userData = (user_callback_data_t *)dataPtr;
    status_t ret                   = kStatus_Fail;
    app_power_mode_t targetMode    = ((power_user_config_t *)notify->targetConfig)->mode;

    switch (notify->notifyType)
    {
        case kNOTIFIER_NotifyBefore:
            userData->beforeNotificationCounter++;
            /* Wait for debug console output finished. */
            while (
                !(kLPUART_TransmissionCompleteFlag & LPUART_GetStatusFlags((LPUART_Type *)BOARD_DEBUG_UART_BASEADDR)))
            {
            }
            DbgConsole_Deinit();

            if ((kAPP_PowerModeRun != targetMode) && (kAPP_PowerModeVlpr != targetMode))
            {
                /*
                 * Set pin for current leakage.
                 * Debug console RX pin: Set to pinmux to disable.
                 * Debug console TX pin: Don't need to change.
                 */
                PORT_SetPinMux(DEBUG_CONSOLE_RX_PORT, DEBUG_CONSOLE_RX_PIN, kPORT_PinDisabledOrAnalog);
            }

            ret = kStatus_Success;
            break;
        case kNOTIFIER_NotifyRecover:
            break;
        case kNOTIFIER_CallbackAfter:
            userData->afterNotificationCounter++;
            if ((kAPP_PowerModeRun != targetMode) && (kAPP_PowerModeVlpr != targetMode))
            {
                /*
                 * Debug console RX pin is set to disable for current leakage, nee to re-configure pinmux.
                 * Debug console TX pin: Don't need to change.
                 */
                PORT_SetPinMux(DEBUG_CONSOLE_RX_PORT, DEBUG_CONSOLE_RX_PIN, DEBUG_CONSOLE_RX_PINMUX);
            }

            APP_InitDebugConsole();

            ret = kStatus_Success;
            break;
        default:
            break;
    }
    return ret;
}

/*!
 * @brief LPTMR0 interrupt handler.
 */
void APP_LPTMR_IRQHANDLER(void)
{
    if (kLPTMR_TimerInterruptEnable & LPTMR_GetEnabledInterrupts(APP_LPTMR))
    {
        LPTMR_DisableInterrupts(APP_LPTMR, kLPTMR_TimerInterruptEnable);
        LPTMR_ClearStatusFlags(APP_LPTMR, kLPTMR_TimerCompareFlag);
        LPTMR_StopTimer(APP_LPTMR);
    }
    __DSB();
}

/*!
 * @brief button interrupt handler.
 */
void APP_WAKEUP_BUTTON_IRQ_HANDLER(void)
{
    if ((1U << APP_WAKEUP_BUTTON_GPIO_PIN) & PORT_GetPinsInterruptFlags(APP_WAKEUP_BUTTON_PORT))
    {
        /* Disable interrupt. */
        PORT_SetPinInterruptConfig(APP_WAKEUP_BUTTON_PORT, APP_WAKEUP_BUTTON_GPIO_PIN, kPORT_InterruptOrDMADisabled);
        PORT_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_PORT, (1U << APP_WAKEUP_BUTTON_GPIO_PIN));
    }
    __DSB();
}

/*!
 * @brief Get input from user to obtain wakeup timeout
 */
static uint8_t APP_GetWakeupTimeout(void)
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
            return timeout - '0';
        }
        PRINTF("Wrong value!\r\n");
    }
}

/*!
 * @brief Get wakeup source by user input.
 */
static app_wakeup_source_t APP_GetWakeupSource(void)
{
    uint8_t ch;

    while (1)
    {
        PRINTF("Select the wake up source:\r\n");
        PRINTF("Press T for LPTMR - Low Power Timer\r\n");
        PRINTF("Press S for switch/button %s. \r\n", APP_WAKEUP_BUTTON_NAME);

        PRINTF("\r\nWaiting for key press..\r\n\r\n");

        ch = GETCHAR();

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }

        if (ch == 'T')
        {
            return kAPP_WakeupSourceLptmr;
        }
        else if (ch == 'S')
        {
            return kAPP_WakeupSourcePin;
        }
        else
        {
            PRINTF("Wrong value!\r\n");
        }
    }
}

/*! @brief Get wakeup timeout and wakeup source. */
void APP_GetWakeupConfig(app_power_mode_t targetMode)
{
    /* Get wakeup source by user input. */
    {
        /* Get wakeup source by user input. */
        s_wakeupSource = APP_GetWakeupSource();
    }

    if (kAPP_WakeupSourceLptmr == s_wakeupSource)
    {
        /* Wakeup source is LPTMR, user should input wakeup timeout value. */
        s_wakeupTimeout = APP_GetWakeupTimeout();
        PRINTF("Will wakeup in %d seconds.\r\n", s_wakeupTimeout);
    }
    else
    {
        PRINTF("Press %s to wake up.\r\n", APP_WAKEUP_BUTTON_NAME);
    }
}

/*! @brief Set wakeup timeout and wakeup source. */
void APP_SetWakeupConfig(app_power_mode_t targetMode)
{
    /* Set LPTMR timeout value. */
    if (kAPP_WakeupSourceLptmr == s_wakeupSource)
    {
        LPTMR_SetTimerPeriod(APP_LPTMR, (1000U * s_wakeupTimeout) - 1U);
        LPTMR_StartTimer(APP_LPTMR);
    }

    /* Set the wakeup module. */
    if (kAPP_WakeupSourceLptmr == s_wakeupSource)
    {
        LPTMR_EnableInterrupts(APP_LPTMR, kLPTMR_TimerInterruptEnable);
    }
    else
    {
        PORT_SetPinInterruptConfig(APP_WAKEUP_BUTTON_PORT, APP_WAKEUP_BUTTON_GPIO_PIN, APP_WAKEUP_BUTTON_IRQ_TYPE);
    }

    /* If targetMode is VLLS/LLS, setup LLWU. */
    if ((kAPP_PowerModeWait != targetMode) && (kAPP_PowerModeVlpw != targetMode) &&
        (kAPP_PowerModeVlps != targetMode) && (kAPP_PowerModeStop != targetMode))
    {
    }
}

/*! @brief Show current power mode. */
void APP_ShowPowerMode(smc_power_state_t currentPowerState)
{
    switch (currentPowerState)
    {
        case kSMC_PowerStateRun:
            PRINTF("    Power mode: RUN\r\n");
            break;
        case kSMC_PowerStateVlpr:
            PRINTF("    Power mode: VLPR\r\n");
            break;
        default:
            PRINTF("    Power mode wrong\r\n");
            break;
    }
}

/*!
 * @brief check whether could switch to target power mode from current mode.
 * Return true if could switch, return false if could not switch.
 */
bool APP_CheckPowerMode(smc_power_state_t currentPowerState, app_power_mode_t targetPowerMode)
{
    bool modeValid = true;

    /*
     * Check wether the mode change is allowed.
     *
     * 1. If current mode is HSRUN mode, the target mode must be RUN mode.
     * 2. If current mode is RUN mode, the target mode must not be VLPW mode.
     * 3. If current mode is VLPR mode, the target mode must not be HSRUN/WAIT/STOP mode.
     * 4. If already in the target mode, don't need to change.
     */
    switch (currentPowerState)
    {
        case kSMC_PowerStateRun:
            if (kAPP_PowerModeVlpw == targetPowerMode)
            {
                PRINTF("Could not enter VLPW mode from RUN mode.\r\n");
                modeValid = false;
            }
            break;

        case kSMC_PowerStateVlpr:
            if ((kAPP_PowerModeWait == targetPowerMode) || (kAPP_PowerModeStop == targetPowerMode))
            {
                PRINTF("Could not enter STOP/WAIT mode from VLPR mode.\r\n");
                modeValid = false;
            }
            break;
        default:
            PRINTF("Wrong power state.\r\n");
            modeValid = false;
            break;
    }

    if (!modeValid)
    {
        return false;
    }

    /* Don't need to change power mode if current mode is already the target mode. */
    if (((kAPP_PowerModeRun == targetPowerMode) && (kSMC_PowerStateRun == currentPowerState)) ||
        ((kAPP_PowerModeVlpr == targetPowerMode) && (kSMC_PowerStateVlpr == currentPowerState)))
    {
        PRINTF("Already in the target power mode.\r\n");
        return false;
    }

    return true;
}

/*!
 * @brief Power mode switch.
 * This function is used to register the notifier_handle_t struct's member userFunction.
 */
status_t APP_PowerModeSwitch(notifier_user_config_t *targetConfig, void *userData)
{
    smc_power_state_t currentPowerMode;         /* Local variable with current power mode */
    app_power_mode_t targetPowerMode;           /* Local variable with target power mode name*/
    power_user_config_t *targetPowerModeConfig; /* Local variable with target power mode configuration */

    targetPowerModeConfig = (power_user_config_t *)targetConfig;
    currentPowerMode      = SMC_GetPowerModeState(SMC);
    targetPowerMode       = targetPowerModeConfig->mode;

    switch (targetPowerMode)
    {
        case kAPP_PowerModeVlpr:
            APP_SetClockVlpr();
            SMC_SetPowerModeVlpr(SMC);
            while (kSMC_PowerStateVlpr != SMC_GetPowerModeState(SMC))
            {
            }
            break;

        case kAPP_PowerModeRun:

            /* Power mode change. */
            SMC_SetPowerModeRun(SMC);
            while (kSMC_PowerStateRun != SMC_GetPowerModeState(SMC))
            {
            }

            /* If enter RUN from VLPR, change clock after the power mode change. */
            if (kSMC_PowerStateVlpr == currentPowerMode)
            {
                APP_SetClockRunFromVlpr();
            }
            break;

        /* For wait modes. */
        case kAPP_PowerModeWait:
            SMC_PreEnterWaitModes();
            SMC_SetPowerModeWait(SMC);
            SMC_PostExitWaitModes();
            break;
        case kAPP_PowerModeVlpw:
            SMC_PreEnterWaitModes();
            SMC_SetPowerModeVlpw(SMC);
            SMC_PostExitWaitModes();
            break;

        /* For stop modes. */
        case kAPP_PowerModeStop:
            SMC_PreEnterStopModes();
            SMC_SetPowerModeStop(SMC, kSMC_PartialStop);
            SMC_PostExitStopModes();
            break;

        case kAPP_PowerModeVlps:
            SMC_PreEnterStopModes();
            SMC_SetPowerModeVlps(SMC);
            SMC_PostExitStopModes();
            break;

        default:
            PRINTF("Wrong value");
            break;
    }
    return kStatus_Success;
}

/*!
 * @brief main demo function.
 */
int main(void)
{
    uint32_t freq = 0;
    uint8_t ch;
    uint8_t targetConfigIndex;
    notifier_handle_t powerModeHandle;
    smc_power_state_t currentPowerState;
    app_power_mode_t targetPowerMode;
    bool needSetWakeup; /* Flag of whether or not need to set wakeup. */

    /*Power mode configurations*/
    power_user_config_t vlprConfig = {
        kAPP_PowerModeVlpr,

    };

    power_user_config_t vlpwConfig = vlprConfig;
    power_user_config_t vlpsConfig = vlprConfig;
    power_user_config_t waitConfig = vlprConfig;
    power_user_config_t stopConfig = vlprConfig;
    power_user_config_t runConfig  = vlprConfig;

    /* Initializes array of pointers to power mode configurations */
    notifier_user_config_t *powerConfigs[] = {
        &runConfig, &waitConfig, &stopConfig, &vlprConfig, &vlpwConfig, &vlpsConfig,

    };

    /* User callback data0 */
    user_callback_data_t callbackData0;

    /* Initializes callback configuration structure */
    notifier_callback_config_t callbackCfg0 = {callback0, kNOTIFIER_CallbackBeforeAfter, (void *)&callbackData0};

    /* Initializes array of callback configurations */
    notifier_callback_config_t callbacks[] = {callbackCfg0};

    memset(&callbackData0, 0, sizeof(user_callback_data_t));

    /* Initializes configuration structures */
    vlpwConfig.mode = kAPP_PowerModeVlpw;
    vlpsConfig.mode = kAPP_PowerModeVlps;
    waitConfig.mode = kAPP_PowerModeWait;
    stopConfig.mode = kAPP_PowerModeStop;
    runConfig.mode  = kAPP_PowerModeRun;

    /* Create Notifier Handle */
    NOTIFIER_CreateHandle(&powerModeHandle, powerConfigs, ARRAY_SIZE(powerConfigs), callbacks, 1U, APP_PowerModeSwitch,
                          NULL);

    /* Power related. */
    SMC_SetPowerModeProtection(SMC, kSMC_AllowPowerModeAll);

    BOARD_InitBootPins();
    APP_InitClock();
    APP_InitDebugConsole();
    BOARD_InitBootPeripherals();

    EnableIRQ(APP_WAKEUP_BUTTON_IRQ);

    while (1)
    {
        currentPowerState = SMC_GetPowerModeState(SMC);

        freq = CLOCK_GetFreq(kCLOCK_CoreSysClk);

        PRINTF("\r\n####################  Power Manager Demo ####################\n\r\n");
        PRINTF("    Core Clock = %dHz \r\n", freq);

        APP_ShowPowerMode(currentPowerState);

        PRINTF("\r\nSelect the desired operation \n\r\n");
        PRINTF("Press  %c for enter: RUN      - Normal RUN mode\r\n", kAPP_PowerModeRun);
        PRINTF("Press  %c for enter: WAIT     - Wait mode\r\n", kAPP_PowerModeWait);
        PRINTF("Press  %c for enter: STOP     - Stop mode\r\n", kAPP_PowerModeStop);
        PRINTF("Press  %c for enter: VLPR     - Very Low Power Run mode\r\n", kAPP_PowerModeVlpr);
        PRINTF("Press  %c for enter: VLPW     - Very Low Power Wait mode\r\n", kAPP_PowerModeVlpw);
        PRINTF("Press  %c for enter: VLPS     - Very Low Power Stop mode\r\n", kAPP_PowerModeVlps);

        PRINTF("\r\nWaiting for power mode select..\r\n\r\n");

        /* Wait for user response */
        ch = GETCHAR();

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }

        targetPowerMode = (app_power_mode_t)ch;

        if ((targetPowerMode > kAPP_PowerModeMin) && (targetPowerMode < kAPP_PowerModeMax))
        {
            /* If could not set the target power mode, loop continue. */
            if (!APP_CheckPowerMode(currentPowerState, targetPowerMode))
            {
                continue;
            }

            /* If target mode is RUN/VLPR/HSRUN, don't need to set wakeup source. */
            if ((kAPP_PowerModeRun == targetPowerMode) || (kAPP_PowerModeVlpr == targetPowerMode))
            {
                needSetWakeup = false;
            }
            else
            {
                needSetWakeup = true;
            }

            if (needSetWakeup)
            {
                APP_GetWakeupConfig(targetPowerMode);
                APP_SetWakeupConfig(targetPowerMode);
            }

            callbackData0.originPowerState = currentPowerState;
            targetConfigIndex              = targetPowerMode - kAPP_PowerModeMin - 1;
            NOTIFIER_SwitchConfig(&powerModeHandle, targetConfigIndex, kNOTIFIER_PolicyAgreement);
            PRINTF("\r\nNext loop\r\n");
        }
    }
}
