/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "fsl_common.h"
#include "power_mode_switch.h"
#include "fsl_debug_console.h"
#include "lpm.h"
#include "fsl_gpt.h"
#include "fsl_lpuart.h"
#include "specific.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "peripherals.h"
#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CPU_NAME "iMXRT1062"

#define APP_WAKEUP_BUTTON_GPIO        BOARD_USER_BUTTON_GPIO
#define APP_WAKEUP_BUTTON_GPIO_PIN    BOARD_USER_BUTTON_GPIO_PIN
#define APP_WAKEUP_BUTTON_IRQ         BOARD_USER_BUTTON_IRQ
#define APP_WAKEUP_BUTTON_IRQ_HANDLER BOARD_USER_BUTTON_IRQ_HANDLER
#define APP_WAKEUP_BUTTON_NAME        BOARD_USER_BUTTON_NAME

#define APP_WAKEUP_GPT_BASE         DEMO_GPT_PERIPHERAL
#define APP_WAKEUP_GPT_IRQn         GPT2_IRQn
#define APP_WAKEUP_GPT_IRQn_HANDLER GPT2_IRQHandler

#define APP_WAKEUP_SNVS_IRQ         SNVS_HP_WRAPPER_IRQn
#define APP_WAKEUP_SNVS_IRQ_HANDLER SNVS_HP_WRAPPER_IRQHandler

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint8_t s_wakeupTimeout;            /* Wakeup timeout. (Unit: Second) */
static app_wakeup_source_t s_wakeupSource; /* Wakeup source.                 */
static lpm_power_mode_t s_targetPowerMode;
static lpm_power_mode_t s_curRunMode = LPM_PowerModeOverRun;
static SemaphoreHandle_t s_wakeupSig;
static const char *s_modeNames[] = {"Over RUN",    "Full Run",       "Low Speed Run", "Low Power Run",
                                    "System Idle", "Low Power Idle", "Suspend",       "SNVS"};

/*******************************************************************************
 * Code
 ******************************************************************************/
void SetLowPowerClockGate(void)
{
    CLOCK_ControlGate(kCLOCK_Aips_tz1, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Aips_tz2, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Mqs, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_FlexSpiExsc, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Sim_M_Main, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Dcp, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Lpuart3, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Can1, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Can1S, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Can2, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Can2S, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Trace, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Gpt2, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Gpt2S, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Lpuart2, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Gpio2, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Lpspi1, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Lpspi2, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Lpspi3, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Lpspi4, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Adc2, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Enet, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Pit, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Aoi2, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Adc1, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_SemcExsc, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Gpt1, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Gpt1S, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Lpuart4, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Gpio1, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Csu, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Gpio5, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_OcramExsc, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Csi, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_IomuxcSnvs, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Lpi2c1, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Lpi2c2, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Lpi2c3, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Ocotp, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Xbar3, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Ipmux1, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Ipmux2, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Ipmux3, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Xbar1, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Xbar2, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Gpio3, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Lcd, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Pxp, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Flexio2, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Lpuart5, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Semc, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Lpuart6, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Aoi1, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_LcdPixel, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Gpio4, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Ewm0, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Wdog1, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_FlexRam, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Acmp1, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Acmp2, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Acmp3, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Acmp4, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Ocram, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_IomuxcSnvsGpr, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Sim_m7_clk_r, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Iomuxc, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_IomuxcGpr, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Bee, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_SimM7, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Tsc, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_SimM, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_SimEms, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Pwm1, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Pwm2, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Pwm3, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Pwm4, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Enc1, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Enc2, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Enc3, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Enc4, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Rom, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Flexio1, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Wdog3, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Dma, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Kpp, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Wdog2, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Aips_tz4, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Spdif, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_SimMain, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Sai1, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Sai2, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Sai3, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Lpuart1, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Lpuart7, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_SnvsHp, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_SnvsLp, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_UsbOh3, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Usdhc1, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Usdhc2, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Dcdc, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Ipmux4, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_FlexSpi, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Trng, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Lpuart8, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Timer4, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Aips_tz3, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_SimPer, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Anadig, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Lpi2c4, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Timer1, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Timer2, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Timer3, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Enet2, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_FlexSpi2, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Axbs_l, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Can3, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Can3S, kCLOCK_ClockNotNeeded);
    CLOCK_ControlGate(kCLOCK_Aips_lite, kCLOCK_ClockNeededRun);
    CLOCK_ControlGate(kCLOCK_Flexio3, kCLOCK_ClockNotNeeded);
}

void PowerDownUSBPHY(void)
{
    USBPHY1->CTRL = 0xFFFFFFFFU;
    USBPHY2->CTRL = 0xFFFFFFFFU;
}

void APP_WAKEUP_GPT_IRQn_HANDLER(void)
{
    GPT_ClearStatusFlags(APP_WAKEUP_GPT_BASE, kGPT_OutputCompare1Flag);
    GPT_StopTimer(APP_WAKEUP_GPT_BASE);
    LPM_DisableWakeupSource(APP_WAKEUP_GPT_IRQn);

    xSemaphoreGiveFromISR(s_wakeupSig, NULL);
    portYIELD_FROM_ISR(pdTRUE);
    SDK_ISR_EXIT_BARRIER;
}

void APP_WAKEUP_BUTTON_IRQ_HANDLER(void)
{
    if ((1U << APP_WAKEUP_BUTTON_GPIO_PIN) & GPIO_GetPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO))
    {
        /* Disable interrupt. */
        GPIO_DisableInterrupts(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
        GPIO_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
        LPM_DisableWakeupSource(APP_WAKEUP_BUTTON_IRQ);
    }

    xSemaphoreGiveFromISR(s_wakeupSig, NULL);
    portYIELD_FROM_ISR(pdTRUE);
    SDK_ISR_EXIT_BARRIER;
}

void APP_WAKEUP_SNVS_IRQ_HANDLER(void)
{
    /* Clear SRTC alarm interrupt. */
    SNVS->LPSR |= SNVS_LPSR_LPTA_MASK;
    SDK_ISR_EXIT_BARRIER;
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
static app_wakeup_source_t APP_GetWakeupSource(lpm_power_mode_t targetMode)
{
    uint8_t ch;

    while (1)
    {
        PRINTF("Select the wake up source:\r\n");
        PRINTF("Press T for Timer\r\n");
        PRINTF("Press S for switch/button %s. \r\n", APP_WAKEUP_BUTTON_NAME);

        PRINTF("\r\nWaiting for key press..\r\n\r\n");

        ch = GETCHAR();

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }

        if (ch == 'T')
        {
            return kAPP_WakeupSourceTimer;
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
static void APP_GetWakeupConfig(lpm_power_mode_t targetMode)
{
#if defined(HAS_WAKEUP_PIN) && (HAS_WAKEUP_PIN == 0)
    /* If no WAKEUP pin available on board, then timer is the only wake up source in SNVS mode. */
    if (targetMode == LPM_PowerModeSNVS)
    {
        s_wakeupSource = kAPP_WakeupSourceTimer;
    }
    else
#endif
    {
        /* Get wakeup source by user input. */
        s_wakeupSource = APP_GetWakeupSource(targetMode);
    }

    if (kAPP_WakeupSourceTimer == s_wakeupSource)
    {
        /* Wakeup source is timer, user should input wakeup timeout value. */
        s_wakeupTimeout = APP_GetWakeupTimeout();
        PRINTF("Will wakeup in %d seconds.\r\n", s_wakeupTimeout);
    }
    else
    {
        PRINTF("Switch %s from off to on to wake up.\r\n", APP_WAKEUP_BUTTON_NAME);
    }
}

static void APP_SetWakeupConfig(lpm_power_mode_t targetMode)
{
    /* Set timer timeout value. */
    if (kAPP_WakeupSourceTimer == s_wakeupSource)
    {
        /* GPT can not work in SNVS mode, so we set SRTC as the wakeup source. */
        if (targetMode == LPM_PowerModeSNVS)
        {
            /* Stop SRTC time counter */
            SNVS->LPCR &= ~SNVS_LPCR_SRTC_ENV_MASK;
            while ((SNVS->LPCR & SNVS_LPCR_SRTC_ENV_MASK))
            {
            }
            /* Disable SRTC alarm interrupt */
            SNVS->LPCR &= ~SNVS_LPCR_LPTA_EN_MASK;
            while ((SNVS->LPCR & SNVS_LPCR_LPTA_EN_MASK))
            {
            }

            SNVS->LPSRTCMR = 0x00;
            SNVS->LPSRTCLR = 0x00;
            /* Set alarm in seconds*/
            SNVS->LPTAR = s_wakeupTimeout;
            EnableIRQ(APP_WAKEUP_SNVS_IRQ);
            /* Enable SRTC time counter and alarm interrupt */
            SNVS->LPCR |= SNVS_LPCR_SRTC_ENV_MASK | SNVS_LPCR_LPTA_EN_MASK;
            while (!(SNVS->LPCR & SNVS_LPCR_LPTA_EN_MASK))
            {
            }

            LPM_EnableWakeupSource(APP_WAKEUP_SNVS_IRQ);
        }
        else
        {
            GPT_StopTimer(APP_WAKEUP_GPT_BASE);
            /* Update compare channel1 value will reset counter */
            GPT_SetOutputCompareValue(APP_WAKEUP_GPT_BASE, kGPT_OutputCompare_Channel1,
                                      (CLOCK_GetRtcFreq() * s_wakeupTimeout) - 1U);

            /* Enable GPT Output Compare1 interrupt */
            GPT_EnableInterrupts(APP_WAKEUP_GPT_BASE, kGPT_OutputCompare1InterruptEnable);
            NVIC_ClearPendingIRQ(APP_WAKEUP_GPT_IRQn);
            NVIC_SetPriority(APP_WAKEUP_GPT_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY + 2);
            EnableIRQ(APP_WAKEUP_GPT_IRQn);

            /* Restart timer */
            GPT_StartTimer(APP_WAKEUP_GPT_BASE);

            LPM_EnableWakeupSource(APP_WAKEUP_GPT_IRQn);
        }
    }
    else
    {
        GPIO_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
        /* Enable GPIO pin interrupt */
        GPIO_EnableInterrupts(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
        NVIC_ClearPendingIRQ(APP_WAKEUP_BUTTON_IRQ);
        NVIC_SetPriority(APP_WAKEUP_BUTTON_IRQ, configMAX_SYSCALL_INTERRUPT_PRIORITY + 2);
        /* Enable the Interrupt */
        EnableIRQ(APP_WAKEUP_BUTTON_IRQ);
        /* Enable GPC interrupt */
        LPM_EnableWakeupSource(APP_WAKEUP_BUTTON_IRQ);
    }
}

lpm_power_mode_t APP_GetRunMode(void)
{
    return s_curRunMode;
}

void APP_SetRunMode(lpm_power_mode_t powerMode)
{
    s_curRunMode = powerMode;
}

static void APP_ShowPowerMode(lpm_power_mode_t powerMode)
{
    if (powerMode <= LPM_PowerModeRunEnd)
    {
        PRINTF("    Power mode: %s\r\n", s_modeNames[powerMode]);
        APP_PrintRunFrequency(1);
    }
    else
    {
        assert(0);
    }
}

/*
 * Check whether could switch to target power mode from current mode.
 * Return true if could switch, return false if could not switch.
 */
bool APP_CheckPowerMode(lpm_power_mode_t originPowerMode, lpm_power_mode_t targetPowerMode)
{
    bool modeValid = true;

    /* Don't need to change power mode if current mode is already the target mode. */
    if (originPowerMode == targetPowerMode)
    {
        PRINTF("Already in the target power mode.\r\n");
        modeValid = false;
    }

    return modeValid;
}

void APP_PowerPreSwitchHook(lpm_power_mode_t targetMode)
{
    if (targetMode == LPM_PowerModeSNVS)
    {
        PRINTF("Now shutting down the system...\r\n");
    }

    if (targetMode > LPM_PowerModeRunEnd)
    {
        /* Wait for debug console output finished. */
        while (!(kLPUART_TransmissionCompleteFlag & LPUART_GetStatusFlags((LPUART_Type *)BOARD_DEBUG_UART_BASEADDR)))
        {
        }
        DbgConsole_Deinit();

        /*
         * Set pin for current leakage.
         * Debug console RX pin: Set pinmux to GPIO input.
         * Debug console TX pin: Don't need to change.
         */
        ConfigUartRxPinToGpio();
    }
}

void APP_PowerPostSwitchHook(lpm_power_mode_t targetMode)
{
    if (targetMode > LPM_PowerModeRunEnd)
    {
        /*
         * Debug console RX pin is set to GPIO input, need to re-configure pinmux.
         * Debug console TX pin: Don't need to change.
         */
        ReConfigUartRxPin();
        BOARD_InitDebugConsole();
    }
    else
    {
        /* update current run mode */
        APP_SetRunMode(targetMode);
    }
}

lpm_power_mode_t APP_GetLPMPowerMode(void)
{
    return s_targetPowerMode;
}
/* Called in idle task */
static bool APP_LpmListener(lpm_power_mode_t curMode, lpm_power_mode_t newMode, void *data)
{
    PRINTF("WorkingTask %d: Transfer from %s to %s\r\n", (uint32_t)data, s_modeNames[curMode], s_modeNames[newMode]);

    /* Do necessary preparation for this mode change */

    return true; /* allow this switch */
}

/*!
 * @brief main task.
 */
static void PowerModeSwitchTask(void *pvParameters)
{
    uint8_t ch;
    uint32_t freq;

    while (1)
    {
        freq = CLOCK_GetFreq(kCLOCK_CpuClk);

        PRINTF("\r\n########## Power Mode Switch Demo ###########\n\r\n");
        PRINTF("    Core Clock = %dHz \r\n", freq);

        APP_ShowPowerMode(s_curRunMode);

        PRINTF("\r\nSelect the desired operation \n\r\n");
        PRINTF("Press  %c for enter: Over RUN       - System Over Run mode\r\n",
               (uint8_t)'A' + (uint8_t)LPM_PowerModeOverRun);
        PRINTF("Press  %c for enter: Full RUN       - System Full Run mode\r\n",
               (uint8_t)'A' + (uint8_t)LPM_PowerModeFullRun);
        PRINTF("Press  %c for enter: Low Speed RUN  - System Low Speed Run mode\r\n",
               (uint8_t)'A' + (uint8_t)LPM_PowerModeLowSpeedRun);
        PRINTF("Press  %c for enter: Low Power RUN  - System Low Power Run mode\r\n",
               (uint8_t)'A' + (uint8_t)LPM_PowerModeLowPowerRun);
        PRINTF("Press  %c for enter: System Idle    - System Wait mode\r\n",
               (uint8_t)'A' + (uint8_t)LPM_PowerModeSysIdle);
        PRINTF("Press  %c for enter: Low Power Idle - Low Power Idle mode\r\n",
               (uint8_t)'A' + (uint8_t)LPM_PowerModeLPIdle);
        PRINTF("Press  %c for enter: Suspend        - Suspend mode\r\n", (uint8_t)'A' + (uint8_t)LPM_PowerModeSuspend);
        PRINTF("Press  %c for enter: SNVS           - Shutdown the system\r\n",
               (uint8_t)'A' + (uint8_t)LPM_PowerModeSNVS);
        PRINTF("\r\nWaiting for power mode select...\r\n\r\n");

        /* Wait for user response */
        ch = GETCHAR();

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }

        s_targetPowerMode = (lpm_power_mode_t)(ch - 'A');

        if (s_targetPowerMode <= LPM_PowerModeEnd)
        {
            /* If could not set the target power mode, loop continue. */
            if (!APP_CheckPowerMode(s_curRunMode, s_targetPowerMode))
            {
                continue;
            }

            if (!LPM_SetPowerMode(s_targetPowerMode))
            {
                PRINTF("Some task doesn't allow to enter mode %s\r\n", s_modeNames[s_targetPowerMode]);
            }
            else
            {
                if (s_targetPowerMode <= LPM_PowerModeRunEnd)
                {
                    switch (s_targetPowerMode)
                    {
                        case LPM_PowerModeOverRun:
                            LPM_OverDriveRun(s_curRunMode);
                            break;
                        case LPM_PowerModeFullRun:
                            LPM_FullSpeedRun(s_curRunMode);
                            break;
                        case LPM_PowerModeLowSpeedRun:
                            LPM_LowSpeedRun(s_curRunMode);
                            break;
                        case LPM_PowerModeLowPowerRun:
                            LPM_LowPowerRun(s_curRunMode);
                            break;
                        default:
                            break;
                    }
                    APP_SetRunMode(s_targetPowerMode);
                    continue;
                }
                else if (LPM_PowerModeSNVS == s_targetPowerMode)
                {
                    APP_GetWakeupConfig(s_targetPowerMode);
                    APP_SetWakeupConfig(s_targetPowerMode);
                    APP_PowerPreSwitchHook(s_targetPowerMode);
                    LPM_EnterSNVS();
                }
                else
                {
                    APP_GetWakeupConfig(s_targetPowerMode);
                    APP_SetWakeupConfig(s_targetPowerMode);
                    if (xSemaphoreTake(s_wakeupSig, portMAX_DELAY) == pdFALSE)
                    {
                        assert(0);
                    }
                    /* Invalidate I-Cache in case instruction fetch incorrectly after wake up. */
                    if (SCB_CCR_IC_Msk == (SCB_CCR_IC_Msk & SCB->CCR))
                    {
                        SCB_InvalidateICache();
                    }
                }
                LPM_SetPowerMode(s_curRunMode);
            }
        }
        PRINTF("\r\nNext loop\r\n");
    }
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

/*!
 * @brief main demo function.
 */
int main(void)
{
    BaseType_t xReturn;

    /* Init board hardware. */
    BOARD_ConfigMPU();

#ifndef SKIP_SYSCLK_INIT
    /*
     * This is generally workaround to prevent CM7 core from speculatively access
     * SDRAM address space(even software doesn't require explicitly) when SDRAM
     * is not initialized.
     *
     * Note:
     * In SDK projects, macro SKIP_SYSCLK_INIT is defined in project setttings for
     * those build configurations where SDRAM (configured by SEMC) should be
     * initialized via either debug script or dcd.
     */
#define SDRAM_BASE_ADDR 0x80000000UL
    uint32_t i, n;

    /* MPU region number */
    n = (MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos;

    /* Go through MPU region to disable regions settings for SDRAM address space */
    for (i = 0; i < n; i++)
    {
        MPU->RNR = i;
        if ((MPU->RBAR & MPU_RBAR_ADDR_Msk) == SDRAM_BASE_ADDR)
        {
            MPU->RBAR = 0U;
            MPU->RASR = 0U;
        }
    }
#endif /* ifndef SKIP_SYSCLK_INIT */

    BOARD_InitBootPins();
    BOARD_InitBootClocks();

    /* When wakeup from suspend, peripheral's doze & stop requests won't be cleared, need to clear them manually */
    IOMUXC_GPR->GPR4  = 0;
    IOMUXC_GPR->GPR7  = 0;
    IOMUXC_GPR->GPR8  = 0;
    IOMUXC_GPR->GPR12 = 0;

    /* Configure UART divider to default */
    CLOCK_SetMux(kCLOCK_UartMux, 1); /* Set UART source to OSC 24M */
    CLOCK_SetDiv(kCLOCK_UartDiv, 0); /* Set UART divider to 1 */

    BOARD_InitDebugConsole();

    /* Since SNVS_PMIC_STBY_REQ_GPIO5_IO02 will output a high-level signal under Stop Mode(Suspend Mode) and this pin is
     * connected to LCD power switch circuit. So it needs to be configured as a low-level output GPIO to reduce the
     * current. */
    BOARD_Init_PMIC_STBY_REQ();
    BOARD_InitBootPeripherals();
    /* Disable clock gates which are not used in this application. User should modify this function based on application
     * requirement. */
    SetLowPowerClockGate();
    /* USBPHY is not used in this application. */
    CLOCK_DisableUsbhs0PhyPllClock();
    CLOCK_DisableUsbhs1PhyPllClock();
    PowerDownUSBPHY();

    PRINTF("\r\nCPU wakeup source 0x%x...\r\n", SRC->SRSR);

    PRINTF("\r\n***********************************************************\r\n");
    PRINTF("\tPower Mode Switch Demo for %s\r\n", CPU_NAME);
    PRINTF("***********************************************************\r\n");
    APP_PrintRunFrequency(0);

    if (true != LPM_Init(s_curRunMode))
    {
        PRINTF("LPM Init Failed!\r\n");
        return -1;
    }

    s_wakeupSig = xSemaphoreCreateBinary();
    /* Make current resource count 0 for signal purpose */
    if (xSemaphoreTake(s_wakeupSig, 0) == pdTRUE)
    {
        assert(0);
    }

    xReturn = xTaskCreate(PowerModeSwitchTask, "Power Mode Switch Task", configMINIMAL_STACK_SIZE + 512, NULL,
                          tskIDLE_PRIORITY + 1U, NULL);
    configASSERT(xReturn);

    xReturn =
        xTaskCreate(WorkingTask, "Working Task1", configMINIMAL_STACK_SIZE, (void *)1, tskIDLE_PRIORITY + 2U, NULL);
    configASSERT(xReturn);

    xReturn =
        xTaskCreate(WorkingTask, "Working Task2", configMINIMAL_STACK_SIZE, (void *)2, tskIDLE_PRIORITY + 3U, NULL);
    configASSERT(xReturn);

    vTaskStartScheduler();

    /* Application should never reach this point. */
    for (;;)
    {
    }
}
