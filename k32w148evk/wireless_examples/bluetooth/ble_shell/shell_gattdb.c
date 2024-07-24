/*! *********************************************************************************
 * \addtogroup SHELL GATTDB
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright 2015 Freescale Semiconductor, Inc.
* Copyright 2016-2024 NXP
*
*
* \file
*
* This file is the source file for the GATTDB Shell module
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
 *************************************************************************************
 * Include
 *************************************************************************************
 ************************************************************************************/
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
#include "gatt_db_dynamic.h"
#include "gatt_database_dynamic.h"
#include "gap_interface.h"

#include "ble_shell.h"
#include "shell_gatt.h"
#include "shell_gattdb.h"

#include <stdlib.h>
#include <string.h>
/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define mShellGattDbCmdsCount_c             4U

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
typedef struct gattDbCmds_tag
{
    char*       name;
    shell_status_t      (*cmd)(uint8_t argc, char * argv[]);
}gattDbCmds_t;


/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/
/* Shell API Functions */
static shell_status_t   ShellGattDb_Read(uint8_t argc, char * argv[]);
static shell_status_t   ShellGattDb_Write(uint8_t argc, char * argv[]);
static shell_status_t   ShellGattDb_AddService(uint8_t argc, char * argv[]);
static shell_status_t   ShellGattDb_Erase(uint8_t argc, char * argv[]);

/* Local helper functions*/
static bleResult_t ShellGattDb_AddServiceInDatabase(serviceInfo_t* pServiceInfo);
/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
static const gattDbCmds_t mGattDbShellCmds[mShellGattDbCmdsCount_c] =
{
    {"read",        ShellGattDb_Read},
    {"write",       ShellGattDb_Write},
    {"addservice",  ShellGattDb_AddService},
    {"erase",       ShellGattDb_Erase}
};

static const char* mGattDbStatus[] = {
    "NoError",
    "InvalidHandle",
    "CharacteristicNotFound",
    "CccdNotFound",
    "ServiceNotFound",
    "DescriptorNotFound"
};

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
 * \brief        Initializes the GATT Dynamic Database with default services.
 *
 * \param[in]    none
 *
 * \return       bleResult_t
 ********************************************************************************** */
bleResult_t ShellGattDb_Init(void)
{
    bleResult_t result = gBleSuccess_c;

    result = GattDbDynamic_AddGattService(NULL);

    if( gBleSuccess_c == result )
    {
        result = GattDbDynamic_AddGapService(NULL);
    }

    return result;
}

/*! *********************************************************************************
 * \brief        Entry point for "gattdb" shell command.
 *
 * \param[in]    argc           Number of arguments
 * \param[in]    argv           Array of argument's values
 *
 * \return       int8_t         Command status
 ********************************************************************************** */
shell_status_t ShellGattDb_Command(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    uint8_t i;

    /* Command must have at least one argument, otherwise return usage */
    if (argc < 2)
    {
        (void)SHELL_PrintfSynchronization((shell_handle_t)shellHandle, "\r\nIncorrect command parameter(s).  Enter \"help\" to view a list of available commands.\r\n\r\n");
        return kStatus_SHELL_Error;
    }

    for (i=0; i< mShellGattDbCmdsCount_c; i++)
    {
        if(0 == strcmp((char*)argv[1], mGattDbShellCmds[i].name) )
        {
            /* Handle the command in the appropriate callback */
            return mGattDbShellCmds[i].cmd((uint8_t)argc-2U, (char **)(&argv[2]));
        }
    }

    (void)SHELL_PrintfSynchronization((shell_handle_t)shellHandle, "\r\nIncorrect command parameter(s).  Enter \"help\" to view a list of available commands.\r\n\r\n");

    return kStatus_SHELL_Error;
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
 * \brief        Handles "gattdb read" shell command.
 *
 * \param[in]    argc           Number of arguments
 * \param[in]    argv           Array of argument's values
 *
 * \return       int8_t         Command status
 ********************************************************************************** */
static shell_status_t   ShellGattDb_Read(uint8_t argc, char * argv[])
{
    uint8_t *pValue = NULL;
    uint8_t result;
    uint16_t length = 0U, handle = 0U;

    /* Must have as argument the handle of the attribute to be read */
    if (argc != 1U)
    {
        shell_write("\r\nIncorrect command parameter(s).  Enter \"help\" to view a list of available commands.\r\n\r\n");
        return kStatus_SHELL_Error;
    }

    /* Allocate memory for output */
    pValue = MEM_BufferAlloc(mMaxCharValueLength_d);

    if (NULL == pValue)
    {
        shell_write("\r\nMemory allocation error!\r\n\r\n");
        return kStatus_SHELL_Error;
    }

    /* Read the attribute */
    handle = (uint16_t)BleApp_atoi(argv[0]);
    result =  (uint8_t)GattDb_ReadAttribute(handle, mMaxCharValueLength_d, pValue, &length);
    result &= 0xFFU;

    if ((bleResult_t)result != gGattDbSuccess_c)
    {
        /* print error reason */
        shell_write("\r\n-->  GATTDB Event: Procedure Error ");
        if (result <= ((uint8_t)gGattDbDescriptorNotFound_c & 0xFFU))
        {
            /* GattDb errors */
            shell_write(mGattDbStatus[result]);
        }
        else
        {
            /* Other errors */
            shell_write("Unknown");
        }
        SHELL_NEWLINE();
    }
    else
    {
        shell_write("\r\n-->  GATTDB Event: Attribute Read ");
        shell_write("\r\n     Value: ");

        for(uint8_t i = (uint8_t)length; i > 0U; i-- )
        {
            shell_writeHex(pValue[i - 1U]);
        }
    }

    /* After printing, free memory */
    (void)MEM_BufferFree(pValue);
    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
 * \brief        Handles "gattdb write" shell command.
 *
 * \param[in]    argc           Number of arguments
 * \param[in]    argv           Array of argument's values
 *
 * \return       int8_t         Command status
 ********************************************************************************** */
static shell_status_t   ShellGattDb_Write(uint8_t argc, char * argv[])
{
    uint16_t length, handle;
    uint8_t result;

    /* Command must contain as arguments the handle and the value */
    if (argc != 2U)
    {
        shell_write("\r\nIncorrect command parameter(s).  Enter \"help\" to view a list of available commands.\r\n\r\n");
        return kStatus_SHELL_Error;
    }

    /* Read handle */
    handle = (uint16_t)BleApp_atoi(argv[0]);

    /* Read value */
    length = BleApp_ParseHexValue(argv[1]);

    if (length > mMaxCharValueLength_d)
    {
        shell_write("\r\n-->  Variable length exceeds maximum!");
        return kStatus_SHELL_Error;
    }

    /* Write value in database */
    result =  (uint8_t)GattDb_WriteAttribute(handle, length, (uint8_t*)argv[1]);
    result &= 0xFFU;

    if ((bleResult_t)result != gGattDbSuccess_c)
    {
        /* Print error reason */
        shell_write("\r\n-->  GATTDB Event: Procedure Error ");
        if (result <= ((uint8_t)gGattDbDescriptorNotFound_c & 0xFFU))
        {
            /* GattDb errors */
            shell_write(mGattDbStatus[result]);
        }
        else
        {
            /* Other errors */
            shell_write("Unknown");
        }
        SHELL_NEWLINE();
    }
    else
    {
        shell_write("\r\n-->  GATTDB Event: Attribute Written ");
    }

    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
 * \brief        Handles "gattdb addservice" shell command.
 *
 * \param[in]    argc           Number of arguments
 * \param[in]    argv           Array of argument's values
 *
 * \return       int8_t         Command status
 ********************************************************************************** */
static shell_status_t   ShellGattDb_AddService(uint8_t argc, char * argv[])
{
    serviceInfo_t* pServiceInfo = NULL;
    uint16_t       serviceUuid = 0, length;
    bleResult_t    result;

    /* Command must have the service UUID as argument */
    if (argc != 1U)
    {
        shell_write("\r\nIncorrect command parameter(s).  Enter \"help\" to view a list of available commands.\r\n\r\n");
        return kStatus_SHELL_Error;
    }

    /* Read service UUID */
    length = BleApp_ParseHexValue(argv[0]);

    /* Accept only UUID16 */
    if (length == 2U)
    {
        FLib_MemCpy(&serviceUuid, argv[0], sizeof(uint16_t));
    }

    /* Allocate memory and initiate known services with default details */
    switch (serviceUuid)
    {
        /* Default profiles */
        case gBleSig_GenericAccessProfile_d:
        case gBleSig_GenericAttributeProfile_d:
        {
            shell_write("\r\n-->  GATTDB Event: Service already exists! ");
            SHELL_NEWLINE();
        }
        break;

        /* Accept only the following services */
        case gBleSig_HeartRateService_d:
        {
            pServiceInfo = GattDbDynamic_GetHeartRateService();
        }
        break;

        case gBleSig_BatteryService_d:
        {
            pServiceInfo = GattDbDynamic_GetBatteryService();
        }
        break;

        case gBleSig_DeviceInformationService_d:
        {
            pServiceInfo = GattDbDynamic_GetDeviceInformationService();
        }
        break;

        case gBleSig_IpsService_d:
        {
            pServiceInfo = GattDbDynamic_GetIpssService();
        }
        break;

        default:
        {
            /* Do not accept other services */
            shell_write("\r\n-->  GATTDB Event: Service not supported ! ");
            SHELL_NEWLINE();
        }
        break;
    }

    /* Add service to database */
    if (pServiceInfo != gBleSuccess_c)
    {
        result = ShellGattDb_AddServiceInDatabase(pServiceInfo);
        if (gBleSuccess_c == result)
        {
            shell_write("\r\n-->  GATTDB Event: Service Added in database. ");
            SHELL_NEWLINE();
        }
    }
    else
    {
        shell_write("\r\n-->  GATTDB Event: Insufficient memory. ");
        SHELL_NEWLINE();
    }
    /* Free memory allocated for service */
    (void)MEM_BufferFree(pServiceInfo);
    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
 * \brief        Handles "gattdb erase" shell command.
 *
 * \param[in]    argc           Number of arguments
 * \param[in]    argv           Array of argument's values
 *
 * \return       int8_t         Command status
 ********************************************************************************** */
static shell_status_t   ShellGattDb_Erase(uint8_t argc, char * argv[])
{
    /* Must have no other arguments */
    if (argc != 0U)
    {
        shell_write("\r\nIncorrect command parameter(s).  Enter \"help\" to view a list of available commands.\r\n\r\n");
        return kStatus_SHELL_Error;
    }

    /* Delete all dynamic database */
    (void)GattDbDynamic_ReleaseDatabase();

    /* Free memory and reset pointers */
    (void)GattDbDynamic_Init();
    (void)ShellGattDb_Init();

    return kStatus_SHELL_Success;
}

/*! *********************************************************************************
 * \brief        Adds a service in GATT Database.
 *
 * \param[in]    pServiceInfo   Pointer to service structure to be added
 *
 * \return       bleResult_t    Status of the operation
 ********************************************************************************** */
static bleResult_t ShellGattDb_AddServiceInDatabase(serviceInfo_t* pServiceInfo)
{
    bleResult_t result = gBleSuccess_c;
    uint16_t    handle = 0U;

    /* Add service */
    result = GattDbDynamic_AddPrimaryServiceDeclaration(0, gBleUuidType16_c,
                                                        (void*)&pServiceInfo->uuid16,
                                                        &handle);
    if (result != gBleSuccess_c)
    {
        return result;
    }

    /* If operation succeeded print service name */
    shell_write("\r\n  --> ");
    shell_write(ShellGatt_GetServiceName(pServiceInfo->uuid16));

    /* Add characteristics */
    for (uint8_t i=0; i < pServiceInfo->nbOfCharacteristics; i++)
    {
        characteristicInfo_t* pCharacteristicInfo = &pServiceInfo->pCharacteristicInfo[i];

        result = GattDbDynamic_AddCharacteristicDeclarationAndValue(gBleUuidType16_c,
                                            (void*)&pCharacteristicInfo->uuid16,
                                            pCharacteristicInfo->properties,
                                            pCharacteristicInfo->maxValueLength,
                                            pCharacteristicInfo->valueLength,
                                            pCharacteristicInfo->pValue,
                                            pCharacteristicInfo->accessPermissions,
                                            &handle);
        if (result != gBleSuccess_c)
        {
            return result;
        }
        /* If operation succeeded print characteristic handle */
        shell_write("\r\n     - ");
        shell_write(ShellGatt_GetCharacteristicName(pCharacteristicInfo->uuid16));

        shell_write("  Value Handle: ");
        shell_writeDec((uint32_t)handle + 1U);

        /* Add descriptors */
        for (uint8_t j=0; j < pCharacteristicInfo->nbOfDescriptors; j++)
        {
            descriptorInfo_t* pDescriptorInfo = &pCharacteristicInfo->pDescriptorInfo[j];

            result = GattDbDynamic_AddCharDescriptor(gBleUuidType16_c,
                                                    (void*)&pDescriptorInfo->uuid16,
                                                    pDescriptorInfo->valueLength,
                                                    pDescriptorInfo->pValue,
                                                    pDescriptorInfo->accessPermissions,
                                                    &handle);
            if (result != gBleSuccess_c)
            {
                return result;
            }
            /* If operation succeeded print descriptor handle */
            shell_write("\r\n       - ");
            shell_write(ShellGatt_GetDescriptorName(pDescriptorInfo->uuid16));
            shell_write("  Descriptor Handle: ");
            shell_writeDec(handle);
        }

        /* Add CCCD */
        if (pCharacteristicInfo->bAddCccd)
        {
             result = GattDbDynamic_AddCccd(&handle);

            if (result != gBleSuccess_c)
            {
                return result;
            }
            /* If operation succeeded print CCCD handle */
            shell_write("\r\n       - CCCD Handle: ");
            shell_writeDec(handle);
        }
    }
    return result;
}

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
