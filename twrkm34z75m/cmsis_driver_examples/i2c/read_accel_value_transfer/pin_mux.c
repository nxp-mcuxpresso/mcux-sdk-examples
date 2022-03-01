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


#define PIN0_IDX                         0u   /*!< Pin number for pin 0 in a port */
#define PIN7_IDX                         7u   /*!< Pin number for pin 7 in a port */

/*
 * TEXT BELOW IS USED AS SETTING FOR THE PINS TOOL *****************************
I2C0_InitPins:
- options: {coreID: singlecore, enableClock: 'true'}
- pin_list:
  - {pin_num: '79', peripheral: I2C0, signal: SCL, pin_signal: CMP0_IN4/PTD7/LLWU_P7/I2C0_SCL/XBAR_IN4/UART3_RX, slew_rate: fast, open_drain: enable, pull_select: up,
    pull_enable: enable}
  - {pin_num: '80', peripheral: I2C0, signal: SDA, pin_signal: PTE0/I2C0_SDA/XBAR_OUT4/UART3_TX/CLKOUT, slew_rate: fast, open_drain: enable, pull_select: up, pull_enable: enable}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR THE PINS TOOL ***
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : I2C0_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void I2C0_InitPins(void) {
  CLOCK_EnableClock(kCLOCK_PortD);                           /* PCTLD Clock Gate Control: Clock enabled */
  CLOCK_EnableClock(kCLOCK_PortE);                           /* PCTLE Clock Gate Control: Clock enabled */

  const port_pin_config_t portd7_pin79_config = {
    kPORT_PullUp,                                            /* Internal pull-up resistor is enabled */
    kPORT_FastSlewRate,                                      /* Fast slew rate is configured */
    kPORT_OpenDrainEnable,                                   /* Open drain is enabled */
    kPORT_MuxAlt2,                                           /* Pin is configured as I2C0_SCL */
    kPORT_UnlockRegister                                     /* Pin Control Register fields [15:0] are not locked */
  };
  PORT_SetPinConfig(PORTD, PIN7_IDX, &portd7_pin79_config);  /* PORTD7 (pin 79) is configured as I2C0_SCL */
  const port_pin_config_t porte0_pin80_config = {
    kPORT_PullUp,                                            /* Internal pull-up resistor is enabled */
    kPORT_FastSlewRate,                                      /* Fast slew rate is configured */
    kPORT_OpenDrainEnable,                                   /* Open drain is enabled */
    kPORT_MuxAlt2,                                           /* Pin is configured as I2C0_SDA */
    kPORT_UnlockRegister                                     /* Pin Control Register fields [15:0] are not locked */
  };
  PORT_SetPinConfig(PORTE, PIN0_IDX, &porte0_pin80_config);  /* PORTE0 (pin 80) is configured as I2C0_SDA */
}


#define PIN0_IDX                         0u   /*!< Pin number for pin 0 in a port */
#define PIN7_IDX                         7u   /*!< Pin number for pin 7 in a port */
/*
 * TEXT BELOW IS USED AS SETTING FOR THE PINS TOOL *****************************
I2C0_DeinitPins:
- options: {coreID: singlecore, enableClock: 'true'}
- pin_list:
  - {pin_num: '79', peripheral: CMP0, signal: 'IN, 4', pin_signal: CMP0_IN4/PTD7/LLWU_P7/I2C0_SCL/XBAR_IN4/UART3_RX}
  - {pin_num: '80', peripheral: n/a, signal: disabled, pin_signal: PTE0/I2C0_SDA/XBAR_OUT4/UART3_TX/CLKOUT}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR THE PINS TOOL ***
 */

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C0_DeinitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 *END**************************************************************************/
void I2C0_DeinitPins(void) {
  CLOCK_EnableClock(kCLOCK_PortD);                           /* PCTLD Clock Gate Control: Clock enabled */
  CLOCK_EnableClock(kCLOCK_PortE);                           /* PCTLE Clock Gate Control: Clock enabled */

  PORT_SetPinMux(PORTD, PIN7_IDX, kPORT_PinDisabledOrAnalog); /* PORTD7 (pin 79) is configured as CMP0_IN4 */
  PORT_SetPinMux(PORTE, PIN0_IDX, kPORT_PinDisabledOrAnalog); /* PORTE0 (pin 80) is disabled */
}

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
