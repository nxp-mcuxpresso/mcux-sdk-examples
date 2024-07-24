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

  - {pin_num: '13', peripheral: LPUART0, signal: TX, pin_signal: ADC0_SE8/CMP0_IN3/PTB1/IRQ_6/LPUART0_TX/LPUART0_RX/I2C0_SDA}

  - {pin_num: '14', peripheral: LPUART0, signal: RX, pin_signal: VREF_OUT/CMP0_IN5/PTB2/IRQ_7/LPUART0_RX/LPUART0_TX}

  - {pin_num: '10', peripheral: GPIOB, signal: 'GPIO, 11', pin_signal: PTB11/TPM0_CH0/SPI0_MISO}

  - {pin_num: '20', peripheral: ADC0, signal: 'SE, 0', pin_signal: ADC0_SE0/CMP0_IN0/PTA12/IRQ_13/LPTMR0_ALT2/TPM1_CH0/TPM_CLKIN0/CLKOUT}

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

    /* PORTA12 (pin 20) is configured as ADC0_SE0 */
    PORT_SetPinMux(PORTA, 12U, kPORT_PinDisabledOrAnalog);

    /* PORTB1 (pin 13) is configured as LPUART0_TX */
    PORT_SetPinMux(PORTB, 1U, kPORT_MuxAlt2);

    /* PORTB11 (pin 10) is configured as PTB11 */
    PORT_SetPinMux(PORTB, 11U, kPORT_MuxAsGpio);

    /* PORTB2 (pin 14) is configured as LPUART0_RX */
    PORT_SetPinMux(PORTB, 2U, kPORT_MuxAlt2);

    SIM->SOPT5 = ((SIM->SOPT5 &
                   /* Mask bits to zero which are setting */
                   (~(SIM_SOPT5_LPUART0RXSRC_MASK)))

                  /* LPUART0 Receive Data Source Select: LPUART_RX pin. */
                  | SIM_SOPT5_LPUART0RXSRC(SOPT5_LPUART0RXSRC_LPUART_RX));
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
