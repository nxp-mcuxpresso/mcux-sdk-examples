/*
 * Copyright (c) 2015-2016,, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "rpmsg_lite.h"
#include "erpc_server_setup.h"
#include "erpc_matrix_multiply_server.h"
#include "erpc_matrix_multiply.h"
#include "erpc_error_handler.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fsl_debug_console.h"

#include "mcmgr.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ERPC_TRANSPORT_RPMSG_LITE_LINK_ID (RL_PLATFORM_IMXRT1170_M7_M4_LINK_ID)
#define MCMGR_USED
#define APP_TASK_STACK_SIZE       (304U)
#define APP_ERPC_READY_EVENT_DATA (1U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
static TaskHandle_t app_task_handle = NULL;
#ifdef MCMGR_USED
static uint32_t startupData;
static mcmgr_status_t mcmgrStatus;
#endif
/*!
 * @brief erpcMatrixMultiply function implementation.
 *
 * This is the implementation of the erpcMatrixMultiply function called by the primary core.
 *
 * @param matrix1 First matrix
 * @param matrix2 Second matrix
 * @param result_matrix Result matrix
 */
void erpcMatrixMultiply(Matrix matrix1, Matrix matrix2, Matrix result_matrix)
{
    int32_t i, j, k;

    /* Clear the result matrix */
    for (i = 0; i < matrix_size; ++i)
    {
        for (j = 0; j < matrix_size; ++j)
        {
            result_matrix[i][j] = 0;
        }
    }

    /* Multiply two matrices */
    for (i = 0; i < matrix_size; ++i)
    {
        for (j = 0; j < matrix_size; ++j)
        {
            for (k = 0; k < matrix_size; ++k)
            {
                result_matrix[i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
    }
}
#ifdef MCMGR_USED
static void SignalReady(void)
{
    /* Signal the other core we are ready by triggering the event and passing the APP_ERPC_READY_EVENT_DATA */
    (void)MCMGR_TriggerEvent(kMCMGR_RemoteApplicationEvent, APP_ERPC_READY_EVENT_DATA);
}

/*!
 * @brief Application-specific implementation of the SystemInitHook() weak function.
 */
void SystemInitHook(void)
{
    /* Initialize MCMGR - low level multicore management library. Call this
       function as close to the reset entry as possible to allow CoreUp event
       triggering. The SystemInitHook() weak function overloading is used in this
       application. */
    (void)MCMGR_EarlyInit();
}
#endif /* MCMGR_USED */

static void app_task(void *param)
{
    /* RPMsg-Lite transport layer initialization */
    erpc_transport_t transport;

    /* Print the initial banner */
    (void)PRINTF("\r\neRPC Matrix Multiply demo started...\r\n");

#ifdef MCMGR_USED
    transport = erpc_transport_rpmsg_lite_rtos_remote_init(101U, 100U, (void *)(char *)startupData,
                                                           ERPC_TRANSPORT_RPMSG_LITE_LINK_ID, SignalReady, NULL);
#elif defined(RPMSG_LITE_MASTER_IS_LINUX)
    transport              = erpc_transport_rpmsg_lite_tty_rtos_remote_init(101U, 1024U, (void *)RPMSG_LITE_SHMEM_BASE,
                                                               ERPC_TRANSPORT_RPMSG_LITE_LINK_ID, NULL,
                                                               RPMSG_LITE_NS_ANNOUNCE_STRING);
#else
    transport = erpc_transport_rpmsg_lite_rtos_remote_init(101U, 100U, (void *)RPMSG_LITE_SHMEM_BASE,
                                                           ERPC_TRANSPORT_RPMSG_LITE_LINK_ID, NULL, NULL);
#endif
    /* MessageBufferFactory initialization */
    erpc_mbf_t message_buffer_factory;

#ifdef RPMSG_LITE_MASTER_IS_LINUX
    message_buffer_factory = erpc_mbf_rpmsg_tty_init(transport);
#else
    message_buffer_factory = erpc_mbf_rpmsg_init(transport);
#endif

    /* eRPC server side initialization */
    (void)erpc_server_init(transport, message_buffer_factory);

    /* adding the service to the server */
    erpc_service_t service = create_MatrixMultiplyService_service();
    erpc_add_service_to_server(service);

    (void)PRINTF("\r\neRPC setup done, waiting for requests...\r\n");

#ifdef RPMSG_LITE_MASTER_IS_LINUX
    /* ignore first hello world message from RPMSG tty device */
    erpc_server_run();
#endif

    /* process message */
    erpc_status_t status = erpc_server_run();

    /* handle error status */
    if (status != (erpc_status_t)kErpcStatus_Success)
    {
        /* print error description */
        erpc_error_handler(status, 0);

        /* removing the service from the server */
        erpc_remove_service_from_server(service);
        destroy_MatrixMultiplyService_service(service);

        /* stop erpc server */
        erpc_server_stop();

        /* print error description */
        erpc_server_deinit();
    }

    for (;;)
    {
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_ConfigMPU();
    BOARD_InitPins();

#ifdef MCMGR_USED

    /* Initialize MCMGR before calling its API */
    (void)MCMGR_Init();

    /* Get the startup data */
    do
    {
        mcmgrStatus = MCMGR_GetStartupData(&startupData);
    } while (mcmgrStatus != kStatus_MCMGR_Success);
#endif

    if (xTaskCreate(app_task, "APP_TASK", APP_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1U, &app_task_handle) != pdPASS)
    {
        /* Failed to create application task */
        return -1;
    }

    vTaskStartScheduler();

    return 0;
}
