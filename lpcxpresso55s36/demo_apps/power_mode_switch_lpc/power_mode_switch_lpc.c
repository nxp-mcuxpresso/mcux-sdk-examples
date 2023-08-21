/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_common.h"
#include "fsl_power.h"
#include "fsl_inputmux.h"
#include "fsl_pint.h"
#include "fsl_usart.h"
#include "fsl_gint.h"

#include "fsl_irtc.h"
#include <stdbool.h>
/*******************************************************************************
 * Variables
 ******************************************************************************/
uint32_t excludeFromDS[2];
uint32_t wakeupFromDS[4];
uint32_t excludeFromPD[1];
uint32_t wakeupFromPD[4];
uint32_t excludeFromDPD[1];
uint32_t wakeupFromDPD[2];

const char *g_wakeupInfoStr[] = {"Sleep [Press the user key to wakeup]", "Deep Sleep [Press the user key to wakeup]",
                                 "Powerdown [Reset to wakeup]", "Deep Powerdown [Reset to wakeup]"};
uint32_t g_currentPowerMode;

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_WAKEUP_WITH_GINT            (1)
#define APP_USART_RX_ERROR               kUSART_RxError
#define APP_RUNNING_INTERNAL_CLOCK       BOARD_BootClockFRO12M()
#define APP_USER_WAKEUP_KEY_NAME         "SW1"
#define APP_USER_WAKEUP_KEY_GPIO         BOARD_SW1_GPIO
#define APP_USER_WAKEUP_KEY_PORT         BOARD_SW1_GPIO_PORT
#define APP_USER_WAKEUP_KEY_PIN          BOARD_SW1_GPIO_PIN
#define APP_USER_WAKEUP_KEY_INPUTMUX_SEL kINPUTMUX_GpioPort1Pin18ToPintsel

#define DEMO_GINT0_PORT kGINT_Port0

/* Select one input, active low for GINT0 */
#define DEMO_GINT0_POL_MASK ~(1U << BOARD_SW3_GPIO_PIN)
#define DEMO_GINT0_ENA_MASK (1U << BOARD_SW3_GPIO_PIN)

#define APP_EXCLUDE_FROM_DEEPSLEEP (excludeFromDS)

#define APP_EXCLUDE_FROM_POWERDOWN (excludeFromPD)

#define APP_EXCLUDE_FROM_DEEPPOWERDOWN (excludeFromDPD)

#define APP_WAKEUP_FROM_DEEPSLEEP (wakeupFromDS)

#define APP_WAKEUP_FROM_POWERDOWN (wakeupFromPD)

#define APP_WAKEUP_FROM_DEEPPOWERDOWN (wakeupFromDPD)

#define APP_SYSCON_STARTER_MASK SYSCON_STARTERSET_GPIO_INT00_SET_MASK
//#define INPUTMUX INPUTMUX0

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void DEMO_PreLowPower(void);
void DEMO_PowerDownWakeup(void);
void DEMO_PreDeepPowerDown(void);
static uint32_t APP_GetUserSelection(void);
static void APP_InitWakeupPin(void);
static void pint_intr_callback(pint_pin_int_t pintr, uint32_t pmatch_status);

/*******************************************************************************
 * Code
 ******************************************************************************/
void RTC_IRQHandler(void)
{
}
void DEMO_PreLowPower(void)
{
    /*!< Nothing to do */
}
void DEMO_PowerDownWakeup(void)
{
    BOARD_BootClockFRO12M();
    BOARD_InitDebugConsole();
}
void DEMO_PreDeepPowerDown(void)
{
    uint32_t sec  = 0;
    uint8_t index = 0;
    irtc_datetime_t date;
    irtc_config_t irtcConfig;

    /* Set a start date time and start RT */
    date.year    = 2021U;
    date.month   = 12U;
    date.day     = 25U;
    date.weekDay = 6U;
    date.hour    = 19U;
    date.minute  = 0;
    date.second  = 0;

    /* Get alarm time from user */
    PRINTF("Please input the number of second to wait for waking up\r\n");
    PRINTF("The second must be positive value(1-59)\r\n");
    while (index != 0x0D)
    {
        index = GETCHAR();
        if ((index >= '0') && (index <= '9'))
        {
            PUTCHAR(index);
            sec = sec * 10 + (index - 0x30U);
        }
    }
    PRINTF("\r\n");

    POWER_EnablePD(kPDRUNCFG_PD_XTAL32K); /*!< Powered down the XTAL 32 kHz RTC oscillator */
    POWER_DisablePD(kPDRUNCFG_PD_FRO32K); /*!< Powered the FRO 32 kHz RTC oscillator */
    CLOCK_AttachClk(kFRO32K_to_OSC32K);   /*!< Switch OSC32K to FRO32K */

    IRTC_GetDefaultConfig(&irtcConfig);

    /* Init RTC */
    IRTC_Init(RTC, &irtcConfig);

    /* Enable the RTC 32KHz oscillator at CFG0 by writing a 0 */
    IRTC_Enable32kClkDuringRegisterWrite(RTC, true);

    /* Clear all Tamper events by writing a 1 to the bits */
    IRTC_ClearTamperStatusFlag(RTC);

    IRTC_SetDatetime(RTC, &date);
    date.second += sec;
    IRTC_SetAlarm(RTC, &date);

    /* Enable RTC alarm interrupt */
    IRTC_EnableInterrupts(RTC, kIRTC_AlarmInterruptEnable);

    /* Enable at the NVIC */
    EnableIRQ(RTC_IRQn);

    PRINTF("System will wakeup at%02ds later\r\n", sec);
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    CLOCK_EnableClock(kCLOCK_Gpio0); /* Enable the clock for GPIO0. */

    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    manage_evk_io_optimization();
    BOARD_BootClockFRO12M();
    BOARD_InitDebugConsole();
#if !defined(DONOT_ENABLE_FLASH_PREFETCH)
    /* enable flash prefetch for better performance */
    SYSCON->FMCCR |= SYSCON_FMCCR_PREFEN_MASK;
#endif

    excludeFromDS[0]  = kPDRUNCFG_PD_DCDC | kPDRUNCFG_PD_FRO192M | kPDRUNCFG_PD_FRO32K;
    excludeFromDS[1]  = 0;
    excludeFromPD[0]  = kPDRUNCFG_PD_LDOMEM | kPDRUNCFG_PD_FRO32K;
    excludeFromDPD[0] = kPDRUNCFG_PD_LDOMEM | kPDRUNCFG_PD_FRO32K;

    wakeupFromDS[0]  = WAKEUP_GPIO_INT0_0;
    wakeupFromDS[1]  = 0;
    wakeupFromDS[2]  = 0;
    wakeupFromDS[3]  = 0;
    wakeupFromPD[0]  = WAKEUP_GPIO_GLOBALINT0 | WAKEUP_GPIO_GLOBALINT1;
    wakeupFromPD[1]  = 0;
    wakeupFromPD[2]  = 0;
    wakeupFromPD[3]  = 0;
    wakeupFromDPD[0] = WAKEUP_RTC_ALARM_WAKEUP;
    wakeupFromDPD[1] = 0;

    /* Running 12 MHz to Core*/
    APP_RUNNING_INTERNAL_CLOCK;

    /* Attach Main Clock as CLKOUT */
    CLOCK_AttachClk(kMAIN_CLK_to_CLKOUT);

    APP_InitWakeupPin();

    PRINTF("Power Manager Demo for LPC device.\r\n");
    PRINTF("The \"user key\" is: %s\r\n", APP_USER_WAKEUP_KEY_NAME);

    while (1)
    {
        g_currentPowerMode = APP_GetUserSelection();
        PRINTF("Entering %s ...\r\n", g_wakeupInfoStr[g_currentPowerMode]);

        /* Enter the low power mode. */
        switch (g_currentPowerMode)
        {
            case kPmu_Sleep: /* Enter sleep mode. */
                POWER_EnterSleep();
                break;
            case kPmu_Deep_Sleep: /* Enter deep sleep mode. */
                POWER_EnterDeepSleep(APP_EXCLUDE_FROM_DEEPSLEEP, 0x0, APP_WAKEUP_FROM_DEEPSLEEP, 0x0);
                break;
            case kPmu_PowerDown: /* Enter power down mode. */
                PRINTF(
                    "Press any key to confirm to enter the power down mode and wakeup the device by press sw3 key on "
                    "board.\r\n\r\n");
                GETCHAR();
                DEMO_PreLowPower();
                POWER_EnterPowerDown(APP_EXCLUDE_FROM_POWERDOWN, kPOWER_SRAM_PDWN_MASK, APP_WAKEUP_FROM_POWERDOWN,
                                     0x20000000);
                DEMO_PowerDownWakeup();
                APP_InitWakeupPin();
                break;
            case kPmu_Deep_PowerDown: /* Enter deep power down mode. */
                PRINTF(
                    "Press any key to confirm to enter the deep power down mode and wakeup the device by "
                    "reset.\r\n\r\n");
                GETCHAR();
                DEMO_PreDeepPowerDown();
                POWER_EnterDeepPowerDown(APP_EXCLUDE_FROM_DEEPPOWERDOWN, kPOWER_SRAM_DPWD_MASK,
                                         APP_WAKEUP_FROM_DEEPPOWERDOWN, 0);
                break;
            default:
                break;
        }

        PRINTF("Wakeup.\r\n");
    }
}

/*!
 * @brief Call back for GINT0 event
 */
void gint0_callback(void)
{
    PRINTF("Gin event occurs\r\n");
}

/*
 * Setup a GPIO input pin as wakeup source.
 */
static void APP_InitWakeupPin(void)
{
    gpio_pin_config_t gpioPinConfigStruct;

    /* Set SW pin as GPIO input. */
    gpioPinConfigStruct.pinDirection = kGPIO_DigitalInput;
    GPIO_PinInit(APP_USER_WAKEUP_KEY_GPIO, APP_USER_WAKEUP_KEY_PORT, APP_USER_WAKEUP_KEY_PIN, &gpioPinConfigStruct);

    /* Configure the Input Mux block and connect the trigger source to PinInt channle. */
    INPUTMUX_Init(INPUTMUX);
    INPUTMUX_AttachSignal(INPUTMUX, kPINT_PinInt0, APP_USER_WAKEUP_KEY_INPUTMUX_SEL); /* Using channel 0. */
    INPUTMUX_Deinit(INPUTMUX); /* Turnoff clock to inputmux to save power. Clock is only needed to make changes */

    /* Configure the interrupt for SW pin. */
    PINT_Init(PINT);
    PINT_PinInterruptConfig(PINT, kPINT_PinInt0, kPINT_PinIntEnableFallEdge, pint_intr_callback);
    PINT_EnableCallback(PINT); /* Enable callbacks for PINT */

    /* Enable wakeup for PinInt0. */
    EnableDeepSleepIRQ(PIN_INT0_IRQn); /* GPIO pin interrupt 0 wake-up. */

    /* Initialize GINT0*/
    GINT_Init(GINT0);

    /* Setup GINT0 for edge trigger, "OR" mode */
    GINT_SetCtrl(GINT0, kGINT_CombineOr, kGINT_TrigEdge, gint0_callback);

    /* Select pins & polarity for GINT0 */
    GINT_ConfigPins(GINT0, DEMO_GINT0_PORT, DEMO_GINT0_POL_MASK, DEMO_GINT0_ENA_MASK);

    /* Enable callbacks for GINT0 & GINT1 */
    GINT_EnableCallback(GINT0);
}

/*
 * Callback function when wakeup key is pressed.
 */
static void pint_intr_callback(pint_pin_int_t pintr, uint32_t pmatch_status)
{
    PRINTF("Pin event occurs\r\n");
}

/*
 * Get user selection from UART.
 */
static uint32_t APP_GetUserSelection(void)
{
    uint32_t ch;

    /* Clear rx overflow error once it happens during low power mode. */
    if (APP_USART_RX_ERROR == (APP_USART_RX_ERROR & USART_GetStatusFlags((USART_Type *)BOARD_DEBUG_UART_BASEADDR)))
    {
        USART_ClearStatusFlags((USART_Type *)BOARD_DEBUG_UART_BASEADDR, APP_USART_RX_ERROR);
    }

    PRINTF(
        "Select an option\r\n"
        "\t1. Sleep mode\r\n"
        "\t2. Deep Sleep mode\r\n"
        "\t3. power down mode\r\n"
        "\t4. Deep power down mode\r\n");
    while (1)
    {
        ch = GETCHAR();
        if ((ch < '1') || (ch > '4')) /* Only '1', '2', '3' , '4'. */
        {
            continue;
        }
        else
        {
            ch = ch - '1'; /* Only 0, 1, 2 (,3). */
            break;
        }
    }
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
            break;
    }
    return ch;
}
