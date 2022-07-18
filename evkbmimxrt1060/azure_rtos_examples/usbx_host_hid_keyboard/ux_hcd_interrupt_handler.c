/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "ux_api.h"
#include "ux_hcd_ehci.h"

#ifdef CPU_MIMXRT1042XJM5B
void USB_OTG_IRQHandler(void)
#else
void USB_OTG1_IRQHandler(void)
#endif
{
    ux_hcd_ehci_interrupt_handler();

    SDK_ISR_EXIT_BARRIER;
}
