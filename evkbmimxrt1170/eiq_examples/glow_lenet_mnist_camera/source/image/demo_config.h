/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _DEMO_CONFIG_H_
#define _DEMO_CONFIG_H_
#include "image.h"

#define DETECTION_TRESHOLD 60
#define EOL "\r\n"

// Model input size
#define IMAGE_CHANNELS              3
#define MODEL_INPUT_HEIGHT          28
#define MODEL_INPUT_WIDTH           28
#define MODEL_INPUT_SIZE            MODEL_INPUT_HEIGHT * MODEL_INPUT_WIDTH * IMAGE_CHANNELS * sizeof(float)

#define RGB_COLOR_ORDER  0
#define BGR_COLOR_ORDER  1

#define NHWC_LAYOUT 0
#define NCHW_LAYOUT 1

#define SCALE_NEG1TO1     0
#define SCALE_0TO1        1
#define SCALE_0TO255      2
#define SCALE_NEG128TO127 3

#define MODEL_COLOR_ORDER           BGR_COLOR_ORDER
#define MODEL_IMAGE_LAYOUT          NCHW_LAYOUT
#define MODEL_IMAGE_SCALE_MODE      SCALE_0TO1


#endif // _DEMO_CONFIG_H_

