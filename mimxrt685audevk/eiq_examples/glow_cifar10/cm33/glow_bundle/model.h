// Bundle API auto-generated header file. Do not edit!
// Glow Tools version: 2021-10-21 (d036c02dd)

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
#define GLOW_MEM_ALIGN(size) __attribute__((aligned(size)))

// Macro function to get the absolute address of a
// placeholder using the base address of the mutable
// weight buffer and placeholder offset definition.
#define GLOW_GET_ADDR(mutableBaseAddr, placeholderOff) (((uint8_t *)(mutableBaseAddr)) + placeholderOff)

#endif

// ---------------------------------------------------------------
//                          Bundle API
// ---------------------------------------------------------------
// Model name: "model"
// Total data size: 137472 (bytes)
// Activations allocation efficiency: 1.0000
// Placeholders:
//
//   Name: "input"
//   Type: i8[S:0.007812500 O:0][-1.000,0.992]<1 x 32 x 32 x 3>
//   Size: 3072 (elements)
//   Size: 3072 (bytes)
//   Offset: 0 (bytes)
//
//   Name: "CifarNet_Predictions_Reshape_1"
//   Type: float<1 x 10>
//   Size: 10 (elements)
//   Size: 40 (bytes)
//   Offset: 3072 (bytes)
//
// NOTE: Placeholders are allocated within the "mutableWeight"
// buffer and are identified using an offset relative to base.
// ---------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

// Placeholder address offsets within mutable buffer (bytes).
#define MODEL_input                          0
#define MODEL_CifarNet_Predictions_Reshape_1 3072

// Memory sizes (bytes).
#define MODEL_CONSTANT_MEM_SIZE    90048
#define MODEL_MUTABLE_MEM_SIZE     3136
#define MODEL_ACTIVATIONS_MEM_SIZE 44288

// Memory alignment (bytes).
#define MODEL_MEM_ALIGN 64

// Bundle entry point (inference function). Returns 0
// for correct execution or some error code otherwise.
int model(uint8_t *constantWeight, uint8_t *mutableWeight, uint8_t *activations);

#ifdef __cplusplus
}
#endif
#endif
