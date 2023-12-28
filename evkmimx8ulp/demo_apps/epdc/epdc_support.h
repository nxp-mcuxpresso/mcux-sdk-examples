/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EPDC_SUPPORT_H_
#define _EPDC_SUPPORT_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

void DEMO_PowerOnPxp(void);
void DEMO_PowerOnEpdc(void);
status_t DEMO_InitEpdcPanel(void);
status_t DEMO_GetEpdcTemp(uint8_t *temp);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _EPDC_SUPPORT_H_ */
