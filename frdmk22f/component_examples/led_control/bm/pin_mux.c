/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
 
/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Pins v4.1
processor: MK22FN512xxx12
package_id: MK22FN512VLH12
mcu_data: ksdk2_0
processor_version: 4.0.0
board: FRDM-K22F
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
    BOARD_InitPWM();
}

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPins:
- options: {callFromInitBoot: 'true', prefix: BOARD_, coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '1', peripheral: UART1, signal: TX, pin_signal: ADC1_SE4a/PTE0/CLKOUT32K/SPI1_PCS1/UART1_TX/I2C1_SDA/RTC_CLKOUT}
  - {pin_num: '2', peripheral: UART1, signal: RX, pin_signal: ADC1_SE5a/PTE1/LLWU_P0/SPI1_SOUT/UART1_RX/I2C1_SCL/SPI1_SIN, identifier: ''}
  - {pin_num: '40', peripheral: GPIOB, signal: 'GPIO, 17', pin_signal: PTB17/SPI1_SIN/UART0_TX/FTM_CLKIN1/FB_AD16/EWM_OUT_b, identifier: ''}
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
    /* Port E Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortE);

    /* PORTB17 (pin 40) is configured as PTB17 */
    PORT_SetPinMux(PORTB, 17U, kPORT_MuxAsGpio);

    /* PORTE0 (pin 1) is configured as UART1_TX */
    PORT_SetPinMux(PORTE, 0U, kPORT_MuxAlt3);

    /* PORTE1 (pin 2) is configured as UART1_RX */
    PORT_SetPinMux(PORTE, 1U, kPORT_MuxAlt3);

    SIM->SOPT5 = ((SIM->SOPT5 &
                   /* Mask bits to zero which are setting */
                   (~(SIM_SOPT5_UART1TXSRC_MASK)))

                  /* UART 1 transmit data source select: UART1_TX pin. */
                  | SIM_SOPT5_UART1TXSRC(SOPT5_UART1TXSRC_UART_TX));
}

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitLEDs:
- options: {callFromInitBoot: 'false', prefix: BOARD_, coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '23', peripheral: GPIOA, signal: 'GPIO, 1', pin_signal: PTA1/UART0_RX/FTM0_CH6/JTAG_TDI/EZP_DI, direction: OUTPUT, gpio_init_state: 'true', slew_rate: slow,
    open_drain: disable, pull_select: down, pull_enable: disable}
  - {pin_num: '24', peripheral: GPIOA, signal: 'GPIO, 2', pin_signal: PTA2/UART0_TX/FTM0_CH7/JTAG_TDO/TRACE_SWO/EZP_DO, direction: OUTPUT, gpio_init_state: 'true',
    slew_rate: slow, open_drain: disable, pull_select: down, pull_enable: disable}
  - {pin_num: '62', peripheral: GPIOD, signal: 'GPIO, 5', pin_signal: ADC0_SE6b/PTD5/SPI0_PCS2/UART0_CTS_b/FTM0_CH5/FB_AD1/EWM_OUT_b/SPI1_SCK, direction: OUTPUT,
    gpio_init_state: 'true', slew_rate: slow, open_drain: disable, drive_strength: low, pull_select: down, pull_enable: disable}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitLEDs
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitLEDs(void)
{
    /* Port A Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortA);
    /* Port D Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortD);

    gpio_pin_config_t LEDRGB_RED_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 1U
    };
    /* Initialize GPIO functionality on pin PTA1 (pin 23)  */
    GPIO_PinInit(BOARD_LEDRGB_RED_GPIO, BOARD_LEDRGB_RED_PIN, &LEDRGB_RED_config);

    gpio_pin_config_t LEDRGB_GREEN_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 1U
    };
    /* Initialize GPIO functionality on pin PTA2 (pin 24)  */
    GPIO_PinInit(BOARD_LEDRGB_GREEN_GPIO, BOARD_LEDRGB_GREEN_PIN, &LEDRGB_GREEN_config);

    gpio_pin_config_t LEDRGB_BLUE_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 1U
    };
    /* Initialize GPIO functionality on pin PTD5 (pin 62)  */
    GPIO_PinInit(BOARD_LEDRGB_BLUE_GPIO, BOARD_LEDRGB_BLUE_PIN, &LEDRGB_BLUE_config);

    const port_pin_config_t LEDRGB_RED = {/* Internal pull-up/down resistor is disabled */
                                          kPORT_PullDisable,
                                          /* Slow slew rate is configured */
                                          kPORT_SlowSlewRate,
                                          /* Passive filter is disabled */
                                          kPORT_PassiveFilterDisable,
                                          /* Open drain is disabled */
                                          kPORT_OpenDrainDisable,
                                          /* Low drive strength is configured */
                                          kPORT_LowDriveStrength,
                                          /* Pin is configured as PTA1 */
                                          kPORT_MuxAsGpio,
                                          /* Pin Control Register fields [15:0] are not locked */
                                          kPORT_UnlockRegister};
    /* PORTA1 (pin 23) is configured as PTA1 */
    PORT_SetPinConfig(BOARD_LEDRGB_RED_PORT, BOARD_LEDRGB_RED_PIN, &LEDRGB_RED);

    const port_pin_config_t LEDRGB_GREEN = {/* Internal pull-up/down resistor is disabled */
                                            kPORT_PullDisable,
                                            /* Slow slew rate is configured */
                                            kPORT_SlowSlewRate,
                                            /* Passive filter is disabled */
                                            kPORT_PassiveFilterDisable,
                                            /* Open drain is disabled */
                                            kPORT_OpenDrainDisable,
                                            /* Low drive strength is configured */
                                            kPORT_LowDriveStrength,
                                            /* Pin is configured as PTA2 */
                                            kPORT_MuxAsGpio,
                                            /* Pin Control Register fields [15:0] are not locked */
                                            kPORT_UnlockRegister};
    /* PORTA2 (pin 24) is configured as PTA2 */
    PORT_SetPinConfig(BOARD_LEDRGB_GREEN_PORT, BOARD_LEDRGB_GREEN_PIN, &LEDRGB_GREEN);

    const port_pin_config_t LEDRGB_BLUE = {/* Internal pull-up/down resistor is disabled */
                                           kPORT_PullDisable,
                                           /* Slow slew rate is configured */
                                           kPORT_SlowSlewRate,
                                           /* Passive filter is disabled */
                                           kPORT_PassiveFilterDisable,
                                           /* Open drain is disabled */
                                           kPORT_OpenDrainDisable,
                                           /* Low drive strength is configured */
                                           kPORT_LowDriveStrength,
                                           /* Pin is configured as PTD5 */
                                           kPORT_MuxAsGpio,
                                           /* Pin Control Register fields [15:0] are not locked */
                                           kPORT_UnlockRegister};
    /* PORTD5 (pin 62) is configured as PTD5 */
    PORT_SetPinConfig(BOARD_LEDRGB_BLUE_PORT, BOARD_LEDRGB_BLUE_PIN, &LEDRGB_BLUE);
}

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPWM:
- options: {callFromInitBoot: 'true', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '23', peripheral: FTM0, signal: 'CH, 6', pin_signal: PTA1/UART0_RX/FTM0_CH6/JTAG_TDI/EZP_DI}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPWM
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitPWM(void)
{
    /* Port A Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortA);

    /* PORTA1 (pin 23) is configured as FTM0_CH6 */
    PORT_SetPinMux(BOARD_INITPWM_LEDRGB_RED_PORT, BOARD_INITPWM_LEDRGB_RED_PIN, kPORT_MuxAlt3);
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
