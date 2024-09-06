/*
 * Copyright 2023-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BOARD_PLATFORM_H_
#define _BOARD_PLATFORM_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! \brief Change Default trimming value for 32MHz crystal,
      will be used by hardware_init.c file during initialization */
/* BOARD_32MHZ_XTAL_TRIM_DEFAULT macro is deprecated
   please use BOARD_32MHZ_XTAL_CDAC_VALUE now */
#define BOARD_32MHZ_XTAL_CDAC_VALUE 12U

/*! \brief Change Default amplifier current selected for 32MHz crystal,
      will be used by hardware_init.c file during initialization */
//#define BOARD_32MHZ_XTAL_ISEL_VALUE    7U

/*! \brief Default load capacitance config for 32KHz crystal,
      can be overidden from board.h,
      user shall define this flag in board.h file to set an other default value
      Values must be adjusted to minimize the jitter on the crystal. This is to avoid
      a shift of 31.25us on the link layer timebase in NBU.
*/
//#define BOARD_32KHZ_XTAL_CLOAD_DEFAULT 8U

/*! \brief Default coarse adjustement config for 32KHz crystal,
      user shall define this flag in board.h file to set an other default value
      Values must be adjusted depending the equivalent series resistance (ESR) of the crystal on the board
*/
//#define BOARD_32KHZ_XTAL_COARSE_ADJ_DEFAULT 1

/*! \brief Number of half slot before LL interrupt to wakeup the 32MHz of the NBU, depends on the 32Mhz crystal on the
 *  board. Default value is 2. Should be set to 3 when using FRO 32KHz.
 */
//#define BOARD_LL_32MHz_WAKEUP_ADVANCE_HSLOT 3

/*! \brief Delay from the 32MHz wake up of the LL to wake up the radio domain in number of 30.5us period. Default value
    is set to 0x10 in fwk_plaform.c file. The lower this value is, the safest it is, but it increases power consumption.
    If you increase too much this value it is possible to have some issue in LL scheduling event as the power domain
    will be awaken too late. The most power optimized value without impacting LL scheduling depends on the board and the
    value of BOARD_LL_32MHz_WAKEUP_ADVANCE_HSLOT. As an example for BOARD_LL_32MHz_WAKEUP_ADVANCE_HSLOT equals to 2 on
    NXP EVK we can increase it to 0x14. Default value is 0x10. Should be set to 0x0F when using FRO 32KHz.
 */
//#define BOARD_RADIO_DOMAIN_WAKE_UP_DELAY 0x0FU

/*! \brief Calibration settings for the FRO32K */
//#define BOARD_FRO32K_PPM_TARGET 200U
//#define BOARD_FRO32K_FILTER_SIZE 4U

/* Define the source clock accuracy. Possible values:
 * Value |       Accuracy in ppm
 *   0   |       251 ppm to 500 ppm
 *   1   |       151 ppm to 250 ppm
 *   2   |       101 ppm to 150 ppm
 *   3   |        76 ppm to 100 ppm
 *   4   |        51 ppm to  75 ppm
 *   5   |        31 ppm to  50 ppm
 *   6   |        21 ppm to  30 ppm
 *   7   |         0 ppm to  20 ppm
 */
//#define BOARD_32KHZ_SRC_CLK_ACCURACY 0

/* Define to 1 to use FRO 32KHz instead of crystal one, by default crystal will be used */
//#define gBoardUseFro32k_d 1

/* Define to 1 to use SWO on application core or 2 to use SWO on NBU core */
//#define BOARD_DBG_SWO_CORE_FUNNEL 1

/* Define to output SWO on a pin */
//#define BOARD_DBG_SWO_PIN_ENABLE 1

/*******************************************************************************
 * Definition checks
 ******************************************************************************/

#if (defined(BOARD_32KHZ_SRC_CLK_ACCURACY) && \
     ((BOARD_32KHZ_SRC_CLK_ACCURACY < 0) || (BOARD_32KHZ_SRC_CLK_ACCURACY > 7)))
#error "BOARD_32KHZ_SRC_CLK_ACCURACY shall be in the range 0 to 7."
#endif

#if (!defined(BOARD_32MHZ_XTAL_CDAC_VALUE)) && (defined(BOARD_32MHZ_XTAL_TRIM_DEFAULT))
/* BOARD_32MHZ_XTAL_TRIM_DEFAULT macro is deprecated
   please use BOARD_32MHZ_XTAL_CDAC_VALUE now */
#define BOARD_32MHZ_XTAL_CDAC_VALUE BOARD_32MHZ_XTAL_TRIM_DEFAULT
#endif

#if ((defined(gBoardUseFro32k_d) && defined(BOARD_32KHZ_SRC_CLK_ACCURACY)) && \
     ((gBoardUseFro32k_d == 1) && (BOARD_32KHZ_SRC_CLK_ACCURACY != 0)))
#error "The clock accuracy needs to be 500ppm if using FRO 32kHz"
#endif

#if ((defined(gBoardUseFro32k_d) && (gBoardUseFro32k_d == 1)) &&                                         \
     (((!defined(BOARD_LL_32MHz_WAKEUP_ADVANCE_HSLOT) || (BOARD_LL_32MHz_WAKEUP_ADVANCE_HSLOT <= 2))) || \
      (!defined(BOARD_RADIO_DOMAIN_WAKE_UP_DELAY) || (BOARD_RADIO_DOMAIN_WAKE_UP_DELAY > 0x0F))))

/* Needs to take higher wakeup time margin for NBU when using FRO 32KHz */

#ifndef BOARD_RADIO_DOMAIN_WAKE_UP_DELAY
#define BOARD_RADIO_DOMAIN_WAKE_UP_DELAY 0xf
#endif

#ifndef BOARD_LL_32MHz_WAKEUP_ADVANCE_HSLOT
#define BOARD_LL_32MHz_WAKEUP_ADVANCE_HSLOT 3
#endif

#endif

#if (defined(BOARD_DBG_SWO_CORE_FUNNEL) && ((BOARD_DBG_SWO_CORE_FUNNEL < 0) || (BOARD_DBG_SWO_CORE_FUNNEL > 2)))
#error "BOARD_SWO_CORE_FUNNEL cannot be different from 0, 1 or 2."
#endif

#if ((defined(BOARD_DBG_SWO_PIN_ENABLE) && (BOARD_DBG_SWO_PIN_ENABLE == 1)) && \
     (defined(BOARD_DBG_SWO_CORE_FUNNEL) && (BOARD_DBG_SWO_CORE_FUNNEL == 0)))
#error "BOARD_DBG_SWO_CORE_FUNNEL needs to be enabled when using BOARD_DBG_SWO_PIN_ENABLE."
#endif

#endif /* _BOARD_PLATFORM_H_ */
