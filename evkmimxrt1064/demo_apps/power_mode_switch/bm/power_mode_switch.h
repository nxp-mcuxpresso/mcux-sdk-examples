/*
 * Copyright 2018-2021 NXP
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
typedef enum _app_wakeup_source
{
    kAPP_WakeupSourceTimer, /*!< Wakeup by Timer.        */
    kAPP_WakeupSourcePin,   /*!< Wakeup by external pin. */
} app_wakeup_source_t;

#endif /* _POWER_MODE_SWITCH_H_ */
