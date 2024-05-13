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

#include "bluetooth/audio/audio.h"

#include "fsl_debug_console.h"

#include "fsl_shell.h"

#include "ccp_client.h"

#include "unicast_audio_server.h"

#include "call_terminal.h"

shell_handle_t s_shellHandle;
SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
extern serial_handle_t g_serialHandle;

static struct bt_conn * default_conn;

static shell_status_t advertising(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t passkey_confirm(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t passkey(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t unpair(shell_handle_t shellHandle, int32_t argc, char **argv);

SHELL_COMMAND_DEFINE(advertising,        "advertising <on>/<off>\r\n",      advertising,         1);
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

static uint8_t unicast_server_adv_data[] = {
	BT_UUID_16_ENCODE(BT_UUID_ASCS_VAL), /* ASCS UUID */
	BT_AUDIO_UNICAST_ANNOUNCEMENT_TARGETED, /* Target Announcement */
	BT_BYTES_LIST_LE16(AVAILABLE_SINK_CONTEXT),
	BT_BYTES_LIST_LE16(AVAILABLE_SOURCE_CONTEXT),
	0x00, /* Metadata length */
};

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_TBS_VAL)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_ASCS_VAL)),
	BT_DATA(BT_DATA_SVC_DATA16, unicast_server_adv_data, ARRAY_SIZE(unicast_server_adv_data)),
};

struct bt_le_ext_adv *adv;

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
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (err)
    {
        PRINTF("Failed to connect to %s (err %u)\n", addr, err);
    }
    else
    {
        default_conn = conn;
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
    default_conn = NULL;
    PRINTF("Disconnected (reason 0x%02x)\n", reason);
    (void)default_conn;
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

    ret = call_client_init(s_shellHandle);

    if (ret < 0)
    {
        PRINTF("call gateway application initialization failed(error %d)!", ret);
        return;
    }

    ret = unicast_audio_server_init();
    if (ret < 0)
    {
        PRINTF("Unicast server application initialization failed(error %d)!", ret);
        return;
    }

    /* Start Advertising */
    /* Create a non-connectable non-scannable advertising set */
    ret = bt_le_ext_adv_create(BT_LE_EXT_ADV_CONN_NAME, NULL, &adv);
    if (ret) {
        PRINTF("Failed to create advertising set (error %d)\n", ret);
        return;
    }

    ret = bt_le_ext_adv_set_data(adv, ad, ARRAY_SIZE(ad), NULL, 0);
    if (ret) {
        PRINTF("Failed to set advertising data (error %d)\n", ret);
        return;
    }

    ret = bt_le_ext_adv_start(adv, BT_LE_EXT_ADV_START_DEFAULT);
    if (ret) {
        PRINTF("Failed to start advertising set (error %d)\n", ret);
        return;
    }

    PRINTF("Advertising successfully started\n");

    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(advertising));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(passkey));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(passkey_confirm));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(unpair));
}

static shell_status_t advertising(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    char * action = NULL;
    int err;

    if (argc < 2)
    {
        return kStatus_SHELL_Error;
    }

    action = argv[1];

    if (0 == strcmp(action, "on"))
    {
        /* Start Advertising */
        err = bt_le_ext_adv_set_data(adv, ad, ARRAY_SIZE(ad), NULL, 0);
        if (err) {
            PRINTF("Failed to set advertising data (error %d)\n", err);
            return kStatus_SHELL_Error;
        }
        err = bt_le_ext_adv_start(adv, BT_LE_EXT_ADV_START_DEFAULT);
        if (err) {
            PRINTF("Failed to start advertising set (error %d)\n", err);
            return kStatus_SHELL_Error;
        }
        PRINTF("Advertising successfully started\n");
    }
    else if (0 == strcmp(action, "off"))
    {
        /* Stop Advertising */
        err = bt_le_ext_adv_stop(adv);
        if (err)
        {
            PRINTF("Advertising failed to stop (err %d)\n", err);
            return kStatus_SHELL_Error;
        }

        PRINTF("Advertising successfully stoped\n");
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

    err = bt_unpair(BT_ID_DEFAULT, NULL);

    if (err)
    {
        PRINTF("Fail to unpair all bonding devices (err %d)\n", err);
        return kStatus_SHELL_Error;
    }

    PRINTF("Unpair completed!\n");
    return kStatus_SHELL_Success;
}

void call_terminal_task(void *param)
{
    int ret;

    /* Init SHELL */
    s_shellHandle = &s_shellHandleBuffer[0];
    SHELL_Init(s_shellHandle, g_serialHandle, "call_terminal>> ");

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
        vTaskDelay(1000);
    }
}
