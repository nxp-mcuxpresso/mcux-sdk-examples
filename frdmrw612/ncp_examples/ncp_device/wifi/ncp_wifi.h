/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */
#ifndef _NCP_WIFI_H_
#define _NCP_WIFI_H_


int wifi_ncp_init(void);

/*If nvm is enabled, set lfs network to Wi-Fi driver.*/
int ncp_wifi_set_nvm_network(void);

#endif /* _NCP_WIFI_H_ */