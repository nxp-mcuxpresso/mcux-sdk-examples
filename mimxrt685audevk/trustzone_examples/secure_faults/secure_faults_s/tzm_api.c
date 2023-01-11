/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if (__ARM_FEATURE_CMSE & 1) == 0
#error "Need ARMv8-M security extensions"
#elif (__ARM_FEATURE_CMSE & 2) == 0
#error "Compile with --cmse"
#endif

#include "fsl_device_registers.h"
#include "arm_cmse.h"
#include "tzm_api.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* typedef for non-secure callback functions */
typedef void (*funcptr_ns)(void) TZM_IS_NONSECURE_CALLED;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief This function jumps to normal world.
 */
void TZM_JumpToNormalWorld(uint32_t nonsecVtorAddress)
{
    funcptr_ns ResetHandler_ns;

    /* Set non-secure main stack (MSP_NS) */
    __TZ_set_MSP_NS(*((uint32_t *)(nonsecVtorAddress)));

    /* Set non-secure vector table */
    SCB_NS->VTOR = nonsecVtorAddress;

    /* Get non-secure reset handler */
    ResetHandler_ns = (funcptr_ns)(*((uint32_t *)((nonsecVtorAddress) + 4U)));

/* IAR issue: in ResetHandler_ns asm code, only push 7 register into stack
   for Armv8-M, Stack pointer need aligned with 8bytes, otherwise will triger
   a exception.
   After IAR fixed this issue ,this code must be deleted.*/
#if defined(IAR_FP_VLSTM_ASSIGNED_ISSUE) && (IAR_FP_VLSTM_ASSIGNED_ISSUE == 1U)
    asm volatile("PUSH {r12}");
#endif
    /* Call non-secure application - jump to normal world */
    ResetHandler_ns();
#if defined(IAR_FP_VLSTM_ASSIGNED_ISSUE) && (IAR_FP_VLSTM_ASSIGNED_ISSUE == 1U)
    asm volatile("POP {r12}");
#endif
}
