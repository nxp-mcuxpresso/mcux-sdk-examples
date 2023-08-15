/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file    main.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "timer.h"
#include "board.h"

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

// ---------------------------- Application -----------------------------
// Cifar10 model input data size (bytes).
#define CIFAR10_INPUT_SIZE    32*32*3

// Cifar10 model number of output classes.
#define CIFAR10_OUTPUT_CLASS  10

// Allocate buffer for input data. This buffer contains the input image
// pre-processed and serialized as text to include here.
uint8_t imageData[CIFAR10_INPUT_SIZE] = {
#include "input_image.inc"
};

// CIFAR-10 object class labels.
const char* CIFAR10_LABELS[] = {
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

/*
 * @brief   Application entry point.
 */
int main(void) {

  // Initialize hardware.
  BOARD_ConfigMPU();
  BOARD_InitBootPins();
  BOARD_InitBootClocks();
  BOARD_InitBootPeripherals();
  BOARD_InitDebugConsole();
  init_timer();

  // Timer variables.
  uint32_t start_time, stop_time;
  uint32_t duration_ms;

  // Produce input data for bundle.
  // Copy the pre-processed image data into the bundle input buffer.

  int8_t *bundleInput =(int8_t *)inputAddr;

  //Do scaling on image.
  for (int idx = 0; idx < sizeof(imageData); idx++)
  {
    int32_t tmp = imageData[idx];
    tmp -= 128;
	bundleInput[idx]=((int8_t)(tmp));
  }

  // Perform inference and compute inference time.
  start_time = get_time_in_us();
  cifar10(constantWeight, mutableWeight, activations);
  stop_time = get_time_in_us();
  duration_ms = (stop_time - start_time) / 1000;

  // Get classification top1 result and confidence.
  float *out_data = (float*)(outputAddr);
  float max_val = 0.0;
  uint32_t max_idx = 0;
  for(int i = 0; i < CIFAR10_OUTPUT_CLASS; i++) {
    if (out_data[i] > max_val) {
      max_val = out_data[i];
      max_idx = i;
    }
  }

  // Print classification results.
  PRINTF("Top1 class = %lu (%s)\r\n", max_idx, CIFAR10_LABELS[max_idx]);
  PRINTF("Confidence = 0.%03u\r\n",(int)(max_val*1000));
  PRINTF("Inference time = %lu (ms)\r\n", duration_ms);

  return 0;
}
