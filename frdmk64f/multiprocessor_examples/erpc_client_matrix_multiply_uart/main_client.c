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
#include "erpc_client_setup.h"
#include "erpc_error_handler.h"
#include "erpc_matrix_multiply.h"

#include "fsl_uart_cmsis.h"
#include "fsl_device_registers.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BUTTON_INIT()       GPIO_PinInit(BOARD_SW2_GPIO, BOARD_SW2_GPIO_PIN, &button_config)
#define IS_BUTTON_PRESSED() !GPIO_PinRead(BOARD_SW2_GPIO, BOARD_SW2_GPIO_PIN)
#define ERPC_DEMO_UART      Driver_USART0
#define MATRIX_ITEM_MAX_VALUE 50

/*******************************************************************************
 * Variables
 ******************************************************************************/

extern bool g_erpc_error_occurred;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

uint32_t UART0_GetFreq(void)
{
    return CLOCK_GetFreq(UART0_CLK_SRC);
}

/*!
 * @brief Fill matrices by random values
 */
static void fill_matrices(Matrix matrix1_ptr, Matrix matrix2_ptr)
{
    int32_t a, b;

    /* Fill both matrices by random values */
    for (a = 0; a < matrix_size; ++a)
    {
        for (b = 0; b < matrix_size; ++b)
        {
            matrix1_ptr[a][b] = rand() % MATRIX_ITEM_MAX_VALUE;
            matrix2_ptr[a][b] = rand() % MATRIX_ITEM_MAX_VALUE;
        }
    }
}

/*!
 * @brief Main function
 */
int main()
{
    Matrix matrix1 = {0}, matrix2 = {0}, result_matrix = {0};

    BOARD_InitPins();
    BOARD_BootClockRUN();

#if defined(BOARD_SW1_GPIO) || defined(BOARD_SW2_GPIO) || defined(BOARD_SW3_GPIO)
    /* Define the init structure for the input switch pin */
    gpio_pin_config_t button_config = {
        kGPIO_DigitalInput,
        0,
    };
    /* Configure BUTTON */
    BUTTON_INIT();
#endif

    /* UART transport layer initialization */
    erpc_transport_t transport;

    transport = erpc_transport_cmsis_uart_init((void *)&ERPC_DEMO_UART);

    /* MessageBufferFactory initialization */
    erpc_mbf_t message_buffer_factory;
    message_buffer_factory = erpc_mbf_dynamic_init();

    /* eRPC client side initialization */
    erpc_client_init(transport, message_buffer_factory);

    /* Set default error handler */
    erpc_client_set_error_handler(erpc_error_handler);

    /* Fill both matrices by random values */
    fill_matrices(matrix1, matrix2);

    while (1)
    {
        /* Send rRPC request to the server */
        erpcMatrixMultiply(matrix1, matrix2, result_matrix);

        /* Check if some error occurred in eRPC */
        if (g_erpc_error_occurred)
        {
            /* Exit program loop */
            break;
        }

#if defined(BOARD_SW1_GPIO) || defined(BOARD_SW2_GPIO) || defined(BOARD_SW3_GPIO)
        /* Press the button to initiate the next matrix multiplication */
        /* Check for button push. Pin is grounded when button is pushed. */
        while (1 != IS_BUTTON_PRESSED())
        {
        }

        /* Wait for a moment to eliminate the button glitch */
        int32_t i;
        for (i = 0; i < 2500000; ++i)
        {
            __NOP();
        }

        /* Fill both matrices by random values */
        fill_matrices(matrix1, matrix2);

#else
        while (1)
        {
        }
#endif
    }
    while (1)
    {
    }
}
