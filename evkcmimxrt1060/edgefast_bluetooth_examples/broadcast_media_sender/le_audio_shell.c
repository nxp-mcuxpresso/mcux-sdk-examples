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
#include <bluetooth/audio/audio.h>

#include "fsl_shell.h"

#include "le_audio_shell.h"

shell_handle_t s_shellHandle;
SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
extern serial_handle_t g_serialHandle;

extern void le_audio_bis_play(void);
extern void le_audio_bis_pause(void);
extern int open_wav_file(char *path);
extern int select_lc3_preset(char *preset_name);
extern void print_all_preset(int sample_rate);
extern int modify_rtn(int rtn);
extern int modify_pd(int pd);
extern int modify_phy(int phy);
extern int modify_packing(int packing);
extern int config_broadcast_code(uint8_t *data, int len);
extern void print_sync_info(void);

static shell_status_t wav_open(shell_handle_t shellHandle, int32_t argc, char **argv)
{
	int err;
	char *path = argv[1];

	err = open_wav_file(path);
	if(err)
	{
		SHELL_Printf(s_shellHandle, "wav open fail %d\n", err);
		return kStatus_SHELL_Error;
	}

	return kStatus_SHELL_Success;
}

static shell_status_t lc3_preset_list(shell_handle_t shellHandle, int32_t argc, char **argv)
{
	print_all_preset(0);

	return kStatus_SHELL_Success;
}

static shell_status_t lc3_preset(shell_handle_t shellHandle, int32_t argc, char **argv)
{
	int err;
	char *name = argv[1];

	err = select_lc3_preset(name);
	if(err)
	{
		SHELL_Printf(s_shellHandle, "set lc3 preset fail %d\n", err);
		return kStatus_SHELL_Error;
	}

	return kStatus_SHELL_Success;
}

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
	print_sync_info();

	return kStatus_SHELL_Success;
}

static shell_status_t config_rtn(shell_handle_t shellHandle, int32_t argc, char **argv)
{
	int err;
	int rtn;
	
	rtn = atoi(argv[1]);

	err = modify_rtn(rtn);
	if(err)
	{
		SHELL_Printf(s_shellHandle, "config rtn fail %d\n", err);
		return kStatus_SHELL_Error;
	}

	return kStatus_SHELL_Success;
}

static shell_status_t config_pd(shell_handle_t shellHandle, int32_t argc, char **argv)
{
	int err;
	int pd;
	
	pd = atoi(argv[1]);

	err = modify_pd(pd);
	if(err)
	{
		SHELL_Printf(s_shellHandle, "config pd fail %d\n", err);
		return kStatus_SHELL_Error;
	}

	return kStatus_SHELL_Success;
}

static shell_status_t config_phy(shell_handle_t shellHandle, int32_t argc, char **argv)
{
	int err;
	int phy;
	
	phy = atoi(argv[1]);

	err = modify_phy(phy);
	if(err)
	{
		SHELL_Printf(s_shellHandle, "config phy fail %d\n", err);
		return kStatus_SHELL_Error;
	}

	return kStatus_SHELL_Success;
}

static shell_status_t config_packing(shell_handle_t shellHandle, int32_t argc, char **argv)
{
	int err;
	int packing;
	
	packing = atoi(argv[1]);

	err = modify_packing(packing);
	if(err)
	{
		SHELL_Printf(s_shellHandle, "config packing fail %d\n", err);
		return kStatus_SHELL_Error;
	}

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

SHELL_COMMAND_DEFINE(wav_open,        "wav_open <path>\r\n",          wav_open,        1);
SHELL_COMMAND_DEFINE(lc3_preset_list, "lc3_preset_list\r\n",          lc3_preset_list, 0);
SHELL_COMMAND_DEFINE(lc3_preset,      "lc3_preset <name>\r\n",        lc3_preset,      1);
SHELL_COMMAND_DEFINE(play,            "play  :resume broadcast.\r\n", play,            0);
SHELL_COMMAND_DEFINE(pause,           "pause :stop broadcast.\r\n",   pause,           0);
SHELL_COMMAND_DEFINE(sync_info,       "sync_info\r\n",                sync_info,       0);
SHELL_COMMAND_DEFINE(config_rtn,      "config_rtn <rtn>\r\n",         config_rtn,      1);
SHELL_COMMAND_DEFINE(config_pd,       "config_pd <pd>\r\n",           config_pd,       1);
SHELL_COMMAND_DEFINE(config_phy,      "config_phy [1,2,4] - 1: 1M, 2: 2M, 4: Coded\r\n", config_phy, 1);
SHELL_COMMAND_DEFINE(config_packing,  "config_packing [0,1] - 0: sequentially, 1: interleaved\r\n", config_packing, 1);
SHELL_COMMAND_DEFINE(set_broadcast_code, "set_broadcast_code [str,hex] [data]\r\n", set_broadcast_code, 2);

void le_audio_shell_init(void)
{
	/* Init SHELL */
	s_shellHandle = &s_shellHandleBuffer[0];
	SHELL_Init(s_shellHandle, g_serialHandle, "BMS>> ");
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(wav_open));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(lc3_preset_list));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(lc3_preset));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(play));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(pause));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(sync_info));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(config_rtn));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(config_pd));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(config_phy));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(config_packing));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(set_broadcast_code));

	SHELL_Printf(s_shellHandle, "\r\nBroadcast Media Sender.\r\n");
}
