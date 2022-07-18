/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "nx_driver_imxrt.h"

#ifndef BOARD_NETWORK_USE_100M_ENET_PORT
#define BOARD_NETWORK_USE_100M_ENET_PORT    1
#endif

extern VOID nx_driver_imx_ethernet_isr(VOID);

/* the IRQ handler for the ENET network driver */
#if defined(BOARD_NETWORK_USE_100M_ENET_PORT) && (BOARD_NETWORK_USE_100M_ENET_PORT == 1)
VOID ENET_IRQHandler(VOID)
#else
VOID ENET_1G_IRQHandler(VOID)
#endif
{
    nx_driver_imx_ethernet_isr();

    SDK_ISR_EXIT_BARRIER;
}
