/** @file wifi_setup.c
 *
 *  @brief main file
 *
 *  Copyright 2020 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

// SDK Included Files
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "wlan_bt_fw.h"
#include "wlan.h"
#include "wifi.h"
#include "wm_net.h"
#include <wm_os.h>

#include "lwip/tcpip.h"
#include "ping.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define APP_DEBUG_UART_TYPE     kSerialPort_Uart
#define APP_DEBUG_UART_INSTANCE 12U
#define APP_DEBUG_UART_CLK_FREQ CLOCK_GetFlexcommClkFreq(12)
#define APP_DEBUG_UART_FRG_CLK \
    (&(const clock_frg_clk_config_t){12U, kCLOCK_FrgPllDiv, 255U, 0U}) /*!< Select FRG0 mux as frg_pll */
#define APP_DEBUG_UART_CLK_ATTACH kFRG_to_FLEXCOMM12
#define APP_DEBUG_UART_BAUDRATE   115200
// Hardwired SSID, passphrase of AP to connect to
// Change this to fit your AP

/* @TEST_ANCHOR */

#ifndef AP_SSID
#define AP_SSID "SSID"
#endif

#ifndef AP_PASSPHRASE
#define AP_PASSPHRASE "PASSWD"
#endif

#ifndef PING_ADDR
#define PING_ADDR "8.8.8.8"
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/
const int taskMain_PRIO       = OS_PRIO_4;
const int taskMain_STACK_SIZE = 800;

portSTACK_TYPE *taskMain_stack = NULL;
TaskHandle_t taskMain_task_handler;

struct wlan_network sta_network;

static char firstResult             = 0;
static TaskHandle_t xInitTaskNotify = NULL;
static TaskHandle_t xJoinTaskNotify = NULL;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/* Initialize debug console. */
void APP_InitAppDebugConsole(void)
{
    uint32_t uartClkSrcFreq;

    /* attach FRG0 clock to FLEXCOMM12 (debug console) */
    CLOCK_SetFRGClock(APP_DEBUG_UART_FRG_CLK);
    CLOCK_AttachClk(APP_DEBUG_UART_CLK_ATTACH);

    uartClkSrcFreq = APP_DEBUG_UART_CLK_FREQ;

    DbgConsole_Init(APP_DEBUG_UART_INSTANCE, APP_DEBUG_UART_BAUDRATE, APP_DEBUG_UART_TYPE, uartClkSrcFreq);
}

static int __scan_cb(unsigned int count)
{
    struct wlan_scan_result res;
    int i;
    int err;

    if (count == 0)
    {
        PRINTF("No networks found!\r\n");
        return 0;
    }

    PRINTF("%d network%s found:\r\n", count, count == 1 ? "" : "s");

    for (i = 0; i < count; i++)
    {
        err = wlan_get_scan_result(i, &res);
        if (err)
        {
            PRINTF("Error: can't get scan res %d\r\n", i);
            continue;
        }

        print_mac(res.bssid);

        if (res.ssid[0])
            PRINTF(" \"%s\"\r\n", res.ssid);
        else
            PRINTF(" (hidden) \r\n");

        PRINTF("\tchannel: %d\r\n", res.channel);
        PRINTF("\trssi: -%d dBm\r\n", res.rssi);
        PRINTF("\tsecurity: ");
        if (res.wep)
            PRINTF("WEP ");
        if (res.wpa && res.wpa2)
            PRINTF("WPA/WPA2 Mixed ");
        else
        {
            if (res.wpa)
                PRINTF("WPA ");
            if (res.wpa2)
                PRINTF("WPA2 ");
            if (res.wpa3_sae)
                PRINTF("WPA3 SAE ");
            if (res.wpa2_entp)
                PRINTF("WPA2 Enterprise");
        }
        if (!(res.wep || res.wpa || res.wpa2 || res.wpa3_sae || res.wpa2_entp))
            PRINTF("OPEN ");
        PRINTF("\r\n");

        PRINTF("\tWMM: %s\r\n", res.wmm ? "YES" : "NO");
    }

    firstResult = 1;
    return 0;
}
static void scan(void)
{
    if (wlan_scan(__scan_cb))
    {
        PRINTF("Error: scan request failed\r\n");
        __BKPT(0);
    }
    else
    {
        PRINTF("Scan scheduled...\r\n");
    }
}
static void conToAp(void)
{
    int ret;

    PRINTF("Connecting to %s .....", sta_network.ssid);

    ret = wlan_connect(sta_network.name);

    if (ret != WM_SUCCESS)
    {
        PRINTF("Failed to connect %d\r\n", ret);
    }
}

#define MAX_RETRY_TICKS 50

static int network_added = 0;

/* Callback Function passed to WLAN Connection Manager. The callback function
 * gets called when there are WLAN Events that need to be handled by the
 * application.
 */
int wlan_event_callback(enum wlan_event_reason reason, void *data)
{
    PRINTF("app_cb: WLAN: received event %d\r\n", reason);

    switch (reason)
    {
        case WLAN_REASON_INITIALIZED:
            PRINTF("app_cb: WLAN initialized\r\n");
            int ret;

            /* Print WLAN FW Version */
            wlan_version_extended();

            if (!network_added)
            {
                uint8_t network_name_len = 0;
                uint8_t ssid_len         = 0;
                uint8_t psk_len          = 0;
                memset(&sta_network, 0, sizeof(struct wlan_network));

                network_name_len = (strlen("sta_network") < WLAN_NETWORK_NAME_MAX_LENGTH) ?
                                       (strlen("sta_network") + 1) :
                                       WLAN_NETWORK_NAME_MAX_LENGTH;
                strncpy(sta_network.name, "sta_network", network_name_len);

                ssid_len = (strlen(AP_SSID) <= IEEEtypes_SSID_SIZE) ? strlen(AP_SSID) : IEEEtypes_SSID_SIZE;
                memcpy(sta_network.ssid, (const char *)AP_SSID, ssid_len);
                sta_network.ip.ipv4.addr_type = ADDR_TYPE_DHCP;
                sta_network.ssid_specific     = 1;

                if (strlen(AP_PASSPHRASE))
                {
                    sta_network.security.type = WLAN_SECURITY_WILDCARD;
                    psk_len = (strlen(AP_PASSPHRASE) <= (WLAN_PSK_MAX_LENGTH - 1)) ? strlen(AP_PASSPHRASE) :
                                                                                     (WLAN_PSK_MAX_LENGTH - 1);
                    strncpy(sta_network.security.psk, AP_PASSPHRASE, psk_len);
                    sta_network.security.psk_len = psk_len;
                }
                else
                {
                    sta_network.security.type = WLAN_SECURITY_NONE;
                }

                ret = wlan_add_network(&sta_network);

                if (ret != 0)
                {
                    PRINTF(" Failed to add network %d\r\n", ret);
                    return 0;
                }
                network_added = 1;
            }

            if (xInitTaskNotify != NULL)
            {
                xTaskNotify(xInitTaskNotify, WM_SUCCESS, eSetValueWithOverwrite);
                xInitTaskNotify = NULL;
            }

            break;
        case WLAN_REASON_INITIALIZATION_FAILED:
            PRINTF("app_cb: WLAN: initialization failed\r\n");

            if (xInitTaskNotify != NULL)
            {
                xTaskNotify(xInitTaskNotify, WM_FAIL, eSetValueWithOverwrite);
                xInitTaskNotify = NULL;
            }

            break;
        case WLAN_REASON_ADDRESS_SUCCESS:
            PRINTF("network mgr: DHCP new lease\r\n");
            break;
        case WLAN_REASON_ADDRESS_FAILED:
            PRINTF("app_cb: failed to obtain an IP address\r\n");
            break;
        case WLAN_REASON_USER_DISCONNECT:
            PRINTF("app_cb: disconnected\r\n");
            break;
        case WLAN_REASON_LINK_LOST:
            PRINTF("app_cb: WLAN: link lost\r\n");
            break;
        case WLAN_REASON_SUCCESS:
            PRINTF("Connected to [%s]\r\n", sta_network.ssid);
            if (xJoinTaskNotify != NULL)
            {
                xTaskNotify(xJoinTaskNotify, WM_SUCCESS, eSetValueWithOverwrite);
                xJoinTaskNotify = NULL;
            }
            break;
        case WLAN_REASON_CONNECT_FAILED:
            PRINTF("[!] WLAN: connect failed\r\n");
            if (xJoinTaskNotify != NULL)
            {
                xTaskNotify(xJoinTaskNotify, WM_FAIL, eSetValueWithOverwrite);
                xJoinTaskNotify = NULL;
            }
            break;
        case WLAN_REASON_NETWORK_NOT_FOUND:
            PRINTF("[!] WLAN: network not found\r\n");
            if (xJoinTaskNotify != NULL)
            {
                xTaskNotify(xJoinTaskNotify, WM_FAIL, eSetValueWithOverwrite);
                xJoinTaskNotify = NULL;
            }
            break;
        case WLAN_REASON_NETWORK_AUTH_FAILED:
            PRINTF("[!] Network Auth failed\r\n");
            if (xJoinTaskNotify != NULL)
            {
                xTaskNotify(xJoinTaskNotify, WM_FAIL, eSetValueWithOverwrite);
                xJoinTaskNotify = NULL;
            }
            break;
        default:
            PRINTF("app_cb: WLAN: Unknown Event: %d\r\n", reason);
    }
    return 0;
}
void taskMain(void *param)
{
    int32_t result = 0;
    (void)result;
    ip4_addr_t ip;

    if (!ipaddr_aton(PING_ADDR, &ip))
    {
        PRINTF("Failed to convert %s to ip\r\n", PING_ADDR);
        __BKPT(0);
    }

    PRINTF("Initialize WLAN Driver\r\n");
    result = wlan_init(wlan_fw_bin, wlan_fw_bin_len);

    if (WM_SUCCESS != result)
    {
        PRINTF("Wlan initialization failed");
        __BKPT(0);
    }

    xInitTaskNotify = xTaskGetCurrentTaskHandle();
    result          = wlan_start(wlan_event_callback);
    if (WM_SUCCESS != result)
    {
        PRINTF("Couldn't start wlan\r\n");
        __BKPT(0);
    }

    // we need to wait for wi-fi initialization
    if (WM_SUCCESS == ulTaskNotifyTake(pdTRUE, portMAX_DELAY))
    {
        scan();
    }
    else
    {
        // WLAN: initialization failed
        __BKPT(0);
    }

    while (!firstResult)
    {
        os_thread_sleep(os_msec_to_ticks(500));
    }

    // Note down the Join task so that
    xJoinTaskNotify = xTaskGetCurrentTaskHandle();
    conToAp();
    if (WM_SUCCESS == ulTaskNotifyTake(pdTRUE, portMAX_DELAY))
    {
        ping_init(&ip);
    }
    else
    {
        PRINTF("Connection Failed! Stopping!\r\n");
        __BKPT(0);
    }

    for (;;)
        ;
}
int main(void)
{
    BaseType_t result = 0;
    (void)result;

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    APP_InitAppDebugConsole();

    PRINTF("Wifi setup example\r\n");

    result = xTaskCreate(taskMain, "main", taskMain_STACK_SIZE, taskMain_stack, taskMain_PRIO, &taskMain_task_handler);
    assert(pdPASS == result);

    vTaskStartScheduler();
    for (;;)
        ;
}
