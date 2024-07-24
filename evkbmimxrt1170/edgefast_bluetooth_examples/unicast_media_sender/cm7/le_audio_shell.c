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

#include "le_audio_shell.h"
#include "le_audio_service.h"

shell_handle_t s_shellHandle;
SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
extern serial_handle_t g_serialHandle;

extern int device_scan(void);
extern int device_select(int index);
extern int open_wav_file(char *path);
extern int select_lc3_preset(char *preset_name);
extern void print_all_preset(int sample_rate);
extern int modify_rtn(int rtn);
extern int modify_pd(int pd);
extern int modify_phy(int phy);
extern int modify_packing(int packing);
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

static shell_status_t scan(shell_handle_t shellHandle, int32_t argc, char **argv)
{
	int ret = device_scan();

	if(ret)
	{
		PRINTF("Device scan failed!\n");
		return kStatus_SHELL_Error;
	}

	PRINTF("Scanning successfully started\n");

	return kStatus_SHELL_Success;
}

static shell_status_t connect(shell_handle_t shellHandle, int32_t argc, char **argv)
{
	int index;

	index = atoi(argv[1]);

	if((index == 0) && (0 != strcmp(argv[1], "0")))
	{
		return kStatus_SHELL_Error;
	}

	if(0 != device_select(index))
	{
		return kStatus_SHELL_Error;
	}

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

 	int ret = le_audio_vcs_vol_set(volume);

	if(ret)
	{
		SHELL_Printf(s_shellHandle, "vol set error %d\n", ret);
		return kStatus_SHELL_Error;
	}

	return kStatus_SHELL_Success;
}

static shell_status_t vol_up(shell_handle_t shellHandle, int32_t argc, char **argv)
{
 	int ret = le_audio_vcs_vol_up();

	if(ret)
	{
		SHELL_Printf(s_shellHandle, "vol up error %d\n", ret);
		return kStatus_SHELL_Error;
	}

	return kStatus_SHELL_Success;
}

static shell_status_t vol_down(shell_handle_t shellHandle, int32_t argc, char **argv)
{
 	int ret = le_audio_vcs_vol_down();

	if(ret)
	{
		SHELL_Printf(s_shellHandle, "vol down error %d\n", ret);
		return kStatus_SHELL_Error;
	}

	return kStatus_SHELL_Success;
}

static shell_status_t vol_mute(shell_handle_t shellHandle, int32_t argc, char **argv)
{
 	int ret = le_audio_vcs_vol_mute();

	if(ret)
	{
		SHELL_Printf(s_shellHandle, "vol mute error %d\n", ret);
		return kStatus_SHELL_Error;
	}

	return kStatus_SHELL_Success;
}

static shell_status_t vol_unmute(shell_handle_t shellHandle, int32_t argc, char **argv)
{
 	int ret = le_audio_vcs_vol_unmute();

	if(ret)
	{
		SHELL_Printf(s_shellHandle, "vol unmute error %d\n", ret);
		return kStatus_SHELL_Error;
	}

	return kStatus_SHELL_Success;
}

extern void le_audio_bis_play(void);
extern void le_audio_bis_pause(void);

static shell_status_t play(shell_handle_t shellHandle, int32_t argc, char **argv)
{
 	int ret = le_audio_mcs_play();

	if(ret)
	{
		SHELL_Printf(s_shellHandle, "mcs play error %d\n", ret);
		return kStatus_SHELL_Error;
	}

	return kStatus_SHELL_Success;
}

static shell_status_t pause(shell_handle_t shellHandle, int32_t argc, char **argv)
{
 	int ret = le_audio_mcs_pause();

	if(ret)
	{
		SHELL_Printf(s_shellHandle, "mcs pause error %d\n", ret);
		return kStatus_SHELL_Error;
	}

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

SHELL_COMMAND_DEFINE(wav_open,        "wav_open <path>\r\n",   wav_open,        1);
SHELL_COMMAND_DEFINE(lc3_preset_list, "lc3_preset_list\r\n",   lc3_preset_list, 0);
SHELL_COMMAND_DEFINE(lc3_preset,      "lc3_preset <name>\r\n", lc3_preset,      1);
SHELL_COMMAND_DEFINE(scan,            "scan\r\n",              scan,            0);
SHELL_COMMAND_DEFINE(connect,         "connect [index]\r\n",   connect,         1);
SHELL_COMMAND_DEFINE(vol_set,         "vol_set [0-255]\r\n",   vol_set,         1);
SHELL_COMMAND_DEFINE(vol_up,          "vol_up\r\n",            vol_up,          0);
SHELL_COMMAND_DEFINE(vol_down,        "vol_down\r\n",          vol_down,        0);
SHELL_COMMAND_DEFINE(vol_mute,        "vol_mute\r\n",          vol_mute,        0);
SHELL_COMMAND_DEFINE(vol_unmute,      "vol_unmute\r\n",        vol_unmute,      0);
SHELL_COMMAND_DEFINE(play,            "play\r\n",              play,            0);
SHELL_COMMAND_DEFINE(pause,           "pause\r\n",             pause,           0);
SHELL_COMMAND_DEFINE(sync_info,       "sync_info\r\n",         sync_info,       0);
SHELL_COMMAND_DEFINE(config_rtn,      "config_rtn <rtn>\r\n",  config_rtn,      1);
SHELL_COMMAND_DEFINE(config_pd,       "config_pd <pd>\r\n",    config_pd,       1);
SHELL_COMMAND_DEFINE(config_phy,      "config_phy [1,2,4] - 1: 1M, 2: 2M, 4: Coded\r\n", config_phy, 1);
SHELL_COMMAND_DEFINE(config_packing,  "config_packing [0,1] - 0: sequentially, 1: interleaved\r\n", config_packing, 1);


void le_audio_shell_init(void)
{
	/* Init SHELL */
	s_shellHandle = &s_shellHandleBuffer[0];
	SHELL_Init(s_shellHandle, g_serialHandle, "UMS>> ");

	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(wav_open));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(lc3_preset_list));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(lc3_preset));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(scan));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(connect));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(vol_set));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(vol_up));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(vol_down));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(vol_mute));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(vol_unmute));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(play));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(pause));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(sync_info));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(config_rtn));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(config_pd));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(config_phy));
	SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(config_packing));

	SHELL_Printf(s_shellHandle, "\r\nUnicast Media Sender.\r\n");
}
