/*
 * Copyright 2017, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SRTM_KEYPAD_SERVICE_H__
#define __SRTM_KEYPAD_SERVICE_H__

#include "srtm_service.h"

/*!
 * @addtogroup srtm_service
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/** @brief Switch to disable keypad service debugging messages. */
#ifndef SRTM_KEYPAD_SERVICE_DEBUG_OFF
#define SRTM_KEYPAD_SERVICE_DEBUG_OFF (0)
#endif

#if SRTM_KEYPAD_SERVICE_DEBUG_OFF
#undef SRTM_DEBUG_VERBOSE_LEVEL
#define SRTM_DEBUG_VERBOSE_LEVEL SRTM_DEBUG_VERBOSE_NONE
#endif

/*! @brief SRTM keypad value */
typedef enum _srtm_keypad_value
{
    SRTM_KeypadValueReleased = 0U,
    SRTM_KeypadValuePressed,
} srtm_keypad_value_t;

/*! @brief SRTM keypad service input event */
typedef enum _srtm_keypad_event
{
    SRTM_KeypadEventNone = 0U, /* Ignore the event */
    SRTM_KeypadEventPress,
    SRTM_KeypadEventRelease,
    SRTM_KeypadEventPressOrRelease,
} srtm_keypad_event_t;

/**
 * @brief SRTM keypad service configure keypad event function type.
 */
typedef srtm_status_t (*srtm_keypad_service_conf_t)(
    srtm_service_t service, srtm_peercore_t core, uint8_t keyIdx, srtm_keypad_event_t event, bool wakeup);

/*******************************************************************************
 * API
 ******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Create keypad service.
 *
 * @return SRTM service handle on success and NULL on failure.
 */
srtm_service_t SRTM_KeypadService_Create(void);

/*!
 * @brief Destroy keypad service.
 *
 * @param service SRTM service to destroy.
 */
void SRTM_KeypadService_Destroy(srtm_service_t service);

/*!
 * @brief Reset keypad service.
 *  This is used to stop all keypad operations and return to initial state for corresponding core.
 *  Registered keys are kept unchanged.
 *
 * @param service SRTM service to reset.
 * @param core Identify which core is to be reset
 */
void SRTM_KeypadService_Reset(srtm_service_t service, srtm_peercore_t core);

/*!
 * @brief Register keypad service key. Only registered key will be serviced.
 *
 * @param service SRTM keypad service handle.
 * @param keyIdx Keypad key index.
 * @param confKEvent Keypad configure event callback.
 * @param param user callback parameter.
 * @return SRTM_Status_Success on success and others on failure.
 */
srtm_status_t SRTM_KeypadService_RegisterKey(srtm_service_t service,
                                             uint8_t keyIdx,
                                             srtm_keypad_service_conf_t confKEvent,
                                             void *param);

/*!
 * @brief Unregister keypad service pin. The operation cannot work when service is running.
 *
 * @param service SRTM keypad service handle.
 * @param keyIdx Keypad key index.
 * @return SRTM_Status_Success on success and others on failure.
 */
srtm_status_t SRTM_KeypadService_UnregisterKey(srtm_service_t service, uint8_t keyIdx);

/*!
 * @brief Notify keypad event to peer core. This function must be called by application after peer core configured
 *  keypad event.
 *
 * @param service SRTM KEYPAD service.
 * @param keyIdx Keypad index.
 * @param value Keypad event value.
 * @return SRTM_Status_Success on success and others on failure.
 */
srtm_status_t SRTM_KeypadService_NotifyKeypadEvent(srtm_service_t service, uint8_t keyIdx, srtm_keypad_value_t value);

#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* __SRTM_KEYPAD_SERVICE_H__ */
