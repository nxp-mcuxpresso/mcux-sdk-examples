// Bundle API header file
// Auto-generated file. Do not edit!
#ifndef _GLOW_BUNDLE_CIFAR10_H
#define _GLOW_BUNDLE_CIFAR10_H

#include <stdint.h>

// ---------------------------------------------------------------
//                       Common definitions
// ---------------------------------------------------------------
#ifndef _GLOW_BUNDLE_COMMON_DEFS
#define _GLOW_BUNDLE_COMMON_DEFS

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
// Total data size: 117504 (bytes)
// Placeholders:
//
//   Name: "data"
//   Type: float
//   Shape: [1, 3, 32, 32]
//   Size: 3072 (elements)
//   Size: 12288 (bytes)
//   Offset: 0 (bytes)
//
//   Name: "softmax"
//   Type: float
//   Shape: [1, 10]
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

// Placeholder address offsets within mutable buffer (bytes)
#define CIFAR10_data     0
#define CIFAR10_softmax  12288

// Memory sizes (bytes)
#define CIFAR10_CONSTANT_MEM_SIZE     33472
#define CIFAR10_MUTABLE_MEM_SIZE      12352
#define CIFAR10_ACTIVATIONS_MEM_SIZE  71680

// Memory alignment (bytes)
#define CIFAR10_MEM_ALIGN  64

// Bundle entry point (inference function)
void cifar10(uint8_t *constantWeight, uint8_t *mutableWeight, uint8_t *activations);

#ifdef __cplusplus
}
#endif
#endif
