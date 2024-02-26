/*
 * Copyright 2018-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _LFS_MFLASH_H_
#define _LFS_MFLASH_H_

#include "lfs.h"
#include "mflash_drv.h"

struct lfs_mflash_ctx
{
    uint32_t start_addr;
};

extern int lfs_get_default_config(struct lfs_config *lfsc);
extern int lfs_storage_init(const struct lfs_config *lfsc);

#endif
