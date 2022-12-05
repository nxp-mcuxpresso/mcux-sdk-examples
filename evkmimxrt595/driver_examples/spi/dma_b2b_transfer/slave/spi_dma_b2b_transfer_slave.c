/*
 * Copyright  2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_device_registers.h"
#include "fsl_spi.h"
#include "fsl_spi_dma.h"
#include "fsl_dma.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_inputmux.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef USE_HS_SPI
#define USE_HS_SPI 0
#endif

#if USE_HS_SPI
#define EXAMPLE_SPI_SLAVE            SPI14
#define EXAMPLE_SPI_SLAVE_IRQ        FLEXCOMM14_IRQn
#define EXAMPLE_SPI_SLAVE_RX_CHANNEL 26
#define EXAMPLE_SPI_SLAVE_TX_CHANNEL 27
#else
#define EXAMPLE_SPI_SLAVE            SPI5
#define EXAMPLE_SPI_SLAVE_IRQ        FLEXCOMM5_IRQn
#define EXAMPLE_SPI_SLAVE_RX_CHANNEL 10
#define EXAMPLE_SPI_SLAVE_TX_CHANNEL 11
#endif

#define EXAMPLE_SPI_SSEL       0
#define EXAMPLE_DMA            DMA0
#define EXAMPLE_SLAVE_SPI_SPOL kSPI_SpolActiveAllLow
#define TRANSFER_SIZE 64U /*! Transfer dataSize */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void SPI_SlaveUserCallback(SPI_Type *base, spi_dma_handle_t *handle, status_t status, void *userData);
static void EXAMPLE_SlaveInit(void);
static void EXAMPLE_SlaveDMASetup(void);
static void EXAMPLE_SlaveStartDMATransfer(void);
static void EXAMPLE_TransferDataCheck(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t slaveRxData[TRANSFER_SIZE] = {0};
uint8_t slaveTxData[TRANSFER_SIZE] = {0};

dma_handle_t slaveTxHandle;
dma_handle_t slaveRxHandle;

spi_dma_handle_t slaveHandle;

volatile bool isTransferCompleted = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void SPI_SlaveUserCallback(SPI_Type *base, spi_dma_handle_t *handle, status_t status, void *userData)
{
    if (status == kStatus_Success)
    {
        isTransferCompleted = true;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Initialzie board setting. */
    /* Configure DMAMUX. */
    RESET_PeripheralReset(kINPUTMUX_RST_SHIFT_RSTn);

    INPUTMUX_Init(INPUTMUX);
    /* Enable DMA request */
#if USE_HS_SPI
    /* Use 48 MHz clock for the FLEXCOMM14 */
    CLOCK_AttachClk(kFRO_DIV4_to_FLEXCOMM14);
    INPUTMUX_EnableSignal(INPUTMUX, kINPUTMUX_Flexcomm14RxToDmac0Ch26RequestEna, true);
    INPUTMUX_EnableSignal(INPUTMUX, kINPUTMUX_Flexcomm14TxToDmac0Ch27RequestEna, true);
#else
    /* Use 48 MHz clock for the FLEXCOMM5 */
    CLOCK_AttachClk(kFRO_DIV4_to_FLEXCOMM5);
    INPUTMUX_EnableSignal(INPUTMUX, kINPUTMUX_Flexcomm5RxToDmac0Ch10RequestEna, true);
    INPUTMUX_EnableSignal(INPUTMUX, kINPUTMUX_Flexcomm5TxToDmac0Ch11RequestEna, true);
#endif
    /* Turnoff clock to inputmux to save power. Clock is only needed to make changes */
    INPUTMUX_Deinit(INPUTMUX);

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Print project information. */
    PRINTF("This is SPI DMA transfer slave example.\r\n");
    PRINTF("This example will communicate with another master SPI on the other board.\r\n");
    PRINTF("Slave board is working...!\r\n");

    /* Initialize the SPI slave instance. */
    EXAMPLE_SlaveInit();

    /* Configure DMA for slave SPI. */
    EXAMPLE_SlaveDMASetup();

    /* Start SPI DMA transfer. */
    EXAMPLE_SlaveStartDMATransfer();

    /* Waiting for transmission complete and check if all data matched. */
    EXAMPLE_TransferDataCheck();

    /* De-intialzie the DMA instance. */
    DMA_Deinit(EXAMPLE_DMA);

    /* De-intialize the SPI instance. */
    SPI_Deinit(EXAMPLE_SPI_SLAVE);

    while (1)
    {
    }
}

static void EXAMPLE_SlaveInit(void)
{
    spi_slave_config_t slaveConfig;

    /* Get default Slave configuration. */
    SPI_SlaveGetDefaultConfig(&slaveConfig);

    /* Initialize the SPI slave. */
    slaveConfig.sselPol = (spi_spol_t)EXAMPLE_SLAVE_SPI_SPOL;
    SPI_SlaveInit(EXAMPLE_SPI_SLAVE, &slaveConfig);
}

static void EXAMPLE_SlaveDMASetup(void)
{
    /* DMA init */
    DMA_Init(EXAMPLE_DMA);

    /* configure channel/priority and create handle for TX and RX. */
    DMA_EnableChannel(EXAMPLE_DMA, EXAMPLE_SPI_SLAVE_TX_CHANNEL);
    DMA_EnableChannel(EXAMPLE_DMA, EXAMPLE_SPI_SLAVE_RX_CHANNEL);
    DMA_SetChannelPriority(EXAMPLE_DMA, EXAMPLE_SPI_SLAVE_TX_CHANNEL, kDMA_ChannelPriority0);
    DMA_SetChannelPriority(EXAMPLE_DMA, EXAMPLE_SPI_SLAVE_RX_CHANNEL, kDMA_ChannelPriority1);
    DMA_CreateHandle(&slaveTxHandle, EXAMPLE_DMA, EXAMPLE_SPI_SLAVE_TX_CHANNEL);
    DMA_CreateHandle(&slaveRxHandle, EXAMPLE_DMA, EXAMPLE_SPI_SLAVE_RX_CHANNEL);
}

static void EXAMPLE_SlaveStartDMATransfer(void)
{
    uint32_t i = 0U;
    spi_transfer_t slaveXfer;

    /* Initialzie the transfer data */
    for (i = 0U; i < TRANSFER_SIZE; i++)
    {
        slaveTxData[i] = i % 256U;
        slaveRxData[i] = 0U;
    }

    /* Create handle for slave instance. */
    SPI_SlaveTransferCreateHandleDMA(EXAMPLE_SPI_SLAVE, &slaveHandle, SPI_SlaveUserCallback, NULL, &slaveTxHandle,
                                     &slaveRxHandle);

    slaveXfer.txData   = (uint8_t *)&slaveTxData;
    slaveXfer.rxData   = (uint8_t *)&slaveRxData;
    slaveXfer.dataSize = TRANSFER_SIZE * sizeof(slaveTxData[0]);

    /* Start transfer, when transmission complete, the SPI_SlaveUserCallback will be called. */
    if (kStatus_Success != SPI_SlaveTransferDMA(EXAMPLE_SPI_SLAVE, &slaveHandle, &slaveXfer))
    {
        PRINTF("There is an error when start SPI_SlaveTransferDMA\r\n");
    }
}

static void EXAMPLE_TransferDataCheck(void)
{
    uint32_t i = 0U, errorCount = 0U;

    /* Wait until transfer completed */
    while (!isTransferCompleted)
    {
    }

    PRINTF("\r\nThe received data are:");
    /*Check if the data is right*/
    for (i = 0; i < TRANSFER_SIZE; i++)
    {
        /* Print 16 numbers in a line */
        if ((i & 0x0FU) == 0U)
        {
            PRINTF("\r\n  ");
        }
        PRINTF("  0x%02X", slaveRxData[i]);
        /* Check if data matched. */
        if (slaveTxData[i] != slaveRxData[i])
        {
            errorCount++;
        }
    }
    if (errorCount == 0)
    {
        PRINTF("\r\nSPI transfer all data matched!\r\n");
    }
    else
    {
        PRINTF("\r\nError occurred in SPI transfer !\r\n");
    }
}
