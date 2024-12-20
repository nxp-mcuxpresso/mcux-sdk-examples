/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Pins v14.0
processor: MCXN236
package_id: MCXN236VDF
mcu_data: ksdk2_0
processor_version: 0.14.2
pin_labels:
- {pin_num: P1, pin_signal: PIO4_0/WUU0_IN18/TRIG_IN6/FC2_P0/CT_INP16/ADC0_A0, label: GPIO_BRIDGE, identifier: GPIO_BRIDGE}
- {pin_num: C6, pin_signal: PIO1_0/WUU0_IN6/LPTMR0_ALT3/TRIG_IN0/FC3_P0/FC4_P4/CT_INP4/FLEXIO0_D8/SAI1_TX_BCLK/ADC0_A16/CMP0_IN0, label: GPIO_HANDSHAKE}
- {pin_num: B10, pin_signal: PIO0_16/WUU0_IN2/FC0_P0/CT0_MAT0/UTICK_CAP2/FLEXIO0_D0/PDM0_CLK/I3C0_SDA/ADC0_A8, label: GPIO_BRIDGE, identifier: GPIO_BRIDGE}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

#include "fsl_common.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
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
- options: {callFromInitBoot: 'true', coreID: cm33_core0, enableClock: 'true'}
- pin_list:
  - {pin_num: B10, peripheral: GPIO0, signal: 'GPIO, 16', pin_signal: PIO0_16/WUU0_IN2/FC0_P0/CT0_MAT0/UTICK_CAP2/FLEXIO0_D0/PDM0_CLK/I3C0_SDA/ADC0_A8, direction: OUTPUT,
    gpio_init_state: 'true', eft_interrupt: disable, slew_rate: fast, open_drain: disable, drive_strength: low, pull_select: down, pull_enable: disable, passive_filter: disable,
    pull_value: low, input_buffer: enable, invert_input: normal}
  - {pin_num: C6, peripheral: LP_FLEXCOMM3, signal: LPFLEXCOMM_P0, pin_signal: PIO1_0/WUU0_IN6/LPTMR0_ALT3/TRIG_IN0/FC3_P0/FC4_P4/CT_INP4/FLEXIO0_D8/SAI1_TX_BCLK/ADC0_A16/CMP0_IN0,
    slew_rate: slow, open_drain: disable, drive_strength: low, pull_select: up, pull_enable: enable, passive_filter: disable, input_buffer: enable, invert_input: normal}
  - {pin_num: C5, peripheral: LP_FLEXCOMM3, signal: LPFLEXCOMM_P1, pin_signal: PIO1_1/TRIG_IN1/FC3_P1/FC4_P5/CT_INP5/FLEXIO0_D9/SAI1_TX_FS/ADC0_A17/CMP1_IN0, slew_rate: slow,
    open_drain: disable, drive_strength: low, pull_select: up, pull_enable: enable, passive_filter: disable, input_buffer: enable, invert_input: normal}
  - {pin_num: C4, peripheral: LP_FLEXCOMM3, signal: LPFLEXCOMM_P2, pin_signal: PIO1_2/TRIG_OUT0/FC3_P2/FC4_P6/CT1_MAT0/FLEXIO0_D10/SAI1_TXD0/CAN0_TXD/ADC0_A18, slew_rate: slow,
    open_drain: disable, drive_strength: low, pull_select: up, pull_enable: enable, input_buffer: enable, invert_input: normal}
  - {pin_num: B4, peripheral: LP_FLEXCOMM3, signal: LPFLEXCOMM_P3, pin_signal: PIO1_3/WUU0_IN7/TRIG_OUT1/FC3_P3/CT1_MAT1/FLEXIO0_D11/SAI1_RXD0/CAN0_RXD/ADC0_A19/CMP0_IN1,
    slew_rate: slow, open_drain: disable, drive_strength: low, pull_select: up, pull_enable: enable, input_buffer: enable, invert_input: normal}
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
    /* Enables the clock for GPIO0: Enables clock */
    CLOCK_EnableClock(kCLOCK_Gpio0);
    /* Enables the clock for PORT0 controller: Enables clock */
    CLOCK_EnableClock(kCLOCK_Port0);
    /* Enables the clock for PORT1: Enables clock */
    CLOCK_EnableClock(kCLOCK_Port1);

    gpio_pin_config_t GPIO_BRIDGE_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 1U
    };
    /* Initialize GPIO functionality on pin PIO0_16 (pin B10)  */
    GPIO_PinInit(BOARD_INITPINS_GPIO_BRIDGE_GPIO, BOARD_INITPINS_GPIO_BRIDGE_PIN, &GPIO_BRIDGE_config);

    /* EFT detect interrupts configuration on PORT0_ */
    PORT_DisableEFTDetectInterrupts(PORT0, 0x010000u);

    const port_pin_config_t GPIO_BRIDGE = {/* Internal pull-up/down resistor is disabled */
                                           kPORT_PullDisable,
                                           /* Low internal pull resistor value is selected. */
                                           kPORT_LowPullResistor,
                                           /* Fast slew rate is configured */
                                           kPORT_FastSlewRate,
                                           /* Passive input filter is disabled */
                                           kPORT_PassiveFilterDisable,
                                           /* Open drain output is disabled */
                                           kPORT_OpenDrainDisable,
                                           /* Low drive strength is configured */
                                           kPORT_LowDriveStrength,
                                           /* Pin is configured as PIO0_16 */
                                           kPORT_MuxAlt0,
                                           /* Digital input enabled */
                                           kPORT_InputBufferEnable,
                                           /* Digital input is not inverted */
                                           kPORT_InputNormal,
                                           /* Pin Control Register fields [15:0] are not locked */
                                           kPORT_UnlockRegister};
    /* PORT0_16 (pin B10) is configured as PIO0_16 */
    PORT_SetPinConfig(BOARD_INITPINS_GPIO_BRIDGE_PORT, BOARD_INITPINS_GPIO_BRIDGE_PIN, &GPIO_BRIDGE);

    const port_pin_config_t port1_0_pinC6_config = {/* Internal pull-up resistor is enabled */
                                                    kPORT_PullUp,
                                                    /* Low internal pull resistor value is selected. */
                                                    kPORT_LowPullResistor,
                                                    /* Slow slew rate is configured */
                                                    kPORT_SlowSlewRate,
                                                    /* Passive input filter is disabled */
                                                    kPORT_PassiveFilterDisable,
                                                    /* Open drain output is disabled */
                                                    kPORT_OpenDrainDisable,
                                                    /* Low drive strength is configured */
                                                    kPORT_LowDriveStrength,
                                                    /* Pin is configured as FC3_P0 */
                                                    kPORT_MuxAlt2,
                                                    /* Digital input enabled */
                                                    kPORT_InputBufferEnable,
                                                    /* Digital input is not inverted */
                                                    kPORT_InputNormal,
                                                    /* Pin Control Register fields [15:0] are not locked */
                                                    kPORT_UnlockRegister};
    /* PORT1_0 (pin C6) is configured as FC3_P0 */
    PORT_SetPinConfig(PORT1, 0U, &port1_0_pinC6_config);

    const port_pin_config_t port1_1_pinC5_config = {/* Internal pull-up resistor is enabled */
                                                    kPORT_PullUp,
                                                    /* Low internal pull resistor value is selected. */
                                                    kPORT_LowPullResistor,
                                                    /* Slow slew rate is configured */
                                                    kPORT_SlowSlewRate,
                                                    /* Passive input filter is disabled */
                                                    kPORT_PassiveFilterDisable,
                                                    /* Open drain output is disabled */
                                                    kPORT_OpenDrainDisable,
                                                    /* Low drive strength is configured */
                                                    kPORT_LowDriveStrength,
                                                    /* Pin is configured as FC3_P1 */
                                                    kPORT_MuxAlt2,
                                                    /* Digital input enabled */
                                                    kPORT_InputBufferEnable,
                                                    /* Digital input is not inverted */
                                                    kPORT_InputNormal,
                                                    /* Pin Control Register fields [15:0] are not locked */
                                                    kPORT_UnlockRegister};
    /* PORT1_1 (pin C5) is configured as FC3_P1 */
    PORT_SetPinConfig(PORT1, 1U, &port1_1_pinC5_config);

    const port_pin_config_t port1_2_pinC4_config = {/* Internal pull-up resistor is enabled */
                                                    kPORT_PullUp,
                                                    /* Low internal pull resistor value is selected. */
                                                    kPORT_LowPullResistor,
                                                    /* Slow slew rate is configured */
                                                    kPORT_SlowSlewRate,
                                                    /* Passive input filter is disabled */
                                                    kPORT_PassiveFilterDisable,
                                                    /* Open drain output is disabled */
                                                    kPORT_OpenDrainDisable,
                                                    /* Low drive strength is configured */
                                                    kPORT_LowDriveStrength,
                                                    /* Pin is configured as FC3_P2 */
                                                    kPORT_MuxAlt2,
                                                    /* Digital input enabled */
                                                    kPORT_InputBufferEnable,
                                                    /* Digital input is not inverted */
                                                    kPORT_InputNormal,
                                                    /* Pin Control Register fields [15:0] are not locked */
                                                    kPORT_UnlockRegister};
    /* PORT1_2 (pin C4) is configured as FC3_P2 */
    PORT_SetPinConfig(PORT1, 2U, &port1_2_pinC4_config);

    const port_pin_config_t port1_3_pinB4_config = {/* Internal pull-up resistor is enabled */
                                                    kPORT_PullUp,
                                                    /* Low internal pull resistor value is selected. */
                                                    kPORT_LowPullResistor,
                                                    /* Slow slew rate is configured */
                                                    kPORT_SlowSlewRate,
                                                    /* Passive input filter is disabled */
                                                    kPORT_PassiveFilterDisable,
                                                    /* Open drain output is disabled */
                                                    kPORT_OpenDrainDisable,
                                                    /* Low drive strength is configured */
                                                    kPORT_LowDriveStrength,
                                                    /* Pin is configured as FC3_P3 */
                                                    kPORT_MuxAlt2,
                                                    /* Digital input enabled */
                                                    kPORT_InputBufferEnable,
                                                    /* Digital input is not inverted */
                                                    kPORT_InputNormal,
                                                    /* Pin Control Register fields [15:0] are not locked */
                                                    kPORT_UnlockRegister};
    /* PORT1_3 (pin B4) is configured as FC3_P3 */
    PORT_SetPinConfig(PORT1, 3U, &port1_3_pinB4_config);
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
