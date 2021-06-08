/*
 * Copyright 2019, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __SRTM_MU_ENDPOINT_H__
#define __SRTM_MU_ENDPOINT_H__

#include "srtm_channel_struct.h"
#include "srtm_channel.h"
#include "fsl_mu.h"
/*!
 * @addtogroup srtm_channel
 * @{
 */
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/**
 * @brief SRTM RPMsg endpoint channel configuration fields
 */
typedef struct _srtm_mu_endpoint_config
{
    MU_Type *mu;
    IRQn_Type mu_nvic_irq;
} srtm_mu_endpoint_config_t;

typedef enum _mu_msg_type
{
    SRTM_MU_MSG_REQ = 0x1,
    SRTM_MU_MSG_RESP,
    SRTM_MU_MSG_READY_A = 0x3,
    SRTM_MU_MSG_READY_B,
} mu_msg_type_t;

/*******************************************************************************
 * API
 ******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
/*!
 * @brief Create MU endpoint channel.
 *
 * @param config SRTM MU endpoint configuration.
 * @return SRTM channel handle on success and NULL on failure.
 */
srtm_channel_t SRTM_MUEndpoint_Create(srtm_mu_endpoint_config_t *config);

/*!
 * @brief Destroy MU endpoint channel.
 *
 * @param channel SRTM channel to destroy.
 */
void SRTM_MUEndpoint_Destroy(srtm_channel_t channel);

/*!
 * @brief Callback for MU ISR
 *
 * @param
 */
void SRTM_MUEndpoint_Handler(void);

#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* __SRTM_MU_ENDPOINT_H__ */
