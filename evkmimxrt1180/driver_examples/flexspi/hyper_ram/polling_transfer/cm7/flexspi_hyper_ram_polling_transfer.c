/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_flexspi.h"
#include "fsl_debug_console.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_common.h"
#include "fsl_cache.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_FLEXSPI                 FLEXSPI2
#define EXAMPLE_FLEXSPI_AMBA_BASE       FlexSPI2_AMBA_BASE
#define EXAMPLE_FLEXSPI_PORT            kFLEXSPI_PortA1
#define EXAMPLE_FLEXSPI_RX_SAMPLE_CLOCK kFLEXSPI_ReadSampleClkExternalInputFromDqsPad

#define HYPERRAM_CMD_LUT_SEQ_IDX_READDATA  0
#define HYPERRAM_CMD_LUT_SEQ_IDX_WRITEDATA 1
#define HYPERRAM_CMD_LUT_SEQ_IDX_READREG   2
#define HYPERRAM_CMD_LUT_SEQ_IDX_WRITEREG  3

#define CUSTOM_LUT_LENGTH               20
#define EXAMPLE_COMBINATION_MODE_ENABLE 1

#define HYRAM_SIZE 0x10000 /* 64Mb/KByte. */
#define TEST_SIZE  0x80000U /* hyperram test size range in demo. */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern flexspi_device_config_t deviceconfig;
extern const uint32_t customLUT[CUSTOM_LUT_LENGTH];
static uint8_t s_hyper_ram_write_buffer[1024];
static uint8_t s_hyper_ram_read_buffer[1024];

/*******************************************************************************
 * Code
 ******************************************************************************/
flexspi_device_config_t deviceconfig = {
    .flexspiRootClk       = 100000000, /* 100MHZ SPI serial clock */
    .isSck2Enabled        = false,
    .flashSize            = HYRAM_SIZE,
    .CSIntervalUnit       = kFLEXSPI_CsIntervalUnit1SckCycle,
    .CSInterval           = 2,
    .CSHoldTime           = 0,
    .CSSetupTime          = 1,
    .dataValidTime        = 1,
    .columnspace          = 3,
    .enableWordAddress    = true,
    .AWRSeqIndex          = HYPERRAM_CMD_LUT_SEQ_IDX_WRITEDATA,
    .AWRSeqNumber         = 1,
    .ARDSeqIndex          = HYPERRAM_CMD_LUT_SEQ_IDX_READDATA,
    .ARDSeqNumber         = 1,
    .AHBWriteWaitUnit     = kFLEXSPI_AhbWriteWaitUnit2AhbCycle,
    .AHBWriteWaitInterval = 0,
    .enableWriteMask      = true,
};

const uint32_t customLUT[CUSTOM_LUT_LENGTH] = {
    /* Read Data */
    [4 * HYPERRAM_CMD_LUT_SEQ_IDX_READDATA] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_DDR, kFLEXSPI_8PAD, 0xA0, kFLEXSPI_Command_RADDR_DDR, kFLEXSPI_8PAD, 0x18),
    [4 * HYPERRAM_CMD_LUT_SEQ_IDX_READDATA + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_CADDR_DDR, kFLEXSPI_8PAD, 0x10, kFLEXSPI_Command_DUMMY_RWDS_DDR, kFLEXSPI_8PAD, 0x07),
    [4 * HYPERRAM_CMD_LUT_SEQ_IDX_READDATA + 2] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_DDR, kFLEXSPI_8PAD, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0x00),

    /* Write Data */
    [4 * HYPERRAM_CMD_LUT_SEQ_IDX_WRITEDATA] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_DDR, kFLEXSPI_8PAD, 0x20, kFLEXSPI_Command_RADDR_DDR, kFLEXSPI_8PAD, 0x18),
    [4 * HYPERRAM_CMD_LUT_SEQ_IDX_WRITEDATA + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_CADDR_DDR, kFLEXSPI_8PAD, 0x10, kFLEXSPI_Command_DUMMY_RWDS_DDR, kFLEXSPI_8PAD, 0x07),
    [4 * HYPERRAM_CMD_LUT_SEQ_IDX_WRITEDATA + 2] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_WRITE_DDR, kFLEXSPI_8PAD, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0x00),

    /* Read Register */
    [4 * HYPERRAM_CMD_LUT_SEQ_IDX_READREG] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_DDR, kFLEXSPI_8PAD, 0xE0, kFLEXSPI_Command_RADDR_DDR, kFLEXSPI_8PAD, 0x18),
    [4 * HYPERRAM_CMD_LUT_SEQ_IDX_READREG + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_CADDR_DDR, kFLEXSPI_8PAD, 0x10, kFLEXSPI_Command_DUMMY_RWDS_DDR, kFLEXSPI_8PAD, 0x07),
    [4 * HYPERRAM_CMD_LUT_SEQ_IDX_READREG + 2] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_DDR, kFLEXSPI_8PAD, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0x00),

    /* Write Register */
    [4 * HYPERRAM_CMD_LUT_SEQ_IDX_WRITEREG] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_DDR, kFLEXSPI_8PAD, 0x60, kFLEXSPI_Command_RADDR_DDR, kFLEXSPI_8PAD, 0x18),
    [4 * HYPERRAM_CMD_LUT_SEQ_IDX_WRITEREG + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_CADDR_DDR, kFLEXSPI_8PAD, 0x10, kFLEXSPI_Command_DUMMY_RWDS_DDR, kFLEXSPI_8PAD, 0x07),
    [4 * HYPERRAM_CMD_LUT_SEQ_IDX_WRITEREG + 2] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_WRITE_DDR, kFLEXSPI_8PAD, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0x00),
};




status_t flexspi_hyper_ram_ipcommand_write_data(FLEXSPI_Type *base, uint32_t address, uint32_t *buffer, uint32_t length)
{
    flexspi_transfer_t flashXfer;
    status_t status;

    /* Write data */
    flashXfer.deviceAddress = address;
    flashXfer.port          = EXAMPLE_FLEXSPI_PORT;
    flashXfer.cmdType       = kFLEXSPI_Write;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = HYPERRAM_CMD_LUT_SEQ_IDX_WRITEDATA;
    flashXfer.data          = buffer;
    flashXfer.dataSize      = length;

    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    return status;
}

void flexspi_hyper_ram_ahbcommand_write_data(FLEXSPI_Type *base, uint32_t address, uint32_t *buffer, uint32_t length)
{
    uint32_t *startAddr = (uint32_t *)(EXAMPLE_FLEXSPI_AMBA_BASE + address);
    memcpy(startAddr, buffer, length);

    /* Delay some time for tx buffer data finished. */
    uint32_t i = 1000;
    while (i--)
    {
        __NOP();
    }
}

status_t flexspi_hyper_ram_ipcommand_read_data(FLEXSPI_Type *base, uint32_t address, uint32_t *buffer, uint32_t length)
{
    flexspi_transfer_t flashXfer;
    status_t status;

    /* Write data */
    flashXfer.deviceAddress = address;
    flashXfer.port          = EXAMPLE_FLEXSPI_PORT;
    flashXfer.cmdType       = kFLEXSPI_Read;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = HYPERRAM_CMD_LUT_SEQ_IDX_READDATA;
    flashXfer.data          = buffer;
    flashXfer.dataSize      = length;

    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    return status;
}

void flexspi_hyper_ram_ahbcommand_read_data(FLEXSPI_Type *base, uint32_t address, uint32_t *buffer, uint32_t length)
{
    uint32_t *startAddr = (uint32_t *)(EXAMPLE_FLEXSPI_AMBA_BASE + address);
    memcpy(buffer, startAddr, length);
}

status_t flexspi_hyper_ram_get_id(FLEXSPI_Type *base, uint8_t *vendorId)
{
    flexspi_transfer_t flashXfer;
    status_t status;
    uint32_t id;

    /* Write data */
    flashXfer.deviceAddress = 0x0U;
    flashXfer.port          = EXAMPLE_FLEXSPI_PORT;
    flashXfer.cmdType       = kFLEXSPI_Read;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = HYPERRAM_CMD_LUT_SEQ_IDX_READREG;
    flashXfer.data          = &id;
    flashXfer.dataSize      = 2;

    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    *vendorId = id & (0xFU);

    return status;
}

int main(void)
{
    uint32_t i = 0;
    flexspi_config_t config;
    status_t status;
    uint8_t vendorID = 0;
    /* To store custom's LUT table in local. */
    uint32_t tempLUT[CUSTOM_LUT_LENGTH] = {0x00U};

    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    CLOCK_SetRootClockMux(kCLOCK_Root_Flexspi2, 5);

    PRINTF("FLEXSPI HyperRAM example started!\r\n");

    /* Copy LUT information from flash region into RAM region, because LUT update maybe corrupt read sequence(LUT[0])
     * and load wrong LUT table from FLASH region. */
    memcpy(tempLUT, customLUT, sizeof(tempLUT));

    /* Get FLEXSPI default settings and configure the flexspi. */
    FLEXSPI_GetDefaultConfig(&config);

    /* Init FLEXSPI. */
    config.rxSampleClock               = EXAMPLE_FLEXSPI_RX_SAMPLE_CLOCK;
    config.enableSckBDiffOpt           = true;
    config.ahbConfig.enableAHBPrefetch = true;
#if defined(EXAMPLE_COMBINATION_MODE_ENABLE) && EXAMPLE_COMBINATION_MODE_ENABLE
    config.enableCombination           = true;
#endif
    FLEXSPI_Init(EXAMPLE_FLEXSPI, &config);

    /* Configure flash settings according to serial flash feature. */
    FLEXSPI_SetFlashConfig(EXAMPLE_FLEXSPI, &deviceconfig, EXAMPLE_FLEXSPI_PORT);

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(EXAMPLE_FLEXSPI, 0, tempLUT, CUSTOM_LUT_LENGTH);

    /* Do software reset. */
    FLEXSPI_SoftwareReset(EXAMPLE_FLEXSPI);

    /* Get vendor ID. */
    status = flexspi_hyper_ram_get_id(EXAMPLE_FLEXSPI, &vendorID);
    if (status != kStatus_Success)
    {
        return status;
    }
    PRINTF("Vendor ID: %d\r\n", vendorID);
    for (i = 0; i < sizeof(s_hyper_ram_write_buffer); i++)
    {
        s_hyper_ram_write_buffer[i] = i;
    }

    for (i = 0; i < TEST_SIZE; i += 2048)
    {
        flexspi_hyper_ram_ipcommand_write_data(EXAMPLE_FLEXSPI, i, (uint32_t *)s_hyper_ram_write_buffer,
                                               sizeof(s_hyper_ram_write_buffer));
        flexspi_hyper_ram_ipcommand_read_data(EXAMPLE_FLEXSPI, i, (uint32_t *)s_hyper_ram_read_buffer,
                                              sizeof(s_hyper_ram_read_buffer));

        if (memcmp(s_hyper_ram_read_buffer, s_hyper_ram_write_buffer, sizeof(s_hyper_ram_write_buffer)) != 0)
        {
            PRINTF("IP Command Read/Write data Failure at 0x%x - 0x%x!\r\n", i, i + 2047);
            return -1;
        }
    }

    PRINTF("IP Command Read/Write data succeed at 0x0 - 0x%x!\r\n", i);

    for (i = sizeof(s_hyper_ram_write_buffer); i > 0; i--)
    {
        s_hyper_ram_write_buffer[i] = i;
    }
    memset(s_hyper_ram_read_buffer, 0, sizeof(s_hyper_ram_read_buffer));

    for (i = 0; i < TEST_SIZE; i += 2048)
    {
        flexspi_hyper_ram_ahbcommand_write_data(EXAMPLE_FLEXSPI, i, (uint32_t *)s_hyper_ram_write_buffer,
                                                sizeof(s_hyper_ram_write_buffer));
        flexspi_hyper_ram_ahbcommand_read_data(EXAMPLE_FLEXSPI, i, (uint32_t *)s_hyper_ram_read_buffer,
                                               sizeof(s_hyper_ram_read_buffer));

        if (memcmp(s_hyper_ram_read_buffer, s_hyper_ram_write_buffer, sizeof(s_hyper_ram_write_buffer)) != 0)
        {
            PRINTF("AHB Command Read/Write data Failure at 0x%x - 0x%x!\r\n", i, i + 2047);
            return -1;
        }
    }

    PRINTF("AHB Command Read/Write data succeed at 0x0 - 0x%x!\r\n", i);

    while (1)
    {
    }
}
