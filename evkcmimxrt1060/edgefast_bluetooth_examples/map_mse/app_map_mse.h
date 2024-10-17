/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __APP_MAP_MSE_H__
#define __APP_MAP_MSE_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/

#define MAP_MSE_MAX_PATH_LEN (45U)

enum map_cmd_id
{
    CMD_ID_NONE,
    CMD_ID_GET_FOLDER_LISTING,
    CMD_ID_SET_FOLDER,
    CMD_ID_GET_MSG_LISTING,
    CMD_ID_GET_MSG,
    CMD_ID_SET_MSG_STATUS,
    CMD_ID_PUSH_MSG,
    CMD_ID_SET_NTF_REG,
    CMD_ID_UPDATE_INBOX,
    CMD_ID_GET_MAS_INST_INFO,
    CMD_ID_SET_OWNER_STATUS,
    CMD_ID_GET_OWNER_STATUS,
    CMD_ID_GET_CONVO_LISTING,
    CMD_ID_SET_NTF_FILTER,
};

struct app_map_mse_instance
{
    struct bt_conn *acl_conn;
    struct bt_map_mse_mas *mse_mas;
    struct bt_map_mse_mns *mse_mns;
    uint64_t msg_handle;
    uint16_t max_pkt_len; /* range from opcode to the end of packet */
    uint16_t mns_max_pkt_len; /* range from opcode to the end of packet */
    uint16_t tx_cnt;
    uint16_t mns_tx_cnt;
    char path[MAP_MSE_MAX_PATH_LEN];
    enum map_cmd_id cmd_id;
};

void map_mse_task(void *pvParameters);

#endif /* __APP_MAP_MCE_H__ */
