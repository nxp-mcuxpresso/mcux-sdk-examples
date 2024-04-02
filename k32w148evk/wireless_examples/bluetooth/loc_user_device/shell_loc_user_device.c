/*! *********************************************************************************
* \addtogroup User Device Central Application
* @{
********************************************************************************** */
/*! *********************************************************************************
* \file shell_loc_user_device.c
*
* Copyright 2023-2024 NXP
*
* NXP Confidential Proprietary
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from NXP.
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
/* Framework / Drivers */
#include "EmbeddedTypes.h"
#include "fsl_shell.h"
#include "fsl_adapter_reset.h"
#include "fsl_component_mem_manager.h"
#include "fsl_component_timer_manager.h"

#include "app.h"

/* BLE Host Stack */
#include "gap_interface.h"
#include "app_localization.h"
#include "loc_user_device.h"
#include "shell_loc_user_device.h"
#include "app_conn.h"

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/
static shell_status_t ShellReset_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellDisconnect_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellFactoryReset_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellStartBle_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellSetCsConfigParams_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellSetCsProcedureParams_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellTriggerDistanceMeasurement_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellSetNumProcs_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);

static uint32_t BleApp_AsciiToHex(char *pString, uint32_t strLen);
static int32_t BleApp_atoi(char *pStr);
static uint8_t BleApp_ParseHexValue(char* pInput);
static void ShellResetTimeoutTimerCallback(void* pParam);

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
static shell_command_t mResetCmd =
{
    .pcCommand = "reset",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellReset_Command,
    .pcHelpString = "\r\n\"reset\": Reset MCU.\r\n",
};

static shell_command_t mFactoryResetCmd =
{
    .pcCommand = "factoryreset",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellFactoryReset_Command,
    .pcHelpString = "\r\n\"factoryreset\": Factory Reset.\r\n",
};

static shell_command_t mSbCmd =
{
    .pcCommand = "sb",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellStartBle_Command,
    .pcHelpString = "\r\n\"sb\": Start BLE.\r\n",
};

static shell_command_t mDcntCmd =
{
    .pcCommand = "dcnt",
    .pcHelpString = "\r\n\"dcnt\": Disconnect all peers.\r\n",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellDisconnect_Command,
};

static shell_command_t mSetCsConfigParamsCmd =
{
    .pcCommand = "setcsconfig",
    .cExpectedNumberOfParameters = SHELL_IGNORE_PARAMETER_COUNT,
    .pFuncCallBack = ShellSetCsConfigParams_Command,
    .pcHelpString = "\r\n\"setcsconfig\": Set default parameters for Channel Sounding Create Config command.\r\n",
};

static shell_command_t mSetCsProcParamsCmd =
{
    .pcCommand = "setcsproc",
    .cExpectedNumberOfParameters = SHELL_IGNORE_PARAMETER_COUNT,
    .pFuncCallBack = ShellSetCsProcedureParams_Command,
    .pcHelpString = "\r\n\"setcsproc\": Set default parameters for Channel Sounding Set Procedure Parameters command.\r\n",
};

static shell_command_t mTriggerCsDistMeasCmd =
{
    .pcCommand = "tdm",
    .cExpectedNumberOfParameters = SHELL_IGNORE_PARAMETER_COUNT,
    .pFuncCallBack = ShellTriggerDistanceMeasurement_Command,
    .pcHelpString = "\r\n\"tdm\": Trigger Channel Sounding distance measurements.\r\n",
};

static TIMER_MANAGER_HANDLE_DEFINE(mResetTmrId);

static shell_command_t mSetNumProcsCmd =
{
    .pcCommand = "setnumprocs",
    .pcHelpString = "\r\n\"setnumprocs\": Set number of CS procedures.\r\n",
    .cExpectedNumberOfParameters = SHELL_IGNORE_PARAMETER_COUNT,
    .pFuncCallBack = ShellSetNumProcs_Command,
};

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
SHELL_HANDLE_DEFINE(g_shellHandle);

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief  Initializes the SHELL module .
*
* \param[in]  prompt the string which will be used for command prompt
*
* \remarks
*
********************************************************************************** */
void AppShellInit(char *prompt)
{
   /* UI */
    shell_status_t status = kStatus_SHELL_Error;
    (void)status;

    status = SHELL_Init((shell_handle_t)g_shellHandle, (serial_handle_t)gSerMgrIf, prompt);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mResetCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mFactoryResetCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mSbCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mDcntCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mSetCsConfigParamsCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mSetCsProcParamsCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mTriggerCsDistMeasCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mSetNumProcsCmd);
    assert(kStatus_SHELL_Success == status);
}

/*! *********************************************************************************
* \brief        Prints string of hex values
*
* \param[in]    pHex    pointer to hex value.
* \param[in]    len     hex value length.
********************************************************************************** */
void BleApp_PrintHex(uint8_t *pHex, uint8_t len)
{
    for (uint32_t i = 0; i<len; i++)
    {
        (void)SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, (char const*)FORMAT_Hex2Ascii(pHex[i]));
    }
}

/*! *********************************************************************************
* \brief        Prints string of hex values in reversed order
*
* \param[in]    pHex    pointer to hex LE value.
* \param[in]    len     hex value length.
********************************************************************************** */
void BleApp_PrintHexLe(uint8_t *pHex, uint8_t len)
{
    for (uint32_t i = 0; i<len; i++)
    {
        (void)SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, (char const*)FORMAT_Hex2Ascii(pHex[((uint32_t)len - 1U) - i]));
    }
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/
/*! *********************************************************************************
 * \brief        Reset MCU.
 *
 ********************************************************************************** */
static shell_status_t ShellReset_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    shell_status_t status = kStatus_SHELL_Error;
    timer_status_t tmrStatus = kStatus_TimerError;
    
    tmrStatus = TM_Open(mResetTmrId);
    
    if (tmrStatus == kStatus_TimerSuccess)
    {
        /* Start 10ms timer before reset to allow for shell prompt to be 
        sent over the serial interface. */
        (void)TM_InstallCallback((timer_handle_t)mResetTmrId, ShellResetTimeoutTimerCallback, NULL);
        tmrStatus = TM_Start((timer_handle_t)mResetTmrId, (uint8_t)kTimerModeSingleShot, 10U);
        
        if (tmrStatus == kStatus_TimerSuccess)
        {
            status = kStatus_SHELL_Success;
        }
    }
    
    return status;
}

/*! *********************************************************************************
* \brief        Reset timer callback.
                Called on timer task.
*
* \param[in]    pParam              not used
********************************************************************************** */
static void ShellResetTimeoutTimerCallback(void* pParam)
{
    (void)pParam;
    HAL_ResetMCU();
}

/*! *********************************************************************************
* \brief        Factory Reset.
*
* \param[in]    shellHandle    Shell handle
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellFactoryReset_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    BleApp_FactoryReset();
    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Initialize Owner Pairing or Passive Entry.
*
* \param[in]    shellHandle    Shell handle
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellStartBle_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    BleApp_Start();
    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Disconnect device.
*
* \param[in]    shellHandle    Shell handle
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellDisconnect_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    BleApp_Disconnect();
    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Set CS Create Config default parameters.
*
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellSetCsConfigParams_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    bleResult_t status = gBleSuccess_c;
    
    if (argc == 12)
    {
        appLocalization_rangeCfg_t csConfigParams;
        /* Read current configuration. */
        (void)AppLocalization_ReadConfig(&csConfigParams);
        /* Update configuration with the new values. */
        csConfigParams.main_mode_type = (uint8_t)BleApp_atoi(argv[1]);
        
        if ((csConfigParams.main_mode_type == 0U) || (csConfigParams.main_mode_type > 3U))
        {
            status = gBleInvalidParameter_c;
        }
        
        if (status == gBleSuccess_c)
        {
            csConfigParams.sub_mode_type = (uint8_t)BleApp_atoi(argv[2]);
            
            if ((csConfigParams.sub_mode_type == 0U) || (csConfigParams.sub_mode_type > 3U))
            {
                status = gBleInvalidParameter_c;
            }
        }
        
        if (status == gBleSuccess_c)
        {
            csConfigParams.main_mode_min = (uint8_t)BleApp_atoi(argv[3]);
            csConfigParams.main_mode_max = (uint8_t)BleApp_atoi(argv[4]);
            csConfigParams.main_mode_repeat = (uint8_t)BleApp_atoi(argv[5]);
            
            if (csConfigParams.main_mode_repeat > 3U)
            {
                status = gBleInvalidParameter_c;
            }
        }
        
        if (status == gBleSuccess_c)
        {
            csConfigParams.mode0_nb = (uint8_t)BleApp_atoi(argv[6]);
            
            if ((csConfigParams.mode0_nb == 0U) || (csConfigParams.mode0_nb > 3U))
            {
                status = gBleInvalidParameter_c;
            }
        }
        
        if (status == gBleSuccess_c)
        {
            uint8_t role = (uint8_t)BleApp_atoi(argv[7]);
            
            if (role > 1U)
            {
                status = gBleInvalidParameter_c;
            }
            else
            {
                csConfigParams.role = (role == 0U) ? gCsInitiator_c : gCsReflector_c;
            }
        }
        
        if (status == gBleSuccess_c)
        {
            csConfigParams.rtt_type = (uint8_t)BleApp_atoi(argv[8]);
            
            if (csConfigParams.rtt_type > 6U)
            {
                status = gBleInvalidParameter_c;
            }
        }
        
        if (status == gBleSuccess_c)
        {
            if (APP_LOCALIZATION_CH_MAP_LEN ==  BleApp_ParseHexValue(argv[9]))
            {
                FLib_MemCpy(csConfigParams.ch_map, argv[9], APP_LOCALIZATION_CH_MAP_LEN);
            }
            else
            {
                status = gBleInvalidParameter_c;
            }
        }
        
        if (status == gBleSuccess_c)
        {
            csConfigParams.ch_map_repeat = (uint8_t)BleApp_atoi(argv[10]);
            
            if (csConfigParams.ch_map_repeat == 0)
            {
                status = gBleInvalidParameter_c;
            }
        }
        
        if (status == gBleSuccess_c)
        {
            csConfigParams.channelSelectionType = (uint8_t)BleApp_atoi(argv[11]);
            
            if (csConfigParams.channelSelectionType > 1)
            {
                status = gBleInvalidParameter_c;
            }
        }
        
        if (status == gBleSuccess_c)
        {
            status = AppLocalization_WriteConfig(&csConfigParams);
        }

        if (status == gBleSuccess_c)
        {
            shell_write("\r\nConfig parameters set successfully.\r\n");
        }
    }
    else
    {
        shell_write("\r\nUsage: \
                    \r\nsetcsconfig main_mode_type, sub_mode_type, main_mode_min_steps, main_mode_max_steps, main_mode_repetition, mode_0_steps, role, rtt_types, chann_map, chan_map_repetition, chan_sel_type \
                    \r\n");
    }
    
    if (status == gBleOutOfMemory_c)
    {
        shell_write("\r\nOut of memory\r\n");
    }
    if (status == gBleInvalidParameter_c)
    {
        shell_write("\r\nInvalid parameter\r\n");
    }

    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Set CS Procedure default parameters.
*
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellSetCsProcedureParams_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    bleResult_t status = gBleSuccess_c;
    
    if (argc == 8)
    {
        appLocalization_rangeCfg_t csConfigParams;
        
        /* Read current configuration. */
        (void)AppLocalization_ReadConfig(&csConfigParams);
        
        /* Update configuration with the new values. */
        csConfigParams.maxProcedureDuration = (uint16_t)BleApp_atoi(argv[1]);
        
        if (csConfigParams.maxProcedureDuration == 0U)
        {
            status = gBleInvalidParameter_c;
        }
        
        if (status == gBleSuccess_c)
        {
            csConfigParams.minPeriodBetweenProcedures = (uint16_t)BleApp_atoi(argv[2]);
            csConfigParams.maxPeriodBetweenProcedures = (uint16_t)BleApp_atoi(argv[3]);
            csConfigParams.maxNumProcedures = (uint16_t)BleApp_atoi(argv[4]);
            csConfigParams.minSubeventLen = (uint32_t)BleApp_atoi(argv[5]);
            csConfigParams.maxSubeventLen = (uint32_t)BleApp_atoi(argv[6]);
            csConfigParams.ant_cfg_index = (uint8_t)BleApp_atoi(argv[7]);
                    
            if (csConfigParams.ant_cfg_index > 7U)
            {
                status = gBleInvalidParameter_c;
            }
        }
        
        if (status == gBleSuccess_c)
        {
            status = AppLocalization_WriteConfig(&csConfigParams);
        }

        if (status == gBleSuccess_c)
        {
            shell_write("\r\nProcedure parameters set successfully.\r\n");
        }
    }
    else
    {
        shell_write("\r\nUsage: \
                    \r\nsetcsproc max_proc_duration, min_period_between_proc, max_period_between_proc, max_num_proc, min_subevent_len, max_subevent_len, ant_config_idx \
                    \r\n");
    }
    
    if (status == gBleOutOfMemory_c)
    {
        shell_write("\r\nOut of memory\r\n");
    }
    if (status == gBleInvalidParameter_c)
    {
        shell_write("\r\nInvalid parameter\r\n");
    }

    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Trigger CS distance measurement.
*
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellTriggerDistanceMeasurement_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    bleResult_t status = gBleSuccess_c;
    
    if ((uint32_t)argc == 2U)
    {
        deviceId_t deviceId = (uint8_t)BleApp_AsciiToHex(argv[1], FLib_StrLen(argv[1]));
        
        if (deviceId < (uint8_t)gAppMaxConnections_c)
        {
            status = BleApp_TriggerCsDistanceMeasurement(deviceId);
        }
        else
        {
            status = gBleInvalidParameter_c;
        }
    }
    else
    {
        shell_write("\r\nUsage: \
                    \r\ntdm device_id \
                    \r\n");
    }
    
    if (status == gBleInvalidParameter_c)
    {
        shell_write("\r\nInvalid parameter\r\n");
    }
    
    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Set the number of CS procedures.
*
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellSetNumProcs_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    if (mRangeSettings.role != gCsInitiatorRole_c)
    {
        shell_write("\r\nDevice must be in initiator role!\r\n");
    }
    else if (argc == 2)
    {
        uint16_t value = (uint16_t)BleApp_AsciiToHex(argv[1], strlen(argv[1]));

        if (sizeof(uint16_t) == (BleApp_ParseHexValue(argv[1]) && (value > 0U)))
        {
            AppLocalization_SetNumberOfProcedures(value);

            shell_write("\r\nNumber of procedures set successfully.\r\n");
        }
        else
        {
            shell_write("\r\nUsage: setnumprocs [0x0001-0xffff].\r\n");
        }
    }
    else
    {
        shell_write("\r\nUsage: setnumprocs [0x0001-0xffff].\r\n");
    }
    return kStatus_SHELL_Success;
}

/*!*************************************************************************************************
 *  \brief  Converts a string into hex.
 *
 *  \param  [in]    pString     pointer to string
 *  \param  [in]    strLen      string length
 *
 * \return uint32_t value in hex
 **************************************************************************************************/
static uint32_t BleApp_AsciiToHex(char *pString, uint32_t strLen)
{
    uint32_t length = strLen;
    uint32_t retValue = 0U;
    int32_t hexDig = 0;
    bool_t validChar;

    /* Loop until reaching the end of the string or the given length */
    while ((length != 0U) && (pString != NULL))
    {
        hexDig = 0;
        validChar = FALSE;

        /* digit 0-9 */
        if (*pString >= '0' && *pString <= '9')
        {
            hexDig = *pString - '0';
            validChar = TRUE;
        }

        /* character 'a' - 'f' */
        if (*pString >= 'a' && *pString <= 'f')
        {
            hexDig = *pString - 'a' + 10;
            validChar = TRUE;
        }

        /* character 'A' - 'B' */
        if (*pString >= 'A' && *pString <= 'F')
        {
            hexDig = *pString - 'A' + 10;
            validChar = TRUE;
        }

        /* a hex digit is 4 bits */
        if (validChar == TRUE)
        {
            retValue = (uint32_t)((retValue << 4U) ^ (uint32_t)hexDig);
        }

        /* Increment position */
        pString++;
        length--;
    }

    return retValue;
}

/**!************************************************************************************************
 * \brief  Converts a string into an integer.
 *
 * \param [in]    pStr       pointer to string
 *
 * \retval     int32_t       integer converted from string.
 * ************************************************************************************************/
static int32_t BleApp_atoi
(
    char *pStr
)
{
    int32_t res = 0;
    bool_t bIsNegative = FALSE;

    if (*pStr == '-')
    {
        bIsNegative = TRUE;
        pStr++;
    }

    while ((*pStr != '\0') && (*pStr != ' ') && (*pStr >= '0') && (*pStr <= '9'))
    {
        res = res * 10 + *pStr - '0';
        pStr++;
    }

    if (bIsNegative)
    {
        res = -res;
    }

    return res;
}

/*! *********************************************************************************
 * \brief        Parses a string input interpreting its content as a hex number and
 *               writes the value at the input address.
 *
 * \param[in]    pInput         Pointer to string
 *
 * \return       uint8_t        Returns the size of the resulted uint value/array
 ********************************************************************************** */
static uint8_t BleApp_ParseHexValue(char* pInput)
{
    uint8_t i, length = (uint8_t)strlen(pInput);
    uint32_t value;
    uint8_t result = 0U;

    /* If the hex misses a 0, return error. Process single character */
    if ((length == 1U) || (length % 2U) == 0U)
    {
        if(0 == strncmp(pInput, "0x", 2))
        {
            length -= 2U;

            /* Save as little endian hex value */
            value = BleApp_AsciiToHex(pInput + 2, FLib_StrLen(pInput+2));

            FLib_MemCpy(pInput, &value, sizeof(uint32_t));

            result = length/2U;
        }
        else if (length > 1U)
        {
            char octet[2];

            /* Save as big endian hex */
            for(i=0U;i < length / 2U; i++)
            {
                FLib_MemCpy(octet, &pInput[i*2U], 2U);

                pInput[i] = (char)BleApp_AsciiToHex(octet, 2U);
            }
            result = length/2U;
        }
        else
        {
            /* Convert single character from ASCII to hex */
            pInput[0] = (char)BleApp_AsciiToHex(pInput, length);
            result = length;
        }
    }

    return result;
}
