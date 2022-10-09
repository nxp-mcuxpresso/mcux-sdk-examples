/*! *********************************************************************************
* Copyright  2016-2020 NXP
* All rights reserved.
*
* \file
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef _POWER_MANAGER_H_
#define _POWER_MANAGER_H_

#include "fsl_common.h"
#include "fsl_power.h"

/*! @addtogroup power_manager */
/*! @{ */

/*! @file */

/*! @brief bit definition for power mode requsets */
enum
{
    kPmReqUserPolicy = (1U << 0U),   /*!< request from user app policy */
    kPmReqBle = (1U << 1U),          /*!< request from ble */
    kPmReqGpio = (1U << 2U),         /*!< request from GPIO */
    kPmReqUserEvents = (1U << 3U),   /*!< request from user event */
    kPmReqCTIMER0 = (1U << 4U),      /*!< request from CTIMER0 */
    kPmReqCTIMER1 = (1U << 5U),      /*!< request from CTIMER1 */
    kPmReqCTIMER2 = (1U << 6U),      /*!< request from CTIMER2 */
    kPmReqCTIMER3 = (1U << 7U),      /*!< request from CTIMER3 */
    kPmReqRTC = (1U << 8U),          /*!< request from RTC */
    kPmReqTimerManager = (1U << 9U), /*!< request from Timer Manager */
    kPmReqAdc = (1U << 10U),         /*!< request from ADC */
};

/*! @brief Power manager environment */
typedef struct _power_manager_env
{
    uint32_t req[4U]; /*!< hold power mode requests made by PM_SetReq() */
} power_manager_env_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern power_manager_env_t g_PmEnv;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Request the chip to stay in Active/Sleep/Power Down0 mode.
 */
static inline void PM_SetReq(power_mode_t pm, uint32_t bits)
{
    uint32_t regPrimask = DisableGlobalIRQ();

    g_PmEnv.req[kPmActive] &= ~bits;
    g_PmEnv.req[kPmSleep] &= ~bits;
    g_PmEnv.req[kPmPowerDown0] &= ~bits;
    g_PmEnv.req[pm] |= bits;

    EnableGlobalIRQ(regPrimask);
}

/*!
 * @brief Clear the request for staying in Active/Sleep/Power Down0 mode.
 */
static inline void PM_ClrReq(uint32_t bits)
{
    uint32_t regPrimask = DisableGlobalIRQ();

    g_PmEnv.req[kPmActive] &= ~bits;
    g_PmEnv.req[kPmSleep] &= ~bits;
    g_PmEnv.req[kPmPowerDown0] &= ~bits;

    EnableGlobalIRQ(regPrimask);
}

/*!
 * @brief Power Manager's initialization.
 */
void PM_Init(void);

/*!
 * @brief Power management.
 */
void PM_PowerManagement(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/*! @} */

#endif /* _POWER_MANAGER_H_ */

/**
 * @}
 */
