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
 * PIT initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'PIT'
- type: 'pit'
- mode: 'LPTMR_GENERAL'
- type_id: 'pit_a4782ba5223c8a2527ba91aeb2bc4159'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'PIT'
- config_sets:
  - fsl_pit:
    - enableRunInDebug: 'false'
    - timingConfig:
      - clockSource: 'BusInterfaceClock'
      - clockSourceFreq: 'GetFreq'
    - channels:
      - 0:
        - channelNumber: '0'
        - enableChain: 'false'
        - timerPeriod: '1000Hz'
        - startTimer: 'false'
        - enableInterrupt: 'true'
        - interrupt:
          - IRQn: 'PIT0_IRQn'
          - enable_priority: 'false'
          - priority: '0'
          - enable_custom_name: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const pit_config_t DEMO_PIT_config = {.enableRunInDebug = false};

void DEMO_PIT_init(void)
{
    /* Initialize the PIT. */
    PIT_Init(DEMO_PIT_PERIPHERAL, &DEMO_PIT_config);
    /* Set channel 0 period to N/A. */
    PIT_SetTimerPeriod(DEMO_PIT_PERIPHERAL, kPIT_Chnl_0, DEMO_PIT_0_TICKS);
    /* Enable interrupts from channel 0. */
    PIT_EnableInterrupts(DEMO_PIT_PERIPHERAL, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
    /* Enable interrupt DEMO_PIT_0_IRQN request in the NVIC */
    EnableIRQ(DEMO_PIT_0_IRQN);
}

/***********************************************************************************************************************
 * FTM initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'FTM'
- type: 'ftm'
- mode: 'QuadratureDecoder'
- type_id: 'ftm_04a15ae4af2b404bf2ae403c3dbe98b3'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'FTM1'
- config_sets:
  - ftm_main_config:
    - ftm_config:
      - prescale: 'kFTM_Prescale_Divide_32'
      - bdmMode: 'kFTM_BdmMode_0'
      - pwmSyncMode: 'kFTM_SoftwareTrigger'
      - reloadPoints: ''
      - faultMode: 'kFTM_Fault_Disable'
      - faultFilterValue: '0'
      - deadTimePrescale: 'kFTM_Deadtime_Prescale_1'
      - deadTimeValue: '0'
      - extTriggers: ''
      - chnlInitState: ''
      - chnlPolarity: ''
      - useGlobalTimeBase: 'false'
    - timer_interrupts: ''
    - enable_irq: 'false'
    - ftm_interrupt:
      - IRQn: 'FTM1_IRQn'
      - enable_priority: 'false'
      - priority: '0'
      - enable_custom_name: 'false'
    - EnableTimerInInit: 'false'
  - ftm_quadrature_decoder_mode:
    - timerModuloVal: '2000'
    - timerInitVal: '0'
    - ftm_quad_decoder_mode: 'kFTM_QuadPhaseEncode'
    - ftm_phase_a_params:
      - enablePhaseFilter: 'true'
      - phaseFilterVal: '0'
      - phasePolarity: 'kFTM_QuadPhaseNormal'
    - ftm_phase_b_params:
      - enablePhaseFilter: 'true'
      - phaseFilterVal: '0'
      - phasePolarity: 'kFTM_QuadPhaseNormal'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const ftm_config_t DEMO_FTM_config             = {.prescale          = kFTM_Prescale_Divide_32,
                                      .bdmMode           = kFTM_BdmMode_0,
                                      .pwmSyncMode       = kFTM_SoftwareTrigger,
                                      .reloadPoints      = 0,
                                      .faultMode         = kFTM_Fault_Disable,
                                      .faultFilterValue  = 0,
                                      .deadTimePrescale  = kFTM_Deadtime_Prescale_1,
                                      .deadTimeValue     = 0,
                                      .extTriggers       = 0,
                                      .chnlInitState     = 0,
                                      .chnlPolarity      = 0,
                                      .useGlobalTimeBase = false};
const ftm_phase_params_t DEMO_FTM_phaseAParams = {
    .enablePhaseFilter = true, .phaseFilterVal = 0, .phasePolarity = kFTM_QuadPhaseNormal

};
const ftm_phase_params_t DEMO_FTM_phaseBParams = {
    .enablePhaseFilter = true, .phaseFilterVal = 0, .phasePolarity = kFTM_QuadPhaseNormal

};

void DEMO_FTM_init(void)
{
    FTM_Init(DEMO_FTM_PERIPHERAL, &DEMO_FTM_config);
    /* Initialization of the timer initial value and modulo value */
    FTM_SetQuadDecoderModuloValue(DEMO_FTM_PERIPHERAL, 0, 2000);
    FTM_SetupQuadDecode(DEMO_FTM_PERIPHERAL, &DEMO_FTM_phaseAParams, &DEMO_FTM_phaseBParams, kFTM_QuadPhaseEncode);
}

/***********************************************************************************************************************
 * Initialization functions
 **********************************************************************************************************************/
void BOARD_InitPeripherals(void)
{
    /* Initialize components */
    DEMO_PIT_init();
    DEMO_FTM_init();
}

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/
void BOARD_InitBootPeripherals(void)
{
    BOARD_InitPeripherals();
}
