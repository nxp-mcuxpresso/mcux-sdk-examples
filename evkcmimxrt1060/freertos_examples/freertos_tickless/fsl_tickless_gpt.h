/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef FSL_TICKLESS_GPT_H
#define FSL_TICKLESS_GPT_H

#include "fsl_clock.h"

#if defined(MIMXRT1176_cm7_SERIES) || defined(MIMXRT1176_cm4_SERIES) || defined(MIMXRT1166_cm7_SERIES) || \
    defined(MIMXRT1166_cm4_SERIES)
#define configGPT_CLOCK_HZ (CLOCK_GetFreq(kCLOCK_OscRc48MDiv2))

#elif defined(IMX8MSCALE_SERIES)
#define configGPT_CLOCK_HZ                                                                  \
    (CLOCK_GetPllFreq(kCLOCK_SystemPll1Ctrl) / (CLOCK_GetRootPreDivider(kCLOCK_RootGpt1)) / \
     (CLOCK_GetRootPostDivider(kCLOCK_RootGpt1)) / 20)
#elif defined(MIMXRT1181_cm33_SERIES) || defined(MIMXRT1182_cm33_SERIES) || defined(MIMXRT1187_cm33_SERIES) || \
    defined(MIMXRT1187_cm7_SERIES) || defined(MIMXRT1189_cm33_SERIES) || defined(MIMXRT1189_cm7_SERIES)
#define configGPT_CLOCK_HZ (CLOCK_GetRootClockFreq(kCLOCK_Root_Gpt1))
#else
#define configGPT_CLOCK_HZ (CLOCK_GetFreq(kCLOCK_IpgClk) / 2)
#endif
/* The GPT is a 32-bit counter. */
#define portMAX_32_BIT_NUMBER (0xffffffffUL)

#endif /* FSL_TICKLESS_GPT_H */
