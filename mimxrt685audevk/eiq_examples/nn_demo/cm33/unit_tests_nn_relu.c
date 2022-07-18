/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "unit_tests_nn.h"
#include "FreeRTOS.h"
#include "queue.h"
#include <stdio.h>
#include "fsl_debug_console.h"
#include "fsl_common.h"
#include "counter.h"
#include "srtm_config.h"
#include "dsp_nn_utils.h"
#include "dsp_nn.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define UNIT_TEST_NAME "RELU"
#define DATA_WIDTH     4
#define VEC_SIZE       1024
#define INPUT_SIZE     VEC_SIZE *DATA_WIDTH
#define OUTPUT_SIZE    VEC_SIZE *DATA_WIDTH
#define THRESHOLD      (1 << 15)

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Buffers */
static float *p_inp = NULL;
static float *p_out = NULL;

/* Variables used for async */
static QueueHandle_t queue;
static srtm_message msg;

/*******************************************************************************
 * Code
 ******************************************************************************/

static void alloc_buffers()
{
    p_inp = SDK_Malloc(INPUT_SIZE, 8);
    p_out = SDK_Malloc(OUTPUT_SIZE, 8);
}

static void reset_buffers()
{
    memset(p_inp, 0, INPUT_SIZE);
    memset(p_out, 0, OUTPUT_SIZE);
}

static void fill_buffers()
{
    int i;

    for (i = 0; i < VEC_SIZE; i++)
    {
        p_inp[i] = (float)(i - VEC_SIZE / 2);
    }
}

static void free_buffers()
{
    SDK_Free(p_inp);
    SDK_Free(p_out);
}

static void process_sync()
{

#if NN_ENABLE_xa_nn_vec_relu_f32_f32 == 1
    xa_nn_vec_relu_f32_f32(p_out, p_inp, THRESHOLD, VEC_SIZE);
#endif
}

static void process_async()
{

#if NN_ENABLE_xa_nn_vec_relu_f32_f32 == 1
    xa_nn_vec_relu_f32_f32_async(nn_cb, queue, p_out, p_inp, THRESHOLD, VEC_SIZE);
    xQueueReceive(queue, &msg, portMAX_DELAY);
#endif
}

static int check_output()
{
    int i;
    float expected_result;

    for (i = 0; i < VEC_SIZE; i++)
    {
        expected_result = p_inp[i];
        if (expected_result < 0)
        {
            expected_result = 0;
        }

        if (((float *)p_out)[i] != expected_result)
        {
            PRINTF("%s unit test failed at index %d: %d != %d\r\n", UNIT_TEST_NAME, i, p_out[i], expected_result);
            return 0;
        }
    }

    PRINTF("%s unit test succeeded\r\n", UNIT_TEST_NAME);
    return 1;
}

void nn_relu_unit_test(int mode)
{
    PRINTF("Running %s %s\r\n", UNIT_TEST_NAME, mode == UNIT_TEST_SYNC ? "SYNC" : "ASYNC");

#if NN_ENABLE_xa_nn_vec_relu_f32_f32 == 1
    int i;
    volatile unsigned long tic, toc;
    unsigned long total_cycles = 0;
    float total_ms;

    if (mode == UNIT_TEST_ASYNC)
    {
        queue = xQueueCreate(1, sizeof(srtm_message));
        vQueueAddToRegistry(queue, UNIT_TEST_NAME "QUEUE");
    }

    alloc_buffers();

    for (i = 0; i < BENCH_ITERS; i++)
    {
        reset_buffers();
        fill_buffers();
        KIN1_ResetCycleCounter();

        // Process
        tic = get_ccount();
        switch (mode)
        {
            case UNIT_TEST_SYNC:
                process_sync();
                break;
            case UNIT_TEST_ASYNC:
                process_async();
                break;
            default:
                PRINTF("WRONG unit test mode\r\n");
                return;
        }
        toc = get_ccount();
        total_cycles += toc - tic;

        if (i == 0)
            check_output();
    }

    total_cycles /= BENCH_ITERS;
    total_ms = COUNT_TO_USEC(total_cycles, SystemCoreClock) / 1000.0;

    PRINTF("Avg Inference cycles: %u time: %d ms\r\n", total_cycles, total_ms);
    PRINTF("Throughput: %d fps\r\n", 1000 / total_ms);

    if (mode == UNIT_TEST_ASYNC)
    {
        vQueueUnregisterQueue(queue);
        vQueueDelete(queue);
    }
    free_buffers();
#else
    PRINTF("%s xa_nn_vec_relu_f32_f32%s not supported\r\n", UNIT_TEST_NAME, mode == UNIT_TEST_ASYNC ? "_async" : "");
#endif
}
