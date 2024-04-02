/*! *********************************************************************************
 * \defgroup Localization Reader application
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* \file shell_loc_reader.h
*
* Copyright 2023 NXP
*
* NXP Confidential Proprietary
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from NXP.
********************************************************************************** */

#ifndef SHELL_LOC_READER_H
#define SHELL_LOC_READER_H

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
#define SHELL_NEWLINE()      (void)SHELL_WriteSynchronization((shell_handle_t)g_shellHandle, "\r\n", 2U)
#define shell_writeN(a,b)    (void)SHELL_WriteSynchronization((shell_handle_t)g_shellHandle, a, b)
#define shell_writeDec(a)    (void)SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, (const char*)FORMAT_Dec2Str(a))
#define shell_writeBool(a)   if(a){(void)SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "TRUE");}else{(void)SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "FALSE");}
#define shell_writeHex       BleApp_PrintHex
#define shell_writeHexLe     BleApp_PrintHexLe
#define shell_cmd_finished() SHELL_PrintPrompt((shell_handle_t)g_shellHandle)

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/
/* APP -  pointer to function for BLE events*/
typedef void (*pfShellCallback_t)(void* pData);

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
void AppShellInit(char* prompt);
void BleApp_PrintHex(uint8_t *pHex, uint8_t len);
void BleApp_PrintHexLe(uint8_t *pHex, uint8_t len);
#ifdef __cplusplus
}
#endif


#endif /* SHELL_LOC_READER_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
