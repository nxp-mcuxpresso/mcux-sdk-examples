/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ncp_debug.h"
#include "ncp_config.h"
#include "ncp_cmd_system.h"
#include "ncp_glue_system.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

extern uint8_t res_buf[NCP_BRIDGE_INBUF_SIZE];

/*******************************************************************************
 * Code
 ******************************************************************************/

NCPCmd_DS_COMMAND *ncp_sys_get_resp_buf()
{
    return (NCPCmd_DS_COMMAND *)(res_buf);
}

/** Prepare TLV command response */
int ncp_sys_prepare_status(uint32_t cmd, uint16_t result)
{
    NCPCmd_DS_COMMAND *cmd_res = ncp_sys_get_resp_buf();
    cmd_res->header.cmd        = cmd;

    cmd_res->header.size     = NCP_BRIDGE_CMD_HEADER_LEN;
    cmd_res->header.seqnum   = 0x00;
    cmd_res->header.result   = result;
    cmd_res->header.msg_type = NCP_BRIDGE_MSG_TYPE_RESP;
    return NCP_SUCCESS;
}

static int ncp_sys_set_config(void *tlv)
{
    NCP_CMD_SYSTEM_CFG *sys_cfg = (NCP_CMD_SYSTEM_CFG *)tlv;
    int ret                     = NCP_SUCCESS;
    const char *mod_name = NULL, *var_name = NULL, *value = NULL;

    if ((*sys_cfg->module_name == '\0') || (*sys_cfg->variable_name == '\0') || (*sys_cfg->value == '\0'))
    {
        ncp_e("invalid params");
        ret = -NCP_FAIL;
        goto done;
    }

    mod_name = sys_cfg->module_name;
    var_name = sys_cfg->variable_name;
    value    = sys_cfg->value;

    ret = ncp_bridge_set_conf(mod_name, var_name, value);

done:
    ncp_sys_prepare_status(NCP_BRIDGE_CMD_SYSTEM_CONFIG_SET, ret);

    return ret;
}

static int ncp_sys_get_config(void *tlv)
{
    NCP_CMD_SYSTEM_CFG *sys_cfg = (NCP_CMD_SYSTEM_CFG *)tlv;
    int ret                     = NCP_SUCCESS;
    const char *mod_name, *var_name;
    char value[CONFIG_VALUE_MAX_LEN] = {0};

    if ((*sys_cfg->module_name == '\0') || (*sys_cfg->variable_name == '\0'))
    {
        ncp_e("invalid params");
        ncp_sys_prepare_status(NCP_BRIDGE_CMD_SYSTEM_CONFIG_GET, NCP_BRIDGE_CMD_RESULT_ERROR);
        return -NCP_FAIL;
    }

    mod_name = sys_cfg->module_name;
    var_name = sys_cfg->variable_name;

    ret = ncp_bridge_get_conf(mod_name, var_name, value, sizeof(value));
    if (ret != WM_SUCCESS)
    {
        ncp_sys_prepare_status(NCP_BRIDGE_CMD_SYSTEM_CONFIG_GET, NCP_BRIDGE_CMD_RESULT_ERROR);
        return ret;
    }

    NCPCmd_DS_COMMAND *cmd_res = ncp_sys_get_resp_buf();
    cmd_res->header.cmd        = NCP_BRIDGE_CMD_SYSTEM_CONFIG_GET;
    cmd_res->header.size       = NCP_BRIDGE_CMD_HEADER_LEN;
    cmd_res->header.seqnum     = 0x00;
    cmd_res->header.result     = NCP_BRIDGE_CMD_RESULT_OK;
    cmd_res->header.msg_type   = NCP_BRIDGE_MSG_TYPE_RESP;

    NCP_CMD_SYSTEM_CFG *sys_cfg_res = (NCP_CMD_SYSTEM_CFG *)&cmd_res->params.system_cfg;
    (void)memcpy(sys_cfg_res->module_name, sys_cfg->module_name, sizeof(sys_cfg_res->module_name));
    (void)memcpy(sys_cfg_res->variable_name, sys_cfg->variable_name, sizeof(sys_cfg_res->variable_name));
    (void)memcpy(sys_cfg_res->value, value, sizeof(sys_cfg_res->value));

    cmd_res->header.size += sizeof(NCP_CMD_SYSTEM_CFG);

    return ret;
}

struct cmd_t system_cmd_config[] = {
    {NCP_BRIDGE_CMD_SYSTEM_CONFIG_SET, "ncp-set", ncp_sys_set_config, CMD_SYNC},
    {NCP_BRIDGE_CMD_SYSTEM_CONFIG_GET, "ncp-get", ncp_sys_get_config, CMD_SYNC},
    {NCP_BRIDGE_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_subclass_t cmd_subclass_system[] = {
    {NCP_BRIDGE_CMD_SYSTEM_CONFIG, system_cmd_config},
    {NCP_BRIDGE_CMD_INVALID, NULL},
};
