/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef POWER_MODE_CONFIG_H_
#define POWER_MODE_CONFIG_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Define the power supply mode on the board.
 *  - DEMO_POWER_SUPPLY_MIXED, the VDDN is supplied by external PMIC. VDD1 and VDD2 are supplied by internal LDOs.
 *  - DEMO_POWER_SUPPLY_PMIC, the VDDN, VDD1 and VDD2 are supplied by PMIC.
 */
#define DEMO_POWER_SUPPLY_OPTION DEMO_POWER_SUPPLY_MIXED

#define DEMO_POWER_SUPPLY_PMIC  2U
#define DEMO_POWER_SUPPLY_MIXED 3U

/* Channel transmit and receive register */
#define APP_MU_REG kMU_MsgReg0
#define BOOT_FLAG  0x1U            /* Flag indicates Core1 Boot Up*/

#define DEMO_POWER_USE_PLL      0U /* Enable MAIN PLL and Audio PLL or not. */
#define DEMO_POWER_ENABLE_DEBUG 1U /* Enable debug or not, disable the debug function/clock can save power. */
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__cplusplus)
}
#endif

#endif /* POWER_MODE_CONFIG_H_ */

/*******************************************************************************
 * EOF
 ******************************************************************************/
