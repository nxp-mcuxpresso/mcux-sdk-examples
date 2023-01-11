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
// Lenet Mnist model input data size (bytes).
#define LENET_MNIST_INPUT_SIZE    28*28*sizeof(float)

// Lenet Mnist model number of output classes.
#define LENET_MNIST_OUTPUT_CLASS  10

// Allocate buffer for input data. This buffer contains the input image
// pre-processed and serialized as text to include here.
uint8_t imageData[LENET_MNIST_INPUT_SIZE] = {
#include "input_image.inc"
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
  memcpy(inputAddr, imageData, sizeof(imageData));

  // Perform inference and compute inference time.
  start_time = get_time_in_us();
  lenet_mnist(constantWeight, mutableWeight, activations);
  stop_time = get_time_in_us();
  duration_ms = (stop_time - start_time) / 1000;

  // Get classification top1 result and confidence.
  float *out_data = (float*)(outputAddr);
  float max_val = 0.0;
  uint32_t max_idx = 0;
  for(int i = 0; i < LENET_MNIST_OUTPUT_CLASS; i++) {
    if (out_data[i] > max_val) {
      max_val = out_data[i];
      max_idx = i;
    }
  }

  // Print classification results.
  PRINTF("Top1 class = %lu\r\n", max_idx);
  PRINTF("Confidence = 0.%03u\r\n",(int)(max_val*1000));
  PRINTF("Inference time = %lu (ms)\r\n", duration_ms);

  return 0;
}

