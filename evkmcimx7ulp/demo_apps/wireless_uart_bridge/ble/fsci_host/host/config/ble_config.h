/*! *********************************************************************************
 * \addtogroup BLE
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

/************************************************************************************
*************************************************************************************
* DO NOT MODIFY THIS FILE!
*************************************************************************************
************************************************************************************/

#ifndef _BLE_CONFIG_H_
#define _BLE_CONFIG_H_

/************************************************************************************
*************************************************************************************
* Public macros - Do not modify directly! Override in app_preinclude.h if needed.
*************************************************************************************
************************************************************************************/
/* Number of bonded devices supported by the application.
 *  Make sure that (gMaxBondedDevices_c * gBleBondDataSize_c) fits into the Flash area
 *  reserved by the application for bond information. */
#ifndef gMaxBondedDevices_c
#define gMaxBondedDevices_c 8
#endif

/*! Maximum number of handles that can be registered for write notifications. */
#ifndef gMaxWriteNotificationHandles_c
#define gMaxWriteNotificationHandles_c 10
#endif

/*! Maximum number of handles that can be registered for read notifications. */
#ifndef gMaxReadNotificationHandles_c
#define gMaxReadNotificationHandles_c 10
#endif

/* Size of prepare write queue. Default value supports 512-byte attributes. */
#ifndef gPrepareWriteQueueSize_c
#define gPrepareWriteQueueSize_c (512 / (gAttMaxMtu_c - 5) + 1)
#endif

/* Preferred value for the maximum transmission number of payload octets to be
 * used for new connections.
 *
 * Range 0x001B - 0x00FB
 */
#ifndef gBleDefaultTxOctets_c
#define gBleDefaultTxOctets_c 0x00FB
#endif

/* Preferred value for the maximum packet transmission time to be
 * used for new connections.
 *
 * Range 0x0148 - 0x0848
 */
#ifndef gBleDefaultTxTime_c
#define gBleDefaultTxTime_c 0x0848
#endif

/* Timeout for Resolvable Private Address generation in Host
 *
 * Unit: 1 second
 * Range: 1 - 65535
 * Default: 900
 */
#ifndef gBleHostPrivacyTimeout_c
#define gBleHostPrivacyTimeout_c 900
#endif

/* Timeout for Resolvable Private Address generation in Controller
 * (Enhanced Privacy feature - BLE 4.2 only)
 *
 * Unit: 1 second
 * Range: 1 - 41400
 * Default: 900
 */
#ifndef gBleControllerPrivacyTimeout_c
#define gBleControllerPrivacyTimeout_c 900
#endif

/* Flag indicating whether device is set into LE Secure Connections Only Mode.
 * If this flag is overwritten as TRUE, then only LE Secure Connections Pairing is accepted.
 * Default: FALSE
 */
#ifndef gBleLeSecureConnectionsOnlyMode_c
#define gBleLeSecureConnectionsOnlyMode_c (FALSE)
#endif

/* Flag indicating whether OOB channel used in LE Secure Connections pairing has MITM protection (BLE 4.2 only).
 * Default: FALSE
 */
#ifndef gBleLeScOobHasMitmProtection_c
#define gBleLeScOobHasMitmProtection_c (FALSE)
#endif

#endif /* _BLE_CONFIG_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
