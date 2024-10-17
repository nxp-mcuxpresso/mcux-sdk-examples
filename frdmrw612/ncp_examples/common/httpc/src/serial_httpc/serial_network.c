/*
* Copyright 2024 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
* The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
*/

#include "serial_network.h"
#include "serial_errno.h"
#include "httpc.h"
#include <websockets.h>
#include <stdlib.h>
#include <stdio.h>

nw_conn_t nw_handles[MAX_HANDLES];

int is_connected(int handle)
{
    int index = 0;
    while (index < MAX_HANDLES)
    {
        if (nw_handles[index].type == SOCKET_HANDLE)
        {
            if (nw_handles[index].h.socket.handle == handle)
            {
                return nw_handles[index].state.is_connected;
            }
        }
        index++;
    }
    return -1;
}

int is_bind(int handle)
{
    int index = 0;
    while (index < MAX_HANDLES)
    {
        if (nw_handles[index].type == SOCKET_HANDLE)
        {
            if (nw_handles[index].h.socket.handle == handle)
            {
                return nw_handles[index].state.is_bind;
            }
        }
        index++;
    }
    return -1;
}

int set_connected_state(int handle, bool val)
{
    int index = 0;
    while (index < MAX_HANDLES)
    {
        if (nw_handles[index].type == SOCKET_HANDLE)
        {
            if (nw_handles[index].h.socket.handle == handle)
            {
                nw_handles[index].state.is_connected = (unsigned int)val;
                return 0;
            }
        }
        index++;
    }
    return -1;
}

int set_bind_state(int handle, bool val)
{
    int index = 0;
    while (index < MAX_HANDLES)
    {
        if (nw_handles[index].type == SOCKET_HANDLE)
        {
            if (nw_handles[index].h.socket.handle == handle)
            {
                nw_handles[index].state.is_bind = (unsigned int)val;
                return 0;
            }
        }
        index++;
    }
    return -1;
}

int get_empty_handle_index()
{
    int i;
    for (i = 0; i < MAX_HANDLES; i++)
    {
        if (nw_handles[i].type == DEFAULT_HANDLE)
            return i;
    }
    return -1;
}

int get_handle(int index)
{
    if (index < 0 || index >= MAX_HANDLES)
        return -1;
    if (nw_handles[index].type == HTTPC_HANDLE)
        return nw_handles[index].h.http.handle;
    else if (nw_handles[index].type == SOCKET_HANDLE)
        return nw_handles[index].h.socket.handle;
    return -1;
}

int *get_handle_p(int index)
{
    if (index < 0 || index >= MAX_HANDLES)
        return NULL;
    if (nw_handles[index].type == HTTPC_HANDLE)
        return &(nw_handles[index].h.http.handle);
    else if (nw_handles[index].type == SOCKET_HANDLE)
        return &(nw_handles[index].h.socket.handle);
    return NULL;
}

ws_context_t *get_http_context_p(int index)
{
    if (index < 0 || index >= MAX_HANDLES)
        return NULL;
    if (nw_handles[index].type == HTTPC_HANDLE)
        return &(nw_handles[index].h.http.ws_ctx);
    return NULL;
}

conn_type_t get_sock_type(int index)
{
    if (index < 0 || index >= MAX_HANDLES)
        return none;
    if (nw_handles[index].type == SOCKET_HANDLE)
        return nw_handles[index].h.socket.conn;
    return none;
}

int set_http_handle(http_session_t handle, int index)
{
    if (index < 0 || index >= MAX_HANDLES)
        return -1;
    if (nw_handles[index].type != DEFAULT_HANDLE)
        return -2;
    int status;

    nw_handles[index].h.http.handle = handle;
    nw_handles[index].type          = HTTPC_HANDLE;

    status = OSA_SemaphoreCreateBinary((osa_semaphore_handle_t)(nw_handles[index].handle_sem));
    return status;
}

int set_socket_handle(int handle, conn_type_t type, int index)
{
    if (index < 0 || index >= MAX_HANDLES)
        return -1;
    if (nw_handles[index].type != DEFAULT_HANDLE)
        return -2;
    int status;

    nw_handles[index].h.socket.handle = handle;
    nw_handles[index].h.socket.conn   = type;
    nw_handles[index].type            = SOCKET_HANDLE;

    status = OSA_SemaphoreCreateBinary((osa_semaphore_handle_t)(nw_handles[index].handle_sem));
    return status;
}

int remove_handle(int index)
{
    if (index < 0 || index >= MAX_HANDLES)
        return -1;
    if (nw_handles[index].type == DEFAULT_HANDLE)
        return -2;
    OSA_SemaphoreDestroy((osa_semaphore_handle_t)(nw_handles[index].handle_sem));
    memset(&nw_handles[index], 0, sizeof(nw_conn_t));
    return 0;
}

nw_conn_t *get_nw_handle_ptr(int index)
{
    if (index < 0 || index >= MAX_HANDLES)
        return NULL;
    return &nw_handles[index];
}
