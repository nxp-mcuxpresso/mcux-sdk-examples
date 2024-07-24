/*
 * Copyright 2021-2022, 2024 NXP
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
#include "fsl_i3c_dma.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_SLAVE              I3C0
#define I3C_SLAVE_CLOCK_FREQUENCY  CLOCK_GetFro1MFreq()
#define EXAMPLE_DMA                DMA0
#define EXAMPLE_I3C_RX_CHANNEL     kDma0RequestI3C0RX
#define EXAMPLE_I3C_TX_CHANNEL     kDma0RequestI3C0TX
#define EXAMPLE_I3C_HDR_SUPPORT    1U
#ifndef I3C_MASTER_SLAVE_ADDR_7BIT
#define I3C_MASTER_SLAVE_ADDR_7BIT 0x1E
#endif
#ifndef I3C_DATA_LENGTH
#define I3C_DATA_LENGTH            32U
#endif
#ifndef EXAMPLE_I3C_HDR_SUPPORT
#define EXAMPLE_I3C_HDR_SUPPORT    0U
#endif

#define I3C_PACKET_LENGTH          (I3C_DATA_LENGTH + 2U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint8_t g_slave_txBuff[I3C_PACKET_LENGTH] = {0};
static uint8_t g_slave_rxBuff[I3C_PACKET_LENGTH] = {0};
static volatile bool g_slaveCompletionFlag     = false;
static volatile bool g_slaveRequestSentFlag    = false;
static i3c_slave_dma_handle_t g_i3c_s_handle;
static dma_handle_t g_tx_dma_handle;
static dma_handle_t g_rx_dma_handle;
/*******************************************************************************
 * Code
 ******************************************************************************/
static void i3c_slave_callback(I3C_Type *base, i3c_slave_dma_transfer_t *xfer, void *userData)
{
    switch ((uint32_t)xfer->event)
    {
        case kI3C_SlaveCompletionEvent:
            if (xfer->completionStatus == kStatus_Success)
            {
                g_slaveCompletionFlag = true;
            }
            break;
        case kI3C_SlaveRequestSentEvent:
            g_slaveRequestSentFlag = true;
            break;

        default:
            break;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    i3c_slave_dma_transfer_t slaveXfer = {0};
    i3c_slave_config_t slaveConfig;
    uint8_t ibiData;

    /* Attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, true);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, false);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* Attach main clock to I3C, 150MHz / 6 = 25MHz. */
    CLOCK_SetClkDiv(kCLOCK_DivI3cFclk, 0U, true);
    CLOCK_SetClkDiv(kCLOCK_DivI3cFclk, 6U, false);
    CLOCK_AttachClk(kMAIN_CLK_to_I3CFCLK);

    CLOCK_SetClkDiv(kCLOCK_DivI3cFclkS, 0U, true);
    CLOCK_SetClkDiv(kCLOCK_DivI3cFclkS, 1U, false);

    /* Enable FRO 1MHz clock. */
    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_FRO1MHZ_CLK_ENA_MASK;

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    PRINTF("\r\nI3C board2board DMA example -- Slave transfer.\r\n");

    I3C_SlaveGetDefaultConfig(&slaveConfig);
    slaveConfig.staticAddr = I3C_MASTER_SLAVE_ADDR_7BIT;
    slaveConfig.vendorID   = 0x123U;
    slaveConfig.offline    = false;
    I3C_SlaveInit(EXAMPLE_SLAVE, &slaveConfig, I3C_SLAVE_CLOCK_FREQUENCY);

    /* Create DMA handle for I3C Tx/Rx. */
    DMA_Init(EXAMPLE_DMA);
    DMA_EnableChannel(EXAMPLE_DMA, EXAMPLE_I3C_RX_CHANNEL);
    DMA_CreateHandle(&g_rx_dma_handle, EXAMPLE_DMA, EXAMPLE_I3C_RX_CHANNEL);
    DMA_EnableChannel(EXAMPLE_DMA, EXAMPLE_I3C_TX_CHANNEL);
    DMA_CreateHandle(&g_tx_dma_handle, EXAMPLE_DMA, EXAMPLE_I3C_TX_CHANNEL);

    /* Create I3C slave DMA transfer handle. */
    I3C_SlaveTransferCreateHandleDMA(EXAMPLE_SLAVE, &g_i3c_s_handle, i3c_slave_callback, NULL, &g_rx_dma_handle,
                                     &g_tx_dma_handle);

    /* Start slave non-blocking DMA transfer. */
    memset(g_slave_rxBuff, 0, I3C_PACKET_LENGTH);
    slaveXfer.rxData     = g_slave_rxBuff;
    slaveXfer.rxDataSize = I3C_PACKET_LENGTH;
    I3C_SlaveTransferDMA(EXAMPLE_SLAVE, &g_i3c_s_handle, &slaveXfer, kI3C_SlaveCompletionEvent);

    PRINTF("\r\nWait I3C master I3C SDR transfer.\r\n");
    /* Wait for master transmit completed. */
    while (!g_slaveCompletionFlag)
    {
    }
    g_slaveCompletionFlag = false;

    PRINTF("Slave received data:\r\n");
    for (uint32_t i = 0U; i < g_slave_rxBuff[0]; i++)
    {
        PRINTF("0x%2x  ", g_slave_rxBuff[i + 1]);
        if (i % 8U == 7U)
        {
            PRINTF("\r\n");
        }
    }

    /* Update slave Tx buffer according to the received buffer. */
    memcpy(&g_slave_txBuff[0], &g_slave_rxBuff[1], I3C_DATA_LENGTH);
    memset(&slaveXfer, 0, sizeof(slaveXfer));
    slaveXfer.txData     = (uint8_t *)g_slave_txBuff;
    slaveXfer.txDataSize = g_slave_rxBuff[0];
    I3C_SlaveTransferDMA(EXAMPLE_SLAVE, &g_i3c_s_handle, &slaveXfer,
                         (kI3C_SlaveCompletionEvent | kI3C_SlaveRequestSentEvent));

    /* Notify master that slave Tx data is prepared, ibi data is the data size slave want to transmit. */
    ibiData = g_slave_rxBuff[0];
    I3C_SlaveRequestIBIWithData(EXAMPLE_SLAVE, &ibiData, 1);
    PRINTF("\r\nI3C slave request IBI event with one mandatory data byte 0x%x.\r\n", ibiData);
    while (!g_slaveRequestSentFlag)
    {
    }
    g_slaveRequestSentFlag = false;
    PRINTF("I3C slave request IBI event sent.\r\n", ibiData);

    PRINTF("\r\nI3C slave sends received data back.\r\n", ibiData);
    /* Wait for slave transmit completed. */
    while (!g_slaveCompletionFlag)
    {
    }
    g_slaveCompletionFlag = false;
    PRINTF("\r\nI3C master I3C SDR transfer finished.\r\n");

#if defined(EXAMPLE_I3C_HDR_SUPPORT) && (EXAMPLE_I3C_HDR_SUPPORT)
    PRINTF("\r\nCheck I3C master I3C HDR transfer.\r\n");

    memset(g_slave_rxBuff, 0, I3C_PACKET_LENGTH);
    memset(&slaveXfer, 0, sizeof(slaveXfer));
    slaveXfer.rxData     = g_slave_rxBuff;
    slaveXfer.rxDataSize = I3C_PACKET_LENGTH;
    I3C_SlaveTransferDMA(EXAMPLE_SLAVE, &g_i3c_s_handle, &slaveXfer,
                         (kI3C_SlaveCompletionEvent | kI3C_SlaveRequestSentEvent));

    I3C_SlaveRequestIBIWithData(EXAMPLE_SLAVE, &ibiData, 1);
    while (!g_slaveRequestSentFlag)
    {
    }
    g_slaveRequestSentFlag = false;

    while (!g_slaveCompletionFlag)
    {
    }
    g_slaveCompletionFlag = false;

    /* The first byte of Rx buffer is HDR command, the second is data size, the following bytes are data content. */
    memcpy(&g_slave_txBuff[0], &g_slave_rxBuff[2], I3C_DATA_LENGTH);
    memset(&slaveXfer, 0, sizeof(slaveXfer));
    slaveXfer.txData     = &g_slave_txBuff[0];
    slaveXfer.txDataSize = g_slave_rxBuff[1];
    I3C_SlaveTransferDMA(EXAMPLE_SLAVE, &g_i3c_s_handle, &slaveXfer,
                         (kI3C_SlaveCompletionEvent | kI3C_SlaveRequestSentEvent));

    PRINTF("Slave received data :\r\n");
    for (uint32_t i = 0U; i < g_slave_rxBuff[1]; i++)
    {
        PRINTF("0x%2x  ", g_slave_rxBuff[i + 2]);
        if (i % 8U == 7U)
        {
            PRINTF("\r\n");
        }
    }

    ibiData = g_slave_rxBuff[1];
    I3C_SlaveRequestIBIWithData(EXAMPLE_SLAVE, &ibiData, 1);
    while (!g_slaveRequestSentFlag)
    {
    }
    g_slaveRequestSentFlag = false;

    /* The second transfer is a I3C SDR read transfer, master will read back the transmit buffer content just sent. */
    /* Wait for slave transmit completed. */
    while (!g_slaveCompletionFlag)
    {
    }
    g_slaveCompletionFlag = false;

    PRINTF("\r\nI3C master I3C HDR transfer finished.\r\n");
#endif

    while (1)
    {
    }
}
