/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __APP_PBAP_PCE_H__
#define __APP_PBAP_PCE_H__

#define CURRENT_PATH_MAX_LEN 26
typedef struct app_pbap_pce_
{
    struct bt_pbap_pce *pbap_pceHandle;
    struct bt_conn *conn;
    uint8_t peer_bd_addr[6];
    uint16_t goep_version;
    uint16_t pbap_version;
    uint32_t peer_feature;
    uint32_t loacal_feature;
    uint8_t supported_repositories;
    char currentpath[CURRENT_PATH_MAX_LEN];
    bool lcl_srmp_wait; /* local srmp */
    bool rem_srmp_wait; /* remote srmp */
    int8_t num_srmp_wait;
} app_pbap_pce_t;

void pbap_pce_task(void *pvParameters);
int app_pbap_connect();
int app_pull_phonebook(char *name);
int app_set_phonebook_path(char *name);
int app_pull_vcard_listing(char *name);
int app_pull_vcard_entry(char *name);

#endif /* __APP_PBAP_PCE_H__ */
