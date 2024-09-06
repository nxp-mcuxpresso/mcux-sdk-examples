/*
* Copyright 2023 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef SERIAL_LINK_CMDS_CTRL_H_
#define SERIAL_LINK_CMDS_CTRL_H_

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include "pdum_apl.h"
#include "zps_apl_af.h"
#include "zcl.h"
#include "zps_apl_aib.h"
#include "appZdpExtraction.h"
#include "serial_link_ctrl.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/


/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
PUBLIC void vSL_HandleApduEvent(uint8 *pu8RxSerialMsg, PDUM_thAPduInstance *pmyPDUM_thAPduInstance, ZPS_tsAfEvent *psStackEvent);
PUBLIC void ZPS_vSetOverrideLocalMacAddress(uint64 *pu64Address);
PUBLIC ZPS_teStatus ZPS_bIsDeviceKeyPresent(uint64 u64IeeeAddress,  bool_t *pbPresent);
PUBLIC void vSL_HandleNwkEvent(uint8 *pu8RxSerialMsg, ZPS_tsAfEvent *psStackEvent, ZPS_tsAfZdpEvent *psApsZdpEvent);
PUBLIC teZCL_Status eAPP_GetKeyTableEntry(uint16 u16Index, ZPS_tsAplApsKeyDescriptorEntry* psEntry);
PUBLIC teZCL_Status eAPP_GetKeyTableEntryByIEEEAddress(uint64 u64IEEEAddress, ZPS_tsAplApsKeyDescriptorEntry* psEntry);
PUBLIC teZCL_Status eAPP_GetBinding(uint16 u16Index, ZPS_tsAplApsmeBindingTableEntry* psEntry);
PUBLIC void vStartJoinNetwork(void);
PUBLIC teZCL_Status eAPP_GetAddressMapTableEntry(uint16 u16Index, uint8 *pu8AddressMap);
PUBLIC teZCL_Status eAPP_GetRoutingTableEntry(uint16 u16Index, ZPS_tsNwkRtEntry *psEntry);
PUBLIC uint8 u8SetAPSFrameCounter(uint32 u32FrameCounter, bool_t bIsInOrOut, uint64 u64IEEEAddress);
PUBLIC uint8 u8SetNwkFrameCounter(uint32 u32FrameCounter,bool_t bIsInOrOut,uint16 u16NwkAddress);
PUBLIC void vChangeChannel(uint32 u32ChannelNum);
PUBLIC void vUpdateDefaultTCAPSLinkKey(uint8 *pu8key);
PUBLIC uint8 u8GetVersionAndDeviceType(uint32 *pu32Version,teSL_ZigBeeDeviceType *peDeviceType, teSL_DeviceMacType *peMacType, uint16 *pu16OptionMask, uint32 *pu32SDKVersion);
PUBLIC uint8 u8GetJNHardwarePartId(uint32 *pu32JNChipId);
PUBLIC uint8 u8SL_GetJnPDMUse(uint8 pu8SegmentIndex, uint32 *pu32JNPDMUse);
PUBLIC void vSetNetworkKey(uint8 au8Key[ZPS_NWK_KEY_LENGTH]);
PUBLIC uint8 u8ClearNetworkKey(uint8 u8KeySeqNo);
PUBLIC void vSL_HandleInterpanEvent(uint8 *pu8RxSerialMsg, PDUM_thAPduInstance *pmyPDUM_thAPduInstance, ZPS_tsAfEvent *psStackEvent);
PUBLIC teZCL_Status eAPP_GetMacAdddress(uint16 u16Index, uint64* pu64Address);
PUBLIC uint8 ZPS_u8AplZdoGetCurrentRadioChannel(void);
PUBLIC uint16 ZPS_u16AplZdoGetNwkPanId(void);
PUBLIC uint32 ZPS_u32AplZdoGetCurrentChannelMask(void);
PUBLIC ZPS_teStatus eSetUpForInterPan(uint32 u32ChannelMask);
PUBLIC void vGetNetworkKey(uint8 au8Key[ZPS_NWK_KEY_LENGTH]);
PUBLIC uint16 u16GetNeighbourTableSize(void);
PUBLIC uint16 u16GetAddressMapTableSize(void);
PUBLIC uint16 u16GetRoutingTableSize(void);
PUBLIC uint16 u16GetApsKeyTableSize(void);
PUBLIC uint8 u8ErasePersistentData(void);
PUBLIC void MAC_vSetFilterStorage(void);
PUBLIC uint32 MAC_eMacFilterAddAccept(uint16 u16Addr, uint8 u8LinkQuality);
PUBLIC uint8 u8GetSecurityMaterialSetSize(void);
PUBLIC teZCL_Status eGetSecurityMaterialSetEntry(uint8 u8Index, ZPS_tsNwkSecMaterialSet* psEntry);
PUBLIC teZCL_Status eClearSecurityMaterialSetEntry(uint8 u8Index);
PUBLIC void vSearchAndRejoinTrustCenter(void);
PUBLIC void vSL_HandleNodeParentIndication(uint8 *pu8RxSerialMsg, uint16 *pu16ShortAddress, uint64 *pu64LongAddress,uint64 *pu64ParentAddress, uint8 *pu8Status,uint16 *pu16ID);
PUBLIC uint32 u32GetJNBootloaderVersion(void);
PUBLIC void vSL_SetJnInternalAttenuator(bool_t bSetAttenuation);
PUBLIC uint8 u8SL_GetLastRssi(void);
PUBLIC ZPS_teStatus vSL_GetGlobalStats(
            uint32                      *pu32TotalSuccessfulTX,
            uint32                      *pu32TotalFailTX,
            uint32                      *pu32TotalRX);

            PUBLIC bool_t bSL_GetDeviceStats(
            uint64                      u64Mac,
            uint8                       *pu8LastLqi,
            uint8                       *pu8AverageLqi);
PUBLIC ZPS_teStatus zps_eAplZdoRemoveDeviceFromTables(uint64 u64ParentAddr, uint64  u64ChildAddress);
PUBLIC bool bCheckFragmentedPacket(void);
PUBLIC ZPS_teStatus eSL_ChangePanId(uint16 u16PanId);
PUBLIC ZPS_teStatus eSL_SetPanIdConflictResolution(bool_t bAllowConflictResolution);
PUBLIC int16 i16APP_ReadJNTempSensorVal(void);
PUBLIC PHY_Enum_e eAppApiPlmeSetTxPower(uint32 u32TxPower);
PUBLIC void vSL_GetMacAddrTable(uint8 *pu8Table);
PUBLIC void vSL_SetMacAddrTable(uint8 *pu8Table);
PUBLIC uint8 u8SL_StopJoin(void);
PUBLIC void vSL_HandleZDPMsgEvent(
                    uint8                       *pu8RxSerialMsg,
                    PDUM_thAPduInstance         *pmyPDUM_thAPduInstance,
                    ZPS_tsAfEvent               *psStackEvent,
                    ZPS_tsAfZdpEvent            *psApsZdpEvent);
PUBLIC ZPS_teStatus JPT_eConvertEnergyTodBm(uint8 pu8Energy, int16* p16EnergyIndBm);
PUBLIC ZPS_teStatus ZPS_eNwkManagerState(uint8* pu8NwkManagerState);
PUBLIC ZPS_teStatus ZPS_eAplZdpActiveEndpointReq(uint16 u16TargetAddress);
PUBLIC ZPS_teStatus u8SerialLinkDataTest(uint8 *pu8Data);
PUBLIC void vAPP_SetMaxMACPollFailCount(uint8 u8MaxMacFailCount);
PUBLIC ZPS_teStatus zps_eAplZdoInsecureRejoinNetwork(bool_t bWithDiscovery, bool_t bAllChannels);
PUBLIC ZPS_teStatus zps_eAplZdoInsecureRejoinToShorttPan( bool_t bAllChannels);
PUBLIC ZPS_teStatus  eAplZdoInsecureRejoinToMask( uint32 u32ChannelMask, bool_t bInsecure, bool_t bToShortPan );
PUBLIC bool bSL_SetNumEZScans(uint8 u8NumScans);

PUBLIC ZPS_teStatus ZPS_eAplZdoGetCurrentRadioChannel(uint8* pu8RadioChan);
PUBLIC ZPS_teStatus eChangeSubGHzChannel(uint32* pu32ChanMask);
PUBLIC ZPS_teStatus eSetNetworkInterfaceRequest(ZPS_tsNwkNlmeReqSetInterface* psSetNWKIntrf);
PUBLIC ZPS_teStatus eGetNetworkInterfaceRequest(uint8 u8InterfaceIndex, ZPS_tsNwkNlmeCfmGetInterface* psGetNWKIntrf);
PUBLIC uint8 u8SerialLinkSetMacInterfaces( bool_t bEnable2G4, bool_t bEnableSG);
PUBLIC ZPS_teStatus eSendMgmtNWKEnhancedUpdateReq(uint16 u16DstAddr, uint8 u8ScanDuration, uint8 u8ScanCount, uint32 u32ChanMask);
PUBLIC ZPS_teStatus eSendMgmtNWKUnsolicitedEnhancedUpdateNotify(uint16 u16DstAddr, uint32 u32ChanInUse, uint16 u16MACTxUnicastTotal, uint16 u16MACTxUnicastFailures, uint16 u16MACTxUnicastRetries, uint8 u8PeriodOfTimeForResults);
PUBLIC ZPS_teStatus eSendStartEnergyDetectScan(uint32 u32ScanChanMask, uint8 u8ScanDuration);
PUBLIC ZPS_teStatus eSendGetClearNibTxUcastBytesCount(uint8 u8GetClear, uint16 u16ShortAddress, uint32* pu32NibTxUcastBytesCount);
PUBLIC ZPS_teStatus eSendGetClearNibRxUcastBytesCount(uint8 u8GetClear, uint16 u16ShortAddress, uint32* pu32NibRxUcastBytesCount);
PUBLIC ZPS_teStatus eSendGetClearMacIfTxFailCount(uint8 u8GetClear, uint8 u8MacId, uint32* pu32TxFailCount);
PUBLIC ZPS_teStatus eSendGetClearMacIfTxRetryCount(uint8 u8GetClear, uint8 u8MacId, uint32* pu32TxRetryCount);
PUBLIC ZPS_teStatus eSetEndDeviceTimeOutTimeOnParent(uint16* pu16NwkAddr, uint8* pu8TimeOutConstInd);
PUBLIC ZPS_teStatus eConvertLqiToRssidBm(uint8* pu8Lqi, uint8* pu8MacId);
PUBLIC ZPS_teStatus APP_eAplZdoBind( uint16 u16ClusterId,
                                      uint8 u8SrcEndpoint,
                                      uint16 u16DstNwkAddr,
                                      uint64 u64DstIeeeAddr,
                                      uint8 u8DstEndpoint,
                                      uint8 u8DstAddrMode);

PUBLIC ZPS_teStatus APP_eAplZdoUnbind( uint16 u16ClusterId,
                                      uint8 u8SrcEndpoint,
                                      uint16 u16DstNwkAddr,
                                      uint64 u64DstIeeeAddr,
                                      uint8 u8DstEndpoint,
                                      uint8 u8DstAddrMode);

PUBLIC uint8 u8SendStrtRouter( uint8* pu8NwkKey,
                               uint64 u64Epid,
                               uint64 u64TCAddr,
                               uint8 u8KSN,
                               uint8 u8Channel,
                                 uint16 u16NwkAddr,
                                 uint16 u16PanId);

PUBLIC uint8 u8SendPdmConversionCommand(uint8 u8FromVersion, uint8 u8ToVersion);
PUBLIC ZPS_teStatus eSL_EnableDisableDualMac(uint8 u8Endpoint,
                                             uint16 u16ClusterId,
                                             bool bOutput,
                                             bool_t bEnableDualMac);
PUBLIC uint8 eHaltJN(void);
PUBLIC uint8 u8CountApdus(uint8* pu8Count);
PUBLIC uint8 u8ExhaustApdus(void);
PUBLIC uint8 u8setStackPollRate(uint16 u16RateMs);
PUBLIC uint32 u32GetInFrameCounter(uint16 u16Index);
PUBLIC uint8 u8BlockMDServer( uint8 u8State);
PUBLIC ZPS_teStatus eSL_NetworkChangeAddress(uint16 u16NwkAddr);
PUBLIC ZPS_teStatus eSL_GetJnAuthorizationTimeout(uint16 *pu16AuthorizationTimeout);
PUBLIC uint8 u8SetLeveDecider(uint8 u8LeaveFlags);
PUBLIC uint8 u8ExhaustDescriptors( void);
PUBLIC uint8 u8SetRxOnIdleState(uint8 u8Intf, uint8 u8State);
PUBLIC uint8 u8GetRxOnIdleState(uint8 u8Intf, uint8 *pu8RxOnIdle);
PUBLIC uint8 u8SleepOL(uint8 u8Mode, uint32 u32TimeMs);
PUBLIC uint8 u8WakeOL( void);
PUBLIC uint8 u8ReadOLParams( void);
PUBLIC bool_t ZPS_bSetAntennaInput (uint8 u8Input);
PUBLIC uint8 u8GetCounterForAddress(uint64 u64Address);
PUBLIC uint8 u8GetSerialStats( tsJNSerialStats *psStats);
PUBLIC uint8 u8SerialOutOfSeqTest( void);
PUBLIC uint8 u8DuplicateOnAirTest(void);
PUBLIC uint8 u8GetNetworkOutFrameCounter( uint32* pu32FrameCounter);
PUBLIC uint8 u8SendErrorTestcode(uint8 u8Code);
PUBLIC uint8 u8GetStatusFlags( uint32 *pu32Flags);
PUBLIC uint8 u8ResetBoundServer(void);
PUBLIC uint8 u8EnableBoundDevices( bool_t bEnable);
PUBLIC uint8 u8SetBoundDevice(uint64 u64Address, bool_t bState);
PUBLIC uint8 u8GetNwkState( uint8* pu8ZdoState, uint8* pu8NwkState);
PUBLIC uint8 u8SendCloneZed( uint8 *pu8NwkKey,
                               uint64 u64Epid,
                               uint64 u64TCAddr,
                               uint8 u8KSN,
                               uint8 u8Channel,
                                 uint16 u16NwkAddr,
                                 uint16 u16PanId);
PUBLIC uint8 u8SetManufacturerCode( uint16 u16ManCode );
PUBLIC uint8 u8GetReprovisionData( uint64 u64Address, tsReprovissionData *psReprovission);
PUBLIC uint8 u8SetReprovisionData( tsReprovissionData *psReprovission);
PUBLIC uint8 u8SetJnAuthorizationTimeout(uint8 u8TimeOut);
PUBLIC uint8 u8SendEndDeviceTimeOut( void);
uint8 u8SetParentTimeoutMethod(uint8 u8Method);
PUBLIC bool_t ZPS_bIsCoprocessorNewModule(void);

/****************************************************************************/
/***        External Functions                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Inlined Functions                                            ***/
/****************************************************************************/

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#endif /* SERIAL_LINK_CMDS_CTRL_H_ */
