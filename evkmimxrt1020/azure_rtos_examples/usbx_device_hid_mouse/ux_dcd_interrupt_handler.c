/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "ux_api.h"
#include "ux_dcd_mcimx6.h"

void USB_OTG1_IRQHandler(void)
{
    _ux_dcd_mcimx6_interrupt_handler();

    SDK_ISR_EXIT_BARRIER;
}
