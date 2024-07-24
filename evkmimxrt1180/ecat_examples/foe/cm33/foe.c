/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*-----------------------------------------------------------------------------------------
------
------    Includes
------
-----------------------------------------------------------------------------------------*/
#include "ecat_def.h"
#include "applInterface.h"
#include "ecatfoe.h"

#define _FOE_ 1
#include "foe.h"
#undef _FOE_

#include "fsl_rgpio.h"
#include "mflash_drv.h"
#include "foe_support.h"
#include "mcuboot_app_support.h"
#include "fsl_debug_console.h"

/*--------------------------------------------------------------------------------------
------
------    local types and defines
------
--------------------------------------------------------------------------------------*/
#define    FOE_PWD                      0x12369874

#define    MAX_FIRMWARE_NAME_SIZE       32

/** \brief  MAX_FIREWARE_SIZE: Maximum file size */
#define MAX_FIREWARE_SIZE               0x40000

#define MCUBOOT_IMAGE_NUMBER            1

#define BOOT_FIRST_FLASH                0x28040000

#define FIRMWARE_INFO_FLASH             0x28440000

#define FIRMWARE_INFO_OFFSET            0x440000

/*-----------------------------------------------------------------------------------------
------
------    local variables and constants
------
-----------------------------------------------------------------------------------------*/

UINT32			    nFirmwareWriteOffset = 0;
CHAR                aFirmwareName[MAX_FIRMWARE_NAME_SIZE];
volatile BOOL	    nStartDownload = FALSE;
volatile BOOL	    nCompleteDownload = FALSE;
UINT32			    firmwareDownloadAddr = BOOT_FIRST_FLASH;
UINT32              firmwareDownloadSize = 0;

/*-----------------------------------------------------------------------------------------
------
------    application specific functions
------
-----------------------------------------------------------------------------------------*/

void FoE_StartDownload(void)
{
	/**
	* Check if the download is already started, if yes return error.                                                                      
    */
	if (nStartDownload) {
		return;		
	}
	/** 
	* Remember a firmware download has started, when BOOT state is requested
	* to flash new binary to the other BANK B.
	*/
	nStartDownload = TRUE;
	nFirmwareWriteOffset = 0;
}

void FoE_Boot2Init(void)
{
    /** 
	* Download is finished which was already initiated
	* to update firmware and clean up flash 
    */
	if (nStartDownload) {
		nCompleteDownload = TRUE;		
	}
}

BOOL FoE_IsDownloaded(void)
{
	return nCompleteDownload;
}
/*-----------------------------------------------------------------------------------------
------
------    generic functions
------
-----------------------------------------------------------------------------------------*/
UINT16 FoE_Read(UINT16 MBXMEM *pName, UINT16 nameSize, UINT32 password, UINT16 maxBlockSize, UINT16 *pData)
{
    PRINTF("File upload start\r\n");
    uint16_t size = 0;
    uint8_t firmwareInformation[MAX_FIRMWARE_NAME_SIZE + 4];
    char aReadFirmwareName[MAX_FIRMWARE_NAME_SIZE];

    if (nameSize > MAX_FIRMWARE_NAME_SIZE - 1) {
        PRINTF("File name size too long\r\n");
        return ECAT_FOE_ERRCODE_DISKFULL;
    }

    /*Read requested file name to endianess conversion if required*/
    MBXSTRCPY(aReadFirmwareName, pName, nameSize);
    aReadFirmwareName[nameSize] = '\0';

    /*copy the firmware information .. n*/
    MEMCPY(firmwareInformation, (void const *)FIRMWARE_INFO_FLASH, MAX_FIRMWARE_NAME_SIZE + 4);

    if (firmwareInformation == NULL) {
        /* No file stored*/
        PRINTF("No file stored\r\n");
        return ECAT_FOE_ERRCODE_NOTFOUND;
    }

    /* compare file name */
    for (uint8_t i = 0; i < nameSize; i++) {
        if (aReadFirmwareName[i] != firmwareInformation[i]) {
            /* file name not found */
            PRINTF("File name not found\r\n");
            PRINTF("Fireware name: ");
            for (uint8_t j = 0; j < nameSize; j++) {
                PRINTF("%c", firmwareInformation[j]);
            }
            PRINTF("\r\n");
            return ECAT_FOE_ERRCODE_NOTFOUND;
        }
    }

    firmwareDownloadSize = (firmwareInformation[MAX_FIRMWARE_NAME_SIZE]) + 
                            (firmwareInformation[MAX_FIRMWARE_NAME_SIZE + 1] << 8) + 
                            (firmwareInformation[MAX_FIRMWARE_NAME_SIZE + 2] << 16) + 
                            (firmwareInformation[MAX_FIRMWARE_NAME_SIZE + 3] << 24);

    size = firmwareDownloadSize < maxBlockSize ? (UINT16)firmwareDownloadSize : maxBlockSize;
    /*copy the first foe data block*/
    MEMCPY(pData, (void const *)firmwareDownloadAddr, size);

    return size;
}

UINT16 FoE_ReadData(UINT32 offset, UINT16 maxBlockSize, UINT16 *pData)
{
    uint16_t size = 0;

    if (firmwareDownloadSize < offset) {
        return 0;
    }

    /*get file length to send*/
    size = (UINT16)(firmwareDownloadSize - offset);
    if (size > maxBlockSize) {
        /*transmit max block size if the file data to be send is greater than the max data block*/
        size = maxBlockSize;
    }
    /*copy the foe data block 2 .. n*/
    MEMCPY(pData, (void const *)(firmwareDownloadAddr + offset), size);

    return size;
}

UINT16 FoE_WriteData(UINT16 MBXMEM *pData, UINT16 Size, BOOL bDataFollowing)
{
    if ((nFirmwareWriteOffset + Size) > MAX_FIREWARE_SIZE) {
        return ECAT_FOE_ERRCODE_DISKFULL;
    }

    /* FoE-Data services will follow */
    if (FoE_StoreImage((uint8_t *)pData, Size, nFirmwareWriteOffset, !bDataFollowing)) {
        PRINTF("FOE_MAXBUSY\r\n");
        return FOE_MAXBUSY;
    }
    nFirmwareWriteOffset += Size; 

    if (!bDataFollowing) {
        FoE_UpdateImage();
        /* Update new firmware information */
        if (FoE_WriteFirmwareInformation(FIRMWARE_INFO_OFFSET, aFirmwareName, MAX_FIRMWARE_NAME_SIZE, nFirmwareWriteOffset)){
            PRINTF("FoE update new firmware information error\r\n");
            return ECAT_FOE_ERRCODE_DISKFULL;
        }
        nFirmwareWriteOffset = 0;
    }
    
    return 0;
}

UINT16 FoE_Write(UINT16 MBXMEM *pName, UINT16 nameSize, UINT32 password)
{
	BOOL writeCheck1 = FALSE;
	BOOL writeCheck2 = FALSE;

    if (nameSize < MAX_FIRMWARE_NAME_SIZE) {
        MBXSTRCPY(aFirmwareName, pName, nameSize);
        /* string termination */
        MBXSTRCPY(aFirmwareName + nameSize, "\0", 1);
        
        nFirmwareWriteOffset = 0;
        firmwareDownloadSize = 0;
		writeCheck1 = TRUE;
    } else {
        return ECAT_FOE_ERRCODE_DISKFULL;
    }
	
	/* Password check is provided */
	if (password == FOE_PWD) {
		writeCheck2 = TRUE;		
	} else {
        PRINTF("Invalid password\r\n");
		return ECAT_FOE_ERRCODE_DISKFULL;		
	}

	if (writeCheck1 && writeCheck2) {
		FoE_StartDownload();
        FoE_UpdatePartition();
        FoE_PartitionInit();
	}
	
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \brief    The function is called when an error state was acknowledged by the master

*////////////////////////////////////////////////////////////////////////////////////////

void    APPL_AckErrorInd(UINT16 stateTrans)
{

}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return    AL Status Code (see ecatslv.h ALSTATUSCODE_....)

 \brief    The function is called in the state transition from INIT to PREOP when
             all general settings were checked to start the mailbox handler. This function
             informs the application about the state transition, the application can refuse
             the state transition when returning an AL Status error code.
            The return code NOERROR_INWORK can be used, if the application cannot confirm
            the state transition immediately, in that case this function will be called cyclically
            until a value unequal NOERROR_INWORK is returned

*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StartMailboxHandler(void)
{
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return     0, NOERROR_INWORK

 \brief    The function is called in the state transition from PREEOP to INIT
             to stop the mailbox handler. This functions informs the application
             about the state transition, the application cannot refuse
             the state transition.

*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StopMailboxHandler(void)
{
    FoE_Boot2Init();
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \param    pIntMask    pointer to the AL Event Mask which will be written to the AL event Mask
                        register (0x204) when this function is succeeded. The event mask can be adapted
                        in this function
 \return    AL Status Code (see ecatslv.h ALSTATUSCODE_....)

 \brief    The function is called in the state transition from PREOP to SAFEOP when
           all general settings were checked to start the input handler. This function
           informs the application about the state transition, the application can refuse
           the state transition when returning an AL Status error code.
           The return code NOERROR_INWORK can be used, if the application cannot confirm
           the state transition immediately, in that case the application need to be complete 
           the transition by calling ECAT_StateChange.
*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StartInputHandler(UINT16 *pIntMask)
{
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return     0, NOERROR_INWORK

 \brief    The function is called in the state transition from SAFEOP to PREEOP
             to stop the input handler. This functions informs the application
             about the state transition, the application cannot refuse
             the state transition.

*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StopInputHandler(void)
{
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return    AL Status Code (see ecatslv.h ALSTATUSCODE_....)

 \brief    The function is called in the state transition from SAFEOP to OP when
             all general settings were checked to start the output handler. This function
             informs the application about the state transition, the application can refuse
             the state transition when returning an AL Status error code.
           The return code NOERROR_INWORK can be used, if the application cannot confirm
           the state transition immediately, in that case the application need to be complete 
           the transition by calling ECAT_StateChange.
*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StartOutputHandler(void)
{
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return     0, NOERROR_INWORK

 \brief    The function is called in the state transition from OP to SAFEOP
             to stop the output handler. This functions informs the application
             about the state transition, the application cannot refuse
             the state transition.

*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StopOutputHandler(void)
{
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
\return     0(ALSTATUSCODE_NOERROR), NOERROR_INWORK
\param      pInputSize  pointer to save the input process data length
\param      pOutputSize  pointer to save the output process data length

\brief    This function calculates the process data sizes from the actual SM-PDO-Assign
            and PDO mapping
*////////////////////////////////////////////////////////////////////////////////////////
UINT16 APPL_GenerateMapping(UINT16 *pInputSize,UINT16 *pOutputSize)
{
    UINT16 result = ALSTATUSCODE_NOERROR;
    UINT16 InputSize = 0;
    UINT16 OutputSize = 0;

#if COE_SUPPORTED
    UINT16 PDOAssignEntryCnt = 0;
    OBJCONST TOBJECT OBJMEM * pPDO = NULL;
    UINT16 PDOSubindex0 = 0;
    UINT32 *pPDOEntry = NULL;
    UINT16 PDOEntryCnt = 0;
   
#if MAX_PD_OUTPUT_SIZE > 0
    /*Scan object 0x1C12 RXPDO assign*/
    for(PDOAssignEntryCnt = 0; PDOAssignEntryCnt < sRxPDOassign.u16SubIndex0; PDOAssignEntryCnt++)
    {
        pPDO = OBJ_GetObjectHandle(sRxPDOassign.aEntries[PDOAssignEntryCnt]);
        if (pPDO != NULL)
        {
            PDOSubindex0 = *((UINT16 *)pPDO->pVarPtr);
            for(PDOEntryCnt = 0; PDOEntryCnt < PDOSubindex0; PDOEntryCnt++)
            {
                pPDOEntry = (UINT32 *)((UINT16 *)pPDO->pVarPtr + (OBJ_GetEntryOffset((PDOEntryCnt+1),pPDO)>>3)/2);    //goto PDO entry
                // we increment the expected output size depending on the mapped Entry
                OutputSize += (UINT16) ((*pPDOEntry) & 0xFF);
            }
        }
        else
        {
            /*assigned PDO was not found in object dictionary. return invalid mapping*/
            OutputSize = 0;
            result = ALSTATUSCODE_INVALIDOUTPUTMAPPING;
            break;
        }
    }

    OutputSize = (OutputSize + 7) >> 3;
#endif

#if MAX_PD_INPUT_SIZE > 0
    if (result == 0)
    {
        /*Scan Object 0x1C13 TXPDO assign*/
        for(PDOAssignEntryCnt = 0; PDOAssignEntryCnt < sTxPDOassign.u16SubIndex0; PDOAssignEntryCnt++)
        {
            pPDO = OBJ_GetObjectHandle(sTxPDOassign.aEntries[PDOAssignEntryCnt]);
            if (pPDO != NULL)
            {
                PDOSubindex0 = *((UINT16 *)pPDO->pVarPtr);
                for(PDOEntryCnt = 0; PDOEntryCnt < PDOSubindex0; PDOEntryCnt++)
                {
                    pPDOEntry = (UINT32 *)((UINT16 *)pPDO->pVarPtr + (OBJ_GetEntryOffset((PDOEntryCnt+1),pPDO)>>3)/2);    //goto PDO entry
                    // we increment the expected output size depending on the mapped Entry
                    InputSize += (UINT16) ((*pPDOEntry) & 0xFF);
                }
            }
            else
            {
                /*assigned PDO was not found in object dictionary. return invalid mapping*/
                InputSize = 0;
                result = ALSTATUSCODE_INVALIDINPUTMAPPING;
                break;
            }
        }
    }
    InputSize = (InputSize + 7) >> 3;
#endif

#else
#if _WIN32
   #pragma message ("Warning: Define 'InputSize' and 'OutputSize'.")
#else
    #warning "Define 'InputSize' and 'OutputSize'."
#endif
#endif

    *pInputSize = InputSize;
    *pOutputSize = OutputSize;
    return result;
}

unsigned char LED_status;
/////////////////////////////////////////////////////////////////////////////////////////
/**
\param      pData  pointer to input process data

\brief      This function will copies the inputs from the local memory to the ESC memory
            to the hardware
*////////////////////////////////////////////////////////////////////////////////////////
void APPL_InputMapping(UINT16* pData)
{
    MEMCPY(pData,&LED_status,SIZEOF(LED_status));
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
\param      pData  pointer to output process data

\brief    This function will copies the outputs from the ESC memory to the local memory
            to the hardware
*////////////////////////////////////////////////////////////////////////////////////////
void APPL_OutputMapping(UINT16* pData)
{
    MEMCPY(&LED_status,pData,SIZEOF(LED_status));
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
\brief    This function will called from the synchronisation ISR 
            or from the mainloop if no synchronisation is supported
*////////////////////////////////////////////////////////////////////////////////////////
void APPL_Application(void)
{
    RGPIO_PinWrite(RGPIO4, 27, LED_status & 0x01);
}

#if EXPLICIT_DEVICE_ID
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return    The Explicit Device ID of the EtherCAT slave

 \brief     Calculate the Explicit Device ID
*////////////////////////////////////////////////////////////////////////////////////////
UINT16 APPL_GetDeviceID()
{
#if _WIN32
   #pragma message ("Warning: Implement explicit Device ID latching")
#else
    #warning "Implement explicit Device ID latching"
#endif
    /* Explicit Device 5 is expected by Explicit Device ID conformance tests*/
    return 0x5;
}
#endif


/////////////////////////////////////////////////////////////////////////////////////////
/**
\param     index               index of the requested object.
\param     subindex            subindex of the requested object.
\param     objSize             size of the requested object data, calculated with OBJ_GetObjectLength
\param     pData               Pointer to the buffer where the data can be copied to
\param     bCompleteAccess     Indicates if a complete read of all subindices of the
                               object shall be done or not

 \return    result of the read operation (0 (success) or an abort code (ABORTIDX_.... defined in
            sdosrv.h))
 *////////////////////////////////////////////////////////////////////////////////////////
UINT8 ReadObject0x6000(UINT16 index, UINT8 subindex, UINT32 dataSize, UINT16 MBXMEM * pData, UINT8 bCompleteAccess) {
    if (bCompleteAccess) {
        return ABORTIDX_UNSUPPORTED_ACCESS;
    }

    if (subindex == 0) {
        *pData = 1;
    } else if (subindex == 1) {
        if (dataSize > 0) {
            MEMCPY(pData, &LED_status, dataSize);
        }
    }
    return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////
/**
\param     index               index of the requested object.
\param     subindex            subindex of the requested object.
\param     objSize             size of the requested object data, calculated with OBJ_GetObjectLength
\param     pData               Pointer to the buffer where the data can be copied to
\param     bCompleteAccess     Indicates if a complete read of all subindices of the
                               object shall be done or not

 \return    result of the read operation (0 (success) or an abort code (ABORTIDX_.... defined in
            sdosrv.h))
 *////////////////////////////////////////////////////////////////////////////////////////
UINT8 WriteObject0x7000(UINT16 index, UINT8 subindex, UINT32 dataSize, UINT16 MBXMEM * pData, UINT8 bCompleteAccess) {
    if ( bCompleteAccess ) {
        /* Complete Access is not supported for object 0x1010 */
        return ABORTIDX_UNSUPPORTED_ACCESS;
    }

    if ( subindex == 0 ) {
        /* Subindex 0 is not writable */
        return ABORTIDX_READ_ONLY_ENTRY;
    } else if (subindex == 1) {
        /* Save the backup entries */
        MEMCPY(&LED_status, pData, SIZEOF(LED_status));
    } else {
        return ABORTIDX_VALUE_EXCEEDED;
    }
    RGPIO_PinWrite(RGPIO4, 27, LED_status & 0x01);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**

 \brief    This is the main function

*////////////////////////////////////////////////////////////////////////////////////////
int main(void)
{
    /* initialize the Hardware and the EtherCAT Slave Controller */
#if FC1100_HW
    if (HW_Init())
    {
        HW_Release();
        return;
    }
#else
    HW_Init();
    mflash_drv_init();
#endif
    MainInit();

    nStartDownload = FALSE;
    nCompleteDownload = FALSE;
    pAPPL_FoeRead = FoE_Read;
    pAPPL_FoeReadData = FoE_ReadData;
    pAPPL_FoeWrite = FoE_Write;
    pAPPL_FoeWriteData = FoE_WriteData;
    
    firmwareDownloadSize = 0;
    bRunApplication = TRUE;

    PRINTF("Image version: 2.4.0\r\n");

    /* determine if there is any image in TEST state */
    for (int image = 0; image < MCUBOOT_IMAGE_NUMBER; image++) {
        status_t status;
        uint32_t imgstate;

        status = bl_get_image_state(image, &imgstate);
        if (status != kStatus_Success) {
            PRINTF("Failed to get state of image %u (ret %d)", image, status);
        }

        if (imgstate == kSwapType_Testing) {
            PRINTF("Image state: testing\r\n");
            /* Set image as confirmed */
            status = bl_update_image_state(image, kSwapType_Permanent);
            if (status != kStatus_Success) {
                PRINTF("FAILED to accept image (ret=%d)\n", status);
            }
            PRINTF("Set image as confirmed: success\r\n");
            break;
        }
    }

    do {
        if (FoE_IsDownloaded()) {
            PRINTF("System reset...\r\n");
            NVIC_SystemReset();
        }
        
        MainLoop();
    } while (bRunApplication == TRUE);

    HW_Release();
    return 0;
}



