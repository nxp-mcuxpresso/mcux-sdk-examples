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
#include "fsl_cmp.h"
#include "fsl_clock.h"
#include "fsl_pit.h"
#include "fsl_aoi.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/
/* Definitions for BOARD_InitPeripherals functional group */
/* Definition of peripheral ID */
#define DEMO_CMP_PERIPHERAL CMP1
/* Definition of positive input source used in CMP_SetInputChannels() function */
#define DEMO_CMP_POSITIVE_INPUT_NUMBER 0U
/* Definition of negative input source used in CMP_SetInputChannels() function */
#define DEMO_CMP_NEGATIVE_INPUT_NUMBER 7U
/* BOARD_InitPeripherals defines for PIT */
/* Definition of peripheral ID. */
#define DEMO_PIT_PERIPHERAL PIT
/* Definition of clock source frequency. */
#define DEMO_PIT_CLK_FREQ 62500000UL
/* Definition of ticks count for channel 0. */
#define DEMO_PIT_0_TICKS 31249999U
/* Alias for AOI peripheral */
#define DEMO_AOI_PERIPHERAL AOI

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
extern const cmp_config_t DEMO_CMP_config;
extern const cmp_dac_config_t DEMO_CMP_dac_config;
extern const pit_config_t DEMO_PIT_config;
extern const aoi_event_config_t DEMO_AOI_event_config[1];

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
