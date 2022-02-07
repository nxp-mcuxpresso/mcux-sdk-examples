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

#define MISC_CTL_UART2IRSEL_NONE      0x00u   /*!< UART2 IrDA Select: Pad RX input PTI[6] or PTE[6] selected for RX input of UART2 and UART2 TX signal is not used for modulation */
#define PIN6_IDX                         6u   /*!< Pin number for pin 6 in a port */
#define PIN7_IDX                         7u   /*!< Pin number for pin 7 in a port */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitBootPins
 * Description   : Calls initialization functions.
 *
 * END ****************************************************************************************************************/
void BOARD_InitBootPins(void)
{
    BOARD_InitPins();
}

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPins:
- options: {callFromInitBoot: 'true', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '6', peripheral: UART2, signal: RX, pin_signal: LCD_P46/PTI6/UART2_RX}
  - {pin_num: '7', peripheral: UART2, signal: TX, pin_signal: LCD_P47/PTI7/UART2_TX}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitPins(void) {
  CLOCK_EnableClock(kCLOCK_PortI);                           /* PCTLI Clock Gate Control: Clock enabled */

  PORT_SetPinMux(PORTI, PIN6_IDX, kPORT_MuxAlt2);            /* PORTI6 (pin 6) is configured as UART2_RX */
  PORT_SetPinMux(PORTI, PIN7_IDX, kPORT_MuxAlt2);            /* PORTI7 (pin 7) is configured as UART2_TX */
  SIM->MISC_CTL = ((SIM->MISC_CTL &
    (~(SIM_MISC_CTL_UART2IRSEL_MASK)))                       /* Mask bits to zero which are setting */
      | SIM_MISC_CTL_UART2IRSEL(MISC_CTL_UART2IRSEL_NONE)    /* UART2 IrDA Select: Pad RX input PTI[6] or PTE[6] selected for RX input of UART2 and UART2 TX signal is not used for modulation */
    );
}


#define PIN2_IDX                         2u   /*!< Pin number for pin 2 in a port */
#define PIN3_IDX                         3u   /*!< Pin number for pin 3 in a port */
#define PIN4_IDX                         4u   /*!< Pin number for pin 4 in a port */
#define PIN5_IDX                         5u   /*!< Pin number for pin 5 in a port */

/*
 * TEXT BELOW IS USED AS SETTING FOR THE PINS TOOL *****************************
SPI0_InitPins:
- options: {coreID: singlecore, enableClock: 'true'}
- pin_list:
  - {pin_num: '112', peripheral: SPI0, signal: PCS0, pin_signal: LCD_P9/ADC0_SE11/PTG2/LLWU_P1/SPI0_PCS0}
  - {pin_num: '113', peripheral: SPI0, signal: SCK, pin_signal: LCD_P10/PTG3/SPI0_SCK/I2C0_SCL}
  - {pin_num: '114', peripheral: SPI0, signal: MOSI, pin_signal: LCD_P11/PTG4/SPI0_MOSI/I2C0_SDA}
  - {pin_num: '115', peripheral: SPI0, signal: MISO, pin_signal: LCD_P12/PTG5/SPI0_MISO/LPTMR0_ALT2}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR THE PINS TOOL ***
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : SPI0_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void SPI0_InitPins(void) {
  CLOCK_EnableClock(kCLOCK_PortG);                           /* PCTLG Clock Gate Control: Clock enabled */

  PORT_SetPinMux(PORTG, PIN2_IDX, kPORT_MuxAlt2);            /* PORTG2 (pin 112) is configured as SPI0_PCS0 */
  PORT_SetPinMux(PORTG, PIN3_IDX, kPORT_MuxAlt2);            /* PORTG3 (pin 113) is configured as SPI0_SCK */
  PORT_SetPinMux(PORTG, PIN4_IDX, kPORT_MuxAlt2);            /* PORTG4 (pin 114) is configured as SPI0_MOSI */
  PORT_SetPinMux(PORTG, PIN5_IDX, kPORT_MuxAlt2);            /* PORTG5 (pin 115) is configured as SPI0_MISO */
}


#define PIN2_IDX                         2u   /*!< Pin number for pin 2 in a port */
#define PIN3_IDX                         3u   /*!< Pin number for pin 3 in a port */
#define PIN4_IDX                         4u   /*!< Pin number for pin 4 in a port */
#define PIN5_IDX                         5u   /*!< Pin number for pin 5 in a port */
/*
 * TEXT BELOW IS USED AS SETTING FOR THE PINS TOOL *****************************
SPI0_DeinitPins:
- options: {coreID: singlecore, enableClock: 'true'}
- pin_list:
  - {pin_num: '112', peripheral: ADC0, signal: 'SE, 11', pin_signal: LCD_P9/ADC0_SE11/PTG2/LLWU_P1/SPI0_PCS0}
  - {pin_num: '113', peripheral: LCD, signal: 'P, 10', pin_signal: LCD_P10/PTG3/SPI0_SCK/I2C0_SCL}
  - {pin_num: '114', peripheral: LCD, signal: 'P, 11', pin_signal: LCD_P11/PTG4/SPI0_MOSI/I2C0_SDA}
  - {pin_num: '115', peripheral: LCD, signal: 'P, 12', pin_signal: LCD_P12/PTG5/SPI0_MISO/LPTMR0_ALT2}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR THE PINS TOOL ***
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : SPI0_DeinitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void SPI0_DeinitPins(void) {
  CLOCK_EnableClock(kCLOCK_PortG);                           /* PCTLG Clock Gate Control: Clock enabled */

  PORT_SetPinMux(PORTG, PIN2_IDX, kPORT_PinDisabledOrAnalog); /* PORTG2 (pin 112) is configured as ADC0_SE11 */
  PORT_SetPinMux(PORTG, PIN3_IDX, kPORT_PinDisabledOrAnalog); /* PORTG3 (pin 113) is configured as LCD_P10 */
  PORT_SetPinMux(PORTG, PIN4_IDX, kPORT_PinDisabledOrAnalog); /* PORTG4 (pin 114) is configured as LCD_P11 */
  PORT_SetPinMux(PORTG, PIN5_IDX, kPORT_PinDisabledOrAnalog); /* PORTG5 (pin 115) is configured as LCD_P12 */
}


/*
 * TEXT BELOW IS USED AS SETTING FOR THE PINS TOOL *****************************
SPI1_InitPins:
- options: {coreID: singlecore, enableClock: 'true'}
- pin_list: []
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR THE PINS TOOL ***
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : SPI1_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void SPI1_InitPins(void) {
}


/*
 * TEXT BELOW IS USED AS SETTING FOR THE PINS TOOL *****************************
SPI1_DeinitPins:
- options: {coreID: singlecore, enableClock: 'true'}
- pin_list: []
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR THE PINS TOOL ***
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : SPI1_DeinitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void SPI1_DeinitPins(void) {
}

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
