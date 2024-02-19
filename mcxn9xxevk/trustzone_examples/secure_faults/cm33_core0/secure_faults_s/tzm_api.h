/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __TZM_API_H__
#define __TZM_API_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define TZM_IS_NONSECURE_CALLED __attribute__((cmse_nonsecure_call))
#define TZM_IS_NOSECURE_ENTRY   __attribute__((cmse_nonsecure_entry))

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

void TZM_JumpToNormalWorld(uint32_t nonsecVtorAddress);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* __TZM_API_H__ */
