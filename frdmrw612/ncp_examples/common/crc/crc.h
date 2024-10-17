/*
 * Copyright 2022-2024 NXP 
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

#ifndef _CRC_H_
#define _CRC_H_

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define CRC32_POLY   0x04c11db7
#define CHECKSUM_LEN 4

/*******************************************************************************
 * API
 ******************************************************************************/

void ncp_tlv_chksum_init(void);
uint32_t ncp_tlv_chksum(uint8_t *buf, uint16_t len);

#endif
