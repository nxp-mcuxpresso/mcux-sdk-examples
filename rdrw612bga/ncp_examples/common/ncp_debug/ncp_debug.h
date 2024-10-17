/*
 * Copyright 2022-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

#ifndef __NCP_DEBUG_H__
#define __NCP_DEBUG_H__

#include <assert.h>
#include "fsl_debug_console.h"

/** Dump buffer in hex format on console
 *
 * This function prints the received buffer in HEX format on the console
 *
 * \param[in] data Pointer to the data buffer
 * \param[in] len Length of the data
 */
static inline void ncp_dump_hex(const void *data, unsigned len)
{
    (void)PRINTF("**** Dump @ %p Len: %d ****\n\r", data, len);

    unsigned int i    = 0;
    const char *data8 = (const char *)data;
    while (i < len)
    {
        (void)PRINTF("%02x ", data8[i++]);
        if (!(i % 16))
        {
            (void)PRINTF("\n\r");
        }
    }

    (void)PRINTF("\n\r******** End Dump *******\n\r");
}

#define ncplog_e(_mod_name_, _fmt_, ...)  (void)PRINTF("[%s]%s" _fmt_ "\n\r", _mod_name_, " Error: ", ##__VA_ARGS__)
#define ncplog_w(_mod_name_, _fmt_, ...)  (void)PRINTF("[%s]%s" _fmt_ "\n\r", _mod_name_, " Warn: ", ##__VA_ARGS__)
#define ncplog(_mod_name_, _fmt_, ...)    (void)PRINTF("[%s] " _fmt_ "\n\r", _mod_name_, ##__VA_ARGS__)

#define ncp_e(...) ncplog_e("NCP", ##__VA_ARGS__)
#define ncp_w(...) ncplog_w("NCP", ##__VA_ARGS__)
#if CONFIG_NCP_DEBUG
#define ncp_d(...) ncplog("NCP", ##__VA_ARGS__)
#else
#define ncp_d(...)
#endif

#define NCP_ASSERT(test) assert(test)

#if CONFIG_NCP_DEBUG
#define CONFIG_NCP_DEBUG_ADAP 1
#endif

#define NCP_SUCCESS 0
#define NCP_FAIL    1

#endif /* __NCP_DEBUG_H__ */
