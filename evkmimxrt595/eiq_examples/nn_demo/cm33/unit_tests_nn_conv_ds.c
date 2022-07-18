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

#include "conv_ds_in.h"
#include "conv_ds_out.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define UNIT_TEST_NAME      "CONV DS"
#define INPUT_WIDTH         40
#define INPUT_HEIGHT        32
#define INPUT_CHANNELS      32
#define KERNEL_WIDTH        5
#define KERNEL_HEIGHT       7
#define OUT_CHANNELS        24
#define X_STRIDE            1
#define Y_STRIDE            1
#define X_PADDING           0
#define Y_PADDING           0
#define OUTPUT_WIDTH        36
#define OUTPUT_HEIGHT       26
#define CHANNELS_MULTIPLIER 1
#define PRECISION           -1
#define DATA_WIDTH          4

#define INPUT_SIZE        (INPUT_CHANNELS * INPUT_HEIGHT * INPUT_WIDTH) * DATA_WIDTH
#define KERNEL_DEPTH_SIZE (CHANNELS_MULTIPLIER * INPUT_CHANNELS * KERNEL_HEIGHT * (KERNEL_WIDTH + 3) & ~3) * DATA_WIDTH
#define BIAS_DEPTH_SIZE   (CHANNELS_MULTIPLIER * INPUT_CHANNELS) * DATA_WIDTH
#define KERNEL_POINT_SIZE (OUT_CHANNELS * CHANNELS_MULTIPLIER * INPUT_CHANNELS * 1 * 1 * OUT_CHANNELS) * DATA_WIDTH
#define BIAS_POINT_SIZE   (OUT_CHANNELS) * DATA_WIDTH
#define OUTPUT_SIZE       (OUT_CHANNELS * OUTPUT_HEIGHT * OUTPUT_WIDTH) * DATA_WIDTH
#define DW_SIZE           (CHANNELS_MULTIPLIER * INPUT_CHANNELS * OUTPUT_HEIGHT * OUTPUT_WIDTH) * DATA_WIDTH

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Buffers */
static float *p_inp          = NULL;
static float *p_kernel_depth = NULL;
static float *p_bias_depth   = NULL;
static float *p_kernel_point = NULL;
static float *p_bias_point   = NULL;
static float *p_out          = NULL;
static float *p_dw           = NULL;
static void *p_scratch       = NULL;

/* Variables used for async */
static QueueHandle_t queue;
static srtm_message msg;

/*******************************************************************************
 * Code
 ******************************************************************************/

static void alloc_buffers()
{
    p_inp          = SDK_Malloc(INPUT_SIZE, 8);
    p_kernel_depth = SDK_Malloc(KERNEL_DEPTH_SIZE, 8);
    p_bias_depth   = SDK_Malloc(BIAS_DEPTH_SIZE, 8);
    p_kernel_point = SDK_Malloc(KERNEL_POINT_SIZE, 8);
    p_bias_point   = SDK_Malloc(BIAS_POINT_SIZE, 8);
    p_out          = SDK_Malloc(OUTPUT_SIZE, 8);
    p_dw           = xa_nn_malloc(DW_SIZE);

    // Interrogate DSP for necessary scratch size and allocate
    int scratch_size = xa_nn_conv2d_depthwise_getsize(INPUT_HEIGHT, INPUT_WIDTH, INPUT_CHANNELS, KERNEL_HEIGHT,
                                                      KERNEL_WIDTH, CHANNELS_MULTIPLIER, X_STRIDE, Y_STRIDE, X_PADDING,
                                                      Y_PADDING, OUTPUT_HEIGHT, OUTPUT_WIDTH, PRECISION, 1);
    p_scratch        = xa_nn_malloc(scratch_size);
}

static void reset_buffers()
{
    memset(p_inp, 0, INPUT_SIZE);
    memset(p_kernel_depth, 0, KERNEL_DEPTH_SIZE);
    memset(p_bias_depth, 0, BIAS_DEPTH_SIZE);
    memset(p_kernel_point, 0, KERNEL_POINT_SIZE);
    memset(p_bias_point, 0, BIAS_POINT_SIZE);
    memset(p_out, 0, OUTPUT_SIZE);
//    memset(p_dw, 0, DW_SIZE);
}

static void fill_buffers()
{
    // Get input/output from test files
    const void *INPUT_START  = conv_ds_in;
    void *KERNEL_DEPTH_START = (void *)((unsigned int)INPUT_START + INPUT_SIZE);
    void *BIAS_DEPTH_START   = (void *)((unsigned int)KERNEL_DEPTH_START + KERNEL_DEPTH_SIZE);
    void *KERNEL_POINT_START = (void *)((unsigned int)BIAS_DEPTH_START + BIAS_DEPTH_SIZE);
    void *BIAS_POINT_START   = (void *)((unsigned int)KERNEL_POINT_START + KERNEL_POINT_SIZE);

    // Fill buffers
    memcpy(p_inp, INPUT_START, INPUT_SIZE);
    memcpy(p_kernel_depth, KERNEL_DEPTH_START, KERNEL_DEPTH_SIZE);
    memcpy(p_bias_depth, BIAS_DEPTH_START, BIAS_DEPTH_SIZE);
    memcpy(p_kernel_point, KERNEL_POINT_START, KERNEL_POINT_SIZE);
    memcpy(p_bias_point, BIAS_POINT_START, BIAS_POINT_SIZE);
}

static void free_buffers()
{
    SDK_Free(p_inp);
    SDK_Free(p_kernel_depth);
    SDK_Free(p_bias_depth);
    SDK_Free(p_kernel_point);
    SDK_Free(p_bias_point);
    SDK_Free(p_out);
    xa_nn_free(p_dw);
    xa_nn_free(p_scratch);
}

static void process_sync()
{

#if NN_ENABLE_xa_nn_conv2d_depthwise_f32 == 1
    xa_nn_conv2d_depthwise_f32(p_dw, p_kernel_depth, p_inp, p_bias_depth, INPUT_HEIGHT, INPUT_WIDTH, INPUT_CHANNELS,
                               KERNEL_HEIGHT, KERNEL_WIDTH, CHANNELS_MULTIPLIER, X_STRIDE, Y_STRIDE, X_PADDING,
                               Y_PADDING, OUTPUT_HEIGHT, OUTPUT_WIDTH, 1, 0, p_scratch);

    xa_nn_conv2d_pointwise_f32(p_out, p_kernel_point, p_dw, p_bias_point, OUTPUT_HEIGHT, OUTPUT_WIDTH,
                               INPUT_CHANNELS * CHANNELS_MULTIPLIER, OUT_CHANNELS, 1);
#endif
}

static void process_async()
{

#if NN_ENABLE_xa_nn_conv2d_depthwise_f32 == 1
    xa_nn_conv2d_depthwise_f32_async(nn_cb, queue, p_dw, p_kernel_depth, p_inp, p_bias_depth, INPUT_HEIGHT, INPUT_WIDTH,
                                     INPUT_CHANNELS, KERNEL_HEIGHT, KERNEL_WIDTH, CHANNELS_MULTIPLIER, X_STRIDE,
                                     Y_STRIDE, X_PADDING, Y_PADDING, OUTPUT_HEIGHT, OUTPUT_WIDTH, 1, 0, p_scratch);

    xa_nn_conv2d_pointwise_f32_async(nn_cb, queue, p_out, p_kernel_point, p_dw, p_bias_point, OUTPUT_HEIGHT,
                                     OUTPUT_WIDTH, INPUT_CHANNELS * CHANNELS_MULTIPLIER, OUT_CHANNELS, 1);

    xQueueReceive(queue, &msg, portMAX_DELAY);
    xQueueReceive(queue, &msg, portMAX_DELAY);
#endif
}

static int check_output()
{
    int i;
    const void *OUTPUT_START = conv_ds_out;

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

void nn_conv_ds_unit_test(int mode)
{
	   PRINTF("Running %s %s\r\n", UNIT_TEST_NAME, mode == UNIT_TEST_SYNC ? "SYNC" : "ASYNC");
#if NN_ENABLE_xa_nn_conv2d_depthwise_f32 == 1

	int i;
    volatile unsigned long tic, toc;
    unsigned long total_cycles = 0;
    float total_ms;

    if (mode == UNIT_TEST_ASYNC)
    {
        queue = xQueueCreate(2, sizeof(srtm_message));
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
    PRINTF("Throughput: %df fps\r\n", 1000 / total_ms);

    if (mode == UNIT_TEST_ASYNC)
    {
        vQueueUnregisterQueue(queue);
        vQueueDelete(queue);
    }
    free_buffers();
#else
    PRINTF("%s xa_nn_conv2d_depthwise_f32%s not supported\r\n", UNIT_TEST_NAME, mode == UNIT_TEST_ASYNC ? "_async" : "");
#endif
}
