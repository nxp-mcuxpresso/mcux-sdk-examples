/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 */

#ifndef _LITTLEFS_ADAPTER_H_
#define _LITTLEFS_ADAPTER_H_

/***********************************************************************************************************************
 * User includes
 **********************************************************************************************************************/
#include "mflash_drv.h"

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
//#define LITTLEFS_START_ADDR 0x900000
#define LITTLEFS_START_ADDR 0xC00000

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
/* Erasable block count definition */
#define LITTLEFS_BLOCK_COUNT 16
/* Minimum block cache size definition */
#define LITTLEFS_CACHE_SIZE 256
/* Minimum lookahead buffer size definition */
#define LITTLEFS_LOOKAHEAD_SIZE 16

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
/* LittleFS context */
extern struct lfs_mflash_ctx LittleFS_ctx;
/* LittleFS configuration */
extern const struct lfs_config LittleFS_config;

struct lfs_mflash_ctx
{
    uint32_t start_addr;
};

/***********************************************************************************************************************
 * API functions
 **********************************************************************************************************************/
/* LittleFS read a block region callback*/
int lfs_mflash_read(const struct lfs_config *, lfs_block_t, lfs_off_t, void *, lfs_size_t);
/* LittleFS program a block region callback*/
int lfs_mflash_prog(const struct lfs_config *, lfs_block_t, lfs_off_t, const void *, lfs_size_t);
/* LittleFS erase a block callback*/
int lfs_mflash_erase(const struct lfs_config *, lfs_block_t);
/* LittleFS state sync callback*/
int lfs_mflash_sync(const struct lfs_config *);

#ifdef LFS_THREADSAFE
/* LittleFS lock/unlock callback*/
int lfs_create_lock(void);
int lfs_lock(const struct lfs_config *lfsc);
int lfs_unlock(const struct lfs_config *lfsc);
#endif

int lfs_get_default_config(struct lfs_config *lfsc);

/* LittleFS read/write API */
int lfs_save_file(char *path, unsigned int flag, int offset, unsigned char *buf, int len);
int lfs_load_file(char *path, unsigned int flag, int offset, unsigned char *buf, int len);

int littlefs_init(void);

#if defined(__cplusplus)
}
#endif

#endif /* _LITTLEFS_ADAPTER_H_ */
