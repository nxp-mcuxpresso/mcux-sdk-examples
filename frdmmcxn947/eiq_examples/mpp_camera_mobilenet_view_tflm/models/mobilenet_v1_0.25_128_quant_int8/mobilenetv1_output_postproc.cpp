/*
 * Copyright 2020-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * The function that processes the tensor output of model MobileNet_v1
 */

#include <stdio.h>

#include "mobilenetv1_output_postproc.h"
#include "../get_top_n.h"
#include "mobilenetv1_labels.h"
#include "fsl_debug_console.h"

extern "C" {
#include "mpp_api.h"
}

#define DETECTION_TRESHOLD 23
#define NUM_RESULTS        1
#define EOL "\r\n"
#define MOBILENET_TENSOR_SIZE 1001
#define MOBILENET_TENSOR_TYPE MPP_TENSOR_TYPE_FLOAT32

int32_t MOBILENETv1_ProcessOutput(const mpp_inference_cb_param_t *inf_out, void *mpp,
        mpp_elem_handle_t elem, mpp_labeled_rect_t *rects, mobilenet_post_proc_data_t* out_data)
{
    const float threshold = (float)DETECTION_TRESHOLD / 100;
    result_t topResults[NUM_RESULTS];
    const char* label = "No label detected";
    uint32_t tensor_size;
    mpp_tensor_type_t tensor_type;
    /* Some inference type does not provide model output size and type.
     * For instance, for glow, these informations can be found in models/mobilenet_v1_glow.h */
    if (inf_out->out_tensors[0]->dims.size == 0){
        tensor_size = MOBILENET_TENSOR_SIZE;
        tensor_type = MOBILENET_TENSOR_TYPE;
    }
    else{
        tensor_size = inf_out->out_tensors[0]->dims.data[inf_out->out_tensors[0]->dims.size - 1];
        tensor_type = inf_out->out_tensors[0]->type;
    }

    /* Find best label candidates. */
    MODEL_GetTopN(inf_out->out_tensors[0]->data, tensor_size, tensor_type, NUM_RESULTS, threshold, topResults);

    float confidence = 0;
    if (topResults[0].index >= 0)
    {
        auto result = topResults[0];
        confidence = result.score;
        int index = result.index;
        if (confidence * 100 > DETECTION_TRESHOLD)
        {
            label = labels[index];
        }
    }

    int score = (int)(confidence * 100);
    if (out_data)
    {
        out_data->score = score;
        out_data->label = label;
    }
    if ( (mpp != NULL) && (elem != 0) && (rects != NULL) )
    {
        mpp_element_params_t params;
        uint8_t label_size = sizeof(params.labels.rectangles[0].label);
        /* update the label in first rectangle */
        params.labels.detected_count = 1;
        params.labels.max_count = 1;
        params.labels.rectangles = rects;
        strncpy((char *) params.labels.rectangles[0].label, label, label_size);
        params.labels.rectangles[0].label[label_size-1] = '\0';
        mpp_element_update(mpp, elem, &params);
    }

    return 0;
}
