/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include "axis_nc.h"
#include "cia402.h"
#include "ethercat.h"
#include "servo.h"

#define skip_blank(p) while ((*(p)) == ' ') (p) += 1
inline static int skip_semicolon(char **str)
{
	char *p = strchr((char *)*str, ';');
	if (!p) {
		printf("Each instruction must end with ';'\r\n");
		return -1;
	}
	*str = p + 1;
	return 0;
}

static int parameter_parser(char **str, char *parameter_name, int32_t *parameter)
{
	int len = strlen(parameter_name);
	int n;
	char *s = *str;
	if (strncmp(parameter_name, s, len) == 0) {
		s += len;
		n = sscanf(s, "=%d", parameter);
		if (n != 1) {
			printf("The parameter \"%s\" can not be assigned\r\n", parameter_name);
			return -1;
		}
		if (skip_semicolon(str) < 0) {
			return -1;
		}
		return 1;
	}
	return 0;

}

int32_t axis_nc_get_next_pos(struct axis_t *axis)
{
	struct axis_csp_status_t *status = &axis->axis_status;
	struct trajectory_planner_t *tp = &status->tp[status->current_tp];
	double value;
	int32_t ret;
	uint32_t index = tp->current_point;
	if (index <  tp->point_accel_end)
		value = 0.5 * status->acceleration * index * index;
	else if (index > tp->point_decel_start)
		value = tp->decel_pos + 0.5 * status->deceleration * pow((index - tp->point_decel_start), 2);
	else
		value = tp->accel_pos + tp->uniform_speed * ( index - tp->point_accel_end);
	value *= tp->direction;
	value += tp->origin_pos;
	value *= axis->scale;
	ret = (int64_t)value & 0xFFFFFFFF;

	tp->current_point++;
	if (tp->current_point < tp->points_num) {
		return ret;
	}

	tp->current_point = 0;
	status->current_tp++;
	if (status->current_tp < status->tp_num) {
		return ret;
	}

	if (status->is_cyclic && status->csp_status != csp_status_pre_stop) {
		status->current_tp = 1;
	} else {
		status->csp_status = csp_status_stop;
		status->current_tp = 0;
	}
	return ret;
}

/* Uising trapezoidal move profile */
static int trajectory_planner(struct axis_t *axis, struct trajectory_planner_t *tp)
{
	struct axis_csp_status_t *status = &axis->axis_status;
	double delta_pos = tp->target_pos - tp->origin_pos;
	double accel = status->acceleration;
	double decel = status->deceleration;
	double point_num = tp->points_num;
	double a0 = 0.5*(accel + accel * accel / decel);
	double b0 = -accel*point_num;
	double c0;

	tp->direction = 1;
	if (delta_pos < 0) {
		tp->direction = -1;
	}

	delta_pos *= tp->direction;

	c0 = delta_pos;
	double delta = b0 * b0 - 4 * a0 * c0;
	if ( delta < 0 ) {
		return -1;
	}
	double t0 = (-1 * b0 - sqrt(delta)) / 2 / a0;
	double t1 = point_num - accel * t0 / decel;
	if (t0 * accel > status->max_speed) {
		return -2;
	}

	tp->accel_pos = 0.5 * accel * t0 * t0;
	tp->decel_pos = tp->accel_pos + accel * t0 * (t1 - t0);
	tp->point_accel_end = (uint32_t)(t0 + 0.5);
	tp->point_decel_start = (uint32_t)(t1 + 0.5);
	tp->uniform_speed = accel * t0;
	return 0;
}

static int Tp_arrays_parser(struct axis_t *axis, char *tp_str, uint32_t period_ns)
{
	struct axis_csp_status_t *status = &axis->axis_status;
	double next_pos;
	uint32_t time;
	int n = 0, i = 0;
	char *p = tp_str;
	char *tp_arrays = NULL;
	int ret;
	int32_t value;
	double origin_pos = 0; // = (double)axis->current_position / axis->scale;
	double bias = 0;
	if (axis->mode != op_mode_csp) {
		return -1;
	}

	if (status->csp_status == csp_status_running) {
		return -2;
	}

	while (*p != '\0') {
		skip_blank(p);
		ret = parameter_parser(&p, "Cyclic", &value);
		if (ret < 0)
			return -5;
		else if ( ret == 1) {
			status->is_cyclic = value > 0 ? 1 : 0;
			continue;
		}

		ret = parameter_parser(&p, "Scale", &value);
		if (ret < 0)
			return -5;
		else if ( ret == 1) {
			axis->scale = value;
			continue;
		}

		ret = parameter_parser(&p, "Bias", &value);
		if (ret < 0)
			return -1;
		else if ( ret == 1) {
			status->bias = value;
			continue;
		}

		ret = parameter_parser(&p, "Accel", &value);
		if (ret < 0)
			return -1;
		else if ( ret == 1) {
			status->acceleration = value;
			continue;
		}

		ret = parameter_parser(&p, "Decel", &value);
		if (ret < 0)
			return -1;
		else if ( ret == 1) {
			status->deceleration = value;
			continue;
		}

		ret = parameter_parser(&p, "Max_speed", &value);
		if (ret < 0)
			return -1;
		else if ( ret == 1) {
			status->max_speed = value;
			continue;
		}

		if (strncmp("TpArrays", p, 8) == 0) {
			p += 8;
			tp_arrays = strchr(p, '[');
			if (!tp_arrays) {
				return -5;
			}
			char *pp = strchr(p, ']');
			if (!pp) {
				return -5;
			}
			tp_arrays += 1; // skip '['
			pp = strchr(pp + 1, ';');
			if (!pp) {
				return -5;
			}
			p = pp + 1;
			continue;
		}
		p++;
	}
	p = tp_arrays;
	while (1) {
		if (p == NULL) {
			break;
		}
		if ((p = strchr(p, '(')) == NULL)
			break;
		n++;
		p++;
	}

	if (n == 0) {
		return -3;
	}

	if (n >= TRAJECTORY_PLANNER_MAX) {
		return -4;
	}

	if (status->is_cyclic) {
		n += 1;
	}

	status->tp_num = n;

	p = tp_arrays;
	bias = (double)status->bias + (double)axis->current_position / axis->scale;
	origin_pos = bias;
	while (1) {
		skip_blank(p);
		n = sscanf(p, "(%d:%d)", &value, &time);
		if (n != 2)
			return -5;
		next_pos = bias + value;
		status->tp[i].points_num = (uint64_t)time * 1000000 / period_ns;
		status->tp[i].current_point = 0;
		status->tp[i].origin_pos = origin_pos;
		status->tp[i].target_pos = next_pos;
		trajectory_planner(axis, status->tp + i);
		i++;
		origin_pos = next_pos;
		skip_blank(p);
		if ((p = strchr(p, ',')) == NULL)
			break;
		p++;
	}
	if (status->is_cyclic) {
		n = sscanf(tp_arrays, "(%d:%d)", &value, &time);
		if (n != 2)
			return -5;
		next_pos = bias + value;
		status->tp[i].points_num = (uint64_t)time * 1000000 / period_ns;
		status->tp[i].current_point = 0;
		status->tp[i].origin_pos = origin_pos;
		status->tp[i].target_pos = next_pos;
		trajectory_planner(axis, status->tp + i);
	}
	status->csp_status = csp_status_ready;
	return 0;
}

int axis_nc_init(struct axis_t *axis, char *tp, uint32_t period_ns)
{
	struct axis_csp_status_t *status = &axis->axis_status;
	ec_slavet *slave = axis->servo->slave;
	int ret;
	int psize = 4;
	axis->mode = op_mode_csp;
	status->csp_status = csp_status_stop;
	axis->scale = 1;
	axis->output_offset = slave->outputs + axis->axis_offset * rxpdos_axis_size;
	axis->input_offset = slave->inputs + axis->axis_offset * txpdos_axis_size;
	ec_SDOread(axis->servo->slave_id + 1, 0x6064 + axis->axis_offset * 0x800, 0, FALSE, &psize, &axis->current_position, EC_TIMEOUTSAFE);
	ret = Tp_arrays_parser(axis, tp, period_ns);
	if (ret < 0) {
		return ret;
	}
	status->csp_status = csp_status_ready;
	return 0;
}

int axis_nc_start(struct axis_t *axis)
{
	struct axis_csp_status_t *status = &axis->axis_status;
	status->csp_status = csp_status_running;
	return 0;
}

int axis_nc_stop(struct axis_t *axis)
{
	struct axis_csp_status_t *status = &axis->axis_status;
	status->csp_status = csp_status_pre_stop;
	return 0;
}
