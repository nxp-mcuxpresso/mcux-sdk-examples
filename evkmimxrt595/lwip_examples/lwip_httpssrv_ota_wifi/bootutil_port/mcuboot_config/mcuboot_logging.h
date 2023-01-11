/*
 * Copyright (c) 2018 Runtime Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MCUBOOT_LOGGING_H__
#define __MCUBOOT_LOGGING_H__

#include "fsl_debug_console.h"
///*
// * When building for targets running Zephyr, delegate to its native
// * logging subsystem.
// */

#define MCUBOOT_LOG_MODULE_DECLARE(domain)  // LOG_MODULE_DECLARE(domain, CONFIG_MCUBOOT_LOG_LEVEL)
#define MCUBOOT_LOG_MODULE_REGISTER(domain) // LOG_MODULE_REGISTER(domain, CONFIG_MCUBOOT_LOG_LEVEL)

#define MCUBOOT_LOG_ERR(...)            \
    {                                   \
        DbgConsole_Printf(__VA_ARGS__); \
        DbgConsole_Printf("\r\n");      \
    }
#define MCUBOOT_LOG_WRN(...)            \
    {                                   \
        DbgConsole_Printf(__VA_ARGS__); \
        DbgConsole_Printf("\r\n");      \
    }
#define MCUBOOT_LOG_INF(...)            \
    {                                   \
        DbgConsole_Printf(__VA_ARGS__); \
        DbgConsole_Printf("\r\n");      \
    }
#define MCUBOOT_LOG_DBG(...)            \
    {                                   \
        DbgConsole_Printf(__VA_ARGS__); \
        DbgConsole_Printf("\r\n");      \
    }

#endif /* __MCUBOOT_LOGGING_H__ */
