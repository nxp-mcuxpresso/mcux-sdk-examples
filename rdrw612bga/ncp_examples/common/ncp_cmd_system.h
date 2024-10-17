/*!\file ncp_cmd_system.h
 *\brief This file provies power save commands for NCP system.
 */
/* Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __NCP_CMD_SYSTEM_H__
#define __NCP_CMD_SYSTEM_H__

#include "ncp_cmd_common.h"

/** System NCP subclass */
/** subclass type for system configure */
#define NCP_CMD_SYSTEM_CONFIG      0x00000000
/** subclass type for system test */
#define NCP_CMD_SYSTEM_TEST        0x00100000
/** subclass type for system power managerment */
#define NCP_CMD_SYSTEM_POWERMGMT   0x00200000
/** subclass type for system asynchronous event */
#define NCP_CMD_SYSTEM_ASYNC_EVENT 0x00300000

/** System Configure command */
/** Wi-Fi system set configuration command ID */
#define NCP_CMD_SYSTEM_CONFIG_SET  (NCP_CMD_SYSTEM | NCP_CMD_SYSTEM_CONFIG | NCP_MSG_TYPE_CMD | 0x00000001) /* set-device-cfg */
/** Wi-Fi system set configuration command response ID */
#define NCP_RSP_SYSTEM_CONFIG_SET  (NCP_CMD_SYSTEM | NCP_CMD_SYSTEM_CONFIG | NCP_MSG_TYPE_RESP | 0x00000001)
/** Wi-Fi system get configuration command ID */
#define NCP_CMD_SYSTEM_CONFIG_GET  (NCP_CMD_SYSTEM | NCP_CMD_SYSTEM_CONFIG | NCP_MSG_TYPE_CMD | 0x00000002) /* get-device-cfg */
/** Wi-Fi system get configuration command response ID */
#define NCP_RSP_SYSTEM_CONFIG_GET  (NCP_CMD_SYSTEM | NCP_CMD_SYSTEM_CONFIG | NCP_MSG_TYPE_RESP | 0x00000002) 
/** Wi-Fi system power manager wakeup configuration command ID */
#define NCP_CMD_SYSTEM_POWERMGMT_WAKE_CFG (NCP_CMD_SYSTEM | NCP_CMD_SYSTEM_POWERMGMT | NCP_MSG_TYPE_CMD | 0x00000001) /* ncp-wake-cfg */
/** Wi-Fi system power manager wakeup configuration command response ID */
#define NCP_RSP_SYSTEM_POWERMGMT_WAKE_CFG (NCP_CMD_SYSTEM | NCP_CMD_SYSTEM_POWERMGMT | NCP_MSG_TYPE_RESP | 0x00000001) 
/** Wi-Fi system MCU deep sleep power save mode command ID */
#define NCP_CMD_SYSTEM_POWERMGMT_MCU_SLEEP (NCP_CMD_SYSTEM | NCP_CMD_SYSTEM_POWERMGMT | NCP_MSG_TYPE_CMD | 0x00000002) /* ncp-mcu-sleep */
/** Wi-Fi system MCU deep sleep power save mode command response ID */
#define NCP_RSP_SYSTEM_POWERMGMT_MCU_SLEEP (NCP_CMD_SYSTEM | NCP_CMD_SYSTEM_POWERMGMT | NCP_MSG_TYPE_RESP | 0x00000002)
/** Wi-Fi system power manager wakeup host command ID */
#define NCP_CMD_SYSTEM_POWERMGMT_WAKEUP_HOST (NCP_CMD_SYSTEM | NCP_CMD_SYSTEM_POWERMGMT | NCP_MSG_TYPE_CMD | 0x00000003) /* ncp-wakeup-host */
/** Wi-Fi system power manager wakeup host command response ID */
#define NCP_RSP_SYSTEM_POWERMGMT_WAKEUP_HOST (NCP_CMD_SYSTEM | NCP_CMD_SYSTEM_POWERMGMT | NCP_MSG_TYPE_RESP | 0x00000003)
/** Wi-Fi system power manager MCU sleep command ID */
#define NCP_CMD_SYSTEM_POWERMGMT_MCU_SLEEP_CFM (NCP_CMD_SYSTEM | NCP_CMD_SYSTEM_POWERMGMT | NCP_MSG_TYPE_CMD | 0x00000004)
/** Wi-Fi system power manager MCU sleep command response ID */
#define NCP_RSP_SYSTEM_POWERMGMT_MCU_SLEEP_CFM (NCP_CMD_SYSTEM | NCP_CMD_SYSTEM_POWERMGMT | NCP_MSG_TYPE_RESP | 0x00000004)

/** Wi-Fi enter MCU sleep mode event ID */
#define NCP_EVENT_MCU_SLEEP_ENTER (NCP_CMD_SYSTEM | NCP_CMD_SYSTEM_ASYNC_EVENT | NCP_MSG_TYPE_EVENT | 0x00000001)
/** Wi-Fi exit MCU sleep mode event ID */
#define NCP_EVENT_MCU_SLEEP_EXIT  (NCP_CMD_SYSTEM | NCP_CMD_SYSTEM_ASYNC_EVENT | NCP_MSG_TYPE_EVENT | 0x00000002)
/** Wi-Fi system test loopback command ID */
#define NCP_CMD_SYSTEM_TEST_LOOPBACK  (NCP_CMD_SYSTEM | NCP_CMD_SYSTEM_TEST | NCP_MSG_TYPE_CMD | 0x00000001) /* test-loopback */
#define MODULE_NAME_MAX_LEN 16
#define VAR_NAME_MAX_LEN  32
#define CONFIG_VALUE_MAX_LEN 256

/** NCP system configuration */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SYSTEM_CFG
{
    /* the name of system config file: sys, prov, wlan */
    char module_name[MODULE_NAME_MAX_LEN];
    /* the name of entry */
    char variable_name[VAR_NAME_MAX_LEN];
    /* set value/returned result */
    char value[CONFIG_VALUE_MAX_LEN];
} NCP_TLV_PACK_END NCP_CMD_SYSTEM_CFG;

/** NCP power manager wakeup configuration */
typedef NCP_TLV_PACK_START struct _NCP_CMD_POWERMGMT_WAKE_CFG
{   
    /** 
    host wake up mode, 
    0x1: INTF
    0x2: wakes up through GPIO
    */
    uint8_t wake_mode;
    /**
    0 – unsubscribe(default)
    1 – subscribe(mandatory if wake_mode == 0x2)
    */
    uint8_t subscribe_evt;
    /** wake up duration, minimum is 0 */
    uint32_t wake_duration;
} NCP_TLV_PACK_END NCP_CMD_POWERMGMT_WAKE_CFG;

/** NCP device sleep configuration */
typedef NCP_TLV_PACK_START struct _NCP_CMD_POWERMGMT_MCU_SLEEP
{
    /** 
    0x0: disable,
    0x1: enable
    */
	uint8_t enable;
    /**
    0x0: power manager,
    0x1: manual
    */
    uint8_t is_manual;
    /** used to configure timeout for RTC timer and it is used with power manager only */
    int rtc_timeout;
} NCP_TLV_PACK_END NCP_CMD_POWERMGMT_MCU_SLEEP;

/** NCP device wakeup NCP host configuration. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_POWERMGMT_WAKEUP_HOST
{   
    /** 
    0: disable,
    1: enable
    */
    uint8_t enable;
} NCP_TLV_PACK_END NCP_CMD_POWERMGMT_WAKEUP_HOST;

/** NCP system command */
typedef NCP_TLV_PACK_START struct _NCPCmd_DS_SYS_COMMAND
{
    /** Command Header : Command */
    NCP_COMMAND header;
    /** Command Body */
    union
    {
        /** System configuration */
        NCP_CMD_SYSTEM_CFG system_cfg;
        /** NCP power manager wakeup configuration */
        NCP_CMD_POWERMGMT_WAKE_CFG wake_config;
        /** NCP device sleep configuration. */
        NCP_CMD_POWERMGMT_MCU_SLEEP mcu_sleep_config;
        /** Control for NCP device wakeup NCP host. */
        NCP_CMD_POWERMGMT_WAKEUP_HOST host_wakeup_ctrl;
    } params;
} NCP_TLV_PACK_END NCPCmd_DS_SYS_COMMAND, MCU_NCPCmd_DS_SYS_COMMAND;

#endif /* __NCP_CMD_SYSTEM_H__ */
