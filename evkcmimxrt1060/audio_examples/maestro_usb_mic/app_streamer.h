/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _APP_STREAMER_H_
#define _APP_STREAMER_H_

/* streamer library includes. */
#include "streamer_api.h"
#include "streamer_element_properties.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Maestro network audio buffer size */
#define AUDIO_BUFFER_SIZE (30 * 1024)

/*! @brief Maestro Streamer interface structure */
typedef struct _streamer_handle_t
{
    STREAMER_T *streamer;
    volatile bool audioPlaying;
} streamer_handle_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initialize the Maestro streamer interface
 *
 * This function initializes the Maestro streamer library, and initializes the PCM
 * output interface. This function should be called before creating a streamer
 * handle.
 */
void STREAMER_Init(void);

/*!
 * @brief Create an Maestro streamer interface handle
 *
 * This function creates an Maestro streamer interface and starts a task for
 * handling media playback, and a task for sending status and error messages
 * back to the application.
 *
 * @param handle Pointer to input handle
 * @return kStatus_Success on success, otherwise an error.
 */
status_t STREAMER_mic_Create(streamer_handle_t *handle);

/*!
 * @brief Destroy an Maestro streamer interface handle
 *
 * This function destroys an Maestro streamer interface.
 *
 * @param handle Pointer to input handle
 */
void STREAMER_Destroy(streamer_handle_t *handle);

/*!
 * @brief Start audio recording for the streamer interface
 *
 * This function puts the streamer in a running state.
 *
 * @param handle Pointer to input handle
 */
void STREAMER_Start(streamer_handle_t *handle);

/*!
 * @brief Stop audio recording for the streamer interface
 *
 * This function puts the streamer in a stopped state.
 *
 * @param handle Pointer to input handle
 */
void STREAMER_Stop(streamer_handle_t *handle);

/*!
 * @brief Check if streamer interface is playing
 *
 * This function returns true/false of the playing state for the interface
 *
 * @param handle Pointer to input handle
 * @return true if playing, false if not
 */
bool STREAMER_IsPlaying(streamer_handle_t *handle);

#if defined(__cplusplus)
}
#endif

#endif
