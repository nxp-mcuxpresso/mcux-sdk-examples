/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "lwip/netif.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief Configure and enable MDNS service.
 *
 * @param netif          netif on which mdns service shall be enabled
 * @param mdns_hostname  mdns hostaname string
 */
void http_server_enable_mdns(struct netif *netif, const char *mdns_hostname);

/*!
 * @brief Initializes server.
 */
void http_server_socket_init(void);

/*!
 * @brief Prints IP configuration.
 *
 * @param netif  netif whose cfg. shall be printed
 */
void http_server_print_ip_cfg(struct netif *netif);

#if LWIP_IPV6
/*!
 * @brief Prints valid IPv6 addresses.
 *
 * @param netif  netif whose addresses shall be printed
 */
void http_server_print_ipv6_addresses(struct netif *netif);
#endif /* LWIP_IPV6 */

/*******************************************************************************
 * Variables
 ******************************************************************************/
