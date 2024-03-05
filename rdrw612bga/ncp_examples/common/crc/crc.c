/*
 * Copyright 2022-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_crc.h"
#include "crc.h"

#ifdef CONFIG_CRC32_HW_ACCELERATE
void ncp_tlv_chksum_init(void)
{
    crc_config_t crcUserConfig;
    crcUserConfig.seed          = 0U;
    crcUserConfig.polynomial    = kCRC_Polynomial_CRC_32;
    crcUserConfig.reverseIn     = false;
    crcUserConfig.reverseOut    = false;
    crcUserConfig.complementIn  = false;
    crcUserConfig.complementOut = false;
    CRC_Init(CRC, &crcUserConfig);
}

uint32_t ncp_tlv_chksum(uint8_t *buf, uint16_t len)
{
    uint32_t crc;

    CRC_WriteSeed(CRC, 0xffffffffU);
    CRC_WriteData(CRC, buf, len);
    crc = CRC_Get32bitResult(CRC);

    return ~crc;
}
#else
void ncp_tlv_chksum_init(void)
{
    int i, j;
    unsigned int c;
    for (i = 0; i < 256; ++i)
    {
        for (c = i << 24, j = 8; j > 0; --j)
            c = c & 0x80000000 ? (c << 1) ^ CRC32_POLY : (c << 1);
        crc32_table[i] = c;
    }
}

uint32_t ncp_tlv_chksum(uint8_t *buf, uint16_t len)
{
    uint8_t *p;
    unsigned int crc;
    crc = 0xffffffff;
    for (p = buf; len > 0; ++p, --len)
        crc = (crc << 8) ^ (crc32_table[(crc >> 24) ^ *p]);
    return ~crc;
}
#endif
