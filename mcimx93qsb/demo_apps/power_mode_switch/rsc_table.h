/*
 * Copyright 2022 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* This file populates resource table for BM remote
 * for use by the Linux Master */

#ifndef RSC_TABLE_H_
#define RSC_TABLE_H_

#include <stddef.h>
#include <remoteproc.h>

#if defined __cplusplus
extern "C" {
#endif

#define NO_RESOURCE_ENTRIES (2)
#define RSC_VDEV_FEATURE_NS (1) /* Support name service announcement */

#define RESOURCE_TABLE_START 0x2001E000U
#define RESOURCE_TABLE_SIZE  0x1000U

/* Resource table for the given remote */
METAL_PACKED_BEGIN
struct remote_resource_table
{
    uint32_t version;
    uint32_t num;
    uint32_t reserved[2];
    uint32_t offset[NO_RESOURCE_ENTRIES];

    /* rpmsg vdev entry for srtm communication */
    struct fw_rsc_vdev srtm_vdev;
    struct fw_rsc_vdev_vring srtm_vring0;
    struct fw_rsc_vdev_vring srtm_vring1;
    /* rpmsg vdev entry for user app communication */
    struct fw_rsc_vdev user_vdev;
    struct fw_rsc_vdev_vring user_vring0;
    struct fw_rsc_vdev_vring user_vring1;
} METAL_PACKED_END;

/*
 * Copy resource table to shared memory base for early M Core boot case.
 * In M Core early boot case, Linux kernel need to get resource table before file system gets loaded.
 */
void copyResourceTable(void);

#if defined __cplusplus
}
#endif

#endif /* RSC_TABLE_H_ */
