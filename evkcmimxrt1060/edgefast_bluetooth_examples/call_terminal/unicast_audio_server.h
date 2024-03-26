/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __UNICAST_AUDIO_SERVER_H__
#define __UNICAST_AUDIO_SERVER_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define AVAILABLE_SINK_CONTEXT  (BT_AUDIO_CONTEXT_TYPE_UNSPECIFIED | \
                 BT_AUDIO_CONTEXT_TYPE_CONVERSATIONAL | \
                 BT_AUDIO_CONTEXT_TYPE_MEDIA | \
                 BT_AUDIO_CONTEXT_TYPE_RINGTONE)

#define AVAILABLE_SOURCE_CONTEXT (BT_AUDIO_CONTEXT_TYPE_UNSPECIFIED | \
                  BT_AUDIO_CONTEXT_TYPE_CONVERSATIONAL)

/*******************************************************************************
 * API
 ******************************************************************************/

int unicast_audio_server_init(void);

#endif /* __UNICAST_AUDIO_SERVER_H__ */