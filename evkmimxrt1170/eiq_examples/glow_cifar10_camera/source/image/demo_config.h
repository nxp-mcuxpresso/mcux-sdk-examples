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
#define MODEL_INPUT_HEIGHT          32
#define MODEL_INPUT_WIDTH           32
#define MODEL_INPUT_SIZE            MODEL_INPUT_HEIGHT * MODEL_INPUT_WIDTH * IMAGE_CHANNELS

#define MODEL_COLOR_ORDER           BGR_COLOR_ORDER
#define MODEL_IMAGE_LAYOUT          NHWC_LAYOUT
#define MODEL_IMAGE_SCALE_MODE      SCALE_0TO1

#define EXTRACT_HEIGHT  160
#define EXTRACT_WIDTH   160

#endif // _DEMO_CONFIG_H_
