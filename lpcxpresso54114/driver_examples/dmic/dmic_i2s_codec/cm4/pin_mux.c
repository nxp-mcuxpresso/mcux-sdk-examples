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
processor: LPC54114J256
package_id: LPC54114J256BD64
mcu_data: ksdk2_0
processor_version: 0.0.13
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

#include "fsl_common.h"
#include "fsl_iocon.h"
#include "pin_mux.h"



/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPins:
- options: {callFromInitBoot: 'true', coreID: cm4, enableClock: 'true'}
- pin_list:
  - {pin_num: '31', peripheral: FLEXCOMM0, signal: RXD_SDA_MOSI, pin_signal: PIO0_0/FC0_RXD_SDA_MOSI/FC3_CTS_SDA_SSEL0/CTIMER0_CAP0/SCT0_OUT3, mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: standard, open_drain: disabled}
  - {pin_num: '32', peripheral: FLEXCOMM0, signal: TXD_SCL_MISO, pin_signal: PIO0_1/FC0_TXD_SCL_MISO/FC3_RTS_SCL_SSEL1/CTIMER0_CAP1/SCT0_OUT1, mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: standard, open_drain: disabled}
  - {pin_num: '3', peripheral: FLEXCOMM4, signal: RTS_SCL_SSEL1, pin_signal: PIO0_25/FC4_RTS_SCL_SSEL1/FC6_CTS_SDA_SSEL0/CTIMER0_CAP2/CTIMER1_CAP1, invert: disabled,
    glitch_filter: disabled, i2c_slew: i2c, i2c_drive: high, i2c_filter: disabled}
  - {pin_num: '4', peripheral: FLEXCOMM4, signal: CTS_SDA_SSEL0, pin_signal: PIO0_26/FC4_CTS_SDA_SSEL0/CTIMER0_CAP3, invert: disabled, glitch_filter: disabled, i2c_slew: i2c,
    i2c_drive: high, i2c_filter: disabled}
  - {pin_num: '39', peripheral: FLEXCOMM6, signal: RXD_SDA_MOSI_DATA, pin_signal: PIO0_5/FC6_RXD_SDA_MOSI_DATA/SCT0_OUT6/CTIMER0_MAT0, mode: pullUp, invert: disabled,
    glitch_filter: disabled, slew_rate: standard, open_drain: disabled}
  - {pin_num: '40', peripheral: FLEXCOMM6, signal: TXD_SCL_MISO_WS, pin_signal: PIO0_6/FC6_TXD_SCL_MISO_WS/CTIMER0_MAT1/UTICK_CAP0, mode: pullUp, invert: disabled,
    glitch_filter: disabled, slew_rate: standard, open_drain: disabled}
  - {pin_num: '41', peripheral: FLEXCOMM6, signal: SCK, pin_signal: PIO0_7/FC6_SCK/SCT0_OUT0/CTIMER0_MAT2/CTIMER0_CAP2, mode: pullUp, invert: disabled, glitch_filter: disabled,
    slew_rate: standard, open_drain: disabled}
  - {pin_num: '28', peripheral: FLEXCOMM7, signal: TXD_SCL_MISO_WS, pin_signal: PIO1_8/FC7_TXD_SCL_MISO_WS/CTIMER1_MAT3/CTIMER1_CAP3/ADC0_11, mode: pullUp, invert: disabled,
    glitch_filter: disabled, open_drain: disabled}
  - {pin_num: '51', peripheral: FLEXCOMM7, signal: SCK, pin_signal: PIO1_12/FC5_RXD_SDA_MOSI/CTIMER1_MAT0/FC7_SCK/UTICK_CAP2, mode: pullUp, invert: disabled, glitch_filter: disabled,
    slew_rate: standard, open_drain: disabled}
  - {pin_num: '54', peripheral: FLEXCOMM7, signal: RXD_SDA_MOSI_DATA, pin_signal: PIO1_13/FC5_TXD_SCL_MISO/CTIMER1_MAT1/FC7_RXD_SDA_MOSI_DATA, mode: pullUp, invert: disabled,
    glitch_filter: disabled, slew_rate: standard, open_drain: disabled}
  - {pin_num: '10', peripheral: SYSCON, signal: MCLK, pin_signal: PIO1_17/MCLK/UTICK_CAP3, mode: inactive, invert: disabled, glitch_filter: disabled, slew_rate: standard,
    open_drain: disabled}
  - {pin_num: '13', peripheral: DMIC0, signal: 'CLK, 0', pin_signal: PIO0_31/PDM0_CLK/FC2_CTS_SDA_SSEL0/CTIMER2_CAP2/CTIMER0_CAP3/CTIMER0_MAT3/ADC0_2, mode: pullUp,
    invert: disabled, glitch_filter: disabled, open_drain: disabled}
  - {pin_num: '14', peripheral: DMIC0, signal: 'DATA, 0', pin_signal: PIO1_0/PDM0_DATA/FC2_RTS_SCL_SSEL1/CTIMER3_MAT1/CTIMER0_CAP0/ADC0_3, mode: pullUp, invert: disabled,
    glitch_filter: disabled, open_drain: disabled}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
/* Function assigned for the undefined */
void BOARD_InitPins(void)
{
    /* Enables the clock for the IOCON block. 0 = Disable; 1 = Enable.: 0x01u */
    CLOCK_EnableClock(kCLOCK_Iocon);

    const uint32_t port0_pin0_config = (/* Pin is configured as FC0_RXD_SDA_MOSI */
                                        IOCON_PIO_FUNC1 |
                                        /* No addition pin function */
                                        IOCON_PIO_MODE_INACT |
                                        /* Input function is not inverted */
                                        IOCON_PIO_INV_DI |
                                        /* Enables digital function */
                                        IOCON_PIO_DIGITAL_EN |
                                        /* Input filter disabled */
                                        IOCON_PIO_INPFILT_OFF |
                                        /* Standard mode, output slew rate control is enabled */
                                        IOCON_PIO_SLEW_STANDARD |
                                        /* Open drain is disabled */
                                        IOCON_PIO_OPENDRAIN_DI);
    /* PORT0 PIN0 (coords: 31) is configured as FC0_RXD_SDA_MOSI */
    IOCON_PinMuxSet(IOCON, 0U, 0U, port0_pin0_config);

    const uint32_t port0_pin1_config = (/* Pin is configured as FC0_TXD_SCL_MISO */
                                        IOCON_PIO_FUNC1 |
                                        /* No addition pin function */
                                        IOCON_PIO_MODE_INACT |
                                        /* Input function is not inverted */
                                        IOCON_PIO_INV_DI |
                                        /* Enables digital function */
                                        IOCON_PIO_DIGITAL_EN |
                                        /* Input filter disabled */
                                        IOCON_PIO_INPFILT_OFF |
                                        /* Standard mode, output slew rate control is enabled */
                                        IOCON_PIO_SLEW_STANDARD |
                                        /* Open drain is disabled */
                                        IOCON_PIO_OPENDRAIN_DI);
    /* PORT0 PIN1 (coords: 32) is configured as FC0_TXD_SCL_MISO */
    IOCON_PinMuxSet(IOCON, 0U, 1U, port0_pin1_config);

    const uint32_t port0_pin25_config = (/* Pin is configured as FC4_RTS_SCL_SSEL1 */
                                         IOCON_PIO_FUNC1 |
                                         /* I2C mode */
                                         IOCON_PIO_I2CSLEW_I2C |
                                         /* Input function is not inverted */
                                         IOCON_PIO_INV_DI |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Input filter disabled */
                                         IOCON_PIO_INPFILT_OFF |
                                         /* High drive: 20 mA */
                                         IOCON_PIO_I2CDRIVE_HIGH |
                                         /* I2C 50 ns glitch filter disabled */
                                         IOCON_PIO_I2CFILTER_DI);
    /* PORT0 PIN25 (coords: 3) is configured as FC4_RTS_SCL_SSEL1 */
    IOCON_PinMuxSet(IOCON, 0U, 25U, port0_pin25_config);

    const uint32_t port0_pin26_config = (/* Pin is configured as FC4_CTS_SDA_SSEL0 */
                                         IOCON_PIO_FUNC1 |
                                         /* I2C mode */
                                         IOCON_PIO_I2CSLEW_I2C |
                                         /* Input function is not inverted */
                                         IOCON_PIO_INV_DI |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Input filter disabled */
                                         IOCON_PIO_INPFILT_OFF |
                                         /* High drive: 20 mA */
                                         IOCON_PIO_I2CDRIVE_HIGH |
                                         /* I2C 50 ns glitch filter disabled */
                                         IOCON_PIO_I2CFILTER_DI);
    /* PORT0 PIN26 (coords: 4) is configured as FC4_CTS_SDA_SSEL0 */
    IOCON_PinMuxSet(IOCON, 0U, 26U, port0_pin26_config);

    const uint32_t port0_pin31_config = (/* Pin is configured as PDM0_CLK */
                                         IOCON_PIO_FUNC1 |
                                         /* Selects pull-up function */
                                         IOCON_PIO_MODE_PULLUP |
                                         /* Input function is not inverted */
                                         IOCON_PIO_INV_DI |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Input filter disabled */
                                         IOCON_PIO_INPFILT_OFF |
                                         /* Open drain is disabled */
                                         IOCON_PIO_OPENDRAIN_DI);
    /* PORT0 PIN31 (coords: 13) is configured as PDM0_CLK */
    IOCON_PinMuxSet(IOCON, 0U, 31U, port0_pin31_config);

    const uint32_t port0_pin5_config = (/* Pin is configured as FC6_RXD_SDA_MOSI_DATA */
                                        IOCON_PIO_FUNC1 |
                                        /* Selects pull-up function */
                                        IOCON_PIO_MODE_PULLUP |
                                        /* Input function is not inverted */
                                        IOCON_PIO_INV_DI |
                                        /* Enables digital function */
                                        IOCON_PIO_DIGITAL_EN |
                                        /* Input filter disabled */
                                        IOCON_PIO_INPFILT_OFF |
                                        /* Standard mode, output slew rate control is enabled */
                                        IOCON_PIO_SLEW_STANDARD |
                                        /* Open drain is disabled */
                                        IOCON_PIO_OPENDRAIN_DI);
    /* PORT0 PIN5 (coords: 39) is configured as FC6_RXD_SDA_MOSI_DATA */
    IOCON_PinMuxSet(IOCON, 0U, 5U, port0_pin5_config);

    const uint32_t port0_pin6_config = (/* Pin is configured as FC6_TXD_SCL_MISO_WS */
                                        IOCON_PIO_FUNC1 |
                                        /* Selects pull-up function */
                                        IOCON_PIO_MODE_PULLUP |
                                        /* Input function is not inverted */
                                        IOCON_PIO_INV_DI |
                                        /* Enables digital function */
                                        IOCON_PIO_DIGITAL_EN |
                                        /* Input filter disabled */
                                        IOCON_PIO_INPFILT_OFF |
                                        /* Standard mode, output slew rate control is enabled */
                                        IOCON_PIO_SLEW_STANDARD |
                                        /* Open drain is disabled */
                                        IOCON_PIO_OPENDRAIN_DI);
    /* PORT0 PIN6 (coords: 40) is configured as FC6_TXD_SCL_MISO_WS */
    IOCON_PinMuxSet(IOCON, 0U, 6U, port0_pin6_config);

    const uint32_t port0_pin7_config = (/* Pin is configured as FC6_SCK */
                                        IOCON_PIO_FUNC1 |
                                        /* Selects pull-up function */
                                        IOCON_PIO_MODE_PULLUP |
                                        /* Input function is not inverted */
                                        IOCON_PIO_INV_DI |
                                        /* Enables digital function */
                                        IOCON_PIO_DIGITAL_EN |
                                        /* Input filter disabled */
                                        IOCON_PIO_INPFILT_OFF |
                                        /* Standard mode, output slew rate control is enabled */
                                        IOCON_PIO_SLEW_STANDARD |
                                        /* Open drain is disabled */
                                        IOCON_PIO_OPENDRAIN_DI);
    /* PORT0 PIN7 (coords: 41) is configured as FC6_SCK */
    IOCON_PinMuxSet(IOCON, 0U, 7U, port0_pin7_config);

    const uint32_t port1_pin0_config = (/* Pin is configured as PDM0_DATA */
                                        IOCON_PIO_FUNC1 |
                                        /* Selects pull-up function */
                                        IOCON_PIO_MODE_PULLUP |
                                        /* Input function is not inverted */
                                        IOCON_PIO_INV_DI |
                                        /* Enables digital function */
                                        IOCON_PIO_DIGITAL_EN |
                                        /* Input filter disabled */
                                        IOCON_PIO_INPFILT_OFF |
                                        /* Open drain is disabled */
                                        IOCON_PIO_OPENDRAIN_DI);
    /* PORT1 PIN0 (coords: 14) is configured as PDM0_DATA */
    IOCON_PinMuxSet(IOCON, 1U, 0U, port1_pin0_config);

    const uint32_t port1_pin12_config = (/* Pin is configured as FC7_SCK */
                                         IOCON_PIO_FUNC4 |
                                         /* Selects pull-up function */
                                         IOCON_PIO_MODE_PULLUP |
                                         /* Input function is not inverted */
                                         IOCON_PIO_INV_DI |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Input filter disabled */
                                         IOCON_PIO_INPFILT_OFF |
                                         /* Standard mode, output slew rate control is enabled */
                                         IOCON_PIO_SLEW_STANDARD |
                                         /* Open drain is disabled */
                                         IOCON_PIO_OPENDRAIN_DI);
    /* PORT1 PIN12 (coords: 51) is configured as FC7_SCK */
    IOCON_PinMuxSet(IOCON, 1U, 12U, port1_pin12_config);

    const uint32_t port1_pin13_config = (/* Pin is configured as FC7_RXD_SDA_MOSI_DATA */
                                         IOCON_PIO_FUNC4 |
                                         /* Selects pull-up function */
                                         IOCON_PIO_MODE_PULLUP |
                                         /* Input function is not inverted */
                                         IOCON_PIO_INV_DI |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Input filter disabled */
                                         IOCON_PIO_INPFILT_OFF |
                                         /* Standard mode, output slew rate control is enabled */
                                         IOCON_PIO_SLEW_STANDARD |
                                         /* Open drain is disabled */
                                         IOCON_PIO_OPENDRAIN_DI);
    /* PORT1 PIN13 (coords: 54) is configured as FC7_RXD_SDA_MOSI_DATA */
    IOCON_PinMuxSet(IOCON, 1U, 13U, port1_pin13_config);

    const uint32_t port1_pin17_config = (/* Pin is configured as MCLK */
                                         IOCON_PIO_FUNC4 |
                                         /* No addition pin function */
                                         IOCON_PIO_MODE_INACT |
                                         /* Input function is not inverted */
                                         IOCON_PIO_INV_DI |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Input filter disabled */
                                         IOCON_PIO_INPFILT_OFF |
                                         /* Standard mode, output slew rate control is enabled */
                                         IOCON_PIO_SLEW_STANDARD |
                                         /* Open drain is disabled */
                                         IOCON_PIO_OPENDRAIN_DI);
    /* PORT1 PIN17 (coords: 10) is configured as MCLK */
    IOCON_PinMuxSet(IOCON, 1U, 17U, port1_pin17_config);

    const uint32_t port1_pin8_config = (/* Pin is configured as FC7_TXD_SCL_MISO_WS */
                                        IOCON_PIO_FUNC2 |
                                        /* Selects pull-up function */
                                        IOCON_PIO_MODE_PULLUP |
                                        /* Input function is not inverted */
                                        IOCON_PIO_INV_DI |
                                        /* Enables digital function */
                                        IOCON_PIO_DIGITAL_EN |
                                        /* Input filter disabled */
                                        IOCON_PIO_INPFILT_OFF |
                                        /* Open drain is disabled */
                                        IOCON_PIO_OPENDRAIN_DI);
    /* PORT1 PIN8 (coords: 28) is configured as FC7_TXD_SCL_MISO_WS */
    IOCON_PinMuxSet(IOCON, 1U, 8U, port1_pin8_config);
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
