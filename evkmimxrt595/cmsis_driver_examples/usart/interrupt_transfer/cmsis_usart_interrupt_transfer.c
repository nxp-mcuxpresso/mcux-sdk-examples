/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "board.h"

#include "fsl_usart_cmsis.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_USART Driver_USART0
#define ECHO_BUFFER_LENGTH 8
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* USART user SignalEvent */
void USART_SignalEvent_t(uint32_t event);
/*******************************************************************************
 * Variables
 ******************************************************************************/
const uint8_t g_tipString[] =
    "USART CMSIS interrupt example\r\nBoard receives 8 characters then sends them out\r\nNow please input:\r\n";
uint8_t g_txBuffer[ECHO_BUFFER_LENGTH] = {0};
uint8_t g_rxBuffer[ECHO_BUFFER_LENGTH] = {0};
volatile bool rxBufferEmpty            = true;
volatile bool txBufferFull             = false;
volatile bool txOnGoing                = false;
volatile bool rxOnGoing                = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

uint32_t USART0_GetFreq(void)
{
    return CLOCK_GetFlexcommClkFreq(0);
}
uint32_t USART1_GetFreq(void)
{
    return CLOCK_GetFlexcommClkFreq(1);
}
uint32_t USART2_GetFreq(void)
{
    return CLOCK_GetFlexcommClkFreq(2);
}
uint32_t USART3_GetFreq(void)
{
    return CLOCK_GetFlexcommClkFreq(3);
}
uint32_t USART4_GetFreq(void)
{
    return CLOCK_GetFlexcommClkFreq(4);
}
uint32_t USART5_GetFreq(void)
{
    return CLOCK_GetFlexcommClkFreq(5);
}
void USART_SignalEvent_t(uint32_t event)
{
    if (ARM_USART_EVENT_SEND_COMPLETE == event)
    {
        txBufferFull = false;
        txOnGoing    = false;
    }

    if (ARM_USART_EVENT_RECEIVE_COMPLETE == event)
    {
        rxBufferEmpty = false;
        rxOnGoing     = false;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t i;

    /* attach FRG0 clock to FLEXCOMM0 (debug console) */
    CLOCK_SetFRGClock(BOARD_DEBUG_UART_FRG_CLK);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    USART0_InitPins();
    BOARD_BootClockRUN();

    DEMO_USART.Initialize(USART_SignalEvent_t);
    DEMO_USART.PowerControl(ARM_POWER_FULL);

    /* Set baudrate. */
    DEMO_USART.Control(ARM_USART_MODE_ASYNCHRONOUS, BOARD_DEBUG_UART_BAUDRATE);

    txOnGoing = true;
    DEMO_USART.Send(g_tipString, sizeof(g_tipString) - 1);

    /* Wait send finished */
    while (txOnGoing)
    {
    }

    while (1)
    {
        /* If g_txBuffer is empty and g_rxBuffer is full, copy g_rxBuffer to g_txBuffer. */
        if ((!rxBufferEmpty) && (!txBufferFull))
        {
            memcpy(g_txBuffer, g_rxBuffer, ECHO_BUFFER_LENGTH);
            rxBufferEmpty = true;
            txBufferFull  = true;
        }

        /* If RX is idle and g_rxBuffer is empty, start to read data to g_rxBuffer. */
        if ((!rxOnGoing) && rxBufferEmpty)
        {
            rxOnGoing = true;
            DEMO_USART.Receive(g_rxBuffer, ECHO_BUFFER_LENGTH);
        }

        /* If TX is idle and g_txBuffer is full, start to send data. */
        if ((!txOnGoing) && txBufferFull)
        {
            txOnGoing = true;
            DEMO_USART.Send(g_txBuffer, ECHO_BUFFER_LENGTH);
        }

        /* Delay some time, simulate the app is processing other things, input data save to ring buffer. */
        i = 0x10000U;
        while (i--)
        {
            __NOP();
        }
    }
}
