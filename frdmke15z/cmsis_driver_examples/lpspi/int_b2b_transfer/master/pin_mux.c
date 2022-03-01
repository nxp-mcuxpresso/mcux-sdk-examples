/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Pins v3.0
processor: MKE15Z256xxx7
package_id: MKE15Z256VLL7
mcu_data: ksdk2_0
processor_version: 0.0.9
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
- options: {callFromInitBoot: 'true', prefix: BOARD_, coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '81', peripheral: LPUART1, signal: RX, pin_signal: ADC1_SE4/TSI0_CH15/PTC6/LPUART1_RX}
  - {pin_num: '80', peripheral: LPUART1, signal: TX, pin_signal: ADC1_SE5/TSI0_CH16/PTC7/LPUART1_TX}
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
    /* Clock Control: 0x01u */
    CLOCK_EnableClock(kCLOCK_PortC);

    /* PORTC6 (pin 81) is configured as LPUART1_RX */
    PORT_SetPinMux(PORTC, 6U, kPORT_MuxAlt2);

    /* PORTC7 (pin 80) is configured as LPUART1_TX */
    PORT_SetPinMux(PORTC, 7U, kPORT_MuxAlt2);
}

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
LPSPI0_InitPins:
- options: {coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '85', peripheral: LPSPI0, signal: SOUT, pin_signal: ADC1_SE10/TSI0_CH19/PTE2/LPSPI0_SOUT/LPTMR0_ALT3/PWT_IN3/LPUART1_CTS}
  - {pin_num: '93', peripheral: LPSPI0, signal: SIN, pin_signal: TSI0_CH14/PTE1/LPSPI0_SIN/LPI2C0_HREQ/LPI2C1_SCL}
  - {pin_num: '94', peripheral: LPSPI0, signal: SCK, pin_signal: TSI0_CH13/PTE0/LPSPI0_SCK/TCLK1/LPI2C1_SDA/FTM1_FLT2}
  - {pin_num: '83', peripheral: LPSPI0, signal: PCS3, pin_signal: PTA15/FTM1_CH2/LPSPI0_PCS3}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : LPSPI0_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void LPSPI0_InitPins(void)
{
    /* Clock Control: 0x01u */
    CLOCK_EnableClock(kCLOCK_PortA);
    /* Clock Control: 0x01u */
    CLOCK_EnableClock(kCLOCK_PortE);

    /* PORTA15 (pin 83) is configured as LPSPI0_PCS3 */
    PORT_SetPinMux(PORTA, 15U, kPORT_MuxAlt3);

    /* PORTE0 (pin 94) is configured as LPSPI0_SCK */
    PORT_SetPinMux(PORTE, 0U, kPORT_MuxAlt2);

    /* PORTE1 (pin 93) is configured as LPSPI0_SIN */
    PORT_SetPinMux(PORTE, 1U, kPORT_MuxAlt2);

    /* PORTE2 (pin 85) is configured as LPSPI0_SOUT */
    PORT_SetPinMux(PORTE, 2U, kPORT_MuxAlt2);
}

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
LPSPI0_DeinitPins:
- options: {coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '83', peripheral: n/a, signal: disabled, pin_signal: PTA15/FTM1_CH2/LPSPI0_PCS3}
  - {pin_num: '85', peripheral: ADC1, signal: 'SE, 10', pin_signal: ADC1_SE10/TSI0_CH19/PTE2/LPSPI0_SOUT/LPTMR0_ALT3/PWT_IN3/LPUART1_CTS}
  - {pin_num: '93', peripheral: TSI, signal: 'CH, 14', pin_signal: TSI0_CH14/PTE1/LPSPI0_SIN/LPI2C0_HREQ/LPI2C1_SCL}
  - {pin_num: '94', peripheral: TSI, signal: 'CH, 13', pin_signal: TSI0_CH13/PTE0/LPSPI0_SCK/TCLK1/LPI2C1_SDA/FTM1_FLT2}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : LPSPI0_DeinitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void LPSPI0_DeinitPins(void)
{
    /* Clock Control: 0x01u */
    CLOCK_EnableClock(kCLOCK_PortA);
    /* Clock Control: 0x01u */
    CLOCK_EnableClock(kCLOCK_PortE);

    /* PORTA15 (pin 83) is disabled */
    PORT_SetPinMux(PORTA, 15U, kPORT_PinDisabledOrAnalog);

    /* PORTE0 (pin 94) is configured as TSI0_CH13 */
    PORT_SetPinMux(PORTE, 0U, kPORT_PinDisabledOrAnalog);

    /* PORTE1 (pin 93) is configured as TSI0_CH14 */
    PORT_SetPinMux(PORTE, 1U, kPORT_PinDisabledOrAnalog);

    /* PORTE2 (pin 85) is configured as ADC1_SE10 */
    PORT_SetPinMux(PORTE, 2U, kPORT_PinDisabledOrAnalog);
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
