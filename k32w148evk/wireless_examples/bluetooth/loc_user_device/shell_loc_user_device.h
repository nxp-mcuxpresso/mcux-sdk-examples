/*! *********************************************************************************
 * \defgroup Localization User Device application
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* \file shell_loc_user_device.h
*
* Copyright 2023 NXP
*
* NXP Confidential Proprietary
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from NXP.
********************************************************************************** */

#ifndef SHELL_LOC_USER_DEVICE_H
#define SHELL_LOC_USER_DEVICE_H

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "app_localization.h"

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
typedef struct appBondingData_tag
{
    uint8_t nvmIndex;
    uint8_t addrType;
    uint8_t deviceAddr[6];
    uint8_t aLtk[16];
    uint8_t aIrk[16];
}appBondingData_t;

typedef struct appCsConfigParams_tag
{
    uint8_t     mainModeType;
    uint8_t     subModeType;
    uint8_t     mainModeMinSteps;
    uint8_t     mainModeMaxSteps;
    uint8_t     mainModeRepetition;
    uint8_t     mode0Steps;
    uint8_t     role; /*!< 0 - Initiator, 1 - Reflector. */
    uint8_t     RTTType;
    uint8_t     channelMap[APP_LOCALIZATION_CH_MAP_LEN];
    uint8_t     channelMapRepetition;
    uint8_t     channelSelectionType;
} appCsConfigParams_t;

typedef struct appCsProcedureParams_tag
{
    uint16_t    maxProcedureDuration;
    uint16_t    minPeriodBetweenProcedures;
    uint16_t    maxPeriodBetweenProcedures;
    uint16_t    maxNumProcedures;
    uint32_t    minSubeventLen;
    uint32_t    maxSubeventLen;
    uint8_t     antCfgIndex;
} appCsProcedureParams_t;

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


#endif /* SHELL_LOC_USER_DEVICE_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */