/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _PMIC_SUPPORT_H_
#define _PMIC_SUPPORT_H_

#include "fsl_pca9422.h"
#include "fsl_power.h"

/*******************************************************************************
 * DEFINITION
 ******************************************************************************/
#ifndef BOARD_PMIC_CONFIG_USE_SEMA4 /* Set this macro to non-zero value, if CPU0 and CPU1 need both operate on the \
                                       PMIC. */
#define BOARD_PMIC_CONFIG_USE_SEMA4 0U
#endif
#ifndef BOARD_PMIC_I2C_SEMA42_GATE
#define BOARD_PMIC_I2C_SEMA42_GATE (63U)
#endif

#define BOARD_DVS_CTRL_GPIO       GPIO7
#define BOARD_DVS_CTRL_GPIO_RESET kGPIO0_RST_SHIFT_RSTn
#define BOARD_DVS_CTRL_GPIO_CLOCK kCLOCK_Gpio7
#define BOARD_DVS_CTRL0_GPIO_PIN  14U
#define BOARD_DVS_CTRL1_GPIO_PIN  15U
#define BOARD_DVS_CTRL2_GPIO_PIN  16U

#ifndef BOARD_PMIC_I2C_SEMA42_BASE
#define BOARD_PMIC_I2C_SEMA42_BASE SEMA42_0
#endif
#if defined(MIMXRT798S_cm33_core0_SERIES) || defined(MIMXRT758S_cm33_core0_SERIES) || \
    defined(MIMXRT735S_cm33_core0_SERIES)
#define BOARD_PMIC_I2C_SEMA42_ID (0U)
#elif defined(MIMXRT798S_cm33_core1_SERIES) || defined(MIMXRT758S_cm33_core1_SERIES) || \
    defined(MIMXRT735S_cm33_core1_SERIES)
#define BOARD_PMIC_I2C_SEMA42_ID (5U)
#else
#error "Unsupported core!"
#endif

extern pca9422_handle_t pca9422Handle;
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

/*!
 * @brief  Initialize the PMIC.
 */
void BOARD_InitPmic(void);
/*!
 * @brief  Set the PMIC before deep power down.
 */
void BOARD_SetPmicVoltageBeforeDeepPowerDown(void);
/*!
 *  @brief  Set PMIC for VDD1 voltage.
 *  @param  volt: the voltage in uV to be set.
 */
void BOARD_SetPmicVdd1Voltage(uint32_t volt);
/*!
 *  @brief  Set PMIC for VDD2 voltage.
 *  @param  volt: the voltage in uV to be set.
 */
void BOARD_SetPmicVdd2Voltage(uint32_t volt);

#if defined(PMC0)
/*!
 *  @brief  Initialize PMIC DVS_CTRL pins.
 */
void BOARD_InitPmicDVSPin(void);
/*!
 * @brief  Get the PMIC DVS_CTRL pins status.
 * @retval DVS_CTRL pins status.
 */
uint8_t BOARD_GetPmicDVSPinStatus(void);
/*!
 * @brief  Set the PMIC DVS_CTRL pins status.
 * @param dvs_out PMIC DVS_CTRL pins value.
 */
void BOARD_SetPmicDVSPinStatus(uint8_t dvs_out);
#endif
#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _PMIC_SUPPORT_H_ */
