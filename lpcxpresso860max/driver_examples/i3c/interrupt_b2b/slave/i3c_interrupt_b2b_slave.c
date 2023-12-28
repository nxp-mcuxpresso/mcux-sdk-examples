/*
 * Copyright 2021-2023 NXP
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

#include "fsl_power.h"
#include "fsl_iocon.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_SLAVE              I3C0
#define I3C_SLAVE_CLOCK_FREQUENCY  CLOCK_GetLpOscClkFreq()
#define I3C_TIME_OUT_INDEX         100000000
#define I3C_MASTER_SLAVE_ADDR_7BIT 0x1EU
#define I3C_DATA_LENGTH            34U
#define EXAMPLE_DMA                DMA0
#define EXAMPLE_I3C_RX_CHANNEL     12
#define EXAMPLE_I3C_TX_CHANNEL     13

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Invovked in IRQ handler for buffer write/read and status update */
volatile size_t g_buffSize = 0;
volatile uint8_t *g_buff;
volatile size_t g_transferSize = 0;
volatile uint8_t g_ibiBuff[10];
volatile uint8_t g_ibiBuffSzie       = 0;
volatile bool g_slaveCompletionFlag  = false;
volatile bool g_slaveRequestSentFlag = false;
volatile status_t g_completionStatus = kStatus_Success;
volatile bool g_addressMatchFlag     = false;
volatile bool g_txEnable             = false;

/* user buffer for transmit and receive */
uint8_t g_slave_txBuff[I3C_DATA_LENGTH + 1] = {0};
uint8_t g_slave_rxBuff[I3C_DATA_LENGTH + 1] = {0};
size_t g_txFifoSize                         = 0U;
/*******************************************************************************
 * Code
 ******************************************************************************/
void I3C0_IRQHandler(void)
{
    uint32_t statusFlags = EXAMPLE_SLAVE->SSTATUS;
    uint32_t errorFlags  = EXAMPLE_SLAVE->SERRWARN;
    I3C_SlaveClearStatusFlags(EXAMPLE_SLAVE, statusFlags);

    size_t txCount, rxCount;
    I3C_SlaveGetFifoCounts(EXAMPLE_SLAVE, &rxCount, &txCount);
    txCount = g_txFifoSize - txCount;

    /* Check error in the transfer. */
    if (0U != errorFlags)
    {
        I3C_SlaveClearErrorStatusFlags(EXAMPLE_SLAVE, errorFlags);
        g_completionStatus = kStatus_Fail;
        return;
    }

    /* Handle TXNOTFULL event to fill tx fifo. */
    if (g_txEnable && (0U != (statusFlags & (uint32_t)kI3C_SlaveTxReadyFlag)))
    {
        while ((txCount != 0U) && (g_transferSize < (g_buffSize - 1U)))
        {
            EXAMPLE_SLAVE->SWDATAB = g_buff[g_transferSize];
            g_transferSize++;
            txCount--;
        }

        if (txCount != 0U)
        {
            EXAMPLE_SLAVE->SWDATABE = g_buff[g_buffSize - 1U];
        }
    }

    /* Handle RXSEND event to read rx fifo. */
    if (0U != (statusFlags & (uint32_t)kI3C_SlaveRxReadyFlag))
    {
        while (rxCount != 0U)
        {
            g_buff[g_transferSize] = EXAMPLE_SLAVE->SRDATAB;
            g_transferSize++;
            rxCount--;
        }
    }

    /* Handle match address event. */
    if (0U != (statusFlags & (uint32_t)kI3C_SlaveMatchedFlag))
    {
        g_addressMatchFlag = true;
    }

    /* Handle bus stop event when bus is match. */
    if (g_addressMatchFlag && (0U != (statusFlags & (uint32_t)kI3C_SlaveBusStopFlag)))
    {
        g_slaveCompletionFlag = true;

        if (g_transferSize == g_buffSize)
        {
            I3C_SlaveDisableInterrupts(EXAMPLE_SLAVE, ((uint32_t)kI3C_SlaveTxReadyFlag));
            g_txEnable = false;
        }

        g_addressMatchFlag = false;
        g_transferSize     = 0U;
        return;
    }

    /* Report event sent. */
    if (0U != (statusFlags & (uint32_t)kI3C_SlaveEventSentFlag))
    {
        g_slaveRequestSentFlag = true;
    }
}

static status_t i3c_slave_receiveBuff(uint8_t *buff, size_t buffSize)
{
    status_t result = kStatus_Success;
    volatile uint32_t timeout = 0U;

    g_slaveCompletionFlag = false;
    g_completionStatus    = kStatus_Success;
    g_buff                = buff;
    g_buffSize            = buffSize;

    I3C_SlaveEnableInterrupts(EXAMPLE_SLAVE, (uint32_t)(kI3C_SlaveRxReadyFlag | kI3C_SlaveErrorFlag));

    /* Wait for transfer completed. */
    while ((!g_slaveCompletionFlag) && (g_completionStatus == kStatus_Success) && (++timeout < I3C_TIME_OUT_INDEX))
    {
        __NOP();
    }

    result = g_completionStatus;

    if (timeout == I3C_TIME_OUT_INDEX)
    {
        result = kStatus_Timeout;
    }

    I3C_SlaveDisableInterrupts(EXAMPLE_SLAVE, (uint32_t)(kI3C_SlaveRxReadyFlag | kI3C_SlaveErrorFlag));

    return result;
}

static void i3c_slave_writeBuff(uint8_t *buff, size_t buffSize)
{
    g_slaveCompletionFlag = false;
    g_completionStatus    = kStatus_Success;
    g_buff                = buff;
    g_buffSize            = buffSize;

    g_txEnable = true;
    I3C_SlaveEnableInterrupts(EXAMPLE_SLAVE, (uint32_t)(kI3C_SlaveTxReadyFlag | kI3C_SlaveErrorFlag));
}

/*!
 * @brief Main function
 */
int main(void)
{
    i3c_slave_config_t slaveConfig;
    status_t result = kStatus_Success;
    volatile uint32_t timeout_i = 0U;

    /* Select the main clock as source clock of USART0 (debug console) */
    CLOCK_Select(BOARD_DEBUG_USART_CLK_ATTACH);

    /* Enable lp_osc 1MHz */
    POWER_DisablePD(kPDRUNCFG_PD_LPO_OSC);

    CLOCK_Select(kI3C_Clk_From_Fro);
    CLOCK_SetI3CFClkDiv(1);

    CLOCK_Select(kI3C_TC_Clk_From_LpOsc);
    CLOCK_SetI3CTCClkDiv(1);
    CLOCK_SetI3CSClkDiv(1);

    BOARD_InitPins();
    BOARD_BootClockFRO60M();
    BOARD_InitDebugConsole();

    PRINTF("\r\nI3C board2board Interrupt example -- Slave.\r\n\r\n");

    I3C_SlaveGetDefaultConfig(&slaveConfig);

    slaveConfig.staticAddr = I3C_MASTER_SLAVE_ADDR_7BIT;
    slaveConfig.vendorID   = 0x123U;
    slaveConfig.offline    = false;

    I3C_SlaveInit(EXAMPLE_SLAVE, &slaveConfig, I3C_SLAVE_CLOCK_FREQUENCY);

    PRINTF("\r\nCheck I3C master I3C SDR transfer.\r\n");

    g_txFifoSize =
        2UL << ((EXAMPLE_SLAVE->SCAPABILITIES & I3C_SCAPABILITIES_FIFOTX_MASK) >> I3C_SCAPABILITIES_FIFOTX_SHIFT);
    /* Enable I3C IRQ in NVIC */
    (void)EnableIRQ(I3C0_IRQn);
    /* Always enable slave request sent event */
    I3C_SlaveEnableInterrupts(EXAMPLE_SLAVE,
                              (uint32_t)(kI3C_SlaveEventSentFlag | kI3C_SlaveMatchedFlag | kI3C_SlaveBusStopFlag));

    /* Slave read buffer */
    result = i3c_slave_receiveBuff(g_slave_rxBuff, I3C_DATA_LENGTH);

    if (result != kStatus_Success)
    {
        return -1;
    }

    memcpy(g_slave_txBuff, &g_slave_rxBuff[1], g_slave_rxBuff[0]);

    i3c_slave_writeBuff(g_slave_txBuff, g_slave_rxBuff[0]);

    /* Notify master that slave tx data is prepared, ibi data is the data size slave want to transmit. */
    uint8_t ibiData = g_slave_rxBuff[0];
    I3C_SlaveRequestIBIWithData(EXAMPLE_SLAVE, &ibiData, 1);
    PRINTF("\r\nI3C slave request IBI event with one mandatory data byte 0x%x.\r\n", ibiData);

    while ((!g_slaveRequestSentFlag) && (++timeout_i < I3C_TIME_OUT_INDEX))
    {
    }
    if (timeout_i == I3C_TIME_OUT_INDEX)
    {
        PRINTF("\r\nSlave request timeout.\r\n");
        return -1;
    }
    g_slaveRequestSentFlag = false;

    PRINTF("\r\nI3C slave request IBI event sent.\r\n", ibiData);

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
    while ((!g_slaveCompletionFlag) && (g_completionStatus == kStatus_Success) && (++timeout_i < I3C_TIME_OUT_INDEX))
    {
    }
    g_slaveCompletionFlag = false;
    result                = g_completionStatus;

    if (result != kStatus_Success)
    {
        return -1;
    }

    if (timeout_i == I3C_TIME_OUT_INDEX)
    {
        PRINTF("\r\nTransfer timeout.\r\n");
        return -1;
    }

    PRINTF("\r\n I3C Slave I3C SDR transfer finished .\r\n");

    while (1)
    {
    }
}
