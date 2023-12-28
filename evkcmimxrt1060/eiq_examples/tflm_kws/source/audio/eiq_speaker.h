/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EIQ_SPEAKER_H_
#define _EIQ_SPEAKER_H_

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

#include "eiq_iui.h"
#include "eiq_speaker_conf.h"
#include "eiq_micro_conf.h"

/*!
 * @addtogroup eiq_speaker
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Audio structure */
typedef struct
{
    EIQ_IUi_t base;
    void (*setReadyCallback)(EIQ_IUpdater_t updater);
    void (*setBuffer)(uint32_t buffAddr);
} EIQ_Speaker_t;

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @brief Initializes speaker.
 *
 * This function initializes speaker.
 *
 * @return pointer to initialized speaker instance
 */
EIQ_Speaker_t* EIQ_SpeakerInit(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _EIQ_SPEAKER_H_ */
