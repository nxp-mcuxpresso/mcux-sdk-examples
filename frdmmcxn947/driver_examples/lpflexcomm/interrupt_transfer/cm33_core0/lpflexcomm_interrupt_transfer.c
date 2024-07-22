/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*  Standard C Included Files */
#include <stdio.h>
#include <string.h>
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_lpi2c.h"
#include "fsl_lpuart.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_I2C_MASTER LPI2C1
#define EXAMPLE_I2C_SLAVE  LPI2C2

#define EXAMPLE_LPUART1 LPUART1
#define EXAMPLE_LPUART2 LPUART2

#define EXAMPLE_LPFLEXCOMM_INSTANCE1 1U
#define EXAMPLE_LPFLEXCOMM_INSTANCE2 2U

/* Get frequency of lpi2c clock */
#define EXAMPLE_CLOCK_FREQUENCY CLOCK_GetLPFlexCommClkFreq(1u)

#define EXAMPLE_LPI2C_MASTER_CLOCK_FREQUENCY EXAMPLE_CLOCK_FREQUENCY
#define EXAMPLE_LPI2C_SLAVE_CLOCK_FREQUENCY  EXAMPLE_CLOCK_FREQUENCY
#define I2C_MASTER_SLAVE_ADDR_7BIT (0x7EU)
#define I2C_BAUDRATE               (100000) /* 100K */

#define I2C_DATA_LENGTH (32) /* MAX is 256 */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void lpi2c_slave_callback(LPI2C_Type *base, lpi2c_slave_transfer_t *xfer, void *param);

/* LPUART user callback */
static void lpuart1_callback(LPUART_Type *base, lpuart_handle_t *handle, status_t status, void *userData);
static void lpuart2_callback(LPUART_Type *base, lpuart_handle_t *handle, status_t status, void *userData);

static void EXAMPLE_TransferDataCheck();
static void EXAMPLE_PrintAndClearArray(bool clearMaster);

/*******************************************************************************
 * Variables
 ******************************************************************************/


uint8_t g_slave_buff[I2C_DATA_LENGTH];
uint8_t g_master_buff[I2C_DATA_LENGTH];

lpi2c_master_handle_t g_m_handle;
lpi2c_slave_handle_t g_s_handle;

volatile bool g_slaveCompleted = false;

lpuart_handle_t g_lpuart1Handle;
lpuart_handle_t g_lpuart2Handle;

volatile bool txOnGoing1 = false;
volatile bool rxOnGoing1 = false;
volatile bool txOnGoing2 = false;
volatile bool rxOnGoing2 = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void lpi2c_slave_callback(LPI2C_Type *base, lpi2c_slave_transfer_t *xfer, void *param)
{
    switch (xfer->event)
    {
        /* Setup the slave receive buffer */
        case kLPI2C_SlaveReceiveEvent:
            xfer->data     = g_slave_buff;
            xfer->dataSize = I2C_DATA_LENGTH;
            break;

        /* The master has sent a stop transition on the bus */
        case kLPI2C_SlaveCompletionEvent:
            g_slaveCompleted = true;
            break;

        /* Transmit event not handled in this demo */
        case kLPI2C_SlaveTransmitEvent:
            /*  Update information for transmit process */
            xfer->data     = g_slave_buff;
            xfer->dataSize = I2C_DATA_LENGTH;
        default:
            break;
    }
}

static void lpuart1_callback(LPUART_Type *base, lpuart_handle_t *handle, status_t status, void *userData)
{
    userData = userData;

    if (kStatus_LPUART_TxIdle == status)
    {
        txOnGoing1 = false;
    }

    if (kStatus_LPUART_RxIdle == status)
    {
        rxOnGoing1 = false;
    }
}

static void lpuart2_callback(LPUART_Type *base, lpuart_handle_t *handle, status_t status, void *userData)
{
    userData = userData;

    if (kStatus_LPUART_TxIdle == status)
    {
        txOnGoing2 = false;
    }

    if (kStatus_LPUART_RxIdle == status)
    {
        rxOnGoing2 = false;
    }
}

static void EXAMPLE_TransferDataCheck()
{
    uint32_t i;
    /* Transfer completed. Check the data. */
    for (i = 0; i < I2C_DATA_LENGTH; i++)
    {
        if (g_slave_buff[i] != g_master_buff[i])
        {
            PRINTF("\r\nError occurred in this transfer ! \r\n");
            break;
        }
    }

    if (i == I2C_DATA_LENGTH)
    {
        PRINTF("\r\nTransfer all data matched! \r\n");
    }
}

static void EXAMPLE_PrintAndClearArray(bool clearMaster)
{
    for (uint32_t i = 0; i < I2C_DATA_LENGTH; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        if (clearMaster)
        {
            PRINTF("0x%2x  ", g_slave_buff[i]);
            g_master_buff[i] = 0;
        }
        else
        {
            PRINTF("0x%2x  ", g_master_buff[i]);
            g_slave_buff[i] = 0;
        }
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t i = 0;
    lpi2c_slave_config_t slaveConfig;
    lpi2c_master_transfer_t masterXfer = {0};
    status_t reVal                     = kStatus_Fail;

    lpuart_config_t config  = {0};
    lpuart_transfer_t mxfer = {0};
    lpuart_transfer_t sxfer = {0};

    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach FRO 12M to FLEXCOMM1 */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom1Clk, 1u);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM1);

    /* attach FRO 12M to FLEXCOMM2 */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 1u);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

#if defined(LPFLEXCOMM_INIT_NOT_USED_IN_DRIVER) && LPFLEXCOMM_INIT_NOT_USED_IN_DRIVER
    /* Init lpflexcomm first. */
    LP_FLEXCOMM_Init(EXAMPLE_LPFLEXCOMM_INSTANCE1, LP_FLEXCOMM_PERIPH_LPI2CAndLPUART);
    LP_FLEXCOMM_Init(EXAMPLE_LPFLEXCOMM_INSTANCE2, LP_FLEXCOMM_PERIPH_LPI2CAndLPUART);
#endif

    PRINTF(
        " \r\nLPFLEXCOMM example.\r\n"
        "In this example 2 lpflexcomm instances are used.\r\n"
        "First the lpflexcomm instances are used as I2C, I2C master of instance 1 sends data to I2C slave of instance"
        "2.\r\n"
        "Then the lpflexcomm instances are used as UART, UART2 sends data back to UART1.\r\n"
        "The next round UART1 sends data to UART2, then lpflexcomm instances switch back to I2C mode, \r\n"
        "I2C master of instance 1 reads data from I2C slave of instance 2 of the data received as UART.\r\n");

    /* Set up i2c slave first*/
    /*
     * slaveConfig.address0 = 0U;
     * slaveConfig.address1 = 0U;
     * slaveConfig.addressMatchMode = kLPI2C_MatchAddress0;
     * slaveConfig.filterDozeEnable = true;
     * slaveConfig.filterEnable = true;
     * slaveConfig.enableGeneralCall = false;
     * slaveConfig.ignoreAck = false;
     * slaveConfig.enableReceivedAddressRead = false;
     * slaveConfig.sdaGlitchFilterWidth_ns = 0;
     * slaveConfig.sclGlitchFilterWidth_ns = 0;
     * slaveConfig.dataValidDelay_ns = 0;
     * slaveConfig.clockHoldTime_ns = 0;
     */
    LPI2C_SlaveGetDefaultConfig(&slaveConfig);

    /* Change the slave address */
    slaveConfig.address0 = I2C_MASTER_SLAVE_ADDR_7BIT;

    /* Initialize the LPI2C slave peripheral */
    LPI2C_SlaveInit(EXAMPLE_I2C_SLAVE, &slaveConfig, EXAMPLE_LPI2C_SLAVE_CLOCK_FREQUENCY);

    /* Create the LPI2C handle for the non-blocking transfer */
    LPI2C_SlaveTransferCreateHandle(EXAMPLE_I2C_SLAVE, &g_s_handle, lpi2c_slave_callback, NULL);

    /* Start accepting I2C transfers on the LPI2C slave peripheral */
    reVal = LPI2C_SlaveTransferNonBlocking(EXAMPLE_I2C_SLAVE, &g_s_handle,
                                           kLPI2C_SlaveReceiveEvent | kLPI2C_SlaveCompletionEvent);
    if (reVal != kStatus_Success)
    {
        return -1;
    }
    /* Set up i2c master to send data to slave*/
    for (i = 0; i < I2C_DATA_LENGTH; i++)
    {
        g_master_buff[i] = i;
    }

    /* Display the data the master will send */
    PRINTF("Master will send data :");
    for (i = 0; i < I2C_DATA_LENGTH; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_master_buff[i]);
    }

    PRINTF("\r\n\r\n");

    lpi2c_master_config_t masterConfig;

    /*
     * masterConfig.debugEnable = false;
     * masterConfig.ignoreAck = false;
     * masterConfig.pinConfig = kLPI2C_2PinOpenDrain;
     * masterConfig.baudRate_Hz = 100000U;
     * masterConfig.busIdleTimeout_ns = 0;
     * masterConfig.pinLowTimeout_ns = 0;
     * masterConfig.sdaGlitchFilterWidth_ns = 0;
     * masterConfig.sclGlitchFilterWidth_ns = 0;
     */
    LPI2C_MasterGetDefaultConfig(&masterConfig);

    /* Change the default baudrate configuration */
    masterConfig.baudRate_Hz = I2C_BAUDRATE;

    /* Initialize the LPI2C master peripheral */
    LPI2C_MasterInit(EXAMPLE_I2C_MASTER, &masterConfig, EXAMPLE_LPI2C_MASTER_CLOCK_FREQUENCY);

    /* Create the LPI2C handle for the non-blocking transfer */
    LPI2C_MasterTransferCreateHandle(EXAMPLE_I2C_MASTER, &g_m_handle, NULL, NULL);

    /* lpuart init start */
    LPUART_GetDefaultConfig(&config);
    config.baudRate_Bps = BOARD_DEBUG_UART_BAUDRATE;
    config.enableTx     = true;
    config.enableRx     = true;
    mxfer.data          = g_master_buff;
    mxfer.dataSize      = I2C_DATA_LENGTH;
    sxfer.data          = g_slave_buff;
    sxfer.dataSize      = I2C_DATA_LENGTH;
    LPUART_Init(EXAMPLE_LPUART1, &config, EXAMPLE_CLOCK_FREQUENCY);
    LPUART_TransferCreateHandle(EXAMPLE_LPUART1, &g_lpuart1Handle, lpuart1_callback, NULL);

    LPUART_Init(EXAMPLE_LPUART2, &config, EXAMPLE_CLOCK_FREQUENCY);
    LPUART_TransferCreateHandle(EXAMPLE_LPUART2, &g_lpuart2Handle, lpuart2_callback, NULL);

    PRINTF("\r\nWill send data using LPI2C --- MASTER TO SLAVE\r\n");
    /* Setup the master transfer */
    masterXfer.slaveAddress   = I2C_MASTER_SLAVE_ADDR_7BIT;
    masterXfer.direction      = kLPI2C_Write;
    masterXfer.subaddress     = 0;
    masterXfer.subaddressSize = 0;
    masterXfer.data           = g_master_buff;
    masterXfer.dataSize       = I2C_DATA_LENGTH;
    masterXfer.flags          = kLPI2C_TransferDefaultFlag;

    /* Send master non-blocking data to slave */
    reVal = LPI2C_MasterTransferNonBlocking(EXAMPLE_I2C_MASTER, &g_m_handle, &masterXfer);
    if (reVal != kStatus_Success)
    {
        return -1;
    }
    /* Wait for the transfer to complete. */
    while (!g_slaveCompleted)
    {
    }
    EXAMPLE_TransferDataCheck();
    PRINTF("\r\nSlave received data by LPI2C:");
    EXAMPLE_PrintAndClearArray(true);
    PRINTF("\r\n\r\nWill reply data using LPUART --- instance 2 TO instance 1\r\n");
    /* start back by lpuart */
    /* Master Reciver Nonblocking */
    rxOnGoing1 = true;
    LPUART_TransferReceiveNonBlocking(EXAMPLE_LPUART1, &g_lpuart1Handle, &mxfer, NULL);

    /* Slave Send Nonblocking */
    txOnGoing2 = true;
    LPUART_TransferSendNonBlocking(EXAMPLE_LPUART2, &g_lpuart2Handle, &sxfer);
    while (rxOnGoing1 || txOnGoing2)
    {
    }
    EXAMPLE_TransferDataCheck();
    PRINTF("\r\nMaster received data by LPUART:");
    EXAMPLE_PrintAndClearArray(false);
    PRINTF("\r\n\r\nWill reply data using LPUART --- instance 1 TO instance 2\r\n");

    /* Slave Reciver Nonblocking */
    rxOnGoing2 = true;
    LPUART_TransferReceiveNonBlocking(EXAMPLE_LPUART2, &g_lpuart2Handle, &sxfer, NULL);

    txOnGoing1 = true;
    LPUART_TransferSendNonBlocking(EXAMPLE_LPUART1, &g_lpuart1Handle, &mxfer);

    while (txOnGoing1 || rxOnGoing2)
    {
    }
    EXAMPLE_TransferDataCheck();
    PRINTF("\r\nSlave received data by LPUART:");
    EXAMPLE_PrintAndClearArray(true);
    PRINTF("\r\n\r\nWill reply data using LPI2C --- SLAVE TO MASTER\r\n");

    masterXfer.direction = kLPI2C_Read;
    /* Send master non-blocking data to slave */
    g_slaveCompleted = false;
    reVal            = LPI2C_MasterTransferNonBlocking(EXAMPLE_I2C_MASTER, &g_m_handle, &masterXfer);

    if (reVal != kStatus_Success)
    {
        return -1;
    }
    /* Wait for the transfer to complete. */
    while (!g_slaveCompleted)
    {
    }
    EXAMPLE_TransferDataCheck();
    PRINTF("\r\nMaster received data by LPI2C:");
    EXAMPLE_PrintAndClearArray(false);
    PRINTF("\r\nDEMO END!\r\n");

    /* Hang at the end */
    while (1)
    {
    }
}
