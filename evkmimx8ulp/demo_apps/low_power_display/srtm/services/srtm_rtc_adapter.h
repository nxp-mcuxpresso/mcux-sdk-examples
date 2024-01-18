/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SRTM_RTC_ADAPTER_H__
#define __SRTM_RTC_ADAPTER_H__

#include "srtm_rtc_service.h"
#include "fsl_adapter_rtc.h"
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
 * @brief Create RTC adapter.
 *
 * @return SRTM RTC adapter on success or NULL on failure.
 */
srtm_rtc_adapter_t SRTM_RtcAdapter_Create(hal_rtc_handle_t halRtcHandle);

/*!
 * @brief Destroy RTC adapter.
 *
 * @param adapter RTC adapter to destroy.
 */
void SRTM_RtcAdapter_Destroy(srtm_rtc_adapter_t adapter);

/*!
 * @brief Notify alarm event.
 *
 * @param adapter RTC adapter to be used.
 */
void SRTM_RtcAdapter_NotifyAlarm(srtm_rtc_adapter_t adapter);

/*!
 * @brief Disable rtc alarm interupt.
 *
 * @param adapter RTC adapter to be used.
 */
void SRTM_RtcAdapter_DisableAlarmInt(srtm_rtc_adapter_t adapter);
#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* __SRTM_RTC_ADAPTER_H__ */
