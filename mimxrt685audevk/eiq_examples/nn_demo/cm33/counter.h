/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __COUNTER_H__
#define __COUNTER_H__

/* MICROs to implement cycle counts on Cortex M */
#ifndef __ARMCCOUNT
#define __ARMCCOUNT
/* DWT (Data Watchpoint and Trace) registers, only exists on ARM Cortex with a DWT unit */
#define KIN1_DWT_CONTROL (*((volatile uint32_t *)0xE0001000))
/*!< DWT Control register */
#define KIN1_DWT_CYCCNTENA_BIT (1UL << 0)
/*!< CYCCNTENA bit in DWT_CONTROL register */
#define KIN1_DWT_CYCCNT (*((volatile uint32_t *)0xE0001004))
/*!< DWT Cycle Counter register */
#define KIN1_DEMCR (*((volatile uint32_t *)0xE000EDFC))
/*!< DEMCR: Debug Exception and Monitor Control Register */
#define KIN1_TRCENA_BIT (1UL << 24)
/*!< Trace enable bit in DEMCR register */

#define KIN1_InitCycleCounter() KIN1_DEMCR |= KIN1_TRCENA_BIT
/*!< TRCENA: Enable trace and debug block DEMCR (Debug Exception and Monitor Control Register */

#define KIN1_ResetCycleCounter() KIN1_DWT_CYCCNT = 0
/*!< Reset cycle counter */

#define KIN1_EnableCycleCounter() KIN1_DWT_CONTROL |= KIN1_DWT_CYCCNTENA_BIT
/*!< Enable cycle counter */

#define KIN1_DisableCycleCounter() KIN1_DWT_CONTROL &= ~KIN1_DWT_CYCCNTENA_BIT
/*!< Disable cycle counter */

#define KIN1_GetCycleCounter() KIN1_DWT_CYCCNT
/*!< Read cycle counter register */
unsigned long get_ccount(void);
#endif

#endif // __COUNTER_H__
