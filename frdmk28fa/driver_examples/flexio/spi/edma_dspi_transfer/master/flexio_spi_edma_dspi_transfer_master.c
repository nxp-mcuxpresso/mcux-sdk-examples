/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_dspi.h"
#include "fsl_flexio_spi_edma.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_dmamux.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BOARD_FLEXIO_BASE                    FLEXIO0
#define FLEXIO_SPI_SOUT_PIN                  24U
#define FLEXIO_SPI_SIN_PIN                   25U
#define FLEXIO_SPI_CLK_PIN                   3U
#define FLEXIO_SPI_PCS_PIN                   2U
#define FLEXIO_CLOCK_FREQUENCY               CLOCK_GetFreq(kCLOCK_PllFllSelClk)
#define FLEXIO_DMA_REQUEST_BASE              kDmaRequestMux0Group1FlexIO0Channel0
#define BOARD_DSPI_SLAVE_BASE                SPI0
#define SLAVE_SPI_IRQ                        SPI0_IRQn
#define MASTER_SPI_IRQ                       FLEXIO0_IRQn
#define EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR   DMAMUX0
#define EXAMPLE_FLEXIO_SPI_DMA_DSPI_BASEADDR DMA0
#define FLEXIO_SPI_TX_DMA_DSPI_CHANNEL       16U
#define FLEXIO_SPI_RX_DMA_DSPI_CHANNEL       17U
#define FLEXIO_TX_SHIFTER_INDEX              0U
#define FLEXIO_RX_SHIFTER_INDEX              1U
#define EXAMPLE_TX_DMA_SOURCE                (FLEXIO_DMA_REQUEST_BASE + FLEXIO_TX_SHIFTER_INDEX)
#define EXAMPLE_RX_DMA_SOURCE                (FLEXIO_DMA_REQUEST_BASE + FLEXIO_RX_SHIFTER_INDEX)
#define TRANSFER_SIZE     256U    /*! Transfer dataSize */
#define TRANSFER_BAUDRATE 500000U /*! Transfer baudrate - 500k */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* DSPI user callback */
void FLEXIO_SPI_MasterUserCallback(FLEXIO_SPI_Type *base,
                                   flexio_spi_master_edma_handle_t *handle,
                                   status_t status,
                                   void *userData);
void DSPI_SlaveUserCallback(SPI_Type *base, dspi_slave_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t masterRxData[TRANSFER_SIZE] = {0U};
uint8_t masterTxData[TRANSFER_SIZE] = {0U};
uint8_t slaveRxData[TRANSFER_SIZE]  = {0U};
uint8_t slaveTxData[TRANSFER_SIZE]  = {0U};

FLEXIO_SPI_Type spiDev;
flexio_spi_master_edma_handle_t g_m_handle;
dspi_slave_handle_t g_s_handle;

edma_handle_t txHandle;
edma_handle_t rxHandle;

volatile bool isTransferCompleted = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

void FLEXIO_SPI_MasterUserCallback(FLEXIO_SPI_Type *base,
                                   flexio_spi_master_edma_handle_t *handle,
                                   status_t status,
                                   void *userData)
{
    if (status == kStatus_Success)
    {
        __NOP();
    }
}

void DSPI_SlaveUserCallback(SPI_Type *base, dspi_slave_handle_t *handle, status_t status, void *userData)
{
    if (status == kStatus_Success)
    {
        __NOP();
    }

    if (status == kStatus_DSPI_Error)
    {
        __NOP();
    }

    isTransferCompleted = true;

    PRINTF("This is DSPI slave call back.\r\n");
}

int main(void)
{
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetFlexio0Clock(1U);

    PRINTF("FLEXIO Master edma - DSPI Slave interrupt example start.\r\n");
    PRINTF("This example use one flexio spi as master and one dspi instance as slave on one board.\r\n");
    PRINTF("Master uses edma and slave uses interrupt way.\r\n");
    PRINTF("Please make sure you make the correct line connection. Basically, the connection is:\r\n");
    PRINTF("FLEXIO_SPI_master -- DSPI_slave\r\n");
    PRINTF("   CLK      --    CLK\r\n");
    PRINTF("   PCS      --    PCS\r\n");
    PRINTF("   SOUT     --    SIN\r\n");
    PRINTF("   SIN      --    SOUT\r\n");

    uint32_t errorCount;
    uint32_t i;
    flexio_spi_master_config_t masterConfig;
    dspi_slave_config_t slaveConfig;
    flexio_spi_transfer_t masterXfer;
    dspi_transfer_t slaveXfer;
    dma_request_source_t dma_request_source_tx;
    dma_request_source_t dma_request_source_rx;
    edma_config_t config;

    /* Master config */
    /*
     * masterConfig.enableMaster = true;
     * masterConfig.enableInDoze = false;
     * masterConfig.enableInDebug = true;
     * masterConfig.enableFastAccess = false;
     * masterConfig.baudRate_Bps = 500000U;
     * masterConfig.phase = kFLEXIO_SPI_ClockPhaseFirstEdge;
     * masterConfig.dataMode = kFLEXIO_SPI_8BitMode;
     */
    FLEXIO_SPI_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Bps = 500000U;

    spiDev.flexioBase      = BOARD_FLEXIO_BASE;
    spiDev.SDOPinIndex     = FLEXIO_SPI_SOUT_PIN;
    spiDev.SDIPinIndex     = FLEXIO_SPI_SIN_PIN;
    spiDev.SCKPinIndex     = FLEXIO_SPI_CLK_PIN;
    spiDev.CSnPinIndex     = FLEXIO_SPI_PCS_PIN;
    spiDev.shifterIndex[0] = FLEXIO_TX_SHIFTER_INDEX;
    spiDev.shifterIndex[1] = FLEXIO_RX_SHIFTER_INDEX;
    spiDev.timerIndex[0]   = 0U;
    spiDev.timerIndex[1]   = 1U;

    FLEXIO_SPI_MasterInit(&spiDev, &masterConfig, FLEXIO_CLOCK_FREQUENCY);

    /* Slave config */
    slaveConfig.whichCtar                  = kDSPI_Ctar0;
    slaveConfig.ctarConfig.bitsPerFrame    = 8;
    slaveConfig.ctarConfig.cpol            = kDSPI_ClockPolarityActiveHigh;
    slaveConfig.ctarConfig.cpha            = kDSPI_ClockPhaseFirstEdge;
    slaveConfig.enableContinuousSCK        = false;
    slaveConfig.enableRxFifoOverWrite      = false;
    slaveConfig.enableModifiedTimingFormat = false;
    slaveConfig.samplePoint                = kDSPI_SckToSin0Clock;

    DSPI_SlaveInit(BOARD_DSPI_SLAVE_BASE, &slaveConfig);

    /* Set dspi slave interrupt priority higher. */
    NVIC_SetPriority(MASTER_SPI_IRQ, 1U);
    NVIC_SetPriority(SLAVE_SPI_IRQ, 0U);

    /* Set up the transfer data */
    for (i = 0U; i < TRANSFER_SIZE; i++)
    {
        masterTxData[i] = i % 256U;
        masterRxData[i] = 0U;

        slaveTxData[i] = ~masterTxData[i];
        slaveRxData[i] = 0U;
    }

    isTransferCompleted = false;
    /* Set up slave first */
    DSPI_SlaveTransferCreateHandle(BOARD_DSPI_SLAVE_BASE, &g_s_handle, DSPI_SlaveUserCallback, NULL);

    /*Set slave transfer ready to receive/send data*/
    slaveXfer.txData      = slaveTxData;
    slaveXfer.rxData      = slaveRxData;
    slaveXfer.dataSize    = TRANSFER_SIZE;
    slaveXfer.configFlags = kDSPI_SlaveCtar0;

    DSPI_SlaveTransferNonBlocking(BOARD_DSPI_SLAVE_BASE, &g_s_handle, &slaveXfer);

    /*Init EDMA for example.*/
    DMAMUX_Init(EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR);
    EDMA_GetDefaultConfig(&config);
    EDMA_Init(EXAMPLE_FLEXIO_SPI_DMA_DSPI_BASEADDR, &config);

    dma_request_source_tx = (dma_request_source_t)(EXAMPLE_TX_DMA_SOURCE);
    dma_request_source_rx = (dma_request_source_t)(EXAMPLE_RX_DMA_SOURCE);

    /* Request DMA channels for TX & RX. */
    DMAMUX_SetSource(EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR, FLEXIO_SPI_TX_DMA_DSPI_CHANNEL, dma_request_source_tx);
    DMAMUX_SetSource(EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR, FLEXIO_SPI_RX_DMA_DSPI_CHANNEL, dma_request_source_rx);
    DMAMUX_EnableChannel(EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR, FLEXIO_SPI_TX_DMA_DSPI_CHANNEL);
    DMAMUX_EnableChannel(EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR, FLEXIO_SPI_RX_DMA_DSPI_CHANNEL);
    EDMA_CreateHandle(&txHandle, EXAMPLE_FLEXIO_SPI_DMA_DSPI_BASEADDR, FLEXIO_SPI_TX_DMA_DSPI_CHANNEL);
    EDMA_CreateHandle(&rxHandle, EXAMPLE_FLEXIO_SPI_DMA_DSPI_BASEADDR, FLEXIO_SPI_RX_DMA_DSPI_CHANNEL);

    /* Set up master transfer */
    FLEXIO_SPI_MasterTransferCreateHandleEDMA(&spiDev, &g_m_handle, FLEXIO_SPI_MasterUserCallback, NULL, &txHandle,
                                              &rxHandle);

    /*Start master transfer*/
    masterXfer.txData   = masterTxData;
    masterXfer.rxData   = masterRxData;
    masterXfer.dataSize = TRANSFER_SIZE;
    masterXfer.flags    = kFLEXIO_SPI_8bitMsb;

    FLEXIO_SPI_MasterTransferEDMA(&spiDev, &g_m_handle, &masterXfer);

    /* Wait slave received all data. */
    while (!isTransferCompleted)
    {
    }

    errorCount = 0U;
    for (i = 0U; i < TRANSFER_SIZE; i++)
    {
        if (masterTxData[i] != slaveRxData[i])
        {
            errorCount++;
        }

        if (slaveTxData[i] != masterRxData[i])
        {
            errorCount++;
        }
    }
    if (errorCount == 0U)
    {
        PRINTF("FLEXIO SPI master <-> DSPI slave transfer all data matched!\r\n");
    }
    else
    {
        PRINTF("Error occurred in FLEXIO SPI master <-> DSPI slave transfer!\r\n");
    }

    FLEXIO_SPI_MasterDeinit(&spiDev);
    DSPI_Deinit(BOARD_DSPI_SLAVE_BASE);

    while (1)
    {
    }
}
