/** @file main.c
 *
 *  @brief main file
 *
 *  Copyright 2022 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "wlan_bt_fw.h"
#include "wlan.h"
#include "wifi.h"
#include "wm_net.h"
#include <wm_os.h>
#include "fsl_shell.h"

#include "shell_task.h"

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

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static shell_status_t cmd_connect(void *shellHandle, int32_t argc, char **argv);
static shell_status_t cmd_scan(void *shellHandle, int32_t argc, char **argv);
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

static const char *NET_NAME   = "my_net";
static const char *MAC_FORMAT = "%02x:%02x:%02x:%02x:%02x:%02x";

static volatile int wlan_connected   = 0;
static volatile int wlan_initialized = 0;

SHELL_COMMAND_DEFINE(wlan_scan, "\r\n\"wlan_scan\": Scans networks.\r\n", cmd_scan, 0);

SHELL_COMMAND_DEFINE(wlan_connect,
                     "\r\n\"wlan_connect ssid\":\r\n"
                     "   Connects to the specified network without password.\r\n"
                     " Usage:\r\n"
                     "   ssid:        network SSID or BSSID\r\n",
                     cmd_connect,
                     1);

SHELL_COMMAND_DEFINE(wlan_connect_with_password,
                     "\r\n\"wlan_connect_with_password ssid password\":\r\n"
                     "   Connects to the specified network with password.\r\n"
                     " Usage:\r\n"
                     "   ssid:        network SSID or BSSID\r\n"
                     "   password:    password\r\n",
                     cmd_connect,
                     2);

static void printSeparator(void)
{
    PRINTF("========================================\r\n");
}

static struct wlan_network sta_network;

/* Callback Function passed to WLAN Connection Manager. The callback function
 * gets called when there are WLAN Events that need to be handled by the
 * application.
 */
int wlan_event_callback(enum wlan_event_reason reason, void *data)
{
    int ret;
    struct wlan_ip_config addr;
    char ip[16];
    static int auth_fail = 0;

    printSeparator();
    PRINTF("app_cb: WLAN: received event %d\r\n", reason);
    printSeparator();

    switch (reason)
    {
        case WLAN_REASON_INITIALIZED:
            PRINTF("app_cb: WLAN initialized\r\n");
            printSeparator();
            wlan_initialized = 1;
            break;
        case WLAN_REASON_INITIALIZATION_FAILED:
            PRINTF("app_cb: WLAN: initialization failed\r\n");
            break;
        case WLAN_REASON_SUCCESS:
            wlan_connected = 1;
            PRINTF("app_cb: WLAN: connected to network\r\n");
            ret = wlan_get_address(&addr);
            if (ret != WM_SUCCESS)
            {
                PRINTF("failed to get IPv4 address\r\n");
                return 0;
            }

            net_inet_ntoa(addr.ipv4.address, ip);

            ret = wlan_get_current_network(&sta_network);
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to get External AP network\r\n");
                return 0;
            }

            PRINTF("Connected to following BSS:\r\n");
            PRINTF("SSID = [%s]\r\n", sta_network.ssid, ip);
            auth_fail = 0;

            break;
        case WLAN_REASON_CONNECT_FAILED:
            PRINTF("app_cb: WLAN: connect failed\r\n");
            break;
        case WLAN_REASON_NETWORK_NOT_FOUND:
            PRINTF("app_cb: WLAN: network not found\r\n");
            break;
        case WLAN_REASON_NETWORK_AUTH_FAILED:
            PRINTF("app_cb: WLAN: network authentication failed\r\n");
            auth_fail++;
            if (auth_fail >= 3)
            {
                PRINTF("Authentication Failed. Disconnecting ... \r\n");
                wlan_disconnect();
                auth_fail = 0;
            }
            break;
        case WLAN_REASON_ADDRESS_SUCCESS:
            PRINTF("network mgr: DHCP new lease\r\n");
            break;
        case WLAN_REASON_ADDRESS_FAILED:
            PRINTF("app_cb: failed to obtain an IP address\r\n");
            break;
        case WLAN_REASON_USER_DISCONNECT:
            wlan_connected = 0;
            PRINTF("app_cb: disconnected\r\n");
            auth_fail = 0;
            break;
        case WLAN_REASON_LINK_LOST:
            wlan_connected = 0;
            PRINTF("app_cb: WLAN: link lost\r\n");
            break;
        case WLAN_REASON_CHAN_SWITCH:
            PRINTF("app_cb: WLAN: channel switch\r\n");
            break;
        case WLAN_REASON_PS_ENTER:
            PRINTF("app_cb: WLAN: PS_ENTER\r\n");
            break;
        case WLAN_REASON_PS_EXIT:
            PRINTF("app_cb: WLAN: PS EXIT\r\n");
            break;
        default:
            PRINTF("app_cb: WLAN: Unknown Event: %d\r\n", reason);
    }
    return 0;
}

void task_main(void *param)
{
    int32_t err = 0;

    static shell_command_t *wifi_commands[] = {
        SHELL_COMMAND(wlan_scan), SHELL_COMMAND(wlan_connect), SHELL_COMMAND(wlan_connect_with_password),
        NULL // end of list
    };

    PRINTF("Initialize WLAN Driver\r\n");
    printSeparator();

    /* Initialize WIFI Driver */
    err = wlan_init(wlan_fw_bin, wlan_fw_bin_len);
    if (err)
        PRINTF("wlan_init() failed.\r\n");
    else
    {
        err = wlan_start(wlan_event_callback);
        if (err)
            PRINTF("wlan_start() failed.\r\n");
        else
        {
            while (!wlan_initialized)
                os_thread_sleep(os_msec_to_ticks(200));

            PRINTF("Initialize CLI\r\n");
            printSeparator();
            shell_task_init(wifi_commands, 256);
        }
    }

    vTaskDelete(NULL);
}

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static int wifi_scan_cb(unsigned count)
{
    struct wlan_scan_result res;
    const char *bb = res.bssid;
    int i;
    int err;

    PRINTF("\r\n%u network%s found:\r\n", count, (count <= 1) ? " was" : "s were");

    for (i = 0; i < count; i++)
    {
        err = wlan_get_scan_result(i, &res);
        if (err != 0)
        {
            PRINTF("Error: can't get scan res %d\r\n", i);
            continue;
        }

        PRINTF(" #%-3d", i + 1);
        PRINTF(MAC_FORMAT, bb[0], bb[1], bb[2], bb[3], bb[4], bb[5]);

        if (res.ssid[0] != '\0')
            PRINTF("\"%s\"\r\n", res.ssid);
        else
            PRINTF("(hidden)\r\n");
    }

    return 0;
}

static int fill_bssid_or_ssid(struct wlan_network *network, const char *arg_ssid)
{
    unsigned bbssid[6];
    int parsed_bssid_bytes_cnt =
        sscanf(arg_ssid, MAC_FORMAT, &bbssid[0], &bbssid[1], &bbssid[2], &bbssid[3], &bbssid[4], &bbssid[5]);
    if (parsed_bssid_bytes_cnt == 6)
    { // is BSSID
        int i;
        for (i = 0; i < 6; i++)
            network->bssid[i] = bbssid[i];
    }
    else
    {
        int ssid_len = strlen(arg_ssid);

        if (ssid_len > MLAN_MAX_SSID_LENGTH)
        {
            PRINTF("SSID tool long (max %d characters).", MLAN_MAX_SSID_LENGTH);
            return 1;
        }
        strcpy(network->ssid, arg_ssid);
    }

    return 0;
}

static int fill_password(struct wlan_network *network, const char *arg_pwd)
{
    const int len = strlen(arg_pwd);

    if (len > sizeof(network->security.password))
    {
        PRINTF("Password too long.\r\n");
        return 1;
    }

    memcpy(network->security.password, arg_pwd, len);
    network->security.password_len = len;

    if (len < sizeof(network->security.psk))
    {
        memcpy(network->security.psk, network->security.password, len);
        network->security.psk_len = len;
    }
    return 0;
}

static shell_status_t cmd_connect(void *shellHandle, int32_t argc, char **argv)
{
    (void)shellHandle;

    static int wlan_added = 0;

    int err;
    struct wlan_network network;

    memset(&network, 0, sizeof(network));
    strcpy(network.name, NET_NAME);
    network.ip.ipv4.addr_type = ADDR_TYPE_DHCP;

    err = fill_bssid_or_ssid(&network, argv[1]);
    if (err)
        return kStatus_SHELL_Success;

    if (argc > 2)
    {
        err = fill_password(&network, argv[2]);
        if (err)
            return kStatus_SHELL_Success;

        network.security.type = WLAN_SECURITY_WILDCARD;
    }
    else
    {
        network.security.type = WLAN_SECURITY_NONE;
    }

    if (wlan_connected)
    {
        err = wlan_disconnect();
        if (err)
            PRINTF("wlan_disconnect() failed with code %d\r\n", err);
    }

    if (wlan_added)
    {
        while (wlan_connected)
            os_thread_sleep(os_msec_to_ticks(500));

        err = wlan_remove_network(NET_NAME);
        if (err)
            PRINTF("wlan_remove_network() failed with code %d\r\n", err);
        wlan_added = 0;
    }

    err = wlan_add_network(&network);
    if (err)
    {
        PRINTF("wlan_add_network() failed with code %d\r\n", err);
        return kStatus_SHELL_Success;
    }
    wlan_added = 1;

    err = wlan_connect(network.name);
    if (err)
        PRINTF("wlan_connect() failed with code %d\r\n", err);
    else
        PRINTF("Connecting in progress. Wait for further messages from callback.\r\n", err);

    return kStatus_SHELL_Success;
}

static shell_status_t cmd_scan(void *shellHandle, int32_t argc, char **argv)
{
    (void)shellHandle;
    (void)argc;

    int err = wlan_scan(wifi_scan_cb);

    if (err)
        PRINTF("Failed to launch scan. (err=%d)\r\n", err);
    else
        PRINTF("Scanning\r\n");

    return kStatus_SHELL_Success;
}

int main(void)
{
    const int TASK_MAIN_PRIO       = OS_PRIO_3;
    const int TASK_MAIN_STACK_SIZE = 800;

    BaseType_t result = 0;
    (void)result;

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    APP_InitAppDebugConsole();

    printSeparator();

    result = xTaskCreate(task_main, "main", TASK_MAIN_STACK_SIZE, NULL, TASK_MAIN_PRIO, NULL);
    assert(pdPASS == result);

    vTaskStartScheduler();
    for (;;)
        ;
}
