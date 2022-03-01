/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_spi_cmsis.h"
#include "pin_mux.h"
#include "board.h"

#include "fsl_inputmux.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DRIVER_SLAVE_SPI Driver_SPI5

#define TRANSFER_SIZE 64U /* Transfer dataSize */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* SPI user SlaveSignalEvent */
void SPI_SlaveSignalEvent_t(uint32_t event);

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t slaveRxData[TRANSFER_SIZE] = {0};
uint8_t slaveTxData[TRANSFER_SIZE] = {0};

volatile bool isTransferCompleted = false;
/*******************************************************************************
 * Code
 ******************************************************************************/

uint32_t SPI5_GetFreq(void)
{
    return CLOCK_GetFlexcommClkFreq(5U);
}
void SPI_SlaveSignalEvent_t(uint32_t event)
{
    /* user code */
    isTransferCompleted = true;
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Configure DMAMUX. */
    RESET_PeripheralReset(kINPUTMUX_RST_SHIFT_RSTn);

    INPUTMUX_Init(INPUTMUX);
    CLOCK_AttachClk(kFRO_DIV4_to_FLEXCOMM5);
    INPUTMUX_EnableSignal(INPUTMUX, kINPUTMUX_Flexcomm5RxToDmac0Ch10RequestEna, true);
    INPUTMUX_EnableSignal(INPUTMUX, kINPUTMUX_Flexcomm5TxToDmac0Ch11RequestEna, true);
    /* Turnoff clock to inputmux to save power. Clock is only needed to make changes */
    INPUTMUX_Deinit(INPUTMUX);

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("CMSIS SPI board2board DMA example -- Slave transfer.\r\n\r\n");

    uint32_t i          = 0U;
    uint32_t errorCount = 0U;

    /*SPI slave init*/
    DRIVER_SLAVE_SPI.Initialize(SPI_SlaveSignalEvent_t);
    DRIVER_SLAVE_SPI.PowerControl(ARM_POWER_FULL);
    DRIVER_SLAVE_SPI.Control(ARM_SPI_MODE_SLAVE, false);

    PRINTF("\r\n Slave example is running...\r\n");

    /* Reset the send and receive buffer */
    for (i = 0U; i < TRANSFER_SIZE; i++)
    {
        slaveRxData[i] = 0U;
        slaveTxData[i] = i % 256;
    }

    isTransferCompleted = false;
    /* Set slave transfer to receive and send data */
    DRIVER_SLAVE_SPI.Transfer(slaveTxData, slaveRxData, TRANSFER_SIZE);
    /* Wait until transfer completed */
    while (!isTransferCompleted)
    {
    }

    for (i = 0; i < TRANSFER_SIZE; i++)
    {
        if (slaveRxData[i] != i)
        {
            PRINTF("\n\rThe %d number is wrong! It is %dn\r", i, slaveRxData[i]);
            errorCount++;
        }
    }

    PRINTF("\r\n");
    if (errorCount == 0)
    {
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
        PRINTF("Succeed!\n\r");
    }
    else
    {
        PRINTF("Error occurd in SPI transfer!\n\r");
    }
    while (1)
    {
    }
}
