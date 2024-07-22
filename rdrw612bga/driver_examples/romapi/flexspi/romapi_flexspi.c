/*
 * Copyright 2022,2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_romapi_flexspi.h"

#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*
 * @brief Helper to assert function return status.
 *
 * @details Print finshed message and trap forever upon failure.
 */
#define APP_ASSERT(expected, actual, ...) \
    do                                    \
    {                                     \
        if (expected != actual)           \
        {                                 \
            PRINTF("Example failed: ");   \
            PRINTF(__VA_ARGS__);          \
            while (1)                     \
                ;                         \
        }                                 \
    } while (0);

#define SYSTEM_IS_XIP_FLEXSPI()                                                                               \
    ((((uint32_t)SystemCoreClockUpdate >= 0x08000000U) && ((uint32_t)SystemCoreClockUpdate < 0x10000000U)) || \
     (((uint32_t)SystemCoreClockUpdate >= 0x18000000U) && ((uint32_t)SystemCoreClockUpdate < 0x20000000U)))

#define FLEXSPI_CFG_BLK_TAG (0x42464346UL) // ascii "FCFB" Big Endian

#define CMD_SDR        0x01
#define CMD_DDR        0x21
#define RADDR_SDR      0x02
#define RADDR_DDR      0x22
#define CADDR_SDR      0x03
#define CADDR_DDR      0x23
#define MODE1_SDR      0x04
#define MODE1_DDR      0x24
#define MODE2_SDR      0x05
#define MODE2_DDR      0x25
#define MODE4_SDR      0x06
#define MODE4_DDR      0x26
#define MODE8_SDR      0x07
#define MODE8_DDR      0x27
#define WRITE_SDR      0x08
#define WRITE_DDR      0x28
#define READ_SDR       0x09
#define READ_DDR       0x29
#define LEARN_SDR      0x0A
#define LEARN_DDR      0x2A
#define DATSZ_SDR      0x0B
#define DATSZ_DDR      0x2B
#define DUMMY_SDR      0x0C
#define DUMMY_DDR      0x2C
#define DUMMY_RWDS_SDR 0x0D
#define DUMMY_RWDS_DDR 0x2D
#define JMP_ON_CS      0x1F
#define STOP_EXE       0

#define FLEXSPI_1PAD 0
#define FLEXSPI_2PAD 1
#define FLEXSPI_4PAD 2
#define FLEXSPI_8PAD 3

#define FLEXSPI_LUT_SEQ(cmd0, pad0, op0, cmd1, pad1, op1)                                                              \
    (FLEXSPI_LUT_OPERAND0(op0) | FLEXSPI_LUT_NUM_PADS0(pad0) | FLEXSPI_LUT_OPCODE0(cmd0) | FLEXSPI_LUT_OPERAND1(op1) | \
     FLEXSPI_LUT_NUM_PADS1(pad1) | FLEXSPI_LUT_OPCODE1(cmd1))

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/


flexspi_nor_config_t default_dummy_cycle_config = {
    .memConfig =
        {
            .tag                 = FLEXSPI_CFG_BLK_TAG,
            .version             = 0,
            .readSampleClkSrc    = 1,
            .csHoldTime          = 3,
            .csSetupTime         = 3,
            .deviceModeCfgEnable = 1,
            .deviceModeSeq       = {.seqNum = 1, .seqId = 2},
            .deviceModeArg       = 0x0740,
            .configCmdEnable     = 0,
            .deviceType          = 0x1,
            .sflashPadType       = 4,
            .serialClkFreq       = 4,
            .sflashA1Size        = 0x4000000U,
            .sflashA2Size        = 0,
            .sflashB1Size        = 0,
            .sflashB2Size        = 0,
            .lookupTable =
                {
                    /* Read */
                    [0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0xEB, RADDR_SDR, FLEXSPI_4PAD, 0x18),
                    [1] = FLEXSPI_LUT_SEQ(DUMMY_SDR, FLEXSPI_4PAD, 0x06, READ_SDR, FLEXSPI_4PAD, 0x04),

                    /* Read Status */
                    [4 * 1 + 0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0x05, READ_SDR, FLEXSPI_1PAD, 0x04),

                    /* Write Status */
                    [4 * 2 + 0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0x01, WRITE_SDR, FLEXSPI_1PAD, 0x02),

                    /* Write Enable */
                    [4 * 3 + 0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0x06, STOP_EXE, FLEXSPI_1PAD, 0x00),

                    /* Sector erase */
                    [4 * 5 + 0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0x20, RADDR_SDR, FLEXSPI_1PAD, 0x18),

                    /* Block erase */
                    [4 * 8 + 0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0x52, RADDR_SDR, FLEXSPI_1PAD, 0x18),

                    /* Page program */
                    [4 * 9 + 0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0x02, RADDR_SDR, FLEXSPI_1PAD, 0x18),
                    [4 * 9 + 1] = FLEXSPI_LUT_SEQ(WRITE_SDR, FLEXSPI_1PAD, 0x00, STOP_EXE, FLEXSPI_1PAD, 0x00),

                    /* chip erase */
                    [4 * 11 + 0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0x60, STOP_EXE, FLEXSPI_1PAD, 0x00),
                },
        },
    .pageSize           = 0x100,
    .sectorSize         = 0x1000,
    .ipcmdSerialClkFreq = 0,
    .blockSize          = 0x8000,
};

/*******************************************************************************
 * Code
 ******************************************************************************/
void run_examples()
{
    uint32_t status = kStatus_Fail;
#define FLEXSPI_INSTANCE      (0)
#define FLASH_OPTION_QSPI_SDR 0xc0000004

    /**********************************************************************************************************************
     * API: flexspi_nor_set_clock_source
     *********************************************************************************************************************/
    uint32_t flexspi_clcok_source = 0x0;
    uint32_t flexspiNorTotalSize;
    uint32_t flexspiNorSectorSize;
    uint32_t flexspiNorPageSize;
    status = flexspi_nor_set_clock_source(flexspi_clcok_source);
    APP_ASSERT(kStatus_Success, status, "flexspi_nor_set_clock_source returned with code [%d]\r\n", status);

    /**********************************************************************************************************************
     * API: flexspi_clock_config
     *********************************************************************************************************************/
    uint32_t flexspi_freqOption    = 0x1;
    uint32_t flexspi_sampleClkMode = 0x0;
    flexspi_clock_config(FLEXSPI_INSTANCE, flexspi_freqOption, flexspi_sampleClkMode);

    /**********************************************************************************************************************
     * API: flexspi_nor_get_config
     *********************************************************************************************************************/
    // In order to be able to sucessfully perform the SFDP read used internally by flexspi_nor_get_config, the Flash
    // chip has to be reconfigured to use the default amount of dummy cycles.
    status = flexspi_nor_flash_init(FLEXSPI_INSTANCE, &default_dummy_cycle_config);
    APP_ASSERT(kStatus_Success, status, "flexspi_nor_flash_init returned with code [%d]\r\n", status);

    flexspi_nor_config_t flashConfig             = {0};
    serial_nor_config_option_t flashConfigOption = {.option0 = {.U = FLASH_OPTION_QSPI_SDR}};
    // get the valid flashConfig data here
    status = flexspi_nor_get_config(FLEXSPI_INSTANCE, &flashConfig, &flashConfigOption);
    APP_ASSERT(kStatus_Success, status, "flexspi_nor_get_config returned with code [%d]\r\n", status);

    /**********************************************************************************************************************
     * API: flexspi_nor_flash_init
     *********************************************************************************************************************/
    // Call the API with proper flashConfig parameter
    status = flexspi_nor_flash_init(FLEXSPI_INSTANCE, &flashConfig);
    APP_ASSERT(kStatus_Success, status, "flexspi_nor_flash_init returned with code [%d]\r\n", status);

    flexspiNorTotalSize  = flashConfig.memConfig.sflashA1Size;
    flexspiNorSectorSize = flashConfig.sectorSize;
    flexspiNorPageSize   = flashConfig.pageSize;
    PRINTF("\r\n FlexSpi flash Information: ");
    PRINTF("\r\n FlexSpi flash size:\t%d KB, Hex: (0x%x)", (flexspiNorTotalSize / 1024U), flexspiNorTotalSize);
    PRINTF("\r\n FlexSpi flash sector size:\t%d KB, Hex: (0x%x) ", (flexspiNorSectorSize / 1024U),
           flexspiNorSectorSize);
    PRINTF("\r\n FlexSpi flash page size:\t%d B, Hex: (0x%x)\r\n", flexspiNorPageSize, flexspiNorPageSize);

    /**********************************************************************************************************************
     * API: flexspi_update_lut
     *********************************************************************************************************************/
#define NOR_CMD_LUT_FOR_IP_CMD 1 //!< 1 Dedicated LUT Sequence Index for IP Command
    uint32_t lut_entry_read_status[4] = {
        FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0x05, READ_SDR, FLEXSPI_1PAD, 0x04),
        0,
        0,
        0,
    };
    flexspi_xfer_t flashXfer = {.baseAddress = sizeof(flexspi_nor_config_t),
                                .seqId       = NOR_CMD_LUT_FOR_IP_CMD,
                                .seqNum      = 1,
                                .operation   = kFlexSpiOperation_Read};

    status = flexspi_update_lut(FLEXSPI_INSTANCE, NOR_CMD_LUT_FOR_IP_CMD, &lut_entry_read_status[0], flashXfer.seqNum);
    APP_ASSERT(kStatus_Success, status, "flexspi_update_lut returned with code [%d]\r\n", status);

    /**********************************************************************************************************************
     * API: flexspi_command_xfer
     *********************************************************************************************************************/
    status = flexspi_command_xfer(FLEXSPI_INSTANCE, &flashXfer);
    APP_ASSERT(kStatus_Success, status, "flexspi_command_xfer returned with code [%d]\r\n", status);

    /**********************************************************************************************************************
     * API: flexspi_nor_flash_read
     *********************************************************************************************************************/
    uint32_t start_offset     = 0x1000UL;
    uint32_t read_buffer[512] = {0};
    status = flexspi_nor_flash_read(FLEXSPI_INSTANCE, &flashConfig, (uint32_t *)&read_buffer, start_offset,
                                    sizeof(read_buffer));
    APP_ASSERT(kStatus_Success, status, "flexspi_nor_flash_read returned with code [%d]\r\n", status);

    /**********************************************************************************************************************
     * API: flexspi_nor_flash_erase_sector
     *********************************************************************************************************************/
    uint32_t address = 0x12000UL;
    status           = flexspi_nor_flash_erase_sector(FLEXSPI_INSTANCE, &flashConfig, address);
    APP_ASSERT(kStatus_Success, status, "flexspi_nor_flash_erase_sector returned with code [%d]\r\n", status);

    /**********************************************************************************************************************
     * API: flexspi_nor_flash_erase_block
     *********************************************************************************************************************/
    status = flexspi_nor_flash_erase_block(FLEXSPI_INSTANCE, &flashConfig, address);
    APP_ASSERT(kStatus_Success, status, "flexspi_nor_flash_erase_block returned with code [%d]\r\n", status);

    /**********************************************************************************************************************
     * API: flexspi_nor_flash_erase
     *********************************************************************************************************************/
    uint32_t offset        = 0x10000UL;
    uint32_t lengthInBytes = 0x1000;
    status                 = flexspi_nor_flash_erase(FLEXSPI_INSTANCE, &flashConfig, offset, lengthInBytes);
    APP_ASSERT(kStatus_Success, status, "flexspi_nor_flash_erase returned with code [%d]\r\n", status);

#if 0
/**********************************************************************************************************************
 * API: flexspi_nor_flash_erase_all
 *********************************************************************************************************************/
    status = flexspi_nor_flash_erase_all(FLEXSPI_INSTANCE, &flashConfig);
    APP_ASSERT(kStatus_Success, status, "flexspi_nor_flash_erase_all returned with code [%d]\r\n", status);
#endif

    /**********************************************************************************************************************
     * API: flexspi_nor_flash_page_program
     *********************************************************************************************************************/
    uint32_t dstAddr = 0x10000UL;
    uint8_t programBuffer[256];
    for (uint32_t i = 0; i < sizeof(programBuffer); i++)
    {
        programBuffer[i] = (uint8_t)(i & 0xFF);
    }
    status = flexspi_nor_flash_page_program(FLEXSPI_INSTANCE, &flashConfig, dstAddr, (const uint32_t *)&programBuffer,
                                            false);
    APP_ASSERT(kStatus_Success, status, "flexspi_nor_flash_page_program returned with code [%d]\r\n", status);

    /**********************************************************************************************************************
     * API: flexspi_nor_flash_partial_program
     *********************************************************************************************************************/
    uint32_t flashOffset = 0x10100UL;
    uint32_t progBuff[256 / sizeof(uint32_t)];
    uint8_t *buf_u8 = (uint8_t *)progBuff;
    for (uint32_t i = 0; i < sizeof(progBuff); i++)
    {
        *buf_u8++ = ~(uint8_t)(i & 0xFFU);
    }
    status = flexspi_nor_flash_partial_program(FLEXSPI_INSTANCE, &flashConfig, flashOffset, (const uint32_t *)&progBuff,
                                               sizeof(progBuff), false);
    APP_ASSERT(kStatus_Success, status, "flexspi_nor_flash_partial_program returned with code [%d]\r\n", status);
}

int main()
{
    /* Init hardware */
    BOARD_InitBootPins();

    if (BOARD_IS_XIP())
    {
        BOARD_BootClockLPR();
        CLOCK_EnableClock(kCLOCK_Otp);
        CLOCK_EnableClock(kCLOCK_Els);
        CLOCK_EnableClock(kCLOCK_ElsApb);
        RESET_PeripheralReset(kOTP_RST_SHIFT_RSTn);
        RESET_PeripheralReset(kELS_APB_RST_SHIFT_RSTn);
    }
    else
    {
        BOARD_InitBootClocks();
        CLOCK_EnableClock(kCLOCK_Flexspi);
        RESET_ClearPeripheralReset(kFLEXSPI_RST_SHIFT_RSTn);
        /* Use aux0_pll_clk / 2 */
        BOARD_SetFlexspiClock(FLEXSPI, 2U, 2U);
    }
    BOARD_InitDebugConsole();
    PRINTF("\r\nROM API FlexSPI Driver version [0x%08X]\r\n", flexspi_nor_flash_version());

    run_examples();

    PRINTF("Finished executing ROM API FlexSPI Driver examples\r\n");
    /* End of example */
    while (1)
    {
    }
}
