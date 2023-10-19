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

#define MISC_CTL_TMR1SCSEL_PTG0_PTC6  0x00u   /*!< Quadtimer Channel1 Secondary Count source Select: Pad PTG0 or PTC6, depending upon PCTL configuration. */
#define MISC_CTL_UART2IRSEL_NONE      0x00u   /*!< UART2 IrDA Select: Pad RX input PTI[6] or PTE[6] selected for RX input of UART2 and UART2 TX signal is not used for modulation */
#define PIN0_IDX                         0u   /*!< Pin number for pin 0 in a port */
#define PIN1_IDX                         1u   /*!< Pin number for pin 1 in a port */
#define PIN4_IDX                         4u   /*!< Pin number for pin 4 in a port */
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
  - {pin_num: '110', peripheral: TMR1, signal: SEC_IN, pin_signal: LCD_P7/PTG0/QTMR0_TMR1/LPTMR0_ALT3}
  - {pin_num: '95', peripheral: TMR0, signal: OUT, pin_signal: LCD_P0/ADC0_SE8/CMP2_IN4/PTF1/QTMR0_TMR0/XBAR_OUT6}
  - {pin_num: '63', peripheral: GPIOJ, signal: 'GPIO, 4', pin_signal: PTJ4/LPUART0_CTS_b}
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
  CLOCK_EnableClock(kCLOCK_PortF);                           /* PCTLF Clock Gate Control: Clock enabled */
  CLOCK_EnableClock(kCLOCK_PortG);                           /* PCTLG Clock Gate Control: Clock enabled */
  CLOCK_EnableClock(kCLOCK_PortI);                           /* PCTLI Clock Gate Control: Clock enabled */
  CLOCK_EnableClock(kCLOCK_PortJ);                           /* PCTLJ Clock Gate Control: Clock enabled */

  PORT_SetPinMux(PORTF, PIN1_IDX, kPORT_MuxAlt2);            /* PORTF1 (pin 95) is configured as QTMR0_TMR0 */
  PORT_SetPinMux(PORTG, PIN0_IDX, kPORT_MuxAlt2);            /* PORTG0 (pin 110) is configured as QTMR0_TMR1 */
  PORT_SetPinMux(PORTI, PIN6_IDX, kPORT_MuxAlt2);            /* PORTI6 (pin 6) is configured as UART2_RX */
  PORT_SetPinMux(PORTI, PIN7_IDX, kPORT_MuxAlt2);            /* PORTI7 (pin 7) is configured as UART2_TX */
  PORT_SetPinMux(PORTJ, PIN4_IDX, kPORT_MuxAsGpio);          /* PORTJ4 (pin 63) is configured as PTJ4 */
  SIM->MISC_CTL = ((SIM->MISC_CTL &
    (~(SIM_MISC_CTL_UART2IRSEL_MASK | SIM_MISC_CTL_TMR1SCSEL_MASK))) /* Mask bits to zero which are setting */
      | SIM_MISC_CTL_UART2IRSEL(MISC_CTL_UART2IRSEL_NONE)    /* UART2 IrDA Select: Pad RX input PTI[6] or PTE[6] selected for RX input of UART2 and UART2 TX signal is not used for modulation */
      | SIM_MISC_CTL_TMR1SCSEL(MISC_CTL_TMR1SCSEL_PTG0_PTC6) /* Quadtimer Channel1 Secondary Count source Select: Pad PTG0 or PTC6, depending upon PCTL configuration. */
    );
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
