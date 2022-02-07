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
#define I2C3_DEINITPINS_I2C3_SCL_GPIO_PIN_MASK                       (1U << 18U)   /*!< GPIO pin mask */

/* Symbols to be used with GPIO driver */
#define I2C3_DEINITPINS_I2C3_SDA_GPIO_PIN_MASK                       (1U << 19U)   /*!< GPIO pin mask */
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
void I2C3_DeinitPins(void);                                /*!< Function assigned for the core: Cortex-M4[m4] */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void I2C3_InitPins(void);                                  /*!< Function assigned for the core: Cortex-M4[m4] */

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
