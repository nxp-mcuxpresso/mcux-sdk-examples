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

/* GPIO_09 (number 3), LPUART1_RXD */
/* Routed pin properties */
#define BOARD_LPUART1_RXD_PERIPHERAL                                     LPUART1   /*!< Peripheral name */
#define BOARD_LPUART1_RXD_SIGNAL                                             RXD   /*!< Signal name */

/* GPIO_10 (number 2), LPUART1_TXD */
/* Routed pin properties */
#define BOARD_LPUART1_TXD_PERIPHERAL                                     LPUART1   /*!< Peripheral name */
#define BOARD_LPUART1_TXD_SIGNAL                                             TXD   /*!< Signal name */

/* GPIO_AD_09 (number 48), ADC12_4/JTAG_TDO/J55[6]/J26[4] */
/* Routed pin properties */
#define BOARD_ADC12_4_PERIPHERAL                                             ARM   /*!< Peripheral name */
#define BOARD_ADC12_4_SIGNAL                                       arm_trace_swo   /*!< Signal name */

/* GPIO_AD_02 (number 58), ADC12_2/J26[12]/J56[16] */
/* Routed pin properties */
#define BOARD_ADC12_2_PERIPHERAL                                         LPUART4   /*!< Peripheral name */
#define BOARD_ADC12_2_SIGNAL                                                 TXD   /*!< Signal name */

/* GPIO_AD_01 (number 59), ADC12_1/J26[10]/J56[14] */
/* Routed pin properties */
#define BOARD_ADC12_1_PERIPHERAL                                         LPUART4   /*!< Peripheral name */
#define BOARD_ADC12_1_SIGNAL                                                 RXD   /*!< Signal name */


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
