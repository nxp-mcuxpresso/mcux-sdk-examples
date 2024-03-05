/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _LPM_H_
#define _LPM_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if defined(configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY)
#ifndef LPM_RTC_PIN1_PRIORITY
#define LPM_RTC_PIN1_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1)
#endif
#else
#ifndef LPM_RTC_PIN1_PRIORITY
#define LPM_RTC_PIN1_PRIORITY (3U)
#endif
#endif

/*******************************************************************************
 * API
 ******************************************************************************/
void lpm_pm3_exit_hw_reinit();
int LPM_Init(void);
#endif /* _LPM_H_ */
