/*
 * Copyright 2017, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SRTM_PMIC_SERVICE_H__
#define __SRTM_PMIC_SERVICE_H__

#include "srtm_service.h"

/*!
 * @addtogroup srtm_service
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/** @brief Switch to disable PMIC service debugging messages. */
#ifndef SRTM_PMIC_SERVICE_DEBUG_OFF
#define SRTM_PMIC_SERVICE_DEBUG_OFF (0)
#endif

#if SRTM_PMIC_SERVICE_DEBUG_OFF
#undef SRTM_DEBUG_VERBOSE_LEVEL
#define SRTM_DEBUG_VERBOSE_LEVEL SRTM_DEBUG_VERBOSE_NONE
#endif

/**
 * @brief SRTM PMIC adapter structure pointer.
 */
typedef struct _srtm_pmic_adapter *srtm_pmic_adapter_t;

/**
 * @brief SRTM PMIC adapter structure
 */
struct _srtm_pmic_adapter
{
    srtm_status_t (*enable)(srtm_pmic_adapter_t adapter, uint8_t regulator);
    srtm_status_t (*disable)(srtm_pmic_adapter_t adapter, uint8_t regulator);
    bool (*isEnabled)(srtm_pmic_adapter_t adapter, uint8_t regulator);
    srtm_status_t (*setVoltage)(srtm_pmic_adapter_t adapter, uint8_t regulator, uint32_t volt);
    srtm_status_t (*getVoltage)(srtm_pmic_adapter_t adapter, uint8_t regulator, uint32_t *pVolt);
    srtm_status_t (*setRegister)(srtm_pmic_adapter_t adapter, uint8_t reg, uint32_t value);
    srtm_status_t (*getRegister)(srtm_pmic_adapter_t adapter, uint8_t reg, uint32_t *pValue);
    srtm_status_t (*setStandbyVoltage)(srtm_pmic_adapter_t adapter, uint8_t regulator, uint32_t volt);
};

/**
 * @brief SRTM PMIC payload structure
 */
SRTM_PACKED_BEGIN struct _srtm_pmic_payload
{
    uint8_t reg;
    uint8_t retCode;
    uint8_t status;
    uint32_t value;
} SRTM_PACKED_END;

/*******************************************************************************
 * API
 ******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Create PMIC service.
 *
 * @param adapter PMIC adapter to handle real pmic operations.
 * @return SRTM service handle on success and NULL on failure.
 */
srtm_service_t SRTM_PmicService_Create(srtm_pmic_adapter_t adapter);

/*!
 * @brief Destroy PMIC service.
 *
 * @param service SRTM service to destroy.
 */
void SRTM_PmicService_Destroy(srtm_service_t service);

#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* __SRTM_PMIC_SERVICE_H__ */
