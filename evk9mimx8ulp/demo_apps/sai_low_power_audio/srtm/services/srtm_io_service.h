/*
 * Copyright 2017, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SRTM_IO_SERVICE_H__
#define __SRTM_IO_SERVICE_H__

#include "srtm_service.h"

/*!
 * @addtogroup srtm_service
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/** @brief Switch to disable IO service debugging messages. */
#ifndef SRTM_IO_SERVICE_DEBUG_OFF
#define SRTM_IO_SERVICE_DEBUG_OFF (0)
#endif

#if SRTM_IO_SERVICE_DEBUG_OFF
#undef SRTM_DEBUG_VERBOSE_LEVEL
#define SRTM_DEBUG_VERBOSE_LEVEL SRTM_DEBUG_VERBOSE_NONE
#endif

/*! @brief SRTM IO service output value */
typedef enum _srtm_io_value
{
    SRTM_IoValueLow = 0U,
    SRTM_IoValueHigh,
} srtm_io_value_t;

/*! @brief SRTM IO service input event */
typedef enum _srtm_io_event
{
    SRTM_IoEventNone = 0U, /* Ignore the event */
    SRTM_IoEventRisingEdge,
    SRTM_IoEventFallingEdge,
    SRTM_IoEventEitherEdge,
    SRTM_IoEventLowLevel,
    SRTM_IoEventHighLevel,
} srtm_io_event_t;

/**
 * @brief SRTM IO service set output value function type.
 */
typedef srtm_status_t (*srtm_io_service_set_output_t)(srtm_service_t service,
                                                      srtm_peercore_t core,
                                                      uint16_t ioId,
                                                      srtm_io_value_t ioValue);

/**
 * @brief SRTM IO service get input value function type.
 */
typedef srtm_status_t (*srtm_io_service_get_input_t)(srtm_service_t service,
                                                     srtm_peercore_t core,
                                                     uint16_t ioId,
                                                     srtm_io_value_t *pIoValue);

/**
 * @brief SRTM IO service configure input event function type.
 */
typedef srtm_status_t (*srtm_io_service_conf_input_t)(
    srtm_service_t service, srtm_peercore_t core, uint16_t ioId, srtm_io_event_t event, bool wakeup);

/*******************************************************************************
 * API
 ******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Create IO service.
 *
 * @return SRTM service handle on success and NULL on failure.
 */
srtm_service_t SRTM_IoService_Create(void);

/*!
 * @brief Destroy IO service.
 *
 * @param service SRTM service to destroy.
 */
void SRTM_IoService_Destroy(srtm_service_t service);

/*!
 * @brief Reset IO service.
 *  This is used to stop all IO operations and return to initial state for corresponding core.
 *  Registered pins are kept unchanged.
 *
 * @param service SRTM service to reset.
 * @param core Identify which core is to be reset
 */
void SRTM_IoService_Reset(srtm_service_t service, srtm_peercore_t core);

/*!
 * @brief Register IO service pin. Only registered pin will be serviced.
 *
 * @param service SRTM IO service handle.
 * @param ioId IO pin identification.
 * @param setOutput IO pin set output value callback.
 * @param getInput IO pin get input value callback.
 * @param confIEvent IO pin configure input event callback.
 * @param param user callback parameter.
 * @return SRTM_Status_Success on success and others on failure.
 */
srtm_status_t SRTM_IoService_RegisterPin(srtm_service_t service,
                                         uint16_t ioId,
                                         srtm_io_service_set_output_t setOutput,
                                         srtm_io_service_get_input_t getInput,
                                         srtm_io_service_conf_input_t confIEvent,
                                         void *param);

/*!
 * @brief Unregister IO service pin. The operation cannot work when service is running.
 *
 * @param service SRTM IO service handle.
 * @param ioId IO pin identification.
 * @return SRTM_Status_Success on success and others on failure.
 */
srtm_status_t SRTM_IoService_UnregisterPin(srtm_service_t service, uint16_t ioId);

/*!
 * @brief Notify Input event to peer core. This function must be called by application after peer core configured
 *  input event.
 *
 * @param service SRTM IO service.
 * @param ioId IO pin identification.
 * @return SRTM_Status_Success on success and others on failure.
 */
srtm_status_t SRTM_IoService_NotifyInputEvent(srtm_service_t service, uint16_t ioId);

#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* __SRTM_IO_SERVICE_H__ */
