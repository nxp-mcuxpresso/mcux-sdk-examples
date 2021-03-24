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
 * CMP initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'CMP'
- type: 'cmp'
- mode: 'polling'
- type_id: 'cmp_306724f57b92dbe1771f1514089d2b18'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'CMP1'
- config_sets:
  - fsl_filter:
    - filter_config:
      - filteringOptions: ''
    - quick_selection: 'ContinuousMode'
  - fsl_cmp:
    - main_config:
      - explicitEnableCmp: 'true'
      - hysteresisMode: 'kCMP_HysteresisLevel0'
      - enableHighSpeed: 'false'
      - enableInvertOutput: 'false'
      - useUnfilteredOutput: 'false'
      - enablePinOut: 'false'
    - positiveChannel: 'IN.0'
    - negativeChannel: 'IN.7'
  - fsl_dac:
    - enableDAC: 'true'
    - dac_config:
      - referenceVoltageSource: 'DAC_6bit_VIN2'
      - DACValue: '33'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
/* DEMO_CMP main configuration */
const cmp_config_t DEMO_CMP_config = {
    .enableCmp           = false,
    .hysteresisMode      = kCMP_HysteresisLevel0,
    .enableHighSpeed     = false,
    .enableInvertOutput  = false,
    .useUnfilteredOutput = false,
    .enablePinOut        = false,
};
/* Configuration of the DAC sub-module, used in the CMP_SetDACConfig() function */
const cmp_dac_config_t DEMO_CMP_dac_config = {.referenceVoltageSource = kCMP_VrefSourceVin2, .DACValue = 32U};

void DEMO_CMP_init(void)
{
    /* Initialize CMP main sub-module functionality */
    CMP_Init(DEMO_CMP_PERIPHERAL, &DEMO_CMP_config);
    /* Set up internal DAC sub-module, that can be used as input 7 of the CMP both inputs. */
    CMP_SetDACConfig(DEMO_CMP_PERIPHERAL, &DEMO_CMP_dac_config);
    /* Initialize CMP main sub-module functionality */
    CMP_SetInputChannels(DEMO_CMP_PERIPHERAL, DEMO_CMP_POSITIVE_INPUT_NUMBER, DEMO_CMP_NEGATIVE_INPUT_NUMBER);
    /* Explicitly enables CMP periphery to satisfy glitch limitations. */
    CMP_Enable(DEMO_CMP_PERIPHERAL, true);
}

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
    - enableSharedInterrupt: 'false'
    - sharedInterrupt:
      - IRQn: 'PIT_IRQn'
      - enable_priority: 'false'
      - priority: '0'
      - enable_custom_name: 'false'
    - timingConfig:
      - clockSource: 'BusInterfaceClock'
      - clockSourceFreq: 'BOARD_BootClockRUN'
    - channels:
      - 0:
        - channelNumber: '0'
        - enableChain: 'false'
        - timerPeriod: '500ms'
        - startTimer: 'true'
        - enableInterrupt: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const pit_config_t DEMO_PIT_config = {.enableRunInDebug = false};

void DEMO_PIT_init(void)
{
    /* Initialize the PIT. */
    PIT_Init(DEMO_PIT_PERIPHERAL, &DEMO_PIT_config);
    /* Set channel 0 period to 500 ms (37500000 ticks). */
    PIT_SetTimerPeriod(DEMO_PIT_PERIPHERAL, kPIT_Chnl_0, DEMO_PIT_0_TICKS);
    /* Start channel 0. */
    PIT_StartTimer(DEMO_PIT_PERIPHERAL, kPIT_Chnl_0);
}

/***********************************************************************************************************************
 * AOI initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'AOI'
- type: 'aoi'
- mode: 'AOI'
- type_id: 'aoi_5a2efbfd7a8a5208f8f552077e2b4ded'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'AOI1'
- config_sets:
  - fsl_aoi:
    - events:
      - 0:
        - product_terms:
          - 0:
            - input0: 'IN.0'
            - input1: 'invertedIN.4'
            - input2: 'kAOI_LogicOne'
            - input3: 'kAOI_LogicOne'
          - 1:
            - input0: 'kAOI_LogicZero'
            - input1: 'kAOI_LogicOne'
            - input2: 'kAOI_LogicOne'
            - input3: 'kAOI_LogicOne'
          - 2:
            - input0: 'kAOI_LogicZero'
            - input1: 'kAOI_LogicOne'
            - input2: 'kAOI_LogicOne'
            - input3: 'kAOI_LogicOne'
          - 3:
            - input0: 'kAOI_LogicZero'
            - input1: 'kAOI_LogicOne'
            - input2: 'kAOI_LogicOne'
            - input3: 'kAOI_LogicOne'
        - define: 'true'
        - initialize: 'true'
      - 1:
        - product_terms:
          - 0:
            - input0: 'kAOI_LogicZero'
            - input1: 'kAOI_LogicZero'
            - input2: 'kAOI_LogicZero'
            - input3: 'kAOI_LogicZero'
          - 1:
            - input0: 'kAOI_LogicZero'
            - input1: 'kAOI_LogicZero'
            - input2: 'kAOI_LogicZero'
            - input3: 'kAOI_LogicZero'
          - 2:
            - input0: 'kAOI_LogicZero'
            - input1: 'kAOI_LogicZero'
            - input2: 'kAOI_LogicZero'
            - input3: 'kAOI_LogicZero'
          - 3:
            - input0: 'kAOI_LogicZero'
            - input1: 'kAOI_LogicZero'
            - input2: 'kAOI_LogicZero'
            - input3: 'kAOI_LogicZero'
        - define: 'false'
        - initialize: 'false'
      - 2:
        - product_terms:
          - 0:
            - input0: 'kAOI_LogicZero'
            - input1: 'kAOI_LogicZero'
            - input2: 'kAOI_LogicZero'
            - input3: 'kAOI_LogicZero'
          - 1:
            - input0: 'kAOI_LogicZero'
            - input1: 'kAOI_LogicZero'
            - input2: 'kAOI_LogicZero'
            - input3: 'kAOI_LogicZero'
          - 2:
            - input0: 'kAOI_LogicZero'
            - input1: 'kAOI_LogicZero'
            - input2: 'kAOI_LogicZero'
            - input3: 'kAOI_LogicZero'
          - 3:
            - input0: 'kAOI_LogicZero'
            - input1: 'kAOI_LogicZero'
            - input2: 'kAOI_LogicZero'
            - input3: 'kAOI_LogicZero'
        - define: 'false'
        - initialize: 'false'
      - 3:
        - product_terms:
          - 0:
            - input0: 'kAOI_LogicZero'
            - input1: 'kAOI_LogicZero'
            - input2: 'kAOI_LogicZero'
            - input3: 'kAOI_LogicZero'
          - 1:
            - input0: 'kAOI_LogicZero'
            - input1: 'kAOI_LogicZero'
            - input2: 'kAOI_LogicZero'
            - input3: 'kAOI_LogicZero'
          - 2:
            - input0: 'kAOI_LogicZero'
            - input1: 'kAOI_LogicZero'
            - input2: 'kAOI_LogicZero'
            - input3: 'kAOI_LogicZero'
          - 3:
            - input0: 'kAOI_LogicZero'
            - input1: 'kAOI_LogicZero'
            - input2: 'kAOI_LogicZero'
            - input3: 'kAOI_LogicZero'
        - define: 'false'
        - initialize: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const aoi_event_config_t DEMO_AOI_event_config[1] = {{.PT0AC = kAOI_InputSignal,
                                                      .PT0BC = kAOI_InvInputSignal,
                                                      .PT0CC = kAOI_LogicOne,
                                                      .PT0DC = kAOI_LogicOne,
                                                      .PT1AC = kAOI_LogicZero,
                                                      .PT1BC = kAOI_LogicOne,
                                                      .PT1CC = kAOI_LogicOne,
                                                      .PT1DC = kAOI_LogicOne,
                                                      .PT2AC = kAOI_LogicZero,
                                                      .PT2BC = kAOI_LogicOne,
                                                      .PT2CC = kAOI_LogicOne,
                                                      .PT2DC = kAOI_LogicOne,
                                                      .PT3AC = kAOI_LogicZero,
                                                      .PT3BC = kAOI_LogicOne,
                                                      .PT3CC = kAOI_LogicOne,
                                                      .PT3DC = kAOI_LogicOne}};

void DEMO_AOI_init(void)
{
    /* Initialize AOI1 peripheral. */
    AOI_Init(DEMO_AOI_PERIPHERAL);
    /* Initialize AOI1 Event 0. */
    AOI_SetEventLogicConfig(DEMO_AOI_PERIPHERAL, kAOI_Event0, &DEMO_AOI_event_config[0]);
}

/***********************************************************************************************************************
 * Initialization functions
 **********************************************************************************************************************/
void BOARD_InitPeripherals(void)
{
    /* Initialize components */
    DEMO_CMP_init();
    DEMO_PIT_init();
    DEMO_AOI_init();
}

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/
void BOARD_InitBootPeripherals(void)
{
    BOARD_InitPeripherals();
}
