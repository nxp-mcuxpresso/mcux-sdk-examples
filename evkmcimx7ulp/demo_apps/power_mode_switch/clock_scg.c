/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_config.h"
#include <assert.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
AT_QUICKACCESS_SECTION_CODE(extern void BOARD_SetRunMode(
    SCG_Type *scg, uint32_t scgRunConfig, QuadSPI_Type *qspi, clock_ip_name_t qspiClock, uint32_t qspiClockConfig));
extern const scg_sys_clk_config_t g_sysClkConfigNormalRun;
extern const scg_sys_clk_config_t g_sysClkConfigVlpr;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern bool BOARD_IsRunOnQSPI(void);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*
 * Set the clock configuration for HSRUN mode.
 */
void APP_SetClockHsrun(void)
{
}

/*
 * Set the clock configuration for RUN mode from HSRUN mode.
 */
void APP_SetClockRunFromHsrun(void)
{
}

/*
 * Set the clock configuration for RUN mode from VLPR mode.
 */
void APP_SetClockRunFromVlpr(void)
{
    QuadSPI_Type *qspi = BOARD_IsRunOnQSPI() ? QuadSPI0 : NULL;
    uint32_t *config   = (uint32_t *)(&g_sysClkConfigNormalRun);

    /* Recover SPLL */
    SCG0->SPLLCSR |= SCG_SPLLCSR_SPLLEN_MASK;
    while (!CLOCK_IsSysPllValid())
    {
    }

    /* When switching from VLPR to RUN, switch RUN clock source back to RUN source */
    BOARD_SetRunMode(SCG0, *config, qspi, kCLOCK_Qspi,
                     PCC1_PCC_QSPI_OTFAD_CGC_MASK | PCC1_PCC_QSPI_OTFAD_PCS(3)); /* QSPI source: 48M FIRC Async */
}

/*
 * Set the clock configuration for VLPR mode.
 */
void APP_SetClockVlpr(void)
{
    QuadSPI_Type *qspi = BOARD_IsRunOnQSPI() ? QuadSPI0 : NULL;
    uint32_t *config   = (uint32_t *)(&g_sysClkConfigVlpr);

    /* When switching from RUN to VLPR, first switch RUN clock source to VLPR source */
    BOARD_SetRunMode(SCG0, *config, qspi, kCLOCK_Qspi,
                     PCC1_PCC_QSPI_OTFAD_CGC_MASK | PCC1_PCC_QSPI_OTFAD_PCS(2)); /* QSPI source: 16M SIRC Async */

    /* Disable SPLL to work around hardware issue */
    SCG0->SPLLCSR &= ~SCG_SPLLCSR_SPLLEN_MASK;
}
