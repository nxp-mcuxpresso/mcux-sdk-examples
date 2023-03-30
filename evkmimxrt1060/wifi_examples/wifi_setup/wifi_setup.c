/** @file wifi_setup.c
 *
 *  @brief main file
 *
 *  Copyright 2020-2023 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

// SDK Included Files
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "lwip/tcpip.h"
#include "ping.h"
#include "wpl.h"
#include "stdbool.h"

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/


#ifndef main_task_PRIORITY
#define main_task_PRIORITY 0
#endif

#ifndef main_task_STACK_DEPTH
#define main_task_STACK_DEPTH 800
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/

static char ssid[WPL_WIFI_SSID_LENGTH];
static char password[WPL_WIFI_PASSWORD_LENGTH];
static ip4_addr_t ip;
TaskHandle_t mainTaskHandle;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void LinkStatusChangeCallback(bool linkState);
static void promptJoinNetwork(void);
static void promptPingAddress(void);
static void main_task(void *param);

/*******************************************************************************
 * Code
 ******************************************************************************/

/* Link lost callback */
static void LinkStatusChangeCallback(bool linkState)
{
    if (linkState == false)
    {
        PRINTF("-------- LINK LOST --------\r\n");
    }
    else
    {
        PRINTF("-------- LINK REESTABLISHED --------\r\n");
    }
}

static void promptJoinNetwork(void)
{
    wpl_ret_t result;
    int i = 0;
    char ch;

    while (true)
    {
        PRINTF("\r\nPlease enter parameters of WLAN to connect\r\n");

        /* SSID prompt */
        PRINTF("\r\nSSID: ");
        i = 0;
        while ((ch = GETCHAR()) != '\r' && i < WPL_WIFI_SSID_LENGTH - 1)
        {
            ssid[i] = ch;
            PUTCHAR(ch);
            i++;
        }
        ssid[i] = '\0';

        /* Password prompt */
        PRINTF("\r\nPassword (for unsecured WLAN press Enter): ");
        i = 0;
        while ((ch = GETCHAR()) != '\r' && i < WPL_WIFI_PASSWORD_LENGTH - 1)
        {
            password[i] = ch;
            PUTCHAR('*');
            i++;
        }
        password[i] = '\0';
        PRINTF("\r\n");

        /* Add Wifi network as known network */
        result = WPL_AddNetwork(ssid, password, ssid);
        if (result != WPLRET_SUCCESS)
        {
            PRINTF("[!] WPL_AddNetwork: Failed to add network, error:  %d\r\n", (uint32_t)result);
            continue;
        }
        PRINTF("[i] WPL_AddNetwork: Success\r\n");

        /* Join the network using label */
        PRINTF("[i] Trying to join the network...\r\n");
        result = WPL_Join(ssid);
        if (result != WPLRET_SUCCESS)
        {
            PRINTF("[!] WPL_Join: Failed to join network, error: %d\r\n", (uint32_t)result);
            if (WPL_RemoveNetwork(ssid) != WPLRET_SUCCESS)
                __BKPT(0);
            continue;
        }
        PRINTF("[i] WPL_Join: Success\r\n");

        /* SSID and password was OK, exit the prompt */
        break;
    }
}

static void promptPingAddress(void)
{
    int i = 0;
    char ip_string[IP4ADDR_STRLEN_MAX];
    char ch;

    while (true)
    {
        PRINTF("\r\nPlease enter a valid IPv4 address to test the connection\r\n");

        /* Ping IP address prompt */
        PRINTF("\r\nIP address: ");
        i = 0;
        while ((ch = GETCHAR()) != '\r' && i < IP4ADDR_STRLEN_MAX - 1)
        {
            ip_string[i] = ch;
            PUTCHAR(ch);
            i++;
        }
        ip_string[i] = '\0';
        PRINTF("\r\n");

        if (ipaddr_aton(ip_string, &ip) == 0)
        {
            PRINTF("[!] %s is not a valid IPv4 address\r\n", ip_string);
            continue;
        }

        /* Ping IP address was OK */
        break;
    }
}

static void main_task(void *param)
{
    wpl_ret_t result;
    char *scan_result;

    PRINTF(
        "\r\n"
        "Starting wifi_setup DEMO\r\n");

    result = WPL_Init();
    if (result != WPLRET_SUCCESS)
    {
        PRINTF("[!] WPL_Init: Failed, error: %d\r\n", (uint32_t)result);
        __BKPT(0);
    }
    PRINTF("[i] WPL_Init: Success\r\n");

    result = WPL_Start(LinkStatusChangeCallback);
    if (result != WPLRET_SUCCESS)
    {
        PRINTF("[!] WPL_Start: Failed, error: %d\r\n", (uint32_t)result);
        __BKPT(0);
    }
    PRINTF("[i] WPL_Start: Success\r\n");

    /* Scan the local area for available Wifi networks. The scan will print the results to the terminal
     * and return the results as JSON string */
    PRINTF("\r\nInitiating scan...\r\n\r\n");
    scan_result = WPL_Scan();
    if (scan_result == NULL)
    {
        PRINTF("[!] WPL_Scan: Failed to scan\r\n");
        __BKPT(0);
    }
    vPortFree(scan_result);

    promptJoinNetwork();
    promptPingAddress();

    /* Setup a ping task to test the connection */
    PRINTF("Starting ping task...\r\n");
    ping_init(&ip);

    vTaskDelete(NULL);
}

int main(void)
{
    /* Initialize the hardware */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Create the main Task */
    if (xTaskCreate(main_task, "main_task", main_task_STACK_DEPTH, NULL, main_task_PRIORITY, &mainTaskHandle) != pdPASS)
    {
        PRINTF("[!] MAIN Task creation failed!\r\n");
        while (true)
        {
            ;
        }
    }

    /* Run RTOS */
    vTaskStartScheduler();

    while (true)
    {
        ;
    }
}
