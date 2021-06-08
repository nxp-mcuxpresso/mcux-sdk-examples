/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
#include "fsl_llwu.h"
#include "fsl_lptmr.h"
#include "fsl_msmc.h"
#include "fsl_pmc0.h"
#include "fsl_mu.h"
#include "fsl_debug_console.h"

#include "pin_mux.h"
#include "board.h"
#include "lpm.h"
#include "app_srtm.h"
#include "power_mode_switch.h"

#include "fsl_iomuxc.h"
#include "fsl_lpuart.h"
/*******************************************************************************
 * Struct Definitions
 ******************************************************************************/
#define APP_DEBUG_UART_BAUDRATE       (115200U)             /* Debug console baud rate. */
#define APP_DEBUG_UART_DEFAULT_CLKSRC kCLOCK_IpSrcSircAsync /* SCG SIRC clock. */

/* LPTMR0 is LLWU internal module 0. */
#define SYSTICK_LLWU_MODULE (0U)
/* Allow systick to be a wakeup source in VLLS. */
#define SYSTICK_LLWU_WAKEUP (false)

/* Enable GPIO PAD low power operation.
 * NOTE: ONLY ON EVK BOARD CAN THIS OPTION SET TO 1.
 * Rational: i.MX7ULP supports voltage detection circuitry on pads which results in additional power consumption.
 *           We can disable this feature to save power, but it may cause malfunction or even SoC pad damage.
 *           Please read "GPIO pads operating range configuration" in Reference Manual SIM module carefully
 *           before turn APP_ENABLE_GPIO_PAD_LOW_POWER to "1". If there's any change on board, the pad range setting
 *           code need to be modified accordingly.
 */
#define APP_ENABLE_GPIO_PAD_LOW_POWER (1)

#define APP_LPTMR1_IRQ_PRIO (5U)

#define LLWU_WAKEUP_PIN_IDX    (1U) /* LLWU_P1 used for VOL+ button */
#define LLWU_WAKEUP_PIN_TYPE   kLLWU_ExternalPinFallingEdge
#define APP_WAKEUP_BUTTON_NAME "VOL+"

typedef enum _app_wakeup_source
{
    kAPP_WakeupSourceLptmr, /*!< Wakeup by LPTMR.        */
    kAPP_WakeupSourcePin    /*!< Wakeup by external pin. */
} app_wakeup_source_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
/* Set the clock configuration for HSRUN mode. */
void APP_SetClockHsrun(void);
/* Set the clock configuration for RUN mode from HSRUN mode. */
void APP_SetClockRunFromHsrun(void);
/* Set the clock configuration for RUN mode from VLPR mode. */
void APP_SetClockRunFromVlpr(void);
/* Set the clock configuration for VLPR mode. */
void APP_SetClockVlpr(void);
extern void APP_InitPMC0(void);
extern void APP_PowerPreSwitchHook(smc_power_state_t originPowerState, lpm_power_mode_t targetMode);
extern void APP_PowerPostSwitchHook(smc_power_state_t originPowerState, lpm_power_mode_t targetMode, bool result);
extern void APP_UpdateSimDgo(uint32_t gpIdx, uint32_t mask, uint32_t value);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint8_t s_wakeupTimeout;            /* Wakeup timeout. (Unit: Second) */
static app_wakeup_source_t s_wakeupSource; /* Wakeup source.                 */
static SemaphoreHandle_t s_wakeupSig;
static const char *s_modeNames[] = {"RUN", "WAIT", "STOP", "VLPR", "VLPW", "VLPS", "HSRUN", "LLS", "VLLS"};
bool disableWirelessPinsInVLLS;
bool disableJtagPinsInVLLS;

/*******************************************************************************
 * Function Code
 ******************************************************************************/
extern const scg_sys_clk_config_t g_sysClkConfigVlpr;
extern const scg_sys_clk_config_t g_sysClkConfigHsrun;
extern bool disableWirelessPinsInVLLS;
extern bool disableJtagPinsInVLLS;
extern void LLWU0_IRQHandler(void);
extern bool BOARD_IsRunOnQSPI(void);

static uint32_t iomuxBackup[32 + 20]; /* Backup 32 PTA and 20 PTB IOMUX registers */

static const pmc0_hsrun_mode_config_t s_pmc0HsrunModeConfig = {
    .coreRegulatorVoltLevel = 33U, /* 0.596 + 33 * 0.01083 = 0.95339 */
    .enableForwardBias      = 1U};

static const pmc0_run_mode_config_t s_pmc0RunModeConfig = {.coreRegulatorVoltLevel =
                                                               28U}; /* 0.596 + 28 * 0.01083 = 0.89924 */

static const pmc0_vlpr_mode_config_t s_pmc0VlprModeConfig = {
    .arrayRegulatorSelect   = 0U,
    .coreRegulatorSelect    = 0U,
    .lvdMonitorSelect       = 0U,
    .hvdMonitorSelect       = 0U,
    .enableForceHpBandgap   = 0U,
    .coreRegulatorVoltLevel = 24U, /* 0.596 + 24 * 0.01083 = 0.85592 */
    .enableReverseBackBias  = 1U};

static const pmc0_stop_mode_config_t s_pmc0StopModeConfig = {.coreRegulatorVoltLevel =
                                                                 28U}; /* 0.596 + 28 * 0.01083 = 0.89924 */

static const pmc0_vlps_mode_config_t s_pmc0VlpsModeConfig = {
    .arrayRegulatorSelect   = 0U,
    .coreRegulatorSelect    = 0U,
    .lvdMonitorSelect       = 0U,
    .hvdMonitorSelect       = 0U,
    .enableForceHpBandgap   = 0U,
    .coreRegulatorVoltLevel = 28U, /* 0.596 + 28 * 0.01083 = 0.89924 */
    .enableReverseBackBias  = 1U};

static const pmc0_lls_mode_config_t s_pmc0LlsModeConfig = {
    .arrayRegulatorSelect   = 0U,
    .coreRegulatorSelect    = 0U,
    .lvdMonitorSelect       = 0U,
    .hvdMonitorSelect       = 0U,
    .enableForceHpBandgap   = 0U,
    .coreRegulatorVoltLevel = 13U, /* 0.596 + 13 * 0.01083 = 0.73679 */
    .enableReverseBackBias  = 1U};

static const pmc0_vlls_mode_config_t s_pmc0VllsModeConfig = {
    .arrayRegulatorSelect = 2U, .lvdMonitorSelect = 0U, .hvdMonitorSelect = 0U, .enableForceHpBandgap = 0U};

static void APP_InitDebugConsole(void)
{
    CLOCK_SetIpSrc(BOARD_DEBUG_UART_PCC_ADDRESS, APP_DEBUG_UART_DEFAULT_CLKSRC);
    uint32_t uartClkSrcFreq = CLOCK_GetIpFreq(BOARD_DEBUG_UART_PCC_ADDRESS);
    DbgConsole_Init(BOARD_DEBUG_UART_INSTANCE, APP_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, uartClkSrcFreq);
}

void APP_InitPMC0(void)
{
    pmc0_bias_config_t bcfg = {.RBBNWellVoltageLevelSelect = 5,
                               .RBBPWellVoltageLevelSelect = 5,
                               .DisableRBBPullDown         = 0,
                               .FBBNWellVoltageLevelSelect = 5,
                               .FBBPWellVoltageLevelSelect = 5};

    /* Initialize PMC0 */
    PMC0_SetBiasConfig(&bcfg);
    PMC0_ConfigureHsrunMode(&s_pmc0HsrunModeConfig);
    PMC0_ConfigureRunMode(&s_pmc0RunModeConfig);
    PMC0_ConfigureVlprMode(&s_pmc0VlprModeConfig);
    PMC0_ConfigureStopMode(&s_pmc0StopModeConfig);
    PMC0_ConfigureVlpsMode(&s_pmc0VlpsModeConfig);
    PMC0_ConfigureLlsMode(&s_pmc0LlsModeConfig);
    PMC0_ConfigureVllsMode(&s_pmc0VllsModeConfig);
}

static void BOARD_InitClock(void)
{
    CLOCK_EnableClock(kCLOCK_PctlA);
    CLOCK_EnableClock(kCLOCK_PctlB);
    CLOCK_EnableClock(kCLOCK_Rgpio2p0);

    BOARD_BootClockRUN();
    CLOCK_SetVlprModeSysClkConfig(&g_sysClkConfigVlpr);
    CLOCK_SetHsrunModeSysClkConfig(&g_sysClkConfigHsrun);

    CLOCK_SetIpSrc(kCLOCK_Lpi2c3, kCLOCK_IpSrcSircAsync);
    CLOCK_SetIpSrc(kCLOCK_Lpi2c0, kCLOCK_IpSrcSystem);

    /* Use AUXPLL main clock source */
    CLOCK_SetIpSrcDiv(kCLOCK_Sai0, kCLOCK_IpSrcRtcAuxPllAsync, 0, 0);
    /* SAI clock is needed by Codec initialization. */
    CLOCK_EnableClock(kCLOCK_Sai0);

#ifdef ENABLE_RAM_VECTOR_TABLE
    /* Use RAM vector if needed. Any irq installation can trigger the RAM vector table copy and VTOR update. */
    InstallIRQHandler(LLWU0_IRQn, (uint32_t)LLWU0_IRQHandler);
#endif
}

static void BOARD_InitClockAndPins(void)
{
    BOARD_InitClock();
    BOARD_InitPins();
    APP_SRTM_I2C_ReleaseBus();
    BOARD_I2C_ConfigurePins();
}


static void APP_Suspend(void)
{
    uint32_t i;

    /* Backup PTA/PTB IOMUXC registers */
    for (i = kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTA0; i <= kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTB19; i++)
    {
        iomuxBackup[i] = IOMUXC0->SW_MUX_CTL_PAD[i];
    }

    /* Set PTA0 - PTA2 pads to analog. */
    for (i = kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTA0; i <= kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTA2; i++)
    {
        IOMUXC0->SW_MUX_CTL_PAD[i] = 0;
    }

    /* PTA3 */
    if ((LLWU->PE1 & LLWU_PE1_WUPE1_MASK) == 0)
    {
        /* VOL+ button is not a wakeup source, can be disabled */
        IOMUXC0->SW_MUX_CTL_PAD[kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTA3] = 0;
    }
    else
    {
        /* It's wakeup source, need to set as GPIO */
        IOMUXC0->SW_MUX_CTL_PAD[kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTA3] = IOMUXC0_SW_MUX_CTL_PAD_MUX_MODE(1);
    }

    /* Set PTA4 - PTA13 pads to analog. */
    for (i = kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTA4; i <= kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTA13; i++)
    {
        IOMUXC0->SW_MUX_CTL_PAD[i] = 0;
    }

    if (disableWirelessPinsInVLLS)
    {
        IOMUXC0->SW_MUX_CTL_PAD[kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTA14] = 0;
        IOMUXC0->SW_MUX_CTL_PAD[kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTA15] = 0;
    }
    else
    {
        /* PTA14 - PTA15 keep unchanged to avoid wireless driver failure in Linux resume. */
    }

    /* Set PTA16 - PTA25 pads to analog. */
    for (i = kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTA16; i <= kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTA25; i++)
    {
        IOMUXC0->SW_MUX_CTL_PAD[i] = 0;
    }

    if (disableJtagPinsInVLLS)
    {
        for (i = kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTA26; i <= kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTA30; i++)
        {
            IOMUXC0->SW_MUX_CTL_PAD[i] = 0;
        }
    }
    else
    {
        /* PTA26 - PTA30 keep unchanged as JTAG pads. */
    }

    /* PTA31 */
    if ((LLWU->PE1 & LLWU_PE1_WUPE7_MASK) == 0)
    {
        /* WL_HOST_WAKE is not a wakeup source, can be disabled */
        IOMUXC0->SW_MUX_CTL_PAD[kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTA31] = 0;
    }

    /* Set PTB0 - PTB6 pads to analog */
    for (i = kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTB0; i <= kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTB6; i++)
    {
        IOMUXC0->SW_MUX_CTL_PAD[i] = 0;
    }

    /* PTB7 */
    if ((LLWU->PE1 & LLWU_PE1_WUPE11_MASK) == 0)
    {
        /* BT_HOST_WAKE is not a wakeup source, can be disabled */
        IOMUXC0->SW_MUX_CTL_PAD[kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTB7] = 0;
    }

    /* PTB8 keep unchanged as QSPI pad in XIP. Otherwise set it to analog. */
    if (!BOARD_IsRunOnQSPI())
    {
        IOMUXC0->SW_MUX_CTL_PAD[kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTB8] = 0;
    }

    /* Set PTB9 - PTB14 pads to analog */
    for (i = kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTB9; i <= kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTB14; i++)
    {
        IOMUXC0->SW_MUX_CTL_PAD[i] = 0;
    }

    /* PTB15 - PTB19 keep unchanged as QSPI pads in XIP. Otherwise set them to analog. */
    if (!BOARD_IsRunOnQSPI())
    {
        for (i = kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTB15; i <= kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTB19; i++)
        {
            IOMUXC0->SW_MUX_CTL_PAD[i] = 0;
        }
    }

    /* Save SRTM context */
    APP_SRTM_Suspend();
}

static void APP_Resume(bool resume)
{
    uint32_t i;

    /* Restore PTA/PTB IOMUXC registers */
    for (i = kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTA0; i <= kIOMUXC0_IOMUXC0_SW_MUX_CTL_PAD_PTB19; i++)
    {
        IOMUXC0->SW_MUX_CTL_PAD[i] = iomuxBackup[i];
    }

    if (resume)
    {
        /* Only init clock. Pins are recovered at the beginning of resume (LPM_Resume). */
        BOARD_InitClock();
        /* Need to recover LPTMR clock, though LPTMR itself can keep its state in VLLS. */
        CLOCK_EnableClock(kCLOCK_Lptmr1); /* Wakeup timer */
    }

    APP_SRTM_Resume(resume);
}

void APP_PowerPreSwitchHook(smc_power_state_t originPowerState, lpm_power_mode_t targetMode)
{
    if ((LPM_PowerModeRun != targetMode) && (LPM_PowerModeHsrun != targetMode) && (LPM_PowerModeVlpr != targetMode))
    {
        /* Wait for debug console output finished. */
        while (!(kLPUART_TransmissionCompleteFlag & LPUART_GetStatusFlags((LPUART_Type *)BOARD_DEBUG_UART_BASEADDR)))
        {
        }
        DbgConsole_Deinit();
        /*
         * Set pin for current leakage.
         * Debug console RX pin: Set to pinmux to analog.
         * Debug console TX pin: Set to pinmux to analog.
         */
        IOMUXC_SetPinMux(IOMUXC_PTA19_CMP1_IN3_3V, 0);
        IOMUXC_SetPinConfig(IOMUXC_PTA19_CMP1_IN3_3V, 0);
        IOMUXC_SetPinMux(IOMUXC_PTA18_CMP1_IN1_3V, 0);
        IOMUXC_SetPinConfig(IOMUXC_PTA18_CMP1_IN1_3V, 0);

        if (LPM_PowerModeVlls == targetMode)
        {
            APP_Suspend();
        }
    }
}

void APP_PowerPostSwitchHook(smc_power_state_t originPowerState, lpm_power_mode_t targetMode, bool result)
{
    if ((LPM_PowerModeRun != targetMode) && (LPM_PowerModeHsrun != targetMode) && (LPM_PowerModeVlpr != targetMode))
    {
        if (LPM_PowerModeVlls == targetMode)
        {
            APP_Resume(result);
        }
        /*
         * Debug console RX pin was set to disable for current leakage, need to re-configure pinmux.
         * Debug console TX pin was set to disable for current leakage, need to re-configure pinmux.
         */
        IOMUXC_SetPinMux(IOMUXC_PTA18_LPUART0_TX, 0U);
        IOMUXC_SetPinConfig(IOMUXC_PTA18_LPUART0_TX, IOMUXC0_SW_MUX_CTL_PAD_PE_MASK | IOMUXC0_SW_MUX_CTL_PAD_PS_MASK);
        IOMUXC_SetPinMux(IOMUXC_PTA19_LPUART0_RX, 0U);
        IOMUXC_SetPinConfig(IOMUXC_PTA19_LPUART0_RX, IOMUXC0_SW_MUX_CTL_PAD_PE_MASK | IOMUXC0_SW_MUX_CTL_PAD_PS_MASK);
        APP_InitDebugConsole();
    }
    PRINTF("== Power switch %s ==\r\n", result ? "OK" : "FAIL");
}

/* LLWU interrupt handler. */
void APP_LLWU0_IRQHandler(void)
{
    bool wakeup = false;

    if (LLWU_GetInternalWakeupModuleFlag(LLWU, LLWU_MODULE_LPTMR1))
    {
        /* Woken up by LPTMR, then clear LPTMR flag. */
        LPTMR_ClearStatusFlags(LPTMR1, kLPTMR_TimerCompareFlag);
        LPTMR_DisableInterrupts(LPTMR1, kLPTMR_TimerInterruptEnable);
        LPTMR_StopTimer(LPTMR1);
        wakeup = true;
    }

    if (LLWU_GetExternalWakeupPinFlag(LLWU, LLWU_WAKEUP_PIN_IDX))
    {
        /* Woken up by external pin. */
        LLWU_ClearExternalWakeupPinFlag(LLWU, LLWU_WAKEUP_PIN_IDX);
        wakeup = true;
    }

    if (LLWU_GetInternalWakeupModuleFlag(LLWU, SYSTICK_LLWU_MODULE))
    {
        /* Woken up by Systick LPTMR, then clear LPTMR flag. */
        LPTMR_ClearStatusFlags(SYSTICK_BASE, kLPTMR_TimerCompareFlag);
    }

    if (LLWU_GetInternalWakeupModuleFlag(LLWU, LLWU_MODULE_USBPHY))
    {
        /* Woken up by USB PHY, then need to wakeup. */
        wakeup = true;
    }

    if (wakeup)
    {
        xSemaphoreGiveFromISR(s_wakeupSig, NULL);
        portYIELD_FROM_ISR(pdTRUE);
    }
}

/* LPTMR1 interrupt handler. */
void LPTMR1_IRQHandler(void)
{
    bool wakeup = false;

    if (kLPTMR_TimerInterruptEnable & LPTMR_GetEnabledInterrupts(LPTMR1))
    {
        LPTMR_ClearStatusFlags(LPTMR1, kLPTMR_TimerCompareFlag);
        LPTMR_DisableInterrupts(LPTMR1, kLPTMR_TimerInterruptEnable);
        LPTMR_StopTimer(LPTMR1);
        wakeup = true;
    }

    if (wakeup)
    {
        xSemaphoreGiveFromISR(s_wakeupSig, NULL);
        portYIELD_FROM_ISR(pdTRUE);
    }

    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
    exception return operation might vector to incorrect interrupt */
    __DSB();
}

static void APP_IRQDispatcher(IRQn_Type irq, void *param)
{
    switch (irq)
    {
        case LLWU0_IRQn:
            APP_LLWU0_IRQHandler();
            break;
        case SNVS_IRQn:
            break;
        case PCTLA_IRQn:
            if ((1U << APP_PIN_IDX(APP_PIN_VOL_PLUS)) & PORT_GetPinsInterruptFlags(PORTA))
            {
                /* Flag will be cleared by app_srtm.c */
                xSemaphoreGiveFromISR(s_wakeupSig, NULL);
                portYIELD_FROM_ISR(pdTRUE);
            }
            break;
        case PCTLB_IRQn:
            break;
        default:
            break;
    }
}

static void APP_ShowPowerMode(smc_power_state_t powerMode)
{
    switch (powerMode)
    {
        case kSMC_PowerStateRun:
            PRINTF("    Power mode: RUN\r\n");
            break;
        case kSMC_PowerStateVlpr:
            PRINTF("    Power mode: VLPR\r\n");
            break;
        case kSMC_PowerStateHsrun:
            PRINTF("    Power mode: HSRUN\r\n");
            break;
        default:
            PRINTF("    Power mode wrong\r\n");
            break;
    }
}

/* Get input from user about wakeup timeout. */
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

/* Get wakeup timeout and wakeup source. */
static void APP_GetWakeupConfig(void)
{
    /* Get wakeup source by user input. */
    s_wakeupSource = APP_GetWakeupSource();

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

static void APP_SetWakeupConfig(lpm_power_mode_t targetMode)
{
    if (kAPP_WakeupSourceLptmr == s_wakeupSource)
    {
        LPTMR_SetTimerPeriod(LPTMR1, (1000U * s_wakeupTimeout));
        LPTMR_StartTimer(LPTMR1);
        LPTMR_EnableInterrupts(LPTMR1, kLPTMR_TimerInterruptEnable);
    }

    /* To avoid conflicting access of LLWU with SRTM dispatcher, we put the LLWU setting into SRTM dispatcher context.*/
    /* If targetMode is VLLS/LLS, setup LLWU. */
    if ((LPM_PowerModeLls == targetMode) || (LPM_PowerModeVlls == targetMode))
    {
        if (kAPP_WakeupSourceLptmr == s_wakeupSource)
        {
            /* Set LLWU LPTMR1 module wakeup source. */
            APP_SRTM_SetWakeupModule(LLWU_MODULE_LPTMR1, true);
        }
        else
        {
            /* Set PORT and LLWU wakeup pin. */
            APP_SRTM_SetWakeupPin(APP_PIN_VOL_PLUS, (uint16_t)LLWU_WAKEUP_PIN_TYPE | 0x100);
        }
    }
    else
    {
        /* Set PORT pin. */
        if (kAPP_WakeupSourcePin == s_wakeupSource)
        {
            APP_SRTM_SetWakeupPin(APP_PIN_VOL_PLUS, (uint16_t)LLWU_WAKEUP_PIN_TYPE);
        }
    }
}

static void APP_ClearWakeupConfig(lpm_power_mode_t targetMode)
{
    if (kAPP_WakeupSourcePin == s_wakeupSource)
    {
        APP_SRTM_SetWakeupPin(APP_PIN_VOL_PLUS, (uint16_t)kLLWU_ExternalPinDisable);
    }
    else if ((LPM_PowerModeLls == targetMode) || (LPM_PowerModeVlls == targetMode))
    {
        APP_SRTM_SetWakeupModule(LLWU_MODULE_LPTMR1, false);
    }
}

static status_t APP_PowerModeSwitch(smc_power_state_t curPowerState, lpm_power_mode_t targetPowerMode)
{
    status_t status = kStatus_Success;

    switch (targetPowerMode)
    {
        case LPM_PowerModeVlpr:
            APP_SetClockVlpr();
            status = SMC_SetPowerModeVlpr(MSMC0);
            while (kSMC_PowerStateVlpr != SMC_GetPowerModeState(MSMC0))
            {
            }
            break;

        case LPM_PowerModeRun:
            /* If enter RUN from HSRUN, fisrt change clock. */
            if (kSMC_PowerStateHsrun == curPowerState)
            {
                APP_SetClockRunFromHsrun();
            }

            /* Power mode change. */
            status = SMC_SetPowerModeRun(MSMC0);
            while (kSMC_PowerStateRun != SMC_GetPowerModeState(MSMC0))
            {
            }

            /* If enter RUN from VLPR, change clock after the power mode change. */
            if (kSMC_PowerStateVlpr == curPowerState)
            {
                APP_SetClockRunFromVlpr();
            }
            break;

        case LPM_PowerModeHsrun:
            status = SMC_SetPowerModeHsrun(MSMC0);
            while (kSMC_PowerStateHsrun != SMC_GetPowerModeState(MSMC0))
            {
            }

            APP_SetClockHsrun(); /* Change clock setting after power mode change. */
            break;

        default:
            PRINTF("Wrong value\r\n");
            break;
    }

    if (status != kStatus_Success)
    {
        PRINTF("!!!! Power switch failed !!!!!\r\n");
    }

    return status;
}

static uint32_t APP_GetInputNumWithEcho(uint32_t length, bool allowZero)
{
    uint8_t ch;
    uint8_t digBuffer[8U] = {0U};
    uint8_t i, j;
    uint8_t digCount = 0U;
    uint32_t temp1, temp2 = 0U;
    uint32_t result  = 0U;
    bool getFirstDig = false;

    assert(length <= (sizeof(digBuffer) / sizeof(digBuffer[0U])));

    /* Get user input and echo it back to terminal. */
    for (;;)
    {
        ch = GETCHAR();

        if (('a' <= ch) && ('f' >= ch))
        {
            ch = ch - ('a' - 'A');
        }

        if (((('0' <= ch) && ('9' >= ch)) || (('A' <= ch) && ('F' >= ch))) && (digCount < length))
        {
            if (false == getFirstDig)
            {
                if (allowZero || ('0' < ch))
                {
                    getFirstDig = true;
                }
                else
                {
                    continue;
                }
            }

            PUTCHAR(ch);
            digBuffer[digCount] = ch;
            digCount++;
        }
        else if ((0x7FU == ch) && (0U != digCount))
        {
            digBuffer[digCount] = 0x0U;
            digCount--;
            PUTCHAR(0x7FU);
        }
        else
        {
            if ('\r' == ch)
            {
                break;
            }
        }
    }

    /* Reconstruct user input number. */
    temp1 = digCount - 1;
    for (i = 0; i < digCount; i++)
    {
        if (('0' <= digBuffer[i]) && ('9' >= digBuffer[i]))
        {
            temp2 = digBuffer[i] - '0';
        }
        else if (('A' <= digBuffer[i]) && ('F' >= digBuffer[i]))
        {
            temp2 = digBuffer[i] - 'A' + 10U;
        }
        else
        {
        }

        for (j = 0U; j < temp1; j++)
        {
            temp2 *= 16U;
        }
        temp1--;
        result += temp2;
    }

    return result;
}

static void APP_ReadPmicRegister(void)
{
    uint32_t reg;
    uint32_t value;

    for (;;)
    {
        /* Scroll to a new page. */
        PRINTF("Please select the register address to dump(0~FF):");
        reg = APP_GetInputNumWithEcho(2U, true);
        if (0xFFU >= reg)
        {
            break;
        }
    }
    PRINTF("\r\n");

    /* Read register from PF1550 in SRTM dispatcher context */
    value = APP_SRTM_GetPmicReg(reg);

    PRINTF("\r\nDump Format: [Register Address] = Register Content:\r\n");
    PRINTF("[0x%x] = 0x%x\r\n", reg, value);
    PRINTF("\r\n");
    PRINTF("Press Any Key to Home Page...");
    GETCHAR();
}

static void APP_SetPmicRegister(void)
{
    uint32_t reg;
    uint32_t value;
    uint32_t temp;

    for (;;)
    {
        /* Scroll to a new page. */
        PRINTF("Please input the register address to set(0~FF):");
        reg = APP_GetInputNumWithEcho(2U, true);
        if (255U >= reg)
        {
            break;
        }
    }
    PRINTF("\r\n");

    for (;;)
    {
        /* Scroll to a new page. */
        PRINTF("Please input the register content to set(0~FF):");
        value = APP_GetInputNumWithEcho(2U, true);
        if (0xFFU >= value)
        {
            break;
        }
    }
    PRINTF("\r\n");

    /* Set register content to PF1550 in SRTM dispatcher context */
    APP_SRTM_SetPmicReg(reg, value);

    /* Read register from PF1550 in SRTM dispatcher context */
    temp = APP_SRTM_GetPmicReg(reg);

    PRINTF("\r\nDump Format: [Register Address] = Register Content:\r\n");
    PRINTF("[0x%x] = 0x%x\r\n", reg, temp);
    PRINTF("\r\n");
    PRINTF("Press Any Key to Home Page...");
    GETCHAR();
}

static void APP_PowerTestMode(void)
{
    uint32_t cmd;
    uint32_t value;

    for (;;)
    {
        PRINTF("[Power Test Mode]\r\n");
        PRINTF("Commands:\r\n");
        /* Should not disable SW3, just for trial */
        PRINTF("0: Toggle PMIC SW3 regulator");
        PRINTF(" - Just for trial. Should not do it on EVK board.\r\n");
        /* Should not disable DDR_SW_EN, just for trial */
        PRINTF("1: Toggle DDR_SW_EN load switch");
        PRINTF(" - Just for trial. Should not do it on EVK board.\r\n");
        /* Have impact on Linux WIFI feature resume, but can reduce leakage. */
        PRINTF("2: %s WL_REG_ON/BT_REG_ON pins in VLLS\r\n", disableWirelessPinsInVLLS ? "Enable" : "Disable");
        PRINTF("   - Disable these pins leads to failure in Linux WIFI driver resume.\r\n");
        PRINTF("   - Just for showing minimum board leakage. \r\n");
        /* Have impact on debug feature during suspend/resume. */
        PRINTF("3: %s JTAG pins in VLLS\r\n", disableJtagPinsInVLLS ? "Enable" : "Disable");
        /* May have risk in M4 running during PMIC standby mode. */
        PRINTF("4: Toggle PMIC LDO3 low power mode in VLLS.\r\n");
        PRINTF("Selection:");
        cmd = APP_GetInputNumWithEcho(1U, true);
        if (4 >= cmd)
        {
            break;
        }
    }
    PRINTF("\r\n");

    if (cmd == 0)
    {
        APP_SRTM_ToggleSW3();
    }
    else if (cmd == 1)
    {
        GPIO_PortToggle(GPIOB, 1U << 6);
    }
    else if (cmd == 2)
    {
        disableWirelessPinsInVLLS = !disableWirelessPinsInVLLS;
    }
    else if (cmd == 3)
    {
        disableJtagPinsInVLLS = !disableJtagPinsInVLLS;
    }
    else
    {
        value = APP_SRTM_GetPmicReg(0x53U);
        if ((value & 0x8U) == 0)
        {
            value |= 0x8U; /* Enable low power mode */
        }
        else
        {
            value &= ~0x8U; /* Disable low power mode */
        }
        APP_SRTM_SetPmicReg(0x53U, value);
    }

    PRINTF("Press Any Key to Home Page...");
    GETCHAR();
}

void APP_SetPowerMode(smc_power_state_t powerMode)
{
    switch (powerMode)
    {
        case kSMC_PowerStateRun:
            LPM_SetPowerMode(LPM_PowerModeRun);
            break;
        case kSMC_PowerStateVlpr:
            LPM_SetPowerMode(LPM_PowerModeVlpr);
            break;
        case kSMC_PowerStateHsrun:
            LPM_SetPowerMode(LPM_PowerModeHsrun);
            break;
        default:
            break;
    }
}

/* Power Mode Switch task */
void PowerModeSwitchTask(void *pvParameters)
{
    status_t status;
    lptmr_config_t lptmrConfig;
    smc_power_state_t curPowerState;
    mu_power_mode_t powerMode;
    lpm_power_mode_t targetPowerMode;
    uint32_t resetSrc;
    uint32_t freq = 0U;
    uint8_t ch;
    const char *errorMsg;

    /* As IRQ handler main entry locates in app_srtm.c to support services, here need an entry to handle application
     * IRQ events.
     */
    APP_SRTM_SetIRQHandler(APP_IRQDispatcher, NULL);
    /* Add Systick as LLS/VLLS wakeup source, depending on SYSTICK_LLWU_WAKEUP value. */
    APP_SRTM_SetWakeupModule(SYSTICK_LLWU_MODULE, SYSTICK_LLWU_WAKEUP);

    /* Setup LPTMR. */
    LPTMR_GetDefaultConfig(&lptmrConfig);
    lptmrConfig.prescalerClockSource = kLPTMR_PrescalerClock_1; /* Use LPO 1KHz as clock source. */
    lptmrConfig.bypassPrescaler      = true;
    LPTMR_Init(LPTMR1, &lptmrConfig);
    NVIC_SetPriority(LPTMR1_IRQn, APP_LPTMR1_IRQ_PRIO);

    EnableIRQ(LPTMR1_IRQn);

    resetSrc = SMC_GetPreviousResetSources(MSMC0);
    PRINTF("\r\nMCU wakeup source 0x%x...\r\n", resetSrc);

    APP_SRTM_BootCA7();

    for (;;)
    {
        freq = CLOCK_GetFreq(kCLOCK_CoreSysClk);
        PRINTF("\r\n####################  Power Mode Switch Task ####################\n\r\n");
        PRINTF("    Build Time: %s--%s \r\n", __DATE__, __TIME__);
        PRINTF("    Core Clock: %dHz \r\n", freq);
        curPowerState = SMC_GetPowerModeState(MSMC0);
        APP_ShowPowerMode(curPowerState);
        PRINTF("\r\nSelect the desired operation \n\r\n");
        PRINTF("Press  %c for enter: RUN      - Normal RUN mode\r\n", kAPP_PowerModeRun);
        PRINTF("Press  %c for enter: WAIT     - Wait mode\r\n", kAPP_PowerModeWait);
        PRINTF("Press  %c for enter: STOP     - Stop mode\r\n", kAPP_PowerModeStop);
        PRINTF("Press  %c for enter: VLPR     - Very Low Power Run mode\r\n", kAPP_PowerModeVlpr);
        PRINTF("Press  %c for enter: VLPW     - Very Low Power Wait mode\r\n", kAPP_PowerModeVlpw);
        PRINTF("Press  %c for enter: VLPS     - Very Low Power Stop mode\r\n", kAPP_PowerModeVlps);
        PRINTF("Press  %c for enter: HSRUN    - High Speed RUN mode\r\n", kAPP_PowerModeHsrun);
        PRINTF("Press  %c for enter: LLS      - Low Leakage Stop mode\r\n", kAPP_PowerModeLls);
        PRINTF("Press  %c for enter: VLLS     - Very Low Leakage Stop mode\r\n", kAPP_PowerModeVlls);
        PRINTF("Press  Q for query CA7 core power status.\r\n");
        PRINTF("Press  W for wake up CA7 core in VLLS/VLPS.\r\n");
        PRINTF("Press  T for reboot CA7 core.\r\n");
        PRINTF("Press  U for shutdown CA7 core.\r\n");
        PRINTF("Press  V for boot CA7 core.\r\n");
        PRINTF("Press  R for read PF1550 Register.\r\n");
        PRINTF("Press  S for set PF1550 Register.\r\n");
        PRINTF("Press  Z for enhanced power configuration.\r\n");
        PRINTF("\r\nWaiting for power mode select..\r\n\r\n");

        /* Wait for user response */
        ch = GETCHAR();
        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }

        targetPowerMode = (lpm_power_mode_t)(ch - 'A');

        if (targetPowerMode <= LPM_PowerModeVlls)
        {
            if (!LPM_IsTargetModeValid(targetPowerMode, &errorMsg))
            {
                assert(errorMsg);
                PRINTF(errorMsg);
            }
            else if (!LPM_SetPowerMode(targetPowerMode))
            {
                PRINTF("Some task doesn't allow to enter mode %s\r\n", s_modeNames[targetPowerMode]);
            }
            else if ((LPM_PowerModeRun == targetPowerMode) || (LPM_PowerModeHsrun == targetPowerMode) ||
                     (LPM_PowerModeVlpr == targetPowerMode))
            {
                /* If target mode is RUN/VLPR/HSRUN, switch directly. */
                APP_PowerPreSwitchHook(curPowerState, targetPowerMode);
                status = APP_PowerModeSwitch(curPowerState, targetPowerMode);
                APP_PowerPostSwitchHook(curPowerState, targetPowerMode, status == kStatus_Success);
            }
            else /* Idle task will handle the low power state. */
            {
                APP_GetWakeupConfig();
                APP_SetWakeupConfig(targetPowerMode);
                xSemaphoreTake(s_wakeupSig, portMAX_DELAY);
                if (targetPowerMode == LPM_PowerModeVlls)
                {
                    /* PMC0 need to be reconfigured after VLLS. */
                    APP_InitPMC0();
                }
                /* Need to reset power mode to avoid unintentional WFI. */
                curPowerState = SMC_GetPowerModeState(MSMC0);
                APP_SetPowerMode(curPowerState);
                /* The call might be blocked by SRTM dispatcher task. Must be called after power mode reset. */
                APP_ClearWakeupConfig(targetPowerMode);
            }
        }
        else if ('Q' == ch)
        {
            powerMode = MU_GetOtherCorePowerMode(MUA);
            switch (powerMode)
            {
                case kMU_PowerModeRun:
                    PRINTF("CA7 power mode RUN!\r\n");
                    break;

                case kMU_PowerModeWait:
                    PRINTF("CA7 power mode WAIT!\r\n");
                    break;

                case kMU_PowerModeStop:
                    PRINTF("CA7 power mode STOP/VLPS!\r\n");
                    break;

                case kMU_PowerModeDsm:
                    PRINTF("CA7 power mode LLS/VLLS!\r\n");
                    break;

                default:
                    PRINTF("Wrong power mode value %d!\r\n", (int32_t)powerMode);
                    break;
            }
        }
        else if ('W' == ch)
        {
            APP_SRTM_WakeupCA7();
        }
        else if ('T' == ch)
        {
            APP_SRTM_RebootCA7();
        }
        else if ('U' == ch)
        {
            APP_SRTM_ShutdownCA7();
        }
        else if ('V' == ch)
        {
            APP_SRTM_BootCA7();
        }
        else if ('R' == ch)
        {
            APP_ReadPmicRegister();
        }
        else if ('S' == ch)
        {
            APP_SetPmicRegister();
        }
        else if ('Z' == ch)
        {
            APP_PowerTestMode();
        }
        else
        {
            PRINTF("Invalid command %c[0x%x]\r\n", ch, ch);
        }

        PRINTF("\r\nNext loop\r\n");
    }
}

void vApplicationMallocFailedHook(void)
{
    PRINTF("Malloc Failed!!!\r\n");
}

void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
    uint32_t irqMask;
    smc_power_state_t curPowerState;
    lpm_power_mode_t targetPowerMode;
    bool result;

    irqMask = DisableGlobalIRQ();

    /* Only when no context switch is pending and no task is waiting for the scheduler
     * to be unsuspended then enter low power entry.
     */
    if (eTaskConfirmSleepModeStatus() != eAbortSleep)
    {
        targetPowerMode = LPM_GetPowerMode();
        if (targetPowerMode != LPM_PowerModeRun && targetPowerMode != LPM_PowerModeVlpr &&
            targetPowerMode != LPM_PowerModeHsrun)
        {
            /* Only wait when target power mode is not running */
            curPowerState = SMC_GetPowerModeState(MSMC0);
            APP_PowerPreSwitchHook(curPowerState, targetPowerMode);
            result = LPM_WaitForInterrupt((uint64_t)1000 * xExpectedIdleTime / configTICK_RATE_HZ);
            APP_PowerPostSwitchHook(curPowerState, targetPowerMode, result);
        }
    }

    EnableGlobalIRQ(irqMask);
}

/* Called in PowerModeSwitchTask */
static bool APP_LpmListener(lpm_power_mode_t curMode, lpm_power_mode_t newMode, void *data)
{
    PRINTF("WorkingTask %d: Transfer from %s to %s\r\n", (uint32_t)data, s_modeNames[curMode], s_modeNames[newMode]);

    /* Do necessary preparation for this mode change */

    return true; /* allow this switch */
}

/*!
 * @brief simulating working task.
 */
static void WorkingTask(void *pvParameters)
{
    LPM_RegisterPowerListener(APP_LpmListener, pvParameters);

    for (;;)
    {
        /* Use App task logic to replace vTaskDelay */
        PRINTF("Task %d is working now\r\n", (uint32_t)pvParameters);
        vTaskDelay(portMAX_DELAY);
    }
}

/*! @brief Main function */
int main(void)
{
    /* Power related. */
    SMC_SetPowerModeProtection(MSMC0, kSMC_AllowPowerModeAll);

    BOARD_InitClockAndPins();
    APP_InitPMC0();
    APP_InitDebugConsole();

    APP_SRTM_Init();
    LPM_Init();

#if APP_ENABLE_GPIO_PAD_LOW_POWER
    /* NOTE: Please see the definition of APP_ENABLE_GPIO_PAD_LOW_POWER before using the DGO update here. */
    /* Set PTB/PTC/PTE to low range, PTA/PTF to high range to save power. Need to align with board design. */
    APP_UpdateSimDgo(11,
                     SIM_SIM_DGO_GP11_PTA_RANGE_CTRL_MASK | SIM_SIM_DGO_GP11_PTB_RANGE_CTRL_MASK |
                         SIM_SIM_DGO_GP11_PTC_RANGE_CTRL_MASK | SIM_SIM_DGO_GP11_PTE_RANGE_CTRL_MASK |
                         SIM_SIM_DGO_GP11_PTF_RANGE_CTRL_MASK,
                     SIM_SIM_DGO_GP11_PTA_RANGE_CTRL(2) | SIM_SIM_DGO_GP11_PTB_RANGE_CTRL(1) |
                         SIM_SIM_DGO_GP11_PTC_RANGE_CTRL(1) | SIM_SIM_DGO_GP11_PTE_RANGE_CTRL(1) |
                         SIM_SIM_DGO_GP11_PTF_RANGE_CTRL(2));
#endif

    s_wakeupSig = xSemaphoreCreateBinary();

    xTaskCreate(PowerModeSwitchTask, "Main Task", 512U, NULL, tskIDLE_PRIORITY + 1U, NULL);
    xTaskCreate(WorkingTask, "Working Task", configMINIMAL_STACK_SIZE, (void *)1, tskIDLE_PRIORITY + 2U, NULL);

    /* Start FreeRTOS scheduler. */
    vTaskStartScheduler();

    /* Application should never reach this point. */
    for (;;)
    {
    }
}
