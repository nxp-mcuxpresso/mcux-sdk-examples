/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "fsl_sd.h"
#include "fsl_debug_console.h"
#include "ff.h"
#include "diskio.h"
#include "fsl_sd_disk.h"
#include "pin_mux.h"
#include "board.h"
#include "sdmmc_config.h"
#include "mflash_drv.h"
#include "crypto_support.h"
#include "fsl_shell.h"
#include "flash_partitioning.h"
#include "mcuboot_app_support.h"
#include "version.h"

#include <stdbool.h>
#include "fsl_iocon.h"
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define SHELL_Printf     PRINTF
#define READ_BUFFER_SIZE MFLASH_PAGE_SIZE

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static shell_status_t shellCmd_ls(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t shellCmd_install(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t shellCmd_ota(shell_handle_t shellHandle, int32_t argc, char **argv);

/*******************************************************************************
 * Variables
 ******************************************************************************/

SHELL_COMMAND_DEFINE(ls, "\n\"ls\": List files on SD card in the root directory\n", shellCmd_ls, 0);

SHELL_COMMAND_DEFINE(install,
                     "\n\"install path\": Installs a candidate ota image at given path\n",
                     shellCmd_install,
                     1);

SHELL_COMMAND_DEFINE(ota,
                     "\n\"ota action\": Execute one of possible OTA actions\n"
                     "      status  - prints current OTA image status\n"
                     "      accept  - mark current primary image as permanent\n"
                     "      version - print primary image version\n"
                     "      reboot  - triggers software reset\n",
                     shellCmd_ota,
                     1);

SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
static shell_handle_t s_shellHandle;
extern serial_handle_t g_serialHandle;

static FATFS fileSystem;

/* @brief decription about the read/write buffer
 * The size of the read/write buffer should be a multiple of 512, since SDHC/SDXC card uses 512-byte fixed
 * block length and this driver example is enabled with a SDHC/SDXC card.If you are using a SDSC card, you
 * can define the block length by yourself if the card supports partial access.
 * The address of the read/write buffer should align to the specific DMA data buffer address align value if
 * DMA transfer is used, otherwise the buffer address is not important.
 * At the same time buffer address/size should be aligned to the cache line size if cache is supported.
 */

SDK_ALIGN(uint8_t fReadBuffer[READ_BUFFER_SIZE], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);

/*******************************************************************************
 * Code
 ******************************************************************************/

static void hexdump(const void *src, size_t size)
{
    const unsigned char *src8 = src;
    const int CNT             = 16;

    for (size_t i = 0; i < size; i++)
    {
        int n = i % CNT;
        if (n == 0)
            PRINTF("%08x  ", i);
        PRINTF("%02X ", src8[i]);
        if ((i && n == CNT - 1) || i + 1 == size)
        {
            int rem = CNT - 1 - n;
            for (int j = 0; j < rem; j++)
                PRINTF("   ");
            PRINTF("|");
            for (int j = n; j >= 0; j--)
                PUTCHAR(isprint(src8[i - j]) ? src8[i - j] : '.');
            PRINTF("|\n");
        }
    }
    PUTCHAR('\n');
}

static int flash_erase(uint32_t offset, size_t size)
{
    uint32_t endaddr = offset + size;
    status_t status;
    int sectorCnt = (size + MFLASH_SECTOR_SIZE - 1) / MFLASH_SECTOR_SIZE;

    PRINTF("Erasing %u sectors of flash from offset 0x%x\n", sectorCnt, offset);

    /* All sectors are of the same size */

    for (; offset < endaddr; offset += MFLASH_SECTOR_SIZE)
    {
        status = mflash_drv_sector_erase(offset);
        if (status != kStatus_Success)
            break;
    }

    return status;
}

static int flash_program(uint32_t offset, void *data, size_t size)
{
    /* mflash expects 4B aligned buffer */
    uint32_t pagebuf[MFLASH_PAGE_SIZE / sizeof(uint32_t)];
    uint8_t *pagebuf8 = (uint8_t *)pagebuf;
    uint8_t *data8    = data;
    status_t status;

    while (size > 0)
    {
        size_t chunk = (size > sizeof(pagebuf)) ? sizeof(pagebuf) : size;

        memcpy(pagebuf, data8, chunk);

        /* In case of non-page-aligned size pad with 0xff */
        if (chunk < sizeof(pagebuf))
        {
            memset(pagebuf8 + chunk, 0xff, sizeof(pagebuf) - chunk);
        }

        status = mflash_drv_page_program(offset, pagebuf);
        if (status != kStatus_Success)
        {
            return status;
        }

        offset += chunk;
        data8 += chunk;
        size -= chunk;
    }

    return kStatus_Success;
}


static shell_status_t shellCmd_ls(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    FRESULT ret;
    DIR dir;
    FILINFO finfo;

    ret = f_opendir(&dir, "/");
    if (ret)
    {
        PRINTF("Failed to open root directory, ret=%d\n", ret);
        return kStatus_SHELL_Error;
    }

    for (;;)
    {
        ret = f_readdir(&dir, &finfo);

        /* To the end. */
        if ((ret != FR_OK) || (finfo.fname[0] == '\0'))
        {
            break;
        }
        if (finfo.fname[0] == '.')
        {
            continue;
        }
        if (finfo.fattrib & AM_DIR)
        {
            PRINTF("DIR  ");
        }
        else
        {
            PRINTF("FILE ");
        }

        PRINTF("%s\n", finfo.fname);
    }

    return kStatus_SHELL_Success;
}

static shell_status_t shellCmd_install(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    FRESULT fRet;
    int ret;
    FIL f;
    const char *path = argv[1];
    size_t bytesRead = 0;
    unsigned char sha1[20], sha1Flash[sizeof(sha1)];
    sha1_ctx_t sha1Ctx;
    shell_status_t result = kStatus_SHELL_Success;

    ret = sha1_init(&sha1Ctx);
    if (ret != kStatus_Success)
    {
        PRINTF("Failed to init hash engine (ret=%d)\n", ret);
        return kStatus_SHELL_Error;
    }

    PRINTF("Installing OTA image from file %s\n", path);

    fRet = f_open(&f, path, FA_READ);
    if (fRet)
    {
        PRINTF("Failed to open file (ret=%d)\n", fRet);
        return kStatus_SHELL_Error;
    }

    PRINTF("Size of file %s is %u bytes\n", path, f_size(&f));

    /* Flash Erase */

    ret = flash_erase(BOOT_FLASH_CAND_APP, BOOT_FLASH_CAND_APP - BOOT_FLASH_ACT_APP);
    if (ret != kStatus_Success)
    {
        PRINTF("Failed to erase flash (ret=%d)\n", ret);
        result = kStatus_SHELL_Error;
        goto cleanup;
    }

    PRINTF("Programming flash...\n");

    /* Flash Program */

    while (bytesRead < f_size(&f))
    {
        uint32_t flashOffset = BOOT_FLASH_CAND_APP + bytesRead;
        size_t remains       = f_size(&f) - bytesRead;
        size_t cntToRead     = (remains < sizeof(fReadBuffer)) ? remains : sizeof(fReadBuffer);
        size_t cntRead;

        fRet = f_read(&f, fReadBuffer, cntToRead, &cntRead);
        if (fRet)
        {
            PRINTF("Failed to read file at offset %u (ret=%d)\n", bytesRead, fRet);
            result = kStatus_SHELL_Error;
            goto cleanup;
        }

        sha1_update(&sha1Ctx, fReadBuffer, cntRead);
        flash_program(flashOffset, fReadBuffer, cntRead);

        bytesRead += cntRead;
    }

    /* Verify */

    /* SHA1 of processed data */

    ret = sha1_finish(&sha1Ctx, sha1);
    if (ret != kStatus_Success)
    {
        PRINTF("Failed to get sha1 (ret=%d)\n", ret);
        result = kStatus_SHELL_Error;
        goto cleanup;
    }

    /* SHA1 of flashed data */

    ret = sha1_init(&sha1Ctx);
    if (ret != kStatus_Success)
    {
        PRINTF("Failed to init hash engine (ret=%d)\n", ret);
        result = kStatus_SHELL_Error;
        goto cleanup;
    }

    sha1_update(&sha1Ctx, (uint8_t *)BOOT_FLASH_CAND_APP, f_size(&f));
    ret = sha1_finish(&sha1Ctx, sha1Flash);
    if (ret != kStatus_Success)
    {
        PRINTF("Failed to get sha1 (ret=%d)\n", ret);
        result = kStatus_SHELL_Error;
        goto cleanup;
    }

    /* SHA1 compare */

    if (memcmp(sha1, sha1Flash, sizeof(sha1)) != 0)
    {
        PRINTF("Checksum mismatch. Failed to verify flashed data.\n", ret);

        PRINTF("SHA1 hexdump of processed file content:\n");
        hexdump(sha1, sizeof(sha1));

        PRINTF("SHA1 hexdump of flashed data:\n");
        hexdump(sha1Flash, sizeof(sha1Flash));

        result = kStatus_SHELL_Error;
        goto cleanup;
    }

    /* Verification of MCUBoot structures in the programmed image */

    ret = bl_verify_image((uint8_t *)BOOT_FLASH_CAND_APP, f_size(&f));
    if (ret == 0)
    {
        PRINTF("Failed to verify MCUBoot structures in the programmed image\n");
        result = kStatus_SHELL_Error;
        goto cleanup;
    }

    /* Mark downloaded image ready for test */

    ret = bl_update_image_state(kSwapType_ReadyForTest);
    if (ret != kStatus_Success)
    {
        PRINTF("Failed to mark OTA image as ReadyForTest (ret=%d)\n", ret);
        result = kStatus_SHELL_Error;
        goto cleanup;
    }

    PRINTF("OTA image was installed successfully.\n");

cleanup:
    f_close(&f);

    return result;
}

static shell_status_t shellCmd_ota(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    const char *action = argv[1];

    /* version */
    if (strcmp(action, "version") == 0)
    {
        PRINTF("%s\n", VERSION_STR);
    }

    /* status */
    else if (strcmp(action, "status") == 0)
    {
        uint32_t state;
        char *stateStr = NULL;

        if (bl_get_image_state(&state) != kStatus_Success)
        {
            PRINTF("Failed to read image status\n");
            return kStatus_SHELL_Error;
        }

        switch (state)
        {
            case kSwapType_None:
                stateStr = "None";
                break;
            case kSwapType_ReadyForTest:
                stateStr = "ReadyForTest";
                break;
            case kSwapType_Testing:
                stateStr = "Testing";
                break;
            case kSwapType_Permanent:
                stateStr = "Permanent";
                break;
            default:
                stateStr = "Unknown";
                break;
        }

        PRINTF("%s\n", stateStr);
    }

    /* accept */
    else if (strcmp(action, "accept") == 0)
    {
#if defined(MFLASH_PAGE_INTEGRITY_CHECKS) && MFLASH_PAGE_INTEGRITY_CHECKS
        /* Flash with ECC currenlty doesn't support MCUBoot's revert mechanism */
        PRINTF("Not supported\n");
#else
        int ret = bl_update_image_state(kSwapType_Permanent);
        if (ret != kStatus_Success)
        {
            PRINTF("Failed to mark current image as PERMANENT (ret=%d)\n", ret);
            return kStatus_SHELL_Error;
        }
#endif
    }

    /* reboot */
    else if (strcmp(action, "reboot") == 0)
    {
        PRINTF("System reset!\n");
        NVIC_SystemReset();
    }

    /* unrecognized */
    else
    {
        PRINTF("Unknown command\n");
        return kStatus_SHELL_Error;
    }

    return kStatus_SHELL_Success;
}

/*!
 * @brief Main function
 */
int main(void)
{
    int ret;
    FRESULT fret;
    const TCHAR fatfs_drive[] = {SDDISK + '0', ':', '/'};

    /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);
    CLOCK_EnableClock(kCLOCK_InputMux);
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockFROHF96M();
    BOARD_InitDebugConsole();
    BOARD_SD_Config(&g_sd, NULL, BOARD_SDMMC_SD_HOST_IRQ_PRIORITY, NULL);
    mflash_drv_init();

    PRINTF("\nOTA example with SD card and FATFS.\n");

    if (SD_HostInit(&g_sd) != kStatus_Success)
    {
        PRINTF("SD host init failed\n");
        goto failed_init;
    }

    /* Check SD card is inserted */

    if (SD_IsCardPresent(&g_sd))
    {
        PRINTF("SD card recognized.\n");
    }
    else
    {
        PRINTF("Waiting for SD card to be inserted...\n");

        if (SD_PollingCardInsert(&g_sd, kSD_Inserted) == kStatus_Success)
        {
            PRINTF("Card inserted\n");
            /* power off card */
            SD_SetCardPower(&g_sd, false);
            /* power on the card */
            SD_SetCardPower(&g_sd, true);
        }
        else
        {
            PRINTF("Card detect fail.\n");
            return kStatus_Fail;
        }
    }

    /* Mount the drive and select it as current working directory */

    if (f_mount(&fileSystem, fatfs_drive, 1))
    {
        PRINTF("Failed to mount drive.\n");
        goto failed_init;
    }

    fret = f_chdrive(fatfs_drive);
    if (fret)
    {
        PRINTF("Failed to select drive.\n");
        goto failed_init;
    }

    /* Shell init */

    s_shellHandle = &s_shellHandleBuffer[0];

    ret = SHELL_Init(s_shellHandle, g_serialHandle, "SHELL>> ");
    if (ret != kStatus_SHELL_Success)
    {
        PRINTF("Failed to init shell.\n");
        goto failed_init;
    }

    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(ls));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(install));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(ota));

    while (1)
    {
#if !(defined(SHELL_NON_BLOCKING_MODE) && (SHELL_NON_BLOCKING_MODE > 0U))
        SHELL_Task(s_shellHandle);
#endif
    }

failed_init:
    while (1)
    {
    }
}
