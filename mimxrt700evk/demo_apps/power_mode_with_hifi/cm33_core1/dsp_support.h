/*
 * Copyright 2024 NXP
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
#define DSP_IMAGE_LITERAL_START (uint32_t *)__section_begin("__dsp_literal_section")
#define DSP_IMAGE_LITERAL_SIZE \
    (int32_t) __section_end("__dsp_literal_section") - (uint32_t)__section_begin("__dsp_literal_section");
#define DSP_IMAGE_VECTOR_START (uint32_t *)__section_begin("__dsp_vector_section")
#define DSP_IMAGE_VECTOR_SIZE \
    (int32_t) __section_end("__dsp_vector_section") - (uint32_t)__section_begin("__dsp_vector_section");
#define DSP_IMAGE_TEXT_START (uint32_t *)__section_begin("__dsp_text_section")
#define DSP_IMAGE_TEXT_SIZE \
    (int32_t) __section_end("__dsp_text_section") - (uint32_t)__section_begin("__dsp_text_section");
#define DSP_IMAGE_DATA_START (uint32_t *)__section_begin("__dsp_data_section")
#define DSP_IMAGE_DATA_SIZE \
    (int32_t) __section_end("__dsp_data_section") - (uint32_t)__section_begin("__dsp_data_section");
#ifdef DSP_NCACHE
#define DSP_IMAGE_NCACHE_START (uint32_t *)__section_begin("__dsp_ncache_section")
#define DSP_IMAGE_NCACHE_SIZE \
    (int32_t) __section_end("__dsp_ncache_section") - (uint32_t)__section_begin("__dsp_ncache_section");
#endif
#elif (defined(__ARMCC_VERSION))
extern const char dsp_literal_image_start[];
extern const char dsp_literal_image_end[];
extern const char dsp_vector_image_start[];
extern const char dsp_vector_image_end[];
extern const char dsp_text_image_start[];
extern const char dsp_text_image_end[];
extern const char dsp_data_image_start[];
extern const char dsp_data_image_end[];
#ifdef DSP_NCACHE
extern const char dsp_ncache_image_start[];
extern const char dsp_ncache_image_end[];
#endif
#define DSP_IMAGE_LITERAL_START ((uint32_t *)dsp_literal_image_start)
#define DSP_IMAGE_LITERAL_SIZE  ((uint32_t)dsp_literal_image_end - (uint32_t)dsp_literal_image_start)
#define DSP_IMAGE_VECTOR_START  ((uint32_t *)dsp_vector_image_start)
#define DSP_IMAGE_VECTOR_SIZE   ((uint32_t)dsp_vector_image_end - (uint32_t)dsp_vector_image_start)
#define DSP_IMAGE_TEXT_START    ((uint32_t *)dsp_text_image_start)
#define DSP_IMAGE_TEXT_SIZE     ((uint32_t)dsp_text_image_end - (uint32_t)dsp_text_image_start)
#define DSP_IMAGE_DATA_START    ((uint32_t *)dsp_data_image_start)
#define DSP_IMAGE_DATA_SIZE     ((uint32_t)dsp_data_image_end - (uint32_t)dsp_data_image_start)
#ifdef DSP_NCACHE
#define DSP_IMAGE_NCACHE_START ((uint32_t *)dsp_ncache_image_start)
#define DSP_IMAGE_NCACHE_SIZE  ((uint32_t)dsp_ncache_image_end - (uint32_t)dsp_ncache_image_start)
#endif
#elif defined(__GNUC__)
extern const char dsp_literal_image_start[];
extern int dsp_literal_image_size;
extern const char dsp_vector_image_start[];
extern int dsp_vector_image_size;
extern const char dsp_text_image_start[];
extern int dsp_text_image_size;
extern const char dsp_data_image_start[];
extern int dsp_data_image_size;
#ifdef DSP_NCACHE
extern const char dsp_ncache_image_start[];
extern int dsp_ncache_image_size;
#endif
#define DSP_IMAGE_LITERAL_START ((uint32_t *)dsp_literal_image_start)
#define DSP_IMAGE_LITERAL_SIZE  ((int32_t)dsp_literal_image_size)
#define DSP_IMAGE_VECTOR_START  ((uint32_t *)dsp_vector_image_start)
#define DSP_IMAGE_VECTOR_SIZE   ((int32_t)dsp_vector_image_size)
#define DSP_IMAGE_TEXT_START    ((uint32_t *)dsp_text_image_start)
#define DSP_IMAGE_TEXT_SIZE     ((int32_t)dsp_text_image_size)
#define DSP_IMAGE_DATA_START    ((uint32_t *)dsp_data_image_start)
#define DSP_IMAGE_DATA_SIZE     ((int32_t)dsp_data_image_size)
#ifdef DSP_NCACHE
#define DSP_IMAGE_NCACHE_START ((uint32_t *)dsp_ncache_image_start)
#define DSP_IMAGE_NCACHE_SIZE  ((int32_t)dsp_ncache_image_size)
#endif
#endif
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

/*!
 * @brief   Initialize and boot the DSP/HiFi core.
 * @param   src : Clock source for the HiFi core.
 *                For HiFi4, 0-baseclk_dsp, 1-main_pll_pfd0, 2-fro0_max, 3-main_pll_pfd1.
 *                For HiFi1, 0-baseclk_sense, 1-fro2_max, 2-aduio_pll_pdf1, 3-fro1_max.
 * @param   div : Clock divider for DSP clock.
 * @param   copyImage : Do image copy or not.
 */
void BOARD_DSP_Init(uint32_t src, uint32_t div, bool copyImage);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _DSP_SUPPORT_H_ */
