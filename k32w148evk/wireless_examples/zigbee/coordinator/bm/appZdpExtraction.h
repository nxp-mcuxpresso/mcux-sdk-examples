/****************************************************************************
 *
 * Copyright 2020, 2022, 2023 NXP
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
 * MODULE:      Utils
 *
 * COMPONENT:   appZdpExtractions.h
 *
 * DESCRIPTION:
 *
 *****************************************************************************/

#ifndef APPZDPEXTRACTION_H_
#define APPZDPEXTRACTION_H_

#include <stdbool.h>
#include <jendefs.h>
#include "zps_apl_zdp.h"
#include "zps_apl_af.h"
#include "pdum_apl.h"
#include "pdum_nwk.h"

/* Maximum number of binding table entries */
#ifndef APP_ZDP_MAX_NUM_BINDING_TABLE_ENTRIES
#define APP_ZDP_MAX_NUM_BINDING_TABLE_ENTRIES 5
#endif

/* Maximum number of network descriptors */
#ifndef APP_ZDP_MAX_NUM_NETWORK_DESCR
#define APP_ZDP_MAX_NUM_NETWORK_DESCR   5
#endif

/* Maximum number of neighbor table entries */
#ifndef APP_ZDP_MAX_NUM_NT_LIST_ENTRIES
#define APP_ZDP_MAX_NUM_NT_LIST_ENTRIES 2
#endif

/* Maximum number of reported discovery cache entries */
#ifndef APP_ZDP_MAX_NUM_DISCOVERY_CACHE
#define APP_ZDP_MAX_NUM_DISCOVERY_CACHE 5
#endif

typedef struct {
    uint8   u8Status;
} ZPS_tsAplZdpSingleStatusRsp;

typedef struct {
    union {
        ZPS_tsAplZdpDeviceAnnceReq sDeviceAnnce;
        ZPS_tsAplZdpMgmtNwkUpdateReq sMgmtNwkUpdateReq;
        ZPS_tsAplZdpMgmtNwkEnhanceUpdateReq sMgmtEnhancedUpdateReq;
#ifdef R23_UPDATES
        ZPS_tsAplZdpMgmtNwkBeaconSurveyReq sMgmtBeaconSurveyReq;
#endif
        ZPS_tsAplZdpMgmtPermitJoiningReq sPermitJoiningReq;
#ifndef R23_UPDATES
        ZPS_tsAplZdpDiscoveryCacheRsp sDiscoveryCacheRsp;
        ZPS_tsAplZdpDiscoveryStoreRsp sDiscoveryStoreRsp;
        ZPS_tsAplZdpNodeDescStoreRsp sNodeDescStoreRsp;
        ZPS_tsAplZdpActiveEpStoreRsp sActiveEpStoreRsp;
        ZPS_tsAplZdpSimpleDescStoreRsp sSimpleDescStoreRsp;
        ZPS_tsAplZdpRemoveNodeCacheRsp sRemoveNodeCacheRsp;
        ZPS_tsAplZdpEndDeviceBindRsp sEndDeviceBindRsp;
#endif
        ZPS_tsAplZdpBindRsp sBindRsp;
        ZPS_tsAplZdpUnbindRsp sUnbindRsp;
        ZPS_tsAplZdpSingleStatusRsp sSingleStatusRsp;
#ifndef R23_UPDATES
        ZPS_tsAplZdpReplaceDeviceRsp sReplaceDeviceRsp;
        ZPS_tsAplZdpStoreBkupBindEntryRsp sStoreBkupBindEntryRsp;
        ZPS_tsAplZdpRemoveBkupBindEntryRsp sRemoveBkupBindEntryRsp;
        ZPS_tsAplZdpBackupSourceBindRsp sBackupSourceBindRsp;
#endif
        ZPS_tsAplZdpMgmtLeaveRsp sMgmtLeaveRsp;
#ifndef R23_UPDATES
        ZPS_tsAplZdpMgmtDirectJoinRsp sMgmtDirectJoinRsp;
#endif
        ZPS_tsAplZdpMgmtPermitJoiningRsp sPermitJoiningRsp;
        ZPS_tsAplZdpNodeDescRsp sNodeDescRsp;
        ZPS_tsAplZdpPowerDescRsp sPowerDescRsp;
        ZPS_tsAplZdpSimpleDescRsp sSimpleDescRsp;
        ZPS_tsAplZdpNwkAddrRsp sNwkAddrRsp;
        ZPS_tsAplZdpIeeeAddrRsp sIeeeAddrRsp;
#ifndef R23_UPDATES
        ZPS_tsAplZdpUserDescConf sUserDescConf;
#endif
        ZPS_tsAplZdpSystemServerDiscoveryRsp sSystemServerDiscoveryRsp;
#ifndef R23_UPDATES
        ZPS_tsAplZdpPowerDescStoreRsp sPowerDescStoreRsp;
        ZPS_tsAplZdpUserDescRsp sUserDescRsp;
#endif
        ZPS_tsAplZdpActiveEpRsp sActiveEpRsp;
        ZPS_tsAplZdpMatchDescRsp sMatchDescRsp;
#ifndef R23_UPDATES
        ZPS_tsAplZdpComplexDescRsp sComplexDescRsp;
        ZPS_tsAplZdpFindNodeCacheRsp sFindNodeCacheRsp;
        ZPS_tsAplZdpExtendedSimpleDescRsp sExtendedSimpleDescRsp;
        ZPS_tsAplZdpExtendedActiveEpRsp sExtendedActiveEpRsp;
        ZPS_tsAplZdpBindRegisterRsp sBindRegisterRsp;
        ZPS_tsAplZdpBackupBindTableRsp sBackupBindTableRsp;
        ZPS_tsAplZdpRecoverBindTableRsp sRecoverBindTableRsp;
        ZPS_tsAplZdpRecoverSourceBindRsp sRecoverSourceBindRsp;
        ZPS_tsAplZdpMgmtNwkDiscRsp sMgmtNwkDiscRsp;
#endif
        ZPS_tsAplZdpMgmtLqiRsp sMgmtLqiRsp;
        ZPS_tsAplZdpMgmtRtgRsp sRtgRsp;
        ZPS_tsAplZdpMgmtBindRsp sMgmtBindRsp;
#ifndef R23_UPDATES
        ZPS_tsAplZdpMgmtCacheRsp sMgmtCacheRsp;
#endif
#ifdef R23_UPDATES
        ZPS_tsAplZdpMgmtNwkBeaconSurveyRsp sMgmtBeaconSurveyRsp;
#endif
        ZPS_tsAplZdpMgmtNwkUpdateNotify sMgmtNwkUpdateNotify;
        ZPS_tsAplZdpMgmtNwkUnSolictedUpdateNotify sMgmtNwkUnsolicitedUpdateNotify;
        ZPS_tsAplZdpMgmtMibIeeeRsp sMgmtMibIeeeRsp;
    }uZdpData;
    union {
        ZPS_tsAplZdpBindingTableEntry asBindingTable[APP_ZDP_MAX_NUM_BINDING_TABLE_ENTRIES];
        ZPS_tsAplZdpNetworkDescr asNwkDescTable[APP_ZDP_MAX_NUM_NETWORK_DESCR];
        ZPS_tsAplZdpNtListEntry asNtList[APP_ZDP_MAX_NUM_NT_LIST_ENTRIES];
#ifndef R23_UPDATES
        ZPS_tsAplDiscoveryCache aDiscCache[APP_ZDP_MAX_NUM_DISCOVERY_CACHE];
#endif
        uint16 au16Data[34];
        uint8 au8Data[77];
        uint64 au64Data[9];
    }uLists;
    uint16 u16ClusterId;
    uint8 u8SequNumber;
}ZPS_tsAfZdpEvent;


PUBLIC bool zps_bAplZdpUnpackNwkAddressResponse(ZPS_tsAfEvent *psZdoServerEvent, 
                                                ZPS_tsAfZdpEvent* psReturnStruct);
                                                
PUBLIC bool zps_bAplZdpUnpackIeeeAddressResponse(ZPS_tsAfEvent *psZdoServerEvent, 
                                                ZPS_tsAfZdpEvent* psReturnStruct);
                                                
PUBLIC bool zps_bAplZdpUnpackNwkUpdateReq(ZPS_tsAfEvent *psZdoServerEvent, 
                                          ZPS_tsAfZdpEvent* psReturnStruct);

PUBLIC bool zps_bAplZdpUnpackEnhancedNwkUpdateReq(ZPS_tsAfEvent *psZdoServerEvent,
                                          ZPS_tsAfZdpEvent* psReturnStruct);
                                          
PUBLIC bool zps_bAplZdpUnpackPermitJoinReq(ZPS_tsAfEvent *psZdoServerEvent,
                                           ZPS_tsAfZdpEvent* psReturnStruct);
                                           
PUBLIC bool zps_bAplZdpUnpackDevicAnnounce(ZPS_tsAfEvent *psZdoServerEvent,
                                           ZPS_tsAfZdpEvent* psReturnStruct);
                                           
PUBLIC bool zps_bAplZdpUnpackNodeDescResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                              ZPS_tsAfZdpEvent* psReturnStruct);                                              
                                              
PUBLIC bool zps_bAplZdpUnpackPowerDescResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                               ZPS_tsAfZdpEvent* psReturnStruct);
                                               
PUBLIC bool zps_bAplZdpUnpackSimpleDescResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                ZPS_tsAfZdpEvent* psReturnStruct);
                                                
PUBLIC bool zps_bAplZdpUnpackActiveEpResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                              ZPS_tsAfZdpEvent* psReturnStruct);
                                              
PUBLIC bool zps_bAplZdpUnpackMatchDescResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                               ZPS_tsAfZdpEvent* psReturnStruct);
                                               
#ifndef R23_UPDATES
PUBLIC bool zps_bAplZdpUnpackDiscCacheResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                               ZPS_tsAfZdpEvent* psReturnStruct);
                                               
PUBLIC bool zps_bAplZdpUnpackDiscStoreResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                               ZPS_tsAfZdpEvent* psReturnStruct);
                                               
PUBLIC bool zps_bAplZdpUnpackNodeDescStoreResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                   ZPS_tsAfZdpEvent* psReturnStruct);
                                                   
PUBLIC bool zps_bAplZdpUnpackActiveEpStoreResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                   ZPS_tsAfZdpEvent* psReturnStruct);
                                                   
PUBLIC bool zps_bAplZdpUnpackSimpleDescStoreResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                     ZPS_tsAfZdpEvent* psReturnStruct);
                                                     
PUBLIC bool zps_bAplZdpUnpackRemoveNodeCacheResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                     ZPS_tsAfZdpEvent* psReturnStruct);
                                                     
PUBLIC bool zps_bAplZdpUnpackBackUpSourceBindResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                      ZPS_tsAfZdpEvent* psReturnStruct);
#endif
                                                                                                            
PUBLIC bool zps_bAplZdpUnpackMgmtLeaveResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                               ZPS_tsAfZdpEvent* psReturnStruct);
                                               
#ifndef R23_UPDATES
PUBLIC bool zps_bAplZdpUnpackMgmtDirectJoinResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                    ZPS_tsAfZdpEvent* psReturnStruct);
#endif
                                                    
PUBLIC bool zps_bAplZdpUnpackMgmtPermitJoinResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                    ZPS_tsAfZdpEvent* psReturnStruct);
                                                    
#ifndef R23_UPDATES
PUBLIC bool zps_bAplZdpUnpackEndDeviceBindResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                   ZPS_tsAfZdpEvent* psReturnStruct);
#endif
                                                   
PUBLIC bool zps_bAplZdpUnpackBindResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                          ZPS_tsAfZdpEvent* psReturnStruct);
                                          
PUBLIC bool zps_bAplZdpUnpackUnBindResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                            ZPS_tsAfZdpEvent* psReturnStruct);
                                            
#ifndef R23_UPDATES
PUBLIC bool zps_bAplZdpUnpackReplaceDeviceResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                   ZPS_tsAfZdpEvent* psReturnStruct);
#endif
                                                   
PUBLIC bool zps_bAplZdpUnpackStoreBkupBindResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                   ZPS_tsAfZdpEvent* psReturnStruct);
                                                   
PUBLIC bool zps_bAplZdpUnpackRemoveBkupBindResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                    ZPS_tsAfZdpEvent* psReturnStruct);
                                                    
PUBLIC bool zps_bAplZdpUnpackMgmtLqiResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                             ZPS_tsAfZdpEvent* psReturnStruct);
                                             
PUBLIC bool zps_bAplZdpUnpackMgmtRtgResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                             ZPS_tsAfZdpEvent* psReturnStruct);
                                             
PUBLIC bool zps_bAplZdpUnpackNwkUpdateNotifyResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                     ZPS_tsAfZdpEvent* psReturnStruct);

PUBLIC bool zps_bAplZdpUnpackNwkUnsolicitedUpdateNotify(ZPS_tsAfEvent *psZdoServerEvent,
                                                     ZPS_tsAfZdpEvent* psReturnStruct );
                                                     
#ifndef R23_UPDATES
PUBLIC bool zps_bAplZdpUnpackComplexDescResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                 ZPS_tsAfZdpEvent* psReturnStruct);
                                                     
PUBLIC bool zps_bAplZdpUnpackUserDescResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                              ZPS_tsAfZdpEvent* psReturnStruct);

PUBLIC bool zps_bAplZdpUnpackUserDescConfirmResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                     ZPS_tsAfZdpEvent* psReturnStruct);
#endif
                                                     
PUBLIC bool zps_bAplZdpUnpackSystemServerDiscResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                      ZPS_tsAfZdpEvent* psReturnStruct);
                                                      
PUBLIC bool zps_bAplZdpUnpackBkupBindTableResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                   ZPS_tsAfZdpEvent* psReturnStruct);
                                                   
#ifndef R23_UPDATES
PUBLIC bool zps_bAplZdpUnpackPowerDescStoreResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                    ZPS_tsAfZdpEvent* psReturnStruct);
                                                    
PUBLIC bool zps_bAplZdpUnpackFindNodeCacheResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                   ZPS_tsAfZdpEvent* psReturnStruct);
                                                   
PUBLIC bool zps_bAplZdpUnpackExtendedSimpleDescResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                        ZPS_tsAfZdpEvent* psReturnStruct);
#endif
                                                        
PUBLIC bool zps_bAplZdpUnpackExtendedActiveEndpointResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                            ZPS_tsAfZdpEvent* psReturnStruct);
                                                            
#ifndef R23_UPDATES
PUBLIC bool zps_bAplZdpUnpackBindRegisterResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                  ZPS_tsAfZdpEvent* psReturnStruct);
                                                  
PUBLIC bool zps_bAplZdpUnpackRecoverBindTableResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                      ZPS_tsAfZdpEvent* psReturnStruct);
                                                      
PUBLIC bool zps_bAplZdpUnpackRecoverSourceBindResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                       ZPS_tsAfZdpEvent* psReturnStruct);                                               

                                                  
PUBLIC bool zps_bAplZdpUnpackMgmtNwkDiscResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                                 ZPS_tsAfZdpEvent* psReturnStruct);
#endif
                                                 
PUBLIC bool zps_bAplZdpUnpackMgmtBindResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                              ZPS_tsAfZdpEvent* psReturnStruct);
                                              
#ifndef R23_UPDATES
PUBLIC bool zps_bAplZdpUnpackMgmtCacheResponse(ZPS_tsAfEvent *psZdoServerEvent,
                                               ZPS_tsAfZdpEvent* psReturnStruct);
#endif
                                               
PUBLIC bool zps_bAplZdpUnpackResponse (ZPS_tsAfEvent *psZdoServerEvent,
                                       ZPS_tsAfZdpEvent* psReturnStruct);


#endif /* APPZDPEXTRACTION_H_ */
