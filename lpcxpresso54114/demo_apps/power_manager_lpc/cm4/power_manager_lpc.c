/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
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
#if (defined(FSL_FEATURE_POWERLIB_EXTEND) && FSL_FEATURE_POWERLIB_EXTEND)
#include "fsl_gint.h"
#endif

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define APP_USART_RX_ERROR               kUSART_RxError
#define APP_RUNNING_INTERNAL_CLOCK       BOARD_BootClockFRO12M()
#define APP_USER_WAKEUP_KEY_NAME         "SW1"
#define APP_USER_WAKEUP_KEY_GPIO         BOARD_SW1_GPIO
#define APP_USER_WAKEUP_KEY_PORT         BOARD_SW1_GPIO_PORT
#define APP_USER_WAKEUP_KEY_PIN          BOARD_SW1_GPIO_PIN
#define APP_USER_WAKEUP_KEY_INPUTMUX_SEL kINPUTMUX_GpioPort0Pin24ToPintsel

#define APP_EXCLUDE_FROM_SLEEP                                                                             \
    (SYSCON_PDRUNCFG_PDEN_SRAMX_MASK | SYSCON_PDRUNCFG_PDEN_SRAM0_MASK | SYSCON_PDRUNCFG_PDEN_SRAM1_MASK | \
     SYSCON_PDRUNCFG_PDEN_SRAM2_MASK)

#define APP_EXCLUDE_FROM_DEEPSLEEP                                                                         \
    (SYSCON_PDRUNCFG_PDEN_SRAMX_MASK | SYSCON_PDRUNCFG_PDEN_SRAM0_MASK | SYSCON_PDRUNCFG_PDEN_SRAM1_MASK | \
     SYSCON_PDRUNCFG_PDEN_SRAM2_MASK)

#define APP_SYSCON_STARTER_MASK SYSCON_STARTERP_PIN_INT0_MASK
const char *gWakeupInfoStr[] = {"Sleep [Press the user key to wakeup]", "Deep Sleep [Press the user key to wakeup]",
#if (defined(FSL_FEATURE_SYSCON_HAS_POWERDOWN_MODE) && FSL_FEATURE_SYSCON_HAS_POWERDOWN_MODE)
                                "Powerdown [Reset to wakeup]", "Deep Powerdown [Reset to wakeup]"};
#else
                                "Deep Powerdown [Reset to wakeup]", "Deep Powerdown [Reset to wakeup]"};
#endif
uint32_t gCurrentPowerMode;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static uint32_t APP_GetUserSelection(void);
static void APP_InitWakeupPin(void);
static void pint_intr_callback(pint_pin_int_t pintr, uint32_t pmatch_status);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    CLOCK_EnableClock(kCLOCK_Gpio0); /* Enable the clock for GPIO0. */

    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Running 12 MHz to Core*/
    APP_RUNNING_INTERNAL_CLOCK;

    /* Attach Main Clock as CLKOUT */
    CLOCK_AttachClk(kMAIN_CLK_to_CLKOUT);

    APP_InitWakeupPin();

    PRINTF("Power Manager Demo for LPC device.\r\n");
    PRINTF("The \"user key\" is: %s\r\n", APP_USER_WAKEUP_KEY_NAME);

    while (1)
    {
        gCurrentPowerMode = APP_GetUserSelection();
        PRINTF("Entering %s ...\r\n", gWakeupInfoStr[gCurrentPowerMode]);

        /* Enter the low power mode. */
        switch (gCurrentPowerMode)
        {
            case kPmu_Sleep: /* Enter sleep mode. */
                POWER_EnterSleep();
                break;
            case kPmu_Deep_Sleep: /* Enter deep sleep mode. */
#if (defined(FSL_FEATURE_POWERLIB_EXTEND) && FSL_FEATURE_POWERLIB_EXTEND)
                POWER_EnterDeepSleep(APP_EXCLUDE_FROM_DEEPSLEEP, 0x0, WAKEUP_GPIO_INT0_0, 0x0);
#else
                POWER_EnterDeepSleep(APP_EXCLUDE_FROM_DEEPSLEEP);
#endif
                break;
#if (defined(FSL_FEATURE_SYSCON_HAS_POWERDOWN_MODE) && FSL_FEATURE_SYSCON_HAS_POWERDOWN_MODE)
            case kPmu_PowerDown: /* Enter power down mode. */
                PRINTF(
                    "Press any key to confirm to enter the power down mode and wakeup the device by press s1 key on "
                    "board.\r\n\r\n");
                GETCHAR();

#if (defined(FSL_FEATURE_POWERLIB_EXTEND) && FSL_FEATURE_POWERLIB_EXTEND)
#if (defined(DEMO_WAKEUP_WITH_GINT) && DEMO_WAKEUP_WITH_GINT)
                DEMO_PreLowPower();
                POWER_EnterPowerDown(APP_EXCLUDE_FROM_POWERDOWN, 0x7FFF,
                                     WAKEUP_GPIO_GLOBALINT0 | WAKEUP_GPIO_GLOBALINT1, 1);
                DEMO_PowerDownWakeup();
                APP_InitWakeupPin();
#endif
#else
                POWER_EnterPowerDown(APP_EXCLUDE_FROM_POWERDOWN);
#endif
                break;
#endif
            case kPmu_Deep_PowerDown: /* Enter deep power down mode. */
                PRINTF(
                    "Press any key to confirm to enter the deep power down mode and wakeup the device by "
                    "reset.\r\n\r\n");
                GETCHAR();
#if (defined(FSL_FEATURE_POWERLIB_EXTEND) && FSL_FEATURE_POWERLIB_EXTEND)
                DEMO_PreDeepPowerDown();
                POWER_EnterDeepPowerDown(APP_EXCLUDE_FROM_DEEPPOWERDOWN,
                                         LOWPOWER_SRAMRETCTRL_RETEN_RAMX2 | LOWPOWER_SRAMRETCTRL_RETEN_RAMX3,
                                         WAKEUP_RTC_LITE_ALARM_WAKEUP, 1);
#else
                POWER_EnterDeepPowerDown(APP_EXCLUDE_FROM_DEEPSLEEP);
#endif
                break;
            default:
                break;
        }

        PRINTF("Wakeup.\r\n");
    }
}

#if (defined(DEMO_WAKEUP_WITH_GINT) && DEMO_WAKEUP_WITH_GINT)
/*!
 * @brief Call back for GINT0 event
 */
void gint0_callback(void)
{
    PRINTF("Gin event occurs\r\n");
}
#endif

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

#if !(defined(FSL_FEATURE_POWERLIB_EXTEND) && FSL_FEATURE_POWERLIB_EXTEND)
    /* Enable wakeup for PinInt0. */
    SYSCON->STARTERSET[0] |= APP_SYSCON_STARTER_MASK; /* GPIO pin interrupt 0 wake-up. */
#endif

    /* Configure the interrupt for SW pin. */
    PINT_Init(PINT);
    PINT_PinInterruptConfig(PINT, kPINT_PinInt0, kPINT_PinIntEnableFallEdge, pint_intr_callback);
    PINT_EnableCallback(PINT); /* Enable callbacks for PINT */

#if !(defined(FSL_FEATURE_POWERLIB_EXTEND) && FSL_FEATURE_POWERLIB_EXTEND)
    EnableDeepSleepIRQ(PIN_INT0_IRQn);
#endif

#if (defined(DEMO_WAKEUP_WITH_GINT) && DEMO_WAKEUP_WITH_GINT)
    /* Initialize GINT0*/
    GINT_Init(GINT0);

    /* Setup GINT0 for edge trigger, "OR" mode */
    GINT_SetCtrl(GINT0, kGINT_CombineOr, kGINT_TrigEdge, gint0_callback);

    /* Select pins & polarity for GINT0 */
    GINT_ConfigPins(GINT0, DEMO_GINT0_PORT, DEMO_GINT0_POL_MASK, DEMO_GINT0_ENA_MASK);

    /* Enable callbacks for GINT0 & GINT1 */
    GINT_EnableCallback(GINT0);
#endif
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
#if (defined(FSL_FEATURE_SYSCON_HAS_POWERDOWN_MODE) && FSL_FEATURE_SYSCON_HAS_POWERDOWN_MODE)
        "\t3. power down mode\r\n"
        "\t4. Deep power down mode\r\n");
#else
        "\t3. Deep power down mode\r\n");
#endif
    while (1)
    {
        ch = GETCHAR();
#if (defined(FSL_FEATURE_SYSCON_HAS_POWERDOWN_MODE) && FSL_FEATURE_SYSCON_HAS_POWERDOWN_MODE)
        if ((ch < '1') || (ch > '4')) /* Only '1', '2', '3' , '4'. */
#else
        if ((ch < '1') || (ch > '3')) /* Only '1', '2', '3'. */
#endif
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
#if (defined(FSL_FEATURE_SYSCON_HAS_POWERDOWN_MODE) && FSL_FEATURE_SYSCON_HAS_POWERDOWN_MODE)
        case 2:
            ch = kPmu_PowerDown;
            break;
        case 3:
            ch = kPmu_Deep_PowerDown;
            break;
#else
        case 2:
            ch = kPmu_Deep_PowerDown;
            break;
#endif
    }
    return ch;
}
