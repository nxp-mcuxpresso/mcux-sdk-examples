/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MAIN_CM33_H__
#define __MAIN_CM33_H__

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "rpmsg_lite.h"
#include "rpmsg_ns.h"
#include "rpmsg_queue.h"
#include "ff.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Data bytes to send for codec initialization for file playback. */
#define FILE_PLAYBACK_INITIAL_READ_SIZE (16 * 1024)
/* Data bytes to send during playback */
#define FILE_PLAYBACK_READ_SIZE (4 * 1024)

typedef struct _app_handle
{
    TaskHandle_t shell_task_handle;
    TaskHandle_t ipc_task_handle;

    /* SD card management */
    SemaphoreHandle_t sdcardSem;
    volatile bool sdcardInserted;
    volatile bool sdcardInsertedPrev;
    FATFS fileSystem;
    FIL fileObject;
} app_handle_t;

#endif /* __MAIN_CM33_H__ */
