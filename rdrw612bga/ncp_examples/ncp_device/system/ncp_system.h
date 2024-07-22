/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _NCP_SYSTEM_H_
#define _NCP_SYSTEM_H_

int system_ncp_init(void);
int system_ncp_send_response(uint8_t *pbuf);

#endif /* _NCP_SYSTEM_H_ */
