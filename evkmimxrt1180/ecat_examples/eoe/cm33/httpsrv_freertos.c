/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "httpsrv_freertos.h"

#include "lwip/opt.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#ifndef __REDLIB__
#include <inttypes.h>
#else
#define PRIu32 "u"
#endif

#include "lwip/netif.h"
#include "lwip/sys.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/tcpip.h"
#include "lwip/ip.h"
#include "lwip/netifapi.h"
#include "lwip/sockets.h"
#include "netif/etharp.h"

#include "httpsrv.h"
#include "lwip/apps/mdns.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef HTTPD_DEBUG
#define HTTPD_DEBUG LWIP_DBG_ON
#endif

#ifndef DEBUG_WS
#define DEBUG_WS 0
#endif

#define CGI_DATA_LENGTH_MAX (96)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void cgi_urldecode(char *url);
static int cgi_rtc_data(HTTPSRV_CGI_REQ_STRUCT *param);
static int cgi_example(HTTPSRV_CGI_REQ_STRUCT *param);
static int ssi_config(HTTPSRV_SSI_PARAM_STRUCT *param);
static bool cgi_get_varval(char *var_str, char *var_name, char *var_val, uint32_t length);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static char s_mdns_hostname[65] = "";

/* FS data.*/
extern const HTTPSRV_FS_DIR_ENTRY httpsrv_fs_data[];

/*
 * Authentication users
 */
static const HTTPSRV_AUTH_USER_STRUCT users[] = {
    {"admin", "admin"}, {NULL, NULL} /* Array terminator */
};

/*
 * Authentication information.
 */
static const HTTPSRV_AUTH_REALM_STRUCT auth_realms[] = {
    {"Please use uid:admin pass:admin to login", "/auth.html", HTTPSRV_AUTH_BASIC, users},
    {NULL, NULL, HTTPSRV_AUTH_INVALID, NULL} /* Array terminator */
};

char cgi_data[CGI_DATA_LENGTH_MAX + 1];

const HTTPSRV_CGI_LINK_STRUCT cgi_lnk_tbl[] = {
    {"rtcdata", cgi_rtc_data},
    {"get", cgi_example},
    {"post", cgi_example},
    {0, 0} // DO NOT REMOVE - last item - end of table
};

const HTTPSRV_SSI_LINK_STRUCT ssi_lnk_tbl[] = {{"config", ssi_config}, {0, 0}};

/*******************************************************************************
 * Code
 ******************************************************************************/
static int cgi_rtc_data(HTTPSRV_CGI_REQ_STRUCT *param)
{
#define BUFF_SIZE sizeof("00\n00\n00\n")
    HTTPSRV_CGI_RES_STRUCT response;
    uint32_t time;
    uint32_t min;
    uint32_t hour;
    uint32_t sec;

    char str[BUFF_SIZE];
    uint32_t length = 0;

    if (param->request_method != HTTPSRV_REQ_GET)
    {
        return (0);
    }

    time = sys_now();

    sec  = time / 1000;
    min  = sec / 60;
    hour = min / 60;
    min %= 60;
    sec %= 60;

    response.ses_handle   = param->ses_handle;
    response.content_type = HTTPSRV_CONTENT_TYPE_PLAIN;
    response.status_code  = HTTPSRV_CODE_OK;
    /*
    ** When the keep-alive is used we have to calculate a correct content length
    ** so the receiver knows when to ACK the data and continue with a next request.
    ** Please see RFC2616 section 4.4 for further details.
    */

    /* Calculate content length while saving it to buffer */
    length                  = snprintf(str, BUFF_SIZE, "%" PRIu32 "\n%" PRIu32 "\n%" PRIu32 "\n", hour, min, sec);
    response.data           = str;
    response.data_length    = length;
    response.content_length = response.data_length;
    /* Send response */
    HTTPSRV_cgi_write(&response);
    return (response.content_length);
}

/* Example Common Gateway Interface callback. */
static int cgi_example(HTTPSRV_CGI_REQ_STRUCT *param)
{
    HTTPSRV_CGI_RES_STRUCT response = {0};

    response.ses_handle  = param->ses_handle;
    response.status_code = HTTPSRV_CODE_OK;

    if (param->request_method == HTTPSRV_REQ_GET)
    {
        char *c;

        /* Replace '+' with spaces. */
        while ((c = strchr(cgi_data, '+')) != NULL)
        {
            *c = ' ';
        }
        response.content_type   = HTTPSRV_CONTENT_TYPE_PLAIN;
        response.data           = cgi_data;
        response.data_length    = strlen(cgi_data);
        response.content_length = response.data_length;
        HTTPSRV_cgi_write(&response);
    }
    else if (param->request_method == HTTPSRV_REQ_POST)
    {
        uint32_t length = 0;
        uint32_t read;
        char buffer[sizeof("post_input = ") + CGI_DATA_LENGTH_MAX] = {0};

        length = param->content_length;
        read   = HTTPSRV_cgi_read(param->ses_handle, buffer, (length > sizeof(buffer)) ? sizeof(buffer) : length);

        if (read > 0)
        {
            cgi_get_varval(buffer, "post_input", cgi_data, sizeof(cgi_data));
            cgi_urldecode(cgi_data);
        }

        /* Write the response using chunked transmission coding. */
        response.content_type = HTTPSRV_CONTENT_TYPE_HTML;
        /* Set content length to -1 to indicate unknown content length. */
        response.content_length = -1;
        response.data           = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">";
        response.data_length    = strlen(response.data);
        HTTPSRV_cgi_write(&response);
        response.data        = "<html><head><title>POST successfull!</title>";
        response.data_length = strlen(response.data);
        HTTPSRV_cgi_write(&response);
        response.data        = "<meta http-equiv=\"refresh\" content=\"0; url=cgi.html\"></head><body></body></html>";
        response.data_length = strlen(response.data);
        HTTPSRV_cgi_write(&response);
        response.data_length = 0;
        HTTPSRV_cgi_write(&response);
    }

    return (response.content_length);
}

static bool cgi_get_varval(char *src, char *var_name, char *dst, uint32_t length)
{
    char *name;
    bool result;
    uint32_t index;
    uint32_t n_length;

    result = false;
    dst[0] = 0;
    name   = src;

    n_length = strlen(var_name);

    while ((name != NULL) && ((name = strstr(name, var_name)) != NULL))
    {
        if (name[n_length] == '=')
        {
            name += n_length + 1;

            index = strcspn(name, "&");
            if (index >= length)
            {
                index = length - 1;
            }
            strncpy(dst, name, index);
            dst[index] = '\0';
            result     = true;
            break;
        }
        else
        {
            name = strchr(name, '&');
        }
    }

    return (result);
}

/* Example Server Side Include callback. */
static int ssi_config(HTTPSRV_SSI_PARAM_STRUCT *param)
{
    char *string_value = NULL;
    int int_value      = -1;
    char str[16];

    if (strcmp(param->com_param, "SERVER_STACK_SIZE") == 0)
    {
        int_value = HTTPSRV_CFG_SERVER_STACK_SIZE;
    }
    else if (strcmp(param->com_param, "HTTP_SESSION_STACK_SIZE") == 0)
    {
        int_value = HTTPSRV_CFG_HTTP_SESSION_STACK_SIZE;
    }
    else if (strcmp(param->com_param, "HTTPS_SESSION_STACK_SIZE") == 0)
    {
        int_value = HTTPSRV_CFG_HTTPS_SESSION_STACK_SIZE;
    }
    else if (strcmp(param->com_param, "DEFAULT_PRIORITY") == 0)
    {
        int_value = HTTPSRV_CFG_DEFAULT_PRIORITY;
    }
    else if (strcmp(param->com_param, "DEFAULT_HTTP_PORT") == 0)
    {
        int_value = HTTPSRV_CFG_DEFAULT_HTTP_PORT;
    }
    else if (strcmp(param->com_param, "DEFAULT_HTTPS_PORT") == 0)
    {
        int_value = HTTPSRV_CFG_DEFAULT_HTTPS_PORT;
    }
    else if (strcmp(param->com_param, "DEFAULT_INDEX_PAGE") == 0)
    {
        string_value = HTTPSRV_CFG_DEFAULT_INDEX_PAGE;
    }
    else if (strcmp(param->com_param, "CACHE_MAXAGE") == 0)
    {
        int_value = HTTPSRV_CFG_CACHE_MAXAGE;
    }
    else if (strcmp(param->com_param, "DEFAULT_SES_CNT") == 0)
    {
        int_value = HTTPSRV_CFG_DEFAULT_SES_CNT;
    }
    else if (strcmp(param->com_param, "SES_BUFFER_SIZE") == 0)
    {
        int_value = HTTPSRV_CFG_SES_BUFFER_SIZE;
    }
    else if (strcmp(param->com_param, "DEFAULT_URL_LEN") == 0)
    {
        int_value = HTTPSRV_CFG_DEFAULT_URL_LEN;
    }
    else if (strcmp(param->com_param, "MAX_SCRIPT_LN") == 0)
    {
        int_value = HTTPSRV_CFG_MAX_SCRIPT_LN;
    }
    else if (strcmp(param->com_param, "KEEPALIVE_ENABLED") == 0)
    {
        int_value = HTTPSRV_CFG_KEEPALIVE_ENABLED;
    }
    else if (strcmp(param->com_param, "KEEPALIVE_TIMEOUT") == 0)
    {
        int_value = HTTPSRV_CFG_KEEPALIVE_TIMEOUT;
    }
    else if (strcmp(param->com_param, "SES_TIMEOUT") == 0)
    {
        int_value = HTTPSRV_CFG_SES_TIMEOUT;
    }
    else if (strcmp(param->com_param, "SEND_TIMEOUT") == 0)
    {
        int_value = HTTPSRV_CFG_SEND_TIMEOUT;
    }
    else if (strcmp(param->com_param, "RECEIVE_TIMEOUT") == 0)
    {
        int_value = HTTPSRV_CFG_RECEIVE_TIMEOUT;
    }
    else if (strcmp(param->com_param, "WEBSOCKET_ENABLED") == 0)
    {
        int_value = HTTPSRV_CFG_WEBSOCKET_ENABLED;
    }
    else if (strcmp(param->com_param, "WOLFSSL_ENABLE") == 0)
    {
        int_value = HTTPSRV_CFG_WOLFSSL_ENABLE;
    }
    else if (strcmp(param->com_param, "MBEDTLS_ENABLE") == 0)
    {
        int_value = HTTPSRV_CFG_MBEDTLS_ENABLE;
    }

    if (string_value != NULL)
    {
        HTTPSRV_ssi_write(param->ses_handle, string_value, strlen(string_value));
    }
    else
    {
        sprintf(str, "%d", int_value);
        HTTPSRV_ssi_write(param->ses_handle, str, strlen(str));
    }

    return (0);
}

/* Decode URL encoded string in place. */
static void cgi_urldecode(char *url)
{
    char *src = url;
    char *dst = url;

    while (*src != '\0')
    {
        if ((*src == '%') && (isxdigit((unsigned char)*(src + 1))) && (isxdigit((unsigned char)*(src + 2))))
        {
            *src       = *(src + 1);
            *(src + 1) = *(src + 2);
            *(src + 2) = '\0';
            *dst++     = strtol(src, NULL, 16);
            src += 3;
        }
        else
        {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

#if HTTPSRV_CFG_WEBSOCKET_ENABLED
/*
 * Echo plugin code - simple plugin which echoes any message it receives back to
 * client.
 */
uint32_t ws_echo_connect(void *param, WS_USER_CONTEXT_STRUCT context)
{
#if DEBUG_WS
    PRINTF("WebSocket echo client connected.\r\n");
#endif
    return (0);
}

uint32_t ws_echo_disconnect(void *param, WS_USER_CONTEXT_STRUCT context)
{
#if DEBUG_WS
    PRINTF("WebSocket echo client disconnected.\r\n");
#endif
    return (0);
}

uint32_t ws_echo_message(void *param, WS_USER_CONTEXT_STRUCT context)
{
    WS_send(&context); /* Send back what was received.*/
#if DEBUG_WS
    if (context.data.type == WS_DATA_TEXT)
    {
        /* Print received text message to console. */
        context.data.data_ptr[context.data.length] = 0;
        PRINTF("WebSocket message received:\r\n%s\r\n", context.data.data_ptr);
    }
    else
    {
        /* Inform user about binary message. */
        PRINTF("WebSocket binary data with length of %d bytes received.", context.data.length);
    }
#endif

    return (0);
}

uint32_t ws_echo_error(void *param, WS_USER_CONTEXT_STRUCT context)
{
#if DEBUG_WS
    PRINTF("WebSocket error: 0x%X.\r\n", context.error);
#endif
    return (0);
}

WS_PLUGIN_STRUCT ws_tbl[] = {{"/echo", ws_echo_connect, ws_echo_message, ws_echo_error, ws_echo_disconnect, NULL},
                             {0, 0, 0, 0, 0, 0}};
#endif /* HTTPSRV_CFG_WEBSOCKET_ENABLED */

/*!
 * @brief Callback function to generate TXT mDNS record for HTTP service.
 */
static void http_srv_txt(struct mdns_service *service, void *txt_userdata)
{
    mdns_resp_add_service_txtitem(service, "path=/", 6);
}

/*!
 * @brief Configure and enable MDNS service.
 */
void http_server_enable_mdns(struct netif *netif, const char *mdns_hostname)
{
    LOCK_TCPIP_CORE();
    mdns_resp_init();
    mdns_resp_add_netif(netif, mdns_hostname);
    mdns_resp_add_service(netif, mdns_hostname, "_http", DNSSD_PROTO_TCP, 80, http_srv_txt, NULL);
    UNLOCK_TCPIP_CORE();

    (void)strncpy(s_mdns_hostname, mdns_hostname, sizeof(s_mdns_hostname) - 1);
    s_mdns_hostname[sizeof(s_mdns_hostname) - 1] = '\0'; // Make sure string will be always terminated.
}

/*!
 * @brief Initializes server.
 */
void http_server_socket_init(void)
{
    HTTPSRV_PARAM_STRUCT params;
    uint32_t httpsrv_handle;

    /* Init Fs*/
    HTTPSRV_FS_init(httpsrv_fs_data);

    /* Init HTTPSRV parameters.*/
    memset(&params, 0, sizeof(params));
    params.root_dir    = "";
    params.index_page  = "/index.html";
    params.auth_table  = auth_realms;
    params.cgi_lnk_tbl = cgi_lnk_tbl;
    params.ssi_lnk_tbl = ssi_lnk_tbl;
#if HTTPSRV_CFG_WEBSOCKET_ENABLED
    params.ws_tbl = ws_tbl;
#endif

    /* Init HTTP Server.*/
    httpsrv_handle = HTTPSRV_init(&params);
    if (httpsrv_handle == 0)
    {
        PRINTF(("HTTPSRV_init() is Failed"));
    }
}

/*!
 * @brief Prints IP configuration.
 */
void http_server_print_ip_cfg(struct netif *netif)
{
    PRINTF("\r\n***********************************************************\r\n");
    PRINTF(" HTTP Server example\r\n");
    PRINTF("***********************************************************\r\n");
#if LWIP_IPV4
    PRINTF(" IPv4 Address     : %s\r\n", ip4addr_ntoa(netif_ip4_addr(netif)));
    PRINTF(" IPv4 Subnet mask : %s\r\n", ip4addr_ntoa(netif_ip4_netmask(netif)));
    PRINTF(" IPv4 Gateway     : %s\r\n", ip4addr_ntoa(netif_ip4_gw(netif)));
#endif /* LWIP_IPV4 */
#if LWIP_IPV6
    http_server_print_ipv6_addresses(netif);
#endif /* LWIP_IPV6 */
    PRINTF(" mDNS hostname    : %s\r\n", s_mdns_hostname);
    PRINTF("***********************************************************\r\n");
}

#if LWIP_IPV6
/*!
 * @brief Prints valid IPv6 addresses.
 */
void http_server_print_ipv6_addresses(struct netif *netif)
{
    for (int i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++)
    {
        const char *str_ip = "-";
        if (ip6_addr_isvalid(netif_ip6_addr_state(netif, i)))
        {
            str_ip = ip6addr_ntoa(netif_ip6_addr(netif, i));
        }
        PRINTF(" IPv6 Address%d    : %s\r\n", i, str_ip);
    }
}
#endif /* LWIP_IPV6 */
