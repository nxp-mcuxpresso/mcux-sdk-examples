/*! *********************************************************************************
 * \defgroup Heart Rate Sensor
 * @{
 ********************************************************************************** */
/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_

/*************************************************************************************
**************************************************************************************
* Public macros
**************************************************************************************
*************************************************************************************/

/* App Configuration */

/*! Enable/disable use of bonding capability */
#ifndef gAppUseBonding_d
#define gAppUseBonding_d 0
#endif

/*! Enable/disable use of pairing procedure */
#ifndef gAppUsePairing_d
#define gAppUsePairing_d 0
#endif

/*! Enable/disable use of privacy */
#ifndef gAppUsePrivacy_d
#define gAppUsePrivacy_d 0
#endif

#if (gAppUseBonding_d) && (!gAppUsePairing_d)
#error "Enable pairing to make use of bonding"
#endif

/*! Maximum number of active connections */
#ifndef gAppMaxConnections_c
#define gAppMaxConnections_c 1
#endif

#if (gAppMaxConnections_c > 1)
#error "The application does not support more than 1 connection"
#endif

#define gAppFSCIHostInterfaceBaud_d gUARTBaudRate115200_c
#define gAppFSCIHostInterfaceType_d gSerialMgrLpuart_c
#define gAppFSCIHostInterfaceId_d   2

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* _APP_CONFIG_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
