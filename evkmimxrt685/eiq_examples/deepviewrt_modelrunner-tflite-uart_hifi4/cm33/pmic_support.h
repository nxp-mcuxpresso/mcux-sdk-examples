/*
 * Copyright 2019, 2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _PMIC_SUPPORT_H_
#define _PMIC_SUPPORT_H_

#include "fsl_pca9420.h"
#include "fsl_power.h"

/*******************************************************************************
 * DEFINITION
 ******************************************************************************/
extern pca9420_handle_t pca9420Handle;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

/**
 * @brief   Set PMIC volatage for VDDCORE.
 * @param   millivolt : expected VDDCORE voltage in mV
 */
void BOARD_SetVddCoreVoltage(uint32_t millivolt);

/**
 * @brief   Set PMIC volatage for particular frequency.
 * NOTE: The API is only valid when MAINPLLCLKDIV[7:0] and DSPPLLCLKDIV[7:0] are 0.
 *       If LVD falling trip voltage is higher than the required core voltage for particular frequency,
 *       LVD voltage will be decreased to safe level to avoid unexpected LVD reset or interrupt event.
 * @param   tempRange : part temperature range
 * @param   voltOpRange : voltage operation range.
 * @param   cm33Freq : CM33 CPU clock frequency value
 * @param   dspFreq : DSP CPU clock frequency value
 * @return  true for success and false for CPU frequency out of specified voltOpRange.
 */
bool BOARD_SetPmicVoltageForFreq(power_part_temp_range_t tempRange,
                                 power_volt_op_range_t voltOpRange,
                                 uint32_t cm33freq,
                                 uint32_t dspFreq);
/**
 * @brief   Initialize PMIC module.
 */
void BOARD_InitPmic(void);
/**
 * @brief   Ensure PMIC current mode voltage to higher than LVD voltage.
 */
void BOARD_SetPmicVoltageBeforeDeepSleep(void);
/**
 * @brief   Restore PMIC current mode voltage setting.
 */
void BOARD_RestorePmicVoltageAfterDeepSleep(void);
/**
 * @brief   Ensure PMIC current mode voltage >= 1.0V.
 */
void BOARD_SetPmicVoltageBeforeDeepPowerDown(void);
#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _PMIC_SUPPORT_H_ */
