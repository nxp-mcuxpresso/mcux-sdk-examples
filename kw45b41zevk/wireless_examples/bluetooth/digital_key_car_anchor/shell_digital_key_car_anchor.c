/*! *********************************************************************************
* \addtogroup Digital Key Car Anchor Application
* @{
********************************************************************************** */
/*! *********************************************************************************
* \file shell_digital_key_car_anchor.c
*
* Copyright 2021-2024 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
/* Framework / Drivers */
#include "EmbeddedTypes.h"
#include "fsl_adapter_reset.h"
#include "fsl_component_mem_manager.h"
#include "fsl_component_timer_manager.h"
#include "fsl_format.h"
#include "app.h"

/* BLE Host Stack */
#include "gap_interface.h"

#include "app_conn.h"
#include "digital_key_car_anchor.h"
#include "shell_digital_key_car_anchor.h"

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
#if defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
SHELL_HANDLE_DEFINE(g_shellHandle);
/* Shell */
static shell_status_t ShellReset_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellFactoryReset_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellStartDiscovery_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellStopDiscovery_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellDisconnect_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellTriggerTimeSync_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellSetBondingData_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellListBondedDev_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellRemoveBondedDev_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);

#if defined(gHandoverDemo_d) && (gHandoverDemo_d == 1)
static shell_status_t ShellHandoverSendL2cap_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellHandoverAnchorMonitor_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
static shell_status_t ShellHandoverPacketMonitor_Command(shell_handle_t shellHandle, int32_t argc, char * argv[]);
#endif /* gHandoverDemo_d */

static uint8_t BleApp_ParseHexValue(char* pInput);
static uint32_t BleApp_AsciiToHex(char *pString, uint32_t strLen);
static void ShellResetTimeoutTimerCallback(void* pParam);
#endif /* gAppUseShellInApplication_d */
/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
#if defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)

/* shell callback */
static pfShellCallback_t mpfShellEventHandler = NULL;

static shell_command_t mResetCmd =
{
    .pcCommand = "reset",
    .pcHelpString = "\r\n\"reset\": Reset MCU.\r\n",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellReset_Command,
};

static shell_command_t mFactoryResetCmd =
{
    .pcCommand = "factoryreset",
    .pcHelpString = "\r\n\"factoryreset\": Factory Reset.\r\n",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellFactoryReset_Command,
};

static shell_command_t mSdCmd =
{
    .pcCommand = "sd",
    .pcHelpString = "\r\n\"sd\": Start Discovery for Owner Pairing or Passive Entry.\r\n",
    .cExpectedNumberOfParameters = SHELL_IGNORE_PARAMETER_COUNT,
    .pFuncCallBack = ShellStartDiscovery_Command,
};

static shell_command_t mSpdCmd =
{
    .pcCommand = "spd",
    .pcHelpString = "\r\n\"spd\": Stop Discovery.\r\n",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellStopDiscovery_Command,
};

static shell_command_t mDcntCmd =
{
    .pcCommand = "dcnt",
    .pcHelpString = "\r\n\"dcnt\": Disconnect all peers.\r\n",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellDisconnect_Command,
};

static shell_command_t mTriggerTimeSyncCmd =
{
    .pcCommand = "ts",
    .pcHelpString = "\r\n\"ts\": Trigger a Time Sync from Device.\r\n",
    .cExpectedNumberOfParameters = SHELL_IGNORE_PARAMETER_COUNT,
    .pFuncCallBack = ShellTriggerTimeSync_Command,
};

static shell_command_t mSetBondingDataCmd =
{
    .pcCommand = "setbd",
    .pcHelpString = "\r\n\"setbd\": Set bonding data.\r\n",
    .cExpectedNumberOfParameters = SHELL_IGNORE_PARAMETER_COUNT,
    .pFuncCallBack = ShellSetBondingData_Command,
};

static shell_command_t mListBondedDevCmd =
{
    .pcCommand = "listbd",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellListBondedDev_Command,
    .pcHelpString = "\r\n\"listbd\": List bonded devices.\r\n",
};

static shell_command_t mRemoveBondedDevCmd =
{
    .pcCommand = "removebd",
    .cExpectedNumberOfParameters = SHELL_IGNORE_PARAMETER_COUNT,
    .pFuncCallBack = ShellRemoveBondedDev_Command,
    .pcHelpString = "\r\n\"removebd\": Remove bonded devices.\r\n",
};

#if defined(gHandoverDemo_d) && (gHandoverDemo_d == 1)
static shell_command_t mHandoverSendL2capCmd =
{
    .pcCommand = "send",
    .cExpectedNumberOfParameters = 0,
    .pFuncCallBack = ShellHandoverSendL2cap_Command,
    .pcHelpString = "\r\n\"send\": Send a message over the L2CAP Credit Based channel.\r\n",
};

static shell_command_t mHandoverAnchorMonitorCmd =
{
    .pcCommand = "monitor",
    .cExpectedNumberOfParameters = SHELL_IGNORE_PARAMETER_COUNT,
    .pFuncCallBack = ShellHandoverAnchorMonitor_Command,
    .pcHelpString = "\r\n\"monitor\": Start or stop SN/NESN anchor monitoring.\r\n",
};

static shell_command_t mHandoverPacketMonitorCmd =
{
    .pcCommand = "packetmon",
    .cExpectedNumberOfParameters = SHELL_IGNORE_PARAMETER_COUNT,
    .pFuncCallBack = ShellHandoverPacketMonitor_Command,
    .pcHelpString = "\r\n\"packetmon\": Start or stop packet monitoring.\r\n",
};
#endif /* gHandoverDemo_d */

#endif /* gAppUseShellInApplication_d */

static TIMER_MANAGER_HANDLE_DEFINE(mResetTmrId);

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
void AppShellInit(char* prompt)
{
#if defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
    shell_status_t status = kStatus_SHELL_Error;
    
    /* Avoid compiler warning in release mode. */
    (void)status;
    status = SHELL_Init((shell_handle_t)g_shellHandle, (serial_handle_t)gSerMgrIf, prompt);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mResetCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mFactoryResetCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mSdCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mSpdCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mDcntCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mTriggerTimeSyncCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mSetBondingDataCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mListBondedDevCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mRemoveBondedDevCmd);
    assert(kStatus_SHELL_Success == status);
#if defined(gHandoverDemo_d) && (gHandoverDemo_d == 1)
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mHandoverSendL2capCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mHandoverAnchorMonitorCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mHandoverPacketMonitorCmd);
    assert(kStatus_SHELL_Success == status);
#endif /* gHandoverDemo_d */
#endif
}

#if defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
/*! *********************************************************************************
 * \brief        Register function to handle commands from shell
 *
 * \param[in]    pCallback       event handler
 ********************************************************************************** */
void AppShell_RegisterCmdHandler(pfBleCallback_t pfShellEventHandler)
{
    mpfShellEventHandler = pfShellEventHandler;
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
* \param[in]    shellHandle     Shell handle
* \param[in]    argc            Number of arguments
* \param[in]    argv            Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
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
    if(mpfShellEventHandler != NULL)
    {
        appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
        if(pEventData != NULL)
        {
            pEventData->appEvent = mAppEvt_Shell_Reset_Command_c;
            if (gBleSuccess_c != App_PostCallbackMessage(mpfShellEventHandler, pEventData))
            {
               (void)MEM_BufferFree(pEventData);
            }
        }
    }
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
    if(mpfShellEventHandler != NULL)
    {
        appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
        if(pEventData != NULL)
        {
            pEventData->appEvent = mAppEvt_Shell_FactoryReset_Command_c;
            if (gBleSuccess_c != App_PostCallbackMessage(mpfShellEventHandler, pEventData))
            {
               (void)MEM_BufferFree(pEventData);
            }
        }
    }

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
static shell_status_t ShellStartDiscovery_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    const char* ownerPairingCmd = "op";
    const char* passiveEntryCmd = "pe";
    if ((uint32_t)argc == 2U)
    {
        if (TRUE == FLib_MemCmp(argv[1], ownerPairingCmd, 2))
        {
            if(mpfShellEventHandler != NULL)
            {
                appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
                if(pEventData != NULL)
                {
                    pEventData->appEvent = mAppEvt_Shell_ShellStartDiscoveryOP_Command_c;
                    if (gBleSuccess_c != App_PostCallbackMessage(mpfShellEventHandler, pEventData))
                    {
                       (void)MEM_BufferFree(pEventData);
                    }
                }
            }
        }
        else if (TRUE == FLib_MemCmp(argv[1], passiveEntryCmd, 2))
        {
            if(mpfShellEventHandler != NULL)
            {
                appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
                if(pEventData != NULL)
                {
                    pEventData->appEvent = mAppEvt_Shell_ShellStartDiscoveryPE_Command_c;
                    if (gBleSuccess_c != App_PostCallbackMessage(mpfShellEventHandler, pEventData))
                    {
                       (void)MEM_BufferFree(pEventData);
                    }
                }
            }
        }
        else
        {
            shell_write("\r\nUsage: \
                        \r\nsd op - Start advertising for Owner Pairing \
                        \r\nsd pe - Start advertising for Passive Entry \
                        \r\n");
        }
    }
    else
    {
            shell_write("\r\nUsage: \
                        \r\nsd op - Start advertising for Owner Pairing \
                        \r\nsd pe - Start advertising for Passive Entry \
                        \r\n");
    }
    return kStatus_SHELL_Success;
}
                
/*! *********************************************************************************
* \brief        Stop discovery, if active.
*
* \param[in]    shellHandle    Shell handle
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellStopDiscovery_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    if(mpfShellEventHandler != NULL)
    {
        appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
        if(pEventData != NULL)
        {
            pEventData->appEvent = mAppEvt_Shell_StopDiscovery_Command_c;
            if (gBleSuccess_c != App_PostCallbackMessage(mpfShellEventHandler, pEventData))
            {
               (void)MEM_BufferFree(pEventData);
            }
        }
    }
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
    if(mpfShellEventHandler != NULL)
    {
        appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
        if(pEventData != NULL)
        {
            pEventData->appEvent = mAppEvt_Shell_Disconnect_Command_c;
            if (gBleSuccess_c != App_PostCallbackMessage(mpfShellEventHandler, pEventData))
            {
               (void)MEM_BufferFree(pEventData);
            }
        }
    }
    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Trigger a Time Sync from Device.
*
* \param[in]    shellHandle    Shell handle
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellTriggerTimeSync_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    if ((uint32_t)argc == 2U)
    {
        deviceId_t deviceId = (uint8_t)BleApp_AsciiToHex(argv[1], FLib_StrLen(argv[1]));
        if (deviceId < (uint8_t)gAppMaxConnections_c)
        {
            if(mpfShellEventHandler != NULL)
            {
                appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
                if(pEventData != NULL)
                {
                    pEventData->appEvent = mAppEvt_Shell_TriggerTimeSync_Command_c;
                    pEventData->eventData.peerDeviceId = deviceId;
                    if (gBleSuccess_c != App_PostCallbackMessage(mpfShellEventHandler, pEventData))
                    {
                       (void)MEM_BufferFree(pEventData);
                    }
                }
            }
        }
        else
        {
            shell_write("\r\nUsage: \
                        \r\nts peer_id \
                        \r\n");
        }
    }
    else
    {
        shell_write("\r\nUsage: \
                    \r\nts peer_id \
                    \r\n");
    }

    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Save Bonding Data on device.
*
* \param[in]    shellHandle    Shell handle
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellSetBondingData_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    if (argc == 6)
    {
        if(mpfShellEventHandler != NULL)
        {
            appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t) + sizeof(appBondingData_t));
            if(pEventData != NULL)
            {
                pEventData->appEvent = mAppEvt_Shell_SetBondingData_Command_c;
                pEventData->eventData.pData = &pEventData[1];
                appBondingData_t *pAppBondingEventData = pEventData->eventData.pData;
                if ( sizeof(uint8_t) == BleApp_ParseHexValue(argv[1]) )
                {
                    pAppBondingEventData->nvmIndex = (uint8_t)*argv[1];
                }
                if ( sizeof(bleAddressType_t) == BleApp_ParseHexValue(argv[2]) )
                {
                    pAppBondingEventData->addrType = (uint8_t)*argv[2];
                }
                
                if ( gcBleDeviceAddressSize_c ==  BleApp_ParseHexValue(argv[3]) )
                {
                    FLib_MemCpy(pAppBondingEventData->deviceAddr, argv[3], gcBleDeviceAddressSize_c);
                }
                
                if ( gcSmpMaxLtkSize_c == BleApp_ParseHexValue(argv[4]) )
                {
                    FLib_MemCpy(pAppBondingEventData->aLtk, argv[4], gcSmpMaxLtkSize_c);
                }
                
                if ( gcSmpIrkSize_c ==  BleApp_ParseHexValue(argv[5]) )
                {
                    FLib_MemCpy(pAppBondingEventData->aIrk, argv[5], gcSmpIrkSize_c);
                }

                if (gBleSuccess_c != App_PostCallbackMessage(mpfShellEventHandler, pEventData))
                {
                   (void)MEM_BufferFree(pEventData);
                }
            }
        }
    }
    else
    {
        shell_write("\r\nUsage: \
                    \r\nsetbd nvm_index addr_type peer_device_address ltk irk  \
                    \r\n");
    }
    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        List bonded devices.
*
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellListBondedDev_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    if(mpfShellEventHandler != NULL)
    {
        appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
        if(pEventData != NULL)
        {
            pEventData->appEvent = mAppEvt_Shell_ListBondedDev_Command_c;
            if (gBleSuccess_c != App_PostCallbackMessage(mpfShellEventHandler, pEventData))
            {
               (void)MEM_BufferFree(pEventData);
            }
        }
    }
    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        remove bonded devices.
*
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellRemoveBondedDev_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    if (argc == 2)
    {
        if(mpfShellEventHandler != NULL)
        {
            appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
            if(pEventData != NULL)
            {
                pEventData->appEvent = mAppEvt_Shell_RemoveBondedDev_Command_c;
                if ( sizeof(uint8_t) == BleApp_ParseHexValue(argv[1]) )
                {
                    /* Store nvm index to be removed in eventData.peerDeviceId  */
                    pEventData->eventData.peerDeviceId = (uint8_t)*argv[1];
                    if (gBleSuccess_c != App_PostCallbackMessage(mpfShellEventHandler, pEventData))
                    {
                       (void)MEM_BufferFree(pEventData);
                    }
                }
            }
        }
    }
    else
    {
        shell_write("\r\nUsage: \
                    \r\nremovebd nvm_index \
                    \r\n");
    }
    return kStatus_SHELL_Success;
}

#if defined(gHandoverDemo_d) && (gHandoverDemo_d == 1)
/*! *********************************************************************************
* \brief        Send a predefined L2CAP Credit Based message for demo purposes.
*
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellHandoverSendL2cap_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{

    if(mpfShellEventHandler != NULL)
    {
        appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
        if(pEventData != NULL)
        {
            pEventData->appEvent = mAppEvt_Shell_HandoverSendL2cap_Command_c;
            if (gBleSuccess_c != App_PostCallbackMessage(mpfShellEventHandler, pEventData))
            {
               (void)MEM_BufferFree(pEventData);
            }
        }
    }

    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Start or stop anchor monitoring via SN/NESN.
*
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellHandoverAnchorMonitor_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    const char* startCmd = "start";
    const char* stopCmd = "stop";

    if ((uint32_t)argc == 3U)
    {
        deviceId_t deviceId = (uint8_t)BleApp_AsciiToHex(argv[1], FLib_StrLen(argv[1]));
        
        if (deviceId < (uint8_t)gAppMaxConnections_c)
        {
            if (TRUE == FLib_MemCmp(argv[2], startCmd, 4))
            {
                if(mpfShellEventHandler != NULL)
                {
                    appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
                    if(pEventData != NULL)
                    {
                        pEventData->eventData.monitorStart.monitorMode = gRssiSniffingMode_c;
                        pEventData->appEvent = mAppEvt_Shell_HandoverStartAnchorMonitor_Command_c;
                        pEventData->eventData.monitorStart.deviceId = deviceId;
                        if (gBleSuccess_c != App_PostCallbackMessage(mpfShellEventHandler, pEventData))
                        {
                            (void)MEM_BufferFree(pEventData);
                        }
                    }
                }
            }
            else if (TRUE == FLib_MemCmp(argv[2], stopCmd, 4))
            {
                if(mpfShellEventHandler != NULL)
                {
                    appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
                    if(pEventData != NULL)
                    {
                        pEventData->appEvent = mAppEvt_Shell_HandoverStopAnchorMonitor_Command_c;
                        pEventData->eventData.peerDeviceId = deviceId;
                        if (gBleSuccess_c != App_PostCallbackMessage(mpfShellEventHandler, pEventData))
                        {
                            (void)MEM_BufferFree(pEventData);
                        }
                    }
                }
            }
            else
            {
                shell_write("\r\nUsage: monitor deviceId start|stop\r\n");
            }
        }
        else
        {
            shell_write("\r\nInvalid deviceId\r\n");
        }
    }
    else
    {
            shell_write("\r\nUsage: monitor deviceId start|stop\r\n");
    }
    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
* \brief        Start or stop packet monitoring.
*
* \param[in]    argc           Number of arguments
* \param[in]    argv           Pointer to arguments
*
* \return       shell_status_t  Returns the command processing status
********************************************************************************** */
static shell_status_t ShellHandoverPacketMonitor_Command(shell_handle_t shellHandle, int32_t argc, char * argv[])
{
    const char* startCmd = "start";
    const char* stopCmd = "stop";

    if ((uint32_t)argc == 3U)
    {
        deviceId_t deviceId = (uint8_t)BleApp_AsciiToHex(argv[1], FLib_StrLen(argv[1]));
        
        if (deviceId < (uint8_t)gAppMaxConnections_c)
        {
            if (TRUE == FLib_MemCmp(argv[2], startCmd, 4))
            {
                if(mpfShellEventHandler != NULL)
                {
                    appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
                    
                    if(pEventData != NULL)
                    {
                        pEventData->appEvent = mAppEvt_Shell_HandoverStartAnchorMonitor_Command_c;
                        pEventData->eventData.monitorStart.deviceId = deviceId;
                        pEventData->eventData.monitorStart.monitorMode = gPacketMode_c;
                        if (gBleSuccess_c != App_PostCallbackMessage(mpfShellEventHandler, pEventData))
                        {
                            (void)MEM_BufferFree(pEventData);
                        }
                    }
                }
            }
            else if (TRUE == FLib_MemCmp(argv[2], stopCmd, 4))
            {
                if(mpfShellEventHandler != NULL)
                {
                    appEventData_t *pEventData = MEM_BufferAlloc(sizeof(appEventData_t));
                    
                    if(pEventData != NULL)
                    {
                        pEventData->appEvent = mAppEvt_Shell_HandoverStopAnchorMonitor_Command_c;
                        pEventData->eventData.peerDeviceId = deviceId;
                        if (gBleSuccess_c != App_PostCallbackMessage(mpfShellEventHandler, pEventData))
                        {
                            (void)MEM_BufferFree(pEventData);
                        }
                    }
                }
            }
            else
            {
                shell_write("\r\nUsage: packetmon deviceId start|stop\r\n");
            }
        }
        else
        {
            shell_write("\r\nInvalid deviceId\r\n");
        }
    }
    else
    {
            shell_write("\r\nUsage: packetmon deviceId start|stop\r\n");
    }
    return kStatus_SHELL_Success;
}
#endif /* gHandoverDemo_d */
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
            value = BleApp_AsciiToHex(&pInput[2], FLib_StrLen(&pInput[2]));

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
#endif