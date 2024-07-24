/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdbool.h>

#include "boot.h"
#include "flash_partitioning.h"
#include "bootutil/bootutil_log.h"
#include "bootutil/image.h"
#include "bootutil/bootutil.h"
#include "sysflash/sysflash.h"
#include "flash_map.h"
#include "bootutil_priv.h"

#include "fsl_debug_console.h"
#include "mflash_drv.h"

#ifdef CONFIG_MCUBOOT_ENCRYPTED_XIP_SUPPORT
#include "mcuboot_enc_support.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#ifdef NDEBUG
#undef assert
#define assert(x) ((void)(x))
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#ifdef CONFIG_BOOT_SIGNATURE
status_t CRYPTO_InitHardware(void);
#endif

#ifdef CONFIG_MCUBOOT_FLASH_REMAP_ENABLE
extern void SBL_EnableRemap(uint32_t start_addr, uint32_t end_addr, uint32_t off);
extern void SBL_DisableRemap(void);
#endif

#pragma weak cleanup
void cleanup(void);

extern void SBL_DisablePeripherals(void);

/*******************************************************************************
 * Types
 ******************************************************************************/

struct arm_vector_table
{
    uint32_t msp;
    uint32_t reset;
};

static struct arm_vector_table *vt;

/*******************************************************************************
 * Code
 ******************************************************************************/

/* Starts selected application */

void do_boot(struct boot_rsp *rsp)
{
    uintptr_t flash_base;
    int rc;

    /* The beginning of the image is the ARM vector table, containing
     * the initial stack pointer address and the reset vector
     * consecutively. Manually set the stack pointer and jump into the
     * reset vector
     */
    rc = flash_device_base(rsp->br_flash_dev_id, &flash_base);
    assert(rc == 0);

#if defined(MCUBOOT_DIRECT_XIP) && defined(CONFIG_MCUBOOT_FLASH_REMAP_ENABLE)

    /* In case direct-xip mode and enabled flash remapping function check if
     * the secondary slot is chosen to boot. If so we have to modify boot_rsp
     * structure here and enable flash remapping just before the jumping to app.
     * Flash remapping function has to be disabled when bootloader starts.
     */

    if (rsp->br_image_off == (BOOT_FLASH_CAND_APP - BOOT_FLASH_BASE))
    {
        uintptr_t start, end, off;
        start = BOOT_FLASH_ACT_APP;
        end   = BOOT_FLASH_ACT_APP + (BOOT_FLASH_CAND_APP - BOOT_FLASH_ACT_APP);
        off   = BOOT_FLASH_CAND_APP - BOOT_FLASH_ACT_APP;

        SBL_EnableRemap(start, end, off);
        rsp->br_image_off = BOOT_FLASH_ACT_APP - BOOT_FLASH_BASE;
        PRINTF("Booting the secondary slot - flash remapping is enabled\n");
    }
    else
    {
        PRINTF("Booting the primary slot - flash remapping is disabled\n");
    }
#endif

    vt = (struct arm_vector_table *)(flash_base + rsp->br_image_off + rsp->br_hdr->ih_hdr_size);

    cleanup();

    __set_CONTROL(0);
    __set_MSP(vt->msp);
    __ISB();
    ((void (*)(void))vt->reset)();
}


/* Calls MCUBoot and executes image selected by the bootloader */

int sbl_boot_main(void)
{
    int rc = -1;
    struct boot_rsp rsp;

#ifdef CONFIG_BOOT_SIGNATURE
    CRYPTO_InitHardware();
#endif

    mflash_drv_init();

    BOOT_LOG_INF("Bootloader Version %s", BOOTLOADER_VERSION);

    rc = boot_go(&rsp);
    if (rc != 0)
    {
        BOOT_LOG_ERR("Unable to find bootable image");
        for (;;)
            ;
    }

#ifdef CONFIG_MCUBOOT_ENCRYPTED_XIP_SUPPORT
    BOOT_LOG_INF("\nStarting post-bootloader process of encrypted image...");
    if(mcuboot_process_encryption(&rsp) != kStatus_Success){
      BOOT_LOG_ERR("Fatal error: failed to process encrypted image");
      while(1)
        ;
    }
    BOOT_LOG_INF("Post-bootloader process of encrypted image successful\n");
#endif
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
