/*
 * Copyright 2022-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

#ifndef __NCP_INTF_PM_H__
#define __NCP_INTF_PM_H__
#include "fsl_common.h"

/* Power Mode Index */
#define NCP_PM_STATE_PM0           (0U)
#define NCP_PM_STATE_PM1           (1U)
#define NCP_PM_STATE_PM2           (2U)
#define NCP_PM_STATE_PM3           (3U)
#define NCP_PM_STATE_PM4           (4U)

/* NCP PM status */
typedef enum _ncp_pm_status
{
    NCP_PM_STATUS_ERROR      = -1,
    NCP_PM_STATUS_NOT_READY  = -2,
    NCP_PM_STATUS_SUCCESS    = 0,
} ncp_pm_status_t;

typedef struct _ncp_intf_pm_ops
{
    int (*enter)(int32_t pm_state);
    int (*exit)(int32_t pm_state);
} ncp_intf_pm_ops_t;

int ncp_intf_pm_enter(int32_t pm_state);
int ncp_intf_pm_exit(int32_t pm_state);

#endif /* __NCP_INTF_PM_H__ */
