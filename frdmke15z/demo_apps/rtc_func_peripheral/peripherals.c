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
functionalGroups:
- name: BOARD_InitPeripherals
  called_from_default_init: true
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
 * DEMO_RTC initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'DEMO_RTC'
- type: 'rtc'
- mode: 'general'
- type_id: 'rtc_603f70732a5387a85b5715615cba9e65'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'RTC'
- config_sets:
  - fsl_rtc:
    - clockConfig_t: []
    - rtc_config:
      - updateMode: 'false'
      - supervisorAccess: 'false'
      - compensationIntervalInt: '1'
      - compensationTimeInt: '0'
      - setDateTime: 'true'
      - rtc_datetime:
        - year: '2015'
        - month: '11'
        - day: '11'
        - hour: '11'
        - minute: '11'
        - second: '11'
      - setAlarm: 'false'
      - start: 'true'
    - interruptsCfg:
      - interruptSources: ''
      - isSecondsInterruptEnabled: 'false'
      - secondsInterrupt:
        - IRQn: 'RTC_IRQn'
        - enable_priority: 'false'
        - priority: '1'
        - enable_custom_name: 'true'
        - handler_custom_name: 'DEMO_RTC_IRQn_Handle'
      - isInterruptEnabled: 'true'
      - commonInterrupt:
        - IRQn: 'RTC_IRQn'
        - enable_priority: 'false'
        - priority: '0'
        - enable_custom_name: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const rtc_config_t DEMO_RTC_config     = {.wakeupSelect         = false,
                                      .updateMode           = false,
                                      .supervisorAccess     = false,
                                      .compensationInterval = 0x0U,
                                      .compensationTime     = 0x0U};
rtc_datetime_t DEMO_RTC_dateTimeStruct = {.year = 2015, .month = 11, .day = 11, .hour = 11, .minute = 11, .second = 11};

void DEMO_RTC_init(void)
{
    /* RTC initialization */
    RTC_Init(DEMO_RTC_PERIPHERAL, &DEMO_RTC_config);
    /* Stop RTC timer */
    RTC_StopTimer(DEMO_RTC_PERIPHERAL);
    /* Date and time initialization */
    RTC_SetDatetime(DEMO_RTC_PERIPHERAL, &DEMO_RTC_dateTimeStruct);
    /* Start RTC timer */
    RTC_StartTimer(DEMO_RTC_PERIPHERAL);
    /* Enable interrupt RTC_IRQn request in the NVIC */
    EnableIRQ(RTC_IRQn);
}

/***********************************************************************************************************************
 * Initialization functions
 **********************************************************************************************************************/
void BOARD_InitPeripherals(void)
{
    /* Initialize components */
    DEMO_RTC_init();
}

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/
void BOARD_InitBootPeripherals(void)
{
    BOARD_InitPeripherals();
}
