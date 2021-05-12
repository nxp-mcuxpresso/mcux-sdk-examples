/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
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
#define IOCON_PIO_FUNC2               0x02u   /*!< Selects pin function 2 */
#define IOCON_PIO_INPFILT_OFF       0x0200u   /*!< Input filter disabled */
#define IOCON_PIO_INV_DI              0x00u   /*!< Input function is not inverted */
#define IOCON_PIO_MODE_INACT          0x00u   /*!< No addition pin function */
#define IOCON_PIO_OPENDRAIN_DI        0x00u   /*!< Open drain is disabled */
#define IOCON_PIO_SLEW_FAST         0x0400u   /*!< Fast mode, slew rate control is disabled */
#define IOCON_PIO_SLEW_STANDARD       0x00u   /*!< Standard mode, output slew rate control is enabled */
#define PIN3_IDX                         3u   /*!< Pin number for pin 3 in a port 2 */
#define PIN4_IDX                         4u   /*!< Pin number for pin 4 in a port 2 */
#define PIN5_IDX                         5u   /*!< Pin number for pin 5 in a port 2 */
#define PIN6_IDX                         6u   /*!< Pin number for pin 6 in a port 2 */
#define PIN7_IDX                         7u   /*!< Pin number for pin 7 in a port 2 */
#define PIN8_IDX                         8u   /*!< Pin number for pin 8 in a port 2 */
#define PIN9_IDX                         9u   /*!< Pin number for pin 9 in a port 2 */
#define PIN10_IDX                       10u   /*!< Pin number for pin 10 in a port 2 */
#define PIN15_IDX                       15u   /*!< Pin number for pin 15 in a port 3 */
#define PIN29_IDX                       29u   /*!< Pin number for pin 29 in a port 0 */
#define PIN30_IDX                       30u   /*!< Pin number for pin 30 in a port 0 */
#define PORT0_IDX                        0u   /*!< Port index */
#define PORT2_IDX                        2u   /*!< Port index */
#define PORT3_IDX                        3u   /*!< Port index */

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPins:
- options: {callFromInitBoot: 'true', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: A2, peripheral: FLEXCOMM0, signal: TXD_SCL_MISO, pin_signal: PIO0_30/FC0_TXD_SCL_MISO/CTIMER0_MAT0/SCT0_OUT9/TRACEDATA(1), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: standard, open_drain: disabled}
  - {pin_num: B13, peripheral: FLEXCOMM0, signal: RXD_SDA_MOSI, pin_signal: PIO0_29/FC0_RXD_SDA_MOSI/CTIMER2_MAT3/SCT0_OUT8/TRACEDATA(2), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: standard, open_drain: disabled}
  - {pin_num: B1, peripheral: SDIF, signal: SD_CLK, pin_signal: PIO2_3/ENET_TXD2/SD_CLK/FC1_RXD_SDA_MOSI/CTIMER2_MAT0, mode: inactive, invert: disabled, glitch_filter: disabled,
    slew_rate: fast, open_drain: disabled}
  - {pin_num: D3, peripheral: SDIF, signal: SD_CMD, pin_signal: PIO2_4/ENET_TXD3/SD_CMD/FC1_TXD_SCL_MISO/CTIMER2_MAT1, mode: inactive, invert: disabled, glitch_filter: disabled,
    slew_rate: fast, open_drain: disabled}
  - {pin_num: C1, peripheral: SDIF, signal: SD_POW_EN, pin_signal: PIO2_5/ENET_TX_ER/SD_POW_EN/FC1_CTS_SDA_SSEL0/CTIMER1_MAT2, mode: inactive, invert: disabled, glitch_filter: disabled,
    slew_rate: standard, open_drain: disabled}
  - {pin_num: F3, peripheral: SDIF, signal: 'SD_D, 0', pin_signal: PIO2_6/ENET_TX_CLK/SD_D(0)/FC1_RTS_SCL_SSEL1/CTIMER0_CAP0, mode: inactive, invert: disabled, glitch_filter: disabled,
    slew_rate: fast, open_drain: disabled}
  - {pin_num: J2, peripheral: SDIF, signal: 'SD_D, 1', pin_signal: PIO2_7/ENET_COL/SD_D(1)/FREQME_GPIO_CLK_B/CTIMER0_CAP1, mode: inactive, invert: disabled, glitch_filter: disabled,
    slew_rate: fast, open_drain: disabled}
  - {pin_num: F4, peripheral: SDIF, signal: 'SD_D, 2', pin_signal: PIO2_8/ENET_RXD2/SD_D(2)/CTIMER0_MAT0, mode: inactive, invert: disabled, glitch_filter: disabled,
    slew_rate: fast, open_drain: disabled}
  - {pin_num: K2, peripheral: SDIF, signal: 'SD_D, 3', pin_signal: PIO2_9/ENET_RXD3/SD_D(3)/CTIMER0_MAT1, mode: inactive, invert: disabled, glitch_filter: disabled,
    slew_rate: fast, open_drain: disabled}
  - {pin_num: P1, peripheral: SDIF, signal: SD_CARD_DET, pin_signal: PIO2_10/ENET_RX_ER/SD_CARD_DET_N, mode: inactive, invert: disabled, glitch_filter: disabled,
    slew_rate: standard, open_drain: disabled}
  - {pin_num: D2, peripheral: SDIF, signal: SD_WR_PRT, pin_signal: PIO3_15/FC8_SCK/SD_WR_PRT, mode: inactive, invert: disabled, glitch_filter: disabled, slew_rate: standard,
    open_drain: disabled}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/*FUNCTION**********************************************************************
 *
 * Function Name : BOARD_InitPins
 *
 *END**************************************************************************/
void BOARD_InitPins(void) { /* Function assigned for the Core #0 (ARM Cortex-M4) */
  CLOCK_EnableClock(kCLOCK_Iocon);                           /* Enables the clock for the IOCON block. 0 = Disable; 1 = Enable.: 0x01u */

  const uint32_t port0_pin29_config = (
    IOCON_PIO_FUNC1 |                                        /* Pin is configured as FC0_RXD_SDA_MOSI */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_STANDARD |                                /* Standard mode, output slew rate control is enabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN29_IDX, port0_pin29_config); /* PORT0 PIN29 (coords: B13) is configured as FC0_RXD_SDA_MOSI */
  const uint32_t port0_pin30_config = (
    IOCON_PIO_FUNC1 |                                        /* Pin is configured as FC0_TXD_SCL_MISO */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_STANDARD |                                /* Standard mode, output slew rate control is enabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN30_IDX, port0_pin30_config); /* PORT0 PIN30 (coords: A2) is configured as FC0_TXD_SCL_MISO */
  const uint32_t port2_pin10_config = (
    IOCON_PIO_FUNC2 |                                        /* Pin is configured as SD_CARD_DET_N */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_STANDARD |                                /* Standard mode, output slew rate control is enabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT2_IDX, PIN10_IDX, port2_pin10_config); /* PORT2 PIN10 (coords: P1) is configured as SD_CARD_DET_N */
  const uint32_t port2_pin3_config = (
    IOCON_PIO_FUNC2 |                                        /* Pin is configured as SD_CLK */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT2_IDX, PIN3_IDX, port2_pin3_config); /* PORT2 PIN3 (coords: B1) is configured as SD_CLK */
  const uint32_t port2_pin4_config = (
    IOCON_PIO_FUNC2 |                                        /* Pin is configured as SD_CMD */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT2_IDX, PIN4_IDX, port2_pin4_config); /* PORT2 PIN4 (coords: D3) is configured as SD_CMD */
  const uint32_t port2_pin5_config = (
    IOCON_PIO_FUNC2 |                                        /* Pin is configured as SD_POW_EN */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_STANDARD |                                /* Standard mode, output slew rate control is enabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT2_IDX, PIN5_IDX, port2_pin5_config); /* PORT2 PIN5 (coords: C1) is configured as SD_POW_EN */
  const uint32_t port2_pin6_config = (
    IOCON_PIO_FUNC2 |                                        /* Pin is configured as SD_D(0) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT2_IDX, PIN6_IDX, port2_pin6_config); /* PORT2 PIN6 (coords: F3) is configured as SD_D(0) */
  const uint32_t port2_pin7_config = (
    IOCON_PIO_FUNC2 |                                        /* Pin is configured as SD_D(1) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT2_IDX, PIN7_IDX, port2_pin7_config); /* PORT2 PIN7 (coords: J2) is configured as SD_D(1) */
  const uint32_t port2_pin8_config = (
    IOCON_PIO_FUNC2 |                                        /* Pin is configured as SD_D(2) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT2_IDX, PIN8_IDX, port2_pin8_config); /* PORT2 PIN8 (coords: F4) is configured as SD_D(2) */
  const uint32_t port2_pin9_config = (
    IOCON_PIO_FUNC2 |                                        /* Pin is configured as SD_D(3) */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT2_IDX, PIN9_IDX, port2_pin9_config); /* PORT2 PIN9 (coords: K2) is configured as SD_D(3) */
  const uint32_t port3_pin15_config = (
    IOCON_PIO_FUNC2 |                                        /* Pin is configured as SD_WR_PRT */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_STANDARD |                                /* Standard mode, output slew rate control is enabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT3_IDX, PIN15_IDX, port3_pin15_config); /* PORT3 PIN15 (coords: D2) is configured as SD_WR_PRT */
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
