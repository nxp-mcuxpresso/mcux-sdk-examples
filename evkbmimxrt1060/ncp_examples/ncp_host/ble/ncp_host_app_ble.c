/** @file ncp_host_app_ble.c
 *
 *  @brief  This file provides interface for receiving tlv responses and processing tlv responses.
 *
 *  Copyright 2008-2023 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 */
#if CONFIG_NCP_BLE
#include "ncp_host_command_ble.h"
#include <ble_service/ht.h>
#include <ble_service/hr.h>
#include <ble_service/bas.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct ncp_cmd_t ble_ncp_command_t;
extern uint32_t mcu_last_resp_rcvd;
extern uint32_t mcu_last_cmd_sent;

#define BLE_NCP_STACK_SIZE   4096
static OSA_TASK_HANDLE_DEFINE(ble_ncp_task_handle);

#define BLE_NCP_TASK_PRIO   1
void ble_ncp_task(void *pvParameters);
static OSA_TASK_DEFINE(ble_ncp_task, BLE_NCP_TASK_PRIO, 1, BLE_NCP_STACK_SIZE, 0);

/*BLE NCP COMMAND TASK*/
#define BLE_NCP_COMMAND_QUEUE_NUM 16
static osa_msgq_handle_t ble_ncp_command_queue; /* ncp adapter TX msgq */
OSA_MSGQ_HANDLE_DEFINE(ble_ncp_command_queue_buff, BLE_NCP_COMMAND_QUEUE_NUM,  sizeof(ble_ncp_command_t));

/*******************************************************************************
 * Prototypes
 ******************************************************************************/


/*******************************************************************************
 * Code
 ******************************************************************************/
static void ble_ncp_callback(void *tlv, size_t tlv_sz, int status)
{
    int ret = 0;
    ble_ncp_command_t cmd_item;
    cmd_item.block_type = 0;
    cmd_item.command_sz = tlv_sz;
    cmd_item.cmd_buff = (ncp_tlv_qelem_t *)OSA_MemoryAllocate(tlv_sz);
    if (!cmd_item.cmd_buff)
    {
        ncp_adap_e("failed to allocate memory for tlv queue element");
        return ;
    }
    memcpy(cmd_item.cmd_buff, tlv, tlv_sz);

    ret = OSA_MsgQPut(ble_ncp_command_queue, &cmd_item);
    if (ret != NCP_STATUS_SUCCESS)
    {
        ncp_e("send to ble ncp cmd queue failed");
        OSA_MemoryFree(cmd_item.cmd_buff);
    }
    else
        ncp_d("success to send ncp command on queue");
}

static int ble_ncp_handle_cmd_input(uint8_t *cmd)
{
    uint32_t msg_type = 0;
    int ret;
    
    msg_type = GET_MSG_TYPE(((NCP_HOST_COMMAND *)cmd)->cmd);
    if (msg_type == NCP_MSG_TYPE_EVENT)
    {
        ret = ble_process_ncp_event(cmd);
        if (ret != NCP_STATUS_SUCCESS)
            PRINTF("Failed to parse ncp event\r\n");
    }
    else
    {
        ret = ble_process_response(cmd);
        if (ret == NCP_STATUS_ERROR)
            PRINTF("Failed to parse ncp tlv reponse\r\n");

        mcu_last_resp_rcvd = ((NCPCmd_DS_COMMAND *)cmd)->header.cmd;
        if (mcu_last_resp_rcvd == NCP_CMD_BLE_INVALID_CMD)
        {
            PRINTF("Previous command is invalid\r\n");
            mcu_last_resp_rcvd = 0;
        }
    }

    if (msg_type == NCP_MSG_TYPE_RESP)
    {
        /*If failed to receive response or successed to parse tlv reponse, release mcu command response semaphore to
         * allow processing new string commands. If reponse can't match to command, don't release command reponse
         * semaphore until receive response which id is same as command id.*/
        if (mcu_last_resp_rcvd == 0 || mcu_last_resp_rcvd == (mcu_last_cmd_sent | NCP_MSG_TYPE_RESP ))
            mcu_put_command_resp_sem();
        /* service run will automatically start adv/scan, so bypass this procedure response to release the semaphore */
        else if((mcu_last_cmd_sent == NCP_CMD_BLE_GATT_REGISTER_SERVICE) && 
            ((mcu_last_resp_rcvd == NCP_RSP_BLE_GAP_START_ADV) || (mcu_last_resp_rcvd == NCP_RSP_BLE_GAP_START_SCAN)) ) 
        {
            mcu_put_command_resp_sem();
        }else {
            PRINTF("Receive %d command response and wait for %d comamnd response.\r\n", mcu_last_resp_rcvd, mcu_last_cmd_sent);
        }
    }
    return ret;
}

void ble_ncp_task(void *pvParameters)
{
    int ret = 0;
    ble_ncp_command_t cmd_item;
    uint8_t *cmd_buf = NULL;
    while (1)
    {
        ret = OSA_MsgQGet(ble_ncp_command_queue, &cmd_item, osaWaitForever_c);
        if (ret != NCP_STATUS_SUCCESS)
        {
            ncp_e("ble ncp command queue receive failed");
            continue;
        }
        else
        {
            cmd_buf = cmd_item.cmd_buff;
            ble_ncp_handle_cmd_input(cmd_buf);
            OSA_MemoryFree(cmd_buf);
            cmd_buf = NULL;
        }
    }
}

static int ncp_ble_svc_init(void)
{
#if CONFIG_NCP_HTC
    htc_init();
#endif
#if CONFIG_NCP_HRC
    hrc_init();
#endif
    return NCP_STATUS_SUCCESS;    
}

int ncp_ble_app_init(void)
{
    int ret;
    ble_ncp_command_queue = (osa_msgq_handle_t)ble_ncp_command_queue_buff;
    ret = OSA_MsgQCreate(ble_ncp_command_queue, BLE_NCP_COMMAND_QUEUE_NUM,  sizeof(ble_ncp_command_t));
    if (ret != NCP_STATUS_SUCCESS)
    {
        ncp_e("failed to create ble ncp command queue: %d", ret);
        return -NCP_STATUS_ERROR;
    }

    ncp_tlv_install_handler(NCP_GET_CLASS(NCP_CMD_BLE), (void *)ble_ncp_callback);

    (void)OSA_TaskCreate((osa_task_handle_t)ble_ncp_task_handle, OSA_TASK(ble_ncp_task), NULL);
 
    ncp_ble_svc_init();
    
    return ret;
}
#endif