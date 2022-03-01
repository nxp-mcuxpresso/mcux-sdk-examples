/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Pins v4.0
processor: LPC51U68
package_id: LPC51U68JBD64
mcu_data: ksdk2_0
processor_version: 0.0.0
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

#include "fsl_common.h"
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

#define PIO00_DIGIMODE_DIGITAL        0x01u   /*!< Select Analog/Digital mode.: Digital mode. */
#define PIO00_FUNC_ALT1               0x01u   /*!< Selects pin function.: Alternative connection 1. */
#define PIO01_DIGIMODE_DIGITAL        0x01u   /*!< Select Analog/Digital mode.: Digital mode. */
#define PIO01_FUNC_ALT1               0x01u   /*!< Selects pin function.: Alternative connection 1. */

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPins:
- options: {callFromInitBoot: 'true', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '31', peripheral: FLEXCOMM0, signal: RXD_SDA_MOSI, pin_signal: PIO0_0/FC0_RXD_SDA_MOSI/FC3_CTS_SDA_SSEL0/CTIMER0_CAP0/SCT0_OUT3}
  - {pin_num: '32', peripheral: FLEXCOMM0, signal: TXD_SCL_MISO, pin_signal: PIO0_1/FC0_TXD_SCL_MISO/FC3_RTS_SCL_SSEL1/CTIMER0_CAP1/SCT0_OUT1}
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

  IOCON->PIO[0][0] = ((IOCON->PIO[0][0] &
    (~(IOCON_PIO_FUNC_MASK | IOCON_PIO_DIGIMODE_MASK)))      /* Mask bits to zero which are setting */
      | IOCON_PIO_FUNC(PIO00_FUNC_ALT1)                      /* Selects pin function.: PORT00 (pin 31) is configured as FC0_RXD_SDA_MOSI */
      | IOCON_PIO_DIGIMODE(PIO00_DIGIMODE_DIGITAL)           /* Select Analog/Digital mode.: Digital mode. */
    );
  IOCON->PIO[0][1] = ((IOCON->PIO[0][1] &
    (~(IOCON_PIO_FUNC_MASK | IOCON_PIO_DIGIMODE_MASK)))      /* Mask bits to zero which are setting */
      | IOCON_PIO_FUNC(PIO01_FUNC_ALT1)                      /* Selects pin function.: PORT01 (pin 32) is configured as FC0_TXD_SCL_MISO */
      | IOCON_PIO_DIGIMODE(PIO01_DIGIMODE_DIGITAL)           /* Select Analog/Digital mode.: Digital mode. */
    );
}


#define PIO011_DIGIMODE_DIGITAL       0x01u   /*!< Select Analog/Digital mode.: Digital mode. */
#define PIO011_FUNC_ALT1              0x01u   /*!< Selects pin function.: Alternative connection 1. */
#define PIO012_DIGIMODE_DIGITAL       0x01u   /*!< Select Analog/Digital mode.: Digital mode. */
#define PIO012_FUNC_ALT1              0x01u   /*!< Selects pin function.: Alternative connection 1. */
#define PIO013_DIGIMODE_DIGITAL       0x01u   /*!< Select Analog/Digital mode.: Digital mode. */
#define PIO013_FUNC_ALT1              0x01u   /*!< Selects pin function.: Alternative connection 1. */
#define PIO04_DIGIMODE_DIGITAL        0x01u   /*!< Select Analog/Digital mode.: Digital mode. */
#define PIO04_FUNC_ALT2               0x02u   /*!< Selects pin function.: Alternative connection 2. */

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
SPI3_InitPins:
- options: {coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '47', peripheral: FLEXCOMM3, signal: RXD_SDA_MOSI, pin_signal: PIO0_12/FC3_RXD_SDA_MOSI/FC6_TXD_SCL_MISO_WS/CTIMER2_MAT3}
  - {pin_num: '46', peripheral: FLEXCOMM3, signal: SCK, pin_signal: PIO0_11/FC3_SCK/FC6_RXD_SDA_MOSI_DATA/CTIMER2_MAT1}
  - {pin_num: '38', peripheral: FLEXCOMM3, signal: SSEL2, pin_signal: PIO0_4/FC0_SCK/FC3_SSEL2/CTIMER0_CAP2}
  - {pin_num: '48', peripheral: FLEXCOMM3, signal: TXD_SCL_MISO, pin_signal: PIO0_13/FC3_TXD_SCL_MISO/SCT0_OUT4/CTIMER2_MAT0}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/*FUNCTION**********************************************************************
 *
 * Function Name : SPI3_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 *END**************************************************************************/
void SPI3_InitPins(void) { /* Function assigned for the Cortex-M0P */
  CLOCK_EnableClock(kCLOCK_Iocon);                           /* Enables the clock for the IOCON block. 0 = Disable; 1 = Enable.: 0x01u */

  IOCON->PIO[0][11] = ((IOCON->PIO[0][11] &
    (~(IOCON_PIO_FUNC_MASK | IOCON_PIO_DIGIMODE_MASK)))      /* Mask bits to zero which are setting */
      | IOCON_PIO_FUNC(PIO011_FUNC_ALT1)                     /* Selects pin function.: PORT011 (pin 46) is configured as FC3_SCK */
      | IOCON_PIO_DIGIMODE(PIO011_DIGIMODE_DIGITAL)          /* Select Analog/Digital mode.: Digital mode. */
    );
  IOCON->PIO[0][12] = ((IOCON->PIO[0][12] &
    (~(IOCON_PIO_FUNC_MASK | IOCON_PIO_DIGIMODE_MASK)))      /* Mask bits to zero which are setting */
      | IOCON_PIO_FUNC(PIO012_FUNC_ALT1)                     /* Selects pin function.: PORT012 (pin 47) is configured as FC3_RXD_SDA_MOSI */
      | IOCON_PIO_DIGIMODE(PIO012_DIGIMODE_DIGITAL)          /* Select Analog/Digital mode.: Digital mode. */
    );
  IOCON->PIO[0][13] = ((IOCON->PIO[0][13] &
    (~(IOCON_PIO_FUNC_MASK | IOCON_PIO_DIGIMODE_MASK)))      /* Mask bits to zero which are setting */
      | IOCON_PIO_FUNC(PIO013_FUNC_ALT1)                     /* Selects pin function.: PORT013 (pin 48) is configured as FC3_TXD_SCL_MISO */
      | IOCON_PIO_DIGIMODE(PIO013_DIGIMODE_DIGITAL)          /* Select Analog/Digital mode.: Digital mode. */
    );
  IOCON->PIO[0][4] = ((IOCON->PIO[0][4] &
    (~(IOCON_PIO_FUNC_MASK | IOCON_PIO_DIGIMODE_MASK)))      /* Mask bits to zero which are setting */
      | IOCON_PIO_FUNC(PIO04_FUNC_ALT2)                      /* Selects pin function.: PORT04 (pin 38) is configured as FC3_SSEL2 */
      | IOCON_PIO_DIGIMODE(PIO04_DIGIMODE_DIGITAL)           /* Select Analog/Digital mode.: Digital mode. */
    );
}


#define PIO011_DIGIMODE_DIGITAL       0x01u   /*!< Select Analog/Digital mode.: Digital mode. */
#define PIO011_FUNC_ALT0              0x00u   /*!< Selects pin function.: Alternative connection 0. */
#define PIO012_DIGIMODE_DIGITAL       0x01u   /*!< Select Analog/Digital mode.: Digital mode. */
#define PIO012_FUNC_ALT0              0x00u   /*!< Selects pin function.: Alternative connection 0. */
#define PIO013_DIGIMODE_DIGITAL       0x01u   /*!< Select Analog/Digital mode.: Digital mode. */
#define PIO013_FUNC_ALT0              0x00u   /*!< Selects pin function.: Alternative connection 0. */
#define PIO04_DIGIMODE_DIGITAL        0x01u   /*!< Select Analog/Digital mode.: Digital mode. */
#define PIO04_FUNC_ALT0               0x00u   /*!< Selects pin function.: Alternative connection 0. */

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
SPI3_DeinitPins:
- options: {coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '47', peripheral: GPIO, signal: 'PIO0, 12', pin_signal: PIO0_12/FC3_RXD_SDA_MOSI/FC6_TXD_SCL_MISO_WS/CTIMER2_MAT3}
  - {pin_num: '46', peripheral: GPIO, signal: 'PIO0, 11', pin_signal: PIO0_11/FC3_SCK/FC6_RXD_SDA_MOSI_DATA/CTIMER2_MAT1}
  - {pin_num: '38', peripheral: GPIO, signal: 'PIO0, 4', pin_signal: PIO0_4/FC0_SCK/FC3_SSEL2/CTIMER0_CAP2}
  - {pin_num: '48', peripheral: GPIO, signal: 'PIO0, 13', pin_signal: PIO0_13/FC3_TXD_SCL_MISO/SCT0_OUT4/CTIMER2_MAT0}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/*FUNCTION**********************************************************************
 *
 * Function Name : SPI3_DeinitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 *END**************************************************************************/
void SPI3_DeinitPins(void) { /* Function assigned for the Cortex-M0P */
  CLOCK_EnableClock(kCLOCK_Iocon);                           /* Enables the clock for the IOCON block. 0 = Disable; 1 = Enable.: 0x01u */

  IOCON->PIO[0][11] = ((IOCON->PIO[0][11] &
    (~(IOCON_PIO_FUNC_MASK | IOCON_PIO_DIGIMODE_MASK)))      /* Mask bits to zero which are setting */
      | IOCON_PIO_FUNC(PIO011_FUNC_ALT0)                     /* Selects pin function.: PORT011 (pin 46) is configured as PIO0_11 */
      | IOCON_PIO_DIGIMODE(PIO011_DIGIMODE_DIGITAL)          /* Select Analog/Digital mode.: Digital mode. */
    );
  IOCON->PIO[0][12] = ((IOCON->PIO[0][12] &
    (~(IOCON_PIO_FUNC_MASK | IOCON_PIO_DIGIMODE_MASK)))      /* Mask bits to zero which are setting */
      | IOCON_PIO_FUNC(PIO012_FUNC_ALT0)                     /* Selects pin function.: PORT012 (pin 47) is configured as PIO0_12 */
      | IOCON_PIO_DIGIMODE(PIO012_DIGIMODE_DIGITAL)          /* Select Analog/Digital mode.: Digital mode. */
    );
  IOCON->PIO[0][13] = ((IOCON->PIO[0][13] &
    (~(IOCON_PIO_FUNC_MASK | IOCON_PIO_DIGIMODE_MASK)))      /* Mask bits to zero which are setting */
      | IOCON_PIO_FUNC(PIO013_FUNC_ALT0)                     /* Selects pin function.: PORT013 (pin 48) is configured as PIO0_13 */
      | IOCON_PIO_DIGIMODE(PIO013_DIGIMODE_DIGITAL)          /* Select Analog/Digital mode.: Digital mode. */
    );
  IOCON->PIO[0][4] = ((IOCON->PIO[0][4] &
    (~(IOCON_PIO_FUNC_MASK | IOCON_PIO_DIGIMODE_MASK)))      /* Mask bits to zero which are setting */
      | IOCON_PIO_FUNC(PIO04_FUNC_ALT0)                      /* Selects pin function.: PORT04 (pin 38) is configured as PIO0_4 */
      | IOCON_PIO_DIGIMODE(PIO04_DIGIMODE_DIGITAL)           /* Select Analog/Digital mode.: Digital mode. */
    );
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
