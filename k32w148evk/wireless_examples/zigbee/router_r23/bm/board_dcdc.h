/* -------------------------------------------------------------------------- */
/*                           Copyright 2021-2022 NXP                          */
/*                            All rights reserved.                            */
/*                    SPDX-License-Identifier: BSD-3-Clause                   */
/* -------------------------------------------------------------------------- */

#ifndef _BOARD_DCDC_H_
#define _BOARD_DCDC_H_

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */
#include <stdint.h>

/* -------------------------------------------------------------------------- */
/*                                Public macros                               */
/* -------------------------------------------------------------------------- */

#if !defined(gBoardDcdcBuckMode_d)
/*!
 * \brief Defines if the DCDC is used (buck mode) or not (bypass mode)
 *
 */
#define gBoardDcdcBuckMode_d 1
#endif

/* -------------------------------------------------------------------------- */
/*                              Public prototypes                             */
/* -------------------------------------------------------------------------- */

/*!
 * \brief Initializes DCDC for buck or bypass mode (defined by
 *        gBoardDcdcBuckMode_d) and applies optimized configuration based on
 *        the TX power needed (defined by gAppMaxTxPowerDbm_c)
 *        If gAppMaxTxPowerDbm_c is not defined, max power is assumed (10dBm)
 *
 */
void BOARD_InitDcdc(void);

#endif /* _BOARD_DCDC_H_ */
