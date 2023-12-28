/*
 * Copyright 2021 NXP
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
#include "fsl_trdc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_SYSTICK_CLK_FREQ (CLOCK_GetFreq(kCLOCK_RtcOscClk) * 6)
#define TEST_SW7_GPIO         GPIOB
#define TEST_SW8_GPIO         GPIOB
#define TEST_NS_GPIOA         GPIOA

#define TEST_SW7_GPIO_PIN 13U
#define TEST_SW8_GPIO_PIN 12U

#define TEST_GPIOA_PIN15 15U
#define TEST_GPIOA_PIN18 18U

#define NON_SECURE_START DEMO_CODE_START_NS
#define SW_PRESSED       0

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_SetTrdcGlobalConfig(void);

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
    App_SetTrdcMBCNSE(0, 3, 0, false);
    App_SetTrdcMBCNSE(0, 3, 1, false);

    TEST_SW7_GPIO->PCNS &= (~(1 << TEST_SW7_GPIO_PIN));
    if (RGPIO_PinRead(TEST_SW7_GPIO, TEST_SW7_GPIO_PIN) != SW_PRESSED)
    {
        RGPIO_PinWrite(TEST_NS_GPIOA, TEST_GPIOA_PIN15, 0);
    }
    else
    {
        RGPIO_PinWrite(TEST_NS_GPIOA, TEST_GPIOA_PIN15, 1);
    }

    /* Control GPIO MASK based on SW8 button press */
    if (RGPIO_PinRead(TEST_SW8_GPIO, TEST_SW8_GPIO_PIN) != SW_PRESSED)
    {
        /*secure access*/
        TEST_SW7_GPIO->PCNS &= (~(1 << TEST_SW7_GPIO_PIN));
    }
    else
    {
        /*non-secure access*/
        TEST_SW7_GPIO->PCNS |= (1 << TEST_SW7_GPIO_PIN);
    }

    App_SetTrdcMBCNSE(0, 3, 0, true);
    App_SetTrdcMBCNSE(0, 3, 1, true);
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Define the init structure for the input switch pin */
    rgpio_pin_config_t sw_config = {
        kRGPIO_DigitalInput,
        0,
    };
    rgpio_pin_config_t output_config = {
        kRGPIO_DigitalOutput,
        0,
    };

    /* Board pin init */
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_EnableClock(kCLOCK_RgpioB);
    CLOCK_EnableClock(kCLOCK_RgpioA);
    App_SetTrdcMBCNSE(0, 3, 0, false);
    App_SetTrdcMBCNSE(0, 3, 1, false);

    RGPIO_PinInit(TEST_SW7_GPIO, TEST_SW7_GPIO_PIN, &sw_config);
    RGPIO_PinInit(TEST_SW8_GPIO, TEST_SW8_GPIO_PIN, &sw_config);
    RGPIO_PinInit(TEST_NS_GPIOA, TEST_GPIOA_PIN15, &output_config);
    RGPIO_PinInit(TEST_NS_GPIOA, TEST_GPIOA_PIN18, &output_config);

    TEST_NS_GPIOA->PCNS |= (1 << TEST_GPIOA_PIN18);

    App_SetTrdcMBCNSE(0, 3, 0, true);
    App_SetTrdcMBCNSE(0, 3, 1, true);

    /* Set systick reload value to generate 5ms interrupt */
    SysTick_Config(USEC_TO_COUNT(500000U, DEMO_SYSTICK_CLK_FREQ));

    /* Call non-secure application - jump to normal world */
    TZM_JumpToNormalWorld(NON_SECURE_START);

    while (1)
    {
        /* This point should never be reached */
    }
}
