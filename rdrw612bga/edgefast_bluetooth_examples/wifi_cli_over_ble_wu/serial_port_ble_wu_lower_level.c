/*
 * Copyright 2019, 2021-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno/errno.h>
#include <toolchain.h>
#include <porting.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/wu.h>
#include <fsl_debug_console.h>

#include "fsl_component_button.h"
#include "board.h"

#define DEVICE_NAME    CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN    (sizeof(DEVICE_NAME) - 1)

#ifndef WIRELESS_UART_TASK_PRIORITY
#define WIRELESS_UART_TASK_PRIORITY 6
#endif

#ifndef WIRELESS_UART_TASK_STACK_SIZE
#define WIRELESS_UART_TASK_STACK_SIZE 2048
#endif

#undef PRINTF
#define PRINTF(...)

#define WIRELESS_UART_SENDING_BUFFER_LENGTH 64

typedef struct _wireless_uart_peer_state
{
    struct bt_conn *conn;
    uint8_t* wait4SendingBuffer;
    volatile uint32_t wait4SendingLength;
} wireless_uart_peer_state_t;

typedef struct _wireless_uart_state
{
    serial_manager_callback_t serialManagerTxCb;
    void * serialManagerTxCbParam;
    serial_manager_callback_t serialManagerRxCb;
    void * serialManagerRxCbParam;
    uint8_t* orignialBuffer;
    volatile uint32_t orignialBufferLength;
    wireless_uart_peer_state_t peerCentral[CONFIG_BT_MAX_CONN];
    wireless_uart_peer_state_t peerPeripheral[CONFIG_BT_MAX_CONN];
#if (defined(BUTTON_COUNT) && (BUTTON_COUNT > 0U))
    uint8_t buttonHandleBuffer[BUTTON_COUNT][BUTTON_HANDLE_SIZE];
    button_handle_t buttonHandle[BUTTON_COUNT];
#endif
    uint8_t peerCentralConnCount;
    uint8_t peerPeripheralConnCount;
} wireless_uart_state_t;

typedef struct _scan_dev_info_t
{
    const bt_addr_le_t *addr;       /* the address of the scanned device */
    uint8_t ad_type;                /* the ad type of the scanned device */
    uint8_t ad_len;                 /* the ad len of the scanned device */
    int8_t  rssi;                   /* the rssi of the scanned device */
} scan_dev_info_t;
/* TODO: insert other definitions and declarations here. */

static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);
static void wireless_uart_rx_callback(void *callbackParam,
                                               serial_manager_callback_message_t *message,
                                               serial_manager_status_t status);
static void wu_central_scan_callback(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
             struct net_buf_simple *ad);
#if CONFIG_BT_SMP
static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err);

static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey);
static void auth_cancel(struct bt_conn *conn);
#endif

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID128_SOME, WIRELESS_UART_SERIVCE_UUID),
};

static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static struct bt_conn_cb conn_callbacks = {
    .connected = connected,
    .disconnected = disconnected,
#if CONFIG_BT_SMP
    .security_changed = security_changed,
#endif
};

#if CONFIG_BT_SMP
static struct bt_conn_auth_cb auth_cb_display = {
/*    .passkey_display = auth_passkey_display,*/
    .passkey_display = NULL,
    .passkey_entry = NULL,
    .cancel = auth_cancel,
};
#endif

wireless_uart_state_t g_WirelessUartState;

#if (defined(BUTTON_COUNT) && (BUTTON_COUNT > 0U))
extern button_config_t g_buttonConfig[BUTTON_COUNT];
#endif

extern int wifi_is_ready(void);

static void connected(struct bt_conn *conn, uint8_t err)
{
    char addr[BT_ADDR_LE_STR_LEN];
    struct bt_conn_info info;

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (err)
    {
        PRINTF("Failed to connect %s (err %u)\n", addr, err);
    }
    else
    {
        uint32_t i;
        int index = -1;

        err = bt_conn_get_info(conn, &info);
        if (err) {
            PRINTF("Failed to get info");
            return;
        }

        switch (info.role) {
        case BT_HCI_ROLE_PERIPHERAL:
            for (i = 0;i < CONFIG_BT_MAX_CONN;i++)
            {
                if (conn == g_WirelessUartState.peerCentral[i].conn)
                {
                    PRINTF("the connection 0x08X is attached\r\n", conn);
                    return;
                }
                else if (NULL == g_WirelessUartState.peerCentral[i].conn)
                {
                    if (-1 == index)
                    {
                        OSA_SR_ALLOC();

                        index = (int)i;

                        OSA_ENTER_CRITICAL();
                        g_WirelessUartState.peerCentral[index].conn = conn;
                        g_WirelessUartState.peerCentralConnCount++;
                        g_WirelessUartState.peerCentral[index].wait4SendingBuffer = g_WirelessUartState.orignialBuffer;
                        g_WirelessUartState.peerCentral[index].wait4SendingLength = g_WirelessUartState.orignialBufferLength;
                        OSA_EXIT_CRITICAL();
                    }
                }
                else
                {
                }
            }
            break;
        case BT_HCI_ROLE_CENTRAL:
            for (i = 0;i < CONFIG_BT_MAX_CONN;i++)
            {
                OSA_SR_ALLOC();

                OSA_ENTER_CRITICAL();
                if (conn == g_WirelessUartState.peerPeripheral[i].conn)
                {
                    index = (int)i;
                    g_WirelessUartState.peerPeripheralConnCount++;
                    g_WirelessUartState.peerPeripheral[index].wait4SendingBuffer = g_WirelessUartState.orignialBuffer;
                    g_WirelessUartState.peerPeripheral[index].wait4SendingLength = g_WirelessUartState.orignialBufferLength;
                }
                else
                {
                }
                OSA_EXIT_CRITICAL();
            }
            break;
        }

        if (-1 == index)
        {
            PRINTF("All connection slots are used\r\n");
            return;
        }
        bt_gatt_wu_connected(conn);
        PRINTF("Connected to %s\n", addr);
#if CONFIG_BT_SMP
        if(BT_HCI_ROLE_PERIPHERAL == info.role)
        {
            if (bt_conn_set_security(conn, BT_SECURITY_L2))
            {
                PRINTF("Failed to set security\n");
            }
        }
#endif
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    char addr[BT_ADDR_LE_STR_LEN];
    uint32_t i;
    uint8_t found = 0;

    for (i = 0;i < CONFIG_BT_MAX_CONN;i++)
    {
        if (conn == g_WirelessUartState.peerCentral[i].conn)
        {
            g_WirelessUartState.peerCentral[i].conn = NULL;
            g_WirelessUartState.peerCentralConnCount--;
            found = 1;
            break;
        }
        else
        {
        }
    }
    for (i = 0;i < CONFIG_BT_MAX_CONN;i++)
    {
        if (conn == g_WirelessUartState.peerPeripheral[i].conn)
        {
            g_WirelessUartState.peerPeripheral[i].conn = NULL;
            g_WirelessUartState.peerPeripheralConnCount--;
            found = 1;
            bt_conn_unref(conn);
            break;
        }
        else
        {
        }
    }
    if (0 == found)
    {
        PRINTF("The connection 0x%08X is not found.\r\n", conn);
        return;
    }
    bt_gatt_wu_disconnected(conn);

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    PRINTF("Disconnected from %s (reason %u)\n", addr, reason);
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

static void auth_cancel(struct bt_conn *conn)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    PRINTF("Pairing cancelled: %s\n", addr);
}
#endif

static int wireless_uart_data_received(struct bt_conn *conn, uint8_t* buffer, ssize_t length)
{
    serial_manager_callback_message_t message;
    serial_manager_status_t status = kStatus_SerialManager_Success;

    if (NULL != g_WirelessUartState.serialManagerRxCb)
    {
        message.buffer = buffer;
        message.length = length;
        g_WirelessUartState.serialManagerRxCb(g_WirelessUartState.serialManagerRxCbParam, &message, status);
    }
    return 0;
}

static int wireless_uart_read(struct bt_conn *conn, bt_gatt_wu_read_response_t response, void *param)
{
    uint32_t index;
    serial_manager_callback_message_t message;
    uint8_t* buffer;
    ssize_t length;
    int ret = -1;
    uint8_t txDone = 1U;
    OSA_SR_ALLOC();

    for (index = 0U;index < CONFIG_BT_MAX_CONN;index++)
    {
        if (conn == g_WirelessUartState.peerCentral[index].conn)
        {
            if (g_WirelessUartState.peerCentral[index].wait4SendingLength > 0U)
            {
                OSA_ENTER_CRITICAL();
                buffer = &g_WirelessUartState.peerCentral[index].wait4SendingBuffer[0];
                length = g_WirelessUartState.peerCentral[index].wait4SendingLength;
                if ((NULL != response))
                {
                    ret = response(param, buffer, length);
                    if (ret >= 0)
                    {
                        g_WirelessUartState.peerCentral[index].wait4SendingLength -= ret;
                        g_WirelessUartState.peerCentral[index].wait4SendingBuffer += ret;
                    }
                }
                OSA_EXIT_CRITICAL();
            }
            break;
        }
    }
    for (index = 0U;index < CONFIG_BT_MAX_CONN;index++)
    {
        if (conn == g_WirelessUartState.peerPeripheral[index].conn)
        {
            if (g_WirelessUartState.peerPeripheral[index].wait4SendingLength > 0U)
            {
                OSA_ENTER_CRITICAL();
                buffer = &g_WirelessUartState.peerPeripheral[index].wait4SendingBuffer[0];
                length = g_WirelessUartState.peerPeripheral[index].wait4SendingLength;
                if ((NULL != response))
                {
                    ret = response(param, buffer, length);
                    if (ret >= 0)
                    {
                        g_WirelessUartState.peerPeripheral[index].wait4SendingLength -= ret;
                        g_WirelessUartState.peerPeripheral[index].wait4SendingBuffer += ret;
                    }
                }
                OSA_EXIT_CRITICAL();
            }
            break;
        }
    }

    for (index = 0U;index < CONFIG_BT_MAX_CONN;index++)
    {
        if (NULL != g_WirelessUartState.peerCentral[index].conn)
        {
            if (g_WirelessUartState.peerCentral[index].wait4SendingLength > 0U)
            {
                txDone = 0U;
            }
        }
        if (NULL != g_WirelessUartState.peerPeripheral[index].conn)
        {
            if (g_WirelessUartState.peerPeripheral[index].wait4SendingLength > 0U)
            {
                txDone = 0U;
            }
        }
    }

    if (txDone > 0U)
    {
        message.buffer = g_WirelessUartState.orignialBuffer;
        message.length = g_WirelessUartState.orignialBufferLength;
        g_WirelessUartState.orignialBuffer = NULL;
        g_WirelessUartState.orignialBufferLength = 0U;
        for (index = 0U;index < CONFIG_BT_MAX_CONN;index++)
        {
            g_WirelessUartState.peerCentral[index].wait4SendingBuffer = NULL;
            g_WirelessUartState.peerCentral[index].wait4SendingLength = 0U;
        }
        for (index = 0U;index < CONFIG_BT_MAX_CONN;index++)
        {
            g_WirelessUartState.peerPeripheral[index].wait4SendingBuffer = NULL;
            g_WirelessUartState.peerPeripheral[index].wait4SendingLength = 0U;
        }
        if (NULL != g_WirelessUartState.serialManagerTxCb)
        {
            serial_manager_status_t status = kStatus_SerialManager_Success;
            g_WirelessUartState.serialManagerTxCb(g_WirelessUartState.serialManagerTxCbParam, &message, status);
        }
    }
    return ret;
}

static bool wu_central_parse_callback(struct bt_data *data, void *user_data)
{
    scan_dev_info_t *deviceInfo = (scan_dev_info_t *)user_data;
    char             dev[BT_ADDR_LE_STR_LEN];
    int              i;
    int              error = -1;
    uint8_t          index;

#if 0
    PRINTF("[AD]: %u data_len %u\n", data->type, data->data_len);
#endif
    switch (data->type)
    {
    case BT_DATA_UUID128_SOME:
    case BT_DATA_UUID128_ALL:
        if (data->data_len % (sizeof(struct bt_uuid_128) - sizeof(struct bt_uuid)) != 0) {
            PRINTF("AD malformed\n");
            return true;
        }

        for (i = 0; i < data->data_len; i += (sizeof(struct bt_uuid_128) - sizeof(struct bt_uuid))) {
            struct bt_uuid_128 uuid;

            uuid.uuid.type = BT_UUID_TYPE_128;
            memcpy(&uuid.val[0], &data->data[i], sizeof(struct bt_uuid_128) - sizeof(struct bt_uuid));
            if (bt_uuid_cmp((struct bt_uuid *)&uuid, WIRELESS_UART_SERIVCE)) {
                continue;
            }

            bt_addr_le_to_str(deviceInfo->addr, dev, sizeof(dev));
            PRINTF("[DEVICE]: %s, AD evt type %u, AD data len %u, RSSI %i\n", dev, deviceInfo->ad_type, deviceInfo->ad_len, deviceInfo->rssi);


            for (index = 0;index < CONFIG_BT_MAX_CONN;index++)
            {
                if (NULL != g_WirelessUartState.peerPeripheral[index].conn)
                {
                    struct bt_conn_info info;

                    int err = bt_conn_get_info(g_WirelessUartState.peerPeripheral[index].conn, &info);
                    if (0 == err)
                    {
                        if (0 == bt_addr_le_cmp(info.le.remote, deviceInfo->addr))
                        {
                            return true;
                        }
                    }
                    break;
                }
                else
                {
                }
            }

            bt_le_scan_stop();
            if((g_WirelessUartState.peerPeripheralConnCount + g_WirelessUartState.peerCentralConnCount) <= CONFIG_BT_MAX_CONN)
            {
                for (index = 0;index < CONFIG_BT_MAX_CONN;index++)
                {
                    if (NULL == g_WirelessUartState.peerPeripheral[index].conn)
                    {
                        struct bt_conn_le_create_param param = BT_CONN_LE_CREATE_PARAM_INIT(
                            BT_CONN_LE_OPT_NONE,
                            BT_GAP_SCAN_FAST_INTERVAL,
                            BT_GAP_SCAN_FAST_INTERVAL);
                        error = bt_conn_le_create(deviceInfo->addr, &param, BT_LE_CONN_PARAM_DEFAULT, &g_WirelessUartState.peerPeripheral[index].conn);
                        if (0 == error)
                        {
                            break;
                        }
                    }
                    else
                    {
                    }
                }

            }


            if (0 == error)
            {
                return false;
            }
            else
            {
                return true;
            }
        }
    }

    return true;
}

static void wu_central_scan_callback(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
             struct net_buf_simple *ad)
{
    scan_dev_info_t deviceInfo;

    assert(NULL != addr);
    deviceInfo.addr    = addr;
    deviceInfo.ad_type = type;
    deviceInfo.ad_len  = ad->len;
    deviceInfo.rssi    = rssi;

#if 0
    char dev[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(addr, dev, sizeof(dev));
    PRINTF("[DEVICE]: %s, AD evt type %u, AD data len %u, RSSI %i\n",
           deviceInfo.dev, deviceInfo.ad_type, deviceInfo.ad_len, deviceInfo.rssi);
#endif
    /* We're only interested in connectable events */
    if (type == BT_HCI_ADV_IND || type == BT_HCI_ADV_DIRECT_IND)
    {
        bt_data_parse(ad, wu_central_parse_callback, (void *)&deviceInfo);
    }
}

#if (defined(BUTTON_COUNT) && (BUTTON_COUNT > 0U))
button_status_t wireless_uart_button_callback(void *buttonHandle, button_callback_message_t *message, void *callbackParam)
{
    button_status_t status = kStatus_BUTTON_Success;
#if (BUTTON_COUNT > 1U)
    int i = (int)callbackParam;

    if (0 == i)
    {
        bt_le_scan_stop();
        bt_le_adv_stop();

        if ((g_WirelessUartState.peerPeripheralConnCount + g_WirelessUartState.peerCentralConnCount) <= CONFIG_BT_MAX_CONN)
        {
            int error = bt_le_scan_start(BT_LE_SCAN_ACTIVE, wu_central_scan_callback);
            if (error)
            {
                PRINTF("Scanning failed to start (err %d)\n", error);
            }
            else
            {
                PRINTF("Scanning successfully started\n");
            }
        }
    }
    else
    {
        bt_le_scan_stop();
        bt_le_adv_stop();

        if ((g_WirelessUartState.peerPeripheralConnCount + g_WirelessUartState.peerCentralConnCount) <= CONFIG_BT_MAX_CONN)
        {
            int error = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
            if (error)
            {
                PRINTF("Advertising failed to start (error %d)\n", error);
            }
            else
            {
                PRINTF("Advertising successfully started\n");
            }
        }
    }
#else
    switch (message->event)
    {
        case kBUTTON_EventOneClick:
        case kBUTTON_EventShortPress:
            bt_le_scan_stop();
            bt_le_adv_stop();

            if ((g_WirelessUartState.peerPeripheralConnCount + g_WirelessUartState.peerCentralConnCount) <= CONFIG_BT_MAX_CONN)
            {
                int error = bt_le_scan_start(BT_LE_SCAN_ACTIVE, wu_central_scan_callback);
                if (error)
                {
                    PRINTF("Scanning failed to start (err %d)\n", error);
                }
                else
                {
                    PRINTF("Scanning successfully started\n");
                }
            }
            else
            {


            }
            break;
        case kBUTTON_EventDoubleClick:
        case kBUTTON_EventLongPress:
            bt_le_scan_stop();
            bt_le_adv_stop();

            if ((g_WirelessUartState.peerPeripheralConnCount + g_WirelessUartState.peerCentralConnCount) <= CONFIG_BT_MAX_CONN)
            {
                int error = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
                if (error)
                {
                    PRINTF("Advertising failed to start (error %d)\n", error);
                }
                else
                {
                    PRINTF("Advertising successfully started\n");
                }
            }
            break;
        case kBUTTON_EventError:
            PRINTF("kBUTTON_EventError\r\n");
            break;
        default:
            status = kStatus_BUTTON_Error;
            break;
    }
#endif
    return status;
}
#endif

static void bt_ready(int error)
{
    bt_gatt_wu_config_t wuConfig;
    if (error)
    {
        PRINTF("Bluetooth init failed (error %d)\n", error);
        return;
    }

    if (IS_ENABLED(CONFIG_BT_SETTINGS))
    {
        settings_load();
    }
    PRINTF("Bluetooth initialized\n");

    wuConfig.data_received = wireless_uart_data_received;
    wuConfig.read = wireless_uart_read;

    bt_gatt_wu_init("Wireless Uart Demo", "WU1234567890", &wuConfig);

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		settings_load();
	}

    bt_conn_cb_register(&conn_callbacks);
#if CONFIG_BT_SMP
    bt_conn_auth_cb_register(&auth_cb_display);
#endif

#if (defined(BUTTON_COUNT) && (BUTTON_COUNT > 0U))
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        g_WirelessUartState.buttonHandle[i] = (button_handle_t)&g_WirelessUartState.buttonHandleBuffer[i][0];
        BUTTON_Init(g_WirelessUartState.buttonHandle[i], &g_buttonConfig[i]);
        BUTTON_InstallCallback(g_WirelessUartState.buttonHandle[i], wireless_uart_button_callback, (void*)i);
    }
#endif
#if (BUTTON_COUNT == 1)
    error = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (error)
    {
        PRINTF("Advertising failed to start (error %d)\n", error);
        return;
    }
    PRINTF("Advertising successfully started\n");
#endif
}

/*
 * @brief   Application entry point.
 */
void wireless_uart_task(void *argument)
{
    uint32_t index;
    int err;

    for (index = 0;index < CONFIG_BT_MAX_CONN;index++)
    {
        g_WirelessUartState.peerCentral[index].wait4SendingBuffer = NULL;
        g_WirelessUartState.peerCentral[index].wait4SendingLength = 0;
    }
    for (index = 0;index < CONFIG_BT_MAX_CONN;index++)
    {
        g_WirelessUartState.peerPeripheral[index].wait4SendingBuffer = NULL;
        g_WirelessUartState.peerPeripheral[index].wait4SendingLength = 0;
    }

    g_WirelessUartState.orignialBuffer = NULL;
    g_WirelessUartState.orignialBufferLength = 0;

    while (true)
    {
        err = wifi_is_ready();
        if (0 == err)
        {
            break;
        }
        else
        {
            vTaskDelay(10);
        }
    }

    err = bt_enable(bt_ready);
    if (err)
    {
        PRINTF("Bluetooth init failed (err %d)\n", err);
        while (1)
        {
            vTaskDelay(2000);
        }
    }

    while (USE_RTOS)
    {
        vTaskDelay(10);

        /* Wireless Uart notification thread */
        bt_gatt_wu_notify(NULL);
    }
}

#if (!defined(configUSE_IDLE_HOOK) || (configUSE_IDLE_HOOK == 0))
extern void vApplicationIdleHook(void);
/*
 * @brief   Application entry point.
 */
void getchar_task(void *argument)
{
    while (USE_RTOS)
    {
        vApplicationIdleHook();
    }
}
#endif /* configUSE_IDLE_HOOK */

status_t BLE_WuInit(void);
status_t BLE_WuInit(void)
{
    status_t ret = kStatus_Success;
    if (xTaskCreate(wireless_uart_task, "wu_task", configMINIMAL_STACK_SIZE * 8, NULL, tskIDLE_PRIORITY + 2, NULL) != pdPASS)
    {
        ret = kStatus_Fail;
    }

#if (!defined(configUSE_IDLE_HOOK) || (configUSE_IDLE_HOOK == 0))
    if (kStatus_Success == ret)
    {
        if (xTaskCreate(getchar_task, "getchar_task", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
        {
            ret = kStatus_Fail;
        }
    }
#endif /* configUSE_IDLE_HOOK */

    return ret;
}

status_t BLE_WuDeinit(void);
status_t BLE_WuDeinit(void)
{
    return kStatus_Success;
}

status_t BLE_WuWrite(uint8_t *buffer, uint32_t length);
status_t BLE_WuWrite(uint8_t *buffer, uint32_t length)
{
    OSA_SR_ALLOC();

    OSA_ENTER_CRITICAL();
    for (int index = 0;index < CONFIG_BT_MAX_CONN;index++)
    {
        if (NULL != g_WirelessUartState.peerCentral[index].conn)
        {
            g_WirelessUartState.peerCentral[index].wait4SendingBuffer = buffer;
            g_WirelessUartState.peerCentral[index].wait4SendingLength = length;
        }
        if (NULL != g_WirelessUartState.peerPeripheral[index].conn)
        {
            g_WirelessUartState.peerPeripheral[index].wait4SendingBuffer = buffer;
            g_WirelessUartState.peerPeripheral[index].wait4SendingLength = length;
        }
    }
    g_WirelessUartState.orignialBuffer = buffer;
    g_WirelessUartState.orignialBufferLength = length;
    OSA_EXIT_CRITICAL();
    return kStatus_Success;
}

status_t BLE_WuRead(uint8_t *buffer, uint32_t length);
status_t BLE_WuRead(uint8_t *buffer, uint32_t length)
{
    return kStatus_Fail;
}

status_t BLE_WuInstallTxCallback(serial_manager_callback_t callback,
                                                     void *callbackParam);
status_t BLE_WuInstallTxCallback(serial_manager_callback_t callback,
                                                     void *callbackParam)
{
    g_WirelessUartState.serialManagerTxCb = callback;
    g_WirelessUartState.serialManagerTxCbParam = callbackParam;
    return kStatus_Success;
}

status_t BLE_WuInstallRxCallback(serial_manager_callback_t callback,
                                                     void *callbackParam);
status_t BLE_WuInstallRxCallback(serial_manager_callback_t callback,
                                                     void *callbackParam)
{
    g_WirelessUartState.serialManagerRxCb = callback;
    g_WirelessUartState.serialManagerRxCbParam = callbackParam;
    return kStatus_Success;
}
