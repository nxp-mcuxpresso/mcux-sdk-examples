/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_rtc.h"
#include "fsl_usart.h"
#include "fsl_power.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_USART                 USART0
#define DEMO_USART_CLK_SRC         kCLOCK_Flexcomm0
#define DEMO_USART_CLK_FREQ        CLOCK_GetFlexCommClkFreq(0)
#define DEMO_USART_IRQHandler      FLEXCOMM0_IRQHandler
#define DEMO_USART_IRQn            FLEXCOMM0_IRQn
#define APP_EXCLUDE_FROM_DEEPSLEEP (kPDRUNCFG_PD_FRO192M | kPDRUNCFG_PD_FRO32K)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t ch;
bool deepsleep = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_SwitchBackClockSrc()
{
	BOARD_BootClockFROHF96M();
}
void DEMO_USART_IRQHandler(void)
{
    /* If new data arrived. */
    if (((kUSART_RxFifoNotEmptyFlag | kUSART_RxError) & USART_GetStatusFlags(DEMO_USART)) != 0U)
    {
        ch = USART_ReadByte(DEMO_USART);
        PRINTF("Received %c\r\n", ch);
        if (deepsleep)
        {
            deepsleep = false;
            PRINTF(
                "Waking up from deep sleep, please change the baudrate setting of your local terminal back to %d\r\n",
                BOARD_DEBUG_UART_BAUDRATE);
            /* After recover from deepsleep, switch back to former clock source. */
            BOARD_SwitchBackClockSrc();
            PRINTF("\r\nPress 1 to enter deep sleep\r\nPress any other key to wake up soc\r\n");
            USART_Enable32kMode(DEMO_USART, BOARD_DEBUG_UART_BAUDRATE, false, DEMO_USART_CLK_FREQ);
        }
        else if (ch == 0x31)
        {
            PRINTF("Entering deep sleep mode, please change the baudrate setting of your local terminal to 9600\r\n");
            USART_Enable32kMode(DEMO_USART, 9600U, true, DEMO_USART_CLK_FREQ);
            deepsleep = true;
        }
        else
        {
            PRINTF("\r\nPress 1 to enter deep sleep\r\nPress any other key to wake up soc\r\n");
        }
    }
    SDK_ISR_EXIT_BARRIER;
}

/* Initialzie the USART module. */
static void EXAMPLE_InitUSART(void)
{
    usart_config_t config;

    /* Initialize the UART.
     * config.baudRate_Bps = 115200U;
     * config.parityMode = kUART_ParityDisabled;
     * config.stopBitCount = kUART_OneStopBit;
     * config.txFifoWatermark = 0;
     * config.rxFifoWatermark = 1;
     * config.enableTx = false;
     * config.enableRx = false;
     */
    USART_GetDefaultConfig(&config);
    config.baudRate_Bps = BOARD_DEBUG_UART_BAUDRATE;
    config.enableTx     = true;
    config.enableRx     = true;

    USART_Init(DEMO_USART, &config, DEMO_USART_CLK_FREQ);
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Ungate the RTC clock and enables the RTC oscillator */
    RTC_Init(RTC);

    EXAMPLE_InitUSART();

    /* Enable RX interrupt. */
    USART_EnableInterrupts(DEMO_USART,
                           (uint32_t)kUSART_RxLevelInterruptEnable | (uint32_t)kUSART_RxErrorInterruptEnable);
    EnableIRQ(DEMO_USART_IRQn);

    PRINTF(
        "Usart waking up soc from deep sleep example, please note USART can only work at 9600 baudrate in deep sleep "
        "mode\r\n");
    PRINTF("\r\nPress 1 to enter deep sleep\r\nPress any other key to wake up soc\r\n");

    while (1)
    {
        if (deepsleep == true)
        {
            /* Before entering deep sleep, change core clock source to FRO since it does not power doem in deep sleep. */
            BOARD_BootClockFRO12M();
#if (defined(FSL_FEATURE_POWERLIB_EXTEND) && FSL_FEATURE_POWERLIB_EXTEND)
            POWER_EnterDeepSleep(APP_EXCLUDE_FROM_DEEPSLEEP, 0x0, WAKEUP_FLEXCOMM0, 0x0);
#else
            POWER_EnterDeepSleep(APP_EXCLUDE_FROM_DEEPSLEEP);
#endif
        }
    }
}
