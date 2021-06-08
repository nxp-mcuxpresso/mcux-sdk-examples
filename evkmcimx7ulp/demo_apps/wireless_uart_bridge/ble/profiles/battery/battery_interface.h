/*! *********************************************************************************
 * \defgroup Battery Service
 * @{
 ********************************************************************************** */
/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BATTERY_INTERFACE_H_
#define _BATTERY_INTERFACE_H_

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public constants & macros
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/

/*! Battery Service - Configuration */
typedef struct basConfig_tag
{
    uint16_t serviceHandle;
    uint8_t batteryLevel;
} basConfig_t;

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*!**********************************************************************************
 * \brief        Starts Battery Service functionality
 *
 * \param[in]    pServiceConfig  Pointer to structure that contains server
 *                               configuration information.
 *
 * \return       gBleSuccess_c or error.
 ************************************************************************************/
bleResult_t Bas_Start(basConfig_t *pServiceConfig);

/*!**********************************************************************************
 * \brief        Stops Battery Service functionality
 *
 * \param[in]    pServiceConfig  Pointer to structure that contains server
 *                               configuration information.
 *
 * \return       gBleSuccess_c or error.
 ************************************************************************************/
bleResult_t Bas_Stop(basConfig_t *pServiceConfig);

/*!**********************************************************************************
 * \brief        Subscribes a GATT client to the Battery service
 *
 * \param[in]    pClient  Client Id in Device DB.
 *
 * \return       gBleSuccess_c or error.
 ************************************************************************************/
bleResult_t Bas_Subscribe(deviceId_t clientDeviceId);

/*!**********************************************************************************
 * \brief        Unsubscribes a GATT client from the Battery service
 *
 * \param[in]    pClient  Client Id in Device DB.
 *
 * \return       gBleSuccess_c or error.
 ************************************************************************************/
bleResult_t Bas_Unsubscribe(void);

/*!**********************************************************************************
 * \brief        Handles command on the Battery Control Point
 *
 * \param[in]    serviceHandle   Service handle.
 * \param[in]    value           Command Value.
 *
 * \return       gAttErrCodeNoError_c or error.
 *************************************************************************************/
uint8_t Bas_ControlPointHandler(uint16_t serviceHandle, uint8_t value);

/*!**********************************************************************************
 * \brief        Records battery measurement on a specified service handle.
 *
 * \param[in]    serviceHandle   Service handle.
 * \param[in]    value           Battery Value.
 *
 * \return       gBleSuccess_c or error.
 ************************************************************************************/
bleResult_t Bas_RecordBatteryMeasurement(uint16_t serviceHandle, uint8_t batteryLevel);

#ifdef __cplusplus
}
#endif

#endif /* _BATTERY_INTERFACE_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
