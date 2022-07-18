/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include "mcuboot_app_support.h"
#include "mflash_drv.h"

#include "fsl_debug_console.h"

const uint32_t boot_img_magic[] = {
    0xf395c277,
    0x7fefd260,
    0x0f505235,
    0x8079b62c,
};

static int32_t flash_read(uint32_t addr, uint32_t *buffer, uint32_t len)
{
    uint8_t *buffer_u8 = (uint8_t *)buffer;

    /* inter-page lenght of the first read can be smaller than page size */
    size_t plen = MFLASH_PAGE_SIZE - (addr % MFLASH_PAGE_SIZE);

    while (len > 0)
    {
        size_t readsize = (len > plen) ? plen : len;

#if defined(MFLASH_PAGE_INTEGRITY_CHECKS) && MFLASH_PAGE_INTEGRITY_CHECKS
        if (mflash_drv_is_readable(addr) != kStatus_Success)
        {
            /* PRINTF("%s: UNREADABLE PAGE at %x\n", __func__, addr); */
            memset(buffer_u8, 0xff, readsize);
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

static int check_unset(uint8_t *p, int len)
{
    while (len > 0)
    {
        if (*p != 0xff)
        {
            return 0;
        }
        p++;
        len--;
    }
    return 1;
}

static int boot_img_magic_check(uint8_t *p)
{
    int len    = sizeof(boot_img_magic);
    uint8_t *q = (uint8_t *)boot_img_magic;

    while ((len > 0) && (*p == *q))
    {
        len--;
        p++;
        q++;
    }

    if (len == 0)
    {
        /* all bytes matched the magic */
        return 1;
    }

    return 0;
}

static status_t boot_swap_test(void)
{
    uint32_t off;
    status_t status;

    uint32_t buf[MFLASH_PAGE_SIZE / 4]; /* ensure the buffer is word aligned */
    struct image_trailer *image_trailer_p =
        (struct image_trailer *)((uint8_t *)buf + MFLASH_PAGE_SIZE - sizeof(struct image_trailer));

    off = FLASH_AREA_IMAGE_2_OFFSET + FLASH_AREA_IMAGE_2_SIZE - MFLASH_PAGE_SIZE;

    memset(buf, 0xff, MFLASH_PAGE_SIZE);
    memcpy(image_trailer_p->magic, boot_img_magic, sizeof(boot_img_magic));

    PRINTF("write magic number offset = 0x%x\r\n", off);

    status = mflash_drv_sector_erase(FLASH_AREA_IMAGE_2_OFFSET + FLASH_AREA_IMAGE_2_SIZE - MFLASH_SECTOR_SIZE);
    if (status != kStatus_Success)
    {
        PRINTF("%s: failed to erase trailer2\r\n", __func__);
        return status;
    }

    status = mflash_drv_page_program(off, buf);
    if (status != kStatus_Success)
    {
        PRINTF("%s: failed to write trailer2\r\n", __func__);
        return status;
    }

    return status;
}

status_t boot_swap_perm(void)
{
    uint32_t off;
    status_t status;

    uint32_t buf[MFLASH_PAGE_SIZE / 4]; /* ensure the buffer is word aligned */
    struct image_trailer *image_trailer_p =
        (struct image_trailer *)((uint8_t *)buf + MFLASH_PAGE_SIZE - sizeof(struct image_trailer));

    off = FLASH_AREA_IMAGE_2_OFFSET + FLASH_AREA_IMAGE_2_SIZE - MFLASH_PAGE_SIZE;

    memset(buf, 0xff, MFLASH_PAGE_SIZE);
    memcpy(image_trailer_p->magic, boot_img_magic, sizeof(boot_img_magic));
    image_trailer_p->image_ok = BOOT_FLAG_SET;

    status = mflash_drv_sector_erase(FLASH_AREA_IMAGE_2_OFFSET + FLASH_AREA_IMAGE_2_SIZE - MFLASH_SECTOR_SIZE);
    if (status != kStatus_Success)
    {
        PRINTF("%s: failed to erase trailer2\r\n", __func__);
        return status;
    }

    status = mflash_drv_page_program(off, buf);
    if (status != kStatus_Success)
    {
        PRINTF("%s: failed to write trailer2\r\n", __func__);
        return status;
    }

    return status;
}

static status_t boot_swap_ok(void)
{
    uint32_t off;
    status_t status;

    uint32_t buf[MFLASH_PAGE_SIZE / 4]; /* ensure the buffer is word aligned */
    struct image_trailer *image_trailer_p =
        (struct image_trailer *)((uint8_t *)buf + MFLASH_PAGE_SIZE - sizeof(struct image_trailer));

    off = FLASH_AREA_IMAGE_1_OFFSET + FLASH_AREA_IMAGE_1_SIZE - MFLASH_PAGE_SIZE;

    status = flash_read(off, buf, MFLASH_PAGE_SIZE);
    if (status != kStatus_Success)
    {
        PRINTF("%s: failed to read trailer\r\n", __func__);
        return status;
    }

    if ((boot_img_magic_check(image_trailer_p->magic) == 0) || (image_trailer_p->copy_done != 0x01))
    {
        /* the image in the slot is likely incomplete (or none) */
        PRINTF("%s: there is no image awaiting confirmation\r\n", __func__);
        status = kStatus_NoData;
        return status;
    }

    if (image_trailer_p->image_ok == BOOT_FLAG_SET)
    {
        /* nothing to be done, report it and return */
        PRINTF("%s: image already confirmed\r\n", __func__);
        return status;
    }

    /* mark image ok */
    image_trailer_p->image_ok = BOOT_FLAG_SET;

    /* erase trailer */
    status = mflash_drv_sector_erase(FLASH_AREA_IMAGE_1_OFFSET + FLASH_AREA_IMAGE_1_SIZE - MFLASH_SECTOR_SIZE);
    if (status != kStatus_Success)
    {
        PRINTF("%s: failed to erase trailer1\r\n, __func__");
        return status;
    }

    /* write trailer */
    status = mflash_drv_page_program(off, buf);
    if (status != kStatus_Success)
    {
        PRINTF("%s: failed to write trailer1\r\n, __func__");
        return status;
    }

    return status;
}

int32_t bl_verify_image(const uint8_t *data, uint32_t size)
{
    struct image_header *ih;
    struct image_tlv_info *it;
    uint32_t decl_size;
    uint32_t tlv_size;

    ih = (struct image_header *)data;

    /* do we have at least the header */
    if (size < sizeof(struct image_header))
    {
        return 0;
    }

    /* check magic number */
    if (ih->ih_magic != IMAGE_MAGIC)
    {
        return 0;
    }

    /* check that we have at least the amount of data declared by the header */
    decl_size = ih->ih_img_size + ih->ih_hdr_size + ih->ih_protect_tlv_size;
    if (size < decl_size)
    {
        return 0;
    }

    /* check protected TLVs if any */
    if (ih->ih_protect_tlv_size > 0)
    {
        if (ih->ih_protect_tlv_size < sizeof(struct image_tlv_info))
        {
            return 0;
        }
        it = (struct image_tlv_info *)(data + ih->ih_img_size + ih->ih_hdr_size);
        if ((it->it_magic != IMAGE_TLV_PROT_INFO_MAGIC) || (it->it_tlv_tot != ih->ih_protect_tlv_size))
        {
            return 0;
        }
    }

    /* check for optional TLVs following the image as declared by the header */
    tlv_size = size - decl_size;
    if (tlv_size > 0)
    {
        if (tlv_size < sizeof(struct image_tlv_info))
        {
            return 0;
        }
        it = (struct image_tlv_info *)(data + decl_size);
        if ((it->it_magic != IMAGE_TLV_INFO_MAGIC) || (it->it_tlv_tot != tlv_size))
        {
            return 0;
        }
    }

    return 1;
}

status_t bl_get_update_partition_info(partition_t *partition)
{
    memset(partition, 0x0, sizeof(*partition));

    partition->start = BOOT_FLASH_CAND_APP;
    partition->size  = BOOT_FLASH_CAND_APP - BOOT_FLASH_ACT_APP;

    return kStatus_Success;
}

status_t bl_update_image_state(uint32_t state)
{
    status_t status;

    switch (state)
    {
        case kSwapType_ReadyForTest:
            status = boot_swap_test();
            break;

        case kSwapType_Permanent:
            status = boot_swap_ok();
            break;

        default:
            status = kStatus_InvalidArgument;
            break;
    }

    return status;
}

status_t bl_get_image_state(uint32_t *state)
{
    status_t status;
    uint32_t off;

    struct image_trailer image_trailer1;
    struct image_trailer image_trailer2;

    off    = FLASH_AREA_IMAGE_1_OFFSET + FLASH_AREA_IMAGE_1_SIZE - sizeof(struct image_trailer);
    status = flash_read(off, (uint32_t *)&image_trailer1, sizeof(struct image_trailer));
    if (status)
    {
        PRINTF("%s: failed to read trailer1\r\n", __func__);
        return status;
    }

    off    = FLASH_AREA_IMAGE_2_OFFSET + FLASH_AREA_IMAGE_2_SIZE - sizeof(struct image_trailer);
    status = flash_read(off, (uint32_t *)&image_trailer2, sizeof(struct image_trailer));
    if (status)
    {
        PRINTF("%s: failed to read trailer2\r\n", __func__);
        return status;
    }

    if (boot_img_magic_check(image_trailer2.magic))
    {
        if (check_unset(&image_trailer2.image_ok, sizeof(image_trailer2.image_ok)))
        {
            /* State I (request for swaping upon next reboot) */
            *state = kSwapType_ReadyForTest;
            return kStatus_Success;
        }
        else if (image_trailer2.image_ok == 0x01)
        {
            /* State II (image marked for permanent change) */
            *state = kSwapType_Permanent;
            return kStatus_Success;
        }
    }
    else if (check_unset(image_trailer2.magic, sizeof(image_trailer2.magic)))
    {
        if (boot_img_magic_check(image_trailer1.magic) && (image_trailer1.image_ok == 0xff) &&
            (image_trailer1.copy_done == 0x01))
        {
            /* State III (revert scheduled for next reboot => image is under test) */
            *state = kSwapType_Testing;
            return kStatus_Success;
        }
    }

    /* State IV (none of the above) */
    *state = kSwapType_None;
    return kStatus_Success;
}
