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

#if defined(IAR_FP_VLSTM_ALIGNED_ISSUE) && (IAR_FP_VLSTM_ALIGNED_ISSUE == 1U) && defined(__ICCARM__)
#define COMPILER_VERSION_MAJOR ((__VER__) / 1000000)
#define COMPILER_VERSION_MINOR (((__VER__) / 1000) % 1000)
#if (COMPILER_VERSION_MAJOR == 9) && (COMPILER_VERSION_MINOR < 32)
    {
        /*
         * IAR issue: in ResetHandler_ns asm code, only push 7 register into stack
         * for Armv8-M, Stack pointer need aligned with 8bytes, otherwise will triger
         * a exception.
         * After IAR (9.32) fixed this issue ,this code must be deleted.
         */

        register unsigned int tmpSp;
        int byteAlign;

        asm volatile("MOV %0, SP\n" : "=r"(tmpSp));

        /* One register size is 4byte */
        tmpSp = (tmpSp % 32) / 4;
        for (byteAlign = 0; (byteAlign + tmpSp + 7) % 8 != 0; byteAlign++)
        {
            asm volatile("PUSH {r12}");
        }
    }
#endif
#endif
    /* Call non-secure application - jump to normal world */
    ResetHandler_ns();
#if defined(IAR_FP_VLSTM_ALIGNED_ISSUE) && (IAR_FP_VLSTM_ALIGNED_ISSUE == 1U) && defined(__ICCARM__)
    {
        /* Never return here, so do not need 'POP' */
    }
#endif
}
