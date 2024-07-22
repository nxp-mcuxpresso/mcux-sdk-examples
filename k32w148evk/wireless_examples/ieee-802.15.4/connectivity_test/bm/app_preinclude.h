/*! *********************************************************************************
* Copyright 2020 NXP
* All rights reserved.
*
* \file
*
* This file is the app configuration file which is pre included.
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef _APP_PREINCLUDE_H_
#define _APP_PREINCLUDE_H_

/*! Enable Serial Console with external device or host using the LPUART */
#define gAppUseSerialManager_c          1

#define gAspCapability_d                1

/*! Enable/Disable SMAC security features */
#define gSmacUseSecurity_c              0

/* Uncomment to test EnhAck mode7 */
//#define gSmacUseExtendedAddr_c          1
//#define gEnhAckMode8                    0

/* Uncomment to test EnhAck mode8 */
//#define gSmacUseExtendedAddr_c          1
//#define gEnhAckMode8                    1

/* Defines if the DCDC is used (buck mode) or not (bypass mode) */
#define gBoardDcdcBuckMode_d            1

/*Configure the DCDC output voltage in buck mode
 *0dBm,1.25V
 *7dBm, 1.8V
 *10dBm, 2.5V*/
#define gAppMaxTxPowerDbm_c 10


#endif /* _APP_PREINCLUDE_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
