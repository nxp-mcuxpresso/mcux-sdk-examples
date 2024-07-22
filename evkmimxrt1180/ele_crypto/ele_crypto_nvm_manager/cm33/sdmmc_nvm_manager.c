/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "fsl_sd.h"
#include "ff.h"
#include "diskio.h"
#include "fsl_sd_disk.h"
#include "sdmmc_config.h"

#include "ele_nvm_manager.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief wait card insert function.
 */
static status_t sdcardWaitCardInsert(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static FATFS g_fileSystem; /* File system object */

/*******************************************************************************
 * Code
 ******************************************************************************/

static int create_dir(char *dir_path)
{
    FRESULT error;

    error = f_mkdir(_T(dir_path));
    if (error)
    {
         /* The directory may already exist, that is not an error */
        if (error != FR_EXIST)
        {
            return -1;
        }
    }
    return 0;
}

/* Create the filepath of chunk associated with blob_id_msb, blob_id_lsb and blob_ext */
static int get_chunk_file_path(
    char *path, uint8_t path_buf_sz, char *nvm_storage_dname,
    uint32_t blob_id_msb, uint32_t blob_id_lsb, uint32_t blob_ext,
    bool create_path)
{
    uint8_t path_len = 0;
    int err = -1;
    char *path_current = path;
    char *path_end = NULL;

    /* 1 extra byte in path_len is for accommodating null termination char
     * \0 in path string.Since fatfs is not enabled with Long file system
     * names (LFS) due to space limitation, we are only using blob_ext as filename
     * and rest of the unique values like blob_id_msb and lsb are used in the file path.
     * Chunk file path will be named <blob_ext> and the path will be
     * <nvm_storage_dname>/blob_ext/blob_id_msb/blob_id_lsb
     */
    path_len = strlen(nvm_storage_dname) + 1 +  /* 1 additional byte for / after every name */
               sizeof(blob_id_msb) * 2 + 1 +
               sizeof(blob_id_lsb) * 2 + 1 +
               sizeof(blob_ext) * 2 + 1;

    if (path_buf_sz < path_len)
    {
        PRINTF("Insufficient size of path buffer \r\n");
        return -1;
    }

    path_end = path + path_len; /* For keeping track of free space when appending to the path string */

    /* If path needs to be created, we need to check and add the required directories */
    if (create_path)
    {
        path_current += snprintf(path, path_len, "%s/%lx", nvm_storage_dname, blob_ext);
        err = create_dir(path);
        if (err)
        {
            return -1;
        }

        path_current += snprintf(path_current, path_end - path_current, "/%lx", blob_id_msb);
        err = create_dir(path);
        if (err)
        {
            return -1;
        }

        path_current += snprintf(path_current, path_end - path_current, "/%lx", blob_id_lsb);
    }
    else
    {
        snprintf(path, path_len - 1, "%s/%lx/%lx/%lx",
                 nvm_storage_dname,
                 blob_ext, blob_id_msb, blob_id_lsb);
    }

    return 0;
}

/* Create and write to a file identified by passed blob_id */
status_t sd_file_write(uint32_t blob_id_msb, uint32_t blob_id_lsb, uint32_t blob_ext, uint32_t *chunk, size_t chunk_sz)
{
    FRESULT error;
    static FIL g_fileObject; /* File object */
    char path[64];
    UINT bytesWritten;

    /* Get the filepath of chunk associated with blob ID and blob_ext. */
    if (get_chunk_file_path((char *)path, sizeof(path), "/nvm_demo",
                            blob_id_msb, blob_id_lsb, blob_ext, true))
    {
        return kStatus_Fail;
    }

    error = f_open(&g_fileObject, path, (FA_WRITE | FA_READ | FA_CREATE_ALWAYS));
    if (error)
    {
        if (error != FR_EXIST)
        {
            PRINTF("Open File failed \r\n");
            return kStatus_Fail;
        }
    }

    error = f_write(&g_fileObject, chunk, chunk_sz, &bytesWritten);
    if ((error) || (bytesWritten != chunk_sz))
    {
        PRINTF("Write file failed. \r\n");
        return kStatus_Fail;
    }

    if (f_close(&g_fileObject))
    {
        PRINTF("\r\nClose file failed.\r\n");
        return kStatus_Fail;
    }

    return kStatus_Success;
}

uint32_t *sd_file_read(uint32_t blob_id_msb, uint32_t blob_id_lsb, uint32_t blob_id_ext, uint32_t *chunk, size_t *sz)
{
    FRESULT error;
    static FIL g_fileObject; /* File object */
    char path[64];
    uint32_t file_sz;
    uint32_t *buffer = NULL;
    UINT bytesRead;

    if (get_chunk_file_path((char *)path, sizeof(path), "/nvm_demo",
                             blob_id_msb, blob_id_lsb, blob_id_ext, false))
    {
        return NULL;
    }

    error = f_open(&g_fileObject, path, (FA_READ));
    if (error)
    {
        if (error != FR_EXIST)
        {
            return NULL;
        }
    }

    /* Get the size of file associated with chunk */
    file_sz = f_size(&g_fileObject);

    /* If buffer for chunk is not passed, allocate based on sie of file */
    if (!chunk)
    {
        buffer = malloc(file_sz);
        *sz    = file_sz;
        chunk  = buffer;
    }

    if (!buffer || *sz < file_sz)
    {
        return NULL;
    }

    memset(chunk, 0U, *sz);
    /* Read the chunk data into buffer */
    error = f_read(&g_fileObject, chunk, *sz, &bytesRead);
    if ((error) || (bytesRead != *sz))
    {
        free(buffer);
        return NULL;
    }

    if (f_close(&g_fileObject))
    {
        PRINTF("\r\nClose file failed.\r\n");
        free(buffer);
        return NULL;
    }

    return chunk;
}

/* Initialize the FAT FS for SD */
int sd_fs_initialize(void)
{
    const TCHAR driverNumberBuffer[3U] = {SDDISK + '0', ':', '/'};
    FRESULT error;

    if (sdcardWaitCardInsert() != kStatus_Success)
    {
        return -1;
    }

    if (f_mount(&g_fileSystem, driverNumberBuffer, 0U))
    {
        PRINTF("Mount volume failed.\r\n");
        return -1;
    }

#if (FF_FS_RPATH >= 2U)
    error = f_chdrive((char const *)&driverNumberBuffer[0U]);
    if (error)
    {
        PRINTF("Change drive failed.\r\n");
        return kStatus_Fail;
    }
#endif

    PRINTF("\r\nCreate directory......\r\n");
    error = f_mkdir(_T("/nvm_demo"));
    if (error)
    {
        if (error != FR_EXIST)
        {
            PRINTF("Make directory failed.\r\n");
            return -1;
        }
    }

    return kStatus_Success;
}

static status_t sdcardWaitCardInsert(void)
{
    BOARD_SD_Config(&g_sd, NULL, BOARD_SDMMC_SD_HOST_IRQ_PRIORITY, NULL);

    /* SD host init function */
    if (SD_HostInit(&g_sd) != kStatus_Success)
    {
        PRINTF("\r\nSD host init fail\r\n");
        return kStatus_Fail;
    }

    /* wait card insert */
    if (SD_PollingCardInsert(&g_sd, kSD_Inserted) == kStatus_Success)
    {
        PRINTF("\r\nCard inserted.\r\n");
        /* power off card */
        SD_SetCardPower(&g_sd, false);
        /* power on the card */
        SD_SetCardPower(&g_sd, true);
    }
    else
    {
        PRINTF("\r\nCard detect fail.\r\n");
        return kStatus_Fail;
    }

    return kStatus_Success;
}
