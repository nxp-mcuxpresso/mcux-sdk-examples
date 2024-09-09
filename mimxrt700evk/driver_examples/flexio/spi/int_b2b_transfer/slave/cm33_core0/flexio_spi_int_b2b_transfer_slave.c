/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_flexio_spi.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"

#include "fsl_clock.h"
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BOARD_FLEXIO_BASE   (FLEXIO)
#define FLEXIO_SPI_MOSI_PIN 1U
#define FLEXIO_SPI_MISO_PIN 14U
#define FLEXIO_SPI_SCK_PIN  15U
#define FLEXIO_SPI_CSn_PIN  0U
#define BUFFER_SIZE (64)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static flexio_spi_master_handle_t g_spiHandle;
FLEXIO_SPI_Type spiDev;
static uint8_t sendBuff[BUFFER_SIZE];
static uint8_t recvBuff[BUFFER_SIZE];
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    uint8_t i                  = 0;
    uint8_t err                = 0;
    flexio_spi_transfer_t xfer = {0};
    flexio_spi_slave_config_t userConfig;
    size_t transferredCount = 0;

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();

    BOARD_InitDebugConsole();

    /* The domain own FLEXIO, disable mask in SHARED_MASK0. */
    POWER_DisableLPRequestMask(kPower_MaskFlexio);

    CLOCK_AttachClk(kMAIN_PLL_PFD3_to_FLEXIO);
    CLOCK_SetClkDiv(kCLOCK_DivFlexioClk, 5U);
    RESET_ClearPeripheralReset(kFLEXIO0_RST_SHIFT_RSTn);
    PRINTF("\r\nFlexIO SPI interrupt example\r\n");
    PRINTF("Slave is working...\r\n");

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
    spiDev.shifterIndex[0] = 0U;
    spiDev.shifterIndex[1] = 1U;
    spiDev.timerIndex[0]   = 0U;
    FLEXIO_SPI_SlaveInit(&spiDev, &userConfig);

    /* Init Buffer */
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        sendBuff[i] = i;
    }

    /* Receive data from master */
    xfer.txData   = sendBuff;
    xfer.rxData   = recvBuff;
    xfer.dataSize = BUFFER_SIZE;
    xfer.flags    = kFLEXIO_SPI_8bitMsb;
    FLEXIO_SPI_SlaveTransferCreateHandle(&spiDev, &g_spiHandle, NULL, NULL);
    FLEXIO_SPI_SlaveTransferNonBlocking(&spiDev, &g_spiHandle, &xfer);

    while (transferredCount != BUFFER_SIZE)
    {
        FLEXIO_SPI_SlaveTransferGetCount(&spiDev, &g_spiHandle, &transferredCount);
    }

    /* Check if the data is right */
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
