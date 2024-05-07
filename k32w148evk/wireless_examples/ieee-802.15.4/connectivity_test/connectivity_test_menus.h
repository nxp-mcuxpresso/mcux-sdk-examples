/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2021 NXP
* All rights reserved.
*
* \file
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#include "EmbeddedTypes.h"          /*Include special data types*/    
#include "fsl_component_serial_manager.h"
/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
extern char * const cu8Logo[];
extern char * const cu8MainMenu[]; 
extern char * const cu8ShortCutsBar[];
extern char * const cu8ContinuousTestMenu[]; 
extern char * const cu8PerTxTestMenu[]; 
extern char * const cu8PerRxTestMenu[];
extern char * const cu8RangeTxTestMenu[];
extern char * const cu8RangeRxTestMenu[];
extern char * const cu8RadioRegistersEditMenu[];
extern char * const cu8RadioCSTCSelectMenu[];
extern char * const cu8CsTcTestMenu[];
extern char * const cu8ContinuousTestTags[]; 
extern char * const cu8SelectTags[];
extern char * const cu8TxTestTags[];
extern char * const cu8RxTestTags[];
extern char * const cu8TxModTestTags[];
#if gMpmMaxPANs_c == 2
extern char * const cu8MpmMenuPs[];
#endif
/************************************************************************************
*************************************************************************************
* Module macros
*************************************************************************************
************************************************************************************/

#define isAsciiHex(Data) (((Data >= '0') && (Data <= '9')) \
                       || ((Data >= 'A') && (Data <= 'F')) \
                       || ((Data >= 'a') && (Data <= 'f'))) 
               
/************************************************************************************
*************************************************************************************
* Interface functions prototypes
*************************************************************************************
************************************************************************************/
void PrintMenu(char * const pu8Menu[], serial_write_handle_t writeHandle);
