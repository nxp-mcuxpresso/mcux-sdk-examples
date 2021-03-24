/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Peripherals v6.0
processor: MK22FN512xxx12
package_id: MK22FN512VLH12
mcu_data: ksdk2_0
processor_version: 6.0.1
board: FRDM-K22F
functionalGroups:
- name: BOARD_InitPeripherals
  called_from_default_init: true
  id_prefix: DEMO_
  selectedCore: core0
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/

/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
component:
- type: 'system'
- type_id: 'system'
- global_system_definitions: []
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/

/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
component:
- type: 'msg'
- type_id: 'msg_6e2baaf3b97dbeef01c0043275f9a0e7'
- global_messages: []
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

/***********************************************************************************************************************
 * Included files
 **********************************************************************************************************************/
#include "peripherals.h"

/***********************************************************************************************************************
 * BOARD_InitPeripherals functional group
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * LPTMR initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'LPTMR'
- type: 'lptmr'
- mode: 'LPTMR_GENERAL'
- type_id: 'lptmr_2eeab91a1a42f8238f9ac768f18c65ae'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'LPTMR0'
- config_sets:
  - fsl_lptmr:
    - enableInterrupt: 'true'
    - interrupt:
      - IRQn: 'LPTMR0_IRQn'
      - enable_priority: 'false'
      - priority: '0'
      - enable_custom_name: 'false'
    - lptmr_config:
      - timerMode: 'kLPTMR_TimerModeTimeCounter'
      - pinSelect: 'ALT.0'
      - pinPolarity: 'kLPTMR_PinPolarityActiveHigh'
      - enableFreeRunning: 'false'
      - bypassPrescaler: 'true'
      - prescalerClockSource: 'kLPTMR_PrescalerClock_1'
      - clockSource: 'BOARD_BootClockHSRUN'
      - value: 'kLPTMR_Prescale_Glitch_0'
      - timerPeriod: '1000000 us'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const lptmr_config_t DEMO_LPTMR_config = {.timerMode            = kLPTMR_TimerModeTimeCounter,
                                          .pinSelect            = kLPTMR_PinSelectInput_0,
                                          .pinPolarity          = kLPTMR_PinPolarityActiveHigh,
                                          .enableFreeRunning    = false,
                                          .bypassPrescaler      = true,
                                          .prescalerClockSource = kLPTMR_PrescalerClock_1,
                                          .value                = kLPTMR_Prescale_Glitch_0};

void DEMO_LPTMR_init(void)
{
    /* Initialize the LPTMR */
    LPTMR_Init(DEMO_LPTMR_PERIPHERAL, &DEMO_LPTMR_config);
    /* Set LPTMR period to 1000000us */
    LPTMR_SetTimerPeriod(DEMO_LPTMR_PERIPHERAL, DEMO_LPTMR_TICKS);
    /* Configure timer interrupt */
    LPTMR_EnableInterrupts(DEMO_LPTMR_PERIPHERAL, kLPTMR_TimerInterruptEnable);
    /* Enable interrupt LPTMR0_IRQn request in the NVIC */
    EnableIRQ(LPTMR0_IRQn);
}

/***********************************************************************************************************************
 * LLWU initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'LLWU'
- type: 'llwu'
- mode: 'LLWU_GENERAL'
- type_id: 'llwu_3300b573fd6e3303f27cdce526747338'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'LLWU'
- config_sets:
  - fsl_llwu:
    - enable_irq: 'true'
    - interrupt:
      - IRQn: 'LLWU_IRQn'
      - enable_priority: 'false'
      - priority: '0'
      - enable_custom_name: 'false'
    - input_pins: []
    - internal_modules: []
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

void DEMO_LLWU_init(void)
{
    /* Enable interrupt DEMO_LLWU_IRQN request in the NVIC */
    EnableIRQ(DEMO_LLWU_IRQN);
}

/***********************************************************************************************************************
 * Initialization functions
 **********************************************************************************************************************/
void BOARD_InitPeripherals(void)
{
    /* Initialize components */
    DEMO_LPTMR_init();
    DEMO_LLWU_init();
}

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/
void BOARD_InitBootPeripherals(void)
{
    BOARD_InitPeripherals();
}
