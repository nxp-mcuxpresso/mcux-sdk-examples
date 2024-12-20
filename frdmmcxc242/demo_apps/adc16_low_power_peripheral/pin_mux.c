/* 
 * Copyright 2024 NXP
 * All rights reserved.
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
product: Pins v15.0
processor: MCXC242
package_id: MCXC242VLH
mcu_data: ksdk2_0
processor_version: 0.16.2
board: FRDM-MCXC242
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
- options: {callFromInitBoot: 'true', prefix: BOARD_, coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '29', peripheral: GPIOA, signal: 'GPIO, 13', pin_signal: PTA13/TPM1_CH1, direction: OUTPUT, gpio_init_state: 'true'}
  - {pin_num: '41', peripheral: GPIOB, signal: 'GPIO, 18', pin_signal: PTB18/TPM2_CH0, direction: OUTPUT, gpio_init_state: 'true'}
  - {pin_num: '42', peripheral: GPIOB, signal: 'GPIO, 19', pin_signal: PTB19/TPM2_CH1, direction: OUTPUT, gpio_init_state: 'true'}
  - {pin_num: '23', peripheral: LPUART0, signal: RX, pin_signal: PTA1/LPUART0_RX/TPM2_CH0}
  - {pin_num: '24', peripheral: LPUART0, signal: TX, pin_signal: PTA2/LPUART0_TX/TPM2_CH1}
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

    gpio_pin_config_t LED_BLUE_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 1U
    };
    /* Initialize GPIO functionality on pin PTA13 (pin 29)  */
    GPIO_PinInit(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_PIN, &LED_BLUE_config);

    gpio_pin_config_t LED_RED_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 1U
    };
    /* Initialize GPIO functionality on pin PTB18 (pin 41)  */
    GPIO_PinInit(BOARD_LED_RED_GPIO, BOARD_LED_RED_PIN, &LED_RED_config);

    gpio_pin_config_t LED_GREEN_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 1U
    };
    /* Initialize GPIO functionality on pin PTB19 (pin 42)  */
    GPIO_PinInit(BOARD_LED_GREEN_GPIO, BOARD_LED_GREEN_PIN, &LED_GREEN_config);

    /* PORTA1 (pin 23) is configured as LPUART0_RX */
    PORT_SetPinMux(BOARD_DEBUG_UART_RX_PORT, BOARD_DEBUG_UART_RX_PIN, kPORT_MuxAlt2);

    /* PORTA13 (pin 29) is configured as PTA13 */
    PORT_SetPinMux(BOARD_LED_BLUE_PORT, BOARD_LED_BLUE_PIN, kPORT_MuxAsGpio);

    /* PORTA2 (pin 24) is configured as LPUART0_TX */
    PORT_SetPinMux(BOARD_DEBUG_UART_TX_PORT, BOARD_DEBUG_UART_TX_PIN, kPORT_MuxAlt2);

    /* PORTB18 (pin 41) is configured as PTB18 */
    PORT_SetPinMux(BOARD_LED_RED_PORT, BOARD_LED_RED_PIN, kPORT_MuxAsGpio);

    /* PORTB19 (pin 42) is configured as PTB19 */
    PORT_SetPinMux(BOARD_LED_GREEN_PORT, BOARD_LED_GREEN_PIN, kPORT_MuxAsGpio);

    SIM->SOPT5 = ((SIM->SOPT5 &
                   /* Mask bits to zero which are setting */
                   (~(SIM_SOPT5_LPUART0TXSRC_MASK | SIM_SOPT5_LPUART0RXSRC_MASK)))

                  /* LPUART0 Transmit Data Source Select: LPUART0_TX pin. */
                  | SIM_SOPT5_LPUART0TXSRC(SOPT5_LPUART0TXSRC_LPUART_TX)

                  /* LPUART0 Receive Data Source Select: LPUART_RX pin. */
                  | SIM_SOPT5_LPUART0RXSRC(SOPT5_LPUART0RXSRC_LPUART_RX));
}

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_ConfigTriggerSource:
- options: {callFromInitBoot: 'false', coreID: core0, enableClock: 'true'}
- pin_list:
  - {peripheral: ADC0, signal: 'TRG, A', pin_signal: Low_power_timer}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_ConfigTriggerSource
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_ConfigTriggerSource(void)
{

    SIM->SOPT7 = ((SIM->SOPT7 &
                   /* Mask bits to zero which are setting */
                   (~(SIM_SOPT7_ADC0TRGSEL_MASK | SIM_SOPT7_ADC0PRETRGSEL_MASK | SIM_SOPT7_ADC0ALTTRGEN_MASK)))

                  /* ADC0 Trigger Select: LPTMR0 trigger. */
                  | SIM_SOPT7_ADC0TRGSEL(SOPT7_ADC0TRGSEL_LPTMR)

                  /* ADC0 Pretrigger Select: Pre-trigger ADHDWTSA is selected, thus ADC0 will use ADC0_SC1A
                   * configuration for the next ADC conversion and store the result in ADC0_RA register. */
                  | SIM_SOPT7_ADC0PRETRGSEL(SOPT7_ADC0PRETRGSEL_A)

                  /* ADC0 Alternate Trigger Enable: ADC ADHWT trigger comes from a peripheral event selected by
                   * ADC0TRGSEL bits.
                   * ADC0PRETRGSEL bit will select the optional ADHWTSA or ADHWTSB select lines for choosing the
                   * ADCx_SC1x config and ADCx_Rx result regsiter to store the ADC conversion. */
                  | SIM_SOPT7_ADC0ALTTRGEN(SOPT7_ADC0ALTTRGEN_TRGSEL));
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
