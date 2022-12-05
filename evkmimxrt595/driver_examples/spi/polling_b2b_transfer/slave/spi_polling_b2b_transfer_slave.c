/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_spi.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef USE_HS_SPI
#define USE_HS_SPI 0
#endif

#if USE_HS_SPI
#define EXAMPLE_SPI_SLAVE SPI14
#else
#define EXAMPLE_SPI_SLAVE SPI5
#endif

#define EXAMPLE_SPI_SSEL 0
#define EXAMPLE_SPI_SPOL kSPI_SpolActiveAllLow

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
#define TRANSFER_SIZE (64)
static uint8_t receiveBuff[TRANSFER_SIZE];
static uint8_t sendBuff[TRANSFER_SIZE];
spi_slave_handle_t handle;
static volatile bool slaveFinished = false;
/*******************************************************************************
 * Code
 ******************************************************************************/
static void slaveCallback(SPI_Type *base, spi_slave_handle_t *slaveHandle, status_t status, void *userData)
{
    slaveFinished = true;
}

int main(void)
{
    uint32_t i          = 0;
    uint8_t err         = 0;
    spi_transfer_t xfer = {0};
    spi_slave_config_t userConfig;

#if USE_HS_SPI
    /* Use 48 MHz clock for the FLEXCOMM14 */
    CLOCK_AttachClk(kFRO_DIV4_to_FLEXCOMM14);
#else
    /* Use 48 MHz clock for the FLEXCOMM5 */
    CLOCK_AttachClk(kFRO_DIV4_to_FLEXCOMM5);
#endif

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    PRINTF("\r\nSlave is working....\r\n");

    /*
     * userConfig.enableSlave = true;
     * userConfig.polarity = kSPI_ClockPolarityActiveHigh;
     * userConfig.phase = kSPI_ClockPhaseFirstEdge;
     * userConfig.direction = kSPI_MsbFirst;
     */
    SPI_SlaveGetDefaultConfig(&userConfig);
    userConfig.sselPol = (spi_spol_t)EXAMPLE_SPI_SPOL;
    SPI_SlaveInit(EXAMPLE_SPI_SLAVE, &userConfig);
    SPI_SlaveTransferCreateHandle(EXAMPLE_SPI_SLAVE, &handle, slaveCallback, NULL);
    for (i = 0; i < 64; i++)
    {
        sendBuff[i] = i;
    }

    /* receive data from master */
    xfer.txData   = sendBuff;
    xfer.rxData   = receiveBuff;
    xfer.dataSize = sizeof(sendBuff);
    SPI_SlaveTransferNonBlocking(EXAMPLE_SPI_SLAVE, &handle, &xfer);

    while (slaveFinished != true)
    {
    }

    for (i = 0; i < TRANSFER_SIZE; i++)
    {
        if (receiveBuff[i] != sendBuff[i])
        {
            PRINTF("\r\nThe %d number is wrong! It is %d\r\n", i, receiveBuff[i]);
            err++;
        }
    }
    PRINTF("\r\n");
    if (err == 0)
    {
        PRINTF("Succeed!\r\n");
    }

    while (1)
    {
    }
}
