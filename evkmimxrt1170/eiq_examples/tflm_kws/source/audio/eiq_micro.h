/*
 * Copyright 2018-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EIQ_MICRO_H_
#define _EIQ_MICRO_H_

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

#include <stdbool.h>

#include "eiq_iui.h"
#include "eiq_micro_conf.h"

/*!
 * @addtogroup eiq_micro
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Microphone structure */
typedef struct
{
    EIQ_IUi_t base;
    void (*setReadyCallback)(EIQ_IBufferAddrUpdater_t updater);
    uint8_t* (*getReadyBuff)(void);
    bool (*isReady)(void);
} EIQ_Micro_t;

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @brief Initializes microphone.
 *
 * This function initializes microphone.
 *
 * @return pointer to initialized microphone instance
 */
EIQ_Micro_t* EIQ_MicroInit(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _EIQ_MICRO_H_ */
