/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_lpspi.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_lpspi_edma.h"
#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
#include "fsl_dmamux.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_LPSPI_MASTER_BASEADDR         LPSPI0
#define EXAMPLE_LPSPI_MASTER_CLOCK_NAME       kCLOCK_Lpspi0
#define LPSPI_MASTER_CLK_FREQ                 (CLOCK_GetIpFreq(EXAMPLE_LPSPI_MASTER_CLOCK_NAME))
#define EXAMPLE_LPSPI_MASTER_CLOCK_SOURCE     (kCLOCK_IpSrcFircAsync)
#define EXAMPLE_LPSPI_MASTER_PCS_FOR_INIT     kLPSPI_Pcs0
#define EXAMPLE_LPSPI_MASTER_PCS_FOR_TRANSFER kLPSPI_MasterPcs0

#define EXAMPLE_LPSPI_MASTER_DMA_MUX_BASE          DMAMUX0
#define EXAMPLE_LPSPI_MASTER_DMA_RX_REQUEST_SOURCE kDmaRequestMux0LPSPI0Rx
#define EXAMPLE_LPSPI_MASTER_DMA_TX_REQUEST_SOURCE kDmaRequestMux0LPSPI0Tx
#define EXAMPLE_LPSPI_MASTER_DMA_BASE              DMA0
#define EXAMPLE_LPSPI_MASTER_DMA_RX_CHANNEL        0U
#define EXAMPLE_LPSPI_MASTER_DMA_TX_CHANNEL        1U

#define EXAMPLE_LPSPI_DEALY_COUNT 0xfffff
#define TRANSFER_SIZE     64U     /* Transfer dataSize */
#define TRANSFER_BAUDRATE 500000U /* Transfer baudrate - 500k */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* LPSPI user callback */
void LPSPI_MasterUserCallback(LPSPI_Type *base, lpspi_master_edma_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/
AT_NONCACHEABLE_SECTION_INIT(uint8_t masterRxData[TRANSFER_SIZE]) = {0};
AT_NONCACHEABLE_SECTION_INIT(uint8_t masterTxData[TRANSFER_SIZE]) = {0};

AT_NONCACHEABLE_SECTION_INIT(lpspi_master_edma_handle_t g_m_edma_handle) = {0};
edma_handle_t lpspiEdmaMasterRxRegToRxDataHandle;
edma_handle_t lpspiEdmaMasterTxDataToTxRegHandle;

volatile bool isTransferCompleted  = false;
volatile uint32_t g_systickCounter = 20U;
/*******************************************************************************
 * Code
 ******************************************************************************/
void SysTick_Handler(void)
{
    if (g_systickCounter != 0U)
    {
        g_systickCounter--;
    }
}

void LPSPI_MasterUserCallback(LPSPI_Type *base, lpspi_master_edma_handle_t *handle, status_t status, void *userData)
{
    if (status == kStatus_Success)
    {
        PRINTF("This is LPSPI master edma transfer completed callback.\r\n\r\n");
    }

    isTransferCompleted = true;
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t srcClock_Hz;
    uint32_t errorCount;
    uint32_t loopCount = 1U;
    uint32_t i;
    lpspi_master_config_t masterConfig;
    lpspi_transfer_t masterXfer;
    edma_config_t userConfig;

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /*Set clock source for LPSPI and get master clock source*/
    CLOCK_SetIpSrc(EXAMPLE_LPSPI_MASTER_CLOCK_NAME, EXAMPLE_LPSPI_MASTER_CLOCK_SOURCE);

    PRINTF("LPSPI board to board edma example.\r\n");
    PRINTF("This example use one board as master and another as slave.\r\n");
    PRINTF("Master and slave uses EDMA way. Slave should start first.\r\n");
    PRINTF("Please make sure you make the correct line connection. Basically, the connection is:\r\n");
    PRINTF("LPSPI_master --  LPSPI_slave\r\n");
    PRINTF("   CLK       --    CLK\r\n");
    PRINTF("   PCS       --    PCS\r\n");
    PRINTF("   SOUT      --    SIN\r\n");
    PRINTF("   SIN       --    SOUT\r\n");
    PRINTF("   GND       --    GND\r\n");

/*DMA Mux setting and EDMA init*/
#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
    /* DMA MUX init*/
    DMAMUX_Init(EXAMPLE_LPSPI_MASTER_DMA_MUX_BASE);

    DMAMUX_SetSource(EXAMPLE_LPSPI_MASTER_DMA_MUX_BASE, EXAMPLE_LPSPI_MASTER_DMA_RX_CHANNEL,
                     EXAMPLE_LPSPI_MASTER_DMA_RX_REQUEST_SOURCE);
    DMAMUX_EnableChannel(EXAMPLE_LPSPI_MASTER_DMA_MUX_BASE, EXAMPLE_LPSPI_MASTER_DMA_RX_CHANNEL);

    DMAMUX_SetSource(EXAMPLE_LPSPI_MASTER_DMA_MUX_BASE, EXAMPLE_LPSPI_MASTER_DMA_TX_CHANNEL,
                     EXAMPLE_LPSPI_MASTER_DMA_TX_REQUEST_SOURCE);
    DMAMUX_EnableChannel(EXAMPLE_LPSPI_MASTER_DMA_MUX_BASE, EXAMPLE_LPSPI_MASTER_DMA_TX_CHANNEL);
#endif
    /* EDMA init*/
    /*
     * userConfig.enableRoundRobinArbitration = false;
     * userConfig.enableHaltOnError = true;
     * userConfig.enableContinuousLinkMode = false;
     * userConfig.enableDebugMode = false;
     */
    EDMA_GetDefaultConfig(&userConfig);
    EDMA_Init(EXAMPLE_LPSPI_MASTER_DMA_BASE, &userConfig);

    /*Master config*/
    LPSPI_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate = TRANSFER_BAUDRATE;
    masterConfig.whichPcs = EXAMPLE_LPSPI_MASTER_PCS_FOR_INIT;

    srcClock_Hz = LPSPI_MASTER_CLK_FREQ;
    LPSPI_MasterInit(EXAMPLE_LPSPI_MASTER_BASEADDR, &masterConfig, srcClock_Hz);

    /*Set up lpspi master*/
    memset(&(lpspiEdmaMasterRxRegToRxDataHandle), 0, sizeof(lpspiEdmaMasterRxRegToRxDataHandle));
    memset(&(lpspiEdmaMasterTxDataToTxRegHandle), 0, sizeof(lpspiEdmaMasterTxDataToTxRegHandle));

    EDMA_CreateHandle(&(lpspiEdmaMasterRxRegToRxDataHandle), EXAMPLE_LPSPI_MASTER_DMA_BASE,
                      EXAMPLE_LPSPI_MASTER_DMA_RX_CHANNEL);
    EDMA_CreateHandle(&(lpspiEdmaMasterTxDataToTxRegHandle), EXAMPLE_LPSPI_MASTER_DMA_BASE,
                      EXAMPLE_LPSPI_MASTER_DMA_TX_CHANNEL);
#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    EDMA_SetChannelMux(EXAMPLE_LPSPI_MASTER_DMA_BASE, EXAMPLE_LPSPI_MASTER_DMA_TX_CHANNEL,
                       DEMO_LPSPI_TRANSMIT_EDMA_CHANNEL);
    EDMA_SetChannelMux(EXAMPLE_LPSPI_MASTER_DMA_BASE, EXAMPLE_LPSPI_MASTER_DMA_RX_CHANNEL,
                       DEMO_LPSPI_RECEIVE_EDMA_CHANNEL);
#endif
    LPSPI_MasterTransferCreateHandleEDMA(EXAMPLE_LPSPI_MASTER_BASEADDR, &g_m_edma_handle, LPSPI_MasterUserCallback,
                                         NULL, &lpspiEdmaMasterRxRegToRxDataHandle,
                                         &lpspiEdmaMasterTxDataToTxRegHandle);

    while (1)
    {
        /* Set up the transfer data */
        for (i = 0U; i < TRANSFER_SIZE; i++)
        {
            masterTxData[i] = (i + loopCount) % 256U;
            masterRxData[i] = 0U;
        }

        /* Print out transmit buffer */
        PRINTF("\r\n Master transmit:\r\n");
        for (i = 0; i < TRANSFER_SIZE; i++)
        {
            /* Print 16 numbers in a line */
            if ((i & 0x0FU) == 0U)
            {
                PRINTF("\r\n");
            }
            PRINTF(" %02X", masterTxData[i]);
        }
        PRINTF("\r\n");

        /*Start master transfer*/
        masterXfer.txData   = masterTxData;
        masterXfer.rxData   = NULL;
        masterXfer.dataSize = TRANSFER_SIZE;
        masterXfer.configFlags =
            EXAMPLE_LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterByteSwap | kLPSPI_MasterPcsContinuous;

        isTransferCompleted = false;
        LPSPI_MasterTransferEDMA(EXAMPLE_LPSPI_MASTER_BASEADDR, &g_m_edma_handle, &masterXfer);

        /* Wait until transfer completed */
        while (!isTransferCompleted)
        {
        }

        /* Delay to wait slave is ready */
        if (SysTick_Config(SystemCoreClock / 1000U))
        {
            while (1)
            {
            }
        }
        /* Delay 20 ms */
        g_systickCounter = 20U;
        while (g_systickCounter != 0U)
        {
        }
        /* Start master transfer, receive data from slave */
        isTransferCompleted = false;
        masterXfer.txData   = NULL;
        masterXfer.rxData   = masterRxData;
        masterXfer.dataSize = TRANSFER_SIZE;
        masterXfer.configFlags =
            EXAMPLE_LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterByteSwap | kLPSPI_MasterPcsContinuous;

        LPSPI_MasterTransferEDMA(EXAMPLE_LPSPI_MASTER_BASEADDR, &g_m_edma_handle, &masterXfer);

        /* Wait until transfer completed */
        while (!isTransferCompleted)
        {
        }

        errorCount = 0;
        for (i = 0; i < TRANSFER_SIZE; i++)
        {
            if (masterTxData[i] != masterRxData[i])
            {
                errorCount++;
            }
        }
        if (errorCount == 0)
        {
            PRINTF("\r\nLPSPI transfer all data matched!\r\n");
            /* Print out receive buffer */
            PRINTF("\r\n Master received:\r\n");
            for (i = 0; i < TRANSFER_SIZE; i++)
            {
                /* Print 16 numbers in a line */
                if ((i & 0x0FU) == 0U)
                {
                    PRINTF("\r\n");
                }
                PRINTF(" %02X", masterRxData[i]);
            }
            PRINTF("\r\n");
        }
        else
        {
            PRINTF("\r\nError occurred in LPSPI transfer !\r\n");
        }

        /* Wait for press any key */
        PRINTF("\r\n Press any key to run again\r\n");
        GETCHAR();
        /* Increase loop count to change transmit buffer */
        loopCount++;
    }
}
