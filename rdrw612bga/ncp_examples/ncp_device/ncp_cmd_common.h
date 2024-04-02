/* @file ncp_bridge_cmd.h
 *
 *  @brief This file contains ncp bridge command/response/event definitions
 *
 *  Copyright 2008-2023 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __NCP_CMD_COMMON_H__
#define __NCP_CMD_COMMON_H__

#include "ncp_tlv_adapter.h"

#define NCP_BRIDGE_CMD_SIZE_LOW_BYTES      4
#define NCP_BRIDGE_CMD_SIZE_HIGH_BYTES     5
#define NCP_BRIDGE_CMD_SEQUENCE_LOW_BYTES  6
#define NCP_BRIDGE_CMD_SEQUENCE_HIGH_BYTES 7

#define NCP_BRIDGE_SEND_DATA_INBUF_SIZE 1600
#define NCP_BRIDGE_INBUF_SIZE     4096
#define NCP_BRIDGE_CMD_HEADER_LEN sizeof(NCP_BRIDGE_COMMAND)
#define NCP_BRIDGE_TLV_HEADER_LEN sizeof(TypeHeader_t)

/*NCP Bridge command class*/
#define NCP_BRIDGE_CMD_WLAN   0x00000000
#define NCP_BRIDGE_CMD_BLE    0x01000000
#define NCP_BRIDGE_CMD_15D4   0x02000000
#define NCP_BRIDGE_CMD_MATTER 0x03000000
#define NCP_BRIDGE_CMD_SYSTEM 0x04000000

/*NCP Bridge Message Type*/
#define NCP_BRIDGE_MSG_TYPE_CMD   0x0000
#define NCP_BRIDGE_MSG_TYPE_RESP  0x0001
#define NCP_BRIDGE_MSG_TYPE_EVENT 0x0002

/*NCP Bridge CMD response state*/
/*General result code ok*/
#define NCP_BRIDGE_CMD_RESULT_OK 0x0000
/*General error*/
#define NCP_BRIDGE_CMD_RESULT_ERROR 0x0001
/*NCP Bridge Command is not valid*/
#define NCP_BRIDGE_CMD_RESULT_NOT_SUPPORT 0x0002
/*NCP Bridge Command is pending*/
#define NCP_BRIDGE_CMD_RESULT_PENDING 0x0003
/*System is busy*/
#define NCP_BRIDGE_CMD_RESULT_BUSY 0x0004
/*Data buffer is not big enough*/
#define NCP_BRIDGE_CMD_RESULT_PARTIAL_DATA 0x0005
/*Wrong input service id */
#define NCP_BRIDGE_CMD_RESULT_INVALID_INDEX 0x0006

/* Get command class/subclass/cmd_id/tlv */
#define GET_CMD_CLASS(cmd)    (((cmd)&0xff000000) >> 24)
#define GET_CMD_SUBCLASS(cmd) (((cmd)&0x00ff0000) >> 16)
#define GET_CMD_ID(cmd)       ((cmd)&0x0000ffff)
#define GET_CMD_TLV(cmd) \
    (((cmd)->size == NCP_BRIDGE_CMD_HEADER_LEN) ? NULL : (uint8_t *)((uint8_t *)(cmd) + NCP_BRIDGE_CMD_HEADER_LEN))

/* Names for the values of the `async' field of `struct cmd_t'.  */
#define CMD_SYNC  0
#define CMD_ASYNC 1

#define NCP_HASH_TABLE_SIZE  64
#define NCP_HASH_INVALID_KEY (uint8_t)(-1)
#define NCP_BRIDGE_CMD_INVALID 0xFFFFFFFF

/*NCP Bridge command header*/
typedef NCP_TLV_PACK_START struct bridge_command_header
{
    /*bit0 ~ bit15 cmd id  bit16 ~ bit23 cmd subclass bit24 ~ bit31 cmd class*/
    uint32_t cmd;
    uint16_t size;
    uint16_t seqnum;
    uint16_t result;
    uint16_t msg_type;
} NCP_TLV_PACK_END NCP_BRIDGE_COMMAND, NCP_BRIDGE_RESPONSE;

/*NCP Bridge tlv header*/
typedef NCP_TLV_PACK_START struct TLVTypeHeader_t
{
    uint16_t type;
    uint16_t size;
} NCP_TLV_PACK_END TypeHeader_t, NCP_BRIDGE_TLV_HEADER, NCP_MCU_HOST_TLV_HEADER;

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

int ncp_cmd_list_init(void);
int ncp_register_class(struct cmd_class_t *cmd_class);
struct cmd_t *lookup_class(uint32_t cmd_class, uint32_t cmd_subclass, uint32_t cmd_id);

#endif /* __NCP_CMD_COMMON_H__ */
