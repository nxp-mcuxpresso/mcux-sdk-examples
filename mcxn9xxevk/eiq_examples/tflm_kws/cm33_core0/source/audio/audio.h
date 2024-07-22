/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <stdint.h>

#include "fsl_common.h"

/*!
 * @addtogroup audio
 * @{
 */

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Processes the static recording and microphone data sources and facilitates
 * the storage of the relevant quantized MFCC data into the output buffer in each call.
 *
 * @param dstData output buffer pointer
 * @param size size of the output buffer in bytes
 * @retval kStatus_Success output buffer has been successfully filled
 */
status_t AUDIO_GetSpectralSample(uint8_t* dstData, size_t size);

/*!
 * @brief Iterates over the audio buffer divided into smaller blocks and extracts features from those blocks.
 *
 * @param srcData address of source buffer for reading audio data
 * @param audioBlocksPerBuffer audio buffer size divided by the size of the audio block
 */
void AUDIO_PreprocessSample(const int16_t* srcData, size_t audioBlocksPerBuffer);

/*!
 * @brief Gets audio sample name.
 *
 * @return sample name
 */
const char* AUDIO_GetSampleName(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _AUDIO_H_ */
