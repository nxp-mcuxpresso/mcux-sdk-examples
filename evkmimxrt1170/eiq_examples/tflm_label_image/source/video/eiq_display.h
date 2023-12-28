/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EIQ_DISPLAY_H_
#define _EIQ_DISPLAY_H_

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

#include "eiq_iui.h"
#include "eiq_display_conf.h"

/*!
 * @addtogroup eiq_display
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Display structure */
typedef struct
{
    EIQ_IUi_t base;
    uint32_t (*getEmptyBuffer)(void);
    void (*setReadyCallback)(EIQ_IUpdater_t updater);
} EIQ_Display_t;

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @brief Initializes display.
 *
 * @return Pointer to initialized display instance.
 */
EIQ_Display_t* EIQ_InitDisplay(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _IMAGE_H_ */
