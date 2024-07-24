/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MYBINARY_H
#define MYBINARY_H

#include "fsl_loader_utils.h"

#define RW61X_WIFI_BIN_LEN      0x00084BA0

#if (RW61X_WIFI_BIN_LEN > WIFI_IMAGE_SIZE_MAX)
#warning "Embedded wifi binary file exceeds WIFI_IMAGE_SIZE_MAX used by loader component."
#endif

extern const unsigned char rw61x_wifi_bin[];

#endif /* MYBINARY_H */
