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
processor: LPC54628J512
package_id: LPC54628J512ET180
mcu_data: ksdk2_0
processor_version: 0.9.0
board: LPCXpresso54628
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
- options: {callFromInitBoot: 'true', prefix: BOARD_, coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: B13, peripheral: FLEXCOMM0, signal: RXD_SDA_MOSI, pin_signal: PIO0_29/FC0_RXD_SDA_MOSI/CTIMER2_MAT3/SCT0_OUT8/TRACEDATA(2), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: standard, open_drain: disabled}
  - {pin_num: A2, peripheral: FLEXCOMM0, signal: TXD_SCL_MISO, pin_signal: PIO0_30/FC0_TXD_SCL_MISO/CTIMER0_MAT0/SCT0_OUT9/TRACEDATA(1), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: standard, open_drain: disabled}
  - {pin_num: P2, peripheral: SWD, signal: SWO, pin_signal: PIO0_10/FC6_SCK/CTIMER2_CAP2/CTIMER2_MAT0/FC1_TXD_SCL_MISO/SWO/ADC0_0, mode: inactive, invert: disabled,
    glitch_filter: disabled, open_drain: disabled}
  - {pin_num: D1, peripheral: FLEXCOMM8, signal: TXD_SCL_MISO, pin_signal: PIO1_18/FC8_TXD_SCL_MISO/SCT0_OUT5/CAN1_RD/EMC_BLSN(1)}
  - {pin_num: E1, peripheral: FLEXCOMM8, signal: RXD_SDA_MOSI, pin_signal: PIO3_16/FC8_RXD_SDA_MOSI/SD_D(4)}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
/* Function assigned for the Cortex-M4F */
void BOARD_InitPins(void)
{
    /* Enables the clock for the IOCON block. 0 = Disable; 1 = Enable.: 0x01u */
    CLOCK_EnableClock(kCLOCK_Iocon);

    const uint32_t SWO_TRGT = (/* Pin is configured as SWO */
                               IOCON_PIO_FUNC6 |
                               /* No addition pin function */
                               IOCON_PIO_MODE_INACT |
                               /* Input function is not inverted */
                               IOCON_PIO_INV_DI |
                               /* Enables digital function */
                               IOCON_PIO_DIGITAL_EN |
                               /* Input filter disabled */
                               IOCON_PIO_INPFILT_OFF |
                               /* Open drain is disabled */
                               IOCON_PIO_OPENDRAIN_DI);
    /* PORT0 PIN10 (coords: P2) is configured as SWO */
    IOCON_PinMuxSet(IOCON, BOARD_SWO_TRGT_PORT, BOARD_SWO_TRGT_PIN, SWO_TRGT);

    const uint32_t ISP_FC0_RXD = (/* Pin is configured as FC0_RXD_SDA_MOSI */
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
    /* PORT0 PIN29 (coords: B13) is configured as FC0_RXD_SDA_MOSI */
    IOCON_PinMuxSet(IOCON, BOARD_ISP_FC0_RXD_PORT, BOARD_ISP_FC0_RXD_PIN, ISP_FC0_RXD);

    const uint32_t ISP_FC0_TXD = (/* Pin is configured as FC0_TXD_SCL_MISO */
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
    /* PORT0 PIN30 (coords: A2) is configured as FC0_TXD_SCL_MISO */
    IOCON_PinMuxSet(IOCON, BOARD_ISP_FC0_TXD_PORT, BOARD_ISP_FC0_TXD_PIN, ISP_FC0_TXD);

    IOCON->PIO[1][18] = ((IOCON->PIO[1][18] &
                          /* Mask bits to zero which are setting */
                          (~(IOCON_PIO_FUNC_MASK | IOCON_PIO_DIGIMODE_MASK)))

                         /* Selects pin function.
                          * : PORT118 (pin D1) is configured as FC8_TXD_SCL_MISO. */
                         | IOCON_PIO_FUNC(PIO118_FUNC_ALT2)

                         /* Select Analog/Digital mode.
                          * : Digital mode. */
                         | IOCON_PIO_DIGIMODE(PIO118_DIGIMODE_DIGITAL));

    IOCON->PIO[3][16] = ((IOCON->PIO[3][16] &
                          /* Mask bits to zero which are setting */
                          (~(IOCON_PIO_FUNC_MASK | IOCON_PIO_DIGIMODE_MASK)))

                         /* Selects pin function.
                          * : PORT316 (pin E1) is configured as FC8_RXD_SDA_MOSI. */
                         | IOCON_PIO_FUNC(PIO316_FUNC_ALT1)

                         /* Select Analog/Digital mode.
                          * : Digital mode. */
                         | IOCON_PIO_DIGIMODE(PIO316_DIGIMODE_DIGITAL));
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
