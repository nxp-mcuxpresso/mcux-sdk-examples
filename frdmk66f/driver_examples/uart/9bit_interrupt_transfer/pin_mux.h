/*
 * Copyright 2021 NXP
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
void BOARD_InitBootPins(void);

#define SOPT5_UART0TXSRC_UART_TX 0x00u /*!<@brief UART 0 transmit data source select: UART0_TX pin */

/*! @name PORTB16 (coord E10), U7[4]/UART0_RX
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_DEBUG_UART_RX_PORT PORTB                /*!<@brief PORT peripheral base pointer */
#define BOARD_DEBUG_UART_RX_PIN 16U                   /*!<@brief PORT pin number */
#define BOARD_DEBUG_UART_RX_PIN_MASK (1U << 16U)      /*!<@brief PORT pin mask */
                                                      /* @} */

/*! @name PORTB17 (coord E9), U10[1]/UART0_TX
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_DEBUG_UART_TX_PORT PORTB                /*!<@brief PORT peripheral base pointer */
#define BOARD_DEBUG_UART_TX_PIN 17U                   /*!<@brief PORT pin number */
#define BOARD_DEBUG_UART_TX_PIN_MASK (1U << 17U)      /*!<@brief PORT pin mask */
                                                      /* @} */

/*! @name PORTA2 (coord K6), J9[6]/JTAG_TDO/TRACE_SWO/EZP_DO
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_TRACE_SWO_PORT PORTA               /*!<@brief PORT peripheral base pointer */
#define BOARD_TRACE_SWO_PIN 2U                   /*!<@brief PORT pin number */
#define BOARD_TRACE_SWO_PIN_MASK (1U << 2U)      /*!<@brief PORT pin mask */
                                                 /* @} */

/*! @name PORTD3 (coord B4), J2[10]/SPI0_SIN/FB_AD3
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_SPI0_SIN_PORT PORTD               /*!<@brief PORT peripheral base pointer */
#define BOARD_SPI0_SIN_PIN 3U                   /*!<@brief PORT pin number */
#define BOARD_SPI0_SIN_PIN_MASK (1U << 3U)      /*!<@brief PORT pin mask */
                                                /* @} */

/*! @name PORTD2 (coord C4), J2[8]/SPI0_SOUT/FB_AD4
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_SPI0_SOUT_PORT PORTD               /*!<@brief PORT peripheral base pointer */
#define BOARD_SPI0_SOUT_PIN 2U                   /*!<@brief PORT pin number */
#define BOARD_SPI0_SOUT_PIN_MASK (1U << 2U)      /*!<@brief PORT pin mask */
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
