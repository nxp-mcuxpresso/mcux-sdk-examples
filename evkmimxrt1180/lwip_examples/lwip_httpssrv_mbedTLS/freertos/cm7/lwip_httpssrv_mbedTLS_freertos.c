/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2024 NXP
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

#include "ethernetif.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_phy.h"

#include "lwip/netif.h"
#include "lwip/sys.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/netifapi.h"
#include "lwip/tcpip.h"
#include "lwip/ip.h"
#include "lwip/sockets.h"
#include "netif/etharp.h"

#ifdef MBEDTLS_MCUX_ELE_S400_API
#include "ele_mbedtls.h"
#else
#include "ksdk_mbedtls.h"
#endif /* MBEDTLS_MCUX_ELE_S400_API */

#include "httpsrv.h"

// dm #include "certs_buff.h"
#include "mbedtls/certs.h"

#include "fsl_phyrtl8201.h"
#include "fsl_netc_endpoint.h"
#include "fsl_netc_switch.h"
#include "fsl_netc_mdio.h"
#include "fsl_phyrtl8211f.h"
#include "fsl_msgintr.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define PHY_PAGE_SELECT_REG 0x1FU /*!< The PHY page select register. */
#define EXAMPLE_EP0_PORT  0x00U
#define EXAMPLE_SWT_PORT0 0x01U
#define EXAMPLE_SWT_PORT1 0x02U
#define EXAMPLE_SWT_PORT2 0x03U
#define EXAMPLE_SWT_PORT3 0x04U

#define EXAMPLE_EP0_PHY_ADDR       0x03U
#define EXAMPLE_SWT_PORT0_PHY_ADDR 0x02U
#define EXAMPLE_SWT_PORT1_PHY_ADDR 0x05U
#define EXAMPLE_SWT_PORT2_PHY_ADDR 0x04U
#define EXAMPLE_SWT_PORT3_PHY_ADDR 0x08U
#define EXAMPLE_NETC_FREQ          CLOCK_GetRootClockFreq(kCLOCK_Root_Netc)

#define EXAMPLE_EP_RING_NUM          3U
#define EXAMPLE_EP_RXBD_NUM          8U
#define EXAMPLE_EP_TXBD_NUM          8U
#define EXAMPLE_EP_BD_ALIGN          128U
#define EXAMPLE_EP_BUFF_SIZE_ALIGN   64U
#define EXAMPLE_EP_RXBUFF_SIZE       1518U
#define EXAMPLE_EP_RXBUFF_SIZE_ALIGN SDK_SIZEALIGN(EXAMPLE_EP_RXBUFF_SIZE, EXAMPLE_EP_BUFF_SIZE_ALIGN)
#define EXAMPLE_EP_TEST_FRAME_SIZE   1000U

#define EXAMPLE_EP_TXFRAME_NUM 20U
#define EXAMPLE_TX_RX_INTERRUPT_HANDLE

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

/* Ethernet configuration. */

#define configMAC_ADDR                     \
    {                                      \
        0x00, 0x00, 0xfa, 0xfa, 0xdd, 0x05 \
    }

#define EXAMPLE_PHY_ADDRESS  EXAMPLE_EP0_PHY_ADDR
#define EXAMPLE_PHY_OPS      &g_app_phy_rtl8201_ops
#define EXAMPLE_PHY_RESOURCE &g_phy_rtl8201_resource
#define EXAMPLE_CLOCK_FREQ   EXAMPLE_NETC_FREQ // CLOCK_GetFreq(kCLOCK_IpgClk)
/* Must be after include of app.h */
#ifndef configMAC_ADDR
#include "fsl_silicon_id.h"
#endif

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

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
status_t APP_EP0_MDIOWrite(uint8_t phyAddr, uint8_t regAddr, uint16_t data);
status_t APP_EP0_MDIORead(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData);

static void cgi_urldecode(char *url);
static int cgi_rtc_data(HTTPSRV_CGI_REQ_STRUCT *param);
static int cgi_example(HTTPSRV_CGI_REQ_STRUCT *param);
static int ssi_config(HTTPSRV_SSI_PARAM_STRUCT *param);
static bool cgi_get_varval(char *var_str, char *var_name, char *var_val, uint32_t length);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static status_t APP_PHY_RTL8201_Init(phy_handle_t *handle, const phy_config_t *config);
static void APP_MDIO_Init(void);

/* PHY operation. */
static netc_mdio_handle_t s_mdio_handle;
// static phy_handle_t s_phy_handle;
// static uint8_t s_phy_address       = EXAMPLE_EP0_PHY_ADDR;
static mdioRead s_mdio_func_read   = APP_EP0_MDIORead;
static mdioWrite s_mdio_func_write = APP_EP0_MDIOWrite;

phy_rtl8201_resource_t g_phy_rtl8201_resource /*= {.read = }*/;

const phy_operations_t g_app_phy_rtl8201_ops = {.phyInit             = APP_PHY_RTL8201_Init,
                                                .phyWrite            = PHY_RTL8201_Write,
                                                .phyRead             = PHY_RTL8201_Read,
                                                .getAutoNegoStatus   = PHY_RTL8201_GetAutoNegotiationStatus,
                                                .getLinkStatus       = PHY_RTL8201_GetLinkStatus,
                                                .getLinkSpeedDuplex  = PHY_RTL8201_GetLinkSpeedDuplex,
                                                .setLinkSpeedDuplex  = PHY_RTL8201_SetLinkSpeedDuplex,
                                                .enableLoopback      = PHY_RTL8201_EnableLoopback,
                                                .enableLinkInterrupt = PHY_RTL8201_EnableLinkInterrupt,
                                                .clearInterrupt      = PHY_RTL8201_ClearInterrupt};

static phy_handle_t phyHandle;

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

/* This does initialization and then reconfigures CRS/DV pin to RXDV signal. */
static status_t APP_PHY_RTL8201_Init(phy_handle_t *handle, const phy_config_t *config)
{
    status_t result;
    uint16_t data;

    APP_MDIO_Init();

    /* Reset PHY8201 for ETH4. Reset 10ms, wait 72ms. */
    RGPIO_PinWrite(BOARD_INITPHYACCESSPINS_ENET4_RST_B_GPIO, BOARD_INITPHYACCESSPINS_ENET4_RST_B_GPIO_PIN, 0);
    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    RGPIO_PinWrite(BOARD_INITPHYACCESSPINS_ENET4_RST_B_GPIO, BOARD_INITPHYACCESSPINS_ENET4_RST_B_GPIO_PIN, 1);
    SDK_DelayAtLeastUs(72000, CLOCK_GetFreq(kCLOCK_CpuClk));

    result = PHY_RTL8201_Init(handle, config);
    if (result != kStatus_Success)
    {
        return result;
    }

    result = PHY_Write(handle, PHY_PAGE_SELECT_REG, 7);
    if (result != kStatus_Success)
    {
        return result;
    }
    result = PHY_Read(handle, 16, &data);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* CRS/DV pin is RXDV signal. */
    data |= (1U << 2);
    result = PHY_Write(handle, 16, data);
    if (result != kStatus_Success)
    {
        return result;
    }
    result = PHY_Write(handle, PHY_PAGE_SELECT_REG, 0);

    return result;
}

static void APP_MDIO_Init(void)
{
    status_t result = kStatus_Success;

    netc_mdio_config_t mdioConfig = {
        .mdio =
            {
                .type = kNETC_EMdio,
            },
        .isPreambleDisable = false,
        .isNegativeDriven  = false,
        .srcClockHz        = EXAMPLE_NETC_FREQ,
    };

    mdioConfig.mdio.port = (netc_hw_eth_port_idx_t)kNETC_ENETC0EthPort;
    result               = NETC_MDIOInit(&s_mdio_handle, &mdioConfig);
    while (result != kStatus_Success)
    {
        // failed
    }
}

status_t APP_EP0_MDIOWrite(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    return NETC_MDIOWrite(&s_mdio_handle, phyAddr, regAddr, data);
}

status_t APP_EP0_MDIORead(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
    return NETC_MDIORead(&s_mdio_handle, phyAddr, regAddr, pData);
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

#if HTTPSRV_CFG_MBEDTLS_ENABLE
static HTTPSRV_TLS_PARAM_STRUCT tls_params;
#endif

#if LWIP_IPV6
static void print_ipv6_addresses(struct netif *netif)
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

static void netif_ipv6_callback(struct netif *cb_netif)
{
    PRINTF("IPv6 address update, valid addresses:\r\n");
    print_ipv6_addresses(cb_netif);
    PRINTF("\r\n");
}
#endif /* LWIP_IPV6 */

/*!
 * @brief Initializes lwIP stack.
 */
static void stack_init(void)
{
    status_t status;
#if LWIP_IPV4
    ip4_addr_t netif_ipaddr, netif_netmask, netif_gw;
#endif /* LWIP_IPV4 */
    ethernetif_config_t enet_config = {.phyHandle   = &phyHandle,
                                       .phyAddr     = EXAMPLE_PHY_ADDRESS,
                                       .phyOps      = EXAMPLE_PHY_OPS,
                                       .phyResource = EXAMPLE_PHY_RESOURCE,
                                       .srcClockHz  = EXAMPLE_CLOCK_FREQ,
#ifdef configMAC_ADDR
                                       .macAddress = configMAC_ADDR
#endif
    };

    status = CRYPTO_InitHardware();
    LWIP_ASSERT("CRYPTO_InitHardware() has failed\r\n", status == kStatus_Success);

    tcpip_init(NULL, NULL);

    /* Set MAC address. */
#ifndef configMAC_ADDR
    (void)SILICONID_ConvertToMacAddr(&enet_config.macAddress);
#endif

#if LWIP_IPV4
    IP4_ADDR(&netif_ipaddr, configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3);
    IP4_ADDR(&netif_netmask, configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3);
    IP4_ADDR(&netif_gw, configGW_ADDR0, configGW_ADDR1, configGW_ADDR2, configGW_ADDR3);

    netifapi_netif_add(&netif, &netif_ipaddr, &netif_netmask, &netif_gw, &enet_config, EXAMPLE_NETIF_INIT_FN,
                       tcpip_input);
#else
    netifapi_netif_add(&netif, &enet_config, EXAMPLE_NETIF_INIT_FN, tcpip_input);
#endif /* LWIP_IPV4 */
    netifapi_netif_set_default(&netif);
    netifapi_netif_set_up(&netif);

#if LWIP_IPV6
    LOCK_TCPIP_CORE();
    netif_create_ip6_linklocal_address(&netif, 1);
    UNLOCK_TCPIP_CORE();
#endif /* LWIP_IPV6 */

    while (ethernetif_wait_linkup(&netif, 5000) != ERR_OK)
    {
        PRINTF("PHY Auto-negotiation failed. Please check the cable connection and link partner setting.\r\n");
    }

#if LWIP_IPV6
    LOCK_TCPIP_CORE();
    set_ipv6_valid_state_cb(netif_ipv6_callback);
    UNLOCK_TCPIP_CORE();
#endif /* LWIP_IPV6 */

    /*
     * Lock prints since it could interfere with prints from netif_ipv6_callback
     * in case IPv6 address would become valid early.
     */
    LOCK_TCPIP_CORE();
    LWIP_PLATFORM_DIAG(("\r\n***********************************************************\r\n"));
    LWIP_PLATFORM_DIAG(("mbedTLS HTTPS Server example\r\n"));
    LWIP_PLATFORM_DIAG(("***********************************************************\r\n"));
#if LWIP_IPV4
    LWIP_PLATFORM_DIAG((" IPv4 Address     : %u.%u.%u.%u\r\n", ((u8_t *)&netif_ipaddr)[0], ((u8_t *)&netif_ipaddr)[1],
                        ((u8_t *)&netif_ipaddr)[2], ((u8_t *)&netif_ipaddr)[3]));
    LWIP_PLATFORM_DIAG((" IPv4 Subnet mask : %u.%u.%u.%u\r\n", ((u8_t *)&netif_netmask)[0], ((u8_t *)&netif_netmask)[1],
                        ((u8_t *)&netif_netmask)[2], ((u8_t *)&netif_netmask)[3]));
    LWIP_PLATFORM_DIAG((" IPv4 Gateway     : %u.%u.%u.%u\r\n", ((u8_t *)&netif_gw)[0], ((u8_t *)&netif_gw)[1],
                        ((u8_t *)&netif_gw)[2], ((u8_t *)&netif_gw)[3]));
#endif /* LWIP_IPV4 */
#if LWIP_IPV6
    print_ipv6_addresses(&netif);
#endif /* LWIP_IPV6 */
    LWIP_PLATFORM_DIAG(("***********************************************************\r\n"));
    UNLOCK_TCPIP_CORE();
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
#if HTTPSRV_CFG_MBEDTLS_ENABLE
    tls_params.certificate_buffer      = (const unsigned char *)mbedtls_test_srv_crt;
    tls_params.certificate_buffer_size = mbedtls_test_srv_crt_len;
    tls_params.private_key_buffer      = (const unsigned char *)mbedtls_test_srv_key;
    tls_params.private_key_buffer_size = mbedtls_test_srv_key_len;

    params.tls_param = &tls_params;
#endif
    /* Init HTTP Server.*/
    httpsrv_handle = HTTPSRV_init(&params);
    LWIP_ASSERT("HTTPSRV_init() has failed\r\n", httpsrv_handle != 0);
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
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* RMII mode */
    BLK_CTRL_WAKEUPMIX->NETC_LINK_CFG[0] = BLK_CTRL_WAKEUPMIX_NETC_LINK_CFG_MII_PROT(1);
    BLK_CTRL_WAKEUPMIX->NETC_LINK_CFG[4] = BLK_CTRL_WAKEUPMIX_NETC_LINK_CFG_MII_PROT(1);

    /* RGMII mode */
    BLK_CTRL_WAKEUPMIX->NETC_LINK_CFG[1] = BLK_CTRL_WAKEUPMIX_NETC_LINK_CFG_MII_PROT(2);

    /* Output reference clock for RMII */
    BLK_CTRL_WAKEUPMIX->NETC_PORT_MISC_CFG |= BLK_CTRL_WAKEUPMIX_NETC_PORT_MISC_CFG_PORT0_RMII_REF_CLK_DIR_MASK |
                                              BLK_CTRL_WAKEUPMIX_NETC_PORT_MISC_CFG_PORT4_RMII_REF_CLK_DIR_MASK;

    /* Unlock the IERB. It will warm reset whole NETC. */
    NETC_PRIV->NETCRR &= ~NETC_PRIV_NETCRR_LOCK_MASK;
    while ((NETC_PRIV->NETCRR & NETC_PRIV_NETCRR_LOCK_MASK) != 0U)
    {
    }

    /* Set PHY address in IERB to use MAC port MDIO, otherwise the access will be blocked. */
    NETC_IERB->L0BCR = NETC_IERB_L0BCR_MDIO_PHYAD_PRTAD(EXAMPLE_SWT_PORT0_PHY_ADDR);
    NETC_IERB->L1BCR = NETC_IERB_L0BCR_MDIO_PHYAD_PRTAD(EXAMPLE_SWT_PORT1_PHY_ADDR);
    NETC_IERB->L4BCR = NETC_IERB_L0BCR_MDIO_PHYAD_PRTAD(EXAMPLE_EP0_PHY_ADDR);

    /* Set the access attribute, otherwise MSIX access will be blocked. */
    NETC_IERB->ARRAY_NUM_RC[0].RCMSIAMQR &= ~(7U << 27);
    NETC_IERB->ARRAY_NUM_RC[0].RCMSIAMQR |= (1U << 27);

    /* Lock the IERB. */
    NETC_PRIV->NETCRR |= NETC_PRIV_NETCRR_LOCK_MASK;
    while ((NETC_PRIV->NETCSR & NETC_PRIV_NETCSR_STATE_MASK) != 0U)
    {
    }

    /*result = APP_MDIO_Init();

    if (result != kStatus_Success)
    {
        while (true)
        {
        }
    }*/

    g_phy_rtl8201_resource.write    = s_mdio_func_write;
    g_phy_rtl8201_resource.writeExt = NULL;
    g_phy_rtl8201_resource.read     = s_mdio_func_read;
    g_phy_rtl8201_resource.readExt  = NULL;

    /* create server thread in RTOS */
    if (sys_thread_new("main", main_thread, NULL, HTTPD_STACKSIZE, HTTPD_PRIORITY) == NULL)
        LWIP_ASSERT("main(): Task creation failed.", 0);

    /* run RTOS */
    vTaskStartScheduler();

    /* should not reach this statement */
    for (;;)
        ;
}

#endif /* LWIP_SOCKET */
