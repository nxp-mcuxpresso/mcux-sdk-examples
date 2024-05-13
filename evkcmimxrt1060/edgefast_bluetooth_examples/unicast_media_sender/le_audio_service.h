/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __LE_AUDIO_SERVICE_H_
#define __LE_AUDIO_SERVICE_H_

#include <bluetooth/conn.h>

typedef void (*vcs_client_discover_callback_t)(struct bt_conn *conn, int err);
typedef void (*vcs_server_vol_callback_t)(uint8_t volume, uint8_t mute);

int le_audio_vcs_server_init(vcs_server_vol_callback_t callback);
int le_audio_vcs_client_init(vcs_client_discover_callback_t callback);
int le_audio_vcs_discover(struct bt_conn *conn, uint8_t channel);
int le_audio_vcs_vol_set(uint8_t volume);
int le_audio_vcs_vol_up(void);
int le_audio_vcs_vol_down(void);
int le_audio_vcs_vol_mute(void);
int le_audio_vcs_vol_unmute(void);

typedef void (*mcs_server_discover_cb_t)(struct bt_conn *conn);

#define MCS_SERVER_STATE_PLAYING 0x01
#define MCS_SERVER_STATE_PAUSED  0x02

typedef void (*mcs_server_state_cb_t)(int state);

int le_audio_mcs_server_init(mcs_server_state_cb_t callback);
int le_audio_mcs_client_init(mcs_server_discover_cb_t callback);
int le_audio_mcs_discover(struct bt_conn *conn);
int le_audio_mcs_play(void);
int le_audio_mcs_pause(void);

#endif /* __LE_AUDIO_SERVICE_H_ */