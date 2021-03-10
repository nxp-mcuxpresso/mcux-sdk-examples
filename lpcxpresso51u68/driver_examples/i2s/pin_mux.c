/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Pins v6.0
processor: LPC51U68
package_id: LPC51U68JBD64
mcu_data: ksdk2_0
processor_version: 0.0.11
board: LPCXpresso51U68
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

#define IOCON_PIO_DIGITAL_EN          0x80u   /*!< Enables digital function */
#define IOCON_PIO_FUNC1               0x01u   /*!< Selects pin function 1 */
#define IOCON_PIO_FUNC2               0x02u   /*!< Selects pin function 2 */
#define IOCON_PIO_FUNC4               0x04u   /*!< Selects pin function 4 */
#define IOCON_PIO_I2CDRIVE_HIGH     0x0200u   /*!< High drive: 20 mA */
#define IOCON_PIO_I2CFILTER_DI      0x0400u   /*!< I2C 50 ns glitch filter disabled */
#define IOCON_PIO_I2CSLEW_I2C         0x00u   /*!< I2C mode */
#define IOCON_PIO_INPFILT_OFF       0x0100u   /*!< Input filter disabled */
#define IOCON_PIO_INV_DI              0x00u   /*!< Input function is not inverted */
#define IOCON_PIO_MODE_INACT          0x00u   /*!< No addition pin function */
#define IOCON_PIO_MODE_PULLUP         0x10u   /*!< Selects pull-up function */
#define IOCON_PIO_OPENDRAIN_DI        0x00u   /*!< Open drain is disabled */
#define IOCON_PIO_SLEW_FAST         0x0200u   /*!< Fast mode, slew rate control is disabled */
#define IOCON_PIO_SLEW_STANDARD       0x00u   /*!< Standard mode, output slew rate control is enabled */
#define PIN0_IDX                         0u   /*!< Pin number for pin 0 in a port 0 */
#define PIN1_IDX                         1u   /*!< Pin number for pin 1 in a port 0 */
#define PIN5_IDX                         5u   /*!< Pin number for pin 5 in a port 0 */
#define PIN6_IDX                         6u   /*!< Pin number for pin 6 in a port 0 */
#define PIN7_IDX                         7u   /*!< Pin number for pin 7 in a port 0 */
#define PIN8_IDX                         8u   /*!< Pin number for pin 8 in a port 1 */
#define PIN12_IDX                       12u   /*!< Pin number for pin 12 in a port 1 */
#define PIN17_IDX                       17u   /*!< Pin number for pin 17 in a port 1 */
#define PIN25_IDX                       25u   /*!< Pin number for pin 25 in a port 0 */
#define PIN26_IDX                       26u   /*!< Pin number for pin 26 in a port 0 */
#define PORT0_IDX                        0u   /*!< Port index */
#define PORT1_IDX                        1u   /*!< Port index */

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPins:
- options: {callFromInitBoot: 'true', coreID: core0, enableClock: 'true'}
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
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: '40', peripheral: FLEXCOMM6, signal: TXD_SCL_MISO_WS, pin_signal: PIO0_6/FC6_TXD_SCL_MISO_WS/CTIMER0_MAT1/UTICK_CAP0, mode: pullUp, invert: disabled,
    glitch_filter: disabled, slew_rate: fast, open_drain: disabled}
  - {pin_num: '41', peripheral: FLEXCOMM6, signal: SCK, pin_signal: PIO0_7/FC6_SCK/SCT0_OUT0/CTIMER0_MAT2/CTIMER0_CAP2, mode: pullUp, invert: disabled, glitch_filter: disabled,
    slew_rate: fast, open_drain: disabled}
  - {pin_num: '28', peripheral: FLEXCOMM7, signal: TXD_SCL_MISO_WS, pin_signal: PIO1_8/FC7_TXD_SCL_MISO_WS/CTIMER1_MAT3/CTIMER1_CAP3/ADC0_11, mode: pullUp, invert: disabled,
    glitch_filter: disabled, open_drain: disabled}
  - {pin_num: '51', peripheral: FLEXCOMM7, signal: SCK, pin_signal: PIO1_12/FC5_RXD_SDA_MOSI/CTIMER1_MAT0/FC7_SCK/UTICK_CAP2, mode: pullUp, invert: disabled, glitch_filter: disabled,
    slew_rate: fast, open_drain: disabled}
  - {pin_num: '27', peripheral: FLEXCOMM7, signal: RXD_SDA_MOSI_DATA, pin_signal: PIO1_7/FC7_RXD_SDA_MOSI_DATA/CTIMER1_MAT2/CTIMER1_CAP2/ADC0_10, mode: pullUp, invert: disabled,
    glitch_filter: disabled, open_drain: disabled}
  - {pin_num: '10', peripheral: SYSCON, signal: MCLK, pin_signal: PIO1_17/MCLK/UTICK_CAP3, mode: inactive, invert: disabled, glitch_filter: disabled, slew_rate: standard,
    open_drain: disabled}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/*FUNCTION**********************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 *END**************************************************************************/
void BOARD_InitPins(void) { /* Function assigned for the Cortex-M0P */
  CLOCK_EnableClock(kCLOCK_Iocon);                           /* Enables the clock for the IOCON block. 0 = Disable; 1 = Enable.: 0x01u */

  const uint32_t port0_pin0_config = (
    IOCON_PIO_FUNC1 |                                        /* Pin is configured as FC0_RXD_SDA_MOSI */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_STANDARD |                                /* Standard mode, output slew rate control is enabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN0_IDX, port0_pin0_config); /* PORT0 PIN0 (coords: 31) is configured as FC0_RXD_SDA_MOSI */
  const uint32_t port0_pin1_config = (
    IOCON_PIO_FUNC1 |                                        /* Pin is configured as FC0_TXD_SCL_MISO */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_STANDARD |                                /* Standard mode, output slew rate control is enabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN1_IDX, port0_pin1_config); /* PORT0 PIN1 (coords: 32) is configured as FC0_TXD_SCL_MISO */
  const uint32_t port0_pin25_config = (
    IOCON_PIO_FUNC1 |                                        /* Pin is configured as FC4_RTS_SCL_SSEL1 */
    IOCON_PIO_I2CSLEW_I2C |                                  /* I2C mode */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_I2CDRIVE_HIGH |                                /* High drive: 20 mA */
    IOCON_PIO_I2CFILTER_DI                                   /* I2C 50 ns glitch filter disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN25_IDX, port0_pin25_config); /* PORT0 PIN25 (coords: 3) is configured as FC4_RTS_SCL_SSEL1 */
  const uint32_t port0_pin26_config = (
    IOCON_PIO_FUNC1 |                                        /* Pin is configured as FC4_CTS_SDA_SSEL0 */
    IOCON_PIO_I2CSLEW_I2C |                                  /* I2C mode */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_I2CDRIVE_HIGH |                                /* High drive: 20 mA */
    IOCON_PIO_I2CFILTER_DI                                   /* I2C 50 ns glitch filter disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN26_IDX, port0_pin26_config); /* PORT0 PIN26 (coords: 4) is configured as FC4_CTS_SDA_SSEL0 */
  const uint32_t port0_pin5_config = (
    IOCON_PIO_FUNC1 |                                        /* Pin is configured as FC6_RXD_SDA_MOSI_DATA */
    IOCON_PIO_MODE_PULLUP |                                  /* Selects pull-up function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN5_IDX, port0_pin5_config); /* PORT0 PIN5 (coords: 39) is configured as FC6_RXD_SDA_MOSI_DATA */
  const uint32_t port0_pin6_config = (
    IOCON_PIO_FUNC1 |                                        /* Pin is configured as FC6_TXD_SCL_MISO_WS */
    IOCON_PIO_MODE_PULLUP |                                  /* Selects pull-up function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN6_IDX, port0_pin6_config); /* PORT0 PIN6 (coords: 40) is configured as FC6_TXD_SCL_MISO_WS */
  const uint32_t port0_pin7_config = (
    IOCON_PIO_FUNC1 |                                        /* Pin is configured as FC6_SCK */
    IOCON_PIO_MODE_PULLUP |                                  /* Selects pull-up function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT0_IDX, PIN7_IDX, port0_pin7_config); /* PORT0 PIN7 (coords: 41) is configured as FC6_SCK */
  const uint32_t port1_pin12_config = (
    IOCON_PIO_FUNC4 |                                        /* Pin is configured as FC7_SCK */
    IOCON_PIO_MODE_PULLUP |                                  /* Selects pull-up function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_FAST |                                    /* Fast mode, slew rate control is disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN12_IDX, port1_pin12_config); /* PORT1 PIN12 (coords: 51) is configured as FC7_SCK */
  const uint32_t port1_pin17_config = (
    IOCON_PIO_FUNC4 |                                        /* Pin is configured as MCLK */
    IOCON_PIO_MODE_INACT |                                   /* No addition pin function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_SLEW_STANDARD |                                /* Standard mode, output slew rate control is enabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN17_IDX, port1_pin17_config); /* PORT1 PIN17 (coords: 10) is configured as MCLK */
  const uint32_t port1_pin7_config = (
    IOCON_PIO_FUNC2 |                                        /* Pin is configured as FC7_RXD_SDA_MOSI_DATA */
    IOCON_PIO_MODE_PULLUP |                                  /* Selects pull-up function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN7_IDX, port1_pin7_config); /* PORT1 PIN7 (coords: 27) is configured as FC7_RXD_SDA_MOSI_DATA */
  const uint32_t port1_pin8_config = (
    IOCON_PIO_FUNC2 |                                        /* Pin is configured as FC7_TXD_SCL_MISO_WS */
    IOCON_PIO_MODE_PULLUP |                                  /* Selects pull-up function */
    IOCON_PIO_INV_DI |                                       /* Input function is not inverted */
    IOCON_PIO_DIGITAL_EN |                                   /* Enables digital function */
    IOCON_PIO_INPFILT_OFF |                                  /* Input filter disabled */
    IOCON_PIO_OPENDRAIN_DI                                   /* Open drain is disabled */
  );
  IOCON_PinMuxSet(IOCON, PORT1_IDX, PIN8_IDX, port1_pin8_config); /* PORT1 PIN8 (coords: 28) is configured as FC7_TXD_SCL_MISO_WS */
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
