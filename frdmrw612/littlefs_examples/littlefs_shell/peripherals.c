/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/***********************************************************************************************************************
 * Included files
 **********************************************************************************************************************/
#include "peripherals.h"

/***********************************************************************************************************************
 * BOARD_InitPeripherals functional group
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * LittleFS initialization code
 **********************************************************************************************************************/
const struct lfs_config LittleFS_config = {
  .context = (void*)&LittleFS_ctx,
  .read = lfs_mflash_read,
  .prog = lfs_mflash_prog,
  .erase = lfs_mflash_erase,
  .sync = lfs_mflash_sync,
  .read_size = LITTLEFS_READ_SIZE,
  .prog_size = LITTLEFS_PROG_SIZE,
  .block_size = LITTLEFS_BLOCK_SIZE,
  .block_count = 1024,
  .block_cycles = 100,
  .cache_size = LITTLEFS_CACHE_SIZE,
  .lookahead_size = LITTLEFS_LOOKAHEAD_SIZE
};

/* Empty initialization function (commented out)
static void LittleFS_init(void) {
} */

/***********************************************************************************************************************
 * Initialization functions
 **********************************************************************************************************************/
void BOARD_InitPeripherals(void)
{
  /* Initialize components */
}

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/
void BOARD_InitBootPeripherals(void)
{
  BOARD_InitPeripherals();
}
