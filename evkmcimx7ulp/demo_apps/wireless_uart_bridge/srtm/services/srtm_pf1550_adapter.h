/*
 * Copyright 2017, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SRTM_PF1550_ADAPTER_H__
#define __SRTM_PF1550_ADAPTER_H__

#include "srtm_pmic_service.h"
#include "fsl_pf1550.h"

/*!
 * @addtogroup srtm_service
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Create PF1550 adapter.
 *
 * @param driver PF1550 driver handle.
 * @return SRTM PMIC adapter on success or NULL on failure.
 */
srtm_pmic_adapter_t SRTM_Pf1550Adapter_Create(pf1550_handle_t *driver);

/*!
 * @brief Destroy PF1550 adapter.
 *
 * @param adapter PF1550 adapter to destroy.
 */
void SRTM_Pf1550Adapter_Destroy(srtm_pmic_adapter_t adapter);

#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* __SRTM_PF1550_ADAPTER_H__ */
