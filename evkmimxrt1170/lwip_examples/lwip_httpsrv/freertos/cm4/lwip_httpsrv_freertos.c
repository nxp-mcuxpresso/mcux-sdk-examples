/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "lwip/opt.h"

#if LWIP_SOCKET
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "enet_ethernetif.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_silicon_id.h"
#include "fsl_phy.h"

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

#if BOARD_NETWORK_USE_100M_ENET_PORT
#include "fsl_phyksz8081.h"
#else
#include "fsl_phyrtl8211f.h"
#endif
#include "fsl_enet_mdio.h"
#include "fsl_enet.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* @TEST_ANCHOR */

/* IP address configuration. */
#ifndef configIP_ADDR0
#define configIP_ADDR0 192
#endif
#ifndef configIP_ADDR1
#define configIP_ADDR1 168
#endif
#ifndef configIP_ADDR2
#define configIP_ADDR2 0
#endif
#ifndef configIP_ADDR3
#define configIP_ADDR3 102
#endif

/* Netmask configuration. */
#ifndef configNET_MASK0
#define configNET_MASK0 255
#endif
#ifndef configNET_MASK1
#define configNET_MASK1 255
#endif
#ifndef configNET_MASK2
#define configNET_MASK2 255
#endif
#ifndef configNET_MASK3
#define configNET_MASK3 0
#endif

/* Gateway address configuration. */
#ifndef configGW_ADDR0
#define configGW_ADDR0 192
#endif
#ifndef configGW_ADDR1
#define configGW_ADDR1 168
#endif
#ifndef configGW_ADDR2
#define configGW_ADDR2 0
#endif
#ifndef configGW_ADDR3
#define configGW_ADDR3 100
#endif

#if BOARD_NETWORK_USE_100M_ENET_PORT
/* Address of PHY interface. */
#define EXAMPLE_PHY_ADDRESS BOARD_ENET0_PHY_ADDRESS
/* PHY operations. */
#define EXAMPLE_PHY_OPS phyksz8081_ops
/* ENET instance select. */
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init
#else
/* Address of PHY interface. */
#define EXAMPLE_PHY_ADDRESS   BOARD_ENET1_PHY_ADDRESS
/* PHY operations. */
#define EXAMPLE_PHY_OPS       phyrtl8211f_ops
/* ENET instance select. */
#define EXAMPLE_NETIF_INIT_FN ethernetif1_init
#endif

/* MDIO operations. */
#define EXAMPLE_MDIO_OPS enet_ops

/* ENET clock frequency. */
#define EXAMPLE_CLOCK_FREQ CLOCK_GetRootClockFreq(kCLOCK_Root_Bus)


#ifndef EXAMPLE_NETIF_INIT_FN
/*! @brief Network interface initialization function. */
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init
#endif /* EXAMPLE_NETIF_INIT_FN */

#ifndef HTTPD_DEBUG
#define HTTPD_DEBUG LWIP_DBG_ON
#endif
#ifndef HTTPD_STACKSIZE
#define HTTPD_STACKSIZE DEFAULT_THREAD_STACKSIZE
#endif
#ifndef HTTPD_PRIORITY
#define HTTPD_PRIORITY DEFAULT_THREAD_PRIO
#endif
#ifndef DEBUG_WS
#define DEBUG_WS 0
#endif

#define CGI_DATA_LENGTH_MAX (96)

#define MDNS_HOSTNAME "lwip-http"

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

static mdio_handle_t mdioHandle = {.ops = &EXAMPLE_MDIO_OPS};
static phy_handle_t phyHandle   = {.phyAddr = EXAMPLE_PHY_ADDRESS, .mdioHandle = &mdioHandle, .ops = &EXAMPLE_PHY_OPS};

static struct netif netif;
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
void BOARD_InitModuleClock(void)
{
    const clock_sys_pll1_config_t sysPll1Config = {
        .pllDiv2En = true,
    };
    CLOCK_InitSysPll1(&sysPll1Config);

#if BOARD_NETWORK_USE_100M_ENET_PORT
    clock_root_config_t rootCfg = {.mux = 4, .div = 10}; /* Generate 50M root clock. */
    CLOCK_SetRootClock(kCLOCK_Root_Enet1, &rootCfg);
#else
    clock_root_config_t rootCfg = {.mux = 4, .div = 4}; /* Generate 125M root clock. */
    CLOCK_SetRootClock(kCLOCK_Root_Enet2, &rootCfg);
#endif
}

void IOMUXC_SelectENETClock(void)
{
#if BOARD_NETWORK_USE_100M_ENET_PORT
    IOMUXC_GPR->GPR4 |= IOMUXC_GPR_GPR4_ENET_REF_CLK_DIR_MASK; /* 50M ENET_REF_CLOCK output to PHY and ENET module. */
#else
    IOMUXC_GPR->GPR5 |= IOMUXC_GPR_GPR5_ENET1G_RGMII_EN_MASK; /* bit1:iomuxc_gpr_enet_clk_dir
                                                                 bit0:GPR_ENET_TX_CLK_SEL(internal or OSC) */
#endif
}

void BOARD_ENETFlexibleConfigure(enet_config_t *config)
{
#if BOARD_NETWORK_USE_100M_ENET_PORT
    config->miiMode = kENET_RmiiMode;
#else
    config->miiMode = kENET_RgmiiMode;
#endif
}

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
    length                  = snprintf(str, BUFF_SIZE, "%ld\n%ld\n%ld\n", hour, min, sec);
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
 * @brief Initializes lwIP stack.
 */
static void stack_init(void)
{
    ip4_addr_t netif_ipaddr, netif_netmask, netif_gw;
    ethernetif_config_t enet_config = {
        .phyHandle = &phyHandle,
#ifdef configMAC_ADDR
        .macAddress = configMAC_ADDR,
#endif
    };

    mdioHandle.resource.csrClock_Hz = EXAMPLE_CLOCK_FREQ;

    tcpip_init(NULL, NULL);

#ifndef configMAC_ADDR
    /* Set special address for each chip. */
    (void)SILICONID_ConvertToMacAddr(&enet_config.macAddress);
#endif

    IP4_ADDR(&netif_ipaddr, configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3);
    IP4_ADDR(&netif_netmask, configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3);
    IP4_ADDR(&netif_gw, configGW_ADDR0, configGW_ADDR1, configGW_ADDR2, configGW_ADDR3);

    netifapi_netif_add(&netif, &netif_ipaddr, &netif_netmask, &netif_gw, &enet_config, EXAMPLE_NETIF_INIT_FN,
                       tcpip_input);
    netifapi_netif_set_default(&netif);
    netifapi_netif_set_up(&netif);

    LOCK_TCPIP_CORE();
    mdns_resp_init();
    mdns_resp_add_netif(&netif, MDNS_HOSTNAME);
    mdns_resp_add_service(&netif, MDNS_HOSTNAME, "_http", DNSSD_PROTO_TCP, 80, http_srv_txt, NULL);
    UNLOCK_TCPIP_CORE();

    LWIP_PLATFORM_DIAG(("\r\n************************************************"));
    LWIP_PLATFORM_DIAG((" HTTP Server example"));
    LWIP_PLATFORM_DIAG(("************************************************"));
    LWIP_PLATFORM_DIAG((" IPv4 Address     : %u.%u.%u.%u", ((u8_t *)&netif_ipaddr)[0], ((u8_t *)&netif_ipaddr)[1],
                        ((u8_t *)&netif_ipaddr)[2], ((u8_t *)&netif_ipaddr)[3]));
    LWIP_PLATFORM_DIAG((" IPv4 Subnet mask : %u.%u.%u.%u", ((u8_t *)&netif_netmask)[0], ((u8_t *)&netif_netmask)[1],
                        ((u8_t *)&netif_netmask)[2], ((u8_t *)&netif_netmask)[3]));
    LWIP_PLATFORM_DIAG((" IPv4 Gateway     : %u.%u.%u.%u", ((u8_t *)&netif_gw)[0], ((u8_t *)&netif_gw)[1],
                        ((u8_t *)&netif_gw)[2], ((u8_t *)&netif_gw)[3]));
    LWIP_PLATFORM_DIAG((" mDNS hostname    : %s", MDNS_HOSTNAME));
    LWIP_PLATFORM_DIAG(("************************************************"));
}

/*!
 * @brief Initializes server.
 */
static void http_server_socket_init(void)
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
        LWIP_PLATFORM_DIAG(("HTTPSRV_init() is Failed"));
    }
}

/*!
 * @brief The main function containing server thread.
 */
static void main_thread(void *arg)
{
    LWIP_UNUSED_ARG(arg);

    stack_init();
    http_server_socket_init();

    vTaskDelete(NULL);
}

/*!
 * @brief Main function.
 */
int main(void)
{
    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitModuleClock();

    IOMUXC_SelectENETClock();

#if BOARD_NETWORK_USE_100M_ENET_PORT
    BOARD_InitEnetPins();
    GPIO_PinInit(GPIO9, 11, &gpio_config);
    GPIO_PinInit(GPIO12, 12, &gpio_config);
    /* Pull up the ENET_INT before RESET. */
    GPIO_WritePinOutput(GPIO9, 11, 1);
    GPIO_WritePinOutput(GPIO12, 12, 0);
    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    GPIO_WritePinOutput(GPIO12, 12, 1);
    SDK_DelayAtLeastUs(6, CLOCK_GetFreq(kCLOCK_CpuClk));
#else
    BOARD_InitEnet1GPins();
    GPIO_PinInit(GPIO11, 14, &gpio_config);
    /* For a complete PHY reset of RTL8211FDI-CG, this pin must be asserted low for at least 10ms. And
     * wait for a further 30ms(for internal circuits settling time) before accessing the PHY register */
    GPIO_WritePinOutput(GPIO11, 14, 0);
    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    GPIO_WritePinOutput(GPIO11, 14, 1);
    SDK_DelayAtLeastUs(30000, CLOCK_GetFreq(kCLOCK_CpuClk));

    EnableIRQ(ENET_1G_MAC0_Tx_Rx_1_IRQn);
    EnableIRQ(ENET_1G_MAC0_Tx_Rx_2_IRQn);
#endif

    mdioHandle.resource.csrClock_Hz = EXAMPLE_CLOCK_FREQ;

    /* create server thread in RTOS */
    if (sys_thread_new("main", main_thread, NULL, HTTPD_STACKSIZE, HTTPD_PRIORITY) == NULL)
        LWIP_ASSERT("main(): Task creation failed.", 0);

    /* run RTOS */
    vTaskStartScheduler();

    /* should not reach this statement */
    for (;;)
        ;
}

#endif // LWIP_SOCKET
