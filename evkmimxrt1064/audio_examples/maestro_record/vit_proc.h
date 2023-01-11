/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _VIT_PROC_H_
#define _VIT_PROC_H_

#include "PL_platformTypes_CortexM.h"
#include "VIT.h"

typedef int (*VIT_Initialize_T)(void *arg);
typedef int (*VIT_Execute_T)(void *arg, void *inputBuffer, int size);
typedef int (*VIT_Deinit_T)(void);

extern VIT_Initialize_T VIT_Initialize_func;
extern VIT_Execute_T VIT_Execute_func;
extern VIT_Deinit_T VIT_Deinit_func;

typedef enum
{
    EN,
    CN
} VIT_Language_T;
extern VIT_Language_T Vit_Language;
#endif
