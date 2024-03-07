/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018,2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _TSI_KEY_H_
#define _TSI_KEY_H_

#include "fsl_common.h"
#include "TSI_config.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief TSI scan mode. */
enum _tsi_scanmode
{
    kTSI_SelfMode    = 0,
    kTSI_MutualMode  = 1,
    kTSI_InvalidMode = 2
};

/*******************************************************************************
 * API
 ******************************************************************************/
/*!
 * @brief Initiate TSI module to self-mode.
 *
 * This function is used to initiate TSI moudle,set charge current/voltage,
 * oscillator frequency, track TSI baseline through TSI calibration function.
 * @param base     Pointer to the TSI param structure
 */
void TSI_Init_SelfMode(TSI_Type *base);

/*!
 * @brief Initiate TSI module to mutual-mode.
 *
 * This function is used to initiate TSI moudle,set charge current/voltage,
 * oscillator frequency, track TSI baseline through TSI calibration function.
 * @param base    Pointer to the TSI param structure
 */
void TSI_Init_MutualMode(TSI_Type *base);

/*!
 * @brief: Detect self-mode/mutual-mode touch key
 *
 * @param current_key_id  return valid touched key id, ranges from 0 - 99.
 * @return return key event, which is the same as TSI event.
 */
uint8_t TSI_KeyDetect(uint8_t *current_key_id);

/*
 * @brief Initialize TSI Key.
 *
 * This function configures TSI IP, calibrates TSI idle value, save as TSI baseline
 */
void TSI_KeyInit(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _TSI_KEY_H_ */
