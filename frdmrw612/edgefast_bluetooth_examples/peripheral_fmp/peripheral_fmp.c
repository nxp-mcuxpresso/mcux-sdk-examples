/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <toolchain.h>
#include <porting.h>
#include "fsl_debug_console.h"

#include <bluetooth/services/fmp.h>
#include <bluetooth/gatt.h>
#include <bluetooth/conn.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BT_LE_ADV_PARAMS BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE | \
					                     BT_LE_ADV_OPT_USE_NAME, \
                                         BT_GAP_ADV_FAST_INT_MIN_1, \
					                     BT_GAP_ADV_FAST_INT_MAX_1, NULL)


#define BT_LE_ADV_LP_PARAMS BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE | \
					                        BT_LE_ADV_OPT_USE_NAME, \
                                            BT_GAP_ADV_SLOW_INT_MIN, \
					                        BT_GAP_ADV_SLOW_INT_MAX, NULL)

#define ADV_PERIOD_MS       30000

/*******************************************************************************
 * Variables
 ******************************************************************************/
struct bt_conn *default_conn;
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL,
                  BT_UUID_16_ENCODE(BT_UUID_IAS_VAL))
};
static TimerHandle_t adv_timer = NULL;
static QueueHandle_t adv_timeout_sync;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void alert_ui(uint8_t alert_level);
static void advertising_timeout_handler(TimerHandle_t timer_id);
static void bt_app_start_advertising(void);
static void advertising_timeout_process(void);
/*******************************************************************************
 * APIs
 ******************************************************************************/
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
        if (adv_timer != NULL)
        {
            xTimerStop(adv_timer, 0);
            xTimerDelete(adv_timer, 0);
            adv_timer = NULL;
        }
        
        uint8_t alert_level = NO_ALERT;

        default_conn = bt_conn_ref(conn);
        PRINTF("Connected to peer: %s\n", addr);

        alert_ui(alert_level);
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
    PRINTF("Disconnected (reason 0x%02x)\n", reason);

    if (default_conn)
    {
        bt_conn_unref(default_conn);
        default_conn = NULL;
    }

    bt_app_start_advertising();
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
    .passkey_entry = NULL,
    .cancel = auth_cancel,
};
#endif

static void alert_ui(uint8_t alert_level)
{
    switch(alert_level)
    {
        case NO_ALERT:
            PRINTF("\r\nALERT: OFF\r\n");
            break;

        case MILD_ALERT:
            PRINTF("\r\nALERT: MILD\r\n");
            break;

        case HIGH_ALERT:
            PRINTF("\r\nALERT: HIGH\r\n");
            break;

        default:
            break;
    }
}


static void bt_app_start_advertising(void)
{
    int err = bt_le_adv_start(BT_LE_ADV_PARAMS, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err)
    {
        PRINTF("Fast Advertising failed to start (err %d)\n", err);
        return;
    }

    PRINTF("Fast Advertising successfully started\n");

    if (adv_timer == NULL)
    {
        adv_timer = xTimerCreate("advertising timer", (ADV_PERIOD_MS / portTICK_PERIOD_MS),
                                  pdFALSE, NULL, advertising_timeout_handler);
    }
    xTimerStart(adv_timer, 0);
}

static void advertising_timeout_handler(TimerHandle_t timer_id)
{
    if (NULL != adv_timeout_sync)
    {
        /* it is not in isr context */
        xSemaphoreGive(adv_timeout_sync);
    }
    else
    {
        advertising_timeout_process();
    }
}

static void advertising_timeout_process(void)
{
    if (adv_timer != NULL)
    {
        /* Stop advertising timer */
       xTimerStop(adv_timer, 0);
       xTimerDelete(adv_timer, 0);
       adv_timer = NULL;
    }

    /* Stop fast advertising */
    int err = bt_le_adv_stop();
    if (err)
    {
        PRINTF("Advertising failed to stop (err %d)\n", err);
    }

    /* Enable slow advertising to preserve power */
    err = bt_le_adv_start(BT_LE_ADV_LP_PARAMS, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err)
    {
        PRINTF("Slow Advertising failed to start (err %d)\n", err);
        return;
    }

    PRINTF("Slow Advertising successfully started\n");
}


static void bt_ready(int err)
{
    if (err)
    {
        PRINTF("Bluetooth init failed (err %d)\n", err);
        return;
    }

    if (IS_ENABLED(CONFIG_BT_SETTINGS))
    {
        settings_load();
    }
    PRINTF("Bluetooth initialized\n");

    fmp_init(alert_ui);

    bt_conn_cb_register(&conn_callbacks);
#if CONFIG_BT_SMP
    bt_conn_auth_cb_register(&auth_cb_display);
#endif

    bt_app_start_advertising();
}

void peripheral_fmp_task(void *pvParameters)
{
    int err;
    
    adv_timeout_sync = xSemaphoreCreateCounting(0xFFu, 0u);
    if (NULL == adv_timeout_sync)
    {
        PRINTF("faile to create adv_timeout_sync\n");
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

    if (NULL != adv_timeout_sync)
    {
        while(1)
        {
            if (pdTRUE == xSemaphoreTake(adv_timeout_sync, portMAX_DELAY))
            {
                advertising_timeout_process();
            }
        }
    }
    else
    {
      while(1)
      {
          vTaskDelay(1000);
      }
    }
}
