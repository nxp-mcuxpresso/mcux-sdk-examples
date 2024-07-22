/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


/*******************************************************************************
 * Includes
 ******************************************************************************/
/*${header:start}*/
#include "board.h"

#include "ethernetif.h"
#include "lwip/netifapi.h"

#include "network_cfg.h"

#include "fsl_silicon_id.h"
/*${header:end}*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
#define INIT_SUCCESS 0
#define INIT_FAIL    1

/* @TEST_ANCHOR */

#ifndef EXAMPLE_BANNER
#define EXAMPLE_BANNER "ROM OTA HTTP Server Example"
#endif
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
/*${prototype:end}*/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*${variable:start}*/
static phy_handle_t phyHandle;

static struct netif netif;

/*${variable:end}*/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*${function:start}*/
int initNetwork(void)
{
    ip4_addr_t netif_ipaddr, netif_netmask, netif_gw;
    ethernetif_config_t enet_config = {
        .phyHandle   = &phyHandle,
        .phyAddr     = EXAMPLE_PHY_ADDRESS,
        .phyOps      = EXAMPLE_PHY_OPS,
        .phyResource = EXAMPLE_PHY_RESOURCE,
        .srcClockHz  = EXAMPLE_CLOCK_FREQ,
#ifdef configMAC_ADDR
        .macAddress = configMAC_ADDR,
#endif
    };

#ifndef configMAC_ADDR
    /* Set special address for each chip. */
    (void)SILICONID_ConvertToMacAddr(&enet_config.macAddress);
#endif

    ip4addr_aton(IP_ADDR, &netif_ipaddr);
    ip4addr_aton(IP_MASK, &netif_netmask);
    ip4addr_aton(GW_ADDR, &netif_gw);

    tcpip_init(NULL, NULL);

    netifapi_netif_add(&netif, &netif_ipaddr, &netif_netmask, &netif_gw, &enet_config, EXAMPLE_NETIF_INIT_FN,
                       tcpip_input);
    netifapi_netif_set_default(&netif);
    netifapi_netif_set_up(&netif);

    while (ethernetif_wait_linkup(&netif, 5000) != ERR_OK)
    {
        PRINTF("PHY Auto-negotiation failed. Please check the cable connection and link partner setting.\r\n");
    }

    LWIP_PLATFORM_DIAG(("\r\n************************************************\r\n"));
    LWIP_PLATFORM_DIAG((EXAMPLE_BANNER"\r\n"));
    LWIP_PLATFORM_DIAG(("Built " __DATE__ " " __TIME__ "\r\n"));
    LWIP_PLATFORM_DIAG(("************************************************\r\n"));
    LWIP_PLATFORM_DIAG((" IPv4 Address     : %u.%u.%u.%u\r\n", ((u8_t *)&netif_ipaddr)[0], ((u8_t *)&netif_ipaddr)[1],
                        ((u8_t *)&netif_ipaddr)[2], ((u8_t *)&netif_ipaddr)[3]));
    LWIP_PLATFORM_DIAG((" IPv4 Subnet mask : %u.%u.%u.%u\r\n", ((u8_t *)&netif_netmask)[0], ((u8_t *)&netif_netmask)[1],
                        ((u8_t *)&netif_netmask)[2], ((u8_t *)&netif_netmask)[3]));
    LWIP_PLATFORM_DIAG((" IPv4 Gateway     : %u.%u.%u.%u\r\n", ((u8_t *)&netif_gw)[0], ((u8_t *)&netif_gw)[1],
                        ((u8_t *)&netif_gw)[2], ((u8_t *)&netif_gw)[3]));
    LWIP_PLATFORM_DIAG(("************************************************\r\n"));

    return INIT_SUCCESS;
}
/*${function:end}*/
