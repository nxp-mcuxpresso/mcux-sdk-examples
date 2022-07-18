/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __UNIT_TESTS_NN_H__
#define __UNIT_TESTS_NN_H__

#include "srtm_config.h"

/* Enable (1) or disable (0) neural networks unit tests */
#define CONV_DS_UNIT_TEST  1
#define CONV_STD_UNIT_TEST 1
#define RELU_UNIT_TEST     1
#define MAXPOOL_UNIT_TEST  1
#define ECHO_UNIT_TEST     1

/**
 * @brief Mode of the unit test
 * Defines if a unit test uses the async or the sync API
 */
typedef enum _unit_test_mode
{
    UNIT_TEST_SYNC,
    UNIT_TEST_ASYNC
} unit_test_mode_t;

/* Number of iterations for benchamrking */
#ifndef BENCH_ITERS
#define BENCH_ITERS 10
#endif

/**
 * @brief Callback function used to signal the end of a layer processing
 * @param params        the parameters of the callback
 * @param msg           the incomming message
 */
void nn_cb(void *params, srtm_message *msg);

/*******************************************************************************
       Neural Networks UNIT TESTS
*******************************************************************************/

/*!
 * @brief Perform a Depthwise Separable Convolution unit test
 * @param mode          mode of the unit test (sync/async)
 */
void nn_conv_ds_unit_test(int mode);

/*!
 * @brief Perform a Standard Convolution unit test
 * @param mode          mode of the unit test (sync/async)
 */
void nn_conv_std_unit_test(int mode);

/*!
 * @brief Perform a Rectified Linera Unit unit test
 * @param mode          mode of the unit test (sync/async)
 */
void nn_relu_unit_test(int mode);

/*!
 * @brief Perform a Max Pooling unit test
 * @param mode          mode of the unit test (sync/async)
 */
void nn_maxpool_unit_test(int mode);

/*!
 * @brief Perform an echo Unit unit test
 * @param mode          mode of the unit test (sync/async)
 */
void nn_echo_unit_test(int mode);

#endif // __UNIT_TESTS_NN_H__
