/*! *********************************************************************************
 * \defgroup EATT Central
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright 2021-2023 NXP
*
*
* \file
*
* This file is the interface file for the EATT central application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef EATT_CENTRAL_H
#define EATT_CENTRAL_H

#include "fsl_format.h"
#include "fsl_shell.h"
#include "ble_conn_manager.h"
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

#define gBleSig_AService_d            0xA00A
#define gBleSig_BService_d            0xB00B

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
void BleApp_PrintHex(uint8_t *pHex, uint8_t len);

#ifdef __cplusplus
}
#endif


#endif /* ADV_EXT_CENTRAL_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
