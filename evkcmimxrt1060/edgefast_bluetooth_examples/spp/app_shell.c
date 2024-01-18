/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno/errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <porting.h>

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "ff.h"
#include <stdbool.h>
#include <sys/atomic.h>
#include <sys/byteorder.h>
#include <sys/util.h>
#include <sys/slist.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/addr.h>
#include "bluetooth/spp.h"
#include "fsl_debug_console.h"
#include "fsl_shell.h"
#include "app_shell.h"
#include "app_spp.h"
#include "app_discover.h"
#include "app_connect.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BT_DEVICE_ADDR_POINTER(ref)\
        (ref)[0],(ref)[1],(ref)[2],(ref)[3],(ref)[4],(ref)[5]

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static uint8_t shell_parse_parameter(int32_t argc, char **argv);

/* callback function handling bt command */
static shell_status_t shell_bt(shell_handle_t shellHandle, int32_t argc, char **argv);

static int spp_sdp_discover_callback(struct bt_conn *conn, uint8_t count, uint16_t *channel);

/* callback function handling spp command */
static shell_status_t shell_spp(shell_handle_t shellHandle, int32_t argc, char **argv);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* spp shell handle */
static shell_handle_t spp_shell_handle;

/* spp shell handle buffer */
SDK_ALIGN(static uint8_t spp_shell_handle_buffer[SHELL_HANDLE_SIZE], 4);

/* bt command */
SHELL_COMMAND_DEFINE(bt,
                     "\r\n\"bt\": BT related function\r\n"
                     "  USAGE: bt <discover|connect|disconnect|delete>\r\n"
                     "    bt conns          print all active bt connection\r\n"
                     "    bt switch <index> switch a bt connection\r\n"
                     "    bt discover       start to find BT devices\r\n"
                     "    bt connect        connect to the device that is found, for example: bt connectdevice n (from 1)\r\n"
                     "    bt disconnect     disconnect current connection.\r\n"
                     "    bt delete         delete all devices. Ensure to disconnect the HCI link connection with the peer device before attempting to delete the bonding information.\r\n",
                     shell_bt,
                     SHELL_IGNORE_PARAMETER_COUNT);

/* spp command */
SHELL_COMMAND_DEFINE(spp,
                     "\r\n\"spp\": SPP related function\r\n"
                     "  USAGE: \r\n"
                     "    spp handle                display active spp handle list\r\n"
                     "    spp switch <hanlde>       switch spp handle\r\n"
                     "    spp register <cid>        register a spp server channel(cid)\r\n"
                     "    spp discover              discover spp server channel on peer device\r\n"
                     "    spp connect <cid>         create spp connection\r\n"
                     "    spp disconnect            disconnect current spp connection.\r\n"
                     "    spp send <1|2|3|4>        send data over spp connection.\r\n"
                     "    spp get_port <s|c> <cid>  get spp port setting of server/client channel(cid).\r\n"
                     "    spp set_port <s|c> <cid>  set spp port setting of server/client channel(cid).\r\n"
                     "    spp set_pn <s|c> <cid>    set pn of server/client channel(cid).\r\n"
                     "    spp get_pn <s|c> <cid>    get local pn of server/client channel(cid).\r\n"
                     "    spp send_rls              send rls.\r\n"
                     "    spp send_msc              send msc.\r\n",
                     shell_spp,
                     SHELL_IGNORE_PARAMETER_COUNT);

static discover_cb_t discover_callback =
{
    .cb = spp_sdp_discover_callback,
};

/*******************************************************************************
 * Code
 ******************************************************************************/
static uint8_t shell_parse_parameter(int32_t argc, char **argv)
{
    uint8_t select_index = 0U;
    char   *ch = argv[argc-1];

    for (select_index = 0U; select_index < strlen(ch); select_index++)
    {
        if ((ch[select_index] < '0') || (ch[select_index] > '9'))
        {
            PRINTF("the parameter is wrong\r\n");
            return 0xFFU;
        }
    }

    switch (strlen(ch))
    {
    case 1:
        select_index = ch[0] - '0';
        break;
    case 2:
        select_index = (ch[0] - '0') * 10U + (ch[1] - '0');
        break;
    default:
        PRINTF("the parameter is wrong\r\n");
        select_index = 0xFFU;
        break;
    }

    return select_index;
}

static shell_status_t shell_bt(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    uint8_t *addr;
    uint8_t select_index = 0U;
    int     err          = 0;
    char conn_addr[BT_ADDR_LE_STR_LEN];

    if (argc < 1)
    {
        PRINTF("the parameter count is wrong\r\n");
    }

    if (strcmp(argv[1], "discover") == 0)
    {
        app_discover();
    }
    else if (strcmp(argv[1], "conns") == 0)
    {
        PRINTF("Connected device address:\r\n");
        for(select_index = 0U; select_index < CONFIG_BT_MAX_CONN; select_index++)
        {
            if(NULL != br_conns[select_index])
            {
                bt_addr_to_str(bt_conn_get_dst_br(br_conns[select_index]), conn_addr, sizeof(conn_addr));
                PRINTF("conn handle %d: %s\r\n", select_index, conn_addr);
            }
        }

        if(NULL != default_conn)
        {
            bt_addr_to_str(bt_conn_get_dst_br(default_conn), conn_addr, sizeof(conn_addr));
            PRINTF("Default conn address:%s\r\n", conn_addr);
        }
    }
    else if (strcmp(argv[1], "switch") == 0)
    {
        select_index = shell_parse_parameter(argc, argv);

        if (0xFFU == select_index)
        {
            PRINTF("the parameter is wrong\r\n");
            return kStatus_SHELL_Error;
        }
        else
        {
            default_conn = br_conns[select_index];
        }
    }
    else if (strcmp(argv[1], "connect") == 0)
    {
        select_index = shell_parse_parameter(argc, argv);

        if (0xFFU == select_index)
        {
            PRINTF("the parameter is wrong\r\n");
            return kStatus_SHELL_Error;
        }
        else
        {
            addr = app_get_addr(select_index - 1U);
            app_connect(addr);
        }
    }
    else if (strcmp(argv[1], "disconnect") == 0)
    {
        app_disconnect();
    }
    else if (strcmp(argv[1], "delete") == 0)
    {
        err = bt_unpair(BT_ID_DEFAULT, NULL);
        if (err != 0)
        {
            PRINTF("failed reason = %d\r\n", err);
        }
        else
        {
            PRINTF("success\r\n");
        }
    }
    else
    {
        PRINTF("Invalid bt command, please enter help to get the bt command list.\n");
    }

    return kStatus_SHELL_Success;
}

static int spp_sdp_discover_callback(struct bt_conn *conn, uint8_t count, uint16_t *channel)
{
    uint8_t index;

    PRINTF("Discover %d SPP server channel from device %02X:%02X:%02X:%02X:%02X:%02X!\n", count, BT_DEVICE_ADDR_POINTER(bt_conn_get_dst_br(conn)->val));

    for(index = 0U; index < count; index++)
    {
        PRINTF("0x%04x\n", channel[index]);
    }

    return 0;
}

static shell_status_t shell_spp(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    uint8_t select_index = 0U;

    if (argc < 1)
    {
        PRINTF("the parameter count is wrong\r\n");
        return kStatus_SHELL_Error;
    }

    if (strcmp(argv[1], "handle") == 0)
    {
        spp_appl_handle_info();
    }
    else if (strcmp(argv[1], "switch") == 0)
    {
        select_index = shell_parse_parameter(argc, argv);

        spp_appl_handle_select(select_index);
    }
    else if (strcmp(argv[1], "register") == 0)
    {
        select_index = shell_parse_parameter(argc, argv);

        if (0xFFU == select_index)
        {
            PRINTF("the parameter is wrong\r\n");
            return kStatus_SHELL_Error;
        }

        spp_appl_server_register(select_index);
    }
    else if (strcmp(argv[1], "discover") == 0)
    {
        (void)bt_spp_discover(default_conn, &discover_callback);
    }
    else if (strcmp(argv[1], "connect") == 0)
    {
        select_index = shell_parse_parameter(argc, argv);

        if (0xFFU == select_index)
        {
            PRINTF("the parameter is wrong\r\n");
            return kStatus_SHELL_Error;
        }

        spp_appl_connect(default_conn, select_index);
    }
    else if (strcmp(argv[1], "disconnect") == 0)
    {
        spp_appl_disconnect();
    }
    else if (strcmp(argv[1], "send") == 0)
    {
        select_index = shell_parse_parameter(argc, argv);

        spp_appl_send(select_index);
    }
    else if (strcmp(argv[1], "get_port") == 0)
    {
        if (strcmp(argv[2], "s") == 0)
        {
            select_index = shell_parse_parameter(argc, argv);
            spp_appl_get_server_port(select_index);
        }
        else if (strcmp(argv[2], "c") == 0)
        {
            select_index = shell_parse_parameter(argc, argv);
            spp_appl_get_client_port(select_index);
        }
        else
        {
            PRINTF("Invalid spp get_port parameter, please enter help to get the usage of spp get_port.\n");
        }
    }
    else if (strcmp(argv[1], "set_port") == 0)
    {
        if (strcmp(argv[2], "s") == 0)
        {
            select_index = shell_parse_parameter(argc, argv);
            spp_appl_set_server_port(select_index);
        }
        else if (strcmp(argv[2], "c") == 0)
        {
            select_index = shell_parse_parameter(argc, argv);
            spp_appl_set_client_port(select_index);
        }
        else
        {
            PRINTF("Invalid spp set_port parameter, please enter help to get the usage of spp set_port.\n");
        }
    }
    else if (strcmp(argv[1], "set_pn") == 0)
    {
        if (strcmp(argv[2], "s") == 0)
        {
            select_index = shell_parse_parameter(argc, argv);
            spp_appl_set_server_pn(select_index);
        }
        else if (strcmp(argv[2], "c") == 0)
        {
            select_index = shell_parse_parameter(argc, argv);
            spp_appl_set_client_pn(select_index);
        }
        else
        {
            PRINTF("Invalid spp set_pn parameter, please enter help to get the usage of spp set_pn.\n");
        }
    }
    else if (strcmp(argv[1], "get_pn") == 0)
    {
        if (strcmp(argv[2], "s") == 0)
        {
            select_index = shell_parse_parameter(argc, argv);
            spp_appl_get_local_server_pn(select_index);
        }
        else if (strcmp(argv[2], "c") == 0)
        {
            select_index = shell_parse_parameter(argc, argv);
            spp_appl_get_local_client_pn(select_index);
        }
        else
        {
            PRINTF("Invalid spp get local parameter, please enter help to get the usage of spp get_pn.\n");
        }
    }
    else if (strcmp(argv[1], "send_rls") == 0)
    {
        spp_appl_send_rls();
    }
    else if (strcmp(argv[1], "send_msc") == 0)
    {
        spp_appl_send_msc();
    }
    else
    {
        PRINTF("Invalid spp command, please enter help to get the spp command list.\n");
    }

    return kStatus_SHELL_Success;
}

void app_shell_init(void)
{
    DbgConsole_Flush();
    /* Init SHELL */
    spp_shell_handle = &spp_shell_handle_buffer[0];
    SHELL_Init(spp_shell_handle, g_serialHandle, ">> ");

    /* Add new command to commands list */
    SHELL_RegisterCommand(spp_shell_handle, SHELL_COMMAND(bt));
    SHELL_RegisterCommand(spp_shell_handle, SHELL_COMMAND(spp));

    /* Init spp appl */
    spp_appl_init();
}
