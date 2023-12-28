/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_common.h"
#include "fsl_power.h"
#include "fsl_usart.h"
#include "fsl_syscon.h"
#include "fsl_wkt.h"
#include "fsl_iocon.h"
#include <stdbool.h>
#include "fsl_pint.h"
#include "fsl_swm.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_ACTIVE_IN_DEEPSLEEP               (kPDSLEEPCFG_DeepSleepBODActive | kPDSLEEPCFG_DeepSleepLPOscActive)
#define DEMO_USER_WAKEUP_KEY_GPIO              GPIO
#define DEMO_USER_WAKEUP_KEY_PORT              BOARD_SW2_GPIO_PORT
#define DEMO_USER_WAKEUP_KEY_PIN               BOARD_SW2_GPIO_PIN
#define DEMO_SYSCON_PIN_INT_SEL                (4U)
#define DEMO_SYSCON_STARTER0_MASK              (1U)
#define DEMO_SYSCON_STARTER1_MASK              (1U << 15U)
#define DEMO_WKT_TIMEOUT_VALUE                 (250000U * 12U)
#define DEMO_PINT_PIN_INT0_SRC                 kSYSCON_GpioPort0Pin4ToPintsel
#define DEMO_WKT_CLK_FREQ                      (10000U)
#define DEMO_SLEEP_WAKEUP_SOURCE               "\t1. Wkt timer\r\n\t2. SW2, wakeup key\r\n\t3. SW3, reset key\r\n"
#define DEMO_DEEP_SLEEP_WAKEUP_SOURCE          "\t1. Wkt timer\r\n\t2. SW2, wakeup key\r\n\t3. SW3, reset key\r\n"
#define DEMO_POWER_DOWN_WAKEUP_SOURCE          "\t1. Wkt timer\r\n\t2. SW2, wakeup key\r\n\t3. SW3, reset key\r\n"
#define DEMO_DEEP_POWERDOWN_WAKEUP_SOURCE      "\t1. Wkt timer\r\n\t2. SW2, wakeup key\r\n\t3. SW3, reset key\r\n"
#define DEMO_SLEEP_WAKEUP_SOURCE_SIZE          (3U)
#define DEMO_DEEP_SLEEP_WAKEUP_SOURCE_SIZE     (3U)
#define DEMO_POWER_DOWN_WAKEUP_SOURCE_SIZE     (3U)
#define DEMO_DEEP_POWERDOWN_WAKEUP_SOURCE_SIZE (3U)
#define DEMO_WAKEUP_CASE_WKT                   (0U)
#define DEMO_WAKEUP_CASE_WAKEUP                (1U)
#define DEMO_WAKEUP_CASE_RESET                 (2U)
#define DEMO_DEEP_POWERDOWN_RESET_ENABLE       (1U)
#define POWER_DPD_ENABLE_WAKEUP_PIN            POWER_EnableWakeupPinForDeepPowerDown(true, true);
#define POWER_DPD_ENABLE_RESET_PIN             POWER_EnableResetPinForDeepPowerDown(true, true);

static power_mode_cfg_t s_CurrentPowerMode;
static uint32_t s_CurrentWakeupSource;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void DEMO_PreEnterLowPower(void);
void DEMO_LowPowerWakeup(void);
static power_mode_cfg_t DEMO_GetUserSelection(void);
static uint32_t DEMO_GetWakeUpSource(power_mode_cfg_t targetPowerMode);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*
 * Callback function when wakeup key is pressed.
 */
static void pint_intr_callback(pint_pin_int_t pintr, uint32_t pmatch_status)
{
    /* do nothing here */
}

static void DEMO_InitWkt(void)
{
    wkt_config_t wktConfig;

    POWER_EnableLPO(true);
    POWER_EnableLPOInDeepPowerDownMode(true);

    wktConfig.clockSource = kWKT_LowPowerClockSource;

    /* Init wkt module */
    WKT_Init(WKT, &wktConfig);

    /* Clear Pending Interrupt */
    NVIC_ClearPendingIRQ(WKT_IRQn);
    /* Enable at the NVIC */
    EnableIRQ(WKT_IRQn);
}

static void DEMO_InitResetPin(void)
{
    SWM_SetFixedPinSelect(SWM0, kSWM_RESETN, true);
}

/*
 * Setup a GPIO input pin as wakeup source.
 */
static void DEMO_InitWakeupPin(void)
{
    gpio_pin_config_t gpioPinConfigStruct;

    /* Set SW pin as GPIO input. */
    gpioPinConfigStruct.pinDirection = kGPIO_DigitalInput;
    GPIO_PinInit(DEMO_USER_WAKEUP_KEY_GPIO, DEMO_USER_WAKEUP_KEY_PORT, DEMO_USER_WAKEUP_KEY_PIN, &gpioPinConfigStruct);

    SYSCON_AttachSignal(SYSCON, kPINT_PinInt0, DEMO_PINT_PIN_INT0_SRC);

    /* Configure the interrupt for SW pin. */
    PINT_Init(PINT);
    PINT_PinInterruptConfig(PINT, kPINT_PinInt0, kPINT_PinIntEnableFallEdge, pint_intr_callback);
    PINT_EnableCallback(PINT); /* Enable callbacks for PINT */
}


void DEMO_PreEnterLowPower(void)
{
    /* switch main clock source to FRO 18M */
    POWER_DisablePD(kPDRUNCFG_PD_FRO_OUT);
    POWER_DisablePD(kPDRUNCFG_PD_FRO);
    CLOCK_SetFroOscFreq(kCLOCK_FroOscOut36M);
    CLOCK_SetFroOutClkSrc(kCLOCK_FroSrcFroOscDiv);
    CLOCK_SetMainClkSrc(kCLOCK_MainClkSrcFro);

    /*
     * system osc power down
     * application should decide if more part need to power down to achieve better power consumption
     */
    POWER_EnablePD(kPDRUNCFG_PD_SYSOSC);
    CLOCK_DisableClock(kCLOCK_Iocon);
    CLOCK_DisableClock(kCLOCK_Uart0);
}

void DEMO_LowPowerWakeup(void)
{
    /* clock configurations restore */
    BOARD_BootClockPll48M();

    CLOCK_EnableClock(kCLOCK_Iocon);
    CLOCK_EnableClock(kCLOCK_Uart0);
}
void WKT_IRQHandler(void)
{
    /* Clear interrupt flag.*/
    WKT_ClearStatusFlags(WKT, kWKT_AlarmFlag);
    WKT_StartTimer(WKT, USEC_TO_COUNT(DEMO_WKT_TIMEOUT_VALUE, DEMO_WKT_CLK_FREQ));
}

static uint32_t DEMO_GetUserInput(uint32_t maxChoice)
{
    uint32_t ch = 0U;

    while (1)
    {
        ch = GETCHAR();
        if ((ch < '1') || (ch > (maxChoice + '0')))
        {
            continue;
        }
        else
        {
            ch = ch - '1';
            break;
        }
    }

    return ch;
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    /* Select the main clock as source clock of USART0 (debug console) */
    CLOCK_Select(BOARD_DEBUG_USART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_BootClockPll48M();
    BOARD_InitDebugConsole();

    /* Init existed wake up source */
    DEMO_InitWakeupPin();
    DEMO_InitResetPin();
    DEMO_InitWkt();

    PRINTF("Power mode switch Demo for LPC8xx.\r\n");

    while (1)
    {
        s_CurrentPowerMode    = DEMO_GetUserSelection();
        s_CurrentWakeupSource = DEMO_GetWakeUpSource(s_CurrentPowerMode);

        /* prepare to enter low power mode */
        DEMO_PreEnterLowPower();

        /* Enter the low power mode. */
        switch (s_CurrentPowerMode)
        {
            case kPmu_Sleep: /* Enter sleep mode. */
                POWER_EnterSleep();
                break;
            case kPmu_Deep_Sleep: /* Enter deep sleep mode. */
                POWER_EnterDeepSleep(DEMO_ACTIVE_IN_DEEPSLEEP);
                break;
            case kPmu_PowerDown: /* Enter deep sleep mode. */
                POWER_EnterPowerDown(DEMO_ACTIVE_IN_DEEPSLEEP);
                break;
            case kPmu_Deep_PowerDown: /* Enter deep power down mode. */
                POWER_EnterDeepPowerDownMode();
                break;
            default:
                break;
        }

        /* restore the active mode configurations */
        DEMO_LowPowerWakeup();
        PRINTF("Wakeup.\r\n");

        if (s_CurrentWakeupSource == DEMO_WAKEUP_CASE_WKT)
        {
            WKT_StopTimer(WKT);
        }
    }
}

static uint32_t DEMO_GetWakeUpSource(power_mode_cfg_t targetPowerMode)
{
    uint32_t ch = 0U;

    switch (targetPowerMode)
    {
        case kPmu_Sleep:
            PRINTF(DEMO_SLEEP_WAKEUP_SOURCE);
            ch = DEMO_GetUserInput(DEMO_SLEEP_WAKEUP_SOURCE_SIZE);
            break;
        case kPmu_Deep_Sleep:
            PRINTF(DEMO_DEEP_SLEEP_WAKEUP_SOURCE);
            ch = DEMO_GetUserInput(DEMO_DEEP_SLEEP_WAKEUP_SOURCE_SIZE);
            break;
        case kPmu_PowerDown:
            PRINTF(DEMO_POWER_DOWN_WAKEUP_SOURCE);
            ch = DEMO_GetUserInput(DEMO_POWER_DOWN_WAKEUP_SOURCE_SIZE);
            break;
        case kPmu_Deep_PowerDown:
            PRINTF(DEMO_DEEP_POWERDOWN_WAKEUP_SOURCE);
            ch = DEMO_GetUserInput(DEMO_DEEP_POWERDOWN_WAKEUP_SOURCE_SIZE);
            break;
    }

    switch (ch)
    {
        case DEMO_WAKEUP_CASE_WKT:
            /* Enable wakeup for wkt. */
            EnableDeepSleepIRQ(WKT_IRQn);
            DisableDeepSleepIRQ(PIN_INT0_IRQn);
            WKT_StartTimer(WKT, USEC_TO_COUNT(DEMO_WKT_TIMEOUT_VALUE, DEMO_WKT_CLK_FREQ));
            break;
        case DEMO_WAKEUP_CASE_WAKEUP:
            /* Enable wakeup for PinInt0. */
            EnableDeepSleepIRQ(PIN_INT0_IRQn);
            DisableDeepSleepIRQ(WKT_IRQn);
            if (targetPowerMode == kPmu_Deep_PowerDown)
            {
                POWER_DPD_ENABLE_WAKEUP_PIN
            }
            break;
        case DEMO_WAKEUP_CASE_RESET:
            if (targetPowerMode == kPmu_Deep_PowerDown)
            {
                POWER_DPD_ENABLE_RESET_PIN
            }
            break;
    }

    return ch;
}

/*
 * Get user selection from UART.
 */
static power_mode_cfg_t DEMO_GetUserSelection(void)
{
    uint32_t ch = 0U;

    PRINTF(
        "\r\nSelect an option\r\n"
        "\t1. Sleep mode\r\n"
        "\t2. Deep Sleep mode\r\n"
        "\t3. Power Down mode\r\n"
        "\t4. Deep power down mode\r\n");

    ch = DEMO_GetUserInput(4);

    switch (ch)
    {
        case 0:
            ch = kPmu_Sleep;
            break;
        case 1:
            ch = kPmu_Deep_Sleep;
            break;
        case 2:
            ch = kPmu_PowerDown;
            break;
        case 3:
            ch = kPmu_Deep_PowerDown;
        default:
            break;
    }
    return (power_mode_cfg_t)ch;
}
