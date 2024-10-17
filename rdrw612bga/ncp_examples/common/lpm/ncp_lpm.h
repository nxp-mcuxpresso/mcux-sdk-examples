/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */
#ifndef _LPM_H_
#define _LPM_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*
 * NCP handshake process definition
 * 0 -> not start
 * 1 -> in process
 * 2 -> finish
*/
#define NCP_LMP_HANDSHAKE_NOT_START  0
#define NCP_LMP_HANDSHAKE_IN_PROCESS 1
#define NCP_LMP_HANDSHAKE_FINISH     2


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

uint8_t lpm_getHandshakeState(void);
void lpm_setHandshakeState(uint8_t state);
void LPM_ConfigureNextLowPowerMode(uint8_t nextMode, uint32_t timeS);
#endif /* _LPM_H_ */
