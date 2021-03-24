/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

/*! @brief Direction type  */
typedef enum _pin_mux_direction
{
    kPIN_MUX_DirectionInput = 0U,        /* Input direction */
    kPIN_MUX_DirectionOutput = 1U,       /* Output direction */
    kPIN_MUX_DirectionInputOrOutput = 2U /* Input or output direction */
} pin_mux_direction_t;

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
 * @brief UART 0 Receive Data Source Select: UART0_RX pin */
#define SOPT5_UART0RXSRC_UART_RX 0x00u
/*!
 * @brief UART 0 Transmit Data Source Select: UART0_TX pin */
#define SOPT5_UART0TXSRC_UART_TX 0x00u
/*!
 * @brief FTM0 channel 0 output PWM/OCMP mode source select: FTM0CH0 pin is the output of FTM0 channel 0 PWM/OCMP */
#define SOPT8_FTM0OCH0SRC_NO_MODULATION 0x00u
/*!
 * @brief FTM0 channel 1 output PWM/OCMP mode source select: FTM0CH1 pin is the output of FTM0 channel 1 PWM/OCMP */
#define SOPT8_FTM0OCH1SRC_NO_MODULATION 0x00u

/*! @name PORTB16 (number 39), J1[6]/U3[4]/UART0_RX_TGTMCU
  @{ */
#define BOARD_DEBUG_UART_RX_PERIPHERAL UART0                    /*!<@brief Device name: UART0 */
#define BOARD_DEBUG_UART_RX_SIGNAL RX                           /*!<@brief UART0 signal: RX */
#define BOARD_DEBUG_UART_RX_PORT PORTB                          /*!<@brief PORT device name: PORTB */
#define BOARD_DEBUG_UART_RX_PIN 16U                             /*!<@brief PORTB pin index: 16 */
#define BOARD_DEBUG_UART_RX_PIN_NAME UART0_RX                   /*!<@brief Pin name */
#define BOARD_DEBUG_UART_RX_LABEL "J1[6]/U3[4]/UART0_RX_TGTMCU" /*!<@brief Label */
#define BOARD_DEBUG_UART_RX_NAME "DEBUG_UART_RX"                /*!<@brief Identifier name */
                                                                /* @} */

/*! @name PORTB17 (number 40), U5[1]/UART0_TX_TGTMCU
  @{ */
#define BOARD_DEBUG_UART_TX_PERIPHERAL UART0              /*!<@brief Device name: UART0 */
#define BOARD_DEBUG_UART_TX_SIGNAL TX                     /*!<@brief UART0 signal: TX */
#define BOARD_DEBUG_UART_TX_PORT PORTB                    /*!<@brief PORT device name: PORTB */
#define BOARD_DEBUG_UART_TX_PIN 17U                       /*!<@brief PORTB pin index: 17 */
#define BOARD_DEBUG_UART_TX_PIN_NAME UART0_TX             /*!<@brief Pin name */
#define BOARD_DEBUG_UART_TX_LABEL "U5[1]/UART0_TX_TGTMCU" /*!<@brief Label */
#define BOARD_DEBUG_UART_TX_NAME "DEBUG_UART_TX"          /*!<@brief Identifier name */
                                                          /* @} */

/*! @name PORTD6 (number 63), J2[6]/LD1[1]/PTD6
  @{ */
#define BOARD_LED_RED_PERIPHERAL FTM0                    /*!<@brief Device name: FTM0 */
#define BOARD_LED_RED_SIGNAL CH                          /*!<@brief FTM0 signal: CH */
#define BOARD_LED_RED_PORT PORTD                         /*!<@brief PORT device name: PORTD */
#define BOARD_LED_RED_PIN 6U                             /*!<@brief PORTD pin index: 6 */
#define BOARD_LED_RED_CHANNEL 0                          /*!<@brief FTM0 channel: 0 */
#define BOARD_LED_RED_PIN_NAME FTM0_CH0                  /*!<@brief Pin name */
#define BOARD_LED_RED_LABEL "J2[6]/LD1[1]/PTD6"          /*!<@brief Label */
#define BOARD_LED_RED_NAME "LED_RED"                     /*!<@brief Identifier name */
#define BOARD_LED_RED_DIRECTION kPIN_MUX_DirectionOutput /*!<@brief Direction */
                                                         /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPins(void);

/*! @name PORTC6 (number 51), J2[8]/J2[13]/U10[4]/I2C0_SCL/SPI0_SOUT/PTC6
  @{ */
/*!
 * @brief Device name: I2C0 */
#define BOARD_I2C_CONFIGUREPINS_ACCEL_SCL_PERIPHERAL I2C0
/*!
 * @brief I2C0 signal: SCL */
#define BOARD_I2C_CONFIGUREPINS_ACCEL_SCL_SIGNAL SCL
/*!
 * @brief PORT device name: PORTC */
#define BOARD_I2C_CONFIGUREPINS_ACCEL_SCL_PORT PORTC
/*!
 * @brief PORTC pin index: 6 */
#define BOARD_I2C_CONFIGUREPINS_ACCEL_SCL_PIN 6U
/*!
 * @brief Pin name */
#define BOARD_I2C_CONFIGUREPINS_ACCEL_SCL_PIN_NAME I2C0_SCL
/*!
 * @brief Label */
#define BOARD_I2C_CONFIGUREPINS_ACCEL_SCL_LABEL "J2[8]/J2[13]/U10[4]/I2C0_SCL/SPI0_SOUT/PTC6"
/*!
 * @brief Identifier name */
#define BOARD_I2C_CONFIGUREPINS_ACCEL_SCL_NAME "ACCEL_SCL"
/* @} */

/*! @name PORTC7 (number 52), J1[15]/J1[16]/U10[6]/I2C0_SDA/PTC7
  @{ */
/*!
 * @brief Device name: I2C0 */
#define BOARD_I2C_CONFIGUREPINS_ACCEL_SDA_PERIPHERAL I2C0
/*!
 * @brief I2C0 signal: SDA */
#define BOARD_I2C_CONFIGUREPINS_ACCEL_SDA_SIGNAL SDA
/*!
 * @brief PORT device name: PORTC */
#define BOARD_I2C_CONFIGUREPINS_ACCEL_SDA_PORT PORTC
/*!
 * @brief PORTC pin index: 7 */
#define BOARD_I2C_CONFIGUREPINS_ACCEL_SDA_PIN 7U
/*!
 * @brief Pin name */
#define BOARD_I2C_CONFIGUREPINS_ACCEL_SDA_PIN_NAME I2C0_SDA
/*!
 * @brief Label */
#define BOARD_I2C_CONFIGUREPINS_ACCEL_SDA_LABEL "J1[15]/J1[16]/U10[6]/I2C0_SDA/PTC7"
/*!
 * @brief Identifier name */
#define BOARD_I2C_CONFIGUREPINS_ACCEL_SDA_NAME "ACCEL_SDA"
/* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_I2C_ConfigurePins(void);

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
