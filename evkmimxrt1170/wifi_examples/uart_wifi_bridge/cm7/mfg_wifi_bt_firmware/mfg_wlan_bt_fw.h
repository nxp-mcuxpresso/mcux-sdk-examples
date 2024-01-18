/*
 *  Copyright 2021 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __MFG_WLAN_BT_FW_H__
#define __MFG_WLAN_BT_FW_H__

#if defined(SD8801)
#include "sd8801_mfg_wlan.h"
#elif defined(SD8978)
#include "sduartIW416_mfg_wlan_bt.h"
#elif defined(SD8987)
#include "sduart8987_mfg_wlan_bt.h"
#elif defined(SD9177)
#include "sduart_nw61x_mfg_se.h"
#endif

#endif /* __MFG_WLAN_BT_FW_H__ */
