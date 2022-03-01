/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "erpc_server_setup.h"
#include "erpc_matrix_multiply_server.h"
#include "erpc_matrix_multiply.h"
#include "erpc_error_handler.h"

#include "fsl_lpuart_cmsis.h"
#include "fsl_device_registers.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ERPC_DEMO_UART Driver_USART1

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

uint32_t LPUART1_GetFreq(void)
{
    return BOARD_DebugConsoleSrcFreq();
}

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

/*!
 * @brief Main function
 */
int main()
{
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* UART transport layer initialization */
    erpc_transport_t transport;

    transport = erpc_transport_cmsis_uart_init((void *)&ERPC_DEMO_UART);

    /* MessageBufferFactory initialization */
    erpc_mbf_t message_buffer_factory;
    message_buffer_factory = erpc_mbf_dynamic_init();

    /* eRPC server side initialization */
    erpc_server_init(transport, message_buffer_factory);

    /* adding the service to the server */
    erpc_service_t service = create_MatrixMultiplyService_service();
    erpc_add_service_to_server(service);

    while (1)
    {
        /* process message */
        erpc_status_t status = erpc_server_poll();

        /* handle error status */
        if (status != kErpcStatus_Success)
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

            /* exit program loop */
            break;
        }

        /* do other tasks */
        int i;
        for (i = 0; i < 10000; i++)
        {
        }
    }
    while (1)
    {
    }
}
