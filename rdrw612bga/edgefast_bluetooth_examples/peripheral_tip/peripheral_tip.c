/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
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
#include <bluetooth/services/tip.h>
#include <fsl_debug_console.h>
#include <host_msd_fatfs.h>

/* Epoch timestamp for 10.07.2022 00:00:00 GMT */
#define EPOCH_TIMESTAMP     1657584000

#define SEMAPHORE_WAIT      0x000001FFUL

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
 static void connected(struct bt_conn *conn, uint8_t err);
 static void disconnected(struct bt_conn *conn, uint8_t reason);

 static void notifications_enabled(void);
 static void cts_notification_timeout_handler(TimerHandle_t timer_id);
 static void adv_timeout_handler(TimerHandle_t timer_id);
 static void timeout_process(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
struct bt_conn *default_conn;
static QueueHandle_t adv_timeout_sync;

OSA_MSGQ_HANDLE_DEFINE(app_queue, 10, sizeof(uint32_t));

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL,
              BT_UUID_16_ENCODE(BT_UUID_CTS_VAL)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL,
          BT_UUID_16_ENCODE(BT_UUID_RTUS_VAL)),
};
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

/* Service data */
#define CTS_TIMER_PERIOD_MS    5000

static uint32_t local_time = 1648731169;
static cts_measurement_t current_time;
static cts_local_time_info_t local_time_info;
static cts_reference_time_info_t reference_time_info;
static TimerHandle_t cts_notification_timer;

/* Advertising parameters */
#define BT_LE_ADV_FAST_CONN_NAME BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE | \
					         BT_LE_ADV_OPT_USE_NAME, \
					         BT_GAP_ADV_FAST_INT_MIN_1, \
					         BT_GAP_ADV_FAST_INT_MIN_1, NULL)

#define BT_LE_ADV_REDUCED_CONN_NAME BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE | \
					            BT_LE_ADV_OPT_USE_NAME, \
					            BT_GAP_ADV_SLOW_INT_MIN, \
					            BT_GAP_ADV_SLOW_INT_MAX, NULL)

/* Advertising timeout value */
#define ADV_TIMER_PERIOD_MS    30000

/* Advertising timer handle */
static TimerHandle_t adv_timer;

static bool bConnected = false;

/*******************************************************************************
 * Code
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
        PRINTF("Connected to peer: %s\n", addr);
        default_conn = bt_conn_ref(conn);
        bConnected = true;
        
        if (adv_timer != NULL)
        {
            /* Stop advertising timer */
            xTimerStop(adv_timer, 0);
            xTimerDelete(adv_timer, 0);
            adv_timer = NULL;
        }
        
        if (NULL != adv_timeout_sync)
		{
		   /* Free application task */
		   xSemaphoreGive(adv_timeout_sync);
		}

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
        if (cts_notification_timer != NULL)
        {
            xTimerStop(cts_notification_timer, 0);
            xTimerDelete(cts_notification_timer, 0);
            cts_notification_timer = NULL;
        }
    }
}

static void notifications_enabled(void)
{
    if (cts_notification_timer == NULL)
    {
        cts_notification_timer = xTimerCreate("notification timer", (CTS_TIMER_PERIOD_MS / portTICK_PERIOD_MS),
                      pdTRUE, NULL, cts_notification_timeout_handler);
    }

    xTimerStart(cts_notification_timer, 0);
}

static void cts_notification_timeout_handler(TimerHandle_t timer_id)
{
    /* get local timestamp */
    local_time = OSA_TimeGetMsec() / 1000U;
    /* add to epoch */
    local_time += EPOCH_TIMESTAMP;

    /* Post message and wake up application task */
    (void)OSA_MsgQPut(app_queue, &local_time);
    if (NULL != adv_timeout_sync)
    {
    	xSemaphoreGive(adv_timeout_sync);
    }
}

static void adv_timeout_handler(TimerHandle_t timer_id)
{
    if (NULL != adv_timeout_sync)
    {
        /* it is not in isr context */
        xSemaphoreGive(adv_timeout_sync);
    }
    else
    {
        /* it is in isr context */
        timeout_process();
    }
}

static void timeout_process(void)
{
    int err;
    
    if (bConnected == false)
    {
    	if (adv_timer != NULL)
		{
		   /* Stop advertising timer */
		   xTimerStop(adv_timer, 0);
		   xTimerDelete(adv_timer, 0);
		   adv_timer = NULL;
		}

		/* Stop advertising */
		err = bt_le_adv_stop();
		if (err)
		{
			PRINTF("Advertising failed to stop (err %d)\n", err);
		}

		/* Restart advertising in reduced power mode */
		err = bt_le_adv_start(BT_LE_ADV_REDUCED_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
		if (err)
		{
			PRINTF("Advertising failed to start (err %d)\n", err);
		}
    }
  
	if (NULL != adv_timeout_sync)
	{
	   /* Free application task */
	   xSemaphoreGive(adv_timeout_sync);
	}
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

    bt_conn_cb_register(&conn_callbacks);
#if CONFIG_BT_SMP
    bt_conn_auth_cb_register(&auth_cb_display);
#endif

    /* Register notifications enables callback */
    bt_cts_register_notification_callback(notifications_enabled);

    /* Set current time */
    current_time.exact_time.cts_day_date_time.date_time.year = 2022;
    current_time.exact_time.cts_day_date_time.date_time.month = UNIT_APRIL;
    current_time.exact_time.cts_day_date_time.date_time.day = 01;
    current_time.exact_time.cts_day_date_time.date_time.hours = 9;
    current_time.exact_time.cts_day_date_time.date_time.minutes = 0;
    current_time.exact_time.cts_day_date_time.date_time.seconds = 0;
    current_time.exact_time.fractions = 0;
    current_time.adjust_reason = CTS_MANUAL_UPDATE;
    bt_cts_set_current_time(NULL, current_time, local_time);

    /* Set local time */
    local_time_info.cts_time_zone = UTCp0200;
    local_time_info.cts_dst_offset = CTS_DAYLIGHT_TIME;
    bt_cts_set_local_time(local_time_info);

    /* Set reference time */
    reference_time_info.source = CTS_MANUAL;
    reference_time_info.accuracy = CTS_ACCURACY_UNKNOWN;
    reference_time_info.days_since_update = 0;
    reference_time_info.hours_since_update = 0;
    bt_cts_set_reference_time(reference_time_info);

    /* Create advertising timer */
    adv_timer = xTimerCreate("advertising timer", (ADV_TIMER_PERIOD_MS / portTICK_PERIOD_MS),
                                  pdTRUE, NULL, adv_timeout_handler);

    err = bt_le_adv_start(BT_LE_ADV_FAST_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err)
    {
        PRINTF("Advertising failed to start (err %d)\n", err);
        return;
    }

    PRINTF("Advertising successfully started\n");

    /* Start advertising timer */
    xTimerStart(adv_timer, 0);
}

void peripheral_tip_task(void *pvParameters)
{
	int err;
	uint32_t rx_data;

	adv_timeout_sync = xSemaphoreCreateCounting(0xFFU, 0U);
	if (NULL == adv_timeout_sync)
	{
		PRINTF("failed to create adv_timeout_sync\n");
	}

	err = OSA_MsgQCreate(app_queue, 10, sizeof(void *));
	if (err)
	{
		PRINTF("Failed to create app msg queue (err %d)\n", err);
		return;
	}

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_ready);
	if (err)
	{
		PRINTF("Bluetooth init failed (err %d)\n", err);
	}

	while(1)
	{
		if (NULL != adv_timeout_sync)
		{
			if (pdTRUE == xSemaphoreTake(adv_timeout_sync, SEMAPHORE_WAIT))
			{
				timeout_process();
			}
		}

		err = OSA_MsgQGet(app_queue, &rx_data, 1000);
		if (err == KOSA_StatusSuccess)
		{
			bt_cts_set_current_time(default_conn, current_time, local_time);
		}
	}
}
