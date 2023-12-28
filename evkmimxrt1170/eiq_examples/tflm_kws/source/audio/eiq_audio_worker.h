/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EIQ_VIDEO_WORKER_H_
#define _EIQ_VIDEO_WORKER_H_

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

#include "eiq_iworker.h"
#include "eiq_micro.h"
#include "eiq_speaker.h"

/*!
 * @addtogroup audio_worker
 * @{
 */


/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Audio structure */
typedef struct
{
    EIQ_IWorker_t base;
    EIQ_Micro_t *receiver;
    EIQ_Speaker_t *sender;
} EIQ_AudioWorker_t;

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @brief Initializes the AudioWorker.
 *
 * This function initializes microphone and speaker.
 *
 * @return pointer to initialized AudioWorker
 */
EIQ_AudioWorker_t* EIQ_AudioWorkerInit(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _IMAGE_H_ */
