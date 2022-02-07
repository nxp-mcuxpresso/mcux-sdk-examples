// Bundle API header file
// Auto-generated file. Do not edit!
#ifndef _GLOW_BUNDLE_LENET_MNIST_H
#define _GLOW_BUNDLE_LENET_MNIST_H

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
// Model name: "lenet_mnist"
// Total data size: 455552 (bytes)
// Placeholders:
//
//   Name: "data"
//   Type: float
//   Shape: [1, 1, 28, 28]
//   Size: 784 (elements)
//   Size: 3136 (bytes)
//   Offset: 0 (bytes)
//
//   Name: "softmax"
//   Type: float
//   Shape: [1, 10]
//   Size: 10 (elements)
//   Size: 40 (bytes)
//   Offset: 3136 (bytes)
//
// NOTE: Placeholders are allocated within the "mutableWeight"
// buffer and are identified using an offset relative to base.
// ---------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

// Placeholder address offsets within mutable buffer (bytes)
#define LENET_MNIST_data     0
#define LENET_MNIST_softmax  3136

// Memory sizes (bytes)
#define LENET_MNIST_CONSTANT_MEM_SIZE     431360
#define LENET_MNIST_MUTABLE_MEM_SIZE      3200
#define LENET_MNIST_ACTIVATIONS_MEM_SIZE  20992

// Memory alignment (bytes)
#define LENET_MNIST_MEM_ALIGN  64

// Bundle entry point (inference function)
void lenet_mnist(uint8_t *constantWeight, uint8_t *mutableWeight, uint8_t *activations);

#ifdef __cplusplus
}
#endif
#endif
