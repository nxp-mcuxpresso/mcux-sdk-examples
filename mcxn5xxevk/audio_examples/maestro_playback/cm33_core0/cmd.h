/*
 * Copyright 2019-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CMD_H_
#define _CMD_H_

/*${header:start}*/
#include "main.h"
/*${header:end}*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*${macro:start}*/
typedef void handleShellMessageCallback_t(void *arg);
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*!
 * @brief Common function for getting user input using shell console.
 *
 * @param[in] handleShellMessageCallback Callback to function which should
 * handle serialized message.
 * @param[in] arg Data to pass to callback handler.
 */
void shellCmd(void);

void cmdList();
void cmdPlay(int32_t argc, char **argv);
void cmdPause();
void cmdVolume(int32_t argc, char **argv);
void cmdSeek(int32_t argc, char **argv);
void cmdStop();
/*${prototype:end}*/

#endif
