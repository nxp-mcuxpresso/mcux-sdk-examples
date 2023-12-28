/*
 * Copyright 2017, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "srtm_pf1550_adapter.h"
#include "srtm_heap.h"

#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* PF1550 adapter */
typedef struct _srtm_pf1550_adapter
{
    struct _srtm_pmic_adapter adapter;
    pf1550_handle_t *driver;
} *srtm_pf1550_adapter_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const pf1550_module_t pf1550ModuleMap[] = {
    kPF1550_ModuleSwitch1, kPF1550_ModuleSwitch2, kPF1550_ModuleSwitch3, kPF1550_ModuleVrefDdr,
    kPF1550_ModuleLdo1,    kPF1550_ModuleLdo2,    kPF1550_ModuleLdo3,
};

/*******************************************************************************
 * Code
 ******************************************************************************/
static srtm_status_t SRTM_Pf1550Adapter_Enable(srtm_pmic_adapter_t adapter, uint8_t regulator)
{
    srtm_pf1550_adapter_t handle = (srtm_pf1550_adapter_t)(void *)adapter;

    assert(handle->driver);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s: %d\r\n", __func__, regulator);

    if (regulator >= ARRAY_SIZE(pf1550ModuleMap))
    {
        return SRTM_Status_InvalidParameter;
    }

    PF1550_EnableRegulator(handle->driver, pf1550ModuleMap[regulator], kPF1550_OperatingStatusRun, true);

    return SRTM_Status_Success;
}

static srtm_status_t SRTM_Pf1550Adapter_Disable(srtm_pmic_adapter_t adapter, uint8_t regulator)
{
    srtm_pf1550_adapter_t handle = (srtm_pf1550_adapter_t)(void *)adapter;

    assert(handle->driver);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s: %d\r\n", __func__, regulator);

    if (regulator >= ARRAY_SIZE(pf1550ModuleMap))
    {
        return SRTM_Status_InvalidParameter;
    }

    PF1550_EnableRegulator(handle->driver, pf1550ModuleMap[regulator], kPF1550_OperatingStatusRun, false);

    return SRTM_Status_Success;
}

static bool SRTM_Pf1550Adapter_IsEnabled(srtm_pmic_adapter_t adapter, uint8_t regulator)
{
    srtm_pf1550_adapter_t handle = (srtm_pf1550_adapter_t)(void *)adapter;

    assert(handle->driver);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s: %d\r\n", __func__, regulator);

    if (regulator >= ARRAY_SIZE(pf1550ModuleMap))
    {
        return false;
    }

    return PF1550_IsRegulatorEnabled(handle->driver, pf1550ModuleMap[regulator], kPF1550_OperatingStatusRun);
}

static srtm_status_t SRTM_Pf1550Adapter_SetVoltage(srtm_pmic_adapter_t adapter, uint8_t regulator, uint32_t volt)
{
    srtm_pf1550_adapter_t handle = (srtm_pf1550_adapter_t)(void *)adapter;

    assert(handle->driver);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s: %d %d\r\n", __func__, regulator, volt);

    if (regulator >= ARRAY_SIZE(pf1550ModuleMap))
    {
        return SRTM_Status_InvalidParameter;
    }

    PF1550_SetRegulatorOutputVoltage(handle->driver, pf1550ModuleMap[regulator], kPF1550_OperatingStatusRun, volt);

    return SRTM_Status_Success;
}

static srtm_status_t SRTM_Pf1550Adapter_GetVoltage(srtm_pmic_adapter_t adapter, uint8_t regulator, uint32_t *pVolt)
{
    srtm_pf1550_adapter_t handle = (srtm_pf1550_adapter_t)(void *)adapter;

    assert(handle->driver);
    assert(pVolt);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s: %d\r\n", __func__, regulator);

    if (regulator >= ARRAY_SIZE(pf1550ModuleMap))
    {
        return SRTM_Status_InvalidParameter;
    }

    *pVolt = PF1550_GetRegulatorOutputVoltage(handle->driver, pf1550ModuleMap[regulator], kPF1550_OperatingStatusRun);

    return SRTM_Status_Success;
}

static srtm_status_t SRTM_Pf1550Adapter_SetRegister(srtm_pmic_adapter_t adapter, uint8_t reg, uint32_t value)
{
    srtm_pf1550_adapter_t handle = (srtm_pf1550_adapter_t)(void *)adapter;

    assert(handle->driver);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s: %d %d\r\n", __func__, reg, value);

    return PF1550_WriteReg(handle->driver, reg, (uint8_t)value) ? SRTM_Status_Success : SRTM_Status_Error;
}

static srtm_status_t SRTM_Pf1550Adapter_GetRegister(srtm_pmic_adapter_t adapter, uint8_t reg, uint32_t *pValue)
{
    srtm_pf1550_adapter_t handle = (srtm_pf1550_adapter_t)(void *)adapter;

    assert(handle->driver);
    assert(pValue);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s: %d\r\n", __func__, reg);

    return PF1550_DumpReg(handle->driver, reg, (uint8_t *)pValue, 1U) ? SRTM_Status_Success : SRTM_Status_Error;
}

static srtm_status_t SRTM_Pf1550Adapter_SetStandbyVoltage(srtm_pmic_adapter_t adapter, uint8_t regulator, uint32_t volt)
{
    srtm_pf1550_adapter_t handle = (srtm_pf1550_adapter_t)(void *)adapter;

    assert(handle->driver);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s: %d %d\r\n", __func__, regulator, volt);

    if (regulator >= ARRAY_SIZE(pf1550ModuleMap))
    {
        return SRTM_Status_InvalidParameter;
    }

    if (volt == 0)
    {
        PF1550_EnableRegulator(handle->driver, pf1550ModuleMap[regulator], kPF1550_OperatingStatusStandby, false);
    }
    else
    {
        PF1550_SetRegulatorOutputVoltage(handle->driver, pf1550ModuleMap[regulator], kPF1550_OperatingStatusStandby,
                                         volt);
    }

    return SRTM_Status_Success;
}

srtm_pmic_adapter_t SRTM_Pf1550Adapter_Create(pf1550_handle_t *driver)
{
    srtm_pf1550_adapter_t handle;

    assert(driver);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    handle = (srtm_pf1550_adapter_t)SRTM_Heap_Malloc(sizeof(struct _srtm_pf1550_adapter));
    assert(handle);

    handle->driver = driver;

    handle->adapter.enable            = SRTM_Pf1550Adapter_Enable;
    handle->adapter.disable           = SRTM_Pf1550Adapter_Disable;
    handle->adapter.isEnabled         = SRTM_Pf1550Adapter_IsEnabled;
    handle->adapter.setVoltage        = SRTM_Pf1550Adapter_SetVoltage;
    handle->adapter.getVoltage        = SRTM_Pf1550Adapter_GetVoltage;
    handle->adapter.setRegister       = SRTM_Pf1550Adapter_SetRegister;
    handle->adapter.getRegister       = SRTM_Pf1550Adapter_GetRegister;
    handle->adapter.setStandbyVoltage = SRTM_Pf1550Adapter_SetStandbyVoltage;

    return &handle->adapter;
}

void SRTM_Pf1550Adapter_Destroy(srtm_pmic_adapter_t adapter)
{
    srtm_pf1550_adapter_t handle = (srtm_pf1550_adapter_t)adapter;

    assert(adapter);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    SRTM_Heap_Free(handle);
}
