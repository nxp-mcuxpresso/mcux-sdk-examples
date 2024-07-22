/*****************************************************************************
 *
 * MODULE:             AT-ZCP
 *
 * COMPONENT:          MDI_Programmer.h
 *
 * AUTHOR:             Mohammed Suhel
 *
 * DESCRIPTION:        MantraG MDI Programmer
 *
 * $HeadURL: https://www.collabnet.nxp.com/svn/lprf_sware/Projects/AT-Jenie/Modules/AT-Jenie_Application/Branches/AT-ZCPv1.1NoRtos_R22_SE/Source/Common/MDI_Programmer.h $
 *
 * $Revision:  $
 *
 * $LastChangedBy: nxp71769 $
 *
 * $LastChangedDate:  $
 *
 * $Id:  $
 *
 *****************************************************************************
 *
 * This software is owned by Jennic and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on Jennic products. You, and any third parties must reproduce
 * the copyright and warranty notice and any other legend of ownership on each
 * copy or partial copy of the software.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS". JENNIC MAKES NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
 * ACCURACY OR LACK OF NEGLIGENCE. JENNIC SHALL NOT, IN ANY CIRCUMSTANCES,
 * BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT LIMITED TO, SPECIAL,
 * INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON WHATSOEVER.
 *
 * Copyright Jennic Ltd 2007. All rights reserved
 *
 ****************************************************************************/
#ifndef MDI_PROGRAMMER_H_
#define MDI_PROGRAMMER_H_

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#if JENNIC_CHIP_FAMILY == JN516x
#include "dbg_uart.h"
#include "PeripheralRegs_JN516x.h"
#endif
#include "dbg.h"
#include "MDI_ReadFirmVer.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define DIO_SETTINGS_ADDRESS		(0x00080012UL)
#define PROG_OL_SETTINGS_ADDRESS	(0x00080013UL)
#define LOCK_OL_SETTINGS_ADDRESS	(0x00080014UL)
#define CUSTOMER_1_SETTINGS 			(0x00U)
#define CUSTOMER_2_SETTINGS 			(0x01U)
#define NXP_SETTINGS 					(0x02U)
#define PROG_OL_DISABLED				(0x00U)
#define PROG_OL_ENABLED					(0x01U)
#define LOCK_OL_DISABLED				(0x00U)
#define LOCK_OL_ENABLED					(0x01U)
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
/* MDI Command ID's */
typedef enum
{
	E_MDI_CMD_ENTER_MON_MODE				= (0x5A3Cu),
	E_MDI_CMD_ERASE_EROM_ENTER_INIT_MODE	= (0x0011u),
	E_MDI_CMD_ENTER_PROTECTED_MODE			= (0x0010u),
	E_MDI_CMD_PROG_EROM_PAGE				= (0x0009u),
	E_MDI_CMD_PROG_EROM_WORD				= (0x0008u),
	E_MDI_CMD_READ_EROM_WORDS				= (0x000Eu)
}MDI_teMDICommandID;

/* HOST command or data identifiers */
typedef enum
{
	E_MDI_HOST_DATA,
	E_MDI_HOST_CMND
}MDI_teHostDataCmnd;

/* MDI Command response status codes */
typedef enum
{
	E_MDI_RESP_CMND_EXECUTED_SUCCESSFULLY	= 0x00u,
	E_MDI_RESP_BOOTING_IN_MON_MODE			= 0x03u,
	E_MDI_RESP_INVALID_CMND					= 0x04u,
	E_MDI_RESP_CMND_EXE_ABORTED				= 0x05u,
	E_MDI_RESP_INVALID_PARAM				= 0x06u,
	E_MDI_RESP_NON_VOL_PROG_ERROR			= 0x07u
}MDI_teMDIStatusCode;

/* MDI state machine status codes */
typedef enum
{
	E_MDI_SUCCESS = 0x00,
	E_MDI_FAILURE,
	E_MDI_FAILURE_ERR_CODE1,
	E_MDI_FAILURE_ERR_CODE2,
	E_MDI_FAILURE_ERR_CODE3,
	E_MDI_FAILURE_ERR_CODE4,
	E_MDI_FAILURE_ERR_CODE5,
	E_MDI_FAILURE_ERR_CODE6,
	E_MDI_FAILURE_ERR_CODE7,
	E_MDI_FAILURE_ERR_CODE8,
	E_MDI_FAILURE_ERR_CODE9,
	E_MDI_FAILURE_ERR_CODE10,   //Just for testing added few more error codes
	E_MDI_FAILURE_ERR_CODE11,
	E_MDI_FAILURE_ERR_CODE12,
	E_MDI_FAILURE_ERR_CODE13,
	E_MDI_FAILURE_ERR_CODE14,
	E_MDI_FAILURE_ERR_CODE15,
	E_MDI_FAILURE_ERR_CODE16,
	E_MDI_FAILURE_ERR_CODE17,
	E_MDI_FAILURE_ERR_CODE18,
	E_MDI_FAILURE_ERR_CODE19,
	E_MDI_FAILURE_ERR_CODE20,
	E_MDI_FAILURE_ERR_CODE21,
	E_MDI_FAILURE_ERR_CODE22,
	E_MDI_FAILURE_ERR_CODE23,
	E_MDI_FAILURE_ERR_CODE24,
	E_MDI_FAILURE_ERR_CODE25,
	E_MDI_FAILURE_ERR_CODE26,
	E_MDI_FAILURE_ERR_CODE27,
	E_MDI_FAILURE_ERR_CODE28,
	E_MDI_FAILURE_ERR_CODE29,
	E_MDI_FAILURE_ERR_CODE30,
	E_MDI_FAILURE_ERR_CODE31,
	E_MDI_FAILURE_ERR_CODE32,
	E_MDI_FAILURE_ERR_CODE33,
	E_MDI_FAILURE_ERR_CODE34,
	E_MDI_FAILURE_ERR_CODE35,
}MDI_teStatus;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
PUBLIC void vMDI_PrintIOConfig(void);
PUBLIC void vMDI_SetUpMDIAndResetLines (void);
PUBLIC void vMDI_Reset868MtGRadio (void);
PUBLIC MDI_teStatus eMDI_EnterMonitorMode (bool_t bSendEraseCmd);
PUBLIC MDI_teStatus eMDI_ProgramMtGBinary (void);
PUBLIC MDI_teStatus eMDI_PutMtGtoProtMode (void);
PUBLIC void vMDI_ToggleReset868MtG( void);
PUBLIC void vMDI_WakeOL(void);
PUBLIC void vMDI_SetAllIOasInputs (void);
PUBLIC void vMDI_RecoverDioSettings (void);
/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Inlined Functions                                            ***/
/****************************************************************************/
#if JENNIC_CHIP_FAMILY == JN516x  // TODO chip type
INLINE uint8 u8ReadDioSettings(void)
{
	/* 0x00080010 is the beginning of Encryption IV space */
	uint8* pu8CustomerDioSettings = (uint8*)DIO_SETTINGS_ADDRESS;
	return *pu8CustomerDioSettings;
}

INLINE uint8 u8ReadProgOLSettings(void)
{
	/* 0x00080010 is the beginning of Encryption IV space */
	uint8* pu8ProgOLSettings = (uint8*)PROG_OL_SETTINGS_ADDRESS;
	return *pu8ProgOLSettings;
}

INLINE uint8 u8ReadLockOLSettings(void)
{
	/* 0x00080010 is the beginning of Encryption IV space */
	uint8* pu8LockOLSettings = (uint8*)LOCK_OL_SETTINGS_ADDRESS;
	return *pu8LockOLSettings;
}
#else
INLINE uint8 u8ReadDioSettings(void)
{
	/* For 8x thee is only one io setting */
	return NXP_SETTINGS;
}

INLINE uint8 u8ReadProgOLSettings(void)
{
#ifndef APP_MDI_ENABLED
	return PROG_OL_DISABLED;
#else
	return PROG_OL_ENABLED;
#endif
}

INLINE uint8 u8ReadLockOLSettings(void)
{
#ifndef PROTECT_OL
	return LOCK_OL_DISABLED;
#else
	return LOCK_OL_ENABLED	;
#endif
}
#endif

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#endif /* MDI_PROGRAMMER_H_ */
