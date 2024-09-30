/*
* Copyright 2024 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
* The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
*/

#ifndef _SERIAL_MWM_SOCKET_H_
#define _SERIAL_MWM_SOCKET_H_

typedef enum
{
    none,
    tcp,
    udp,
    raw,
} conn_type_t;

typedef struct
{
    int handle;
    conn_type_t conn;
} socket_handle_t;

int ncp_sock_open(const char *socket_type, const char *domain_type, const char *prot);
int ncp_sock_connect(uint32_t handle, uint32_t port, char *ip);
int ncp_sock_receive(uint32_t handle, uint32_t recv_size, uint32_t timeo, char *recv_data);
int ncp_sock_receivefrom(
    uint32_t handle, uint32_t recv_size, uint32_t timeo, char *peer_ip, uint32_t *peer_port, char *recv_data);
int ncp_sock_send(uint32_t handle, uint32_t send_size, char *send_data);
int ncp_sock_sendto(uint32_t handle, char *ip_addr, uint32_t port, uint32_t send_size, char *send_data);
int ncp_sock_bind(uint32_t handle, uint32_t port, char *ip);
int ncp_sock_listen(uint32_t handle, uint32_t number);
int ncp_sock_accept(uint32_t handle);
int ncp_sock_close(uint32_t handle);

#endif
