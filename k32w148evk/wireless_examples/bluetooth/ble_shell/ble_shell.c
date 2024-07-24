/*! *********************************************************************************
 * \addtogroup Bluetooth Shell Application
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright 2015 Freescale Semiconductor, Inc.
* Copyright 2016-2024 NXP
*
*
* \file
*
* This file is the source file for the Bluetooth Shell Application
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

#include "fsl_component_button.h"
#include "fsl_component_led.h"
#include "fsl_component_timer_manager.h"
#include "fsl_shell.h"
#include "fsl_component_panic.h"
#include "fsl_component_mem_manager.h"
#include "fsl_adapter_reset.h"
#include "app.h"
#include "fwk_platform_ble.h"

#include "RNG_Interface.h"
#include "FunctionLib.h"
#include "fsl_os_abstraction.h"
#include "board.h"
#include "fwk_platform.h"

/* BLE Host Stack */
#include "gatt_interface.h"
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gatt_database.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"

/* Shell APIs*/
#include "shell_gap.h"
#include "shell_gatt.h"
#include "shell_gattdb.h"
#include "shell_thrput.h"

#include "ble_conn_manager.h"
#include "ble_shell.h"

#include "app_conn.h"
#include "board.h"
#include "app.h"

#if (defined(CPU_MKW37A512VFT4) || defined(CPU_MKW38A512VFT4))
#if (defined(gRFCalibration_d) && (gRFCalibration_d > 0))
#include "board.h"
#include "nxp2p4_xcvr.h"
#include "nxp_xcvr_gfsk_bt_0p5_h_0p5_config.h"
#include "nxp_xcvr_coding_config.h"
#include "Flash_Adapter.h"
#include "controller_interface.h"
#endif
#endif /* (defined(CPU_MKW37A512VFT4) || defined(CPU_MKW38A512VFT4)) */

/************************************************************************************
*************************************************************************************
* Extern functions
*************************************************************************************
************************************************************************************/
SHELL_HANDLE_DEFINE(g_shellHandle);

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
static uint8_t mSupressEvents = 0;

#define mShellThrBufferCountDefault_c      (1000U)
#define mShellThrBufferSizeDefault_c       (gAttMaxNotifIndDataSize_d(gAttMaxMtu_c))

#if (defined(CPU_MKW37A512VFT4) || defined(CPU_MKW38A512VFT4))
#if (defined(gRFCalibration_d) && (gRFCalibration_d > 0))
/*! Default trimming value for 32MHz crystal8 */
#define DEFAULT_TRIM_VALUE      0x4B
#endif
#endif /* (defined(CPU_MKW37A512VFT4) || defined(CPU_MKW38A512VFT4)) */
/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
static shell_status_t ShellReset_Command(shell_handle_t shellHandle, int32_t argc, char **argv);

/************************************************************************************
 *************************************************************************************
 * Private memory declarations
 *************************************************************************************
 ************************************************************************************/
/* Shell */
static shell_command_t mGapCmd =
{
    .pcCommand = "gap",
    .pcHelpString = "\"gap\": Contains commands for advertising, scanning, connecting, pairing or disconnecting.\r\n"
                    "  gap address [hexvalue] -peer [hexValue] -type \r\n"
                    "  gap devicename [devicename]\r\n"
                    "  gap advcfg [-interval intervalInMs]  [-type type]\r\n"
                    "  gap advstart\r\n"
                    "  gap advstop\r\n"
                    "  gap advdata [-erase] [type payload]\r\n"
                    "  gap scanstart [filter]\r\n"
                    "  gap scanstop\r\n"
#if defined(BLE_SHELL_DBAF_SUPPORT) && (BLE_SHELL_DBAF_SUPPORT)
                    "  gap scancfg [-type type] [-interval intervalInMs] [-window windowInMs] [-phy scanningPhy] [-duration durationInMs] [-period periodInMs] [-filter filterPolicy]\r\n"
#else
                    "  gap scancfg [-type type] [-interval intervalInMs] [-window windowInMs] [-phy scanningPhy] [-duration durationInMs] [-period periodInMs]\r\n"
#endif /* BLE_SHELL_DBAF_SUPPORT */
                    "  gap scandata [-erase] [type payload]\r\n"
#if defined(BLE_SHELL_DBAF_SUPPORT) && (BLE_SHELL_DBAF_SUPPORT)
                    "  gap connectcfg [-interval intervalInMs] [-latency latency] [-timeout timeout] [-filter filterPolicy]\r\n"
#else
                    "  gap connectcfg [-interval intervalInMs] [-latency latency] [-timeout timeout]\r\n"
#endif /* BLE_SHELL_DBAF_SUPPORT */
                    "  gap connect scannedDeviceId\r\n"
                    "  gap disconnect <peerID>\r\n"
                    "  gap connupdate <peerID> mininterval maxinterval latency timeout\r\n"
                    "  gap paircfg [-usebonding usebonding] [-seclevel seclevel] [-keyflags flags]\r\n"
                    "  gap pair <peerID>\r\n"
                    "  gap enterpin <peerID> pin\r\n"
                    "  gap bonds [-erase] [-remove deviceIndex]\r\n"
                    "  gap phy <peerID> [-rx rxPhy] [-tx txPhy] [-o phyOptions]\r\n"
                    "  gap txpower [adv/conn] [powerLevel]\r\n"
                    "  gap rssimonitor [peerID]/[peerAddress] [-c]\r\n"
                    "  gap rssistop\r\n"
#if defined(BLE_SHELL_AE_SUPPORT) && (BLE_SHELL_AE_SUPPORT)
                    "  gap extadvstart\r\n"
                    "  gap extadvstop\r\n"
                    "  gap extadvcfg [-min minIntervalInMs] [-max maxIntervalInMs] [-type advProperties] [-phy1 primaryPHY] [-phy2 secondaryPHY] [-tx advTxPower] [-sn enableScanNotification]\r\n"
                    "  gap extadvdata [-erase] [type payload]\r\n"
                    "  gap extscandata [-erase] [type payload]\r\n"
                    "  gap periodicstart\r\n"
                    "  gap periodicstop\r\n"
                    "  gap periodiccfg [-mininterval minIntervalInMs] [-maxinterval maxIntervalInMs] [-txpower advTxPower]\r\n"
                    "  gap periodicdata [type] [payload]\r\n"
                    "  gap periodicsync [-peer peerAddr] [-type peerAddrType]\r\n"
                    "  gap periodicsyncstop\r\n"
#endif /* BLE_SHELL_AE_SUPPORT */
#if BLE_SHELL_DBAF_SUPPORT
                    "  gap setdecinstr\r\n"
                    "  gap adddecinstr [-group testGroup] [-criteria passCriteria] [-field relevantField] "
                                      "[-restagkey resolvableTagKey] "
                                      "[-arbmask arbitraryDataMask] [-arbtarget arbitraryDataTarget] "
                                      "[-rssimin rssiMin] [-rssimax rssiMax] "
                                      "[-lossmin pathLossMin] [-lossmax pathLossMax] "
                                      "[-advacheck advACheck] [-add1type advAAddress1Type] [-add1 advAAdress1] [-add2type advAAddress2Type] [-add2 advAAdress2] "
                                      "[-advmode advMode]\r\n"
                    "  gap deldecinstr\r\n"
                    "  gap extadvdecdata [-key key] [-prand rand] [-decdata decisionData] [-datalen dataLength] [-restag resolvableTagPresent]\r\n"
#endif /* BLE_SHELL_DBAF_SUPPORT */
                    ,
    .pFuncCallBack = ShellGap_Command,
    .cExpectedNumberOfParameters = 0xFF,
    .link = {0}
};

static shell_command_t mGattCmd =
{
    .pcCommand = "gatt",
    .pcHelpString = "\"gatt\": Contains commands for service discovery, read, write, notify and indicate. "
                               "Values in hex must be formatted as 0xXX..X\r\n"
                    "  gatt discover <peerID> [-all] [-service serviceUuid16InHex] \r\n"
                    "  gatt read <peerID> handle\r\n"
                    "  gatt write <peerID> handle valueInHex\r\n"
                    "  gatt writecmd <peerID> handle valueInHex\r\n"
                    "  gatt notify <peerID> handle\r\n"
                    "  gatt indicate <peerID> handle\r\n",
    .pFuncCallBack = ShellGatt_Command,
    .cExpectedNumberOfParameters = 0xFF,
    .link = {0}
};

static shell_command_t mGattDbCmd =
{
    .pcCommand = "gattdb",
    .pcHelpString = "\"gattdb\": Contains commands for adding services, reading and writing characteristics on the local database."
                                 "Values in hex must be formatted as 0xXX..X\r\n"
                    "  gattdb read handle\r\n"
                    "  gattdb write handle valueInHex\r\n"
                    "  gattdb addservice serviceUuid16InHex\r\n"
                    "  gattdb erase\r\n",
    .pFuncCallBack = ShellGattDb_Command,
    .cExpectedNumberOfParameters = 0xFF,
    .link = {0}
};

static shell_command_t mThrputCmd =
{
    .pcCommand = "thrput",
    .pcHelpString = "\"thrput\": Contains commands for setting up and running throughput test\r\n"
                    "  thrput start <peerID> tx [-c packet count] [-s payload size]\r\n"
                    "  thrput start <peerID> rx [-ci min max]\r\n"
                    "  thrput stop\r\n",
    .pFuncCallBack = ShellThrput_Command,
    .cExpectedNumberOfParameters = 0xFF,
    .link = {0}
};

static shell_command_t mResetCmd =
{
    .pcCommand = "reset",
    .pcHelpString = "\"reset\": Reset MCU\r\n",
    .pFuncCallBack = ShellReset_Command,
    .cExpectedNumberOfParameters = 0,
    .link = {0}
};

#if (defined(CPU_MKW37A512VFT4) || defined(CPU_MKW38A512VFT4))
#if (defined(gRFCalibration_d) && (gRFCalibration_d > 0))
static cmd_tbl_t mCalibrationCmd =
{
    .name = "calibration",
    .maxargs = 4,
    .repeatable = 0,
    .cmd = ShellCalibration_Command,
    .usage = (char*)mpCalibrationHelp,
    .help = "Enalbe/Disable RF calibration. Set/Get frequency trim and RSSI adjustment"
};

/* Command structure type definition */
typedef struct calibrationCmds_tag
{
    char*       name;
    int8_t      (*cmd)(uint8_t argc, char * argv[]);
} thrCmds_t;

static int8_t ShellCalibration_EnableRFCalibration(uint8_t argc, char * argv[]);
static int8_t ShellCalibration_GetXtal32MHzTrim(uint8_t argc, char * argv[]);
static int8_t ShellCalibration_SetXtal32MHzTrim(uint8_t argc, char * argv[]);
static int8_t ShellCalibration_GetRssiAdjustment(uint8_t argc, char * argv[]);
static int8_t ShellCalibration_SetRssiAdjustment(uint8_t argc, char * argv[]);

/* Calibration test shell commands */
static const thrCmds_t mCalibrationShellCmds[] =
{
    {"enable",         ShellCalibration_EnableRFCalibration},
    {"getxtaltrim",    ShellCalibration_GetXtal32MHzTrim},
    {"setxtaltrim",    ShellCalibration_SetXtal32MHzTrim},
    {"getrssiadj",     ShellCalibration_GetRssiAdjustment},
    {"setrssiadj",     ShellCalibration_SetRssiAdjustment},
};
#endif /* gRFCalibration_d */
#endif /* (defined(CPU_MKW37A512VFT4) || defined(CPU_MKW38A512VFT4)) */

static char maPrompt[] = "BLE Shell>";

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
/* Active connections bitmask */
uint8_t gActiveConnections = 0U;

/* Throughput test configuration structure */
thrConfig_t gThroughputConfig[gAppMaxConnections_c];

/* When enabled generic events are redirected to ShellThr_GenericCallback() */
bool_t gUseShellThrGenericCb = FALSE;

#if (defined(CPU_MKW37A512VFT4) || defined(CPU_MKW38A512VFT4))
#if (defined(gRFCalibration_d) && (gRFCalibration_d > 0))
static bool_t mCalibrationEnabled = FALSE;
static uint8_t gXtalTrimValue = 0U;
static int8_t  gRssiValue = 0;
#endif /* gRFCalibration_d */
#endif /* (defined(CPU_MKW37A512VFT4) || defined(CPU_MKW38A512VFT4)) */

#if defined(BLE_SHELL_DBAF_SUPPORT) && (BLE_SHELL_DBAF_SUPPORT)
extern uint8_t gHostInitExpmFeatures;
#endif /* BLE_SHELL_DBAF_SUPPORT */

/************************************************************************************
 *************************************************************************************
 * Private functions prototypes
 *************************************************************************************
 ************************************************************************************/
static void BluetoothLEHost_Initialized(void);

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief    Initializes application specific functionality before the BLE stack init.
*
********************************************************************************** */
void BleApp_Init(void)
{
    /* UI */
    shell_status_t status = kStatus_SHELL_Error;
 
    status = SHELL_Init((shell_handle_t)g_shellHandle, (serial_handle_t)gSerMgrIf, (char *)maPrompt);
    assert(kStatus_SHELL_Success == status);

    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mGapCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mGattCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mGattDbCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mThrputCmd);
    assert(kStatus_SHELL_Success == status);
    status = SHELL_RegisterCommand((shell_handle_t)g_shellHandle, &mResetCmd);
    assert(kStatus_SHELL_Success == status);
    
    /* Clear warning for release mode*/
    (void)status;

#if (defined(CPU_MKW37A512VFT4) || defined(CPU_MKW38A512VFT4))
#if (defined(gRFCalibration_d) && (gRFCalibration_d > 0))
    (void)shell_register_function(&mCalibrationCmd);
#endif /* gRFCalibration_d */
#endif /* (defined(CPU_MKW37A512VFT4) || defined(CPU_MKW38A512VFT4)) */

    /* Initialize throughput structures */
    for (uint32_t iCount = 0; iCount < gAppMaxConnections_c; iCount++)
    {
        /* Throughput test default configuration */
        gThroughputConfig[iCount].buffCnt = mShellThrBufferCountDefault_c;
        gThroughputConfig[iCount].buffSz =  mShellThrBufferSizeDefault_c;
    }
}

/*! *********************************************************************************
 * \brief        Parses a string input interpreting its content as a hex number and
 *               writes the value at the input address.
 *
 * \param[in]    pInput         Pointer to string
 *
 * \return       uint8_t        Returns the size of the resulted uint value/array
 ********************************************************************************** */
uint8_t BleApp_ParseHexValue(char* pInput)
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
/**!************************************************************************************************
 * \brief  Converts a string into an integer.
 *
 * \param [in]    pStr       pointer to string
 *
 * \retval     int32_t       integer converted from string.
 * ************************************************************************************************/
int32_t BleApp_atoi
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
/*!*************************************************************************************************
 *  \brief  Converts a string into hex.
 *
 *  \param  [in]    pString     pointer to string
 *  \param  [in]    strLen      string length
 *
 * \return uint32_t value in hex
 **************************************************************************************************/
uint32_t BleApp_AsciiToHex
(
    char *pString,
    uint32_t strLen
)
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
/************************************************************************************
 *************************************************************************************
 * Private functions
 *************************************************************************************
 ************************************************************************************/
/*! *********************************************************************************
 * \brief        Handles BLE generic callback.
 *
 * \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
 ********************************************************************************** */
static void BleApp_GenericCallback (gapGenericEvent_t* pGenericEvent)
{
    BleConnManager_GenericEvent(pGenericEvent);

    if (mSupressEvents == 0U)
    {
        ShellGap_GenericCallback(pGenericEvent);

        if (gUseShellThrGenericCb == TRUE)
        {
            /* Redirect generic events to ShellThr_GenericCallback */
            ShellThr_GenericCallback(pGenericEvent);
        }
    }
    else
    {
        mSupressEvents--;
    }

}

/*! *********************************************************************************
 * \brief        Reset MCU.
 *
 ********************************************************************************** */
static shell_status_t ShellReset_Command(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    BOARD_UninitAppConsole();
    HAL_ResetMCU();

    return kStatus_SHELL_Success;
}

static void BluetoothLEHost_Initialized(void)
{
	
    /* Adding GAP and GATT services in the database */
    (void)ShellGattDb_Init();

     /* Register for callbacks*/
    (void)App_RegisterGattServerCallback(ShellGatt_ServerCallback);
    (void)App_RegisterGattClientProcedureCallback(ShellGatt_ClientCallback);
    (void)App_RegisterGattClientIndicationCallback(ShellGatt_IndicationCallback);
    (void)App_RegisterGattClientNotificationCallback(ShellGatt_NotificationCallback);

    /* Configure GAP */
#if defined(BLE_SHELL_AE_SUPPORT) && (BLE_SHELL_AE_SUPPORT)
    (void)Gap_SetExtAdvertisingParameters(&gExtAdvParams);
    mSupressEvents += 1U;
#endif /* BLE_SHELL_AE_SUPPORT */
    (void)Gap_ReadPublicDeviceAddress();
    (void)Gap_SetDefaultPairingParameters(&gPairingParameters);
    mSupressEvents += 1U;

    /* Register shell commads */
    BleApp_Init();

#if (defined(CPU_MKW37A512VFT4) || defined(CPU_MKW38A512VFT4))
#if (defined(gRFCalibration_d) && (gRFCalibration_d > 0))
    hardwareParameters_t hwParams = {0};
    /* get default RSSI and xtal trim values from hardware params structure */
    (void)NV_ReadHWParameters(&hwParams);
    /* Set XTAL trim value */
    if ((uint8_t)hwParams.xtalTrim != 0xFFU)
    {
        (void)XCVR_SetXtalTrim((uint8_t)hwParams.xtalTrim);
    }
    /* Set RSSI Adjustment Value */
    (void)XCVR_SetRssiAdjustment(hwParams.rssiAdjustment);
#endif
#endif

#if (defined(gAppLedCnt_c) && (gAppLedCnt_c > 0))
    LedStartFlashingAllLeds();
#endif
}

/*! *********************************************************************************
* \brief  This is the initialization function for each application. This function
*         should contain all the initialization code required by the bluetooth demo
********************************************************************************** */
void BluetoothLEHost_AppInit(void)
{
    /* Set generic callback */
    BluetoothLEHost_SetGenericCallback(BleApp_GenericCallback);

    /* Enable experimental features */
#if defined(BLE_SHELL_DBAF_SUPPORT) && (BLE_SHELL_DBAF_SUPPORT)
    gHostInitExpmFeatures |= gExpmDecisionBasedAdvertisingFilteringBit_d;
#endif /* BLE_SHELL_DBAF_SUPPORT */

    /* Initialize Bluetooth Host Stack */
    BluetoothLEHost_Init(BluetoothLEHost_Initialized);
}

#if (defined(CPU_MKW37A512VFT4) || defined(CPU_MKW38A512VFT4))
#if (defined(gRFCalibration_d) && (gRFCalibration_d > 0))
/*! *********************************************************************************
 * \brief        RF Calibration command
 *
 * \param[in]    argc    shell argument count
 *
 * \param[in]    argv    shell argument value
 *
 * \return       status of the called callback
 ********************************************************************************** */
int8_t ShellCalibration_Command(uint8_t argc, char * argv[])
{
    uint8_t i;
    int8_t status = CMD_RET_USAGE;

    /* Must have at least one argument */
    if (argc > 1U)
    {
        for (i = 0; i < NumberOfElements(mCalibrationShellCmds); i++)
        {
            if (0 == strcmp((char*)argv[1], mCalibrationShellCmds[i].name) )
            {
                /* Handle the Calibration command in the appropriate callback */
                status = mCalibrationShellCmds[i].cmd(argc - 2U, (char **)(&argv[2]));
                break;
            }
        }
    }

    return status;
}

/*! *********************************************************************************
 * \brief        Handles the calibration enable command.
 *
 * \param[in]    argc     argument count
 *
 * \param[in]    argv     argument value
 *
 * \return       CMD_RET_FAILURE - wrong argument count
                 CMD_RET_USAGE - inappropriate argument value
                 CMD_RET_SUCCESS
 ********************************************************************************** */
static int8_t ShellCalibration_EnableRFCalibration(uint8_t argc, char * argv[])
{
    bool_t enable;
    int8_t status = CMD_RET_SUCCESS;
    uint32_t frequency = 2402000000UL;
    uint32_t enable_val;

    const xcvr_config_t *xcvrConfig = &xcvr_gfsk_bt_0p5_h_0p5_1mbps_full_config;
    const xcvr_coding_config_t *rbmeConfig = &xcvr_ble_coded_s8_config;
    xcvrStatus_t xvr_status;

    if ((gXtalTrimValue == 0U) && (gRssiValue == 0))
    {
        hardwareParameters_t hwParams = {0};
        /* get default RSSI value from hardware params structure */
        (void)NV_ReadHWParameters(&hwParams);
        gRssiValue = hwParams.rssiAdjustment;
        /* default 32MHz Xtal trim */
        gXtalTrimValue = DEFAULT_TRIM_VALUE;
    }

    if ((argc > 0U) && (argc <= 2U))
    {
        enable_val = (uint32_t)BleApp_atoi(argv[0]);
        if (enable_val == 1U)
        {
            enable = TRUE;
        }
        else if (enable_val == 0U)
        {
            enable = FALSE;
        }
        else
        {
            status = CMD_RET_USAGE;
        }
    }
    else
    {
        shell_write("Invalid argument count!\r\n");
        status = CMD_RET_FAILURE;
    }

    if (status == CMD_RET_SUCCESS)
    {
        if (enable)
        {
            /* get frequency value for calibration enablement - if it is provided */
            if (argc == 2U)
            {
                frequency = (uint32_t)BleApp_atoi(argv[1]);
            }

            /* Generate a Continuous Unmodulated Signal when calibration is enabled */
            xvr_status = XCVR_DftTxCW(frequency);
            if (xvr_status == gXcvrSuccess_c)
            {
                mCalibrationEnabled = TRUE;
                shell_write("Calibration enabled!\r\n");

            }
            else
            {
                if (xvr_status == gXcvrInvalidParameters_c)
                {
                    shell_write("Invalid frequency value!\r\n");
                }
                else
                {
                    shell_write("Calibration enablement failed!\r\n");
                }
                status = CMD_RET_FAILURE;
            }
        }
        else
        {
            /* Turn OFF the transmitter */
            XCVR_ForceTxWd();
            /* Initialize the radio for BLE */
            (void)XCVR_Init(&xcvrConfig, &rbmeConfig);
            /* Set XTAL trim value */
            (void)XCVR_SetXtalTrim(gXtalTrimValue);
            /* Set RSSI Adjustment Value */
            (void)XCVR_SetRssiAdjustment(gRssiValue);
            mCalibrationEnabled = FALSE;
            shell_write("Calibration disabled!\r\n");
        }
    }
    return status;
}

/*! *********************************************************************************
 * \brief        Handles the calibration getxtaltrim command.
 *
 * \param[in]    argc     argument count
 *
 * \param[in]    argv     argument value
 *
 * \return       CMD_RET_FAILURE - calibration not enabled or wrong argument count
                 CMD_RET_USAGE - inappropriate argument value
                 CMD_RET_SUCCESS
 ********************************************************************************** */
static int8_t ShellCalibration_GetXtal32MHzTrim(uint8_t argc, char * argv[])
{
    bool_t regRead;
    uint32_t regReadVal;
    uint8_t xTalTrim;
    int8_t status = CMD_RET_SUCCESS;

    if (argc != 1U)
    {
        status = CMD_RET_FAILURE;
    }
    else
    {
        regReadVal = (uint32_t)BleApp_atoi(argv[0]);
        if (regReadVal == 1U)
        {
            regRead = TRUE;
        }
        else if (regReadVal == 0U)
        {
            regRead = FALSE;
        }
        else
        {
            shell_write("Invalid argument!\r\n");
            status = CMD_RET_USAGE;
        }

        if (status == CMD_RET_SUCCESS)
        {
            xTalTrim = PLATFORM_GetXtal32MhzTrim(regRead);
            shell_write("Current Xtal trim value: ");
            shell_writeDec(xTalTrim);
            shell_write("\r\n");
        }
    }

    return status;
}

/*! *********************************************************************************
 * \brief        Handles the calibration setxtaltrim command.
 *
 * \param[in]    argc     argument count
 *
 * \param[in]    argv     argument value
 *
 * \return       CMD_RET_FAILURE - calibration not enabled or wrong argument count
                 CMD_RET_USAGE - inappropriate argument value
                 CMD_RET_SUCCESS
 ********************************************************************************** */
static int8_t ShellCalibration_SetXtal32MHzTrim(uint8_t argc, char * argv[])
{
    uint32_t trimValue;
    bool_t saveToHwParams;
    uint32_t saveToHwParamsVal;
    uint8_t idx = 0U;
    int8_t status = CMD_RET_SUCCESS;

    if (mCalibrationEnabled == FALSE)
    {
        shell_write("Calibration not enabled!\r\n");
        status = CMD_RET_FAILURE;
    }
    else if (argc != 2U)
    {
        shell_write("Invalid argument count!\r\n");
        status = CMD_RET_FAILURE;
    }
    else
    {
        /* Extract parameters and call function to set frequency offset */
        trimValue = (uint32_t)BleApp_atoi(argv[idx]);
        if (trimValue > 127U)
        {
            shell_write("Invalid argument!\r\n");
            status = CMD_RET_USAGE;
        }
        else
        {
            idx++;
            saveToHwParamsVal = (uint32_t)BleApp_atoi(argv[idx]);
            if (saveToHwParamsVal == 1U)
            {
                saveToHwParams = TRUE;
            }
            else if (saveToHwParamsVal == 0U)
            {
                saveToHwParams = FALSE;
            }
            else
            {
                shell_write("Invalid argument!\r\n");
                status = CMD_RET_USAGE;
            }
        }

        if (status == CMD_RET_SUCCESS)
        {
            gXtalTrimValue = (uint8_t)trimValue;
            PLATFORM_SetXtal32MhzTrim((uint8_t)trimValue, saveToHwParams);
            shell_writeDec(trimValue);
            shell_write(" - new Xtal trim value set!\r\n");
        }
    }

    return status;
}

/*! *********************************************************************************
 * \brief        Handles the calibration getrssiadj command.
 *
 * \param[in]    argc     argument count
 *
 * \param[in]    argv     argument value
 *
 * \return       CMD_RET_FAILURE - calibration not enabled or wrong argument count
                 CMD_RET_USAGE - inappropriate argument value
                 CMD_RET_SUCCESS
 ********************************************************************************** */
static int8_t ShellCalibration_GetRssiAdjustment(uint8_t argc, char * argv[])
{
    bool_t regRead;
    uint32_t regReadVal;
    int8_t rssi;
    int8_t status = CMD_RET_SUCCESS;

    if (argc != 1U)
    {
        status = CMD_RET_FAILURE;
    }
    else
    {
        /* Extract parameters and call the function to get the current RSSI adjustment value */
        regReadVal = (uint32_t)BleApp_atoi(argv[0]);
        if (regReadVal == 1U)
        {
            regRead = TRUE;
        }
        else if (regReadVal == 0U)
        {
            regRead = FALSE;
        }
        else
        {
            shell_write("Invalid argument!\r\n");
            status = CMD_RET_USAGE;
        }

        if (status == CMD_RET_SUCCESS)
        {
            rssi = PLATFORM_GetRssiAdjustment(regRead);
            shell_write("Current RSSI value: ");

            if(((uint8_t)rssi >> 7) != 0U)
            {
                /* Negative Value */
                SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "-");
                rssi = ~((uint8_t)rssi - 1U);
            }
            shell_writeDec(rssi);
            shell_write("\r\n");
        }
    }

    return status;
}

/*! *********************************************************************************
 * \brief        Handles the calibration setrssiadj command.
 *
 * \param[in]    argc     argument count
 *
 * \param[in]    argv     argument value
 *
 * \return       CMD_RET_FAILURE - calibration not enabled or wrong argument count
                 CMD_RET_USAGE - inappropriate argument value
                 CMD_RET_SUCCESS
 ********************************************************************************** */
static int8_t ShellCalibration_SetRssiAdjustment(uint8_t argc, char * argv[])
{
    int32_t newRssiAdjustmentValue;
    bool_t saveToHwParams;
    uint32_t saveToHwParamsVal;
    uint8_t idx = 0U;
    int8_t status = CMD_RET_SUCCESS;

    if (mCalibrationEnabled == FALSE)
    {
        shell_write("Calibration not enabled!\r\n");
        status = CMD_RET_FAILURE;
    }
    else if (argc != 2U)
    {
        shell_write("Invalid argument count!\r\n");
        status = CMD_RET_FAILURE;
    }
    else
    {
        /* Extract parameters and call function to set the RSSI adjustment value */
        newRssiAdjustmentValue = BleApp_atoi(argv[idx]);
        idx++;
        saveToHwParamsVal = (uint32_t)BleApp_atoi(argv[idx]);
        if (saveToHwParamsVal == 1U)
        {
            saveToHwParams = TRUE;
        }
        else if (saveToHwParamsVal == 0U)
        {
            saveToHwParams = FALSE;
        }
        else
        {
            shell_write("Invalid argument!\r\n");
            status = CMD_RET_USAGE;
        }

        if (status == CMD_RET_SUCCESS)
        {
            gRssiValue = (int8_t)newRssiAdjustmentValue;
            PLATFORM_SetRssiAdjustment((int8_t)newRssiAdjustmentValue, saveToHwParams);

            if(((uint8_t)newRssiAdjustmentValue >> 7) != 0U)
            {
                /* Negative Value */
                SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "-");
                newRssiAdjustmentValue = ~((uint8_t)newRssiAdjustmentValue - 1U);
            }
            shell_writeDec(newRssiAdjustmentValue);
            shell_write(" - new RSSI value set!\r\n");
        }
    }

    return status;
}
#endif
#endif /* (defined(CPU_MKW37A512VFT4) || defined(CPU_MKW38A512VFT4)) */
/*! *********************************************************************************
 * @}
 ********************************************************************************** */
