/****************************************************************************
 *
 * Copyright 2020 NXP
 *
 * NXP Confidential. 
 * 
 * This software is owned or controlled by NXP and may only be used strictly 
 * in accordance with the applicable license terms.  
 * By expressly accepting such terms or by downloading, installing, activating 
 * and/or otherwise using the software, you are agreeing that you have read, 
 * and that you agree to comply with and are bound by, such license terms.  
 * If you do not agree to be bound by the applicable license terms, 
 * then you may not retain, install, activate or otherwise use the software. 
 * 
 *
 ****************************************************************************/


/*****************************************************************************
 *
 * MODULE:             Basic Cluster
 *
 * COMPONENT:          BinaryInputBasic_internal.h
 *
 * DESCRIPTION:        The internal API for the Binary Input Basic Cluster
 *
 *****************************************************************************/

#ifndef  BINARY_INPUT_BASIC_INTERNAL_H_INCLUDED
#define  BINARY_INPUT_BASIC_INTERNAL_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/

#include "jendefs.h"

#include "zcl.h"
#include "BinaryInputBasic.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

#ifdef BINARY_INPUT_BASIC_SERVER                
PUBLIC teZCL_Status eCLD_BinaryInputBasicHandler(
                uint8                               u8SourceEndPointId);
#endif  

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* BINARY_INPUT_BASIC_INTERNAL_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
