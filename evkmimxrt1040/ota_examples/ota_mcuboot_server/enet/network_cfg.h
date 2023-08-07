/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _NETWORK_CFG_H_
#define _NETWORK_CFG_H_

/*${header:start}*/
#include "fsl_phyksz8081.h"
/*${header:end}*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/

/* @TEST_ANCHOR */

/* IP address configuration. */
#ifndef IP_ADDR
#define IP_ADDR "192.168.0.102"
#endif

/* Netmask configuration. */
#ifndef IP_MASK
#define IP_MASK "255.255.255.0"
#endif

/* Gateway address configuration. */
#ifndef GW_ADDR
#define GW_ADDR "192.168.0.100"
#endif

/* Network interface initialization function. */
#ifndef EXAMPLE_NETIF_INIT_FN
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init
#endif

/* Ethernet configuration. */
extern phy_ksz8081_resource_t g_phy_resource;
#define EXAMPLE_ENET         ENET
#define EXAMPLE_PHY_ADDRESS  0x02U
#define EXAMPLE_PHY_OPS      &phyksz8081_ops
#define EXAMPLE_PHY_RESOURCE &g_phy_resource
#define EXAMPLE_CLOCK_FREQ   CLOCK_GetFreq(kCLOCK_IpgClk)
/*${macro:end}*/

#endif /* _NETWORK_CFG_H_ */
