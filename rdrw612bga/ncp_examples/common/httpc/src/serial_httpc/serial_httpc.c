/** @file httpc_test.c
 *
 * @brief This file provides CLI based tests for HTTP Client.
 *
 *  Copyright 2008-2020 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "cli.h"
#include "httpc.h"
#include <stdio.h>
#if !(defined(__ARMCC_VERSION) || defined(__ICCARM__))
#include <strings.h>
#endif
#include "serial_network.h"
#include "serial_socket.h"
#include "serial_httpc.h"
#include "cli_utils.h"
#include "wm_mbedtls_helper_api.h"
#include "serial_errno.h"

#define HTTP_RETRY_CNT 3
#define HTTP_FLAGS     0

static http_header_t headers[MAX_HEADERS];
static int no_of_headers;

static int set_header(const char *name, const char *value)
{
    int i;
    char *nameptr = NULL, *valptr = NULL;
    if (no_of_headers == MAX_HEADERS)
    {
        /* max limit reached */
        return -E_FAIL;
    }
    if (name == NULL || value == NULL)
    {
        return -E_FAIL;
    }
    for (i = 0; i < MAX_HEADERS; i++)
    {
        if ((headers[i].name && !strcasecmp(name, headers[i].name)) &&
            (headers[i].value && !strcasecmp(value, headers[i].value)))
        {
            /* duplicate name and value */
            break;
        }
        if (headers[i].name && !strcasecmp(name, headers[i].name))
        {
            /* same header name with different value overwrites */
            /* previous header value for same header name */
            valptr = (char *)OSA_MemoryAllocate(strlen(value) + 1);
            if (!valptr)
            {
                return -E_FAIL;
            }
            memcpy(valptr, value, strlen(value));
            valptr[strlen(value)] = '\0';
            OSA_MemoryFree(headers[i].value);
            headers[i].value = valptr;
            break;
        }
    }
    if (i == MAX_HEADERS)
    {
        for (i = 0; i < MAX_HEADERS; i++)
        {
            if (headers[i].name == NULL)
            {
                nameptr = (char *)OSA_MemoryAllocate(strlen(name) + 1);
                if (!nameptr)
                {
                    return -E_FAIL;
                }
                memcpy(nameptr, name, strlen(name));
                nameptr[strlen(name)] = '\0';
                headers[i].name       = nameptr;

                valptr = (char *)OSA_MemoryAllocate(strlen(value) + 1);
                if (!valptr)
                {
                    OSA_MemoryFree(nameptr);
                    nameptr = NULL;
                    return -E_FAIL;
                }
                memcpy(valptr, value, strlen(value));
                valptr[strlen(value)] = '\0';
                OSA_MemoryFree(headers[i].value);
                headers[i].value = valptr;
                no_of_headers += 1;
                break;
            }
        }
    }
    return 0;
}

static int unset_header(const char *name)
{
    int i;
    if (name == NULL)
        return -E_FAIL;
    for (i = 0; i < MAX_HEADERS; i++)
    {
        if (headers[i].name && !(strcasecmp(name, headers[i].name)))
        {
            OSA_MemoryFree(headers[i].name);
            headers[i].name = NULL;
            OSA_MemoryFree(headers[i].value);
            headers[i].value = NULL;
            no_of_headers -= 1;
            break;
        }
    }
    if (i == MAX_HEADERS)
    {
        return -E_FAIL;
    }
    return 0;
}

static int insert_header(http_session_t handle, const http_req_t *req, uint8_t method)
{
    int count_header;
    int status = 0;
    for (count_header = 0; count_header < MAX_HEADERS; count_header++)
    {
        if (headers[count_header].name && headers[count_header].value)
        {
            if ((method != HTTP_POST && method != HTTP_PUT) &&
                !(strcasecmp(headers[count_header].name, "content-type")))
                continue;
            status |= http_add_header(handle, req, headers[count_header].name, headers[count_header].value);
        }
    }
    return status;
}

int ncp_http_setheader(char *name, char *value)
{
    const char *hname, *hval;
    int status;

    /* header name */
    hname = name;
    if (*hname == '\0')
    {
        httpc_e("status:invalid params");
        return -E_FAIL;
    }
    /* header value */
    hval = value;
    if (*hval == '\0')
    {
        httpc_e("status:invalid params");
        return -E_FAIL;
    }

    status = set_header(hname, hval);
    if (status != E_SUCCESS)
    {
        httpc_e("status:unable to set header", NULL);
        return -E_FAIL;
    }
    httpc_d("SUCCESS");
    return E_SUCCESS;
}

int ncp_http_unsetheader(char *name)
{
    const char *hname = name;
    int status;
    status = unset_header(hname);
    if (status != E_SUCCESS)
    {
        httpc_e("status:unable to unset header", NULL);
        return -E_FAIL;
    }
    httpc_d("SUCCESS");
    return E_SUCCESS;
}

#define HTTP_RETRY_CNT 3
#define HTTP_FLAGS     0
int ncp_http_connect(char *host)
{
    int rc = WM_SUCCESS;
    http_session_t handle;
    int h_slot;
#if CONFIG_ENABLE_HTTPC_SECURE
    uint8_t *cert = 0;
    int cert_len  = 0;
    char cert_partition[6];
    uint8_t https_conn_attempts = 0;

tryagain:
    cert     = (uint8_t *)NGINX_CA_CRT;
    cert_len = NGINX_CA_CRT_LEN;
    if (cert == NULL)
        cert_len = 0;
#endif
    /* host */
    if (host && *host == '\0')
    {
        httpc_e("status:invalid params", NULL);
        return -E_FAIL;
    }

    h_slot = get_empty_handle_index();
    if (h_slot < 0)
    {
        httpc_e("status:max handles reached");
        return -E_FAIL;
    }

    httpc_cfg_t httpc_cfg;
    memset(&httpc_cfg, 0x00, sizeof(httpc_cfg_t));
    httpc_cfg.flags     = HTTP_FLAGS;
    httpc_cfg.retry_cnt = HTTP_RETRY_CNT;
#if CONFIG_ENABLE_HTTPC_SECURE
    https_conn_attempts++;
    wm_mbedtls_cert_t wm_cert;
    memset(&wm_cert, 0, sizeof(wm_cert));
    wm_cert.ca_chain = wm_mbedtls_parse_cert((unsigned char *)cert, cert_len);
    httpc_cfg.ctx    = wm_mbedtls_ssl_config_new(&wm_cert, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_VERIFY_REQUIRED);
    if (!httpc_cfg.ctx)
    {
        httpc_e("status:tls context create failed");
        return -E_FAIL;
    }
#endif /* CONFIG_ENABLE_HTTPC_SECURE */
    rc = http_open_session(&handle, host, &httpc_cfg);
    if (rc == -WM_E_HTTPC_TLS_NOT_ENABLED)
    {
        httpc_e("status:cannot create session, tls is not enabled.");
#if CONFIG_ENABLE_HTTPC_SECURE
        if (httpc_cfg.ctx)
        {
            wm_mbedtls_ssl_config_free(httpc_cfg.ctx);
            wm_mbedtls_free_cert(wm_cert.ca_chain);
        }
#endif
        return -E_FAIL;
    }
    if (rc != WM_SUCCESS)
    {
#if CONFIG_ENABLE_HTTPC_SECURE
        if (httpc_cfg.ctx)
        {
            wm_mbedtls_ssl_config_free(httpc_cfg.ctx);
            wm_mbedtls_free_cert(wm_cert.ca_chain);
        }

        if (https_conn_attempts < 2)
        {
            int size = sizeof(cert_partition);
            if (strncmp(cert_partition, "cert0", size) == 0)
            {
                strcpy(cert_partition, "cert1");
            }
            else if (strncmp(cert_partition, "cert1", size) == 0)
            {
                strcpy(cert_partition, "cert0");
            }
            /*
                        if (cert)
                            OSA_MemoryFree(cert);
            */
            goto tryagain;
        }
#endif
        httpc_e(
            "status:cannot create session, "
            "http connect failed",
            NULL);
        return -E_FAIL;
    }

    rc = set_http_handle(handle, h_slot);
    if (rc != WM_SUCCESS)
    {
        http_close_session(&handle);
#if CONFIG_ENABLE_HTTPC_SECURE
        if (httpc_cfg.ctx)
        {
            wm_mbedtls_ssl_config_free(httpc_cfg.ctx);
            wm_mbedtls_free_cert(wm_cert.ca_chain);
        }

#endif
        httpc_e(
            "status:internal error"
            " setting handle",
            NULL);
        return -E_FAIL;
    }
    httpc_d("SUCCESS");
    httpc_d("handle:%d", (uint32_t)h_slot);
#ifndef HTTP_DEPRECATED
    serial_mwm_register_recv_cb(http_get_sockfd_from_handle(handle), (void *)h_slot);
#endif
    return (int)h_slot;
}

int ncp_http_disconnect(uint32_t handle)
{
    int h_slot;
    http_session_t hs;
    ws_context_t *ws_ctx;

    /* handle */
    h_slot = handle;
    hs     = get_handle(h_slot);
    if (hs < 0)
    {
        httpc_e("status:invalid handle", NULL);
        return -E_FAIL;
    }

    ws_ctx = get_http_context_p(h_slot);
    if (*ws_ctx)
    {
        ws_close_ctx(ws_ctx);
        ws_ctx = NULL;
    }

#if CONFIG_ENABLE_HTTPC_SECURE
    mbedtls_ssl_config *ctx = http_get_tls_context_from_handle(hs);
#endif
    http_close_session(&hs);
#if CONFIG_ENABLE_HTTPC_SECURE
    if (ctx)
        wm_mbedtls_ssl_config_free(ctx);
#endif
    remove_handle(h_slot);

    httpc_d("SUCCESS");
    return E_SUCCESS;
}

static void flush_outstanding_data(http_session_t handle)
{
    char temp_buf[10];
    while (http_read_content(handle, temp_buf, sizeof(temp_buf)) > 0)
        ;
}

/*  - $$ indicates start of datamode
 *  - User may pump in as much data as he wants to (ascii/binary)
 *  - '\' (backslash) is used to escape all '$'s and '\'s that
 *    are a part of the data to be posted
 *  - Subsequent $$ indicates end of datamode */

#define ASC_DATATYPE "a"

int post_data_chunked(post_data_func_t post_this, char *send_data, int send_size, void *arg, int max_chunk_size)
{
    int status   = 0;
    int send_pos = 0;
    int sendnum  = send_size / max_chunk_size;
    for (int i = 0; i < sendnum + 1; i++)
    {
        if (sendnum == i)
            status = post_this(send_data, send_size - send_pos, arg, DATA_DONE);
        else
            status = post_this(send_data, max_chunk_size, arg, DATA_CONTINUE);
        if (status != 0)
        {
            return status;
        }
        send_pos += max_chunk_size;
    }
    /*send null chunk packet to finish the chunk.*/
    httpc_write_chunked((int)arg, NULL, 0);
    return 0;
}

int post_single_http_chunk(char *data, unsigned len, void *arg, int data_chunk)
{
    http_session_t handle = (http_session_t)arg;
    return httpc_write_chunked(handle, data, len);
}

#define MAX_RESP_HDR 16
/*
argv[0]: handle;
argv[1]: method;
argv[2]: uri;
argv[3]: req_data;
argv[4]: req_size;
argv[5]: recv_header;
*/
int ncp_http_request(uint32_t handle, char *s_method, char *uri, uint32_t req_size, char *req_data, char *recv_header)
{
    const char *str_val;
    int h_slot;
    http_session_t hs;
    uint8_t method;
    const char *reqdata = req_data;
    http_req_t req;
    http_header_pair_t *header_arr;
    int header_arr_count = MAX_RESP_HDR;
    int arr_index        = 0;
    int status;
    int send_size   = req_size;
    bool datamode   = false;
    int ret_size    = 0;
    /* handle */
    h_slot = handle;
    hs     = get_handle(h_slot);
    if (hs < 0)
    {
        httpc_e("status:invalid handle", NULL);
        return -E_FAIL;
    }
    /* Set socket to blocking mode */
    struct timeval timeout = {0, 0};
    status      = http_setsockopt(hs, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if (status != WM_SUCCESS)
    {
        httpc_e("status:cannot set recv timeout");
        return -E_FAIL;
    }
    /* method */
    str_val = s_method;
    if (*str_val == '\0')
    {
        httpc_e("status:invalid params", NULL);
        return -E_FAIL;
    }

    if (strcasecmp(str_val, "get") == 0)
    {
        method = HTTP_GET;
    }
    else if (strcasecmp(str_val, "post") == 0)
    {
        method = HTTP_POST;
    }
    else if (strcasecmp(str_val, "put") == 0)
    {
        method = HTTP_PUT;
    }
    else if (strcasecmp(str_val, "delete") == 0)
    {
        method = HTTP_DELETE;
    }
    else if (strcasecmp(str_val, "option") == 0)
    {
        method = HTTP_OPTIONS;
    }
    else if (strcasecmp(str_val, "head") == 0)
    {
        method = HTTP_HEAD;
    }
    else
    {
        httpc_e("status:invalid http method", NULL);
        return -E_FAIL;
    }

    /* In the case of method=get, data and datatype are not
     *  required, hence not mandatory. Else the parameter 'uri'
     *  is expected to be followed by 'datatype' and 'data'
     */
    if (reqdata && send_size > 0)
        datamode = true;

    req.type        = (http_method_t)method;
    req.resource    = uri;
    req.version     = HTTP_VER_1_1;
    req.content     = NULL;
    req.content_len = 0;
    flush_outstanding_data(hs);
    status =
        (datamode == false) ?
            http_prepare_req(hs, &req, (http_hdr_field_sel_t)(STANDARD_HDR_FLAGS | HDR_ADD_CONN_KEEP_ALIVE)) :
            http_prepare_req(
                hs, &req, (http_hdr_field_sel_t)(STANDARD_HDR_FLAGS | HDR_ADD_CONN_KEEP_ALIVE | HDR_ADD_TYPE_CHUNKED));
    if (status == -WM_E_INVAL)
    {
        httpc_e("status:invalid parameter, cannot prepare http request");
        goto end;
    }
    else if (status != WM_SUCCESS)
    {
        httpc_e("status:unknown error, cannot prepare http request");
        goto end;
    }

    status = insert_header(hs, &req, method);
    if (status != WM_SUCCESS)
    {
        httpc_e("status:error while adding header");
        goto end;
    }

    status = http_send_request(hs, &req);
    if (status == -WM_E_INVAL)
    {
        httpc_e("status:invalid parameter, cannot send http request");
        goto end;
    }
    else if (status == -WM_E_IO)
    {
        httpc_e("status:i/o error,cannot send http request");
        goto end;
    }
    else if (status != WM_SUCCESS)
    {
        httpc_e("status:unknown error, cannot send http request");
        goto end;
    }

    if (datamode)
    {
        /* post data */
        status = post_data_chunked(post_single_http_chunk, (char *)reqdata, send_size, (void *)hs, MAX_CHUNK_LEN);
        /* Put the uart semaphore after data mode */
        if (status == -E_FAIL)
        {
            httpc_e("status:http post failed, malformed data");
            return -E_FAIL;
        }
        if (status != E_SUCCESS)
        {
            httpc_e("status:http post failed");
            return -E_FAIL;
        }
    }

    header_arr = OSA_MemoryAllocate(header_arr_count * sizeof(http_header_pair_t));
    if (header_arr == NULL)
    {
        httpc_d("status:out of memory", NULL);
        return -E_FAIL;
    }
    status = http_get_response_hdr_all(hs, header_arr, &header_arr_count);
    if (status == -WM_E_INVAL)
    {
        OSA_MemoryFree(header_arr);
        header_arr = NULL;
        httpc_e(
            "status:invalid parameter, "
            "unable to get response header",
            NULL);
        return -E_FAIL;
    }
    else if (status == -WM_E_IO)
    {
        OSA_MemoryFree(header_arr);
        header_arr = NULL;
        httpc_e("status:i/o error, unable to get response header");
        return -E_FAIL;
    }
    else if (status == -WM_FAIL)
    {
        OSA_MemoryFree(header_arr);
        header_arr = NULL;
        httpc_e("status:invalid response");
        return -E_FAIL;
    }
    else if (status != WM_SUCCESS)
    {
        OSA_MemoryFree(header_arr);
        header_arr = NULL;
        httpc_e("status:unknown error, unable to get response header");
        return -E_FAIL;
    }
    httpc_d("SUCCESS");
    while (arr_index < header_arr_count)
    {
        httpc_d("%s:%s", header_arr[arr_index].name, header_arr[arr_index].value);
        if (recv_header)
        {
            memcpy(recv_header, header_arr[arr_index].name, strlen(header_arr[arr_index].name) + 1);
            recv_header += strlen(header_arr[arr_index].name) + 1;
            ret_size += strlen(header_arr[arr_index].name) + 1;
            memcpy(recv_header, header_arr[arr_index].value, strlen(header_arr[arr_index].value) + 1);
            recv_header += strlen(header_arr[arr_index].value) + 1;
            ret_size += strlen(header_arr[arr_index].value) + 1;
        }
        arr_index++;
    }
    OSA_MemoryFree(header_arr);
    header_arr = NULL;
    return ret_size;
end:
    return -E_FAIL;
}

int ncp_http_recv(uint32_t handle, uint32_t recv_size, uint32_t timeout, char *recv_data)
{
    int h_slot;
    http_session_t hs;
    int ret_len;
    uint8_t *outbuf;
    int size;
    int status;

    h_slot = handle;
    hs     = get_handle(h_slot);
    if (hs < 0)
    {
        httpc_e("status:invalid handle", NULL);
        return -E_FAIL;
    }
    size = recv_size;
    /* timeout in milliseconds (0 means the receive
       call will not time out) */
    outbuf = (uint8_t *)recv_data;
    if (!outbuf)
    {
        httpc_e("status:out of memory", NULL);
        return -E_FAIL;
    }
    struct timeval timeo = {timeout / 1000, (timeout % 1000) * 1000};
    status = http_setsockopt(hs, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo));
    if (status != WM_SUCCESS)
    {
        outbuf = NULL;
        httpc_e("status:cannot set recv timeout");
        return -E_FAIL;
    }
    ret_len = http_read_content(hs, outbuf, size);
    if (ret_len < 0)
    {
        outbuf = NULL;
        if (ret_len == -WM_E_HTTPC_SOCKET_ERROR)
        {
            httpc_e("status:error in reading data");
            httpc_e("errno:%d", errno);
            return -E_FAIL;
        }
        else if (ret_len == -WM_E_HTTPC_SOCKET_SHUTDOWN)
        {
            httpc_e("status:peer has performed orderly shutdown");
            return -E_FAIL;
        }
        else
        {
            httpc_e("status:unkown error in reading data");
            return -E_FAIL;
        }
    }
    httpc_d("SUCCESS");
    httpc_d("content-len:%d", ret_len);
    if (ret_len)
    {
        httpc_d("%s\n", outbuf);
    }
    outbuf = NULL;
    /* Put the semaphore to allow next asynchronous event to be scheduled */
    OSA_SemaphorePost(get_nw_handle_ptr(h_slot)->handle_sem);
    return ret_len;
}

#if APPCONFIG_WEB_SOCKET_SUPPORT
int ncp_ws_upg(uint32_t handle, const char *uri, const char *proto)
{
    int h_slot;
    http_session_t *hs;
    ws_context_t *ctx;
    http_req_t req;
    int status;

    h_slot = handle;
    hs     = get_handle_p(h_slot);
    if (!hs)
    {
        httpc_e("status:invalid handle", NULL);
        return -E_FAIL;
    }
    /* Set socket to blocking mode */
    struct timeval timeo = {0, 0};
    status      = http_setsockopt(*hs, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo));
    if (status != WM_SUCCESS)
    {
        httpc_e("status:cannot set recv timeout");
        return -E_FAIL;
    }
    ctx = get_http_context_p(h_slot);

    req.type        = HTTP_GET;
    req.resource    = uri;
    req.version     = HTTP_VER_1_1;
    req.content     = NULL;
    req.content_len = 0;

    status = http_prepare_req(*hs, &req, STANDARD_HDR_FLAGS);
    if (status == -WM_E_INVAL)
    {
        httpc_e("status:invalid parameter, cannot prepare http request");
        return -E_FAIL;
    }
    else if (status != WM_SUCCESS)
    {
        httpc_e("status:unknown error, cannot prepare http request");
        return -E_FAIL;
    }

    status = ws_upgrade_socket(hs, &req, proto, ctx);
    if (status == -WM_E_INVAL)
    {
        httpc_e("status:invalid parameter, cannot upgrade to web sockets");
        return -E_FAIL;
    }
    else if (status == -WM_E_IO)
    {
        httpc_d("status:i/o error, cannot upgrade to web sockets");
        return -E_FAIL;
    }
    else if (status == -WM_FAIL)
    {
        httpc_e("status:WS upgrade failed, cannot upgrade to web sockets");
        return -E_FAIL;
    }
    else if (status != WM_SUCCESS)
    {
        httpc_e("status:unknown error, cannot upgrade to web sockets");
        return -E_FAIL;
    }

    return E_SUCCESS;
}

struct ws_data_wrapper
{
    ws_context_t *ctx;
    ws_frame_t *f;
};

int post_data_on_ws(char *data, unsigned len, void *arg, int data_chunk)
{
    struct ws_data_wrapper *wd = (struct ws_data_wrapper *)arg;
    ws_context_t *ctx          = wd->ctx;
    ws_frame_t *f              = wd->f;

    if (data_chunk == DATA_DONE)
        f->fin = 1;
    else
        f->fin = 0;

    f->data     = (uint8_t *)data;
    f->data_len = len;

    int ret = ws_frame_send(ctx, f);
    if (ret < 0)
        return -WM_FAIL;
    /* This indicates that data is being sent in fragments
       hence the opcode should be set to zero */
    if (data_chunk == DATA_CONTINUE)
        f->opcode = 0;

    return WM_SUCCESS;
}

#define TEXT_FRM_STR "text"
#define BIN_FRM_STR  "bin"
#define PING_FRM_STR "ping"
#define PONG_FRM_STR "pong"
#define CONT_FRM_STR "cont"

int get_ws_str_to_opcode(const char *str_val, ws_opcode_t *opcode)
{
    if (strcmp(TEXT_FRM_STR, str_val) == 0)
        *opcode = WS_TEXT_FRAME;
    else if (strcmp(BIN_FRM_STR, str_val) == 0)
        *opcode = WS_BIN_FRAME;
    else if (strcmp(PING_FRM_STR, str_val) == 0)
        *opcode = WS_PING_FRAME;
    else if (strcmp(PONG_FRM_STR, str_val) == 0)
        *opcode = WS_PONG_FRAME;
    else if (strcmp(CONT_FRM_STR, str_val) == 0)
        *opcode = WS_CONT_FRAME;
    else
        return -WM_FAIL;

    return WM_SUCCESS;
}

char *get_ws_opcode_to_str(ws_opcode_t opcode)
{
    switch (opcode)
    {
        case WS_TEXT_FRAME:
            return TEXT_FRM_STR;
            break;
        case WS_BIN_FRAME:
            return BIN_FRM_STR;
            break;
        case WS_PING_FRAME:
            return PING_FRM_STR;
            break;
        case WS_PONG_FRAME:
            return PONG_FRM_STR;
            break;
        case WS_CONT_FRAME:
            return CONT_FRM_STR;
        default:
            return NULL;
            break;
    }
}

int ncp_ws_send(uint32_t handle, char *type, uint32_t size, char *send_data)
{
    const char *str_val;
    int h_slot;
    ws_context_t *ctx;
    ws_frame_t f;
    const char *reqdata = NULL;
    int status;
    int send_size = size;

    /* handle */
    h_slot = handle;
    ctx    = get_http_context_p(h_slot);
    if (!ctx || !*ctx)
    {
        httpc_e("status:invalid handle", NULL);
        return -E_FAIL;
    }

    /* opcode */
    str_val = type;
    if (*str_val == '\0')
    {
        httpc_e("status:invalid params", NULL);
        return -E_FAIL;
    }
    status = get_ws_str_to_opcode(str_val, (ws_opcode_t *)&f.opcode);
    if (status != WM_SUCCESS)
    {
        httpc_e("status:invalid params", NULL);
        return -E_FAIL;
    }

    /* data */
    reqdata = send_data;
    if (!reqdata)
    {
        httpc_e("status:invalid data", NULL);
        return -E_FAIL;
    }

    /* Only opcode member in the frame is populated at this point. All
     * others are updated in the post_data_on_ws() callback.
     */
    struct ws_data_wrapper ws_wrap;
    ws_wrap.ctx = ctx;
    ws_wrap.f   = &f;

    /* post data */
    status = post_data_chunked(post_data_on_ws, (char *)reqdata, send_size, &ws_wrap, MAX_CHUNK_LEN);

    if (status != E_SUCCESS)
    {
        httpc_e("status:http post failed");
        return -E_FAIL;
    }

    return E_SUCCESS;
}

int ncp_ws_recv(uint32_t handle, uint32_t recv_size, uint32_t timeout, uint32_t *fin, char *recv_data)
{
    int h_slot;
    ws_context_t *ctx;
    ws_frame_t f;
    http_session_t *hs;
    int ret_len;
    int status;

    /* handle */
    h_slot = handle;
    ctx    = get_http_context_p(h_slot);
    if (!ctx || !*ctx)
    {
        httpc_e("status:invalid handle", NULL);
        return -E_FAIL;
    }

    hs = get_handle_p(h_slot);
    if (!hs)
    {
        httpc_e("status:invalid handle", NULL);
        return -E_FAIL;
    }
    /* timeout in milliseconds (0 means the receive
       call will not time out) */
    struct timeval timeo = {timeout / 1000, (timeout % 1000) * 1000};
    status = http_setsockopt(*hs, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo));
    if (status != WM_SUCCESS)
    {
        httpc_e("status:cannot set recv timeout");
        return -E_FAIL;
    }

    f.opcode = -1;
    ret_len  = ws_frame_recv(ctx, &f);
    if (ret_len < 0 || (f.fin != 0 && f.fin != 1))
    {
        if (ret_len == -WM_E_HTTPC_SOCKET_ERROR)
        {
            httpc_e("status:error in reading data");
            httpc_e("errno:%d", errno);
            return -WM_E_HTTPC_SOCKET_ERROR;
        }
        else if (ret_len == -WM_E_HTTPC_SOCKET_SHUTDOWN)
        {
            httpc_e("status:peer has performed orderly shutdown");
            return -WM_E_HTTPC_SOCKET_SHUTDOWN;
        }
        else
        {
            httpc_e("status:unkown error in reading data");
            return -1;
        }
    }
    httpc_d("content-len:%d", ret_len);
    /* ws_frame_recv returns the length of payload data. Sometimes,
     * ping frames etc. are received with zero payload but with
     * valid header. Hence we set the opcode field to 0 before
     * calling ws_frame_recv, and use that to determine if new data
     * is received. */
    httpc_d("final:%d", f.fin);
    httpc_d("opcode:%s", get_ws_opcode_to_str((ws_opcode_t)f.opcode));
    memcpy(recv_data, f.data, ret_len);
    *fin = f.fin;
    return ret_len;
}
#endif /* APPCONFIG_WEB_SOCKET_SUPPORT */
