/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef void (*unicast_client_discover_done_callback_t)(struct bt_conn *conn, int err);

/*******************************************************************************
 * API
 ******************************************************************************/

int unicast_audio_client_init(unicast_client_discover_done_callback_t callback);

int unicast_client_configure_streams(void);

int unicast_client_set_stream_qos(void);

int unicast_client_create_group(void);

int unicast_client_delete_group(void);

int unicast_client_enable_stream_unidirectional(uint8_t is_tx, uint16_t context);

int unicast_client_enable_streams(uint16_t tx_context, uint16_t rx_context);

int unicast_client_disable_stream_unidirectional(uint8_t is_tx);

int unicast_client_disable_streams(void);

int unicast_client_start_stream_unidirectional(uint8_t is_tx);

int unicast_client_start_streams(void);

int unicast_client_stop_streams(void);

int unicast_client_stop_stream_unidirectional(uint8_t is_tx);

int unicast_client_release_streams(void);

int unicast_client_start_ringtone(const uint8_t *pcm, size_t pcm_length);

int unicast_client_stop_ringtone(void);

int unicast_client_start_voice(void);

int unicast_client_stop_voice(void);

int unicast_client_hold(void);

int unicast_client_retrieve(void);

int unicast_client_metadata(uint16_t tx_context, uint16_t rx_context);
