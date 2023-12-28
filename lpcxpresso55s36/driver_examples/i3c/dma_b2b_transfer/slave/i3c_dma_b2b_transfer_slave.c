/*
 * Copyright 2021-2022 NXP
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
#define I3C_MASTER_SLAVE_ADDR_7BIT 0x1EU
#define I3C_DATA_LENGTH            33U
#define EXAMPLE_DMA                DMA0
#define EXAMPLE_I3C_RX_CHANNEL     kDma0RequestI3C0RX
#define EXAMPLE_I3C_TX_CHANNEL     kDma0RequestI3C0TX

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
#ifdef EXAMPLE_I3C_DMA_TX_WORK_AROUND
static uint8_t g_slave_txBuff[2 * I3C_DATA_LENGTH - 6U] = {0};
#else
static uint8_t g_slave_txBuff[I3C_DATA_LENGTH] = {0};
#endif
static uint8_t g_slave_rxBuff[I3C_DATA_LENGTH] = {0};
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

    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, true);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, false);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* Attach main clock to I3C, 150MHz / 4 = 37.5MHz. */
    CLOCK_SetClkDiv(kCLOCK_DivI3cFclk, 0U, true);
    CLOCK_SetClkDiv(kCLOCK_DivI3cFclk, 4U, false);
    CLOCK_AttachClk(kMAIN_CLK_to_I3CFCLK);

    CLOCK_SetClkDiv(kCLOCK_DivI3cFclkS, 0U, true);
    CLOCK_SetClkDiv(kCLOCK_DivI3cFclkS, 1U, false);

    // Enable FRO 1MHz clock
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

    /* Create I3C DMA Tx/Rx handle. */
    DMA_Init(EXAMPLE_DMA);
    DMA_EnableChannel(EXAMPLE_DMA, EXAMPLE_I3C_RX_CHANNEL);
    DMA_CreateHandle(&g_rx_dma_handle, EXAMPLE_DMA, EXAMPLE_I3C_RX_CHANNEL);
    DMA_EnableChannel(EXAMPLE_DMA, EXAMPLE_I3C_TX_CHANNEL);
    DMA_CreateHandle(&g_tx_dma_handle, EXAMPLE_DMA, EXAMPLE_I3C_TX_CHANNEL);
    /* Create slave handle. */
    I3C_SlaveTransferCreateHandleDMA(EXAMPLE_SLAVE, &g_i3c_s_handle, i3c_slave_callback, NULL, &g_rx_dma_handle,
                                     &g_tx_dma_handle);

    /* Start slave non-blocking transfer. */
    memset(g_slave_rxBuff, 0, I3C_DATA_LENGTH);
    slaveXfer.rxData     = g_slave_rxBuff;
    slaveXfer.rxDataSize = I3C_DATA_LENGTH;
    I3C_SlaveTransferDMA(EXAMPLE_SLAVE, &g_i3c_s_handle, &slaveXfer, kI3C_SlaveCompletionEvent);

    PRINTF("\r\nWait I3C master I3C SDR transfer.\r\n");
    /* Wait for master transmit completed. */
    while (!g_slaveCompletionFlag)
    {
    }
    g_slaveCompletionFlag = false;

    PRINTF("Slave received data:");
    for (uint32_t i = 0U; i < g_slave_rxBuff[0]; i++)
    {
        if (i % 8U == 0U)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_slave_rxBuff[i + 1]);
    }
    PRINTF("\r\n");

#ifdef EXAMPLE_I3C_DMA_TX_WORK_AROUND
    /* DMA Tx need workaround when transfer length is larger than 8 bytes.
       The workaround needs two dummy data be inserted every two real bytes, so,
       from the 9th byte, we have two dummy data(0U) inserted, then two real transfer
       data. The workaround buffer g_master_txBuff_workaround will be filled in this
       way and need to be used in the DMA Tx transfer. */
    uint32_t j = 0U, ii = 0U, jj = 0U;
    for (uint32_t i = 0U; i < 2U * I3C_DATA_LENGTH - 6U; i++)
    {
        if (i < 8U)
        {
            g_slave_txBuff[i] = i;
        }
        else
        {
            j  = i % 4U;
            ii = i / 4U;
            jj = 2U * ii + 4U;
            switch (j)
            {
                case 0U:
                    g_slave_txBuff[i] = 0U;
                    break;
                case 1U:
                    g_slave_txBuff[i] = 0U;
                    break;
                case 2U:
                    g_slave_txBuff[i] = jj;
                    break;
                case 3U:
                    g_slave_txBuff[i] = jj + 1U;
                    break;
                default:
                    break;
            }
        }
    }
#else
    for (uint32_t i = 0U; i < I3C_DATA_LENGTH; i++)
    {
        g_slave_txBuff[i] = i;
    }
#endif

    /* Update slave Tx buffer according to the received buffer. */
    memset(&slaveXfer, 0, sizeof(slaveXfer));
    slaveXfer.txData = (uint8_t *)g_slave_txBuff;
#ifdef EXAMPLE_I3C_DMA_TX_WORK_AROUND
    slaveXfer.txDataSize = 2U * g_slave_rxBuff[0] - 6U;
#else
    slaveXfer.txDataSize = g_slave_rxBuff[0];
#endif
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

    while (1)
    {
    }
}
