/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _MODEL_H_
#define _MODEL_H_

#include <stdint.h>

#include "fsl_common.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define MAX_TENSOR_DIMS 4

typedef struct
{
  uint32_t size;
  uint32_t data[MAX_TENSOR_DIMS];
} tensor_dims_t;

typedef enum
{
    kTensorType_FLOAT32 = 0,
    kTensorType_UINT8 = 1,
    kTensorType_INT8 = 2
} tensor_type_t;

status_t MODEL_Init(void);
uint8_t* MODEL_GetInputTensorData(tensor_dims_t* dims, tensor_type_t* type);
uint8_t* MODEL_GetOutputTensorData(tensor_dims_t* dims, tensor_type_t* type);
void MODEL_ConvertInput(uint8_t* data, tensor_dims_t* dims, tensor_type_t type);
status_t MODEL_RunInference(void);
const char* MODEL_GetModelName(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _MODEL_H_ */
