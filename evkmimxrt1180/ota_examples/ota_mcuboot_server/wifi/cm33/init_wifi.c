/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020, 2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
/*${header:start}*/
#include "fsl_debug_console.h"

#include "wpl.h"
#include "wifi.h"
#include "wlan.h"
#include "network_cfg.h"
/*${header:end}*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
#ifdef RW610
#include "rw61x_wifi_bin.h"
#endif
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
/*${prototype:end}*/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*${variable:start}*/
/* Set to true for AP mode, false for client mode */
bool wifi_ap_mode = WIFI_AP_MODE;
/*${variable:end}*/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*${function:start}*/

/* Link lost callback */
static void LinkStatusChangeCallback(bool linkState)
{
    if (linkState == false)
    {
        /* -------- LINK LOST -------- */
        /* DO SOMETHING */
        PRINTF("-------- LINK LOST --------\r\n");
    }
    else
    {
        /* -------- LINK REESTABLISHED -------- */
        /* DO SOMETHING */
        PRINTF("-------- LINK REESTABLISHED --------\r\n");
    }
}

int initNetwork()
{
    uint32_t result;

    /* Initialize WiFi board */
    PRINTF("Initializing WiFi connection... \r\n");

    result = WPL_Init();
    if (result != WPLRET_SUCCESS)
    {
        PRINTF("WPL_Init failed: %d\r\n", (uint32_t)result);
        return -1;
    }
    
#if defined(RW610) && defined(WPL_NO_WLAN_INIT)
    result = wlan_init(rw61x_wifi_bin, RW61X_WIFI_BIN_LEN);
    if (result != WM_SUCCESS)
    {
        PRINTF("wlan_init failed with %d\n", result);
        return -1;
    }
#endif

    result = WPL_Start(LinkStatusChangeCallback);
    if (result != WPLRET_SUCCESS)
    {
        PRINTF("WPL_Start failed: %d\r\n", (uint32_t)result);
        return -1;
    }

    PRINTF("Successfully initialized WiFi module\r\n");

    if (wifi_ap_mode)
    {
        /* AP mode */
        PRINTF("Starting Access Point: SSID: %s, Chnl: %d\r\n", WIFI_SSID, WIFI_AP_CHANNEL);

        result = WPL_Start_AP(WIFI_SSID, WIFI_PASSWORD, WIFI_AP_CHANNEL);
        if (result != WPLRET_SUCCESS)
        {
            PRINTF("Failed to start access point\r\n");
            return -1;
        }

        char ip[16];
        WPL_GetIP(ip, 0);

        PRINTF("Join \'%s\' network and visit http://%s\r\n", WIFI_SSID, ip);

        return 0;
    }

    /* Client mode */
    PRINTF("Joining: " WIFI_SSID "\r\n");

    /* Add Wi-Fi network */
    result = WPL_AddNetwork(WIFI_SSID, WIFI_PASSWORD, WIFI_SSID);
    if (result != WPLRET_SUCCESS)
    {
        PRINTF("WPL_AddNetwork failed: %d\r\n", (uint32_t)result);
        return -1;
    }

    result = WPL_Join(WIFI_SSID);
    if (result != WPLRET_SUCCESS)
    {
        PRINTF("Failed to join: " WIFI_SSID "\r\n");
        return -1;
    }
    else
    {
        PRINTF("Successfully joined: " WIFI_SSID "\r\n");

        char ip[16];
        WPL_GetIP(ip, 1);

        PRINTF("Visit https://%s\r\n", ip);
    }

    return 0;
}
/*${function:end}*/
