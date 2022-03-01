/* ----------------------------------------------------------------------
* Copyright (C) 2010-2018 Arm Limited. All rights reserved.
* Copyright 2018-2019 NXP. All rights reserved.
*
*
* Project:       CMSIS NN Library
* Title:         arm_nnexamples_cifar10.cpp
*
* Description:   Convolutional Neural Network Example
*
* Target Processor: Cortex-M4/Cortex-M7
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*   - Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   - Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.
*   - Neither the name of Arm LIMITED nor the names of its contributors
*     may be used to endorse or promote products derived from this
*     software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
* -------------------------------------------------------------------- */

/* File modified by NXP. Changes are described in file
   /middleware/eiq/cmsis-nn/readme.txt in section "Release notes" */

#include <stdio.h>

#include <cmsis_compiler.h>

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"

#include "arm_nnfunctions.h"
#include "board_init.h"
#include "demo_config.h"
#include "image.h"
#include "labels.h"
#include "parameter.h"
#include "timer.h"
#include "weights.h"
#include "image.h"
#include "chgui.h"

// ----------------------------- Bundle API -----------------------------
// Bundle includes.
#include "glow_bundle_utils.h"
#include "cifar10.h"

// Statically allocate memory for constant weights (model weights) and initialize.
GLOW_MEM_ALIGN(CIFAR10_MEM_ALIGN)
uint8_t constantWeight[CIFAR10_CONSTANT_MEM_SIZE] = {
#include "cifar10.weights.txt"
};

// Statically allocate memory for mutable weights (model input/output data).
GLOW_MEM_ALIGN(CIFAR10_MEM_ALIGN)
uint8_t mutableWeight[CIFAR10_MUTABLE_MEM_SIZE];

// Statically allocate memory for activations (model intermediate results).
GLOW_MEM_ALIGN(CIFAR10_MEM_ALIGN)
uint8_t activations[CIFAR10_ACTIVATIONS_MEM_SIZE];

// Bundle input data absolute address.
uint8_t *inputAddr = GLOW_GET_ADDR(mutableWeight, CIFAR10_input);

// Bundle output data absolute address.
uint8_t *outputAddr = GLOW_GET_ADDR(mutableWeight, CIFAR10_CifarNet_Predictions_Reshape_1);


/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define IMAGE_WIDTH 32
#define IMAGE_HEIGHT 32
#define IMAGE_CHANNELS 3

#define INPUT_MEAN_SHIFT {125,123,114}
#define INPUT_RIGHT_SHIFT {8,8,8}

#define RGB_COLOR_ORDER  0
#define BGR_COLOR_ORDER  1

#define NHWC_LAYOUT 0
#define NCHW_LAYOUT 1

#define SCALE_NEG1TO1     0
#define SCALE_0TO1        1
#define SCALE_0TO255      2
#define SCALE_NEG128TO127 3

// Number of output classes for the model.
#define MODEL_NUM_OUTPUT_CLASSES  10

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static uint8_t s_imageData[IMAGE_WIDTH * IMAGE_HEIGHT * IMAGE_CHANNELS];
uint32_t max_idx, display_idx, last_max_idx = 0;
const char* LABELS[] = {
    "airplane",
    "automobile",
    "bird",
    "cat",
    "deer",
    "dog",
    "frog",
    "horse",
    "ship",
    "truck"
};

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief  Run inference. It processed static image if static image is not NULL
 * otherwise camera input is processed.
 *
 * param staticImage pointer to address of static image. Use NULL when static image
 * is not required.
 * param staticImageLen size of static image.
 */
void run_inference(uint8_t *image_data, const char* labels[])
{
  uint32_t start = 0U;
  uint32_t end = 0U;
  uint32_t duration_ms = 0U;

  int8_t *bundleInput =(int8_t *)inputAddr;

  //Do scaling on image.
  for (int idx = 0; idx < MODEL_INPUT_SIZE; idx++)
  {
    int32_t tmp = image_data[idx];
    tmp -= 128;
	bundleInput[idx]=((int8_t)(tmp));
  }

  start = get_time_in_us();
  cifar10(constantWeight, mutableWeight, activations);
  end = get_time_in_us();
  duration_ms = (end - start) / 1000;

  // Get classification top1 result and confidence.
 	  float *out_data = (float*)(outputAddr);
 	  float max_val = 0.0;

 	  for(int i = 0; i < MODEL_NUM_OUTPUT_CLASSES; i++) {
 	    if (out_data[i] > max_val) {
 	      max_val = out_data[i];
 	      max_idx = i;
 	    }
 	  }

 	  //Update LCD display category if get same result twice in a row.
 	  if(max_idx==last_max_idx)
 	  {
 		  display_idx=max_idx;
 	  }
 	  last_max_idx=max_idx;

 	  // Print classification results if Confidence > Threshold.
 	 printf("Top1 class = %lu (%s)\r\n", max_idx, labels[max_idx]);
 	printf("Confidence = 0.%03u\r\n",(int)(max_val*1000));
 	printf("Inference time = %lu (ms)\r\n", duration_ms);
 	GUI_PrintfToBuffer(GUI_X_POS, GUI_Y_POS, "Detected: %.20s (%d%%)", labels[max_idx], (int)(max_val*100));
}

/*!
 * @brief Main function
 */
int main(void)
{
  BOARD_Init();
  init_timer();

  printf("CIFAR-10 object recognition example using Glow.\r\n");
  printf("Detection threshold: %d%%\r\n", DETECTION_TRESHOLD);

  while (1)
  {
    if (IMAGE_GetImage(s_imageData, IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_CHANNELS) != kStatus_Success)
    {
      printf("Failed retrieving input image\r\n");
      for (;;) {}
    }

    run_inference(s_imageData, LABELS);
  }
}
