/*
 * Copyright 2017-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

#include "board.h"

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

/* ADC_IN2 (coord V32), M40_UART0_RX */
#define BOARD_INITPINS_M40_UART0_RX_PIN_FUNCTION_ID                 SC_P_ADC_IN2   /*!< Pin function id */

/* ADC_IN3 (coord V30), M40_UART0_TX */
#define BOARD_INITPINS_M40_UART0_TX_PIN_FUNCTION_ID                 SC_P_ADC_IN3   /*!< Pin function id */

/* ADC_IN0 (coord U35), M4_I2C0_1V8_SCL */
#define BOARD_INITPINS_M4_I2C0_1V8_SCL_PIN_FUNCTION_ID              SC_P_ADC_IN0   /*!< Pin function id */

/* ADC_IN1 (coord U33), M4_I2C0_1V8_SDA */
#define BOARD_INITPINS_M4_I2C0_1V8_SDA_PIN_FUNCTION_ID              SC_P_ADC_IN1   /*!< Pin function id */

/* USB_SS3_TC1 (coord H14), I2C1_SCL(SS3_SCL) */
#define BOARD_INITPINS_I2C1_SCL_PIN_FUNCTION_ID                 SC_P_USB_SS3_TC1   /*!< Pin function id */

/* USB_SS3_TC3 (coord C15), I2C1_SDA(SS3_SDA) */
#define BOARD_INITPINS_I2C1_SDA_PIN_FUNCTION_ID                 SC_P_USB_SS3_TC3   /*!< Pin function id */

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
 * @param ipc scfw ipchandle.
 *
 */
void BOARD_InitPins(sc_ipc_t ipc);                         /*!< Function assigned for the core: Cortex-M4F[m4] */

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
