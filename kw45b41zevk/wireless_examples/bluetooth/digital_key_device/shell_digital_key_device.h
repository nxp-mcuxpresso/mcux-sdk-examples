/*! *********************************************************************************
 * \defgroup Digital Key Device Application
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* \file shell_digital_key_device.h
*
* Copyright 2021, 2024 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef SHELL_DIGITAL_KEY_H
#define SHELL_DIGITAL_KEY_H

/*************************************************************************************
**************************************************************************************
* Public macros
**************************************************************************************
*************************************************************************************/
#if !defined(gAppUseShellInApplication_d) || (defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 0))
    #define shell_write(function, ...)
    #define shell_writeHex(function, ...)
    #define shell_cmd_finished(function, ...)
    #define shell_init(function, ...)
    #define shell_register_function(function, ...)
    #define shell_refresh(function, ...)
    #define kStatus_SHELL_Success   0
#else
    extern SHELL_HANDLE_DEFINE(g_shellHandle);

    #define shell_write(a)       (void)SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, a)
    #define SHELL_NEWLINE()      (void)SHELL_WriteSynchronization((shell_handle_t)g_shellHandle, "\r\n", 2U)
    #define shell_writeN(a,b)    (void)SHELL_WriteSynchronization((shell_handle_t)g_shellHandle, a, b)
    #define shell_writeDec(a)    (void)SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, (const char*)FORMAT_Dec2Str(a))
    #define shell_writeBool(a)   if(a){(void)SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "TRUE");}else{(void)SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "FALSE");}
    #define shell_writeHex       BleApp_PrintHex
    #define shell_writeHexLe     BleApp_PrintHexLe
    #define shell_cmd_finished() SHELL_PrintPrompt((shell_handle_t)g_shellHandle)
#endif
/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/
typedef struct appBondingData_tag
{
    uint8_t nvmIndex;
    uint8_t addrType;
    uint8_t deviceAddr[6];
    uint8_t aLtk[16];
    uint8_t aIrk[16];
}appBondingData_t;
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

#ifdef __cplusplus
}
#endif


#endif /* SHELL_DIGITAL_KEY_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */