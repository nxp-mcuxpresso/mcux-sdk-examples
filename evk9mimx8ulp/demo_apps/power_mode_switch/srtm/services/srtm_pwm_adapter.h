/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SRTM_PWM_ADAPTER_H__
#define __SRTM_PWM_ADAPTER_H__

#include "srtm_pwm_service.h"
#include "fsl_adapter_pwm.h"

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
 * @brief Create PWM adapter.
 *
 */
srtm_pwm_adapter_t SRTM_PwmAdapter_Create(hal_pwm_handle_t *handles, uint32_t handleNum);

/*!
 * @brief Destroy PWM adapter.
 *
 * @param adapter PWM adapter to destroy.
 */
void SRTM_PwmAdapter_Destroy(srtm_pwm_adapter_t adapter);

#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* __SRTM_SNVS_LP_RTC_ADAPTER_H__ */
