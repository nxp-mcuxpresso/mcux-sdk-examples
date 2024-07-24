/*
 * Copyright (c) 2021-2022 Nordic Semiconductor ASA
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stdio.h>
#include <stddef.h>
#include <errno/errno.h>
#include <toolchain.h>
#include <porting.h>

#include <bluetooth/audio/tmap.h>

#include "fsl_debug_console.h"
#include "fsl_os_abstraction.h"

#include "fsl_shell.h"

#include "ccp_server.h"

#include "unicast_audio_client.h"

#include "set_coordinator.h"

#include "call_gateway.h"

shell_handle_t s_shellHandle;
SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
extern serial_handle_t g_serialHandle;

struct bt_conn * default_conn;

static volatile uint32_t conn_count;

enum event_bitmap
{
    KAPP_EventUnicastDiscoverDone = BIT(0),
};

static OSA_EVENT_HANDLE_DEFINE(eventHandle);

static shell_status_t scanning(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t passkey_confirm(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t passkey(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t unpair(shell_handle_t shellHandle, int32_t argc, char **argv);

SHELL_COMMAND_DEFINE(scanning,           "scanning <on>/<off>\r\n",         scanning,            1);
SHELL_COMMAND_DEFINE(passkey,            "passkey <6 digital number>\r\n",  passkey,             1);
SHELL_COMMAND_DEFINE(passkey_confirm,    "passkey_confirm <yes>/<no>\r\n",  passkey_confirm,     1);
SHELL_COMMAND_DEFINE(unpair,             "unpair\r\n",                      unpair,              0);

static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);
static void bt_ready(int err);

#if CONFIG_BT_SMP
static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err);
static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey);
static void auth_passkey_entry(struct bt_conn *conn);
static void auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey);
static void auth_cancel(struct bt_conn *conn);
#endif

static int scan_start(void);

static struct bt_conn_cb conn_callbacks = {
    .connected = connected,
    .disconnected = disconnected,
#if CONFIG_BT_SMP
    .security_changed = security_changed,
#endif
};

#if CONFIG_BT_SMP
static struct bt_conn_auth_cb auth_cb_display = {
    .passkey_display = auth_passkey_display,
    .passkey_entry = auth_passkey_entry,
    .passkey_confirm = auth_passkey_confirm,
    .cancel = auth_cancel,
};
#endif

static bool device_scanned(struct bt_data *data, void *user_data)
{
    bt_addr_le_t *addr = user_data;
    struct bt_uuid *uuid;
    uint16_t u16;
    int err;
    int i;
    char dev[BT_ADDR_LE_STR_LEN];
    bool continueParse = true;

    /* return true to continue parsing or false to stop parsing */
    switch (data->type)
    {
        case BT_DATA_NAME_SHORTENED:
        case BT_DATA_NAME_COMPLETE:
        {
            PRINTF("Device name: ");
            for (int i = 0; i < data->data_len; i++) {
                PUTCHAR(data->data[i]);
            }
            PRINTF("\n");
            break;
        }
        case BT_DATA_UUID16_SOME:
        case BT_DATA_UUID16_ALL:
        {
            if (data->data_len % sizeof(uint16_t) != 0U)
            {
                PRINTF("AD malformed\n");
                return true;
            }

            for (i = 0; i < data->data_len; i += sizeof(uint16_t))
            {
                memcpy(&u16, &data->data[i], sizeof(u16));
                uuid = BT_UUID_DECLARE_16(sys_le16_to_cpu(u16));

                /* search for the HTS UUID in the advertising data */
                if (bt_uuid_cmp(uuid, BT_UUID_TBS) == 0)
                {
                    /* found the temperature server - stop scanning */
                    err = bt_le_scan_stop();
                    if (err)
                    {
                        PRINTF("Stop LE scan failed (err %d)\n", err);
                        break;
                    }
                    bt_addr_le_to_str(addr, dev, sizeof(dev));
                    PRINTF("Found device: %s\n", dev);

                    /* Send connection request */
                    err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN,
                                            BT_LE_CONN_PARAM_DEFAULT,
                                            &default_conn);
                    if (err)
                    {
                        PRINTF("Create connection failed (err %d)\n", err);
                        scan_start();
                    }
                    else
                    {
                        bt_conn_unref(default_conn);
                    }

                    continueParse = false;
                    break;
                }
            }
            break;
        }
        case BT_DATA_SVC_DATA16:
        {
            if (data->data_len < 2)
            {
                PRINTF("AD malformed\n");
                return true;
            }
            memcpy(&u16, &data->data[0], sizeof(u16));
            uuid = BT_UUID_DECLARE_16(sys_le16_to_cpu(u16));

            /* search for the TMAP UUID in the advertising data */
            if (bt_uuid_cmp(uuid, BT_UUID_GATT_TMAPR) == 0)
            {
                if (data->data_len < (2 + 2))
                {
                    PRINTF("AD malformed\n");
                    return true;
                }
                memcpy(&u16, &data->data[2], sizeof(u16));
                if ((u16 & BT_TMAP_ROLE_CT) == 0)
                {
                    break;
                }
                if (addr->type != BT_ADDR_LE_PUBLIC)
                {
                    break;
                }
                /* found the temperature server - stop scanning */
                err = bt_le_scan_stop();
                if (err)
                {
                    PRINTF("Stop LE scan failed (err %d)\n", err);
                    break;
                }
                bt_addr_le_to_str(addr, dev, sizeof(dev));
                PRINTF("Found device: %s", dev);

                /* Send connection request */
                err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN,
                                        BT_LE_CONN_PARAM_DEFAULT,
                                        &default_conn);
                if (err)
                {
                    PRINTF("Create connection failed (err %d)\n", err);
                    scan_start();
                }
                else
                {
                    bt_conn_unref(default_conn);
                }

                continueParse = false;
                break;
            }
            break;
        }
        default:
        {
            break;
        }
    }

    return continueParse;
}

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
             struct net_buf_simple *ad)
{
    char dev[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(addr, dev, sizeof(dev));
	PRINTF("[DEVICE]: %s, AD evt type %u, AD data len %u, RSSI %i\n",
	       dev, type, ad->len, rssi);

    if (rssi < -60)
    {
        return;
    }
	/* We're only interested in connectable events */
	if (type == BT_GAP_ADV_TYPE_ADV_IND ||
	    type == BT_GAP_ADV_TYPE_ADV_DIRECT_IND ||
        type == BT_GAP_ADV_TYPE_EXT_ADV)
    {
		bt_data_parse(ad, device_scanned, (void *)addr);
	}
}

static int scan_start(void)
{
    return bt_le_scan_start(BT_LE_SCAN_CODED_ACTIVE, device_found);
}

#if CONFIG_BT_SMP
static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    PRINTF("Security changed: %s level %u (error %d)\n", addr, level, err);
}

static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    PRINTF("Passkey for %s: %06u\n", addr, passkey);
}

static void auth_passkey_entry(struct bt_conn *conn)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    PRINTF("Enter the passkey for %s\n", addr);
}

static void auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    PRINTF("Passkey for %s: %06u\n", addr, passkey);
}

static void auth_cancel(struct bt_conn *conn)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    PRINTF("Pairing cancelled: %s\n", addr);
}
#endif

static void connected(struct bt_conn *conn, uint8_t err)
{
    char addr[BT_ADDR_LE_STR_LEN];
    uint32_t reg;

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (err)
    {
        PRINTF("Failed to connect to %s (err %u)\n", addr, err);
    }
    else
    {
        reg = DisableGlobalIRQ();
        conn_count++;
        default_conn = conn;
        EnableGlobalIRQ(reg);
        PRINTF("Connected to peer: %s\n", addr);
#if CONFIG_BT_SMP
        if (bt_conn_set_security(conn, BT_SECURITY_L2))
        {
            PRINTF("Failed to set security\n");
        }
#endif
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    uint32_t reg;

    reg = DisableGlobalIRQ();
    if (default_conn == conn)
    {
        default_conn = NULL;
    }
    conn_count--;
    EnableGlobalIRQ(reg);
    PRINTF("Disconnected (reason 0x%02x)\n", reason);
}

static void unicast_client_discover_done_callback(struct bt_conn *conn, int err)
{
    if (err == 0)
    {
        OSA_EventSet((osa_event_handle_t)eventHandle, KAPP_EventUnicastDiscoverDone);
    }
}

static void bt_ready(int err)
{
    int ret;
    if (err)
    {
        PRINTF("Bluetooth init failed (err %d)\n", err);
        return;
    }

#if (defined(CONFIG_BT_SETTINGS) && (CONFIG_BT_SETTINGS > 0))
    settings_load();
#endif /* CONFIG_BT_SETTINGS */

    PRINTF("Bluetooth initialized\n");

    bt_conn_cb_register(&conn_callbacks);

#if CONFIG_BT_SMP
    bt_conn_auth_cb_register(&auth_cb_display);
#endif

    ret = call_server_init(s_shellHandle);

    if (ret < 0)
    {
        PRINTF("call terminal application initialization failed(error %d)!", ret);
        return;
    }

    ret = unicast_audio_client_init(unicast_client_discover_done_callback);
    if (ret < 0)
    {
        PRINTF("Unicast client initialization failed(error %d)!", ret);
        return;
    }

    ret = set_coordinator_init();
    if (ret < 0)
    {
        PRINTF("Set coordinator initialization failed(error %d)!", ret);
        return;
    }

    /* Start scanning */
    ret = scan_start();
    if (ret)
    {
        PRINTF("Scanning failed to start (error %d)\n", ret);
        return;
    }

    PRINTF("Scanning started\n");

    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(scanning));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(passkey));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(passkey_confirm));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(unpair));

#if 0
    extern int bt_csip_sef(const uint8_t k[16],
		const uint8_t sirk[16],
		uint8_t out_sirk[16]);
    uint8_t k[16] = {0x67, 0x6e, 0x1b, 0x9b, 0xd4, 0x48, 0x69, 0x6f, 0x06, 0x1e, 0xc6, 0x22, 0x3c, 0xe5, 0xce, 0xd9};
    uint8_t sirk[16] = {0x45, 0x7d, 0x7d, 0x09, 0x21, 0xa1, 0xfd, 0x22, 0xce, 0xcd, 0x8c, 0x86, 0xdd, 0x72, 0xcc, 0xcd};
    uint8_t out_sirk[16];

    bt_csip_sef(k, sirk, out_sirk);
    PRINTF("%d\n", out_sirk[0]);
    bt_csip_sef(k, out_sirk, sirk);
    PRINTF("%d\n", sirk[0]);

    uint8_t r[3] = {0x69, 0xf5, 0x63};
    uint8_t out[3];
    extern int bt_csip_sih(const uint8_t sirk[16], uint8_t r[3],
    uint8_t out[3]);

    uint8_t revert_sirk[16];
    sys_memcpy_swap(revert_sirk, sirk, 16);

    uint8_t revert_r[3];
    sys_memcpy_swap(revert_r, r, 3);
    bt_csip_sih(revert_sirk, revert_r, out);
    PRINTF("%d\n", out[0]);
#endif
}

static shell_status_t scanning(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    char * action = NULL;
    int err;

    if (argc < 2)
    {
        return kStatus_SHELL_Error;
    }

    if (conn_count > 0)
    {
        PRINTF("Cannot start a new scanning\n");
        return kStatus_SHELL_Error;
    }

    action = argv[1];

    if (0 == strcmp(action, "on"))
    {
        /* Start scanning */
        err = scan_start();
        if (err)
        {
            PRINTF("Scanning failed to start (err %d)\n", err);
            return kStatus_SHELL_Error;
        }

        PRINTF("Scanning started\n");
    }
    else if (0 == strcmp(action, "off"))
    {
        /* Stop Advertising */
        err = bt_le_scan_stop();
        if (err)
        {
            PRINTF("scanning failed to stop (err %d)\n", err);
            return kStatus_SHELL_Error;
        }

        PRINTF("scanning successfully stoped\n");
    }
    else
    {
        PRINTF("Invalid commander parameter\n");
        return kStatus_SHELL_Error;
    }
    return kStatus_SHELL_Success;
}

static shell_status_t passkey(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    unsigned int value = 0;
    int err;

    if (argc < 2)
    {
        return kStatus_SHELL_Error;
    }

    if (NULL == default_conn)
    {
        PRINTF("Not valid connection!\n");
        return kStatus_SHELL_Error;
    }

    for (int i = 0;i < strlen(argv[1]);i++)
    {
        if ('-' == argv[1][i])
        {
            PRINTF(s_shellHandle, "invalid parameter\n");
            return kStatus_SHELL_Error;
        }
        if (('0' > argv[1][i]) || ('9' < argv[1][i]))
        {
            PRINTF(s_shellHandle, "invalid parameter\n");
            return kStatus_SHELL_Error;
        }
    }

    value = (unsigned int)atoi(argv[1]);

    err = bt_conn_auth_passkey_entry(default_conn, value);
    if (err)
    {
        PRINTF("Fail to enter passkey (err %d)\n", err);
        return kStatus_SHELL_Error;
    }

    PRINTF("Passkey inputted!\n");

    return kStatus_SHELL_Success;
}

static shell_status_t passkey_confirm(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    char * action = NULL;
    int err;

    if (argc < 2)
    {
        return kStatus_SHELL_Error;
    }

    if (NULL == default_conn)
    {
        PRINTF("Not valid connection!\n");
        return kStatus_SHELL_Error;
    }

    action = argv[1];

    if (0 == strcmp(action, "yes"))
    {
        /* Start scanning */
        err = bt_conn_auth_passkey_confirm(default_conn);
        if (err)
        {
            PRINTF("Fail to confirm passkey (err %d)\n", err);
            return kStatus_SHELL_Error;
        }

        PRINTF("Passkey confirmed!\n");
    }
    else if (0 == strcmp(action, "no"))
    {
        /* Stop Advertising */
        err = bt_conn_auth_cancel(default_conn);
        if (err)
        {
            PRINTF("Fail to cancel auth (err %d)\n", err);
            return kStatus_SHELL_Error;
        }

        PRINTF("Auth canceled!\n");
    }
    else
    {
        PRINTF("Invalid commander parameter\n");
        return kStatus_SHELL_Error;
    }
    return kStatus_SHELL_Success;
}

static shell_status_t unpair(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int err;

    err = set_coordinator_clear();
    if (err)
    {
        PRINTF("Fail to clear set coordinator (err %d)\n", err);
        return kStatus_SHELL_Error;
    }

    err = bt_unpair(BT_ID_DEFAULT, NULL);

    if (err)
    {
        PRINTF("Fail to unpair all bonding devices (err %d)\n", err);
        return kStatus_SHELL_Error;
    }

    PRINTF("Unpair completed!\n");
    return kStatus_SHELL_Success;
}

void call_gateway_task(void *param)
{
    int ret;
    osa_status_t osa_ret;
    osa_event_flags_t flags;

    osa_ret = OSA_EventCreate((osa_event_handle_t)eventHandle, 1U);
    if (KOSA_StatusSuccess != osa_ret)
    {
        PRINTF("Event create failed\n");
        while (true)
        {
            vTaskDelay(2000);
        }
    }

    /* Init SHELL */
    s_shellHandle = &s_shellHandleBuffer[0];
    SHELL_Init(s_shellHandle, g_serialHandle, "call_gateway>> ");

    ret = bt_enable(bt_ready);
    if (ret)
    {
        PRINTF("Bluetooth init failed (error %d)\n", ret);
        while (true)
        {
            vTaskDelay(2000);
        }
    }

    while (true)
    {
        osa_ret = OSA_EventWait(eventHandle, (osa_event_flags_t)0xFFFFFFFFUL, 0U, osaWaitForever_c, &flags);
        if (KOSA_StatusSuccess == osa_ret)
        {
            switch (flags)
            {
            case KAPP_EventUnicastDiscoverDone:
                call_server_enable_command();
                break;

            default:
                break;
            }
        }
    }
}
