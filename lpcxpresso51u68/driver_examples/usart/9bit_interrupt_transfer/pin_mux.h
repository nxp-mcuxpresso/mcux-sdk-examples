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

#define IOCON_PIO_DIGITAL_EN                                               0x80u   /*!<@brief Enables digital function */
#define IOCON_PIO_FUNC1                                                    0x01u   /*!<@brief Selects pin function 1 */
#define IOCON_PIO_INPFILT_OFF                                            0x0100u   /*!<@brief Input filter disabled */
#define IOCON_PIO_INV_DI                                                   0x00u   /*!<@brief Input function is not inverted */
#define IOCON_PIO_MODE_INACT                                               0x00u   /*!<@brief No addition pin function */
#define IOCON_PIO_OPENDRAIN_DI                                             0x00u   /*!<@brief Open drain is disabled */
#define IOCON_PIO_SLEW_STANDARD                                            0x00u   /*!<@brief Standard mode, output slew rate control is enabled */
#define PIO09_DIGIMODE_DIGITAL                                             0x01u   /*!<@brief Select Analog/Digital mode.: Digital mode. */
#define PIO09_FUNC_ALT1                                                    0x01u   /*!<@brief Selects pin function.: Alternative connection 1. */
#define PIO114_DIGIMODE_DIGITAL                                            0x01u   /*!<@brief Select Analog/Digital mode.: Digital mode. */
#define PIO114_FUNC_ALT2                                                   0x02u   /*!<@brief Selects pin function.: Alternative connection 2. */

/*! @name FC0_RXD_SDA_MOSI (number 31), U18[4]/TO_MUX_P0_0-ISP_RX */
#define BOARD_DEBUG_UART_RX_PORT                                              0U   /*!<@brief PORT device index: 0 */
#define BOARD_DEBUG_UART_RX_PIN                                               0U   /*!<@brief PORT pin number */
#define BOARD_DEBUG_UART_RX_PIN_MASK                                  (1U << 0U)   /*!<@brief PORT pin mask */

/*! @name FC0_TXD_SCL_MISO (number 32), U6[4]/U22[3]/P0_1-ISP_TX */
#define BOARD_DEBUG_UART_TX_PORT                                              0U   /*!<@brief PORT device index: 0 */
#define BOARD_DEBUG_UART_TX_PIN                                               1U   /*!<@brief PORT pin number */
#define BOARD_DEBUG_UART_TX_PIN_MASK                                  (1U << 1U)   /*!<@brief PORT pin mask */

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

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
