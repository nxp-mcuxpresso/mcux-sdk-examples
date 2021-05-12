/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef FSL_TICKLESS_LPTMR_H
#define FSL_TICKLESS_LPTMR_H

#if defined(CPU_MIMX8QM6AVUFF_cm4_core0)
#define configLPIT_CLOCK_HZ CLOCK_GetIpFreq(kCLOCK_M4_0_Lpit)
#elif defined(CPU_MIMX8QM6AVUFF_cm4_core1)
#define configLPIT_CLOCK_HZ CLOCK_GetIpFreq(kCLOCK_M4_1_Lpit)
#elif defined(CPU_MIMX8QM6AVUFF)
#define configLPIT_CLOCK_HZ CLOCK_GetIpFreq(kCLOCK_M4_0_Lpit)
#elif defined(CPU_MIMX8QX6AVLFZ)
#define configLPIT_CLOCK_HZ CLOCK_GetIpFreq(kCLOCK_M4_0_Lpit)
#elif defined(CPU_MIMX8DL1AVNFZ)
#define configLPIT_CLOCK_HZ CLOCK_GetIpFreq(kCLOCK_M4_0_Lpit)
#endif
/* The LPIT is a 32-bit counter. */
#define portMAX_32_BIT_NUMBER (0xffffffffUL)

#endif /* FSL_TICKLESS_LPTMR_H */
