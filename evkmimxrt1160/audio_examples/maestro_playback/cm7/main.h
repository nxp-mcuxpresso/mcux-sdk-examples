/*
 * Copyright 2020-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MAIN_H__
#define __MAIN_H__

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "ff.h"

#include <stdbool.h>
#include <fsl_common.h>
#include <fsl_debug_console.h>

#include <app_data.h>
#include <eap_att.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef struct _app_handle
{
    TaskHandle_t shell_task_handle;
    TaskHandle_t gpio_task_handle;
    TaskHandle_t app_task_handle;
    TaskHandle_t sdcard_task_handle;

    /* SD card management */
    SemaphoreHandle_t sdcardSem;
    volatile bool sdcardInserted;
    volatile bool sdcardInsertedPrev;
    FATFS fileSystem;
    FIL fileObject;
} app_handle_t;

status_t list_files(bool autoInput);

#endif /* __MAIN_H__ */
