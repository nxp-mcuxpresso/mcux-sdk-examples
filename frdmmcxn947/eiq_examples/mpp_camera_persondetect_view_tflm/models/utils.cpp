/*
 * Copyright 2022-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Utility functions for output tensor post-processing
 */


#include <stdio.h>
#include "utils.h"
#include "fsl_common.h"

#define EOL "\r\n"

static int32_t area(box_data *box) {
    return (box->right - box->left +1) * (box->bottom - box->top +1);
}

float iou(box_data* box1, box_data* box2){
    assert(box1 != NULL);
    assert(box2 != NULL);

    int16_t x_1 = MAX(box1->left, box2->left);
    int16_t x_2 = MIN(box1->right, box2->right);
    int16_t y_1 = MAX(box1->top, box2->top);
    int16_t y_2 = MIN(box1->bottom, box2->bottom);
    int16_t w = MAX(0, x_2 - x_1 + 1);
    int16_t h = MAX(0, y_2 - y_1 + 1);

    float area_of_intersection = (float)w * (float)h;
    float area_of_union = (float)box1->area + (float)box2->area - area_of_intersection;

    return area_of_intersection / area_of_union;
}

static void swap_boxes(box_data boxes[], int32_t i, int32_t j) {
    box_data tmp = boxes[i];
    boxes[i] = boxes[j];
    boxes[j] = tmp;
}

/* Inserts boxes in box_pointers then sorts the array by score.
 * Boxes with low scores are filtered out (zeroed).
 * Also pre-computes the area of remaining boxes in box->area.
 * Returns the number of boxes that have a score above score_thr.
 */
static int32_t sort_boxes(box_data boxes[], int32_t num_boxes, float score_thr) {

    int n_selected_boxes = 0;
    /* first box, initialize box counting and area computation */
    if(boxes[0].score < score_thr)
        boxes[0] = {0};
    else
    {
        n_selected_boxes++;
        boxes[0].area = area(&boxes[0]);
    }

    for(int32_t i = 1; i < num_boxes; i++) {
        /* filter-out box with low scores */
        /* else pre-compute area of box */
        if(boxes[i].score < score_thr)
        {
            boxes[i] = {0};
        }
        else
        {
            n_selected_boxes++;
            boxes[i].area = area(&boxes[i]);
        }

        /* bubble sort... */
        /* TODO: replace with quicksort */
        for(int32_t j = i; j > 0; j-- ) {
            if(boxes[j].score > boxes[j-1].score) {
                swap_boxes(boxes, j-1, j);
            }
        }
    }

    return n_selected_boxes;
}
/* Filters boxes using NMS threshold on IoU.
 * Boxes that are filtered out are set to NULL.
 */
static void filter_boxes(box_data boxes[], int32_t num_boxes, float nms_thr) {

    for(int32_t i = 0; i < num_boxes; i++) {

        box_data* curr_box = &boxes[i];

        /* ignore box if already suppressed */
        if(curr_box->area == 0) {
            continue;
        }

        int32_t curr_label = curr_box->label;

        for(int32_t j = i+1; j < num_boxes; j++) {

            box_data* candidate = &boxes[j];

            /* ignore box if already suppressed */
            if(candidate->area == 0) {
                continue;
            }

            /* ignore box if label is different */
            if(candidate->label != curr_label) {
                continue;
            }

            /* remove box if IoU is too high */
            if (iou(curr_box, candidate) >= nms_thr) boxes[j] = {0};
        }
    }
}

void nms(box_data boxes[], int32_t num_boxes, float nms_thr, float score_thr) {
    assert(boxes != NULL);
    /* sort and filter boxes by score */
    int32_t n_selected_boxes = sort_boxes(boxes, num_boxes, score_thr);
    /* filter boxes by IoU */
    filter_boxes(boxes, n_selected_boxes, nms_thr);
}

int32_t nms_insert_box(box_data boxes[], box_data curr_box, int32_t n_inserted, float nms_thr, int32_t max_boxes) {

    // Compute area of the candidate box.
    curr_box.area = area(&curr_box);

    for (int32_t i = 0; i <= n_inserted; i++) {
        if(boxes[i].score >= curr_box.score && iou(&boxes[i], &curr_box) >= nms_thr) {
            // curr_box suppressed by IoU threshold
            return n_inserted;
        } else if(boxes[i].score < curr_box.score) {
            // Found box with lower score than the candidate.
            // Thus, insert box at this index.

            // Filter subsequent boxes by IoU
            for (int32_t j = i; j < n_inserted; j++) {
                if (iou(&boxes[j], &curr_box) >= nms_thr) {
                    // This box is overlapping and has lower score
                    // thus we remove it.
                    boxes[j] = {0};
                    // Fill the gap by moving all subsequent boxes up.
                    for (int32_t k = j; k < n_inserted-1; k++) {
                        swap_boxes(boxes, k, k+1);
                    }
                    // Next iteration must check the current index again
                    // since all subsequent boxes have been shifted.
                    j--;
                    // We removed a box.
                    n_inserted--;
                }
            }

            // Push all subsequent boxes by 1 to make room for curr_box.
            for (int32_t j = n_inserted-1; j >= i; j--) {
                swap_boxes(boxes, j, j+1);
            }

            // Insert curr_box at the chosen index.
            boxes[i] = curr_box;

            // Increment n_inserted but limit to max_boxes
            n_inserted = MIN(n_inserted+1, max_boxes);
            break;
        } else {
            continue;
        }
    }

    return n_inserted;
}
