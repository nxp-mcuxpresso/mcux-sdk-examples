/*
 * Copyright 2017, 2024 NXP
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

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Master related */
#define EXAMPLE_LPSPI_MASTER_BASEADDR (LPSPI1)

#define EXAMPLE_LPSPI_MASTER_PCS_FOR_INIT     (kLPSPI_Pcs0)
#define EXAMPLE_LPSPI_MASTER_PCS_FOR_TRANSFER (kLPSPI_MasterPcs0)

#define LPSPI_MASTER_CLK_FREQ CLOCK_GetLPFlexCommClkFreq(1u)
#define TRANSFER_SIZE     64U     /*! Transfer dataSize */
#define TRANSFER_BAUDRATE 500000U /*! Transfer baudrate - 500k */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* LPSPI user callback */
void LPSPI_MasterUserCallback(LPSPI_Type *base, lpspi_master_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint8_t masterRxData[TRANSFER_SIZE] = {0U};
uint8_t masterTxData[TRANSFER_SIZE] = {0U};

lpspi_master_handle_t g_m_handle;
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

void LPSPI_MasterUserCallback(LPSPI_Type *base, lpspi_master_handle_t *handle, status_t status, void *userData)
{
    if (status == kStatus_Success)
    {
        __NOP();
    }

    isTransferCompleted = true;
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach FRO 12M to FLEXCOMM1 */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom1Clk, 1u);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM1);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("LPSPI interrupt board to board (b2b) transfer master example.\r\n");
    PRINTF("This example use one board as master and another as slave.\r\n");
    PRINTF("Master and slave uses interrupt way. Slave should start first.\r\n");
    PRINTF("Please make sure you make the correct line connection. Basically, the connection is:\r\n");
    PRINTF("LPSPI_master -- LPSPI_slave\r\n");
    PRINTF("    CLK      --    CLK\r\n");
    PRINTF("    PCS      --    PCS\r\n");
    PRINTF("    SOUT     --    SIN\r\n");
    PRINTF("    SIN      --    SOUT\r\n");
    PRINTF("    GND      --    GND\r\n");

    uint32_t srcClock_Hz;
    uint32_t errorCount;
    uint32_t loopCount = 1;
    uint32_t i;
    lpspi_master_config_t masterConfig;
    lpspi_transfer_t masterXfer;

    /* Master config */
    LPSPI_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate = TRANSFER_BAUDRATE;
    masterConfig.whichPcs = EXAMPLE_LPSPI_MASTER_PCS_FOR_INIT;
    masterConfig.pcsToSckDelayInNanoSec        = 1000000000U / (masterConfig.baudRate * 2U);
    masterConfig.lastSckToPcsDelayInNanoSec    = 1000000000U / (masterConfig.baudRate * 2U);
    masterConfig.betweenTransferDelayInNanoSec = 1000000000U / (masterConfig.baudRate * 2U);
    
    srcClock_Hz = LPSPI_MASTER_CLK_FREQ;
    LPSPI_MasterInit(EXAMPLE_LPSPI_MASTER_BASEADDR, &masterConfig, srcClock_Hz);

    LPSPI_MasterTransferCreateHandle(EXAMPLE_LPSPI_MASTER_BASEADDR, &g_m_handle, LPSPI_MasterUserCallback, NULL);

    while (1)
    {
        /* Set up the transfer data */
        for (i = 0U; i < TRANSFER_SIZE; i++)
        {
            masterTxData[i] = (i + loopCount) % 256U;
            masterRxData[i] = 0U;
        }

        /* Print out transmit buffer */
        PRINTF("\r\n Master transmit:");
        for (i = 0U; i < TRANSFER_SIZE; i++)
        {
            /* Print 16 numbers in a line */
            if ((i & 0x0FU) == 0U)
            {
                PRINTF("\r\n");
            }
            PRINTF(" %02X", masterTxData[i]);
        }
        PRINTF("\r\n");

        /* Start master transfer, send data to slave */
        isTransferCompleted = false;
        masterXfer.txData   = masterTxData;
        masterXfer.rxData   = NULL;
        masterXfer.dataSize = TRANSFER_SIZE;
        masterXfer.configFlags =
            EXAMPLE_LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous | kLPSPI_MasterByteSwap;

        LPSPI_MasterTransferNonBlocking(EXAMPLE_LPSPI_MASTER_BASEADDR, &g_m_handle, &masterXfer);

        /* Wait transfer complete */
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
            EXAMPLE_LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous | kLPSPI_MasterByteSwap;

        LPSPI_MasterTransferNonBlocking(EXAMPLE_LPSPI_MASTER_BASEADDR, &g_m_handle, &masterXfer);

        while (!isTransferCompleted)
        {
        }

        errorCount = 0U;
        for (i = 0U; i < TRANSFER_SIZE; i++)
        {
            if (masterTxData[i] != masterRxData[i])
            {
                errorCount++;
            }
        }
        if (errorCount == 0U)
        {
            PRINTF(" \r\nLPSPI transfer all data matched!\r\n");
            /* Print out receive buffer */
            PRINTF("\r\n Master received:");
            for (i = 0U; i < TRANSFER_SIZE; i++)
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
            PRINTF(" \r\nError occurred in LPSPI transfer!\r\n");
        }

        /* Wait for press any key */
        PRINTF("\r\nPress any key to run again\r\n");
        GETCHAR();

        /* Increase loop count to change transmit buffer */
        loopCount++;
    }
}
