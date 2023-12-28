/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_DATA_H_
#define _APP_DATA_H_

#include <stdint.h>
#include <streamer_element_properties.h>

#ifndef MAX_FILES_LIST
#define MAX_FILES_LIST 256
#endif

#ifndef MAX_FILE_NAME_LENGTH
#define MAX_FILE_NAME_LENGTH 64
#endif

typedef enum _app_status
{
    kAppIdle    = 0, /*!< application is idle */
    kAppRunning = 1, /*!< application is running */
    kAppPaused  = 2, /*!< application is running with paused playback */
    kAppError   = -1 /*!< signals any kind of error, read error message and restart*/
} app_status_t;

typedef enum _app_error_code
{
    kAppCodeOk    = 0, /*!< No problem */
    kAppCodeError = 1, /*!< Status for generic error */
} app_error_code_t;

typedef struct _app_data
{
    int logEnabled;
    app_status_t status;
    app_error_code_t lastError;
    int trackTotal;                   // audio track duration in [ms]
    int trackCurrent;                 // audio track actual time in [ms]
    char input[MAX_FILE_NAME_LENGTH]; /* Set default input on startup or retrieve current value modified in ATT UI. */
    char availableInputs[MAX_FILES_LIST][MAX_FILE_NAME_LENGTH]; /* Fill available inputs data if wanted. */
    int volume;                                                 // Volume in range 0-100%, 0 is muted
    int32_t seek_time;                                          // Seek time
    ext_proc_args proc_args;
#ifdef MULTICHANNEL_EXAMPLE
    uint8_t num_channels; // number of channels set with cli
#endif
} app_data_t;

app_data_t *get_app_data();

#endif
