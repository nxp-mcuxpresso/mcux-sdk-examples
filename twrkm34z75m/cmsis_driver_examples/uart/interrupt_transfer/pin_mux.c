/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Pins v9.0
processor: MKM34Z256xxx7
package_id: MKM34Z256VLQ7
mcu_data: ksdk2_0
processor_version: 9.0.0
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

#include "fsl_common.h"
#include "fsl_port.h"
#include "pin_mux.h"

#define MISC_CTL_UART0IRSEL_NONE      0x00u   /*!< UART0 IrDA Select: Pad RX input (PTD[0], PTF[3] or PTK[3], as selected in Pinmux control) selected for RX input of UART0 and UART0 TX signal is not used for modulation */
#define PIN2_IDX                         2u   /*!< Pin number for pin 2 in a port */
#define PIN3_IDX                         3u   /*!< Pin number for pin 3 in a port */

/*
 * TEXT BELOW IS USED AS SETTING FOR THE PINS TOOL *****************************
UART0_InitPins:
- options: {coreID: singlecore, enableClock: 'true'}
- pin_list:
  - {pin_num: '81', peripheral: UART0, signal: TX, pin_signal: ADC0_SE14/PTK2/UART0_TX}
  - {pin_num: '82', peripheral: UART0, signal: RX, pin_signal: ADC0_SE15/PTK3/LLWU_P19/UART0_RX}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR THE PINS TOOL ***
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : UART0_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void UART0_InitPins(void) {
  CLOCK_EnableClock(kCLOCK_PortK);                           /* PCTLK Clock Gate Control: Clock enabled */

  PORT_SetPinMux(PORTK, PIN2_IDX, kPORT_MuxAlt2);            /* PORTK2 (pin 81) is configured as UART0_TX */
  PORT_SetPinMux(PORTK, PIN3_IDX, kPORT_MuxAlt2);            /* PORTK3 (pin 82) is configured as UART0_RX */
  SIM->MISC_CTL = ((SIM->MISC_CTL &
    (~(SIM_MISC_CTL_UART0IRSEL_MASK)))                       /* Mask bits to zero which are setting */
      | SIM_MISC_CTL_UART0IRSEL(MISC_CTL_UART0IRSEL_NONE)    /* UART0 IrDA Select: Pad RX input (PTD[0], PTF[3] or PTK[3], as selected in Pinmux control) selected for RX input of UART0 and UART0 TX signal is not used for modulation */
    );
}


#define PIN2_IDX                         2u   /*!< Pin number for pin 2 in a port */
#define PIN3_IDX                         3u   /*!< Pin number for pin 3 in a port */
/*
 * TEXT BELOW IS USED AS SETTING FOR THE PINS TOOL *****************************
UART0_DeinitPins:
- options: {coreID: singlecore, enableClock: 'true'}
- pin_list:
  - {pin_num: '81', peripheral: ADC0, signal: 'SE, 14', pin_signal: ADC0_SE14/PTK2/UART0_TX}
  - {pin_num: '82', peripheral: ADC0, signal: 'SE, 15', pin_signal: ADC0_SE15/PTK3/LLWU_P19/UART0_RX}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR THE PINS TOOL ***
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : UART0_DeinitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void UART0_DeinitPins(void) {
  CLOCK_EnableClock(kCLOCK_PortK);                           /* PCTLK Clock Gate Control: Clock enabled */

  PORT_SetPinMux(PORTK, PIN2_IDX, kPORT_PinDisabledOrAnalog); /* PORTK2 (pin 81) is configured as ADC0_SE14 */
  PORT_SetPinMux(PORTK, PIN3_IDX, kPORT_PinDisabledOrAnalog); /* PORTK3 (pin 82) is configured as ADC0_SE15 */
}


#define MISC_CTL_UART1IRSEL_NONE      0x00u   /*!< UART1 IrDA Select: Pad RX input (PTD[2], PTI[0] or PTK[5], as selected in Pinmux control) selected for RX input of UART1 and UART1 TX signal is not used for modulation */
#define PIN5_IDX                         5u   /*!< Pin number for pin 5 in a port */
#define PIN6_IDX                         6u   /*!< Pin number for pin 6 in a port */

/*
 * TEXT BELOW IS USED AS SETTING FOR THE PINS TOOL *****************************
UART1_InitPins:
- options: {coreID: singlecore, enableClock: 'true'}
- pin_list:
  - {pin_num: '98', peripheral: UART1, signal: RX, pin_signal: PTK5/UART1_RX}
  - {pin_num: '99', peripheral: UART1, signal: TX, pin_signal: PTK6/UART1_TX}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR THE PINS TOOL ***
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : UART1_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void UART1_InitPins(void) {
  CLOCK_EnableClock(kCLOCK_PortK);                           /* PCTLK Clock Gate Control: Clock enabled */

  PORT_SetPinMux(PORTK, PIN5_IDX, kPORT_MuxAlt2);            /* PORTK5 (pin 98) is configured as UART1_RX */
  PORT_SetPinMux(PORTK, PIN6_IDX, kPORT_MuxAlt2);            /* PORTK6 (pin 99) is configured as UART1_TX */
  SIM->MISC_CTL = ((SIM->MISC_CTL &
    (~(SIM_MISC_CTL_UART1IRSEL_MASK)))                       /* Mask bits to zero which are setting */
      | SIM_MISC_CTL_UART1IRSEL(MISC_CTL_UART1IRSEL_NONE)    /* UART1 IrDA Select: Pad RX input (PTD[2], PTI[0] or PTK[5], as selected in Pinmux control) selected for RX input of UART1 and UART1 TX signal is not used for modulation */
    );
}


#define PIN5_IDX                         5u   /*!< Pin number for pin 5 in a port */
#define PIN6_IDX                         6u   /*!< Pin number for pin 6 in a port */
/*
 * TEXT BELOW IS USED AS SETTING FOR THE PINS TOOL *****************************
UART1_DeinitPins:
- options: {coreID: singlecore, enableClock: 'true'}
- pin_list:
  - {pin_num: '98', peripheral: n/a, signal: disabled, pin_signal: PTK5/UART1_RX}
  - {pin_num: '99', peripheral: n/a, signal: disabled, pin_signal: PTK6/UART1_TX}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR THE PINS TOOL ***
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : UART1_DeinitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void UART1_DeinitPins(void) {
  CLOCK_EnableClock(kCLOCK_PortK);                           /* PCTLK Clock Gate Control: Clock enabled */

  PORT_SetPinMux(PORTK, PIN5_IDX, kPORT_PinDisabledOrAnalog); /* PORTK5 (pin 98) is disabled */
  PORT_SetPinMux(PORTK, PIN6_IDX, kPORT_PinDisabledOrAnalog); /* PORTK6 (pin 99) is disabled */
}


#define MISC_CTL_UART2IRSEL_NONE      0x00u   /*!< UART2 IrDA Select: Pad RX input PTI[6] or PTE[6] selected for RX input of UART2 and UART2 TX signal is not used for modulation */
#define PIN6_IDX                         6u   /*!< Pin number for pin 6 in a port */
#define PIN7_IDX                         7u   /*!< Pin number for pin 7 in a port */

/*
 * TEXT BELOW IS USED AS SETTING FOR THE PINS TOOL *****************************
UART2_InitPins:
- options: {prefix: BOARD_, coreID: singlecore, enableClock: 'true'}
- pin_list:
  - {pin_num: '6', peripheral: UART2, signal: RX, pin_signal: LCD_P46/PTI6/UART2_RX}
  - {pin_num: '7', peripheral: UART2, signal: TX, pin_signal: LCD_P47/PTI7/UART2_TX}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR THE PINS TOOL ***
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : UART2_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void UART2_InitPins(void) {
  CLOCK_EnableClock(kCLOCK_PortI);                           /* PCTLI Clock Gate Control: Clock enabled */

  PORT_SetPinMux(PORTI, PIN6_IDX, kPORT_MuxAlt2);            /* PORTI6 (pin 6) is configured as UART2_RX */
  PORT_SetPinMux(PORTI, PIN7_IDX, kPORT_MuxAlt2);            /* PORTI7 (pin 7) is configured as UART2_TX */
  SIM->MISC_CTL = ((SIM->MISC_CTL &
    (~(SIM_MISC_CTL_UART2IRSEL_MASK)))                       /* Mask bits to zero which are setting */
      | SIM_MISC_CTL_UART2IRSEL(MISC_CTL_UART2IRSEL_NONE)    /* UART2 IrDA Select: Pad RX input PTI[6] or PTE[6] selected for RX input of UART2 and UART2 TX signal is not used for modulation */
    );
}


#define PIN6_IDX                         6u   /*!< Pin number for pin 6 in a port */
#define PIN7_IDX                         7u   /*!< Pin number for pin 7 in a port */
/*
 * TEXT BELOW IS USED AS SETTING FOR THE PINS TOOL *****************************
UART2_DeinitPins:
- options: {coreID: singlecore, enableClock: 'true'}
- pin_list:
  - {pin_num: '6', peripheral: LCD, signal: 'P, 46', pin_signal: LCD_P46/PTI6/UART2_RX}
  - {pin_num: '7', peripheral: LCD, signal: 'P, 47', pin_signal: LCD_P47/PTI7/UART2_TX}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR THE PINS TOOL ***
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : UART2_DeinitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void UART2_DeinitPins(void) {
  CLOCK_EnableClock(kCLOCK_PortI);                           /* PCTLI Clock Gate Control: Clock enabled */

  PORT_SetPinMux(PORTI, PIN6_IDX, kPORT_PinDisabledOrAnalog); /* PORTI6 (pin 6) is configured as LCD_P46 */
  PORT_SetPinMux(PORTI, PIN7_IDX, kPORT_PinDisabledOrAnalog); /* PORTI7 (pin 7) is configured as LCD_P47 */
}


#define MISC_CTL_UART3IRSEL_NONE      0x00u   /*!< UART3 IrDA Select: Pad RX input (PTC[3] or PTD[7], as selected in Pinmux control) selected for RX input of UART3 and UART3 TX signal is not used for modulation */
#define PIN2_IDX                         2u   /*!< Pin number for pin 2 in a port */
#define PIN3_IDX                         3u   /*!< Pin number for pin 3 in a port */

/*
 * TEXT BELOW IS USED AS SETTING FOR THE PINS TOOL *****************************
UART3_InitPins:
- options: {coreID: singlecore, enableClock: 'true'}
- pin_list:
  - {pin_num: '29', peripheral: UART3, signal: TX, pin_signal: LCD_P41/PTC2/UART3_TX/XBAR_OUT1}
  - {pin_num: '30', peripheral: UART3, signal: RX, pin_signal: LCD_P42/CMP0_IN3/PTC3/LLWU_P13/UART3_RX}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR THE PINS TOOL ***
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : UART3_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void UART3_InitPins(void) {
  CLOCK_EnableClock(kCLOCK_PortC);                           /* PCTLC Clock Gate Control: Clock enabled */

  PORT_SetPinMux(PORTC, PIN2_IDX, kPORT_MuxAlt2);            /* PORTC2 (pin 29) is configured as UART3_TX */
  PORT_SetPinMux(PORTC, PIN3_IDX, kPORT_MuxAlt2);            /* PORTC3 (pin 30) is configured as UART3_RX */
  SIM->MISC_CTL = ((SIM->MISC_CTL &
    (~(SIM_MISC_CTL_UART3IRSEL_MASK)))                       /* Mask bits to zero which are setting */
      | SIM_MISC_CTL_UART3IRSEL(MISC_CTL_UART3IRSEL_NONE)    /* UART3 IrDA Select: Pad RX input (PTC[3] or PTD[7], as selected in Pinmux control) selected for RX input of UART3 and UART3 TX signal is not used for modulation */
    );
}


#define PIN2_IDX                         2u   /*!< Pin number for pin 2 in a port */
#define PIN3_IDX                         3u   /*!< Pin number for pin 3 in a port */
/*
 * TEXT BELOW IS USED AS SETTING FOR THE PINS TOOL *****************************
UART3_DeinitPins:
- options: {coreID: singlecore, enableClock: 'true'}
- pin_list:
  - {pin_num: '29', peripheral: LCD, signal: 'P, 41', pin_signal: LCD_P41/PTC2/UART3_TX/XBAR_OUT1}
  - {pin_num: '30', peripheral: CMP0, signal: 'IN, 3', pin_signal: LCD_P42/CMP0_IN3/PTC3/LLWU_P13/UART3_RX}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR THE PINS TOOL ***
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : UART3_DeinitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void UART3_DeinitPins(void) {
  CLOCK_EnableClock(kCLOCK_PortC);                           /* PCTLC Clock Gate Control: Clock enabled */

  PORT_SetPinMux(PORTC, PIN2_IDX, kPORT_PinDisabledOrAnalog); /* PORTC2 (pin 29) is configured as LCD_P41 */
  PORT_SetPinMux(PORTC, PIN3_IDX, kPORT_PinDisabledOrAnalog); /* PORTC3 (pin 30) is configured as CMP0_IN3 */
}

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
