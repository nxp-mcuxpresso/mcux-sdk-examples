/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "fsl_rgpio.h"
#include "fsl_lptmr.h"
#include "fsl_lpuart.h"
#include "fsl_mu.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "lpm.h"
#include "power_mode_switch.h"
#include "fsl_sentinel.h"
#include "fsl_rgpio.h"

#include "fsl_iomuxc.h"
#include "rsc_table.h"
/*******************************************************************************
 * Struct Definitions
 ******************************************************************************/
#define APP_DEBUG_UART_BAUDRATE (115200U) /* Debug console baud rate. */
#define APP_LPTMR2_IRQ_PRIO     (5U)
#define APP_LPUART2_IRQ_PRIO    (5U)
typedef enum _app_wakeup_source
{
    kAPP_WakeupSourceLptmr, /*!< Wakeup by LPTMR.        */
    kAPP_WakeupSourceLpuart /*!< Wakeup by LPUART.       */
} app_wakeup_source_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
extern void APP_PowerPreSwitchHook(lpm_power_mode_t targetMode);
extern void APP_PowerPostSwitchHook(lpm_power_mode_t targetMode, bool result);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t s_wakeupTimeout;           /* Wakeup timeout. (Unit: Second) */
static app_wakeup_source_t s_wakeupSource; /* Wakeup source.                 */
static SemaphoreHandle_t s_wakeupSig;
static const char *s_modeNames[] = {"RUN", "WAIT", "STOP", "SUSPEND"};
extern lpm_power_mode_t s_curMode;

/*******************************************************************************
 * Function Code
 ******************************************************************************/

/* Get input from user about wakeup timeout. */
static uint32_t APP_GetWakeupTimeout(void)
{
    uint32_t timeout = 0U;
    uint8_t c;

    while (1)
    {
        PRINTF("Select the wake up timeout in seconds.\r\n");
        PRINTF("The allowed range is 1s ~ 999s.\r\n");
        PRINTF("Eg. enter 5 to wake up in 5 seconds.\r\n");
        PRINTF("\r\nWaiting for input timeout value...\r\n\r\n");

        do
        {
            c = GETCHAR();
            if ((c >= '0') && (c <= '9'))
            {
                PRINTF("%c", c);
                timeout = timeout * 10U + c - '0';
            }
            else if ((c == '\r') || (c == '\n'))
            {
                break;
            }
            else
            {
                PRINTF("%c\r\nWrong value!\r\n", c);
                timeout = 0U;
            }
        } while (timeout != 0U && timeout < 100U);

        if (timeout > 0U)
        {
            PRINTF("\r\n");
            break;
        }
    }

    return timeout;
}

/* Get wakeup source by user input. */
static app_wakeup_source_t APP_GetWakeupSource(void)
{
    uint8_t ch;

    while (1)
    {
        PRINTF("Select the wake up source:\r\n");
        PRINTF("Press T for LPTMR - Low Power Timer\r\n");
        PRINTF("Press U for LPUART - Low Power Uart\r\n");

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
        else if (ch == 'U')
        {
            return kAPP_WakeupSourceLpuart;
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
    if (kAPP_WakeupSourceLpuart == s_wakeupSource)
    {
        PRINTF("Press any button to wake up.\r\n");
    }
}

static void APP_SetWakeupConfig(lpm_power_mode_t targetMode)
{
    if (kAPP_WakeupSourceLptmr == s_wakeupSource)
    {
        LPTMR_SetTimerPeriod(LPTMR2, (32000UL * s_wakeupTimeout / 1U));
        LPTMR_StartTimer(LPTMR2);
        LPTMR_EnableInterrupts(LPTMR2, kLPTMR_TimerInterruptEnable);
    }

    if (kAPP_WakeupSourceLpuart == s_wakeupSource)
    {
        PRINTF("LPUART2 is used for wakeup source.\r\n");
        LPUART_EnableInterrupts(LPUART2, kLPUART_RxDataRegFullInterruptEnable);
        /* In low power mode, LPUART can generate wakeup via STAT[RXEDGIF]
         * Config STAT[RXINV] and BAUD[RXEDGIE] to enable STAT[RXEDGIF].
         */
        LPUART2->STAT &= ~LPUART_STAT_RXINV_MASK;
        LPUART2->BAUD |= LPUART_BAUD_RXEDGIE_MASK;
    }
}

/* LPTMR2 interrupt handler. */
void LPTMR2_IRQHandler(void)
{
    bool wakeup = false;
    if (kLPTMR_TimerInterruptEnable & LPTMR_GetEnabledInterrupts(LPTMR2))
    {
        LPTMR_ClearStatusFlags(LPTMR2, kLPTMR_TimerCompareFlag);
        LPTMR_DisableInterrupts(LPTMR2, kLPTMR_TimerInterruptEnable);
        LPTMR_StopTimer(LPTMR2);
        wakeup = true;
    }

    if (wakeup)
    {
        xSemaphoreGiveFromISR(s_wakeupSig, NULL);
        portYIELD_FROM_ISR(pdTRUE);
    }
}

/* LPUART2 interrupt handler. */
void LPUART2_IRQHandler(void)
{
    bool wakeup = false;
    if (kLPUART_RxDataRegFullInterruptEnable & LPUART_GetEnabledInterrupts(LPUART2))
    {
        LPUART_DisableInterrupts(LPUART2, kLPUART_RxDataRegFullInterruptEnable);
        LPUART2->BAUD &= ~LPUART_BAUD_RXEDGIE_MASK;
        wakeup = true;
    }

    if (wakeup)
    {
        xSemaphoreGiveFromISR(s_wakeupSig, NULL);
        portYIELD_FROM_ISR(pdTRUE);
    }
}

/* Power Mode Switch task */
void PowerModeSwitchTask(void *pvParameters)
{
    lptmr_config_t lptmrConfig;

    lpm_power_mode_t targetPowerMode;
    uint32_t freq = 0U;
    uint8_t ch;

    /* Setup LPTMR. */
    LPTMR_GetDefaultConfig(&lptmrConfig);
    lptmrConfig.prescalerClockSource = kLPTMR_PrescalerClock_1;  /* Use RTC 32KHz as clock source. */
    lptmrConfig.bypassPrescaler      = true;
    lptmrConfig.value                = kLPTMR_Prescale_Glitch_0; /* Divide clock source by 2. */
    LPTMR_Init(LPTMR2, &lptmrConfig);
    NVIC_SetPriority(LPTMR2_IRQn, APP_LPTMR2_IRQ_PRIO);

    EnableIRQ(LPTMR2_IRQn);

    /* Setup LPUART. */
    NVIC_SetPriority(LPUART2_IRQn, APP_LPUART2_IRQ_PRIO);

    EnableIRQ(LPUART2_IRQn);

    /* Config OSCPLL LPM setting for M33 SUSPEND */
    for (unsigned int i = OSCPLL_LPM_START; i <= OSCPLL_LPM_END; i++)
    {
        CCM_CTRL->OSCPLL[i].LPM0 |= CCM_OSCPLL_LPM0_LPM_SETTING_D2_MASK;
    }

    for (;;)
    {
        freq = CLOCK_GetIpFreq(kCLOCK_Root_M33);
        PRINTF("\r\n####################  Power Mode Switch Task ####################\n\r\n");
        PRINTF("    Build Time: %s--%s \r\n", __DATE__, __TIME__);
        PRINTF("    Core Clock: %dHz \r\n", freq);
        PRINTF("\r\nSelect the desired operation \n\r\n");
        PRINTF("Press  %c to enter: Normal RUN mode\r\n", kAPP_PowerModeRun);
        PRINTF("Press  %c to enter: WAIT mode\r\n", kAPP_PowerModeWait);
        PRINTF("Press  %c to enter: STOP mode\r\n", kAPP_PowerModeStop);
        PRINTF("Press  %c to enter: SUSPEND mode\r\n", kAPP_PowerModeSuspend);
        PRINTF("Press  W to wakeup A55 core\r\n");
        PRINTF("Press  M for switch M33 Root Clock frequency between OD/ND.\r\n");
        PRINTF("\r\nWaiting for power mode select..\r\n\r\n");

        /* Wait for user response */
        do
        {
            ch = GETCHAR();
        } while ((ch == '\r') || (ch == '\n'));

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }
        targetPowerMode = (lpm_power_mode_t)(ch - 'A');

        if (targetPowerMode <= LPM_PowerModeSuspend)
        {
            if (targetPowerMode == s_curMode)
            {
                /* Same mode, skip it */
                continue;
            }
            if (targetPowerMode == LPM_PowerModeSuspend)
            {
                PRINTF("Application get in Suspend mode\r\n");
            }
            if (!LPM_SetPowerMode(targetPowerMode))
            {
                PRINTF("Some task doesn't allow to enter mode %s\r\n", s_modeNames[targetPowerMode]);
            }
            else /* Idle task will handle the low power state. */
            {
                APP_GetWakeupConfig();
                APP_SetWakeupConfig(targetPowerMode);
                PRINTF("Target powermode get in %s\r\n", s_modeNames[targetPowerMode]);
                xSemaphoreTake(s_wakeupSig, portMAX_DELAY);
            }
        }
        else if ('W' == ch)
        {
            /* GCR[GIR1] is used to wakeup A55. */
            MU1_MUA->GCR |= MU_GCR_GIR1_MASK;
            PRINTF("Set GCR[GIR1] Register to wakeup A55\r\n");
        }
        else if ('M' == ch)
        {
            PRINTF("Press O for OverDrive mode - choose M33 ROOT Clock freq as 250MHz\r\n");
            PRINTF("Press N for Nominal   mode - choose M33 ROOT Clock freq as 200MHz\r\n");
            PRINTF("\r\nWaiting for key press..\r\n\r\n");
            ch = GETCHAR();

            if ((ch >= 'a') && (ch <= 'z'))
            {
                ch -= 'a' - 'A';
            }
            if (ch == 'O')
            {
                const clock_root_config_t m33ClkCfg = {
                    .clockOff = false,
	            .mux = 1, // 500MHz oscillator source
	            .div = 2  /* 250Mhz */
                };
                CLOCK_SetRootClock(kCLOCK_Root_M33, &m33ClkCfg);
            }
            else if (ch == 'N')
            {
                const clock_root_config_t m33ClkCfg = {
                    .clockOff = false,
	            .mux = 2, // 400MHz oscillator source
	            .div = 2  /* 200MHz */
                };
                CLOCK_SetRootClock(kCLOCK_Root_M33, &m33ClkCfg);
            }
            else
            {
                PRINTF("%c\r\nWrong value!\r\n");
            }
        }
        else
        {
            PRINTF("Invalid command %c[0x%x]\r\n", ch, ch);
        }

        /* update Mode state */
        s_curMode = LPM_PowerModeRun;
        PRINTF("\r\nNext loop\r\n");
    }
}

void vApplicationMallocFailedHook(void)
{
    PRINTF("Malloc Failed!!!\r\n");
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

void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
    uint32_t irqMask;
    lpm_power_mode_t targetPowerMode;
    bool result;

    irqMask = DisableGlobalIRQ();

    /* Only when no context switch is pending and no task is waiting for the scheduler
     * to be unsuspended then enter low power entry.
     */
    if (eTaskConfirmSleepModeStatus() != eAbortSleep)
    {
        targetPowerMode = LPM_GetPowerMode();
        if (targetPowerMode != LPM_PowerModeRun)
        {
            /* Only wait when target power mode is not running */
            APP_PowerPreSwitchHook(targetPowerMode);
            result = LPM_WaitForInterrupt((uint64_t)1000 * xExpectedIdleTime / configTICK_RATE_HZ);
            APP_PowerPostSwitchHook(targetPowerMode, result);
        }
    }
    EnableGlobalIRQ(irqMask);
}

void APP_PowerPreSwitchHook(lpm_power_mode_t targetMode)
{
    if ((LPM_PowerModeRun != targetMode))
    {
        /* Enable CPULPM mode for LPUART before sleep. */
        CCM_CTRL->LPCG[53].AUTHEN &= CCM_LPCG_AUTHEN_WHITE_LIST(4);
        CCM_CTRL->LPCG[53].LPM0 |= CCM_LPCG_LPM0_LPM_SETTING_D2_MASK;
        CCM_CTRL->LPCG[53].AUTHEN |= CCM_LPCG_AUTHEN_CPULPM_MODE_MASK;
    }
}

void APP_PowerPostSwitchHook(lpm_power_mode_t targetMode, bool result)
{
    if (LPM_PowerModeRun != targetMode)
    {
        /* Reset the CM MODE CTRL to run mode. */
        GPC_CTRL_CM33->CM_MODE_CTRL = GPC_CPU_CTRL_CM_MODE_CTRL_CPU_MODE_TARGET(0);
        /* Disable CPULPM mode for LPUART after sleep. */
        CCM_CTRL->LPCG[53].AUTHEN &= ~CCM_LPCG_AUTHEN_CPULPM_MODE_MASK;
        CCM_CTRL->LPCG[53].LPM0 &= ~CCM_LPCG_LPM0_LPM_SETTING_D2_MASK;
        CCM_CTRL->LPCG[53].AUTHEN |= CCM_LPCG_AUTHEN_WHITE_LIST_MASK;
    }
    PRINTF("== Power switch %s ==\r\n", result ? "OK" : "FAIL");
}

/*! @brief Main function */
int main(void)
{
    /* clang-format off */
    const clock_root_config_t lpi2cClkCfg = {
        .clockOff = false,
	.mux = 0, // 24MHz oscillator source
	.div = 1
    };
    const clock_root_config_t lptmrClkCfg = {
        .clockOff = false,
	.mux = 0, // 24MHz oscillator source
	.div = 1
    };

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetRootClock(kCLOCK_Root_Lptmr1, &lptmrClkCfg);
    CLOCK_EnableClock(kCLOCK_Lptmr1);
    CLOCK_SetRootClock(kCLOCK_Root_Lptmr2, &lptmrClkCfg);
    CLOCK_EnableClock(kCLOCK_Lptmr2);
    CLOCK_SetRootClock(kCLOCK_Root_Lpi2c1, &lpi2cClkCfg);
    CLOCK_EnableClock(kCLOCK_Lpi2c1);
    CLOCK_SetRootClock(BOARD_ADP5585_I2C_CLOCK_ROOT, &lpi2cClkCfg);
    CLOCK_EnableClock(BOARD_ADP5585_I2C_CLOCK_GATE);

    /* Select PDM/SAI signals */
    adp5585_handle_t handle;
    BOARD_InitADP5585(&handle);
    ADP5585_SetDirection(&handle, (1 << BOARD_ADP5585_PDM_MQS_SEL) | (1 << BOARD_ADP5585_EXP_SEL), kADP5585_Output);
    ADP5585_ClearPins(&handle, (1 << BOARD_ADP5585_PDM_MQS_SEL) | (1 << BOARD_ADP5585_EXP_SEL));

    /* copy resource table to destination address(TCM) */
    copyResourceTable();

    LPM_Init();

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
