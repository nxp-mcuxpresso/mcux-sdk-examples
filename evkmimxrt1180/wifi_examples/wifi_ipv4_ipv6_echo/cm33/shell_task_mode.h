/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SHELL_TASK_MODE_H_
#define SHELL_TASK_MODE_H_

#define SHELL_MODE_NO_PROMPT ""
#define SHELL_MODE_DEFAULT   "SHELL>> "

void shell_task_set_mode(const char *mode_name);

#endif /* SHELL_TASK_MODE_H_ */
