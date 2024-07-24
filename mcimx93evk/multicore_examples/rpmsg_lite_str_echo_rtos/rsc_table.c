/*
 * Copyright 2022 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* This file populates resource table for BM remote
 * for use by the Linux Master */

#include <string.h>
#include "board.h"
#include "rsc_table.h"
#include "rpmsg_lite.h"

#define NUM_VRINGS 0x02

/* Place resource table in special ELF section */
#if defined(__ARMCC_VERSION) || defined(__GNUC__)
__attribute__((section(".resource_table")))
#elif defined(__ICCARM__)
#pragma location = ".resource_table"
#else
#error Compiler not supported!
#endif
const struct remote_resource_table resources = {
    /* Version */
    1,

    /* NUmber of table entries */
    NO_RESOURCE_ENTRIES,

    /* reserved fields */
    {
        0,
        0,
    },

    /* Offsets of rsc entries */
    {
        offsetof(struct remote_resource_table, srtm_vdev),
        offsetof(struct remote_resource_table, user_vdev),
    },

    /* SRTM virtio device entry */
    {
        RSC_VDEV,
        7,
        0,
        RSC_VDEV_FEATURE_NS,
        0,
        0,
        0,
        NUM_VRINGS,
        {0, 0},
    },

    /* Vring rsc entry - part of vdev rsc entry */
    {VDEV0_VRING_BASE, VRING_ALIGN, RL_BUFFER_COUNT, 0, 0},
    {VDEV0_VRING_BASE + VRING_SIZE, VRING_ALIGN, RL_BUFFER_COUNT, 1, 0},

    /* SRTM virtio device entry */
    {
        RSC_VDEV,
        7,
        1,
        RSC_VDEV_FEATURE_NS,
        0,
        0,
        0,
        NUM_VRINGS,
        {0, 0},
    },

    /* Vring rsc entry - part of vdev rsc entry */
    {VDEV1_VRING_BASE, VRING_ALIGN, RL_BUFFER_COUNT, 2, 0},
    {VDEV1_VRING_BASE + VRING_SIZE, VRING_ALIGN, RL_BUFFER_COUNT, 3, 0},
};

void copyResourceTable(void)
{
    /* TCM ECC is enabled in default. Must initialize to use. */
    memset((void *)RESOURCE_TABLE_START, 0, RESOURCE_TABLE_SIZE);
    memcpy((void *)RESOURCE_TABLE_START, &resources, sizeof(resources));
}
