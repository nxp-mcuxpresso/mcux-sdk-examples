/*
 * Copyright 2018-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EIQ_IWORKER_H_
#define _EIQ_IWORKER_H_

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

#include "eiq_common.h"
#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

struct EIQ_IWorker_t;

typedef void (*EIQ_IWorkerUpdater_t)(struct EIQ_IWorker_t* worker);

/*! @brief Image structure */
typedef struct EIQ_IWorker_t
{
  void (*start)(void);
  void (*stop)(void);
  bool (*isReady)(void);
  void (*notify)(void);
  Dims_t (*getResolution)(void);
  uint8_t* (*getData)(void);
  void (*refresh)(uint32_t bufferAddr);
  void (*setReadyCallback)(EIQ_IWorkerUpdater_t iworker);
} EIQ_IWorker_t;

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _EIQ_IWORKER_H_ */
