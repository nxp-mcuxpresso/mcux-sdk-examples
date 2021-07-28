/*
 * Copyright 2019 NXP
 * All rights reserved.
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
#include "fsl_i3c.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_SLAVE              I3C0
#define I3C_SLAVE_CLOCK_FREQUENCY  CLOCK_GetLpOscFreq()
#define I3C_TIME_OUT_INDEX         100000000
#define I3C_MASTER_SLAVE_ADDR_7BIT 0x1EU
#define I3C_DATA_LENGTH            34U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint8_t g_slave_txBuff[I3C_DATA_LENGTH + 1] = {0};
uint8_t g_slave_rxBuff[I3C_DATA_LENGTH + 1] = {0};
volatile bool g_slaveCompletionFlag         = false;
i3c_slave_handle_t g_i3c_s_handle;
uint8_t *g_txBuff;
uint32_t g_txSize = I3C_DATA_LENGTH;
/*******************************************************************************
 * Code
 ******************************************************************************/

static void i3c_slave_callback(I3C_Type *base, i3c_slave_transfer_t *xfer, void *userData)
{
    switch ((uint32_t)xfer->event)
    {
        /*  Transmit request */
        case kI3C_SlaveTransmitEvent:
            /*  Update information for transmit process */
            xfer->txData     = g_txBuff;
            xfer->txDataSize = g_txSize;
            break;

        /*  Receive request */
        case kI3C_SlaveReceiveEvent:
            /*  Update information for received process */
            xfer->rxData     = g_slave_rxBuff;
            xfer->rxDataSize = I3C_DATA_LENGTH;
            break;

        /*  Transmit request */
        case (kI3C_SlaveTransmitEvent | kI3C_SlaveHDRCommandMatchEvent):
            /*  Update information for transmit process */
            xfer->txData     = g_slave_txBuff;
            xfer->txDataSize = I3C_DATA_LENGTH;
            break;

        /*  Receive request */
        case (kI3C_SlaveReceiveEvent | kI3C_SlaveHDRCommandMatchEvent):
            /*  Update information for received process */
            xfer->rxData     = g_slave_rxBuff;
            xfer->rxDataSize = I3C_DATA_LENGTH;
            break;

        /*  Transfer done */
        case kI3C_SlaveCompletionEvent:
            if (xfer->completionStatus == kStatus_Success)
            {
                g_slaveCompletionFlag = true;
            }
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
    i3c_slave_config_t slaveConfig;

    /* Attach main clock to I3C, 396MHz / 4 = 99MHz. */
    CLOCK_AttachClk(kMAIN_CLK_to_I3C_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivI3cClk, 4);

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("\r\nI3C board2board polling example -- Slave transfer.\r\n\r\n");

    I3C_SlaveGetDefaultConfig(&slaveConfig);

    slaveConfig.staticAddr = I3C_MASTER_SLAVE_ADDR_7BIT;
    slaveConfig.vendorID   = 0x123U;
    slaveConfig.offline    = false;

    I3C_SlaveInit(EXAMPLE_SLAVE, &slaveConfig, I3C_SLAVE_CLOCK_FREQUENCY);

    /* Create slave handle. */
    I3C_SlaveTransferCreateHandle(EXAMPLE_SLAVE, &g_i3c_s_handle, i3c_slave_callback, NULL);

    g_txBuff = g_slave_txBuff;
    /* Start slave non-blocking transfer. */
    I3C_SlaveTransferNonBlocking(EXAMPLE_SLAVE, &g_i3c_s_handle, kI3C_SlaveCompletionEvent);

    PRINTF("\r\nCheck I3C master I2C transfer.\r\n");
    memset(g_slave_rxBuff, 0, I3C_DATA_LENGTH);
    /* Wait for master transmit completed. */
    uint32_t timeout_i = 0U;
    while ((!g_slaveCompletionFlag) && (++timeout_i < 10 * I3C_TIME_OUT_INDEX))
    {
    }
    g_slaveCompletionFlag = false;

    memcpy(g_slave_txBuff, g_slave_rxBuff, I3C_DATA_LENGTH);
    /* Update slave tx buffer according to the received buffer. */
    g_txBuff = &g_slave_txBuff[2];
    g_txSize = g_slave_txBuff[1];

    PRINTF("Slave received data :");

    for (uint32_t i = 0U; i < I3C_DATA_LENGTH; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_slave_rxBuff[i]);
    }
    PRINTF("\r\n\r\n");

    /* Wait for slave transmit completed. */
    timeout_i = 0U;
    while ((!g_slaveCompletionFlag) && (++timeout_i < I3C_TIME_OUT_INDEX))
    {
    }
    g_slaveCompletionFlag = false;

    if (timeout_i == I3C_TIME_OUT_INDEX)
    {
        PRINTF("\r\n Transfer timeout .\r\n");
        return -1;
    }

    PRINTF("\r\n I3C master I2C transfer finished .\r\n");

    PRINTF("\r\nCheck I3C master I3C SDR transfer.\r\n");

    /* Wait for master transmit completed. */
    memset(g_slave_rxBuff, 0, I3C_DATA_LENGTH);
    timeout_i = 0U;
    while ((!g_slaveCompletionFlag) && (++timeout_i < I3C_TIME_OUT_INDEX))
    {
    }
    g_slaveCompletionFlag = false;
    if (timeout_i == I3C_TIME_OUT_INDEX)
    {
        PRINTF("\r\n Transfer timeout .\r\n");
        return -1;
    }

    /* Update slave tx buffer according to the received buffer. */
    memcpy(g_slave_txBuff, g_slave_rxBuff, I3C_DATA_LENGTH);
    g_txBuff = &g_slave_txBuff[1];
    g_txSize = g_slave_txBuff[0];

    PRINTF("Slave received data :");

    for (uint32_t i = 0U; i < g_slave_rxBuff[0]; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_slave_rxBuff[i + 1]);
    }
    PRINTF("\r\n\r\n");

    /* Wait for slave transmit completed. */
    timeout_i = 0U;
    while ((!g_slaveCompletionFlag) && (++timeout_i < I3C_TIME_OUT_INDEX))
    {
    }
    g_slaveCompletionFlag = false;
    if (timeout_i == I3C_TIME_OUT_INDEX)
    {
        PRINTF("\r\n Transfer timeout .\r\n");
        return -1;
    }

    PRINTF("\r\n I3C master I3C SDR transfer finished .\r\n");

    while (1)
    {
    }
}
