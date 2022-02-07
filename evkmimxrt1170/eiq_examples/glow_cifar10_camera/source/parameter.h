/*
 * Copyright (C) 2018 Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define CONV1_IN_DIM 32
#define CONV1_IN_CH 3
#define CONV1_KER_DIM 5
#define CONV1_PAD 2
#define CONV1_STRIDE 1
#define CONV1_OUT_CH 32
#define CONV1_OUT_DIM 32

#define POOL1_IN_DIM 32
#define POOL1_IN_CH 32
#define POOL1_KER_DIM 3
#define POOL1_STRIDE 2
#define POOL1_PAD 0
#define POOL1_OUT_DIM 16

#define RELU1_OUT_CH 32
#define RELU1_OUT_DIM 16

#define CONV2_IN_DIM 16
#define CONV2_IN_CH 32
#define CONV2_KER_DIM 5
#define CONV2_PAD 2
#define CONV2_STRIDE 1
#define CONV2_OUT_CH 32
#define CONV2_OUT_DIM 16

#define RELU2_OUT_CH 32
#define RELU2_OUT_DIM 16

#define POOL2_IN_DIM 16
#define POOL2_IN_CH 32
#define POOL2_KER_DIM 3
#define POOL2_STRIDE 2
#define POOL2_PAD 0
#define POOL2_OUT_DIM 8

#define CONV3_IN_DIM 8
#define CONV3_IN_CH 32
#define CONV3_KER_DIM 5
#define CONV3_PAD 2
#define CONV3_STRIDE 1
#define CONV3_OUT_CH 64
#define CONV3_OUT_DIM 8

#define RELU3_OUT_CH 64
#define RELU3_OUT_DIM 8

#define POOL3_IN_DIM 8
#define POOL3_IN_CH 64
#define POOL3_KER_DIM 3
#define POOL3_STRIDE 2
#define POOL3_PAD 0
#define POOL3_OUT_DIM 4

#define IP1_IN_DIM 1024
#define IP1_OUT_DIM 10

#define DATA_RSHIFT 0
#define CONV1_BIAS_LSHIFT 0
#define CONV1_OUT_RSHIFT 9
#define CONV2_BIAS_LSHIFT 0
#define CONV2_OUT_RSHIFT 9
#define CONV3_BIAS_LSHIFT 0
#define CONV3_OUT_RSHIFT 9
#define IP1_BIAS_LSHIFT 3
#define IP1_OUT_RSHIFT 5
