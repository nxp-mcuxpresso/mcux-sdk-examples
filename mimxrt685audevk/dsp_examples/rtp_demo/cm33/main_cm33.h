/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MAIN_CM33_H__
#define __MAIN_CM33_H__

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef struct _app_handle
{
    QueueHandle_t buffer_queue;
    TaskHandle_t rtp_receiver_task_handle;
} app_handle_t;

#endif /* __MAIN_CM33_H__ */
