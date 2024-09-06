/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/***********************************************************************************************************************
 * Included files
 **********************************************************************************************************************/
#include "peripherals.h"

/***********************************************************************************************************************
 * LittleFS initialization code
 **********************************************************************************************************************/
/* LittleFS context */
extern struct lfs_mflash_ctx LittleFS_ctx;
const struct lfs_config      LittleFS_config = {.context = (void *)&LittleFS_ctx,
                                                .read    = lfs_mflash_read,
                                                .prog    = lfs_mflash_prog,
                                                .erase   = lfs_mflash_erase,
                                                .sync    = lfs_mflash_sync,
#ifdef LFS_THREADSAFE
                                           .lock   = lfs_mutex_lock,
                                           .unlock = lfs_mutex_unlock,
#endif
                                           .read_size      = LITTLEFS_READ_SIZE,
                                           .prog_size      = LITTLEFS_PROG_SIZE,
                                           .block_size     = LITTLEFS_BLOCK_SIZE,
                                           .block_count    = LITTLEFS_BLOCK_COUNT,
                                           .block_cycles   = LITTLEFS_BLOCK_CYCLES,
                                           .cache_size     = LITTLEFS_CACHE_SIZE,
                                           .lookahead_size = LITTLEFS_LOOKAHEAD_SIZE};

/* Empty initialization function (commented out)
static void LittleFS_init(void) {
} */
