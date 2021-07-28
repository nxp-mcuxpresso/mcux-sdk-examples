/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_spi.h"
#include "fsl_spi_freertos.h"
#include "pin_mux.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_SPI_MASTER      SPI0
#define EXAMPLE_SPI_MASTER_BASE (SPI0_BASE)
#define EXAMPLE_SPI_MASTER_IRQN (SPI0_IRQn)
#define SPI_MASTER_CLK_SRC      (SPI0_CLK_SRC)
#define SPI_MASTER_CLK_FREQ     CLOCK_GetFreq((SPI0_CLK_SRC))
#ifndef EXAMPLE_SPI_MASTER_BASE
#error Undefined SPI base address!
#endif // ifndef EXAMPLE_SPI_MASTER_BASE

#define EXAMPLE_SPI_MASTER_BASEADDR ((SPI_Type *)EXAMPLE_SPI_MASTER_BASE)

#define SPI_NVIC_PRIO 2

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
#define BUFFER_SIZE (64)
static uint8_t srcBuff[BUFFER_SIZE];
static uint8_t destBuff[BUFFER_SIZE];
spi_rtos_handle_t master_rtos_handle;
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Task priorities. */
#define master_task_PRIORITY (configMAX_PRIORITIES - 1)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void master_task(void *pvParameters);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Application entry point.
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("SPI FreeRTOS example start.\r\n");
    PRINTF("This example use one SPI instance in master mode\r\n");
    PRINTF("to transfer data through loopback.\r\n");
    PRINTF("Please be sure to externally connect together SOUT and SIN signals.\r\n");
    PRINTF("   SOUT     --    SIN  \r\n");

    if (xTaskCreate(master_task, "Master_task", configMINIMAL_STACK_SIZE + 100, NULL, master_task_PRIORITY, NULL) !=
        pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
    vTaskStartScheduler();
    for (;;)
        ;
}

/*!
 * @brief Task responsible for master SPI communication.
 */
static void master_task(void *pvParameters)
{
    spi_master_config_t masterConfig;
    spi_transfer_t masterXfer = {0};
    uint32_t sourceClock;
    status_t status;

    NVIC_SetPriority(EXAMPLE_SPI_MASTER_IRQN, SPI_NVIC_PRIO);

    /*
     * masterConfig.enableStopInWaitMode = false;
     * masterConfig.polarity = kSPI_ClockPolarityActiveHigh;
     * masterConfig.phase = kSPI_ClockPhaseFirstEdge;
     * masterConfig.direction = kSPI_MsbFirst;
     * masterConfig.dataMode = kSPI_8BitMode;
     * masterConfig.txWatermark = kSPI_TxFifoOneHalfEmpty;
     * masterConfig.rxWatermark = kSPI_RxFifoOneHalfFull;
     * masterConfig.pinMode = kSPI_PinModeNormal;
     * masterConfig.outputMode = kSPI_SlaveSelectAutomaticOutput;
     * masterConfig.baudRate_Bps = 500000U;
     */
    SPI_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Bps = 500000;

    sourceClock = SPI_MASTER_CLK_FREQ;
    status      = SPI_RTOS_Init(&master_rtos_handle, EXAMPLE_SPI_MASTER_BASEADDR, &masterConfig, sourceClock);

    if (status != kStatus_Success)
    {
        PRINTF("DSPI master: error during initialization. \r\n");
        vTaskSuspend(NULL);
    }

    /* Init Buffer */
    uint8_t i = 0;
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        srcBuff[i] = i;
    }

    /* Send and receive data through loopback  */
    masterXfer.txData   = srcBuff;
    masterXfer.rxData   = destBuff;
    masterXfer.dataSize = BUFFER_SIZE;

    status = SPI_RTOS_Transfer(&master_rtos_handle, &masterXfer);

    if (status == kStatus_Success)
    {
        PRINTF("SPI transfer completed successfully. \r\n");
    }
    else
    {
        PRINTF("SPI transfer completed with error. \r\n");
    }

    /* Verify received data */
    uint32_t err = 0;
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        if (srcBuff[i] != destBuff[i])
        {
            err++;
            PRINTF("Mismatch at offset %d. %d != %d \n\r", i, destBuff[i], srcBuff[i]);
        }
    }
    if (err == 0)
    {
        PRINTF("Data verified ok.\n\r");
    }
    vTaskSuspend(NULL);
}
