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
#include "lwip/tcpip.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "wpl.h"
#include "timers.h"
#include "httpsrv.h"

#include "fsl_debug_console.h"
#include "httpsrv_freertos.h"

#include <stdio.h>

#include "FreeRTOS.h"

#include "fsl_common.h"
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef AP_SSID
#define AP_SSID "my_network"
#endif

#ifndef AP_PASSWORD
#define AP_PASSWORD "my_password"
#endif

#define WIFI_NETWORK_LABEL "my_wifi"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

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

/* Connect to the external AP */
static void ConnectTo()
{
    int32_t result;

    /* Add Wi-Fi network */
    result = WPL_AddNetwork(AP_SSID, AP_PASSWORD, WIFI_NETWORK_LABEL);
    if (result == WPLRET_SUCCESS)
    {
        PRINTF("Connecting as client to ssid: %s with password %s\r\n", AP_SSID, AP_PASSWORD);
        result = WPL_Join(WIFI_NETWORK_LABEL);
    }

    if (result != WPLRET_SUCCESS)
    {
        PRINTF("[!] Cannot connect to Wi-Fi\r\n[!]ssid: %s\r\n[!]passphrase: %s\r\n", AP_SSID, AP_PASSWORD);

        while (1)
        {
            __BKPT(0);
        }
    }
    else
    {
        PRINTF("[i] Connected to Wi-Fi\r\nssid: %s\r\n[!]passphrase: %s\r\n", AP_SSID, AP_PASSWORD);
        char ip[16];
        WPL_GetIP(ip, 1);
        PRINTF(" Now join that network on your device and connect to this IP: %s\r\n", ip);
    }
}

/*!
 * @brief The main task function
 */
static void main_task(void *arg)
{
    uint32_t result = 0;

    PRINTF(
        "\r\n"
        "Starting httpsrv DEMO\r\n");

    /* Initialize Wi-Fi board */
    PRINTF("[i] Initializing Wi-Fi connection... \r\n");

    result = WPL_Init();
    if (result != WPLRET_SUCCESS)
    {
        PRINTF("[!] WPL Init failed: %d\r\n", (uint32_t)result);
        __BKPT(0);
    }

    result = WPL_Start(LinkStatusChangeCallback);
    if (result != WPLRET_SUCCESS)
    {
        PRINTF("[!] WPL Start failed %d\r\n", (uint32_t)result);
        __BKPT(0);
    }

    PRINTF("[i] Successfully initialized Wi-Fi module\r\n");

    http_server_socket_init();

    ConnectTo();

    http_server_enable_mdns(netif_default, "wifi-http");

    http_server_print_ip_cfg(netif_default);

    PRINTF("\r\nReady\r\n\r\n");
    vTaskDelete(NULL);
}

/*!
 * @brief Main function.
 */
int main(void)
{
    /* Initialize the hardware */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Create the main Task */
    if (xTaskCreate(main_task, "main_task", 1024, NULL, configMAX_PRIORITIES - 4, NULL) != pdPASS)
    {
        PRINTF("[!] MAIN Task creation failed!\r\n");
        while (1)
            ;
    }

    /* Run RTOS */
    vTaskStartScheduler();

    /* Should not reach this statement */
    for (;;)
        ;
}
