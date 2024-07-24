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
#define EXAMPLE_MASTER             I3C0
#define EXAMPLE_I2C_BAUDRATE       400000
#define EXAMPLE_I3C_OD_BAUDRATE    1500000
#define EXAMPLE_I3C_PP_BAUDRATE    6000000
#define I3C_MASTER_CLOCK_FREQUENCY CLOCK_GetI3cClkFreq()
#define EXAMPLE_DMA                DMA0
#define EXAMPLE_I3C_RX_CHANNEL     12
#define EXAMPLE_I3C_TX_CHANNEL     13
#ifndef I3C_MASTER_SLAVE_ADDR_7BIT
#define I3C_MASTER_SLAVE_ADDR_7BIT 0x1E
#endif
#ifndef I3C_DATA_LENGTH
#define I3C_DATA_LENGTH            32U
#endif
#ifndef EXAMPLE_I3C_HDR_SUPPORT
#define EXAMPLE_I3C_HDR_SUPPORT    0U
#endif

#define I3C_PACKET_LENGTH          (I3C_DATA_LENGTH + 1U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void i3c_master_ibi_callback(I3C_Type *base,
                                    i3c_master_dma_handle_t *handle,
                                    i3c_ibi_type_t ibiType,
                                    i3c_ibi_state_t ibiState);
static void i3c_master_callback(I3C_Type *base, i3c_master_dma_handle_t *handle, status_t status, void *userData);
/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint8_t g_master_txBuff[I3C_PACKET_LENGTH];
static uint8_t g_master_rxBuff[I3C_PACKET_LENGTH];
static uint8_t g_master_ibiBuff[10];
static i3c_master_dma_handle_t g_i3c_m_handle;
static dma_handle_t g_tx_dma_handle;
static dma_handle_t g_rx_dma_handle;
static const i3c_master_dma_callback_t masterCallback = {
    .slave2Master = NULL, .ibiCallback = i3c_master_ibi_callback, .transferComplete = i3c_master_callback};
static volatile bool g_masterCompletionFlag = false;
static volatile bool g_ibiWonFlag           = false;
static volatile status_t g_completionStatus = kStatus_Success;
static uint8_t g_ibiBuff[10U];
static uint8_t g_ibiPayloadSize = 0;
/*******************************************************************************
 * Code
 ******************************************************************************/
static void i3c_master_ibi_callback(I3C_Type *base,
                                    i3c_master_dma_handle_t *handle,
                                    i3c_ibi_type_t ibiType,
                                    i3c_ibi_state_t ibiState)
{
    switch (ibiType)
    {
        case kI3C_IbiNormal:
            if (ibiState == kI3C_IbiDataBuffNeed)
            {
                handle->ibiBuff = g_master_ibiBuff;
            }
            else
            {
                memcpy(g_ibiBuff, (void *)handle->ibiBuff, handle->ibiPayloadSize);
                g_ibiPayloadSize = handle->ibiPayloadSize;
            }
            break;

        default:
            assert(false);
            break;
    }
}

static void i3c_master_callback(I3C_Type *base, i3c_master_dma_handle_t *handle, status_t status, void *userData)
{
    if (status == kStatus_Success)
    {
        g_masterCompletionFlag = true;
    }

    if (status == kStatus_I3C_IBIWon)
    {
        g_ibiWonFlag = true;
    }

    g_completionStatus = status;
}

/*!
 * @brief Main function
 */
int main(void)
{
    status_t result = kStatus_Success;
    i3c_master_config_t masterConfig;
    i3c_master_transfer_t masterXfer;

    /* Select the main clock as source clock of USART0 (debug console) */
    CLOCK_Select(BOARD_DEBUG_USART_CLK_ATTACH);

    /* I3C FCLK 60M/5 = 12M */
    CLOCK_Select(kI3C_Clk_From_Fro);
    CLOCK_SetI3CFClkDiv(5);

    CLOCK_Select(kI3C_TC_Clk_From_LpOsc);
    CLOCK_SetI3CTCClkDiv(1);
    CLOCK_SetI3CSClkDiv(1);

    BOARD_InitPins();
    BOARD_BootClockFRO60M();
    BOARD_InitDebugConsole();

    PRINTF("\r\nI3C board2board DMA example -- Master transfer.\r\n");

    /* First byte in txBuff is data length of the transmitting data. */
    g_master_txBuff[0] = I3C_DATA_LENGTH;
    for (uint32_t i = 1U; i < I3C_PACKET_LENGTH; i++)
    {
        g_master_txBuff[i] = i - 1;
    }

    PRINTF("\r\nMaster will send data :\r\n");
    for (uint32_t i = 0U; i < I3C_DATA_LENGTH; i++)
    {
        PRINTF("0x%2x  ", g_master_txBuff[i + 1]);
        if (i % 8U == 7U)
        {
            PRINTF("\r\n");
        }
    }

    I3C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Hz.i2cBaud          = EXAMPLE_I2C_BAUDRATE;
    masterConfig.baudRate_Hz.i3cPushPullBaud  = EXAMPLE_I3C_PP_BAUDRATE;
    masterConfig.baudRate_Hz.i3cOpenDrainBaud = EXAMPLE_I3C_OD_BAUDRATE;
    masterConfig.enableOpenDrainStop          = false;
    I3C_MasterInit(EXAMPLE_MASTER, &masterConfig, I3C_MASTER_CLOCK_FREQUENCY);

    /* Create DMA handle for I3C Tx/Rx. */
    DMA_Init(EXAMPLE_DMA);
    DMA_EnableChannel(EXAMPLE_DMA, EXAMPLE_I3C_RX_CHANNEL);
    DMA_CreateHandle(&g_rx_dma_handle, EXAMPLE_DMA, EXAMPLE_I3C_RX_CHANNEL);
    DMA_EnableChannel(EXAMPLE_DMA, EXAMPLE_I3C_TX_CHANNEL);
    DMA_CreateHandle(&g_tx_dma_handle, EXAMPLE_DMA, EXAMPLE_I3C_TX_CHANNEL);

    /* Create I3C master DMA transfer handle. */
    I3C_MasterTransferCreateHandleDMA(EXAMPLE_MASTER, &g_i3c_m_handle, &masterCallback, NULL, &g_rx_dma_handle,
                                      &g_tx_dma_handle);

    PRINTF("\r\nI3C master do dynamic address assignment to the I3C slaves on bus.\r\n");
    /* Reset dynamic address before DAA */
    memset(&masterXfer, 0, sizeof(masterXfer));
    masterXfer.slaveAddress   = 0x7EU; /* Broadcast address */
    masterXfer.subaddress     = 0x06U; /* CCC command RSTDAA */
    masterXfer.subaddressSize = 1U;
    masterXfer.direction      = kI3C_Write;
    masterXfer.busType        = kI3C_TypeI3CSdr;
    masterXfer.flags          = kI3C_TransferDefaultFlag;
    masterXfer.ibiResponse    = kI3C_IbiRespAckMandatory;
    result                    = I3C_MasterTransferDMA(EXAMPLE_MASTER, &g_i3c_m_handle, &masterXfer);
    if (kStatus_Success != result)
    {
        PRINTF("I3C DMA transfer start with error code %u!\r\n", result);
        return result;
    }
    while (!g_masterCompletionFlag)
    {
        if (g_completionStatus != kStatus_Success)
        {
            PRINTF("I3C DMA transfer gets error code %u!\r\n", g_completionStatus);
            return -1;
        }
    }
    g_masterCompletionFlag = false;

    uint8_t addressList[6] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36};
    result                 = I3C_MasterProcessDAA(EXAMPLE_MASTER, addressList, sizeof(addressList));
    if (result != kStatus_Success)
    {
        PRINTF("I3C ProcessDAA fails with error code %u!\r\n", result);
        return -1;
    }

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
    if (slaveAddr == 0U)
    {
        PRINTF("Fails to assign dynamic address!\r\n");
        return -1;
    }
    PRINTF("I3C master dynamic address assignment done.\r\n");

    /* Prepare for In-band interrupt. */
    i3c_register_ibi_addr_t ibiRecord = {.address = {slaveAddr}, .ibiHasPayload = true};
    I3C_MasterRegisterIBI(EXAMPLE_MASTER, &ibiRecord);

    PRINTF("\r\nStart to do I3C master transfer in I3C SDR mode.\r\n");
    memset(&masterXfer, 0, sizeof(masterXfer));
    masterXfer.slaveAddress = slaveAddr;
    masterXfer.data         = g_master_txBuff;
    masterXfer.dataSize     = I3C_PACKET_LENGTH;
    masterXfer.direction    = kI3C_Write;
    masterXfer.busType      = kI3C_TypeI3CSdr;
    masterXfer.flags        = kI3C_TransferDefaultFlag;
    masterXfer.ibiResponse  = kI3C_IbiRespAckMandatory;
    result                  = I3C_MasterTransferDMA(EXAMPLE_MASTER, &g_i3c_m_handle, &masterXfer);
    if (kStatus_Success != result)
    {
        PRINTF("I3C DMA transfer start with error code %u!\r\n", result);
        return result;
    }
    while (!g_masterCompletionFlag)
    {
        if (g_completionStatus != kStatus_Success)
        {
            PRINTF("I3C DMA transfer gets error code %u!\r\n", g_completionStatus);
            return -1;
        }
    }
    g_masterCompletionFlag = false;

    PRINTF("\r\nI3C master wait for slave IBI event to notify the slave transmit size.\r\n");
    while (!g_ibiWonFlag)
    {
        __NOP();
    }
    g_ibiWonFlag       = false;
    g_completionStatus = kStatus_Success;
    PRINTF("I3C master received slave IBI event, %d byte(s), value 0x%x.\r\n", g_ibiPayloadSize, g_ibiBuff[0]);

    memset(g_master_rxBuff, 0, sizeof(g_master_rxBuff));
    masterXfer.slaveAddress = slaveAddr;
    masterXfer.data         = g_master_rxBuff;
    masterXfer.dataSize     = g_ibiBuff[0];
    masterXfer.direction    = kI3C_Read;
    masterXfer.busType      = kI3C_TypeI3CSdr;
    masterXfer.flags        = kI3C_TransferDefaultFlag;
    masterXfer.ibiResponse  = kI3C_IbiRespAckMandatory;
    result                  = I3C_MasterTransferDMA(EXAMPLE_MASTER, &g_i3c_m_handle, &masterXfer);
    if (kStatus_Success != result)
    {
        PRINTF("I3C DMA transfer start with error code %u!\r\n", result);
        return result;
    }
    while (!g_masterCompletionFlag)
    {
        if (g_completionStatus != kStatus_Success)
        {
            PRINTF("I3C DMA transfer gets error code %u!\r\n", g_completionStatus);
            return -1;
        }
    }
    g_masterCompletionFlag = false;

    for (uint32_t i = 0U; i < g_master_txBuff[0]; i++)
    {
        if (g_master_rxBuff[i] != g_master_txBuff[i + 1])
        {
            PRINTF("\r\nError occurred in the transfer! \r\n");
            return -1;
        }
    }
    PRINTF("\r\nI3C master transfer successful in I3C SDR mode.\r\n");

#if defined(EXAMPLE_I3C_HDR_SUPPORT) && (EXAMPLE_I3C_HDR_SUPPORT)
    PRINTF("\r\nStart to do I3C master transfer in I3C HDR mode.\r\n");

    /* Wait for IBI event. */
    while (!g_ibiWonFlag)
    {
    }
    g_ibiWonFlag = false;
    g_completionStatus = kStatus_Success;

    memset(&masterXfer, 0, sizeof(masterXfer));
    masterXfer.slaveAddress = slaveAddr;
    masterXfer.subaddress     = 0x0U; /* HDR-DDR command */
    masterXfer.subaddressSize = 1U;
    masterXfer.data         = g_master_txBuff;
    masterXfer.dataSize     = I3C_PACKET_LENGTH;
    masterXfer.direction    = kI3C_Write;
    masterXfer.busType      = kI3C_TypeI3CDdr;
    masterXfer.flags        = kI3C_TransferDefaultFlag;
    masterXfer.ibiResponse  = kI3C_IbiRespAckMandatory;

    result = I3C_MasterTransferDMA(EXAMPLE_MASTER, &g_i3c_m_handle, &masterXfer);
    if (kStatus_Success != result)
    {
        PRINTF("I3C DMA transfer start with error code %u!\r\n", result);
        return result;
    }
    while (!g_masterCompletionFlag)
    {
        if (g_completionStatus != kStatus_Success)
        {
            PRINTF("I3C DMA transfer gets error code %u!\r\n", g_completionStatus);
            return -1;
        }
    }
    g_masterCompletionFlag = false;

    /* Wait for IBI event. */
    while (g_completionStatus != kStatus_I3C_IBIWon)
    {
    }
    g_completionStatus = kStatus_Success;

    memset(g_master_rxBuff, 0, sizeof(g_master_rxBuff));
    masterXfer.slaveAddress = slaveAddr;
    masterXfer.subaddress     = 0x80U; /* HDR-DDR command */
    masterXfer.subaddressSize = 1U;
    masterXfer.data         = g_master_rxBuff;
    masterXfer.dataSize     = I3C_DATA_LENGTH;
    masterXfer.direction    = kI3C_Read;
    masterXfer.busType      = kI3C_TypeI3CDdr;
    masterXfer.flags        = kI3C_TransferDefaultFlag;
    masterXfer.ibiResponse  = kI3C_IbiRespAckMandatory;

    result = I3C_MasterTransferDMA(EXAMPLE_MASTER, &g_i3c_m_handle, &masterXfer);
    if (kStatus_Success != result)
    {
        return result;
    }
    while ((!g_masterCompletionFlag) && (g_completionStatus == kStatus_Success))
    {
    }
    result = g_completionStatus;
    if (result != kStatus_Success)
    {
        return -1;
    }
    g_masterCompletionFlag = false;

    PRINTF("Receive sent data from slave :\r\n");
    for (uint32_t i = 0U; i < I3C_DATA_LENGTH; i++)
    {
        PRINTF("0x%2x  ", g_master_rxBuff[i]);
        if (i % 8U == 7U)
        {
            PRINTF("\r\n");
        }
    }

    for (uint32_t i = 0U; i < g_master_txBuff[0]; i++)
    {
        if (g_master_rxBuff[i] != g_master_txBuff[i + 1])
        {
            PRINTF("\r\nError occurred in the transfer!\r\n");
            return -1;
        }
    }
    PRINTF("\r\nI3C master transfer successful in I3C HDR mode.\r\n");
#endif

    while (1)
    {
    }
}
