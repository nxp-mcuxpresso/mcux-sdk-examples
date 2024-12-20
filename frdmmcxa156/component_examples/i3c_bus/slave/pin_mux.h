/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

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
void BOARD_InitBootPins(void);

#define PCR_IBE_ibe1 0x01u /*!<@brief Input Buffer Enable: Enables */

/*! @name PORT1_8 (number 1), I3C0_SDA
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_INITPINS_UART_RXD_PORT PORT1               /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_UART_RXD_PIN 8U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_UART_RXD_PIN_MASK (1U << 8U)      /*!<@brief PORT pin mask */
                                                         /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPins(void);

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
