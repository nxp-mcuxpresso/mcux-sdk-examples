/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "ux_api.h"
#include "ux_hcd_ehci.h"

void USB_OTG1_IRQHandler(void)
{
    ux_hcd_ehci_interrupt_handler();

    SDK_ISR_EXIT_BARRIER;
}
