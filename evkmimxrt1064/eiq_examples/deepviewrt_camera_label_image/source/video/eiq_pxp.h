/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EIQ_PXP_H_
#define _EIQ_PXP_H_

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

#include "fsl_pxp.h"

/*!
 * @addtogroup eiq_pxp
 * @{
 */

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @brief PXP initialization.
 *
 * Initializes PXP driver for conversion image data from camera buffer to LCD buffer.
 *
 */
void EIQ_PXP_Init(void);

/*!
 * @brief PXP Rotate.
 *
 * This function copies and transforms input buffer data to the output data buffer.
 *
 */
void EIQ_PXP_Rotate(uint32_t input_buffer, uint32_t output_buffer);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _IMAGE_H_ */
