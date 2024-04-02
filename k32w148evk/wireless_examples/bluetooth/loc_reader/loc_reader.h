/*! *********************************************************************************
 * \defgroup Localization Reader application
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* \file loc_reader.h
*
* Copyright 2023 NXP
*
* NXP Confidential Proprietary
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from NXP.
********************************************************************************** */

#ifndef LOC_READER_H
#define LOC_READER_H

#include "fsl_component_timer_manager.h"
/* Profile / Services */
#include "ranging_interface.h"
#include "app_localization.h"

/*************************************************************************************
**************************************************************************************
* Public macros
**************************************************************************************
*************************************************************************************/

/* Profile Parameters */
#define gReducedPowerMinAdvInterval_c   1600 /* 1 s */
#define gReducedPowerMaxAdvInterval_c   4000 /* 2.5 s */

#define gAdvTime_c                      30 /* 30 s*/
#define gGoToSleepAfterDataTime_c       5 /* 5 s*/

#ifndef gAppDeepSleepMode_c
#define gAppDeepSleepMode_c 1
#endif
/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */
extern gapAdvertisingData_t             gAppAdvertisingData;
extern gapScanResponseData_t            gAppScanRspData;
extern gapExtAdvertisingParameters_t    gAdvParams;
extern gapExtAdvertisingParameters_t    gAdvParams;
extern appExtAdvertisingParams_t        gAppAdvParams;

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
void BleApp_Start(void);
void BleApp_Disconnect(void);
void BleApp_FactoryReset(void);
bleResult_t BleApp_TriggerCsDistanceMeasurement(deviceId_t deviceId);
#ifdef __cplusplus
}
#endif


#endif /* LOC_READER_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
