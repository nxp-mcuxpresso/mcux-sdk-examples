/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MAIN_H__
#define __MAIN_H__

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#ifdef SD_ENABLED
#include "ff.h"
#endif

#include <stdbool.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef struct _app_handle
{
    TaskHandle_t shell_task_handle;

#ifdef SD_ENABLED
    /* SD card management */
    SemaphoreHandle_t sdcardSem;
    volatile bool sdcardInserted;
    volatile bool sdcardInsertedPrev;
    FATFS fileSystem;
    FIL fileObject;
#endif
} app_handle_t;

#ifdef SD_ENABLED
bool SDCARD_inserted(void);
#endif

#endif /* __MAIN_H__ */
