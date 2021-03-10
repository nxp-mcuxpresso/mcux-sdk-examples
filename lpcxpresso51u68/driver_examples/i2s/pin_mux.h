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
#define BOARD_INITPINS_DEBUG_UART_RX_PERIPHERAL                        FLEXCOMM0   /*!< Device name: FLEXCOMM0 */
#define BOARD_INITPINS_DEBUG_UART_RX_SIGNAL                         RXD_SDA_MOSI   /*!< FLEXCOMM0 signal: RXD_SDA_MOSI */
#define BOARD_INITPINS_DEBUG_UART_RX_PIN_NAME                   FC0_RXD_SDA_MOSI   /*!< Pin name */
#define BOARD_INITPINS_DEBUG_UART_RX_LABEL           "U18[4]/TO_MUX_P0_0-ISP_RX"   /*!< Label */
#define BOARD_INITPINS_DEBUG_UART_RX_NAME                        "DEBUG_UART_RX"   /*!< Identifier name */

/* FC0_TXD_SCL_MISO (number 32), U6[4]/U22[3]/P0_1-ISP_TX */
#define BOARD_INITPINS_DEBUG_UART_TX_PERIPHERAL                        FLEXCOMM0   /*!< Device name: FLEXCOMM0 */
#define BOARD_INITPINS_DEBUG_UART_TX_SIGNAL                         TXD_SCL_MISO   /*!< FLEXCOMM0 signal: TXD_SCL_MISO */
#define BOARD_INITPINS_DEBUG_UART_TX_PIN_NAME                   FC0_TXD_SCL_MISO   /*!< Pin name */
#define BOARD_INITPINS_DEBUG_UART_TX_LABEL            "U6[4]/U22[3]/P0_1-ISP_TX"   /*!< Label */
#define BOARD_INITPINS_DEBUG_UART_TX_NAME                        "DEBUG_UART_TX"   /*!< Identifier name */

/* FC4_RTS_SCL_SSEL1 (number 3), J1[1]/JS4[1]/U10[7]/P0_25-FC4_SCLX */
#define BOARD_INITPINS_FC4_SCLX_PERIPHERAL                             FLEXCOMM4   /*!< Device name: FLEXCOMM4 */
#define BOARD_INITPINS_FC4_SCLX_SIGNAL                             RTS_SCL_SSEL1   /*!< FLEXCOMM4 signal: RTS_SCL_SSEL1 */
#define BOARD_INITPINS_FC4_SCLX_PIN_NAME                       FC4_RTS_SCL_SSEL1   /*!< Pin name */
#define BOARD_INITPINS_FC4_SCLX_LABEL       "J1[1]/JS4[1]/U10[7]/P0_25-FC4_SCLX"   /*!< Label */
#define BOARD_INITPINS_FC4_SCLX_NAME                                  "FC4_SCLX"   /*!< Identifier name */

/* FC4_CTS_SDA_SSEL0 (number 4), J1[3]/JS5[1]/U10[5]/P0_26-FC4_SDAX */
#define BOARD_INITPINS_FC4_SDAX_PERIPHERAL                             FLEXCOMM4   /*!< Device name: FLEXCOMM4 */
#define BOARD_INITPINS_FC4_SDAX_SIGNAL                             CTS_SDA_SSEL0   /*!< FLEXCOMM4 signal: CTS_SDA_SSEL0 */
#define BOARD_INITPINS_FC4_SDAX_PIN_NAME                       FC4_CTS_SDA_SSEL0   /*!< Pin name */
#define BOARD_INITPINS_FC4_SDAX_LABEL       "J1[3]/JS5[1]/U10[5]/P0_26-FC4_SDAX"   /*!< Label */
#define BOARD_INITPINS_FC4_SDAX_NAME                                  "FC4_SDAX"   /*!< Identifier name */

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
