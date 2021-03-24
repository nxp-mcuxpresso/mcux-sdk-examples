/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
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
    kPIN_MUX_DirectionInput = 0U,        /* Input direction */
    kPIN_MUX_DirectionOutput = 1U,       /* Output direction */
    kPIN_MUX_DirectionInputOrOutput = 2U /* Input or output direction */
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
 * @brief UART 0 Receive Data Source Select: UART0_RX pin */
#define SOPT5_UART0RXSRC_UART_RX 0x00u
/*!
 * @brief UART 0 Transmit Data Source Select: UART0_TX pin */
#define SOPT5_UART0TXSRC_UART_TX 0x00u
/*!
 * @brief FTM0 channel 0 output PWM/OCMP mode source select: FTM0CH0 pin is the output of FTM0 channel 0 PWM/OCMP */
#define SOPT8_FTM0OCH0SRC_NO_MODULATION 0x00u
/*!
 * @brief FTM0 channel 1 output PWM/OCMP mode source select: FTM0CH1 pin is the output of FTM0 channel 1 PWM/OCMP */
#define SOPT8_FTM0OCH1SRC_NO_MODULATION 0x00u
/*!
 * @brief FTM0 channel 2 output PWM/OCMP mode source select: FTM0CH2 pin is the output of FTM0 channel 2 PWM/OCMP */
#define SOPT8_FTM0OCH2SRC_NO_MODULATION 0x00u
/*!
 * @brief FTM0 channel 3 output PWM/OCMP mode source select: FTM0CH3 pin is the output of FTM0 channel 3 PWM/OCMP */
#define SOPT8_FTM0OCH3SRC_NO_MODULATION 0x00u
/*!
 * @brief FTM0 channel 4 output PWM/OCMP mode source select: FTM0CH4 pin is the output of FTM0 channel 4 PWM/OCMP */
#define SOPT8_FTM0OCH4SRC_NO_MODULATION 0x00u
/*!
 * @brief FTM0 channel 5 output PWM/OCMP mode source select: FTM0CH5 pin is the output of FTM0 channel 5 PWM/OCMP */
#define SOPT8_FTM0OCH5SRC_NO_MODULATION 0x00u

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
