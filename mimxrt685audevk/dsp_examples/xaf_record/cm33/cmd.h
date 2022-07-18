/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*${header:start}*/
#include "dsp_config.h"
#include "main_cm33.h"
#include "srtm_config.h"
/*${header:end}*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
typedef void handleShellMessageCallback_t(srtm_message *msg, void *arg);
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*${prototype:start}*/
/*!
 * @brief Common function for handle message from DSP.
 *
 * Can be used on both side both sides. Primary core or DSP core side
 * (DSP core side in case single DSP core only execution).
 * Parse received message and show output/result.
 *
 * @param[in] msg Message structure where cmd data are serialized.
 */
void handleDSPMessage(app_handle_t *app, srtm_message *msg);

/*!
 * @brief Common function for getting user input using shell console.
 *
 * Can be used on both side both sides. Primary core or DSP core side
 * (DSP core side in case single DSP core only execution).
 * Initializing and starting shell console. Each shell command is serializing
 * srtm command and call handleShellMessageCallback.
 *
 * @param[in] handleShellMessageCallback Callback to function which should
 * handle serialized message.
 * @param[in] arg Data to pass to callback handler.
 */
void shellCmd(handleShellMessageCallback_t *handleShellMessageCallback, void *arg);

/*!
 * @brief Common function for muting the right channel.
 *
 * Can be used during mono playback
 *
 * @param[in] mute Mute right channel
 */
void BOARD_MuteRightChannel(bool mute);
/*${prototype:end}*/
