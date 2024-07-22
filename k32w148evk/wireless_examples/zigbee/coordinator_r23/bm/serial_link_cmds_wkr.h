/*
* Copyright 2023 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef SERIAL_LINK_CMDS_WKR_H_
#define SERIAL_LINK_CMDS_WKR_H_

#include "app_common.h"
#include "zps_gen.h"
#include "PDM.h"
#include "pdum_gen.h"

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/


/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define PDM_ID_APP_CHAN_MASK_LIST 	  (0x1002)

#define VAR_UNUSED(x) (void)(x)

#define MAX_PACKET_RX_SIZE  (1600+35)       /* 35 bytes for additional fields apart from APDU */

#define MAX_NUMBER_OF_STATS_ENTRY   16

PUBLIC uint64 ZNC_RTN_U64(uint8 *, uint64);
PUBLIC uint32 ZNC_RTN_U32(uint8 *, uint64);
PUBLIC uint16 ZNC_RTN_U16(uint8 *, uint64);

PUBLIC void ZNC_BUF_U64_UPD_R(uint8 *, uint64);
PUBLIC void ZNC_BUF_U32_UPD_R(uint8 *, uint32);
PUBLIC void ZNC_BUF_U16_UPD_R(uint8 *, uint16);

#define ZNC_BUF_U8_UPD( BUFFER, U8VALUE, LEN)    (  ( *( (uint8*)( ( BUFFER ) ) ) = ( ( ( ( uint8 ) ( U8VALUE ) ) & 0xFF ) ) ) ,\
     ( ( LEN ) += sizeof( uint8 ) ) )

#define ZNC_BUF_U64_UPD( BUFFER, U64VALUE, LEN)    (ZNC_BUF_U64_UPD_R ( (uint8*) ( BUFFER ), ( uint64 ) ( U64VALUE ) ),\
    ( ( LEN ) += sizeof( uint64 ) ) )

#define ZNC_BUF_U32_UPD( BUFFER, U32VALUE, LEN )     (ZNC_BUF_U32_UPD_R ( (uint8*) ( BUFFER ), ( uint32 ) ( U32VALUE ) ),\
    ( ( LEN ) += sizeof ( uint32 ) ) )

#define ZNC_BUF_U16_UPD( BUFFER, U16VALUE, LEN )     (ZNC_BUF_U16_UPD_R ( (uint8*) ( BUFFER ), ( uint16 ) ( U16VALUE ) ),\
    ( ( LEN ) += sizeof( uint16 ) ) )

/* Macros take buffer and return data and the next offset of within the buffer */
#define ZNC_RTN_U8_OFFSET(BUFFER, i, OFFSET )   ( ( uint8 ) (BUFFER)[ i ] & 0xFF );\
( ( OFFSET ) += sizeof (uint8) )

#define ZNC_RTN_U16_OFFSET(BUFFER, i, OFFSET )   ( ZNC_RTN_U16 (BUFFER, i) );\
( ( OFFSET ) += sizeof (uint16) )

#define ZNC_RTN_U32_OFFSET(BUFFER, i, OFFSET )   (  ZNC_RTN_U32 (BUFFER, i) );\
( ( OFFSET ) += sizeof (uint32) )

#define ZNC_RTN_U64_OFFSET(BUFFER, i, OFFSET )  (  ZNC_RTN_U64 (BUFFER, i) );\
( ( OFFSET ) += sizeof (uint64) )

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
#ifndef PACK
#define PACK __attribute__ ((packed))
#endif

typedef struct {
	teState eState;
	teNodeState eNodeState;
    uint8 u8DeviceType;
    uint64 u64MacAddress;
#if (defined MULTIMAC_SELECTION)
    bool_t bIsDualMac;
#endif
    uint8 u8BlockLeaves;
} tsNcpDeviceDesc;

typedef struct
{
    uint32 u32ClId_DstEp_SrcEp;
    uint8 u8BrdAddrMode;
    uint16 u16GrpAddr;
    uint16 u16DestAddr;
    uint64 u64DestAddr;
    uint16 u16SecMd_Radius;
    uint16 u16PayloadLen;
    uint16 u16ProfileId;
    uint8 u8DestAddrMode;
}tsCommon;

typedef struct {
    uint8 u8LastLQI;
    uint8 u8AverageLQI;
    uint32 u32RxCount;
    uint64 u64MacAddress;
}tsDeviceStats;

typedef struct {
    uint32 u32TotalSuccessfulTX;
    uint32 u32TotalFailTX;
    uint32 u32TotalRX;
    tsDeviceStats sDeviceStats[MAX_NUMBER_OF_STATS_ENTRY];
} tsNwkStats;

extern tsNcpDeviceDesc     sNcpDeviceDesc;
extern tsNwkStats sNwkStats;
extern PUBLIC uint32 g_u32ChannelMaskList[ZPS_MAX_CHANNEL_LIST_SIZE];

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
PUBLIC void vCleanStackTables(uint64 u64RemoveDevice);
PUBLIC void vSaveDevicePdmRecord(void);
PUBLIC void vProcessIncomingSerialCommands(void);

/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/
extern PUBLIC uint32 g_u32ChannelMaskList[ZPS_MAX_CHANNEL_LIST_SIZE];

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#endif /* SERIAL_LINK_CMDS_WKR_H_ */
