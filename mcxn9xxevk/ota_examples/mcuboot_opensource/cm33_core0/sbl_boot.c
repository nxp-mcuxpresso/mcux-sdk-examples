/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Freescale includes. */
#include "sbl.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "boot.h"

/* MCUboot */
#include "bootutil/bootutil_log.h"
#include "bootutil/image.h"
#include "bootutil/bootutil.h"
#include "sysflash/sysflash.h"
#include "flash_map.h"
#include "bootutil_priv.h"

#include "mflash_drv.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define VERSION_SPACER     "."
#define STR2(VERSION)      #VERSION
#define STR(VERSION)       STR2(VERSION)
#define BOOTLOADER_VERSION STR(MAJOR_VERSION) VERSION_SPACER STR(MINOR_VERSION) VERSION_SPACER STR(REVISE_VERSION)

#define IMAGE_TRAILER_SIZE sizeof(struct image_trailer)

#ifdef SINGLE_IMAGE
int boot_single_go(struct boot_rsp *rsp);
#endif
extern void SBL_DisablePeripherals(void);
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#if (defined(COMPONENT_KBOOT))
extern int isp_kboot_main(bool isInfiniteIsp);
#endif

#ifdef CONFIG_BOOT_SIGNATURE
status_t CRYPTO_InitHardware(void);
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/

struct image_trailer image_trailer2;
/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Application entry point.
 */
/* mbedtls\port\ksdk\ksdk_mbedtls.c */
// extern void CRYPTO_InitHardware(void);
void os_heap_init(void);

#ifdef SINGLE_IMAGE

static struct boot_loader_state boot_data_single;

static int boot_read_first_image_header(struct image_header *out_hdr)
{
    const struct flash_area *fap;
    int area_id;
    int rc;

    area_id = flash_area_id_from_image_slot(BOOT_PRIMARY_SLOT);
    rc      = flash_area_open(area_id, &fap);
    if (rc != 0)
    {
        rc = BOOT_EFLASH;
        goto done;
    }

    rc = flash_area_read(fap, 0, out_hdr, sizeof *out_hdr);
    if (rc != 0)
    {
        rc = BOOT_EFLASH;
        goto done;
    }

    rc = 0;

done:
    flash_area_close(fap);
    return rc;
}

static inline bool boot_data_check(uint8_t val, void *data, size_t len)
{
    uint8_t i;
    uint8_t *p = (uint8_t *)data;
    for (i = 0; i < len; i++)
    {
        if (val != p[i])
        {
            return false;
        }
    }
    return true;
}

static int boot_check_first_header_erased(void)
{
    const struct flash_area *fap;
    struct image_header *hdr;
    uint8_t erased_val;
    int rc;

    rc = flash_area_open(flash_area_id_from_image_slot(BOOT_PRIMARY_SLOT), &fap);
    if (rc != 0)
    {
        return -1;
    }

    erased_val = flash_area_erased_val(fap);
    flash_area_close(fap);

    hdr = boot_img_hdr(&boot_data_single, BOOT_PRIMARY_SLOT);
    if (!boot_data_check(erased_val, &hdr->ih_magic, sizeof(hdr->ih_magic)))
    {
        return -1;
    }

    return 0;
}

/*
 * Validate image hash/signature in a slot.
 */
static int boot_first_image_check(struct image_header *hdr, const struct flash_area *fap, struct boot_status *bs)
{
    static uint8_t tmpbuf[BOOT_TMPBUF_SZ];

    if (bootutil_img_validate(NULL, 0, hdr, fap, tmpbuf, BOOT_TMPBUF_SZ, NULL, 0, NULL))
    {
        return BOOT_EBADIMAGE;
    }
    return 0;
}

static int boot_validate_first_slot(struct boot_status *bs)
{
    const struct flash_area *fap;
    struct image_header *hdr;
    int rc;

    rc = flash_area_open(flash_area_id_from_image_slot(BOOT_PRIMARY_SLOT), &fap);
    if (rc != 0)
    {
        return BOOT_EFLASH;
    }

    hdr = boot_img_hdr(&boot_data_single, BOOT_PRIMARY_SLOT);
    if (boot_check_first_header_erased() == 0 || (hdr->ih_flags & IMAGE_F_NON_BOOTABLE))
    {
        /* No bootable image in slot; continue booting from the primary slot. */
        rc = -1;
        goto out;
    }

    if ((hdr->ih_magic != IMAGE_MAGIC || boot_first_image_check(hdr, fap, bs) != 0))
    {
        BOOT_LOG_ERR("Image is not valid!");
        rc = -1;
        goto out;
    }

out:
    flash_area_close(fap);
    return rc;
}

/**
 * Prepares the booting process.  This function verify the image around in first
 * slot and tell you which address to boot to.
 *
 * @param rsp                   On success, indicates how booting should occur.
 *
 * @return                      0 on success; nonzero on failure.
 */
int boot_single_go(struct boot_rsp *rsp)
{
    int rc;
    int fa_id;

    /* The array of slot sectors are defined here (as opposed to file scope) so
     * that they don't get allocated for non-boot-loader apps.  This is
     * necessary because the gcc option "-fdata-sections" doesn't seem to have
     * any effect in older gcc versions (e.g., 4.8.4).
     */
    static boot_sector_t primary_slot_sectors[BOOT_MAX_IMG_SECTORS];
    boot_data_single.imgs[0][BOOT_PRIMARY_SLOT].sectors = primary_slot_sectors;

    fa_id = flash_area_id_from_image_slot(BOOT_PRIMARY_SLOT);
    rc    = flash_area_open(fa_id, &BOOT_IMG_AREA(&boot_data_single, BOOT_PRIMARY_SLOT));
    assert(rc == 0);

    //    /* Determine the sector layout of the image slot. */
    //    rc = boot_initialize_area(&boot_data_single, FLASH_AREA_IMAGE_PRIMARY(0));
    //    if (rc != 0) {
    //        BOOT_LOG_WRN("Failed reading sectors; BOOT_MAX_IMG_SECTORS=%d - too small?",
    //                BOOT_MAX_IMG_SECTORS);
    //        goto out;
    //    }

    /* Attempt to read an image header from each slot. */
    rc = boot_read_first_image_header(boot_img_hdr(&boot_data_single, BOOT_PRIMARY_SLOT));
    if (rc != 0)
    {
        goto out;
    }

#ifdef CONFIG_BOOT_SIGNATURE
    rc = boot_validate_first_slot(NULL);
    if (rc != 0)
    {
        rc = BOOT_EBADIMAGE;
        goto out;
    }
#endif

    /* Always boot from the primary slot. */
    rsp->br_flash_dev_id = boot_data_single.imgs[0][BOOT_PRIMARY_SLOT].area->fa_device_id;
    rsp->br_image_off    = boot_img_slot_off(&boot_data_single, BOOT_PRIMARY_SLOT);
    rsp->br_hdr          = boot_img_hdr(&boot_data_single, BOOT_PRIMARY_SLOT);

out:
    flash_area_close(BOOT_IMG_AREA(&boot_data_single, BOOT_PRIMARY_SLOT));
    return rc;
}
#endif /* SINGLE_IMAGE */

int sbl_boot_main(void)
{
    struct image_header br_hdr1 = {.ih_hdr_size = 0x2000};
    struct boot_rsp rsp         = {.br_hdr = &br_hdr1, .br_flash_dev_id = 1, .br_image_off = 0x80000};
    int rc                      = 0;

#ifdef CONFIG_BOOT_SIGNATURE
    CRYPTO_InitHardware();
#endif

    mflash_drv_init();

    BOOT_LOG_INF("Bootloader Version %s", BOOTLOADER_VERSION);
    os_heap_init();

#ifdef SINGLE_IMAGE
    rc = boot_single_go(&rsp);
#else
    rc = boot_go(&rsp);
#endif /* SINGLE_IMAGE*/
    if (rc != 0)
    {
        BOOT_LOG_ERR("Unable to find bootable image");
        for (;;)
            ;
    }

    BOOT_LOG_INF("Bootloader chainload address offset: 0x%x", rsp.br_image_off);
    BOOT_LOG_INF("Reset_Handler address offset: 0x%x", rsp.br_image_off + rsp.br_hdr->ih_hdr_size);
    BOOT_LOG_INF("Jumping to the image\r\n\r\n");
    do_boot(&rsp);

    BOOT_LOG_ERR("Never should get here");
    for (;;)
        ;
}

void cleanup(void)
{
    SBL_DisablePeripherals();
}

#if !defined(MCUBOOT_DIRECT_XIP) && !defined(MCUBOOT_SWAP_USING_MOVE) && !defined(MCUBOOT_OVERWRITE_ONLY)
#warning "Make sure scratch area is defined in 'boot_flash_map' array if required by defined swap mechanism"
#endif

#if defined(MCUBOOT_DIRECT_XIP) && CONFIG_UPDATEABLE_IMAGE_NUMBER > 1
#error "DIRECT_XIP (using remapping) and multiple images is not currently supported"
#endif
