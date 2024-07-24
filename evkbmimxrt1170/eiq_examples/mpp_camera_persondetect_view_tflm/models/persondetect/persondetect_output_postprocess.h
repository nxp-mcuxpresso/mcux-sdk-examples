/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PERSON_DETECT_H
#define PERSON_DETECT_H

#include "mpp_api_types.h"
#include "../utils.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

/**
 * @brief Process the output tensors from the person detection model.
 *
 * This function takes the output tensors from the person detection model, performs post-processing,
 * and populates the final bounding boxes for detected persons.
 *
 * @param[in] inf_out Pointer to the inference output containing tensor data and description.
 * @param[out] final_boxes Pointer to an array of bounding boxes for detected persons.
 *
 * @return 0 if the post-processing succeeds, -1 if there are errors or null pointers in the input parameters.
 */
int32_t Persondetect_Output_postprocessing(
		const mpp_inference_cb_param_t *inf_out,
		box_data* final_boxes);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif // PERSON_DETECT_H
