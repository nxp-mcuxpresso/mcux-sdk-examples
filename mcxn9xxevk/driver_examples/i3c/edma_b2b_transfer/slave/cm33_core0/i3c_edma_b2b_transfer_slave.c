/*
 * Copyright 2022-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*  Standard C Included Files */
#include <string.h>
/*  SDK Included Files */
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_i3c_edma.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_SLAVE              I3C1
#define I3C_SLAVE_CLOCK_FREQUENCY  CLOCK_GetFreq(kCLOCK_Clk1M)
#define I3C_TIME_OUT_INDEX         100000000
#define I3C_MASTER_SLAVE_ADDR_7BIT 0x1EU
#define I3C_DATA_LENGTH            34U

#define EXAMPLE_DMA                    DMA0
#define EXAMPLE_I3C_TX_DMA_CHANNEL     (0U)
#define EXAMPLE_I3C_RX_DMA_CHANNEL     (1U)
#define EXAMPLE_I3C_TX_DMA_CHANNEL_MUX (kDma0RequestMuxI3c1Tx)
#define EXAMPLE_I3C_RX_DMA_CHANNEL_MUX (kDma0RequestMuxI3c1Rx)

#ifndef I3C_MASTER_SLAVE_ADDR_7BIT
#define I3C_MASTER_SLAVE_ADDR_7BIT 0x1EU
#endif

#ifndef I3C_DATA_LENGTH
#define I3C_DATA_LENGTH 33U
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
i3c_slave_edma_handle_t g_i3c_s_handle;
edma_handle_t g_tx_dma_handle;
edma_handle_t g_rx_dma_handle;
AT_NONCACHEABLE_SECTION(uint8_t g_slave_rxBuff[I3C_DATA_LENGTH]);
volatile bool g_slaveCompletionFlag  = false;
volatile bool g_slaveRequestSentFlag = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

static void i3c_slave_callback(I3C_Type *base, i3c_slave_edma_transfer_t *xfer, void *userData)
{
    switch ((uint32_t)xfer->event)
    {
        /*  Transfer done */
        case kI3C_SlaveCompletionEvent:
            if (xfer->completionStatus == kStatus_Success)
            {
                g_slaveCompletionFlag = true;
            }
            break;

        case kI3C_SlaveRequestSentEvent:
            g_slaveRequestSentFlag = true;
            break;

#if defined(I3C_ASYNC_WAKE_UP_INTR_CLEAR)
        /*  Handle async wake up interrupt on specific platform. */
        case kI3C_SlaveAddressMatchEvent:
            I3C_ASYNC_WAKE_UP_INTR_CLEAR
            break;
#endif

        default:
            break;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t eventMask = kI3C_SlaveCompletionEvent;
#if defined(I3C_ASYNC_WAKE_UP_INTR_CLEAR)
    eventMask |= kI3C_SlaveAddressMatchEvent;
#endif
    i3c_slave_edma_transfer_t slaveXfer;
    i3c_slave_config_t slaveConfig;
    edma_config_t config;
    status_t result;

    RESET_PeripheralReset(kDMA0_RST_SHIFT_RSTn);

    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* Attach PLL0 clock to I3C, 150MHz / 6 = 25MHz. */
    CLOCK_SetClkDiv(kCLOCK_DivI3c1FClk, 6U);
    CLOCK_AttachClk(kPLL0_to_I3C1FCLK);

    CLOCK_SetClkDiv(kCLOCK_DivI3c1FClkS, 1U);
    CLOCK_AttachClk(kCLK_1M_to_I3C1FCLKS);

    /* Enable 1MHz clock. */
    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_FRO1MHZ_CLK_ENA_MASK;

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("\r\nI3C board2board EDMA example -- Slave transfer.\r\n");

    I3C_SlaveGetDefaultConfig(&slaveConfig);
    slaveConfig.staticAddr = I3C_MASTER_SLAVE_ADDR_7BIT;
    slaveConfig.vendorID   = 0x123U;
    slaveConfig.offline    = false;
    I3C_SlaveInit(EXAMPLE_SLAVE, &slaveConfig, I3C_SLAVE_CLOCK_FREQUENCY);

    PRINTF("\r\nCheck I3C master I3C SDR transfer.\r\n");

    /* Create I3C DMA tx/rx handle. */
    EDMA_GetDefaultConfig(&config);
#if defined(BOARD_GetEDMAConfig)
    BOARD_GetEDMAConfig(config);
#endif
    EDMA_Init(EXAMPLE_DMA, &config);
    EDMA_CreateHandle(&g_tx_dma_handle, EXAMPLE_DMA, EXAMPLE_I3C_TX_DMA_CHANNEL);
    EDMA_CreateHandle(&g_rx_dma_handle, EXAMPLE_DMA, EXAMPLE_I3C_RX_DMA_CHANNEL);
    EDMA_SetChannelMux(EXAMPLE_DMA, EXAMPLE_I3C_TX_DMA_CHANNEL, EXAMPLE_I3C_TX_DMA_CHANNEL_MUX);
    EDMA_SetChannelMux(EXAMPLE_DMA, EXAMPLE_I3C_RX_DMA_CHANNEL, EXAMPLE_I3C_RX_DMA_CHANNEL_MUX);

    /* Create slave handle. */
    I3C_SlaveTransferCreateHandleEDMA(EXAMPLE_SLAVE, &g_i3c_s_handle, i3c_slave_callback, NULL, &g_rx_dma_handle,
                                      &g_tx_dma_handle);

    /* Start slave non-blocking transfer. */
    memset(&slaveXfer, 0, sizeof(slaveXfer));
    memset(g_slave_rxBuff, 0, sizeof(g_slave_rxBuff));
    slaveXfer.rxData     = g_slave_rxBuff;
    slaveXfer.rxDataSize = I3C_DATA_LENGTH;
    result = I3C_SlaveTransferEDMA(EXAMPLE_SLAVE, &g_i3c_s_handle, &slaveXfer, eventMask);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* Wait for master transmit completed. */
    while (!g_slaveCompletionFlag)
    {
    }
    g_slaveCompletionFlag = false;

    PRINTF("Slave received data :");
    for (uint32_t i = 0U; i < g_slave_rxBuff[0]; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_slave_rxBuff[i + 1]);
    }
    PRINTF("\r\n");

    /* Update slave tx buffer according to the received buffer. */
    memset(&slaveXfer, 0, sizeof(slaveXfer));
    slaveXfer.txData     = (uint8_t *)&g_slave_rxBuff[1];
    slaveXfer.txDataSize = g_slave_rxBuff[0];
    result = I3C_SlaveTransferEDMA(EXAMPLE_SLAVE, &g_i3c_s_handle, &slaveXfer,
                                   (eventMask | kI3C_SlaveRequestSentEvent));
    if (result != kStatus_Success)
    {
        return result;
    }

    /* Notify master that slave tx data is prepared, ibi data is the data size slave want to transmit. */
    uint8_t ibiData = g_slave_rxBuff[0];
    I3C_SlaveRequestIBIWithData(EXAMPLE_SLAVE, &ibiData, 1);
    PRINTF("\r\nI3C slave request IBI event with one mandatory data byte 0x%x.", ibiData);

    while (!g_slaveRequestSentFlag)
    {
    }
    g_slaveRequestSentFlag = false;

    PRINTF("\r\nI3C slave request IBI event sent.\r\n", ibiData);

    /* Wait for slave transmit completed. */
    while (!g_slaveCompletionFlag)
    {
    }
    g_slaveCompletionFlag = false;

    PRINTF("\r\nI3C master I3C SDR transfer finished.\r\n");

    while (1)
    {
    }
}
