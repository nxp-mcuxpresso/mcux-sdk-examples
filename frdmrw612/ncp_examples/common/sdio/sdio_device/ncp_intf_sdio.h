/*
 * Copyright 2022-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

#ifndef __NCP_INTF_SDIO_H__
#define __NCP_INTF_SDIO_H__

#include "ncp_tlv_adapter.h"

int ncp_sdio_init(void *argv);
int ncp_sdio_deinit(void *argv);
int ncp_sdio_send(uint8_t *tlv_buf, size_t tlv_sz, tlv_send_callback_t cb);
//int ncp_sdio_recv(uint8_t *tlv_buf, size_t *tlv_sz);

#endif /* __NCP_INTF_SDIO_H__ */