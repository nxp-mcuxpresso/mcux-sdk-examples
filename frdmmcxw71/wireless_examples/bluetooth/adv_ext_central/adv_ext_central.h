/*! *********************************************************************************
 * \defgroup Extended Advertising Central
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright 2020-2021, 2023-2024 NXP
*
*
* \file
*
* This file is the interface file for the extended advertising central application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef ADV_EXT_CENTRAL_H
#define ADV_EXT_CENTRAL_H

/*************************************************************************************
**************************************************************************************
* Public macros
**************************************************************************************
*************************************************************************************/
#define gScanningTime_c                60

#define gWaitForDataTime_c             5

#ifndef gAppDeepSleepMode_c
#define gAppDeepSleepMode_c            1
#endif

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
extern gapConnectionRequestParameters_t gConnReqParams;
extern gapScanningParameters_t          gScanParams;
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
extern gapDecisionInstructionsData_t    gDbafDecisionInstructions;
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void BleApp_Start(void);

#ifdef __cplusplus
}
#endif


#endif /* ADV_EXT_CENTRAL_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
