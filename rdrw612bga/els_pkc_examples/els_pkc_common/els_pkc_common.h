/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _ELS_PKC_COMMON_H_
#define _ELS_PKC_COMMON_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXIT_CODE_ERROR 0x00U
#define EXIT_CODE_OK    0x01U
/*******************************************************************************
 * API
 ******************************************************************************/
extern bool mcuxClEls_Rng_Prng_Get_Random_example(void);
extern bool mcuxClEls_Common_Get_Info_example(void);
extern bool mcuxCsslFlowProtection_example(void);
extern uint32_t data_invariant_memory_compare(void);
extern uint32_t data_invariant_memory_copy(void);
extern bool mcuxClKey_example(void);

#endif /* _ELS_PKC_COMMON_H_ */
