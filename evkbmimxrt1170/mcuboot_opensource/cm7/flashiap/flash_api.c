/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 * Copyright (c) 2015 Runtime Inc
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sbl.h>
#include <stdint.h>
#include <string.h>
#include "flash_map.h"
#include "flash_partitioning.h"
#include "sysflash/sysflash.h"
#include "bootutil/bootutil_log.h"
#include "mflash_drv.h"

#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#define ALIGN_VAL  1
#define ERASED_VAL 0xFF

static uint32_t flash_page_buf[MFLASH_PAGE_SIZE / sizeof(uint32_t)];

int flash_device_base(uint8_t fd_id, uintptr_t *ret)
{
    if (fd_id != FLASH_DEVICE_ID)
    {
        BOOT_LOG_ERR("invalid flash ID %d; expected %d", fd_id, FLASH_DEVICE_ID);
        return -1;
    }
    *ret = MFLASH_BASE_ADDRESS;
    return 0;
}

int flash_area_open(uint8_t id, const struct flash_area **area)
{
    uint32_t i = 0;

    for (i = 0; i < MCUBOOT_IMAGE_SLOT_NUMBER; i++)
    {
        if (boot_flash_map[i].fa_id == id)
        {
            *area = &boot_flash_map[i];
            return 0;
        }
    }

    return -1;
}

void flash_area_close(const struct flash_area *area)
{
}

/*
 * Read/write/erase. Offset is relative from beginning of flash area.
 */
int flash_area_read(const struct flash_area *area, uint32_t off, void *dst, uint32_t len)
{
    uint32_t addr = area->fa_off + off;

    if (area->fa_device_id != FLASH_DEVICE_ID)
    {
        return -1;
    }

    uint8_t *buffer_u8 = dst;

    /* inter-page lenght of the first read can be smaller than page size */
    size_t plen = MFLASH_PAGE_SIZE - (addr % MFLASH_PAGE_SIZE);

    while (len > 0)
    {
        size_t readsize = (len > plen) ? plen : len;

#if defined(MFLASH_PAGE_INTEGRITY_CHECKS) && MFLASH_PAGE_INTEGRITY_CHECKS
        if (mflash_drv_is_readable(addr) != kStatus_Success)
        {
            /* PRINTF("%s: UNREADABLE PAGE at %x\n", __func__, addr); */
            memset(buffer_u8, ERASED_VAL, readsize);
        }
        else
#endif
        {
            void *flash_ptr = mflash_drv_phys2log(addr, readsize);
            if (flash_ptr == NULL)
            {
                return -1;
            }
            /* use direct memcpy as mflash_drv_read low layer may expects len to be word aligned */
            memcpy(buffer_u8, flash_ptr, readsize);
        }

        len -= readsize;
        addr += readsize;
        buffer_u8 += readsize;

        plen = MFLASH_PAGE_SIZE;
    }

    return 0;
}

/* Make sure the data address is 4 bytes aligned */
int flash_area_write(const struct flash_area *area, uint32_t off, const void *src, uint32_t len)
{
    status_t status = kStatus_Success;

    uint8_t *src_ptr  = (uint8_t *)src;
    uint32_t dst_addr = area->fa_off + off;
    uint32_t page_addr;

    uint32_t chunk_size;
    uint32_t chunk_ofs;

    if (area->fa_device_id != FLASH_DEVICE_ID)
    {
        return -1;
    }

    /* offset within the first page and max size that would fit */
    chunk_ofs  = dst_addr % MFLASH_PAGE_SIZE;
    chunk_size = MFLASH_PAGE_SIZE - chunk_ofs;

    /* calculate starting address of the page */
    page_addr = dst_addr - chunk_ofs;

    while (len > 0)
    {
        if (chunk_size > len)
        {
            chunk_size = len; /* last chunk of data */
        }

        /* check if part of the buffer is not going to be filled with data */
        if ((chunk_ofs > 0) || (chunk_size < MFLASH_PAGE_SIZE))
        {
            /* fill the buffer with erased value, tweaking to clean just the unused range does not pay off */
            memset(flash_page_buf, ERASED_VAL, MFLASH_PAGE_SIZE);
        }

        memcpy((uint8_t *)flash_page_buf + chunk_ofs, src_ptr, chunk_size);
        status = mflash_drv_page_program(page_addr, flash_page_buf);
        if (status != kStatus_Success)
        {
            break;
        }

        src_ptr += chunk_size;
        len -= chunk_size;

        /* re-initialize chunk to full size of the page */
        chunk_ofs  = 0;
        chunk_size = MFLASH_PAGE_SIZE;

        /* advance to next page */
        page_addr += MFLASH_PAGE_SIZE;
    }

    if (status != kStatus_Success)
    {
        /* some callers check only for negative error codes, translate to -1 to stay on the safe side */
        return -1;
    }

    return 0;
}

int flash_area_erase(const struct flash_area *area, uint32_t off, uint32_t len)
{
    status_t status  = kStatus_Success;
    uint32_t address = area->fa_off + off;

    if (area->fa_device_id != FLASH_DEVICE_ID)
    {
        return -1;
    }

    if ((address % MFLASH_SECTOR_SIZE) || (len % MFLASH_SECTOR_SIZE))
    {
        return -1;
    }

    for (; len > 0; len -= MFLASH_SECTOR_SIZE)
    {
        /* Erase sectors. */
        status = mflash_drv_sector_erase(address);
        if (status != kStatus_Success)
        {
            break;
        }

        address += MFLASH_SECTOR_SIZE;
    }

    if (status != kStatus_Success)
    {
        /* some callers check only for negative error codes, translate to -1 to stay on the safe side */
        return -1;
    }

    return 0;
}

/* The minimum write size */
uint8_t flash_area_align(const struct flash_area *area)
{
    if (area->fa_device_id == FLASH_DEVICE_ID)
        return ALIGN_VAL;
    else
        return 0;
}

uint8_t flash_area_erased_val(const struct flash_area *area)
{
    return ERASED_VAL;
}

#if 0 // implementation of blank check in the mflash layer may be needed
int flash_area_read_is_empty(const struct flash_area *area, uint32_t off, void *dst, uint32_t len)
{
    uint8_t i;
    uint8_t *u8dst;
    int rc;

    rc = flash_area_read(area, off, dst, len);
    if (rc)
    {
        return -1;
    }

    for (i = 0, u8dst = (uint8_t *)dst; i < len; i++)
    {
        if (u8dst[i] != ERASED_VAL)
        {
            return 0;
        }
    }

    return 1;
}
#endif

/*
 * Lookup the sector map for a given flash area.  This should fill in
 * `sectors` with all of the sectors in the area.  `*count` will be set to
 * the storage at `sectors` and should be set to the final number of
 * sectors in this area.
 */
int flash_area_get_sectors(int fa_id, uint32_t *count, struct flash_sector *sectors)
{
    const struct flash_area *fa;
    uint32_t max_cnt = *count;
    uint32_t rem_len;
    int rc = -1;

    if (flash_area_open(fa_id, &fa))
        goto out;

    if (*count < 1)
        goto fa_close_out;

    rem_len = fa->fa_size;
    *count  = 0;
    while ((rem_len > 0) && (*count < max_cnt))
    {
        if (rem_len < MFLASH_SECTOR_SIZE)
        {
            goto fa_close_out;
        }

        sectors[*count].fs_off  = MFLASH_SECTOR_SIZE * (*count);
        sectors[*count].fs_size = MFLASH_SECTOR_SIZE;
        *count                  = *count + 1;
        rem_len -= MFLASH_SECTOR_SIZE;
    }

    if (*count >= max_cnt)
    {
        goto fa_close_out;
    }

    rc = 0;

fa_close_out:
    flash_area_close(fa);
out:
    return rc;
}

/*
 * This depends on the mappings defined in flash_partitioning.c.
 * MCUBoot uses continuous numbering for the primary slot, the secondary slot,
 * and the scratch while zephyr might number it differently.
 */
int flash_area_id_from_multi_image_slot(int image_index, int slot)
{
    switch (slot)
    {
        case 0:
            return FLASH_AREA_IMAGE_PRIMARY(image_index);

        case 1:
            return FLASH_AREA_IMAGE_SECONDARY(image_index);

        default:
            return -1; /* flash_area_open will fail on that */
    }
}

int flash_area_id_from_image_slot(int slot)
{
    return flash_area_id_from_multi_image_slot(0, slot);
}
