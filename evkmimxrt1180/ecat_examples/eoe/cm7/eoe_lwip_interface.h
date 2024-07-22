/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*-----------------------------------------------------------------------------------------
------
------    Includes
------
-----------------------------------------------------------------------------------------*/
#ifndef _EOE_LWIP_INTERFACE_H_
#define _EOE_LWIP_INTERFACE_H_

#include "applInterface.h"
#include "eoe_ethernetif.h"

void EoE_RecvFrame(uint16_t *pFrame, uint16_t frameSize);

err_t EoE_SendFrame(void);

void EoE_SetIp(uint16_t *aMacAdd, uint16_t *aIpAdd, uint16_t *aSubNetMask, uint16_t *aDefaultGateway, uint16_t *aDnsIp);

#endif // _EOE_LWIP_INTERFACE_H_
