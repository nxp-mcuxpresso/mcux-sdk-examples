/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _VIRTUAL_NIC_NETCIF_H_
#define _VIRTUAL_NIC_NETCIF_H_

#include "fsl_netc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if defined(__GNUC__)
#ifndef __ALIGN_END
#define __ALIGN_END __attribute__((aligned(NETC_BUFF_ALIGNMENT)))
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
#define __ALIGN_BEGIN __attribute__((aligned(NETC_BUFF_ALIGNMENT)))
#elif defined(__ICCARM__)
#define __ALIGN_BEGIN
#endif
#endif
#endif
#define NETC_RXBD_NUM    (8)
#define NETC_TXBD_NUM    (8)
#define NETC_RXBUFF_SIZE (1518U)
#define NETC_TXBUFF_SIZE (1518U)
//#define NETC_BuffSizeAlign(n) NETC_ALIGN(n, NETC_BUFF_ALIGNMENT)
#define NETC_DATA_LENGTH       (1000)
#define NETC_TRANSMIT_DATA_NUM (20)
#define NETC_ALIGN(x, align)   ((unsigned int)((x) + ((align)-1)) & (unsigned int)(~(unsigned int)((align)-1)))
#define NETC_EP_BD_ALIGN       (128U)
#define NETC_BUFF_ALIGNMENT    (64U)
/* @TEST_ANCHOR */

/* MAC address configuration. */
#ifndef configMAC_ADDR0
#define configMAC_ADDR0 0x54
#endif
#ifndef configMAC_ADDR1
#define configMAC_ADDR1 0x27
#endif
#ifndef configMAC_ADDR2
#define configMAC_ADDR2 0x8d
#endif
#ifndef configMAC_ADDR3
#define configMAC_ADDR3 0x00
#endif
#ifndef configMAC_ADDR4
#define configMAC_ADDR4 0x00
#endif
#ifndef configMAC_ADDR5
#define configMAC_ADDR5 0x00
#endif

/* Error type */
typedef signed char enet_err_t;
#define ENET_OK          (0U)
#define ENET_BUSY        (1U)
#define ENET_ERROR       (0xffU)
#define ENET_PHY_TIMEOUT (0xFFFFU)

/* MAC address size in bytes. */
#define NETC_MAC_ADDR_SIZE (6U)
#define ENET_MAC_ADDR_SIZE NETC_MAC_ADDR_SIZE

#define ETHTYPE_ARP  0x0806U
#define ETHTYPE_IP   0x0800U
#define ETHTYPE_VLAN 0x8100U

#define EXAMPLE_TX_RX_INTERRUPT_HANDLE

#if defined(__GIC_PRIO_BITS)
#define ENET_INTERRUPT_PRIORITY (25U)
#elif defined(__NVIC_PRIO_BITS) && (__NVIC_PRIO_BITS >= 3)
#define ENET_INTERRUPT_PRIORITY (6U)
#else
#define ENET_INTERRUPT_PRIORITY (3U)
#endif

/* Packet buffer definition. */
typedef struct _pbuf
{
    uint8_t *payload;
    uint32_t length;
} pbuf_t;

/* Ethernet address type. */
typedef struct _netc_addr
{
    uint8_t addr[NETC_MAC_ADDR_SIZE];
} netc_addr_t;

/* Ethernet header. */
typedef struct _enet_header
{
    netc_addr_t dest;
    netc_addr_t src;
    uint16_t type;
} netc_header_t;

#endif /* _VIRTUAL_NIC_NETCIF_H_ */
