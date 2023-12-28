/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "fsl_component_serial_manager.h"
#include "fsl_shell.h"
#include "lfs.h"
#include "lfs_mflash.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/


#define SHELL_Printf PRINTF

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* SHELL LFS command handlers */
static shell_status_t lfs_format_handler(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t lfs_mount_handler(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t lfs_unmount_handler(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t lfs_cd_handler(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t lfs_ls_handler(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t lfs_rm_handler(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t lfs_mkdir_handler(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t lfs_write_handler(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t lfs_cat_handler(shell_handle_t shellHandle, int32_t argc, char **argv);

/*******************************************************************************
 * Variables
 ******************************************************************************/


lfs_t lfs;
struct lfs_config cfg;
int lfs_mounted;

SHELL_COMMAND_DEFINE(format,
                     "\r\n\"format yes\": Formats the filesystem\r\n",
                     lfs_format_handler,
                     SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(mount, "\r\n\"mount\": Mounts the filesystem\r\n", lfs_mount_handler, 0);
SHELL_COMMAND_DEFINE(unmount, "\r\n\"unmount\": Unmounts the filesystem\r\n", lfs_unmount_handler, 0);
SHELL_COMMAND_DEFINE(umount, "", lfs_unmount_handler, 0); // unmount alias
SHELL_COMMAND_DEFINE(cd, "", lfs_cd_handler, SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(ls,
                     "\r\n\"ls <path>\": Lists directory content\r\n",
                     lfs_ls_handler,
                     SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(dir, "", lfs_ls_handler, SHELL_IGNORE_PARAMETER_COUNT); // ls alias
SHELL_COMMAND_DEFINE(rm, "\r\n\"rm <path>\": Removes file or directory\r\n", lfs_rm_handler, 1);
SHELL_COMMAND_DEFINE(mkdir, "\r\n\"mkdir <path>\": Creates a new directory\r\n", lfs_mkdir_handler, 1);
SHELL_COMMAND_DEFINE(write, "\r\n\"write <path> <text>\": Writes/appends text to a file\r\n", lfs_write_handler, 2);
SHELL_COMMAND_DEFINE(cat, "\r\n\"cat <path>\": Prints file content\r\n", lfs_cat_handler, 1);

SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
static shell_handle_t s_shellHandle;

extern serial_handle_t g_serialHandle;

/*******************************************************************************
 * Code
 ******************************************************************************/

static shell_status_t lfs_format_handler(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int res;

    if (lfs_mounted)
    {
        SHELL_Printf("LFS is mounted, please unmount it first.\r\n");
        return kStatus_SHELL_Success;
    }

    if (argc != 2 || strcmp(argv[1], "yes"))
    {
        SHELL_Printf("Are you sure? Please issue command \"format yes\" to proceed.\r\n");
        return kStatus_SHELL_Success;
    }

    res = lfs_format(&lfs, &cfg);
    if (res)
    {
        PRINTF("\rError formatting LFS: %d\r\n", res);
    }

    return kStatus_SHELL_Success;
}

static shell_status_t lfs_mount_handler(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int res;

    if (lfs_mounted)
    {
        SHELL_Printf("LFS already mounted\r\n");
        return kStatus_SHELL_Success;
    }

    res = lfs_mount(&lfs, &cfg);
    if (res)
    {
        PRINTF("\rError mounting LFS\r\n");
    }
    else
    {
        lfs_mounted = 1;
    }

    return kStatus_SHELL_Success;
}

static shell_status_t lfs_unmount_handler(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int res;

    if (!lfs_mounted)
    {
        SHELL_Printf("LFS not mounted\r\n");
        return kStatus_SHELL_Success;
    }

    res = lfs_unmount(&lfs);
    if (res)
    {
        PRINTF("\rError unmounting LFS: %i\r\n", res);
    }

    lfs_mounted = 0;
    return kStatus_SHELL_Success;
}

static shell_status_t lfs_cd_handler(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    SHELL_Printf(
        "There is no concept of current directory in this example.\r\nPlease always specify the full path.\r\n");
    return kStatus_SHELL_Success;
}

static shell_status_t lfs_ls_handler(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int res;
    char *path;
    lfs_dir_t dir;
    struct lfs_info info;
    int files;
    int dirs;

    if (!lfs_mounted)
    {
        SHELL_Printf("LFS not mounted\r\n");
        return kStatus_SHELL_Success;
    }

    if (argc > 2)
    {
        SHELL_Printf("Invalid number of parameters\r\n");
        return kStatus_SHELL_Success;
    }

    if (argc < 2)
    {
        path = "/";
    }
    else
    {
        path = argv[1];
    }

    /* open the directory */
    res = lfs_dir_open(&lfs, &dir, path);
    if (res)
    {
        PRINTF("\rError opening directory: %i\r\n", res);
        return kStatus_SHELL_Success;
    }

    PRINTF(" Directory of %s\r\n", path);
    files = 0;
    dirs  = 0;

    /* iterate until end of directory */
    while ((res = lfs_dir_read(&lfs, &dir, &info)) != 0)
    {
        if (res < 0)
        {
            /* break the loop in case of an error */
            PRINTF("\rError reading directory: %i\r\n", res);
            break;
        }

        if (info.type == LFS_TYPE_REG)
        {
            SHELL_Printf("%8d %s\r\n", info.size, info.name);
            files++;
        }
        else if (info.type == LFS_TYPE_DIR)
        {
            SHELL_Printf("%     DIR %s\r\n", info.name);
            dirs++;
        }
        else
        {
            SHELL_Printf("%???\r\n");
        }
    }

    res = lfs_dir_close(&lfs, &dir);
    if (res)
    {
        PRINTF("\rError closing directory: %i\r\n", res);
        return kStatus_SHELL_Success;
    }

    PRINTF(" %d File(s), %d Dir(s)\r\n", files, dirs);

    return kStatus_SHELL_Success;
}

static shell_status_t lfs_rm_handler(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int res;

    if (!lfs_mounted)
    {
        SHELL_Printf("LFS not mounted\r\n");
        return kStatus_SHELL_Success;
    }

    res = lfs_remove(&lfs, argv[1]);

    if (res)
    {
        PRINTF("\rError while removing: %i\r\n", res);
    }

    return kStatus_SHELL_Success;
}

static shell_status_t lfs_mkdir_handler(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int res;

    if (!lfs_mounted)
    {
        SHELL_Printf("LFS not mounted\r\n");
        return kStatus_SHELL_Success;
    }

    res = lfs_mkdir(&lfs, argv[1]);

    if (res)
    {
        PRINTF("\rError creating directory: %i\r\n", res);
    }

    return kStatus_SHELL_Success;
}

static shell_status_t lfs_write_handler(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int res;
    lfs_file_t file;

    if (!lfs_mounted)
    {
        SHELL_Printf("LFS not mounted\r\n");
        return kStatus_SHELL_Success;
    }

    res = lfs_file_open(&lfs, &file, argv[1], LFS_O_WRONLY | LFS_O_APPEND | LFS_O_CREAT);
    if (res)
    {
        PRINTF("\rError opening file: %i\r\n", res);
        return kStatus_SHELL_Success;
    }

    res = lfs_file_write(&lfs, &file, argv[2], strlen(argv[2]));
    if (res > 0)
        res = lfs_file_write(&lfs, &file, "\r\n", 2);

    if (res < 0)
    {
        PRINTF("\rError writing file: %i\r\n", res);
    }

    res = lfs_file_close(&lfs, &file);
    if (res)
    {
        PRINTF("\rError closing file: %i\r\n", res);
    }

    return kStatus_SHELL_Success;
}

static shell_status_t lfs_cat_handler(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int res;
    lfs_file_t file;
    uint8_t buf[16];

    if (!lfs_mounted)
    {
        SHELL_Printf("LFS not mounted\r\n");
        return kStatus_SHELL_Success;
    }

    res = lfs_file_open(&lfs, &file, argv[1], LFS_O_RDONLY);
    if (res)
    {
        PRINTF("\rError opening file: %i\r\n", res);
        return kStatus_SHELL_Success;
    }

    do
    {
        res = lfs_file_read(&lfs, &file, buf, sizeof(buf));
        if (res < 0)
        {
            PRINTF("\rError reading file: %i\r\n", res);
            break;
        }
        SHELL_Write(s_shellHandle, (char *)buf, res);
    } while (res);

    res = lfs_file_close(&lfs, &file);
    if (res)
    {
        PRINTF("\rError closing file: %i\r\n", res);
    }

    return kStatus_SHELL_Success;
}

int main(void)
{
    status_t status;

    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    lfs_get_default_config(&cfg);

    status = lfs_storage_init(&cfg);
    if (status != kStatus_Success)
    {
        PRINTF("LFS storage init failed: %i\r\n", status);
        return status;
    }

    /* Init SHELL */
    s_shellHandle = &s_shellHandleBuffer[0];
    SHELL_Init(s_shellHandle, g_serialHandle, "LFS>> ");

    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(format));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(mount));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(unmount));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(umount));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(cd));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(ls));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(dir));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(rm));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(mkdir));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(write));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(cat));

    while (1)
    {
#if !(defined(SHELL_NON_BLOCKING_MODE) && (SHELL_NON_BLOCKING_MODE > 0U))
        SHELL_Task(s_shellHandle);
#endif
    }
}
