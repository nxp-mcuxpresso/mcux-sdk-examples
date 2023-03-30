/** @file main.c
 *
 *  @brief main file
 *
 *  Copyright 2022-2023 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////

#include "FreeRTOS.h"
#include "task.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_shell.h"
#include "wpl.h"
#include "shell_task.h"

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/


#define main_task_PRIORITY          1
#define main_task_STACK_DEPTH       800
#define SHELL_ADDITIONAL_STACK_SIZE 256
#define DEMO_WIFI_LABEL             "MyWifi"

/*******************************************************************************
 * Variables
 ******************************************************************************/

static volatile bool wlan_connected = false;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static shell_status_t cmd_connect(void *shellHandle, int32_t argc, char **argv);
static shell_status_t cmd_scan(void *shellHandle, int32_t argc, char **argv);
static shell_status_t cmd_disconnect(void *shellHandle, int32_t argc, char **argv);

/*******************************************************************************
 * Code
 ******************************************************************************/

SHELL_COMMAND_DEFINE(wlan_scan, "\r\n\"wlan_scan\": Scans networks.\r\n", cmd_scan, 0);

SHELL_COMMAND_DEFINE(wlan_connect_with_password,
                     "\r\n\"wlan_connect_with_password ssid password\":\r\n"
                     "   Connects to the specified network with password.\r\n"
                     " Usage:\r\n"
                     "   ssid:        network SSID\r\n"
                     "   password:    password\r\n",
                     cmd_connect,
                     2);

SHELL_COMMAND_DEFINE(wlan_connect,
                     "\r\n\"wlan_connect ssid\":\r\n"
                     "   Connects to the specified network without password.\r\n"
                     " Usage:\r\n"
                     "   ssid:        network SSID\r\n",
                     cmd_connect,
                     1);

SHELL_COMMAND_DEFINE(wlan_disconnect,
                     "\r\n\"wlan_disconnect\":\r\n"
                     "   Disconnect from connected network\r\n",
                     cmd_disconnect,
                     0);

static void printSeparator(void)
{
    PRINTF("========================================\r\n");
}

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

void task_main(void *param)
{
    wpl_ret_t err = WPLRET_FAIL;

    static shell_command_t *wifi_commands[] = {
        SHELL_COMMAND(wlan_scan), SHELL_COMMAND(wlan_connect), SHELL_COMMAND(wlan_connect_with_password),
        SHELL_COMMAND(wlan_disconnect), NULL // end of list
    };

    PRINTF("Initialize WLAN \r\n");
    printSeparator();

    /* Initialize WIFI*/
    err = WPL_Init();
    if (err != WPLRET_SUCCESS)
    {
        PRINTF("[!] WPL_Init: Failed, error: %d\r\n", (uint32_t)err);
        while (true)
        {
            ;
        }
    }

    err = WPL_Start(LinkStatusChangeCallback);
    if (err != WPLRET_SUCCESS)
    {
        PRINTF("[!] WPL_Start: Failed, error: %d\r\n", (uint32_t)err);
        while (true)
        {
            ;
        }
    }

    PRINTF("Initialize CLI\r\n");
    printSeparator();
    shell_task_init(wifi_commands, SHELL_ADDITIONAL_STACK_SIZE);

    vTaskDelete(NULL);
}

static shell_status_t cmd_connect(void *shellHandle, int32_t argc, char **argv)
{
    (void)shellHandle;

    wpl_ret_t err;
    if (wlan_connected == true)
    {
        PRINTF("Leave network before connecting to a new one!\r\n");
        return kStatus_SHELL_Success;
    }

    if (argc < 3)
        err = WPL_AddNetwork(argv[1], "", DEMO_WIFI_LABEL);
    else
        err = WPL_AddNetwork(argv[1], argv[2], DEMO_WIFI_LABEL);

    if (err != WPLRET_SUCCESS)
    {
        PRINTF("Failed to add network profile!\r\n");
        return kStatus_SHELL_Success;
    }

    PRINTF("Joining: %s\r\n", argv[1]);
    err = WPL_Join(DEMO_WIFI_LABEL);
    if (err != WPLRET_SUCCESS)
    {
        PRINTF("Failed to join network!\r\n");
        if (WPL_RemoveNetwork(DEMO_WIFI_LABEL) != WPLRET_SUCCESS)
        {
            PRINTF("Failed to remove network!\r\n");
        }
        return kStatus_SHELL_Success;
    }

    PRINTF("Network joined\r\n");
    wlan_connected = true;
    return kStatus_SHELL_Success;
}

static shell_status_t cmd_disconnect(void *shellHandle, int32_t argc, char **argv)
{
    if (wlan_connected == false)
    {
        PRINTF("No network connected!\r\n");
        return kStatus_SHELL_Success;
    }

    if (WPL_Leave() != WPLRET_SUCCESS)
    {
        PRINTF("Failed to leave the network!\r\n");
        return kStatus_SHELL_Success;
    }

    if (WPL_RemoveNetwork(DEMO_WIFI_LABEL) != WPLRET_SUCCESS)
    {
        PRINTF("Failed to remove network profile!\r\n");
        return kStatus_SHELL_Success;
    }

    PRINTF("Disconnected from network\r\n");
    wlan_connected = false;
    return kStatus_SHELL_Success;
}

static shell_status_t cmd_scan(void *shellHandle, int32_t argc, char **argv)
{
    (void)shellHandle;
    (void)argc;
    char *scanData = NULL;

    PRINTF("\r\nInitiating scan...\r\n");
    scanData = WPL_Scan();
    if (scanData == NULL)
    {
        PRINTF("Error while scanning!\r\n");
    }
    else
    {
        vPortFree(scanData);
    }

    return kStatus_SHELL_Success;
}

int main(void)
{
    BaseType_t result = 0;
    (void)result;

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    printSeparator();

    result = xTaskCreate(task_main, "main", main_task_STACK_DEPTH, NULL, main_task_PRIORITY, NULL);
    assert(pdPASS == result);

    vTaskStartScheduler();
    for (;;)
        ;
}
