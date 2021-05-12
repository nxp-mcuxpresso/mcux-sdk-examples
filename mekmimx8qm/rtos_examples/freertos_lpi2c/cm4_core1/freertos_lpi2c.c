/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Standard C Included Files */
#include <stdio.h>
#include <string.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Freescale includes. */
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_lpi2c.h"
#include "fsl_lpi2c_freertos.h"

#include "fsl_irqsteer.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_I2C_MASTER_BASE (DMA__LPI2C0)
#define EXAMPLE_I2C_MASTER_IRQN (IRQSTEER_3_IRQn) /*DMA__LPI2C0 interrupt is connected to IRQSteer Master 3 */

/* Used when compiled as Slave. */
#define EXAMPLE_I2C_SLAVE_BASE (DMA__LPI2C4)
#define EXAMPLE_I2C_SLAVE_IRQN (IRQSTEER_4_IRQn) /*DMA__LPI2C4 interrupt is connected to IRQSteer Master 4 */

#define LPI2C_CLOCK_FREQUENCY CLOCK_GetIpFreq(kCLOCK_DMA_Lpi2c0)
/* clang-format off */
/*
The example support single board communication(one instance works as master, another
instance works as slave) or board to board communication. Defult is single board mode.
*/
#define SINGLE_BOARD 0
#define BOARD_TO_BOARD 1
#ifndef EXAMPLE_CONNECT_I2C
#define EXAMPLE_CONNECT_I2C SINGLE_BOARD
#endif /* EXAMPLE_CONNECT_I2C */
#if (EXAMPLE_CONNECT_I2C == BOARD_TO_BOARD)
#define isMASTER 0
#define isSLAVE 1
#ifndef I2C_MASTER_SLAVE
#define I2C_MASTER_SLAVE isMASTER
#endif /* I2C_MASTER_SLAVE */
#endif /* EXAMPLE_CONNECT_I2C */

#define EXAMPLE_I2C_MASTER   ((LPI2C_Type *)EXAMPLE_I2C_MASTER_BASE)
#define EXAMPLE_I2C_SLAVE   ((LPI2C_Type *)EXAMPLE_I2C_SLAVE_BASE)

#define I2C_MASTER_SLAVE_ADDR_7BIT (0x7EU)
#define I2C_BAUDRATE (100000) /* 100K */
#define I2C_DATA_LENGTH (32) /* MAX is 256 */

/* Task priorities. */
#define lpi2c_task_PRIORITY (configMAX_PRIORITIES - 1)
/* clang-format on */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t g_slave_buff[I2C_DATA_LENGTH];
uint8_t g_master_buff[I2C_DATA_LENGTH];

#if ((I2C_MASTER_SLAVE == isSLAVE) || (EXAMPLE_CONNECT_I2C == SINGLE_BOARD))
lpi2c_slave_handle_t g_s_handle;
#endif
volatile uint32_t timeout = 100000;

SemaphoreHandle_t lpi2c_sem;
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Task priorities. */
#define slave_task_PRIORITY  (configMAX_PRIORITIES - 2)
#define master_task_PRIORITY (configMAX_PRIORITIES - 1)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void slave_task(void *pvParameters);
#if ((I2C_MASTER_SLAVE == isMaster) || (EXAMPLE_CONNECT_I2C == SINGLE_BOARD))
static void master_task(void *pvParameters);
#endif
/*******************************************************************************
 * Code
 ******************************************************************************/
int main(void)
{
    uint32_t i = 0;

    /* Init board hardware. */
    sc_ipc_t ipc;
    uint32_t freq;

    ipc = BOARD_InitRpc();

    BOARD_InitPins(ipc);
    BOARD_BootClockRUN();
    BOARD_InitMemory();
    BOARD_InitDebugConsole();

    /* Power on LPI2C. */
    if (sc_pm_set_resource_power_mode(ipc, SC_R_I2C_0, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to power on LPI2C\r\n");
    }
    if (sc_pm_set_resource_power_mode(ipc, SC_R_I2C_4, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to power on LPI2C\r\n");
    }

    /* Set LPI2C clock */
    freq = CLOCK_SetIpFreq(kCLOCK_DMA_Lpi2c0, SC_24MHZ);
    if (freq == 0)
    {
        PRINTF("Error: Failed to set LPI2C frequency\r\n");
    }
    freq = CLOCK_SetIpFreq(kCLOCK_DMA_Lpi2c4, SC_24MHZ);
    if (freq == 0)
    {
        PRINTF("Error: Failed to set LPI2C frequency\r\n");
    }

    /* Enable interrupt in irqsteer */
    if (sc_pm_set_resource_power_mode(ipc, SC_R_IRQSTR_M4_1, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to power on IRQSTEER!\r\n");
    }
    IRQSTEER_Init(IRQSTEER);
    IRQSTEER_EnableInterrupt(IRQSTEER, DMA_I2C0_INT_IRQn);
    IRQSTEER_EnableInterrupt(IRQSTEER, DMA_I2C4_INT_IRQn);

    /* Set IRQ priority for freertos_lpi2c */
    NVIC_SetPriority(EXAMPLE_I2C_MASTER_IRQN, 3);
    NVIC_SetPriority(EXAMPLE_I2C_SLAVE_IRQN, 2);
    PRINTF("\r\nLPI2C example -- MasterInterrupt_SlaveInterrupt.\r\n");

    /* Set up i2c master to send data to slave */
    for (i = 0; i < I2C_DATA_LENGTH; i++)
    {
        g_master_buff[i] = i;
    }

    if (xTaskCreate(slave_task, "Slave_task", configMINIMAL_STACK_SIZE + 100, NULL, slave_task_PRIORITY, NULL) !=
        pdPASS)
    {
        PRINTF("Failed to create slave task");
        while (1)
            ;
    }

    vTaskStartScheduler();

    while (1)
    {
    }
}

/*!
 * @brief Data structure and callback function for slave I2C communication.
 */

typedef struct _callback_message_t
{
    status_t async_status;
    SemaphoreHandle_t sem;
} callback_message_t;

#if ((I2C_MASTER_SLAVE == isSLAVE) || (EXAMPLE_CONNECT_I2C == SINGLE_BOARD))
static void lpi2c_slave_callback(LPI2C_Type *base, lpi2c_slave_transfer_t *xfer, void *userData)
{
    callback_message_t *cb_msg = (callback_message_t *)userData;
    BaseType_t reschedule      = 0;

    switch (xfer->event)
    {
        case kLPI2C_SlaveReceiveEvent:
            xfer->data     = g_slave_buff;
            xfer->dataSize = I2C_DATA_LENGTH;
            break;
        case kLPI2C_SlaveCompletionEvent:
            cb_msg->async_status = xfer->completionStatus;
            xSemaphoreGiveFromISR(cb_msg->sem, &reschedule);
            portYIELD_FROM_ISR(reschedule);
            break;
        default:
            break;
    }
}
#endif

/*!
 * @brief Task responsible for slave I2C communication.
 */

static void slave_task(void *pvParameters)
{
    callback_message_t cb_msg;

    cb_msg.sem = xSemaphoreCreateBinary();
    lpi2c_sem  = cb_msg.sem;
    if (cb_msg.sem == NULL)
    {
        PRINTF("I2C slave: Error creating semaphore\r\n");
        vTaskSuspend(NULL);
    }

#if ((I2C_MASTER_SLAVE == isSLAVE) || (EXAMPLE_CONNECT_I2C == SINGLE_BOARD))
    /* Set up i2c slave first */
    /*
     * slaveConfig.address0 = 0U;
     * slaveConfig.address1 = 0U;
     * slaveConfig.addressMatchMode = kLPI2C_MatchAddress0;
     * slaveConfig.filterDozeEnable = true;
     * slaveConfig.filterEnable = true;
     * slaveConfig.enableGeneralCall = false;
     * slaveConfig.ignoreAck = false;
     * slaveConfig.enableReceivedAddressRead = false;
     * slaveConfig.sdaGlitchFilterWidth_ns = 0;
     * slaveConfig.sclGlitchFilterWidth_ns = 0;
     * slaveConfig.dataValidDelay_ns = 0;
     * slaveConfig.clockHoldTime_ns = 0;
     */
    lpi2c_slave_config_t slaveConfig;
    LPI2C_SlaveGetDefaultConfig(&slaveConfig);

    slaveConfig.address0 = I2C_MASTER_SLAVE_ADDR_7BIT;
    slaveConfig.address1 = 0;
    LPI2C_SlaveInit(EXAMPLE_I2C_SLAVE, &slaveConfig, LPI2C_CLOCK_FREQUENCY);

    memset(&g_s_handle, 0, sizeof(g_s_handle));
    memset(&g_slave_buff, 0, sizeof(g_slave_buff));

    LPI2C_SlaveTransferCreateHandle(EXAMPLE_I2C_SLAVE, &g_s_handle, lpi2c_slave_callback, &cb_msg);
    LPI2C_SlaveTransferNonBlocking(EXAMPLE_I2C_SLAVE, &g_s_handle,
                                   kLPI2C_SlaveReceiveEvent | kLPI2C_SlaveCompletionEvent);
#endif
#if ((I2C_MASTER_SLAVE == isMASTER) || (EXAMPLE_CONNECT_I2C == SINGLE_BOARD))
    if (xTaskCreate(master_task, "Master_task", configMINIMAL_STACK_SIZE + 100, NULL, master_task_PRIORITY, NULL) !=
        pdPASS)
    {
        vTaskSuspend(NULL);
        PRINTF("Failed to create master task");
    }
#endif

    /* Wait for transfer to finish */
    if (xSemaphoreTake(cb_msg.sem, portMAX_DELAY) != pdTRUE)
    {
        PRINTF("Failed to take semaphore.\r\n");
    }

#if ((I2C_MASTER_SLAVE == isSLAVE) || (EXAMPLE_CONNECT_I2C == SINGLE_BOARD))
    if (cb_msg.async_status == kStatus_Success)
    {
        PRINTF("I2C slave transfer completed successfully. \r\n\r\n");
    }
    else
    {
        PRINTF("I2C slave transfer completed with error. \r\n\r\n");
    }

    int i;
    /* Transfer completed. Check the data. */
    for (i = 0; i < I2C_DATA_LENGTH; i++)
    {
        if (g_slave_buff[i] != g_master_buff[i])
        {
            PRINTF("\r\nError occurred in this transfer ! \r\n");
            break;
        }
    }

    if (i == 32)
    {
        PRINTF("\r\n Transfer successfully!\r\n ");
    }

    PRINTF("\r\nSlave received data :");
    for (i = 0; i < I2C_DATA_LENGTH; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_slave_buff[i]);
    }
    PRINTF("\r\n\r\n");
#endif

    vTaskSuspend(NULL);
}

#if ((I2C_MASTER_SLAVE == isMaster) || (EXAMPLE_CONNECT_I2C == SINGLE_BOARD))
static void master_task(void *pvParameters)
{
    lpi2c_master_config_t masterConfig;
    lpi2c_rtos_handle_t master_rtos_handle;
    lpi2c_master_transfer_t masterXfer;
    status_t status;
    uint32_t i = 0;

    PRINTF("Master will send data :");
    for (i = 0; i < I2C_DATA_LENGTH; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_master_buff[i]);
    }
    PRINTF("\r\n\r\n");

    /*
     * masterConfig.debugEnable = false;
     * masterConfig.ignoreAck = false;
     * masterConfig.pinConfig = kLPI2C_2PinOpenDrain;
     * masterConfig.baudRate_Hz = 100000U;
     * masterConfig.busIdleTimeout_ns = 0;
     * masterConfig.pinLowTimeout_ns = 0;
     * masterConfig.sdaGlitchFilterWidth_ns = 0;
     * masterConfig.sclGlitchFilterWidth_ns = 0;
     */
    LPI2C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Hz = I2C_BAUDRATE;

    status = LPI2C_RTOS_Init(&master_rtos_handle, EXAMPLE_I2C_MASTER, &masterConfig, LPI2C_CLOCK_FREQUENCY);
    if (status != kStatus_Success)
    {
        PRINTF("LPI2C master: Error initializing LPI2C!\r\n");
        vTaskSuspend(NULL);
    }

    memset(&masterXfer, 0, sizeof(masterXfer));
    masterXfer.slaveAddress   = I2C_MASTER_SLAVE_ADDR_7BIT;
    masterXfer.direction      = kLPI2C_Write;
    masterXfer.subaddress     = 0;
    masterXfer.subaddressSize = 0;
    masterXfer.data           = g_master_buff;
    masterXfer.dataSize       = I2C_DATA_LENGTH;
    masterXfer.flags          = kLPI2C_TransferDefaultFlag;

    status = LPI2C_RTOS_Transfer(&master_rtos_handle, &masterXfer);
    if (status == kStatus_Success)
    {
#if (EXAMPLE_CONNECT_I2C == BOARD_TO_BOARD)
        xSemaphoreGive(lpi2c_sem);
#endif
        PRINTF("I2C master transfer completed successfully.\r\n");
    }
    else
    {
        PRINTF("I2C master transfer completed with error!\r\n");
    }

    vTaskSuspend(NULL);
}
#endif
