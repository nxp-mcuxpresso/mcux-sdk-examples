/*
 * Copyright 2021 NXP
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno/errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <porting.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include "service.h"
#include "service/hrs.h"

static struct bt_conn *default_conn = NULL;

/*******************************************************************************
 * Code
 ******************************************************************************/
void peripheral_hrs_connect(void *args)
{
    service_connect_param_t *param = (service_connect_param_t *) args;
    struct bt_conn *conn = param->conn;
    default_conn = bt_conn_ref(conn);
}

void peripheral_hrs_disconnect(void *args)
{
    if (default_conn) {
        bt_conn_unref(default_conn);
        default_conn = NULL;
    }
    // todo : enable adv ?
}

void peripheral_hrs_notify(void)
{
	static uint8_t heartrate = 90U;

	/* Heartrate measurements simulation */
	heartrate++;
	if (heartrate == 160U) {
		heartrate = 90U;
	}

	bt_hrs_notify(heartrate);
}

void peripheral_hrs_task(void *pvParameters)
{
    while(1)
    {
        vTaskDelay(1000);

		/* Heartrate measurements simulation */
        peripheral_hrs_notify();
    }
}
