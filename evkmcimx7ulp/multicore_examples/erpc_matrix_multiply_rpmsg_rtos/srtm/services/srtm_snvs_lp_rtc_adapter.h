/*
 * Copyright 2017, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SRTM_SNVS_LP_RTC_ADAPTER_H__
#define __SRTM_SNVS_LP_RTC_ADAPTER_H__

#include "srtm_rtc_service.h"
#include "fsl_snvs_lp.h"

/*!
 * @addtogroup srtm_service
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Create SNVS_LP RTC adapter.
 *
 * @param base SNVS base address.
 * @return SRTM RTC adapter on success or NULL on failure.
 */
srtm_rtc_adapter_t SRTM_SnvsLpRtcAdapter_Create(SNVS_Type *base);

/*!
 * @brief Destroy SNVS_LP RTC adapter.
 *
 * @param adapter SNVS_LP RTC adapter to destroy.
 */
void SRTM_SnvsLpRtcAdapter_Destroy(srtm_rtc_adapter_t adapter);

/*!
 * @brief Notify alarm event.
 *
 * @param adapter SNVS_LP RTC adapter to destroy.
 */
void SRTM_SnvsLpRtcAdapter_NotifyAlarm(srtm_rtc_adapter_t adapter);

#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* __SRTM_SNVS_LP_RTC_ADAPTER_H__ */
