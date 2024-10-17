/*
* Copyright 2024 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
* The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
*/

#include "serial_httpc.h"
#include <stdint.h>
#include "serial_socket.h"
#include "osa.h"

enum mwm_handle_type
{
    DEFAULT_HANDLE,
    HTTPC_HANDLE,
    SOCKET_HANDLE,
};

#define MAX_HANDLES 8
typedef struct
{
    uint8_t type;
    OSA_SEMAPHORE_HANDLE_DEFINE(handle_sem);
    union
    {
        http_handle_t http;
        socket_handle_t socket;
    } h;
    struct
    {
        unsigned int is_connected : 1;
        unsigned int is_bind : 1;
    } state;
} nw_conn_t;

int get_empty_handle_index();
int get_handle(int index);
int *get_handle_p(int index);
ws_context_t *get_http_context_p(int index);
int set_http_handle(http_session_t handle, int index);
int set_socket_handle(int handle, conn_type_t type, int index);
int remove_handle(int index);
conn_type_t get_sock_type(int index);
nw_conn_t *get_nw_handle_ptr(int index);
int is_connected(int handle);
int set_connected_state(int handle, bool val);
int is_bind(int handle);
int set_bind_state(int handle, bool val);
