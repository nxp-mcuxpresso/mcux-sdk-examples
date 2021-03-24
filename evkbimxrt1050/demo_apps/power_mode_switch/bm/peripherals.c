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
processor: MIMXRT1052xxxxB
package_id: MIMXRT1052DVL6B
mcu_data: ksdk2_0
processor_version: 6.0.1
board: IMXRT1050-EVKB
functionalGroups:
- name: BOARD_InitPeripherals
  called_from_default_init: true
  id_prefix: DEMO_
  selectedCore: core0
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/

/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
component:
- type: 'system'
- type_id: 'system_54b53072540eeeb8f8e9343e71f28176'
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
 * GPT initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'GPT'
- type: 'gpt'
- mode: 'general'
- type_id: 'gpt_e92a0cbd07e389b82a1d19b05eb9fdda'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'GPT2'
- config_sets:
  - fsl_gpt:
    - gpt_config:
      - clockSource: 'kGPT_ClockSource_LowFreq'
      - clockSourceFreq: 'BOARD_BootClockRUN'
      - oscDivider: '1'
      - divider: '1'
      - enableFreeRun: 'false'
      - enableRunInWait: 'true'
      - enableRunInStop: 'true'
      - enableRunInDoze: 'true'
      - enableRunInDbg: 'false'
      - enableMode: 'true'
    - input_capture_channels: []
    - output_compare_channels: []
    - interrupt_requests: ''
    - isInterruptEnabled: 'false'
    - interrupt:
      - IRQn: 'GPT2_IRQn'
      - enable_priority: 'false'
      - priority: '0'
      - enable_custom_name: 'false'
    - EnableTimerInInit: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const gpt_config_t DEMO_GPT_config = {.clockSource     = kGPT_ClockSource_LowFreq,
                                      .divider         = 1,
                                      .enableFreeRun   = false,
                                      .enableRunInWait = true,
                                      .enableRunInStop = true,
                                      .enableRunInDoze = true,
                                      .enableRunInDbg  = false,
                                      .enableMode      = true};

void DEMO_GPT_init(void)
{
    /* GPT device and channels initialization */
    GPT_Init(DEMO_GPT_PERIPHERAL, &DEMO_GPT_config);
    GPT_SetOscClockDivider(DEMO_GPT_PERIPHERAL, 1);
    /* Enable GPT interrupt sources */
    GPT_EnableInterrupts(DEMO_GPT_PERIPHERAL, 0);
}

/***********************************************************************************************************************
 * Initialization functions
 **********************************************************************************************************************/
void BOARD_InitPeripherals(void)
{
    /* Initialize components */
    DEMO_GPT_init();
}

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/
void BOARD_InitBootPeripherals(void)
{
    BOARD_InitPeripherals();
}
