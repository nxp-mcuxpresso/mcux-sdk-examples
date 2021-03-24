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
#include "fsl_rtc.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/
/* Definitions for BOARD_InitPeripherals functional group */
/* Definition of peripheral ID */
#define DEMO_RTC_PERIPHERAL RTC
/* DEMO_RTC interrupt vector ID (number). */
#define DEMO_RTC_COMMON_IRQN RTC_IRQn
/* DEMO_RTC interrupt handler identifier. */
#define DEMO_RTC_COMMON_IRQHANDLER RTC_IRQHandler

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
extern const rtc_config_t DEMO_RTC_config;
/* Date and time structure */
extern rtc_datetime_t DEMO_RTC_dateTimeStruct;

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
