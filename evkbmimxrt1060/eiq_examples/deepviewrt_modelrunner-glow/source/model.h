// Bundle API auto-generated header file. Do not edit!
// Glow Tools version: 2020-11-26

#ifndef _GLOW_BUNDLE_MODEL_H
#define _GLOW_BUNDLE_MODEL_H

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
// Model name: "model"
// Total data size: 2704000 (bytes)
// Placeholders:
//
//   Name: "input"
//   Type: i8[S:0.007874016 O:0][-1.008,1.000]<1 x 224 x 224 x 3>
//   Size: 150528 (elements)
//   Size: 150528 (bytes)
//   Offset: 0 (bytes)
//
//   Name: "MobilenetV2_Predictions_Softmax"
//   Type: float<1 x 1001>
//   Size: 1001 (elements)
//   Size: 4004 (bytes)
//   Offset: 150528 (bytes)
//
// NOTE: Placeholders are allocated within the "mutableWeight"
// buffer and are identified using an offset relative to base.
// ---------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

// Placeholder address offsets within mutable buffer (bytes).
#define MODEL_input                            0
#define MODEL_MobilenetV2_Predictions_Softmax  150528

// Memory sizes (bytes).
#define MODEL_CONSTANT_MEM_SIZE     1696448
#define MODEL_MUTABLE_MEM_SIZE      154560
#define MODEL_ACTIVATIONS_MEM_SIZE  852992

// Memory alignment (bytes).
#define MODEL_MEM_ALIGN  64

// Bundle entry point (inference function). Returns 0
// for correct execution or some error code otherwise.
int model(uint8_t *constantWeight, uint8_t *mutableWeight, uint8_t *activations);

#ifdef __cplusplus
}
#endif
#endif
