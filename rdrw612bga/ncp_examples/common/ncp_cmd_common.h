/* @file ncp_cmd_common.h
 *
 *  @brief This file contains ncp command/response/event definitions
 *
 *  Copyright 2008-2023 NXP
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

/*NCP command class*/
#define NCP_CMD_WLAN   0x00000000
#define NCP_CMD_BLE    0x10000000
#define NCP_CMD_15D4   0x20000000
#define NCP_CMD_MATTER 0x30000000
#define NCP_CMD_SYSTEM 0x40000000

/*NCP Message Type*/
#define NCP_MSG_TYPE_CMD   0x00010000
#define NCP_MSG_TYPE_EVENT 0x00020000
#define NCP_MSG_TYPE_RESP  0x00030000

/*NCP CMD response state*/
/*General result code ok*/
#define NCP_CMD_RESULT_OK 0x0000
/*General error*/
#define NCP_CMD_RESULT_ERROR 0x0001
/*NCP Command is not valid*/
#define NCP_CMD_RESULT_NOT_SUPPORT 0x0002
/*NCP Command is pending*/
#define NCP_CMD_RESULT_PENDING 0x0003
/*System is busy*/
#define NCP_CMD_RESULT_BUSY 0x0004
/*Data buffer is not big enough*/
#define NCP_CMD_RESULT_PARTIAL_DATA 0x0005
/*Wrong input service id */
#define NCP_CMD_RESULT_INVALID_INDEX 0x0006

/* Get command class/subclass/cmd_id/tlv */
#define GET_CMD_CLASS(cmd)    (((cmd)&0xf0000000) >> 28)
#define GET_CMD_SUBCLASS(cmd) (((cmd)&0x0ff00000) >> 20)
#define GET_CMD_ID(cmd)       ((cmd)&0x0000ffff)
#define GET_CMD_TLV(cmd) \
    (((cmd)->size == NCP_CMD_HEADER_LEN) ? NULL : (uint8_t *)((uint8_t *)(cmd) + NCP_CMD_HEADER_LEN))

#define GET_MSG_TYPE(cmd)     ((cmd) & 0x000f0000)

/* Names for the values of the `async' field of `struct cmd_t'.  */
#define CMD_SYNC  0
#define CMD_ASYNC 1

#define NCP_HASH_TABLE_SIZE  64
#define NCP_HASH_INVALID_KEY (uint8_t)(-1)
#define NCP_CMD_INVALID 0xFFFFFFFF

/*NCP command header*/
/* 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8  6 5 4 3 2 1 0 */
/* |  class   |       subclass        |  msg type |               command id          | */
/* |              sequence number                 |                size               | */
/* |                 reserved                     |                  result           | */
typedef NCP_TLV_PACK_START struct ncp_command_header
{
    /* class: bit 28 ~ 31 / subclass: bit 20 ~27 / msg type: bit 16 ~ 19 / command id: bit 0 ~ 15*/
    uint32_t cmd;
    uint16_t size;
    uint16_t seqnum;
    uint16_t result;
    uint16_t rsvd;
} NCP_TLV_PACK_END NCP_COMMAND, NCP_RESPONSE, NCP_HOST_COMMAND, NCP_HOST_RESPONSE;

/*NCP tlv header*/
typedef NCP_TLV_PACK_START struct TLVTypeHeader_t
{
    uint16_t type;
    uint16_t size;
} NCP_TLV_PACK_END TypeHeader_t, NCP_TLV_HEADER, NCP_MCU_HOST_TLV_HEADER;

struct cmd_t
{
    uint32_t cmd;
    const char *help;
    int (*handler)(void *tlv);
    /* The field `async' is:
     *   CMD_SYNC     (or 0) if the command is executed synchronously,
     *   CMD_ASYNC    (or 1) if the command is executed asynchronously,
     */
    bool async;
};

struct cmd_subclass_t
{
    uint32_t cmd_subclass;
    struct cmd_t *cmd;
    /* Mapping of subclass list */
    uint8_t hash[NCP_HASH_TABLE_SIZE];
};

struct cmd_class_t
{
    uint32_t cmd_class;
    struct cmd_subclass_t *cmd_subclass;
    /* Length of subclass list */
    uint16_t subclass_len;
    /* Mapping of cmd list */
    uint8_t hash[NCP_HASH_TABLE_SIZE];
};

struct ncp_cmd_t
{
    uint32_t block_type;
    uint32_t command_sz;
    void     *cmd_buff;
};

typedef NCP_TLV_PACK_START struct _power_cfg_t
{
    uint8_t enable;
    uint8_t wake_mode;
    uint8_t subscribe_evt;
    uint32_t wake_duration;
    uint8_t is_mef;
    uint32_t wake_up_conds;
    uint8_t is_manual;
    uint32_t rtc_timeout;
    uint8_t is_periodic;
    uint8_t wakeup_host;
} NCP_TLV_PACK_END power_cfg_t;

/* Host wakes up MCU through interface */
#define WAKE_MODE_INTF 0x1
/* Host wakes up MCU through GPIO */
#define WAKE_MODE_GPIO 0x2

int ncp_cmd_list_init(void);
int ncp_register_class(struct cmd_class_t *cmd_class);
struct cmd_t *lookup_class(uint32_t cmd_class, uint32_t cmd_subclass, uint32_t cmd_id);

#endif /* __NCP_CMD_COMMON_H__ */
