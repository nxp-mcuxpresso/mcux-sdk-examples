/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CIA_402__H__
#define __CIA_402__H__
#include <inttypes.h>

#include "axis_nc.h"

#define status_word_offset 0
#define actual_position_offset 2
#define op_mode_display_offset 6
#define actual_velocity_offset 7

#define control_word_offset 0
#define op_mode_offset 2
#define targe_position_offset 3 

#define txpdos_axis_size 11 
#define rxpdos_axis_size 7 

#define PDO_write_control_word(si, value) 		*((uint16 *)((si)->output_offset + control_word_offset)) = (value)
#define PDO_write_op_mode(si, value) 			*((uint8 *)((si)->output_offset + op_mode_offset)) = (value)
#define PDO_write_targe_position(si, value) 	*((int32 *)((si)->output_offset + targe_position_offset)) = (value)

#define PDO_read_control_word(si) 		*((uint16 *)((si)->output_offset + control_word_offset))
#define PDO_read_op_mode(si) 			*((uint8 *)((si)->output_offset + op_mode_offset))
#define PDO_read_targe_position(si) 	*((uint32 *)((si)->output_offset + targe_position_offset))

#define PDO_read_status_word(si) 		*((uint16 *)((si)->input_offset + status_word_offset))
#define PDO_read_actual_position(si) 	*((uint32 *)((si)->input_offset + actual_position_offset))
#define PDO_read_op_mode_display(si) 	*((uint8 *)((si)->input_offset + op_mode_display_offset))
#define PDO_read_actual_velocity(si) 	*((uint32 *)((si)->input_offset + actual_velocity_offset))

#define	no_ready_to_switch_on 0
#define	switch_on_disable 1
#define ready_to_switch_on 2
#define	switched_on 3
#define	operation_enable 4
#define quick_stop_active 5
#define fault_reaction_active 6
#define fault 7

#define    op_mode_no   0
#define    op_mode_pp   1
#define    op_mode_vl   2
#define    op_mode_pv   3
#define    op_mode_hm   6
#define    op_mode_ip   7
#define    op_mode_csp  8
#define    op_mode_csv  9
#define    op_mode_cst  10

int servo_pdo_remap(struct servo_t *servo);
int axis_start(struct axis_t *axis, uint8_t mode);
#endif
