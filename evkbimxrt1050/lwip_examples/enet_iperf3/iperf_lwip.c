/*
 * Copyright (c) 2013 - 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "assert.h"
#include "stdint.h"
#include "stdio.h"
#include "iperf_api.h"
#include <lwip/sockets.h>
#include "lwip/prot/dhcp.h"

static struct sockaddr_in s_addr = {0};

int iperf_ip_to_ip(struct iperf_ip *iperf_ip, uint32_t *ip)
{
    assert(!((NULL == iperf_ip) || (NULL == ip)));
    if ((NULL == iperf_ip) || (NULL == ip))
        return -1;

    *ip = ((iperf_ip->ip[0] & 0xFF) | ((iperf_ip->ip[1] & 0xFF) << 8) | ((iperf_ip->ip[2] & 0xFF) << 16) |
           ((iperf_ip->ip[3] & 0xFF) << 24));

    return 0;
}

void iperf_hw_init(struct iperf_ctx *ctx)
{
    ctx->ctrl_buf = pvPortMalloc(IPERF_BUFFER_MAX);
    assert(NULL != ctx->ctrl_buf);
    memset(ctx->ctrl_buf, 0, IPERF_BUFFER_MAX);

    ctx->send_buf = pvPortMalloc(IPERF_BUFFER_MAX);
    assert(NULL != ctx->send_buf);
    memset(ctx->send_buf, 0, IPERF_BUFFER_MAX);

    ctx->recv_buf = pvPortMalloc(IPERF_BUFFER_MAX);
    assert(NULL != ctx->recv_buf);
    memset(ctx->recv_buf, 0, IPERF_BUFFER_MAX);

    memset(&s_addr, 0, sizeof(struct sockaddr_in));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port   = htons(SENDER_PORT_NUM);
    s_addr.sin_len    = sizeof(s_addr);
    iperf_ip_to_ip(&ctx->server_ip, &s_addr.sin_addr.s_addr);

    ctx->addr     = &s_addr;
    ctx->addr_len = sizeof(s_addr);
}

int iperf_socket(int protocol)
{
    int socket;

    if (protocol == 1)
        protocol = SOCK_STREAM;
    if (protocol == 2)
        protocol = SOCK_DGRAM;

    socket = lwip_socket(PF_INET, protocol, 0);
    assert(!(socket == -1));
    if ((socket == -1))
        return -1;

    return socket;
}

int iperf_connect(int socket, void *addr, uint32_t addrlen)
{
    int conn;
    conn = lwip_connect(socket, (struct sockaddr *)addr, addrlen);
    return conn;
}

int iperf_send(int socket, void *buffer, size_t len, int flag)
{
    int sent_bytes;
    sent_bytes = send(socket, buffer, len, flag);
    return sent_bytes;
}

int iperf_send_to(int socket, char *buffer, int len, int flags, void *to, int tolen)
{
    int sent_bytes;
    sent_bytes = lwip_sendto(socket, buffer, len, flags, (struct sockaddr *)to, tolen);
    return sent_bytes;
}

int iperf_recv_noblock(int socket, void *buf, size_t len, int flags)
{
    int recv_bytes = 0;
    flags |= MSG_DONTWAIT;
    recv_bytes = lwip_recv(socket, buf, len, flags);
    return recv_bytes;
}

int iperf_recv_from_blocked(int socket, void *buf, size_t len, int flags)
{
    struct sockaddr_in tmp_struct = {0};
    u32_t sock_len                = sizeof(tmp_struct);
    int recv_bytes                = -1;

    recv_bytes = lwip_recvfrom(socket, buf, len, flags, (struct sockaddr *)&tmp_struct, &sock_len);

    return recv_bytes;
}

int iperf_recv_from_timeout(int socket, void *buf, size_t len, int flags, int timeout_ms)
{
    struct sockaddr_in tmp_struct = {0};
    u32_t sock_len                = sizeof(tmp_struct);
    int recv_bytes                = -1;
    int nready;
    struct pollfd fds;

    assert(timeout_ms > 0);

    fds.fd      = socket;
    fds.events  = POLLIN;
    fds.revents = 0;

    nready = lwip_poll(&fds, 1, timeout_ms);

    if (nready > 0)
    {
        recv_bytes = lwip_recvfrom(socket, buf, len, flags, (struct sockaddr *)&tmp_struct, &sock_len);
    }

    return recv_bytes;
}

int iperf_recv_blocked(int socket, void *buf, size_t len, int flags)
{
    int recv_bytes;
    recv_bytes = lwip_recv(socket, buf, len, flags);
    return recv_bytes;
}

int iperf_recv_timeout(int socket, void *buf, size_t len, int flags, int timeout_ms)
{
    int recv_bytes = 0;
    int nready;
    struct pollfd fds;

    assert(timeout_ms > 0);

    fds.fd      = socket;
    fds.events  = POLLIN;
    fds.revents = 0;

    nready = lwip_poll(&fds, 1, timeout_ms);
    if (nready > 0)
    {
        flags |= MSG_DONTWAIT;
        recv_bytes = lwip_recv(socket, buf, len, flags);
    }

    return recv_bytes;
}

int iperf_socket_close(int socket)
{
    lwip_close(socket);
    return 0;
}
