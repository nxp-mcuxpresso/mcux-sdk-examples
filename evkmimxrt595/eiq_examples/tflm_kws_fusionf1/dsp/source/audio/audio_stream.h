/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AUDIO_IO_H_
#define _AUDIO_IO_H_

#include <stdint.h>

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Initialize audio input/output hardware.
 */
void AUDIO_Init(void);

/*!
 * @brief Get next recorded audio frame buffer.
 *
 * @return Pointer to the next recorded audio frame buffer.
 */
bool AUDIO_GetNextFrame(int16_t** buffer);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _AUDIO_IO_H_ */
