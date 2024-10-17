/*!\file ncp_host_app_ot.c
 *\brief This file provides interface for receiving tlv responses and processing tlv responses.
 */
/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if CONFIG_NCP_OT

/* -------------------------------------------------------------------------- */
/*                                Includes                                    */
/* -------------------------------------------------------------------------- */

#include "fsl_os_abstraction.h"
#include "ncp_cmd_common.h"
#include "ncp_host_app.h"
#include "ncp_adapter.h"
#include "ncp_tlv_adapter.h"

/* -------------------------------------------------------------------------- */
/*                                Definitions                                 */
/* -------------------------------------------------------------------------- */

typedef struct ncp_cmd_t ot_ncp_command_t;
extern uint32_t mcu_last_resp_rcvd;
extern uint32_t mcu_last_cmd_sent;

#define OT_NCP_STACK_SIZE   4096
static OSA_TASK_HANDLE_DEFINE(ot_ncp_task_handle);

#define OT_NCP_TASK_PRIO   1
void ot_ncp_task(void *pvParameters);
static OSA_TASK_DEFINE(ot_ncp_task, OT_NCP_TASK_PRIO, 1, OT_NCP_STACK_SIZE, 0);

#define OT_NCP_COMMAND_QUEUE_NUM 16
static osa_msgq_handle_t ot_ncp_command_queue;
OSA_MSGQ_HANDLE_DEFINE(ot_ncp_command_queue_buff, OT_NCP_COMMAND_QUEUE_NUM,  sizeof(ot_ncp_command_t));

/* -------------------------------------------------------------------------- */
/*                                Code                                        */
/* -------------------------------------------------------------------------- */

static void ot_ncp_callback(void *tlv, size_t tlv_sz, int status)
{
    int ret = 0;
    ot_ncp_command_t cmd_item;

    cmd_item.block_type = 0;
    cmd_item.command_sz = tlv_sz;
    cmd_item.cmd_buff = (ncp_tlv_qelem_t *)OSA_MemoryAllocate(tlv_sz);
    if (!cmd_item.cmd_buff)
    {
        ncp_adap_e("failed to allocate memory for tlv queue element");
        return ;
    }

    memcpy(cmd_item.cmd_buff, tlv, tlv_sz);

    ret = OSA_MsgQPut(ot_ncp_command_queue, &cmd_item);
    if (ret != NCP_STATUS_SUCCESS)
    {
        ncp_e("send to ot ncp cmd queue failed");
        OSA_MemoryFree(cmd_item.cmd_buff);
    }
    else
    {
        ncp_d("success to send ncp command on queue");
    }
}

static int ot_ncp_handle_cmd_input(uint8_t *cmd, uint32_t len)
{
    uint32_t ret = 0;

    cmd[len] = '\0';
    PRINTF("%s", cmd + NCP_CMD_HEADER_LEN);

    mcu_put_command_resp_sem();

    return ret;
}

void ot_ncp_task(void *pvParameters)
{
    int ret = 0;
    ot_ncp_command_t cmd_item;

    while (1)
    {
        ret = OSA_MsgQGet(ot_ncp_command_queue, &cmd_item, osaWaitForever_c);
        if (ret != NCP_STATUS_SUCCESS)
        {
            ncp_e("ot ncp command queue receive failed");
            continue;
        }
        else
        {
            ot_ncp_handle_cmd_input(cmd_item.cmd_buff, cmd_item.command_sz);
            OSA_MemoryFree(cmd_item.cmd_buff);
            cmd_item.cmd_buff = NULL;
        }
    }
}

int ncp_ot_app_init(void)
{
    int ret;

    ot_ncp_command_queue = (osa_msgq_handle_t)ot_ncp_command_queue_buff;

    ret = OSA_MsgQCreate(ot_ncp_command_queue, OT_NCP_COMMAND_QUEUE_NUM,  sizeof(ot_ncp_command_t));
    if (ret != NCP_STATUS_SUCCESS)
    {
        ncp_e("failed to create ot ncp command queue: %d", ret);
        return NCP_STATUS_ERROR;
    }

    ncp_tlv_install_handler(NCP_GET_CLASS(NCP_CMD_15D4), (void *)ot_ncp_callback);

    (void)OSA_TaskCreate((osa_task_handle_t)ot_ncp_task_handle, OSA_TASK(ot_ncp_task), NULL);
 
    return ret;
}

#endif