/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __APPL_PERIPHERAL_HFP_AG_MAIN_H__
#define __APPL_PERIPHERAL_HFP_AG_MAIN_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/

void peripheral_hfp_ag_task(void *param);
int app_hfp_ag_disconnect(void);
int app_hfp_ag_discover(struct bt_conn *conn, uint8_t channel);
int app_hfp_ag_start_incoming_call(void);
int app_hfp_ag_accept_incoming_call(void);
int app_hfp_ag_stop_incoming_call(void);
int app_hfp_ag_start_twc_incoming_call(void);
void app_hfp_ag_set_phnum_tag(char *name);
void app_hfp_ag_volume_update(hf_volume_type_t type, int volume);
int app_hfp_ag_codec_select(uint8_t codec);
void app_hfp_ag_open_audio();
void app_hfp_ag_close_audio();
#endif /* __APPL_PERIPHERAL_HFP_AG_MAIN_H__ */
