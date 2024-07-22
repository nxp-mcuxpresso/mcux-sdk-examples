/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _NX_DRIVER_PHY_H
#define _NX_DRIVER_PHY_H

#include "fsl_phy.h"
#include "fsl_enet.h"

typedef status_t (*mdio_read_f)(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData);

typedef status_t (*mdio_write_f)(uint8_t phyAddr, uint8_t regAddr, uint16_t data);

void nx_driver_mdio_init(mdio_read_f read, mdio_write_f write);

void nx_drvier_phy_init(phy_handle_t *handle,
                        enet_config_t *config,
                        mdio_read_f read,
                        mdio_write_f write);

#endif /* _NX_DRIVER_PHY_H */
