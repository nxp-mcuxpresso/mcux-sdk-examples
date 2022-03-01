/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_ecspi_cmsis.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DRIVER_MASTER_SPI       Driver_SPI2
#define EXAMPLE_MASTER_SPI_BASE ECSPI2
#define TRANSFER_SIZE     256U    /*! Transfer dataSize */
#define TRANSFER_BAUDRATE 500000U /*! Transfer baudrate - 500k */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void ECSPI_EnableLoopBackTransfer(ECSPI_Type *base);
/* ECSPI user SignalEvent */
void ECSPI_MasterSignalEvent_t(uint32_t event);
/*******************************************************************************
 * Variables
 ******************************************************************************/
uint32_t masterRxData[TRANSFER_SIZE] = {0U};
uint32_t masterTxData[TRANSFER_SIZE] = {0U};

volatile bool isTransferCompleted = false;
/*******************************************************************************
 * Code
 ******************************************************************************/

uint32_t ECSPI2_GetFreq(void)
{
    return (CLOCK_GetPllFreq(kCLOCK_SystemPll1Ctrl) / (CLOCK_GetRootPreDivider(kCLOCK_RootEcspi2)) /
            (CLOCK_GetRootPostDivider(kCLOCK_RootEcspi2)));
}

void ECSPI_EnableLoopBackTransfer(ECSPI_Type *base)
{
    base->TESTREG |= ECSPI_TESTREG_LBC(1);
}
void ECSPI_MasterSignalEvent_t(uint32_t event)
{
    /* user code */
    isTransferCompleted = true;
    PRINTF("\r\n  This is ECSPI_MasterSignalEvent_t.\r\n");
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t errorCount;
    uint32_t i;

    /* M7 has its local cache and enabled by default,
     * need to set smart subsystems (0x28000000 ~ 0x3FFFFFFF)
     * non-cacheable before accessing this address region */
    BOARD_InitMemory();

    /* Board specific RDC settings */
    BOARD_RdcInit();

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetRootMux(kCLOCK_RootEcspi2, kCLOCK_EcspiRootmuxSysPll1); /* Set ECSPI2 source to SYSTEM PLL1 800MHZ */
    CLOCK_SetRootDivider(kCLOCK_RootEcspi2, 2U, 5U);                 /* Set root clock to 800MHZ / 10 = 80MHZ */

    PRINTF("This is ECSPI CMSIS interrupt loopback transfer example.\r\n");
    PRINTF("The ECSPI will connect the transmitter and receiver sections internally.\r\n");

    /*DSPI master init*/
    DRIVER_MASTER_SPI.Initialize(ECSPI_MasterSignalEvent_t);
    DRIVER_MASTER_SPI.PowerControl(ARM_POWER_FULL);
    DRIVER_MASTER_SPI.Control(ARM_SPI_MODE_MASTER, TRANSFER_BAUDRATE);

    /* Enable loopback transfer. */
    ECSPI_EnableLoopBackTransfer(EXAMPLE_MASTER_SPI_BASE);
    /* Set up the transfer data */
    for (i = 0U; i < TRANSFER_SIZE; i++)
    {
        masterTxData[i] = i % 256U;
        masterRxData[i] = 0U;
    }

    isTransferCompleted = false;
    PRINTF("Start transfer...\r\n");
    /* Start master transfer */
    DRIVER_MASTER_SPI.Transfer(masterTxData, masterRxData, TRANSFER_SIZE);

    /* Wait slave received all data. */
    while (!isTransferCompleted)
    {
    }

    PRINTF("\r\nTransfer completed!");
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
        PRINTF("\r\nECSPI transfer all data matched! \r\n");
    }
    else
    {
        PRINTF(" \r\nError occurred in ECSPI loopback transfer ! \r\n");
    }

    DRIVER_MASTER_SPI.PowerControl(ARM_POWER_OFF);
    DRIVER_MASTER_SPI.Uninitialize();

    while (1)
    {
    }
}
