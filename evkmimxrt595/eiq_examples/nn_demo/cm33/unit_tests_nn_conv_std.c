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

#include "conv_std_in.h"
#include "conv_std_out.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define UNIT_TEST_NAME  "CONV STD"
#define INPUT_WIDTH     40
#define INPUT_HEIGHT    32
#define INPUT_CHANNELS  32
#define KERNEL_WIDTH    5
#define KERNEL_HEIGHT   7
#define OUTPUT_CHANNELS 24
#define X_STRIDE        1
#define Y_STRIDE        1
#define X_PADDING       0
#define Y_PADDING       0
#define OUTPUT_FORMAT   0
#define INPUT_PRECISION 16
#define OUTPUT_WIDTH    36
#define OUTPUT_HEIGHT   26
#define BIAS_SHIFT      0
#define ACC_SHIFT       0

#define INPUT_SIZE (2 * INPUT_HEIGHT * INPUT_WIDTH * INPUT_CHANNELS)
// For kernel, floating point precision, input channels must be multiple of 4
#define INPUT_CHANNELS_PAD ((INPUT_CHANNELS + 4 - 1) & ~(4 - 1))
#define KERNEL_SIZE        (1 * KERNEL_HEIGHT * KERNEL_WIDTH * OUTPUT_CHANNELS * INPUT_CHANNELS_PAD)
#define BIAS_SIZE          (2 * OUTPUT_CHANNELS)
#define OUTPUT_SIZE        (2 * OUTPUT_HEIGHT * OUTPUT_WIDTH * OUTPUT_CHANNELS)

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Buffers */
static void *p_inp     = NULL;
static void *p_kernel  = NULL;
static void *p_bias    = NULL;
static void *p_out     = NULL;
static void *p_scratch = NULL;

/* Variables used for async */
static QueueHandle_t queue;
static srtm_message msg;

/*******************************************************************************
 * Code
 ******************************************************************************/

static void alloc_buffers()
{
    p_inp    = SDK_Malloc(INPUT_SIZE, 8);
    p_kernel = SDK_Malloc(KERNEL_SIZE, 8);
    p_bias   = SDK_Malloc(BIAS_SIZE, 8);
    p_out    = SDK_Malloc(OUTPUT_SIZE, 8);

    // Interrogate DSP for necessary scratch size and allocate
    int scratch_size = xa_nn_conv2d_std_getsize(INPUT_HEIGHT, INPUT_CHANNELS, KERNEL_HEIGHT, KERNEL_WIDTH, Y_STRIDE,
                                                Y_PADDING, OUTPUT_HEIGHT, OUTPUT_CHANNELS, INPUT_PRECISION);
    p_scratch        = xa_nn_malloc(scratch_size);
}

static void reset_buffers()
{
    memset(p_inp, 0, INPUT_SIZE);
    memset(p_kernel, 0, KERNEL_SIZE);
    memset(p_bias, 0, BIAS_SIZE);
    memset(p_out, 0, OUTPUT_SIZE);
}

static void fill_buffers()
{
    // Get input/output from test files
    void *INPUT_START  = (void *)conv_std_in;
    void *KERNEL_START = (void *)((unsigned int)INPUT_START + INPUT_SIZE);
    void *BIAS_START   = (void *)((unsigned int)KERNEL_START + KERNEL_SIZE);

    // Fill buffers
    memcpy(p_inp, INPUT_START, INPUT_SIZE);
    memcpy(p_kernel, KERNEL_START, KERNEL_SIZE);
    memcpy(p_bias, BIAS_START, BIAS_SIZE);
}

static void free_buffers()
{
    SDK_Free(p_inp);
    SDK_Free(p_kernel);
    SDK_Free(p_bias);
    SDK_Free(p_out);
    xa_nn_free(p_scratch);
}

static void process_sync()
{
    xa_nn_conv2d_std_8x16(p_out, p_inp, p_kernel, p_bias, INPUT_HEIGHT, INPUT_WIDTH, INPUT_CHANNELS, KERNEL_HEIGHT,
                          KERNEL_WIDTH, OUTPUT_CHANNELS, X_STRIDE, Y_STRIDE, X_PADDING, Y_PADDING, OUTPUT_HEIGHT,
                          OUTPUT_WIDTH, BIAS_SHIFT, ACC_SHIFT, OUTPUT_FORMAT, p_scratch);
}

static void process_async()
{
    xa_nn_conv2d_std_8x16_async(nn_cb, queue, p_out, p_inp, p_kernel, p_bias, INPUT_HEIGHT, INPUT_WIDTH, INPUT_CHANNELS,
                                KERNEL_HEIGHT, KERNEL_WIDTH, OUTPUT_CHANNELS, X_STRIDE, Y_STRIDE, X_PADDING, Y_PADDING,
                                OUTPUT_HEIGHT, OUTPUT_WIDTH, BIAS_SHIFT, ACC_SHIFT, OUTPUT_FORMAT, p_scratch);
    xQueueReceive(queue, &msg, portMAX_DELAY);
}

static int check_output()
{
    int i;
    const void *OUTPUT_START = conv_std_out;

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

void nn_conv_std_unit_test(int mode)
{
    int i;
    volatile unsigned long tic, toc;
    unsigned long total_cycles = 0;
    float total_ms;

    PRINTF("Running %s %s\r\n", UNIT_TEST_NAME, mode == UNIT_TEST_SYNC ? "SYNC" : "ASYNC");

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

    PRINTF("Avg Inference cycles: %u time: %d ms\r\n", total_cycles, (int) total_ms);
    PRINTF("Throughput: %d fps\r\n",  (int) (1000 / total_ms));

    if (mode == UNIT_TEST_ASYNC)
    {
        vQueueUnregisterQueue(queue);
        vQueueDelete(queue);
    }
    free_buffers();
}

