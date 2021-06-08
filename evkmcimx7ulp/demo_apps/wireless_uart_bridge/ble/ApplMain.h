/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APPL_MAIN_H_
#define _APPL_MAIN_H_

/*! *********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */
#include "EmbeddedTypes.h"
#include "l2ca_cb_interface.h"

/*! *********************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
********************************************************************************** */
typedef void *appCallbackParam_t;
typedef void (*appCallbackHandler_t)(appCallbackParam_t param);

/*! *********************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
 * \brief  Application wrapper function for Gap_Connect.
 *
 * \param[in] pParameters           Callback used by the application to receive advertising events.
 *                                  Can be NULL.
 * \param[in] connCallback Callback used to receive connection events.
 *
 * \return  gBleSuccess_c or error.
 *
 * \remarks This function should be used by the application if the callback should
 *          be executed in the context of the Application Task.
 *
 ********************************************************************************** */
bleResult_t App_Connect(gapConnectionRequestParameters_t *pParameters, gapConnectionCallback_t connCallback);

/*! *********************************************************************************
 * \brief  Application wrapper function for Gap_StartAdvertising.
 *
 * \param[in] advertisingCallback   Callback used by the application to receive advertising events.
 *                                  Can be NULL.
 * \param[in] connectionCallback    Callback used by the application to receive connection events.
 *                                  Can be NULL.
 *
 * \return  gBleSuccess_c or error.
 *
 * \remarks This function should be used by the application if the callback should
 *          be executed in the context of the Application Task.
 *
 ********************************************************************************** */
bleResult_t App_StartAdvertising(gapAdvertisingCallback_t advertisingCallback,
                                 gapConnectionCallback_t connectionCallback);

/*! *********************************************************************************
 * \brief  Application wrapper function for Gap_StartScanning.
 *
 * \param[in] pScanningParameters The scanning parameters; may be NULL.
 * \param[in] scanningCallback The scanning callback.
 *
 * \return  gBleSuccess_c or error.
 *
 * \remarks This function should be used by the application if the callback should
 *          be executed in the context of the Application Task.
 *
 ********************************************************************************** */
bleResult_t App_StartScanning(gapScanningParameters_t *pScanningParameters, gapScanningCallback_t scanningCallback);

/*! *********************************************************************************
 * \brief  Application wrapper function for GattClient_RegisterNotificationCallback.
 *
 * \param[in] callback   Application defined callback to be triggered by this module.
 *
 * \return  gBleSuccess_c or error.
 *
 * \remarks This function should be used by the application if the callback should
 *          be executed in the context of the Application Task.
 *
 ********************************************************************************** */
bleResult_t App_RegisterGattClientNotificationCallback(gattClientNotificationCallback_t callback);

/*! *********************************************************************************
 * \brief  Application wrapper function for GattClient_RegisterIndicationCallback.
 *
 * \param[in] callback   Application defined callback to be triggered by this module.
 *
 * \return  gBleSuccess_c or error.
 *
 * \remarks This function should be used by the application if the callback should
 *          be executed in the context of the Application Task.
 *
 ********************************************************************************** */
bleResult_t App_RegisterGattClientIndicationCallback(gattClientIndicationCallback_t callback);

/*! *********************************************************************************
 * \brief  Application wrapper function for GattServer_RegisterCallback.
 *
 * \param[in] callback Application-defined callback to be triggered by this module.
 *
 * \return  gBleSuccess_c or error.
 *
 * \remarks This function should be used by the application if the callback should
 *          be executed in the context of the Application Task.
 *
 ********************************************************************************** */
bleResult_t App_RegisterGattServerCallback(gattServerCallback_t serverCallback);

/*! *********************************************************************************
 * \brief  Application wrapper function for App_RegisterGattClientProcedureCallback.
 *
 * \param[in] callback Application-defined callback to be triggered by this module.
 *
 * \return  gBleSuccess_c or error.
 *
 * \remarks This function should be used by the application if the callback should
 *          be executed in the context of the Application Task.
 *
 ********************************************************************************** */
bleResult_t App_RegisterGattClientProcedureCallback(gattClientProcedureCallback_t callback);

/*! *********************************************************************************
 * \brief  Application wrapper function for L2ca_RegisterLeCbCallbacks.
 *
 * \param[in] callback Application-defined callback to be triggered by this module.
 *
 * \return  gBleSuccess_c or error.
 *
 * \remarks This function should be used by the application if the callback should
 *          be executed in the context of the Application Task.
 *
 ********************************************************************************** */
bleResult_t App_RegisterLeCbCallbacks(l2caLeCbDataCallback_t pCallback, l2caLeCbControlCallback_t pCtrlCallback);

/*! *********************************************************************************
 * \brief  Posts an application event containing a callback handler and parameter.
 *
 * \param[in] handler Handler function, to be executed when the event is processed.
 * \param[in] param   Parameter for the handler function.
 *
 * \return  gBleSuccess_c or error.
 *
 * \remarks This function should be used by the application if a callback must
 *          be executed in the context of the Application Task.
 *
 ********************************************************************************** */
bleResult_t App_PostCallbackMessage(appCallbackHandler_t handler, appCallbackParam_t param);

#endif /* _APPL_MAIN_H_ */
