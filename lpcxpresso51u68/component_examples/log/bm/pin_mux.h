/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_


/*******************************************************************************
 * Definitions
 ******************************************************************************/  

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

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Calls initialization functions.
 *
 */
void BOARD_InitBootPins(void);

/* FC0_RXD_SDA_MOSI (number 31), U18[4]/TO_MUX_P0_0-ISP_RX */
#define BOARD_DEBUG_UART_RX_PERIPHERAL                                 FLEXCOMM0   /*!< Device name: FLEXCOMM0 */
#define BOARD_DEBUG_UART_RX_SIGNAL                                  RXD_SDA_MOSI   /*!< FLEXCOMM0 signal: RXD_SDA_MOSI */
#define BOARD_DEBUG_UART_RX_PIN_NAME                            FC0_RXD_SDA_MOSI   /*!< Pin name */
#define BOARD_DEBUG_UART_RX_LABEL                    "U18[4]/TO_MUX_P0_0-ISP_RX"   /*!< Label */
#define BOARD_DEBUG_UART_RX_NAME                                 "DEBUG_UART_RX"   /*!< Identifier name */

/* FC0_TXD_SCL_MISO (number 32), U6[4]/U22[3]/P0_1-ISP_TX */
#define BOARD_DEBUG_UART_TX_PERIPHERAL                                 FLEXCOMM0   /*!< Device name: FLEXCOMM0 */
#define BOARD_DEBUG_UART_TX_SIGNAL                                  TXD_SCL_MISO   /*!< FLEXCOMM0 signal: TXD_SCL_MISO */
#define BOARD_DEBUG_UART_TX_PIN_NAME                            FC0_TXD_SCL_MISO   /*!< Pin name */
#define BOARD_DEBUG_UART_TX_LABEL                     "U6[4]/U22[3]/P0_1-ISP_TX"   /*!< Label */
#define BOARD_DEBUG_UART_TX_NAME                                 "DEBUG_UART_TX"   /*!< Identifier name */

/* PIO0_29 (number 11), J2[5]/D2[1]/P0_29-CT32B0_MAT3-RED */
#define BOARD_LED_RED_GPIO                                                  GPIO   /*!< GPIO device name: GPIO */
#define BOARD_LED_RED_PORT                                                    0U   /*!< PORT device index: 0 */
#define BOARD_LED_RED_GPIO_PIN                                               29U   /*!< PIO0 pin index: 29 */
#define BOARD_LED_RED_PIN_NAME                                           PIO0_29   /*!< Pin name */
#define BOARD_LED_RED_LABEL                  "J2[5]/D2[1]/P0_29-CT32B0_MAT3-RED"   /*!< Label */
#define BOARD_LED_RED_NAME                                             "LED_RED"   /*!< Identifier name */

/* PIO1_10 (number 30), J9[8]/D2[4]/P1_10-SCT4-LED_GREEN */
#define BOARD_LED_GREEN_GPIO                                                GPIO   /*!< GPIO device name: GPIO */
#define BOARD_LED_GREEN_PORT                                                  1U   /*!< PORT device index: 1 */
#define BOARD_LED_GREEN_GPIO_PIN                                             10U   /*!< PIO1 pin index: 10 */
#define BOARD_LED_GREEN_PIN_NAME                                         PIO1_10   /*!< Pin name */
#define BOARD_LED_GREEN_LABEL                 "J9[8]/D2[4]/P1_10-SCT4-LED_GREEN"   /*!< Label */
#define BOARD_LED_GREEN_NAME                                         "LED_GREEN"   /*!< Identifier name */

/* PIO1_9 (number 29), J9[5]/D2[3]/P1_9-BLUE_LED */
#define BOARD_LED_BLUE_GPIO                                                 GPIO   /*!< GPIO device name: GPIO */
#define BOARD_LED_BLUE_PORT                                                   1U   /*!< PORT device index: 1 */
#define BOARD_LED_BLUE_GPIO_PIN                                               9U   /*!< PIO1 pin index: 9 */
#define BOARD_LED_BLUE_PIN_NAME                                           PIO1_9   /*!< Pin name */
#define BOARD_LED_BLUE_LABEL                         "J9[5]/D2[3]/P1_9-BLUE_LED"   /*!< Label */
#define BOARD_LED_BLUE_NAME                                           "LED_BLUE"   /*!< Identifier name */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPins(void); /* Function assigned for the Cortex-M0P */

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */
#endif /* _PIN_MUX_H_ */

/*******************************************************************************
 * EOF
 ******************************************************************************/
