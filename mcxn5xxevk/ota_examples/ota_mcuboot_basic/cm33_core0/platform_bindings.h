/*
 * Copyright 2016-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_BINDINGS_H__
#define __PLATFORM_BINDINGS_H__

#include "fsl_common.h"
#include "tinycrypt/sha256.h"
#include "fsl_component_serial_manager.h"


typedef struct tc_sha256_state_struct hashctx_t;

status_t sha256_init(hashctx_t *ctx);
status_t sha256_update(hashctx_t *ctx, const void *data, size_t size);
status_t sha256_finish(hashctx_t *ctx, void *output);

int xmodem_putc(int);
int xmodem_getc(void);
int xmodem_canread(void);
#define xmodem_canread_retries (SystemCoreClock/16)

#endif /*__PLATFORM_BINDINGS_H__*/
