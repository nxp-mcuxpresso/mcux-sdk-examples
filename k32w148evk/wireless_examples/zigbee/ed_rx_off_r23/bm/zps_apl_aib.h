/****************************************************************************
 *
 * Copyright 2020, 2022-2024 NXP
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
 * MODULE:      ZPSAPL
 *
 * COMPONENT:   zps_apl_aib.h
 *
 * DESCRIPTION:
 *
 *****************************************************************************/

#ifndef ZPS_APL_AIB_H_
#define ZPS_APL_AIB_H_

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/

#include "zps_nwk_sec.h"
#include <zps_apl_zdo.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#ifndef ZPS_AIB_INLINE
#define ZPS_AIB_INLINE  INLINE
#endif

#define ZPS_APL_AIB_MAX_ENDPOINTS   242

#define apsBindingTable             psAplApsmeAibBindingTable
#define apsDesignatedCoordinator    bApsDesignatedCoordinator
#define apsUseExtendedPANID         u64ApsUseExtendedPanid
#define apsGroupTable               psAplApsmeGroupTable
#ifndef R23_UPDATES
#define apsNonmemberRadius          u8ApsNonMemberRadius
#endif
#define apsPermissionsConfiguration /* not implemented */
#define apsUseInsecureJoin          bApsUseInsecureJoin
#define apsInterframeDelay          u8ApsInterframeDelay
#define apsLastChannelEnergy        u8ApsLastChannelEnergy
#define apsLastChannelFailureRate   u8ApsLastChannelFailureRate
#define apsChannelTimer             u8ApsChannelTimer
#define apsDeviceKeyPairTable       psAplDeviceKeyPairTable
#define apsDefaultTCApsLinkKey      psAplDefaultTCApsLinkKey
#define apsTrustCenterAddress       u64ApsTrustCenterAddress
#define apsSecurityTimeoutPeriod    u16ApsSecurityTimeOutPeriod
#ifdef R23_UPDATES
#define apsSupportedKeyNegotiationMethods u8ApsSupportedKeyNegotiationMethods
#define apsSharedSecretsMask        u8SharedSecretsMask
#define apsZdoRestrictedMode        bApsZdoRestrictedMode
#endif

/* TODO - needs to be a table to support fragmentation */
#define apsMaxWindowSize            u8ApsMaxWindowSize
#ifdef R23_UPDATES
#define apsMaxSizeAsdu              u8ApsMaxSizeAsdu
#endif

#define apscMaxDescriptorSize       (64UL)
#define apscMaxFrameRetries         (3UL)
//This for depth 7 #define apscAckWaitDuration         (50000UL)
// Trac #543 Set wait for Ack time for nwk depth of 15
//This for depth 15
#define apscAckWaitDuration         (100000UL)

#define apscMinDuplicateRejectionTableSize (1UL)
#define apscMinHeaderOverhead       (12UL)
#define apscapsParentAnnounceBaseTimer    10  // Usage in seconds
#define apsParentAnnounceJitterMax        10  // Usage in seconds
#ifdef R23_UPDATES
#define apscJoinerTlvsUnfragmentedMaxSize 79
#endif

#ifdef WWAH_SUPPORT
#define ZPS_APL_WWAH_KEY_ROTATION        0x1
#define ZPS_APL_WWAH_CONFIG_MODE         0x2
#endif


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

#ifdef R23_UPDATES
typedef enum
{
    ZPS_E_INITIAL_JOIN_NO_AUTHENTICATION                   = 0x00,
    ZPS_E_INITIAL_JOIN_INSTALL_CODE_KEY                    = 0x01,
    ZPS_E_INITIAL_JOIN_ANONYMOUS_KEY_NEGOTIATION           = 0x02,
    ZPS_E_INITIAL_JOIN_KEY_NEGOTIATION_WITH_AUTHENTICATION = 0x03
} ZPS_teAibInitialJoinAuth;

typedef enum
{
    ZPS_E_POST_JOIN_KEY_UPDATE_NOT_UPDATE             = 0x00,
    ZPS_E_POST_JOIN_KEY_UPDATE_KEY_REQUEST_METHOD     = 0x01,
    ZPS_E_POST_JOIN_KEY_UPDATE_UNAUTH_KEY_NEGOTIATION = 0x02,
    ZPS_E_POST_JOIN_KEY_UPDATE_AUTH_KEY_NEGOTIATION   = 0x03,
    ZPS_E_POST_JOIN_KEY_UPDATE_APP_DEF_CERTIF_BASED   = 0x04
} ZPS_teAibPostJoinKeyUpdateMethod;

typedef enum
{
    ZPS_E_DLK_STATE_NONE               = 0x00,
    ZPS_E_DLK_STATE_START              = 0x01,
    ZPS_E_DLK_STATE_COMPLETE           = 0x02,
} ZPS_teAibKeyNegotiationState;
#endif

/* [I SP001349_sfr 56]  */
typedef struct
{
    uint16  u16Groupid;
    uint8   au8Endpoint[(242 + 7)/8];
}ZPS_tsAplApsmeGroupTableEntry;

typedef struct
{
    ZPS_tsAplApsmeGroupTableEntry *psAplApsmeGroupTableId;
    uint32 u32SizeOfGroupTable;
}ZPS_tsAplApsmeAIBGroupTable;


typedef struct
{
    uint16  u16Groupid;
    uint16  u16BitMap;
}ZPS_tsAPdmGroupTableEntry;

typedef struct
{
	ZPS_tsAPdmGroupTableEntry *psAplApsmeGroupTableId;
    uint32 u32SizeOfGroupTable;
}ZPS_tsPdmGroupTable;

/* [I SP001349_sfr 50,54] */
#define ZPS_tsAplApsmeBindingTableStoreEntry ZPS_tsAplApsmeBindingTableEntryDualAddr

typedef struct
{
	ZPS_tuAddress  uDstAddress;
    uint16         u16ClusterId;
    uint8          u8DstAddrMode;
    uint8          u8SourceEndpoint;
    uint8          u8DestinationEndPoint;
}ZPS_tsAplApsmeBindingTableEntry;

typedef struct
{
	ZPS_tuAddress  uDstAddress;
    uint16         u16NwkAddrResolved;
    uint16         u16ClusterId;
    uint8          u8DstAddrMode;
    uint8          u8SourceEndpoint;
    uint8          u8DestinationEndPoint;
}ZPS_tsAplApsmeBindingTableEntryDualAddr;

typedef struct
{
	ZPS_tsAplApsmeBindingTableStoreEntry* pvAplApsmeBindingTableEntryForSpSrcAddr;
    uint32 u32SizeOfBindingTable;
}ZPS_tsAplApsmeBindingTable;

#ifndef R23_UPDATES
typedef struct
{
    ZPS_tsAplApsmeBindingTableEntry* pvAplApsmeBindingTableForRemoteSrcAddr;
    uint32 u32SizeOfBindingCache;
    uint64* pu64RemoteDevicesList;
}ZPS_tsAplApsmeBindingTableCache;
#endif

typedef struct
{
    ZPS_tsAplApsmeBindingTable* psAplApsmeBindingTable;
#ifndef R23_UPDATES
    ZPS_tsAplApsmeBindingTableCache* psAplApsmeBindingTableCache;
#endif
}ZPS_tsAplApsmeBindingTableType;

typedef struct
{
    uint32 u32OutgoingFrameCounter;
    uint16 u16ExtAddrLkup;
    uint8  au8LinkKey[ZPS_SEC_KEY_LENGTH];
    uint8  u8BitMapSecLevl;
#ifdef R23_UPDATES
    uint8 u8InitialJoinAuth;
    uint8 u8PostJoinKeyUpdateMethod;
    uint8 u8KeyNegotiationMethod;
    uint8 u8PresharedSecretType;
    struct
	{
        uint8 u8KeyNegotiationState: 2;
        uint8 bPassphraseUpdateAllowed: 1;
        uint8 u8PassphraseLen:       5; /* Len == 0 means Passphrase unset */
	};
    uint8 au8Passphrase[ZPS_SEC_KEY_LENGTH];
#endif
} ZPS_tsAplApsKeyDescriptorEntry;

typedef struct
{
uint16    u16CredOffset;
uint16    u16TclkRetries;
uint8     u8KeyType;
} ZPS_TclkDescriptorEntry;

typedef struct
{
    ZPS_tsAplApsKeyDescriptorEntry *psAplApsKeyDescriptorEntry;
    uint16  u16SizeOfKeyDescriptorTable;
} ZPS_tsAplApsKeyDescriptorTable;

#ifdef R23_UPDATES
typedef struct 
{
    uint64   u64ApsChallengeTargetEui64;
    uint64   u64ApsChallengeValue;
    uint8    u8ApsChallengePeriodRemainingSeconds;
    ZPS_tsTsvTimer       sChallengeTimer;
} ZPS_tsAplApsChallengeReqEntry; 

typedef struct
{
    ZPS_tsAplApsChallengeReqEntry *psAplApsChallengeReqEntry;
    uint16  u16SizeOfChallengeReqTable;
} ZPS_tsAplApsChallengeReqTable;

typedef struct 
{
    uint16 u16LookupAddr;
    uint16 u16MaxIncomingTxSize;
    bool_t bSupported;

} ZPS_tsAplApsFragmentationEntry; 

typedef struct
{
    ZPS_tsAplApsFragmentationEntry *psAplApsFragmentationEntry;
    uint16  u16SizeOfFragmentationTable;
} ZPS_tsAplApsFragmentationCacheTable;
#endif


/* [I SP001349_sfr 19]  */
typedef struct
{
    /* persisted to flash */
    uint64  u64ApsTrustCenterAddress;           /* [I SP001379_sr 17, 344] */
    uint64  u64ApsUseExtendedPanid;
    bool_t  bApsDesignatedCoordinator;
    bool_t  bApsUseInsecureJoin;
    bool_t  bDecryptInstallCode;
#ifdef R23_UPDATES
    bool_t  bApsZdoRestrictedMode;
    bool_t  bRequireLinkKeyEncryptionForApsTransportKey;
    uint8   u8ApsSupportedKeyNegotiationMethods;
    uint8   u8SharedSecretsMask;
#endif
    uint8   u8KeyType;
    /* volatile */
#ifndef R23_UPDATES
#define AIB_START_VOLATILE_SECTION u8ApsNonMemberRadius
    uint8   u8ApsNonMemberRadius;
#else
#define AIB_START_VOLATILE_SECTION u8ApsInterframeDelay
#endif
    uint8   u8ApsInterframeDelay;
    uint8   u8ApsLastChannelEnergy;
    uint8   u8ApsLastChannelFailureRate;
    uint8   u8ApsChannelTimer;
    uint8   u8ApsMaxWindowSize;
#ifdef R23_UPDATES
    uint8   u8ApsMaxSizeAsdu; /* TODO: clarify overlap w/ ApsContext.u8MaxFragBlockSize */
#endif
    ZPS_tsAplApsmeBindingTableType *psAplApsmeAibBindingTable;
    ZPS_tsAplApsmeAIBGroupTable    *psAplApsmeGroupTable;
    ZPS_tsAplApsKeyDescriptorTable  *psAplDeviceKeyPairTable; /* [I SP001379_sr 344] */
#ifdef R23_UPDATES
    ZPS_tsAplApsFragmentationCacheTable *psAplFragmentationTable;
#endif
    ZPS_tsAplApsKeyDescriptorEntry  *psAplDefaultTCAPSLinkKey;
    bool_t   bParentAnnouncePending;
    bool_t   bUseInstallCode;
    uint16   u16ApsSecurityTimeOutPeriod;        /* [I SP001379_sr 344] */
#ifdef R23_UPDATES
    ZPS_tsAplApsChallengeReqTable *psAplChallengeReqTable;
    uint8    u8ApsChallengePeriodTimeoutSeconds;
    uint32   u32ApsChallengeFrameCounter;
    bool_t   *pbVerifiedFrameCounter;
#endif
    uint32   *pu32IncomingFrameCounter;
    uint32   *pau32ApsChannelMask;
} ZPS_tsAplAib;

#ifdef WWAH_SUPPORT
typedef struct
{
     uint8    u8WwahModeBitmask;
} ZPS_tsAplAibExtensions;
#endif
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

PUBLIC ZPS_tsAplAib *zps_psAplAibGetAib(void *);
PUBLIC uint64 zps_u64AplAibGetApsUseExtendedPanId(void *pvApl);
PUBLIC ZPS_teStatus zps_eAplAibSetApsUseExtendedPanId(void *pvApl, uint64 u64UseExtPanId);
PUBLIC uint32* zps_pu32AplAibGetApsChannelMask(void *pvApl, uint8 *u8ChannelMaskCount);
PUBLIC ZPS_teStatus zps_eAplAibSetApsChannelMask(void *pvApl, uint32 u32ChannelMask);
PUBLIC uint16 zps_u16AplAibGetDeviceKeyPairTableSize(void *pvApl);
PUBLIC ZPS_tsAplApsKeyDescriptorEntry zps_tsAplAibGetDeviceKeyPairTableEntry(void *pvApl, uint16 u16Index);
PUBLIC ZPS_teStatus zps_eAplAibSetApsChannelMaskByMacID (void *pvApl, uint16 u16MacID, uint32 u32ChannelMask);
PUBLIC ZPS_teStatus zps_eAplAibSetApsDesignatedCoordinator(void *pvApl, bool bDesignatedCoordinator);
PUBLIC ZPS_teStatus zps_eAplAibSetApsUseInstallCode(void *pvApl, bool bUseInstallCode);
PUBLIC bool zps_bAplAibGetApsUseInsecureJoin(void *pvApl);
PUBLIC ZPS_teStatus zps_eAplAibSetApsUseInsecureJoin(void *pvApl, bool bUseInsecureJoin);
PUBLIC ZPS_teStatus zps_eAplAibSetApsTrustCenterAddress(void *pvApl, uint64 u64TcAddress);
PUBLIC uint64 zps_eAplAibGetApsTrustCenterAddress(void *pvApl);
PUBLIC ZPS_teStatus zps_eAplAibRemoveBindTableEntryForMacAddress( void *pvApl, uint64 u64MacAddress );
PUBLIC uint8 zps_u8AplAibGetDeviceApsKeyType(void *pvApl, uint64 u64IeeeAddress, bool_t bFindFixTclk);
PUBLIC ZPS_teStatus zps_eAplAibSetDeviceApsKeyType(void *pvApl,uint64 u64IeeeAddress, uint8 u8KeyType);
PUBLIC ZPS_tsAplApsKeyDescriptorEntry** zps_psAplDefaultTrustCenterAPSLinkKey(void *pvApl);
#ifdef R23_UPDATES
PUBLIC ZPS_teStatus zps_eAplAibAddChallengeReqEntry(void* pvApl, uint64 u64Addr, uint64 u64ChallengeValue);
PUBLIC ZPS_teStatus zps_eAplAibRemoveChallengeReqEntry(void* pvApl, uint64 u64Addr);
PUBLIC bool zps_eAplAibFindChallengeReqTableEntry(void* pvApl, uint64 u64Addr, ZPS_tsAplApsChallengeReqEntry **psChallengeReqEntry);
PUBLIC ZPS_teStatus zps_eAplAibAddFragmentationTableEntry(void* pvApl, tuFragParams *psTlvFrag);
PUBLIC bool zps_eAplAibFindFragmentationTableEntry(void* pvApl, uint16 u16Addr, ZPS_tsAplApsFragmentationEntry **psFragmentationEntry);
#endif
#if defined(R23_UPDATES) || defined(WWAH_SUPPORT)
PUBLIC bool_t zps_bIsClusterReqWithApsKey(void *pvApl, uint8 u8Endpoint, uint16 u16ClusterId, ZPS_tuAddress uAddr, bool_t bExt);
#endif
#ifdef WWAH_SUPPORT
PUBLIC void ZPS_vAplExtdedAibSetWWAH ( uint8 u8BitmaskSet );
PUBLIC uint8 ZPS_u8AplExtdedAibGetWWAH (void);
#endif
/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/
extern ZPS_tsAplApsKeyDescriptorEntry** ZPS_psAplDefaultDistributedAPSLinkKey(void);
extern ZPS_tsAplApsKeyDescriptorEntry** ZPS_psAplDefaultGlobalAPSLinkKey(void);

#ifdef WWAH_SUPPORT
extern const void *zps_g_pvAib_extensions;
#endif
/****************************************************************************/
/***        Inlined Functions                                            ***/
/****************************************************************************/


ZPS_AIB_INLINE ZPS_tsAplAib *ZPS_psAplAibGetAib(void) ALWAYS_INLINE;
ZPS_AIB_INLINE ZPS_tsAplAib *ZPS_psAplAibGetAib(void)
{
    return zps_psAplAibGetAib(ZPS_pvAplZdoGetAplHandle());
}
#ifdef WWAH_SUPPORT
ZPS_AIB_INLINE ZPS_tsAplAibExtensions *ZPS_psAplAibGetAibExtentions(void) ALWAYS_INLINE;
ZPS_AIB_INLINE ZPS_tsAplAibExtensions *ZPS_psAplAibGetAibExtentions(void)
{
    return (void*)zps_g_pvAib_extensions;
}
#endif

ZPS_AIB_INLINE uint64 ZPS_u64AplAibGetApsUseExtendedPanId(void) ALWAYS_INLINE;
ZPS_AIB_INLINE uint64 ZPS_u64AplAibGetApsUseExtendedPanId(void)
{
    return zps_u64AplAibGetApsUseExtendedPanId(ZPS_pvAplZdoGetAplHandle());
}

ZPS_AIB_INLINE ZPS_teStatus ZPS_eAplAibSetApsUseExtendedPanId(uint64 u64UseExtPanId) ALWAYS_INLINE;
ZPS_AIB_INLINE ZPS_teStatus ZPS_eAplAibSetApsUseExtendedPanId(uint64 u64UseExtPanId)
{
    return zps_eAplAibSetApsUseExtendedPanId(ZPS_pvAplZdoGetAplHandle(), u64UseExtPanId);
}

ZPS_AIB_INLINE uint32* ZPS_pu32AplAibGetApsChannelMask(uint8 *channelMaskCount) ALWAYS_INLINE;
ZPS_AIB_INLINE uint32* ZPS_pu32AplAibGetApsChannelMask(uint8 *channelMaskCount)
{
    return zps_pu32AplAibGetApsChannelMask(ZPS_pvAplZdoGetAplHandle(), channelMaskCount);
}

ZPS_AIB_INLINE ZPS_teStatus ZPS_eAplAibSetApsChannelMask(uint32 u32ChannelMask) ALWAYS_INLINE;
ZPS_AIB_INLINE ZPS_teStatus ZPS_eAplAibSetApsChannelMask(uint32 u32ChannelMask)
{
    return zps_eAplAibSetApsChannelMask(ZPS_pvAplZdoGetAplHandle(), u32ChannelMask);
}

ZPS_AIB_INLINE PUBLIC uint16 ZPS_u16AplAibGetDeviceKeyPairTableSize(void) ALWAYS_INLINE;
ZPS_AIB_INLINE PUBLIC uint16 ZPS_u16AplAibGetDeviceKeyPairTableSize(void)
{
     return zps_u16AplAibGetDeviceKeyPairTableSize(ZPS_pvAplZdoGetAplHandle());
}

ZPS_AIB_INLINE ZPS_teStatus ZPS_eAplAibSetApsChannelMaskByMacID(uint16 u16MacID, uint32 u32ChannelMask) ALWAYS_INLINE;
ZPS_AIB_INLINE ZPS_teStatus ZPS_eAplAibSetApsChannelMaskByMacID(uint16 u16MacID, uint32 u32ChannelMask)
{
    return zps_eAplAibSetApsChannelMaskByMacID(ZPS_pvAplZdoGetAplHandle(), u16MacID, u32ChannelMask);
}

ZPS_AIB_INLINE PUBLIC ZPS_tsAplApsKeyDescriptorEntry ZPS_tsAplAibGetDeviceKeyPairTableEntry(uint16 u16Index) ALWAYS_INLINE;
ZPS_AIB_INLINE PUBLIC ZPS_tsAplApsKeyDescriptorEntry ZPS_tsAplAibGetDeviceKeyPairTableEntry(uint16 u16Index)
{
    return zps_tsAplAibGetDeviceKeyPairTableEntry(ZPS_pvAplZdoGetAplHandle(), u16Index);
}

ZPS_AIB_INLINE ZPS_teStatus ZPS_eAplAibSetApsDesignatedCoordinator(bool bDesignatedCoordinator) ALWAYS_INLINE;
ZPS_AIB_INLINE ZPS_teStatus ZPS_eAplAibSetApsDesignatedCoordinator(bool bDesignatedCoordinator)
{
    return zps_eAplAibSetApsDesignatedCoordinator(ZPS_pvAplZdoGetAplHandle(), bDesignatedCoordinator);
}

ZPS_AIB_INLINE ZPS_teStatus ZPS_eAplAibSetApsUseInstallCode(bool bUseInstallCode) ALWAYS_INLINE;
ZPS_AIB_INLINE ZPS_teStatus ZPS_eAplAibSetApsUseInstallCode(bool bUseInstallCode)
{
    return zps_eAplAibSetApsUseInstallCode(ZPS_pvAplZdoGetAplHandle(), bUseInstallCode);
}

ZPS_AIB_INLINE bool ZPS_bAplAibGetApsUseInsecureJoin(void) ALWAYS_INLINE;
ZPS_AIB_INLINE bool ZPS_bAplAibGetApsUseInsecureJoin(void)
{
    return zps_bAplAibGetApsUseInsecureJoin(ZPS_pvAplZdoGetAplHandle());
}

ZPS_AIB_INLINE ZPS_teStatus ZPS_eAplAibSetApsUseInsecureJoin(bool bUseInsecureJoin) ALWAYS_INLINE;
ZPS_AIB_INLINE ZPS_teStatus ZPS_eAplAibSetApsUseInsecureJoin(bool bUseInsecureJoin)
{
    return zps_eAplAibSetApsUseInsecureJoin(ZPS_pvAplZdoGetAplHandle(), bUseInsecureJoin);
}

ZPS_AIB_INLINE ZPS_teStatus ZPS_eAplAibSetApsTrustCenterAddress(uint64 u64TcAddress) ALWAYS_INLINE;
ZPS_AIB_INLINE ZPS_teStatus ZPS_eAplAibSetApsTrustCenterAddress(uint64 u64TcAddress)
{
    return zps_eAplAibSetApsTrustCenterAddress(ZPS_pvAplZdoGetAplHandle(), u64TcAddress);
}

ZPS_AIB_INLINE uint64 ZPS_eAplAibGetApsTrustCenterAddress( void) ALWAYS_INLINE;
ZPS_AIB_INLINE uint64 ZPS_eAplAibGetApsTrustCenterAddress( void)
{
    return zps_eAplAibGetApsTrustCenterAddress(ZPS_pvAplZdoGetAplHandle());
}

ZPS_AIB_INLINE ZPS_teStatus ZPS_eAplAibRemoveBindTableEntryForMacAddress( uint64    u64MacAddress) ALWAYS_INLINE;
ZPS_AIB_INLINE ZPS_teStatus ZPS_eAplAibRemoveBindTableEntryForMacAddress( uint64    u64MacAddress)
{
    return zps_eAplAibRemoveBindTableEntryForMacAddress( ZPS_pvAplZdoGetAplHandle() , u64MacAddress );
}
ZPS_AIB_INLINE uint8 ZPS_u8AplAibGetDeviceApsKeyType(uint64 u64IeeeAddress) ALWAYS_INLINE;
ZPS_AIB_INLINE uint8 ZPS_u8AplAibGetDeviceApsKeyType(uint64 u64IeeeAddress)
{
    return zps_u8AplAibGetDeviceApsKeyType(ZPS_pvAplZdoGetAplHandle(), u64IeeeAddress, FALSE);
}

ZPS_AIB_INLINE ZPS_teStatus ZPS_eAplAibSetDeviceApsKeyType(uint64 u64IeeeAddress, uint8 u8KeyType ) ALWAYS_INLINE;
ZPS_AIB_INLINE ZPS_teStatus ZPS_eAplAibSetDeviceApsKeyType(uint64 u64IeeeAddress, uint8 u8KeyType)
{
    return zps_eAplAibSetDeviceApsKeyType(ZPS_pvAplZdoGetAplHandle(), u64IeeeAddress, u8KeyType);
}

ZPS_AIB_INLINE ZPS_tsAplApsKeyDescriptorEntry** ZPS_psAplDefaultTrustCenterAPSLinkKey( void) ALWAYS_INLINE;
ZPS_AIB_INLINE ZPS_tsAplApsKeyDescriptorEntry** ZPS_psAplDefaultTrustCenterAPSLinkKey( void)
{
    return zps_psAplDefaultTrustCenterAPSLinkKey(ZPS_pvAplZdoGetAplHandle());
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#endif /* ZPS_APL_AIB_H_ */
