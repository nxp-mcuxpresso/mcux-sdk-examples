/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CMD_H_
#define _CMD_H_

/*${header:start}*/
#include "main.h"
#ifdef OPUS_ENCODE
#include "opusmem2mem_file.h"
#endif
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
/*${prototype:end}*/

/** Constants **/
#ifdef OPUS_ENCODE
#define OPUSMEM2MEM_INBUF_SIZE  96000
#define OPUSMEM2MEM_OUTBUF_SIZE 32960
#endif

#endif
