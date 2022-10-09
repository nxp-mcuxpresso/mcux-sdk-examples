
/*! *********************************************************************************
* Copyright 2020 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "fsl_os_abstraction.h"
#include "ble_general.h"
#include "controller_interface.h"
#include "Eeprom.h"
#include "FunctionLib.h"

/************************************************************************************
*************************************************************************************
* Private macro definitions
*************************************************************************************
************************************************************************************/

#if defined gLoggingActive_d && (gLoggingActive_d > 0)
#include "dbg_logging.h"
#ifndef DBG_OPERATION
#define DBG_OPERATION 0
#endif
#define OPERATION_DBG_LOG(fmt, ...)   if (DBG_OPERATION) do { DbgLogAdd(__FUNCTION__ , fmt, VA_NUM_ARGS(__VA_ARGS__), ##__VA_ARGS__); } while (0);
#else
#define OPERATION_DBG_LOG(...)
#endif

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

typedef enum
{
    opErase = 0,
    opWrite,
    opRead,
} eOperationType;

typedef enum
{
    opNotRegister = 0,
    opRegister,
    opRunning
} eOperationStatus;

typedef void (*processFonction)(void);

typedef struct
{
    eOperationType opType;
    eOperationStatus opStatus;
    uint32_t opFlashOffset;
    processFonction opProcess;
} sOperation;

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
static sOperation currentOperation = {opErase, opNotRegister, 0, NULL};

static uint8_t buffer[256];

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/
void OperationTest_Task(osaTaskParam_t param);
OSA_TASK_DEFINE( OperationTest_Task, 8, 1, 1000, FALSE );

static void processEepromReadOperation();
static void processEepromWriteOperation();
static void processEepromEraseOperation();

/************************************************************************************
*************************************************************************************
* Public functions prototypes
*************************************************************************************
************************************************************************************/

extern struct app_cfg app_configuration;

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

void OperationTest_Register()
{
    /* Create the task */
    if (OSA_TaskCreate(OSA_TASK(OperationTest_Task), NULL) == NULL)
    {
        panic( 0, (uint32_t)OperationTest_Register, 0, 0 );
    }

    EEPROM_Init();
}

void OperationTest_Task(osaTaskParam_t param)
{
    while(1)
    {
        if (currentOperation.opStatus == opRunning)
        {
            currentOperation.opProcess();
        }
        //DbgLogDump(FALSE);
        /* For BareMetal break the while(1) after 1 run */
        if( gUseRtos_c == 0 )
        {
            break;
        }
    }
}

int OperationTest_Prepare(uint8_t operationType)
{
    int status = -1;
    do
    {
        /* In case an operation is already in progress return error */
        if (currentOperation.opStatus != opNotRegister)
            break;
        if (operationType != opErase && operationType != opWrite && operationType != opRead)
            break;
        currentOperation.opType = (eOperationType) operationType;
        currentOperation.opStatus = opRegister;
        currentOperation.opFlashOffset = 0;
        /* Make sure that the flash is completely erased */
        if (operationType == opWrite)
        {
            FLib_MemSet(buffer, 0xAA, sizeof(buffer));
            currentOperation.opProcess = processEepromWriteOperation;
            EEPROM_ChipErase();
            while (EEPROM_isBusy());
        }
        else
        {
            /* Wait until the end of the eeprom operation */
            while (EEPROM_isBusy());
        }
        if (operationType == opRead)
        {
            currentOperation.opProcess = processEepromReadOperation;
        }
        else if (operationType == opErase)
        {
            currentOperation.opProcess = processEepromEraseOperation;
        }
        status = 0;
    }
    while (0);
    return status;
}

int OperationTest_Process()
{
    int status = -1;
    do
    {
        if (currentOperation.opStatus != opRegister)
            break;
        /* Make sure the eeprom is not busy */
        if (EEPROM_isBusy())
            break;
        OPERATION_DBG_LOG("Operation: %d is starting ...", currentOperation.opType);
        currentOperation.opStatus = opRunning;
        currentOperation.opProcess();
        status = 0;
    }
    while (0);
    return status;
}

int OperationTest_Stop()
{
    currentOperation.opStatus = opNotRegister;
    return 0;
}


/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/


static void processEepromReadOperation()
{
    if (!EEPROM_isBusy())
    {
        if (currentOperation.opFlashOffset >= gEepromParams_TotalSize_c)
        {
            currentOperation.opFlashOffset = 0;
            OPERATION_DBG_LOG("Operation: %d, full flash", currentOperation.opType);
            //DbgLogDump(TRUE);
        }
        EEPROM_ReadData(sizeof(buffer), currentOperation.opFlashOffset, buffer);
#if 0
        for(int i=0; i<sizeof(buffer); i++)
        {
            assert(buffer[i] == 0xaa);
        }
#endif
        FLib_MemSet(buffer, 0x0, sizeof(buffer));
        currentOperation.opFlashOffset+=sizeof(buffer);
    }
}

static void processEepromWriteOperation()
{
    if (!EEPROM_isBusy())
    {
        if (currentOperation.opFlashOffset >= gEepromParams_TotalSize_c)
        {
            currentOperation.opFlashOffset = 0;
            OPERATION_DBG_LOG("Operation: %d, full flash", currentOperation.opType);
            assert(0);
            //DbgLogDump(TRUE);
        }
        EEPROM_WriteData(sizeof(buffer), currentOperation.opFlashOffset, buffer);
        currentOperation.opFlashOffset+=sizeof(buffer);
    }
}

static void processEepromEraseOperation()
{
    if (!EEPROM_isBusy())
    {
        if (currentOperation.opFlashOffset >= gEepromParams_TotalSize_c)
        {
            currentOperation.opFlashOffset = 0;
            OPERATION_DBG_LOG("Operation: %d, full flash", currentOperation.opType);
            //DbgLogDump(TRUE);
        }
        EEPROM_EraseBlock(currentOperation.opFlashOffset, gEepromParams_SectorSize_c);
        currentOperation.opFlashOffset+=gEepromParams_SectorSize_c;
    }
}

/*! *********************************************************************************
* @}
********************************************************************************** */
