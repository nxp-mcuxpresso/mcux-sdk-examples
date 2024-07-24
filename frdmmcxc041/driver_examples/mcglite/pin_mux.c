/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************

!!GlobalInfo

product: Pins v3.0

processor: MCXC041

package_id: MCXC041VFK

mcu_data: ksdk2_0

processor_version: 0.0.8

 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

#include "fsl_common.h"
#include "fsl_port.h"
#include "pin_mux.h"



/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************

BOARD_InitPins:

- options: {callFromInitBoot: 'true', coreID: core0, enableClock: 'true'}

- pin_list:

  - {pin_num: '9', peripheral: GPIOB, signal: 'GPIO, 10', pin_signal: PTB10/TPM0_CH1/SPI0_SS_b}

  - {pin_num: '5', peripheral: OSC, signal: EXTAL, pin_signal: EXTAL0/PTA3/I2C0_SCL/I2C0_SDA/LPUART0_TX}

  - {pin_num: '6', peripheral: OSC, signal: XTAL, pin_signal: XTAL0/PTA4/I2C0_SDA/I2C0_SCL/LPUART0_RX/CLKOUT}

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
    /* Port A Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortA);
    /* Port B Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortB);

    /* PORTA3 (pin 5) is configured as EXTAL0 */
    PORT_SetPinMux(PORTA, 3U, kPORT_PinDisabledOrAnalog);

    /* PORTA4 (pin 6) is configured as XTAL0 */
    PORT_SetPinMux(PORTA, 4U, kPORT_PinDisabledOrAnalog);

    /* PORTB10 (pin 9) is configured as PTB10 */
    PORT_SetPinMux(PORTB, 10U, kPORT_MuxAsGpio);
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
