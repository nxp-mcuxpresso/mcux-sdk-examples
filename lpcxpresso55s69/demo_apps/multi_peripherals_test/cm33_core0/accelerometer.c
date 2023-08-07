/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_debug_console.h"
#include "board.h"
#include "demo_config.h"

#ifdef ACCELEROMETER_EXISTS

#include "task.h"
#include "event_groups.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern EventGroupHandle_t g_errorEvent;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void acc_task(void *pvParameters);

/*******************************************************************************
 * Code
 ******************************************************************************/

void acc_init(void)
{
    if (xTaskCreate(acc_task, "ACC Task", 1000UL / sizeof(portSTACK_TYPE), NULL, 5U, NULL) != pdPASS)
    {
        PRINTF("ACC Task Creation Failed!\r\n");
        while (1)
            ;
    }
}

static void acc_task(void *pvParameters)
{
    status_t status;
    uint8_t deviceId;

    BOARD_I2C_Init(BOARD_ACCEL_I2C_BASEADDR, BOARD_ACCEL_I2C_CLOCK_FREQ);
    status = BOARD_I2C_Receive(BOARD_ACCEL_I2C_BASEADDR, DEMO_ACC_DEVICE_ADDRESS, DEMO_ACC_SUB_ADDRESS,
                               DEMO_ACC_SUB_ADDRESS_SIZE, &deviceId, 1);

    if (status == kStatus_Success)
    {
        if (deviceId == DEMO_ACC_DEVICE_ID)
        {
            PRINTF("Accelerometer: Found Accelerometer!\r\n");
        }
        else
        {
            PRINTF("Accelerometer: Unknown accelerometer id: 0x%02x\r\n", deviceId);
            xEventGroupSetBits(g_errorEvent, 1U);
        }
    }
    else
    {
        PRINTF("Accelerometer: Failed to communicate with accelerometer\r\n");
        xEventGroupSetBits(g_errorEvent, 1U);
    }
    vTaskSuspend(NULL);
}
#endif
