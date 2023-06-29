/*! *********************************************************************************
 * \defgroup private_profile_server
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2014, Freescale Semiconductor, Inc.
* Copyright 2016-2018 NXP
* All rights reserved.
*
* \file private_profile_server.h
* This file is the interface file for the QPP Server application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef _PRIVATE_PROFILE_SERVER_H_
#define _PRIVATE_PROFILE_SERVER_H_
/*************************************************************************************
**************************************************************************************
* Public macros
**************************************************************************************
*************************************************************************************/


/* Profile Parameters */

#define gFastConnMinAdvInterval_c       32 /* 20 ms */
#define gFastConnMaxAdvInterval_c       48 /* 30 ms */

#define gReducedPowerMinAdvInterval_c   1600 /* 1 s */
#define gReducedPowerMaxAdvInterval_c   4000 /* 2.5 s */

#define gFastConnAdvTime_c              30  /* s */
#define gReducedPowerAdvTime_c          300 /* s */

#if gAppUseBonding_d
#define gFastConnWhiteListAdvTime_c     10 /* s */
#else
#define gFastConnWhiteListAdvTime_c     0
#endif
enum
{   
    QPPS_VALUE_NTF_OFF = 0x00,
    QPPS_VALUE_NTF_ON  = 0x01,
};
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

void BleApp_Init(void);
void BleApp_Start(void);
void BleApp_GenericCallback (gapGenericEvent_t* pGenericEvent);
#ifdef __cplusplus
}
#endif 


#endif /* _PRIVATE_PROFILE_SERVER_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
