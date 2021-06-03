/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_common.h"
#include "fsl_mu.h"
#include "fsl_gpio.h"
#include "fsl_msmc.h"
#include "pin_mux.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_MU                        MUB
#define APP_MU_XFER_CMD_CHANNEL_INDEX 0U
#define APP_MU_SHAKE_HAND_VALUE       0xAAAAAAAA

#define APP_SMC SMC1

/* Flag indicates Core Boot Up*/
#define BOOT_FLAG 0x01U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Application entry.
 */
int main()
{
    uint32_t shake_hand_value;

    /* Init board hardware.*/
    BOARD_InitPins();
    /* Unlock the Very Low Power protection. */
    SMC_SetPowerModeProtection(APP_SMC, kSMC_AllowPowerModeAll);

    /* MUB init */
    MU_Init(APP_MU);
    /* Send flag to Core 0 to indicate Core 1 has startup */
    MU_SetFlags(APP_MU, BOOT_FLAG);

    /* Shake hands with master core. */
    shake_hand_value = MU_ReceiveMsg(APP_MU, APP_MU_XFER_CMD_CHANNEL_INDEX);
    if (APP_MU_SHAKE_HAND_VALUE == shake_hand_value)
    {
        MU_SendMsg(APP_MU, APP_MU_XFER_CMD_CHANNEL_INDEX, shake_hand_value);
    }

    while (1)
    {
        SMC_SetPowerModeVlls0(APP_SMC); /* Dive into the lowest power mode. */
    }
}
