/*
 * Copyright 2020-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * The function that processes the tensor output of model Nanodet-M
 */

#include <stdio.h>
#include <math.h>

#include "../utils.h"
#include "nanodet_m_output_postproc.h"
#include "../get_top_n.h"
#include "fsl_debug_console.h"

extern "C" {
#include "mpp_api.h"
}

#define DETECTION_TRESHOLD  30
#define NUM_RESULTS         1
#define EOL                 "\r\n"
#define NANODET_TENSOR_TYPE MPP_TENSOR_TYPE_FLOAT32
#define NANODET_WIDTH       320
#define NANODET_HEIGHT      320
#define MODEL_STRIDE        32
#define NANODET_NUM_CLASS   80
#define NANODET_NUM_REGS    32
#define REG_MAX             7
#define NMS_THRESH          0.4f
#define NUM_BOXES_MAX       MAX_POINTS
#define NUM_BOX_COORDS      4   /* top, left, bottom, right */

typedef struct {
    int x;
    int y;
    int stride;
} center_prior;

int8_t g_cls_int[NANODET_NUM_CLASS * NUM_BOXES_MAX * sizeof(int8_t)] = {0};
int8_t g_reg_int[(REG_MAX + 1) * NUM_BOX_COORDS * NUM_BOXES_MAX * sizeof(int8_t)] = {0};


static void generate_center_priors(const int input_height, const int input_width, int stride, center_prior *centers, int center_size){
    int num_points = 0;
    int feat_w = ceil((float)input_width / stride);
    int feat_h = ceil((float)input_height / stride);
    for (int y = 0; y < feat_h; y++)
    {
        for (int x = 0; x < feat_w; x++)
        {
            if (num_points >= center_size){
                printf("Invalid number of points\n");
                break;
            }
            centers[num_points].x = x;
            centers[num_points].y = y;
            centers[num_points].stride = stride;
            num_points = num_points + 1;
        }
    }
    return;
}

#ifdef NMS_USE_SOFTMAX
static void softmax_activations(const float *src_preds, float *dst_preds, int idx){
    float m = -INFINITY;
    for (int i = idx; i < idx + (REG_MAX + 1); i++) {
        if (src_preds[i] > m) {
            m = src_preds[i];
        }
    }

    float sum = 0.0;
    for (int i = idx; i < idx + (REG_MAX + 1); i++) {
        sum += expf(src_preds[i] - m);
    }

    float offset = m + logf(sum);
    for (int i = idx; i < idx + (REG_MAX + 1); i++) {
        dst_preds[i - idx] = expf(src_preds[i] - offset);
    }
}
#endif /* NMS_USE_SOFTMAX */

#ifdef NMS_USE_FLOAT
static void boxes_distribution_prediction(box_data *box, const float *reg_preds, const center_prior center){
    float ct_x = center.x * MODEL_STRIDE;
    float ct_y = center.y * MODEL_STRIDE;
    float dist_preds[NUM_BOX_COORDS];
    for (int i=0; i < NUM_BOX_COORDS; i++){
        float max = -1000.0f;
        float dist = 0.0f;
        int offset = i*(REG_MAX +1);
        for(int j = 0; j < REG_MAX + 1; j++){
            float curr_score = reg_preds[j+offset];
            if(curr_score > max ){
                max = curr_score;
                dist = (float)j + 0.5f;
            }
        }
        dist = dist * MODEL_STRIDE;
        dist_preds[i] = dist;
    }
    box->left = MAX((ct_x - dist_preds[0]), 0);
    box->top = MAX(ct_y - dist_preds[1], 0);
    box->right = MIN(ct_x + dist_preds[2], (float)NANODET_WIDTH);
    box->bottom = MIN(ct_y + dist_preds[3], (float)NANODET_HEIGHT);
}
#endif /* NMS_USE_FLOAT */

static void boxes_distribution_prediction_int8(box_data *box, const int8_t *reg_preds, const center_prior center){
    int ct_x = center.x * MODEL_STRIDE;
    int ct_y = center.y * MODEL_STRIDE;
    int dist_preds[NUM_BOX_COORDS];
    for (int i=0; i < NUM_BOX_COORDS; i++){
        int8_t max = -128;
        int dist = 0;
        int offset = i*(REG_MAX +1);
        for(int j = 0; j < REG_MAX + 1; j++){
        	int8_t curr_score = reg_preds[j+offset];
            if(curr_score > max ){
                max = curr_score;
                dist = j;
            }
        }
        dist = dist * MODEL_STRIDE;
        dist_preds[i] = dist;
    }
    box->left = MAX((ct_x - dist_preds[0]), 0);
    box->top = MAX(ct_y - dist_preds[1], 0);
    box->right = MIN(ct_x + dist_preds[2], (float)NANODET_WIDTH);
    box->bottom = MIN(ct_y + dist_preds[3], (float)NANODET_HEIGHT);
}

#ifdef NMS_USE_FLOAT
static void decode_output(const float *cls_predictions, const float *reg_predictions,
        const center_prior *centers, box_data boxes[])
{
    int cls_offset = 0;
    int reg_offset = 0;
    float value = 0.0f;
    int n_inserted = 0;
    box_data curr_box;

    /* initialize elements */
    for (int i = 0; i < NUM_BOXES_MAX; i++){
        float score = -1000.0;
        int curr_label = 0;
        const float threshold = (float)DETECTION_TRESHOLD / 100.0f;
        const float *reg_preds = reg_predictions + reg_offset;
        // Get top prediction score + index
        for (int j = cls_offset; j < NANODET_NUM_CLASS + cls_offset; j++){
            value = cls_predictions[j];
            if (value > score){
                score = value;
                curr_label = j - cls_offset;
            }
        }
        curr_box.label = curr_label;
        curr_box.score = score;
        if (score >= threshold){
            boxes_distribution_prediction(&curr_box, reg_preds, centers[i]);

            n_inserted = nms_insert_box(boxes, curr_box, n_inserted, NMS_THRESH, NUM_BOXES_MAX);
        }
        cls_offset = cls_offset + NANODET_NUM_CLASS;
        reg_offset = reg_offset + NANODET_NUM_REGS;
    }
}
#endif /* NMS_USE_FLOAT */

/* decode the output tensor and fill-in boxes above detection threshold.
 * returns the number of valid boxes.
 **/
static int decode_output_int8(const int8_t *cls_predictions, const int8_t *reg_predictions,
        const center_prior *centers, box_data boxes[])
{
    int cls_offset = 0;
    int reg_offset = 0;
    int8_t value = 0;
    int n_inserted = 0;
    box_data curr_box;

    /* initialize elements */
    for (int i = 0; i < NUM_BOXES_MAX; i++)
    {
    	int8_t score = -128;
        int curr_label = 0;
        const int threshold = DETECTION_TRESHOLD * 256 / 100 - 128;
        const int8_t *reg_preds = reg_predictions + reg_offset;
        // Get top prediction score + index
        for (int j = cls_offset; j < NANODET_NUM_CLASS + cls_offset; j++)
        {
            value = cls_predictions[j];
            if (value > score)
            {
                score = value;
                curr_label = j - cls_offset;
            }
        }

        if (score >= threshold)
        {
            curr_box.label = curr_label;
            curr_box.score = (score + 128.0f) / 256.0f;
            boxes_distribution_prediction_int8(&curr_box, reg_preds, centers[i]);

            n_inserted = nms_insert_box(boxes, curr_box, n_inserted, NMS_THRESH, NUM_BOXES_MAX);
        }
        cls_offset = cls_offset + NANODET_NUM_CLASS;
        reg_offset = reg_offset + NANODET_NUM_REGS;
    }
    return n_inserted;
}

// Used for models with float output tensors
static void convert_float_int(const float *cls_preds, const float *reg_preds,
        int8_t *cls_preds_int, int8_t *reg_preds_int)
{
	// class scores from GLOW are floats between 0 and 1 (sigmoid output).
	// We stretch these scores to the int8 range to match TFLite output.
	int n_cls_preds = NANODET_NUM_CLASS * NUM_BOXES_MAX;
	for (int i = 0; i < n_cls_preds; i++) {
		cls_preds_int[i] = (int8_t)(cls_preds[i]*256-128);
	}

	// box predictions do not have a predefined range (logits).
	// Observed values from nanodet_m range from -1.75 to 1.5.
	// Casting to int directly would destroy predictions.
	// Since actual values do not matter (we only want the max index when decoding),
	// we multiply scores by 20 to stretch them to a usable range when casting.
	int n_reg_preds = (REG_MAX + 1) * NUM_BOX_COORDS * NUM_BOXES_MAX;
	for (int i = 0; i < n_reg_preds; i++) {
		reg_preds_int[i] = (int8_t)(reg_preds[i]*20);
	}
}

int32_t NANODET_ProcessOutput(const mpp_inference_cb_param_t *inf_out, box_data* final_boxes)
{
    if (inf_out == NULL)
    {
        PRINTF("ERROR: NANODET_ProcessOutput parameter 'inf_out' is null pointer" EOL);
        return -1;
    }
    if (final_boxes == NULL)
    {
        PRINTF("ERROR: NANODET_ProcessOutput parameter 'final_boxes' is null pointer" EOL);
        return -1;
    }
    int8_t* cls_preds_int;
    int8_t *reg_preds_int;

    if(inf_out->inference_type == MPP_INFERENCE_TYPE_GLOW)
    {
        /* TODO remove this code as soon as Glow can provide int8 tensors */
        const float *cls_preds = (const float *)(inf_out->out_tensors[0]->data);
        float *reg_preds = (float *)(inf_out->out_tensors[1]->data);
        if (cls_preds == NULL)
        {
            PRINTF("ERROR: NANODET_ProcessOutput: cls_preds NULL pointer" EOL);
            return -1;
        }
        if (reg_preds == NULL)
        {
            PRINTF("ERROR: NANODET_ProcessOutput: reg_preds NULL pointer" EOL);
            return -1;
        }

        cls_preds_int = g_cls_int;
        reg_preds_int = g_reg_int;

        convert_float_int(cls_preds, reg_preds, cls_preds_int, reg_preds_int);
    }
    else if(inf_out->inference_type == MPP_INFERENCE_TYPE_TFLITE)
    {
        cls_preds_int = (int8_t *) inf_out->out_tensors[0]->data; /* [1, 100, 80] matrix */
        reg_preds_int = (int8_t *) inf_out->out_tensors[1]->data; /* [1, 100, 32] matrix */
        if (cls_preds_int == NULL)
        {
            PRINTF("ERROR: NANODET_ProcessOutput: cls_preds_int NULL pointer" EOL);
            return -1;
        }
        if (reg_preds_int == NULL)
        {
            PRINTF("ERROR: NANODET_ProcessOutput: reg_preds_int NULL pointer" EOL);
            return -1;
        }
    }
    else
    {
        PRINTF("ERROR: NANODET_ProcessOutput: Undefined Inference Engine" EOL);
        return -1;
    }
    center_prior centers[MAX_POINTS];

    generate_center_priors(NANODET_HEIGHT, NANODET_WIDTH, MODEL_STRIDE, centers, MAX_POINTS);

    memset(final_boxes, 0, NUM_BOXES_MAX*sizeof(box_data));

    decode_output_int8(cls_preds_int, reg_preds_int, (const center_prior *)centers, final_boxes);

    return 0;
}
