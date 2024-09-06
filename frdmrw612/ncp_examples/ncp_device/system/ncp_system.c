/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "fsl_os_abstraction.h"
#include "ncp_cmd_system.h"
#include "ncp_config.h"
#include "app_notify.h"
#include "ncp_glue_system.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef struct ncp_cmd_t system_ncp_command_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern uint16_t g_cmd_seqno;
extern uint8_t cmd_buf[NCP_INBUF_SIZE];
extern uint8_t sys_res_buf[NCP_SYS_INBUF_SIZE];

#define SYSTEM_TASK_PRIO        3
#define SYSTEM_NCP_STACK_SIZE   2048

static OSA_TASK_HANDLE_DEFINE(system_ncp_handle);

static void system_ncp_task(void *pvParameters);
OSA_TASK_DEFINE(system_ncp_task, SYSTEM_TASK_PRIO, 1, SYSTEM_NCP_STACK_SIZE, 0);

OSA_SEMAPHORE_HANDLE_DEFINE(system_ncp_lock);
OSA_SEMAPHORE_HANDLE_DEFINE(ncp_sys_resp_buf_lock);

#define SYSTEM_NCP_COMMAND_QUEUE_NUM 8
static osa_msgq_handle_t system_ncp_command_queue; /* ncp adapter TX msgq */
OSA_MSGQ_HANDLE_DEFINE(system_ncp_command_queue_buff, SYSTEM_NCP_COMMAND_QUEUE_NUM,  sizeof(system_ncp_command_t));


/*******************************************************************************
 * Code
 ******************************************************************************/

/* system_ncp_send_response() handles the response from the wifi driver.
 * This involves
 * 1) sending cmd response out to interface
 * 2) computation of the crc of the cmd resp
 * 3) reset cmd_buf & sys_res_buf
 * 4) release lock
 */
int system_ncp_send_response(uint8_t *pbuf)
{
    int ret                = NCP_SUCCESS;
    uint16_t transfer_len = 0;
    NCP_COMMAND *res = (NCP_COMMAND *)pbuf;

    /* set cmd seqno */
    res->seqnum = g_cmd_seqno;
    transfer_len        = res->size;
    if (transfer_len >= NCP_CMD_HEADER_LEN)
    {
        /* write response to host */
        ret = ncp_tlv_send(pbuf, transfer_len);
        if (ret != NCP_SUCCESS)
        {
            ncp_e("failed to write response");
            ret = -NCP_FAIL;
        }
    }
    else
    {
        ncp_e("command length is less than 12, cmd_len = %d", transfer_len);
        ret = -NCP_FAIL;
    }

    if (GET_MSG_TYPE(res->cmd) != NCP_MSG_TYPE_EVENT)
    {
        /* Reset cmd_buf */
        memset(cmd_buf, 0, sizeof(cmd_buf));
        /* Reset sys_res_buf */
        memset(sys_res_buf, 0, sizeof(sys_res_buf));
        OSA_SemaphorePost(system_ncp_lock);
        ncp_put_sys_resp_buf_lock();
        ncp_d("put lock");
    }


    return ret;
}

static int system_ncp_command_handle_input(uint8_t *cmd)
{
    NCP_COMMAND *input_cmd = (NCP_COMMAND *)cmd;
    struct cmd_t *command         = NULL;
    int ret                       = NCP_SUCCESS;

    uint32_t cmd_class    = GET_CMD_CLASS(input_cmd->cmd);
    uint32_t cmd_subclass = GET_CMD_SUBCLASS(input_cmd->cmd);
    uint32_t cmd_id       = GET_CMD_ID(input_cmd->cmd);
    void *cmd_tlv         = GET_CMD_TLV(input_cmd);

    command = lookup_class(cmd_class, cmd_subclass, cmd_id);
    if (NULL == command)
    {
        ncp_d("ncp system lookup cmd failed\r\n");
        return -NCP_FAIL;
    }
    ncp_d("ncp system got command: <%s>", command->help);
    ret = command->handler(cmd_tlv);

    if (command->async == CMD_SYNC)
    {
         system_ncp_send_response(sys_res_buf);
    }
    else
    {
        /* Wait for cmd to execute, then
         * 1) send cmd response
         * 2) reset cmd_buf & sys_res_buf
         * 3) release system_ncp_lock */
        ncp_put_sys_resp_buf_lock();
    }

    return ret;
}

static void system_ncp_task(void *pvParameters)
{
    int ret = 0;
    system_ncp_command_t cmd_item;
    uint8_t *cmd_buf = NULL;
    while (1)
    {
        ret = OSA_MsgQGet(system_ncp_command_queue, &cmd_item, osaWaitForever_c);
        if (ret != NCP_SUCCESS)
        {
            ncp_e("system ncp command queue receive failed");
            continue;
        }
        else
        {
            cmd_buf = cmd_item.cmd_buff;
            if(((NCP_COMMAND *)cmd_buf)->cmd != NCP_CMD_SYSTEM_POWERMGMT_MCU_SLEEP_CFM)
                OSA_SemaphoreWait(system_ncp_lock, osaWaitForever_c);
            system_ncp_command_handle_input(cmd_buf);
            OSA_MemoryFree(cmd_buf);
            cmd_buf = NULL;
        }
    }
}

static void system_ncp_callback(void *tlv, size_t tlv_sz, int status)
{
    int ret = 0;
    system_ncp_command_t cmd_item;

    cmd_item.block_type = 0;
    cmd_item.command_sz = tlv_sz;
    cmd_item.cmd_buff = (ncp_tlv_qelem_t *)OSA_MemoryAllocate(tlv_sz);
    if (!cmd_item.cmd_buff)
    {
        NCP_TLV_STATS_INC(drop);
        ncp_adap_d("%s: failed to allocate memory for tlv queue element", __FUNCTION__);
        return ;
    }
    memcpy(cmd_item.cmd_buff, tlv, tlv_sz);

    ret = OSA_MsgQPut(system_ncp_command_queue, &cmd_item);
    if (ret != kStatus_Success)
    {
        if (cmd_item.cmd_buff)
        {
            OSA_MemoryFree(cmd_item.cmd_buff);
            cmd_item.cmd_buff = NULL;
        }
        ncp_e("send to wifi ncp cmd queue failed");
    }
    else
        ncp_d("success to send ncp command on queue");
}

int system_ncp_init(void)
{
    int ret;
    system_ncp_command_queue = (osa_msgq_handle_t)system_ncp_command_queue_buff;

    ret = ncp_config_init();
    assert(NCP_SUCCESS == ret);

    ret = OSA_MsgQCreate(system_ncp_command_queue, SYSTEM_NCP_COMMAND_QUEUE_NUM,  sizeof(system_ncp_command_t));
    if (ret != NCP_SUCCESS)
    {
        ncp_e("failed to create system ncp command queue: %d", ret);
        return -NCP_FAIL;
    }

    ret = OSA_SemaphoreCreateBinary(system_ncp_lock);
    if (ret != kStatus_Success)
    {
        ncp_e("failed to create system_ncp_lock: %d", ret);
        return ret;
    }
    else
    {
        OSA_SemaphorePost(system_ncp_lock);
    }

    ret = OSA_SemaphoreCreateBinary(ncp_sys_resp_buf_lock);
    if (ret != kStatus_Success)
    {
        ncp_e("failed to create system_ncp_lock: %d", ret);
        return ret;
    }
    else
    {
        OSA_SemaphorePost(ncp_sys_resp_buf_lock);
    }

    ncp_tlv_install_handler(GET_CMD_CLASS(NCP_CMD_SYSTEM), (void *)system_ncp_callback);
    ret = OSA_TaskCreate((osa_task_handle_t) system_ncp_handle, OSA_TASK(system_ncp_task), NULL);
    if (ret != KOSA_StatusSuccess)
    {
        ncp_e("failed to create ncp system task: %d", ret);
        return -NCP_FAIL;
    }

    ret = app_notify_init();
    if (ret != WM_SUCCESS)
    {
        ncp_e("app notify failed to initialize: %d", ret);
        return -WM_FAIL;
    }

    return NCP_SUCCESS;
}

void ncp_get_sys_resp_buf_lock()
{
    OSA_SemaphoreWait(ncp_sys_resp_buf_lock, osaWaitForever_c);
}

void ncp_put_sys_resp_buf_lock()
{
    OSA_SemaphorePost(ncp_sys_resp_buf_lock);
}
