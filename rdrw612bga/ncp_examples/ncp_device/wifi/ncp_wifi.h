/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _NCP_WIFI_H_
#define _NCP_WIFI_H_


int wifi_ncp_init(void);

/*If nvm is enabled, set lfs network to Wi-Fi driver.*/
int ncp_wifi_set_nvm_network(void);

#endif /* _NCP_WIFI_H_ */