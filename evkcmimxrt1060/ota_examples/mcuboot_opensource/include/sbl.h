/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SBL_H__
#define __SBL_H__

#include <sblconfig.h>
#include <sbldef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * boot interface
 */
int sbl_boot_main(void);

#ifdef __cplusplus
}
#endif

#endif
