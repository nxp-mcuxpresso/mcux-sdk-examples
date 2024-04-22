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
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#ifndef _FOE_SUPP_H_
#define _FOE_SUPP_H_

int32_t FoE_PartitionInit(void);

int32_t FoE_StoreImage(uint8_t *data, uint16_t size, uint32_t offset, uint8_t isEnd);

void FoE_UpdatePartition(void);

void FoE_UpdateImage(void);

uint8_t FoE_WriteFirmwareInformation(uint32_t flash_addr, uint8_t *name, uint8_t name_size, uint32_t firmware_size);

#endif /* _FOE_SUPP_H_ */

