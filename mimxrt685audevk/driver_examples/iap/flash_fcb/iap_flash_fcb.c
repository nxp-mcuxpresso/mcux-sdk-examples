/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <string.h>
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "fsl_iap.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FLASH_CONFIG_BLOCK_TAG     (0x42464346)
#define FLASH_CONFIG_BLOCK_VERSION (0x56010400)

#define CMD_SDR        0x01
#define CMD_DDR        0x21
#define RADDR_SDR      0x02
#define RADDR_DDR      0x22
#define MODE8_SDR      0x07
#define WRITE_SDR      0x08
#define WRITE_DDR      0x28
#define READ_SDR       0x09
#define READ_DDR       0x29
#define DUMMY_SDR      0x0C
#define DUMMY_DDR      0x2C
#define DUMMY_RWDS_SDR 0x0D
#define DUMMY_RWDS_DDR 0x2D
#define STOP_EXE       0

#define FLEXSPI_1PAD 0
#define FLEXSPI_2PAD 1
#define FLEXSPI_4PAD 2
#define FLEXSPI_8PAD 3

#define FLEXSPI_LUT_SEQ(cmd0, pad0, op0, cmd1, pad1, op1)                                                              \
    (FLEXSPI_LUT_OPERAND0(op0) | FLEXSPI_LUT_NUM_PADS0(pad0) | FLEXSPI_LUT_OPCODE0(cmd0) | FLEXSPI_LUT_OPERAND1(op1) | \
     FLEXSPI_LUT_NUM_PADS1(pad1) | FLEXSPI_LUT_OPCODE1(cmd1))
#define EXAMPLE_NOR_INSTANCE       0U        /* FLEXSPI */
#define NOR_FLASH_OP_START_ADDRESS 0x200000U /* Operation on 2MB offset */
#define NOR_FLASH_OP_SIZE          0x2000U   /* Test 8KB region */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
AT_QUICKACCESS_SECTION_CODE(status_t Demo_FlexspiNorInit(uint32_t instance, flexspi_nor_config_t *config));


/*******************************************************************************
 * Variables
 ******************************************************************************/
flexspi_nor_config_t config = {
    .memConfig =
        {
            .tag                  = FLASH_CONFIG_BLOCK_TAG,
            .version              = FLASH_CONFIG_BLOCK_VERSION,
            .csHoldTime           = 3,
            .csSetupTime          = 3,
            .configCmdEnable      = 0,
            .controllerMiscOption = 0x10U, /* Safe Configuration Frequency enable */
            .deviceType           = 0x1,
            .sflashPadType        = kSerialFlash_4Pads,
            .serialClkFreq        = 2,
            .sflashA1Size         = 0,
            .sflashA2Size         = 0,
            .sflashB1Size         = BOARD_FLASH_SIZE,
            .sflashB2Size         = 0,
            .lookupTable =
                {
                    /* Read */
                    [0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0xEC, RADDR_SDR, FLEXSPI_4PAD, 0x20),
                    [1] = FLEXSPI_LUT_SEQ(MODE8_SDR, FLEXSPI_4PAD, 0x00, DUMMY_SDR, FLEXSPI_4PAD, 0x04),
                    [2] = FLEXSPI_LUT_SEQ(READ_SDR, FLEXSPI_4PAD, 0x04, STOP_EXE, FLEXSPI_1PAD, 0x00),

                    /* Read Status */
                    [4 * 1 + 0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0x05, READ_SDR, FLEXSPI_1PAD, 0x04),

                    /* Write Enable */
                    [4 * 3 + 0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0x06, STOP_EXE, FLEXSPI_1PAD, 0x00),

                    /* Sector erase */
                    [4 * 5 + 0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0x21, RADDR_SDR, FLEXSPI_1PAD, 0x20),

                    /* block erase */
                    [4 * 8 + 0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0xDC, RADDR_SDR, FLEXSPI_1PAD, 0x20),

                    /* 4PP4B */
                    [4 * 9 + 0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0x3E, RADDR_SDR, FLEXSPI_4PAD, 0x20),
                    [4 * 9 + 1] = FLEXSPI_LUT_SEQ(WRITE_SDR, FLEXSPI_4PAD, 0x04, STOP_EXE, FLEXSPI_1PAD, 0x00),

                    /* chip erase */
                    [4 * 11 + 0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0x60, STOP_EXE, FLEXSPI_1PAD, 0x00),
                },
        },
    .pageSize           = 0x100,
    .sectorSize         = 0x1000,
    .ipcmdSerialClkFreq = 1,
    .blockSize          = 0x10000,
};
extern flexspi_nor_config_t config;

static uint32_t mem_writeBuffer[(NOR_FLASH_OP_SIZE + 3) / 4];
static uint32_t mem_readBuffer[(NOR_FLASH_OP_SIZE + 3) / 4];

/*******************************************************************************
 * Code
 ******************************************************************************/
AT_QUICKACCESS_SECTION_CODE(status_t Demo_FlexspiNorInit(uint32_t instance, flexspi_nor_config_t *config))
{
    /* Wait until the FLEXSPI is idle */
    register uint32_t delaycnt = 10000u;

    while ((delaycnt--) != 0U)
    {
    }
    status_t status = IAP_FlexspiNorInit(instance, config);

    return status;
}
/*!
 * @brief Main function
 */
int main(void)
{
    status_t status;

    uint32_t i;
    uint32_t pages, irqMask;
    uint8_t *pReadBuf  = (uint8_t *)mem_readBuffer;
    uint8_t *pWriteBuf = (uint8_t *)mem_writeBuffer;

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Switch flexspi clock to FFRO*/
    IAP_FlexspiSetClockSource(3);

    PRINTF("\r\nIAP flash example started!\r\n");

    do
    {
        status = Demo_FlexspiNorInit(EXAMPLE_NOR_INSTANCE, &config);

        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NOR Flash Initialization Failed!***\r\n");
            break;
        }
        PRINTF("\r\n***NOR Flash Initialization Success!***\r\n");

        /* Erase sectors */
        irqMask = DisableGlobalIRQ();
        status  = IAP_FlexspiNorErase(EXAMPLE_NOR_INSTANCE, &config, NOR_FLASH_OP_START_ADDRESS, NOR_FLASH_OP_SIZE);
        EnableGlobalIRQ(irqMask);
        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NOR Flash Erase Failed!***\r\n");
            break;
        }

        status = IAP_FlexspiNorRead(EXAMPLE_NOR_INSTANCE, &config, mem_readBuffer, NOR_FLASH_OP_START_ADDRESS,
                                    NOR_FLASH_OP_SIZE);
        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NOR Flash Read Failed!***\r\n");
            break;
        }

        for (i = 0; i < NOR_FLASH_OP_SIZE; i++)
        {
            if (pReadBuf[i] != 0xFF)
            {
                PRINTF("\r\n***NOR Flash Erase Check Failed!***\r\n");
                break;
            }
        }

        PRINTF("\r\n***NOR Flash Erase Success!***\r\n");

        /* Program the page data. */
        /* Initialize the write buffers. */
        for (i = 0; i < NOR_FLASH_OP_SIZE; i++)
        {
            pWriteBuf[i] = i & 0xFF;
        }

        pages   = NOR_FLASH_OP_SIZE / config.pageSize;
        irqMask = DisableGlobalIRQ();
        for (i = 0; i < pages; i++)
        {
            status = IAP_FlexspiNorPageProgram(EXAMPLE_NOR_INSTANCE, &config,
                                               NOR_FLASH_OP_START_ADDRESS + i * config.pageSize, mem_writeBuffer);
            if (status != kStatus_Success)
            {
                PRINTF("\r\n***NOR Flash Page %d Program Failed!***\r\n", i);
                break;
            }
        }
        EnableGlobalIRQ(irqMask);

        status = IAP_FlexspiNorRead(EXAMPLE_NOR_INSTANCE, &config, mem_readBuffer, NOR_FLASH_OP_START_ADDRESS,
                                    NOR_FLASH_OP_SIZE);
        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NOR Flash Read Failed!***\r\n");
            break;
        }

        if (memcmp(mem_writeBuffer, mem_readBuffer, NOR_FLASH_OP_SIZE) != 0)
        {
            PRINTF("\r\n***NOR Flash Page %d Read/Write Failed!***\r\n", i);
            break;
        }

        PRINTF("\r\n***NOR Flash All Pages Read/Write Success!***\r\n");
    } while (0);

    while (1)
    {
    }
}
