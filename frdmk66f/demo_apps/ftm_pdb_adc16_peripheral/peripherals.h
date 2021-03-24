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
#include "fsl_adc16.h"
#include "fsl_pdb.h"
#include "fsl_clock.h"
#include "fsl_ftm.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/
/* Definitions for BOARD_InitPeripherals functional group */
/* Alias for ADC0 peripheral */
#define DEMO_ADC0_PERIPHERAL ADC0
/* ADC0 interrupt vector ID (number). */
#define DEMO_ADC0_IRQN ADC0_IRQn
/* ADC0 interrupt handler identifier. */
#define DEMO_ADC0_IRQHANDLER ADC0_IRQHandler
/* Alias for ADC1 peripheral */
#define DEMO_ADC1_PERIPHERAL ADC1
/* ADC1 interrupt vector ID (number). */
#define DEMO_ADC1_IRQN ADC1_IRQn
/* ADC1 interrupt handler identifier. */
#define DEMO_ADC1_IRQHANDLER ADC1_IRQHandler
/* BOARD_InitPeripherals defines for PDB0 */
/* Definition of peripheral ID */
#define DEMO_PDB_PERIPHERAL PDB0
/* Definition of the ADC pre-trigger (PT) configurations. Used as index to DEMO_PDB_ADC0_pretriggers_value arrays. */
enum DEMO_PDB_adc0_trigger_config_indexes
{
    DEMO_PDB_ADC0_PT0 = 0U,
    DEMO_PDB_ADC0_PT1 = 1U
};
/* Definition of the ADC pre-trigger (PT) configurations. Used as index to DEMO_PDB_ADC1_pretriggers_value arrays. */
enum DEMO_PDB_adc1_trigger_config_indexes
{
    DEMO_PDB_ADC1_PT0 = 0U,
    DEMO_PDB_ADC1_PT1 = 1U
};
/* Number of the pre-triggers used in appropriate configuration` */
#define DEMO_PDB_ADC0_PRETRIGGERS_COUNT 2
/* Number of the pre-triggers used in appropriate configuration` */
#define DEMO_PDB_ADC1_PRETRIGGERS_COUNT 2
/* Number of the CMP widow sample pulse out configuration */
#define DEMO_PDB_DAC_TRIGGERS_COUNT 0
/* Number of the CMP widow sample pulse out configuration */
#define DEMO_PDB_CMP_PULSE_OUT_COUNT 0
/* Definition of peripheral ID */
#define DEMO_FTM_PERIPHERAL FTM0
/* Definition of the clock source frequency */
#define DEMO_FTM_CLOCK_SOURCE CLOCK_GetFreq(kCLOCK_BusClk)
/* FTM interrupt vector ID (number). */
#define DEMO_FTM_IRQN FTM0_IRQn
/* FTM interrupt handler identifier. */
#define DEMO_FTM_IRQHANDLER FTM0_IRQHandler

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
extern adc16_channel_config_t DEMO_ADC0_channelsConfig[2];
extern const adc16_config_t DEMO_ADC0_config;
extern const adc16_channel_mux_mode_t DEMO_ADC0_muxMode;
extern const adc16_hardware_average_mode_t DEMO_ADC0_hardwareAverageMode;
extern adc16_channel_config_t DEMO_ADC1_channelsConfig[2];
extern const adc16_config_t DEMO_ADC1_config;
extern const adc16_channel_mux_mode_t DEMO_ADC1_muxMode;
extern const adc16_hardware_average_mode_t DEMO_ADC1_hardwareAverageMode;
extern const pdb_config_t DEMO_PDB_config;
extern pdb_adc_pretrigger_config_t DEMO_PDB_ADC0_pretriggers_config;
extern const uint32_t DEMO_PDB_ADC0_pretriggers_value[DEMO_PDB_ADC0_PRETRIGGERS_COUNT];
extern pdb_adc_pretrigger_config_t DEMO_PDB_ADC1_pretriggers_config;
extern const ftm_config_t DEMO_FTM_config;

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
