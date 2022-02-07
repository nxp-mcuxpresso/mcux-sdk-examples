/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if (__ARM_FEATURE_CMSE & 1) == 0
#error "Need ARMv8-M security extensions"
#elif (__ARM_FEATURE_CMSE & 2) == 0
#error "Compile with --cmse"
#endif

#include "arm_cmse.h"
#include "veneer_table.h"

/*******************************************************************************
 * This example doesn't use any function called from secure world.
 ******************************************************************************/
