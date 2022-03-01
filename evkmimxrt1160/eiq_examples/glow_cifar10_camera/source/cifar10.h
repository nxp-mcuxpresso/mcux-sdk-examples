// Bundle API auto-generated header file. Do not edit!
// Glow Tools version: 2021-12-10 (1a77debd7) (Glow_Release_MCUX_SDK_2.11.0_REL15)

#ifndef _GLOW_BUNDLE_CIFAR10_H
#define _GLOW_BUNDLE_CIFAR10_H

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
// Model name: "cifar10"
// Total data size: 134208 (bytes)
// Activations allocation efficiency: 1.0000
// Placeholders:
//
//   Name: "CifarNet_Predictions_Reshape_1"
//   Type: float<1 x 10>
//   Size: 10 (elements)
//   Size: 40 (bytes)
//   Offset: 3072 (bytes)
//
//   Name: "input"
//   Type: i8[S:0.007812500 O:0][-1.000,0.992]<1 x 32 x 32 x 3>
//   Size: 3072 (elements)
//   Size: 3072 (bytes)
//   Offset: 0 (bytes)
//
// NOTE: Placeholders are allocated within the "mutableWeight"
// buffer and are identified using an offset relative to base.
// ---------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

// Placeholder address offsets within mutable buffer (bytes).
#define CIFAR10_CifarNet_Predictions_Reshape_1  3072
#define CIFAR10_input                           0

// Memory sizes (bytes).
#define CIFAR10_CONSTANT_MEM_SIZE     90112
#define CIFAR10_MUTABLE_MEM_SIZE      3136
#define CIFAR10_ACTIVATIONS_MEM_SIZE  40960

// Memory alignment (bytes).
#define CIFAR10_MEM_ALIGN  64

// Bundle entry point (inference function). Returns 0
// for correct execution or some error code otherwise.
int cifar10(uint8_t *constantWeight, uint8_t *mutableWeight, uint8_t *activations);

#ifdef __cplusplus
}
#endif
#endif
