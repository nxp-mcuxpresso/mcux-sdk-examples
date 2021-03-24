/*
 * Copyright 2019 NXP
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

/*! @name PORTB17 (coord E9), U10[1]/UART0_TX
  @{ */
#define BOARD_DEBUG_UART_TX_PORT PORTB /*!<@brief PORT device name: PORTB */
#define BOARD_DEBUG_UART_TX_PIN 17U    /*!<@brief PORTB pin index: 17 */
                                       /* @} */

/*! @name PORTB16 (coord E10), U7[4]/UART0_RX
  @{ */
#define BOARD_DEBUG_UART_RX_PORT PORTB /*!<@brief PORT device name: PORTB */
#define BOARD_DEBUG_UART_RX_PIN 16U    /*!<@brief PORTB pin index: 16 */
                                       /* @} */

/*! @name PORTC1 (coord B11), J1[5]/U20[C5]/I2S_TXD
  @{ */
#define BOARD_I2S_TXD_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_I2S_TXD_PIN 1U     /*!<@brief PORTC pin index: 1 */
                                 /* @} */

/*! @name PORTC2 (coord A12), J1[14]/FTM0_CH1/CMP1_IN0/FB_AD12
  @{ */
#define BOARD_FTM0_CH1_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_FTM0_CH1_PIN 2U     /*!<@brief PORTC pin index: 2 */
                                  /* @} */

/*! @name PORTC3 (coord A11), J1[2]/UART1_RX/FTM0_CH2/CLKOUT
  @{ */
#define BOARD_UART1_RX_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_UART1_RX_PIN 3U     /*!<@brief PORTC pin index: 3 */
                                  /* @} */

/*! @name PORTA6 (coord J7), J3[7]/CLKOUT
  @{ */
#define BOARD_CLKOUT_PORT PORTA /*!<@brief PORT device name: PORTA */
#define BOARD_CLKOUT_PIN 6U     /*!<@brief PORTA pin index: 6 */
                                /* @} */

/*! @name PORTA7 (coord J8), J3[9]/FTM0_CH4/TRACE_D3
  @{ */
#define BOARD_FTM0_CH4_PORT PORTA /*!<@brief PORT device name: PORTA */
#define BOARD_FTM0_CH4_PIN 7U     /*!<@brief PORTA pin index: 7 */
                                  /* @} */

/*! @name PORTA2 (coord K6), J9[6]/JTAG_TDO/TRACE_SWO/EZP_DO
  @{ */
#define BOARD_TRACE_SWO_PORT PORTA /*!<@brief PORT device name: PORTA */
#define BOARD_TRACE_SWO_PIN 2U     /*!<@brief PORTA pin index: 2 */
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
