/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_lpspi.h"
#include "fsl_flexio_spi_edma.h"
#include "pin_mux.h"
#include "board.h"
#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
#include "fsl_dmamux.h"
#endif

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*Master related*/
#define TRANSFER_SIZE     256U    /*! Transfer dataSize */
#define TRANSFER_BAUDRATE 200000U /*! Transfer baudrate - 200k */

#define MASTER_FLEXIO_SPI_BASEADDR (FLEXIO1)
#define FLEXIO_SPI_SOUT_PIN        21U
#define FLEXIO_SPI_SIN_PIN         22U
#define FLEXIO_SPI_CLK_PIN         26U
#define FLEXIO_SPI_PCS_PIN         0U

#define MASTER_FLEXIO_SPI_IRQ FLEXIO1_IRQn

/* Select USB1 PLL (480 MHz) as flexio clock source */
#define MASTER_FLEXIO_SPI_CLOCK_SELECT (3U)
/* Clock pre divider for flexio clock source */
#define MASTER_FLEXIO_SPI_CLOCK_PRE_DIVIDER (4U)
/* Clock divider for flexio clock source */
#define MASTER_FLEXIO_SPI_CLOCK_DIVIDER (7U)
#define MASTER_FLEXIO_SPI_CLOCK_FREQUENCY                                            \
    (CLOCK_GetFreq(kCLOCK_Usb1PllClk) / (MASTER_FLEXIO_SPI_CLOCK_PRE_DIVIDER + 1U) / \
     (MASTER_FLEXIO_SPI_CLOCK_DIVIDER + 1U))

#define EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR    DMAMUX
#define EXAMPLE_FLEXIO_SPI_DMA_LPSPI_BASEADDR DMA0
#define FLEXIO_SPI_TX_DMA_LPSPI_CHANNEL       (0U)
#define FLEXIO_SPI_RX_DMA_LPSPI_CHANNEL       (1U)
#define FLEXIO_TX_SHIFTER_INDEX               0U
#define FLEXIO_RX_SHIFTER_INDEX               2U
#define EXAMPLE_TX_DMA_SOURCE                 (kDmaRequestMuxFlexIO1Request0Request1)
#define EXAMPLE_RX_DMA_SOURCE                 (kDmaRequestMuxFlexIO1Request2Request3)

/*Slave related*/
#define SLAVE_LPSPI_BASEADDR   (LPSPI1)
#define SLAVE_LPSPI_IRQ_HANDLE (LPSPI1_DriverIRQHandler)
#define SLAVE_LPSPI_IRQN       (LPSPI1_IRQn)

/* Select USB1 PLL PFD0 (720 MHz) as lpspi clock source */
#define SLAVE_LPSPI_CLOCK_SELECT (1U)
/* Clock divider for lpspi clock source */
#define SLAVE_LPSPI_CLOCK_DIVIDER (7U)

#define SLAVE_LPSPI_PCS_FOR_INIT     (kLPSPI_Pcs0)
#define SLAVE_LPSPI_PCS_FOR_TRANSFER (kLPSPI_SlavePcs0)


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* LPSPI user callback */
void FLEXIO_SPI_MasterUserCallback(FLEXIO_SPI_Type *base,
                                   flexio_spi_master_edma_handle_t *handle,
                                   status_t status,
                                   void *userData);
void LPSPI_SlaveUserCallback(LPSPI_Type *base, lpspi_slave_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/
AT_NONCACHEABLE_SECTION_INIT(uint8_t masterRxData[TRANSFER_SIZE]) = {0U};
AT_NONCACHEABLE_SECTION_INIT(uint8_t masterTxData[TRANSFER_SIZE]) = {0U};
AT_NONCACHEABLE_SECTION_INIT(uint8_t slaveRxData[TRANSFER_SIZE])  = {0U};
AT_NONCACHEABLE_SECTION_INIT(uint8_t slaveTxData[TRANSFER_SIZE])  = {0U};

FLEXIO_SPI_Type spiDev;
flexio_spi_master_edma_handle_t g_m_handle;
lpspi_slave_handle_t g_s_handle;

edma_handle_t txHandle;
edma_handle_t rxHandle;

volatile bool isSlaveTransferCompleted  = false;
volatile bool isMasterTransferCompleted = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

void FLEXIO_SPI_MasterUserCallback(FLEXIO_SPI_Type *base,
                                   flexio_spi_master_edma_handle_t *handle,
                                   status_t status,
                                   void *userData)
{
    isMasterTransferCompleted = true;
}

void LPSPI_SlaveUserCallback(LPSPI_Type *base, lpspi_slave_handle_t *handle, status_t status, void *userData)
{
    if (status == kStatus_Success)
    {
        __NOP();
    }

    if (status == kStatus_LPSPI_Error)
    {
        __NOP();
    }

    isSlaveTransferCompleted = true;

    PRINTF("This is LPSPI slave call back.\r\n");
}

int main(void)
{
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Clock setting for Flexio */
    CLOCK_SetMux(kCLOCK_Flexio1Mux, MASTER_FLEXIO_SPI_CLOCK_SELECT);
    CLOCK_SetDiv(kCLOCK_Flexio1PreDiv, MASTER_FLEXIO_SPI_CLOCK_PRE_DIVIDER);
    CLOCK_SetDiv(kCLOCK_Flexio1Div, MASTER_FLEXIO_SPI_CLOCK_DIVIDER);

    /* Clock setting for Lpspi */
    CLOCK_SetMux(kCLOCK_LpspiMux, SLAVE_LPSPI_CLOCK_SELECT);
    CLOCK_SetDiv(kCLOCK_LpspiDiv, SLAVE_LPSPI_CLOCK_DIVIDER);

    PRINTF("FLEXIO Master edma - LPSPI Slave interrupt example start.\r\n");
    PRINTF("This example use one flexio spi as master and one lpspi instance as slave on one board.\r\n");
    PRINTF("Master uses edma and slave uses interrupt way.\r\n");
    PRINTF("Please make sure you make the correct line connection. Basically, the connection is:\r\n");
    PRINTF("FLEXIO_SPI_master -- LPSPI_slave\r\n");
    PRINTF("       CLK        --    CLK\r\n");
    PRINTF("       PCS        --    PCS\r\n");
    PRINTF("       SOUT       --    SIN\r\n");
    PRINTF("       SIN        --    SOUT\r\n");

    uint32_t errorCount;
    uint32_t i;
    flexio_spi_master_config_t masterConfig;
    lpspi_slave_config_t slaveConfig;
    flexio_spi_transfer_t masterXfer;
    lpspi_transfer_t slaveXfer;

    edma_config_t config;

    /* Master config */
    FLEXIO_SPI_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Bps = TRANSFER_BAUDRATE;

    spiDev.flexioBase      = MASTER_FLEXIO_SPI_BASEADDR;
    spiDev.SDOPinIndex     = FLEXIO_SPI_SOUT_PIN;
    spiDev.SDIPinIndex     = FLEXIO_SPI_SIN_PIN;
    spiDev.SCKPinIndex     = FLEXIO_SPI_CLK_PIN;
    spiDev.CSnPinIndex     = FLEXIO_SPI_PCS_PIN;
    spiDev.shifterIndex[0] = FLEXIO_TX_SHIFTER_INDEX;
    spiDev.shifterIndex[1] = FLEXIO_RX_SHIFTER_INDEX;
    spiDev.timerIndex[0]   = 0U;
    spiDev.timerIndex[1]   = 1U;

    FLEXIO_SPI_MasterInit(&spiDev, &masterConfig, MASTER_FLEXIO_SPI_CLOCK_FREQUENCY);

    /*Slave config*/
    slaveConfig.bitsPerFrame = 8;
    slaveConfig.cpol         = kLPSPI_ClockPolarityActiveHigh;
    slaveConfig.cpha         = kLPSPI_ClockPhaseFirstEdge;
    slaveConfig.direction    = kLPSPI_MsbFirst;

    slaveConfig.whichPcs           = SLAVE_LPSPI_PCS_FOR_INIT;
    slaveConfig.pcsActiveHighOrLow = kLPSPI_PcsActiveLow;

    slaveConfig.pinCfg        = kLPSPI_SdiInSdoOut;
    slaveConfig.dataOutConfig = kLpspiDataOutRetained;

    LPSPI_SlaveInit(SLAVE_LPSPI_BASEADDR, &slaveConfig);

    /* Set up the transfer data */
    for (i = 0U; i < TRANSFER_SIZE; i++)
    {
        masterTxData[i] = i % 256U;
        masterRxData[i] = 0U;

        slaveTxData[i] = ~masterTxData[i];
        slaveRxData[i] = 0U;
    }

    isSlaveTransferCompleted = false;
    /* Set up slave first */
    LPSPI_SlaveTransferCreateHandle(SLAVE_LPSPI_BASEADDR, &g_s_handle, LPSPI_SlaveUserCallback, NULL);

    /*Set slave transfer ready to receive/send data*/
    slaveXfer.txData      = slaveTxData;
    slaveXfer.rxData      = slaveRxData;
    slaveXfer.dataSize    = TRANSFER_SIZE;
    slaveXfer.configFlags = SLAVE_LPSPI_PCS_FOR_TRANSFER | kLPSPI_SlaveByteSwap;

    LPSPI_SlaveTransferNonBlocking(SLAVE_LPSPI_BASEADDR, &g_s_handle, &slaveXfer);

#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
    /*Init DMAMUX */
    DMAMUX_Init(EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR);
    /* Set channel for FLEXIO */
    DMAMUX_SetSource(EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR, FLEXIO_SPI_TX_DMA_LPSPI_CHANNEL, EXAMPLE_TX_DMA_SOURCE);
    DMAMUX_SetSource(EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR, FLEXIO_SPI_RX_DMA_LPSPI_CHANNEL, EXAMPLE_RX_DMA_SOURCE);
    DMAMUX_EnableChannel(EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR, FLEXIO_SPI_TX_DMA_LPSPI_CHANNEL);
    DMAMUX_EnableChannel(EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR, FLEXIO_SPI_RX_DMA_LPSPI_CHANNEL);
#endif

    /* Init the EDMA module */
    EDMA_GetDefaultConfig(&config);
    EDMA_Init(EXAMPLE_FLEXIO_SPI_DMA_LPSPI_BASEADDR, &config);
    EDMA_CreateHandle(&txHandle, EXAMPLE_FLEXIO_SPI_DMA_LPSPI_BASEADDR, FLEXIO_SPI_TX_DMA_LPSPI_CHANNEL);
    EDMA_CreateHandle(&rxHandle, EXAMPLE_FLEXIO_SPI_DMA_LPSPI_BASEADDR, FLEXIO_SPI_RX_DMA_LPSPI_CHANNEL);

#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    EDMA_SetChannelMux(EXAMPLE_FLEXIO_SPI_DMA_LPSPI_BASEADDR, FLEXIO_SPI_TX_DMA_LPSPI_CHANNEL, EXAMPLE_TX_DMA_SOURCE);
    EDMA_SetChannelMux(EXAMPLE_FLEXIO_SPI_DMA_LPSPI_BASEADDR, FLEXIO_SPI_RX_DMA_LPSPI_CHANNEL, EXAMPLE_RX_DMA_SOURCE);
#endif

    /* Set up master transfer */
    FLEXIO_SPI_MasterTransferCreateHandleEDMA(&spiDev, &g_m_handle, FLEXIO_SPI_MasterUserCallback, NULL, &txHandle,
                                              &rxHandle);

    /*Start master transfer*/
    masterXfer.txData   = masterTxData;
    masterXfer.rxData   = masterRxData;
    masterXfer.dataSize = TRANSFER_SIZE;
    masterXfer.flags    = kFLEXIO_SPI_8bitMsb;

    isMasterTransferCompleted = false;
    FLEXIO_SPI_MasterTransferEDMA(&spiDev, &g_m_handle, &masterXfer);

    /* Wait slave received all data. */
    while (!(isSlaveTransferCompleted && isMasterTransferCompleted))
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
        PRINTF("FLEXIO SPI master <-> LPSPI slave transfer all data matched!\r\n");
    }
    else
    {
        PRINTF("Error occurred in FLEXIO SPI master <-> LPSPI slave transfer!\r\n");
    }

    FLEXIO_SPI_MasterDeinit(&spiDev);
    LPSPI_Deinit(SLAVE_LPSPI_BASEADDR);

    PRINTF("\r\nEnd of example.\r\n");

    while (1)
    {
    }
}
