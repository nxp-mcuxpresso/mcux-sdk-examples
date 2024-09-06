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
#include "clock_config.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define TRANSFER_SIZE     256U    /*! Transfer dataSize */
#define TRANSFER_BAUDRATE 500000U /*! Transfer baudrate - 500k */
/* Slave related */
#define SLAVE_FLEXIO_SPI_BASEADDR (FLEXIO0)
#define FLEXIO_SPI_SIN_PIN        21U
#define FLEXIO_SPI_SOUT_PIN       7U
#define FLEXIO_SPI_CLK_PIN        8U
#define FLEXIO_SPI_PCS_PIN        20U
#define SLAVE_FLEXIO_SPI_IRQ      FLEXIO0_IRQn
/* Master related */
#define MASTER_LPSPI_BASEADDR         (LPSPI0)
#define MASTER_LPSPI_IRQN             (LPSPI0_IRQn)
#define MASTER_LPSPI_PCS_FOR_INIT     (kLPSPI_Pcs0)
#define MASTER_LPSPI_PCS_FOR_TRANSFER (kLPSPI_MasterPcs0)
#define MASTER_LPSPI_CLOCK_FREQUENCY  CLOCK_GetIpFreq(kCLOCK_Lpspi0)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* LPSPI user callback */
void LPSPI_MasterUserCallback(LPSPI_Type *base, lpspi_master_handle_t *handle, status_t status, void *userData);
void FLEXIO_SPI_SlaveUserCallback(FLEXIO_SPI_Type *base,
                                  flexio_spi_slave_handle_t *handle,
                                  status_t status,
                                  void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t masterRxData[TRANSFER_SIZE] = {0U};
uint8_t masterTxData[TRANSFER_SIZE] = {0U};
uint8_t slaveRxData[TRANSFER_SIZE]  = {0U};
uint8_t slaveTxData[TRANSFER_SIZE]  = {0U};

lpspi_master_handle_t g_m_handle;
FLEXIO_SPI_Type spiDev;
flexio_spi_slave_handle_t g_s_handle;

volatile bool isSlaveTransferCompleted  = false;
volatile bool isMasterTransferCompleted = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

void LPSPI_MasterUserCallback(LPSPI_Type *base, lpspi_master_handle_t *handle, status_t status, void *userData)
{
    if (status == kStatus_Success)
    {
        __NOP();
    }
    isMasterTransferCompleted = true;
}

void FLEXIO_SPI_SlaveUserCallback(FLEXIO_SPI_Type *base,
                                  flexio_spi_slave_handle_t *handle,
                                  status_t status,
                                  void *userData)
{
    if (status == kStatus_Success)
    {
        __NOP();
    }

    isSlaveTransferCompleted = true;

    PRINTF("This is FLEXIO SPI slave call back.\r\n");
}

int main(void)
{
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    /* Set MRCC FLEXIO0 selection */
    CLOCK_SetIpSrc(kCLOCK_Flexio0, kCLOCK_IpSrcFro192M);
    /* Set MRCC FLEXIO0 fraction divider */
    CLOCK_SetIpSrcDiv(kCLOCK_Flexio0, kSCG_SysClkDivBy16);
    /* Set MRCC LPSPI0 selection */
    CLOCK_SetIpSrc(kCLOCK_Lpspi0, kCLOCK_IpSrcFro192M);
    /* Set MRCC LPSPI0 fraction divider */
    CLOCK_SetIpSrcDiv(kCLOCK_Lpspi0, kSCG_SysClkDivBy16);

    PRINTF("LPSPI Master interrupt - FLEXIO SPI Slave interrupt example start.\r\n");
    PRINTF("This example use one lpspi instance as master and one flexio spi slave on one board.\r\n");
    PRINTF("Master and slave are both use interrupt way.\r\n");
    PRINTF("Please make sure you make the correct line connection. Basically, the connection is: \r\n");
    PRINTF("LPSPI_master -- FLEXIO_SPI_slave   \r\n");
    PRINTF("   CLK      --    CLK  \r\n");
    PRINTF("   PCS      --    PCS  \r\n");
    PRINTF("   SOUT     --    SIN  \r\n");
    PRINTF("   SIN      --    SOUT \r\n");

    uint32_t errorCount;
    uint32_t i;
    lpspi_master_config_t masterConfig;
    flexio_spi_slave_config_t slaveConfig;
    lpspi_transfer_t masterXfer;
    flexio_spi_transfer_t slaveXfer;

    /*Master config*/
    LPSPI_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate     = TRANSFER_BAUDRATE;
    masterConfig.bitsPerFrame = 8;
    masterConfig.cpol         = kLPSPI_ClockPolarityActiveHigh;
    masterConfig.cpha         = kLPSPI_ClockPhaseFirstEdge;
    masterConfig.direction    = kLPSPI_MsbFirst;

    masterConfig.pcsToSckDelayInNanoSec        = 1000000000 / masterConfig.baudRate;
    masterConfig.lastSckToPcsDelayInNanoSec    = 1000000000 / masterConfig.baudRate;
    masterConfig.betweenTransferDelayInNanoSec = 1000000000 / masterConfig.baudRate;

    masterConfig.whichPcs           = MASTER_LPSPI_PCS_FOR_INIT;
    masterConfig.pcsActiveHighOrLow = kLPSPI_PcsActiveLow;

    masterConfig.pinCfg           = kLPSPI_SdiInSdoOut;
    masterConfig.dataOutConfig    = kLpspiDataOutRetained;
    masterConfig.enableInputDelay = false;

    LPSPI_MasterInit(MASTER_LPSPI_BASEADDR, &masterConfig, MASTER_LPSPI_CLOCK_FREQUENCY);

    /* Slave config */
    FLEXIO_SPI_SlaveGetDefaultConfig(&slaveConfig);

    spiDev.flexioBase      = SLAVE_FLEXIO_SPI_BASEADDR;
    spiDev.SDOPinIndex     = FLEXIO_SPI_SOUT_PIN;
    spiDev.SDIPinIndex     = FLEXIO_SPI_SIN_PIN;
    spiDev.SCKPinIndex     = FLEXIO_SPI_CLK_PIN;
    spiDev.CSnPinIndex     = FLEXIO_SPI_PCS_PIN;
    spiDev.shifterIndex[0] = 0U;
    spiDev.shifterIndex[1] = 1U;
    spiDev.timerIndex[0]   = 0U;
    FLEXIO_SPI_SlaveInit(&spiDev, &slaveConfig);

    /* Set slave interrupt priority higher. */
#if defined(__CORTEX_M) && (__CORTEX_M == 0U) && defined(FSL_FEATURE_NUMBER_OF_LEVEL1_INT_VECTORS) && \
    (FSL_FEATURE_NUMBER_OF_LEVEL1_INT_VECTORS > 0)
    if (SLAVE_FLEXIO_SPI_IRQ < FSL_FEATURE_NUMBER_OF_LEVEL1_INT_VECTORS)
    {
        NVIC_SetPriority(SLAVE_FLEXIO_SPI_IRQ, 0U);
    }
    if (MASTER_LPSPI_IRQN < FSL_FEATURE_NUMBER_OF_LEVEL1_INT_VECTORS)
    {
        NVIC_SetPriority(MASTER_LPSPI_IRQN, 1U);
    }
#else
    NVIC_SetPriority(SLAVE_FLEXIO_SPI_IRQ, 0U);
    NVIC_SetPriority(MASTER_LPSPI_IRQN, 1U);
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
    FLEXIO_SPI_SlaveTransferCreateHandle(&spiDev, &g_s_handle, FLEXIO_SPI_SlaveUserCallback, NULL);

    /*Set slave transfer ready to receive/send data*/
    slaveXfer.txData   = slaveTxData;
    slaveXfer.rxData   = slaveRxData;
    slaveXfer.dataSize = TRANSFER_SIZE;
    slaveXfer.flags    = kFLEXIO_SPI_8bitMsb;

    FLEXIO_SPI_SlaveTransferNonBlocking(&spiDev, &g_s_handle, &slaveXfer);

    /* Set up master transfer */
    LPSPI_MasterTransferCreateHandle(MASTER_LPSPI_BASEADDR, &g_m_handle, LPSPI_MasterUserCallback, NULL);

    /*Start master transfer*/
    masterXfer.txData      = masterTxData;
    masterXfer.rxData      = masterRxData;
    masterXfer.dataSize    = TRANSFER_SIZE;
    masterXfer.configFlags = MASTER_LPSPI_PCS_FOR_TRANSFER;

    LPSPI_MasterTransferNonBlocking(MASTER_LPSPI_BASEADDR, &g_m_handle, &masterXfer);

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
        PRINTF("LPSPI master <-> FLEXIO SPI slave transfer all data matched!\r\n");
    }
    else
    {
        PRINTF("Error occurred in LPSPI master <-> FLEXIO SPI slave transfer!\r\n");
    }

    LPSPI_Deinit(MASTER_LPSPI_BASEADDR);
    FLEXIO_SPI_SlaveDeinit(&spiDev);

    PRINTF("\r\nEnd of Example. \r\n");

    while (1)
    {
    }
}
