/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*-----------------------------------------------------------------------------------------
------
------    Includes
------
-----------------------------------------------------------------------------------------*/
#define _FOE_SUPP_ 1
#include "foe_support.h"
#undef _FOE_SUPP_

#include "fsl_debug_console.h"
#include "mflash_drv.h"
#include "sysflash/sysflash.h"
#include "flash_map.h"
#include "mcuboot_app_support.h"

static uint32_t image;
static partition_t storage;
static uint32_t base_flash_addr;
static uint32_t next_erase_addr;
/* Received/processed data counter */
static uint32_t total_processed = 0;

/* set flash information before update image */
int32_t FoE_PartitionInit(void)
{
    int32_t retval = 0;
    /* Preset result code to indicate "no error" */
    int32_t mflash_result = 0;

    /* Check partition alignment */
    if (!mflash_drv_is_sector_aligned(storage.start) || !mflash_drv_is_sector_aligned(storage.size)) {
        PRINTF("%s: partition not aligned\r\n", __func__);
        return -1;
    }

    /* Pre-set address of area not erased so far */
    next_erase_addr  = storage.start;
    base_flash_addr = storage.start;

    return 0;
}

static uint8_t buffer[MFLASH_PAGE_SIZE];
static uint16_t buffer_index = 0;
/* receive update image and store it in the flash partition */
int32_t FoE_StoreImage(uint8_t *data, uint16_t size, uint32_t offset, uint8_t isEnd)
{
    int32_t retval = 0;
    /* Preset result code to indicate "no error" */
    int32_t mflash_result = 0;
    uint16_t data_size = size;
    uint16_t space_left;
    uint16_t len;
    uint16_t data_index = 0;
    uint32_t offset_base = offset / MFLASH_PAGE_SIZE;

    if (size == 0) {
        retval = -1;
        return retval;
    }
    
    if (offset + size >= storage.size) {
        /* Partition boundary exceeded */
        PRINTF("\n%s: partition boundary exceedded\r\n", __func__);
        retval = -1;
        return retval;
    }

    /* Perform erase when encountering next sector */
    while ((base_flash_addr + offset + size) > next_erase_addr) {
        mflash_result = mflash_drv_sector_erase(next_erase_addr);
        if (mflash_result != 0) {
            PRINTF("\n%s: FLASH ERROR AT offset 0x%x\r\n", __func__, (base_flash_addr + offset));
            return mflash_result;
        }
        next_erase_addr += MFLASH_SECTOR_SIZE;
    }

    while (data_size > 0) {
        space_left = MFLASH_PAGE_SIZE - buffer_index;
        len = data_size < space_left ? data_size : space_left;
        data_size -= len;
        memcpy(buffer + buffer_index, data + data_index, len);
        data_index += len;
        buffer_index += len;

        if (buffer_index == MFLASH_PAGE_SIZE || isEnd) {
            /* Clear the unused portion of the buffer (applicable to the last chunk) */
            if (buffer_index < MFLASH_PAGE_SIZE) {
                memset(buffer + buffer_index, 0xff, MFLASH_PAGE_SIZE - buffer_index);
            }
            /* Program the page */
            mflash_result = mflash_drv_page_program(base_flash_addr + (offset_base * MFLASH_PAGE_SIZE), (uint32_t *)buffer);
            offset_base++;
            if (mflash_result != 0) {
                PRINTF("\n%s: FLASH ERROR AT offset 0x%x\r\n", __func__, (base_flash_addr + offset));
                return mflash_result;
            }
            buffer_index = 0;
        }
    }
    total_processed += size;
    PRINTF("\r%s: processed %i bytes \r\n", __func__, total_processed);
    return retval;
}

void FoE_UpdateImage(void)
{
    status_t status;
    /* Unless retval is already set (to an error code) */
    PRINTF("\n%s: upload complete (%u bytes)\r\n", __func__, total_processed);

    if (total_processed < 0) {
        /* Error during upload */
        PRINTF("Error during upload\r\n");
    }

    if (bl_verify_image(storage.start, total_processed) <= 0) {
        /* Image validation failed */
        PRINTF("Image validation failed\r\n");
    }

    /* Mark image swap type as Testing */
    status = bl_update_image_state(image, kSwapType_ReadyForTest);
    if (status != kStatus_Success) {
        PRINTF("FAILED to mark image as ReadyForTest (ret=%d)\r\n", status);
    } else {
        PRINTF("Update image success\r\n");
    }
    return;
}

void FoE_UpdatePartition(void)
{
    image = 0;

    if (bl_get_update_partition_info(image, &storage) != kStatus_Success) {
        PRINTF("FAILED to determine address for download\r\n");
    }
    PRINTF("storage addr: 0x28%lX \r\n", storage.start);
    return;
}

uint8_t FoE_WriteFirmwareInformation(uint32_t flash_addr, uint8_t *name, uint8_t name_size, uint32_t firmware_size)
{
    /* Preset result code to indicate "no error" */
    int32_t mflash_result = 0;
    uint8_t buffer[MFLASH_PAGE_SIZE];

    if (firmware_size <= 0 || name_size <= 0) {
        return 1;
    }

    mflash_result = mflash_drv_sector_erase(flash_addr);
    if (mflash_result != 0) {
        PRINTF("\n%s: FLASH ERROR AT offset 0x%x\r\n", __func__, (flash_addr));
        return mflash_result;
    }

    memcpy(buffer, name, name_size);
    int index = name_size;
    for (int i = 0; i < 4; i++)
    {
        buffer[index++] = (firmware_size >> (i * 8)) & 0xFF;
    }

    memset(buffer + index, 0xff, MFLASH_PAGE_SIZE - index);
    /* Program the page */
    mflash_result = mflash_drv_page_program(flash_addr, (uint32_t *)buffer);

    return mflash_result;    
}
