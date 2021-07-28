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
processor: MKV11Z128xxx7
package_id: MKV11Z128VLF7
mcu_data: ksdk2_0
processor_version: 8.0.1
board: HVP-KV11Z75M
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
- options: {callFromInitBoot: 'true', prefix: BOARD_, coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '41', peripheral: UART1, signal: RX, pin_signal: PTD0/LLWU_P12/SPI0_PCS0/UART0_CTS_b/FTM0_CH0/UART1_RX/FTM3_CH0}
  - {pin_num: '42', peripheral: UART1, signal: TX, pin_signal: ADC0_SE2/PTD1/SPI0_SCK/UART0_RTS_b/FTM0_CH1/UART1_TX/FTM3_CH1, direction: OUTPUT}
  - {pin_num: '31', peripheral: UART0, signal: RX, pin_signal: PTB16/UART0_RX/FTM_CLKIN2/CAN0_TX/EWM_IN}
  - {pin_num: '32', peripheral: UART0, signal: TX, pin_signal: PTB17/UART0_TX/FTM_CLKIN1/CAN0_RX/EWM_OUT_b}
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
    /* Port D Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortD);

    /* PORTB16 (pin 31) is configured as UART0_RX */
    PORT_SetPinMux(BOARD_RXD_PORT, BOARD_RXD_PIN, kPORT_MuxAlt3);

    /* PORTB17 (pin 32) is configured as UART0_TX */
    PORT_SetPinMux(BOARD_TXD_PORT, BOARD_TXD_PIN, kPORT_MuxAlt3);

    /* PORTD0 (pin 41) is configured as UART1_RX */
    PORT_SetPinMux(PORTD, 0U, kPORT_MuxAlt5);

    /* PORTD1 (pin 42) is configured as UART1_TX */
    PORT_SetPinMux(PORTD, 1U, kPORT_MuxAlt5);

    SIM->SOPT5 =
        ((SIM->SOPT5 &
          /* Mask bits to zero which are setting */
          (~(SIM_SOPT5_UART0TXSRC_MASK | SIM_SOPT5_UART0RXSRC_MASK | SIM_SOPT5_UART1TXSRC_MASK | SIM_SOPT5_UART1RXSRC_MASK)))

         /* UART 0 Transmit Data Source Select: UART0_TX pin. */
         | SIM_SOPT5_UART0TXSRC(SOPT5_UART0TXSRC_UART_TX)

         /* UART 0 Receive Data Source Select: UART0_RX pin. */
         | SIM_SOPT5_UART0RXSRC(SOPT5_UART0RXSRC_UART_RX)

         /* UART 1 Transmit Data Source Select: UART1_TX pin. */
         | SIM_SOPT5_UART1TXSRC(SOPT5_UART1TXSRC_UART_TX)

         /* UART 1 Receive Data Source Select: UART1_RX pin. */
         | SIM_SOPT5_UART1RXSRC(SOPT5_UART1RXSRC_UART_RX));
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
