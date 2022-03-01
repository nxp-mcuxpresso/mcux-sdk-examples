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
#include "srtm_config.h"

void nn_cb(void *params, srtm_message *msg)
{
    QueueHandle_t queue = (QueueHandle_t)params;
    xQueueSend(queue, msg, portMAX_DELAY);
}
