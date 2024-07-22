/*
 * Copyright 2016-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __IPERF_API_H__
#define __IPERF_API_H__

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* @TEST_ANCHOR */

/******************************************************************/
/*                          Iperf States                          */
#define TEST_START       1
#define TEST_RUNNING     2
#define TEST_END         4
#define PARAM_EXCHANGE   9
#define CREATE_STREAMS   10
#define SERVER_TERMINATE 11
#define CLIENT_TERMINATE 12
#define EXCHANGE_RESULTS 13
#define DISPLAY_RESULTS  14
#define IPERF_START      15
#define IPERF_DONE       16
#define ACCESS_DENIED    (-1)
#define SERVER_ERROR     (-2)
#define IPERF_NOP        (-42) // Created not an official IPERF3 state
/*******************************************************************/

#define IPERF_CLIENT_RX     1
#define IPERF_CLIENT_TX     0
#define IPERF_CLIENT_TX_UDP 2
#define IPERF_CLIENT_RX_UDP 3
#define IPERF_BUFFER_MAX    1024
#define CTRL_IPERF_MSG_LEN  1
#define MAGIC_COOKIE        "abeceda890abeceda890abeceda890123456"
#define TCP                 1
#define UDP                 2
#define OUTPUT_STR_MAX      (1024)
#ifndef SENDER_PORT_NUM
#define SENDER_PORT_NUM 5201
#endif

struct iperf_ip
{
    uint8_t ip[4];
};

struct iperf_ctx
{
    /*Declarations*/
    int socket_type;
    int mode;
    volatile signed char state;
    int send_counter;
    int ctrl_sock;
    int real_send_buff_counter;
    int real_recv_buff_counter;
    char *ctrl_buf;
    void *addr;
    uint32_t addr_len;
    char *send_buf;
    char *recv_buf;
    struct iperf_ip server_ip;
    int data_sock;
    int skip_state_start;
    int iperf_done;
};

void iperf_hw_init(struct iperf_ctx *ctx);
int iperf_send(int socket, void *buffer, size_t len, int flag);
int iperf_send_to(int socket, char *buffer, int len, int flags, void *to, int tolen);
int iperf_connect(int socket, void *addr, uint32_t addrlen);
int iperf_socket(int protocol);
int iperf_recv_from_blocked(int socket, void *buf, size_t len, int flags);
int iperf_recv_from_timeout(int socket, void *buf, size_t len, int flags, int timeout_ms);
int iperf_recv_noblock(int socket, void *buf, size_t len, int flags);
int iperf_recv_blocked(int socket, void *buf, size_t len, int flags);
int iperf_recv_timeout(int socket, void *buf, size_t len, int flags, int timeout_ms);
int iperf_socket_close(int socket);

#endif
