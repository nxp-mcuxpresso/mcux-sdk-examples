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
 * @brief Calls initialization functions.
 *
 */
void BOARD_InitBootPins(void);

/* GPIO_AD_B0_13 (coord L14), UART1_RXD */
/* Routed pin properties */
#define BOARD_UART1_RXD_PERIPHERAL                                       LPUART1   /*!< Peripheral name */
#define BOARD_UART1_RXD_SIGNAL                                                RX   /*!< Signal name */

/* GPIO_AD_B0_12 (coord K14), UART1_TXD */
/* Routed pin properties */
#define BOARD_UART1_TXD_PERIPHERAL                                       LPUART1   /*!< Peripheral name */
#define BOARD_UART1_TXD_SIGNAL                                                TX   /*!< Signal name */

/* GPIO_B0_13 (coord D10), LCDIF_D9/BT_CFG[9] */
/* Routed pin properties */
#define BOARD_LCDIF_D9_PERIPHERAL                                            ARM   /*!< Peripheral name */
#define BOARD_LCDIF_D9_SIGNAL                                      arm_trace_swo   /*!< Signal name */

/* GPIO_AD_B1_02 (coord L11), SPDIF_OUT/J22[7] */
/* Routed pin properties */
#define BOARD_SPDIF_OUT_PERIPHERAL                                       LPUART2   /*!< Peripheral name */
#define BOARD_SPDIF_OUT_SIGNAL                                                TX   /*!< Signal name */

/* GPIO_AD_B1_03 (coord M12), SPDIF_IN/J22[8] */
/* Routed pin properties */
#define BOARD_SPDIF_IN_PERIPHERAL                                        LPUART2   /*!< Peripheral name */
#define BOARD_SPDIF_IN_SIGNAL                                                 RX   /*!< Signal name */


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
