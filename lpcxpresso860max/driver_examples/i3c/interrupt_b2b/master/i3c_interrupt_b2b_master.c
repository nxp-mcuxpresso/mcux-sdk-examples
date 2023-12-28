/*
 * Copyright 2021, 2023 NXP
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
#define EXAMPLE_MASTER          I3C0
#define EXAMPLE_I2C_BAUDRATE    100000
#define EXAMPLE_I3C_OD_BAUDRATE 400000
/* PP baudrate should be set equal to I3C FCLK dividing even number. */
#define EXAMPLE_I3C_PP_BAUDRATE    1875000
#define I3C_MASTER_CLOCK_FREQUENCY CLOCK_GetI3cClkFreq()
#define I3C_MASTER_SLAVE_ADDR_7BIT 0x1E
#define I3C_DATA_LENGTH            33

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
volatile bool g_masterCompletionFlag = false;
volatile bool g_ibiWonHasData        = false;
volatile bool g_ibiWonFlag           = false;
volatile status_t g_completionStatus = kStatus_Success;

/* user buffer for transmit and receive */
uint8_t g_master_txBuff[I3C_DATA_LENGTH];
uint8_t g_txFifoSize = 0;
uint8_t g_master_rxBuff[I3C_DATA_LENGTH];
#define I3C_TIME_OUT_INDEX 100000000U

/*******************************************************************************
 * Code
 ******************************************************************************/
void I3C0_IRQHandler(void)
{
    uint32_t statusFlags = EXAMPLE_MASTER->MSTATUS;
    uint32_t errorFlags  = EXAMPLE_MASTER->MERRWARN;
    bool txEnable        = EXAMPLE_MASTER->MINTMASKED & (uint32_t)kI3C_MasterTxReadyFlag;
    I3C_MasterClearStatusFlags(EXAMPLE_MASTER,
                               (uint32_t)(kI3C_MasterTxReadyFlag | kI3C_MasterRxReadyFlag | kI3C_MasterErrorFlag |
                                          kI3C_MasterSlaveStartFlag | kI3C_MasterArbitrationWonFlag));

    size_t txCount, rxCount;
    I3C_MasterGetFifoCounts(EXAMPLE_MASTER, &rxCount, &txCount);
    txCount = g_txFifoSize - txCount;

    if (0U != errorFlags)
    {
        I3C_MasterClearErrorStatusFlags(EXAMPLE_MASTER, errorFlags);
        g_completionStatus = kStatus_Fail;
        return;
    }

    /* Handle IBI event */
    if (0U != (statusFlags & (uint32_t)kI3C_MasterArbitrationWonFlag))
    {
        uint32_t masterState = (statusFlags & I3C_MSTATUS_STATE_MASK) >> I3C_MSTATUS_STATE_SHIFT;

        /* User could rewrite the logic to handle the IBI event need to do manual ACK/NACK */
        if (masterState == kI3C_MasterStateIbiAck)
        {
            /* In this example directly NAK the IBI event need master to do manual operation */
            I3C_MasterEmitIBIResponse(EXAMPLE_MASTER, kI3C_IbiRespNack);
        }
        else
        {
            if ((rxCount == 0U) && (0U != (statusFlags & (uint32_t)kI3C_MasterCompleteFlag)))
            {
                g_ibiWonFlag = true;
                I3C_MasterEmitRequest(EXAMPLE_MASTER, kI3C_RequestEmitStop);
            }
            else
            {
                g_ibiWonHasData = true;
                I3C_MasterEnableInterrupts(EXAMPLE_MASTER, (uint32_t)kI3C_MasterRxReadyFlag);
            }
        }

        return;
    }

    if (txEnable && (0U != (statusFlags & (uint32_t)kI3C_MasterTxReadyFlag)))
    {
        while ((txCount != 0U) && (g_transferSize < (g_buffSize - 1U)))
        {
            EXAMPLE_MASTER->MWDATAB = g_buff[g_transferSize];
            g_transferSize++;
            txCount--;
        }

        if (txCount != 0U)
        {
            EXAMPLE_MASTER->MWDATABE = g_buff[g_buffSize - 1U];
            g_transferSize++;
        }
    }

    if (0U != (statusFlags & (uint32_t)kI3C_MasterRxReadyFlag))
    {
        if (g_ibiWonHasData)
        {
            while (rxCount != 0U)
            {
                g_ibiBuff[g_ibiBuffSzie++] = EXAMPLE_MASTER->MRDATAB;
                rxCount--;
            }

            I3C_MasterEmitRequest(EXAMPLE_MASTER, kI3C_RequestEmitStop);

            g_ibiWonFlag    = true;
            g_ibiWonHasData = false;
        }

        while ((rxCount != 0U) && (g_transferSize < g_buffSize))
        {
            g_buff[g_transferSize] = EXAMPLE_MASTER->MRDATAB;
            g_transferSize++;
            rxCount--;
        }
    }

    /* Emit broadcast address to get slave IBI event */
    if (0U != (statusFlags & (uint32_t)kI3C_MasterSlaveStartFlag))
    {
        /* Emit start + 0x7E */
        I3C_MasterEmitRequest(EXAMPLE_MASTER, kI3C_RequestAutoIbi);
        return;
    }

    if ((g_transferSize != 0U) && (g_transferSize == g_buffSize))
    {
        g_masterCompletionFlag = true;
        I3C_MasterDisableInterrupts(EXAMPLE_MASTER,
                                    ((uint32_t)kI3C_MasterTxReadyFlag | (uint32_t)kI3C_MasterRxReadyFlag));
    }
}

static status_t i3c_master_transferBuff(uint8_t *buff, size_t buffSize, i3c_direction_t dir)
{
    status_t result = kStatus_Success;
    volatile uint32_t timeout = 0U;

    g_masterCompletionFlag = false;
    g_completionStatus     = kStatus_Success;
    g_buff                 = buff;
    g_buffSize             = buffSize;
    g_transferSize         = 0U;

    /* Add DSB instruction here to ensure the variable assignment takes place earlier than the interrupt enablement code
     * below. */
    __DSB();

    if (dir == kI3C_Write)
    {
        I3C_MasterEnableInterrupts(EXAMPLE_MASTER, (uint32_t)(kI3C_MasterTxReadyFlag | kI3C_MasterErrorFlag));
    }
    else
    {
        I3C_MasterEnableInterrupts(EXAMPLE_MASTER, (uint32_t)(kI3C_MasterRxReadyFlag | kI3C_MasterErrorFlag));
    }

    /* Wait for transfer completed. */
    while ((!g_masterCompletionFlag) && (g_completionStatus == kStatus_Success) && (++timeout < I3C_TIME_OUT_INDEX))
    {
        __NOP();
    }

    result = g_completionStatus;

    if (timeout == I3C_TIME_OUT_INDEX)
    {
        result = kStatus_Timeout;
    }

    if (result == kStatus_Success)
    {
        timeout = 0;
        while (((EXAMPLE_MASTER->MSTATUS & kI3C_MasterCompleteFlag) == 0U) && (++timeout < I3C_TIME_OUT_INDEX))
        {
            __NOP();
        }
        if (timeout == I3C_TIME_OUT_INDEX)
        {
            result = kStatus_Timeout;
        }
    }

    I3C_MasterDisableInterrupts(EXAMPLE_MASTER,
                                (uint32_t)(kI3C_MasterTxReadyFlag | kI3C_MasterRxReadyFlag | kI3C_MasterErrorFlag));

    return result;
}

static status_t i3c_master_pollIBIEvent(void)
{
    /* Wait for IBI event. */
    volatile uint32_t timeout = 0U;
    status_t result  = kStatus_Success;

    while ((!g_ibiWonFlag) && (++timeout < I3C_TIME_OUT_INDEX) && (g_completionStatus == kStatus_Success))
    {
        __NOP();
    }

    result = g_completionStatus;

    if (timeout == I3C_TIME_OUT_INDEX)
    {
        result = kStatus_Timeout;
    }

    g_ibiWonFlag       = false;
    g_completionStatus = kStatus_Success;
    g_ibiBuffSzie      = 0U;

    return result;
}
/*!
 * @brief Main function
 */
int main(void)
{
    i3c_master_config_t masterConfig;
    status_t result = kStatus_Success;

    /* Select the main clock as source clock of USART0 (debug console) */
    CLOCK_Select(BOARD_DEBUG_USART_CLK_ATTACH);

    CLOCK_Select(kI3C_Clk_From_Fro);

    /* I3C FCLK = 60M/4 = 15M */
    CLOCK_SetI3CFClkDiv(4);

    BOARD_InitPins();
    BOARD_BootClockFRO60M();
    BOARD_InitDebugConsole();

    PRINTF("\r\nI3C board2board Interrupt example -- Master.\r\n");

    /* I3C mode: Set up i3c master to work in I3C mode, send data to slave*/
    /* First data in txBuff is data length of the transmitting data. */
    g_master_txBuff[0] = I3C_DATA_LENGTH - 1U;
    for (uint32_t i = 1U; i < I3C_DATA_LENGTH; i++)
    {
        g_master_txBuff[i] = i - 1;
    }

    PRINTF("Master will send data :");
    for (uint32_t i = 0U; i < I3C_DATA_LENGTH - 1U; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_master_txBuff[i + 1]);
    }
    PRINTF("\r\n\r\n");

    I3C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Hz.i2cBaud          = EXAMPLE_I2C_BAUDRATE;
    masterConfig.baudRate_Hz.i3cPushPullBaud  = EXAMPLE_I3C_PP_BAUDRATE;
    masterConfig.baudRate_Hz.i3cOpenDrainBaud = EXAMPLE_I3C_OD_BAUDRATE;
    masterConfig.enableOpenDrainStop          = false;
    I3C_MasterInit(EXAMPLE_MASTER, &masterConfig, I3C_MASTER_CLOCK_FREQUENCY);

    /* Enable I3C IRQ in NVIC */
    (void)EnableIRQ(I3C0_IRQn);
    /* Always enable slave start and IBI Won interrupt for IBI handle */
    I3C_MasterEnableInterrupts(EXAMPLE_MASTER, (uint32_t)(kI3C_MasterSlaveStartFlag | kI3C_MasterArbitrationWonFlag));

    g_txFifoSize =
        2UL << ((EXAMPLE_MASTER->SCAPABILITIES & I3C_SCAPABILITIES_FIFOTX_MASK) >> I3C_SCAPABILITIES_FIFOTX_SHIFT);

    PRINTF("\r\nI3C master do dynamic address assignment to the I3C slaves on bus.\r\n");

    /* Reset dynamic address before DAA */

    I3C_MasterStart(EXAMPLE_MASTER, kI3C_TypeI3CSdr, 0x7EU, kI3C_Write);

    uint8_t cmdCode = 0x06U;
    result          = i3c_master_transferBuff(&cmdCode, 1U, kI3C_Write);

    if (kStatus_Success != result)
    {
        return result;
    }

    I3C_MasterStop(EXAMPLE_MASTER);

    uint8_t addressList[8] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37};
    result                 = I3C_MasterProcessDAA(EXAMPLE_MASTER, addressList, 8);
    if (result != kStatus_Success)
    {
        return -1;
    }

    PRINTF("\r\nI3C master dynamic address assignment done.\r\n");

    uint8_t devCount;
    i3c_device_info_t *devList;
    uint8_t slaveAddr = 0x0U;
    devList           = I3C_MasterGetDeviceListAfterDAA(EXAMPLE_MASTER, &devCount);
    for (uint8_t devIndex = 0; devIndex < devCount; devIndex++)
    {
        if (devList[devIndex].vendorID == 0x123U)
        {
            slaveAddr = devList[devIndex].dynamicAddr;
            break;
        }
    }

    PRINTF("\r\nStart to do I3C master transfer in I3C SDR mode.\r\n");
    I3C_MasterStart(EXAMPLE_MASTER, kI3C_TypeI3CSdr, slaveAddr, kI3C_Write);
    result = i3c_master_transferBuff(g_master_txBuff, I3C_DATA_LENGTH, kI3C_Write);

    if (result != kStatus_Success)
    {
        return -1;
    }

    I3C_MasterStop(EXAMPLE_MASTER);

    i3c_register_ibi_addr_t ibiRecord = {.address = {slaveAddr}, .ibiHasPayload = true};
    I3C_MasterRegisterIBI(EXAMPLE_MASTER, &ibiRecord);
    EXAMPLE_MASTER->MCTRL |= I3C_MCTRL_IBIRESP(kI3C_IbiRespAckMandatory);

    PRINTF("\r\nI3C master wait for slave IBI event to notify the slave transmit size.\r\n");

    result = i3c_master_pollIBIEvent();

    if (result != kStatus_Success)
    {
        return -1;
    }
    PRINTF("\r\nI3C master received slave IBI event, data size %d, value 0x%x.\r\n", g_ibiBuffSzie, g_ibiBuff[0]);

    /* Start read data back from slave. */
    I3C_MasterStart(EXAMPLE_MASTER, kI3C_TypeI3CSdr, slaveAddr, kI3C_Read);

    result = i3c_master_transferBuff(g_master_rxBuff, g_ibiBuff[0], kI3C_Read);

    if (result != kStatus_Success)
    {
        return -1;
    }

    I3C_MasterStop(EXAMPLE_MASTER);

    for (uint32_t i = 0U; i < g_master_txBuff[0]; i++)
    {
        if (g_master_rxBuff[i] != g_master_txBuff[i + 1])
        {
            PRINTF("\r\nError occurred in the transfer ! \r\n");
            return -1;
        }
    }

    PRINTF("\r\nI3C master transfer successful in I3C SDR mode .\r\n");

    while (1)
    {
    }
}
