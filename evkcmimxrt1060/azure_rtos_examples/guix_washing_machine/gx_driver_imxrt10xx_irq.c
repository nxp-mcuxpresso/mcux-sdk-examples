/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "tx_api.h"

extern void APP_LCDIF_IRQHandler(void);

/* the IRQ handler for the LCDIF driver */
VOID LCDIF_IRQHandler(VOID)
{
    APP_LCDIF_IRQHandler();

    SDK_ISR_EXIT_BARRIER;
}
