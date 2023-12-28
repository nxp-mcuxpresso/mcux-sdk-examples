/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <zephyr/types.h>
#include <stdio.h>
#include <stddef.h>
#include <errno/errno.h>
#include <toolchain.h>
#include <porting.h>
#include "fsl_debug_console.h"

#include "fsl_shell.h"

#include "le_audio_shell.h"

shell_handle_t s_shellHandle;
SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
extern serial_handle_t g_serialHandle;

extern void le_audio_bis_play(void);
extern void le_audio_bis_pause(void);

static shell_status_t play(shell_handle_t shellHandle, int32_t argc, char **argv)
{
	le_audio_bis_play();
	return kStatus_SHELL_Success;
}

static shell_status_t pause(shell_handle_t shellHandle, int32_t argc, char **argv)
{
	le_audio_bis_pause();
	return kStatus_SHELL_Success;
}

SHELL_COMMAND_DEFINE(play,  "play  :resume broadcast.\r\n", play,  0);
SHELL_COMMAND_DEFINE(pause, "pause :stop broadcast.\r\n",   pause, 0);

void le_audio_shell_init(void)
{
	/* Init SHELL */
    s_shellHandle = &s_shellHandleBuffer[0];
    SHELL_Init(s_shellHandle, g_serialHandle, "BMS>> ");
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(play));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(pause));

	SHELL_Printf(s_shellHandle, "\r\nBroadcast Media Sender.\r\n");
}
