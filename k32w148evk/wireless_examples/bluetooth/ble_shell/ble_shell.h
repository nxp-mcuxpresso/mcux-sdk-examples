/*! *********************************************************************************
 * \defgroup Bluetooth Shell Application
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright 2015 Freescale Semiconductor, Inc.
* Copyright 2016-2024 NXP
*
*
* \file
*
* This file is the interface file for the Bluetooth Shell Application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef APP_H
#define APP_H

#include "gap_interface.h"
#include "fsl_format.h"
#include "app_conn.h"
#include "fsl_shell.h"
/*************************************************************************************
**************************************************************************************
* Public macros
**************************************************************************************
*************************************************************************************/

/* Enable Advertising Extension shell commands */
#ifndef BLE_SHELL_AE_SUPPORT
#define BLE_SHELL_AE_SUPPORT    0U
#endif

/* Enable Decision Based Advertising Filtering shell commands */
#ifndef BLE_SHELL_DBAF_SUPPORT
#define BLE_SHELL_DBAF_SUPPORT  0U
#endif

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/
typedef struct thrConfig_tag
{
    uint32_t buffCnt;
    uint8_t  buffSz;
    bool_t   bTestInProgress;
} thrConfig_t;

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */
/*! @brief Declare shell handle. */
extern SHELL_HANDLE_DEFINE(g_shellHandle);

#define shell_write(a)       (void)SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, a)
#define SHELL_NEWLINE()      (void)SHELL_WriteSynchronization((shell_handle_t)g_shellHandle, "\r\n", 2U)
#define shell_writeN(a,b)    (void)SHELL_WriteSynchronization((shell_handle_t)g_shellHandle, a, b)
#define shell_writeDec(a)    (void)SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, (const char*)FORMAT_Dec2Str(a))
#define shell_writeBool(a)   if(a){(void)SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "TRUE");}else{(void)SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "FALSE");}
#define shell_writeHex(a)    (void)SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, (char const*)FORMAT_Hex2Ascii(a))
#define shell_cmd_finished() SHELL_PrintPrompt((shell_handle_t)g_shellHandle)

extern gapAdvertisingData_t             gAppAdvertisingData;
extern gapScanResponseData_t            gAppScanRspData;
extern gapAdvertisingParameters_t       gAdvParams;
extern gapScanningParameters_t          gAppScanParams;
extern gapConnectionRequestParameters_t gConnReqParams;
extern gapPairingParameters_t           gPairingParameters;
extern gapExtAdvertisingParameters_t    gExtAdvParams;
extern gapScanResponseData_t            gAppExtScanRspData;
extern gapAdvertisingData_t             gAppExtAdvertisingData;
extern gapPeriodicAdvParameters_t       gPeriodicAdvParams;
extern gapAdvertisingData_t             gAppPeriodicAdvData;

#if defined(BLE_SHELL_DBAF_SUPPORT) && (BLE_SHELL_DBAF_SUPPORT)
extern uint8_t gaDecisionKey[];
extern uint8_t gaPrand[];
extern uint8_t gaDecisionData[];
extern gapAdvertisingDecisionData_t     gAppExtAdvDecisionData;

extern gapDecisionInstructionsData_t gaDecisionInstructions[];
extern uint8_t gNumDecisionInstructions;
#endif /* BLE_SHELL_DBAF_SUPPORT */

/* Peer Device ID */
extern uint8_t                          gActiveConnections;

extern thrConfig_t  gThroughputConfig[];
extern bool_t       gUseShellThrGenericCb;

/** Bit manipulation macros */
#define SET_NEW_CONN(bit)           ((gActiveConnections) |=  (1U << (bit)))

#define CLEAR_CONN(bit)             ((gActiveConnections)  &= ~(1U << (bit)))

#define IS_CONNECTED(bit)           ((1U << (bit)) & (gActiveConnections))

/* Timeout to consider a throughput test finished.
   This is multiplied with the number of active connections */
#ifndef SHELL_THRPUT_TIMEOUT_MS
#define SHELL_THRPUT_TIMEOUT_MS     (3U*((uint32_t)gGapDefaultMaxConnectionInterval_d + (uint32_t)gGapDefaultMaxConnectionInterval_d/4U))
#endif
/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*! *********************************************************************************
* \brief    Initializes application specific functionality before the BLE stack init.
*
********************************************************************************** */
void BleApp_Init(void);

/*! *********************************************************************************
 * \brief        Parses a string input interpreting its content as a hex number.
 *
 * \param[in]    pInput         Pointer to string
 *
 * \return       uint8_t        Returns the equivalent of the string as an uint8_t number
 ********************************************************************************** */
uint8_t BleApp_ParseHexValue(char* pInput);

/*!*************************************************************************************************
\fn     int32_t BleApp_atoi(char *str)
\brief  Converts a string into an integer.

\param  [in]    pStr       Pointer to string

\return                    Integer converted from string.
***************************************************************************************************/
int32_t BleApp_atoi(char *pStr);

/*!*************************************************************************************************
\fn     uint32_t BleApp_AsciiToHex(char *pString, uint32_t strLen)
\brief  Converts a string into hex.

\param  [in]    pString     pointer to string
\param  [in]    strLen      string length

\return uint32_t value in hex
***************************************************************************************************/
uint32_t BleApp_AsciiToHex(char *pString, uint32_t strLen);

#if (defined(CPU_MKW37A512VFT4) || defined(CPU_MKW38A512VFT4))
#if (defined(gRFCalibration_d) && (gRFCalibration_d > 0))
/*! *********************************************************************************
 * \brief        Calibration command. Contains commands for enabling/disabling RF
                 calibration and setters/getters for frequency offset and RSSI
 *
 * \param[in]    argc    shell argument count
 *
 * \param[in]    argv    shell argument value
 *
 * \return       command status
 ********************************************************************************** */
int8_t ShellCalibration_Command(uint8_t argc, char * argv[]);
#endif
#endif /* (defined(CPU_MKW37A512VFT4) || defined(CPU_MKW38A512VFT4)) */

#ifdef __cplusplus
}
#endif


#endif /* APP_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
