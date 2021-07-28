/*
 * Copyright  2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_dma.h"
#include "fsl_spi.h"
#include "pin_mux.h"
#include "board.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_SPI_MASTER          SPI0
#define EXAMPLE_CLK_SRC             kCLOCK_MainClk
#define EXAMPLE_SPI_MASTER_CLK_FREQ CLOCK_GetFreq(EXAMPLE_CLK_SRC)
#define EXAMPLE_SPI_MASTER_BAUDRATE 500000U
#define EXAMPLE_SPI_MASTER_SSEL     kSPI_Ssel0Assert

#define EXAMPLE_SPI_MASTER_DMA_BASEADDR DMA0
#define EXAMPLE_SPI_MASTER_TX_CHANNEL   11
#define EXAMPLE_SPI_MASTER_RX_CHANNEL   10

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void SPI_DmaTxCallback(dma_handle_t *handle, void *param, bool transferDone, uint32_t tcds);
static void SPI_DmaRxCallback(dma_handle_t *handle, void *param, bool transferDone, uint32_t tcds);
static void EXAMPLE_SPIMasterInit(void);
static void EXAMPLE_MasterDMASetup(void);
static void EXAMPLE_MasterStartDMATransfer(void);
static void EXAMPLE_TransferDataCheck(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
#define BUFFER_SIZE (64)
static uint8_t txBuffer[BUFFER_SIZE];
static uint8_t rxBuffer[BUFFER_SIZE];

dma_handle_t masterTxHandle;
dma_handle_t masterRxHandle;

static volatile bool masterTxFinished = false;
static volatile bool masterRxFinished = false;

/*! @brief Static table of descriptors */
SDK_ALIGN(dma_descriptor_t txDescriptor, 16) = {0};

/*! @brief The last data to be sent. */
SDK_ALIGN(uint32_t lastData, 4) = 0U;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void SPI_DmaTxCallback(dma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    masterTxFinished = true;
}

static void SPI_DmaRxCallback(dma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    masterRxFinished = true;
}

int main(void)
{
    /* Initialize the boards */
    /* Attach main clock to USART0 (debug console) */
    CLOCK_Select(kUART0_Clk_From_MainClk);

    BOARD_InitBootPins();
    BOARD_BootClockFRO30M();
    BOARD_InitDebugConsole();

    /* Attach main clock to SPI0. */
    CLOCK_Select(kSPI0_Clk_From_MainClk);

    /* Turn on LED RED */
    LED_RED_INIT(LOGIC_LED_ON);

    PRINTF("This is SPI dma transfer master example!\n\r");
    PRINTF("To make sure the transfer work successfully, please start the slave board first!\n\r");

    /* Initialize the SPI master with configuration. */
    EXAMPLE_SPIMasterInit();

    /* Set up DMA configuration. */
    EXAMPLE_MasterDMASetup();

    /* Start transfer with slave board. */
    EXAMPLE_MasterStartDMATransfer();

    /* Check the received data. */
    EXAMPLE_TransferDataCheck();

    /* De-initialize the DMA. */
    DMA_Deinit(EXAMPLE_SPI_MASTER_DMA_BASEADDR);

    /* De-initialize the SPI. */
    SPI_Deinit(EXAMPLE_SPI_MASTER);

    while (1)
    {
    }
}

static void EXAMPLE_SPIMasterInit(void)
{
    uint32_t srcFreq = 0U;
    spi_master_config_t masterConfig;
    /* configuration from using SPI_MasterGetDefaultConfig():
     * userConfig->enableLoopback = false;
     * userConfig->enableMaster = true;
     * userConfig->polarity = kSPI_ClockPolarityActiveHigh;
     * userConfig->phase = kSPI_ClockPhaseFirstEdge;
     * userConfig->direction = kSPI_MsbFirst;
     * userConfig->baudRate_Bps = 500000U;
     * userConfig->dataWidth = kSPI_Data8Bits;
     * userConfig->sselNum = kSPI_Ssel0Assert;
     * userConfig->sselPol = kSPI_SpolActiveAllLow;
     */
    SPI_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Bps = EXAMPLE_SPI_MASTER_BAUDRATE;
    masterConfig.sselNumber   = EXAMPLE_SPI_MASTER_SSEL;
    srcFreq                   = EXAMPLE_SPI_MASTER_CLK_FREQ;
    SPI_MasterInit(EXAMPLE_SPI_MASTER, &masterConfig, srcFreq);
}

static void EXAMPLE_MasterDMASetup(void)
{
    /* DMA init */
    DMA_Init(EXAMPLE_SPI_MASTER_DMA_BASEADDR);

    /* Enable channel and Create handle for RX channel. */
    DMA_EnableChannel(EXAMPLE_SPI_MASTER_DMA_BASEADDR, EXAMPLE_SPI_MASTER_RX_CHANNEL);
    DMA_CreateHandle(&masterRxHandle, EXAMPLE_SPI_MASTER_DMA_BASEADDR, EXAMPLE_SPI_MASTER_RX_CHANNEL);
    DMA_SetCallback(&masterRxHandle, SPI_DmaRxCallback, NULL);

    /* Enable channel and Create handle for TX channel. */
    DMA_EnableChannel(EXAMPLE_SPI_MASTER_DMA_BASEADDR, EXAMPLE_SPI_MASTER_TX_CHANNEL);
    DMA_CreateHandle(&masterTxHandle, EXAMPLE_SPI_MASTER_DMA_BASEADDR, EXAMPLE_SPI_MASTER_TX_CHANNEL);
    DMA_SetCallback(&masterTxHandle, SPI_DmaTxCallback, NULL);

    /* Set the channel priority. */
    DMA_SetChannelPriority(EXAMPLE_SPI_MASTER_DMA_BASEADDR, EXAMPLE_SPI_MASTER_TX_CHANNEL, kDMA_ChannelPriority3);
    DMA_SetChannelPriority(EXAMPLE_SPI_MASTER_DMA_BASEADDR, EXAMPLE_SPI_MASTER_RX_CHANNEL, kDMA_ChannelPriority2);
}

static void EXAMPLE_MasterStartDMATransfer(void)
{
    uint32_t i = 0U;
    dma_transfer_config_t masterTxDmaConfig, masterRxDmaConfig;

    /* Prepare buffer to send and receive data. */
    for (i = 0U; i < BUFFER_SIZE; i++)
    {
        txBuffer[i] = i;
        rxBuffer[i] = 0U;
    }

    /* Prepare and start DMA RX transfer. */
    DMA_PrepareTransfer(&masterRxDmaConfig, (void *)&EXAMPLE_SPI_MASTER->RXDAT, rxBuffer, sizeof(uint8_t), BUFFER_SIZE,
                        kDMA_PeripheralToMemory, NULL);
    DMA_SubmitTransfer(&masterRxHandle, &masterRxDmaConfig);

    /* Start DMA TX transfer. */
    DMA_StartTransfer(&masterRxHandle);

    /* Set the last byte to be sent, This will de-assert the SSEL pin when transmission is completed.
     * If users want to assert the SSEL pin when transmission is completed, there is no need to set up this descriptor.
     */
    lastData = txBuffer[BUFFER_SIZE - 1] | kSPI_EndOfTransfer | (EXAMPLE_SPI_MASTER->TXCTL & 0xFFFF0000);

    /* DMA transfer configuration setting. */
    dma_xfercfg_t tmp_xfercfg = {0};
    tmp_xfercfg.valid         = true;
    tmp_xfercfg.swtrig        = true;
    tmp_xfercfg.intA          = true;
    tmp_xfercfg.byteWidth     = sizeof(uint32_t);
    tmp_xfercfg.srcInc        = 0;
    tmp_xfercfg.dstInc        = 0;
    tmp_xfercfg.transferCount = 1;

    /* Create chained descriptor to transmit last word */
    DMA_CreateDescriptor(&txDescriptor, &tmp_xfercfg, &lastData, (void *)&EXAMPLE_SPI_MASTER->TXDATCTL, NULL);

    /* Add confifuration parameter to descriptor. */
    DMA_PrepareTransfer(&masterTxDmaConfig, txBuffer, (void *)&EXAMPLE_SPI_MASTER->TXDAT, sizeof(uint8_t),
                        BUFFER_SIZE - 1, kDMA_MemoryToPeripheral, &txDescriptor);

    /* Disable interrupts for first descriptor to avoid calling callback twice. */
    masterTxDmaConfig.xfercfg.intA = false;
    masterTxDmaConfig.xfercfg.intB = false;

    DMA_SubmitTransfer(&masterTxHandle, &masterTxDmaConfig);

    /* Start DMA TX transfer. */
    DMA_StartTransfer(&masterTxHandle);
}

static void EXAMPLE_TransferDataCheck(void)
{
    uint32_t i = 0U, err = 0U;

    /* Waiting for the transfer complete. */
    while (!(masterTxFinished | masterRxFinished))
    {
    }

    PRINTF("\n\rThe received data are:");
    /*Check if the data is right*/
    err = 0U;
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        /* Print 16 numbers in a line */
        if ((i & 0x0FU) == 0U)
        {
            PRINTF("\n\r");
        }
        PRINTF("  0x%02X", rxBuffer[i]);
        /* Check if data matched. */
        if (txBuffer[i] != rxBuffer[i])
        {
            err++;
        }
    }

    if (err == 0)
    {
        PRINTF("\n\rMaster DMA transfer succeed!\n\r");
    }
    else
    {
        PRINTF("\n\rMaster DMA transfer faild!\n\r");
    }
}
