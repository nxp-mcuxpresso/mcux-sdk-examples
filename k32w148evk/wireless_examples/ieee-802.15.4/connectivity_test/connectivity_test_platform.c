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
#include "connectivity_test_platform.h"
#include "fsl_format.h"

/************************************************************************************
*************************************************************************************
* Macros
*************************************************************************************
************************************************************************************/
#define gCTSelf_EVENT_c (1<<7)
#define SelfNotificationEvent() (OSA_EventSet(&gTaskEvent, gCTSelf_EVENT_c));

#define Serial_Print(a,b,c)  SerialManager_WriteBlocking((serial_write_handle_t)g_connWriteHandle, (uint8_t *)b, strlen(b))
#define Serial_PrintDec(a,b) SerialManager_WriteBlocking((serial_write_handle_t)g_connWriteHandle, FORMAT_Dec2Str(b), strlen((char const *)FORMAT_Dec2Str(b)))

/* Converts a 0x00-0x0F number to ascii '0'-'F' */
#define HexToAscii(hex) (uint8_t)( ((hex) & 0x0F) + ((((hex) & 0x0F) <= 9) ? '0' : ('A'-10)) )

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
const registerLimits_t registerIntervals[] = 
{
  {.regStart=0x00 , .regEnd=0x00, FALSE}
};

uint8_t u8Prbs9Buffer[] =
{
  0x42,
  0xff,0xc1,0xfb,0xe8,0x4c,0x90,0x72,0x8b,0xe7,0xb3,0x51,0x89,0x63,0xab,0x23,0x23,  
  0x02,0x84,0x18,0x72,0xaa,0x61,0x2f,0x3b,0x51,0xa8,0xe5,0x37,0x49,0xfb,0xc9,0xca,
  0x0c,0x18,0x53,0x2c,0xfd,0x45,0xe3,0x9a,0xe6,0xf1,0x5d,0xb0,0xb6,0x1b,0xb4,0xbe,
  0x2a,0x50,0xea,0xe9,0x0e,0x9c,0x4b,0x5e,0x57,0x24,0xcc,0xa1,0xb7,0x59,0xb8,0x87
};
/************************************************************************************
*************************************************************************************
* Private function prototypes
*************************************************************************************
************************************************************************************/

serial_manager_status_t Serial_PrintHex(uint8_t *buffer, uint32_t length, uint8_t le)
{
  serial_manager_status_t status = kStatus_SerialManager_Success;
  uint8_t hexString[2];
  uint8_t hex;

  if (le)
    buffer = buffer + length - 1;

  while (length) {
    hex = *buffer;

    hexString[0] = HexToAscii(hex >> 4);
    hexString[1] = HexToAscii(hex);

    buffer = buffer + (le ? -1 : 1);
    length--;

    status = SerialManager_WriteBlocking((serial_write_handle_t)g_connWriteHandle, hexString, 2);
    if (status != kStatus_SerialManager_Success)
      break;
  }

  return status;
}

/************************************************************************************
*
* InitApp_custom
*
************************************************************************************/
void InitApp_custom()
{
}

/************************************************************************************
*
* InitProject_custom
*
************************************************************************************/
void InitProject_custom()
{
#if CT_Feature_Xtal_Trim
  AppToAspMessage_t msg;
  
  msg.msgType = aspMsgTypeGetXtalTrimReq_c;
  xtalTrimValue = APP_ASP_SapHandler(&msg, 0);
#endif
}

/************************************************************************************
*
* PrintTestParameters
*
************************************************************************************/
void PrintTestParameters(bool_t bEraseLine)
{
  uint8_t u8lineLen = 100;
  uint8_t u8Index;
  
  if(bEraseLine)
  {
    Serial_Print(mAppSer, "\r", gAllowToBlock_d);
    for(u8Index = 0;u8Index<u8lineLen;u8Index++)
    {
      Serial_Print(mAppSer, " ", gAllowToBlock_d);
    }
    Serial_Print(mAppSer ,"\r", gAllowToBlock_d);
  }
  
  Serial_Print(mAppSer, "Mode ", gAllowToBlock_d);
  if(mTxOperation_c == testOpMode)
  {
    Serial_Print(mAppSer, "Tx", gAllowToBlock_d);
    if (useAck == gAckTypeImmediate_c) {
      Serial_Print(mAppSer, "[ack]", gAllowToBlock_d);
    } else if (useAck == gAckTypeEnhanced_c) {
      Serial_Print(mAppSer, "[enhAck]", gAllowToBlock_d);
    }
  }
  else
  {
    Serial_Print(mAppSer, "Rx", gAllowToBlock_d);
  }
  Serial_Print(mAppSer, ", Channel ", gAllowToBlock_d);
  Serial_PrintDec(mAppSer, (uint32_t)testChannel);
  Serial_Print(mAppSer,", Power ", gAllowToBlock_d);
  Serial_PrintDec(mAppSer,(uint32_t)testPower);
  Serial_Print(mAppSer,", Payload ", gAllowToBlock_d);
  Serial_PrintDec(mAppSer, (uint32_t)testPayloadLen);
  Serial_Print(mAppSer,", CCA Thresh ", gAllowToBlock_d);
  if(ccaThresh != 0)
  {
    Serial_Print(mAppSer,"-",gAllowToBlock_d);
  }
  Serial_PrintDec(mAppSer, (uint32_t)ccaThresh);
  Serial_Print(mAppSer,"dBm",gAllowToBlock_d);
  
#if CT_Feature_Xtal_Trim
  Serial_Print(mAppSer,", XtalTrim ", gAllowToBlock_d);
  Serial_PrintDec(mAppSer, (uint32_t)xtalTrimValue);
#endif
  
  Serial_Print(mAppSer," >", gAllowToBlock_d);
}

/************************************************************************************
*
* ShortCutsParser
*
* Performs changes in different menus whenever shortcuts are allowed
*
************************************************************************************/
void ShortCutsParser(uint8_t u8UartData)
{
  static ReadAddressStates_t readAddrState = gReadAddressNone_c;
  static uint16_t address = 0;
  static uint8_t addr_digits = 0;
  static uint8_t addr_digit_str[2] = {'\0', '\0'};
  evDataFromUART = FALSE;

  switch (readAddrState) {
  case gReadAddressSrc_c:
    if ((u8UartData >= '0') && (u8UartData <= '9')) {

      address = (address << 4) + (u8UartData - '0');
      addr_digits++;

      addr_digit_str[0] = u8UartData;
      Serial_Print(mAppSer, (const char *)addr_digit_str, gAllowToBlock_d);

    } else if ((u8UartData >= 'a') && (u8UartData <= 'f')) {

      address = (address << 4) + (u8UartData - 'a' + 10);
      addr_digits++;

      addr_digit_str[0] = u8UartData;
      Serial_Print(mAppSer, (const char *)addr_digit_str, gAllowToBlock_d);
    }

    if (addr_digits == 4) {
      gAppTxPacket->smacHeader.srcAddr = address;
      readAddrState = gReadAddressNone_c;

      // also set device short address
      (void)SMACSetShortSrcAddress(address);
      (void)SMACSetExtendedSrcAddress(gAppTxPacket->smacHeader.srcAddr);
      
        Serial_Print(mAppSer, "\r\n\n", gAllowToBlock_d);
      address = 0;
      addr_digits = 0;
      evTestParameters = TRUE;
    }

    return;

  case gReadAddressDst_c:
    if ((u8UartData >= '0') && (u8UartData <= '9')) {

      address = (address << 4) + (u8UartData - '0');
      addr_digits++;

      addr_digit_str[0] = u8UartData;
      Serial_Print(mAppSer, (const char *)addr_digit_str, gAllowToBlock_d);

    } else if ((u8UartData >= 'a') && (u8UartData <= 'f')) {

      address = (address << 4) + (u8UartData - 'a' + 10);
      addr_digits++;

      addr_digit_str[0] = u8UartData;
      Serial_Print(mAppSer, (const char *)addr_digit_str, gAllowToBlock_d);
    }

    if (addr_digits == 4) {
      gAppTxPacket->smacHeader.destAddr = address;
      readAddrState = gReadAddressNone_c;

      Serial_Print(mAppSer, "\r\n\n", gAllowToBlock_d);
      address = 0;
      addr_digits = 0;
      evTestParameters = TRUE;
    }
    return;

  default:
    evTestParameters = TRUE;
  }


  switch(u8UartData){
  case 't':
    testOpMode = mTxOperation_c;
    break;
  case 'r':
    testOpMode = mRxOperation_c;
    break;
  case 'q': 
    if(testChannel == (channels_t)gTotalChannels)
    {
      testChannel = gChannel11_c;
    }
    else
    {
      testChannel++;
    }
    break;
  case 'w':
    if(testChannel == gChannel11_c)
    {
      testChannel = (channels_t)gTotalChannels;
    }
    else
    {
      testChannel--;
    }
    break;
  case 'a':
    testPower++;
    if(gMaxOutputPower_c < testPower)
    {
      testPower = gMinOutputPower_c;
    }
    break;
  case 's':
    if(testPower == gMinOutputPower_c)
    {
      testPower = gMaxOutputPower_c;
    }
    else
    {
      testPower--;	
    }
    break;
  case 'n':
    testPayloadLen++;
    if(gMaxSmacSDULength_c < testPayloadLen)
    {
      testPayloadLen = 17;
    }    
    break;
  case 'm':
    testPayloadLen--;
    if(17 > testPayloadLen)
    {
      testPayloadLen = gMaxSmacSDULength_c;
    }    
    break;
  case 'k':
    ccaThresh++;
    if(ccaThresh > gMaxCCAThreshold_c)
    {
      ccaThresh = gMinCCAThreshold_c;
    }
    break;
  case 'l':
    ccaThresh--;
    if(ccaThresh > gMaxCCAThreshold_c)
    {
      ccaThresh = gMaxCCAThreshold_c;
    }
    break;
#if CT_Feature_Xtal_Trim
  case 'd':
    xtalTrimValue++;
    if(xtalTrimValue > gMaxTrimValue_c)
    {
      xtalTrimValue = gMinTrimValue_c;
    }
    break;
  case 'f':
    if(xtalTrimValue == gMinTrimValue_c)
    {
      xtalTrimValue = gMaxTrimValue_c;
    }
    else
    {
      xtalTrimValue--;
    }
#endif
    break;
  case 'z':
    if (useAck == gAckTypeNone_c) {
      useAck = gAckTypeImmediate_c;
      SMACSetTxAutoAck(true);
    } else if (useAck == gAckTypeImmediate_c) {
      useAck = gAckTypeEnhanced_c;
      SMACSetTxEnhAck(true);
    } else {
      useAck = gAckTypeNone_c;
      SMACSetTxAutoAck(false);
      SMACSetTxEnhAck(false);
    }
    break;
  case 'x':
    Serial_Print(mAppSer, "\r\n\nChange src address (", gAllowToBlock_d);
    Serial_PrintHex((uint8_t*)(&gAppTxPacket->smacHeader.srcAddr), 2, 1);
    Serial_Print(mAppSer, ") [0-9 a-z symbols] >", gAllowToBlock_d);

    readAddrState = gReadAddressSrc_c;
    evTestParameters = FALSE;
    break;
  case 'c':
    Serial_Print(mAppSer, "\r\n\nChange dst address (", gAllowToBlock_d);
    Serial_PrintHex((uint8_t*)(&gAppTxPacket->smacHeader.destAddr), 2, 1);
    Serial_Print(mAppSer, ") [0-9 a-z symbols] >", gAllowToBlock_d);

    readAddrState = gReadAddressDst_c;
    evTestParameters = FALSE;
    break;
  default:
    evDataFromUART = TRUE;
    evTestParameters = FALSE;
    break;
  }
}
