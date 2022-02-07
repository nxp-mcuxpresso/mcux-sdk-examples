// Bundle API auto-generated header file. Do not edit!
// Glow Tools version: 2021-05-28 (ce7f03408)

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
// Total data size: 112064 (bytes)
// Placeholders:
//
//   Name: "data"
//   Type: float<1 x 3 x 32 x 32>
//   Size: 3072 (elements)
//   Size: 12288 (bytes)
//   Offset: 0 (bytes)
//
//   Name: "softmax"
//   Type: float<1 x 10>
//   Size: 10 (elements)
//   Size: 40 (bytes)
//   Offset: 12288 (bytes)
//
// NOTE: Placeholders are allocated within the "mutableWeight"
// buffer and are identified using an offset relative to base.
// ---------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

// Placeholder address offsets within mutable buffer (bytes).
#define MODEL_data     0
#define MODEL_softmax  12288

// Memory sizes (bytes).
#define MODEL_CONSTANT_MEM_SIZE     34176
#define MODEL_MUTABLE_MEM_SIZE      12352
#define MODEL_ACTIVATIONS_MEM_SIZE  65536

// Memory alignment (bytes).
#define MODEL_MEM_ALIGN  64

// Bundle entry point (inference function). Returns 0
// for correct execution or some error code otherwise.
int model(uint8_t *constantWeight, uint8_t *mutableWeight, uint8_t *activations);

#ifdef __cplusplus
}
#endif
#endif
