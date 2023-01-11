/*
 * Copyright 2022 NXP
 * All rights reserved.
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

/*******************************************************************************
 * Variables
 ******************************************************************************/
