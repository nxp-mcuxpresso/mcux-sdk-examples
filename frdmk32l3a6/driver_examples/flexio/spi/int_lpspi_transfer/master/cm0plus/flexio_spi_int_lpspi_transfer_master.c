/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_lpspi.h"
#include "fsl_flexio_spi.h"
#include "pin_mux.h"
#include "board.h"

#include "fsl_common.h"
#include "fsl_intmux.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*Master related*/
#define MASTER_FLEXIO_SPI_BASEADDR (FLEXIO0)
#define FLEXIO_SPI_SOUT_PIN        9U
#define FLEXIO_SPI_SIN_PIN         3U
#define FLEXIO_SPI_CLK_PIN         5U
#define FLEXIO_SPI_PCS_PIN         10U

#define MASTER_FLEXIO_SPI_IRQ             FLEXIO0_IRQn
#define MASTER_FLEXIO_SPI_CLOCK_NAME      (kCLOCK_Flexio0)
#define MASTER_FLEXIO_SPI_CLOCK_SOURCE    (kCLOCK_IpSrcFircAsync)
#define MASTER_FLEXIO_SPI_CLOCK_FREQUENCY (CLOCK_GetIpFreq(MASTER_FLEXIO_SPI_CLOCK_NAME))

/*Slave related*/
#define SLAVE_LPSPI_BASEADDR   (LPSPI0)
#define SLAVE_LPSPI_IRQ_HANDLE (LPSPI0_DriverIRQHandler)
#define SLAVE_LPSPI_IRQN       (LPSPI0_IRQn)

#define SLAVE_LPSPI_CLOCK_NAME   (kCLOCK_Lpspi0)
#define SLAVE_LPSPI_CLOCK_SOURCE (kCLOCK_IpSrcFircAsync)

#define SLAVE_LPSPI_PCS_FOR_INIT     (kLPSPI_Pcs0)
#define SLAVE_LPSPI_PCS_FOR_TRANSFER (kLPSPI_SlavePcs0)

/*INTMUX setting*/
#define EXAMPLE_LPSPI_INTMUX_CHANNLE  (0U)
#define EXAMPLE_FLEXIO_INTMUX_CHANNEL (1U)

#define TRANSFER_SIZE     256U    /*! Transfer dataSize */
#define TRANSFER_BAUDRATE 400000U /*! Transfer baudrate - 400k */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* LPSPI user callback */
void FLEXIO_SPI_MasterUserCallback(FLEXIO_SPI_Type *base,
                                   flexio_spi_master_handle_t *handle,
                                   status_t status,
                                   void *userData);
void LPSPI_SlaveUserCallback(LPSPI_Type *base, lpspi_slave_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t masterRxData[TRANSFER_SIZE] = {0U};
uint8_t masterTxData[TRANSFER_SIZE] = {0U};
uint8_t slaveRxData[TRANSFER_SIZE]  = {0U};
uint8_t slaveTxData[TRANSFER_SIZE]  = {0U};

FLEXIO_SPI_Type spiDev;
flexio_spi_master_handle_t g_m_handle;
lpspi_slave_handle_t g_s_handle;

volatile bool isSlaveTransferCompleted  = false;
volatile bool isMasterTransferCompleted = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

void FLEXIO_SPI_MasterUserCallback(FLEXIO_SPI_Type *base,
                                   flexio_spi_master_handle_t *handle,
                                   status_t status,
                                   void *userData)
{
    if (status == kStatus_Success)
    {
        __NOP();
    }
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
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /*Set clock source for LPSPI and FlexIO*/
    CLOCK_SetIpSrc(MASTER_FLEXIO_SPI_CLOCK_NAME, MASTER_FLEXIO_SPI_CLOCK_SOURCE);
    CLOCK_SetIpSrc(SLAVE_LPSPI_CLOCK_NAME, SLAVE_LPSPI_CLOCK_SOURCE);

    INTMUX_Init(INTMUX1);
    INTMUX_SetChannelMode(INTMUX1, EXAMPLE_LPSPI_INTMUX_CHANNLE, kINTMUX_ChannelLogicOR);
    INTMUX_EnableInterrupt(INTMUX1, EXAMPLE_LPSPI_INTMUX_CHANNLE, SLAVE_LPSPI_IRQN);
    INTMUX_SetChannelMode(INTMUX1, EXAMPLE_FLEXIO_INTMUX_CHANNEL, kINTMUX_ChannelLogicOR);
    INTMUX_EnableInterrupt(INTMUX1, EXAMPLE_FLEXIO_INTMUX_CHANNEL, MASTER_FLEXIO_SPI_IRQ);
    NVIC_SetPriority(INTMUX1_0_IRQn, 0U);
    NVIC_SetPriority(INTMUX1_1_IRQn, 1U);

    PRINTF("FLEXIO Master - LPSPI Slave interrupt example start.\r\n");
    PRINTF("This example use one flexio spi as master and one lpspi instance as slave on one board.\r\n");
    PRINTF("Master and slave are both use interrupt way.\r\n");
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

    /* Master config */
    FLEXIO_SPI_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Bps = TRANSFER_BAUDRATE;

    spiDev.flexioBase      = MASTER_FLEXIO_SPI_BASEADDR;
    spiDev.SDOPinIndex     = FLEXIO_SPI_SOUT_PIN;
    spiDev.SDIPinIndex     = FLEXIO_SPI_SIN_PIN;
    spiDev.SCKPinIndex     = FLEXIO_SPI_CLK_PIN;
    spiDev.CSnPinIndex     = FLEXIO_SPI_PCS_PIN;
    spiDev.shifterIndex[0] = 0U;
    spiDev.shifterIndex[1] = 1U;
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

    /* Set lpspi slave interrupt priority higher. */
#if defined(__CORTEX_M) && (__CORTEX_M == 0U) && defined(FSL_FEATURE_NUMBER_OF_LEVEL1_INT_VECTORS) && \
    (FSL_FEATURE_NUMBER_OF_LEVEL1_INT_VECTORS > 0)
    if (SLAVE_LPSPI_IRQN < FSL_FEATURE_NUMBER_OF_LEVEL1_INT_VECTORS)
    {
        NVIC_SetPriority(SLAVE_LPSPI_IRQN, 0U);
    }
    if (MASTER_FLEXIO_SPI_IRQ < FSL_FEATURE_NUMBER_OF_LEVEL1_INT_VECTORS)
    {
        NVIC_SetPriority(MASTER_FLEXIO_SPI_IRQ, 1U);
    }
#else
    NVIC_SetPriority(SLAVE_LPSPI_IRQN, 0U);
    NVIC_SetPriority(MASTER_FLEXIO_SPI_IRQ, 1U);
#endif

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

    /* Set up master transfer */
    FLEXIO_SPI_MasterTransferCreateHandle(&spiDev, &g_m_handle, FLEXIO_SPI_MasterUserCallback, NULL);

    /*Start master transfer*/
    masterXfer.txData   = masterTxData;
    masterXfer.rxData   = masterRxData;
    masterXfer.dataSize = TRANSFER_SIZE;
    masterXfer.flags    = kFLEXIO_SPI_8bitMsb;

    isMasterTransferCompleted = false;
    FLEXIO_SPI_MasterTransferNonBlocking(&spiDev, &g_m_handle, &masterXfer);

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
