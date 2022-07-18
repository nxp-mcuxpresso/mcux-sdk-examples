/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if (__ARM_FEATURE_CMSE & 1) == 0
#error "Need ARMv8-M security extensions"
#elif (__ARM_FEATURE_CMSE & 2) == 0
#error "Compile with --cmse"
#endif

#include "fsl_device_registers.h"
#include "arm_cmse.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "veneer_table.h"
#include "tzm_config.h"
#include "tzm_api.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_SYSTICK_CLK_FREQ      CLOCK_GetFreq(kCLOCK_SystickClk)
#define DEMO_SECURE_SW1_GPIO       SECGPIO
#define DEMO_SECURE_SW1_GPIO_PORT  0U
#define DEMO_SECURE_SW1_GPIO_PIN   25U
#define DEMO_SW1_GPIO              GPIO
#define DEMO_SW1_GPIO_PORT         0U
#define DEMO_SW1_GPIO_PIN          25U
#define DEMO_SW2_GPIO              GPIO
#define DEMO_SW2_GPIO_PORT         0U
#define DEMO_SW2_GPIO_PIN          10U
#define DEMO_SECURE_GPIO_CLOCK     kCLOCK_ShsGpio0
#define DEMO_SECURE_GPIO_RST       kSHSGPIO0_RST_SHIFT_RSTn
#define DEMO_BLUE_LED_PIN_SEC_MASK AHB_SECURE_CTRL_SEC_GPIO_MASK0_PIO0_PIN25_SEC_MASK_MASK

#if (DEMO_CODE_START_NS == 0x08100000U)
#define BOARD_InitTrustZone XIP_BOARD_InitTrustZone
#else
#define BOARD_InitTrustZone RAM_BOARD_InitTrustZone
#endif
#define NON_SECURE_START DEMO_CODE_START_NS

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Application-specific implementation of the SystemInitHook() weak function.
 */
void SystemInitHook(void)
{
    /* The TrustZone should be configured as early as possible after RESET.
     * Therefore it is called from SystemInit() during startup. The SystemInitHook() weak function
     * overloading is used for this purpose.
     */
    BOARD_InitTrustZone();
}

/*!
 * @brief SysTick Handler
 */
void SysTick_Handler(void)
{
    /* Control GPIO MASK based on S2 button press */
    if (GPIO_PinRead(DEMO_SW2_GPIO, DEMO_SW2_GPIO_PORT, DEMO_SW2_GPIO_PIN))
    {
#ifdef DEMO_ENABLE_SW_IN_NORMAL
        DEMO_ENABLE_SW_IN_NORMAL;
#else
        AHB_SECURE_CTRL->SEC_GPIO_MASK0 |= DEMO_BLUE_LED_PIN_SEC_MASK;
#endif
    }
    else
    {
#ifdef DEMO_DISABLE_SW_IN_NORMAL
        DEMO_DISABLE_SW_IN_NORMAL;
#else
        AHB_SECURE_CTRL->SEC_GPIO_MASK0 &= ~DEMO_BLUE_LED_PIN_SEC_MASK;
#endif
    }
    /* Control green LED based on S1 button press */
    if (GPIO_PinRead(DEMO_SECURE_SW1_GPIO, DEMO_SECURE_SW1_GPIO_PORT, DEMO_SECURE_SW1_GPIO_PIN))
    {
        LED_GREEN_OFF();
    }
    else
    {
        LED_GREEN_ON();
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Board pin init */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Initialize secure GPIO for button S1. */
    CLOCK_EnableClock(DEMO_SECURE_GPIO_CLOCK);
    RESET_PeripheralReset(DEMO_SECURE_GPIO_RST);
    GPIO_PinInit(DEMO_SECURE_SW1_GPIO, DEMO_SECURE_SW1_GPIO_PORT, DEMO_SECURE_SW1_GPIO_PIN,
                 &(gpio_pin_config_t){kGPIO_DigitalInput, 0});

    /* Initialize GPIO ports for buttons S1, S2 and RGB LED*/
    GPIO_PortInit(DEMO_SW1_GPIO, DEMO_SW1_GPIO_PORT);
    GPIO_PortInit(DEMO_SW2_GPIO, DEMO_SW2_GPIO_PORT);
    GPIO_PortInit(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_GPIO_PORT);
    GPIO_PortInit(BOARD_LED_GREEN_GPIO, BOARD_LED_GREEN_GPIO_PORT);

    /* Initialize GPIO pin for button S1. */
    GPIO_PinInit(DEMO_SW1_GPIO, DEMO_SW1_GPIO_PORT, DEMO_SW1_GPIO_PIN, &(gpio_pin_config_t){kGPIO_DigitalInput, 0});

    /* Initialize GPIO pin for button S2. */
    GPIO_PinInit(DEMO_SW2_GPIO, DEMO_SW2_GPIO_PORT, DEMO_SW2_GPIO_PIN, &(gpio_pin_config_t){kGPIO_DigitalInput, 0});

    /* Initialize GPIO PINS for RGB LED*/
    LED_RED_INIT(0x0U);
    LED_BLUE_INIT(0x1U);
    LED_GREEN_INIT(0x1U);

    /* Set systick reload value to generate 5ms interrupt */
    SysTick_Config(USEC_TO_COUNT(5000U, DEMO_SYSTICK_CLK_FREQ));

    /* Call non-secure application - jump to normal world */
    TZM_JumpToNormalWorld(NON_SECURE_START);

    while (1)
    {
        /* This point should never be reached */
    }
}
