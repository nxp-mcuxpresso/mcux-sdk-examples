/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_nor_flash.h"
#include "fsl_flexspi.h"
#include "app.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 *****************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
status_t Nor_Flash_Initialization(nor_config_t *config, nor_handle_t *handle)
{
    FLEXSPI_Type *base = (FLEXSPI_Type *)config->driverBaseAddr;

    /* Waiting for bus idle only when FLEXSPI enabled. */
    if ((base->MCR0 & FLEXSPI_MCR0_MDIS_MASK) != FLEXSPI_MCR0_MDIS_MASK)
    {
        /* Make sure flexspi bus idle before change its clock setting. */
        while (!FLEXSPI_GetBusIdleStatus(base))
        {
        }
    }

    /* Set flexspi clock. */
    FLEXSPI_ClockInit();

    return Nor_Flash_Init(config, handle);
}
