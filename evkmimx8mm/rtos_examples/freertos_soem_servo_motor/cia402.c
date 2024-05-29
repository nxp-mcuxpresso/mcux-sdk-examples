/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "ethercat.h"
#include "axis_nc.h"
#include "servo.h"
#include "cia402.h"

uint32_t tx_pdo_entry[1][4] = {
	{ // axis 0
	0x60410010, // statusword
	0x60640020, // actual_position
	0x60610008, // op_mode_display
	0x606C0020	// actual_velocity
	}
};

uint32_t rx_pdo_entry[1][3] = {
	{ // axis 0
	0x60400010, // controlword
	0x60600008, // op_mode
	0x607A0020 // Target position
	}
};

int servo_pdo_remap(struct servo_t *servo)
{
	int slave = servo->slave_id + 1;
	int wkc = 0, wkc_expected = 10;
	int i;
	uint8 num_8b = 0;
	uint16 map_1c12[2] = {0x0001, 0x1600};
	uint16 map_1c13[2] = {0x0001, 0x1a00};
	uint16 num_16b = 0;
	wkc += ec_SDOwrite(slave, 0x1c12, 0x00, 0, 2, &num_16b, EC_TIMEOUTSAFE);
	wkc += ec_SDOwrite(slave, 0x1c13, 0x00, 0, 2, &num_16b, EC_TIMEOUTSAFE);

	num_8b = 0;
	wkc += ec_SDOwrite(slave, 0x1600, 0x00, 0, 1, &num_8b, EC_TIMEOUTSAFE);
	for (i = 0; i < servo->axis_num; i++) {
		wkc += ec_SDOwrite(slave, 0x1600, i * 3 + 0x01, 0, 4, &rx_pdo_entry[i][0], EC_TIMEOUTSAFE);
		wkc += ec_SDOwrite(slave, 0x1600, i * 3 + 0x02, 0, 4, &rx_pdo_entry[i][1], EC_TIMEOUTSAFE);
		wkc += ec_SDOwrite(slave, 0x1600, i * 3 + 0x03, 0, 4, &rx_pdo_entry[i][2], EC_TIMEOUTSAFE);
		num_8b += 3;
		wkc_expected += 3;
	}
	wkc += ec_SDOwrite(slave, 0x1600, 0x00, 0, 1, &num_8b, EC_TIMEOUTSAFE);

	num_8b = 0;
	wkc += ec_SDOwrite(slave, 0x1a00, 0x00, 0, 1, &num_8b, EC_TIMEOUTSAFE);
	for (i = 0; i < servo->axis_num; i++) {
		wkc += ec_SDOwrite(slave, 0x1a00, i * 4 + 0x01, 0, 4, &tx_pdo_entry[i][0], EC_TIMEOUTSAFE);
		wkc += ec_SDOwrite(slave, 0x1a00, i * 4 + 0x02, 0, 4, &tx_pdo_entry[i][1], EC_TIMEOUTSAFE);
		wkc += ec_SDOwrite(slave, 0x1a00, i * 4 + 0x03, 0, 4, &tx_pdo_entry[i][2], EC_TIMEOUTSAFE);
		wkc += ec_SDOwrite(slave, 0x1a00, i * 4 + 0x04, 0, 4, &tx_pdo_entry[i][3], EC_TIMEOUTSAFE);
		num_8b += 4;
		wkc_expected += 4;
	}
	wkc += ec_SDOwrite(slave, 0x1a00, 0x00, 0, 1, &num_8b, EC_TIMEOUTSAFE);

	wkc += ec_SDOwrite(slave, 0x1c12, 0x01, 0, sizeof(map_1c12[1]), &map_1c12[1], EC_TIMEOUTSAFE);
	wkc += ec_SDOwrite(slave, 0x1c12, 0x00, 0, sizeof(map_1c12[0]), &map_1c12[0], EC_TIMEOUTSAFE);

	wkc += ec_SDOwrite(slave, 0x1c13, 0x01, 0, sizeof(map_1c13[1]), &map_1c13[1], EC_TIMEOUTSAFE);
	wkc += ec_SDOwrite(slave, 0x1c13, 0x00, 0, sizeof(map_1c13[0]), &map_1c13[0], EC_TIMEOUTSAFE);

	return wkc >= wkc_expected ? 1 : 0;
}

#define contrlword_shutdown(c)				(((c) | 0x6) & ~0x81)
#define contrlword_switch_on(c)		 		(((c) | 0x7) & ~0x88)
#define contrlword_disable_voltage(c)		((c)	& ~0x82)
#define contrlword_quick_stop(c)			(((c) | 0x2) & ~0x84)
#define contrlword_disable_operation(c)	 	(((c) | 0x7) & ~0x88)
#define contrlword_enable_operation(c)		(((c) | 0xF) & ~0x80)
#define contrlword_fault_reset(c)			((c) | 0x80)
#define contrlword_new_set_point(c)	 		((c) | 0x10)
#define contrlword_new_set_point_imm(c)	 	((c) | 0x30)

static int get_axle_state(uint16_t status_word) {
	if ((status_word & 0x4F) == 0x40)
		return switch_on_disable;
	if ((status_word & 0x6F) == 0x21)
		return ready_to_switch_on;
	if ((status_word & 0x6F) == 0x23)
		return switched_on;
	if ((status_word & 0x6F) == 0x27)
		return operation_enable;
	if ((status_word & 0x6F) == 0x07)
		return quick_stop_active;
	if ((status_word & 0x4F) == 0xF)
		return fault_reaction_active;
	if ((status_word & 0x4F) == 0x08)
		return fault;
	else
		return no_ready_to_switch_on;
}

int axis_start(struct axis_t *si, uint8_t mode) {
	int ret = 0;
	int ss;
	uint16 s = PDO_read_status_word(si);
	ss = get_axle_state(s);

	if (s & 0x8) {
		PDO_write_control_word(si, 0x80);
		return 0;
	}

	switch (ss) {
		case (no_ready_to_switch_on):
		case (switch_on_disable):
			PDO_write_control_word(si, contrlword_shutdown(0));
			break;
		case (ready_to_switch_on):
			PDO_write_control_word(si, contrlword_switch_on(0));
			break;
		case (switched_on):
			PDO_write_control_word(si, contrlword_enable_operation(0));
			PDO_write_op_mode(si, mode);
			break;
		case (operation_enable):
			ret = 1;
			break;
		case (quick_stop_active):
		case (fault_reaction_active):
		case (fault):
		default:
			ret = -1;
	}
	return ret;
}
