/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_power.h"
#include "fsl_debug_console.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "lpm.h"
#include "power_mode_switch.h"
#include "fsl_usart.h"
#include "fsl_rtc.h"

/*******************************************************************************
 * Struct Definitions
 ******************************************************************************/
#define APP_WAKEUP_PIN1_NAME "SW2"
#define APP_GPIO_PIN_NAME    "SW2"
#define APP_AONPIN_CONNECTED (false)
#define APP_GPIO_INTA_IRQHandler GPIO_INTA_DriverIRQHandler
#define APP_SW_PORT              BOARD_SW2_GPIO_PORT
#define APP_SW_PIN               BOARD_SW2_GPIO_PIN

/* Leave AON modules on in PM2 */
#define APP_PM2_MEM_PU_CFG ((uint32_t)kPOWER_Pm2MemPuAon1 | (uint32_t)kPOWER_Pm2MemPuAon0)
/* All ANA in low power mode in PM2 */
#define APP_PM2_ANA_PU_CFG (0U)
/* Buck18 and Buck11 both in sleep level in PM3 */
#define APP_PM3_BUCK_CFG (0U)
/* clk_32k not derived from cau, capture_timer also not used in the app. It's safe to disable CAU clock in PM3,4. */
#define APP_SLEEP_CAU_PD (true)
/* All clock gated */
#define APP_SOURCE_CLK_GATE ((uint32_t)kPOWER_ClkGateAll)
/* All SRAM kept in retention in PM3, AON SRAM shutdown in PM4 */
#define APP_MEM_PD_CFG (1UL << 8)

typedef struct
{
    uint32_t selA;
    uint32_t selB;
    uint32_t frgSel;
    uint32_t frgctl;
    uint32_t osr;
    uint32_t brg;
} uart_clock_context_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t s_wakeupTimeout;           /* Wakeup timeout. (Unit: seconds) */
static app_wakeup_source_t s_wakeupSource; /* Wakeup source. */
static const char *s_modeNames[] = {"Active", "Idle", "Standby", "Sleep", "Deep Sleep"};
static uart_clock_context_t s_uartClockCtx;

/*******************************************************************************
 * Function Code
 ******************************************************************************/
void APP_GPIO_INTA_IRQHandler(void)
{
    /* clear the interrupt status */
    GPIO_PinClearInterruptFlag(GPIO, APP_SW_PORT, APP_SW_PIN, 0);
    SDK_ISR_EXIT_BARRIER;
}

void BOARD_UART_IRQ_HANDLER(void)
{
    /* If new data arrived. */
    if ((kUSART_RxFifoNotEmptyFlag | kUSART_RxError) & USART_GetStatusFlags(BOARD_DEBUG_UART))
    {
        (void)USART_ReadByte(BOARD_DEBUG_UART);
    }

    PRINTF("Woken up by UART\r\n");

    USART_DisableInterrupts(BOARD_DEBUG_UART, kUSART_RxLevelInterruptEnable | kUSART_RxErrorInterruptEnable);
    DisableIRQ(BOARD_UART_IRQ);

    SDK_ISR_EXIT_BARRIER;
}

/* Get input from user about wakeup timeout. */
static uint8_t APP_GetWakeupTimeout(void)
{
    uint8_t timeout;
    uint32_t i;
    uint8_t num = 0U;

    while (num == 0U)
    {
        PRINTF("Select the wake up timeout in seconds.\r\n");
        PRINTF("The allowed range is 1s ~ 99s.\r\n");
        PRINTF("Eg. enter 05 to wake up in 5 seconds.\r\n");
        PRINTF("\r\nWaiting for input timeout value...\r\n\r\n");

        for (i = 0U; i < 2U; i++)
        {
            timeout = GETCHAR();
            PUTCHAR(timeout);
            if ((timeout >= '0') && (timeout <= '9'))
            {
                num = num * 10U + (timeout - '0');
            }
            else
            {
                PRINTF("Wrong value!");
                num = 0U;
                break;
            }
        }
        PRINTF("\r\n");
    }

    return num;
}

/* Get wakeup source by user input. */
static app_wakeup_source_t APP_GetWakeupSource(uint32_t mode)
{
    uint8_t ch;

    while (true)
    {
        PRINTF("Select the wake up source:\r\n");
        PRINTF("Press T for RTC.\r\n");
#if (APP_AONPIN_CONNECTED)
        PRINTF("Press 1 for wakeup pin1(%s).\r\n", APP_WAKEUP_PIN1_NAME);
#endif
        if (mode <= 2U)
        {
            PRINTF("Press 2 for wakeup gpio pin(%s).\r\n", APP_GPIO_PIN_NAME);
            PRINTF("Press U for UART wakeup.\r\n");
        }

        PRINTF("\r\nWaiting for key press..\r\n\r\n");

        ch = GETCHAR();
        PRINTF("%c\r\n", ch);

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }

        if (ch == 'T')
        {
            return kAPP_WakeupSourceRtc;
        }
#if (APP_AONPIN_CONNECTED)
        else if (ch == '1')
        {
            return kAPP_WakeupSourcePin1;
        }
#endif
        else if (ch == '2')
        {
            return kAPP_WakeupSourceGPIOPIN;
        }
        else if ((ch == 'U') && (mode <= 2U))
        {
            return kAPP_WakeupSourceUart;
        }
        else
        {
            PRINTF("Wrong value!\r\n");
        }
    }
}

void APP_WakeupHandler(IRQn_Type irq)
{
    switch (irq)
    {
        case RTC_IRQn:
            PRINTF("Woken up by RTC\r\n");
            break;
        case PIN1_INT_IRQn:
            PRINTF("Woken up by wakeup pin1\r\n");
            break;
        default:
            PRINTF("Unexpected wakeup by %d\r\n", irq);
            break;
    }
}

static void APP_GetSleepConfig(power_sleep_config_t *config)
{
    config->pm2MemPuCfg = APP_PM2_MEM_PU_CFG;
    config->pm2AnaPuCfg = APP_PM2_ANA_PU_CFG;
    config->clkGate     = APP_SOURCE_CLK_GATE;
    config->memPdCfg    = APP_MEM_PD_CFG;
    config->pm3BuckCfg  = APP_PM3_BUCK_CFG;
}

/* Get wakeup timeout and wakeup source. */
static void APP_GetWakeupConfig(uint32_t mode)
{
    /* Get wakeup source by user input. */
    s_wakeupSource = APP_GetWakeupSource(mode);

    if (kAPP_WakeupSourceRtc == s_wakeupSource)
    {
        /* Wakeup source is RTC, user should input wakeup timeout value. */
        s_wakeupTimeout = APP_GetWakeupTimeout();
    }
    else
    {
        s_wakeupTimeout = 0U;
    }
}

static void APP_SetWakeupConfig(void)
{
    uint32_t currSeconds;

    if (s_wakeupSource == kAPP_WakeupSourceUart)
    {
        /* Enable RX interrupt. */
        USART_EnableInterrupts(BOARD_DEBUG_UART, kUSART_RxLevelInterruptEnable | kUSART_RxErrorInterruptEnable);
        LPM_EnableWakeupSource(BOARD_UART_IRQ);
        PRINTF("Input any key in terminal to wake up.\r\n");
    }
    else if (s_wakeupSource == kAPP_WakeupSourceRtc)
    {
        LPM_EnableWakeupSource(RTC_IRQn);
        /* Read the RTC seconds register to get current time in seconds */
        currSeconds = RTC_GetSecondsTimerCount(RTC);
        /* Add alarm seconds to current time */
        currSeconds += s_wakeupTimeout;
        /* Set alarm time in seconds */
        RTC_SetSecondsTimerMatch(RTC, currSeconds);
        PRINTF("RTC wake up after %d seconds.\r\n", s_wakeupTimeout);
    }
    else if (s_wakeupSource == kAPP_WakeupSourcePin1)
    {
        LPM_EnableWakeupSource(PIN1_INT_IRQn);
        PRINTF("Push wakeup PIN1 to wake up.\r\n");
    }
    else if (s_wakeupSource == kAPP_WakeupSourceGPIOPIN)
    {
        LPM_EnableWakeupSource(GPIO_INTA_IRQn);
        PRINTF("Push wakeup GPIO PIN to wake up.\r\n");    }
    else
    {
    }
}

static void APP_ClearWakeupConfig(void)
{
    if (s_wakeupSource == kAPP_WakeupSourceUart)
    {
        /* Disable RX interrupt. */
        USART_DisableInterrupts(BOARD_DEBUG_UART, kUSART_RxLevelInterruptEnable | kUSART_RxErrorInterruptEnable);
        LPM_DisableWakeupSource(BOARD_UART_IRQ);
    }
    else if (s_wakeupSource == kAPP_WakeupSourceRtc)
    {
        LPM_DisableWakeupSource(RTC_IRQn);
    }
    else
    {
        /* Wakeup pin interrupt disabled in IRQ handler */
    }
}

AT_QUICKACCESS_SECTION_CODE(static void APP_PrePowerSwitch(uint32_t mode, void *param))
{
    if ((s_wakeupSource == kAPP_WakeupSourceUart) && (mode == 2U))
    {
        /* In PM2, only LPOSC and CLK32K are available. To use UART as wakeup source,
           We have to use main_clk as UART clock source, and main_clk comes from LPOSC.
           Use register access directly to avoid possible flash access in function call. */
        s_uartClockCtx.selA   = CLKCTL0->MAINCLKSELA;
        s_uartClockCtx.selB   = CLKCTL0->MAINCLKSELB;
        s_uartClockCtx.frgSel = CLKCTL1->FLEXCOMM[3].FRGCLKSEL;
        s_uartClockCtx.frgctl = CLKCTL1->FLEXCOMM[3].FRGCTL;
        s_uartClockCtx.osr    = USART3->OSR;
        s_uartClockCtx.brg    = USART3->BRG;
        /* Switch main_clk to LPOSC */
        CLKCTL0->MAINCLKSELA = 2;
        CLKCTL0->MAINCLKSELB = 0;
        /* Change UART3 clock source to main_clk */
        CLKCTL1->FLEXCOMM[3].FRGCLKSEL = 0;
        CLKCTL1->FLEXCOMM[3].FRGCTL    = 0;
        /* Set UART baudrate to 1MHz / 9 */
        USART3->OSR = 8;
        USART3->BRG = 0;
    }
}

AT_QUICKACCESS_SECTION_CODE(static void APP_PostPowerSwitch(uint32_t mode, void *param))
{
    if ((s_wakeupSource == kAPP_WakeupSourceUart) && (mode == 2U))
    {
        /* Recover main_clk and UART clock source after wakeup.
           Use register access directly to avoid possible flash access in function call. */
        USART3->OSR                    = s_uartClockCtx.osr;
        USART3->BRG                    = s_uartClockCtx.brg;
        CLKCTL1->FLEXCOMM[3].FRGCLKSEL = s_uartClockCtx.frgSel;
        CLKCTL1->FLEXCOMM[3].FRGCTL    = s_uartClockCtx.frgctl;
        CLKCTL0->MAINCLKSELA           = s_uartClockCtx.selA;
        CLKCTL0->MAINCLKSELB           = s_uartClockCtx.selB;
    }
}

void RTC_IRQHandler()
{
    if (RTC_GetStatusFlags(RTC) & kRTC_AlarmFlag)
    {
        /* Clear alarm flag */
        RTC_ClearStatusFlags(RTC, kRTC_AlarmFlag);
        POWER_ClearWakeupStatus(RTC_IRQn);
        APP_WakeupHandler(RTC_IRQn);
    }
}

void PIN1_INT_IRQHandler()
{
    LPM_DisableWakeupSource(PIN1_INT_IRQn);
    NVIC_ClearPendingIRQ(PIN1_INT_IRQn);
    APP_WakeupHandler(PIN1_INT_IRQn);
}

/*! @brief Main function */
int main(void)
{
    uint32_t resetSrc;
    uint32_t freq = 0U;
    uint32_t i;
    uint32_t pm;
    uint8_t ch;
    uint32_t irqMask;
    lpm_config_t config = {
        /* PM2/tickless idle less than 10 ms will be skipped. */
        .threshold = 10U,
    };
    power_init_config_t initCfg = {
        .iBuck         = true,  /* VCORE AVDD18 supplied from iBuck on RD board. */
        .gateCauRefClk = false, /* CAU_SOC_SLP_REF_CLK needed for LPOSC. */
    };

    power_sleep_config_t slpCfg;

    BOARD_InitBootPins();
    BOARD_BootClockLPR();
    BOARD_InitDebugConsole();
    BOARD_InitSleepPinConfig();

    /* Disable T3 256M clock and SFRO. As a result, DTRNG and GDET are now not working.
     * This is to demonstrate low runtime power. */
    CLOCK_DisableClock(kCLOCK_T3PllMci256mClk);
    /* Deinitialize T3 clocks */
    CLOCK_DeinitT3RefClk();

    CLOCK_AttachClk(kRC32K_to_CLK32K);
    CLOCK_AttachClk(kLPOSC_to_OSTIMER_CLK);
    POWER_PowerOffWlan();
    POWER_PowerOffBle();

    resetSrc = POWER_GetResetCause();
    PRINTF("\r\nMCU wakeup source 0x%x...\r\n", resetSrc);

    POWER_ClearResetCause(resetSrc);
    /* In case PM4 wakeup, the wakeup config and status need to be cleared */
    LPM_DisableWakeupSource(RTC_IRQn);
    LPM_DisableWakeupSource(PIN1_INT_IRQn);

    RTC_Init(RTC);
    /* Start RTC */
    RTC_StartTimer(RTC);
    /* Enable wakeup in PD mode */
    RTC_EnableAlarmTimerInterruptFromDPD(RTC, true);

    LPM_Init(&config);
    POWER_InitPowerConfig(&initCfg);
    POWER_ConfigCauInSleep(APP_SLEEP_CAU_PD);
    POWER_SetPowerSwitchCallback(APP_PrePowerSwitch, NULL, APP_PostPowerSwitch, NULL);

    for (;;)
    {
        freq = CLOCK_GetMainClkFreq();
        PRINTF("\r\n####################  Power Mode Switch ####################\n\r\n");
        PRINTF("    Build Time: %s--%s \r\n", __DATE__, __TIME__);
        PRINTF("    Core Clock: %dHz \r\n", freq);
        PRINTF("\r\nSelect the desired operation \n\r\n");
        for (i = 0U; i <= 4U; i++)
        {
            PRINTF("Press %d for enter: PM%d - %s\r\n", i, i, s_modeNames[i]);
        }
        PRINTF("\r\nWaiting for power mode select..\r\n\r\n");

        /* Wait for user response */
        ch = GETCHAR();
        PRINTF("%c\r\n", ch);
        if (ch >= '0' && ch <= '4')
        {
            pm = ch - '0';
        }
        else
        {
            PRINTF("No such power mode\r\n");
            continue;
        }
        if (pm >= 2U)
        {
            APP_GetSleepConfig(&slpCfg);
        }

        if (pm >= 1U)
        {
            APP_GetWakeupConfig(pm);
            APP_SetWakeupConfig();
        }

        LPM_SetPowerMode(pm, &slpCfg);
        irqMask = DisableGlobalIRQ();
        /* Set wait timeout to max to ensure only assigned source can wake up system. */
        pm = LPM_WaitForInterrupt(0xFFFFFFFFU);
        if (pm == 3U)
        {
            /* Perihperal state lost, need reinitialize in exit from PM3 */
            BOARD_InitBootPins();
            BOARD_BootClockLPR();
            BOARD_InitDebugConsole();
            /* Disable T3 256M clock and SFRO. As a result, DTRNG and GDET are now not working.
             * This is to demonstrate low runtime power. */
            CLOCK_DisableClock(kCLOCK_T3PllMci256mClk);
            /* Deinitialize T3 clocks */
            CLOCK_DeinitT3RefClk();

            POWER_InitPowerConfig(&initCfg);
            CLOCK_AttachClk(kLPOSC_to_OSTIMER_CLK);
        }
        EnableGlobalIRQ(irqMask);

        PRINTF("Exit from power mode %d\r\n", pm);
        if (pm >= 1U)
        {
            APP_ClearWakeupConfig();
        }
    }
}
