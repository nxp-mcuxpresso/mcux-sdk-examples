/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_debug_console.h"
#include "fsl_reset.h"
#include "fsl_upower.h"
#include "fsl_lpuart_cmsis.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_USART                 Driver_USART1
#define EXAMPLE_USART_DMA_BASEADDR DMA0
#define ECHO_BUFFER_LENGTH 8

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

AT_NONCACHEABLE_SECTION_INIT(uint8_t g_tipString[]) =
    "USART EDMA example\r\nSend back received data\r\nEcho every 8 characters\r\n";
AT_NONCACHEABLE_SECTION_INIT(uint8_t g_txBuffer[ECHO_BUFFER_LENGTH]) = {0};
AT_NONCACHEABLE_SECTION_INIT(uint8_t g_rxBuffer[ECHO_BUFFER_LENGTH]) = {0};
volatile bool rxBufferEmpty                                          = true;
volatile bool txBufferFull                                           = false;
volatile bool txOnGoing                                              = false;
volatile bool rxOnGoing                                              = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

uint32_t LPUART1_GetFreq(void)
{
    return CLOCK_GetIpFreq(kCLOCK_Lpuart1);
}


/* USART  callback */
void USART_Callback(uint32_t event)
{
    if (event == ARM_USART_EVENT_SEND_COMPLETE)
    {
        txBufferFull = false;
        txOnGoing    = false;
    }

    if (event == ARM_USART_EVENT_RECEIVE_COMPLETE)
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
    BOARD_BootClockRUN();
    UPOWER_PowerOnMemPart(0U, (uint32_t)kUPOWER_MP1_DMA0);
    CLOCK_EnableClock(kCLOCK_Dma0Ch0);
    CLOCK_EnableClock(kCLOCK_Dma0Ch1);
    edma_config_t edmaConfig = {0};
    EDMA_GetDefaultConfig(&edmaConfig);
    EDMA_Init(EXAMPLE_USART_DMA_BASEADDR, &edmaConfig);
    EDMA_SetChannelMux(EXAMPLE_USART_DMA_BASEADDR, RTE_USART1_DMA_TX_CH, RTE_USART1_DMA_TX_PERI_SEL);
    EDMA_SetChannelMux(EXAMPLE_USART_DMA_BASEADDR, RTE_USART1_DMA_RX_CH, RTE_USART1_DMA_RX_PERI_SEL);
    CLOCK_SetIpSrc(kCLOCK_Lpuart1, kCLOCK_Pcc1BusIpSrcCm33Bus);
    RESET_PeripheralReset(kRESET_Lpuart1);
    if (BOARD_IsLowPowerBootType() != true) /* not low power boot type */
    {
        BOARD_HandshakeWithUboot(); /* Must handshake with uboot, unless will get issues(such as: SoC reset all the
                                       time) */
    }
    else                            /* low power boot type */
    {
        BOARD_SetTrdcGlobalConfig();
    }
    DEMO_USART.Initialize(USART_Callback);
    DEMO_USART.PowerControl(ARM_POWER_FULL);

    /* Set baudrate. */
    DEMO_USART.Control(ARM_USART_MODE_ASYNCHRONOUS, BOARD_DEBUG_UART_BAUDRATE);

    /* Send g_tipString out. */
    txOnGoing = true;

    DEMO_USART.Send(g_tipString, sizeof(g_tipString) - 1);

    /* Wait send finished */
    while (txOnGoing)
    {
    }

    while (1)
    {
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

        /* If g_txBuffer is empty and g_rxBuffer is full, copy g_rxBuffer to g_txBuffer. */
        if ((!rxBufferEmpty) && (!txBufferFull))
        {
            memcpy(g_txBuffer, g_rxBuffer, ECHO_BUFFER_LENGTH);
            rxBufferEmpty = true;
            txBufferFull  = true;
        }
    }
}
