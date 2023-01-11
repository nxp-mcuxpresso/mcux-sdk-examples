/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if defined(BEACON_APP) && (BEACON_APP == 1)
   
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <toolchain.h>
#include <porting.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <fsl_debug_console.h>
#include <host_msd_fatfs.h>


#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#ifndef BEACON_RSSI
#define BEACON_RSSI 0xc8
#endif

#define mAdvCompanyId 0x25, 0x00
#define mBeaconId 0xBC
#define mUuid  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
	BT_DATA_BYTES(BT_DATA_MANUFACTURER_DATA,
		      0x25, 0x00, /* Company Identifier */
		      mBeaconId, /* Beacon Identifier */
		      mUuid, /* UUID */
		      0x00, 0x00, /* A */
		      0x00, 0x00, /* B */
              0x00, 0x00, /* C */
		      BEACON_RSSI) /* Calibrated RSSI @ 1m */
};

/* Set Scan Response data */
static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static void bt_ready(int err)
{
	char addr_s[BT_ADDR_LE_STR_LEN];
	bt_addr_le_t addr = {0};
	size_t count = 1;

	if (err) {
		PRINTF("Bluetooth init failed (err %d)\n", err);
		return;
	}

	PRINTF("Bluetooth initialized\n");

	/* Start advertising */
	err = bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY, ad, ARRAY_SIZE(ad),
                          sd, ARRAY_SIZE(sd));
	if (err) {
		PRINTF("Advertising failed to start (err %d)\n", err);
		return;
	}
    
    /* For connectable advertising you would use
	 * bt_le_oob_get_local().  For non-connectable non-identity
	 * advertising an non-resolvable private address is used;
	 * there is no API to retrieve that.
	 */

	bt_id_get(&addr, &count);
	bt_addr_le_to_str(&addr, addr_s, sizeof(addr_s));

	PRINTF("Beacon started, advertising as %s\n", addr_s);
}

void beacon_task(void *pvParameters)
{
    int err;
    
    PRINTF("Starting Beacon Demo\n");
    
    /* Initialize the Bluetooth Subsystem */
    err = bt_enable(bt_ready);
    if (err)
    {
        PRINTF("Bluetooth init failed (err %d)\n", err);
        return;
    }

    while(1)
    {
        vTaskDelay(1000);
    }
}

#endif /* BEACON_APP */