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
product: Pins v8.0
processor: MCXC242
package_id: MCXC242VLH
mcu_data: ksdk2_0
processor_version: 8.0.1
board: FRDM-MCXC242
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
  - {pin_num: '23', peripheral: LPUART0, signal: RX, pin_signal: PTA1/LPUART0_RX/TPM2_CH0}
  - {pin_num: '24', peripheral: LPUART0, signal: TX, pin_signal: PTA2/LPUART0_TX/TPM2_CH1}
  - {pin_num: '2', peripheral: LPUART1, signal: RX, pin_signal: PTE1/SPI1_MOSI/LPUART1_RX/SPI1_MISO/I2C1_SCL}
  - {pin_num: '1', peripheral: LPUART1, signal: TX, pin_signal: PTE0/CLKOUT32K/SPI1_MISO/LPUART1_TX/RTC_CLKOUT/CMP0_OUT/I2C1_SDA}
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
    /* Port E Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortE);

    /* PORTA1 (pin 23) is configured as LPUART0_RX */
    PORT_SetPinMux(BOARD_DEBUG_UART0_RX_PORT, BOARD_DEBUG_UART0_RX_PIN, kPORT_MuxAlt2);

    /* PORTA2 (pin 24) is configured as LPUART0_TX */
    PORT_SetPinMux(BOARD_DEBUG_UART0_TX_PORT, BOARD_DEBUG_UART0_TX_PIN, kPORT_MuxAlt2);

    /* PORTE0 (pin 1) is configured as LPUART1_TX */
    PORT_SetPinMux(BOARD_CLKOUT32K_PORT, BOARD_CLKOUT32K_PIN, kPORT_MuxAlt3);

    /* PORTE1 (pin 2) is configured as LPUART1_RX */
    PORT_SetPinMux(PORTE, 1U, kPORT_MuxAlt3);

    SIM->SOPT5 =
        ((SIM->SOPT5 &
          /* Mask bits to zero which are setting */
          (~(SIM_SOPT5_LPUART0TXSRC_MASK | SIM_SOPT5_LPUART0RXSRC_MASK | SIM_SOPT5_LPUART1TXSRC_MASK | SIM_SOPT5_LPUART1RXSRC_MASK)))

         /* LPUART0 Transmit Data Source Select: LPUART0_TX pin. */
         | SIM_SOPT5_LPUART0TXSRC(SOPT5_LPUART0TXSRC_LPUART_TX)

         /* LPUART0 Receive Data Source Select: LPUART_RX pin. */
         | SIM_SOPT5_LPUART0RXSRC(SOPT5_LPUART0RXSRC_LPUART_RX)

         /* LPUART1 Transmit Data Source Select: LPUART1_TX pin. */
         | SIM_SOPT5_LPUART1TXSRC(SOPT5_LPUART1TXSRC_LPUART_TX)

         /* LPUART1 Receive Data Source Select: LPUART1_RX pin. */
         | SIM_SOPT5_LPUART1RXSRC(SOPT5_LPUART1RXSRC_LPUART_RX));
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
