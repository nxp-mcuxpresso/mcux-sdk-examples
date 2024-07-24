/*! *********************************************************************************
 * \defgroup ANCS Client
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright 2017-2024 NXP
*
*
* \file
*
* This file is the interface file for the ANCS Client application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef ANCS_CLIENT_H
#define ANCS_CLIENT_H

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "fsl_shell.h"

/*************************************************************************************
**************************************************************************************
* Public macros
**************************************************************************************
*************************************************************************************/
extern SHELL_HANDLE_DEFINE(g_shellHandle);

#define shell_write(a)       (void)SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, a)
#define shell_writeHex(a)    (void)SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, (char const*)FORMAT_Hex2Ascii(a))
#define shell_writeDec(a)    (void)SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, (const char*)FORMAT_Dec2Str(a))
#define shell_writeBool(a)   if(a){(void)SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "TRUE");}else{(void)SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "FALSE");}
#define shell_writeN(a,b)    (void)SHELL_WriteSynchronization((shell_handle_t)g_shellHandle, a, b)

/* Profile Parameters */
#define gFastConnMinAdvInterval_c               32      /* 20 ms */
#define gFastConnMaxAdvInterval_c               48      /* 30 ms */

#define gReducedPowerMinAdvInterval_c           1600    /* 1 s */
#define gReducedPowerMaxAdvInterval_c           4000    /* 2.5 s */

#define gFastConnAdvTime_c                      30      /* s */
#define gReducedPowerAdvTime_c                  300     /* s */

#if gAppUseBonding_d
#define gFastConnFilterAcceptListAdvTime_c      10      /* s */
#else
#define gFastConnFilterAcceptListAdvTime_c      0
#endif

#ifndef gAppUseTimeService_d
#define gAppUseTimeService_d                    0
#endif

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
extern gapAdvertisingData_t             gAppAdvertisingData;
extern gapScanResponseData_t            gAppScanRspData;
extern gapAdvertisingParameters_t       gAdvParams;

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void BleApp_Start(void);
void AppShellInit(char* prompt);
void App_HandleShellCmds(void *pData);

#ifdef __cplusplus
}
#endif


#endif /* ANCS_CLIENT_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
