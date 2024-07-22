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
    /* Port B Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortB);

    /* PORTB1 (pin 13) is configured as LPUART0_TX */
    PORT_SetPinMux(PORTB, 1U, kPORT_MuxAlt2);

    /* PORTB2 (pin 14) is configured as LPUART0_RX */
    PORT_SetPinMux(PORTB, 2U, kPORT_MuxAlt2);

    SIM->SOPT5 = ((SIM->SOPT5 &
                   /* Mask bits to zero which are setting */
                   (~(SIM_SOPT5_LPUART0RXSRC_MASK)))

                  /* LPUART0 Receive Data Source Select: LPUART_RX pin. */
                  | SIM_SOPT5_LPUART0RXSRC(SOPT5_LPUART0RXSRC_LPUART_RX));
}

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************

SPI0_InitPins:

- options: {coreID: core0, enableClock: 'true'}

- pin_list:

  - {pin_num: '7', peripheral: SPI0, signal: PCS0_SS, pin_signal: PTA5/RTC_CLKIN/TPM0_CH1/SPI0_SS_b}

  - {pin_num: '8', peripheral: SPI0, signal: MISO, pin_signal: PTA6/TPM0_CH0/SPI0_MISO}

  - {pin_num: '11', peripheral: SPI0, signal: MOSI, pin_signal: PTA7/IRQ_4/SPI0_MISO/SPI0_MOSI}

  - {pin_num: '12', peripheral: SPI0, signal: SCK, pin_signal: ADC0_SE9/PTB0/IRQ_5/LLWU_P4/EXTRG_IN/SPI0_SCK/I2C0_SCL}

 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : SPI0_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void SPI0_InitPins(void)
{
    /* Port A Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortA);
    /* Port B Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortB);

    /* PORTA5 (pin 7) is configured as SPI0_SS_b */
    PORT_SetPinMux(PORTA, 5U, kPORT_MuxAlt3);

    /* PORTA6 (pin 8) is configured as SPI0_MISO */
    PORT_SetPinMux(PORTA, 6U, kPORT_MuxAlt3);

    /* PORTA7 (pin 11) is configured as SPI0_MOSI */
    PORT_SetPinMux(PORTA, 7U, kPORT_MuxAlt3);

    /* PORTB0 (pin 12) is configured as SPI0_SCK */
    PORT_SetPinMux(PORTB, 0U, kPORT_MuxAlt3);
}

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************

SPI0_DeinitPins:

- options: {coreID: core0, enableClock: 'true'}

- pin_list:

  - {pin_num: '7', peripheral: n/a, signal: disabled, pin_signal: PTA5/RTC_CLKIN/TPM0_CH1/SPI0_SS_b}

  - {pin_num: '8', peripheral: n/a, signal: disabled, pin_signal: PTA6/TPM0_CH0/SPI0_MISO}

  - {pin_num: '11', peripheral: n/a, signal: disabled, pin_signal: PTA7/IRQ_4/SPI0_MISO/SPI0_MOSI}

  - {pin_num: '12', peripheral: ADC0, signal: 'SE, 9', pin_signal: ADC0_SE9/PTB0/IRQ_5/LLWU_P4/EXTRG_IN/SPI0_SCK/I2C0_SCL}

 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : SPI0_DeinitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void SPI0_DeinitPins(void)
{
    /* Port A Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortA);
    /* Port B Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortB);

    /* PORTA5 (pin 7) is disabled */
    PORT_SetPinMux(PORTA, 5U, kPORT_PinDisabledOrAnalog);

    /* PORTA6 (pin 8) is disabled */
    PORT_SetPinMux(PORTA, 6U, kPORT_PinDisabledOrAnalog);

    /* PORTA7 (pin 11) is disabled */
    PORT_SetPinMux(PORTA, 7U, kPORT_PinDisabledOrAnalog);

    /* PORTB0 (pin 12) is configured as ADC0_SE9 */
    PORT_SetPinMux(PORTB, 0U, kPORT_PinDisabledOrAnalog);
}

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************

SPI1_InitPins:

- options: {coreID: core0, enableClock: 'true'}

- pin_list: []

 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : SPI1_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void SPI1_InitPins(void)
{
}

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************

SPI1_DeinitPins:

- options: {coreID: core0, enableClock: 'true'}

- pin_list: []

 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : SPI1_DeinitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void SPI1_DeinitPins(void)
{
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
