/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "persondetect_output_postprocess.h"
#include <cmath>
#include <stdio.h>
#include "../get_top_n.h"
#include "fsl_debug_console.h"
#include "../utils.h"
#define EOL "\r\n"

#define MODEL_WIDTH 		160
#define MODEL_HEIGHT 		128

#define GRID_HEIGHT 		8 // Number of vertical cells in the output grid of Persondetect
#define GRID_WIDTH 			10 // Number of horizontal cells in the output grid of Persondetect

// Constants for data normalization/scaling
#define OUTPUT_ZERO_POINT -50.0f
#define OUTPUT_SCALE 		0.1308698058128357f
#define CANAL 				6 /* box data : Score,x, y, width, height, class  */
#define MAX_BOXES 			GRID_HEIGHT * GRID_WIDTH
#define NMS_THRESHOLD 		0.4f
#define SCORE_THRESHOLD 	30.0f

// Sigmoid function
float _sigmoid(float x)
{
	return (1 / (1 + std::exp(-x)));
}

/* The function that processes the tensor output of model Persondetect*/
int32_t Persondetect_Output_postprocessing(const mpp_inference_cb_param_t *inf_out,
		box_data *final_boxes)
{
	// Check for null pointers in input parameters
	if (inf_out == NULL)
	{
		PRINTF("ERROR: Persondetect_ProcessOutput parameter 'inf_out' is a null pointer" EOL);
		return -1;
	}
	if (final_boxes == NULL)
	{
		PRINTF("ERROR: Persondetect_ProcessOutput parameter 'final_boxes' is a null pointer" EOL);
		return -1;
	}

	// Variables for indexing
	int index1 = 0; /* Index used to access elements in inf_out data */
	int index2 = 0; /* Index used to access elements in final_boxes arrays.*/

	// Loop through the detection grid and process detection results
	for (int i = 0; i < GRID_HEIGHT; i++)
	{
		for (int j = 0; j < GRID_WIDTH; j++)
		{
			// Calculate index for each grid cell
			index1 = i * GRID_WIDTH * CANAL + j * CANAL;

			// Rescaled inf_out->out_tensors[0]->data values in intermediates variables
			float score = _sigmoid(static_cast<float>(((int8_t)(inf_out->out_tensors[0]->data[index1]) - OUTPUT_ZERO_POINT) * OUTPUT_SCALE));
			float x = tanh(static_cast<float>(((int8_t)(inf_out->out_tensors[0]->data[index1 + 1]) - OUTPUT_ZERO_POINT) * OUTPUT_SCALE));
			float y = tanh(static_cast<float>(((int8_t)(inf_out->out_tensors[0]->data[index1 + 2]) - OUTPUT_ZERO_POINT) * OUTPUT_SCALE));
			float w = _sigmoid(static_cast<float>(((int8_t)(inf_out->out_tensors[0]->data[index1 + 3]) - OUTPUT_ZERO_POINT) * OUTPUT_SCALE));
			float h = _sigmoid(static_cast<float>(((int8_t)(inf_out->out_tensors[0]->data[index1 + 4]) - OUTPUT_ZERO_POINT) * OUTPUT_SCALE));
			float classe = _sigmoid(static_cast<float>(((int8_t)(inf_out->out_tensors[0]->data[index1 + 5]) - OUTPUT_ZERO_POINT) * OUTPUT_SCALE));

			// Calculate and Adjust detection boxes dimensions to match those of the 'persondetect' model and save them in the final_boxes array.
			final_boxes[index2].left = (int16_t)(((x + j) / GRID_WIDTH) * MODEL_WIDTH - (w * MODEL_WIDTH) / 2);
			final_boxes[index2].top = (int16_t)(((y + i) / GRID_HEIGHT) * MODEL_HEIGHT - (h * MODEL_HEIGHT) / 2);
			final_boxes[index2].right = (int16_t)(((x + j) / GRID_WIDTH) * MODEL_WIDTH + (w * MODEL_WIDTH) / 2);
			final_boxes[index2].bottom = (int16_t)(((y + i) / GRID_HEIGHT) * MODEL_HEIGHT + (h * MODEL_HEIGHT) / 2);
			final_boxes[index2].score = score;
			final_boxes[index2].area = (int16_t)(w * MODEL_WIDTH * h * MODEL_HEIGHT);
			final_boxes[index2].label = (int16_t)classe;

			// Ensure bounding box coordinates are within model dimensions
			if (final_boxes[index2].left < 0)
			{
				final_boxes[index2].left = 0;
			}
			if (final_boxes[index2].bottom > MODEL_HEIGHT)
			{
				final_boxes[index2].bottom = MODEL_HEIGHT;
			}
			if (final_boxes[index2].right > MODEL_WIDTH)
			{
				final_boxes[index2].right = MODEL_WIDTH;
			}
			if (final_boxes[index2].top < 0)
			{
				final_boxes[index2].top = 0;
			}

			// Set score to 0 if the area is 0
			if (final_boxes[index2].area == 0)
			{
				final_boxes[index2].score = 0.00;
			}
			index2++;
		}
	}

	// Apply non-maximum suppression
	nms(final_boxes, MAX_BOXES, NMS_THRESHOLD, SCORE_THRESHOLD / 100);

	return 0;
}
