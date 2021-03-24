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
processor: MK66FN2M0xxx18
package_id: MK66FN2M0VMD18
mcu_data: ksdk2_0
processor_version: 6.0.1
board: FRDM-K66F
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
 * ADC0 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'ADC0'
- type: 'adc16'
- mode: 'ADC'
- type_id: 'adc16_7d827be2dc433dc756d94a7ce88cbcc5'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'ADC0'
- config_sets:
  - fsl_adc16:
    - adc16_config:
      - referenceVoltageSource: 'kADC16_ReferenceVoltageSourceVref'
      - clockSource: 'kADC16_ClockSourceAlt2'
      - enableAsynchronousClock: 'false'
      - clockDivider: 'kADC16_ClockDivider4'
      - resolution: 'kADC16_ResolutionSE12Bit'
      - longSampleMode: 'kADC16_LongSampleDisabled'
      - enableHighSpeed: 'true'
      - enableLowPower: 'false'
      - enableContinuousConversion: 'false'
    - adc16_channel_mux_mode: 'kADC16_ChannelMuxA'
    - adc16_hardware_compare_config:
      - hardwareCompareModeEnable: 'false'
    - doAutoCalibration: 'false'
    - offset: '0'
    - trigger: 'true'
    - hardwareAverageConfiguration: 'kADC16_HardwareAverageDisabled'
    - enable_dma: 'false'
    - enable_irq: 'true'
    - adc_interrupt:
      - IRQn: 'ADC0_IRQn'
      - enable_priority: 'false'
      - priority: '0'
      - enable_custom_name: 'false'
    - adc16_channels_config:
      - 0:
        - enableDifferentialConversion: 'false'
        - channelNumber: 'SE.9'
        - enableInterruptOnConversionCompleted: 'true'
        - channelGroup: '0'
        - initializeChannel: 'true'
      - 1:
        - enableDifferentialConversion: 'false'
        - channelNumber: 'SE.26'
        - enableInterruptOnConversionCompleted: 'true'
        - channelGroup: '1'
        - initializeChannel: 'true'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
adc16_channel_config_t DEMO_ADC0_channelsConfig[2] = {{
                                                          .channelNumber                        = 9U,
                                                          .enableDifferentialConversion         = false,
                                                          .enableInterruptOnConversionCompleted = true,
                                                      },
                                                      {
                                                          .channelNumber                        = 26U,
                                                          .enableDifferentialConversion         = false,
                                                          .enableInterruptOnConversionCompleted = true,
                                                      }};
const adc16_config_t DEMO_ADC0_config              = {.referenceVoltageSource     = kADC16_ReferenceVoltageSourceVref,
                                         .clockSource                = kADC16_ClockSourceAlt2,
                                         .enableAsynchronousClock    = false,
                                         .clockDivider               = kADC16_ClockDivider4,
                                         .resolution                 = kADC16_ResolutionSE12Bit,
                                         .longSampleMode             = kADC16_LongSampleDisabled,
                                         .enableHighSpeed            = true,
                                         .enableLowPower             = false,
                                         .enableContinuousConversion = false};
const adc16_channel_mux_mode_t DEMO_ADC0_muxMode   = kADC16_ChannelMuxA;
const adc16_hardware_average_mode_t DEMO_ADC0_hardwareAverageMode = kADC16_HardwareAverageDisabled;

void DEMO_ADC0_init(void)
{
    /* Initialize ADC16 converter */
    ADC16_Init(DEMO_ADC0_PERIPHERAL, &DEMO_ADC0_config);
    /* Make sure, that hardware trigger is used */
    ADC16_EnableHardwareTrigger(DEMO_ADC0_PERIPHERAL, true);
    /* Configure hardware average mode */
    ADC16_SetHardwareAverage(DEMO_ADC0_PERIPHERAL, DEMO_ADC0_hardwareAverageMode);
    /* Configure channel multiplexing mode */
    ADC16_SetChannelMuxMode(DEMO_ADC0_PERIPHERAL, DEMO_ADC0_muxMode);
    /* Initialize channel */
    ADC16_SetChannelConfig(DEMO_ADC0_PERIPHERAL, 0U, &DEMO_ADC0_channelsConfig[0]);
    /* Initialize channel */
    ADC16_SetChannelConfig(DEMO_ADC0_PERIPHERAL, 1U, &DEMO_ADC0_channelsConfig[1]);
    /* Enable interrupt DEMO_ADC0_IRQN request in the NVIC */
    EnableIRQ(DEMO_ADC0_IRQN);
}

/***********************************************************************************************************************
 * ADC1 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'ADC1'
- type: 'adc16'
- mode: 'ADC'
- type_id: 'adc16_7d827be2dc433dc756d94a7ce88cbcc5'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'ADC1'
- config_sets:
  - fsl_adc16:
    - adc16_config:
      - referenceVoltageSource: 'kADC16_ReferenceVoltageSourceVref'
      - clockSource: 'kADC16_ClockSourceAlt2'
      - enableAsynchronousClock: 'false'
      - clockDivider: 'kADC16_ClockDivider4'
      - resolution: 'kADC16_ResolutionSE12Bit'
      - longSampleMode: 'kADC16_LongSampleDisabled'
      - enableHighSpeed: 'true'
      - enableLowPower: 'false'
      - enableContinuousConversion: 'false'
    - adc16_channel_mux_mode: 'kADC16_ChannelMuxA'
    - adc16_hardware_compare_config:
      - hardwareCompareModeEnable: 'false'
    - doAutoCalibration: 'false'
    - offset: '0'
    - trigger: 'true'
    - hardwareAverageConfiguration: 'kADC16_HardwareAverageDisabled'
    - enable_dma: 'false'
    - enable_irq: 'true'
    - adc_interrupt:
      - IRQn: 'ADC1_IRQn'
      - enable_priority: 'false'
      - priority: '0'
      - enable_custom_name: 'false'
    - adc16_channels_config:
      - 0:
        - enableDifferentialConversion: 'false'
        - channelNumber: 'SE.9'
        - enableInterruptOnConversionCompleted: 'true'
        - channelGroup: '0'
        - initializeChannel: 'true'
      - 1:
        - enableDifferentialConversion: 'false'
        - channelNumber: 'SE.8'
        - enableInterruptOnConversionCompleted: 'true'
        - channelGroup: '1'
        - initializeChannel: 'true'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
adc16_channel_config_t DEMO_ADC1_channelsConfig[2] = {{
                                                          .channelNumber                        = 9U,
                                                          .enableDifferentialConversion         = false,
                                                          .enableInterruptOnConversionCompleted = true,
                                                      },
                                                      {
                                                          .channelNumber                        = 8U,
                                                          .enableDifferentialConversion         = false,
                                                          .enableInterruptOnConversionCompleted = true,
                                                      }};
const adc16_config_t DEMO_ADC1_config              = {.referenceVoltageSource     = kADC16_ReferenceVoltageSourceVref,
                                         .clockSource                = kADC16_ClockSourceAlt2,
                                         .enableAsynchronousClock    = false,
                                         .clockDivider               = kADC16_ClockDivider4,
                                         .resolution                 = kADC16_ResolutionSE12Bit,
                                         .longSampleMode             = kADC16_LongSampleDisabled,
                                         .enableHighSpeed            = true,
                                         .enableLowPower             = false,
                                         .enableContinuousConversion = false};
const adc16_channel_mux_mode_t DEMO_ADC1_muxMode   = kADC16_ChannelMuxA;
const adc16_hardware_average_mode_t DEMO_ADC1_hardwareAverageMode = kADC16_HardwareAverageDisabled;

void DEMO_ADC1_init(void)
{
    /* Initialize ADC16 converter */
    ADC16_Init(DEMO_ADC1_PERIPHERAL, &DEMO_ADC1_config);
    /* Make sure, that hardware trigger is used */
    ADC16_EnableHardwareTrigger(DEMO_ADC1_PERIPHERAL, true);
    /* Configure hardware average mode */
    ADC16_SetHardwareAverage(DEMO_ADC1_PERIPHERAL, DEMO_ADC1_hardwareAverageMode);
    /* Configure channel multiplexing mode */
    ADC16_SetChannelMuxMode(DEMO_ADC1_PERIPHERAL, DEMO_ADC1_muxMode);
    /* Initialize channel */
    ADC16_SetChannelConfig(DEMO_ADC1_PERIPHERAL, 0U, &DEMO_ADC1_channelsConfig[0]);
    /* Initialize channel */
    ADC16_SetChannelConfig(DEMO_ADC1_PERIPHERAL, 1U, &DEMO_ADC1_channelsConfig[1]);
    /* Enable interrupt DEMO_ADC1_IRQN request in the NVIC */
    EnableIRQ(DEMO_ADC1_IRQN);
}

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
      - clockSourceFreq: 'BOARD_BootClockHSRUN'
    - pdb_config:
      - loadValueMode: 'kPDB_LoadValueImmediately'
      - firstDivider: 'kPDB_PrescalerDivider1'
      - secondDivider: 'kPDB_DividerMultiplicationFactor1'
      - moduloValue_str: '2000'
      - triggerInputSource: 'kPDB_TriggerInput8'
      - initSWtrigger: 'false'
      - enableContinuousMode: 'false'
    - pdb_adc_triggering_config:
      - 0:
        - pdb_adc_triggered_device: 'ADC0'
        - pdb_adc_pretriggers_config:
          - 0:
            - enable: 'true'
            - outputSource: 'B2B'
          - 1:
            - enable: 'true'
            - outputSource: 'delayedTrigger'
            - delayValue_str: '500'
      - 1:
        - pdb_adc_triggered_device: 'ADC1'
        - pdb_adc_pretriggers_config:
          - 0:
            - enable: 'true'
            - outputSource: 'B2B'
          - 1:
            - enable: 'true'
            - outputSource: 'B2B'
    - pdb_dac_triggering_config: []
    - pdb_pulse_out_config: []
    - pdb_delay_interrupt_config:
      - actionAfterDelay: 'interrupt'
      - delayValue_str: '1500'
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
                                      .triggerInputSource          = kPDB_TriggerInput8,
                                      .enableContinuousMode        = false};
/* PDB ADC0 pre-triggers configuration */
pdb_adc_pretrigger_config_t DEMO_PDB_ADC0_pretriggers_config = {
    .enablePreTriggerMask          = 0U | (1U << kPDB_ADCPreTrigger0) | (1U << kPDB_ADCPreTrigger1),
    .enableOutputMask              = 0U | (1U << kPDB_ADCPreTrigger1),
    .enableBackToBackOperationMask = 0U | (1U << kPDB_ADCPreTrigger0)};
const uint32_t DEMO_PDB_ADC0_pretriggers_value[DEMO_PDB_ADC0_PRETRIGGERS_COUNT] = {0U, // Unused
                                                                                   500U};
/* PDB ADC1 pre-triggers configuration */
pdb_adc_pretrigger_config_t DEMO_PDB_ADC1_pretriggers_config = {
    .enablePreTriggerMask          = 0U | (1U << kPDB_ADCPreTrigger0) | (1U << kPDB_ADCPreTrigger1),
    .enableOutputMask              = 0U,
    .enableBackToBackOperationMask = 0U | (1U << kPDB_ADCPreTrigger0) | (1U << kPDB_ADCPreTrigger1)};

void DEMO_PDB_init(void)
{
    /* PDB counter initialization */
    PDB_Init(DEMO_PDB_PERIPHERAL, &DEMO_PDB_config);
    /* PDB counter modulo initialization */
    PDB_SetModulusValue(DEMO_PDB_PERIPHERAL, 2000U);
    /* PDB ADC0 pre-triggers initialization */
    PDB_SetADCPreTriggerConfig(DEMO_PDB_PERIPHERAL, kPDB_ADCTriggerChannel0, &DEMO_PDB_ADC0_pretriggers_config);
    PDB_SetADCPreTriggerDelayValue(DEMO_PDB_PERIPHERAL, kPDB_ADCTriggerChannel0, kPDB_ADCPreTrigger1,
                                   DEMO_PDB_ADC0_pretriggers_value[DEMO_PDB_ADC0_PT1]);
    /* PDB ADC1 pre-triggers initialization */
    PDB_SetADCPreTriggerConfig(DEMO_PDB_PERIPHERAL, kPDB_ADCTriggerChannel1, &DEMO_PDB_ADC1_pretriggers_config);
    /* Interrupts/DMA initialization */
    PDB_SetCounterDelayValue(DEMO_PDB_PERIPHERAL, 1500U);
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
      - timerFrequency: '16000'
      - bdmMode: 'kFTM_BdmMode_0'
      - pwmSyncMode: 'kFTM_SoftwareTrigger'
      - reloadPoints: ''
      - faultMode: 'kFTM_Fault_Disable'
      - faultFilterValue: '0'
      - deadTimePrescale: 'kFTM_Deadtime_Prescale_4'
      - deadTimeValue: '19'
      - extTriggers: ''
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
    - EnableTimerInInit: 'true'
  - ftm_edge_aligned_mode:
    - ftm_edge_aligned_channels_config:
      - 0:
        - edge_aligned_mode: 'kFTM_EdgeAlignedCombinedPwm'
        - combined_pwm:
          - chnlNumber: 'kFTM_Chnl_0'
          - level: 'kFTM_LowTrue'
          - dutyCyclePercent: '50'
          - firstEdgeDelayPercent: '20'
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
                                      .deadTimePrescale  = kFTM_Deadtime_Prescale_4,
                                      .deadTimeValue     = 19,
                                      .extTriggers       = 0,
                                      .chnlInitState     = 0,
                                      .chnlPolarity      = 0,
                                      .useGlobalTimeBase = false};

const ftm_chnl_pwm_signal_param_t DEMO_FTM_combinedPwmSignalParams[] = {
    {.chnlNumber = kFTM_Chnl_0, .level = kFTM_LowTrue, .dutyCyclePercent = 50, .firstEdgeDelayPercent = 20}};

void DEMO_FTM_init(void)
{
    FTM_Init(DEMO_FTM_PERIPHERAL, &DEMO_FTM_config);
    FTM_SetupPwm(DEMO_FTM_PERIPHERAL, DEMO_FTM_combinedPwmSignalParams,
                 sizeof(DEMO_FTM_combinedPwmSignalParams) / sizeof(ftm_chnl_pwm_signal_param_t), kFTM_EdgeAlignedCombinedPwm,
                 16000U, DEMO_FTM_CLOCK_SOURCE);
    FTM_StartTimer(DEMO_FTM_PERIPHERAL, kFTM_SystemClock);
}

/***********************************************************************************************************************
 * Initialization functions
 **********************************************************************************************************************/
void BOARD_InitPeripherals(void)
{
    /* Initialize components */
    DEMO_ADC0_init();
    DEMO_ADC1_init();
    DEMO_PDB_init();
    DEMO_FTM_init();
}

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/
void BOARD_InitBootPeripherals(void)
{
    BOARD_InitPeripherals();
}
