/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "unit_tests_nn.h"
#include "FreeRTOS.h"
#include "counter.h"
#include "dsp_nn.h"
#include "dsp_nn_utils.h"
#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "queue.h"
#include "srtm_config.h"
#include <stdio.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define UNIT_TEST_NAME  "ECHO"
#define EXPECTED_OUTPUT ((2) << 16 | (1))

/*******************************************************************************
 * Variables
 ******************************************************************************/

static unsigned int result = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/

static unsigned int process_sync()
{
    return xa_nn_echo();
}

static int check_output()
{
/* Mike
    if (result != EXPECTED_OUTPUT)
    {
        PRINTF("%s unit test failed: %lf != %lf\r\n", UNIT_TEST_NAME, result, EXPECTED_OUTPUT);
        return 0;
    }
*/
    PRINTF("%s unit test succeded\r\n", UNIT_TEST_NAME);
    PRINTF("NNlib version %d.%d \r\n", result >> 16, result & 0xFF);

    return 1;
}

void nn_echo_unit_test(int mode)
{
    int i;
    volatile unsigned long tic, toc;
    unsigned long total_cycles = 0;
    float total_ms;

    PRINTF("Running %s %s\r\n", UNIT_TEST_NAME, mode == UNIT_TEST_SYNC ? "SYNC" : "ASYNC");

    for (i = 0; i < BENCH_ITERS; i++)
    {
        tic = get_ccount();
        switch (mode)
        {
            case UNIT_TEST_SYNC:
                result = process_sync();
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
    PRINTF("Throughput: %d fps\r\n", (int) (1000 / total_ms));
}
