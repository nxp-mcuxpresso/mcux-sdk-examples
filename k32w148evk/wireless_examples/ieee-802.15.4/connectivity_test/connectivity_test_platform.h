/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2021 NXP
* All rights reserved.
*
* \file
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */


/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "fsl_component_mem_manager.h"
#include "fsl_component_serial_manager.h"
#include "fsl_component_timer_manager.h"
#include "SMAC_Interface.h"         /*@CMA Conn Test*/
#include "FunctionLib.h"
#include "AspInterface.h"
#include "fsl_os_abstraction.h"
#include "board.h"

#include "connectivity_test_menus.h"
/************************************************************************************
*************************************************************************************
* Currently supported feature-sets. 
* DO NOT CHANGE as they MIGHT be platform dependent
*************************************************************************************
************************************************************************************/
/*This feature is only for sub-ghz phy's*/
#ifndef CT_Feature_Bitrate_Select
#define CT_Feature_Bitrate_Select  (0)
#endif

/*This feature is only for platforms that have BER Test Connectors
*(currently only KW01)*/
#ifndef CT_Feature_BER_Test
#define CT_Feature_BER_Test        (0)
#endif

#ifndef CT_Feature_Direct_Registers
#define CT_Feature_Direct_Registers (0)
#endif

#ifndef CT_Feature_Indirect_Registers
#define CT_Feature_Indirect_Registers (0)
#endif

/*This feature is currently supported only on KW01 platforms*/
#ifndef CT_Feature_Calibration
#define CT_Feature_Calibration	   (0)
#endif

/*This feature is only for sub-ghz platforms*/
#ifndef CT_Feature_Custom_CCA_Dur
#define CT_Feature_Custom_CCA_Dur  (0)
#endif

/*This feature is currently supported on KW01. Disabled by default*/
#ifndef CT_Feature_Afc
#define CT_Feature_Afc             (0)
#endif

#ifndef CT_Feature_RSSI_Has_Sign
#define CT_Feature_RSSI_Has_Sign   (1)
#endif

/* This feature is currently supported on MKW41/MKW21. Disabled by default */
#ifndef CT_Feature_Xtal_Trim
 #if gAspCapability_d
 #define CT_Feature_Xtal_Trim      (1)
 #else
 #define CT_Feature_Xtal_Trim      (0)
 #endif
#endif

#if CT_Feature_Calibration
#include "Flash_Adapter.h"
#endif

/************************************************************************************
*************************************************************************************
* Macros
*************************************************************************************
************************************************************************************/
#define gMaxOutputPower_c	       ( 0x20 ) 
#define gMinOutputPower_c              ( 0x00 )
#define gDefaultOutputPower_c          ( 0x05 )

#if CT_Feature_Calibration
#define gMinAdditionalRFOffset_c       ( -1000)
#define gMaxAdditionalRFOffset_c       ( 1000 )
#endif
             
#define gDefaultChannelNumber_c         gChannel11_c                                    
#define gMaxCCAThreshold_c              0x6EU
#define gMinCCAThreshold_c              0x00U
#define gDefaultCCAThreshold_c          0x50U

#if CT_Feature_Xtal_Trim
#define gMaxTrimValue_c                 0x7FU
#define gMinTrimValue_c                 0x00U
#endif

/*register size in bytes and ASCII characters macros*/

#define gRegisterSize_c	     (4)
#define gRegisterSizeASCII_c (2*gRegisterSize_c)

/*register address size in ASCII*/
#define gRegisterAddress_c	(2)
#define gRegisterAddressASCII_c (2*gRegisterAddress_c)

/************************************************************************************
*************************************************************************************
* Memory Type Definitions
*************************************************************************************
************************************************************************************/
typedef enum AckType_tag
{
  gAckTypeNone_c,
  gAckTypeImmediate_c,
  gAckTypeEnhanced_c,
}AckType_t;

/* these type definitions must be changed depending on register size and address range*/
typedef uint32_t registerSize_t;
typedef uint16_t registerAddressSize_t;

/*this structure defines the upper and lower bound for dump registers feature*/
typedef struct registerLimits_tag
{
  registerAddressSize_t regStart;
  registerAddressSize_t regEnd;
  bool_t  bIsRegisterDirect;
}registerLimits_t;

typedef enum operationModes_tag
{
  mTxOperation_c,
  mRxOperation_c
}operationModes_t;

typedef enum ReadAddressStates_tag
{
  gReadAddressNone_c,
  gReadAddressSrc_c,
  gReadAddressDst_c,
}ReadAddressStates_t;

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
extern const registerLimits_t registerIntervals[];

extern uint8_t crtBitrate;

extern bool_t evTestParameters;
extern bool_t evDataFromUART;
extern bool_t bEdDone;
extern bool_t shortCutsEnabled;

extern uint8_t testPower;
extern serial_handle_t mAppSer;
extern SERIAL_MANAGER_READ_HANDLE_DEFINE(g_connReadHandle);
extern SERIAL_MANAGER_WRITE_HANDLE_DEFINE(g_connWriteHandle);
extern uint8_t gu8UartData;
extern uint8_t au8ScanResults[];
extern uint8_t ccaThresh;
extern AckType_t useAck;
extern uint8_t testPayloadLen;
extern channels_t testChannel;

extern OSA_EVENT_HANDLE_DEFINE(gTaskEvent);;
extern operationModes_t testOpMode;

#if CT_Feature_Xtal_Trim
extern uint8_t xtalTrimValue;
#endif

extern txPacket_t *gAppTxPacket;

/************************************************************************************
*************************************************************************************
* Public functions declarations
*************************************************************************************
************************************************************************************/

extern void PrintMenu(char * const pu8Menu[], serial_write_handle_t writeHandle);
/*common functions declarations which have platform dependent behavior*/
extern void PrintTestParameters(bool_t bEraseLine);
extern void ShortCutsParser( uint8_t u8UartData );
extern void InitApp_custom();
extern void InitProject_custom();

