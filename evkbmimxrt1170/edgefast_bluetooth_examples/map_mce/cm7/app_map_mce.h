/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __APP_MAP_MCE_H__
#define __APP_MAP_MCE_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/

enum app_map_state
{
    MAP_NOT_STARTED = 0,

    /* Browsing Feature */
    GET_FOLDER_LISTING_ROOT,
    WAIT_GET_FOLDER_LISTING_ROOT,
    SET_FOLDER_TELECOM,
    WAIT_SET_FOLDER_TELECOM,
    SET_FOLDER_MSG,
    WAIT_SET_FOLDER_MSG,
    SET_FOLDER_INBOX,
    WAIT_SET_FOLDER_INBOX,
    UPDATE_INBOX,
    WAIT_UPDATE_INBOX,
    GET_MSG_LISTING,
    WAIT_GET_MSG_LISTING,
    GET_MSG,
    WAIT_GET_MSG,
    SET_MSG_STATUS,
    WAIT_SET_MSG_STATUS,
    GET_CONVO_LISTING, /* Bit20 - Conversation listing */
    WAIT_GET_CONVO_LISTING,

    /* Instance Information Feature */
    GET_MAS_INST_INFO,
    WAIT_GET_MAS_INST_INFO,

    /* Notification Feature */
    /* SEND_EVENT, */
    SET_NTF_FILTER, /* Bit17 - Notification Filtering */
    WAIT_SET_NTF_FILTER,

    /* Notification Registration Feature */
    SET_NTF_REG_ON,
    WAIT_SET_NTF_REG_ON,
    SET_NTF_REG_OFF,
    WAIT_SET_NTF_REG_OFF,

    /* Uploading Feature */
    GET_OWNER_STATUS, /* Bit21 - Owner Status */
    WAIT_GET_OWNER_STATUS,
    SET_OWNER_STATUS, /* Bit21 - Owner Status */
    WAIT_SET_OWNER_STATUS,
    SET_FOLDER_PARENT,
    WAIT_SET_FOLDER_PARENT,
    SET_FOLDER_OUTBOX,
    WAIT_SET_FOLDER_OUTBOX,
    PUSH_MSG,
    WAIT_PUSH_MSG,

    MCE_MAS_DISCONNECT,
    WAIT_MCE_MAS_DISCONNECT,

#if 0
    /* Delete Feature */
    SET_MSG_STATUS,
    WAIT_SET_MSG_STATUS,
    /* Message Forwarding Feature */
    PUSH_MSG,
    WAIT_PUSH_MSG,
#endif
};

struct app_map_mce_instance
{
    struct bt_conn *acl_conn;
    struct bt_map_mce_mas *mce_mas;
    struct bt_map_mce_mns *mce_mns;
    uint32_t supported_features;
    char msg_handle[BT_MAP_MSG_HANDLE_SIZE / 2U];
    uint16_t max_pkt_len; /* range from opcode to the end of packet */
    uint16_t mns_max_pkt_len; /* range from opcode to the end of packet */
    uint16_t tx_cnt;
    uint16_t goep_version;
    uint16_t map_version;
    uint16_t psm;
    uint8_t scn;
    uint8_t mas_instance_id;
    uint8_t state;
    uint8_t num_srmp_wait;
};

void map_mce_task(void *param);
void app_mce_connect(void);
void app_mce_disconnect(void);
void app_mce_abort(void);
void app_mce_mns_disconnect(void);
void app_get_folder_listing(uint8_t wait);
void app_set_folder(char *name);
void app_get_msg_listing(char *name, uint16_t max_list_cnt, uint8_t wait);
void app_get_msg(char *name, bool attachment, bool charset, uint8_t wait);
void app_set_msg_status(char *name, uint8_t status_ind, uint8_t status_val);
void app_push_msg(char *name, bool charset);
void app_set_ntf_reg(bool ntf_status);
void app_update_inbox(void);
void app_get_mas_inst_info(uint8_t mas_inst_id, uint8_t wait);
void app_set_owner_status(uint8_t chat_state);
void app_get_owner_status(uint8_t wait);
void app_get_convo_listing(uint16_t max_list_cnt, uint8_t wait);
void app_set_ntf_filter(uint32_t ntf_filter_mask);

#endif /* __APP_MAP_MCE_H__ */
