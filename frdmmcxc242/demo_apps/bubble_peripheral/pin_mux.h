/* 
 * Copyright 2024 NXP
 * All rights reserved.
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

#define SOPT4_TPM2CH0SRC_TPM2_CH0 0x00u    /*!<@brief TPM2 Channel 0 Input Capture Source Select: TPM2_CH0 signal */
#define SOPT5_LPUART0RXSRC_LPUART_RX 0x00u /*!<@brief LPUART0 Receive Data Source Select: LPUART_RX pin */
#define SOPT5_LPUART0TXSRC_LPUART_TX 0x00u /*!<@brief LPUART0 Transmit Data Source Select: LPUART0_TX pin */

/*! @name PORTA1 (number 23), D0-UART0_RX
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_DEBUG_UART_RX_PORT PORTA               /*!<@brief PORT peripheral base pointer */
#define BOARD_DEBUG_UART_RX_PIN 1U                   /*!<@brief PORT pin number */
#define BOARD_DEBUG_UART_RX_PIN_MASK (1U << 1U)      /*!<@brief PORT pin mask */
                                                     /* @} */

/*! @name PORTA2 (number 24), D1-UART0_TX
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_DEBUG_UART_TX_PORT PORTA               /*!<@brief PORT peripheral base pointer */
#define BOARD_DEBUG_UART_TX_PIN 2U                   /*!<@brief PORT pin number */
#define BOARD_DEBUG_UART_TX_PIN_MASK (1U << 2U)      /*!<@brief PORT pin mask */
                                                     /* @} */

/*! @name PORTB18 (number 41), J2[11]/LED_RED
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_LED_RED_PORT PORTB                /*!<@brief PORT peripheral base pointer */
#define BOARD_LED_RED_PIN 18U                   /*!<@brief PORT pin number */
#define BOARD_LED_RED_PIN_MASK (1U << 18U)      /*!<@brief PORT pin mask */
                                                /* @} */

/*! @name PORTB19 (number 42), J2[13]/LED_GREEN
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_LED_GREEN_PORT PORTB                /*!<@brief PORT peripheral base pointer */
#define BOARD_LED_GREEN_PIN 19U                   /*!<@brief PORT pin number */
#define BOARD_LED_GREEN_PIN_MASK (1U << 19U)      /*!<@brief PORT pin mask */
                                                  /* @} */

/*! @name PORTD1 (number 58), J1[3]/ACCEL_WAKE
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_ACCEL_WAKE_UP_FGPIO FGPIOD             /*!<@brief FGPIO peripheral base pointer */
#define BOARD_ACCEL_WAKE_UP_GPIO GPIOD               /*!<@brief GPIO peripheral base pointer */
#define BOARD_ACCEL_WAKE_UP_GPIO_PIN_MASK (1U << 1U) /*!<@brief GPIO pin mask */

/* Symbols to be used with PORT driver */
#define BOARD_ACCEL_WAKE_UP_PORT PORTD               /*!<@brief PORT peripheral base pointer */
#define BOARD_ACCEL_WAKE_UP_PIN 1U                   /*!<@brief PORT pin number */
#define BOARD_ACCEL_WAKE_UP_PIN_MASK (1U << 1U)      /*!<@brief PORT pin mask */
                                                     /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPins(void);

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_I2C_ConfigurePins(void);

/*! @name PORTD6 (number 63), J2[18]/J24[1]/D14-I2C1_SDA
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_ACCEL_I2C_SDA_FGPIO FGPIOD             /*!<@brief FGPIO peripheral base pointer */
#define BOARD_ACCEL_I2C_SDA_GPIO GPIOD               /*!<@brief GPIO peripheral base pointer */
#define BOARD_ACCEL_I2C_SDA_GPIO_PIN_MASK (1U << 6U) /*!<@brief GPIO pin mask */

/* Symbols to be used with PORT driver */
#define BOARD_ACCEL_I2C_SDA_PORT PORTD               /*!<@brief PORT peripheral base pointer */
#define BOARD_ACCEL_I2C_SDA_PIN 6U                   /*!<@brief PORT pin number */
#define BOARD_ACCEL_I2C_SDA_PIN_MASK (1U << 6U)      /*!<@brief PORT pin mask */
                                                     /* @} */

/*! @name PORTD7 (number 64), J2[20]/J23[1]/D15-I2C1_SCL
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_ACCEL_I2C_SCL_FGPIO FGPIOD             /*!<@brief FGPIO peripheral base pointer */
#define BOARD_ACCEL_I2C_SCL_GPIO GPIOD               /*!<@brief GPIO peripheral base pointer */
#define BOARD_ACCEL_I2C_SCL_GPIO_PIN_MASK (1U << 7U) /*!<@brief GPIO pin mask */

/* Symbols to be used with PORT driver */
#define BOARD_ACCEL_I2C_SCL_PORT PORTD               /*!<@brief PORT peripheral base pointer */
#define BOARD_ACCEL_I2C_SCL_PIN 7U                   /*!<@brief PORT pin number */
#define BOARD_ACCEL_I2C_SCL_PIN_MASK (1U << 7U)      /*!<@brief PORT pin mask */
                                                     /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_Init_I2C_GPIO_pins(void);

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
