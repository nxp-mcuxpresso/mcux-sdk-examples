/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _DSP_SUPPORT_H_
#define _DSP_SUPPORT_H_

#include "dsp_config.h"

/*******************************************************************************
 * DEFINITION
 ******************************************************************************/

#if defined(__CC_ARM)
extern uint32_t Image$$DSP_REGION$$Base;
extern uint32_t Image$$DSP_REGION$$Length;
#define DSP_IMAGE_START &Image$$DSP_REGION$$Base
#elif defined(__ICCARM__)
#define DSP_IMAGE_RESET_START (uint32_t *)__section_begin("__dsp_reset_section")
#define DSP_IMAGE_RESET_SIZE \
    (int32_t) __section_end("__dsp_reset_section") - (uint32_t)__section_begin("__dsp_reset_section");
#define DSP_IMAGE_TEXT_START (uint32_t *)__section_begin("__dsp_text_section")
#define DSP_IMAGE_TEXT_SIZE \
    (int32_t) __section_end("__dsp_text_section") - (uint32_t)__section_begin("__dsp_text_section");
#define DSP_IMAGE_DATA_START (uint32_t *)__section_begin("__dsp_data_section")
#define DSP_IMAGE_DATA_SIZE \
    (int32_t) __section_end("__dsp_data_section") - (uint32_t)__section_begin("__dsp_data_section");
#elif (defined(__ARMCC_VERSION))
extern const char dsp_reset_image_start[];
extern const char dsp_reset_image_end[];
extern const char dsp_text_image_start[];
extern const char dsp_text_image_end[];
extern const char dsp_data_image_start[];
extern const char dsp_data_image_end[];
#define DSP_IMAGE_RESET_START ((uint32_t *)dsp_reset_image_start)
#define DSP_IMAGE_RESET_SIZE  ((uint32_t)dsp_reset_image_end - (uint32_t)dsp_reset_image_start)
#define DSP_IMAGE_TEXT_START  ((uint32_t *)dsp_text_image_start)
#define DSP_IMAGE_TEXT_SIZE   ((uint32_t)dsp_text_image_end - (uint32_t)dsp_text_image_start)
#define DSP_IMAGE_DATA_START  ((uint32_t *)dsp_data_image_start)
#define DSP_IMAGE_DATA_SIZE   ((uint32_t)dsp_data_image_end - (uint32_t)dsp_data_image_start)
#elif defined(__GNUC__)
extern const char dsp_reset_image_start[];
extern int dsp_reset_image_size;
extern const char dsp_text_image_start[];
extern int dsp_text_image_size;
extern const char dsp_data_image_start[];
extern int dsp_data_image_size;
#define DSP_IMAGE_RESET_START ((uint32_t *)dsp_reset_image_start)
#define DSP_IMAGE_RESET_SIZE  ((int32_t)dsp_reset_image_size)
#define DSP_IMAGE_TEXT_START  ((uint32_t *)dsp_text_image_start)
#define DSP_IMAGE_TEXT_SIZE   ((int32_t)dsp_text_image_size)
#define DSP_IMAGE_DATA_START  ((uint32_t *)dsp_data_image_start)
#define DSP_IMAGE_DATA_SIZE   ((int32_t)dsp_data_image_size)
#endif
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

void BOARD_DSP_Init(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _DSP_SUPPORT_H_ */
