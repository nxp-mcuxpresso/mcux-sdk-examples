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
#if 1
#define NGINX_SERVER_CRT                                                   \
    "-----BEGIN CERTIFICATE-----\r\n"                                      \
    "MIIDPzCCAicCFBn1CjMKYpUm9sSzA+ykaWxqnlEIMA0GCSqGSIb3DQEBCwUAMFwx\r\n" \
    "CzAJBgNVBAYTAkNOMQswCQYDVQQIDAJTSDELMAkGA1UEBwwCU0gxDDAKBgNVBAoM\r\n" \
    "A05YUDEMMAoGA1UECwwDTlhQMRcwFQYDVQQDDA4xOTIuMTY4LjUwLjE4OTAeFw0y\r\n" \
    "MzAyMTUwNjAyMjlaFw0yNDAyMTUwNjAyMjlaMFwxCzAJBgNVBAYTAkNOMQswCQYD\r\n" \
    "VQQIDAJTSDELMAkGA1UEBwwCU0gxDDAKBgNVBAoMA05YUDEMMAoGA1UECwwDTlhQ\r\n" \
    "MRcwFQYDVQQDDA4xOTIuMTY4LjUwLjE4OTCCASIwDQYJKoZIhvcNAQEBBQADggEP\r\n" \
    "ADCCAQoCggEBALMMLA9tFcQycG5vtb41EP8MoDT/W5Jdc8zYBClfcsiOmyoo2vRx\r\n" \
    "AtQVD1WzERf+668sd7ezz2YxdxEPK3K77lOPB4hErL7WAPQ/9H0qjyX4NO7m3oZ5\r\n" \
    "gNIbdPVCip53tfhlZ5n9mTTPurDXtaesPRX2l+rnnEIbcZb80Qo7xvEkRSXZmD7P\r\n" \
    "QmncV1pSl+r2e1dVowb02vyPiP3JkAkPvrfg4qDulw7hV2h0qD1DEz+rTrUoaNPI\r\n" \
    "DniHUl/QQCvKPg9YysXCaB42TBhgoJN1eqvOr4upacvPjjSrV5r/DhO1zrtTzcKN\r\n" \
    "7q06vyA0+F37WEePRl2wovsiUKINbNcPehkCAwEAATANBgkqhkiG9w0BAQsFAAOC\r\n" \
    "AQEAkj8h1M5gqF0yhMq+56mcVa6TVapLsUOFo7p5jlEk/riAP2ySeVFf2PnOdWYT\r\n" \
    "qJn4ZEmR1Qeou2F4JZ/wiX/Ri7QXHgP+uv2FqT0NSVTiRKScXjP7Iq7JwgKAx+DB\r\n" \
    "Y6LCYJJ68eConq29r5wywc9crxTsaZCeaUJej7gBRYvpPuODl8Sh7G/7/trprEjL\r\n" \
    "vQaQHGRWoa0F9b7M0YEII/hFkOmC55j3+Ytwth2x1sVcJW0BGQIvxB9Zc6ySWhy7\r\n" \
    "xK0+Ku2jRnO8S3qPaA14It6nvfZRmW8pvhgSSfCGkWIspa+svlikWi3e16uY1kpt\r\n" \
    "MTZHqg6PYRwzn9bRO9fzLIlrKA==\r\n"                                     \
    "-----END CERTIFICATE-----\r\n"

#else

#define NGINX_SERVER_CRT                                                   \
    "-----BEGIN CERTIFICATE-----\r\n"                                      \
    "MIIDNjCCAh4CCQCVj0rMfE1o2zANBgkqhkiG9w0BAQsFADBdMQswCQYDVQQGEwJD\r\n" \
    "TjELMAkGA1UECAwCU0gxCzAJBgNVBAcMAlNIMQwwCgYDVQQKDANueHAxDDAKBgNV\r\n" \
    "BAsMA254cDEYMBYGA1UEAwwPMTkyLjE2OC4yMDMuMjI0MB4XDTIzMDExODA4MDIx\r\n" \
    "MFoXDTIzMDIxNzA4MDIxMFowXTELMAkGA1UEBhMCQ04xCzAJBgNVBAgMAlNIMQsw\r\n" \
    "CQYDVQQHDAJTSDEMMAoGA1UECgwDbnhwMQwwCgYDVQQLDANueHAxGDAWBgNVBAMM\r\n" \
    "DzE5Mi4xNjguMjAzLjIyNDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\r\n" \
    "AKhtYnMkhdT924DCuRwm8ybGO8zq0j+COwkAL6gL2mhNW+zO0aYpTT8wCOeHlVhR\r\n" \
    "8HYSdNI48eIlYUInwA9RzA3nCOYpLyq3QiSEYuLy5ZF1MezWdGZGuBe/vz+BYg4W\r\n" \
    "APu7AWIZ6yzYjGijvycLrKHY03HibkH89wCz9I9h+5RmzKz94UORYzhDGm71Bo15\r\n" \
    "Ww3LpOb+PTgcYGAe2DzaYWqzdyjnR2a9UkETos27+1FC4kScUM4h/Qs5pXCcMRut\r\n" \
    "4NCo9OMCfclgCrVUIpahhFpJiqij5vvdUBDEE65zPb4BXVpddgQ117erBS2hyMs9\r\n" \
    "KgVXTBD/38xtAdfqQJhnqX0CAwEAATANBgkqhkiG9w0BAQsFAAOCAQEAjDcMBkBJ\r\n" \
    "ABAo9UoyBcIF0epOSHcgAFwL2Is5Vn8Yf5Vu8H7NY6U4vetgo6f9I0iY7DoPtsed\r\n" \
    "DicnepSsEnxtlt/osyyZVCF7+WjipF2bsdPERkk+S3bvz4d+u5ytRYuVI49+X1u5\r\n" \
    "Te+AzvekWbFbMnoTMNYMknBrK+9wYV3K6iM9G8ZX5xr8LaBNrED0Y1prdWv1KnS+\r\n" \
    "To//YNUoAxo/zf9iM5bh4apOWNnSf9D4Zkd/67ej1d78ccTYAHbSce4tQazs2tXd\r\n" \
    "LC7qdQA4F2hBCNLZxPfnH5Zgh22n1KBLz9RkquEt2CurfuQSnmyi2fz/xSohZqLO\r\n" \
    "KbMFEo+9BN3+eA==\r\n"                                                 \
    "-----END CERTIFICATE-----\r\n"
#endif

#define NGINX_SERVER_CRT_LEN sizeof(NGINX_SERVER_CRT)

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
