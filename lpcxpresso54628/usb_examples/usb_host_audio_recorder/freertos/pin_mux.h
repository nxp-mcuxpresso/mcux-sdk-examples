/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

/*!
 * @addtogroup pin_mux
 * @{
 */

/***********************************************************************************************************************
 * API
 **********************************************************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Calls initialization functions.
 *
 */
void BOARD_InitBootPins(void);

#define IOCON_PIO_DIGITAL_EN 0x0100u  /*!<@brief Enables digital function */
#define IOCON_PIO_FUNC1 0x01u         /*!<@brief Selects pin function 1 */
#define IOCON_PIO_FUNC2 0x02u         /*!<@brief Selects pin function 2 */
#define IOCON_PIO_FUNC3 0x03u         /*!<@brief Selects pin function 3 */
#define IOCON_PIO_FUNC7 0x07u         /*!<@brief Selects pin function 7 */
#define IOCON_PIO_INPFILT_OFF 0x0200u /*!<@brief Input filter disabled */
#define IOCON_PIO_INV_DI 0x00u        /*!<@brief Input function is not inverted */
#define IOCON_PIO_MODE_INACT 0x00u    /*!<@brief No addition pin function */
#define IOCON_PIO_MODE_PULLUP 0x20u   /*!<@brief Selects pull-up function */
#define IOCON_PIO_OPENDRAIN_DI 0x00u  /*!<@brief Open drain is disabled */
#define IOCON_PIO_SLEW_FAST 0x0400u   /*!<@brief Fast mode, slew rate control is disabled */
#define IOCON_PIO_SLEW_STANDARD 0x00u /*!<@brief Standard mode, output slew rate control is enabled */

/*! @name PIO0_30 (coord A2), U24[12]/P0_30-ISP_FC0_TXD
  @{ */
#define BOARD_INITPINS_ISP_FC0_TXD_PORT 0U                   /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_ISP_FC0_TXD_PIN 30U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_ISP_FC0_TXD_PIN_MASK (1U << 30U)      /*!<@brief PORT pin mask */
                                                             /* @} */

/*! @name PIO0_29 (coord B13), U24[13]/P0_29-ISP_FC0_RXD
  @{ */
#define BOARD_INITPINS_ISP_FC0_RXD_PORT 0U                   /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_ISP_FC0_RXD_PIN 29U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_ISP_FC0_RXD_PIN_MASK (1U << 29U)      /*!<@brief PORT pin mask */
                                                             /* @} */

/*! @name PIO2_3 (coord B1), U9[3]/P2_3-SD_CLK
  @{ */
#define BOARD_INITPINS_SD_CLK_PORT 2U                  /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_SD_CLK_PIN 3U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_SD_CLK_PIN_MASK (1U << 3U)      /*!<@brief PORT pin mask */
                                                       /* @} */

/*! @name PIO2_4 (coord D3), RP1[6]/U9[2]/P2_4-SD_CMD
  @{ */
#define BOARD_INITPINS_SD_CMD_PORT 2U                  /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_SD_CMD_PIN 4U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_SD_CMD_PIN_MASK (1U << 4U)      /*!<@brief PORT pin mask */
                                                       /* @} */

/*! @name PIO2_5 (coord C1), Q3[1]/P2_5-SD_POW_EN
  @{ */
#define BOARD_INITPINS_SD_POW_EN_PORT 2U                  /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_SD_POW_EN_PIN 5U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_SD_POW_EN_PIN_MASK (1U << 5U)      /*!<@brief PORT pin mask */
                                                          /* @} */

/*! @name PIO2_6 (coord F3), RP1[5]/U9[4]/P2_6-SD_D0
  @{ */
#define BOARD_INITPINS_SD_D0_PORT 2U                  /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_SD_D0_PIN 6U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_SD_D0_PIN_MASK (1U << 6U)      /*!<@brief PORT pin mask */
                                                      /* @} */

/*! @name PIO2_7 (coord J2), RP1[4]/U9[5]/P2_7-SD_D1
  @{ */
#define BOARD_INITPINS_SD_D1_PORT 2U                  /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_SD_D1_PIN 7U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_SD_D1_PIN_MASK (1U << 7U)      /*!<@brief PORT pin mask */
                                                      /* @} */

/*! @name PIO2_8 (coord F4), RP1[3]/U9[6]/P2_8-SD_D2
  @{ */
#define BOARD_INITPINS_SD_D2_PORT 2U                  /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_SD_D2_PIN 8U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_SD_D2_PIN_MASK (1U << 8U)      /*!<@brief PORT pin mask */
                                                      /* @} */

/*! @name PIO2_9 (coord K2), RP1[7]/U9[1]/P2_9-SD_D3
  @{ */
#define BOARD_INITPINS_SD_D3_PORT 2U                  /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_SD_D3_PIN 9U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_SD_D3_PIN_MASK (1U << 9U)      /*!<@brief PORT pin mask */
                                                      /* @} */

/*! @name PIO2_10 (coord P1), RP1[2]/U9[7]/P2_10-SD_CDn
  @{ */
#define BOARD_INITPINS_SD_CDn_PORT 2U                   /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_SD_CDn_PIN 10U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_SD_CDn_PIN_MASK (1U << 10U)      /*!<@brief PORT pin mask */
                                                        /* @} */

/*! @name PIO3_15 (coord D2), RP1[1]/U9[8]/P3_15-SD_WPn
  @{ */
#define BOARD_INITPINS_SD_WPn_PORT 3U                   /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_SD_WPn_PIN 15U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_SD_WPn_PIN_MASK (1U << 15U)      /*!<@brief PORT pin mask */
                                                        /* @} */

/*! @name PIO0_22 (coord B12), U3[4]/J3[1]/P0_22-USB0_VBUS
  @{ */
#define BOARD_INITPINS_USB0_VBUS_PORT 0U                   /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_USB0_VBUS_PIN 22U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_USB0_VBUS_PIN_MASK (1U << 22U)      /*!<@brief PORT pin mask */
                                                           /* @} */

/*! @name PIO1_1 (coord K12), J12[12]/U29[4]/SW5/P1_1-USER_PB-USB1_OVRCURn
  @{ */
#define BOARD_INITPINS_SW5_PORT 1U                  /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_SW5_PIN 1U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_SW5_PIN_MASK (1U << 1U)      /*!<@brief PORT pin mask */
                                                    /* @} */

/*! @name PIO4_8 (coord B14), U12[17]/JP11[1]/P4_8-ENET_TXD0
  @{ */
#define BOARD_INITPINS_ENET_TXD0_PORT 4U                  /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_ENET_TXD0_PIN 8U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_ENET_TXD0_PIN_MASK (1U << 8U)      /*!<@brief PORT pin mask */
                                                          /* @} */

/*! @name PIO1_27 (coord F10), U28[G3]/P1_27-EMC_A9
  @{ */
#define BOARD_INITPINS_EMC_A9_PORT 1U                   /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_EMC_A9_PIN 27U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_EMC_A9_PIN_MASK (1U << 27U)      /*!<@brief PORT pin mask */
                                                        /* @} */

/*! @name PIO1_28 (coord E12), U28[C1]/RP2[4]/P1_28-EMC_D12
  @{ */
#define BOARD_INITPINS_EMC_D12_PORT 1U                   /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_EMC_D12_PIN 28U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_EMC_D12_PIN_MASK (1U << 28U)      /*!<@brief PORT pin mask */
                                                         /* @} */

/*! @name PIO1_29 (coord C11), U28[B2]/RP2[3]/P1_29-EMC_D13
  @{ */
#define BOARD_INITPINS_EMC_D13_PORT 1U                   /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_EMC_D13_PIN 29U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_EMC_D13_PIN_MASK (1U << 29U)      /*!<@brief PORT pin mask */
                                                         /* @} */

/*! @name PIO1_30 (coord A8), U28[B1]/RP2[2]/P1_30-EMC_D14
  @{ */
#define BOARD_INITPINS_EMC_D14_PORT 1U                   /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_EMC_D14_PIN 30U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_EMC_D14_PIN_MASK (1U << 30U)      /*!<@brief PORT pin mask */
                                                         /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPins(void); /* Function assigned for the Cortex-M4F */

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */
#endif /* _PIN_MUX_H_ */

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
