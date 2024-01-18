/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __BLE_AUDIO_CONFIG_H__
#define __BLE_AUDIO_CONFIG_H__

/*!
 * @brief Bluetooth Audio Configuration
 * @defgroup bt_audio_config Bluetooth Audio Configuration
 * @{
 */

#if (defined(CONFIG_BT_AUDIO) && (CONFIG_BT_AUDIO > 0))

/* This hidden option is enabled when any of the profiles/services
 * enables support for receiving of audio data.
 */
#ifndef CONFIG_BT_AUDIO_RX
#define CONFIG_BT_AUDIO_RX 0
#endif

/* This hidden option is enabled when any of the profiles/services
 * enables support for transmitting of audio data.
 */
#ifndef CONFIG_BT_AUDIO_TX
#define CONFIG_BT_AUDIO_TX 0
#endif

/* This option sets the time in 1.25 ms units before the stack will
 * retry to send notification that failed due to lack of TX buffers
 * available.
 * 
 * Delay for notification sending retried attempt in 1.25 ms units
 * 
 * range 6 ~ 3200
 */
#ifndef CONFIG_BT_AUDIO_NOTIFY_RETRY_DELAY
#define CONFIG_BT_AUDIO_NOTIFY_RETRY_DELAY 20
#endif

/* This hidden option is enabled when any of the content control
 * features are enabled.
 */
#ifndef CONFIG_BT_CCID
#define CONFIG_BT_CCID 0
#endif


#if (defined(CONFIG_BT_BAP_UNICAST) && (CONFIG_BT_BAP_UNICAST > 0)) || \
    (defined(CONFIG_BT_BAP_UNICAST_SERVER) && (CONFIG_BT_BAP_UNICAST_SERVER > 0)) || \
    (defined(CONFIG_BT_BAP_UNICAST_CLIENT) && (CONFIG_BT_BAP_UNICAST_CLIENT > 0)) || \
    (defined(CONFIG_BT_BAP_BROADCAST_SOURCE) && (CONFIG_BT_BAP_BROADCAST_SOURCE > 0)) || \
    (defined(CONFIG_BT_BAP_BROADCAST_SINK) && (CONFIG_BT_BAP_BROADCAST_SINK > 0)) || \
    (defined(CONFIG_BT_BAP_SCAN_DELEGATOR) && (CONFIG_BT_BAP_SCAN_DELEGATOR > 0)) || \
    (defined(CONFIG_BT_BAP_BROADCAST_ASSISTANT) && (CONFIG_BT_BAP_BROADCAST_ASSISTANT > 0))
#include "config_bap.h"
#endif

#if (defined(CONFIG_BT_VOCS) && (CONFIG_BT_VOCS > 0)) || \
    (defined(CONFIG_BT_VOCS_MAX_INSTANCE_COUNT) && (CONFIG_BT_VOCS_MAX_INSTANCE_COUNT > 0)) || \
    (defined(CONFIG_BT_VOCS_CLIENT) && (CONFIG_BT_VOCS_CLIENT > 0)) || \
    (defined(CONFIG_BT_VOCS_CLIENT_MAX_INSTANCE_COUNT) && (CONFIG_BT_VOCS_CLIENT_MAX_INSTANCE_COUNT > 0))
#include "config_vocs.h"
#endif

#if (defined(CONFIG_BT_AICS) && (CONFIG_BT_AICS > 0)) || \
    (defined(CONFIG_BT_AICS_MAX_INSTANCE_COUNT) && (CONFIG_BT_AICS_MAX_INSTANCE_COUNT > 0)) || \
    (defined(CONFIG_BT_AICS_CLIENT) && (CONFIG_BT_AICS_CLIENT > 0)) || \
    (defined(CONFIG_BT_AICS_CLIENT_MAX_INSTANCE_COUNT) && (CONFIG_BT_AICS_CLIENT_MAX_INSTANCE_COUNT > 0))
#include "config_aics.h"
#endif

#if (defined(CONFIG_BT_VCP_VOL_REND) && (CONFIG_BT_VCP_VOL_REND > 0)) || \
    (defined(CONFIG_BT_VCP_VOL_CTLR) && (CONFIG_BT_VCP_VOL_CTLR > 0))
#include "config_vcp.h"
#endif

#if (defined(CONFIG_BT_MICP_MIC_DEV) && (CONFIG_BT_MICP_MIC_DEV > 0)) || \
    (defined(CONFIG_BT_MICP_MIC_CTLR) && (CONFIG_BT_MICP_MIC_CTLR > 0))
#include "config_micp.h"
#endif

#if (defined(CONFIG_BT_CSIP_SET_MEMBER) && (CONFIG_BT_CSIP_SET_MEMBER > 0)) || \
    (defined(CONFIG_BT_CSIP_SET_COORDINATOR) && (CONFIG_BT_CSIP_SET_COORDINATOR > 0))
#include "config_csip.h"
#endif

#if (defined(CONFIG_BT_TBS) && (CONFIG_BT_TBS > 0)) || \
    (defined(CONFIG_BT_TBS_CLIENT_GTBS) && (CONFIG_BT_TBS_CLIENT_GTBS > 0)) || \
    (defined(CONFIG_BT_TBS_CLIENT_TBS) && (CONFIG_BT_TBS_CLIENT_TBS > 0))
#include "config_tbs.h"
#endif

#if (defined(CONFIG_BT_MCS) && (CONFIG_BT_MCS > 0)) || \
    (defined(CONFIG_BT_MCC) && (CONFIG_BT_MCC > 0)) || \
    (defined(CONFIG_BT_MCC_OTS) && (CONFIG_BT_MCC_OTS > 0))
#include "config_mcs.h"
#endif

#if (defined(CONFIG_BT_HAS) && (CONFIG_BT_HAS > 0)) || \
    (defined(CONFIG_BT_HAS_CLIENT) && (CONFIG_BT_HAS_CLIENT > 0))
#include "config_has.h"
#endif

#if (defined(CONFIG_BT_MPL) && (CONFIG_BT_MPL > 0))
#include "config_mpl.h"
#endif

#if (defined(CONFIG_MCTL) && (CONFIG_MCTL > 0))
#include "config_mctl.h"
#endif

#if (defined(CONFIG_BT_OTS) && (CONFIG_BT_OTS > 0)) || \
    (defined(CONFIG_BT_OTS_CLIENT) && (CONFIG_BT_OTS_CLIENT > 0))
#include "config_ots.h"
#endif

#if (defined(CONFIG_BT_CAP_ACCEPTOR) && (CONFIG_BT_CAP_ACCEPTOR > 0)) || \
    (defined(CONFIG_BT_CAP_INITIATOR) && (CONFIG_BT_CAP_INITIATOR > 0))
#include "config_cap.h"
#endif

#if (defined(CONFIG_BT_TMAP) && (CONFIG_BT_TMAP > 0))
#include "config_tmap.h"
#endif

#endif /* CONFIG_BT_AUDIO */

/*! @}*/

#endif /* __BLE_AUDIO_CONFIG_H__ */