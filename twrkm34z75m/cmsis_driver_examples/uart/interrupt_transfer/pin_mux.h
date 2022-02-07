/*
 * Copyright 2021 NXP
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
  kPIN_MUX_DirectionInput = 0U,         /* Input direction */
  kPIN_MUX_DirectionOutput = 1U,        /* Output direction */
  kPIN_MUX_DirectionInputOrOutput = 2U  /* Input or output direction */
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
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void UART0_InitPins(void);

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void UART0_DeinitPins(void);

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void UART1_InitPins(void);

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void UART1_DeinitPins(void);

/* PORTI6 (number 6), UART2_RX */
#define BOARD_DEBUG_UART_RX_PERIPHERAL                                     UART2   /*!< Device name: UART2 */
#define BOARD_DEBUG_UART_RX_SIGNAL                                            RX   /*!< UART2 signal: RX */
#define BOARD_DEBUG_UART_RX_PIN_NAME                                    UART2_RX   /*!< Pin name */
#define BOARD_DEBUG_UART_RX_LABEL                                     "UART2_RX"   /*!< Label */
#define BOARD_DEBUG_UART_RX_NAME                                 "DEBUG_UART_RX"   /*!< Identifier name */

/* PORTI7 (number 7), UART2_TX */
#define BOARD_DEBUG_UART_TX_PERIPHERAL                                     UART2   /*!< Device name: UART2 */
#define BOARD_DEBUG_UART_TX_SIGNAL                                            TX   /*!< UART2 signal: TX */
#define BOARD_DEBUG_UART_TX_PIN_NAME                                    UART2_TX   /*!< Pin name */
#define BOARD_DEBUG_UART_TX_LABEL                                     "UART2_TX"   /*!< Label */
#define BOARD_DEBUG_UART_TX_NAME                                 "DEBUG_UART_TX"   /*!< Identifier name */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void UART2_InitPins(void);

/* PORTI6 (number 6), UART2_RX */
#define UART2_DEINITPINS_DEBUG_UART_RX_PERIPHERAL                            LCD   /*!< Device name: LCD */
#define UART2_DEINITPINS_DEBUG_UART_RX_SIGNAL                                  P   /*!< LCD signal: P */
#define UART2_DEINITPINS_DEBUG_UART_RX_CHANNEL                                46   /*!< LCD P channel: 46 */
#define UART2_DEINITPINS_DEBUG_UART_RX_PIN_NAME                          LCD_P46   /*!< Pin name */
#define UART2_DEINITPINS_DEBUG_UART_RX_LABEL                          "UART2_RX"   /*!< Label */
#define UART2_DEINITPINS_DEBUG_UART_RX_NAME                      "DEBUG_UART_RX"   /*!< Identifier name */

/* PORTI7 (number 7), UART2_TX */
#define UART2_DEINITPINS_DEBUG_UART_TX_PERIPHERAL                            LCD   /*!< Device name: LCD */
#define UART2_DEINITPINS_DEBUG_UART_TX_SIGNAL                                  P   /*!< LCD signal: P */
#define UART2_DEINITPINS_DEBUG_UART_TX_CHANNEL                                47   /*!< LCD P channel: 47 */
#define UART2_DEINITPINS_DEBUG_UART_TX_PIN_NAME                          LCD_P47   /*!< Pin name */
#define UART2_DEINITPINS_DEBUG_UART_TX_LABEL                          "UART2_TX"   /*!< Label */
#define UART2_DEINITPINS_DEBUG_UART_TX_NAME                      "DEBUG_UART_TX"   /*!< Identifier name */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void UART2_DeinitPins(void);

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void UART3_InitPins(void);

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void UART3_DeinitPins(void);

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
