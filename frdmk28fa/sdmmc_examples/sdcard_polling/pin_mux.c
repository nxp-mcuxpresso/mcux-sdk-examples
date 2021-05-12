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
processor: MK28FN2M0Axxx15
package_id: MK28FN2M0AVMI15
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
  - {pin_num: F13, peripheral: GPIOB, signal: 'GPIO, 5', pin_signal: PTB5/FTM2_FLT0, slew_rate: fast, open_drain: disable, pull_select: up, pull_enable: enable}
  - {pin_num: K11, peripheral: SDHC, signal: 'D, 1', pin_signal: PTA24/LPUART2_TX/SDHC0_D1/FB_A15/SDRAM_D15/FB_A29/I2S1_TX_BCLK, slew_rate: fast, open_drain: disable,
    pull_select: up, pull_enable: enable}
  - {pin_num: J11, peripheral: SDHC, signal: 'D, 0', pin_signal: PTA25/LPUART2_RX/SDHC0_D0/FB_A14/SDRAM_D14/FB_A28/I2S1_TX_FS, slew_rate: fast, open_drain: disable,
    pull_select: up, pull_enable: enable}
  - {pin_num: J10, peripheral: SDHC, signal: DCLK, pin_signal: PTA26/LPUART2_CTS_b/SDHC0_DCLK/FB_A13/SDRAM_D13/FB_A27/I2S1_TXD0, slew_rate: fast, open_drain: disable,
    pull_select: up, pull_enable: enable}
  - {pin_num: H13, peripheral: SDHC, signal: CMD, pin_signal: PTA27/LPUART2_RTS_b/SDHC0_CMD/FB_A12/SDRAM_D12/FB_A26/I2S1_TXD1, slew_rate: fast, open_drain: disable,
    pull_select: up, pull_enable: enable}
  - {pin_num: H12, peripheral: SDHC, signal: 'D, 3', pin_signal: PTA28/LPUART3_TX/SDHC0_D3/FB_A25/I2S1_RXD1, slew_rate: fast, open_drain: disable, pull_select: up,
    pull_enable: enable}
  - {pin_num: H11, peripheral: SDHC, signal: 'D, 2', pin_signal: PTA29/LPUART3_RX/SDHC0_D2/FB_A24/I2S1_RXD0, slew_rate: fast, open_drain: disable, pull_select: up,
    pull_enable: enable}
  - {pin_num: B7, peripheral: LPUART0, signal: TX, pin_signal: PTC24/LPUART0_TX/FB_A5/SDRAM_D5/QSPI0A_DATA3}
  - {pin_num: A7, peripheral: LPUART0, signal: RX, pin_signal: PTC25/LPUART0_RX/FB_A4/SDRAM_D4/QSPI0A_SCLK}
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
    /* Port C Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortC);

    const port_pin_config_t porta24_pinK11_config = {/* Internal pull-up resistor is enabled */
                                                     kPORT_PullUp,
                                                     /* Fast slew rate is configured */
                                                     kPORT_FastSlewRate,
                                                     /* Passive filter is disabled */
                                                     kPORT_PassiveFilterDisable,
                                                     /* Open drain is disabled */
                                                     kPORT_OpenDrainDisable,
                                                     /* Low drive strength is configured */
                                                     kPORT_LowDriveStrength,
                                                     /* Pin is configured as SDHC0_D1 */
                                                     kPORT_MuxAlt4,
                                                     /* Pin Control Register fields [15:0] are not locked */
                                                     kPORT_UnlockRegister};
    /* PORTA24 (pin K11) is configured as SDHC0_D1 */
    PORT_SetPinConfig(PORTA, 24U, &porta24_pinK11_config);

    const port_pin_config_t porta25_pinJ11_config = {/* Internal pull-up resistor is enabled */
                                                     kPORT_PullUp,
                                                     /* Fast slew rate is configured */
                                                     kPORT_FastSlewRate,
                                                     /* Passive filter is disabled */
                                                     kPORT_PassiveFilterDisable,
                                                     /* Open drain is disabled */
                                                     kPORT_OpenDrainDisable,
                                                     /* Low drive strength is configured */
                                                     kPORT_LowDriveStrength,
                                                     /* Pin is configured as SDHC0_D0 */
                                                     kPORT_MuxAlt4,
                                                     /* Pin Control Register fields [15:0] are not locked */
                                                     kPORT_UnlockRegister};
    /* PORTA25 (pin J11) is configured as SDHC0_D0 */
    PORT_SetPinConfig(PORTA, 25U, &porta25_pinJ11_config);

    const port_pin_config_t porta26_pinJ10_config = {/* Internal pull-up resistor is enabled */
                                                     kPORT_PullUp,
                                                     /* Fast slew rate is configured */
                                                     kPORT_FastSlewRate,
                                                     /* Passive filter is disabled */
                                                     kPORT_PassiveFilterDisable,
                                                     /* Open drain is disabled */
                                                     kPORT_OpenDrainDisable,
                                                     /* Low drive strength is configured */
                                                     kPORT_LowDriveStrength,
                                                     /* Pin is configured as SDHC0_DCLK */
                                                     kPORT_MuxAlt4,
                                                     /* Pin Control Register fields [15:0] are not locked */
                                                     kPORT_UnlockRegister};
    /* PORTA26 (pin J10) is configured as SDHC0_DCLK */
    PORT_SetPinConfig(PORTA, 26U, &porta26_pinJ10_config);

    const port_pin_config_t porta27_pinH13_config = {/* Internal pull-up resistor is enabled */
                                                     kPORT_PullUp,
                                                     /* Fast slew rate is configured */
                                                     kPORT_FastSlewRate,
                                                     /* Passive filter is disabled */
                                                     kPORT_PassiveFilterDisable,
                                                     /* Open drain is disabled */
                                                     kPORT_OpenDrainDisable,
                                                     /* Low drive strength is configured */
                                                     kPORT_LowDriveStrength,
                                                     /* Pin is configured as SDHC0_CMD */
                                                     kPORT_MuxAlt4,
                                                     /* Pin Control Register fields [15:0] are not locked */
                                                     kPORT_UnlockRegister};
    /* PORTA27 (pin H13) is configured as SDHC0_CMD */
    PORT_SetPinConfig(PORTA, 27U, &porta27_pinH13_config);

    const port_pin_config_t porta28_pinH12_config = {/* Internal pull-up resistor is enabled */
                                                     kPORT_PullUp,
                                                     /* Fast slew rate is configured */
                                                     kPORT_FastSlewRate,
                                                     /* Passive filter is disabled */
                                                     kPORT_PassiveFilterDisable,
                                                     /* Open drain is disabled */
                                                     kPORT_OpenDrainDisable,
                                                     /* Low drive strength is configured */
                                                     kPORT_LowDriveStrength,
                                                     /* Pin is configured as SDHC0_D3 */
                                                     kPORT_MuxAlt4,
                                                     /* Pin Control Register fields [15:0] are not locked */
                                                     kPORT_UnlockRegister};
    /* PORTA28 (pin H12) is configured as SDHC0_D3 */
    PORT_SetPinConfig(PORTA, 28U, &porta28_pinH12_config);

    const port_pin_config_t porta29_pinH11_config = {/* Internal pull-up resistor is enabled */
                                                     kPORT_PullUp,
                                                     /* Fast slew rate is configured */
                                                     kPORT_FastSlewRate,
                                                     /* Passive filter is disabled */
                                                     kPORT_PassiveFilterDisable,
                                                     /* Open drain is disabled */
                                                     kPORT_OpenDrainDisable,
                                                     /* Low drive strength is configured */
                                                     kPORT_LowDriveStrength,
                                                     /* Pin is configured as SDHC0_D2 */
                                                     kPORT_MuxAlt4,
                                                     /* Pin Control Register fields [15:0] are not locked */
                                                     kPORT_UnlockRegister};
    /* PORTA29 (pin H11) is configured as SDHC0_D2 */
    PORT_SetPinConfig(PORTA, 29U, &porta29_pinH11_config);

    const port_pin_config_t portb5_pinF13_config = {/* Internal pull-up resistor is enabled */
                                                    kPORT_PullUp,
                                                    /* Fast slew rate is configured */
                                                    kPORT_FastSlewRate,
                                                    /* Passive filter is disabled */
                                                    kPORT_PassiveFilterDisable,
                                                    /* Open drain is disabled */
                                                    kPORT_OpenDrainDisable,
                                                    /* Low drive strength is configured */
                                                    kPORT_LowDriveStrength,
                                                    /* Pin is configured as PTB5 */
                                                    kPORT_MuxAsGpio,
                                                    /* Pin Control Register fields [15:0] are not locked */
                                                    kPORT_UnlockRegister};
    /* PORTB5 (pin F13) is configured as PTB5 */
    PORT_SetPinConfig(PORTB, 5U, &portb5_pinF13_config);

    /* PORTC24 (pin B7) is configured as LPUART0_TX */
    PORT_SetPinMux(PORTC, 24U, kPORT_MuxAlt3);

    /* PORTC25 (pin A7) is configured as LPUART0_RX */
    PORT_SetPinMux(PORTC, 25U, kPORT_MuxAlt3);

    SIM->SOPT5 = ((SIM->SOPT5 &
                   /* Mask bits to zero which are setting */
                   (~(SIM_SOPT5_LPUART0TXSRC_MASK | SIM_SOPT5_LPUART0RXSRC_MASK)))

                  /* LPUART0 transmit data source select: LPUART0_TX pin. */
                  | SIM_SOPT5_LPUART0TXSRC(SOPT5_LPUART0TXSRC_LPUART_TX)

                  /* LPUART0 receive data source select: LPUART0_RX pin. */
                  | SIM_SOPT5_LPUART0RXSRC(SOPT5_LPUART0RXSRC_LPUART_RX));
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
