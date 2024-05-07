/*! *********************************************************************************
 * \addtogroup SHELL GATT
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright 2015 Freescale Semiconductor, Inc.
* Copyright 2016-2023 NXP
*
*
* \file
*
* This file is the source file for the GATT Shell module
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
 *************************************************************************************
 * Include
 *************************************************************************************
 ************************************************************************************/
#include "EmbeddedTypes.h"

/* Framework / Drivers */
#include "fsl_component_timer_manager.h"
#include "fsl_shell.h"
#include "fsl_component_panic.h"
#include "fsl_component_mem_manager.h"
#include "FunctionLib.h"
#include "fsl_os_abstraction.h"
#include "board.h"

/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gatt_db_app_interface.h"
#include "gap_interface.h"

#include "shell_gatt.h"
#include "shell_thrput.h"
#include "ble_shell.h"

#include <stdlib.h>
#include <string.h>
/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define mShellGattCmdsCount_c               6U

#define mMaxServicesCount_d                 6U
#define mMaxServiceCharCount_d              8U
#define mMaxCharDescriptorsCount_d          2U
#define mMaxCharValueLength_d               23U

#define mThroughputReportThreshold_c        100U

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
typedef struct gattCmds_tag
{
    char*       name;
    shell_status_t      (*cmd)(uint8_t argc, char * argv[]);
}gattCmds_t;

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/
/* Shell API Functions */
static shell_status_t ShellGatt_Discover(uint8_t argc, char * argv[]);
static shell_status_t ShellGatt_Read(uint8_t argc, char * argv[]);
static shell_status_t ShellGatt_Write(uint8_t argc, char * argv[], bool_t fNoRsp);
static shell_status_t ShellGatt_WriteCmd(uint8_t argc, char * argv[]);
static shell_status_t ShellGatt_WriteRsp(uint8_t argc, char * argv[]);
static shell_status_t ShellGatt_Notify(uint8_t argc, char * argv[]);
static shell_status_t ShellGatt_Indicate(uint8_t argc, char * argv[]);

static void ShellGatt_DiscoveryFinished(void);
static void ShellGatt_DiscoveryHandler
(
    deviceId_t serverDeviceId,
    gattProcedureType_t procedureType
);
static bool_t ShellGatt_CheckProcedureFinished(void);
static void ShellGatt_PrintIndexedService(uint8_t index);
static void ShellGatt_PrintIndexedCharacteristic(gattService_t *pService, uint8_t index);

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
static const gattCmds_t mGattShellCmds[mShellGattCmdsCount_c] =
{
    {"discover",    ShellGatt_Discover},
    {"read",        ShellGatt_Read},
    {"write",       ShellGatt_WriteRsp},
    {"writecmd",    ShellGatt_WriteCmd},
    {"notify",      ShellGatt_Notify},
    {"indicate",    ShellGatt_Indicate}
};

const gattUuidNames_t mGattServices[]={
    {  0x1800  , "Generic Access"},
    {  0x1801  , "Generic Attribute"},
    {  0x1802  , "Immediate Alert"},
    {  0x1803  , "Link Loss"},
    {  0x1804  , "Tx Power"},
    {  0x1805  , "Current Time Service"},
    {  0x1806  , "Reference Time Update Service"},
    {  0x1807  , "Next DST Change Service"},
    {  0x1808  , "Glucose"},
    {  0x1809  , "Health Thermometer"},
    {  0x180A  , "Device Information"},
    {  0x180D  , "Heart Rate"},
    {  0x180E  , "Phone Alert Status Service"},
    {  0x180F  , "Battery Service"},
    {  0x1810  , "Blood Pressure"},
    {  0x1811  , "Alert Notification Service"},
    {  0x1812  , "Human Interface Device"},
    {  0x1813  , "Scan Parameters"},
    {  0x1814  , "Running Speed and Cadence"},
    {  0x1815  , "Automation IO"},
    {  0x1816  , "Cycling Speed and Cadence"},
    {  0x1818  , "Cycling Power"},
    {  0x1819  , "Location and Navigation"},
    {  0x181A  , "Environmental Sensing"},
    {  0x181B  , "Body Composition"},
    {  0x181C  , "User Data"},
    {  0x181D  , "Weight Scale"},
    {  0x181E  , "Bond Management"},
    {  0x181F  , "Continuous Glucose Monitoring"},
    {  0x1820  , "Internet Protocol Support"},
    {  0x1821  , "Indoor Positioning"},
    {  0x1822  , "Pulse Oximeter"},
    {  0x1823  , "HTTP Proxy"},
    {  0x1824  , "Transport Discovery"},
    {  0x1825  , "Object Transfer"}};

const gattUuidNames_t mGattChars[]={
    {  0x2A00  , "Device Name"},
    {  0x2A01  , "Appearance"},
    {  0x2A02  , "Peripheral Privacy Flag"},
    {  0x2A03  , "Reconnection Address"},
    {  0x2A04  , "Peripheral Preferred Connection Parameters"},
    {  0x2A05  , "Service Changed"},
    {  0x2A06  , "Alert Level"},
    {  0x2A07  , "Tx Power Level"},
    {  0x2A08  , "Date Time"},
    {  0x2A09  , "Day of Week"},
    {  0x2A0A  , "Day Date Time"},
    {  0x2A0C  , "Exact Time 256"},
    {  0x2A0D  , "DST Offset"},
    {  0x2A0E  , "Time Zone"},
    {  0x2A0F  , "Local Time Information"},
    {  0x2A11  , "Time with DST"},
    {  0x2A12  , "Time Accuracy"},
    {  0x2A13  , "Time Source"},
    {  0x2A14  , "Reference Time Information"},
    {  0x2A16  , "Time Update Control Point"},
    {  0x2A17  , "Time Update State"},
    {  0x2A18  , "Glucose Measurement"},
    {  0x2A19  , "Battery Level"},
    {  0x2A1C  , "Temperature Measurement"},
    {  0x2A1D  , "Temperature Type"},
    {  0x2A1E  , "Intermediate Temperature"},
    {  0x2A21  , "Measurement Interval"},
    {  0x2A22  , "Boot Keyboard Input Report"},
    {  0x2A23  , "System ID"},
    {  0x2A24  , "Model Number String"},
    {  0x2A25  , "Serial Number String"},
    {  0x2A26  , "Firmware Revision String"},
    {  0x2A27  , "Hardware Revision String"},
    {  0x2A28  , "Software Revision String"},
    {  0x2A29  , "Manufacturer Name String"},
    {  0x2A2A  , "IEEE 11073-20601 Regulatory Certification Data List"},
    {  0x2A2B  , "Current Time"},
    {  0x2A2C  , "Magnetic Declination"},
    {  0x2A31  , "Scan Refresh"},
    {  0x2A32  , "Boot Keyboard Output Report"},
    {  0x2A33  , "Boot Mouse Input Report"},
    {  0x2A34  , "Glucose Measurement Context"},
    {  0x2A35  , "Blood Pressure Measurement"},
    {  0x2A36  , "Intermediate Cuff Pressure"},
    {  0x2A37  , "Heart Rate Measurement"},
    {  0x2A38  , "Body Sensor Location"},
    {  0x2A39  , "Heart Rate Control Point"},
    {  0x2A3F  , "Alert Status"},
    {  0x2A40  , "Ringer Control Point"},
    {  0x2A41  , "Ringer Setting"},
    {  0x2A42  , "Alert Category ID Bit Mask"},
    {  0x2A43  , "Alert Category ID"},
    {  0x2A44  , "Alert Notification Control Point"},
    {  0x2A45  , "Unread Alert Status"},
    {  0x2A46  , "New Alert"},
    {  0x2A47  , "Supported New Alert Category"},
    {  0x2A48  , "Supported Unread Alert Category"},
    {  0x2A49  , "Blood Pressure Feature"},
    {  0x2A4A  , "HID Information"},
    {  0x2A4B  , "Report Map"},
    {  0x2A4C  , "HID Control Point"},
    {  0x2A4D  , "Report"},
    {  0x2A4E  , "Protocol Mode"},
    {  0x2A4F  , "Scan Interval Window"},
    {  0x2A50  , "PnP ID"},
    {  0x2A51  , "Glucose Feature"},
    {  0x2A52  , "Record Access Control Point"},
    {  0x2A53  , "RSC Measurement"},
    {  0x2A54  , "RSC Feature"},
    {  0x2A55  , "SC Control Point"},
    {  0x2A56  , "Digital"},
    {  0x2A58  , "Analog"},
    {  0x2A5A  , "Aggregate"},
    {  0x2A5B  , "CSC Measurement"},
    {  0x2A5C  , "CSC Feature"},
    {  0x2A5D  , "Sensor Location"},
    {  0x2A5E  , "PLX Spot-Check Measurement"},
    {  0x2A5F  , "PLX Continuous Measurement"},
    {  0x2A60  , "PLX Features"},
    {  0x2A63  , "Cycling Power Measurement"},
    {  0x2A64  , "Cycling Power Vector"},
    {  0x2A65  , "Cycling Power Feature"},
    {  0x2A66  , "Cycling Power Control Point"},
    {  0x2A67  , "Location and Speed"},
    {  0x2A68  , "Navigation"},
    {  0x2A69  , "Position Quality"},
    {  0x2A6A  , "LN Feature"},
    {  0x2A6B  , "LN Control Point"},
    {  0x2A6C  , "Elevation"},
    {  0x2A6D  , "Pressure"},
    {  0x2A6E  , "Temperature"},
    {  0x2A6F  , "Humidity"},
    {  0x2A70  , "True Wind Speed"},
    {  0x2A71  , "True Wind Direction"},
    {  0x2A72  , "Apparent Wind Speed"},
    {  0x2A73  , "Apparent Wind Direction"},
    {  0x2A74  , "Gust Factor"},
    {  0x2A75  , "Pollen Concentration"},
    {  0x2A76  , "UV Index"},
    {  0x2A77  , "Irradiance"},
    {  0x2A78  , "Rainfall"},
    {  0x2A79  , "Wind Chill"},
    {  0x2A7A  , "Heat Index"},
    {  0x2A7B  , "Dew Point"},
    {  0x2A7D  , "Descriptor Value Changed"},
    {  0x2A7E  , "Aerobic Heart Rate Lower Limit"},
    {  0x2A7F  , "Aerobic Threshold"},
    {  0x2A80  , "Age"},
    {  0x2A81  , "Anaerobic Heart Rate Lower Limit"},
    {  0x2A82  , "Anaerobic Heart Rate Upper Limit"},
    {  0x2A83  , "Anaerobic Threshold"},
    {  0x2A84  , "Aerobic Heart Rate Upper Limit"},
    {  0x2A85  , "Date of Birth"},
    {  0x2A86  , "Date of Threshold Assessment"},
    {  0x2A87  , "Email Address"},
    {  0x2A88  , "Fat Burn Heart Rate Lower Limit"},
    {  0x2A89  , "Fat Burn Heart Rate Upper Limit"},
    {  0x2A8A  , "First Name"},
    {  0x2A8B  , "Five Zone Heart Rate Limits"},
    {  0x2A8C  , "Gender"},
    {  0x2A8D  , "Heart Rate Max"},
    {  0x2A8E  , "Height"},
    {  0x2A8F  , "Hip Circumference"},
    {  0x2A90  , "Last Name"},
    {  0x2A91  , "Maximum Recommended Heart Rate"},
    {  0x2A92  , "Resting Heart Rate"},
    {  0x2A93  , "Sport Type for Aerobic and Anaerobic Thresholds"},
    {  0x2A94  , "Three Zone Heart Rate Limits"},
    {  0x2A95  , "Two Zone Heart Rate Limit"},
    {  0x2A96  , "VO2 Max"},
    {  0x2A97  , "Waist Circumference"},
    {  0x2A98  , "Weight"},
    {  0x2A99  , "Database Change Increment"},
    {  0x2A9A  , "User Index"},
    {  0x2A9B  , "Body Composition Feature"},
    {  0x2A9C  , "Body Composition Measurement"},
    {  0x2A9D  , "Weight Measurement"},
    {  0x2A9E  , "Weight Scale Feature"},
    {  0x2A9F  , "User Control Point"},
    {  0x2AA0  , "Magnetic Flux Density - 2D"},
    {  0x2AA1  , "Magnetic Flux Density - 3D"},
    {  0x2AA2  , "Language"},
    {  0x2AA3  , "Barometric Pressure Trend"},
    {  0x2AA4  , "Bond Management Control Point"},
    {  0x2AA5  , "Bond Management Feature"},
    {  0x2AA6  , "Central Address Resolution"},
    {  0x2AA7  , "CGM Measurement"},
    {  0x2AA8  , "CGM Feature"},
    {  0x2AA9  , "CGM Status"},
    {  0x2AAA  , "CGM Session Start Time"},
    {  0x2AAB  , "CGM Session Run Time"},
    {  0x2AAC  , "CGM Specific Ops Control Point"},
    {  0x2AAD  , "Indoor Positioning Configuration"},
    {  0x2AAE  , "Latitude"},
    {  0x2AAF  , "Longitude"},
    {  0x2AB0  , "Local North Coordinate"},
    {  0x2AB1  , "Local East Coordinate"},
    {  0x2AB2  , "Floor Number"},
    {  0x2AB3  , "Altitude"},
    {  0x2AB4  , "Uncertainty"},
    {  0x2AB5  , "Location Name"},
    {  0x2AB6  , "URI"},
    {  0x2AB7  , "HTTP Headers"},
    {  0x2AB8  , "HTTP Status Code"},
    {  0x2AB9  , "HTTP Entity Body"},
    {  0x2ABA  , "HTTP Control Point"},
    {  0x2ABB  , "HTTPS Security"},
    {  0x2ABC  , "TDS Control Point"},
    {  0x2ABD  , "OTS Feature"},
    {  0x2ABE  , "Object Name"},
    {  0x2ABF  , "Object Type"},
    {  0x2AC0  , "Object Size"},
    {  0x2AC1  , "Object First-Created"},
    {  0x2AC2  , "Object Last-Modified"},
    {  0x2AC3  , "Object ID"},
    {  0x2AC4  , "Object Properties"},
    {  0x2AC5  , "Object Action Control Point"},
    {  0x2AC6  , "Object List Control Point"},
    {  0x2AC7  , "Object List Filter"},
    {  0x2AC8  , "Object Changed"}};

const gattUuidNames_t mGattDescriptors[]={
    {0x2900, "Characteristic Extended Properties"},
    {0x2901, "Characteristic User Description"},
    {0x2902, "Client Characteristic Configuration"},
    {0x2903, "Server Characteristic Configuration"},
    {0x2904, "Characteristic Presentation Format"},
    {0x2905, "Characteristic Aggregate Format"},
    {0x2906, "Valid Range"},
    {0x2907, "External Report Reference"},
    {0x2908, "Report Reference"},
    {0x2909, "Number of Digitals"},
    {0x290A, "Value Trigger Setting"},
    {0x290B, "Environmental Sensing Configuration"},
    {0x290C, "Environmental Sensing Measurement"},
    {0x290D, "Environmental Sensing Trigger Setting"},
    {0x290E, "Time Trigger Setting"}};

static const char* mGattStatus[] = {
    "NoError",
    "InvalidHandle",
    "ReadNotPermitted",
    "WriteNotPermitted",
    "InvalidPdu",
    "InsufficientAuthentication",
    "RequestNotSupported",
    "InvalidOffset",
    "InsufficientAuthorization",
    "PrepareQueueFull",
    "AttributeNotFound",
    "AttributeNotLong",
    "InsufficientEncryptionKeySize",
    "InvalidAttributeValueLength",
    "UnlikelyError",
    "InsufficientEncryption",
    "UnsupportedGroupType",
    "InsufficientResources",
};

/* Buffer used for Service Discovery */
static gattService_t *mpServiceDiscoveryBuffer = NULL;
static uint8_t  mcPrimaryServices = 0;

/* Buffer used for Characteristic Discovery */
static gattCharacteristic_t *mpCharBuffer = NULL;
static uint8_t mCurrentServiceInDiscoveryIndex;
static uint8_t mCurrentCharInDiscoveryIndex;
static uint8_t mcCharacteristics = 0;

/* Buffer used for Characteristic Descriptor Discovery */
static gattAttribute_t *mpCharDescriptorBuffer = NULL;

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
 * \brief        Entry point for "gatt" shell command.
 *
 * \param[in]    argc           Number of arguments
 * \param[in]    argv           Array of argument's values
 *
 * \return       int8_t         Command status
 ********************************************************************************** */
shell_status_t ShellGatt_Command
(
    shell_handle_t shellHandle,
    int32_t argc,
    char **argv
)
{
    uint8_t i;
    /* Command must have at least one argument, otherwise return usage */
    if (argc < 2)
    {
        (void)SHELL_PrintfSynchronization((shell_handle_t)shellHandle, "\r\nIncorrect command parameter(s).  Enter \"help\" to view a list of available commands.\r\n\r\n");
        return kStatus_SHELL_Error;
    }

    for (i = 0U; i < mShellGattCmdsCount_c; i++)
    {
        /* Handle the command in the appropriate callback */
        if(0 == strcmp((char*)argv[1], mGattShellCmds[i].name) )
        {
            return mGattShellCmds[i].cmd((uint8_t)argc-2U, (char **)(&argv[2]));
        }
    }

    (void)SHELL_PrintfSynchronization((shell_handle_t)shellHandle, "\r\nIncorrect command parameter(s).  Enter \"help\" to view a list of available commands.\r\n\r\n");

    return kStatus_SHELL_Error;
}

/*! *********************************************************************************
 * \brief        Handles GATT client callback from host stack.
 *
 * \param[in]    serverDeviceId      Server peer device ID.
 * \param[in]    procedureType       GATT procedure type.
 * \param[in]    procedureType       GATT procedure result.
 * \param[in]    error               Result.
 ********************************************************************************** */
void ShellGatt_ClientCallback
(
    deviceId_t serverDeviceId,
    gattProcedureType_t procedureType,
    gattProcedureResult_t procedureResult,
    bleResult_t error
)
{
    /* Handle GATT success procedure events */
    if (procedureResult == gGattProcSuccess_c)
    {
        switch(procedureType)
        {
            case gGattProcExchangeMtu_c:
            {
                /* Redirect to Shell Throughput if enabled*/
                if (gThroughputConfig[serverDeviceId].bTestInProgress)
                {
                    ShellThr_MtuExchangeCallback(serverDeviceId);
                }
                shell_write("\r\n-->  MTU Exchanged.");
                shell_cmd_finished();
            }
            break;

            case gGattProcDiscoverAllCharacteristics_c:
            case gGattProcDiscoverAllCharacteristicDescriptors_c:
            case gGattProcDiscoverAllPrimaryServices_c:
            {
                /* Parse discovered services/characteristics */
                ShellGatt_DiscoveryHandler(serverDeviceId, procedureType);
            }
            break;

            case gGattProcDiscoverPrimaryServicesByUuid_c:
            {
                uint8_t i = 0;
                shell_write("\r\n--> Discovered primary services: ");
                shell_writeDec(mcPrimaryServices);

                /* Print details about each primary service */
                while (i < mcPrimaryServices)
                {
                    ShellGatt_PrintIndexedService(i);
                    i++;
                }
                /* Free allocated memory and reset pointers */
                ShellGatt_DiscoveryFinished();
            }
            break;

            case gGattProcDiscoverCharacteristicByUuid_c:
            {
                uint8_t i = 0;
                shell_write("\r\n--> Discovered characteristics: ");
                shell_writeDec(mcCharacteristics);
                /* Print details about each characteristic */
                while (i < mcCharacteristics)
                {
                    ShellGatt_PrintIndexedService(i);
                    i++;
                }
                /* Free allocated memory and reset pointers */
                ShellGatt_DiscoveryFinished();
            }
            break;

            case gGattProcReadCharacteristicValue_c:
            {
                /* Print characteristic value as hex stream */
                shell_write("\r\n-->  GATT Event: Characteristic Value Read ");
                shell_write("\r\n     Value: ");
                for(uint8_t i = (uint8_t)mpCharBuffer->value.valueLength; i > 0U; i-- )
                {
                    shell_writeHex(mpCharBuffer->value.paValue[i - 1U]);
                }

                shell_cmd_finished();

                /* Free memory buffers */
                (void)MEM_BufferFree(mpCharBuffer->value.paValue);
                (void)MEM_BufferFree(mpCharBuffer);
                mpCharBuffer = NULL;
            }
            break;

            case gGattProcWriteCharacteristicValue_c:
            {
                /* Success for "gatt write" command */
                shell_write("\r\n-->  GATT Event: Characteristic Value Written!");
                shell_cmd_finished();
            }
            break;

            default:
                /* Other GATT event */
                shell_cmd_finished();
                break;
            }
    }
    else
    {
        /* GATT Procedure Error */
        uint8_t attError = (uint8_t)error;
        attError &= 0xFFU;
        shell_write("\r\n-->  GATT Event: Procedure Error ");
        shell_write(mGattStatus[attError]);
        SHELL_NEWLINE();

        /* Free memory buffers and reset pointers */
        ShellGatt_DiscoveryFinished();
    }

}



/*! *********************************************************************************
 * \brief        Handles GATT server callback from host stack.
 *
 * \param[in]    deviceId        Client peer device ID.
 * \param[in]    pServerEvent    Pointer to gattServerEvent_t.
 ********************************************************************************** */
void ShellGatt_ServerCallback
(
    deviceId_t deviceId,
    gattServerEvent_t* pServerEvent
)
{

}

/*! *********************************************************************************
 * \brief        Handles GATT notification message.
 *
 * \param[in]    serverDeviceId               Server peer device ID.
 * \param[in]    characteristicValueHandle    Handle of the notified characteristic
 * \param[in]    aValue                       Pointer to characteristic's value
 * \param[in]    valueLength                  Length of characteristic's value
 ********************************************************************************** */
void ShellGatt_NotificationCallback
(
    deviceId_t          serverDeviceId,
    uint16_t            characteristicValueHandle,
    uint8_t*            aValue,
    uint16_t            valueLength
)
{
    /* Check if SHELL Throughput is running */
    if(gThroughputConfig[serverDeviceId].bTestInProgress)
    {
        /* Handle notification in Shell Throughput module */
        ShellThr_ProcessNotification(valueLength, serverDeviceId);
    }
    else
    {
        /* This event will occur only on the GATT Client device */
        shell_write("\r\n-->  GATT Event: Received Notification ");
        /* Print handle and value of the notified characteristic */
        shell_write("\r\n     Handle: ");
        shell_writeDec(characteristicValueHandle);
        shell_write("\r\n     Value: ");
        for(uint8_t i = (uint8_t)valueLength; i > 0U; i-- )
        {
            shell_writeHex(aValue[i - 1U]);
        }
        SHELL_NEWLINE();
    }
}

/*! *********************************************************************************
 * \brief        Handles GATT indication message.
 *
 * \param[in]    serverDeviceId               Server peer device ID.
 * \param[in]    characteristicValueHandle    Handle of the indicated characteristic
 * \param[in]    aValue                       Pointer to characteristic's value
 * \param[in]    valueLength                  Length of characteristic's value
 ********************************************************************************** */
void ShellGatt_IndicationCallback
(
    deviceId_t          serverDeviceId,
    uint16_t            characteristicValueHandle,
    uint8_t*            aValue,
    uint16_t            valueLength
)
{
    /* This event will occur only on the GATT Client device */
    shell_write("\r\n-->  GATT Event: Received Indication ");
    /* print the handle of the indicated characteristic and its value */
    shell_write("\r\n     Handle: ");
    shell_writeDec(characteristicValueHandle);
    shell_write("\r\n     Value: ");
    for(uint8_t i = (uint8_t)valueLength; i > 0U; i-- )
    {
        shell_writeHex(aValue[i - 1U]);
    }
    SHELL_NEWLINE();
}

/*! *********************************************************************************
 * \brief        Returns the name of a service identified by its UUID.
 *
 * \param[in]    uuid16                  UUID of the service
 * \return       char*                   Pointer to service's name
 ********************************************************************************** */
const char* ShellGatt_GetServiceName(uint16_t uuid16)
{
    for(uint32_t i=0; i < NumberOfElements(mGattServices); i++)
    {
        if (mGattServices[i].uuid == uuid16)
        {
            return mGattServices[i].name;
        }
    }

    return (char*)"N/A";
}

/*! *********************************************************************************
 * \brief        Returns the name of a given characteristic identified by its UUID.
 *
 * \param[in]    uuid16                  UUID of the characteristic
 * \return       char*                   Pointer to characteristic's name
 ********************************************************************************** */
const char* ShellGatt_GetCharacteristicName(uint16_t uuid16)
{
    for(uint8_t i=0; i < NumberOfElements(mGattChars); i++)
    {
        if (mGattChars[i].uuid == uuid16)
        {
            return mGattChars[i].name;
        }
    }

    return "N/A";
}

/*! *********************************************************************************
 * \brief        Returns the name of a given descriptor identified by its UUID.
 *
 * \param[in]    uuid16                  UUID of the descriptor
 * \return       char*                   Pointer to descriptor's name
 ********************************************************************************** */
const char* ShellGatt_GetDescriptorName(uint16_t uuid16)
{
    for(uint8_t i=0; i < NumberOfElements(mGattDescriptors); i++)
    {
        if (mGattDescriptors[i].uuid == uuid16)
        {
            return mGattDescriptors[i].name;
        }
    }

    return "N/A";
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
 * \brief        Handles "gatt discover" shell command.
 *
 * \param[in]    argc           Number of arguments
 * \param[in]    argv           Array of argument's values
 *
 * \return       int8_t         Command status
 ********************************************************************************** */
static shell_status_t ShellGatt_Discover(uint8_t argc, char * argv[])
{
    shell_status_t result = kStatus_SHELL_Error;
    deviceId_t peerId;

    peerId = (deviceId_t)BleApp_atoi(argv[0]);

    if (IS_CONNECTED(peerId) == 0U)
    {
        shell_write("\r\n-->  Please connect the node first...");
        result = kStatus_SHELL_Error;
    }
    else
    {
        /* Check if another procedure has finished */
        if (ShellGatt_CheckProcedureFinished())
        {
            switch(argc)
            {
                case 2U:
                {
                    if(0 == strcmp((char*)argv[1], "-all"))
                    {
                        /* Allocate memory for Service Discovery */
                        mpServiceDiscoveryBuffer = MEM_BufferAlloc(sizeof(gattService_t) * mMaxServicesCount_d);
                        mpCharBuffer = MEM_BufferAlloc(sizeof(gattCharacteristic_t) * mMaxServiceCharCount_d);
                        mpCharDescriptorBuffer = MEM_BufferAlloc(sizeof(gattAttribute_t) * mMaxServiceCharCount_d * mMaxCharDescriptorsCount_d);

                        if ((NULL == mpServiceDiscoveryBuffer) ||
                            (NULL == mpCharBuffer)             ||
                            (NULL == mpCharDescriptorBuffer))
                        {
                            shell_write("\r\nMemory allocation error!\r\n\r\n");
                            result = kStatus_SHELL_Error;
                        }
                        else
                        {
                            /* Start Service Discovery*/
                            (void)GattClient_DiscoverAllPrimaryServices(
                                                        peerId,
                                                        mpServiceDiscoveryBuffer,
                                                        mMaxServicesCount_d,
                                                        &mcPrimaryServices);

                            result = kStatus_SHELL_Success;
                        }
                    }
                }
                break;

                case 3U:
                {
                    if(0 == strcmp((char*)argv[1], "-service"))
                    {
                        bleUuid_t uuid16 = {.uuid16 = (uint16_t)BleApp_AsciiToHex(argv[2], FLib_StrLen(argv[2]))};

                        /* Allocate memory for Service Discovery */
                        mpServiceDiscoveryBuffer = MEM_BufferAlloc(sizeof(gattService_t) * mMaxServicesCount_d);

                        if (NULL == mpServiceDiscoveryBuffer)
                        {
                            /* Memory allocation issue */
                            shell_write("\r\nMemory allocation error!\r\n\r\n");
                            result = kStatus_SHELL_Error;
                        }
                        else
                        {

                            (void)GattClient_DiscoverPrimaryServicesByUuid(
                                                        peerId,
                                                        gBleUuidType16_c,
                                                        &uuid16,
                                                        mpServiceDiscoveryBuffer,
                                                        mMaxServicesCount_d,
                                                        &mcPrimaryServices);
                            result = kStatus_SHELL_Success;
                        }
                    }
                }
                break;

                default:
                {
                    shell_write("\r\nIncorrect command parameter(s).  Enter \"help\" to view a list of available commands.\r\n\r\n");
                    result = kStatus_SHELL_Error;
                }
                break;
            }
        }
        else
        {
            /* Other GATT procedure in progress */
            shell_write("\r\n-->  There is a GATT procedure in progress, the command cannot be executed!");
            result = kStatus_SHELL_Error;
        }
    }
    return result;
}

/*! *********************************************************************************
 * \brief        Handles "gatt read" shell command.
 *
 * \param[in]    argc           Number of arguments
 * \param[in]    argv           Array of argument's values
 *
 * \return       int8_t         Command status
 ********************************************************************************** */
static shell_status_t ShellGatt_Read(uint8_t argc, char * argv[])
{
    shell_status_t result = kStatus_SHELL_Success;
    uint8_t     *pValue = NULL;
    deviceId_t peerId;

    peerId = (deviceId_t)BleApp_atoi(argv[0]);

    /* Command is valid only when connected */
    if (IS_CONNECTED(peerId) == 0U)
    {
        shell_write("\r\n-->  Please connect the node first...");
        result = kStatus_SHELL_Error;
    }
    else if (!ShellGatt_CheckProcedureFinished())
    {
        /* Check if another procedure is ongoing */
        shell_write("\r\n-->  There is another procedure in progress, the command cannot be executed!");
        result = kStatus_SHELL_Error;
    }
    else if (argc != 2U)
    {
        /* Command should contain as argument only the handle to be read */
        shell_write("\r\nIncorrect command parameter(s).  Enter \"help\" to view a list of available commands.\r\n\r\n");
        result = kStatus_SHELL_Error;
    }
    else
    {
        /* Get the handle of the characteristic to be read and allocate memory for response */
        mpCharBuffer = MEM_BufferAlloc(sizeof(gattCharacteristic_t));
        pValue = MEM_BufferAlloc(mMaxCharValueLength_d);

        if ((NULL == mpCharBuffer) || (NULL == pValue))
        {
            /* Memory allocation issue */
            shell_write("\r\nMemory allocation error!\r\n\r\n");
            result = kStatus_SHELL_Error;
        }
        else
        {
            mpCharBuffer->value.handle = (uint16_t)BleApp_atoi(argv[1]);
            mpCharBuffer->value.paValue = pValue;

            /* 50 - Maximum number of bytes to be read */
            (void)GattClient_ReadCharacteristicValue(peerId, mpCharBuffer, 50);
        }
    }
    return result;
}

/*! *********************************************************************************
 * \brief        Handles "gatt writecmd" and "gatt writersp" shell commands.
 *
 * \param[in]    argc           Number of arguments
 * \param[in]    argv           Array of argument's values
 * \param[in]    fNoRsp         TRUE - GATT Write Command, FALSE - GATT Write Response
 *
 * \return       int8_t         Command status
 ********************************************************************************** */
static shell_status_t ShellGatt_Write(uint8_t argc, char * argv[], bool_t fNoRsp)
{
    uint8_t    length = 0;
    shell_status_t result = kStatus_SHELL_Success;
    deviceId_t peerId;

    peerId = (deviceId_t)BleApp_atoi(argv[0]);

    /* Command is valid only when connected */
    if (IS_CONNECTED(peerId) == 0U)
    {
        shell_write("\r\n-->  Please connect the node first...");
        result = kStatus_SHELL_Error;
    }
    else if (!ShellGatt_CheckProcedureFinished())
    {
        /* Check if another procedure is ongoing */
        shell_write("\r\n-->  There is another procedure in progress, the command cannot be executed!");
        result = kStatus_SHELL_Error;
    }
    else if (argc != 3U)
    {
        /* Command must contain as arguments the handle and the value */
        shell_write("\r\nIncorrect command parameter(s).  Enter \"help\" to view a list of available commands.\r\n\r\n");
        result = kStatus_SHELL_Error;
    }
    else
    {
        /* Allocate memory for  */
        mpCharBuffer = MEM_BufferAlloc(sizeof(gattCharacteristic_t));

        if (NULL == mpCharBuffer)
        {
            shell_write("\r\nMemory allocation error!\r\n\r\n");
            result = kStatus_SHELL_Error;
        }
        else
        {
            /* Clear data to be sure that ERPC does not use uninitialized values */
            FLib_MemSet(mpCharBuffer, 0, sizeof(gattCharacteristic_t));

            /* Read handle and value to be written */
            mpCharBuffer->value.handle = (uint16_t)BleApp_atoi(argv[1]);

            length = BleApp_ParseHexValue(argv[2]);

            if (length > mMaxCharValueLength_d)
            {
                shell_write("\r\n-->  Variable length exceeds maximum!");
                result = kStatus_SHELL_Error;
            }
            else
            {
                /* Write characteristic, free memory and reset pointer for characteristic buffer */
                (void)GattClient_WriteCharacteristicValue(
                                            peerId,
                                            mpCharBuffer,
                                            length,
                                            (uint8_t *)argv[2],
                                            fNoRsp,
                                            FALSE,
                                            FALSE,
                                            NULL);
                (void)MEM_BufferFree(mpCharBuffer);
                mpCharBuffer = NULL;
            }
        }
    }
    return result;
}

/*! *********************************************************************************
 * \brief        Handles "gatt writecmd" shell command.
 *
 * \param[in]    argc           Number of arguments
 * \param[in]    argv           Array of argument's values
 *
 * \return       int8_t         Command status
 ********************************************************************************** */
static shell_status_t ShellGatt_WriteCmd(uint8_t argc, char * argv[])
{
    return ShellGatt_Write(argc, argv, TRUE);
}

/*! *********************************************************************************
 * \brief        Handles "gatt writersp" shell command.
 *
 * \param[in]    argc           Number of arguments
 * \param[in]    argv           Array of argument's values
 *
 * \return       int8_t         Command status
 ********************************************************************************** */
static shell_status_t ShellGatt_WriteRsp(uint8_t argc, char * argv[])
{
    return ShellGatt_Write(argc, argv, FALSE);
}

/*! *********************************************************************************
 * \brief        Handles "gatt notify" shell command.
 *
 * \param[in]    argc           Number of arguments
 * \param[in]    argv           Array of argument's values
 *
 * \return       int8_t         Command status
 ********************************************************************************** */
static shell_status_t ShellGatt_Notify(uint8_t argc, char * argv[])
{
    shell_status_t result = kStatus_SHELL_Success;
    uint16_t  handle, hCccd;
    bool_t isNotificationActive;
    deviceId_t peerId;

    peerId = (deviceId_t)BleApp_atoi(argv[0]);

    /* Command must contain the handle of the notified characteristic */
    if (argc != 2U)
    {
        shell_write("\r\nIncorrect command parameter(s).  Enter \"help\" to view a list of available commands.\r\n\r\n");
        result = kStatus_SHELL_Error;
    }
    /* Command valid only if device is connected */
    else if (IS_CONNECTED(peerId) == 0U)
    {
        shell_write("\r\n-->  Please connect the node first...\r\n");
        result = kStatus_SHELL_Error;
    }
    else if (!ShellGatt_CheckProcedureFinished())
    {
        /* Check if another procedure has finished */
        shell_write("\r\n-->  There is another procedure in progress, the command cannot be executed!");
        result = kStatus_SHELL_Error;
    }
    else
    {
        /* Read the handle */
        handle = (uint16_t)BleApp_atoi(argv[1]);

        /* Get handle of CCCD */
        if (GattDb_FindCccdHandleForCharValueHandle(handle, &hCccd) != gBleSuccess_c)
        {
            shell_write("\r\n-->  No CCCD found!\r\n");
            result = kStatus_SHELL_Error;
        }
        /* Check if notifications are enabled for requested characteristic */
        else if (gBleSuccess_c == Gap_CheckNotificationStatus
            (peerId, hCccd, &isNotificationActive) &&
            TRUE == isNotificationActive)
        {
            (void)GattServer_SendNotification(peerId, handle);
        }
        else
        {
            shell_write("\r\n-->  CCCD is not set!\r\n");
            result = kStatus_SHELL_Error;
        }
    }
    return result;
}

/*! *********************************************************************************
 * \brief        Handles "gatt indicate" shell command.
 *
 * \param[in]    argc           Number of arguments
 * \param[in]    argv           Array of argument's values
 *
 * \return       int8_t         Command status
 ********************************************************************************** */
static shell_status_t ShellGatt_Indicate(uint8_t argc, char * argv[])
{
    shell_status_t result = kStatus_SHELL_Success;
    uint16_t  handle, hCccd;
    bool_t isIndicationActive;
    deviceId_t peerId;

    peerId = (deviceId_t)BleApp_atoi(argv[0]);

    /* Command must contain the handle of the indicated characteristic */
    if (argc != 2U)
    {
        shell_write("\r\nIncorrect command parameter(s).  Enter \"help\" to view a list of available commands.\r\n\r\n");
        result = kStatus_SHELL_Error;
    }
    /* Command is valid only if device is connected */
    else if (IS_CONNECTED(peerId) == 0U)
    {
        shell_write("\r\n-->  Please connect the node first...\r\n");
        result = kStatus_SHELL_Error;
    }
    /* Check if another procedure has finished */
    else if (!ShellGatt_CheckProcedureFinished())
    {
        shell_write("\r\n-->  There is another procedure in progress, the command cannot be executed!");
        result = kStatus_SHELL_Error;
    }
    else
    {
        /* Get the handle of the characteristic to be indicated */
        handle = (uint16_t)BleApp_atoi(argv[1]);

        /* Get handle of CCCD */
        if (GattDb_FindCccdHandleForCharValueHandle(handle, &hCccd) != gBleSuccess_c)
        {
            shell_write("\r\n-->  No CCCD found!\r\n");
            result = kStatus_SHELL_Error;
        }
        /* Check if indications are enabled for CCCD */
        else if (gBleSuccess_c == Gap_CheckIndicationStatus
            (peerId, hCccd, &isIndicationActive) &&
            TRUE == isIndicationActive)
        {
            (void)GattServer_SendIndication(peerId, handle);
        }
        else
        {
            shell_write("\r\n-->  CCCD is not set!\r\n");
            result = kStatus_SHELL_Error;
        }
    }
    return result;
}

/*! *********************************************************************************
 * \brief        Prints the start and end handles for a given service, its characteristics
 *               and descriptors.
 *
 * \param[in]    index           Index of the service
 ********************************************************************************** */
static void ShellGatt_PrintIndexedService(uint8_t index)
{
    /* Get pointer to the requested service */
    gattService_t        *pCurrentService = mpServiceDiscoveryBuffer + index;

    /* Print service name */
    SHELL_NEWLINE();
    shell_write("\r\n  --> ");
    shell_write(ShellGatt_GetServiceName(pCurrentService->uuid.uuid16));

    /* print start and end handles */
    shell_write("  Start Handle: ");
    shell_writeDec(pCurrentService->startHandle);
    shell_write("  End Handle: ");
    shell_writeDec(pCurrentService->endHandle);

    /* Print all characteristics */
    for(uint8_t i=0; i < pCurrentService->cNumCharacteristics; i++)
    {
        ShellGatt_PrintIndexedCharacteristic(pCurrentService, i);
    }
}

/*! *********************************************************************************
 * \brief        Prints characteristic's name and its descriptors of a given service
 *
 * \param[in]    pService           The service containing the given characteristic
 * \param[in]    index              Index of the characteristic within the service
 ********************************************************************************** */
static void ShellGatt_PrintIndexedCharacteristic(gattService_t *pService, uint8_t index)
{
    /* Get pointer to requested characteristic */
    gattCharacteristic_t *pCurrentChar =  pService->aCharacteristics + index;

    /* Print characteristic's name */
    shell_write("\r\n     - ");
    shell_write(ShellGatt_GetCharacteristicName(pCurrentChar->value.uuid.uuid16));

    /* Print characteristic's handle */
    shell_write("  Value Handle: ");
    shell_writeDec(pCurrentChar->value.handle);

    /* Print all descriptors */
    for(uint32_t i=0; i < pCurrentChar->cNumDescriptors; i++)
    {
        shell_write("\r\n       - ");
        shell_write(ShellGatt_GetDescriptorName(pCurrentChar->aDescriptors[i].uuid.uuid16));
        shell_write("  Descriptor Handle: ");
        shell_writeDec(pCurrentChar->aDescriptors[i].handle);
    }
}

/*! *********************************************************************************
 * \brief        Handles the discovery procedure callback when notified by host stack
 *
 * \param[in]    serverDeviceId      Server peer device ID.
 * \param[in]    procedureType       GATT Discovery procedure type
 ********************************************************************************** */
static void ShellGatt_DiscoveryHandler
(
    deviceId_t serverDeviceId,
    gattProcedureType_t procedureType
)
{
    switch (procedureType)
    {
        case gGattProcDiscoverAllPrimaryServices_c:
        {
            shell_write("\r\n--> Discovered primary services: ");
            shell_writeDec(mcPrimaryServices);

            /* We found at least one service. Move on to characteristic discovery */
            if (0U != mcPrimaryServices)
            {
                /* Start characteristic discovery with first service*/
                mCurrentServiceInDiscoveryIndex = 0;
                mCurrentCharInDiscoveryIndex = 0;

                mpServiceDiscoveryBuffer->aCharacteristics = mpCharBuffer;

                /* Start Characteristic Discovery for current service */
                (void)GattClient_DiscoverAllCharacteristicsOfService(
                                            serverDeviceId,
                                            mpServiceDiscoveryBuffer,
                                            mMaxServiceCharCount_d);
            }
            else
            {
                shell_write("\r\n--> Found primary services: 0");
                ShellGatt_DiscoveryFinished();
            }
        }
        break;

        case gGattProcDiscoverAllCharacteristicDescriptors_c: /* Fall-through */
        case gGattProcDiscoverAllCharacteristics_c:
        {
            if (gGattProcDiscoverAllCharacteristicDescriptors_c == procedureType)
            {
                /* Move on to the next characteristic */
                mCurrentCharInDiscoveryIndex++;
            }

            gattService_t        *pCurrentService = mpServiceDiscoveryBuffer + mCurrentServiceInDiscoveryIndex;
            gattCharacteristic_t *pCurrentChar = pCurrentService->aCharacteristics + mCurrentCharInDiscoveryIndex;
            bool_t                bDiscoveryStarted = FALSE;

            /* Check if we finished with the current service */
            if (mCurrentCharInDiscoveryIndex < pCurrentService->cNumCharacteristics)
            {
                /* Find next characteristic with descriptors*/
                while (mCurrentCharInDiscoveryIndex < pCurrentService->cNumCharacteristics - 1U)
                {
                    /* Check if we have handles available between adjacent characteristics */
                    if (pCurrentChar->value.handle + 2U < (pCurrentChar + 1U)->value.handle)
                    {
                        pCurrentChar->aDescriptors = mpCharDescriptorBuffer + mCurrentCharInDiscoveryIndex * mMaxCharDescriptorsCount_d;
                        (void)GattClient_DiscoverAllCharacteristicDescriptors(serverDeviceId,
                                                pCurrentChar,
                                                (pCurrentChar + 1U)->value.handle - 2U,
                                                mMaxCharDescriptorsCount_d);
                        bDiscoveryStarted = TRUE;
                        break;
                    }
                    else
                    {
                        mCurrentCharInDiscoveryIndex++;
                        pCurrentChar = pCurrentService->aCharacteristics + mCurrentCharInDiscoveryIndex;
                    }
                }

                if (bDiscoveryStarted == FALSE)
                {
                    /* Made it to the last characteristic. Check against service end handle*/
                    if (pCurrentChar->value.handle < pCurrentService->endHandle)
                    {
                        pCurrentChar->aDescriptors = mpCharDescriptorBuffer + mCurrentCharInDiscoveryIndex * mMaxCharDescriptorsCount_d;
                        (void)GattClient_DiscoverAllCharacteristicDescriptors(serverDeviceId,
                                                    pCurrentChar,
                                                    pCurrentService->endHandle,
                                                    mMaxCharDescriptorsCount_d);
                         bDiscoveryStarted = TRUE;
                    }
                }
            }

            if (bDiscoveryStarted == FALSE)
            {
                ShellGatt_PrintIndexedService(mCurrentServiceInDiscoveryIndex);

                /* Move on to the next service */
                mCurrentServiceInDiscoveryIndex++;

                /* Reset characteristic discovery */
                mCurrentCharInDiscoveryIndex = 0;
                FLib_MemSet(mpCharBuffer, 0, sizeof(gattCharacteristic_t) * mMaxServiceCharCount_d);

                if (mCurrentServiceInDiscoveryIndex < mcPrimaryServices)
                {
                    /* Allocate memory for Char Discovery */
                    (mpServiceDiscoveryBuffer + mCurrentServiceInDiscoveryIndex)->aCharacteristics = mpCharBuffer;

                     /* Start Characteristic Discovery for current service */
                    (void)GattClient_DiscoverAllCharacteristicsOfService(serverDeviceId,
                                                mpServiceDiscoveryBuffer + mCurrentServiceInDiscoveryIndex,
                                                mMaxServiceCharCount_d);
                }
                else
                {
                    ShellGatt_DiscoveryFinished();
                }
            }
        }
        break;

    default:
        ; /* MISRA Compliance */
      break;
    }
}

/*! *********************************************************************************
 * \brief        Checks if there is an ongoing GATT procedure
 *
 * \return       bool_t     TRUE  - no ongoing GATT procedure
 *                          FALSE - ongoing GATT procedure
 ********************************************************************************** */
static bool_t ShellGatt_CheckProcedureFinished(void)
{
  return ((mpServiceDiscoveryBuffer == NULL) &&
          (mpCharBuffer == NULL) &&
          (mpCharDescriptorBuffer == NULL));
}

/*! *********************************************************************************
 * \brief        Frees the allocated memory buffers when a discovery procedure ends
 *
 ********************************************************************************** */
static void ShellGatt_DiscoveryFinished(void)
{
    /* Free buffer allocated for service discovery */
    if (mpServiceDiscoveryBuffer != NULL)
    {
        (void)MEM_BufferFree(mpServiceDiscoveryBuffer);
        mpServiceDiscoveryBuffer = NULL;
    }

    /* Free buffer allocated for characteristic discovery */
    if (mpCharBuffer != NULL)
    {
        (void)MEM_BufferFree(mpCharBuffer);
        mpCharBuffer = NULL;
    }

    /* Free buffer allocated for characteristic's descriptor discovery */
    if (mpCharDescriptorBuffer != NULL)
    {
        (void)MEM_BufferFree(mpCharDescriptorBuffer);
        mpCharDescriptorBuffer = NULL;
    }

    shell_cmd_finished();
}

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
