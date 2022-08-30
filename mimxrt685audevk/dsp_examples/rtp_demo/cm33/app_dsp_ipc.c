/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "app_dsp_ipc.h"

#include "dsp_ipc.h"
#include "fsl_debug_console.h"
#include "main_cm33.h"
#include "message.h"
#include "rtp_buffer.h"
#include "user_config.h"

/*******************************************************************************
 * Code
 ******************************************************************************/

static void app_dsp_ipc_handle_message(app_handle_t *app, message_t *msg)
{
    if ((msg->command == AudioEvent_PacketConsumed) && (msg->params[0] == MessageParam_PacketAddress))
    {
        rtp_buffer_put(app, (uint8_t *)msg->params[1]);
    }
}

static void app_dsp_ipc_task(void *param)
{
    app_handle_t *app = (app_handle_t *)param;
    message_t msg;

    PRINTF("[app_dsp_ipc] start\r\n");

    while (true)
    {
        /* Blocking receive IPC message from DSP. */
        dsp_ipc_recv_sync(&msg);

        /* Process message. */
        app_dsp_ipc_handle_message(app, &msg);
    }

    /* PRINTF("[app_dsp_ipc] done\r\n"); */
    /* vTaskDelete(NULL); */
}

void app_dsp_ipc_init(app_handle_t *app)
{
    TaskHandle_t taskHandle;

    if (xTaskCreate(app_dsp_ipc_task, "app_dsp_ipc", APP_DSP_IPC_TASK_STACK_SIZE / sizeof(configSTACK_DEPTH_TYPE), app,
                    APP_DSP_IPC_TASK_PRIORITY, &taskHandle) != pdPASS)
    {
        PRINTF("\r\nFailed to create DSP IPC task\r\n");
        while (true)
        {
        }
    }
}

void app_dsp_ipc_packet_ready(uint8_t *buffer, size_t size)
{
    message_t msg = {0};

    msg.command = AudioCommand_PlayPacket;

    msg.params[0] = MessageParam_PacketAddress;
    msg.params[1] = (uint32_t)buffer;
    msg.params[2] = MessageParam_PacketSize;
    msg.params[3] = (uint32_t)size;
    msg.params[4] = MessageParam_NULL;

    /* Send message to DSP. */
    dsp_ipc_send_sync(&msg);
}
