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
processor: MKE15Z256xxx7
package_id: MKE15Z256VLL7
mcu_data: ksdk2_0
processor_version: 6.0.1
board: FRDM-KE15Z
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
 * PDB initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'PDB'
- type: 'pdb'
- mode: 'general'
- type_id: 'pdb_32eb8756416d042c4a30e2cf9bd8fca4'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'PDB0'
- config_sets:
  - fsl_pdb:
    - clockConfig:
      - clockSource: 'BusInterfaceClock'
      - clockSourceFreq: 'BOARD_BootClockRUN'
    - pdb_config:
      - loadValueMode: 'kPDB_LoadValueImmediately'
      - firstDivider: 'kPDB_PrescalerDivider1'
      - secondDivider: 'kPDB_DividerMultiplicationFactor1'
      - moduloValue_str: '2000'
      - triggerInputSource: 'kPDB_TriggerInput0'
      - initSWtrigger: 'false'
      - enableContinuousMode: 'false'
    - pdb_adc_triggering_config:
      - 0:
        - pdb_adc_triggered_device: 'ADC0'
        - pdb_adc_pretriggers_config:
          - 0:
            - enable: 'true'
            - outputSource: 'delayedTrigger'
            - delayValue_str: '500'
    - pdb_pulse_out_config: []
    - pdb_delay_interrupt_config:
      - actionAfterDelay: 'noAction'
      - delayValue_str: ''
    - pdb_interrupts_config:
      - interrupt_sel: ''
      - enable_irq: 'false'
      - interrupt:
        - IRQn: 'PDB0_IRQn'
        - enable_priority: 'false'
        - priority: '0'
        - enable_custom_name: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
/* PDB counter configuration */
const pdb_config_t DEMO_PDB_config = {.loadValueMode               = kPDB_LoadValueImmediately,
                                      .prescalerDivider            = kPDB_PrescalerDivider1,
                                      .dividerMultiplicationFactor = kPDB_DividerMultiplicationFactor1,
                                      .triggerInputSource          = kPDB_TriggerInput0,
                                      .enableContinuousMode        = false};
/* PDB ADC0 pre-triggers configuration */
pdb_adc_pretrigger_config_t DEMO_PDB_ADC0_pretriggers_config = {
    .enablePreTriggerMask          = 0U | (1U << kPDB_ADCPreTrigger0),
    .enableOutputMask              = 0U | (1U << kPDB_ADCPreTrigger0),
    .enableBackToBackOperationMask = 0U};
const uint32_t DEMO_PDB_ADC0_pretriggers_value[DEMO_PDB_ADC0_PRETRIGGERS_COUNT] = {500U};

void DEMO_PDB_init(void)
{
    /* PDB counter initialization */
    PDB_Init(DEMO_PDB_PERIPHERAL, &DEMO_PDB_config);
    /* PDB counter modulo initialization */
    PDB_SetModulusValue(DEMO_PDB_PERIPHERAL, 2000U);
    /* PDB ADC0 pre-triggers initialization */
    PDB_SetADCPreTriggerConfig(DEMO_PDB_PERIPHERAL, kPDB_ADCTriggerChannel0, &DEMO_PDB_ADC0_pretriggers_config);
    PDB_SetADCPreTriggerDelayValue(DEMO_PDB_PERIPHERAL, kPDB_ADCTriggerChannel0, kPDB_ADCPreTrigger0,
                                   DEMO_PDB_ADC0_pretriggers_value[DEMO_PDB_ADC0_PT0]);
    /* Load buffered registers values into the working register for the first time */
    PDB_DoLoadValues(DEMO_PDB_PERIPHERAL);
}

/***********************************************************************************************************************
 * FTM initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'FTM'
- type: 'ftm'
- mode: 'EdgeAligned'
- type_id: 'ftm_04a15ae4af2b404bf2ae403c3dbe98b3'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'FTM0'
- config_sets:
  - ftm_main_config:
    - ftm_config:
      - clockSource: 'kFTM_SystemClock'
      - clockSourceFreq: 'GetFreq'
      - prescale: 'kFTM_Prescale_Divide_1'
      - timerFrequency: '24000'
      - bdmMode: 'kFTM_BdmMode_0'
      - pwmSyncMode: 'kFTM_SoftwareTrigger'
      - reloadPoints: ''
      - faultMode: 'kFTM_Fault_Disable'
      - faultFilterValue: '0'
      - deadTimePrescale: 'kFTM_Deadtime_Prescale_1'
      - deadTimeValue: '0'
      - extTriggers: 'kFTM_Chnl0Trigger'
      - chnlInitState: ''
      - chnlPolarity: ''
      - useGlobalTimeBase: 'false'
    - timer_interrupts: ''
    - enable_irq: 'false'
    - ftm_interrupt:
      - IRQn: 'FTM0_IRQn'
      - enable_priority: 'false'
      - priority: '0'
      - enable_custom_name: 'false'
    - EnableTimerInInit: 'false'
  - ftm_edge_aligned_mode:
    - ftm_edge_aligned_channels_config:
      - 0:
        - edge_aligned_mode: 'kFTM_EdgeAlignedCombinedPwm'
        - combined_pwm:
          - chnlNumber: 'kFTM_Chnl_0'
          - level: 'kFTM_LowTrue'
          - dutyCyclePercent: '50'
          - firstEdgeDelayPercent: '0'
          - enable_chan_irq: 'false'
          - enable_chan1_irq: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const ftm_config_t DEMO_FTM_config = {.prescale          = kFTM_Prescale_Divide_1,
                                      .bdmMode           = kFTM_BdmMode_0,
                                      .pwmSyncMode       = kFTM_SoftwareTrigger,
                                      .reloadPoints      = 0,
                                      .faultMode         = kFTM_Fault_Disable,
                                      .faultFilterValue  = 0,
                                      .deadTimePrescale  = kFTM_Deadtime_Prescale_1,
                                      .deadTimeValue     = 0,
                                      .extTriggers       = kFTM_Chnl0Trigger,
                                      .chnlInitState     = 0,
                                      .chnlPolarity      = 0,
                                      .useGlobalTimeBase = false};

const ftm_chnl_pwm_signal_param_t DEMO_FTM_combinedPwmSignalParams[] = {
    {.chnlNumber = kFTM_Chnl_0, .level = kFTM_LowTrue, .dutyCyclePercent = 50, .firstEdgeDelayPercent = 0}};

void DEMO_FTM_init(void)
{
    FTM_Init(DEMO_FTM_PERIPHERAL, &DEMO_FTM_config);
    FTM_SetupPwm(DEMO_FTM_PERIPHERAL, DEMO_FTM_combinedPwmSignalParams,
                 sizeof(DEMO_FTM_combinedPwmSignalParams) / sizeof(ftm_chnl_pwm_signal_param_t), kFTM_EdgeAlignedCombinedPwm,
                 24000U, DEMO_FTM_CLOCK_SOURCE);
}

/***********************************************************************************************************************
 * ADC initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'ADC'
- type: 'adc12'
- mode: 'ADC12'
- type_id: 'adc12_5324d28dd0212c08055a9d9cd4317082'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'ADC0'
- config_sets:
  - fsl_adc12:
    - enable_irq: 'true'
    - adc_interrupt:
      - IRQn: 'ADC0_IRQn'
      - enable_priority: 'false'
      - priority: '0'
      - enable_custom_name: 'false'
    - adc12_config:
      - referenceVoltageSource: 'kADC12_ReferenceVoltageSourceVref'
      - clockSource: 'kADC12_ClockSourceAlt0'
      - clockSourceFreq: 'BOARD_BootClockRUN'
      - clockDivider: 'kADC12_ClockDivider1'
      - resolution: 'kADC12_Resolution8Bit'
      - sampleClockCount: '13'
      - enableContinuousConversion: 'false'
    - adc12HardwareCompareConfig:
      - hardwareCompareModeEnable: 'false'
    - adc12_hardware_average_mode: 'kADC12_HardwareAverageDisabled'
    - hardwareTrigger: 'true'
    - enableDMA: 'false'
    - doAutoCalibration: 'false'
    - offset: '0'
    - gain: '0'
    - adc12_channels_config:
      - 0:
        - channelNumber: 'SE.1'
        - enableInterruptOnConversionCompleted: 'true'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const adc12_config_t DEMO_ADC_config              = {.referenceVoltageSource     = kADC12_ReferenceVoltageSourceVref,
                                        .clockSource                = kADC12_ClockSourceAlt0,
                                        .clockDivider               = kADC12_ClockDivider1,
                                        .resolution                 = kADC12_Resolution8Bit,
                                        .sampleClockCount           = 13,
                                        .enableContinuousConversion = false};
adc12_channel_config_t DEMO_ADC_channelsConfig[1] = {
    {.channelNumber = 1U, .enableInterruptOnConversionCompleted = true}};
const adc12_hardware_average_mode_t DEMO_ADC_hardwareAverageConfig = kADC12_HardwareAverageDisabled;

void DEMO_ADC_init(void)
{
    /* Initialize ADC12 converter */
    ADC12_Init(DEMO_ADC_PERIPHERAL, &DEMO_ADC_config);
    /* Set to hardware trigger mode */
    ADC12_EnableHardwareTrigger(DEMO_ADC_PERIPHERAL, true);
    /* Configure hardware average mode */
    ADC12_SetHardwareAverage(DEMO_ADC_PERIPHERAL, DEMO_ADC_hardwareAverageConfig);
    /* Set the offset value for the conversion result */
    ADC12_SetOffsetValue(DEMO_ADC_PERIPHERAL, (uint32_t)0);
    /* Set the gain value for the conversion result */
    ADC12_SetGainValue(DEMO_ADC_PERIPHERAL, 0);
    /* Enable generating the DMA trigger when conversion is completed */
    ADC12_EnableDMA(DEMO_ADC_PERIPHERAL, false);
    /* Enable interrupt ADC0_IRQn request in the NVIC */
    EnableIRQ(ADC0_IRQn);
}

/***********************************************************************************************************************
 * Initialization functions
 **********************************************************************************************************************/
void BOARD_InitPeripherals(void)
{
    /* Initialize components */
    DEMO_PDB_init();
    DEMO_FTM_init();
    DEMO_ADC_init();
}

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/
void BOARD_InitBootPeripherals(void)
{
    BOARD_InitPeripherals();
}
