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

#define BOARD_SW_GPIO        BOARD_SW3_GPIO
#define BOARD_SW_PORT        BOARD_SW3_PORT
#define BOARD_SW_GPIO_PIN    BOARD_SW3_GPIO_PIN
#define BOARD_SW_IRQ         BOARD_SW3_IRQ
#define BOARD_SW_IRQ_HANDLER BOARD_SW3_IRQ_HANDLER
#define BOARD_SW_NAME        BOARD_SW3_NAME

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitDebugConsoleSWO(unsigned int port, unsigned int baudrate);
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool g_userPress          = false;
volatile bool g_timeOut            = false;
volatile uint32_t g_systickCounter = 1000U;
/*******************************************************************************
 * Code
 ******************************************************************************/
extern volatile bool g_userPress;

void BOARD_SW_IRQ_HANDLER(void)
{
    /* Clear external interrupt flag. */
    GPIO_GpioClearInterruptFlags(BOARD_SW_GPIO, 1U << BOARD_SW_GPIO_PIN);

    /* Change state of button. */
    g_userPress = true;
    SDK_ISR_EXIT_BARRIER;
}

void BOARD_InitKey(void)
{
    /* Define the init structure for the input switch pin */
    gpio_pin_config_t sw_config = {
        kGPIO_DigitalInput,
        0,
    };

    /* Init input switch GPIO. */
    GPIO_SetPinInterruptConfig(BOARD_SW_GPIO, BOARD_SW_GPIO_PIN, kGPIO_InterruptFallingEdge);
    EnableIRQ(BOARD_SW_IRQ);
    GPIO_PinInit(BOARD_SW_GPIO, BOARD_SW_GPIO_PIN, &sw_config);
}


void BOARD_InitDebugConsoleSWO(unsigned int port, unsigned int baudrate)
{
    DbgConsole_Init(port, baudrate, kSerialPort_Swo, SystemCoreClock);
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
        g_timeOut        = true;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitKey();

    /* Write KEY value (0xC5ACCE55) to Lock Access Register to unlock the write access to ATB funnel by CPU. */
    (*(unsigned int *)0xE0044FB0) = 0xC5ACCE55;
    /* As documented in the RM, there is a FUNNEL between the CM33 ITM output
     * and the CM3 (radio) ITM output before the SWO pin output.
     * The FUNNEL registers are located on the PPB bus at 0xE0044000.
     * Configure the FUNNEL register to enable the SWO pin output to CM33 ITM output. */
    (*(unsigned int *)0xE0044000) |= 0x1;
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
