/*
 * Copyright 2017-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*  SDK Included Files */
#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_i2c.h"
#include "fsl_dma.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_I2C_MASTER                 I2C0
#define EXAMPLE_DMA                        DMA0
#define EXAMPLE_I2C_MASTER_CHANNEL         11
#define EXAMPLE_I2C_MASTER_CLOCK_FREQUENCY (12000000UL)
#define EXAMPLE_I2C_MASTER_SLAVE_ADDR_7BIT (0x7EU)
#define EXAMPLE_I2C_BAUDRATE               (100000U) /* 100K */
#define EXAMPLE_I2C_DATA_LENGTH            (32)      /* MAX is 256 */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t g_master_txBuff[EXAMPLE_I2C_DATA_LENGTH];
uint8_t g_master_rxBuff[EXAMPLE_I2C_DATA_LENGTH];

dma_handle_t g_dmaHandle;
volatile bool g_MasterCompletionFlag = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void I2C_DmaCallback(dma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    /* Signal transfer success when received success status. */
    if (transferDone)
    {
        g_MasterCompletionFlag = true;
    }
}

static void EXAMPLE_MasterStartDMATransfer(dma_handle_t *dmaHandle, i2c_master_transfer_t *xfer)
{
    assert(dmaHandle);
    assert(xfer);

    i2c_master_transfer_t *transfer;
    dma_transfer_config_t xferConfig;

    transfer = xfer;

    /* set start flag for later use */
    EXAMPLE_I2C_MASTER->MSTDAT = (uint32_t)(transfer->slaveAddress << 1) | (transfer->direction);
    EXAMPLE_I2C_MASTER->MSTCTL = I2C_MSTCTL_MSTSTART_MASK | I2C_MSTCTL_MSTDMA_MASK;

    switch (transfer->direction)
    {
        case kI2C_Write:
            DMA_PrepareTransfer(&xferConfig, g_master_txBuff, (void *)&EXAMPLE_I2C_MASTER->MSTDAT, sizeof(uint8_t),
                                EXAMPLE_I2C_DATA_LENGTH, kDMA_MemoryToPeripheral, NULL);
            break;

        case kI2C_Read:
            DMA_PrepareTransfer(&xferConfig, (void *)&EXAMPLE_I2C_MASTER->MSTDAT, g_master_rxBuff, sizeof(uint8_t),
                                EXAMPLE_I2C_DATA_LENGTH - 1, kDMA_PeripheralToMemory, NULL);
            break;

        default:
            /* This should never happen */
            assert(0);
            break;
    }

    DMA_SubmitTransfer(dmaHandle, &xferConfig);
    DMA_StartTransfer(dmaHandle);
}

void I2C_MasterReadLastByte(I2C_Type *base, void *rxBuff)
{
    uint8_t *buf = (uint8_t *)(rxBuff);

    assert(rxBuff);

    while (!(I2C_STAT_MSTCODE_RXREADY == (base->STAT & I2C_STAT_MSTSTATE_MASK) >> I2C_STAT_MSTSTATE_SHIFT))
    {
    }

    /* ready to receive next byte */
    *(buf) = base->MSTDAT;
}

/*!
 * @brief Main function
 */
int main(void)
{
    i2c_master_config_t masterConfig;
    i2c_master_transfer_t masterXfer;

    /* Enable clock of uart0. */
    CLOCK_EnableClock(kCLOCK_Uart0);
    /* Ser DIV of uart0. */
    CLOCK_SetClkDivider(kCLOCK_DivUsartClk, 1U);
    /* Enable clock of i2c0. */
    CLOCK_EnableClock(kCLOCK_I2c0);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    DMA_Init(EXAMPLE_DMA);
    DMA_EnableChannel(EXAMPLE_DMA, EXAMPLE_I2C_MASTER_CHANNEL);
    DMA_CreateHandle(&g_dmaHandle, EXAMPLE_DMA, EXAMPLE_I2C_MASTER_CHANNEL);
    DMA_SetCallback(&g_dmaHandle, I2C_DmaCallback, NULL);

    PRINTF("\r\nI2C board2board DMA example -- Master transfer.\r\n");

    /* Set up i2c master to send data to slave*/
    /* First data in txBuff is data length of the transmiting data. */
    for (uint32_t i = 0U; i < EXAMPLE_I2C_DATA_LENGTH; i++)
    {
        g_master_txBuff[i] = i;
    }

    PRINTF("Master will send data :");
    for (uint32_t i = 0U; i < EXAMPLE_I2C_DATA_LENGTH; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_master_txBuff[i]);
    }
    PRINTF("\r\n\r\n");

    /*
     * masterConfig.baudRate_Bps = 100000U;
     * masterConfig.enableStopHold = false;
     * masterConfig.glitchFilterWidth = 0U;
     * masterConfig.enableMaster = true;
     */
    I2C_MasterGetDefaultConfig(&masterConfig);

    /* Change the default baudrate configuration */
    masterConfig.baudRate_Bps = EXAMPLE_I2C_BAUDRATE;

    /* Initialize the I2C master peripheral */
    I2C_MasterInit(EXAMPLE_I2C_MASTER, &masterConfig, EXAMPLE_I2C_MASTER_CLOCK_FREQUENCY);

    memset(&masterXfer, 0, sizeof(masterXfer));

    /* data = g_master_txBuff - write to slave.
      start + slaveaddress(w) + length of data buffer + data buffer + stop*/
    masterXfer.slaveAddress = EXAMPLE_I2C_MASTER_SLAVE_ADDR_7BIT;
    masterXfer.direction    = kI2C_Write;
    masterXfer.data         = g_master_txBuff;
    masterXfer.dataSize     = EXAMPLE_I2C_DATA_LENGTH;
    masterXfer.flags        = kI2C_TransferDefaultFlag;

    EXAMPLE_MasterStartDMATransfer(&g_dmaHandle, &masterXfer);

    /*  wait for transfer completed. */
    while (!g_MasterCompletionFlag)
    {
    }

    EXAMPLE_I2C_MASTER->MSTCTL = 0;
    I2C_MasterStop(EXAMPLE_I2C_MASTER);

    g_MasterCompletionFlag = false;

    PRINTF("Receive sent data from slave :");

    /* data = g_master_rxBuff - read from slave.
      start + slaveaddress(w) + repeated start + slaveaddress(r) + rx data buffer + stop */
    masterXfer.slaveAddress = EXAMPLE_I2C_MASTER_SLAVE_ADDR_7BIT;
    masterXfer.direction    = kI2C_Read;
    masterXfer.data         = g_master_rxBuff;
    masterXfer.dataSize     = EXAMPLE_I2C_DATA_LENGTH;
    masterXfer.flags        = kI2C_TransferDefaultFlag;

    EXAMPLE_MasterStartDMATransfer(&g_dmaHandle, &masterXfer);

    /*  Reset master completion flag to false. */
    g_MasterCompletionFlag = false;

    /*  Wait for transfer completed. */
    while (!g_MasterCompletionFlag)
    {
    }
    /* The I2C hardware will automatic read a byte after send a STOP, the I2C may need to send a STOP when last byte was
     * sent, so create a polling mode to send the last byte. */
    EXAMPLE_I2C_MASTER->MSTCTL = 0;

    I2C_MasterReadLastByte(EXAMPLE_I2C_MASTER, &g_master_rxBuff[EXAMPLE_I2C_DATA_LENGTH - 1]);

    I2C_MasterStop(EXAMPLE_I2C_MASTER);

    g_MasterCompletionFlag = false;

    for (uint32_t i = 0U; i < EXAMPLE_I2C_DATA_LENGTH; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_master_rxBuff[i]);
    }
    PRINTF("\r\n\r\n");

    /* Transfer completed. Check the data.*/
    for (uint32_t i = 0U; i < EXAMPLE_I2C_DATA_LENGTH; i++)
    {
        if (g_master_rxBuff[i] != g_master_txBuff[i])
        {
            PRINTF("\r\nError occurred in the transfer !\r\n");
            break;
        }
    }

    PRINTF("\r\nEnd of I2C example .\r\n");
    while (1)
    {
    }
}
