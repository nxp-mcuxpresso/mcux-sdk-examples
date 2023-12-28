/*
 * Copyright 2020-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _NANODET_M_OUTPUT_POSTPROCESS_H_
#define _NANODET_M_OUTPUT_POSTPROCESS_H_

#include "mpp_api_types.h"
#include "../utils.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MAX_POINTS          100 //maximum number of center priors

/**
 * Process the Nanodet output tensors
 *
 * @param [in] inf_out: inference output (tensor data and descritption)
 * @param [out] final_boxes: array of pointers to object bounding boxes
 *
 * @return: 0 if succeeded, else failed.
 */
int32_t NANODET_ProcessOutput(const mpp_inference_cb_param_t *inf_out, box_data* final_boxes);


#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _NANODET_M_OUTPUT_POSTPROCESS_H_ */
