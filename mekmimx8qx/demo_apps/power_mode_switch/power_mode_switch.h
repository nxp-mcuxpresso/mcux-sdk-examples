/*
 * Copyright 2017-2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _POWER_MODE_SWITCH_H_
#define _POWER_MODE_SWITCH_H_

#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Power mode definition used in application. */
typedef enum _app_power_mode
{
    kAPP_PowerModeMin = 'A' - 1,
    kAPP_PowerModeRun,  /* Normal RUN mode */
    kAPP_PowerModeWait, /* WAIT mode. */
    kAPP_PowerModeStop, /* STOP mode. */
    kAPP_PowerModeVlpr, /* VLPR mode. */
    kAPP_PowerModeVlpw, /* VLPW mode. */
    kAPP_PowerModeVlps, /* VLPS mode. */
    kAPP_PowerModeLls,  /* LLS mode. */
    kAPP_PowerModeVlls, /* VLLS mode. */
    kAPP_PowerModeMax
} app_power_mode_t;

typedef enum _app_wakeup_source
{
    kAPP_WakeupSourceLpit, /*!< Wakeup by Lpit. */
    kAPP_WakeupSourcePin,  /*!< Wakeup by external pin. */
    kAPP_WakeupSourceCan,  /*!< Wakeup by CAN Bus. */
    kAPP_WakeupSourcePad,  /*!< Wakeup by Pad. */
    kAPP_WakeupSourceNone  /*!< Uninitialized State*/
} app_wakeup_source_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
/*${prototype:end}*/
#endif /* _POWER_MODE_SWITCH_H_ */
