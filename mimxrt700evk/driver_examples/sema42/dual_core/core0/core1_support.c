/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "board.h"
#include "core1_support.h"
#include "fsl_cache.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_CopyCore1Image(uint32_t addr)
{
    XCACHE_CleanInvalidateCacheByRange(addr, CORE1_IMAGE_SIZE);
    memcpy((void *)addr, (void *)CORE1_IMAGE_START, CORE1_IMAGE_SIZE);
    XCACHE_CleanInvalidateCacheByRange(addr, CORE1_IMAGE_SIZE);
}

void BOARD_ReleaseCore1Power()
{
    /* Powerup all the SRAM partitions. */
    PMC0->PDRUNCFG2 &= ~0x3FFC0000;
    PMC0->PDRUNCFG3 &= ~0x3FFC0000;

    POWER_DisablePD(kPDRUNCFG_SHUT_SENSEP_MAINCLK);
    POWER_ApplyPD();
}

void BOARD_BootCore1(uint32_t nsVector, uint32_t sVector)
{
    /*Glikey write enable, GLIKEY4*/
    GlikeyWriteEnable(GLIKEY4, 1U);

    /* Boot source for Core 1 from RAM. */
    SYSCON3->CPU1_NSVTOR = (nsVector >> 7U);
    SYSCON3->CPU1_SVTOR  = (sVector >> 7U);
    
    GlikeyClearConfig(GLIKEY4);

    /* Enable cpu1 clock. */
    CLOCK_EnableClock(kCLOCK_Cpu1);

    /* Clear reset*/
    RESET_ClearPeripheralReset(kCPU1_RST_SHIFT_RSTn);

    /* Release cpu wait*/
    SYSCON3->CPU_STATUS &= ~SYSCON3_CPU_STATUS_CPU_WAIT_MASK;
}