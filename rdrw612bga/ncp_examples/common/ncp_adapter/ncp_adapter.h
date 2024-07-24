/*
 * Copyright 2022-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __NCP_ADAPTER_H__
#define __NCP_ADAPTER_H__

#include "ncp_debug.h"
#include "ncp_intf_pm.h"

#ifdef __GNUC__
/** Structure packing begins */
#define NCP_TLV_PACK_START
/** Structure packeing end */
#define NCP_TLV_PACK_END __attribute__((packed))
#else /* !__GNUC__ */
#ifdef PRAGMA_PACK
/** Structure packing begins */
#define NCP_TLV_PACK_START
/** Structure packeing end */
#define NCP_TLV_PACK_END
#else /* !PRAGMA_PACK */
/** Structure packing begins */
#define NCP_TLV_PACK_START __packed
/** Structure packing end */
#define NCP_TLV_PACK_END
#endif /* PRAGMA_PACK */
#endif /* __GNUC__ */

#define ncp_adap_e(...) ncplog_e("NCP Adap", ##__VA_ARGS__)
#define ncp_adap_w(...) ncplog_w("NCP Adap", ##__VA_ARGS__)
#if CONFIG_NCP_DEBUG_ADAP
#define ncp_adap_d(...) ncplog("NCP Adap", ##__VA_ARGS__)
#else
#define ncp_adap_d(...)
#endif

/* NCP status */
typedef enum _ncp_status
{
    NCP_STATUS_ERROR      = -1,
    NCP_STATUS_CHKSUMERR  = -2,
    NCP_STATUS_NOMEM      = -3,
    NCP_STATUS_QUEUE_FULL = -4,
    NCP_STATUS_SUCCESS    = 0,
} ncp_status_t;

ncp_status_t ncp_adapter_init(void);
ncp_status_t ncp_adapter_deinit(void);
void ncp_tlv_install_handler(uint8_t class, void *func_cb);
void ncp_tlv_uninstall_handler(uint8_t class);

#endif /* __NCP_ADAPTER_H__ */