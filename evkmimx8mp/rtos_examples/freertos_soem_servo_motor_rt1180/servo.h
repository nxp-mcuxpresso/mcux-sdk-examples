/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SERVO__NC____H__
#define __SERVO__NC____H__
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "ethercat.h"

struct servo_t {
	uint32 VendorId;
	uint32 ProductID;
	int slave_id;
	ec_slavet *slave;
	int (*setup)(uint16 slave);
	int synced;
	int axis_num;
};

int servo_synced_check(struct servo_t *servo, int servo_num);
int servo_slave_check(struct servo_t *servo, int servo_num);
#endif
