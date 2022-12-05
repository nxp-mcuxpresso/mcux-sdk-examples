/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "fsl_mmc.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "sdmmc_config.h"
#include <stdbool.h>
#include "fsl_iocon.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Data block count accessed in card */
#define DATA_BLOCK_COUNT (5U)
/*! @brief Start data block number accessed in card */
#define DATA_BLOCK_START (2U)
/*! @brief The first group to erase */
#define ERASE_GROUP_START (0U)
/*! @brief The last group to erase */
#define ERASE_GROUP_END (0U)
/*! @brief Data buffer size */
#define DATA_BUFFER_SIZE (FSL_SDMMC_DEFAULT_BLOCK_SIZE * DATA_BLOCK_COUNT)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief printf the card information log.
 *
 * @param card Card descriptor.
 */
static void CardInformationLog(mmc_card_t *card);
/*******************************************************************************
 * Variables
 ******************************************************************************/
/*! @brief Card descriptor. */
mmc_card_t g_mmc;

/* @brief decription about the read/write buffer
 * The size of the read/write buffer in this driver example is multiple of 512, since DDR mode support 512-byte
 * block size only and our middleware switch the timing mode automatically per device capability. You can define the
 * buffer size to meet your requirement.If the card support partial access, you can also re-define the block size.
 * The address of the read/write buffer should align to the specific DMA data buffer address align value if
 * DMA transfer is used, otherwise the buffer address is not important.
 * At the same time buffer address/size should be aligned to the cache line size if cache is support.
 */
/*! @brief Data written to the card */
SDK_ALIGN(uint8_t g_dataWrite[DATA_BUFFER_SIZE], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);
/*! @brief Data read from the card */
SDK_ALIGN(uint8_t g_dataRead[DATA_BUFFER_SIZE], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);

/*******************************************************************************
 * Code
 ******************************************************************************/

/*! @brief Main function */
int main(void)
{
    mmc_card_t *card = &g_mmc;
    bool isReadOnly;
    bool failedFlag = false;
    char ch         = '0';

    CLOCK_EnableClock(kCLOCK_InputMux);
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);
    BOARD_InitPins();
    BOARD_BootClockPLL220M();
    BOARD_InitDebugConsole();
    BOARD_MMC_Config(card, BOARD_SDMMC_MMC_HOST_IRQ_PRIORITY);

    PRINTF("\r\nMMCCARD interrupt example.\r\n");

    /* Init card. */
    if (MMC_Init(card))
    {
        PRINTF("\r\nMMC card init failed\r\n");
        return -1;
    }
    /* card information log */
    CardInformationLog(card);

    PRINTF("\r\nRead/write the user partition continuously until encounter error......\r\n");
    /* Check if card is readonly. */
    isReadOnly = MMC_CheckReadOnly(card);
    if (isReadOnly)
    {
        while (true)
        {
            if (failedFlag || (ch == 'q'))
            {
                break;
            }

            PRINTF("\r\nRead one data block......\r\n");
            if (kStatus_Success != MMC_ReadBlocks(card, g_dataRead, DATA_BLOCK_START, 1U))
            {
                PRINTF("Read one data block failed.\r\n");
                failedFlag = true;
                continue;
            }

            PRINTF("Read multiple data blocks......\r\n");
            if (kStatus_Success != MMC_ReadBlocks(card, g_dataRead, DATA_BLOCK_START, DATA_BLOCK_COUNT))
            {
                PRINTF("Read multiple data blocks failed.\r\n");
                failedFlag = true;
                continue;
            }

            PRINTF(
                "\r\nInput 'q' to quit read process.\
                \r\nInput other char to read data blocks again.\r\n");
            ch = GETCHAR();
            PUTCHAR(ch);
        }
    }
    else
    {
        memset(g_dataWrite, 1U, sizeof(g_dataWrite));

        while (true)
        {
            if (failedFlag || (ch == 'q'))
            {
                break;
            }

            PRINTF("\r\nWrite/read one data block......\r\n");
            if (kStatus_Success != MMC_WriteBlocks(card, g_dataWrite, DATA_BLOCK_START, 1U))
            {
                PRINTF("Write one data block failed.\r\n");
                failedFlag = true;
                continue;
            }

            memset(g_dataRead, 0U, sizeof(g_dataRead));
            if (kStatus_Success != MMC_ReadBlocks(card, g_dataRead, DATA_BLOCK_START, 1U))
            {
                PRINTF("Read one data block failed.\r\n");
                failedFlag = true;
                continue;
            }

            PRINTF("Compare the read/write content......\r\n");
            if (memcmp(g_dataRead, g_dataWrite, FSL_SDMMC_DEFAULT_BLOCK_SIZE))
            {
                PRINTF("The read/write content isn't consistent.\r\n");
                failedFlag = true;
                continue;
            }
            PRINTF("The read/write content is consistent.\r\n");

            PRINTF("Write/read multiple data blocks......\r\n");
            if (kStatus_Success != MMC_WriteBlocks(card, g_dataWrite, DATA_BLOCK_START, DATA_BLOCK_COUNT))
            {
                PRINTF("Write multiple data blocks failed.\r\n");
                failedFlag = true;
                continue;
            }

            memset(g_dataRead, 0U, sizeof(g_dataRead));
            if (kStatus_Success != MMC_ReadBlocks(card, g_dataRead, DATA_BLOCK_START, DATA_BLOCK_COUNT))
            {
                PRINTF("Read multiple data blocks failed.\r\n");
                failedFlag = true;
                continue;
            }

            PRINTF("Compare the read/write content......\r\n");
            if (memcmp(g_dataRead, g_dataWrite, FSL_SDMMC_DEFAULT_BLOCK_SIZE * DATA_BLOCK_COUNT))
            {
                PRINTF("The read/write content isn't consistent.\r\n");
                failedFlag = true;
                continue;
            }
            PRINTF("The read/write content is consistent.\r\n");

            PRINTF("Erase data groups......\r\n");
            if (kStatus_Success != MMC_EraseGroups(card, ERASE_GROUP_START, ERASE_GROUP_END))
            {
                PRINTF("\r\nErases blocks failed\r\n");
                failedFlag = true;
                continue;
            }

            PRINTF(
                "\r\nInput 'q' to quit read/write/erase process.\
                \r\nInput other char to read/write/erase data blocks again.\r\n");
            ch = GETCHAR();
            PUTCHAR(ch);
        }
    }
    PRINTF("\r\nThe example will not read/write data blocks again.\r\n");

    MMC_Deinit(card);

    while (true)
    {
    }
}

static void CardInformationLog(mmc_card_t *card)
{
    assert(card);

    PRINTF("\r\nCard user partition size %d * %d bytes\r\n", card->blockSize, card->userPartitionBlocks);
    PRINTF("\r\nWorking condition:\r\n");

    if (card->hostVoltageWindowVCC == kMMC_VoltageWindows270to360)
    {
        PRINTF("\r\n  Voltage: VCC - 2.7V ~ 3.3V");
    }
    else if (card->hostVoltageWindowVCC == kMMC_VoltageWindow170to195)
    {
        PRINTF("\r\n  Voltage: VCC - 1.7V ~ 1.95V");
    }
    if (card->hostVoltageWindowVCCQ == kMMC_VoltageWindows270to360)
    {
        PRINTF("  VCCQ - 2.7V ~ 3.3V\r\n");
    }
    else if (card->hostVoltageWindowVCCQ == kMMC_VoltageWindow170to195)
    {
        PRINTF("  VCCQ - 1.7V ~ 1.95V\r\n");
    }
    else if (card->hostVoltageWindowVCCQ == kMMC_VoltageWindow120)
    {
        PRINTF("  VCCQ - 1.2V\r\n");
    }

    if (card->busTiming == kMMC_HighSpeedTimingNone)
    {
        PRINTF("\r\n  Timing mode: Default");
    }
    else if (card->busTiming == kMMC_HighSpeedTiming)
    {
        PRINTF("\r\n  Timing mode: High Speed\r\n");
    }
    else if (card->busTiming == kMMC_HighSpeed200Timing)
    {
        PRINTF("\r\n  Timing mode: HS200\r\n");
    }
    else if (card->busTiming == kMMC_HighSpeed400Timing)
    {
        PRINTF("\r\n  Timing mode: HS400\r\n");
    }

    if (card->busWidth == kMMC_DataBusWidth4bitDDR)
    {
        PRINTF("\r\n  Bus width: 4-bit DDR\r\n");
    }
    else if (card->busWidth == kMMC_DataBusWidth8bitDDR)
    {
        PRINTF("\r\n  Bus width: 8-bit DDR\r\n");
    }
    else if (card->busWidth == kMMC_DataBusWidth8bit)
    {
        PRINTF("\r\n  Bus width: 8-bit\r\n");
    }
    else if (card->busWidth == kMMC_DataBusWidth4bit)
    {
        PRINTF("\r\n  Bus width: 4-bit\r\n");
    }
    else if (card->busWidth == kMMC_DataBusWidth1bit)
    {
        PRINTF("\r\n  Bus width: 1-bit\r\n");
    }

    if ((card->busTiming == kMMC_HighSpeedTiming) &&
        ((card->busWidth == kMMC_DataBusWidth4bitDDR) || (card->busWidth == kMMC_DataBusWidth8bitDDR)))
    {
        PRINTF("\r\n  Freq : %d HZ\r\n", card->busClock_Hz / 2U);
    }
    else
    {
        PRINTF("\r\n  Freq : %d HZ\r\n", card->busClock_Hz);
    }
}
