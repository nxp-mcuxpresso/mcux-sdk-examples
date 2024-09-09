/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_crc.h"

#include "fsl_clock.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CRC0 CRC

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Init for CRC-16-CCITT.
 * @details Init CRC peripheral module for CRC-16/CCITT-FALSE protocol:
 *          width=16 poly=0x1021 init=0xffff refin=false refout=false xorout=0x0000 check=0x29b1
 *          http://reveng.sourceforge.net/crc-catalogue/
 * name="CRC-16/CCITT-FALSE"
 */
static void InitCrc16_CcittFalse(CRC_Type *base, uint32_t seed)
{
    crc_config_t config;

    /*
     * config.polynomial = 0x1021;
     * config.seed = 0xFFFF;
     * config.reflectIn = false;
     * config.reflectOut = false;
     * config.complementChecksum = false;
     * config.crcBits = kCrcBits16;
     * config.crcResult = kCrcFinalChecksum;
     */
    CRC_GetDefaultConfig(&config);
    config.seed = seed;
    CRC_Init(base, &config);
}

/*!
 * @brief Init for CRC-16/MAXIM.
 * @details Init CRC peripheral module for CRC-16/MAXIM protocol.
 *          width=16 poly=0x8005 init=0x0000 refin=true refout=true xorout=0xffff check=0x44c2 name="CRC-16/MAXIM"
 *          http://reveng.sourceforge.net/crc-catalogue/
 */
static void InitCrc16(CRC_Type *base, uint32_t seed)
{
    crc_config_t config;

    config.polynomial         = 0x8005;
    config.seed               = seed;
    config.reflectIn          = true;
    config.reflectOut         = true;
    config.complementChecksum = true;
    config.crcBits            = kCrcBits16;
    config.crcResult          = kCrcFinalChecksum;

    CRC_Init(base, &config);
}

/*!
 * @brief Init for CRC-16/KERMIT.
 * @details Init CRC peripheral module for CRC-16/KERMIT protocol.
 *          width=16 poly=0x1021 init=0x0000 refin=true refout=true xorout=0x0000 check=0x2189 name="KERMIT"
 *          http://reveng.sourceforge.net/crc-catalogue/
 */
static void InitCrc16_Kermit(CRC_Type *base, uint32_t seed)
{
    crc_config_t config;

    config.polynomial         = 0x1021;
    config.seed               = seed;
    config.reflectIn          = true;
    config.reflectOut         = true;
    config.complementChecksum = false;
    config.crcBits            = kCrcBits16;
    config.crcResult          = kCrcFinalChecksum;

    CRC_Init(base, &config);
}

/*!
 * @brief Init for CRC-32.
 * @details Init CRC peripheral module for CRC-32 protocol.
 *          width=32 poly=0x04c11db7 init=0xffffffff refin=true refout=true xorout=0xffffffff check=0xcbf43926
 *          name="CRC-32"
 *          http://reveng.sourceforge.net/crc-catalogue/
 */
static void InitCrc32(CRC_Type *base, uint32_t seed)
{
    crc_config_t config;

    config.polynomial         = 0x04C11DB7U;
    config.seed               = seed;
    config.reflectIn          = true;
    config.reflectOut         = true;
    config.complementChecksum = true;
    config.crcBits            = kCrcBits32;
    config.crcResult          = kCrcFinalChecksum;

    CRC_Init(base, &config);
}

/*!
 * @brief Init for CRC-32/POSIX.
 * @details Init CRC peripheral module for CRC-32/POSIX protocol.
 *          width=32 poly=0x04c11db7 init=0x00000000 refin=false refout=false xorout=0xffffffff check=0x765e7680
 *          name="CRC-32/POSIX"
 *          http://reveng.sourceforge.net/crc-catalogue/
 */
static void InitCrc32_Posix(CRC_Type *base, uint32_t seed)
{
    crc_config_t config;

    config.polynomial         = 0x04c11db7u;
    config.seed               = seed;
    config.reflectIn          = false;
    config.reflectOut         = false;
    config.complementChecksum = true;
    config.crcBits            = kCrcBits32;
    config.crcResult          = kCrcFinalChecksum;

    CRC_Init(base, &config);
}

/*!
 * @brief Main function
 */
int main(void)
{
    char testData[]                     = "123456789";
    const uint16_t checkCcittFalseCrc16 = 0x29b1u;
    const uint16_t checkMaximCrc16      = 0x44c2u;
    const uint16_t checkKermitCrc16     = 0x2189u;
    const uint32_t checkCrc32           = 0xcbf43926u;
    const uint32_t checkPosixCrc32      = 0x765e7680u;

    CRC_Type *base = CRC0;
    uint16_t checksum16;
    uint32_t checksum32;

    /* Init hardware*/
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    RESET_ClearPeripheralReset(kCRC0_RST_SHIFT_RSTn);

    PRINTF("CRC Peripheral Driver Example\r\n\r\n");

    /* ***************
     * CRC-16/CCITT-FALSE *
     *************** */
    InitCrc16_CcittFalse(base, 0xFFFFU);
    CRC_WriteData(base, (uint8_t *)&testData[0], sizeof(testData) - 1);
    checksum16 = CRC_Get16bitResult(base);

    PRINTF("Test string: %s\r\n", testData);
    PRINTF("CRC-16 CCITT FALSE: 0x%x\r\n", checksum16);
    if (checksum16 != checkCcittFalseCrc16)
    {
        PRINTF("...Check fail. Expected: 0x%x\r\n", checkCcittFalseCrc16);
    }

    /* ***************
     * CRC-16/MAXIM *
     *************** */
    InitCrc16(base, 0x0U);
    CRC_WriteData(base, (uint8_t *)&testData[0], sizeof(testData) - 1);
    checksum16 = CRC_Get16bitResult(base);

    PRINTF("CRC-16 MAXIM: 0x%x\r\n", checksum16);
    if (checksum16 != checkMaximCrc16)
    {
        PRINTF("...Check fail. Expected: 0x%x\r\n", checkMaximCrc16);
    }

    /* ***************
     * CRC-16 KERMIT *
     *************** */
    InitCrc16_Kermit(base, 0x0U);
    CRC_WriteData(base, (uint8_t *)&testData[0], sizeof(testData) - 1);
    checksum16 = CRC_Get16bitResult(base);

    PRINTF("CRC-16 KERMIT: 0x%x\r\n", checksum16);
    if (checksum16 != checkKermitCrc16)
    {
        PRINTF("...Check fail. Expected: 0x%x\r\n", checkKermitCrc16);
    }

    /* ***************
     * CRC-32 *
     *************** */
    InitCrc32(base, 0xFFFFFFFFU);
    CRC_WriteData(base, (uint8_t *)&testData[0], sizeof(testData) - 1);
    checksum32 = CRC_Get32bitResult(base);

    PRINTF("CRC-32: 0x%x\r\n", checksum32);
    if (checksum32 != checkCrc32)
    {
        PRINTF("...Check fail. Expected: 0x%x\r\n", checkCrc32);
    }

    /* ***************
     * CRC-32/POSIX *
     *************** */
    InitCrc32_Posix(base, 0);
    CRC_WriteData(base, (uint8_t *)&testData[0], sizeof(testData) - 1);
    checksum32 = CRC_Get32bitResult(base);

    PRINTF("CRC-32 POSIX: 0x%x\r\n", checksum32);
    if (checksum32 != checkPosixCrc32)
    {
        PRINTF("...Check fail. Expected: 0x%x\r\n", checkPosixCrc32);
    }

    while (1)
    {
    }
}
