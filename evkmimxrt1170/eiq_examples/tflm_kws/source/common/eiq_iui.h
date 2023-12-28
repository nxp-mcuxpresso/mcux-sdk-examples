/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EIQ_IUI_H_
#define _EIQ_IUI_H_

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

#include "eiq_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
  typedef void (*EIQ_IBufferAddrUpdater_t)(uint32_t bufferAddr);
  typedef void (*EIQ_IUpdater_t)(void);

/*! @brief Image structure */
typedef struct
{
  void (*start)(void);
  Dims_t (*getResolution)(void);
  void (*notify)(void);
} EIQ_IUi_t;

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _IMAGE_H_ */
