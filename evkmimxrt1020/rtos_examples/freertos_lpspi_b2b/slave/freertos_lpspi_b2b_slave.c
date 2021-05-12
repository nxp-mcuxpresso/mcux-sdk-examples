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
#include "semphr.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_lpspi.h"
#include "pin_mux.h"
#include "board.h"

#include "fsl_common.h"
#if ((defined FSL_FEATURE_SOC_INTMUX_COUNT) && (FSL_FEATURE_SOC_INTMUX_COUNT))
#include "fsl_intmux.h"
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Slave related */
#define EXAMPLE_LPSPI_SLAVE_BASEADDR (LPSPI1)
#define EXAMPLE_LPSPI_SLAVE_IRQN     (LPSPI1_IRQn)

#define EXAMPLE_LPSPI_SLAVE_PCS_FOR_INIT     (kLPSPI_Pcs0)
#define EXAMPLE_LPSPI_SLAVE_PCS_FOR_TRANSFER (kLPSPI_SlavePcs0)

/* Select USB1 PLL PFD0 (720 MHz) as lpspi clock source */
#define EXAMPLE_LPSPI_CLOCK_SOURCE_SELECT (1U)
/* Clock divider for master lpspi clock source */
#define EXAMPLE_LPSPI_CLOCK_SOURCE_DIVIDER (7U)

#define TRANSFER_SIZE (512U) /*! Transfer dataSize.*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* LPSPI user callback */
void LPSPI_SlaveUserCallback(LPSPI_Type *base, lpspi_slave_handle_t *handle, status_t status, void *userData);
extern uint32_t LPSPI_GetInstance(LPSPI_Type *base);

/*******************************************************************************
 * Variables
 ******************************************************************************/
lpspi_slave_handle_t g_s_handle;

uint8_t masterSendBuffer[TRANSFER_SIZE]   = {0};
uint8_t slaveReceiveBuffer[TRANSFER_SIZE] = {0};
uint8_t slaveSendBuffer[TRANSFER_SIZE]    = {0};

SemaphoreHandle_t lpspi_sem;

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Task priorities. */
#define slave_task_PRIORITY (configMAX_PRIORITIES - 2)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void slave_task(void *pvParameters);

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
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /*Set clock source for LPSPI*/
    CLOCK_SetMux(kCLOCK_LpspiMux, EXAMPLE_LPSPI_CLOCK_SOURCE_SELECT);
    CLOCK_SetDiv(kCLOCK_LpspiDiv, EXAMPLE_LPSPI_CLOCK_SOURCE_DIVIDER);

    NVIC_SetPriority(EXAMPLE_LPSPI_SLAVE_IRQN, 2);

    PRINTF("FreeRTOS LPSPI slave example starts.\r\n");

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
        masterSendBuffer[i] = i % 256;

        slaveSendBuffer[i]    = ~masterSendBuffer[i];
        slaveReceiveBuffer[i] = 0;
    }

    if (xTaskCreate(slave_task, "Slave_task", configMINIMAL_STACK_SIZE + 64, NULL, slave_task_PRIORITY, NULL) != pdPASS)
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
 * @brief Data structure and callback function for slave SPI communication.
 */

typedef struct _callback_message_t
{
    status_t async_status;
    SemaphoreHandle_t sem;
} callback_message_t;

void LPSPI_SlaveUserCallback(LPSPI_Type *base, lpspi_slave_handle_t *handle, status_t status, void *userData)
{
    callback_message_t *cb_msg = (callback_message_t *)userData;
    BaseType_t reschedule      = 0;

    cb_msg->async_status = status;
    xSemaphoreGiveFromISR(cb_msg->sem, &reschedule);
    portYIELD_FROM_ISR(reschedule);
}

/*!
 * @brief Task responsible for slave SPI communication.
 */
static void slave_task(void *pvParameters)
{
    lpspi_slave_config_t slaveConfig;
    lpspi_transfer_t slaveXfer;

    uint32_t errorCount;
    uint32_t i;
    callback_message_t cb_msg;

    cb_msg.sem = xSemaphoreCreateBinary();
    lpspi_sem  = cb_msg.sem;
    if (cb_msg.sem == NULL)
    {
        PRINTF("LPSPI slave: Error creating semaphore\r\n");
        vTaskSuspend(NULL);
    }

    /* Slave config */
    LPSPI_SlaveGetDefaultConfig(&slaveConfig);
    slaveConfig.whichPcs = EXAMPLE_LPSPI_SLAVE_PCS_FOR_INIT;

    LPSPI_SlaveInit(EXAMPLE_LPSPI_SLAVE_BASEADDR, &slaveConfig);

    /*Set up slave first */
    LPSPI_SlaveTransferCreateHandle(EXAMPLE_LPSPI_SLAVE_BASEADDR, &g_s_handle, LPSPI_SlaveUserCallback, &cb_msg);

    /*Set slave transfer ready to receive/send data*/
    slaveXfer.txData      = slaveSendBuffer;
    slaveXfer.rxData      = slaveReceiveBuffer;
    slaveXfer.dataSize    = TRANSFER_SIZE;
    slaveXfer.configFlags = EXAMPLE_LPSPI_SLAVE_PCS_FOR_TRANSFER | kLPSPI_SlaveByteSwap;

    LPSPI_SlaveTransferNonBlocking(EXAMPLE_LPSPI_SLAVE_BASEADDR, &g_s_handle, &slaveXfer);

    /* Wait for transfer to finish */
    xSemaphoreTake(cb_msg.sem, portMAX_DELAY);

    if (cb_msg.async_status == kStatus_Success)
    {
        PRINTF("LPSPI slave transfer completed successfully.\r\n");
    }
    else
    {
        PRINTF("LPSPI slave transfer completed with error.\r\n");
    }

    errorCount = 0;
    for (i = 0; i < TRANSFER_SIZE; i++)
    {
        if (masterSendBuffer[i] != slaveReceiveBuffer[i])
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
