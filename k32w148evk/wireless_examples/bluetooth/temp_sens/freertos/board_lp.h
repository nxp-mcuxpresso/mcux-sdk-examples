/* -------------------------------------------------------------------------- */
/*                           Copyright 2020-2022 NXP                          */
/*                            All rights reserved.                            */
/*                    SPDX-License-Identifier: BSD-3-Clause                   */
/* -------------------------------------------------------------------------- */

#ifndef _BOARD_LP_H_
#define _BOARD_LP_H_

/*!
 * @addtogroup FWK_Board_module
 * @{
 */
/*!
 * @addtogroup FWK_Board_LP
 * The FWK_Board_LP module
 *
 * FWK_Board_LP module provides APIs to manage low power state.
 * @{
 */

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */

#include "fsl_pm_device.h" // only required for IO wakeup source

/* -------------------------------------------------------------------------- */
/*                                Public macros                               */
/* -------------------------------------------------------------------------- */
#if ((!defined(BOARD_LOCALIZATION_REVISION_SUPPORT)) || BOARD_LOCALIZATION_REVISION_SUPPORT == 0) && \
    ((!defined(BOARD_FRDMCXW71_SUPPORT)) || BOARD_FRDMCXW71_SUPPORT == 0)
/*!
 * \brief Defines the wake up source for the button0
 *        Can be an external pin or an internal module mapped to WUU
 *        Based on supported wake up sources from SDK power manager (see fsl_pm_device.h)
 *        Set by default to GPIOD interrupt as SW2 button is mapped to PTD1 on EVK boards
 *
 */
#define BOARD_WAKEUP_SOURCE_BUTTON0 (uint32_t) PM_WSID_GPIOD_LOW

/*!
 * \brief Defines the wake up source for the button1
 *        Can be an external pin or an internal module mapped to WUU
 *        Based on supported wake up sources from SDK power manager (see fsl_pm_device.h)
 *        Set by default to PTC6 interrupt as SW3 button is mapped to PTC6 on EVK boards
 *
 */
#define BOARD_WAKEUP_SOURCE_BUTTON1 (uint32_t) PM_WSID_PTC6_FALLING_EDGE

#else

#if ((defined(BOARD_FRDMCXW71_SUPPORT)) && BOARD_FRDMCXW71_SUPPORT)
/*!
 * \brief Defines the wake up source for the button1
 *        Can be an external pin or an internal module mapped to WUU
 *        Based on supported wake up sources from SDK power manager (see fsl_pm_device.h)
 *        Set by default to PTC6 interrupt as SW3 button is mapped to PTC6 on EVK boards
 *
 */
#define BOARD_WAKEUP_SOURCE_BUTTON0 (uint32_t) PM_WSID_PTC6_FALLING_EDGE

/*!
 * \brief Defines the wake up source for the button0
 *        Can be an external pin or an internal module mapped to WUU
 *        Based on supported wake up sources from SDK power manager (see fsl_pm_device.h)
 *        Set by default to GPIOD interrupt as SW2 button is mapped to PTD1 on EVK boards
 *
 */
#define BOARD_WAKEUP_SOURCE_BUTTON1 (uint32_t) PM_WSID_GPIOD_LOW

#endif

#if ((defined(BOARD_LOCALIZATION_REVISION_SUPPORT)) && BOARD_LOCALIZATION_REVISION_SUPPORT)
/*!
 * \brief Defines the wake up source for the button0
 *        Can be an external pin or an internal module mapped to WUU
 *        Based on supported wake up sources from SDK power manager (see fsl_pm_device.h)
 *        Set by default to GPIOD interrupt as SW2 button is mapped to PTD1 on EVK boards
 *
 */
#define BOARD_WAKEUP_SOURCE_BUTTON0 (uint32_t) PM_WSID_PTC1_FALLING_EDGE

/*!
 * \brief Defines the wake up source for the button1
 *        Can be an external pin or an internal module mapped to WUU
 *        Based on supported wake up sources from SDK power manager (see fsl_pm_device.h)
 *        Set by default to PTC6 interrupt as SW3 button is mapped to PTC6 on EVK boards
 *
 */
#define BOARD_WAKEUP_SOURCE_BUTTON1 (uint32_t) PM_WSID_PTC7_FALLING_EDGE
#endif
#endif
/* -------------------------------------------------------------------------- */
/*                              Public prototypes                             */
/* -------------------------------------------------------------------------- */

/*!
 * \brief Initialize board ressources related to low power, likely to register
 *        low power entry/exit callbacks for optional peripherals
 *
 */
void BOARD_LowPowerInit(void);

/*!
 * @}  end of FWK_Board_LP addtogroup
 */
/*!
 * @}  end of FWK_Board_module addtogroup
 */

#endif /* _BOARD_LP_H_ */
