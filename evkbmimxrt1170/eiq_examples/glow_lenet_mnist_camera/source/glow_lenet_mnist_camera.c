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
#include "timer.h"
#include "image.h"
#include "chgui.h"

// ----------------------------- Bundle API -----------------------------
// Bundle includes.
#include "lenet_mnist.h"
#include "glow_bundle_utils.h"

// Statically allocate memory for constant weights (model weights) and initialize.
GLOW_MEM_ALIGN(LENET_MNIST_MEM_ALIGN)
uint8_t constantWeight[LENET_MNIST_CONSTANT_MEM_SIZE] = {
#include "lenet_mnist.weights.txt"
};

// Statically allocate memory for mutable weights (model input/output data).
GLOW_MEM_ALIGN(LENET_MNIST_MEM_ALIGN)
uint8_t mutableWeight[LENET_MNIST_MUTABLE_MEM_SIZE];

// Statically allocate memory for activations (model intermediate results).
GLOW_MEM_ALIGN(LENET_MNIST_MEM_ALIGN)
uint8_t activations[LENET_MNIST_ACTIVATIONS_MEM_SIZE];

// Bundle input data absolute address.
uint8_t *inputAddr = GLOW_GET_ADDR(mutableWeight, LENET_MNIST_data);

// Bundle output data absolute address.
uint8_t *outputAddr = GLOW_GET_ADDR(mutableWeight, LENET_MNIST_softmax);

// ---------------------------- Application -----------------------------

#define IMAGE_WIDTH    28
#define IMAGE_HEIGHT   28
#define IMAGE_CHANNELS  3

// Number of output classes for the model.
#define MODEL_NUM_OUTPUT_CLASSES  10

// Allocate buffer for input data. This buffer contains the input image
// pre-processed and serialized as text to include here.
/*
uint8_t imageData[MODEL_INPUT_SIZE] = {
#include "input_image.inc"
};
*/

static uint8_t s_imageData[IMAGE_WIDTH * IMAGE_HEIGHT * IMAGE_CHANNELS];
static uint8_t s_monochrome[MODEL_INPUT_WIDTH * MODEL_INPUT_HEIGHT];
uint32_t max_idx, display_idx, last_max_idx = 0;
const char* LABELS[MODEL_NUM_OUTPUT_CLASSES] = {
		"0",
		"1",
		"2",
		"3",
		"4",
		"5",
		"6",
		"7",
		"8",
		"9"
};

uint8_t color_threshold = 140;

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
void run_inference(const uint8_t *image_data, const char* labels[], uint8_t scale_mode)
{
  uint32_t start = 0U;
  uint32_t end = 0U;
  uint32_t duration_ms = 0U;

  // Produce input data for bundle.
  // Copy the pre-processed image data into the bundle input buffer.s)
  float *bundleInput =(float *)inputAddr;

  //Do scaling on image.
  for (int idx = 0; idx < MODEL_INPUT_HEIGHT * MODEL_INPUT_WIDTH; idx++)
  {
    if(scale_mode==SCALE_NEG1TO1)
    {
      bundleInput[idx]=(((((float)image_data[idx])/255.0)*2)-1.0);
    }
    else if(scale_mode==SCALE_0TO1)
    {
      bundleInput[idx]=(((float)image_data[idx])/255.0);
    }
    else if(scale_mode==SCALE_NEG128TO127)
    {
      bundleInput[idx]=(((float)image_data[idx])-128);
    }
    else if(scale_mode==SCALE_0TO255)
    {
      //Do nothing as data already scaled from 0 to 255
      bundleInput[idx]=(float)image_data[idx];
    }
 }

  start = get_time_in_us();
  lenet_mnist(constantWeight, mutableWeight, activations);
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

void convert_to_monochrome(uint8_t *color, uint8_t *monochrome, bool static_image)
{
  /* Convert color values to a single monochrome value, using luminosity values for better accuracy */
   //.2125 * Red
   //.7154 * Green
   //.0721 * Blue
   //Algorithm is slightly different based on color order and layout order

    int layout = MODEL_IMAGE_LAYOUT;

    if (static_image)
	layout = NCHW_LAYOUT;
    else
	layout = NHWC_LAYOUT;	

    for (int idx = 0; idx < MODEL_INPUT_HEIGHT * MODEL_INPUT_WIDTH; idx++)
    {

      if(MODEL_COLOR_ORDER==RGB_COLOR_ORDER)
      {
        if(layout==NCHW_LAYOUT)
        {
          //RGB NCHW
          monochrome[idx]=(int)(.2125*color[idx]+.7154*color[idx+MODEL_INPUT_WIDTH*MODEL_INPUT_HEIGHT]+.0721*color[idx+MODEL_INPUT_WIDTH*MODEL_INPUT_HEIGHT*2]);
        }
        else
        {
          //RGB NHWC
            monochrome[idx]=(int)(.2125*color[idx*3]+.7154*color[idx*3+1]+.0721*color[idx*3+2]);
        }
      }
      else
      {
        if(layout==NCHW_LAYOUT)
        {
          //BGR NCHW
          monochrome[idx]=(int)(.0721*color[idx]+.7154*color[idx+MODEL_INPUT_WIDTH*MODEL_INPUT_HEIGHT]+.2125*color[idx+MODEL_INPUT_WIDTH*MODEL_INPUT_HEIGHT*2]);
        }
        else
        {
          //BGR NCHW
            monochrome[idx]=(int)(.0721*color[idx*3]+.7154*color[idx*3+1]+.2125*color[idx*3+2]);
        }
      }
    }
    /* LeNet MNIST model expects white numbers on black background. Because camera is looking at black numbers on white background, flip the contrast */
    for (int idx = 0; idx < MODEL_INPUT_HEIGHT * MODEL_INPUT_WIDTH; idx++)
    {
      monochrome[idx]=((monochrome[idx]-255)*-1);

      //To also help with accuracy, make pixels max/min values
      if(monochrome[idx]<color_threshold)
      {
        monochrome[idx]=0;
      }
      else
      {
        monochrome[idx]=255;
      }

      //Makes number thicker by adding extra width if value to the left was zero for better accuracy
      if((monochrome[idx-1]==0) && (monochrome[idx]==255))
      {
        monochrome[idx-1]=255;
      }
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
  BOARD_Init();
  init_timer();

  PRINTF("LeNet MNIST Camera Demo with Glow\r\n");
  printf("Detection threshold: %d%%\r\n", DETECTION_TRESHOLD);

  bool static_image = true;

  while (1)
  {
    if (IMAGE_GetImage(s_imageData, IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_CHANNELS) != kStatus_Success)
    {
      printf("Failed retrieving input image\r\n");
      for (;;) {}
    }

    /* Convert the color image to monochrome */
    convert_to_monochrome(s_imageData, s_monochrome, static_image);

    /* Run inference on the resized and decolorized data */
    run_inference(s_monochrome, LABELS, MODEL_IMAGE_SCALE_MODE);

    static_image = false;
  }
}

