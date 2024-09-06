/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PERIPHERALS_H_
#define _PERIPHERALS_H_

/***********************************************************************************************************************
 * Included files
 **********************************************************************************************************************/
#include "fsl_common.h"
#include "lfs.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/***********************************************************************************************************************
 * User definitions
 **********************************************************************************************************************/
/*
This littlefs configuration is used by OT, OT LITTLE FS is close to m_text_end in the linker
EDGEFAST_LITTLEFS_REGION_SIZE = 0x200000
OT_LITTLEFS_REGION_SIZE = 0x10000
WIFI_LITTLEFS_REGION_SIZE = 0x10000
RESERVED_FOR_USE_REGION_SIZE = 0x1000
LITTLEFS_REGION_SIZE = OT_LITTLEFS_REGION_SIZE + WIFI_LITTLEFS_REGION_SIZE + RESERVED_FOR_USE_REGION_SIZE + EDGEFAST_LITTLEFS_REGION_SIZE
m_text_start = 0x08001280
m_text_size  = 0x03FFED80 - LITTLEFS_REGION_SIZE
*/
#define LITTLEFS_START_ADDR (uint32_t)(0x3DDF000)

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/
/* Definitions for BOARD_InitPeripherals functional group */
/* Minimum block read size definition */
#define LITTLEFS_READ_SIZE 16
/* Minimum block program size definition */
#define LITTLEFS_PROG_SIZE 256
/* Erasable block size definition */
#define LITTLEFS_BLOCK_SIZE 4096
/* Block count definition */
#define LITTLEFS_BLOCK_COUNT 16
/* Block cycles definition */
#define LITTLEFS_BLOCK_CYCLES 100
/* Minimum block cache size definition */
#define LITTLEFS_CACHE_SIZE 256
/* Minimum lookahead buffer size definition */
#define LITTLEFS_LOOKAHEAD_SIZE 16

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
/* LittleFS configuration */
extern const struct lfs_config LittleFS_config;

/***********************************************************************************************************************
 * Callback functions
 **********************************************************************************************************************/
/* LittleFS read a block region callback*/
extern int lfs_mflash_read(const struct lfs_config *, lfs_block_t, lfs_off_t, void *, lfs_size_t);
/* LittleFS program a block region callback*/
extern int lfs_mflash_prog(const struct lfs_config *, lfs_block_t, lfs_off_t, const void *, lfs_size_t);
/* LittleFS erase a block callback*/
extern int lfs_mflash_erase(const struct lfs_config *, lfs_block_t);
/* LittleFS state sync callback*/
extern int lfs_mflash_sync(const struct lfs_config *);
/* LittleFS state lock callback*/
extern int lfs_mutex_lock(const struct lfs_config *);
/* LittleFS state unlock callback*/
extern int lfs_mutex_unlock(const struct lfs_config *);

#if defined(__cplusplus)
}
#endif

#endif /* _PERIPHERALS_H_ */
