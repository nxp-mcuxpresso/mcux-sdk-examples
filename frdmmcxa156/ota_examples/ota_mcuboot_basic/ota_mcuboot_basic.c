/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_shell.h"
#include "pin_mux.h"
#include "board.h"

#include "mcuboot_app_support.h"
#include "mflash_drv.h"
#include "xmodem.h"
#include "platform_bindings.h"

#include <stdio.h>
#include <ctype.h>

#include "fsl_clock.h"
#include "fsl_reset.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static shell_status_t shellCmd_image(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t shellCmd_xmodem(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t shellCmd_mem(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t shellCmd_reboot(shell_handle_t shellHandle, int32_t argc, char **argv);


/*******************************************************************************
 * Variables
 ******************************************************************************/


static SHELL_COMMAND_DEFINE(image,
                            "\n\"image [info]\"          : Print image information"
                            "\n\"image test [imgNum]\"   : Mark secondary slot of given image number as ready for test"
                            "\n\"image accept [imgNum]\" : Mark primary slot of given image number as accepted"
                            "\n\"image erase [imgNum]\"  : Erase secondary slot of given image number"
                            "\n",
                            shellCmd_image,
                            SHELL_IGNORE_PARAMETER_COUNT);

static SHELL_COMMAND_DEFINE(mem,
                            "\n\"mem read addr [size]\" : Read memory at given address"
                            "\n\"mem erase addr \"      : Erase sector containing given address"
                            "\n",
                            shellCmd_mem,
                            SHELL_IGNORE_PARAMETER_COUNT);

static SHELL_COMMAND_DEFINE(xmodem, "\n\"xmodem\": Start receiving with XMODEM-CRC\n", shellCmd_xmodem, 0);

static SHELL_COMMAND_DEFINE(reboot, "\n\"reboot\": Triggers software reset\n", shellCmd_reboot, 0);

SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
static shell_handle_t s_shellHandle;

/*
 * Buffer used to handover data from XMODEM to flash programming routine.
 * Uses 4B alignment to be compatible with mflash.
 **/
static uint32_t progbuf[1024/sizeof(uint32_t)];

static hashctx_t sha256ctx;

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
            PRINTF("%08x  ", (uint32_t)src+i);
        PRINTF("%02X ", src8[i]);
        if ((i && n == CNT - 1) || (i + 1 == size))
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

static void print_hash(const void *src, size_t size)
{
    const unsigned char *src8 = src;
    for (size_t i = 0; i < size; i++)
    {
        PRINTF("%02X", src8[i]);
    }
}

static void print_image_info(void)
{
    for (int image = 0; image < MCUBOOT_IMAGE_NUMBER; image++)
    {
        status_t ret;
        uint32_t imgstate = kSwapType_Fail;
        const char *name = boot_image_names[image];

        ret = bl_get_image_state(image, &imgstate);
        if (ret != kStatus_Success)
        {
            PRINTF("Failed to get state of image %u (ret %d)", image, ret);
        }

        PRINTF("Image %d; name %s; state %s:\n", image, name, bl_imgstate_to_str(imgstate));

        for (int slot = 0; slot < 2; slot++)
        {
            int faid              = image * 2 + slot;
            struct flash_area *fa = &boot_flash_map[faid];
            uint32_t slotaddr     = fa->fa_off + BOOT_FLASH_BASE;
            uint32_t slotsize     = fa->fa_size;
            static struct image_header ih;
            int slotused;
            
            PRINTF("\n  Slot %d %s; address 0x%x; size 0x%x (%u): \n",
                    faid, fa->fa_name, slotaddr, slotsize, slotsize);
            
            ret = mflash_drv_read(fa->fa_off, (uint32_t *)&ih, sizeof(ih));
            if (ret != kStatus_Success)
            {
                PRINTF("    Failed to read!\n");
                continue;
            }
            
            slotused = (ih.ih_magic == IMAGE_MAGIC);

            if (slotused)
            {
                struct image_version *iv = &ih.ih_ver;
                uint8_t sha256[32];

                PRINTF("    <IMAGE: size %u; version %u.%u.%u+%lu>\n",
                       ih.ih_img_size,
                       iv->iv_major, iv->iv_minor, iv->iv_revision, iv->iv_build_num);
                
                sha256_init(&sha256ctx);
                sha256_update(&sha256ctx, (void *)slotaddr, ih.ih_img_size);
                ret = sha256_finish(&sha256ctx, sha256);
                if (ret)
                {
                    PRINTF("    Failed to hash!\n");
                    continue;
                }
                PRINTF("    SHA256: ");
                print_hash(sha256, 10);
                PRINTF("...\n");
                
            }
            else
            {
                PRINTF("    <No Image Found>\n");
            }
        }
    }
    PUTCHAR('\n');
}

static shell_status_t shellCmd_image(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int image = 0;
    int ret;
    status_t status;
    uint32_t imgstate;

    if (argc > 3)
    {
        PRINTF("Too many arguments.\n");
        return kStatus_SHELL_Error;
    }

    /* image [info] */

    if (argc == 1 || (argc == 2 && !strcmp(argv[1], "info")))
    {
        print_image_info();
        return kStatus_SHELL_Success;
    }

    if (argc < 2)
    {
        PRINTF("Wrong arguments. See 'help'\n");
        return kStatus_SHELL_Error;
    }

    if (argc == 3)
    {
        char *parse_end;
        image = strtol(argv[2], &parse_end, 10);
        
        if (image < 0 || image >= MCUBOOT_IMAGE_NUMBER || *parse_end != '\0')
        {
            PRINTF("Wrong image number.\n");
            return kStatus_SHELL_Error;
        }
    }

    status = bl_get_image_state(image, &imgstate);
    if (status != kStatus_Success)
    {
        PRINTF("Failed to get state of image %u (status %d)", image, status);
        return kStatus_SHELL_Error;
    }

    /* image test [imgNum] */

    if (!strcmp(argv[1], "test"))
    {
        status = bl_update_image_state(image, kSwapType_ReadyForTest);
        if (status != kStatus_Success)
        {
            PRINTF("FAILED to mark image state as ReadyForTest (status=%d)\n", status);
            return kStatus_SHELL_Error;
        }
    }

    /* image accept [imgNum] */

    else if (!strcmp(argv[1], "accept"))
    {
        if (imgstate != kSwapType_Testing)
        {
            PRINTF("Image state is not set as Testing. Nothing to accept.\n", status);
            return kStatus_SHELL_Error;
        }

        status = bl_update_image_state(image, kSwapType_Permanent);
        if (status != kStatus_Success)
        {
            PRINTF("FAILED to accept image (status=%d)\n", status);
            return kStatus_SHELL_Error;
        }
    }
    
    /* image erase [imgNum] */
    
    else if (!strcmp(argv[1], "erase"))
    {
        int faid              = image*2 + 1;
        struct flash_area *fa = &boot_flash_map[faid];
        uint32_t slotaddr     = fa->fa_off + BOOT_FLASH_BASE;
        uint32_t slotsize     = fa->fa_size;
        uint32_t slotcnt      = (slotsize-1 + MFLASH_SECTOR_SIZE) / MFLASH_SECTOR_SIZE;
        
        for (int i=0; i < slotcnt; i++)
        {
            ret = mflash_drv_sector_erase(slotaddr);
            if (ret)
            {
                PRINTF("Failed to erase sector at 0x%x (ret=%d)\n", slotaddr, ret);
                return kStatus_SHELL_Error;
            }
            slotaddr += MFLASH_SECTOR_SIZE;
        }
    }
    
    else
    {
        PRINTF("Wrong arguments. See 'help'\n");
        return kStatus_SHELL_Error;
    }

    return kStatus_SHELL_Success;
}

static shell_status_t shellCmd_mem(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int ret;
    uint32_t addr;
    uint32_t size = 128;
    char *parse_end;

    if (argc < 3 || argc > 4)
    {
        PRINTF("Wrong argument count\n");
        return kStatus_SHELL_Error;
    }
 
    addr = strtol(argv[2], &parse_end, 0);
    if (*parse_end != '\0')
    {
        PRINTF("Bad address\n");
        return kStatus_SHELL_Error;
    }
    
    if (argc == 4)
    {
        size = strtol(argv[3], &parse_end, 0);
        if (*parse_end != '\0')
        {
            PRINTF("Bad size\n");
            return kStatus_SHELL_Error;
        }
    }

    /* mem read addr [size] */

    if (!strcmp(argv[1], "read"))
    {
#ifdef MFLASH_PAGE_INTEGRITY_CHECKS
        if (mflash_drv_is_readable(addr) != kStatus_Success)
        {
            PRINTF("Page not readable\n");
            return kStatus_SHELL_Error;
        }
#endif
        hexdump((void *)addr, size);
    }

    /* mem erase addr */

    else if (!strcmp(argv[1], "erase"))
    {
        ret = mflash_drv_sector_erase(addr & ~(MFLASH_SECTOR_SIZE-1));
        if (ret)
        {
            PRINTF("Failed to erase sector (ret=%d)\n", ret);
            return kStatus_SHELL_Error;
        }
    }

    else
    {
        PRINTF("Wrong arguments. See 'help'\n");
        return kStatus_SHELL_Error;
    }

    return kStatus_SHELL_Success;
}

static int process_received_data(uint32_t offset, uint32_t size)
{
    int ret;
    uint32_t *data = progbuf;
    
    /* Use the secondary slot of the first image */
    
    struct flash_area *fa = &boot_flash_map[FLASH_AREA_IMAGE_SECONDARY(0)];
    uint32_t addr         = fa->fa_off + offset;
    
    /* 1kB programming buffer should be ok with all page size alignments */
      
    while (size)
    {
        size_t chunk = (size < MFLASH_PAGE_SIZE) ? size : MFLASH_PAGE_SIZE;
        
        /* mlfash takes entire page, in case of last data of smaller size it will
           program more data, which shouln't be a problem as the space allocated
           for the image slot is page aligned */
        
        ret = mflash_drv_page_program(addr, data);
        if (ret)
        {
            PRINTF("Failed to program flash at %x (ret %d)\n", addr, ret);
            return -1;
        }
        
        sha256_update(&sha256ctx, data, chunk);
        addr += chunk;
        data += chunk/sizeof(uint32_t);
        size -= chunk;
    }
    
    return 0;
}

static shell_status_t shellCmd_xmodem(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    long recvsize;    
    uint8_t sha256[32];
    struct flash_area *fa = &boot_flash_map[FLASH_AREA_IMAGE_SECONDARY(0)];
    uint32_t addr_log     = fa->fa_off + BOOT_FLASH_BASE;
    
    struct xmodem_cfg cfg = {
        .putc = xmodem_putc,
        .getc = xmodem_getc,
        .canread = xmodem_canread,
        .canread_retries = xmodem_canread_retries,
        .maxsize = fa->fa_size,
        .buffer = (uint8_t*)progbuf,
        .buffer_size = sizeof(progbuf),
        .buffer_full_callback = process_received_data
    };
    
    sha256_init(&sha256ctx);
    
    PRINTF("Initiated XMODEM-CRC transfer. Receiving... (Press 'x' to cancel)\n");
    
    recvsize = xmodem_receive(&cfg);
    if (recvsize < 0)
    {
        PRINTF("\nTransfer failed (%d)\n", recvsize);
        return kStatus_SHELL_Error;
    }

    PRINTF("\nReceived %u bytes\n", recvsize);    
    sha256_finish(&sha256ctx, sha256);    

    PRINTF("SHA256 of received data: ");
    print_hash(sha256, 10);
    PRINTF("...\n");
   
    sha256_init(&sha256ctx);
    sha256_update(&sha256ctx, (void *)addr_log, recvsize);
    sha256_finish(&sha256ctx, sha256);
    PRINTF("SHA256 of flashed data:  ");
    print_hash(sha256, 10);
    PRINTF("...\n");

    return kStatus_SHELL_Success;
}


static shell_status_t shellCmd_reboot(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    PRINTF("System reset!\n");
    NVIC_SystemReset();

    /* return kStatus_SHELL_Success; */
}



/*!
 * @brief Main function
 */
int main(void)
{
    int ret;
    s_shellHandle = &s_shellHandleBuffer[0];

    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    
    ret = mflash_drv_init();
    if (ret)
    {
        PRINTF("Failed to init flash driver\n");
    }

    PRINTF("\n"
           "*************************************\n"
           "* Basic MCUBoot application example *\n"
           "*************************************\n\n");
    
    PRINTF("Built " __DATE__ " " __TIME__ "\n");
    
    ret = SHELL_Init(s_shellHandle, g_serialHandle, "$ ");
    if (ret != kStatus_SHELL_Success)
    {
        PRINTF("Failed to init shell\n");
        goto failed_init;
    }

    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(image));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(xmodem));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(mem));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(reboot));


    while (1)
    {
        SHELL_Task(s_shellHandle);
    }

failed_init:
    while (1)
    {        
    }
}
