/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "wifi_client.h"

#include "fsl_debug_console.h"
#include "FreeRTOS.h"
#include "task.h"
#include "wlan_bt_fw.h"
#include "user_config.h"
#include "wlan.h"
#include "wm_net.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static TaskHandle_t xInitTaskNotify = NULL;
static TaskHandle_t xJoinTaskNotify = NULL;

static struct wlan_network network;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*
 * Callback Function passed to WLAN Connection Manager. The callback function
 * gets called when there are WLAN Events that need to be handled by the
 * application.
 */
static int wlan_event_callback(enum wlan_event_reason event, void *data)
{
    struct wlan_ip_config addr;
    wifi_fw_version_ext_t ver;
    char ip[16];
    int ret;

    switch (event)
    {
        case WLAN_REASON_SUCCESS:
            ret = wlan_get_address(&addr);
            if (ret != WM_SUCCESS)
            {
                PRINTF("failed to get IP address\r\n");
                return 0;
            }

            net_inet_ntoa(addr.ipv4.address, ip);

            PRINTF("Connected to \"%s\" with IP = [%s]\r\n", network.name, ip);

            if (xJoinTaskNotify != NULL)
            {
                xTaskNotify(xJoinTaskNotify, pdPASS, eSetValueWithOverwrite);
                xJoinTaskNotify = NULL;
            }

            break;

        case WLAN_REASON_NETWORK_NOT_FOUND:
            PRINTF("Network \"%s\" not found\r\n", network.name);
            break;

        case WLAN_REASON_NETWORK_AUTH_FAILED:
            PRINTF("Authentication to network \"%s\" failed\r\n", network.name);
            break;

        case WLAN_REASON_USER_DISCONNECT:
            PRINTF("Disconnected from \"%s\"\r\n", network.name);
            break;

        case WLAN_REASON_INITIALIZED:
            PRINTF("WLAN initialized\r\n");

            /* Print WLAN FW Version */
            wifi_get_device_firmware_version_ext(&ver);
            PRINTF("WLAN FW version: %s\r\n", ver.version_str);

            if (xInitTaskNotify != NULL)
            {
                xTaskNotifyGive(xInitTaskNotify);
            }

            break;

        default:
            PRINTF("Unhandled wlan_event_callback: event=%d\r\n", event);
            break;
    }

    return 0;
}

static void wifi_client_wlan_validate(app_handle_t *app)
{
    if ((WIFI_SSID == NULL) || (strlen(WIFI_SSID) == 0))
    {
        PRINTF("[!] SSID is not set!\r\n");
        while (true)
        {
        }
    }

    if (strlen(WIFI_SSID) > IEEEtypes_SSID_SIZE)
    {
        PRINTF("[!] SSID is too long!\r\n");
        while (true)
        {
        }
    }

    if (WIFI_PASSWORD == NULL)
    {
        PRINTF("[!] Password is not set!\r\n");
        while (true)
        {
        }
    }

    if (strlen(WIFI_PASSWORD) > (WLAN_PSK_MAX_LENGTH - 1))
    {
        PRINTF("[!] Password is too long!\r\n");
        while (true)
        {
        }
    }
}

static void wifi_client_wlan_init(app_handle_t *app)
{
    int32_t result;

    PRINTF("Initializing WLAN\r\n");

    result = wlan_init(wlan_fw_bin, wlan_fw_bin_len);

    if (result != WM_SUCCESS)
    {
        PRINTF("WLAN initialization failed\r\n");
        while (true)
        {
        }
    }
}

static void wifi_client_wlan_start(app_handle_t *app)
{
    int32_t result;

    PRINTF("Starting WLAN\r\n");

    xInitTaskNotify = xTaskGetCurrentTaskHandle();
    result          = wlan_start(wlan_event_callback);

    if (result != WM_SUCCESS)
    {
        PRINTF("WLAN failed to start\r\n");
        while (true)
        {
        }
    }

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

static void wifi_client_wlan_connect(app_handle_t *app)
{
    int result;
    size_t len;

    xJoinTaskNotify = xTaskGetCurrentTaskHandle();

    (void)memset(&network, 0, sizeof(struct wlan_network));

    len = strlen(WIFI_SSID);
    if (len < sizeof(network.name))
    {
        (void)strcpy(network.name, (const char *)WIFI_SSID);
    }
    else
    {
        (void)strncpy(network.name, (const char *)WIFI_SSID, sizeof(network.name) - 1U);
        network.name[sizeof(network.name) - 1U] = '\0';
    }

    (void)strcpy(network.ssid, (const char *)WIFI_SSID);
    network.ip.ipv4.addr_type = ADDR_TYPE_DHCP;
    network.ssid_specific     = 1;

    network.security.type = WLAN_SECURITY_NONE;

    len = strlen(WIFI_PASSWORD);
    if (len > 0U)
    {
        network.security.type = WLAN_SECURITY_WILDCARD;
        (void)strncpy(network.security.psk, WIFI_PASSWORD, len);
        network.security.psk_len = (char)len;
    }

    result = wlan_add_network(&network);
    if (result != WM_SUCCESS)
    {
        (void)PRINTF("Failed to add network \"%s\"\r\n", network.name);
        while (true)
        {
        }
    }

    (void)PRINTF("Connecting to \"%s\"\r\n", network.name);
    result = wlan_connect(network.name);

    if (result != WM_SUCCESS)
    {
        (void)PRINTF("Failed to connect %d\r\n", result);
        while (true)
        {
        }
    }

    // Wait for response
    if (pdPASS != ulTaskNotifyTake(pdTRUE, portMAX_DELAY))
    {
        (void)PRINTF("ulTaskNotifyTake failed\r\n");
        while (true)
        {
        }
    }
}

static void wifi_client_task(void *param)
{
    app_handle_t *app = (app_handle_t *)param;

    PRINTF("[wifi_client_task] start\r\n");

    wifi_client_wlan_validate(app);
    wifi_client_wlan_init(app);
    wifi_client_wlan_start(app);
    wifi_client_wlan_connect(app);

    xTaskNotifyGive(app->rtp_receiver_task_handle);

    PRINTF("[wifi_client_task] done\r\n");

    vTaskDelete(NULL);
}

void wifi_client_init(app_handle_t *app)
{
    TaskHandle_t taskHandle;

    if (xTaskCreate(wifi_client_task, "wifi_client_task", WIFI_CLIENT_TASK_STACK_SIZE / sizeof(configSTACK_DEPTH_TYPE),
                    app, WIFI_CLIENT_TASK_PRIORITY, &taskHandle) != pdPASS)
    {
        PRINTF("\r\nFailed to create WiFi client task\r\n");
        while (true)
        {
        }
    }
}
