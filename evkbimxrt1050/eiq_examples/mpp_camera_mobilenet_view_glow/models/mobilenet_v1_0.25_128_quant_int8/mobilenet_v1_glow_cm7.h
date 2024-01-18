/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Bundle API auto-generated header file. Do not edit!
// Glow Tools version: 2022-05-11 (2ee55ec50) (Glow_Release_MCUX_SDK_2.12.0)

#ifndef _GLOW_BUNDLE_MOBILENET_V1_H
#define _GLOW_BUNDLE_MOBILENET_V1_H

#include <stdint.h>

// ---------------------------------------------------------------
//                       Common definitions
// ---------------------------------------------------------------
#ifndef _GLOW_BUNDLE_COMMON_DEFS
#define _GLOW_BUNDLE_COMMON_DEFS

// Glow bundle error code for correct execution.
#define GLOW_SUCCESS 0

// Memory alignment definition with given alignment size
// for static allocation of memory.
#define GLOW_MEM_ALIGN(size)  __attribute__((aligned(size)))

// Macro function to get the absolute address of a
// placeholder using the base address of the mutable
// weight buffer and placeholder offset definition.
#define GLOW_GET_ADDR(mutableBaseAddr, placeholderOff)  (((uint8_t*)(mutableBaseAddr)) + placeholderOff)

#endif

// ---------------------------------------------------------------
//                          Bundle API
// ---------------------------------------------------------------
// Model name: "mobilenet_v1"
// Total data size: 630720 (bytes)
// Activations allocation efficiency: 1.0000
// Placeholders:
//
//   Name: "input"
//   Type: i8[S:0.007843138 O:-1][-0.996,1.004]<1 x 128 x 128 x 3>
//   Size: 49152 (elements)
//   Size: 49152 (bytes)
//   Offset: 0 (bytes)
//
//   Name: "MobilenetV1_Predictions_Reshape_1"
//   Type: float<1 x 1001>
//   Size: 1001 (elements)
//   Size: 4004 (bytes)
//   Offset: 49152 (bytes)
//
// NOTE: Placeholders are allocated within the "mutableWeight"
// buffer and are identified using an offset relative to base.
// ---------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

// Placeholder address offsets within mutable buffer (bytes).
#define MOBILENET_V1_input                              0
#define MOBILENET_V1_MobilenetV1_Predictions_Reshape_1  49152

// Memory sizes (bytes).
#define MOBILENET_V1_CONSTANT_MEM_SIZE     479168
#define MOBILENET_V1_MUTABLE_MEM_SIZE      53184
#define MOBILENET_V1_ACTIVATIONS_MEM_SIZE  98368

// Memory alignment (bytes).
#define MOBILENET_V1_MEM_ALIGN  64

// Bundle entry point (inference function). Returns 0
// for correct execution or some error code otherwise.
int mobilenet_v1(uint8_t *constantWeight, uint8_t *mutableWeight, uint8_t *activations);

#ifdef __cplusplus
}
#endif
#endif
