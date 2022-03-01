/*
 * Copyright 2018-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

#include "board.h"

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/


/* Symbols to be used with GPIO driver */
#define ECSPI2_DEINITPINS_UART4_RXD_GPIO_PIN_MASK                    (1U << 28U)   /*!< GPIO pin mask */

/* Symbols to be used with GPIO driver */
#define ECSPI2_DEINITPINS_UART4_TXD_GPIO_PIN_MASK                    (1U << 29U)   /*!< GPIO pin mask */
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

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPins(void);                                 /*!< Function assigned for the core: Cortex-M4[m4] */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void ECSPI2_DeinitPins(void);                              /*!< Function assigned for the core: Cortex-M4[m4] */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void ECSPI2_InitPins(void);                                /*!< Function assigned for the core: Cortex-M4[m4] */

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
