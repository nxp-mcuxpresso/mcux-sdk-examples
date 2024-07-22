/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <sys/byteorder.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/byteorder.h>
#include <bluetooth/conn.h>
#include <bluetooth/audio/audio.h>
#include <bluetooth/audio/bap.h>
#include <bluetooth/audio/bap_lc3_preset.h>
#include <bluetooth/audio/csip.h>
#include <bluetooth/audio/tmap.h>
#include <bluetooth/audio/cap.h>
#include <bluetooth/audio/mcs.h>

#include "tmap_peripheral.h"

#include "fsl_debug_console.h"

#ifndef printk
#define printk PRINTF
#endif

static struct bt_conn *default_conn;
static struct k_work_delayable call_terminate_set_work;
static struct k_work_delayable media_pause_set_work;

static uint8_t unicast_server_addata[] = {
	BT_UUID_16_ENCODE(BT_UUID_ASCS_VAL),    /* ASCS UUID */
	BT_AUDIO_UNICAST_ANNOUNCEMENT_TARGETED, /* Target Announcement */
	BT_BYTES_LIST_LE16(AVAILABLE_SINK_CONTEXT),
	BT_BYTES_LIST_LE16(AVAILABLE_SOURCE_CONTEXT),
	0x00, /* Metadata length */
};

static const uint8_t cap_addata[] = {
	BT_UUID_16_ENCODE(BT_UUID_CAS_VAL),
	BT_AUDIO_UNICAST_ANNOUNCEMENT_TARGETED,
};

static uint8_t tmap_addata[] = {
	BT_UUID_16_ENCODE(BT_UUID_TMAS_VAL),                    /* TMAS UUID */
	BT_BYTES_LIST_LE16(BT_TMAP_ROLE_UMR | BT_TMAP_ROLE_CT), /* TMAP Role */
};

static uint8_t csis_rsi_addata[BT_CSIP_RSI_SIZE];
static bool peer_is_cg;
static bool peer_is_ums;

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_GAP_APPEARANCE,
		      BT_BYTES_LIST_LE16(BT_APPEARANCE_WEARABLE_AUDIO_DEVICE_EARBUD)),
	BT_DATA_BYTES(BT_DATA_UUID16_SOME, BT_UUID_16_ENCODE(BT_UUID_ASCS_VAL),
		      BT_UUID_16_ENCODE(BT_UUID_CAS_VAL), BT_UUID_16_ENCODE(BT_UUID_TMAS_VAL)),
#if defined(CONFIG_BT_CSIP_SET_MEMBER)
	BT_DATA(BT_DATA_CSIS_RSI, csis_rsi_addata, ARRAY_SIZE(csis_rsi_addata)),
#endif /* CONFIG_BT_CSIP_SET_MEMBER */
	BT_DATA(BT_DATA_SVC_DATA16, tmap_addata, ARRAY_SIZE(tmap_addata)),
	BT_DATA(BT_DATA_SVC_DATA16, cap_addata, ARRAY_SIZE(cap_addata)),
	BT_DATA(BT_DATA_SVC_DATA16, unicast_server_addata, ARRAY_SIZE(unicast_server_addata)),
};

static OSA_SEMAPHORE_HANDLE_DEFINE(sem_connected);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_security_updated);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_disconnected);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_discovery_done);

void tmap_discovery_complete(enum bt_tmap_role peer_role, struct bt_conn *conn, int err)
{
	if (conn != default_conn) {
		return;
	}

	if (err) {
		printk("TMAS discovery failed! (err %d)\n", err);
		return;
	}

	peer_is_cg = (peer_role & BT_TMAP_ROLE_CG) != 0;
	peer_is_ums = (peer_role & BT_TMAP_ROLE_UMS) != 0;
	printk("TMAP discovery done\n");
	OSA_SemaphorePost(sem_discovery_done);
}

static struct bt_tmap_cb tmap_callbacks = {
	.discovery_complete = tmap_discovery_complete
};

static void connected(struct bt_conn *conn, uint8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (err != 0) {
		printk("Failed to connect to %s (%u)\n", addr, err);

		default_conn = NULL;
		return;
	}

	printk("Connected: %s\n", addr);
	default_conn = bt_conn_ref(conn);
	OSA_SemaphorePost(sem_connected);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	if (conn != default_conn) {
		return;
	}

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Disconnected: %s (reason 0x%02x)\n", addr, reason);

	bt_conn_unref(default_conn);
	default_conn = NULL;

	OSA_SemaphorePost(sem_disconnected);
}

static void security_changed(struct bt_conn *conn, bt_security_t level,
			     enum bt_security_err err)
{
	if (err == 0) {
		printk("Security changed: %u, level %d\n", err, level);
		OSA_SemaphorePost(sem_security_updated);
	} else {
		printk("Failed to set security level: %u", err);
	}
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
	.security_changed = security_changed,
};

#if (defined(CONFIG_BT_PRIVACY) && (CONFIG_BT_PRIVACY > 0)) && \
	(defined(CONFIG_BT_CSIP_SET_MEMBER) && (CONFIG_BT_CSIP_SET_MEMBER > 0))
static bool adv_rpa_expired_cb(struct bt_le_ext_adv *adv)
{
	char rsi_str[13];
	int err;

	err = csip_generate_rsi(csis_rsi_addata);
	if (err != 0) {
		printk("Failed to generate RSI (err %d)\n", err);
		return false;
	}

	snprintk(rsi_str, ARRAY_SIZE(rsi_str), "%02x%02x%02x%02x%02x%02x",
		 csis_rsi_addata[0], csis_rsi_addata[1], csis_rsi_addata[2],
		 csis_rsi_addata[3], csis_rsi_addata[4], csis_rsi_addata[5]);

	printk("PRSI: 0x%s\n", rsi_str);

	err = bt_le_ext_adv_set_data(adv, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Failed to set advertising data (err %d)\n", err);
		return false;
	}

	return true;
}
#endif /* CONFIG_BT_PRIVACY && CONFIG_BT_CSIP_SET_MEMBER */

#if (defined(CONFIG_BT_PRIVACY) && (CONFIG_BT_PRIVACY > 0)) && \
	(defined(CONFIG_BT_CSIP_SET_MEMBER) && (CONFIG_BT_CSIP_SET_MEMBER > 0))
static const struct bt_le_ext_adv_cb adv_cb = {
	.rpa_expired = adv_rpa_expired_cb,
};
#else
static const struct bt_le_ext_adv_cb adv_cb;
#endif /* CONFIG_BT_PRIVACY && CONFIG_BT_CSIP_SET_MEMBER */

static void audio_timer_timeout(struct k_work *work)
{
	int err = ccp_terminate_call();

	if (err != 0) {
		printk("Error sending call terminate command!\n");
	}
}

static void media_play_timeout(struct k_work *work)
{
	int err = mcp_send_cmd(BT_MCS_OPC_PAUSE);

	if (err != 0) {
		printk("Error sending pause command!\n");
	}
}

void tmap_peripheral_task(void *param)
{
	int err;
	struct bt_le_ext_adv *adv;

	(void)OSA_SemaphoreCreate(sem_connected, 0);
	(void)OSA_SemaphoreCreate(sem_security_updated, 0);
	(void)OSA_SemaphoreCreate(sem_disconnected, 0);
	(void)OSA_SemaphoreCreate(sem_discovery_done, 0);

	err = bt_enable(NULL);
	if (err != 0) {
		printk("Bluetooth init failed (err %d)\n", err);
		while(1);
	}

	printk("Bluetooth initialized\n");

	bt_conn_cb_register(&conn_callbacks);

	k_work_init_delayable(&call_terminate_set_work, audio_timer_timeout);
	k_work_init_delayable(&media_pause_set_work, media_play_timeout);

	printk("Initializing TMAP and setting role\n");
	err = bt_tmap_register((enum bt_tmap_role)(BT_TMAP_ROLE_CT | BT_TMAP_ROLE_UMR));
	if (err != 0) {
		while(1);
	}

#if defined(CONFIG_TMAP_PERIPHERAL_DUO) && (CONFIG_TMAP_PERIPHERAL_DUO > 0)
	if (IS_ENABLED(CONFIG_TMAP_PERIPHERAL_DUO)) {
		err = csip_set_member_init();
		if (err != 0) {
			printk("CSIP Set Member init failed (err %d)\n", err);
			while(1);
		}

		err = csip_generate_rsi(csis_rsi_addata);
		if (err != 0) {
			printk("Failed to generate RSI (err %d)\n", err);
			while(1);
		}
	}
#endif

	err = vcp_vol_renderer_init();
	if (err != 0) {
		while(1);
	}
	printk("VCP initialized\n");

	err = bap_unicast_sr_init();
	if (err != 0) {
		while(1);
	}
	printk("BAP initialized\n");

	err = bt_le_ext_adv_create(BT_LE_EXT_ADV_CONN_NAME, &adv_cb, &adv);
	if (err) {
		printk("Failed to create advertising set (err %d)\n", err);
		while(1);
	}

	err = bt_le_ext_adv_set_data(adv, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Failed to set advertising data (err %d)\n", err);
		while(1);
	}

	err = bt_le_ext_adv_start(adv, BT_LE_EXT_ADV_START_DEFAULT);
	if (err) {
		printk("Failed to start advertising set (err %d)\n", err);
		while(1);
	}

	printk("Advertising successfully started\n");
	OSA_SemaphoreWait(sem_connected, osaWaitForever_c);
	OSA_SemaphoreWait(sem_security_updated, osaWaitForever_c);

	err = bt_tmap_discover(default_conn, &tmap_callbacks);
	if (err != 0) {
		while(1);
	}
	OSA_SemaphoreWait(sem_discovery_done, osaWaitForever_c);

	err = ccp_call_ctrl_init(default_conn);
	if (err != 0) {
		while(1);
	}
	printk("CCP initialized\n");

	err = mcp_ctlr_init(default_conn);
	if (err != 0) {
		while(1);
	}
	printk("MCP initialized\n");

	if (peer_is_cg) {
		/* Initiate a call with CCP */
		err = ccp_originate_call();
		if (err != 0) {
			printk("Error sending call originate command!\n");
		}
		/* Start timer to send terminate call command */
		k_work_schedule(&call_terminate_set_work, 2000);
	}

	if (peer_is_ums) {
		/* Play media with MCP */
		err = mcp_send_cmd(BT_MCS_OPC_PLAY);
		if (err != 0)
			printk("Error sending media play command!\n");

		/* Start timer to send media pause command */
		k_work_schedule(&media_pause_set_work, 2000);

		err = OSA_SemaphoreWait(sem_disconnected, osaWaitForever_c);
		if (err != 0) {
			printk("failed to take sem_disconnected (err %d)\n", err);
		}
	}

	while(1);
}
