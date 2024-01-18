/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _VIRTUAL_NIC_ENETIF_H_
#define _VIRTUAL_NIC_ENETIF_H_

#include "fsl_enet.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if defined(__GNUC__)
#ifndef __ALIGN_END
#define __ALIGN_END __attribute__((aligned(ENET_BUFF_ALIGNMENT)))
#endif
#ifndef __ALIGN_BEGIN
#define __ALIGN_BEGIN
#endif
#else
#ifndef __ALIGN_END
#define __ALIGN_END
#endif
#ifndef __ALIGN_BEGIN
#if defined(__CC_ARM) || (defined(__ARMCC_VERSION))
#define __ALIGN_BEGIN __attribute__((aligned(ENET_BUFF_ALIGNMENT)))
#elif defined(__ICCARM__)
#define __ALIGN_BEGIN
#endif
#endif
#endif
#define ENET_RXBD_NUM (6)
#define ENET_TXBD_NUM (5)
#define ENET_RXBUFF_SIZE (ENET_FRAME_MAX_FRAMELEN)
#define ENET_TXBUFF_SIZE (ENET_FRAME_MAX_FRAMELEN)
//#define ENET_BuffSizeAlign(n) ENET_ALIGN(n, ENET_BUFF_ALIGNMENT)
#define ENET_DATA_LENGTH (1000)
#define ENET_TRANSMIT_DATA_NUM (20)
#define ENET_ALIGN(x, align) ((unsigned int)((x) + ((align)-1)) & (unsigned int)(~(unsigned int)((align)-1)))
#define APP_ENET_BUFF_ALIGNMENT ENET_BUFF_ALIGNMENT

/* @TEST_ANCHOR */

/* MAC address configuration. */
#ifndef configMAC_ADDR0
#define configMAC_ADDR0 0x00
#endif
#ifndef configMAC_ADDR1
#define configMAC_ADDR1 0x12
#endif
#ifndef configMAC_ADDR2
#define configMAC_ADDR2 0x13
#endif
#ifndef configMAC_ADDR3
#define configMAC_ADDR3 0x10
#endif
#ifndef configMAC_ADDR4
#define configMAC_ADDR4 0x15
#endif
#ifndef configMAC_ADDR5
#define configMAC_ADDR5 0x11
#endif

/* Error type */
typedef signed char enet_err_t;
#define ENET_OK (0U)
#define ENET_BUSY (1U)
#define ENET_ERROR (0xffU)
#define ENET_PHY_TIMEOUT (0xFFFFU)

/* MAC address size in bytes. */
#define ENET_MAC_ADDR_SIZE (6U)

#define ETHTYPE_ARP 0x0806U
#define ETHTYPE_IP 0x0800U
#define ETHTYPE_VLAN 0x8100U

/* Packet buffer definition. */
typedef struct _pbuf
{
    uint8_t *payload;
    uint32_t length;
} pbuf_t;

/* Ethernet address type. */
typedef struct _enet_addr
{
    uint8_t addr[ENET_MAC_ADDR_SIZE];
} enet_addr_t;

/* Ethernet header. */
typedef struct _enet_header
{
    enet_addr_t dest;
    enet_addr_t src;
    uint16_t type;
} enet_header_t;

#endif /* _VIRTUAL_NIC_ENETIF_H_ */
