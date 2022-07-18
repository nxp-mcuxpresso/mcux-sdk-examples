/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_lpspi_cmsis.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_LPSPI_SLAVE_IRQN         (LPSPI0_IRQn)
#define EXAMPLE_LPSPI_SLAVE_CLOCK_NAME   (kCLOCK_Lpspi0)
#define EXAMPLE_LPSPI_SLAVE_CLOCK_SOURCE (kCLOCK_IpSrcFircAsync)

#define EXAMPLE_LPSPI_SLAVE_DMA_BASEADDR     DMA0
#define EXAMPLE_LPSPI_SLAVE_DMA_MUX_BASEADDR DMAMUX

#define DRIVER_SLAVE_SPI Driver_SPI0
#define TRANSFER_SIZE 64U /* Transfer dataSize */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* LPSPI user SlaveSignalEvent */
void LPSPI_SlaveSignalEvent_t(uint32_t event);

/*******************************************************************************
 * Variables
 ******************************************************************************/
AT_NONCACHEABLE_SECTION_INIT(uint8_t slaveRxData[TRANSFER_SIZE]) = {0};

volatile bool isTransferCompleted = false;
volatile bool isSlaveOnTransmit   = false;
volatile bool isSlaveOnReceive    = false;
/*******************************************************************************
 * Code
 ******************************************************************************/

uint32_t LPSPI0_GetFreq(void)
{
    return CLOCK_GetIpFreq(kCLOCK_Lpspi0);
}
void LPSPI_SlaveSignalEvent_t(uint32_t event)
{
    /* user code */
    if (true == isSlaveOnReceive)
    {
        PRINTF("This is LPSPI_SlaveSignalEvent_t\r\n");
        PRINTF("Slave receive data from master has completed!\r\n");
        isSlaveOnReceive = false;
    }
    if (true == isSlaveOnTransmit)
    {
        PRINTF("This is LPSPI_SlaveSignalEvent_t\r\n");
        PRINTF("Slave transmit data to master has completed!\r\n");
        isSlaveOnTransmit = false;
    }
    isTransferCompleted = true;
}

/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /*Set clock source for LPSPI and get master clock source*/
    CLOCK_SetIpSrc(EXAMPLE_LPSPI_SLAVE_CLOCK_NAME, EXAMPLE_LPSPI_SLAVE_CLOCK_SOURCE);

    PRINTF("LPSPI CMSIS driver board to board edma example.\r\n");

    uint32_t i;

    /* DMA Mux init and EDMA init */
    edma_config_t edmaConfig = {0};
    EDMA_GetDefaultConfig(&edmaConfig);
    EDMA_Init(EXAMPLE_LPSPI_SLAVE_DMA_BASEADDR, &edmaConfig);
#if (defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX)
    EDMA_SetChannelMux(RTE_SPI1_DMA_TX_DMA_BASE, RTE_SPI1_DMA_TX_CH, RTE_SPI1_DMA_TX_PERI_SEL);
    EDMA_SetChannelMux(RTE_SPI1_DMA_RX_DMA_BASE, RTE_SPI1_DMA_RX_CH, RTE_SPI1_DMA_RX_PERI_SEL);
#endif
#if (defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT)
    DMAMUX_Init(EXAMPLE_LPSPI_SLAVE_DMA_MUX_BASEADDR);
#endif

    /*LPSPI slave init*/
    DRIVER_SLAVE_SPI.Initialize(LPSPI_SlaveSignalEvent_t);
    DRIVER_SLAVE_SPI.PowerControl(ARM_POWER_FULL);
    DRIVER_SLAVE_SPI.Control(ARM_SPI_MODE_SLAVE, false);

    while (1)
    {
        PRINTF("\r\n Slave example is running...\r\n");

        /* Reset the receive buffer */
        for (i = 0U; i < TRANSFER_SIZE; i++)
        {
            slaveRxData[i] = 0U;
        }

        isTransferCompleted = false;
        isSlaveOnReceive    = true;
        /* Set slave transfer to receive data */
        DRIVER_SLAVE_SPI.Receive(slaveRxData, TRANSFER_SIZE);

        while (!isTransferCompleted)
        {
        }

        isTransferCompleted = false;
        isSlaveOnTransmit   = true;
        /* Set slave transfer to send back data */
        DRIVER_SLAVE_SPI.Send(slaveRxData, TRANSFER_SIZE);

        while (!isTransferCompleted)
        {
        }

        /* Print out receive buffer */
        PRINTF("\r\n Slave receive:");
        for (i = 0; i < TRANSFER_SIZE; i++)
        {
            /* Print 16 numbers in a line */
            if ((i & 0x0FU) == 0U)
            {
                PRINTF("\r\n    ");
            }
            PRINTF(" %02X", slaveRxData[i]);
        }
        PRINTF("\r\n");
    }
}
