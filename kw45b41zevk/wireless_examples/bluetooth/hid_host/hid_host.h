/*! *********************************************************************************
 * \defgroup HID Host
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright 2014 Freescale Semiconductor, Inc.
* Copyright 2016-2019, 2021, 2023 NXP
*
*
* \file
*
* This file is the interface file for the HID Host application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef HID_HOST_H
#define HID_HOST_H

#include "fsl_shell.h"

/*************************************************************************************
**************************************************************************************
* Public macros
**************************************************************************************
*************************************************************************************/
extern SHELL_HANDLE_DEFINE(g_shellHandle);

#define shell_write(a)       SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, a)
#define SHELL_NEWLINE()      SHELL_WriteSynchronization((shell_handle_t)g_shellHandle, "\r\n", 2U)
#define shell_writeN(a,b)    SHELL_WriteSynchronization((shell_handle_t)g_shellHandle, a, b)
#define shell_writeDec(a)    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, (const char*)FORMAT_Dec2Str(a))
#define shell_writeBool(a)   if(a){SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "TRUE");}else{SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "FALSE");}
#define shell_writeHex       BleApp_PrintHex
#define shell_cmd_finished() SHELL_PrintPrompt((shell_handle_t)g_shellHandle)

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
extern gapScanningParameters_t gScanParams;
extern gapConnectionRequestParameters_t gConnReqParams;

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void BleApp_Start(void);
void BleApp_GenericCallback (gapGenericEvent_t* pGenericEvent);
void BleApp_PrintHex(uint8_t *pHex, uint8_t len);

#ifdef __cplusplus
}
#endif


#endif /* _APP_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
