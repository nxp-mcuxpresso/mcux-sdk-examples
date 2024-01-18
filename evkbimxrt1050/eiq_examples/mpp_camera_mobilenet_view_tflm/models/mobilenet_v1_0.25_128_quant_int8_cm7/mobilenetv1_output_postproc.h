/*
 * Copyright 2020-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _MOBILENETV1_OUTPUT_POSTPROCESS_H_
#define _MOBILENETV1_OUTPUT_POSTPROCESS_H_

#include "stdio.h"

#include "mpp_api_types.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* structure for mobilenet v1 post-processed outputs */
typedef struct {
    const char* label; /* class of detected object */
    int score;        /* score of detected object */
} mobilenet_post_proc_data_t;

/**
 * Process the output tensors
 *
 * @param [in] inf_out: inference output (tensor data and descritption)
 * @param [in] mpp: the mpp of element to update
 * @param [in] elem: the element to update
 * @param [in] rects: the rectangles to update
 * @param [out] out_data: structure containing the final label/score to be returned
 *
 * @return: 0 if succeeded, else failed.
 */
int32_t MOBILENETv1_ProcessOutput(const mpp_inference_cb_param_t *inf_out, void *mpp,
        mpp_elem_handle_t elem, mpp_labeled_rect_t *rects, mobilenet_post_proc_data_t* out_data);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _MOBILENETV1_OUTPUT_POSTPROCESS_H_ */
