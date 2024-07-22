/*****************************************************************************
 *
 * MODULE:             AT-ZCP
 *
 * COMPONENT:          MDI_ReadFirmVer.h
 *
 * AUTHOR:             Mohammed Suhel
 *
 * DESCRIPTION:        Reads the MantraG firmware version before starting MDI programmer
 *
 * $HeadURL: https://www.collabnet.nxp.com/svn/lprf_sware/Projects/AT-Jenie/Modules/AT-Jenie_Application/Branches/AT-ZCPv1.1NoRtos_R22_SE/Source/Common/MDI_ReadFirmVer.h $
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
#ifndef MDI_READFIRMVER_H_
#define MDI_READFIRMVER_H_

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#if JENNIC_CHIP_FAMILY == JN516x
#include "dbg_uart.h"
#include "app_common.h"
#endif
#include "dbg.h"
#include "SMAC_Uart.h"
#include "SMAC_MsgTypes.h"
#include "SMAC_Common.h"
#include "SMAC_Rpc.h"
#ifdef ZSE_BUILD
#include "version.h"
#endif
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
PUBLIC void vMDI_SyncWithMtG(void);
PUBLIC void vMDI_SMACUartInit (void);
PUBLIC void vMDI_SMACUartClose (void);
PUBLIC bool bMDI_SendSync(void);
PUBLIC bool_t bMDI_VerifyMtGFirmVersion(void);
#ifdef ZSE_BUILD
PUBLIC void bMDI_SendSLBusyMessage(uint8 u8Status);
#else
#define bMDI_SendSLBusyMessage(x)	\
		DBG_vPrintf(TRUE, "OL Module Busy...\r\n");
#endif

/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Inlined Functions                                            ***/
/****************************************************************************/

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#endif /* MDI_READFIRMVER_H_ */
