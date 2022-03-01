/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_DEBUG_CONSOLE_SWO_PORT     (0U)
#define DEMO_DEBUG_CONSOLE_SWO_BAUDRATE (4000000U)

#define APP_SW_PORT              BOARD_SW2_GPIO_PORT
#define APP_SW_PIN               BOARD_SW2_GPIO_PIN
#define APP_GPIO_INTA_IRQHandler GPIO_INTA_DriverIRQHandler
#define APP_SW_IRQ               GPIO_INTA_IRQn

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitDebugConsoleSWO(uint32_t port, uint32_t baudrate);
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool g_userPress = false;
volatile bool g_timeOut = false;
volatile uint32_t g_systickCounter = 1000U;
/*******************************************************************************
 * Code
 ******************************************************************************/
void GPIO_INTA_DriverIRQHandler(void)
{
    /* clear the interrupt status */
    GPIO_PinClearInterruptFlag(GPIO, APP_SW_PORT, APP_SW_PIN, 0);
    /* Change state of switch. */
    g_userPress = true;
    SDK_ISR_EXIT_BARRIER;
}

void BOARD_InitKey(void)
{
    /* Define the init structure for the input switch pin */
    gpio_pin_config_t sw_config    = {kGPIO_DigitalInput, 0};
    gpio_interrupt_config_t config = {kGPIO_PinIntEnableEdge, kGPIO_PinIntEnableLowOrFall};

    /* Init input switch GPIO. */
    EnableIRQ(APP_SW_IRQ);
    GPIO_PortInit(GPIO, APP_SW_PORT);
    GPIO_PinInit(GPIO, APP_SW_PORT, APP_SW_PIN, &sw_config);

    /* Enable GPIO pin interrupt */
    GPIO_SetPinInterruptConfig(GPIO, APP_SW_PORT, APP_SW_PIN, &config);
    GPIO_PinEnableInterrupt(GPIO, APP_SW_PORT, APP_SW_PIN, 0);
}


void BOARD_InitDebugConsoleSWO(uint32_t port, uint32_t baudrate)
{
    uint32_t clkSrcFreq = SystemCoreClock;

    DbgConsole_Init(port, baudrate, kSerialPort_Swo, clkSrcFreq);
}
 void SysTick_Handler(void)
{
    if (g_systickCounter != 0U)
    {
        g_systickCounter--;
    }
    else 
    {
        g_systickCounter = 1000U;
        g_timeOut = true;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitKey();
    BOARD_InitDebugConsoleSWO(DEMO_DEBUG_CONSOLE_SWO_PORT, DEMO_DEBUG_CONSOLE_SWO_BAUDRATE);

    /* Set systick reload value to generate 1ms interrupt */
    if (SysTick_Config(SystemCoreClock / 1000U))
    {
        while (1)
        {
        }
    }

    while (1)
    {
        if (g_userPress)
        {
            PRINTF("SWO: hello_world\r\n");
            g_userPress = false;
        }
        if (g_timeOut)
        {
            PRINTF("SWO: timer_trigger\r\n");
            g_timeOut = false;
        }
    }
}
