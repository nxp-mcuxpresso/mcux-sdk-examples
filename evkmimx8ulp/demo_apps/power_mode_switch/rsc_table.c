/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2015 Xilinx, Inc. All rights reserved.
 * Copyright 2020 NXP.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* This file populates resource table for BM remote
 * for use by the Linux Master */

#include "board.h"
#include "rsc_table.h"
#include "rpmsg_lite.h"
#include <string.h>

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
    {VDEV0_VRING_BASE, RL_VRING_ALIGN_M33_A35_COM, RL_BUFFER_COUNT(0), 0, 0},
    {VDEV0_VRING_BASE + RL_VRING_SIZE_M33_A35_COM, RL_VRING_ALIGN_M33_A35_COM, RL_BUFFER_COUNT(0), 1, 0},

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
    {VDEV1_VRING_BASE, RL_VRING_ALIGN_M33_A35_COM, RL_BUFFER_COUNT(1), 2, 0},
    {VDEV1_VRING_BASE + RL_VRING_SIZE_M33_A35_COM, RL_VRING_ALIGN_M33_A35_COM, RL_BUFFER_COUNT(1), 3, 0},
};

void copyResourceTable(void)
{
    /* On startup, DDR might not be enabled when M4 does resource table copy.
       So we cannot copy to VDEV0_VRING_BASE. Here we take M4 suspend area
       to copy the resource table. */
    memcpy((void *)0x0FFF8000U, &resources, sizeof(resources));
}
