/*!\file ncp_cmd_common.h
 *\brief This file provides NCP command register interfaces.
 */
/*
 *  Copyright 2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __NCP_CMD_COMMON_H__
#define __NCP_CMD_COMMON_H__

#include "ncp_tlv_adapter.h"

#define NCP_CMD_SIZE_LOW_BYTES      4
#define NCP_CMD_SIZE_HIGH_BYTES     5
#define NCP_CMD_SEQUENCE_LOW_BYTES  6
#define NCP_CMD_SEQUENCE_HIGH_BYTES 7

#define NCP_SEND_DATA_INBUF_SIZE 1600
#define NCP_INBUF_SIZE     4096
#define NCP_SYS_INBUF_SIZE 1024
#define NCP_CMD_HEADER_LEN sizeof(NCP_COMMAND)
#define NCP_TLV_HEADER_LEN sizeof(TypeHeader_t)

/** NCP command class */
/** class type for WLAN command */
#define NCP_CMD_WLAN   0x00000000
/** class type for BLE command */
#define NCP_CMD_BLE    0x10000000
/** class type for 15.4 command */
#define NCP_CMD_15D4   0x20000000
/** class type for matter command */
#define NCP_CMD_MATTER 0x30000000
/** class type for NCP system command */
#define NCP_CMD_SYSTEM 0x40000000

/** NCP Message type */
/** NCP command type */
#define NCP_MSG_TYPE_CMD   0x00010000
/** NCP event type */
#define NCP_MSG_TYPE_EVENT 0x00020000
/** NCP command response type */
#define NCP_MSG_TYPE_RESP  0x00030000

/** NCP CMD response state */
/** General result code ok */
#define NCP_CMD_RESULT_OK 0x0000
/** General error */
#define NCP_CMD_RESULT_ERROR 0x0001
/** NCP Command is not valid */
#define NCP_CMD_RESULT_NOT_SUPPORT 0x0002
/** NCP Command is pending */
#define NCP_CMD_RESULT_PENDING 0x0003
/** System is busy */
#define NCP_CMD_RESULT_BUSY 0x0004
/** Data buffer is not big enough */
#define NCP_CMD_RESULT_PARTIAL_DATA 0x0005
/** Wrong input service id */
#define NCP_CMD_RESULT_INVALID_INDEX 0x0006

/** Get command class/subclass/cmd_id/tlv */
#define GET_CMD_CLASS(cmd)    (((cmd)&0xf0000000) >> 28)
/** Get command subclass */
#define GET_CMD_SUBCLASS(cmd) (((cmd)&0x0ff00000) >> 20)
/** Get command cmd_id */
#define GET_CMD_ID(cmd)       ((cmd)&0x0000ffff)
/** Get command tlv */
#define GET_CMD_TLV(cmd) \
    (((cmd)->size == NCP_CMD_HEADER_LEN) ? NULL : (uint8_t *)((uint8_t *)(cmd) + NCP_CMD_HEADER_LEN))

#define GET_MSG_TYPE(cmd)     ((cmd) & 0x000f0000)

/** Names for the values of the `async' field of `struct cmd_t'. */
#define CMD_SYNC  0
#define CMD_ASYNC 1

#define NCP_HASH_TABLE_SIZE  64
#define NCP_HASH_INVALID_KEY (uint8_t)(-1)
#define NCP_CMD_INVALID 0xFFFFFFFF

/** header structure for NCP command, NCP command response and NCP event. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_HEADER
{
    /** class: bit 28 ~ 31, subclass: bit 20 ~27, msg type: bit 16 ~ 19, cmd/cmd resp/event id: bit 0 ~ 15 */
    uint32_t cmd;
    /** NCP command total length include header length and body length */
    uint16_t size;
    /** NCP command sequence number */
    uint16_t seqnum;
    /** default OK(0) for NCP command, operation result for NCP command response */
    uint16_t result;
    /** Reserved fields */
    uint16_t rsvd;
} NCP_TLV_PACK_END NCP_COMMAND, NCP_RESPONSE, NCP_HOST_COMMAND, NCP_HOST_RESPONSE;

/** NCP tlv header */
typedef NCP_TLV_PACK_START struct TLVTypeHeader_t
{   
    /** tlv type */
    uint16_t type;
	/** size */
    uint16_t size;
} NCP_TLV_PACK_END TypeHeader_t, NCP_TLV_HEADER, NCP_MCU_HOST_TLV_HEADER;

/** NCP command */
struct cmd_t
{   
    /** class: bit 28 ~ 31, subclass: bit 20 ~27, msg type: bit 16 ~ 19, cmd/cmd resp/event id: bit 0 ~ 15 */
    uint32_t cmd;
	/** help */
    const char *help;
    int (*handler)(void *tlv);
    /* The field `async' is:
     *   CMD_SYNC     (or 0) if the command is executed synchronously,
     *   CMD_ASYNC    (or 1) if the command is executed asynchronously,
     */
    bool async;
};

/** NCP command subclass */
struct cmd_subclass_t
{   
    /** command subclass */
    uint32_t cmd_subclass;
	/** NCP command */
    struct cmd_t *cmd;
    /* Mapping of subclass list */
    uint8_t hash[NCP_HASH_TABLE_SIZE];
};

/** NCP command class */
struct cmd_class_t
{   
    /** command class */
    uint32_t cmd_class;
	/** command subclass */
    struct cmd_subclass_t *cmd_subclass;
    /** length of subclass list */
    uint16_t subclass_len;
    /** mapping of command list */
    uint8_t hash[NCP_HASH_TABLE_SIZE];
};

struct ncp_cmd_t
{
    uint32_t block_type;
    uint32_t command_sz;
    void     *cmd_buff;
};

/** power save configuration */
typedef NCP_TLV_PACK_START struct _power_cfg_t
{   
    /** enable flag, 1: enable, 0: disable */
    uint8_t enable;
	/** wakeup mode, 0x1: INTF, 0x2: GPIO */
    uint8_t wake_mode;
	/** 
	0 – unsubscribe(default)
	1 – subscribe(mandatory if wake_mode == 0x2)
	*/
    uint8_t subscribe_evt;
	/** wakeup duration, Minimum is 0(s), Default is 5(s) */
    uint32_t wake_duration;
	/** true: MEF, false: not MEF */
    uint8_t is_mef;
	/** value for default wowlan conditions */
    uint32_t wake_up_conds;
	/** wakeup by manual flag */
    uint8_t is_manual;
	/** is used to configure timeout for RTC timer and it is used with Power Manager only.
	If no other wakeup source wakes up NCP device, the RTC timer will wakeup device when it times out.
	*/
    uint32_t rtc_timeout;
	/** 1: periodic, 0: not periodic */
    uint8_t is_periodic;
	/** 0: MCU device won’t wakeup external host, 1: wakeup extern host */
    uint8_t wakeup_host;
} NCP_TLV_PACK_END power_cfg_t;

/** NCP host wakes up NCP device via interface */
#define WAKE_MODE_INTF 0x1
/** NCP host wakes up NCP device via GPIO */
#define WAKE_MODE_GPIO 0x2
/** For FRDMRW612 board only. NCP device will be wokenup by WIFI or NB */
#define WAKE_MODE_WIFI_NB 0x3

/**
* Initialize NCP commands.
*   
* \return 0 if success, return -1 if fails. 
*/
int ncp_cmd_list_init(void);

/**
* Register NCP commands to cmd_class_list. 
*
* \param[in,out] cmd_class a pointer to cmd_class_list
* \return 0 if success, return -1 if fails. 
*/
int ncp_register_class(struct cmd_class_t *cmd_class);

/**
* Traverse the search command list to find the command.
* 
* \param[in] cmd_class command class.
* \param[in] cmd_subclass command sub class.
* \param[in] cmd_id command ID.
* \return command's pointer if find else return fail. 
*/
struct cmd_t *lookup_class(uint32_t cmd_class, uint32_t cmd_subclass, uint32_t cmd_id);

#endif /* __NCP_CMD_COMMON_H__ */
