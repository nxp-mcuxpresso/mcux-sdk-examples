/*
 * Copyright 2021-2022 NXP
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

#include "fsl_device_registers.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Variables
 ******************************************************************************/
erpc_server_t server;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

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
int main(void)
{
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach 12 MHz clock to I2C1 */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom1Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom1Clk, 1u, true);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM1);

    /* reset FLEXCOMM for SPI */
    RESET_PeripheralReset(kFC1_RST_SHIFT_RSTn);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    /* I2C transport layer initialization */
    erpc_transport_t transport;

#if defined(ERPC_BOARD_I2C_BASEADDR)
    transport = erpc_transport_i2c_slave_init((void *)(char *)ERPC_BOARD_I2C_BASEADDR, ERPC_BOARD_I2C_BAUDRATE,
                                              ERPC_BOARD_I2C_CLK_FREQ);
#elif defined(ERPC_BOARD_LPI2C_BASEADDR)
    transport = erpc_transport_lpi2c_slave_init((void *)(char *)ERPC_BOARD_LPI2C_BASEADDR, ERPC_BOARD_LPI2C_BAUDRATE,
                                                ERPC_BOARD_LPI2C_CLK_FREQ);
#endif

    /* MessageBufferFactory initialization */
    erpc_mbf_t message_buffer_factory;
    message_buffer_factory = erpc_mbf_dynamic_init();

    /* eRPC server side initialization */
    server = erpc_server_init(transport, message_buffer_factory);

    /* adding the service to the server */
    erpc_service_t service = create_MatrixMultiplyService_service();
    erpc_add_service_to_server(server, service);

    for (;;)
    {
        /* process message */
        erpc_status_t status = erpc_server_poll(server);

        /* handle error status */
        if (status != (erpc_status_t)kErpcStatus_Success)
        {
            /* print error description */
            erpc_error_handler(status, 0);

            /* removing the service from the server */
            erpc_remove_service_from_server(server, service);
            destroy_MatrixMultiplyService_service(service);

            /* stop erpc server */
            erpc_server_stop(server);

            /* print error description */
            erpc_server_deinit(server);

            /* exit program loop */
            break;
        }

        /* do other tasks */
        int32_t i;
        for (i = 0; i < 10000; i++)
        {
        }
    }
    for (;;)
    {
    }
}
