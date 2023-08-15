/*
 * Copyright 2020-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _SSRC_PROC_H_
#define _SSRC_PROC_H_

#include "ssrc_head.h" // SSRC library

#define MINBLOCKSIZE 100

int SSRC_Proc_Init(void *arg);
int SSRC_Proc_Execute(void *arg, void *inputBuffer, int size);
int SSRC_Proc_Deinit();

int SSRC_register_post_process(void *streamer);

#endif
