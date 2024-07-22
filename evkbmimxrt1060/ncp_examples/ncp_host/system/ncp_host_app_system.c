/** @file ncp_host_app_system.c
 *
 *  @brief  This file provides interface for receiving tlv responses and processing tlv responses.
 *
 *  Copyright 2008-2023 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "ncp_host_command.h"
#include "ncp_cmd_common.h"
#include "ncp_cmd_system.h"

#define SYSTEM_NCP_TASK_PRIO   1
#define SYSTEM_NCP_STACK_SIZE   512
static OSA_TASK_HANDLE_DEFINE(system_ncp_task_handle);
void system_ncp_task(void *pvParameters);
static OSA_TASK_DEFINE(system_ncp_task, SYSTEM_NCP_TASK_PRIO, 1, SYSTEM_NCP_STACK_SIZE, 0);

/*SYSTEM NCP COMMAND TASK*/
#define SYSTEM_NCP_COMMAND_QUEUE_NUM 8
static osa_msgq_handle_t system_ncp_command_queue; /* ncp adapter TX msgq */
OSA_MSGQ_HANDLE_DEFINE(system_ncp_command_queue_buff, SYSTEM_NCP_COMMAND_QUEUE_NUM,  sizeof(system_ncp_command_t));

typedef struct ncp_cmd_t system_ncp_command_t;
extern uint32_t mcu_last_resp_rcvd;
extern uint32_t mcu_last_cmd_sent;

extern int mcu_put_command_resp_sem();

int system_process_response(uint8_t *res);
int system_process_event(uint8_t *res);

static void system_ncp_callback(void *tlv, size_t tlv_sz, int status)
{
    int ret = 0;
    system_ncp_command_t cmd_item;

    cmd_item.block_type = 0;
    cmd_item.command_sz = tlv_sz;
    cmd_item.cmd_buff = (ncp_tlv_qelem_t *)OSA_MemoryAllocate(tlv_sz);
    if (!cmd_item.cmd_buff)
    {
        ncp_e("failed to allocate memory for tlv queue element");
        return ;
    }
    memcpy(cmd_item.cmd_buff, tlv, tlv_sz);

    ret = OSA_MsgQPut(system_ncp_command_queue, &cmd_item);
    if (ret != NCP_STATUS_SUCCESS)
    {
        ncp_e("send to system ncp cmd queue failed");
        OSA_MemoryFree(cmd_item.cmd_buff);
    }
    else
        ncp_d("success to send ncp command on queue");
}

static int system_ncp_handle_cmd_input(uint8_t *cmd)
{
    uint32_t msg_type = 0;
    int ret;

    msg_type = GET_MSG_TYPE(((NCP_HOST_COMMAND *)cmd)->cmd);
    if (msg_type == NCP_MSG_TYPE_EVENT)
    {
        ret = system_process_event(cmd);
        if (ret != NCP_STATUS_SUCCESS)
            ncp_e("Failed to parse ncp event");
    }
    else
    {
        ret = system_process_response(cmd);
        if (ret == -NCP_STATUS_ERROR)
            ncp_e("Failed to parse ncp tlv reponse");
        mcu_last_resp_rcvd = ((MCU_NCPCmd_DS_SYS_COMMAND *)cmd)->header.cmd;
    }

#if !(COFNIG_NCP_SDIO_TEST_LOOPBACK)
    if (msg_type == NCP_MSG_TYPE_RESP)
#endif
    {
        /*If failed to receive response or successed to parse tlv reponse, release mcu command response semaphore to
         * allow processing new string commands. If reponse can't match to command, don't release command reponse
         * semaphore until receive response which id is same as command id.*/
        if (mcu_last_resp_rcvd == 0 || mcu_last_resp_rcvd == (mcu_last_cmd_sent | NCP_MSG_TYPE_RESP))
            mcu_put_command_resp_sem();
        else
            ncp_e("Receive %d command response and wait for %d comamnd response.", mcu_last_resp_rcvd,
                  mcu_last_cmd_sent);
    }
    return ret;
}

void system_ncp_task(void *pvParameters)
{
    int ret = 0;
    system_ncp_command_t cmd_item;
    uint8_t *cmd_buf = NULL;
    while (1)
    {
        ret = OSA_MsgQGet(system_ncp_command_queue, &cmd_item, osaWaitForever_c);
        if (ret != NCP_STATUS_SUCCESS)
        {
            ncp_e("system ncp command queue receive failed");
            continue;
        }
        else
        {
            cmd_buf = cmd_item.cmd_buff;
            system_ncp_handle_cmd_input(cmd_buf);
            OSA_MemoryFree(cmd_buf);
            cmd_buf = NULL;
        }
    }
}

int ncp_system_app_init()
{
    int ret;

    system_ncp_command_queue = (osa_msgq_handle_t)system_ncp_command_queue_buff;
    ret = OSA_MsgQCreate(system_ncp_command_queue, SYSTEM_NCP_COMMAND_QUEUE_NUM,  sizeof(system_ncp_command_t));
    if (ret != NCP_STATUS_SUCCESS)
    {
        ncp_e("failed to create system ncp command queue: %d", ret);
        return -NCP_STATUS_ERROR;
    }

    ncp_tlv_install_handler(NCP_GET_CLASS(NCP_CMD_SYSTEM), (void *)system_ncp_callback);

    (void)OSA_TaskCreate((osa_task_handle_t)system_ncp_task_handle, OSA_TASK(system_ncp_task), NULL);

    return ret;
}
