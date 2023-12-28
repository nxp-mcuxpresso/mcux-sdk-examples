/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "board_setup.h"
#include "sdmmc_config.h"

#include "fx_api.h"


#define DEMO_STACK_SIZE    2048

#define DEMO_SECTOR_SIZE   512
#define DEMO_SECTOR_COUNT  (16 * 1024)
#define DEMO_RAM_DISK_SIZE (DEMO_SECTOR_SIZE * DEMO_SECTOR_COUNT)


static sd_card_t g_sdcard;

static FX_MEDIA sd_disk;
static TX_THREAD thread_main;
static TX_THREAD thread_test;

static CHAR demo_stack_main[DEMO_STACK_SIZE];
static CHAR demo_stack_test[DEMO_STACK_SIZE];
static UCHAR media_memory[DEMO_SECTOR_SIZE * 4];

extern VOID _fx_sdcard_driver(FX_MEDIA *media_ptr);


static void print_card_infor(sd_card_t *card)
{
    assert(card != NULL);

    PRINTF("Card size: %d MB\r\n",
           (card->blockCount / 1000) * card->blockSize / 1000);
    PRINTF("Card block size: %d bytes\r\n", card->blockSize);
    PRINTF("Card block count: %ld\r\n", card->blockCount);

    if (card->operationVoltage == kSDMMC_OperationVoltage330V)
    {
        PRINTF("Voltage: 3.3V\r\n");
    }
    else if (card->operationVoltage == kSDMMC_OperationVoltage180V)
    {
        PRINTF("Voltage: 1.8V\r\n");
    }

    if (card->currentTiming == kSD_TimingSDR12DefaultMode)
    {
        if (card->operationVoltage == kSDMMC_OperationVoltage330V)
        {
            PRINTF("Timing mode: Default mode\r\n");
        }
        else if (card->operationVoltage == kSDMMC_OperationVoltage180V)
        {
            PRINTF("Timing mode: SDR12 mode\r\n");
        }
    }
    else if (card->currentTiming == kSD_TimingSDR25HighSpeedMode)
    {
        if (card->operationVoltage == kSDMMC_OperationVoltage180V)
        {
            PRINTF("Timing mode: SDR25\r\n");
        }
        else
        {
            PRINTF("Timing mode: High Speed\r\n");
        }
    }
    else if (card->currentTiming == kSD_TimingSDR50Mode)
    {
        PRINTF("Timing mode: SDR50\r\n");
    }
    else if (card->currentTiming == kSD_TimingSDR104Mode)
    {
        PRINTF("Timing mode: SDR104\r\n");
    }
    else if (card->currentTiming == kSD_TimingDDR50Mode)
    {
        PRINTF("Timing mode: DDR50\r\n");
    }
}

static VOID thread_main_entry(ULONG thread_input)
{
    sd_card_t *sdcard = &g_sdcard;
    UINT status;

    TX_THREAD_NOT_USED(thread_input);

    PRINTF("Please insert a SD card.\r\n");

    /* Detect SD card */
    if (SD_PollingCardInsert(sdcard, kSD_Inserted) != kStatus_Success)
    {
        PRINTF("ERR: SD card detection failed.\r\n");
        goto err;
    }

    PRINTF("SD card inserted.\r\n");

    /* power off the card */
    SD_SetCardPower(sdcard, false);
    /* power on the card */
    SD_SetCardPower(sdcard, true);

    if (SD_CheckReadOnly(sdcard))
    {
        PRINTF("ERR: The SD card is readonly. Can not do the test.\r\n");
        goto err;
    }

    /* only used part of sectors considering the format time */
    status = fx_media_format(&sd_disk, _fx_sdcard_driver, (VOID *)&g_sdcard,
                             media_memory, sizeof(media_memory), "SD_DISK",
                             2, 32, 0, DEMO_SECTOR_COUNT, DEMO_SECTOR_SIZE,
                             8, 1, 1);
    if (status != FX_SUCCESS)
    {
        PRINTF("ERR: Formatting SD card failed.\r\n");
        goto err;
    }

    print_card_infor(sdcard);

    PRINTF("Formatted SD Card\r\n");

    status = fx_media_open(&sd_disk, "SD DISK", _fx_sdcard_driver,
                           (VOID *)&g_sdcard, media_memory,
                           sizeof(media_memory));
    if (status != FX_SUCCESS)
    {
        PRINTF("ERR: fx_media_open(), 0x%x\r\n", status);
        goto err;
    }

    /* start the test thread */
    tx_thread_resume(&thread_test);
    return;

err:
    while (1)
    {
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
    }
}

static VOID thread_test_entry(ULONG thread_input)
{
    CHAR buffer[64];
    FX_FILE my_file;
    UINT status;
    ULONG actual;
    CHAR *fname = "TEST.TXT";
    bool continue_test = true;
    char *test_str = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\r\n";

    TX_THREAD_NOT_USED(thread_input);

    while (continue_test)
    {
        PRINTF("\r\n");

        /* delete the file */
        status = fx_file_delete(&sd_disk, fname);
        if (status != FX_SUCCESS && status != FX_NOT_FOUND)
        {
            /* Error deleting the file, break the loop.  */
            PRINTF("ERR: fx_file_delete(), 0x%x\r\n", status);
            break;
        }

        PRINTF("Creat %s\r\n", fname);
        /* Create a file in the root directory.  */
        status = fx_file_create(&sd_disk, fname);
        if (status != FX_SUCCESS)
        {
            /* Create error, break the loop.  */
            PRINTF("ERR: fx_file_create(), 0x%x\r\n", status);
            break;
        }

        PRINTF("Open %s\r\n", fname);
        /* Open the test file.  */
        status = fx_file_open(&sd_disk, &my_file, fname, FX_OPEN_FOR_WRITE);
        if (status != FX_SUCCESS)
        {
            /* Error opening file, break the loop.  */
            PRINTF("ERR: fx_file_open(), 0x%x\r\n", status);
            break;
        }

        /* Seek to the beginning of the test file.  */
        status = fx_file_seek(&my_file, 0);
        if (status != FX_SUCCESS)
        {
            /* Error performing file seek, break the loop.  */
            PRINTF("ERR: fx_file_seek(), 0x%x\r\n", status);
            break;
        }

        PRINTF("Write %s\r\n", fname);
        /* Write a string to the test file.  */
        status = fx_file_write(&my_file, (VOID *)test_str, strlen(test_str));
        if (status != FX_SUCCESS)
        {
            /* Error writing to a file, break the loop.  */
            PRINTF("ERR: fx_file_write(), 0x%x\r\n", status);
            break;
        }

        /* Seek to the beginning of the test file.  */
        status = fx_file_seek(&my_file, 0);
        if (status != FX_SUCCESS)
        {
            /* Error performing file seek, break the loop.  */
            PRINTF("ERR: fx_file_seek(), 0x%x\r\n", status);
            break;
        }

        memset(buffer, 0, sizeof(buffer));

        PRINTF("Read %s\r\n", fname);
        /* Read the test file. */
        status = fx_file_read(&my_file, (VOID *)buffer, (ULONG)sizeof(buffer),
                              &actual);
        if ((status != FX_SUCCESS) || (actual != strlen(test_str)))
        {
            /* Error reading file, break the loop.  */
            PRINTF("ERR: fx_file_read(), 0x%x\r\n", status);
            break;
        }

        PRINTF("Close %s\r\n", fname);
        /* Close the test file.  */
        status = fx_file_close(&my_file);
        if (status != FX_SUCCESS)
        {
            /* Error closing the file, break the loop.  */
            PRINTF("ERR: fx_file_close(), 0x%x\r\n", status);
            break;
        }

        status = fx_media_flush(&sd_disk);
        if (status != FX_SUCCESS)
        {
            /* Error flushing the media, break the loop.  */
            PRINTF("ERR: fx_media_flush() failed, 0x%x\r\n", status);
            break;
        }

        PRINTF("\r\nContinue the test (y/n): ");
        while (1)
        {
            int input = GETCHAR();

            if (input == 'y' || input == 'Y')
            {
                break;
            }

            if (input == 'n' || input == 'N')
            {
                continue_test = false;
                break;
            }
        }
        PRINTF("\r\n");
    }

    /* Close the media.  */
    fx_media_close(&sd_disk);

    PRINTF("The SD card can be removed now.\r\n");

    return;
}

void tx_application_define(void *first_unused_memory)
{
    sd_card_t *sdcard = &g_sdcard;
    UINT status;

    TX_THREAD_NOT_USED(first_unused_memory);

    BOARD_SD_Config(sdcard, NULL, BOARD_SDMMC_SD_HOST_IRQ_PRIORITY, NULL);

    /* Init SD controller */
    if (SD_HostInit(sdcard) != kStatus_Success)
    {
        PRINTF("ERR: SD host init failed.\r\n");
        goto err;
    }

    /* Initialize FileX.  */
    fx_system_initialize();

    /* Create the main thread.  */
    status = tx_thread_create(&thread_main, "main thread",
                              thread_main_entry, 0,
                              (VOID *)demo_stack_main, DEMO_STACK_SIZE,
                              10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS)
    {
        PRINTF("ERR: create the main thread, 0x%x\r\n", status);
        goto err;
    }

    /* Create the test thread, but DO NOT start it. */
    status = tx_thread_create(&thread_test, "test thread",
                              thread_test_entry, 0,
                              (VOID *)demo_stack_test, DEMO_STACK_SIZE,
                              10, 10, TX_NO_TIME_SLICE, TX_DONT_START);
    if (status != TX_SUCCESS)
    {
        PRINTF("ERR: create the test thread, 0x%x\r\n", status);
        goto err;
    }

    return;

err:
    while (1)
    {
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
    }
}

int main(void)
{
    board_setup();

    PRINTF("\r\nStart FileX SD Card example\r\n");

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();

    return 0;
}
