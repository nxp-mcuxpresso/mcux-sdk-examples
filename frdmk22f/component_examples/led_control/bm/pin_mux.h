/*
 * Copyright 2018 NXP
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

#define SOPT5_UART1TXSRC_UART_TX 0x00u /*!<@brief UART 1 transmit data source select: UART1_TX pin */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPins(void);

/*! @name PORTA1 (number 23), J2[4]/RED_LED
  @{ */
#define BOARD_LEDRGB_RED_GPIO GPIOA /*!<@brief GPIO device name: GPIOA */
#define BOARD_LEDRGB_RED_PORT PORTA /*!<@brief PORT device name: PORTA */
#define BOARD_LEDRGB_RED_PIN 1U     /*!<@brief PORTA pin index: 1 */
                                    /* @} */

/*! @name PORTA2 (number 24), J1[8]/GREEN_LED
  @{ */
#define BOARD_LEDRGB_GREEN_GPIO GPIOA /*!<@brief GPIO device name: GPIOA */
#define BOARD_LEDRGB_GREEN_PORT PORTA /*!<@brief PORT device name: PORTA */
#define BOARD_LEDRGB_GREEN_PIN 2U     /*!<@brief PORTA pin index: 2 */
                                      /* @} */

/*! @name PORTD5 (number 62), J2[12]/BLUE_LED
  @{ */
#define BOARD_LEDRGB_BLUE_GPIO GPIOD /*!<@brief GPIO device name: GPIOD */
#define BOARD_LEDRGB_BLUE_PORT PORTD /*!<@brief PORT device name: PORTD */
#define BOARD_LEDRGB_BLUE_PIN 5U     /*!<@brief PORTD pin index: 5 */
                                     /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitLEDs(void);

/*! @name PORTA1 (number 23), J2[4]/RED_LED
  @{ */
#define BOARD_INITPWM_LEDRGB_RED_PORT PORTA /*!<@brief PORT device name: PORTA */
#define BOARD_INITPWM_LEDRGB_RED_PIN 1U     /*!<@brief PORTA pin index: 1 */
                                            /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPWM(void);

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
