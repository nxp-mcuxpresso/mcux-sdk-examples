/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _DEMO_COMMON_H_
#define _DEMO_COMMON_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Core 0 sends commands to core 1. */
#define DEMO_CMD_LOCK_GATE(x)   (((x) << 16U) | 1U)
#define DEMO_CMD_UNLOCK_GATE(x) (((x) << 16U) | 2U)

#define DEMO_IS_LOCK_CMD(cmd)       (((cmd)&0x000000FFU) == 1U)
#define DEMO_IS_UNLOCK_CMD(cmd)     (((cmd)&0x000000FFU) == 2U)
#define DEMO_GET_LOCK_CMD_GATE(cmd) (((cmd)&0x00FF0000U) >> 16U)

/* Core 1 sends state to core 0. */
#define DEMO_STAT_CORE1_READY      0xFFFFFFFFU
#define DEMO_STAT_GATE_LOCKED(x)   (((x) << 16U) | 1U)
#define DEMO_STAT_GATE_UNLOCKED(x) (((x) << 16U) | 2U)

#define DEMO_MU_CH 0

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * APIs
 ******************************************************************************/

#endif /* _DEMO_COMMON_H_ */
