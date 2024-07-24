/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_


/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

/*!
 * @addtogroup pin_mux
 * @{
 */

/***********************************************************************************************************************
 * API
 **********************************************************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif


/*!
 * @brief Calls initialization functions.
 *
 */
void BOARD_InitBootPins(void);

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitLpuartPins(void);                           /*!< Function assigned for the core: Cortex-M33[cm33] */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitI2cPins(void);                              /*!< Function assigned for the core: Cortex-M33[cm33] */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPmicI2cPins(void);                          /*!< Function assigned for the core: Cortex-M33[cm33] */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitI2sPins(void);                              /*!< Function assigned for the core: Cortex-M33[cm33] */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitTpmPins(void);                              /*!< Function assigned for the core: Cortex-M33[cm33] */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitHdmiIntPins(void);                          /*!< Function assigned for the core: Cortex-M33[cm33] */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitTouchIntPins(void);                         /*!< Function assigned for the core: Cortex-M33[cm33] */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitButtonPins(void);                           /*!< Function assigned for the core: Cortex-M33[cm33] */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPmicModePins(void);                         /*!< Function assigned for the core: Cortex-M33[cm33] */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitLsm6dsoPins(void);                          /*!< Function assigned for the core: Cortex-M33[cm33] */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPdmPins(void);                              /*!< Function assigned for the core: Cortex-M33[cm33] */

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */
#endif /* _PIN_MUX_H_ */

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
