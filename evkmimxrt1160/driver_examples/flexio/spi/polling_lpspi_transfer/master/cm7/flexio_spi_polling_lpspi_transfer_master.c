/*
 * Copyright 2022 NXP
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
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*Master related*/
#define TRANSFER_SIZE     256U    /*! Transfer dataSize */
#define TRANSFER_BAUDRATE 500000U /*! Transfer baudrate - 500k */

#define MASTER_FLEXIO_SPI_BASEADDR (FLEXIO2)
#define FLEXIO_SPI_SOUT_PIN        12U
#define FLEXIO_SPI_SIN_PIN         11U
#define FLEXIO_SPI_CLK_PIN         10U
#define FLEXIO_SPI_PCS_PIN         13U

#define MASTER_FLEXIO_SPI_CLOCK_FREQUENCY (CLOCK_GetFreqFromObs(CCM_OBS_FLEXIO2_CLK_ROOT))

/*Slave related*/
#define SLAVE_LPSPI_BASEADDR (LPSPI1)
#define SLAVE_LPSPI_IRQN     (LPSPI1_IRQn)

#define SLAVE_LPSPI_PCS_FOR_INIT     (kLPSPI_Pcs0)
#define SLAVE_LPSPI_PCS_FOR_TRANSFER (kLPSPI_SlavePcs0)


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* LPSPI user callback */
void LPSPI_SlaveUserCallback(LPSPI_Type *base, lpspi_slave_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t masterRxData[TRANSFER_SIZE] = {0U};
uint8_t masterTxData[TRANSFER_SIZE] = {0U};
uint8_t slaveRxData[TRANSFER_SIZE]  = {0U};
uint8_t slaveTxData[TRANSFER_SIZE]  = {0U};

FLEXIO_SPI_Type spiDev;
lpspi_slave_handle_t g_s_handle;

volatile bool isSlaveTransferCompleted = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
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
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("FLEXIO Master - LPSPI Slave polling example start.\r\n");
    PRINTF("This example use one flexio spi as master and one lpspi instance as slave on one board.\r\n");
    PRINTF("Master uses polling and slave uses interrupt way.\r\n");
    PRINTF("Please make sure you make the correct line connection. Basically, the connection is:\r\n");
    PRINTF("FLEXIO_SPI_master -- LPSPI_slave   \r\n");
    PRINTF("       CLK        --    CLK  \r\n");
    PRINTF("       PCS        --    PCS  \r\n");
    PRINTF("       SOUT       --    SIN  \r\n");
    PRINTF("       SIN        --    SOUT \r\n");

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
    slaveConfig.bitsPerFrame = 32;
    slaveConfig.cpol         = kLPSPI_ClockPolarityActiveHigh;
    slaveConfig.cpha         = kLPSPI_ClockPhaseFirstEdge;
    slaveConfig.direction    = kLPSPI_LsbFirst;

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
    slaveXfer.configFlags = SLAVE_LPSPI_PCS_FOR_TRANSFER;

    LPSPI_SlaveTransferNonBlocking(SLAVE_LPSPI_BASEADDR, &g_s_handle, &slaveXfer);

    /*Start master transfer*/
    masterXfer.txData   = masterTxData;
    masterXfer.rxData   = masterRxData;
    masterXfer.dataSize = TRANSFER_SIZE;
    masterXfer.flags    = kFLEXIO_SPI_32bitLsb;

    FLEXIO_SPI_MasterTransferBlocking(&spiDev, &masterXfer);

    /* Wait slave received all data. */
    while (!isSlaveTransferCompleted)
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
