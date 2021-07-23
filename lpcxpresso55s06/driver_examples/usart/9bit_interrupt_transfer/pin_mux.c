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
processor: LPC55S06
package_id: LPC55S06JBD64
mcu_data: ksdk2_0
processor_version: 8.0.0
pin_labels:
- {pin_num: '1', pin_signal: PIO0_7/FC3_RTS_SCL_SSEL1/FC5_SCK/FC1_SCK/SECURE_GPIO0_7, label: 'J10[4]/P0_7'}
- {pin_num: '2', pin_signal: PIO0_1/FC3_CTS_SDA_SSEL0/CT_INP0/SCT_GPI1/CMP0_OUT/SECURE_GPIO0_1, label: 'J9[3]/J11[3]/EXP_CMP0_OUT'}
- {pin_num: '3', pin_signal: PIO1_9/FC1_SCK/CT_INP4/SCT0_OUT2/FC4_CTS_SDA_SSEL0/ADC0_12, label: 'SW3/J13[2]/ARD_BTN_USR_P1_9', identifier: SW3;USR}
- {pin_num: '4', pin_signal: PIO1_0/FC0_RTS_SCL_SSEL1/CT_INP2/SCT_GPI4/PLU_OUT3/ADC0_11, label: 'J12[7]/P1_0'}
- {pin_num: '5', pin_signal: PIO0_12/FC3_TXD_SCL_MISO_WS/FREQME_GPIO_CLK_B/SCT_GPI7/SWDIO/FC6_TXD_SCL_MISO_WS/SECURE_GPIO0_12/ADC0_10, label: 'J9[17]/J10[6]/U18[8]/N4M_SWDIO',
  identifier: DEBUG_SWD_SWDIO}
- {pin_num: '6', pin_signal: PIO0_11/FC6_RXD_SDA_MOSI_DATA/CTIMER2_MAT2/FREQME_GPIO_CLK_A/SWCLK/SECURE_GPIO0_11/ADC0_9, label: 'J9[19]/J10[8]/U18[4]/N4M_SWDCLK',
  identifier: DEBUG_SWD_SWDCLK}
- {pin_num: '7', pin_signal: PIO0_16/FC4_TXD_SCL_MISO_WS/CLKOUT/CT_INP4/SECURE_GPIO0_16/ADC0_8, label: 'J7[1]/J13[1]/JP4[1]/ARD_MIK_ADC0_8_N'}
- {pin_num: '8', pin_signal: VDD8, label: 'JP20[2]/MCU_VDD'}
- {pin_num: '16', pin_signal: VDD16, label: 'JP20[2]/MCU_VDD'}
- {pin_num: '28', pin_signal: VDD28, label: 'JP20[2]/MCU_VDD'}
- {pin_num: '43', pin_signal: VDD43, label: 'JP20[2]/MCU_VDD'}
- {pin_num: '56', pin_signal: VDD56, label: 'JP20[2]/MCU_VDD'}
- {pin_num: '9', pin_signal: VDDA, label: 'JP21[2]/MCU_VDDA'}
- {pin_num: '10', pin_signal: VREFP, label: VREFP}
- {pin_num: '11', pin_signal: VREFN/VSSA, label: GND}
- {pin_num: '12', pin_signal: PIO0_23/MCLK/CTIMER1_MAT2/CTIMER3_MAT3/SCT0_OUT4/FC0_CTS_SDA_SSEL0/SECURE_GPIO0_23/ADC0_0, label: 'J12[10]/J13[3]/J13[8]/JP4[3]/ARD_ADC0_0_P'}
- {pin_num: '13', pin_signal: PIO0_10/FC6_SCK/CT_INP10/CTIMER2_MAT0/FC1_TXD_SCL_MISO_WS/SCT0_OUT2/SWO/SECURE_GPIO0_10/ADC0_1, label: 'J9[15]/U18[12]/N4M_SWO', identifier: DEBUG_SWD_SWO}
- {pin_num: '14', pin_signal: PIO0_15/FC6_CTS_SDA_SSEL0/UTICK_CAP2/CT_INP16/SCT0_OUT2/SECURE_GPIO0_15/ADC0_2, label: 'J12[12]/ARD_INT_P0_15'}
- {pin_num: '15', pin_signal: PIO0_31/FC0_CTS_SDA_SSEL0/CTIMER0_MAT1/SCT0_OUT3/SECURE_GPIO0_31/ADC0_3, label: 'J10[2]/P0_31'}
- {pin_num: '17', pin_signal: PIO0_8/FC3_SSEL3/FC5_RXD_SDA_MOSI_DATA/SWO/SECURE_GPIO0_8, label: 'J10[5]/P0_8'}
- {pin_num: '18', pin_signal: PIO0_27/FC2_TXD_SCL_MISO_WS/CTIMER3_MAT2/SCT0_OUT6/FC7_RXD_SDA_MOSI_DATA/PLU_OUT0/SECURE_GPIO0_27, label: 'J12[17]/P0_27'}
- {pin_num: '19', pin_signal: XTAL32M_N, label: 'Y2[1]/XTAL32M_N'}
- {pin_num: '20', pin_signal: XTAL32M_P, label: 'Y2[3]/XTAL32M_P'}
- {pin_num: '21', pin_signal: PIO1_21/FC7_CTS_SDA_SSEL0/CTIMER3_MAT2/FC4_RXD_SDA_MOSI_DATA/PLU_OUT3, label: 'J12[13]/U29[7]/P1_21'}
- {pin_num: '22', pin_signal: PIO1_5/FC0_RXD_SDA_MOSI_DATA/CTIMER2_MAT0/SCT_GPI0, label: 'J8[1]/J12[20]/ARD_MIK_P1_5'}
- {pin_num: '23', pin_signal: RESETN, label: 'SW2/J7[2]/J10[9]/J10[10]/U18[16]/N4M_RESET#', identifier: SW2;RESET}
- {pin_num: '24', pin_signal: VDD_PMU, label: LPC_VDD_PMU}
- {pin_num: '29', pin_signal: FB, label: FB}
- {pin_num: '30', pin_signal: VSS_DCDC, label: GND}
- {pin_num: '31', pin_signal: LX, label: LX}
- {pin_num: '32', pin_signal: VBAT_DCDC, label: 'JP22[2]/MCU_VBAT'}
- {pin_num: '33', pin_signal: VBAT_PMU, label: 'JP22[2]/MCU_VBAT'}
- {pin_num: '34', pin_signal: XTAL32K_P, label: 'Y3[2]/XTAL32K_P'}
- {pin_num: '35', pin_signal: XTAL32K_N, label: 'Y3[1]/XTAL32K_N'}
- {pin_num: '25', pin_signal: PIO1_10/FC1_RXD_SDA_MOSI_DATA/CTIMER1_MAT0/SCT0_OUT3, label: 'J8[3]/J12[16]/J13[4]/ARD_MIK_FC1_USART_RXD'}
- {pin_num: '26', pin_signal: PIO1_22/CTIMER2_MAT3/SCT_GPI5/FC4_SSEL3/PLU_OUT4/CAN0_RD, label: 'J18[4]/ISP_P1_19'}
- {pin_num: '27', pin_signal: PIO1_23/FC2_SCK/SCT0_OUT0/FC3_SSEL2/PLU_OUT5, label: 'J9[5]/U12[11]/ACC_INT_EXP_P1_23', identifier: ACCL_INTR}
- {pin_num: '36', pin_signal: PIO0_0/FC3_SCK/CTIMER0_MAT0/SCT_GPI0/SECURE_GPIO0_0/ACMP0_A, label: 'J9[1]/J11[2]/J13[7]/ARD_CMP0_IN_A'}
- {pin_num: '37', pin_signal: PIO0_9/FC3_SSEL2/FC5_TXD_SCL_MISO_WS/SECURE_GPIO0_9/ACMP0_B, label: 'J11[1]/J13[5]/EXP_CMP0_IN_B'}
- {pin_num: '38', pin_signal: PIO0_18/FC4_CTS_SDA_SSEL0/CTIMER1_MAT0/SCT0_OUT1/PLU_IN3/SECURE_GPIO0_18/ACMP0_C, label: 'J12[4]/Q4[G2]/ARD_LEDG_PWM', identifier: LED_GREEN}
- {pin_num: '39', pin_signal: PIO1_1/FC3_RXD_SDA_MOSI_DATA/CT_INP3/SCT_GPI5/HS_SPI_SSEL1/PLU_OUT4, label: 'J7[3]/J9[16]/J18[1]/U24[14]/CAN__HSSPI_ISP_PIO1_1'}
- {pin_num: '40', pin_signal: PIO0_26/FC2_RXD_SDA_MOSI_DATA/CLKOUT/CT_INP14/SCT0_OUT5/FC0_SCK/HS_SPI_MOSI/SECURE_GPIO0_26, label: 'J7[6]/J9[14]/J18[3]/U24[11]/CAN__HSSPI_ISP_PIO0_26'}
- {pin_num: '41', pin_signal: PIO1_2/CAN0_TD/CTIMER0_MAT3/SCT_GPI6/HS_SPI_SCK/PLU_OUT5, label: 'J18[7]/JP26[2]/U24[13]/CAN__HSSPI_ISP_PIO1_2', identifier: CAN_TXD}
- {pin_num: '42', pin_signal: PIO1_3/CAN0_RD/SCT0_OUT4/HS_SPI_MISO/PLU_OUT6, label: 'J18[5]/JP25[2]/U24[12]/CAN__HSSPI_ISP_PIO1_3', identifier: CAN_RXD}
- {pin_num: '44', pin_signal: PIO0_28/FC0_SCK/CT_INP11/SCT0_OUT7/PLU_OUT1/SECURE_GPIO0_28, label: 'SW1/J8[2]/J10[1]/J12[15]/MIK_EXP_BTN_WK', identifier: SW1;WAKEUP}
- {pin_num: '45', pin_signal: PIO0_24/FC0_RXD_SDA_MOSI_DATA/CT_INP8/SCT_GPI0/SECURE_GPIO0_24, label: 'J8[6]/J9[18]/U12[6]/FC0_I2C_SDA', identifier: FC0_I2C_SDA}
- {pin_num: '46', pin_signal: PIO0_13/FC1_CTS_SDA_SSEL0/UTICK_CAP0/CT_INP0/SCT_GPI0/FC1_RXD_SDA_MOSI_DATA/PLU_IN0/SECURE_GPIO0_13, label: 'J13[9]/J13[10]/J18[8]/FC1_I2C_SDA'}
- {pin_num: '47', pin_signal: PIO0_14/FC1_RTS_SCL_SSEL1/UTICK_CAP1/CT_INP1/SCT_GPI1/FC1_TXD_SCL_MISO_WS/PLU_IN1/SECURE_GPIO0_14, label: 'J12[19]/J13[11]/J13[12]/J18[6]/JS5[2]/FC1_I2C_SCL'}
- {pin_num: '48', pin_signal: PIO0_20/FC3_CTS_SDA_SSEL0/CTIMER1_MAT1/CT_INP15/SCT_GPI2/FC7_RXD_SDA_MOSI_DATA/HS_SPI_SSEL0/PLU_IN5/SECURE_GPIO0_20/FC4_TXD_SCL_MISO_WS,
  label: 'J9[9]/J12[9]/P0_20'}
- {pin_num: '49', pin_signal: PIO0_21/FC3_RTS_SCL_SSEL1/UTICK_CAP3/CTIMER3_MAT3/SCT_GPI3/FC7_SCK/HS_SPI_SSEL3/PLU_CLKIN/SECURE_GPIO0_21, label: 'J9[2]/J9[13]/J12[5]/Q5[G1]/ARD_LEDR_PWM',
  identifier: LED_RED}
- {pin_num: '50', pin_signal: PIO1_25/FC2_TXD_SCL_MISO_WS/SCT0_OUT2/UTICK_CAP0/PLU_CLKIN, label: 'J12[3]/U29[3]/P1_25'}
- {pin_num: '51', pin_signal: PIO0_22/FC6_TXD_SCL_MISO_WS/UTICK_CAP1/CT_INP15/SCT0_OUT3/PLU_OUT7/SECURE_GPIO0_22, label: 'J12[8]/Q4[G1]/ARD_LEDB_PWM', identifier: LED_BLUE}
- {pin_num: '52', pin_signal: PIO0_25/FC0_TXD_SCL_MISO_WS/CT_INP9/SCT_GPI1/SECURE_GPIO0_25, label: 'J8[5]/J10[3]/U12[4]/FC0_I2C_SCL', identifier: FC0_I2C_SC}
- {pin_num: '53', pin_signal: PIO1_29/FC7_RXD_SDA_MOSI_DATA/SCT_GPI6/PLU_IN2, label: 'J10[20]/J12[11]/J18[2]/EXP_ISP_P1_29'}
- {pin_num: '54', pin_signal: PIO0_2/FC3_TXD_SCL_MISO_WS/CT_INP1/SCT0_OUT0/SCT_GPI2/SECURE_GPIO0_2, label: 'J12[2]/JP27[2]/SPI_FLASH_PIO0_2'}
- {pin_num: '55', pin_signal: PIO0_3/FC3_RXD_SDA_MOSI_DATA/CTIMER0_MAT1/SCT0_OUT1/SCT_GPI3/SECURE_GPIO0_3, label: 'J12[1]/JP28[2]/SPI_FLASH_PIO0_3'}
- {pin_num: '57', pin_signal: PIO0_4/CAN0_RD/FC4_SCK/CT_INP12/SCT_GPI4/FC3_CTS_SDA_SSEL0/SECURE_GPIO0_4, label: 'J10[14]/JP29[2]/SPI_FLASH_PIO0_4'}
- {pin_num: '58', pin_signal: PIO0_5/CAN0_TD/FC4_RXD_SDA_MOSI_DATA/CTIMER3_MAT0/SCT_GPI5/FC3_RTS_SCL_SSEL1/MCLK/SECURE_GPIO0_5, label: 'SW4/J9[7]/J17[7]/JS3[1]/U22[11]/U23[4]/N4M_ISP_MODE',
  identifier: SW4;ISP}
- {pin_num: '59', pin_signal: PIO0_6/FC3_SCK/CT_INP13/CTIMER4_MAT0/SCT_GPI6/SECURE_GPIO0_6, label: 'J10[16]/JP30[2]/SPI_FLASH_PIO0_6'}
- {pin_num: '60', pin_signal: PIO0_19/FC4_RTS_SCL_SSEL1/UTICK_CAP0/CTIMER0_MAT2/SCT0_OUT2/FC7_TXD_SCL_MISO_WS/PLU_IN4/SECURE_GPIO0_19, label: 'J9[11]/J12[6]/P0_19'}
- {pin_num: '61', pin_signal: PIO0_29/FC0_RXD_SDA_MOSI_DATA/CTIMER2_MAT3/SCT0_OUT8/CMP0_OUT/PLU_OUT2/SECURE_GPIO0_29, label: 'U11[14]/U22[14]/FC0_USART_RXD', identifier: DEBUG_UART_RX}
- {pin_num: '62', pin_signal: PIO1_11/FC1_TXD_SCL_MISO_WS/CT_INP5, label: 'J8[4]/J12[14]/J13[6]/ARD_MIK_FC1_USART_TXD'}
- {pin_num: '63', pin_signal: PIO0_30/FC0_TXD_SCL_MISO_WS/CTIMER0_MAT0/SCT0_OUT9/SECURE_GPIO0_30, label: 'J10[18]/U11[13]/U22[13]/FC0_USART_TXD', identifier: DEBUG_UART_TX}
- {pin_num: '64', pin_signal: PIO1_4/FC0_SCK/CTIMER2_MAT1/SCT0_OUT0/FREQME_GPIO_CLK_A, label: 'J12[18]/P1_4'}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

#include "fsl_common.h"
#include "fsl_iocon.h"
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
  - {pin_num: '13', peripheral: SWD, signal: SWO, pin_signal: PIO0_10/FC6_SCK/CT_INP10/CTIMER2_MAT0/FC1_TXD_SCL_MISO_WS/SCT0_OUT2/SWO/SECURE_GPIO0_10/ADC0_1, mode: inactive,
    slew_rate: standard, invert: disabled, open_drain: disabled, asw: disabled}
  - {pin_num: '61', peripheral: FLEXCOMM0, signal: RXD_SDA_MOSI_DATA, pin_signal: PIO0_29/FC0_RXD_SDA_MOSI_DATA/CTIMER2_MAT3/SCT0_OUT8/CMP0_OUT/PLU_OUT2/SECURE_GPIO0_29,
    mode: inactive, slew_rate: standard, invert: disabled, open_drain: disabled}
  - {pin_num: '63', peripheral: FLEXCOMM0, signal: TXD_SCL_MISO_WS, pin_signal: PIO0_30/FC0_TXD_SCL_MISO_WS/CTIMER0_MAT0/SCT0_OUT9/SECURE_GPIO0_30, mode: inactive,
    slew_rate: standard, invert: disabled, open_drain: disabled}
  - {pin_num: '55', peripheral: FLEXCOMM3, signal: RXD_SDA_MOSI_DATA, pin_signal: PIO0_3/FC3_RXD_SDA_MOSI_DATA/CTIMER0_MAT1/SCT0_OUT1/SCT_GPI3/SECURE_GPIO0_3}
  - {pin_num: '54', peripheral: FLEXCOMM3, signal: TXD_SCL_MISO_WS, pin_signal: PIO0_2/FC3_TXD_SCL_MISO_WS/CT_INP1/SCT0_OUT0/SCT_GPI2/SECURE_GPIO0_2}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
/* Function assigned for the Cortex-M33 */
void BOARD_InitPins(void)
{
    /* Enables the clock for the I/O controller.: Enable Clock. */
    CLOCK_EnableClock(kCLOCK_Iocon);

    const uint32_t port0_pin10_config = (/* Pin is configured as SWO */
                                         IOCON_PIO_FUNC6 |
                                         /* No addition pin function */
                                         IOCON_PIO_MODE_INACT |
                                         /* Standard mode, output slew rate control is enabled */
                                         IOCON_PIO_SLEW_STANDARD |
                                         /* Input function is not inverted */
                                         IOCON_PIO_INV_DI |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Open drain is disabled */
                                         IOCON_PIO_OPENDRAIN_DI |
                                         /* Analog switch is open (disabled) */
                                         IOCON_PIO_ASW_DI);
    /* PORT0 PIN10 (coords: 13) is configured as SWO */
    IOCON_PinMuxSet(IOCON, 0U, 10U, port0_pin10_config);

    IOCON->PIO[0][2] = ((IOCON->PIO[0][2] &
                         /* Mask bits to zero which are setting */
                         (~(IOCON_PIO_FUNC_MASK | IOCON_PIO_DIGIMODE_MASK)))

                        /* Selects pin function.
                         * : PORT02 (pin 54) is configured as FC3_TXD_SCL_MISO_WS. */
                        | IOCON_PIO_FUNC(PIO0_2_FUNC_ALT1)

                        /* Select Digital mode.
                         * : Enable Digital mode.
                         * Digital input is enabled. */
                        | IOCON_PIO_DIGIMODE(PIO0_2_DIGIMODE_DIGITAL));

    const uint32_t port0_pin29_config = (/* Pin is configured as FC0_RXD_SDA_MOSI_DATA */
                                         IOCON_PIO_FUNC1 |
                                         /* No addition pin function */
                                         IOCON_PIO_MODE_INACT |
                                         /* Standard mode, output slew rate control is enabled */
                                         IOCON_PIO_SLEW_STANDARD |
                                         /* Input function is not inverted */
                                         IOCON_PIO_INV_DI |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Open drain is disabled */
                                         IOCON_PIO_OPENDRAIN_DI);
    /* PORT0 PIN29 (coords: 61) is configured as FC0_RXD_SDA_MOSI_DATA */
    IOCON_PinMuxSet(IOCON, 0U, 29U, port0_pin29_config);

    IOCON->PIO[0][3] = ((IOCON->PIO[0][3] &
                         /* Mask bits to zero which are setting */
                         (~(IOCON_PIO_FUNC_MASK | IOCON_PIO_DIGIMODE_MASK)))

                        /* Selects pin function.
                         * : PORT03 (pin 55) is configured as FC3_RXD_SDA_MOSI_DATA. */
                        | IOCON_PIO_FUNC(PIO0_3_FUNC_ALT1)

                        /* Select Digital mode.
                         * : Enable Digital mode.
                         * Digital input is enabled. */
                        | IOCON_PIO_DIGIMODE(PIO0_3_DIGIMODE_DIGITAL));

    const uint32_t port0_pin30_config = (/* Pin is configured as FC0_TXD_SCL_MISO_WS */
                                         IOCON_PIO_FUNC1 |
                                         /* No addition pin function */
                                         IOCON_PIO_MODE_INACT |
                                         /* Standard mode, output slew rate control is enabled */
                                         IOCON_PIO_SLEW_STANDARD |
                                         /* Input function is not inverted */
                                         IOCON_PIO_INV_DI |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Open drain is disabled */
                                         IOCON_PIO_OPENDRAIN_DI);
    /* PORT0 PIN30 (coords: 63) is configured as FC0_TXD_SCL_MISO_WS */
    IOCON_PinMuxSet(IOCON, 0U, 30U, port0_pin30_config);
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
