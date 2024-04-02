/*
 * Copyright 2022-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __NCP_DEBUG_H__
#define __NCP_DEBUG_H__

#include <assert.h>
#include "fsl_debug_console.h"

#define ncplog_e(_mod_name_, _fmt_, ...)  (void)PRINTF("[%s]%s" _fmt_ "\n\r", _mod_name_, " Error: ", ##__VA_ARGS__)
#define ncplog_w(_mod_name_, _fmt_, ...)  (void)PRINTF("[%s]%s" _fmt_ "\n\r", _mod_name_, " Warn: ", ##__VA_ARGS__)
#define ncplog(_mod_name_, _fmt_, ...)    (void)PRINTF("[%s] " _fmt_ "\n\r", _mod_name_, ##__VA_ARGS__)

#define ncp_e(...) ncplog_e("NCP", ##__VA_ARGS__)
#define ncp_w(...) ncplog_w("NCP", ##__VA_ARGS__)
#ifdef CONFIG_NCP_DEBUG
#define ncp_d(...) ncplog("NCP", ##__VA_ARGS__)
#else
#define ncp_d(...)
#endif

#define NCP_ASSERT(test) assert(test)

#ifdef CONFIG_NCP_DEBUG
#define CONFIG_NCP_DEBUG_ADAP
#endif

#define NCP_SUCCESS 0
#define NCP_FAIL    1

#endif /* __NCP_DEBUG_H__ */
