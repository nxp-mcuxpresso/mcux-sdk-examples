/*! *********************************************************************************
 * \defgroup GATT GATT - Generic Attribute Profile Interface
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

#ifndef _GATT_INTERFACE_H_
#define _GATT_INTERFACE_H_

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "ble_general.h"
#include "gatt_types.h"
#include "gap_types.h"

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*! *********************************************************************************
 * \brief  Initializes the GATT module.
 *
 * \remarks If the GAP module is present, this function is called internally by Ble_HostInitialize().
 * Otherwise, the application must call this function once at device start-up.
 *
 * \remarks This function executes synchronously.
 *
 ********************************************************************************** */
bleResult_t Gatt_Init(void);

/*! *********************************************************************************
 * \brief  Retrieves the MTU used with a given connected device.
 *
 * \param[in]  deviceId The device ID of the connected peer.
 * \param[out] pOutMtu  Pointer to integer to be written.
 *
 * \return  gBleSuccess_c or error.
 *
 * \remarks This function executes synchronously.
 *
 ********************************************************************************** */
bleResult_t Gatt_GetMtu(deviceId_t deviceId, uint16_t *pOutMtu);

#ifdef __cplusplus
}
#endif

#endif /* _GATT_INTERFACE_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
