/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */
#ifndef _NCP_SYSTEM_H_
#define _NCP_SYSTEM_H_

int system_ncp_init(void);
int system_ncp_send_response(uint8_t *pbuf);

#endif /* _NCP_SYSTEM_H_ */
