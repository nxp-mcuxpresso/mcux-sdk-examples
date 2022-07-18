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

#include "maxpool_in.h"
#include "maxpool_out.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define UNIT_TEST_NAME "MAXPOOL"
#define INPUT_WIDTH    8
#define INPUT_HEIGHT   8
#define INPUT_CHANNELS 50
#define KERNEL_WIDTH   2
#define KERNEL_HEIGHT  2
#define X_STRIDE       2
#define Y_STRIDE       2
#define X_PADDING      0
#define Y_PADDING      0
#define OUTPUT_WIDTH   4
#define OUTPUT_HEIGHT  4
#define PRECISION      -1
#define DATA_WIDTH     4

#define INPUT_SIZE  (INPUT_HEIGHT * INPUT_WIDTH * INPUT_CHANNELS) * DATA_WIDTH
#define OUTPUT_SIZE (OUTPUT_HEIGHT * OUTPUT_WIDTH * INPUT_CHANNELS) * DATA_WIDTH

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Buffers */
static float *p_inp    = NULL;
static float *p_out    = NULL;
static void *p_scratch = NULL;

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

    // Interrogate DSP for necessary scratch size and allocate
    int scratch_size = xa_nn_maxpool_getsize(INPUT_CHANNELS, PRECISION, PRECISION, INPUT_HEIGHT, INPUT_WIDTH,
                                             KERNEL_HEIGHT, KERNEL_WIDTH, X_STRIDE, Y_STRIDE, X_PADDING, Y_PADDING,
                                             OUTPUT_HEIGHT, OUTPUT_WIDTH, 1, 1);
    p_scratch        = xa_nn_malloc(scratch_size);
}

static void reset_buffers()
{
    memset(p_inp, 0, INPUT_SIZE);
    memset(p_out, 0, OUTPUT_SIZE);
}

static void fill_buffers()
{
    // Get input/output from test files
    void *INPUT_START = (float *)maxpool_in;

    // Fill buffers
    memcpy(p_inp, INPUT_START, INPUT_SIZE);
}

static void free_buffers()
{
    SDK_Free(p_inp);
    SDK_Free(p_out);
    xa_nn_free(p_scratch);
}

static void process_sync()
{

#if NN_ENABLE_xa_nn_maxpool_f32 == 1
    xa_nn_maxpool_f32(p_out, // OUTPUT
                      p_inp, // INPUT
                      INPUT_HEIGHT, INPUT_WIDTH, INPUT_CHANNELS, KERNEL_HEIGHT, KERNEL_WIDTH, X_STRIDE, Y_STRIDE,
                      X_PADDING, Y_PADDING, OUTPUT_HEIGHT, OUTPUT_WIDTH, 1, 1, p_scratch);

#endif
}

static void process_async()
{

#if NN_ENABLE_xa_nn_maxpool_f32 == 1
    xa_nn_maxpool_f32_async(nn_cb, queue,
                            p_out, // OUTPUT
                            p_inp, // INPUT
                            INPUT_HEIGHT, INPUT_WIDTH, INPUT_CHANNELS, KERNEL_HEIGHT, KERNEL_WIDTH, X_STRIDE, Y_STRIDE,
                            X_PADDING, Y_PADDING, OUTPUT_HEIGHT, OUTPUT_WIDTH, 1, 1, p_scratch);
    xQueueReceive(queue, &msg, portMAX_DELAY);

#endif
}

static int check_output()
{
    int i;
    const void *OUTPUT_START = maxpool_out;

    for (i = 0; i < OUTPUT_SIZE; i++)
    {
        if (((unsigned char *)p_out)[i] != ((unsigned char *)OUTPUT_START)[i])
        {
            PRINTF("%s failed at index %d: %x != %x\r\n", UNIT_TEST_NAME, i, ((unsigned char *)p_out)[i],
                   ((unsigned char *)OUTPUT_START)[i]);
            return 0;
        }
    }

    PRINTF("%s output check succeeded\r\n", UNIT_TEST_NAME);
    return 1;
}

void nn_maxpool_unit_test(int mode)
{
    PRINTF("Running %s %s\r\n", UNIT_TEST_NAME, mode == UNIT_TEST_SYNC ? "SYNC" : "ASYNC");

#if NN_ENABLE_xa_nn_maxpool_f32 == 1
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
    PRINTF("%s xa_nn_maxpool_f32%s not supported\r\n", UNIT_TEST_NAME, mode == UNIT_TEST_ASYNC ? "_async" : "");
#endif
}
