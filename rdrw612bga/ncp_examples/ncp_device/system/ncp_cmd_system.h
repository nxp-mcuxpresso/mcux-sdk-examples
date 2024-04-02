/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __NCP_CMD_SYSTEM_H__
#define __NCP_CMD_SYSTEM_H__

#include "ncp_cmd_common.h"

/* System NCP Bridge subclass */
#define NCP_BRIDGE_CMD_SYSTEM_CONFIG   0x00000000

/* System Configure command */
#define NCP_BRIDGE_CMD_SYSTEM_CONFIG_SET  (NCP_BRIDGE_CMD_SYSTEM | NCP_BRIDGE_CMD_SYSTEM_CONFIG | 0x00000001) /* set-device-cfg */
#define NCP_BRIDGE_CMD_SYSTEM_CONFIG_GET  (NCP_BRIDGE_CMD_SYSTEM | NCP_BRIDGE_CMD_SYSTEM_CONFIG | 0x00000002) /* get-device-cfg */

#define MODULE_NAME_MAX_LEN 16
#define VAR_NAME_MAX_LEN  32
#define CONFIG_VALUE_MAX_LEN 256

/*NCP Bridge system configuration*/
typedef NCP_TLV_PACK_START struct _NCP_CMD_SYSTEM_CFG
{
    /* the name of system config file: sys, prov, wlan */
    char module_name[MODULE_NAME_MAX_LEN];
    /* the name of entry */
    char variable_name[VAR_NAME_MAX_LEN];
    /* set value/returned result */
    char value[CONFIG_VALUE_MAX_LEN];
} NCP_TLV_PACK_END NCP_CMD_SYSTEM_CFG;

typedef NCP_TLV_PACK_START struct _NCPCmd_DS_COMMAND
{
    /** Command Header : Command */
    NCP_BRIDGE_COMMAND header;
    /** Command Body */
    union
    {
        /** System configuration */
        NCP_CMD_SYSTEM_CFG system_cfg;
    } params;
} NCP_TLV_PACK_END NCPCmd_DS_COMMAND;

#endif /* __NCP_CMD_SYSTEM_H__ */