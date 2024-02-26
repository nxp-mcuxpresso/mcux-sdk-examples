/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "fsl_phylan8741.h"
#include "fsl_enet.h"
#include "nx_driver_phy.h"
#include "fsl_debug_console.h"

#define PHY_AUTONEGO_TIMEOUT_COUNT          (1000UL)

static phy_lan8741_resource_t phy_resource;

static phy_config_t phy_config_lan8741 = {
    .phyAddr    = BOARD_ENET0_PHY_ADDRESS,
    .resource   = &phy_resource,
    .ops        = &phylan8741_ops,
    .autoNeg    = true,
};

static phy_config_t *phy_config = &phy_config_lan8741;

void nx_drvier_phy_init(phy_handle_t *handle,
                        enet_config_t *config,
                        mdio_read_f read,
                        mdio_write_f write)
{
    phy_lan8741_resource_t *res;
    phy_duplex_t duplex;
    phy_speed_t speed;
    uint32_t count;
    status_t status;
    bool link;
    bool autonego;

    res = phy_config->resource;
    res->read  = read;
    res->write = write;
    /* Initialize PHY and wait auto-negotiation over. */
    do
    {
        status = PHY_Init(handle, phy_config);
        if (status != kStatus_Success)
        {
            continue;
        }

        count = PHY_AUTONEGO_TIMEOUT_COUNT;
        do
        {
            SDK_DelayAtLeastUs(10000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

            PHY_GetAutoNegotiationStatus(handle, &autonego);
            PHY_GetLinkStatus(handle, &link);

            if (autonego && link)
            {
                break;
            }
        } while (--count);

        if (!autonego)
        {
            PRINTF("PHY Auto-negotiation failed. Please check the cable connection and link partner setting.\r\n");
        }
    } while (!(link && autonego));

    PHY_GetLinkSpeedDuplex(handle, &speed, &duplex);
    config->miiSpeed  = (enet_mii_speed_t)speed;
    config->miiDuplex = (enet_mii_duplex_t)duplex;
}
