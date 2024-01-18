/*
 * Copyright 2017, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SRTM_LFCL_SERVICE_H__
#define __SRTM_LFCL_SERVICE_H__

#include "srtm_service.h"

/*!
 * @addtogroup srtm_service
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/** @brief Switch to disable life cycle service debugging messages. */
#ifndef SRTM_LFCL_SERVICE_DEBUG_OFF
#define SRTM_LFCL_SERVICE_DEBUG_OFF (0)
#endif

#if SRTM_LFCL_SERVICE_DEBUG_OFF
#undef SRTM_DEBUG_VERBOSE_LEVEL
#define SRTM_DEBUG_VERBOSE_LEVEL SRTM_DEBUG_VERBOSE_NONE
#endif

/*! @brief SRTM life cycle service event code */
typedef enum _srtm_lfcl_event
{
    /* SRTM power mode event from peer core */
    SRTM_Lfcl_Event_Running     = 0x20U, /*!< Peer core indicates it is running */
    SRTM_Lfcl_Event_SuspendReq  = 0x21U, /*!< Peer core request to suspend */
    SRTM_Lfcl_Event_RebootReq   = 0x22U, /*!< Peer core request to reboot */
    SRTM_Lfcl_Event_ShutdownReq = 0x23U, /*!< Peer core request to shutdown */

    /* SRTM power mode event from lifecycle service */
    SRTM_Lfcl_Event_WakeupReq = 0x30U, /*!< Request peer core to exit suspend mode */

    /* SRTM heart beat event from peer core */
    SRTM_Lfcl_Event_HeartBeatEnable  = 0x40U, /*!< Peer core request to enable heart beat monitor */
    SRTM_Lfcl_Event_HeartBeatDisable = 0x41U, /*!< Peer core request to disable heart beat monitor */
    SRTM_Lfcl_Event_HeartBeat        = 0x50U, /*!< Peer core heart beat event */
} srtm_lfcl_event_t;

/**
 * @brief SRTM life cycle service callback function type.
 */
typedef srtm_status_t (*srtm_lfcl_service_cb_t)(
    srtm_service_t service, srtm_peercore_t core, srtm_lfcl_event_t event, void *eventParam, void *userParam);

/*******************************************************************************
 * API
 ******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Create life cycle service.
 *
 * @return SRTM service handle on success and NULL on failure.
 */
srtm_service_t SRTM_LfclService_Create(void);

/*!
 * @brief Destroy life cycle service.
 *
 * @param service SRTM service to destroy.
 */
void SRTM_LfclService_Destroy(srtm_service_t service);

/*!
 * @brief Subscribe lifecycle service callback.
 *
 * @param service SRTM life cycle service.
 * @param callback User function to handle life cycle event.
 * @param param User parameter to be used in callback.
 * @return SRTM_Status_Success on success and others on failure.
 */
srtm_status_t SRTM_LfclService_Subscribe(srtm_service_t service, srtm_lfcl_service_cb_t callback, void *param);

/*!
 * @brief Unsubscribe lifecycle service callback.
 *
 * @param service SRTM life cycle service.
 * @param callback User function to handle life cycle event.
 * @param param User parameter to be used in callback.
 * @return SRTM_Status_Success on success and others on failure.
 */
srtm_status_t SRTM_LfclService_Unsubscribe(srtm_service_t service, srtm_lfcl_service_cb_t callback, void *param);

#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* __SRTM_LFCL_SERVICE_H__ */
