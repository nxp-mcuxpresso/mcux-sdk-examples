/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board_setup.h"
#include "fsl_debug_console.h"
#include "fx_levelx_driver.h"

#include "fx_api.h"
#include "lx_api.h"

#define DEMO_STACK_SIZE         (1024 * 4)

#ifndef DEMO_DISK_SIZE_KB
#define DEMO_DISK_SIZE_KB       (512UL)       /* disk size is 512 KB */
#endif

#ifndef DEMO_DISK_FLASH_OFFSET
#define DEMO_DISK_FLASH_OFFSET      (0UL)
#endif

#define DEMO_DISK_SIZE          (DEMO_DISK_SIZE_KB * 1024UL)
#define DEMO_SECTOR_SIZE        (LX_NOR_SECTOR_SIZE_BYTES)
#define DEMO_SECTOR_COUNT       (DEMO_DISK_SIZE / DEMO_SECTOR_SIZE)

#define DEMO_SECTORS_PER_CLUSTER    (8UL)

#define LX_NOR_SECTOR_SIZE_BYTES    (LX_NOR_SECTOR_SIZE * sizeof(ULONG))
#define LX_NOR_SECTOR_PER_BLOCK     (16UL)
#define LX_NOR_BLOCK_SIZE           (LX_NOR_SECTOR_SIZE_BYTES * LX_NOR_SECTOR_PER_BLOCK)
#define LX_NOR_BLOCK_COUNT          (DEMO_DISK_SIZE / LX_NOR_BLOCK_SIZE)

fx_levelx_disk_infor_t g_disk_infor = {
    .disk_size = DEMO_DISK_SIZE,
    .disk_flash_offset = DEMO_DISK_FLASH_OFFSET,
    .levelx_block_size = LX_NOR_BLOCK_SIZE
};

static FX_MEDIA flash_disk;

static TX_THREAD thread_main;
static TX_THREAD thread_test;

static CHAR demo_stack_main[DEMO_STACK_SIZE];
static CHAR demo_stack_test[DEMO_STACK_SIZE];

static ULONG media_memory[LX_NOR_SECTOR_SIZE * 4];

extern VOID _fx_levelx_driver(FX_MEDIA *media_ptr);

extern status_t erase_flash_disk(uint32_t offset, uint32_t disk_size);

static VOID thread_main_entry(ULONG thread_input)
{
    ULONG real_sector_count;
    status_t ret;
    UINT status;

    PRINTF("Erase Flash: offset = 0x%x, size = %d KB\r\n",
                DEMO_DISK_FLASH_OFFSET, DEMO_DISK_SIZE_KB);

    /* erase flash if necessary */
    ret = erase_flash_disk(DEMO_DISK_FLASH_OFFSET, DEMO_DISK_SIZE);
    if (ret != kStatus_Success)
    {
        PRINTF("ERR: failed to erase SPI flash.\r\n");
        goto err;
    }

    /*
     * Removing the sectors used by LevelX to get the real total sector number.
     * If 16 sectors per block in this example, LevelX used one sector for each block.
     */
    real_sector_count = DEMO_SECTOR_COUNT - LX_NOR_BLOCK_COUNT;

    PRINTF("Fromat: disk_size = %d KB\r\n\r\n", real_sector_count * DEMO_SECTOR_SIZE / 1024);

    status = fx_media_format(&flash_disk, _fx_levelx_driver, (VOID *)NULL,
                             (UCHAR *)media_memory, sizeof(media_memory), "FLASH_DISK",
                             2, 32, 0, real_sector_count, DEMO_SECTOR_SIZE,
                             DEMO_SECTORS_PER_CLUSTER, 1, 1);
    if (status != FX_SUCCESS)
    {
        PRINTF("ERR: Formatting SPI Flash failed. 0x%x\r\n", status);
        goto err;
    }

    status = fx_media_open(&flash_disk, "FLASH_DISK", _fx_levelx_driver,
                           (VOID *)NULL, media_memory, sizeof(media_memory));
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
        /* delete the file */
        status = fx_file_delete(&flash_disk, fname);
        if (status != FX_SUCCESS && status != FX_NOT_FOUND)
        {
            /* Error deleting the file, break the loop.  */
            PRINTF("ERR: fx_file_delete(), 0x%x\r\n", status);
            break;
        }

        PRINTF("Creat %s\r\n", fname);
        /* Create a file in the root directory.  */
        status = fx_file_create(&flash_disk, fname);
        if (status != FX_SUCCESS)
        {
            /* Create error, break the loop.  */
            PRINTF("ERR: fx_file_create(), 0x%x\r\n", status);
            break;
        }

        PRINTF("Open %s\r\n", fname);
        /* Open the test file.  */
        status = fx_file_open(&flash_disk, &my_file, fname, FX_OPEN_FOR_WRITE);
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

        /* Close the test file.  */
        status = fx_file_close(&my_file);
        if (status != FX_SUCCESS)
        {
            /* Error closing the file, break the loop.  */
            PRINTF("ERR: fx_file_close(), 0x%x\r\n", status);
            break;
        }

        status = fx_media_flush(&flash_disk);
        if (status != FX_SUCCESS)
        {
            /* Error flushing the media, break the loop.  */
            PRINTF("ERR: fx_media_flush() failed, 0x%x\r\n", status);
            break;
        }

        status = fx_file_open(&flash_disk, &my_file, fname, FX_OPEN_FOR_READ);
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

        status = fx_media_flush(&flash_disk);
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
    fx_media_close(&flash_disk);

    return;
}

void tx_application_define(void *first_unused_memory)
{
    UINT status;

    TX_THREAD_NOT_USED(first_unused_memory);

    /* Initialize LevelX. */
    lx_nor_flash_initialize();

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

#ifdef LX_USE_MFLASH
    PRINTF("\r\nStart FileX LevelX mflash example\r\n");
#else
    PRINTF("\r\nStart FileX LevelX SPI Flash example\r\n");
#endif

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();

    return 0;
}
