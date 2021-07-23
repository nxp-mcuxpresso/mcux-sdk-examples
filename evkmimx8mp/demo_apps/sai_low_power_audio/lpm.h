/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _LPM_H_
#define _LPM_H_

#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SYSTICK_BASE    GPT1
#define SYSTICK_IRQn    GPT1_IRQn
#define SYSTICK_HANDLER GPT1_IRQHandler
#define SYSTICK_CLOCK \
    24000000 / (CLOCK_GetRootPreDivider(kCLOCK_RootGpt1)) / (CLOCK_GetRootPostDivider(kCLOCK_RootGpt1))

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

/*!
 * @brief Configure the system tick(GPT) before entering the low power mode.
 * @return Return the sleep time ticks.
 */
uint32_t LPM_EnterTicklessIdle(uint32_t timeoutMilliSec, uint64_t *pCounter);
/*!
 * @brief Configure the system tick(GPT) after exist the low power mode.
 */
void LPM_ExitTicklessIdle(uint32_t timeoutTicks, uint64_t timeoutCounter);
/*!
 * @brief This function is used to increase the count of the block event.
 */
void LPM_IncreseBlockSleepCnt(void);
/*!
 * @brief This function is used to decrease the count of the block event.
 */
void LPM_DecreaseBlockSleepCnt(void);
/*!
 * @brief This function is used to judge if the system could enter low power mode.
 * @return Return true if there is no block event exists.
 */
bool LPM_AllowSleep(void);
#if defined(__cplusplus)
}
#endif /* __cplusplus*/
#endif /* _LPM_H_ */
