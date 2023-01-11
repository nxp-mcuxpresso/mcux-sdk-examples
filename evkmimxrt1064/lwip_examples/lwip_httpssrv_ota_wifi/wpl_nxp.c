/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "wpl.h"
#include "network_cfg.h"
#include "wlan_bt_fw.h"
#include "wlan.h"
#include "wifi.h"
#include "wm_net.h"
#include <wm_os.h>
#include "dhcp-server.h"
#include <stdio.h>

static TaskHandle_t xInitTaskNotify  = NULL;
static TaskHandle_t xScanTaskNotify  = NULL;
static TaskHandle_t xLeaveTaskNotify = NULL;
static TaskHandle_t xUapTaskNotify   = NULL;
static TaskHandle_t xJoinTaskNotify  = NULL;

char ssids_json[2048];

static struct wlan_network sta_network;
static struct wlan_network uap_network;

/* Callback Function passed to WLAN Connection Manager. The callback function
 * gets called when there are WLAN Events that need to be handled by the
 * application.
 */
static int wlan_event_callback(enum wlan_event_reason reason, void *data)
{
    wifi_fw_version_ext_t ver;
    char ip[16];

    switch (reason)
    {
        case WLAN_REASON_SUCCESS:
            WPL_GetIP(ip, 1);
            PRINTF("Connected to following BSS:");
            PRINTF("SSID = [%s], IP = [%s]\r\n", sta_network.ssid, ip);
            if (xJoinTaskNotify != NULL)
            {
                xTaskNotify(xJoinTaskNotify, WPL_SUCCESS, eSetValueWithOverwrite);
                xJoinTaskNotify = NULL;
            }
            break;
        case WLAN_REASON_CONNECT_FAILED:
            PRINTF("[!] WLAN: connect failed\r\n");

            if (xJoinTaskNotify != NULL)
            {
                xTaskNotify(xJoinTaskNotify, WPL_ERROR, eSetValueWithOverwrite);
                xJoinTaskNotify = NULL;
            }
            break;
        case WLAN_REASON_NETWORK_NOT_FOUND:
            PRINTF("[!] WLAN: network not found\r\n");

            if (xJoinTaskNotify != NULL)
            {
                xTaskNotify(xJoinTaskNotify, WPL_ERROR, eSetValueWithOverwrite);
                xJoinTaskNotify = NULL;
            }
            break;
        case WLAN_REASON_NETWORK_AUTH_FAILED:
            PRINTF("[!] Network Auth failed\r\n");

            if (xJoinTaskNotify != NULL)
            {
                xTaskNotify(xJoinTaskNotify, WPL_ERROR, eSetValueWithOverwrite);
                xJoinTaskNotify = NULL;
            }
            break;
        case WLAN_REASON_ADDRESS_SUCCESS:
            // PRINTF("network mgr: DHCP new lease\r\n");
            break;
        case WLAN_REASON_ADDRESS_FAILED:
            PRINTF("[!] failed to obtain an IP address\r\n");

            if (xJoinTaskNotify != NULL)
            {
                xTaskNotify(xJoinTaskNotify, WPL_ERROR, eSetValueWithOverwrite);
                xJoinTaskNotify = NULL;
            }
            break;

        case WLAN_REASON_LINK_LOST:
            break;
        case WLAN_REASON_CHAN_SWITCH:
            break;

        case WLAN_REASON_WPS_DISCONNECT:
            break;
        case WLAN_REASON_USER_DISCONNECT:
            PRINTF("Dis-connected from: %s\r\n", sta_network.ssid);

            // Remove the network and return
            wlan_remove_network(sta_network.name);
            // Notify the WPL_Leave task only if this has been called from a WPL_Leave task
            if (xLeaveTaskNotify != NULL)
            {
                xTaskNotifyGive(xLeaveTaskNotify);
                // Retset the task notification handle back to NULL
                xLeaveTaskNotify = NULL;
            }
            break;

        case WLAN_REASON_INITIALIZED:
            PRINTF("WLAN initialized\r\n");
            /* Print WLAN FW Version */
            wifi_get_device_firmware_version_ext(&ver);
            PRINTF("WLAN FW Version: %s\r\n", ver.version_str);
            if (xInitTaskNotify != NULL)
            {
                xTaskNotifyGive(xInitTaskNotify);
            }
            break;
        case WLAN_REASON_INITIALIZATION_FAILED:
            PRINTF("app_cb: WLAN: initialization failed\r\n");
            break;

        case WLAN_REASON_PS_ENTER:
            break;
        case WLAN_REASON_PS_EXIT:
            break;

        case WLAN_REASON_UAP_SUCCESS:
            PRINTF("Soft AP started successfully\r\n");
            if (xUapTaskNotify != NULL)
            {
                xTaskNotifyGive(xUapTaskNotify);
                xUapTaskNotify = NULL;
            }

            break;

        case WLAN_REASON_UAP_CLIENT_ASSOC:
            PRINTF("Client => ");
            print_mac((const char *)data);
            PRINTF("Associated with Soft AP\r\n");
            break;
        case WLAN_REASON_UAP_CLIENT_DISSOC:
            PRINTF("Client => ");
            print_mac((const char *)data);
            PRINTF("Dis-Associated from Soft AP\r\n");
            break;

        case WLAN_REASON_UAP_START_FAILED:
            PRINTF("[!] Failed to start AP\r\n");
            break;
        case WLAN_REASON_UAP_STOP_FAILED:
            PRINTF("[!] Failed to stop AP\r\n");
            break;
        case WLAN_REASON_UAP_STOPPED:
            wlan_remove_network(uap_network.name);
            PRINTF("Soft AP stopped successfully\r\n");
            if (xUapTaskNotify != NULL)
            {
                xTaskNotifyGive(xUapTaskNotify);
                xUapTaskNotify = NULL;
            }

            break;
        default:
            PRINTF("Unknown Wifi CB Reason %d\r\n", reason);
            break;
    }

    return WPL_SUCCESS;
}

int WPL_Init()
{
    int32_t result;
    result = wlan_init(wlan_fw_bin, wlan_fw_bin_len);
    if (result == WM_SUCCESS)
        return WPL_SUCCESS;
    else
        return result;
}

int WPL_Start()
{
    int32_t result;
    xInitTaskNotify = xTaskGetCurrentTaskHandle();
    result          = wlan_start(wlan_event_callback);

    if (result == WM_SUCCESS)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        return WPL_SUCCESS;
    }
    else
    {
        return result;
    }
}

int WPL_Stop()
{
    int32_t result = wlan_stop();
    if (result == WM_SUCCESS)
    {
        return WPL_SUCCESS;
    }
    return result;
}

int WPL_Start_AP(char *ssid, char *password, int chan)
{
    int ret;
    uint8_t psk_len = 0;
    xUapTaskNotify  = xTaskGetCurrentTaskHandle();

    if (strlen(ssid) > WPL_WIFI_SSID_LENGTH)
    {
        PRINTF("[!] SSID is too long!\r\n");
        __BKPT(0);
        return WPL_ERROR;
    }

    psk_len = strlen(password);
    if (psk_len > WLAN_PSK_MAX_LENGTH - 1)
    {
        PRINTF("[!] Password is too long!\r\n");
        __BKPT(0);
        return WPL_ERROR;
    }
    else if (psk_len && (psk_len < WLAN_PSK_MIN_LENGTH))
    {
        PRINTF("[!] Password is too short!\r\n");
        __BKPT(0);
        return WPL_ERROR;
    }

    wlan_initialize_uap_network(&uap_network);

    unsigned int uap_ip = ipaddr_addr(WIFI_AP_IP_ADDR);

    /* Set IP address to WIFI_AP_IP_ADDR */
    uap_network.ip.ipv4.address = uap_ip;
    /* Set default gateway to WIFI_AP_IP_ADDR */
    uap_network.ip.ipv4.gw = uap_ip;

    /* Set SSID as passed by the user */
    strncpy(uap_network.ssid, ssid, IEEEtypes_SSID_SIZE + 1);

    uap_network.channel = chan;

    uap_network.security.type = psk_len ? WLAN_SECURITY_WPA2 : WLAN_SECURITY_NONE;

    /* Set the passphrase. Max WPA2 passphrase can be upto 64 ASCII chars */
    strncpy(uap_network.security.psk, password, WLAN_PSK_MAX_LENGTH);
    uap_network.security.psk_len = psk_len;

    ret = wlan_add_network(&uap_network);
    if (ret != WM_SUCCESS)
    {
        return WPL_ERROR;
    }
    ret = wlan_start_network(uap_network.name);
    if (ret != WM_SUCCESS)
    {
        return WPL_ERROR;
    }

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    return WPL_SUCCESS;
}

int WPL_Stop_AP()
{
    int ret;
    xUapTaskNotify = xTaskGetCurrentTaskHandle();

    ret = wlan_stop_network(uap_network.name);
    if (ret != WM_SUCCESS)
    {
        return WPL_ERROR;
    }

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    wlan_remove_network(uap_network.name);

    return WPL_SUCCESS;
}

static int WLP_process_results(unsigned int count)
{
    struct wlan_scan_result scan_result;
    char network_buf[512];

    // Start building JSON
    strcpy(ssids_json, "[");

    for (int i = 0; i < count; i++)
    {
        wlan_get_scan_result(i, &scan_result);

        PRINTF("%s\r\n", scan_result.ssid);
        PRINTF("     BSSID         : %02X:%02X:%02X:%02X:%02X:%02X\r\n", (unsigned int)scan_result.bssid[0],
               (unsigned int)scan_result.bssid[1], (unsigned int)scan_result.bssid[2],
               (unsigned int)scan_result.bssid[3], (unsigned int)scan_result.bssid[4],
               (unsigned int)scan_result.bssid[5]);
        PRINTF("     RSSI          : %ddBm", -(int)scan_result.rssi);
        PRINTF("\r\n");

        PRINTF("     Channel       : %d\r\n", (int)scan_result.channel);

        char security[64];
        security[0] = '\0';

        if (scan_result.wpa2_entp)
        {
            strcat(security, "WPA2_ENTP ");
        }
        if (scan_result.wep)
        {
            strcat(security, "WEP ");
        }
        if (scan_result.wpa)
        {
            strcat(security, "WPA ");
        }
        if (scan_result.wpa2)
        {
            strcat(security, "WPA2 ");
        }
        if (scan_result.wpa3_sae)
        {
            strcat(security, "WPA3_SAE ");
        }

        if (strlen(ssids_json) + 256 > sizeof(ssids_json))
        {
            PRINTF("[!] The SSID_JSON is too small, can not fill all the SSIDS \r\n");
            // We could just stop here and return....
            __BKPT(0);
            xTaskNotify(xScanTaskNotify, WPL_ERROR, eSetValueWithOverwrite);
            return WPL_ERROR;
        }

        if (i != 0)
        {
            // Add , separator after first entry
            strcat(ssids_json, ",");
        }

        snprintf(network_buf, sizeof(network_buf) - 1,
                 "{\"ssid\":\"%s\",\"bssid\":\"%02X:%02X:%02X:%02X:%02X:%02X\",\"signal\":\"%ddBm\",\"channel\":%d,"
                 "\"security\":\"%s\"}",
                 scan_result.ssid, (unsigned int)scan_result.bssid[0], (unsigned int)scan_result.bssid[1],
                 (unsigned int)scan_result.bssid[2], (unsigned int)scan_result.bssid[3],
                 (unsigned int)scan_result.bssid[4], (unsigned int)scan_result.bssid[5], -(int)scan_result.rssi,
                 (int)scan_result.channel, security);
        strcat(ssids_json, network_buf);
    }

    strcat(ssids_json, "]");
    xTaskNotify(xScanTaskNotify, WPL_SUCCESS, eSetValueWithOverwrite);
    return WPL_SUCCESS;
}

int WPL_Scan()
{
    xScanTaskNotify = xTaskGetCurrentTaskHandle();
    PRINTF("\nInitiating scan...\n\n");
    wlan_scan(WLP_process_results);
    return ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

int WPL_Join(char *ssid, char *password)
{
    int ret;
    uint32_t psk_len = 0;
    // Note down the Join task so that
    xJoinTaskNotify = xTaskGetCurrentTaskHandle();

    if (strlen(ssid) > WPL_WIFI_SSID_LENGTH)
    {
        PRINTF("[!] SSID is too long!\r\n");
        __BKPT(0);
        return WPL_ERROR;
    }

    psk_len = strlen(password);
    if (psk_len > WLAN_PSK_MAX_LENGTH - 1)
    {
        PRINTF("[!] Password is too long!\r\n");
        __BKPT(0);
        return WPL_ERROR;
    }
    else if (psk_len && (psk_len < WLAN_PSK_MIN_LENGTH))
    {
        PRINTF("[!] Password is too short!\r\n");
        __BKPT(0);
        return WPL_ERROR;
    }

    memset(&sta_network, 0, sizeof(struct wlan_network));

    strcpy(sta_network.name, "sta_network");

    strncpy(sta_network.ssid, ssid, IEEEtypes_SSID_SIZE + 1);
    sta_network.ip.ipv4.addr_type = ADDR_TYPE_DHCP;
    sta_network.ssid_specific     = 1;

    sta_network.security.type = WLAN_SECURITY_NONE;
    if (psk_len > 0)
    {
        sta_network.security.type = WLAN_SECURITY_WILDCARD;
        strncpy(sta_network.security.psk, password, WLAN_PSK_MAX_LENGTH);
        sta_network.security.psk_len = psk_len;
    }

    ret = wlan_add_network(&sta_network);
    if (ret != WM_SUCCESS)
    {
        return WPL_ERROR;
    }

    ret = wlan_connect(sta_network.name);

    if (ret != WM_SUCCESS)
    {
        PRINTF("Failed to connect %d\r\n", ret);
        return WPL_ERROR;
    }

    // Wait for response
    if (WPL_SUCCESS == ulTaskNotifyTake(pdTRUE, portMAX_DELAY))
    {
        return WPL_SUCCESS;
    }
    else
    {
        WPL_Leave();
        return WPL_ERROR;
    }
}

int WPL_Leave()
{
    // Note down the current task handle so that it can be notified after Leave is done
    xLeaveTaskNotify = xTaskGetCurrentTaskHandle();
    int ret          = wlan_disconnect();

    if (ret != WM_SUCCESS)
    {
        return WPL_ERROR;
    }

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    return WPL_SUCCESS;
}

int WPL_StartDHCPServer(char *ip, char *net)
{
    // The IP and Net parameter are not used on NXP Wi-Fi
    // It is instead extracted form the uap net handle
    char current_ip[16];
    WPL_GetIP(current_ip, 0);
    PRINTF("Starting DHCP Server with IP %s\r\n", current_ip);
    if (dhcp_server_start(net_get_uap_handle()))
    {
        PRINTF("Error in starting dhcp server\r\n");
        return WPL_ERROR;
    }
    return WPL_SUCCESS;
}

int WPL_StopDHCPServer()
{
    dhcp_server_stop();
    return WPL_SUCCESS;
}

char *WPL_getSSIDs()
{
    return ssids_json;
}

int WPL_GetIP(char *ip, int client)
{
    struct wlan_ip_config addr;
    int ret;

    if (client)
        ret = wlan_get_address(&addr);
    else
        ret = wlan_get_uap_address(&addr);

    if (ret != WM_SUCCESS)
    {
        PRINTF("failed to get IP address");
        __BKPT(0);
        return WPL_ERROR;
    }

    net_inet_ntoa(addr.ipv4.address, ip);

    return WPL_SUCCESS;
}
