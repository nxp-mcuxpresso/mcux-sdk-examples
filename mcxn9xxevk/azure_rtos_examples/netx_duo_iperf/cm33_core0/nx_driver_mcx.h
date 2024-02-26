/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _NX_DRIVER_MCX_ENET_H
#define _NX_DRIVER_MCX_ENET_H

#ifdef __cplusplus
extern   "C" {
#endif

#include "fsl_enet.h"

#include "nx_api.h"

/* Driver return values. */
#define NX_DRIVER_ERROR                         0x100
#define NX_DRIVER_ERROR_NOT_INITIALIZED         0x101
#define NX_DRIVER_ERROR_INITIALIZE_FAILED       0x102

/* Ethernet header type values */
#define NX_DRIVER_ETHERNET_IP                   0x0800
#define NX_DRIVER_ETHERNET_IPV6                 0x86dd
#define NX_DRIVER_ETHERNET_ARP                  0x0806
#define NX_DRIVER_ETHERNET_RARP                 0x8035

#define NX_DRIVER_ETHERNET_ADDR_SIZE            (6)
#define NX_DRIVER_ETHERNET_HEADER_PAD           (2)
#define NX_DRIVER_ETHERNET_HEADER_SIZE          (14)
#define NX_DRIVER_IP_MTU                                    \
            (ENET_FRAME_MAX_FRAMELEN - NX_DRIVER_ETHERNET_HEADER_SIZE - ENET_FCS_LEN)

struct eth_header {
    uint8_t dest[NX_DRIVER_ETHERNET_ADDR_SIZE];
    uint8_t source[NX_DRIVER_ETHERNET_ADDR_SIZE];
    uint16_t proto;
} __PACKED;

VOID nx_link_driver(NX_IP_DRIVER *driver_req_ptr);

#ifdef __cplusplus
    }
#endif

#endif
