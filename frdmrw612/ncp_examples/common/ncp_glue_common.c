/* @file ncp_glue_common.c
 *
 *  @brief This file contains declaration of the API functions.
 *
 *  Copyright 2008-2023 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#include "ncp_cmd_common.h"

extern struct cmd_subclass_t cmd_subclass_wlan[12];
#if CONFIG_NCP_BLE
extern struct cmd_subclass_t cmd_subclass_ble[8];
#endif
#if CONFIG_NCP_OT
extern struct cmd_subclass_t cmd_subclass_15D4[2];
#endif
extern struct cmd_subclass_t cmd_subclass_system[2];
extern struct cmd_t error_ack_cmd;

#define CMD_SUBCLASS_WLAN_LEN   (sizeof(cmd_subclass_wlan) / sizeof(struct cmd_subclass_t))
#if CONFIG_NCP_BLE
#define CMD_SUBCLASS_BLE_LEN    (sizeof(cmd_subclass_ble) / sizeof(struct cmd_subclass_t))
#endif
#define CMD_SUBCLASS_15D4_LEN   (sizeof(cmd_subclass_15D4) / sizeof(struct cmd_subclass_t))
#define CMD_SUBCLASS_MATTER_LEN (sizeof(cmd_subclass_matter) / sizeof(struct cmd_subclass_t))
#define CMD_SUBCLASS_SYSTEM_LEN (sizeof(cmd_subclass_system) / sizeof(struct cmd_subclass_t))

static bool ncp_cmd_initialized = false;

__WEAK struct cmd_subclass_t cmd_subclass_15D4[] = {
    {NCP_CMD_INVALID, NULL},
};

struct cmd_subclass_t cmd_subclass_matter[] = {
    {NCP_CMD_INVALID, NULL},
};

struct cmd_class_t cmd_class_list[] = {
    {NCP_CMD_WLAN, cmd_subclass_wlan, CMD_SUBCLASS_WLAN_LEN},
#if CONFIG_NCP_BLE
    {NCP_CMD_BLE, cmd_subclass_ble, CMD_SUBCLASS_BLE_LEN},
#endif
    {NCP_CMD_15D4, cmd_subclass_15D4, CMD_SUBCLASS_15D4_LEN},
    {NCP_CMD_MATTER, cmd_subclass_matter, CMD_SUBCLASS_MATTER_LEN},
    {NCP_CMD_SYSTEM, cmd_subclass_system, CMD_SUBCLASS_SYSTEM_LEN},
    {NCP_CMD_INVALID, NULL, 0},
};

int ncp_register_class(struct cmd_class_t *cmd_class)
{
    int cmd_idx;
    int subclass_idx;
    struct cmd_subclass_t *cmd_subclass = NULL;
    struct cmd_t *cmd = NULL;

    if (!cmd_class || cmd_class->subclass_len > NCP_HASH_TABLE_SIZE)
    {
        return -1;
    }

    memset(cmd_class->hash, NCP_HASH_INVALID_KEY, NCP_HASH_TABLE_SIZE);

    for (subclass_idx = 0; subclass_idx < cmd_class->subclass_len; subclass_idx++)
    {
        cmd_subclass = &cmd_class->cmd_subclass[subclass_idx];

        if (cmd_subclass->cmd_subclass == NCP_CMD_INVALID)
        {
            break;
        }

        cmd_class->hash[GET_CMD_SUBCLASS(cmd_subclass->cmd_subclass)] = subclass_idx;

        memset(cmd_subclass->hash, NCP_HASH_INVALID_KEY, NCP_HASH_TABLE_SIZE);

        for (cmd_idx = 0; cmd_idx < NCP_HASH_TABLE_SIZE; cmd_idx++)
        {
            cmd = &cmd_subclass->cmd[cmd_idx];

            if (cmd->cmd == NCP_CMD_INVALID)
            {
                break;
            }

            cmd_subclass->hash[GET_CMD_ID(cmd->cmd)] = cmd_idx;
        }
    }
    return 0;
}

int ncp_cmd_list_init(void)
{
    int i;
    int ret;
    struct cmd_class_t *cmd_class = NULL;

    if (ncp_cmd_initialized == true)
    {
        return 0;
    }

    for (i = 0; i < ARRAY_SIZE(cmd_class_list); i++)
    {
        cmd_class = &cmd_class_list[i];

        if (cmd_class->cmd_class == NCP_CMD_INVALID)
        {
            break;
        }

        ret = ncp_register_class(cmd_class);
        if (ret != 0)
        {
            return -1;
        }
    }

    ncp_cmd_initialized = true;

    return 0;
}

struct cmd_t *lookup_class(uint32_t cmd_class, uint32_t cmd_subclass, uint32_t cmd_id)
{
    int i;
    struct cmd_class_t *pclass;
    struct cmd_subclass_t *psubclass;
    struct cmd_t *cmd = NULL;

    if (cmd_subclass >= NCP_HASH_TABLE_SIZE || cmd_id >= NCP_HASH_TABLE_SIZE)
    {
        return NULL;
    }

    for (i = 0; i < (sizeof(cmd_class_list) / sizeof(struct cmd_class_t)); i++)
    {
        pclass = &cmd_class_list[i];

        /* when class id valid, subclass ptr should never be NULL */
        if (GET_CMD_CLASS(pclass->cmd_class) == cmd_class)
        {
            /* lookup hash table */
            if (pclass->hash[cmd_subclass] != NCP_HASH_INVALID_KEY)
            {
                psubclass = &pclass->cmd_subclass[pclass->hash[cmd_subclass]];
                if (psubclass->hash[cmd_id] != NCP_HASH_INVALID_KEY)
                {
                    cmd = &psubclass->cmd[psubclass->hash[cmd_id]];
                }
            }
            break;
        }

        if((pclass->cmd_class == NCP_CMD_INVALID) && (pclass->cmd_subclass == NULL))
        {
            break;
        }
    }

    if (cmd == NULL)
        return &error_ack_cmd;
    else
        return cmd;
}

