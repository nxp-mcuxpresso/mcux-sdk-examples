/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

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
void BOARD_InitPins(void);

#define PIO029_DIGIMODE_DIGITAL 0x01u /*!<@brief Select Analog/Digital mode.: Digital mode. */
#define PIO029_FUNC_ALT1 0x01u        /*!<@brief Selects pin function.: Alternative connection 1. */
#define PIO030_DIGIMODE_DIGITAL 0x01u /*!<@brief Select Analog/Digital mode.: Digital mode. */
#define PIO030_FUNC_ALT1 0x01u        /*!<@brief Selects pin function.: Alternative connection 1. */
#define PIO320_DIGIMODE_DIGITAL 0x01u /*!<@brief Select Analog/Digital mode.: Digital mode. */
#define PIO320_FUNC_ALT1 0x01u        /*!<@brief Selects pin function.: Alternative connection 1. */
#define PIO32_DIGIMODE_DIGITAL 0x01u  /*!<@brief Select Analog/Digital mode.: Digital mode. */
#define PIO32_FUNC_ALT2 0x02u         /*!<@brief Selects pin function.: Alternative connection 2. */
#define PIO33_DIGIMODE_DIGITAL 0x01u  /*!<@brief Select Analog/Digital mode.: Digital mode. */
#define PIO33_FUNC_ALT2 0x02u         /*!<@brief Selects pin function.: Alternative connection 2. */

/*! @name FC0_RXD_SDA_MOSI (coord B13), U24[13]/P0_29-ISP_FC0_RXD
  @{ */
/* @} */

/*! @name FC0_TXD_SCL_MISO (coord A2), U24[12]/P0_30-ISP_FC0_TXD
  @{ */
/* @} */

/*! @name FC9_TXD_SCL_MISO (coord A13), J9[8]/LED2/P3_3-USR_LED2
  @{ */
/* @} */

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
