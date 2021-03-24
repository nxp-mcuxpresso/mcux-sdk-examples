/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PERIPHERALS_H_
#define _PERIPHERALS_H_

/***********************************************************************************************************************
 * Included files
 **********************************************************************************************************************/
#include "fsl_common.h"
#include "fsl_lptmr.h"
#include "fsl_llwu.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/
/* Definitions for BOARD_InitPeripherals functional group */
/* BOARD_InitPeripherals defines for LPTMR0 */
/* Definition of peripheral ID */
#define DEMO_LPTMR_PERIPHERAL LPTMR0
/* Definition of the clock source frequency */
#define DEMO_LPTMR_CLK_FREQ 1000UL
/* Definition of the prescaled clock source frequency */
#define DEMO_LPTMR_INPUT_FREQ 1000UL
/* Definition of the timer period in us */
#define DEMO_LPTMR_USEC_COUNT 1000000UL
/* Definition of the timer period in number of ticks */
#define DEMO_LPTMR_TICKS 1000UL
/* LPTMR interrupt vector ID (number). */
#define DEMO_LPTMR_IRQN LPTMR0_IRQn
/* LPTMR interrupt handler identifier. */
#define DEMO_LPTMR_IRQHANDLER LPTMR0_IRQHandler
/* BOARD_InitPeripherals defines for LLWU */
/* Definition of peripheral ID. */
#define DEMO_LLWU_PERIPHERAL LLWU
/* LLWU interrupt vector ID (number). */
#define DEMO_LLWU_IRQN LLWU_IRQn
/* LLWU interrupt handler identifier. */
#define DEMO_LLWU_IRQHANDLER LLWU_IRQHandler

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
extern const lptmr_config_t DEMO_LPTMR_config;

/***********************************************************************************************************************
 * Initialization functions
 **********************************************************************************************************************/
void BOARD_InitPeripherals(void);

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/
void BOARD_InitBootPeripherals(void);

#if defined(__cplusplus)
}
#endif

#endif /* _PERIPHERALS_H_ */
