/*
* Copyright 2024 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
* The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
*/

#ifndef _SERIAL_MWM_HTTPC_H_
#define _SERIAL_MWM_HTTPC_H_
#include "httpc.h"
#include "serial_httpc.h"
#include <websockets.h>
#include "serial_errno.h"

#define HTTPC_FRAME_MAX (4 * 1024)
#define MAX_HEADERS     16
#define MAX_CHUNK_LEN   HTTPC_FRAME_MAX

/** Success */
#define E_SUCCESS 0
#if 0
/** Command formatting error */
#define E_CMD_FORMAT_ERR -2
/** Invalid configuration state for device (configured variable) */
#define E_INVAL_STATE    -3
/** Already exists */
#define E_EXISTS         -4
#endif

#define HTTP_DEPRECATED 1

typedef struct http_header
{
    char *name;
    char *value;
} http_header_t;

typedef struct
{
    http_session_t handle;
    ws_context_t ws_ctx;
} http_handle_t;

typedef int (*post_data_func_t)(char *data, unsigned len, void *arg, int data_chunk);

int post_data_chunked(post_data_func_t post_this, char *send_data, int send_size, void *arg, int max_chunk_size);

#if 0
/*HTTP and WS command type id*/
#define TYPE_HTTP_CONNECT     0x0011001
#define TYPE_HTTP_DISCONNECT  0x0011002
#define TYPE_HTTP_SETHEADER   0x0011003
#define TYPE_HTTP_UNSETHEADER 0x0011004
#define TYPE_HTTP_REQ         0x0011005
#define TYPE_HTTP_RCV         0x0011006
#define TYPE_WS_UPG           0x0011007
#define TYPE_WS_SEND          0x0011008
#define TYPE_WS_RECV          0x0011009
#else
/*HTTP and WS command tmp type id*/
#define TYPE_HTTP_CONNECT     1
#define TYPE_HTTP_DISCONNECT  2
#define TYPE_HTTP_SETHEADER   3
#define TYPE_HTTP_UNSETHEADER 4
#define TYPE_HTTP_REQ         5
#define TYPE_HTTP_RCV         6
#define TYPE_WS_UPG           7
#define TYPE_WS_SEND          8
#define TYPE_WS_RECV          9
#define TYPE_SOCKET_OPEN      10
#define TYPE_SOCKET_CONNECT   11
#define TYPE_SOCKET_RECEIVE   12
#define TYPE_SOCKET_SEND      13
#define TYPE_SOCKET_SENDTO    14
#define TYPE_SOCKET_BIND      15
#define TYPE_SOCKET_LISTEN    16
#define TYPE_SOCKET_ACCEPT    17
#define TYPE_SOCKET_CLOSE     18
#define TYPE_SOCKET_RECVFORM  19

#endif

void httpcmd_dispatch_handle(int type, int argc, const char *value[]);
int serial_httpc_cli_init(void);

/* This is taken from tests/data_files/test-ca-sha1.crt. */
/* BEGIN FILE string macro TEST_CA_CRT_RSA_SHA1_PEM tests/data_files/test-ca-sha1.crt */

/* NOTE:
 * 1) CA valid from 2024/9/5 to 2034/9/5
 * 2) Common Name must be 192.168.50.189
 * */
#define NGINX_CA_CRT                                                       \
    "-----BEGIN CERTIFICATE-----\r\n"                                      \
    "MIIDPzCCAicCFEPsbEntvNtFMnRJNm/KSItl8eHsMA0GCSqGSIb3DQEBCwUAMFwx\r\n" \
    "CzAJBgNVBAYTAkNOMQswCQYDVQQIDAJTSDELMAkGA1UEBwwCU0gxDDAKBgNVBAoM\r\n" \
    "A05YUDEMMAoGA1UECwwDV0NTMRcwFQYDVQQDDA4xOTIuMTY4LjUwLjE4OTAeFw0y\r\n" \
    "NDA5MDUxMDIzMjRaFw0zNDA5MDUxMDIzMjRaMFwxCzAJBgNVBAYTAkNOMQswCQYD\r\n" \
    "VQQIDAJTSDELMAkGA1UEBwwCU0gxDDAKBgNVBAoMA05YUDEMMAoGA1UECwwDV0NT\r\n" \
    "MRcwFQYDVQQDDA4xOTIuMTY4LjUwLjE4OTCCASIwDQYJKoZIhvcNAQEBBQADggEP\r\n" \
    "ADCCAQoCggEBAMfbbbQc/MU5+uUb06fIPp0M32rp3H17A6dfkDe9RZSulMbkqB5a\r\n" \
    "0C3Z5CAuj0/c28e3WDg6MSMqG0l7fGrohR7PQkUHUQfBxGsycRiyNwxgwsE/t6Vg\r\n" \
    "6sQrBou05fAL9lIkJyEptvxGhCB74GtkZLqLrLRyFdxssID/aY8D4PLq6Ekz56va\r\n" \
    "qod+HbpYDF3jl3cQgcWJcA5odks3si+5UnpkpDvXlM6RbpEiqCz6eGIA31wt8GC6\r\n" \
    "TYF97CvoCfNMhcS8Gm0SCWPb9vNPWHvC6PY6Hrfyo5Tg9XGJMMzCYwLDTehJUn6S\r\n" \
    "yo11ocz38F+mLewTUEHXYcDX/Jdkev3Ufo0CAwEAATANBgkqhkiG9w0BAQsFAAOC\r\n" \
    "AQEAUqJfNQg57neb1YmdkMYZTitkVB36GcXnn9LKDRTQFxutYgnSlmGoy80EGv4Z\r\n" \
    "z8a2WnpfhetxpgZjI6WEd3+0c7djg4BoqvSM10AOfokq0mvRhqtDI0B1ykhhN7+0\r\n" \
    "2SP4ngChYtmx1UPdJBk0sq170C+KeX5+EDxDKllZjprD9dGubQJM1+f1gPOiWT4z\r\n" \
    "U+pKnJ4LkutijPGrvCc/HHndOVkTLM0qcHo/mWypq/8aeo6SaLYy+P+aTBHjDV5F\r\n" \
    "c7MbfjKg+02+vxJwCyr+Edsr88DrYiJqpGtZ7cD4xze7bgE+NJ1K6aapGtmz3WOp\r\n" \
    "PQHRpdzTw+rSR9tqSbGmyaBfUQ==\r\n"                                     \
    "-----END CERTIFICATE-----\r\n"

#define NGINX_CA_CRT_LEN sizeof(NGINX_CA_CRT)

enum
{
    DATA_DONE = 1,
    DATA_CONTINUE,
};

int ncp_http_connect(char *host);
int ncp_http_unsetheader(char *name);
int ncp_http_disconnect(uint32_t handle);
int ncp_http_request(uint32_t handle, char *method, char *uri, uint32_t req_size, char *req_data, char *recv_header);
int ncp_http_recv(uint32_t handle, uint32_t recv_size, uint32_t timeout, char *recv_data);
int ncp_http_unsetheader(char *name);
int ncp_http_setheader(char *name, char *value);
int ncp_ws_upg(uint32_t handle, const char *uri, const char *proto);
int ncp_ws_send(uint32_t handle, char *type, uint32_t size, char *send_data);
int ncp_ws_recv(uint32_t handle, uint32_t recv_size, uint32_t timeout, uint32_t *fin, char *recv_data);

#endif
