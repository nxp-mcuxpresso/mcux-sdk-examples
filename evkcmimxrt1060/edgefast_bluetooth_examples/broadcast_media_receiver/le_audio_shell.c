/*
 * Copyright 2023-2024 NXP
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
#include "hw_codec.h"

#include "le_audio_shell.h"
#include "le_audio_common.h"

#if defined(LE_AUDIO_SYNC_ENABLE) && (LE_AUDIO_SYNC_ENABLE > 0)
#include "le_audio_sync.h"
#endif

shell_handle_t s_shellHandle;
SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
extern serial_handle_t g_serialHandle;

extern int config_broadcast_code(uint8_t *data, int len);

static OSA_SEMAPHORE_HANDLE_DEFINE(sem_mode_role_selected);

static shell_status_t sink_init(shell_handle_t shellHandle, int32_t argc, char **argv)
{
	char *role = argv[1];
	static char new_prompt[32];

	memset(new_prompt, 0, sizeof(new_prompt));

	(void)strcpy(new_prompt, "BMR@");

	if (0 == strcmp(role, "left"))
	{
		le_audio_sink_role_set(AUDIO_SINK_ROLE_LEFT);
		(void)strcat(new_prompt, "left>> ");
	}
	else if (0 == strcmp(role, "right"))
	{
		le_audio_sink_role_set(AUDIO_SINK_ROLE_RIGHT);
		(void)strcat(new_prompt, "right>> ");
	}
	else
	{
		return kStatus_SHELL_RetUsage;
	}

	OSA_SemaphorePost(sem_mode_role_selected);

	SHELL_ChangePrompt(shellHandle, new_prompt);

	return kStatus_SHELL_Success;
}

static shell_status_t vol_set(shell_handle_t shellHandle, int32_t argc, char **argv)
{
	int value;
	uint8_t volume;

	value = atoi(argv[1]);

	if((value == 0) && (0 != strcmp(argv[1], "0")))
	{
		return kStatus_SHELL_RetUsage;
	}

	if((value < 0) || (255 < value))
	{
		return kStatus_SHELL_RetUsage;
	}

	volume = (uint8_t)value;

	hw_codec_vol_set(volume * 100 / 255);
	return kStatus_SHELL_Success;
}

static shell_status_t vol_up(shell_handle_t shellHandle, int32_t argc, char **argv)
{
	hw_codec_vol_up();

	PRINTF("vol: %d\n", hw_codec_vol_get() * 255 / 100);

	return kStatus_SHELL_Success;
}

static shell_status_t vol_down(shell_handle_t shellHandle, int32_t argc, char **argv)
{
	hw_codec_vol_down();

	PRINTF("vol: %d\n", hw_codec_vol_get() * 255 / 100);

	return kStatus_SHELL_Success;
}

static shell_status_t vol_mute(shell_handle_t shellHandle, int32_t argc, char **argv)
{
	hw_codec_mute();
	return kStatus_SHELL_Success;
}

static shell_status_t vol_unmute(shell_handle_t shellHandle, int32_t argc, char **argv)
{
	hw_codec_unmute();

	PRINTF("vol: %d\n", hw_codec_vol_get() * 255 / 100);

	return kStatus_SHELL_Success;
}

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

static shell_status_t sync_info(shell_handle_t shellHandle, int32_t argc, char **argv)
{
#if defined(LE_AUDIO_SYNC_ENABLE) && (LE_AUDIO_SYNC_ENABLE > 0)
	struct sync_info info;

	le_audio_sync_info_get(&info);

	PRINTF("sync info - iso_interval: %u, sync_delay: %u, pd: %u, ts: %u\n", info.iso_interval, info.sync_delay, info.presentation_delay, info.time_stamp);
#else
	PRINTF("LE_AUDIO_SYNC_ENABLE not enabled!\n");
#endif

	return kStatus_SHELL_Success;
}

static shell_status_t sync_test_mode(shell_handle_t shellHandle, int32_t argc, char **argv)
{
#if defined(LE_AUDIO_SYNC_ENABLE) && (LE_AUDIO_SYNC_ENABLE > 0)
	int mode;

	mode = atoi(argv[1]);

	if(le_audio_sync_test_mode(mode))
	{
		return kStatus_SHELL_Error;
	}
#else
	PRINTF("LE_AUDIO_SYNC_ENABLE not enabled!\n");
#endif

	return kStatus_SHELL_Success;
}

static shell_status_t set_broadcast_code(shell_handle_t shellHandle, int32_t argc, char **argv)
{
	char *format;
	char *input;
	uint8_t broadcast_code[BT_AUDIO_BROADCAST_CODE_SIZE];
	
	format = argv[1];
	input = argv[2];

	int len = strlen(input);
	
	if(0 == strcmp(format, "str"))
	{
		if(len <= BT_AUDIO_BROADCAST_CODE_SIZE)
		{
			memcpy(broadcast_code, input, len);
		}
		else
		{
			return kStatus_SHELL_Error;
		}

		if(config_broadcast_code(broadcast_code, len))
		{
			return kStatus_SHELL_Error;
		}
	}
	else if(0 == strcmp(format, "hex"))
	{
		if((len % 2 == 0) && (len <= 2 * BT_AUDIO_BROADCAST_CODE_SIZE))
		{
			int ret = hex2bin(input, len, broadcast_code, BT_AUDIO_BROADCAST_CODE_SIZE);
			if(ret != len / 2)
			{
				return kStatus_SHELL_Error;
			}
		}
		else
		{
			return kStatus_SHELL_Error;
		}

		if(config_broadcast_code(broadcast_code, len / 2))
		{
			return kStatus_SHELL_Error;
		}
	}
	else
	{
		return kStatus_SHELL_Error;
	}

	return kStatus_SHELL_Success;
}

SHELL_COMMAND_DEFINE(init,           "init left|right\r\n", sink_init,  1);
SHELL_COMMAND_DEFINE(vol_set,        "vol_set [0-255]\r\n", vol_set,    1);
SHELL_COMMAND_DEFINE(vol_up,         "vol_up\r\n",          vol_up,     0);
SHELL_COMMAND_DEFINE(vol_down,       "vol_down\r\n",        vol_down,   0);
SHELL_COMMAND_DEFINE(vol_mute,       "vol_mute\r\n",        vol_mute,   0);
SHELL_COMMAND_DEFINE(vol_unmute,     "vol_unmute\r\n",      vol_unmute, 0);
SHELL_COMMAND_DEFINE(play,           "play\r\n",            play,       0);
SHELL_COMMAND_DEFINE(pause,          "pause\r\n",           pause,      0);
SHELL_COMMAND_DEFINE(sync_info,      "sync_info\r\n",       sync_info,  0);
SHELL_COMMAND_DEFINE(sync_test_mode, "sync_test_mode [0-2] - 0: disable; 1: 500hz sine; 2: 10ms 500hz sine + 20ms mute\r\n", sync_test_mode,  1);
SHELL_COMMAND_DEFINE(set_broadcast_code, "set_broadcast_code [str,hex] [data]\r\n", set_broadcast_code, 2);

void le_audio_shell_init(void)
{
	/* Init SHELL */
	s_shellHandle = &s_shellHandleBuffer[0];
	SHELL_Init(s_shellHandle, g_serialHandle, "BMR>> ");
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(init));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(vol_set));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(vol_up));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(vol_down));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(vol_mute));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(vol_unmute));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(play));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(pause));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(sync_info));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(sync_test_mode));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(set_broadcast_code));

	SHELL_Printf(s_shellHandle, "\r\nBroadcast Media Receiver.\r\n");
	SHELL_Printf(s_shellHandle, "\r\nPlease select sink role \"left\"|\"right\" use \"init\" command.\r\n");
	SHELL_PrintPrompt(s_shellHandle);
	(void)OSA_SemaphoreCreate(sem_mode_role_selected, 0);
	(void)OSA_SemaphoreWait(sem_mode_role_selected, osaWaitForever_c);
}
