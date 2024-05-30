/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include <stdlib.h>
#include "axis_nc.h"
#include "servo.h"

int servo_slave_check(struct servo_t *servo, int servo_num)
{
	int index;
	int ret = -1;
	int i;
	for (i = 0; i < servo_num; i++) {
		index = servo[i].slave_id + 1;
		if (index > ec_slavecount) {
			ret = -i;
		} else if (ec_slave[index].eep_man != servo[i].VendorId || ec_slave[index].eep_id != servo[i].ProductID) {
			ret = -i;
		} else {
			ret = 1;
		}

		if (ret < 0) {
			break;
			servo[i].slave = NULL;
		} else {
			servo[i].slave = &ec_slave[index];
		}
	}
	return ret;
}

int servo_synced_check(struct servo_t *servo, int servo_num)
{
	int ret = 1;
	int i;
	int wkc;
	int32_t diff;
	for (i = 0; i < servo_num; i++) {
		if (!servo[i].synced) {
			wkc=ecx_FPRD(ecx_context.port, servo[i].slave->configadr, ECT_REG_DCSYSDIFF, sizeof(diff), &diff, EC_TIMEOUTRET);	
			if (wkc == 1 && abs(diff) < 100000) {
				servo[i].synced = 1;
			} else {
				ret = -i;
				break;
			}
		}
	}
	return ret;
}
