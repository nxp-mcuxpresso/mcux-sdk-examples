/*
 * Copyright 2019, 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _POWER_MODE_SWITCH_
#define _POWER_MODE_SWITCH_

#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef enum _app_power_mode
{
    kAPP_PowerModeMin = 'A' - 1,
    kAPP_PowerModeActive,
    kAPP_PowerModeSleep1,
    kAPP_PowerModeDeepSleep1,
    kAPP_PowerModeDeepSleep2,
    kAPP_PowerModeDeepSleep3,
    kAPP_PowerModeDeepSleep4,
    kAPP_PowerModePowerDown1,
    kAPP_PowerModePowerDown2,
    kAPP_PowerModePowerDown3,
    kAPP_PowerModePowerDown4,
    kAPP_PowerModeDeepPowerDown1,
    kAPP_PowerModeDeepPowerDown2,
    kAPP_PowerSwitchOff,
    kAPP_PowerModeMax
} app_power_mode_t;

typedef enum _app_wakeup_source
{
    kAPP_WakeupSourceLptmr = 'A',  /*!< Wakeup by LPTMR.        */
    kAPP_WakeupSourceWakeupButton, /*!< Wakeup from button. */
    kAPP_WakeupSourceVbat,         /*!< Wakeup from VBAT. */
} app_wakeup_source_t;

/*! @name Always On Region */
#if (defined(__ICCARM__))
#define AT_ALWAYS_ON_DATA(var)      var @"AlwaysOnData"
#define AT_ALWAYS_ON_DATA_INIT(var) var @"AlwaysOnData.init"
#elif (defined(__CC_ARM) || defined(__ARMCC_VERSION))
#define AT_ALWAYS_ON_DATA(var)      __attribute__((section("AlwaysOnData"), zero_init)) var
#define AT_ALWAYS_ON_DATA_INIT(var) __attribute__((section("AlwaysOnData.init"))) var
#elif (defined(__GNUC__))
#define AT_ALWAYS_ON_DATA(var)      __attribute__((section("AlwaysOnData,\"aw\",%nobits @"))) var
#define AT_ALWAYS_ON_DATA_INIT(var) __attribute__((section("AlwaysOnData.init"))) var
#else
#error Toolchain not supported.
#endif /* defined(__ICCARM__) */
/*! @} */

#endif /*_POWER_MODE_SWITCH_*/
