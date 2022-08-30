/*
 * Copyright 2020-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "rtp.h"
#include "rtp_receiver.h"

#include "app_dsp_ipc.h"
#include "fsl_debug_console.h"
#include "FreeRTOS.h"
#include "lwip/inet.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "main_cm33.h"
#include "rtp_buffer.h"
#include "task.h"
#include "user_config.h"
#include "wifi.h"

/*******************************************************************************
 * Code
 ******************************************************************************/

static void rtp_receiver_task(void *param)
{
    app_handle_t *app = (app_handle_t *)param;
    int sock;
    struct sockaddr_in local;
    struct sockaddr_in from;
    int fromlen;
    struct ip_mreq ipmreq;
    uint8_t *buffer;
    rtp_header_t *rtp_header;
    ip_addr_t mcast_address;
    struct timeval timeout_tv;
    int result;
    int received_packets = 0;
    size_t received_bytes;

    PRINTF("[rtp_receiver_task] start\r\n");

    /* Initialize RTP stream address. */
    if (RTP_RECV_MCAST_ADDRESS == NULL)
    {
        mcast_address.addr = IPADDR_ANY;
    }
    else
    {
        result = ipaddr_aton(RTP_RECV_MCAST_ADDRESS, &mcast_address);
        LWIP_ASSERT("RTP_RECV_MCAST_ADDRESS is not valid IP address", result != 0);
        LWIP_ASSERT("RTP_RECV_MCAST_ADDRESS is not multicast IP address", ip_addr_ismulticast(&mcast_address));
    }

    /* Wait for network to be ready. */
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    /* Create new socket. */
    sock = lwip_socket(AF_INET, SOCK_DGRAM, 0);
    LWIP_ASSERT("socket() failed", sock >= 0);

    /* Bind socket to local address. */
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port   = PP_HTONS(RTP_RECV_PORT);
    local.sin_addr.s_addr =
        PP_HTONL(INADDR_ANY); /* set to mcast_address.addr instead if you want to to forbid unicast */

    result = lwip_bind(sock, (struct sockaddr *)&local, sizeof(local));
    LWIP_ASSERT("bind() failed", result == 0);

    /* Set no timeout on receive. */
    timeout_tv.tv_sec  = 0;
    timeout_tv.tv_usec = 0;

    result = lwip_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout_tv, sizeof(timeout_tv));
    LWIP_ASSERT("setsockopt(SO_RCVTIMEO) failed", result == 0);

    /* Join multicast group. */
    if (ip_addr_ismulticast(&mcast_address))
    {
        ipmreq.imr_multiaddr.s_addr = mcast_address.addr;
        ipmreq.imr_interface.s_addr = PP_HTONL(INADDR_ANY);

        result = lwip_setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &ipmreq, sizeof(ipmreq));
        LWIP_ASSERT("setsockopt(IP_ADD_MEMBERSHIP) failed", result == 0);
    }

    LWIP_DEBUGF(RTP_DEBUG, ("[rtp_receiver_task] ready to receive\r\n"));

    /* Receive RTP packets. */
    while (true)
    {
        fromlen = sizeof(from);
        buffer  = rtp_buffer_get(app);
        result  = lwip_recvfrom(sock, (void *)buffer, sizeof(u8_t) * (RTP_PACKET_SIZE), 0, (struct sockaddr *)&from,
                               (socklen_t *)&fromlen);
        received_bytes = (size_t)result;

        if ((result > 0) && (received_bytes >= RTP_HEADER_SIZE))
        {
            rtp_header = (rtp_header_t *)buffer;
            received_packets++;

            rtp_header->sequence_number = lwip_ntohs(rtp_header->sequence_number);
            rtp_header->timestamp       = lwip_ntohl(rtp_header->timestamp);
            rtp_header->ssrc_id         = lwip_ntohl(rtp_header->ssrc_id);

            app_dsp_ipc_packet_ready(buffer, received_bytes);

            LWIP_DEBUGF(RTP_DEBUG, ("recvfrom: %s:%d", inet_ntoa(from.sin_addr), lwip_ntohs(from.sin_port)));
            LWIP_DEBUGF(RTP_DEBUG, ("rp: len:%3d snr:%u ts:%u ss:%u", received_bytes, rtp_header->sequence_number,
                                    rtp_header->timestamp, rtp_header->ssrc_id));
            LWIP_DEBUGF(RTP_DEBUG,
                        ("V: %hhu P: %hhu X: %hhu CC: %hhu M: %hhu PT: %hhu\r\n", (rtp_header->version & 192) >> 6,
                         (rtp_header->version & 32) >> 5, (rtp_header->version & 16) >> 4, rtp_header->version & 15,
                         (rtp_header->payload_type & 128) >> 7, rtp_header->payload_type & 127));
        }
        else
        {
            LWIP_DEBUGF(RTP_DEBUG, ("malformed RTP packet or receive error"));

            /* Return the buffer. */
            rtp_buffer_put(app, buffer);
        }
    }

    /* Close the socket. */
    /* lwip_close(sock); */

    /* PRINTF("[rtp_receiver_task] done\r\n"); */

    /* vTaskDelete(NULL); */
}

void rtp_receiver_init(app_handle_t *app)
{
    if (xTaskCreate(rtp_receiver_task, "rtp_receiver_task",
                    RTP_RECEIVER_TASK_STACK_SIZE / sizeof(configSTACK_DEPTH_TYPE), app, RTP_RECEIVER_TASK_PRIORITY,
                    &app->rtp_receiver_task_handle) != pdPASS)
    {
        PRINTF("\r\nFailed to create RTP client task\r\n");
        while (true)
        {
        }
    }
}
