/*
 * Copyright 2021-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Set the operations used in model MobileNet_v1. This allows reducing the code size.
 * Important Notice: User may find the list of operations needed by its model using tool https://netron.app
 */

#include "mpp_config.h"

#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/kernels/softmax.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#if defined(APP_USE_NEUTRON16_MODEL) || defined(APP_USE_NEUTRON64_MODEL)
#include "tensorflow/lite/micro/kernels/neutron/neutron.h"
#endif

tflite::MicroOpResolver &MODEL_GetOpsResolver()
{
#ifdef APP_USE_NEUTRON64_MODEL
    static tflite::MicroMutableOpResolver<5> s_microOpResolver;

    s_microOpResolver.AddDequantize();
    s_microOpResolver.AddReshape();
    s_microOpResolver.AddSlice();
    s_microOpResolver.AddSoftmax();
    s_microOpResolver.AddCustom(tflite::GetString_NEUTRON_GRAPH(),
        tflite::Register_NEUTRON_GRAPH());
#elif defined(APP_USE_NEUTRON16_MODEL)
    static tflite::MicroMutableOpResolver<8> s_microOpResolver;

    s_microOpResolver.AddAveragePool2D();
    s_microOpResolver.AddConv2D();
    s_microOpResolver.AddDepthwiseConv2D();
    s_microOpResolver.AddDequantize();
    s_microOpResolver.AddReshape();
    s_microOpResolver.AddSlice();
    s_microOpResolver.AddSoftmax();
    s_microOpResolver.AddCustom(tflite::GetString_NEUTRON_GRAPH(),
        tflite::Register_NEUTRON_GRAPH());
#else
    static tflite::MicroMutableOpResolver<6> s_microOpResolver;

    s_microOpResolver.AddAveragePool2D();
    s_microOpResolver.AddConv2D();
    s_microOpResolver.AddDepthwiseConv2D();
    s_microOpResolver.AddDequantize();
    s_microOpResolver.AddReshape();
    s_microOpResolver.AddSoftmax();
#endif
    return s_microOpResolver;
}
