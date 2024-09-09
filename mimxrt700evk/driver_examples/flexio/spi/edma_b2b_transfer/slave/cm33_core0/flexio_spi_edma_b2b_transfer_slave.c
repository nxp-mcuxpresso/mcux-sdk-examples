/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_flexio_spi_edma.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"

#include "fsl_clock.h"
#include "fsl_power.h"
#include "fsl_edma_soc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BOARD_FLEXIO_BASE      (FLEXIO)
#define FLEXIO_SPI_MOSI_PIN    1U
#define FLEXIO_SPI_MISO_PIN    14U
#define FLEXIO_SPI_SCK_PIN     15U
#define FLEXIO_SPI_CSn_PIN     0U
#define FLEXIO_CLOCK_FREQUENCY CLOCK_GetFlexioClkFreq()

#define EXAMPLE_FLEXIO_SPI_DMA_BASEADDR DMA0
#define FLEXIO_SPI_TX_DMA_CHANNEL       (0U)
#define FLEXIO_SPI_RX_DMA_CHANNEL       (1U)
#define FLEXIO_TX_SHIFTER_INDEX         0U
#define FLEXIO_RX_SHIFTER_INDEX         2U
#define EXAMPLE_TX_DMA_SOURCE           (kDmaRequestMuxFlexIO0ShiftRegister0Request)
#define EXAMPLE_RX_DMA_SOURCE           (kDmaRequestMuxFlexIO0ShiftRegister2Request)
#define BUFFER_SIZE (64)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static flexio_spi_slave_edma_handle_t g_spiHandle;
static edma_handle_t s_txHandle;
static edma_handle_t s_rxHandle;
FLEXIO_SPI_Type spiDev;
AT_NONCACHEABLE_SECTION_INIT(static uint8_t sendBuff[BUFFER_SIZE]) = {0U};
AT_NONCACHEABLE_SECTION_INIT(static uint8_t recvBuff[BUFFER_SIZE]) = {0U};
volatile bool completeFlag                                         = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

static void spi_slave_completionCallback(FLEXIO_SPI_Type *base,
                                         flexio_spi_slave_edma_handle_t *handle,
                                         status_t status,
                                         void *userData)
{
    if (status == kStatus_Success)
    {
        completeFlag = true;
    }
}

int main(void)
{
    uint8_t i                  = 0;
    uint8_t err                = 0;
    flexio_spi_transfer_t xfer = {0};
    flexio_spi_slave_config_t userConfig;
    dma_request_source_t dma_request_source_tx;
    dma_request_source_t dma_request_source_rx;
    edma_config_t config;

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();

    BOARD_InitDebugConsole();

    BOARD_InitAHBSC();

    POWER_DisableLPRequestMask(kPower_MaskFlexio);

    CLOCK_AttachClk(kMAIN_PLL_PFD3_to_FLEXIO);
    CLOCK_SetClkDiv(kCLOCK_DivFlexioClk, 5U);
    RESET_ClearPeripheralReset(kFLEXIO0_RST_SHIFT_RSTn);
    RESET_ClearPeripheralReset(kDMA0_RST_SHIFT_RSTn);

    EDMA_EnableRequest(EXAMPLE_FLEXIO_SPI_DMA_BASEADDR, EXAMPLE_TX_DMA_SOURCE);
    EDMA_EnableRequest(EXAMPLE_FLEXIO_SPI_DMA_BASEADDR, EXAMPLE_RX_DMA_SOURCE);
    PRINTF("\r\nFlexIO SPI edma example\r\n");
    PRINTF("Slave is working...\r\n");

    /* Init FlexIO SPI. */
    /*
     * userConfig.enableSlave = true;
     * userConfig.enableInDoze = false;
     * userConfig.enableInDebug = true;
     * userConfig.enableFastAccess = false;
     * userConfig.phase = kFLEXIO_SPI_ClockPhaseFirstEdge;
     * userConfig.dataMode = kFLEXIO_SPI_8BitMode;
     */
    FLEXIO_SPI_SlaveGetDefaultConfig(&userConfig);

    spiDev.flexioBase      = BOARD_FLEXIO_BASE;
    spiDev.SDOPinIndex     = FLEXIO_SPI_MISO_PIN;
    spiDev.SDIPinIndex     = FLEXIO_SPI_MOSI_PIN;
    spiDev.SCKPinIndex     = FLEXIO_SPI_SCK_PIN;
    spiDev.CSnPinIndex     = FLEXIO_SPI_CSn_PIN;
    spiDev.shifterIndex[0] = FLEXIO_TX_SHIFTER_INDEX;
    spiDev.shifterIndex[1] = FLEXIO_RX_SHIFTER_INDEX;
    spiDev.timerIndex[0]   = 0U;

    dma_request_source_tx = (dma_request_source_t)EXAMPLE_TX_DMA_SOURCE;
    dma_request_source_rx = (dma_request_source_t)EXAMPLE_RX_DMA_SOURCE;

#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
    /*Init EDMA for example.*/
    DMAMUX_Init(EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR);
    /* Request DMA channels for TX & RX. */
    DMAMUX_SetSource(EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR, FLEXIO_SPI_TX_DMA_CHANNEL, dma_request_source_tx);
    DMAMUX_SetSource(EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR, FLEXIO_SPI_RX_DMA_CHANNEL, dma_request_source_rx);
    DMAMUX_EnableChannel(EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR, FLEXIO_SPI_TX_DMA_CHANNEL);
    DMAMUX_EnableChannel(EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR, FLEXIO_SPI_RX_DMA_CHANNEL);
#endif
    EDMA_GetDefaultConfig(&config);
    EDMA_Init(EXAMPLE_FLEXIO_SPI_DMA_BASEADDR, &config);
    EDMA_CreateHandle(&s_txHandle, EXAMPLE_FLEXIO_SPI_DMA_BASEADDR, FLEXIO_SPI_TX_DMA_CHANNEL);
    EDMA_CreateHandle(&s_rxHandle, EXAMPLE_FLEXIO_SPI_DMA_BASEADDR, FLEXIO_SPI_RX_DMA_CHANNEL);

#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    EDMA_SetChannelMux(EXAMPLE_FLEXIO_SPI_DMA_BASEADDR, FLEXIO_SPI_TX_DMA_CHANNEL, dma_request_source_tx);
    EDMA_SetChannelMux(EXAMPLE_FLEXIO_SPI_DMA_BASEADDR, FLEXIO_SPI_RX_DMA_CHANNEL, dma_request_source_rx);
#endif

    FLEXIO_SPI_SlaveInit(&spiDev, &userConfig);

    /* Init Buffer. */
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        sendBuff[i] = i;
    }

    /* Receive data from master. */
    xfer.txData   = sendBuff;
    xfer.rxData   = recvBuff;
    xfer.dataSize = BUFFER_SIZE;
    xfer.flags    = kFLEXIO_SPI_8bitMsb;
    FLEXIO_SPI_SlaveTransferCreateHandleEDMA(&spiDev, &g_spiHandle, spi_slave_completionCallback, NULL, &s_txHandle,
                                             &s_rxHandle);
    FLEXIO_SPI_SlaveTransferEDMA(&spiDev, &g_spiHandle, &xfer);
    while (!completeFlag)
    {
    }
    completeFlag = false;

    /* Check if the data is right. */
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        if (sendBuff[i] != recvBuff[i])
        {
            PRINTF("The %d is wrong! data is %d\r\n", i, recvBuff[i]);
            err++;
        }
    }
    if (err == 0)
    {
        PRINTF("\r\nSlave runs successfully!\r\n");
    }

    while (1)
    {
    }
}
