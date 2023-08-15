/*
 * Copyright 2018-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "fsl_debug_console.h"
#include "task.h"
#include "queue.h"
#include "dsp_ipc.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "unit_tests_nn.h"

#include "dsp_support.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define NN_APP_TASK_STACK_SIZE    (512)
#define CB_WORKER_TASK_STACK_SIZE (512)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

void app_nameservice_isr_cb(unsigned int new_ept, const char *new_ept_name, unsigned long flags, void *user_data)
{
    unsigned long *data = (unsigned long *)user_data;

    *data = new_ept;
}

/****************************** NEURAL NETWORKS ******************************/

static TaskHandle_t nn_app_task_handle = NULL;
void nn_app_task(void *param)
{
    // Wait for a message signaling DSP is ready
    srtm_message msg;
    dsp_ipc_recv_sync(&msg);

#if ECHO_UNIT_TEST == 1
    nn_echo_unit_test(UNIT_TEST_SYNC);
#endif

#if CONV_DS_UNIT_TEST == 1
    nn_conv_ds_unit_test(UNIT_TEST_SYNC);
    nn_conv_ds_unit_test(UNIT_TEST_ASYNC);
#endif

#if CONV_STD_UNIT_TEST == 1
    nn_conv_std_unit_test(UNIT_TEST_SYNC);
    nn_conv_std_unit_test(UNIT_TEST_ASYNC);
#endif

#if RELU_UNIT_TEST == 1
    nn_relu_unit_test(UNIT_TEST_SYNC);
    nn_relu_unit_test(UNIT_TEST_ASYNC);
#endif

#if MAXPOOL_UNIT_TEST == 1
    nn_maxpool_unit_test(UNIT_TEST_SYNC);
    nn_maxpool_unit_test(UNIT_TEST_ASYNC);
#endif

    while (1)
        ;
}

static TaskHandle_t cb_worker_task_handle = NULL;

/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Clear MUA reset before run DSP core */
    RESET_PeripheralReset(kMU_RST_SHIFT_RSTn);

    PRINTF("\r\nStarted NN UnitTests and benchmarks (%d iterations)\r\n", BENCH_ITERS);

    /* Start dsp firmware */
    BOARD_DSP_IPC_Init();
    BOARD_DSP_IPCAsync_Init(15);
    BOARD_DSP_Init();

#if DSP_IMAGE_COPY_TO_RAM
    PRINTF("DSP Image copied to SRAM\r\n");
#else
    PRINTF("DSP Image not copied. The image must be copied using the debugger\r\n");
#endif // DSP_IMAGE_COPY_TO_RAM

    if (xTaskCreate(nn_app_task, "NN_APP_TASK", NN_APP_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2,
                    &nn_app_task_handle) != pdPASS)
    {
        PRINTF("\r\nFailed to create nn application task\r\n");
        while (1)
            ;
    }

    if (xTaskCreate(dsp_ipc_queue_worker_task, "CB_WORKER_TASK", CB_WORKER_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1,
                    &cb_worker_task_handle) != pdPASS)
    {
        PRINTF("\r\nFailed to create callback worker task\r\n");
        while (1)
            ;
    }

    vTaskStartScheduler();

    PRINTF("Failed to start FreeRTOS on core0.\r\n");
    while (1)
        ;
}
