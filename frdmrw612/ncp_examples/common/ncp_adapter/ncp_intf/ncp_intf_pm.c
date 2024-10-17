/*
 * Copyright 2022-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

#include "ncp_intf_pm.h"
#include "ncp_tlv_adapter.h"

extern ncp_tlv_adapter_t ncp_tlv_adapter;
uint8_t mcu_device_status = 1;

int ncp_intf_pm_enter(int32_t pm_state)
{
    if (0 != ncp_tlv_adapter.intf_ops->pm_ops)
        return ncp_tlv_adapter.intf_ops->pm_ops->enter(pm_state);
    return 0;
}

int ncp_intf_pm_exit(int32_t pm_state)
{
    if (0 != ncp_tlv_adapter.intf_ops->pm_ops)
        return ncp_tlv_adapter.intf_ops->pm_ops->exit(pm_state);
    return 0;
}
