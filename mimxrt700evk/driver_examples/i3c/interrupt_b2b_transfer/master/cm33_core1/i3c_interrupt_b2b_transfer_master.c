/*
 * Copyright 2021-2024 NXP
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
#define EXAMPLE_MASTER             I3C3
#define EXAMPLE_I2C_BAUDRATE       400000
#define EXAMPLE_I3C_OD_BAUDRATE    750000
#define EXAMPLE_I3C_PP_BAUDRATE    1500000
#define WAIT_TIME                  1000
#define I3C_MASTER_CLOCK_FREQUENCY CLOCK_GetI3cClkFreq()
#define EXAMPLE_I3C_IBI_SUPPORT    1
#define EXAMPLE_I3C_HDR_SUPPORT    1
#ifndef I3C_MASTER_SLAVE_ADDR_7BIT
#define I3C_MASTER_SLAVE_ADDR_7BIT 0x1EU
#endif
#ifndef I3C_DATA_LENGTH
#define I3C_DATA_LENGTH 33U
#endif
#ifndef EXAMPLE_I3C_HDR_SUPPORT
#define EXAMPLE_I3C_HDR_SUPPORT 0
#endif
#ifndef WAIT_TIME
#define WAIT_TIME 1000
#endif

#define CCC_RSTDAA  0x06U
#define CCC_SETDASA 0x87U

#define I3C_BROADCAST_ADDR 0x7EU
#define I3C_VENDOR_ID      0x11BU

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void i3c_master_ibi_callback(I3C_Type *base,
                                    i3c_master_handle_t *handle,
                                    i3c_ibi_type_t ibiType,
                                    i3c_ibi_state_t ibiState);
static void i3c_master_callback(I3C_Type *base, i3c_master_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t g_master_txBuff[I3C_DATA_LENGTH];
uint8_t g_master_rxBuff[I3C_DATA_LENGTH];
uint8_t g_master_ibiBuff[8];
i3c_master_handle_t g_i3c_m_handle;
const i3c_master_transfer_callback_t masterCallback = {
    .slave2Master = NULL, .ibiCallback = i3c_master_ibi_callback, .transferComplete = i3c_master_callback};
volatile bool g_masterCompletionFlag = false;
volatile bool g_ibiWonFlag           = false;
volatile status_t g_completionStatus = kStatus_Success;
static uint8_t g_ibiUserBuff[8];

/*******************************************************************************
 * Code
 ******************************************************************************/
static void i3c_master_ibi_callback(I3C_Type *base,
                                    i3c_master_handle_t *handle,
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
                memcpy(g_ibiUserBuff, (void *)handle->ibiBuff, handle->ibiPayloadSize);
            }
            break;

        default:
            assert(false);
            break;
    }
}

static void i3c_master_callback(I3C_Type *base, i3c_master_handle_t *handle, status_t status, void *userData)
{
    if (status == kStatus_I3C_IBIWon)
    {
        g_ibiWonFlag = true;
    }
    else
    {
        /* Signal transfer complete when received complete status. */
        g_masterCompletionFlag = true;
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
    uint8_t slaveAddr = 0;

    BOARD_InitAHBSC();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_AttachClk(kSENSE_BASE_to_I3C23);
    CLOCK_SetClkDiv(kCLOCK_DivI3c23Clk, 4U);

    PRINTF("\r\nI3C board2board interrupt example -- Master transfer.\r\n");

    /* I3C mode: Set up i3c master to work in I3C mode, send data to slave*/
    /* First data in txBuff is data length of the transmitting data. */
    g_master_txBuff[0] = I3C_DATA_LENGTH - 1U;
    for (uint32_t i = 1U; i < I3C_DATA_LENGTH; i++)
    {
        g_master_txBuff[i] = i - 1;
    }

    PRINTF("\r\nStart to do I3C master transfer in I2C mode.\r\n");

    PRINTF("Master will send data :");
    for (uint32_t i = 0U; i < I3C_DATA_LENGTH - 1U; i++)
    {
        if (i % 8U == 0U)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_master_txBuff[i + 1U]);
    }
    PRINTF("\r\n\r\n");

    I3C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Hz.i2cBaud          = EXAMPLE_I2C_BAUDRATE;
    masterConfig.baudRate_Hz.i3cPushPullBaud  = EXAMPLE_I3C_PP_BAUDRATE;
    masterConfig.baudRate_Hz.i3cOpenDrainBaud = EXAMPLE_I3C_OD_BAUDRATE;
    masterConfig.enableOpenDrainStop          = false;
    I3C_MasterInit(EXAMPLE_MASTER, &masterConfig, I3C_MASTER_CLOCK_FREQUENCY);
    I3C_MasterTransferCreateHandle(EXAMPLE_MASTER, &g_i3c_m_handle, &masterCallback, NULL);

    /* Start + slaveaddress(w) + subAddress + length of data buffer + data buffer + stop. */
    memset(&masterXfer, 0, sizeof(masterXfer));
    masterXfer.slaveAddress   = I3C_MASTER_SLAVE_ADDR_7BIT;
    masterXfer.direction      = kI3C_Write;
    masterXfer.busType        = kI3C_TypeI2C;
    masterXfer.subaddress     = 0x01;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = g_master_txBuff;
    masterXfer.dataSize       = I3C_DATA_LENGTH;
    masterXfer.flags          = kI3C_TransferDefaultFlag;

    result = I3C_MasterTransferNonBlocking(EXAMPLE_MASTER, &g_i3c_m_handle, &masterXfer);
    if (kStatus_Success != result)
    {
        return result;
    }

    /* Wait for transfer completed. */
    while (!g_masterCompletionFlag)
    {
        __NOP();
    }

    if (g_completionStatus != kStatus_Success)
    {
        PRINTF("\r\nTransfer error %u.\r\n", g_completionStatus);
        return -1;
    }
    g_masterCompletionFlag = false;

    /* Wait until the slave is ready for transmit, wait time depend on user's case.
       Slave devices that need some time to process received byte or are not ready yet to
       send the next byte, can pull the clock low to signal to the master that it should wait.*/
    for (volatile uint32_t i = 0U; i < WAIT_TIME; i++)
    {
        __NOP();
    }

    /* Start + slaveaddress(w) + subAddress + repeated start + slaveaddress(r) + rx data buffer + stop. */
    masterXfer.slaveAddress   = I3C_MASTER_SLAVE_ADDR_7BIT;
    masterXfer.direction      = kI3C_Read;
    masterXfer.busType        = kI3C_TypeI2C;
    masterXfer.subaddress     = 0x01;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = g_master_rxBuff;
    masterXfer.dataSize       = I3C_DATA_LENGTH - 1U;
    masterXfer.flags          = kI3C_TransferDefaultFlag;

    result = I3C_MasterTransferNonBlocking(EXAMPLE_MASTER, &g_i3c_m_handle, &masterXfer);
    if (kStatus_Success != result)
    {
        return result;
    }

    /* Wait for transfer completed. */
    while (!g_masterCompletionFlag)
    {
        __NOP();
    }

    if (g_completionStatus != kStatus_Success)
    {
        PRINTF("\r\nTransfer error %u.\r\n", g_completionStatus);
        return -1;
    }
    g_masterCompletionFlag = false;

    PRINTF("Receive sent data from slave :");
    for (uint32_t i = 0U; i < I3C_DATA_LENGTH - 1U; i++)
    {
        if (i % 8U == 0U)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_master_rxBuff[i]);
    }

    /* Transfer completed. Check the data.*/
    for (uint32_t i = 0U; i < I3C_DATA_LENGTH - 1U; i++)
    {
        if (g_master_rxBuff[i] != g_master_txBuff[i + 1])
        {
            PRINTF("\r\nError occurred in the transfer!\r\n");
            return -1;
        }
    }

    PRINTF("\r\nI3C master transfer successful in I2C mode.\r\n");

    PRINTF("\r\nI3C master do dynamic address assignment to the I3C slaves on bus.");

    /* Reset dynamic address before DAA */
    memset(&masterXfer, 0, sizeof(masterXfer));
    masterXfer.slaveAddress   = I3C_BROADCAST_ADDR;
    masterXfer.subaddress     = CCC_RSTDAA;
    masterXfer.subaddressSize = 1U;
    masterXfer.direction      = kI3C_Write;
    masterXfer.busType        = kI3C_TypeI3CSdr;
    masterXfer.flags          = kI3C_TransferDefaultFlag;
    masterXfer.ibiResponse    = kI3C_IbiRespAckMandatory;
    result                    = I3C_MasterTransferNonBlocking(EXAMPLE_MASTER, &g_i3c_m_handle, &masterXfer);
    if (kStatus_Success != result)
    {
        return result;
    }

    /* Wait for transfer completed. */
    while (!g_masterCompletionFlag)
    {
        __NOP();
    }

    if (g_completionStatus != kStatus_Success)
    {
        PRINTF("\r\nTransfer error %u.\r\n", g_completionStatus);
        return -1;
    }
    g_masterCompletionFlag = false;

#if defined(EXAMPLE_USE_SETDASA_ASSIGN_ADDR) && (EXAMPLE_USE_SETDASA_ASSIGN_ADDR)
    /* Assign dynmic address. */
    memset(&masterXfer, 0, sizeof(masterXfer));
    masterXfer.slaveAddress   = I3C_BROADCAST_ADDR;
    masterXfer.subaddress     = CCC_SETDASA;
    masterXfer.subaddressSize = 1U;
    masterXfer.direction      = kI3C_Write;
    masterXfer.busType        = kI3C_TypeI3CSdr;
    masterXfer.flags          = kI3C_TransferNoStopFlag;
    masterXfer.ibiResponse    = kI3C_IbiRespAckMandatory;

    result = I3C_MasterTransferNonBlocking(EXAMPLE_MASTER, &g_i3c_m_handle, &masterXfer);
    if (kStatus_Success != result)
    {
        return result;
    }

    /* Wait for transfer completed. */
    while (!g_masterCompletionFlag)
    {
        __NOP();
    }

    if (g_completionStatus != kStatus_Success)
    {
        PRINTF("\r\nTransfer error %u.\r\n", g_completionStatus);
        return -1;
    }
    g_masterCompletionFlag = false;

    slaveAddr = 0x30U;
    memset(&masterXfer, 0, sizeof(masterXfer));
    masterXfer.slaveAddress   = I3C_MASTER_SLAVE_ADDR_7BIT;
    masterXfer.subaddress     = slaveAddr << 1U;
    masterXfer.subaddressSize = 1U;
    masterXfer.direction      = kI3C_Write;
    masterXfer.busType        = kI3C_TypeI3CSdr;
    masterXfer.flags          = kI3C_TransferDefaultFlag;
    masterXfer.ibiResponse    = kI3C_IbiRespAckMandatory;

    result = I3C_MasterTransferNonBlocking(EXAMPLE_MASTER, &g_i3c_m_handle, &masterXfer);
    if (kStatus_Success != result)
    {
        return result;
    }

    /* Wait for transfer completed. */
    while (!g_masterCompletionFlag)
    {
        __NOP();
    }

    if (g_completionStatus != kStatus_Success)
    {
        PRINTF("\r\nTransfer error %u.\r\n", g_completionStatus);
        return -1;
    }
    g_masterCompletionFlag = false;
#else
    uint8_t addressList[8] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37};
    result                 = I3C_MasterProcessDAA(EXAMPLE_MASTER, addressList, 8);
    if (result != kStatus_Success)
    {
        return -1;
    }

    i3c_device_info_t *devList;
    uint8_t devIndex;
    uint8_t devCount;
    devList = I3C_MasterGetDeviceListAfterDAA(EXAMPLE_MASTER, &devCount);
    for (devIndex = 0; devIndex < devCount; devIndex++)
    {
        if (devList[devIndex].vendorID == I3C_VENDOR_ID)
        {
            slaveAddr = devList[devIndex].dynamicAddr;
            break;
        }
    }
    if (devIndex == devCount)
    {
        PRINTF("\r\nI3C master dynamic address assignment fails!\r\n");
        return -1;
    }
#endif

    PRINTF("\r\nI3C master dynamic address assignment done.\r\n");

#if defined(EXAMPLE_I3C_IBI_SUPPORT)
    /* Set the I3C In-band interrupt. */
    i3c_register_ibi_addr_t ibiRecord = {.address = {slaveAddr}, .ibiHasPayload = true};
    I3C_MasterRegisterIBI(EXAMPLE_MASTER, &ibiRecord);
#endif

    PRINTF("\r\nStart to do I3C master transfer in I3C SDR mode.\r\n");

    memset(&masterXfer, 0, sizeof(masterXfer));
    masterXfer.slaveAddress = slaveAddr;
    masterXfer.data         = g_master_txBuff;
    masterXfer.dataSize     = I3C_DATA_LENGTH;
    masterXfer.direction    = kI3C_Write;
    masterXfer.busType      = kI3C_TypeI3CSdr;
    masterXfer.flags        = kI3C_TransferDefaultFlag;
    masterXfer.ibiResponse  = kI3C_IbiRespAckMandatory;
    result                  = I3C_MasterTransferNonBlocking(EXAMPLE_MASTER, &g_i3c_m_handle, &masterXfer);
    if (kStatus_Success != result)
    {
        return result;
    }

    /* Wait for transfer completed. */
    while (!g_masterCompletionFlag)
    {
        __NOP();
    }

    if (g_completionStatus != kStatus_Success)
    {
        PRINTF("\r\nTransfer error %u.\r\n", g_completionStatus);
        return -1;
    }
    g_masterCompletionFlag = false;

#if defined(EXAMPLE_I3C_IBI_SUPPORT)
    /* Wait for IBI event. */
    while (!g_ibiWonFlag)
    {
    }
    g_ibiWonFlag = false;
    g_completionStatus = kStatus_Success;
#else
    /* Wait until the slave is ready for transmit, wait time depend on user's case.*/
    for (volatile uint32_t i = 0U; i < WAIT_TIME; i++)
    {
        __NOP();
    }
#endif

    memset(g_master_rxBuff, 0, I3C_DATA_LENGTH);
    masterXfer.slaveAddress = slaveAddr;
    masterXfer.data         = g_master_rxBuff;
    masterXfer.dataSize     = I3C_DATA_LENGTH - 1U;
    masterXfer.direction    = kI3C_Read;
    masterXfer.busType      = kI3C_TypeI3CSdr;
    masterXfer.flags        = kI3C_TransferDefaultFlag;
    masterXfer.ibiResponse  = kI3C_IbiRespAckMandatory;
    result                  = I3C_MasterTransferNonBlocking(EXAMPLE_MASTER, &g_i3c_m_handle, &masterXfer);
    if (kStatus_Success != result)
    {
        return result;
    }

    /* Wait for transfer completed. */
    while (!g_masterCompletionFlag)
    {
        __NOP();
    }

    if (g_completionStatus != kStatus_Success)
    {
        return -1;
    }
    g_masterCompletionFlag = false;

    PRINTF("Receive sent data from slave :");
    for (uint32_t i = 0U; i < (I3C_DATA_LENGTH - 1U); i++)
    {
        if (i % 8U == 0U)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_master_rxBuff[i]);
    }

    for (uint32_t i = 0U; i < g_master_txBuff[0]; i++)
    {
        if (g_master_rxBuff[i] != g_master_txBuff[i + 1])
        {
            PRINTF("\r\nError occurred in the transfer!\r\n");
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
    masterXfer.dataSize     = I3C_DATA_LENGTH;
    masterXfer.direction    = kI3C_Write;
    masterXfer.busType      = kI3C_TypeI3CDdr;
    masterXfer.flags        = kI3C_TransferDefaultFlag;
    masterXfer.ibiResponse  = kI3C_IbiRespAckMandatory;
    result                  = I3C_MasterTransferNonBlocking(EXAMPLE_MASTER, &g_i3c_m_handle, &masterXfer);
    if (kStatus_Success != result)
    {
        return result;
    }

    /* Wait for transfer completed. */
    while (!g_masterCompletionFlag)
    {
        __NOP();
    }

    if (g_completionStatus != kStatus_Success)
    {
        return -1;
    }
    g_masterCompletionFlag = false;

    /* Wait for IBI event. */
    while (g_completionStatus != kStatus_I3C_IBIWon)
    {
    }
    g_completionStatus = kStatus_Success;

    memset(g_master_rxBuff, 0, I3C_DATA_LENGTH);
    masterXfer.slaveAddress = slaveAddr;
    masterXfer.subaddress     = 0x80U; /* HDR-DDR command */
    masterXfer.subaddressSize = 1U;
    masterXfer.data         = g_master_rxBuff;
    masterXfer.dataSize     = I3C_DATA_LENGTH - 1U;
    masterXfer.direction    = kI3C_Read;
    masterXfer.busType      = kI3C_TypeI3CDdr;
    masterXfer.flags        = kI3C_TransferDefaultFlag;
    masterXfer.ibiResponse  = kI3C_IbiRespAckMandatory;
    result                  = I3C_MasterTransferNonBlocking(EXAMPLE_MASTER, &g_i3c_m_handle, &masterXfer);
    if (kStatus_Success != result)
    {
        return result;
    }

    /* Wait for transfer completed. */
    while (!g_masterCompletionFlag)
    {
        __NOP();
    }

    if (g_completionStatus != kStatus_Success)
    {
        return -1;
    }
    g_masterCompletionFlag = false;

    PRINTF("Receive sent data from slave :");
    for (uint32_t i = 0U; i < (I3C_DATA_LENGTH - 1U); i++)
    {
        if (i % 8U == 0U)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_master_rxBuff[i]);
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
