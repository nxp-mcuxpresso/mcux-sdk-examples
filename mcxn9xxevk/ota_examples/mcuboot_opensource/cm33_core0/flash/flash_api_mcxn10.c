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

/* MCUBOOT flash port layer for MCX N10 series */

#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

/* minimal write size is 16 bytes (phrase size) */
#define ALIGN_VAL 16

#define ERASED_VAL 0xFF

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

    return mflash_drv_read(addr, dst, len);
}

int flash_area_write(const struct flash_area *area, uint32_t off, const void *src, uint32_t len)
{
    status_t status   = kStatus_Success;
    uint8_t *src_ptr  = (uint8_t *)src;
    uint32_t dst_addr = area->fa_off + off;

    uint32_t phrase_buffer[MFLASH_PHRASE_SIZE / sizeof(uint32_t)];

    /* MCUBOOT on MCX N10 series is configured for 16B write alignment */
    assert((len % MFLASH_PHRASE_SIZE) == 0);
    assert((dst_addr % MFLASH_PHRASE_SIZE) == 0);

    if (area->fa_device_id != FLASH_DEVICE_ID)
    {
        return -1;
    }

    while (len > 0)
    {
        /* make sure the source data is 4B aligned */
        memcpy(phrase_buffer, src_ptr, sizeof(phrase_buffer));

        status = mflash_drv_phrase_program(dst_addr, phrase_buffer);
        if (status != kStatus_Success)
        {
            break;
        }

        src_ptr += MFLASH_PHRASE_SIZE;
        dst_addr += MFLASH_PHRASE_SIZE;
        len -= MFLASH_PHRASE_SIZE;
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

int flash_area_get_sector(const struct flash_area *fa, uint32_t off, struct flash_sector *sector)
{
    if (off >= fa->fa_size)
    {
        return -1;
    }

    sector->fs_off  = (off / MFLASH_SECTOR_SIZE) * MFLASH_SECTOR_SIZE;
    sector->fs_size = MFLASH_SECTOR_SIZE;

    return 0;
}

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
