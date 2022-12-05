/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_smc.h"
#include "fsl_rcm.h"
#include "fsl_port.h"
#include "power_mode_switch.h"
#include "fsl_debug_console.h"

#include "pin_mux.h"
#include "peripherals.h"
#include "board.h"
#include "fsl_pmc.h"
#include "fsl_uart.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_DEBUG_UART_BAUDRATE    9600              /* Debug console baud rate.           */
#define APP_DEBUG_UART_CLKSRC_NAME kCLOCK_CoreSysClk /* System clock.       */

#define APP_LLWU            DEMO_LLWU_PERIPHERAL
#define APP_LLWU_IRQHANDLER DEMO_LLWU_IRQHANDLER

#define APP_LPTMR            DEMO_LPTMR_PERIPHERAL
#define APP_LPTMR_IRQHANDLER DEMO_LPTMR_IRQHANDLER

#define LLWU_LPTMR_IDX       0U  /* LLWU_M0IF */
#define LLWU_WAKEUP_PIN_IDX  10U /* LLWU_P10  */
#define LLWU_WAKEUP_PIN_TYPE kLLWU_ExternalPinFallingEdge

#define APP_WAKEUP_BUTTON_GPIO        BOARD_SW2_GPIO
#define APP_WAKEUP_BUTTON_PORT        BOARD_SW2_PORT
#define APP_WAKEUP_BUTTON_GPIO_PIN    BOARD_SW2_GPIO_PIN
#define APP_WAKEUP_BUTTON_IRQ         BOARD_SW2_IRQ
#define APP_WAKEUP_BUTTON_IRQ_HANDLER BOARD_SW2_IRQ_HANDLER
#define APP_WAKEUP_BUTTON_NAME        BOARD_SW2_NAME
#define APP_WAKEUP_BUTTON_IRQ_TYPE    kPORT_InterruptFallingEdge

/* Debug console RX pin: PORTB16 MUX: 3 */
#define DEBUG_CONSOLE_RX_PORT   PORTB
#define DEBUG_CONSOLE_RX_GPIO   GPIOB
#define DEBUG_CONSOLE_RX_PIN    16
#define DEBUG_CONSOLE_RX_PINMUX kPORT_MuxAlt3
/* Debug console TX pin: PORTB17 MUX: 3 */
#define DEBUG_CONSOLE_TX_PORT   PORTB
#define DEBUG_CONSOLE_TX_GPIO   GPIOB
#define DEBUG_CONSOLE_TX_PIN    17
#define DEBUG_CONSOLE_TX_PINMUX kPORT_MuxAlt3
#define CORE_CLK_FREQ           CLOCK_GetFreq(kCLOCK_CoreSysClk)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_PowerPreSwitchHook(smc_power_state_t originPowerState, app_power_mode_t targetMode);
void APP_PowerPostSwitchHook(smc_power_state_t originPowerState, app_power_mode_t targetMode);

/*
 * Set the clock configuration for RUN mode from VLPR mode.
 */
extern void APP_SetClockRunFromVlpr(void);

/*
 * Set the clock configuration for VLPR mode.
 */
extern void APP_SetClockVlpr(void);

/*
 * Hook function called before power mode switch.
 */
extern void APP_PowerPreSwitchHook(smc_power_state_t originPowerState, app_power_mode_t targetMode);

/*
 * Hook function called after power mode switch.
 */
extern void APP_PowerPostSwitchHook(smc_power_state_t originPowerState, app_power_mode_t targetMode);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint8_t s_wakeupTimeout;            /* Wakeup timeout. (Unit: Second) */
static app_wakeup_source_t s_wakeupSource; /* Wakeup source.                 */

/*******************************************************************************
 * Code
 ******************************************************************************/

void APP_SetClockVlpr(void)
{
    const sim_clock_config_t simConfig = {
        .pllFllSel = 3U,          /* PLLFLLSEL select IRC48MCLK. */
        .er32kSrc  = 2U,          /* ERCLK32K selection, use RTC. */
        .clkdiv1   = 0x00040000U, /* SIM_CLKDIV1. */
    };

    CLOCK_SetSimSafeDivs();
    CLOCK_SetInternalRefClkConfig(kMCG_IrclkEnable, kMCG_IrcFast, 0U);

    /* MCG works in PEE mode now, will switch to BLPI mode. */

    CLOCK_ExternalModeToFbeModeQuick();                     /* Enter FBE. */
    CLOCK_SetFbiMode(kMCG_Dmx32Default, kMCG_DrsLow, NULL); /* Enter FBI. */
    CLOCK_SetLowPowerEnable(true);                          /* Enter BLPI. */

    CLOCK_SetSimConfig(&simConfig);
}

void APP_SetClockRunFromVlpr(void)
{
    const sim_clock_config_t simConfig = {
        .pllFllSel = 1U,          /* PLLFLLSEL select PLL. */
        .er32kSrc  = 2U,          /* ERCLK32K selection, use RTC. */
        .clkdiv1   = 0x01140000U, /* SIM_CLKDIV1. */
    };

    const mcg_pll_config_t pll0Config = {
        .enableMode = 0U,
        .prdiv      = 0x13U,
        .vdiv       = 0x18U,
    };

    CLOCK_SetSimSafeDivs();

    /* Currently in BLPI mode, will switch to PEE mode. */
    /* Enter FBI. */
    CLOCK_SetLowPowerEnable(false);
    /* Enter FBE. */
    CLOCK_SetFbeMode(7U, kMCG_Dmx32Default, kMCG_DrsLow, NULL);
    /* Enter PBE. */
    CLOCK_SetPbeMode(kMCG_PllClkSelPll0, &pll0Config);
    /* Enter PEE. */
    CLOCK_SetPeeMode();

    CLOCK_SetSimConfig(&simConfig);
}

static void APP_InitDebugConsole(void)
{
    uint32_t uartClkSrcFreq;
    uartClkSrcFreq = CLOCK_GetFreq(APP_DEBUG_UART_CLKSRC_NAME);
    DbgConsole_Init(BOARD_DEBUG_UART_INSTANCE, APP_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, uartClkSrcFreq);
}


void APP_PowerPreSwitchHook(smc_power_state_t originPowerState, app_power_mode_t targetMode)
{
    /* Wait for debug console output finished. */
    while (!(kUART_TransmissionCompleteFlag & UART_GetStatusFlags((UART_Type *)BOARD_DEBUG_UART_BASEADDR)))
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
        PORT_SetPinMux(DEBUG_CONSOLE_TX_PORT, DEBUG_CONSOLE_TX_PIN, kPORT_PinDisabledOrAnalog);
    }

    /* Disable the PLL in WAIT mode to same more power. FEI mode. */
    if (kAPP_PowerModeWait == targetMode)
    {
        const mcg_config_t mcgConfigStruct = {
            .mcgMode         = kMCG_ModeFEI,      /* FEI - FLL with internal RTC. */
            .irclkEnableMode = kMCG_IrclkEnable,  /* MCGIRCLK enabled, MCGIRCLK disabled in STOP mode */
            .ircs            = kMCG_IrcSlow,      /* Slow internal reference clock selected */
            .fcrdiv          = 0x0U,              /* Fast IRC divider: divided by 1 */
            .frdiv           = 0x0U,              /* FLL reference clock divider: divided by 32 */
            .drs             = kMCG_DrsLow,       /* Low frequency range */
            .dmx32           = kMCG_Dmx32Default, /* DCO has a default range of 25% */
            .oscsel          = kMCG_OscselOsc,    /* Selects System Oscillator (OSCCLK) */
            .pll0Config =
                {
                    .enableMode = 0,     /* MCGPLLCLK disabled */
                    .prdiv      = 0x13U, /* PLL Reference divider: divided by 20 */
                    .vdiv       = 0x18U, /* VCO divider: multiplied by 48 */
                },
        };

        CLOCK_SetMcgConfig(&mcgConfigStruct);
    }
}

void APP_PowerPostSwitchHook(smc_power_state_t originPowerState, app_power_mode_t targetMode)
{
    /* Restore the UART pins. */
    if ((kAPP_PowerModeRun != targetMode) && (kAPP_PowerModeVlpr != targetMode))
    {
        /*
         * Debug console RX pin is set to disable for current leakage, nee to re-configure pinmux.
         * Debug console TX pin: Don't need to change.
         */
        PORT_SetPinMux(DEBUG_CONSOLE_RX_PORT, DEBUG_CONSOLE_RX_PIN, DEBUG_CONSOLE_RX_PINMUX);
        PORT_SetPinMux(DEBUG_CONSOLE_TX_PORT, DEBUG_CONSOLE_TX_PIN, DEBUG_CONSOLE_TX_PINMUX);
    }

    /*
     * For some other platforms, if enter LLS mode from VLPR mode, when wakeup, the
     * power mode is VLPR. But for some platforms, if enter LLS mode from VLPR mode,
     * when wakeup, the power mode is RUN. In this case, the clock setting is still
     * VLPR mode setting, so change to RUN mode setting here.
     */
    if ((kSMC_PowerStateVlpr == originPowerState) && (kSMC_PowerStateRun == SMC_GetPowerModeState(SMC)))
    {
        APP_SetClockRunFromVlpr();
    }

    /*
     * If enter stop modes when MCG in PEE mode, then after wakeup, the MCG is in PBE mode,
     * need to enter PEE mode manually.
     */
    if ((kAPP_PowerModeRun != targetMode) && (kAPP_PowerModeWait != targetMode) && (kAPP_PowerModeVlpw != targetMode) &&
        (kAPP_PowerModeVlpr != targetMode))
    {
        if (kSMC_PowerStateRun == originPowerState)
        {
            /* Wait for PLL lock. */
            while (!(kMCG_Pll0LockFlag & CLOCK_GetStatusFlags()))
            {
            }
            CLOCK_SetPeeMode();
        }
    }

    /* Return the PEE mode when come back to RUN mode from WAIT mode. */
    if ((kAPP_PowerModeWait == targetMode) && (kSMC_PowerStateRun == originPowerState))
    {
        const mcg_config_t mcgConfigStruct = {
            .mcgMode         = kMCG_ModePEE,      /* PEE - PLL Engaged External */
            .irclkEnableMode = kMCG_IrclkEnable,  /* MCGIRCLK enabled, MCGIRCLK disabled in STOP mode */
            .ircs            = kMCG_IrcSlow,      /* Slow internal reference clock selected */
            .fcrdiv          = 0x0U,              /* Fast IRC divider: divided by 1 */
            .frdiv           = 0x0U,              /* FLL reference clock divider: divided by 32 */
            .drs             = kMCG_DrsLow,       /* Low frequency range */
            .dmx32           = kMCG_Dmx32Default, /* DCO has a default range of 25% */
            .oscsel          = kMCG_OscselOsc,    /* Selects System Oscillator (OSCCLK) */
            .pll0Config =
                {
                    .enableMode = 0,     /* MCGPLLCLK disabled */
                    .prdiv      = 0x13U, /* PLL Reference divider: divided by 20 */
                    .vdiv       = 0x18U, /* VCO divider: multiplied by 48 */
                },
        };

        CLOCK_SetMcgConfig(&mcgConfigStruct);
    }

    APP_InitDebugConsole();
}

/*!
 * @brief LLWU interrupt handler.
 */
void APP_LLWU_IRQHANDLER(void)
{
    /* If wakeup by LPTMR. */
    if (LLWU_GetInternalWakeupModuleFlag(APP_LLWU, LLWU_LPTMR_IDX))
    {
        /* Disable lptmr as a wakeup source, so that lptmr's IRQ Handler will be executed when reset from VLLSx mode. */
        LLWU_EnableInternalModuleInterruptWakup(APP_LLWU, LLWU_LPTMR_IDX, false);
    }
    /* If wakeup by external pin. */
    if (LLWU_GetExternalWakeupPinFlag(APP_LLWU, LLWU_WAKEUP_PIN_IDX))
    {
        /* Disable WAKEUP pin as a wakeup source, so that WAKEUP pin's IRQ Handler will be executed when reset from
         * VLLSx mode. */
        LLWU_ClearExternalWakeupPinFlag(APP_LLWU, LLWU_WAKEUP_PIN_IDX);
    }
    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
    exception return operation might vector to incorrect interrupt */
    __DSB();
}

void APP_LPTMR_IRQHANDLER(void)
{
    if (kLPTMR_TimerInterruptEnable & LPTMR_GetEnabledInterrupts(APP_LPTMR))
    {
        LPTMR_DisableInterrupts(APP_LPTMR, kLPTMR_TimerInterruptEnable);
        LPTMR_ClearStatusFlags(APP_LPTMR, kLPTMR_TimerCompareFlag);
        LPTMR_StopTimer(APP_LPTMR);
    }
    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
    exception return operation might vector to incorrect interrupt */
    __DSB();
}

void APP_WAKEUP_BUTTON_IRQ_HANDLER(void)
{
    if ((1U << APP_WAKEUP_BUTTON_GPIO_PIN) & PORT_GetPinsInterruptFlags(APP_WAKEUP_BUTTON_PORT))
    {
        /* Disable interrupt. */
        PORT_SetPinInterruptConfig(APP_WAKEUP_BUTTON_PORT, APP_WAKEUP_BUTTON_GPIO_PIN, kPORT_InterruptOrDMADisabled);
        PORT_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_PORT, (1U << APP_WAKEUP_BUTTON_GPIO_PIN));
    }
    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
    exception return operation might vector to incorrect interrupt */
    __DSB();
}

/*!
 * @brief Get input from user about wakeup timeout
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

/* Get wakeup source by user input. */
static app_wakeup_source_t APP_GetWakeupSource(void)
{
    uint8_t ch;

    while (1)
    {
        PRINTF("Select the wake up source:\r\n");
        PRINTF("Press T for LPTMR - Low Power Timer\r\n");
        PRINTF("Press S for switch/button %s.\r\n", APP_WAKEUP_BUTTON_NAME);

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

/* Get wakeup timeout and wakeup source. */
void APP_GetWakeupConfig(app_power_mode_t targetMode)
{
    /* Get wakeup source by user input. */
    if (targetMode == kAPP_PowerModeVlls0)
    {
        /* In VLLS0 mode, the LPO is disabled, LPTMR could not work. */
        PRINTF("Not support LPTMR wakeup because LPO is disabled in VLLS0 mode.\r\n");
        s_wakeupSource = kAPP_WakeupSourcePin;
    }
    else
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

void APP_SetWakeupConfig(app_power_mode_t targetMode)
{
    /* Set LPTMR timeout value. */
    if (kAPP_WakeupSourceLptmr == s_wakeupSource)
    {
        LPTMR_SetTimerPeriod(APP_LPTMR, (LPO_CLK_FREQ * s_wakeupTimeout) - 1U);
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
        if (kAPP_WakeupSourceLptmr == s_wakeupSource)
        {
            LLWU_EnableInternalModuleInterruptWakup(APP_LLWU, LLWU_LPTMR_IDX, true);
        }
        else
        {
            LLWU_SetExternalWakeupPinMode(APP_LLWU, LLWU_WAKEUP_PIN_IDX, LLWU_WAKEUP_PIN_TYPE);
        }
        NVIC_EnableIRQ(LLWU_IRQn);
    }
}

void APP_ShowPowerMode(smc_power_state_t powerMode)
{
    switch (powerMode)
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

/*
 * Check whether could switch to target power mode from current mode.
 * Return true if could switch, return false if could not switch.
 */
bool APP_CheckPowerMode(smc_power_state_t curPowerState, app_power_mode_t targetPowerMode)
{
    bool modeValid = true;

    /*
     * Check wether the mode change is allowed.
     *
     * 1. If current mode is HSRUN mode, the target mode must be RUN mode.
     * 2. If current mode is RUN mode, the target mode must not be VLPW mode.
     * 3. If current mode is VLPR mode, the target mode must not be HSRUN/WAIT/STOP mode.
     * 4. If already in the target mode.
     */
    switch (curPowerState)
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
                PRINTF("Could not enter HSRUN/STOP/WAIT modes from VLPR mode.\r\n");
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
    if (((kAPP_PowerModeRun == targetPowerMode) && (kSMC_PowerStateRun == curPowerState)) ||
        ((kAPP_PowerModeVlpr == targetPowerMode) && (kSMC_PowerStateVlpr == curPowerState)))
    {
        PRINTF("Already in the target power mode.\r\n");
        return false;
    }

    return true;
}

/*
 * Power mode switch.
 */
void APP_PowerModeSwitch(smc_power_state_t curPowerState, app_power_mode_t targetPowerMode)
{
    smc_power_mode_vlls_config_t vlls_config;
    vlls_config.enablePorDetectInVlls0 = true;

    switch (targetPowerMode)
    {
        case kAPP_PowerModeVlpr:
            APP_SetClockVlpr();
            SMC_SetPowerModeVlpr(SMC, false);
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
            if (kSMC_PowerStateVlpr == curPowerState)
            {
                APP_SetClockRunFromVlpr();
            }
            break;

        case kAPP_PowerModeWait:
            SMC_PreEnterWaitModes();
            SMC_SetPowerModeWait(SMC);
            SMC_PostExitWaitModes();
            break;

        case kAPP_PowerModeStop:
            SMC_PreEnterStopModes();
            SMC_SetPowerModeStop(SMC, kSMC_PartialStop);
            SMC_PostExitStopModes();
            break;

        case kAPP_PowerModeVlpw:
            SMC_PreEnterWaitModes();
            SMC_SetPowerModeVlpw(SMC);
            SMC_PostExitWaitModes();
            break;

        case kAPP_PowerModeVlps:
            SMC_PreEnterStopModes();
            SMC_SetPowerModeVlps(SMC);
            SMC_PostExitStopModes();
            break;

        case kAPP_PowerModeLls:
            SMC_PreEnterStopModes();
            SMC_SetPowerModeLls(SMC);
            SMC_PostExitStopModes();
            break;

        case kAPP_PowerModeVlls0:
            vlls_config.subMode = kSMC_StopSub0;
            SMC_PreEnterStopModes();
            SMC_SetPowerModeVlls(SMC, &vlls_config);
            SMC_PostExitStopModes();
            break;

        case kAPP_PowerModeVlls1:
            vlls_config.subMode = kSMC_StopSub1;
            SMC_PreEnterStopModes();
            SMC_SetPowerModeVlls(SMC, &vlls_config);
            SMC_PostExitStopModes();
            break;

        case kAPP_PowerModeVlls2:
            vlls_config.subMode = kSMC_StopSub2;
            SMC_PreEnterStopModes();
            SMC_SetPowerModeVlls(SMC, &vlls_config);
            SMC_PostExitStopModes();
            break;

        case kAPP_PowerModeVlls3:
            vlls_config.subMode = kSMC_StopSub3;
            SMC_PreEnterStopModes();
            SMC_SetPowerModeVlls(SMC, &vlls_config);
            SMC_PostExitStopModes();
            break;

        default:
            PRINTF("Wrong value");
            break;
    }
}

/*!
 * @brief main demo function.
 */
int main(void)
{
    uint32_t freq = 0;
    uint8_t ch;
    smc_power_state_t curPowerState;
    app_power_mode_t targetPowerMode;
    bool needSetWakeup; /* Need to set wakeup. */

    /* Must configure pins before PMC_ClearPeriphIOIsolationFlag */
    BOARD_InitBootPins();

    /* Power related. */
    SMC_SetPowerModeProtection(SMC, kSMC_AllowPowerModeAll);
    if (kRCM_SourceWakeup & RCM_GetPreviousResetSources(RCM)) /* Wakeup from VLLS. */
    {
        PMC_ClearPeriphIOIsolationFlag(PMC);
        NVIC_ClearPendingIRQ(LLWU_IRQn);
    }

    BOARD_InitBootClocks();
    APP_InitDebugConsole();
    BOARD_InitBootPeripherals();

    NVIC_EnableIRQ(APP_WAKEUP_BUTTON_IRQ);

    if (kRCM_SourceWakeup & RCM_GetPreviousResetSources(RCM)) /* Wakeup from VLLS. */
    {
        PRINTF("\r\nMCU wakeup from VLLS modes...\r\n");
    }
    while (1)
    {
        curPowerState = SMC_GetPowerModeState(SMC);

        freq = CLOCK_GetFreq(kCLOCK_CoreSysClk);

        PRINTF("\r\n####################  Power Mode Switch Demo ####################\r\n\r\n");
        PRINTF("    Core Clock = %dHz\r\n", freq);

        APP_ShowPowerMode(curPowerState);

        PRINTF("\r\nSelect the desired operation\r\n\r\n");
        PRINTF("Press  %c for enter: RUN      - Normal RUN mode\r\n", kAPP_PowerModeRun);
        PRINTF("Press  %c for enter: WAIT     - Wait mode\r\n", kAPP_PowerModeWait);
        PRINTF("Press  %c for enter: STOP     - Stop mode\r\n", kAPP_PowerModeStop);
        PRINTF("Press  %c for enter: VLPR     - Very Low Power Run mode\r\n", kAPP_PowerModeVlpr);
        PRINTF("Press  %c for enter: VLPW     - Very Low Power Wait mode\r\n", kAPP_PowerModeVlpw);
        PRINTF("Press  %c for enter: VLPS     - Very Low Power Stop mode\r\n", kAPP_PowerModeVlps);
        PRINTF("Press  %c for enter: LLS/LLS3 - Low Leakage Stop mode\r\n", kAPP_PowerModeLls);
        PRINTF("Press  %c for enter: VLLS0    - Very Low Leakage Stop 0 mode\r\n", kAPP_PowerModeVlls0);
        PRINTF("Press  %c for enter: VLLS1    - Very Low Leakage Stop 1 mode\r\n", kAPP_PowerModeVlls1);
        PRINTF("Press  %c for enter: VLLS2    - Very Low Leakage Stop 2 mode\r\n", kAPP_PowerModeVlls2);
        PRINTF("Press  %c for enter: VLLS3    - Very Low Leakage Stop 3 mode\r\n", kAPP_PowerModeVlls3);

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
            if (!APP_CheckPowerMode(curPowerState, targetPowerMode))
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
            }

            APP_PowerPreSwitchHook(curPowerState, targetPowerMode);

            if (needSetWakeup)
            {
                APP_SetWakeupConfig(targetPowerMode);
            }

            APP_PowerModeSwitch(curPowerState, targetPowerMode);
            APP_PowerPostSwitchHook(curPowerState, targetPowerMode);

            PRINTF("\r\nNext loop\r\n");
        }
    }
}
