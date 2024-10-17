/* @file lwippools.h
 *
 *  @brief This file contains custom LwIP memory pool definitions
 *
 *  Copyright 2020-2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *  The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

#ifndef __LWIPPOOLS_H__
#define __LWIPPOOLS_H__

#ifndef LWIP_HOOK_FILENAME
#define LWIP_HOOK_FILENAME                               "lwiphooks.h"
#define LWIP_HOOK_TCP_OUT_ADD_TCPOPTS(p, hdr, pcb, opts) lwip_hook_tcp_out_add_tcpopts(p, hdr, pcb, opts)
#endif

#endif /* __LWIPPOOLS_H__ */
