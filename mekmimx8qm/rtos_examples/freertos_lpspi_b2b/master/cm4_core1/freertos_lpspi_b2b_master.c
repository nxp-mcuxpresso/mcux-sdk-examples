/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
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
#include "fsl_lpspi.h"
#include "fsl_lpspi_freertos.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_irqsteer.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*Master related*/
#define EXAMPLE_LPSPI_MASTER_BASEADDR         DMA__LPSPI2
#define EXAMPLE_LPSPI_MASTER_IRQN             IRQSTEER_3_IRQn /*DMA LPSPI 2 interrupt is connected to IRQSteer Master 3 */
#define EXAMPLE_LPSPI_MASTER_PCS_FOR_INIT     kLPSPI_Pcs0
#define EXAMPLE_LPSPI_MASTER_PCS_FOR_TRANSFER kLPSPI_MasterPcs0
#define LPSPI_MASTER_CLK_FREQ                 (CLOCK_GetIpFreq(kCLOCK_DMA_Lpspi2))
#define TRANSFER_SIZE     (512U)    /*! Transfer dataSize.*/
#define TRANSFER_BAUDRATE (500000U) /*! Transfer baudrate - 500k */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* LPSPI user callback */
extern uint32_t LPSPI_GetInstance(LPSPI_Type *base);

/*******************************************************************************
 * Variables
 ******************************************************************************/
lpspi_slave_handle_t g_s_handle;

uint8_t masterReceiveBuffer[TRANSFER_SIZE] = {0};
uint8_t masterSendBuffer[TRANSFER_SIZE]    = {0};
uint8_t slaveSendBuffer[TRANSFER_SIZE]     = {0};

SemaphoreHandle_t lpspi_sem;
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
    int i;

    /* Init board hardware. */
    sc_ipc_t ipc;
    uint32_t freq;

    ipc = BOARD_InitRpc();

    BOARD_InitPins(ipc);
    BOARD_BootClockRUN();
    BOARD_InitMemory();
    BOARD_InitDebugConsole();

    /* Power on SPI. */
    if (sc_pm_set_resource_power_mode(ipc, SC_R_SPI_2, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to power on LPSPI2\r\n");
    }

    /* Setup LPSPI clock */
    freq = CLOCK_SetIpFreq(kCLOCK_DMA_Lpspi2, SC_66MHZ);
    if (freq == 0)
    {
        PRINTF("Error: Failed to set LPSPI2 clock frequency\r\n");
    }

    /* Enable LPSPI interrupt in IRQSTEER */
    if (sc_pm_set_resource_power_mode(ipc, SC_R_IRQSTR_M4_1, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to power on IRQSTEER!\r\n");
    }
    IRQSTEER_Init(IRQSTEER);
    IRQSTEER_EnableInterrupt(IRQSTEER, DMA_SPI2_INT_IRQn);

    NVIC_SetPriority(EXAMPLE_LPSPI_MASTER_IRQN, 3);

    PRINTF("FreeRTOS LPSPI master example starts.\r\n");

    PRINTF("This example uses two boards to connect with one as master and anohter as slave.\r\n");

    PRINTF("Master and slave are both use interrupt way.\r\n");
    PRINTF("Please make sure you make the correct line connection. Basically, the connection is:\r\n");
    PRINTF("LPSPI_master -- LPSPI_slave\r\n");
    PRINTF("    CLK      --    CLK\r\n");
    PRINTF("    PCS      --    PCS\r\n");
    PRINTF("    SOUT     --    SIN\r\n");
    PRINTF("    SIN      --    SOUT\r\n");
    PRINTF("\r\n");

    /* Initialize data in transfer buffers */
    for (i = 0; i < TRANSFER_SIZE; i++)
    {
        masterSendBuffer[i]    = i % 256;
        masterReceiveBuffer[i] = 0;

        slaveSendBuffer[i] = ~masterSendBuffer[i];
    }

    if (xTaskCreate(master_task, "Master_task", configMINIMAL_STACK_SIZE + 64, NULL, master_task_PRIORITY, NULL) !=
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
    lpspi_transfer_t masterXfer;
    lpspi_rtos_handle_t master_rtos_handle;
    lpspi_master_config_t masterConfig;
    status_t status;

    LPSPI_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate = TRANSFER_BAUDRATE;
    masterConfig.whichPcs = EXAMPLE_LPSPI_MASTER_PCS_FOR_INIT;

    status = LPSPI_RTOS_Init(&master_rtos_handle, EXAMPLE_LPSPI_MASTER_BASEADDR, &masterConfig, LPSPI_MASTER_CLK_FREQ);
    if (status != kStatus_Success)
    {
        PRINTF("LPSPI master: error during initialization. \r\n");
        vTaskSuspend(NULL);
    }

    /*Start master transfer*/
    masterXfer.txData      = masterSendBuffer;
    masterXfer.rxData      = masterReceiveBuffer;
    masterXfer.dataSize    = TRANSFER_SIZE;
    masterXfer.configFlags = EXAMPLE_LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous | kLPSPI_SlaveByteSwap;

    status = LPSPI_RTOS_Transfer(&master_rtos_handle, &masterXfer);
    if (status == kStatus_Success)
    {
        PRINTF("LPSPI master transfer completed successfully.\r\n");
    }
    else
    {
        PRINTF("LPSPI master transfer completed with error.\r\n");
    }

    uint32_t errorCount;
    uint32_t i;

    errorCount = 0;
    for (i = 0; i < TRANSFER_SIZE; i++)
    {
        if (slaveSendBuffer[i] != masterReceiveBuffer[i])
        {
            errorCount++;
        }
    }

    if (errorCount == 0)
    {
        PRINTF("LPSPI transfer all data matched !\r\n");
    }
    else
    {
        PRINTF("Error occurred in LPSPI transfer !\r\n");
    }

    vTaskSuspend(NULL);
}
