/*
 * Copyright 2017, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Pins v3.0
processor: LPC54S018
package_id: LPC54S018JET180
mcu_data: ksdk2_0
processor_version: 0.0.0
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

#include "fsl_common.h"
#include "fsl_iocon.h"
#include "pin_mux.h"

/*FUNCTION**********************************************************************
 * 
 * Function Name : BOARD_InitBootPins
 * Description   : Calls initialization functions.
 * 
 *END**************************************************************************/
void BOARD_InitBootPins(void) {
    BOARD_InitPins();
}

#define IOCON_PIO_DIGITAL_EN        0x0100u   /*!< Enables digital function */
#define IOCON_PIO_FUNC1               0x01u   /*!< Selects pin function 1 */
#define IOCON_PIO_FUNC6               0x06u   /*!< Selects pin function 6 */
#define IOCON_PIO_INPFILT_OFF       0x0200u   /*!< Input filter disabled */
#define IOCON_PIO_INV_DI              0x00u   /*!< Input function is not inverted */
#define IOCON_PIO_MODE_INACT          0x00u   /*!< No addition pin function */
#define IOCON_PIO_OPENDRAIN_DI        0x00u   /*!< Open drain is disabled */
#define IOCON_PIO_SLEW_FAST         0x0400u   /*!< Fast mode, slew rate control is disabled */
#define PIN2_IDX                         2u   /*!< Pin number for pin 2 in a port 0 */
#define PIN3_IDX                         3u   /*!< Pin number for pin 3 in a port 0 */
#define PIN4_IDX                         4u   /*!< Pin number for pin 4 in a port 0 */
#define PIN5_IDX                         5u   /*!< Pin number for pin 5 in a port 0 */
#define PIN6_IDX                         6u   /*!< Pin number for pin 6 in a port 0 */
#define PIN7_IDX                         7u   /*!< Pin number for pin 7 in a port 0 */
#define PIN8_IDX                         8u   /*!< Pin number for pin 8 in a port 0 */
#define PIN9_IDX                         9u   /*!< Pin number for pin 9 in a port 0 */
#define PIN10_IDX                       10u   /*!< Pin number for pin 10 in a port 1 */
#define PIN11_IDX                       11u   /*!< Pin number for pin 11 in a port 1 */
#define PIN12_IDX                       12u   /*!< Pin number for pin 12 in a port 1 */
#define PIN13_IDX                       13u   /*!< Pin number for pin 13 in a port 1 */
#define PIN14_IDX                       14u   /*!< Pin number for pin 14 in a port 1 */
#define PIN15_IDX                       15u   /*!< Pin number for pin 15 in a port 0 */
#define PIN16_IDX                       16u   /*!< Pin number for pin 16 in a port 1 */
#define PIN18_IDX                       18u   /*!< Pin number for pin 18 in a port 0 */
#define PIN19_IDX                       19u   /*!< Pin number for pin 19 in a port 0 */
#define PIN20_IDX                       20u   /*!< Pin number for pin 20 in a port 0 */
#define PIN21_IDX                       21u   /*!< Pin number for pin 21 in a port 0 */
#define PIN23_IDX                       23u   /*!< Pin number for pin 23 in a port 1 */
#define PIN24_IDX                       24u   /*!< Pin number for pin 24 in a port 1 */
#define PIN25_IDX                       25u   /*!< Pin number for pin 25 in a port 1 */
#define PIN26_IDX                       26u   /*!< Pin number for pin 26 in a port 1 */
#define PIN27_IDX                       27u   /*!< Pin number for pin 27 in a port 1 */
#define PIN28_IDX                       28u   /*!< Pin number for pin 28 in a port 1 */
#define PIN29_IDX                       29u   /*!< Pin number for pin 29 in a port 0 */
#define PIN30_IDX                       30u   /*!< Pin number for pin 30 in a port 0 */
#define PIN31_IDX                       31u   /*!< Pin number for pin 31 in a port 1 */
#define PORT0_IDX                        0u   /*!< Port index */
#define PORT1_IDX                        1u   /*!< Port index */
#define PORT3_IDX                        3u   /*!< Port index */

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPins:
- options: {callFromInitBoot: 'true', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: B13, peripheral: FLEXCOMM0, signal: RXD_SDA_MOSI, pin_signal: PIO0_29/FC0_RXD_SDA_MOSI/CTIMER2_MAT3/SCT0_OUT8/TRACEDATA(2), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: A2, peripheral: FLEXCOMM0, signal: TXD_SCL_MISO, pin_signal: PIO0_30/FC0_TXD_SCL_MISO/CTIMER0_MAT0/SCT0_OUT9/TRACEDATA(1), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: E9, peripheral: EMC, signal: 'EMC_D, 0', pin_signal: PIO0_2/FC3_TXD_SCL_MISO/CTIMER0_CAP1/SCT0_OUT0/SCT0_GPI2/EMC_D(0), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: A10, peripheral: EMC, signal: 'EMC_D, 1', pin_signal: PIO0_3/FC3_RXD_SDA_MOSI/CTIMER0_MAT1/SCT0_OUT1/SCT0_GPI3/EMC_D(1), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: C8, peripheral: EMC, signal: 'EMC_D, 2', pin_signal: PIO0_4/CAN0_RD/FC4_SCK/CTIMER3_CAP0/SCT0_GPI4/EMC_D(2)/ENET_MDC, mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: E7, peripheral: EMC, signal: 'EMC_D, 3', pin_signal: PIO0_5/CAN0_TD/FC4_RXD_SDA_MOSI/CTIMER3_MAT0/SCT0_GPI5/EMC_D(3)/ENET_MDIO, mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: A5, peripheral: EMC, signal: 'EMC_D, 4', pin_signal: PIO0_6/FC3_SCK/CTIMER3_CAP1/CTIMER4_MAT0/SCT0_GPI6/EMC_D(4)/ENET_RX_DV, mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: H12, peripheral: EMC, signal: 'EMC_D, 5', pin_signal: PIO0_7/FC3_RTS_SCL_SSEL1/SD_CLK/FC5_SCK/FC1_SCK/PDM1_CLK/EMC_D(5)/ENET_RX_CLK, mode: inactive,
    invert: disabled, glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: H10, peripheral: EMC, signal: 'EMC_D, 6', pin_signal: PIO0_8/FC3_SSEL3/SD_CMD/FC5_RXD_SDA_MOSI/SWO/PDM1_DATA/EMC_D(6), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: G12, peripheral: EMC, signal: 'EMC_D, 7', pin_signal: PIO0_9/FC3_SSEL2/SD_POW_EN/FC5_TXD_SCL_MISO/SCI1_IO/EMC_D(7), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: L4, peripheral: EMC, signal: EMC_WE, pin_signal: PIO0_15/FC6_CTS_SDA_SSEL0/UTICK_CAP2/CTIMER4_CAP0/SCT0_OUT2/EMC_WEN/ENET_TX_EN/ADC0_3, mode: inactive,
    invert: disabled, glitch_filter: disabled, open_drain: disabled}
  - {pin_num: C14, peripheral: EMC, signal: 'EMC_A, 0', pin_signal: PIO0_18/FC4_CTS_SDA_SSEL0/SD_WR_PRT/CTIMER1_MAT0/SCT0_OUT1/SCI1_SCLK/EMC_A(0), mode: inactive,
    invert: disabled, glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: C6, peripheral: EMC, signal: 'EMC_A, 1', pin_signal: PIO0_19/FC4_RTS_SCL_SSEL1/UTICK_CAP0/CTIMER0_MAT2/SCT0_OUT2/EMC_A(1)/FC7_TXD_SCL_MISO_WS, mode: inactive,
    invert: disabled, glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: D13, peripheral: EMC, signal: 'EMC_A, 2', pin_signal: PIO0_20/FC3_CTS_SDA_SSEL0/CTIMER1_MAT1/CTIMER3_CAP3/SCT0_GPI2/SCI0_IO/EMC_A(2)/FC7_RXD_SDA_MOSI_DATA,
    mode: inactive, invert: disabled, glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: C13, peripheral: EMC, signal: 'EMC_A, 3', pin_signal: PIO0_21/FC3_RTS_SCL_SSEL1/UTICK_CAP3/CTIMER3_MAT3/SCT0_GPI3/SCI0_SCLK/EMC_A(3)/FC7_SCK, mode: inactive,
    invert: disabled, glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: D4, peripheral: EMC, signal: 'EMC_D, 11', pin_signal: PIO1_4/FC0_SCK/SD_D(0)/CTIMER2_MAT1/SCT0_OUT0/FREQME_GPIO_CLK_A/EMC_D(11), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: E4, peripheral: EMC, signal: 'EMC_A, 4', pin_signal: PIO1_5/FC0_RXD_SDA_MOSI/SD_D(2)/CTIMER2_MAT0/SCT0_GPI0/EMC_A(4), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: G4, peripheral: EMC, signal: 'EMC_A, 5', pin_signal: PIO1_6/FC0_TXD_SCL_MISO/SD_D(3)/CTIMER2_MAT1/SCT0_GPI3/EMC_A(5), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: N1, peripheral: EMC, signal: 'EMC_A, 6', pin_signal: PIO1_7/FC0_RTS_SCL_SSEL1/SD_D(1)/CTIMER2_MAT2/SCT0_GPI4/EMC_A(6), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: P8, peripheral: EMC, signal: 'EMC_A, 7', pin_signal: PIO1_8/FC0_CTS_SDA_SSEL0/SD_CLK/SCT0_OUT1/FC4_SSEL2/EMC_A(7), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: K6, peripheral: EMC, signal: EMC_CAS, pin_signal: PIO1_9/ENET_TXD0/FC1_SCK/CTIMER1_CAP0/SCT0_OUT2/FC4_CTS_SDA_SSEL0/EMC_CASN, mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: N9, peripheral: EMC, signal: EMC_RAS, pin_signal: PIO1_10/ENET_TXD1/FC1_RXD_SDA_MOSI/CTIMER1_MAT0/SCT0_OUT3/EMC_RASN, mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: B4, peripheral: EMC, signal: 'EMC_CLK, 0', pin_signal: PIO1_11/ENET_TX_EN/FC1_TXD_SCL_MISO/CTIMER1_CAP1/USB0_VBUS/EMC_CLK(0), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: K9, peripheral: EMC, signal: 'EMC_DYCS, 0', pin_signal: PIO1_12/ENET_RXD0/FC6_SCK/CTIMER1_MAT1/USB0_PORTPWRN/EMC_DYCSN(0), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: G10, peripheral: EMC, signal: 'EMC_DQM, 0', pin_signal: PIO1_13/ENET_RXD1/FC6_RXD_SDA_MOSI_DATA/CTIMER1_CAP2/USB0_OVERCURRENTN/USB0_FRAME/EMC_DQM(0),
    mode: inactive, invert: disabled, glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: C12, peripheral: EMC, signal: 'EMC_DQM, 1', pin_signal: PIO1_14/ENET_RX_DV/UTICK_CAP2/CTIMER1_MAT2/FC5_CTS_SDA_SSEL0/USB0_LEDN/EMC_DQM(1), mode: inactive,
    invert: disabled, glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: A11, peripheral: EMC, signal: 'EMC_CKE, 0', pin_signal: PIO1_15/ENET_RX_CLK/UTICK_CAP3/CTIMER1_CAP3/FC5_RTS_SCL_SSEL1/FC4_RTS_SCL_SSEL1/EMC_CKE(0),
    mode: inactive, invert: disabled, glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: B7, peripheral: EMC, signal: 'EMC_A, 10', pin_signal: PIO1_16/ENET_MDC/FC6_TXD_SCL_MISO_WS/CTIMER1_MAT3/SD_CMD/EMC_A(10), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: L1, peripheral: EMC, signal: 'EMC_D, 8', pin_signal: PIO1_19/FC8_SCK/SCT0_OUT7/CTIMER3_MAT1/SCT0_GPI7/FC4_SCK/EMC_D(8), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: M1, peripheral: EMC, signal: 'EMC_D, 9', pin_signal: PIO1_20/FC7_RTS_SCL_SSEL1/CTIMER3_CAP2/FC4_TXD_SCL_MISO/EMC_D(9), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: N8, peripheral: EMC, signal: 'EMC_D, 10', pin_signal: PIO1_21/FC7_CTS_SDA_SSEL0/CTIMER3_MAT2/FC4_RXD_SDA_MOSI/EMC_D(10), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: M10, peripheral: EMC, signal: 'EMC_A, 11', pin_signal: PIO1_23/FC2_SCK/SCT0_OUT0/ENET_MDIO/FC3_SSEL2/EMC_A(11), mode: inactive, invert: disabled, glitch_filter: disabled,
    slew_rate: fast, open_drain: disabled}
  - {pin_num: N14, peripheral: EMC, signal: 'EMC_A, 12', pin_signal: PIO1_24/FC2_RXD_SDA_MOSI/SCT0_OUT1/FC3_SSEL3/EMC_A(12), mode: inactive, invert: disabled, glitch_filter: disabled,
    slew_rate: fast, open_drain: disabled}
  - {pin_num: M12, peripheral: EMC, signal: 'EMC_A, 13', pin_signal: PIO1_25/FC2_TXD_SCL_MISO/SCT0_OUT2/UTICK_CAP0/EMC_A(13), mode: inactive, invert: disabled, glitch_filter: disabled,
    slew_rate: fast, open_drain: disabled}
  - {pin_num: J10, peripheral: EMC, signal: 'EMC_A, 8', pin_signal: PIO1_26/FC2_CTS_SDA_SSEL0/SCT0_OUT3/CTIMER0_CAP3/UTICK_CAP1/EMC_A(8), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: F10, peripheral: EMC, signal: 'EMC_A, 9', pin_signal: PIO1_27/FC2_RTS_SCL_SSEL1/SD_D(4)/CTIMER0_MAT3/CLKOUT/EMC_A(9), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: C11, peripheral: EMC, signal: 'EMC_D, 13', pin_signal: PIO1_29/FC7_RXD_SDA_MOSI_DATA/SD_D(6)/SCT0_GPI6/USB1_PORTPWRN/USB1_FRAME/EMC_D(13), mode: inactive,
    invert: disabled, glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: E12, peripheral: EMC, signal: 'EMC_D, 12', pin_signal: PIO1_28/FC7_SCK/SD_D(5)/CTIMER0_CAP2/EMC_D(12), mode: inactive, invert: disabled, glitch_filter: disabled,
    slew_rate: fast, open_drain: disabled}
  - {pin_num: A8, peripheral: EMC, signal: 'EMC_D, 14', pin_signal: PIO1_30/FC7_TXD_SCL_MISO_WS/SD_D(7)/SCT0_GPI7/USB1_OVERCURRENTN/USB1_LEDN/EMC_D(14), mode: inactive,
    invert: disabled, glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: C5, peripheral: EMC, signal: 'EMC_D, 15', pin_signal: PIO1_31/MCLK/CTIMER0_MAT2/SCT0_OUT6/FC8_CTS_SDA_SSEL0/EMC_D(15), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: P9, peripheral: EMC, signal: 'EMC_A, 14', pin_signal: PIO3_25/CTIMER4_CAP2/FC4_SCK/EMC_A(14), mode: inactive, invert: disabled, glitch_filter: disabled,
    slew_rate: fast, open_drain: disabled}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/*FUNCTION**********************************************************************
 *
 * Function Name : BOARD_InitPins
 *
 *END**************************************************************************/
void BOARD_InitPins(void) { /* Function assigned for the Core #0 (ARM Cortex-M4) */
  CLOCK_EnableClock(kCLOCK_Iocon);                           /* Enables the clock for the IOCON block. 0 = Disable; 1 = Enable.: 0x01u */

  const uint32_t port0_pin15_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_WEN */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN15_IDX, port0_pin15_config); /* PORT0 PIN15 (coords: L4) is configured as EMC_WEN */
  const uint32_t port0_pin18_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_A(0) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN18_IDX, port0_pin18_config); /* PORT0 PIN18 (coords: C14) is configured as EMC_A(0) */
  const uint32_t port0_pin19_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_A(1) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN19_IDX, port0_pin19_config); /* PORT0 PIN19 (coords: C6) is configured as EMC_A(1) */
  const uint32_t port0_pin2_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_D(0) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN2_IDX, port0_pin2_config); /* PORT0 PIN2 (coords: E9) is configured as EMC_D(0) */
  const uint32_t port0_pin20_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_A(2) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN20_IDX, port0_pin20_config); /* PORT0 PIN20 (coords: D13) is configured as EMC_A(2) */
  const uint32_t port0_pin21_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_A(3) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN21_IDX, port0_pin21_config); /* PORT0 PIN21 (coords: C13) is configured as EMC_A(3) */
  const uint32_t port0_pin29_config = (
    IOCON_PIO_FUNC1 |                                        /* Pin is configured as FC0_RXD_SDA_MOSI */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN29_IDX, port0_pin29_config); /* PORT0 PIN29 (coords: B13) is configured as FC0_RXD_SDA_MOSI */
  const uint32_t port0_pin3_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_D(1) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN3_IDX, port0_pin3_config); /* PORT0 PIN3 (coords: A10) is configured as EMC_D(1) */
  const uint32_t port0_pin30_config = (
    IOCON_PIO_FUNC1 |                                        /* Pin is configured as FC0_TXD_SCL_MISO */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN30_IDX, port0_pin30_config); /* PORT0 PIN30 (coords: A2) is configured as FC0_TXD_SCL_MISO */
  const uint32_t port0_pin4_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_D(2) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN4_IDX, port0_pin4_config); /* PORT0 PIN4 (coords: C8) is configured as EMC_D(2) */
  const uint32_t port0_pin5_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_D(3) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN5_IDX, port0_pin5_config); /* PORT0 PIN5 (coords: E7) is configured as EMC_D(3) */
  const uint32_t port0_pin6_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_D(4) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN6_IDX, port0_pin6_config); /* PORT0 PIN6 (coords: A5) is configured as EMC_D(4) */
  const uint32_t port0_pin7_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_D(5) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN7_IDX, port0_pin7_config); /* PORT0 PIN7 (coords: H12) is configured as EMC_D(5) */
  const uint32_t port0_pin8_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_D(6) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN8_IDX, port0_pin8_config); /* PORT0 PIN8 (coords: H10) is configured as EMC_D(6) */
  const uint32_t port0_pin9_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_D(7) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN9_IDX, port0_pin9_config); /* PORT0 PIN9 (coords: G12) is configured as EMC_D(7) */
  const uint32_t port1_pin10_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_RASN */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN10_IDX, port1_pin10_config); /* PORT1 PIN10 (coords: N9) is configured as EMC_RASN */
  const uint32_t port1_pin11_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_CLK(0) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN11_IDX, port1_pin11_config); /* PORT1 PIN11 (coords: B4) is configured as EMC_CLK(0) */
  const uint32_t port1_pin12_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_DYCSN(0) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN12_IDX, port1_pin12_config); /* PORT1 PIN12 (coords: K9) is configured as EMC_DYCSN(0) */
  const uint32_t port1_pin13_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_DQM(0) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN13_IDX, port1_pin13_config); /* PORT1 PIN13 (coords: G10) is configured as EMC_DQM(0) */
  const uint32_t port1_pin14_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_DQM(1) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN14_IDX, port1_pin14_config); /* PORT1 PIN14 (coords: C12) is configured as EMC_DQM(1) */
  const uint32_t port1_pin15_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_CKE(0) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN15_IDX, port1_pin15_config); /* PORT1 PIN15 (coords: A11) is configured as EMC_CKE(0) */
  const uint32_t port1_pin16_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_A(10) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN16_IDX, port1_pin16_config); /* PORT1 PIN16 (coords: B7) is configured as EMC_A(10) */
  const uint32_t port1_pin19_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_D(8) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN19_IDX, port1_pin19_config); /* PORT1 PIN19 (coords: L1) is configured as EMC_D(8) */
  const uint32_t port1_pin20_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_D(9) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN20_IDX, port1_pin20_config); /* PORT1 PIN20 (coords: M1) is configured as EMC_D(9) */
  const uint32_t port1_pin21_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_D(10) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN21_IDX, port1_pin21_config); /* PORT1 PIN21 (coords: N8) is configured as EMC_D(10) */
  const uint32_t port1_pin23_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_A(11) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN23_IDX, port1_pin23_config); /* PORT1 PIN23 (coords: M10) is configured as EMC_A(11) */
  const uint32_t port1_pin24_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_A(12) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN24_IDX, port1_pin24_config); /* PORT1 PIN24 (coords: N14) is configured as EMC_A(12) */
  const uint32_t port1_pin25_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_A(13) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN25_IDX, port1_pin25_config); /* PORT1 PIN25 (coords: M12) is configured as EMC_A(13) */
  const uint32_t port1_pin26_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_A(8) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN26_IDX, port1_pin26_config); /* PORT1 PIN26 (coords: J10) is configured as EMC_A(8) */
  const uint32_t port1_pin27_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_A(9) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN27_IDX, port1_pin27_config); /* PORT1 PIN27 (coords: F10) is configured as EMC_A(9) */
  const uint32_t port1_pin28_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_D(12) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN28_IDX, port1_pin28_config); /* PORT1 PIN28 (coords: E12) is configured as EMC_D(12) */
  const uint32_t port1_pin29_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_D(13) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN29_IDX, port1_pin29_config); /* PORT1 PIN29 (coords: C11) is configured as EMC_D(13) */
  const uint32_t port1_pin30_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_D(14) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN30_IDX, port1_pin30_config); /* PORT1 PIN30 (coords: A8) is configured as EMC_D(14) */
  const uint32_t port1_pin31_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_D(15) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN31_IDX, port1_pin31_config); /* PORT1 PIN31 (coords: C5) is configured as EMC_D(15) */
  const uint32_t port1_pin4_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_D(11) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN4_IDX, port1_pin4_config); /* PORT1 PIN4 (coords: D4) is configured as EMC_D(11) */
  const uint32_t port1_pin5_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_A(4) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN5_IDX, port1_pin5_config); /* PORT1 PIN5 (coords: E4) is configured as EMC_A(4) */
  const uint32_t port1_pin6_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_A(5) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN6_IDX, port1_pin6_config); /* PORT1 PIN6 (coords: G4) is configured as EMC_A(5) */
  const uint32_t port1_pin7_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_A(6) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN7_IDX, port1_pin7_config); /* PORT1 PIN7 (coords: N1) is configured as EMC_A(6) */
  const uint32_t port1_pin8_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_A(7) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN8_IDX, port1_pin8_config); /* PORT1 PIN8 (coords: P8) is configured as EMC_A(7) */
  const uint32_t port1_pin9_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_CASN */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN9_IDX, port1_pin9_config); /* PORT1 PIN9 (coords: K6) is configured as EMC_CASN */
  const uint32_t port3_pin25_config = (
    IOCON_PIO_FUNC6 |                                        /* Pin is configured as EMC_A(14) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT3_IDX, PIN25_IDX, port3_pin25_config); /* PORT3 PIN25 (coords: P9) is configured as EMC_A(14) */
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
