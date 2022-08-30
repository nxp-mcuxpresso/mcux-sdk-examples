/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "rtp.h"
#include "rtp_buffer.h"

#include "FreeRTOS.h"
#include "dsp_config.h"
#include "fsl_debug_console.h"
#include "queue.h"
#include "user_config.h"

/*******************************************************************************
 * Code
 ******************************************************************************/

void rtp_buffer_init(app_handle_t *app)
{
    int max_buffers = (AUDIO_SHARED_BUFFER_1_SIZE / RTP_PACKET_SIZE) + (AUDIO_SHARED_BUFFER_2_SIZE / RTP_PACKET_SIZE);
    uint8_t *buffer;
    int i;

    app->buffer_queue = xQueueCreate(max_buffers, sizeof(uint8_t *));
    if (app->buffer_queue == NULL)
    {
        PRINTF("failed to create queue of RTP buffers\r\n");
        while (true)
        {
        }
    }

    buffer = (uint8_t *)AUDIO_SHARED_BUFFER_1;
    for (i = 0; i < (AUDIO_SHARED_BUFFER_1_SIZE / RTP_PACKET_SIZE); i++)
    {
        xQueueSend(app->buffer_queue, (void *)&buffer, portMAX_DELAY);
        buffer += RTP_PACKET_SIZE;
    }

    buffer = (uint8_t *)AUDIO_SHARED_BUFFER_2;
    for (i = 0; i < (AUDIO_SHARED_BUFFER_2_SIZE / RTP_PACKET_SIZE); i++)
    {
        xQueueSend(app->buffer_queue, (void *)&buffer, portMAX_DELAY);
        buffer += RTP_PACKET_SIZE;
    }
}

uint8_t *rtp_buffer_get(app_handle_t *app)
{
    uint8_t *buffer;

    xQueueReceive(app->buffer_queue, &buffer, portMAX_DELAY);

    return buffer;
}

void rtp_buffer_put(app_handle_t *app, uint8_t *buffer)
{
    uint32_t address = (uint32_t)buffer;

    if (((address >= AUDIO_SHARED_BUFFER_1) &&
         (address <= (AUDIO_SHARED_BUFFER_1 + AUDIO_SHARED_BUFFER_1_SIZE - RTP_PACKET_SIZE))) ||
        ((address >= AUDIO_SHARED_BUFFER_2) &&
         (address <= (AUDIO_SHARED_BUFFER_2 + AUDIO_SHARED_BUFFER_2_SIZE - RTP_PACKET_SIZE))))
    {
        xQueueSend(app->buffer_queue, (void *)&buffer, portMAX_DELAY);
    }
    else
    {
        PRINTF("rtp_buffer_put(): address outside of the shared buffer\r\n");
        while (true)
        {
        }
    }
}
