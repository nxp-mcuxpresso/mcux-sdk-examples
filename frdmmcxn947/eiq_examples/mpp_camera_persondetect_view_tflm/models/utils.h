/*
 * Copyright 2022-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _UTILS_H_
#define _UTILS_H_

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

#include <stdint.h>

/* structure for bounding boxes */
typedef struct {
    int16_t left;
    int16_t top;
    int16_t right;
    int16_t bottom;
    int16_t label;
    int32_t area;
    float score;
} box_data;

/* computes IoU of box 1 and 2.
 * box->area must be pre-computed
 */
float iou(box_data* box1, box_data* box2);

/* Performs non-maximum suppression on an array of boxes.
 * box->area will be computed.
 * Process the result of NMS in-place.
 * Boxes where score < score_thr will be zeroed.
 * Boxes removed by NMS will be zeroed.
 */
void nms(box_data boxes[], int32_t num_boxes, float nms_thr, float score_thr);

/* Inserts a box into an array of boxes sorted by score
 * and performs non-maximum suppression in place on the fly.
 * This function does NOT filter bounding boxes with low scores.
 * Returns the number of boxes inserted (and kept) in the array.
 */
int32_t nms_insert_box(box_data boxes[], box_data curr_box, int32_t n_inserted, float nms_thr, int32_t max_boxes);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _UTILS_H_ */
