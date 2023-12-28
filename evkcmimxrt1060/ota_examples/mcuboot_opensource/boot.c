/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "boot.h"
#include "sbl.h"
#include "flash_partitioning.h"
#include "fsl_debug_console.h"

#ifdef NDEBUG
#undef assert
#define assert(x) ((void)(x))
#endif

iapfun jump2app;

#ifdef CONFIG_MCUBOOT_FLASH_REMAP_ENABLE
extern void SBL_EnableRemap(uint32_t start_addr, uint32_t end_addr, uint32_t off);
extern void SBL_DisableRemap(void);
#endif

struct arm_vector_table
{
    uint32_t msp;
    uint32_t reset;
};

static struct arm_vector_table *vt;

#pragma weak cleanup
void cleanup(void);

/* The bootloader of MCUboot */
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

    vt = (struct arm_vector_table *)(flash_base + rsp->br_image_off +
#ifdef MCUBOOT_SIGN_ROM
                                     HAB_IVT_OFFSET +
#endif
                                     rsp->br_hdr->ih_hdr_size);

    cleanup();

    __set_CONTROL(0);
    __set_MSP(vt->msp);
    __ISB();
    ((void (*)(void))vt->reset)();
}
