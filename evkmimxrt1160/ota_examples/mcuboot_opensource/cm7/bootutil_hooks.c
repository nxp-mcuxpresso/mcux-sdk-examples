/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2017 NXP. Not a Contribution
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "bootutil/bootutil_public.h"
#include "bootutil/boot_hooks.h"
#include "bootutil_priv.h"

#include "fsl_debug_console.h"

#ifdef NDEBUG
#undef assert
#define assert(x) ((void)(x))
#endif

int boot_read_image_header_hook(int img_index, int slot, struct image_header *img_head)
{
    return BOOT_HOOK_REGULAR;
}

fih_int boot_image_check_hook(int img_index, int slot)
{
    return BOOT_HOOK_REGULAR;
}

int boot_perform_update_hook(int img_index, struct image_header *img_head, const struct flash_area *area)
{
    return BOOT_HOOK_REGULAR;
}

int boot_copy_region_post_hook(int img_index, const struct flash_area *area, size_t size)
{
    return 0;
}

int boot_read_swap_state_primary_slot_hook(int image_index, struct boot_swap_state *state)
{
    return BOOT_HOOK_REGULAR;
}

int boot_find_active_slot_hook(struct boot_loader_state *state, uint32_t *candidate_slot)
{
#if defined(MCUBOOT_IMAGE_ACCESS_HOOKS) && defined(MCUBOOT_DIRECT_XIP)

    uint32_t slot;
    int rc;
    struct boot_swap_state swap_states[BOOT_NUM_SLOTS];
    uint8_t image_index = BOOT_CURR_IMG(state);

    rc = boot_read_swap_state_by_id(FLASH_AREA_IMAGE_PRIMARY(image_index), &swap_states[BOOT_PRIMARY_SLOT]);
    assert(rc == 0);
    rc = boot_read_swap_state_by_id(FLASH_AREA_IMAGE_SECONDARY(image_index), &swap_states[BOOT_SECONDARY_SLOT]);
    assert(rc == 0);

    for (slot = 0; slot < BOOT_NUM_SLOTS; slot++)
    {
        /* is slot in test state or marked as permanent (image_ok = set)? */
        if (state->slot_usage[BOOT_CURR_IMG(state)].slot_available[slot] && swap_states[slot].magic == 0x1)
        {
            if (swap_states[slot].copy_done != BOOT_FLAG_SET)
            {
                *candidate_slot = slot;
                PRINTF("Found a candidate in slot %X\n", slot);
                return 0;
            }
        }
    }

#endif
    return BOOT_HOOK_REGULAR;
}
