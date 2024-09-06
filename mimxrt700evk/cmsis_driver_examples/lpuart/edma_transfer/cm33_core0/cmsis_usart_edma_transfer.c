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

#include "fsl_lpuart_cmsis.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_USART                 Driver_USART0
#define EXAMPLE_USART_DMA_BASEADDR DMA0
#define EXAMPLE_DMA_CLOCK          kCLOCK_Dma0
#define DEMO_USART_CLK_FREQ       CLOCK_GetLPFlexCommClkFreq(0u)
#define DEMO_USART_TX_EDMA_CHANNEL kDmaRequestMuxLpFlexcomm0Tx
#define DEMO_USART_RX_EDMA_CHANNEL kDmaRequestMuxLpFlexcomm0Rx
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
edma_config_t edmaConfig            = {0};
edma_channel_config_t channelConfig = {
    .enableMasterIDReplication = true,
    .securityLevel             = kEDMA_ChannelSecurityLevelSecure,
    .protectionLevel           = kEDMA_ChannelProtectionLevelPrivileged,
};

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

uint32_t LPUART0_GetFreq()
{
    return DEMO_USART_CLK_FREQ;
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
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();

    /* attach FC0 clock to LP_FLEXCOMM (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_FCCLK_ATTACH);
    CLOCK_SetClkDiv(BOARD_DEBUG_UART_FCCLK_DIV, 1U);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    EDMA_GetDefaultConfig(&edmaConfig);
    edmaConfig.enableMasterIdReplication           = true;
    edmaConfig.channelConfig[RTE_USART0_DMA_TX_CH] = &channelConfig;
    edmaConfig.channelConfig[RTE_USART0_DMA_RX_CH] = &channelConfig;
    EDMA_Init(EXAMPLE_USART_DMA_BASEADDR, &edmaConfig);
#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    EDMA_SetChannelMux(EXAMPLE_USART_DMA_BASEADDR, RTE_USART0_DMA_TX_CH, RTE_USART0_DMA_TX_PERI_SEL);
    EDMA_SetChannelMux(EXAMPLE_USART_DMA_BASEADDR, RTE_USART0_DMA_RX_CH, RTE_USART0_DMA_RX_PERI_SEL);
#endif
    EDMA_EnableRequest(EXAMPLE_USART_DMA_BASEADDR, DEMO_USART_TX_EDMA_CHANNEL);
    EDMA_EnableRequest(EXAMPLE_USART_DMA_BASEADDR, DEMO_USART_RX_EDMA_CHANNEL);
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
