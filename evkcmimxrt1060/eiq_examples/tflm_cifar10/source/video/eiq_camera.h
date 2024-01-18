/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EIQ_CAMERA_H_
#define _EIQ_CAMERA_H_

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

#include "eiq_iui.h"
#include "eiq_camera_conf.h"
#include "eiq_display_conf.h"
#include "stdbool.h"

/*!
 * @addtogroup eiq_camera
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Camera structure. */
typedef struct
{
    EIQ_IUi_t base;
    void (*setReadyCallback)(EIQ_IBufferAddrUpdater_t updater);
} EIQ_Camera_t;

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @brief Initializes camera.
 *
 * This function initializes camera.
 *
 * @return pointer to initialized camera instance.
 */
EIQ_Camera_t* EIQ_InitCamera(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _EIQ_CAMERA_H_ */
