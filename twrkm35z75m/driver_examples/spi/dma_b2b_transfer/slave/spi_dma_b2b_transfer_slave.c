/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_spi_dma.h"
#include "fsl_dmamux.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_SPI_SLAVE        SPI0
#define EXAMPLE_DMA              DMA0
#define EXAMPLE_DMAMUX           DMAMUX
#define EXAMPLE_SPI_TX_CHANNEL   3U
#define EXAMPLE_SPI_RX_CHANNEL   2U
#define EXAMPLE_SPI_TX_SOURCE    kDmaRequestMux0SPI0Tx
#define EXAMPLE_SPI_RX_SOURCE    kDmaRequestMux0SPI0Rx
#define EXAMPLE_SPI_SOURCE_CLOCK kCLOCK_CoreSysClk

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
#define BUFFER_SIZE (64)
static uint8_t buff[BUFFER_SIZE];
static uint8_t sendBuff[BUFFER_SIZE];
static spi_dma_handle_t s_handle;
static dma_handle_t txHandle;
static dma_handle_t rxHandle;
static volatile bool slaveFinished = false;
/*******************************************************************************
 * Code
 ******************************************************************************/
static void slaveCallback(SPI_Type *base, spi_dma_handle_t *handle, status_t status, void *userData)
{
    slaveFinished = true;
}

int main(void)
{
    uint32_t i          = 0;
    uint8_t err         = 0;
    spi_transfer_t xfer = {0};
    spi_slave_config_t userConfig;

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    PRINTF("\r\nSlave is working....\r\n");

    /* Init DMAMUX */
#if FSL_FEATURE_DMA_MODULE_CHANNEL != FSL_FEATURE_DMAMUX_MODULE_CHANNEL
    DMAMUX_Init(EXAMPLE_TX_DMAMUX);
    DMAMUX_Init(EXAMPLE_RX_DMAMUX);
    DMAMUX_SetSource(EXAMPLE_TX_DMAMUX, EXAMPLE_SPI_TX_DMAMUX_CHANNEL, EXAMPLE_SPI_TX_SOURCE);
    DMAMUX_SetSource(EXAMPLE_RX_DMAMUX, EXAMPLE_SPI_RX_DMAMUX_CHANNEL, EXAMPLE_SPI_RX_SOURCE);
    DMAMUX_EnableChannel(EXAMPLE_TX_DMAMUX, EXAMPLE_SPI_TX_DMAMUX_CHANNEL);
    DMAMUX_EnableChannel(EXAMPLE_RX_DMAMUX, EXAMPLE_SPI_RX_DMAMUX_CHANNEL);

    /* Init the DMA module */
    DMA_Init(EXAMPLE_DMA);
    DMA_CreateHandle(&txHandle, EXAMPLE_DMA, EXAMPLE_SPI_TX_DMA_CHANNEL);
    DMA_CreateHandle(&rxHandle, EXAMPLE_DMA, EXAMPLE_SPI_RX_DMA_CHANNEL);
#else
    DMAMUX_Init(EXAMPLE_DMAMUX);
    DMAMUX_SetSource(EXAMPLE_DMAMUX, EXAMPLE_SPI_TX_CHANNEL, EXAMPLE_SPI_TX_SOURCE);
    DMAMUX_SetSource(EXAMPLE_DMAMUX, EXAMPLE_SPI_RX_CHANNEL, EXAMPLE_SPI_RX_SOURCE);
    DMAMUX_EnableChannel(EXAMPLE_DMAMUX, EXAMPLE_SPI_TX_CHANNEL);
    DMAMUX_EnableChannel(EXAMPLE_DMAMUX, EXAMPLE_SPI_RX_CHANNEL);

    /* Init the DMA module */
    DMA_Init(EXAMPLE_DMA);
    DMA_CreateHandle(&txHandle, EXAMPLE_DMA, EXAMPLE_SPI_TX_CHANNEL);
    DMA_CreateHandle(&rxHandle, EXAMPLE_DMA, EXAMPLE_SPI_RX_CHANNEL);
#endif

    /* Init the SPI slave */
    /*
     * userConfig->polarity = kSPI_ClockPolarityActiveHigh;
     * userConfig->phase = kSPI_ClockPhaseFirstEdge;
     * userConfig->direction = kSPI_MsbFirst;
     * userConfig->enableStopInWaitMode = false;
     * userConfig->dataMode = kSPI_8BitMode;
     * userConfig->txWatermark = kSPI_TxFifoOneHalfEmpty;
     * userConfig->rxWatermark = kSPI_RxFifoOneHalfFull;
     */
    SPI_SlaveGetDefaultConfig(&userConfig);
    SPI_SlaveInit(EXAMPLE_SPI_SLAVE, &userConfig);
    SPI_SlaveTransferCreateHandleDMA(EXAMPLE_SPI_SLAVE, &s_handle, slaveCallback, NULL, &txHandle, &rxHandle);
    for (i = 0; i < 64; i++)
    {
        sendBuff[i] = i;
    }

    /* receive data from master */
    xfer.txData   = sendBuff;
    xfer.rxData   = buff;
    xfer.dataSize = BUFFER_SIZE;
    SPI_SlaveTransferDMA(EXAMPLE_SPI_SLAVE, &s_handle, &xfer);

    while (slaveFinished != true)
    {
    }

    for (i = 0; i < BUFFER_SIZE; i++)
    {
        if (buff[i] != i)
        {
            PRINTF("\r\nThe %d number is wrong! It is %d\r\n", i, buff[i]);
            err++;
        }
    }
    PRINTF("\r\n");
    if (err == 0)
    {
        PRINTF("Succeed!\r\n");
    }

    while (1)
    {
    }
}
