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

/*! @name PORTB16 (number 62), U2[24]/U6[13]/TxD_sda/UART_TX_TGTMCU
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_UART_TX_TGTMCU_PORT PORTB                /*!<@brief PORT peripheral base pointer */
#define BOARD_UART_TX_TGTMCU_PIN 16U                   /*!<@brief PORT pin number */
#define BOARD_UART_TX_TGTMCU_PIN_MASK (1U << 16U)      /*!<@brief PORT pin mask */
                                                       /* @} */

/*! @name PORTB17 (number 63), U2[25]/U6[14]/RxD_sda/UART_RX_TGTMCU
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_UART_RX_TGTMCU_PORT PORTB                /*!<@brief PORT peripheral base pointer */
#define BOARD_UART_RX_TGTMCU_PIN 17U                   /*!<@brief PORT pin number */
#define BOARD_UART_RX_TGTMCU_PIN_MASK (1U << 17U)      /*!<@brief PORT pin mask */
                                                       /* @} */

/*! @name PORTE16 (number 10), MB_J1[3]/MB_J11[B26]/MB_U1[13]/MB_U3[25]/MB_TP3/MB_TP7/MB_TxD/J4[B26]/TxD
  @{ */

/* Symbols to be used with PORT driver */
#define MB_TXD_PORT PORTE                /*!<@brief PORT peripheral base pointer */
#define MB_TXD_PIN 16U                   /*!<@brief PORT pin number */
#define MB_TXD_PIN_MASK (1U << 16U)      /*!<@brief PORT pin mask */
                                         /* @} */

/*! @name PORTE17 (number 11), MB_J3[2]/MB_J11[B27]/MB_U1[12]/MB_U3[26]/MB_TP2/MB_TP10/MB_RxD/J4[B27]/RxD/MISO/TP19
  @{ */

/* Symbols to be used with PORT driver */
#define MB_RXD_PORT PORTE                /*!<@brief PORT peripheral base pointer */
#define MB_RXD_PIN 17U                   /*!<@brief PORT pin number */
#define MB_RXD_PIN_MASK (1U << 17U)      /*!<@brief PORT pin mask */
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
