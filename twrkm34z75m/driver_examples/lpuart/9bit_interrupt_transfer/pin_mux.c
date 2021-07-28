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
product: Pins v8.0
processor: MKM34Z256xxx7
package_id: MKM34Z256VLQ7
mcu_data: ksdk2_0
processor_version: 8.0.1
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

#include "fsl_common.h"
#include "fsl_port.h"
#include "pin_mux.h"

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
  - {pin_num: '67', peripheral: LPUART0, signal: RX, pin_signal: PTJ6/LLWU_P18/LPUART0_RX}
  - {pin_num: '66', peripheral: LPUART0, signal: TX, pin_signal: PTJ5/LPUART0_TX}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitPins(void)
{
    /* PCTLI Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortI);
    /* PCTLJ Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortJ);

    /* PORTI6 (pin 6) is configured as UART2_RX */
    PORT_SetPinMux(PORTI, 6U, kPORT_MuxAlt2);

    /* PORTI7 (pin 7) is configured as UART2_TX */
    PORT_SetPinMux(PORTI, 7U, kPORT_MuxAlt2);

    /* PORTJ5 (pin 66) is configured as LPUART0_TX */
    PORT_SetPinMux(PORTJ, 5U, kPORT_MuxAlt2);

    /* PORTJ6 (pin 67) is configured as LPUART0_RX */
    PORT_SetPinMux(PORTJ, 6U, kPORT_MuxAlt2);

    SIM->MISC_CTL = ((SIM->MISC_CTL &
                      /* Mask bits to zero which are setting */
                      (~(SIM_MISC_CTL_UART2IRSEL_MASK)))

                     /* UART2 IrDA Select: Pad RX input PTI[6] or PTE[6] selected for RX input of UART2 and UART2
                      * TX signal is not used for modulation. */
                     | SIM_MISC_CTL_UART2IRSEL(MISC_CTL_UART2IRSEL_NONE));
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
