/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
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
#include "network_cfg.h"
/*${header:end}*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
/*${macro:end}*/

/* @TEST_ANCHOR */

#ifndef WIFI_BANNER
#define WIFI_BANNER "Successfully initialized WiFi module\r\n"
#endif

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
int initNetwork()
{
    int result;

    /* Initialize WiFi board */
    PRINTF("Initializing WiFi connection... \r\n");

    if (((result = WPL_Init()) != WPL_SUCCESS) || ((result = WPL_Start()) != WPL_SUCCESS))
    {
        PRINTF("Could not initialize WiFi module %d\r\n", (uint32_t)result);
        return -1;
    }
    else
    {
        PRINTF(WIFI_BANNER);
    }

    if (wifi_ap_mode)
    {
        /* AP mode */
        PRINTF("Starting Access Point: SSID: %s, Chnl: %d\r\n", WIFI_SSID, WIFI_AP_CHANNEL);

        result = WPL_Start_AP(WIFI_SSID, WIFI_PASSWORD, WIFI_AP_CHANNEL);

        if (result != WPL_SUCCESS)
        {
            PRINTF("Failed to start access point\r\n");
            return -1;
        }

        /* Start DHCP server */
        WPL_StartDHCPServer(WIFI_AP_IP_ADDR, WIFI_AP_NET_MASK);

        char ip[16];
        WPL_GetIP(ip, 0);

        PRINTF("Join \'%s\' network and visit https://%s\r\n", WIFI_SSID, ip);

        return 0;
    }

    /* Client mode */
    PRINTF("Joining: " WIFI_SSID "\r\n");

    result = WPL_Join(WIFI_SSID, WIFI_PASSWORD);
    if (result != WPL_SUCCESS)
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
