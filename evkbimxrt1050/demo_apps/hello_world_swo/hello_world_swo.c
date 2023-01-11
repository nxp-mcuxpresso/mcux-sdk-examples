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

#define APP_SW_PORT              BOARD_USER_BUTTON_GPIO
#define APP_SW_PIN               BOARD_USER_BUTTON_GPIO_PIN
#define APP_GPIO_INTA_IRQHandler GPIO5_Combined_0_15_IRQHandler
#define APP_SW_IRQ               GPIO5_Combined_0_15_IRQn

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

void GPIO5_Combined_0_15_IRQHandler(void)
{
    /* clear the interrupt status */
    GPIO_PortClearInterruptFlags(APP_SW_PORT, 1UL << APP_SW_PIN);
    /* Change state of switch. */
    g_userPress = true;
    SDK_ISR_EXIT_BARRIER;
}

void BOARD_InitKey(void)
{
    /* Define the init structure for the input switch pin */
    gpio_pin_config_t sw_config = {kGPIO_DigitalInput, 0, kGPIO_IntFallingEdge};

    /* Init input switch GPIO. */
    GPIO_PinInit(APP_SW_PORT, APP_SW_PIN, &sw_config);

    /* Enable GPIO pin interrupt */
    GPIO_SetPinInterruptConfig(APP_SW_PORT, APP_SW_PIN, kGPIO_IntFallingEdge);
    GPIO_PortEnableInterrupts(APP_SW_PORT, 1UL << APP_SW_PIN);
    EnableIRQ(APP_SW_IRQ);
}


void BOARD_InitDebugConsoleSWO(uint32_t port, uint32_t baudrate)
{
    SystemCoreClockUpdate();
    CLOCK_EnableClock(kCLOCK_Trace);

    /* For RT1050, ETB(embedded trace buffer) FFCR register needs
     * initialization for none-debug mode swo trace output.
     * Please check Serial_SwoInit in fsl_component_serial_port_swo.c
     * for additional coresight register initialization. */
#define ETB_FFCR (*(volatile unsigned int *)0xE0040304)
    ETB_FFCR = 0x100;

    uint32_t clkSrcFreq = CLOCK_GetClockRootFreq(kCLOCK_TraceClkRoot);
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
    BOARD_ConfigMPU();
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
