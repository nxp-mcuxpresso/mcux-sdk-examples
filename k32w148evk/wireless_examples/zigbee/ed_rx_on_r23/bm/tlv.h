/****************************************************************************
 *
 * Copyright 2022-2024 NXP
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
 * COMPONENT:   tlv.h
 *
 * DESCRIPTION:
 *
 *****************************************************************************/

#ifndef TLV_H_
#define TLV_H_

#include <jendefs.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifndef ZPS_TLV_CONTEXT_COUNT
/* It should not matter that stack and app were built at different moments
 * and with different context counts; the stack only cares about CTX 0 */
#define ZPS_TLV_CONTEXT_COUNT 1
#endif

#define ZPS_TLV_HDR_SIZE    (sizeof(tsTlvGeneric))

/* Minimum length, some TLVs may have optional fields */
#define ZPS_TLVLEN_G_MANUFSPEC         2
#define ZPS_TLVLEN_G_SUPPKEYNEGMETH    2
#define ZPS_TLVLEN_G_PANIDCONFLREP     4
#define ZPS_TLVLEN_G_PANIDNEXT         2
#define ZPS_TLVLEN_G_CHANNEXT          4
#define ZPS_TLVLEN_G_SYMPASSPHRASE     (sizeof(tuSymPassphrase) - ZPS_TLV_HDR_SIZE)
#define ZPS_TLVLEN_G_ROUTERINFO        2
#define ZPS_TLVLEN_G_FRAGPARAMS        5
#define ZPS_TLVLEN_G_CONFIGPARAMS      2

#define ZPS_TLV_L_MINTAG               0
#define ZPS_TLV_L_MAXTAG              63

#define ZPS_TLV_G_SUPPKEYNEGMETH_STATKEYREQ  (1) /* Zigbee 3.0 Mechanism */
#define ZPS_TLV_G_SUPPKEYNEGMETH_SPEKEAES128 (2) /* SPEKE using Curve25519 with Hash AES-MMO-128 */
#define ZPS_TLV_G_SUPPKEYNEGMETH_SPEKESHA256 (4) /* SPEKE using Curve25519 with Hash SHA-256 */

#define ZPS_TLV_G_SELECTKEYNEGMETH_STATKEYREQ  (0) /* Zigbee 3.0 Mechanism */
#define ZPS_TLV_G_SELECTKEYNEGMETH_SPEKEAES128 (1) /* SPEKE using Curve25519 with Hash AES-MMO-128 */
#define ZPS_TLV_G_SELECTKEYNEGMETH_SPEKESHA256 (2) /* SPEKE using Curve25519 with Hash SHA-256 */

#define ZPS_TLV_G_PSK_SYMMETRIC    (1) /* Symmetric authentication token */
#define ZPS_TLV_G_PSK_INSTALLCODE  (2) /* Pre-configured link-ley derived from installation code */
#define ZPS_TLV_G_PSK_PASSCODE     (4) /* Variable-length pass code (for PAKE protocols) */
#define ZPS_TLV_G_PSK_BASICAUTH    (8) /* Basic Authorization Key */
#define ZPS_TLV_G_PSK_ADMINAUTH   (16) /* Administrative Authorization Key */
#define ZPS_TLV_G_PSK_WELLKNOWN  (255) /* Anonymous Well-Known Secret */

#define ZPS_TLV_G_ROUTERINFO_HUBCONN         (1) /* Hub Connectivity */
#define ZPS_TLV_G_ROUTERINFO_LONGUPTIME      (2) /* Uptime > 24 hrs */
#define ZPS_TLV_G_ROUTERINFO_PREFPARENT      (4) /* Preferred Parent */

#define ZPS_TLV_G_CONFIGPARAMS_ZDORESTRMODE  (1) /* apsZdoRestrictedMode */
#define ZPS_TLV_G_CONFIGPARAMS_LINKKEYTRANSPORTKEY  (2) /* requireLinkKeyEncryptionForApsTransportKey */
#define ZPS_TLV_G_CONFIGPARAMS_LEAVEREQALLOW (4) /* nwkLeaveRequestAllowed */

#define ZPS_TLV_ALLOW_ENCAPS_TLV_STRICT  (1) /* Does not allow other TLVs outside encapsulation */
#define ZPS_TLV_ALLOW_ENCAPS_TLV_OTHERS  (2) /* Allow other TLVs outside encapsulation */

#define ZPS_TLV_G_POTENTIAL_PARENTS_MAX  (5) /* Maximum number of entries in the TLV */

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum
{
    ZPS_TLV_ENUM_SUCCESS,             /**< Success (0x00) */
    ZPS_TLV_ENUM_INVALIDTLV,          /**< Malformed TLV, truncated value */
    ZPS_TLV_ENUM_DUPLICATED,          /**< Duplicated TLV */
} ZPS_teTlvEnum;

typedef enum
{
    ZPS_TLV_G_MANUFSPEC = 64,
    ZPS_TLV_G_SUPPKEYNEGMETH = 65,
    ZPS_TLV_G_PANIDCONFLREP = 66,
    ZPS_TLV_G_PANIDNEXT = 67,
    ZPS_TLV_G_CHANNEXT = 68,
    ZPS_TLV_G_SYMPASSPHRASE = 69,
    ZPS_TLV_G_ROUTERINFO = 70,
    ZPS_TLV_G_FRAGPARAMS = 71,
    ZPS_TLV_G_JOINERENCAPS = 72,
    ZPS_TLV_G_BEACONAPPENCAPS = 73,
    /* 74 Reserved */
    ZPS_TLV_G_CONFIGPARAMS = 75,
    ZPS_TLV_G_LASTTAG
} ZPS_teTlvGlobalTag;

typedef struct
{
    uint16 u6CntItems:6;       /* Number of items in the TLV list below */
    uint16 u1NoGlobTlv:1;
    uint16 u1NoLocalTlv:1;
    uint16 u1AllowEncapsTlv:2; /* 0: no encaps; 1: encaps only; 10: encaps + other TLVs */
    uint16 u6CntEncapsItems:5; /* how may TLVs in the list are required by the encapsulation */
    struct
    {
        uint8 u8Tag;
        uint8 u8MinLen;
    } asTlvList[];
} tsTlvDescr;

#define TLV_HEADER_PREFIX \
        uint8 u8Tag; \
        uint8 u8Len;

#define TLV_PARAM_NUL_BUILD_TYPE
#define TLV_DEF_SPEC_1P(type, item, ...) type item;
#define TLV_DEF_SPEC_2P(type, item, ...) type item; TLV_DEF_SPEC_1P(__VA_ARGS__, TLV_PARAM_NUL_BUILD_TYPE, TLV_PARAM_NUL_BUILD_TYPE)
#define TLV_DEF_SPEC_3P(type, item, ...) type item; TLV_DEF_SPEC_2P(__VA_ARGS__, TLV_PARAM_NUL_BUILD_TYPE, TLV_PARAM_NUL_BUILD_TYPE)
#define TLV_DEF_SPEC_4P(type, item, ...) type item; TLV_DEF_SPEC_3P(__VA_ARGS__, TLV_PARAM_NUL_BUILD_TYPE, TLV_PARAM_NUL_BUILD_TYPE)
#define TLV_DEF_SPEC_5P(type, item, ...) type item; TLV_DEF_SPEC_4P(__VA_ARGS__, TLV_PARAM_NUL_BUILD_TYPE, TLV_PARAM_NUL_BUILD_TYPE)
#define TLV_DEF_SPEC_6P(type, item, ...) type item; TLV_DEF_SPEC_5P(__VA_ARGS__, TLV_PARAM_NUL_BUILD_TYPE, TLV_PARAM_NUL_BUILD_TYPE)
#define TLV_DEF_SPEC_7P(type, item, ...) type item; TLV_DEF_SPEC_6P(__VA_ARGS__, TLV_PARAM_NUL_BUILD_TYPE, TLV_PARAM_NUL_BUILD_TYPE)
#define TLV_DEF_SPEC_8P(type, item, ...) type item; TLV_DEF_SPEC_7P(__VA_ARGS__, TLV_PARAM_NUL_BUILD_TYPE, TLV_PARAM_NUL_BUILD_TYPE)

#define TLV_DEF(name, ...) \
typedef union \
{ \
    tsTlvGeneric sTlv; \
    struct __attribute__((__packed__)) \
    { \
        TLV_HEADER_PREFIX; \
        TLV_DEF_SPEC_8P(__VA_ARGS__, TLV_PARAM_NUL_BUILD_TYPE, TLV_PARAM_NUL_BUILD_TYPE) \
    }; \
} __attribute__((__packed__)) name

#define TLV_ENCAPS_SPEC_1P(pref, type, ...) type pref ## type;
#define TLV_ENCAPS_SPEC_2P(pref, type, ...) type pref ## type; TLV_ENCAPS_SPEC_1P(__VA_ARGS__, TLV_PARAM_NUL_BUILD_TYPE, TLV_PARAM_NUL_BUILD_TYPE)
#define TLV_ENCAPS_SPEC_3P(pref, type, ...) type pref ## type; TLV_ENCAPS_SPEC_2P(__VA_ARGS__, TLV_PARAM_NUL_BUILD_TYPE, TLV_PARAM_NUL_BUILD_TYPE)
#define TLV_ENCAPS_SPEC_4P(pref, type, ...) type pref ## type; TLV_ENCAPS_SPEC_3P(__VA_ARGS__, TLV_PARAM_NUL_BUILD_TYPE, TLV_PARAM_NUL_BUILD_TYPE)

#define TLV_ENCAPS(name, size, ...) \
union \
{ \
    struct { \
        uint8 au8Hdr[ZPS_TLV_HDR_SIZE]; \
        uint8 au8Storage[size]; \
    }; \
    struct __attribute__((__packed__)) \
    { \
        TLV_HEADER_PREFIX; \
        TLV_ENCAPS_SPEC_4P(__VA_ARGS__, TLV_PARAM_NUL_BUILD_TYPE, TLV_PARAM_NUL_BUILD_TYPE) \
    }; \
} __attribute__((__packed__)) name

/* 1.2.3 Fields that are longer than a single octet are sent to the PHY in order
 * from the octet containing the lowest numbered bits to the octet containing
 * the highest numbered bits. The multi-byte fields using TLV_DEF below, e.g.
 * uint16, are written on this SoC as little endian, thus in proper TLV order.
 */
#define TLV_MANUFACTURERSPECIFIC_PTR(classifier, prefix, name, man_id) \
static union { \
    tuTlvManufacturerSpecific sTlv; \
    uint8 u8Buf[sizeof(tuTlvManufacturerSpecific)] \
            __attribute__ ((aligned (__alignof__(tuTlvManufacturerSpecific)))); \
} au8Storage_##name = { \
    .u8Buf = { \
        [offsetof(tuTlvManufacturerSpecific, u8Tag)] = ZPS_TLV_G_MANUFSPEC, \
        [offsetof(tuTlvManufacturerSpecific, u8Len)] = sizeof(au8Storage_##name.u8Buf) - 1 - ZPS_TLV_HDR_SIZE, \
        [offsetof(tuTlvManufacturerSpecific, u16ZigbeeManufId)    ] = (uint8)((man_id) & 0xFF), \
        [offsetof(tuTlvManufacturerSpecific, u16ZigbeeManufId) + 1] = (uint8)((man_id) >> 8), \
} }; \
tuTlvManufacturerSpecific classifier *prefix##name = &au8Storage_##name.sTlv;

#define TLV_MANUFACTURERSPECIFIC_EX_PTR(classifier, prefix, name, man_id, extra_bytes, first_byte, ...) \
static union { \
    tuTlvManufacturerSpecific sTlv; \
    uint8 u8Buf[sizeof(tuTlvManufacturerSpecific) + extra_bytes] \
            __attribute__ ((aligned (__alignof__(tuTlvManufacturerSpecific)))); \
} au8Storage_##name = { \
    .u8Buf = { \
        [offsetof(tuTlvManufacturerSpecific, u8Tag)] = ZPS_TLV_G_MANUFSPEC, \
        [offsetof(tuTlvManufacturerSpecific, u8Len)] = sizeof(au8Storage_##name.u8Buf) - 1 - ZPS_TLV_HDR_SIZE, \
        [offsetof(tuTlvManufacturerSpecific, u16ZigbeeManufId)    ] = (uint8)((man_id) & 0xFF), \
        [offsetof(tuTlvManufacturerSpecific, u16ZigbeeManufId) + 1] = (uint8)((man_id) >> 8), \
        [offsetof(tuTlvManufacturerSpecific, au8Extra)            ] = first_byte, \
        __VA_ARGS__ } \
}; \
tuTlvManufacturerSpecific classifier *prefix##name = &au8Storage_##name.sTlv;

#define TLV_CLEARALLBINDINGSREQ_PTR(classifier, prefix, name, count, first_eui64, ...) \
static union { \
    tuClearAllBindingsReq sTlv; \
    uint8 u8Buf[sizeof(tuClearAllBindingsReq) + count * sizeof(uint64)] \
            __attribute__ ((aligned (__alignof__(tuClearAllBindingsReq)))); \
} au8Storage_##name = { \
    .u8Buf = { \
        [offsetof(tuClearAllBindingsReq, u8Tag)] = 0, \
        [offsetof(tuClearAllBindingsReq, u8Len)] = sizeof(au8Storage_##name.u8Buf) - 1 - ZPS_TLV_HDR_SIZE, \
        [offsetof(tuClearAllBindingsReq, u8Count)  ] = (uint8)(count), \
        [offsetof(tuClearAllBindingsReq, au64Eui64)] = first_eui64, \
        __VA_ARGS__ } \
}; \
tuClearAllBindingsReq classifier *prefix##name = &au8Storage_##name.sTlv;

#define TLV_PROCESSINGSTATUSRSP_PTR(classifier, prefix, name, count, first_byte, ...) \
static union { \
    tuProcessingStatusRsp sTlv; \
    uint8 u8Buf[sizeof(tuProcessingStatusRsp) + count * sizeof(uint16)] \
            __attribute__ ((aligned (__alignof__(tuProcessingStatusRsp)))); \
} au8Storage_##name = { \
    .u8Buf = { \
        [offsetof(tuProcessingStatusRsp, u8Tag)] = 0, \
        [offsetof(tuProcessingStatusRsp, u8Len)] = sizeof(au8Storage_##name.u8Buf) - 1 - ZPS_TLV_HDR_SIZE, \
        [offsetof(tuProcessingStatusRsp, u8Count)  ] = (uint8)(count), \
        [offsetof(tuProcessingStatusRsp, asRsp)] = first_byte, \
        __VA_ARGS__ } \
}; \
tuProcessingStatusRsp classifier *prefix##name = &au8Storage_##name.sTlv;

#define TLV_DEVICEEUI64LIST_PTR(classifier, prefix, name, count, first_eui64, ...) \
static union { \
    tuDeviceEUI64List sTlv; \
    uint8 u8Buf[sizeof(tuDeviceEUI64List) + count * sizeof(uint64)] \
            __attribute__ ((aligned (__alignof__(tuDeviceEUI64List)))); \
} au8Storage_##name = { \
    .u8Buf = { \
        [offsetof(tuDeviceEUI64List, u8Tag)] = 0, \
        [offsetof(tuDeviceEUI64List, u8Len)] = sizeof(au8Storage_##name.u8Buf) - 1 - ZPS_TLV_HDR_SIZE, \
        [offsetof(tuDeviceEUI64List, u8Count)  ] = (uint8)(count), \
        [offsetof(tuDeviceEUI64List, au64Eui64)] = first_eui64, \
        __VA_ARGS__ } \
}; \
tuDeviceEUI64List classifier *prefix##name = &au8Storage_##name.sTlv;

#define TLV_USERDEFINED_PTR(classifier, prefix, name, tag, length, first_byte, ...) \
static uint8 au8Storage_##name[ZPS_TLV_HDR_SIZE + length * sizeof(uint8)] = \
{ \
    [offsetof(tsTlvGeneric, u8Tag)] = tag, \
    [offsetof(tsTlvGeneric, u8Len)] = length - 1, \
    [offsetof(tsTlvGeneric, au8Value)] = first_byte, \
    __VA_ARGS__ \
}; \
tsTlvGeneric classifier *prefix##name = (tsTlvGeneric *)&au8Storage_##name;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpacked"
#pragma GCC diagnostic ignored "-Wattributes"
/* Disable warnings given by packed and attributes for the inefficient structure */

typedef struct __attribute__((__packed__))
{
    TLV_HEADER_PREFIX;
    uint8 au8Value[];
} tsTlvGeneric;
_Static_assert(sizeof(tsTlvGeneric) == 2, "TLV type error");

TLV_DEF(tuTlvManufacturerSpecific,
        uint16, u16ZigbeeManufId,
        uint8, au8Extra[]
);
_Static_assert(sizeof(tuTlvManufacturerSpecific) == 4, "TLV type error");

TLV_DEF(tuSupportedKeyNegotiationMethods,
        uint8, u8KeyNegotProtMask,
        uint8, u8SharedSecretsMask,
        uint8, au8SrcIeeeAddr[8]
);
_Static_assert(sizeof(tuSupportedKeyNegotiationMethods) == 12, "TLV type error");

TLV_DEF(tuSelectedKeyNegotiationMethod,
        uint8, u8KeyNegotiationProtocol,
        uint8, u8PresharedSecrets,
        uint8, au8SrcIeeeAddr[8]
);
_Static_assert(sizeof(tuSelectedKeyNegotiationMethod) == 12, "TLV type error");

TLV_DEF(tuCurve25519PublicPoint,
        uint64, u64DeviceEui64,
        uint8, au8PublicPoint[32]
);
_Static_assert(sizeof(tuCurve25519PublicPoint) == 42, "TLV type error");

TLV_DEF(tuSupportedKeyNegotiationMethodsNoAddr,
        uint8, u8KeyNegotProtMask,
        uint8, u8SharedSecretsMask
);
_Static_assert(sizeof(tuSupportedKeyNegotiationMethodsNoAddr) == 4, "TLV type error");

TLV_DEF(tuAuthTokenId,
        uint8, u8TlvTypeTagId
);
_Static_assert(sizeof(tuAuthTokenId) == 3, "TLV type error");

TLV_DEF(tuPanidConflictReport,
        uint16, u16PanidConflictCount
);
_Static_assert(sizeof(tuPanidConflictReport) == 4, "TLV type error");

TLV_DEF(tuNextPanidChange,
        uint16, u16NextPanid
);
_Static_assert(sizeof(tuNextPanidChange) == 4, "TLV type error");

// TODO: bitmap definition 3.2.2.2.1
TLV_DEF(tuNextChannelChange,
        uint32, u32NextChannel
);
_Static_assert(sizeof(tuNextChannelChange) == 6, "TLV type error");

TLV_DEF(tuSymPassphrase,
        uint8, au8SymPassphrase[16]
);
_Static_assert(sizeof(tuSymPassphrase) == 18, "TLV type error");

TLV_DEF(tuRouterInfo,
        uint16, u16BmpRouterInfo
);
_Static_assert(sizeof(tuRouterInfo) == 4, "TLV type error");

TLV_DEF(tuFragParams,
        uint16, u16NodeId,
        uint8, u8FragOpt,
        uint16, u16InMaxLen
);
_Static_assert(sizeof(tuFragParams) == 7, "TLV type error");

// tuJoinerEncaps and tuBeaconAppendEncaps are of type tsTlvGeneric

TLV_DEF(tuConfigParams,
        uint16, u16BmpConfig
);
_Static_assert(sizeof(tuConfigParams) == 4, "TLV type error");

TLV_DEF(tuClearAllBindingsReq,
        uint8, u8Count,
        uint64, au64Eui64[]
);
_Static_assert(sizeof(tuClearAllBindingsReq) == 3, "TLV type error");

TLV_DEF(tuBeaconSurveyResults,
        uint8, u8TotalBeaconsRx,
        uint8, u8OnNetworkBeacons,
        uint8, u8PotentialParentBeacons,
        uint8, u8OtherNetworkBeacons,
);
_Static_assert(sizeof(tuBeaconSurveyResults) == 6, "TLV type error");

TLV_DEF(tuPotentialParents,
        uint16, u16ParentAddr,
        uint8,  u8ParentLqa,
        uint8,  u8CntOtherParents,
        struct __attribute__((__packed__)) { uint16 u16Addr; uint8  u8Lqa; }, asBeacons[ZPS_TLV_G_POTENTIAL_PARENTS_MAX]
);
_Static_assert(sizeof(tuPotentialParents) == (6 + ZPS_TLV_G_POTENTIAL_PARENTS_MAX * 3), "TLV type error");

TLV_DEF(tuProcessingStatusRsp,
        uint8, u8Count,
        struct { uint8 u8TagId; uint8 u8Status; }, asRsp[]
);
_Static_assert(sizeof(tuProcessingStatusRsp) == 3, "TLV type error");

TLV_DEF(tuTargetIEEEAdress,
        uint64, u64IEEEAddrOfInterest
);
_Static_assert(sizeof(tuTargetIEEEAdress) == 10, "TLV type error");

TLV_DEF(tuDeviceAuthLevel,
        uint64, u64IEEEAddrRemoteNode,
        uint8,  u8InitialJoinMethod,
        uint8,  u8ActiveLinkKeyType
);
_Static_assert(sizeof(tuDeviceAuthLevel) == 12, "TLV type error");

TLV_DEF(tuDeviceEUI64List,
        uint8, u8Count,
        uint64, au64Eui64[]
);
_Static_assert(sizeof(tuDeviceEUI64List) == 3, "TLV type error");

TLV_DEF(tuApsFrameCounterChallenge,
        uint64, u64Eui64,
        uint64, u64ChallengeValue
);
_Static_assert(sizeof(tuApsFrameCounterChallenge) == 18, "TLV type error");

TLV_DEF(tuApsFrameCounterResponse,
        uint64, u64Eui64,
        uint64, u64RecChallengeValue,
        uint32, u32ApsFrameCounter,
        uint32, u32ChallengeSecFrameCounter,
        uint64, u64MIC
);
_Static_assert(sizeof(tuApsFrameCounterResponse) == 34, "TLV type error");

#pragma GCC diagnostic pop


typedef enum
{
    ZPS_TLV_STACK_CTX = 0,
#if ZPS_TLV_CONTEXT_COUNT > 1
    ZPS_TLV_STACK_APP = 1,
#if ZPS_TLV_CONTEXT_COUNT > 2
    ZPS_TLV_STACK_APP_END = ZPS_TLV_CONTEXT_COUNT - 1,
#endif
#endif
} tsTlvCtx;

/****************************************************************************
 *
 * NAME:       tpfParseTLVContent
 */
/**
 * Parse the content of a TLV passed as parameter
 *
 * @ingroup tlv
 *
 * @param u8Tag      TLV ID
 * @param u8Len      Length of the value field (0 is 1 byte)
 * @param pu8Payload TLV's payload
 *
 * @note There is only one context for the parser which must be used in the
 *       subsequent TLV operations
 *
 ****************************************************************************/
typedef ZPS_teTlvEnum (*tpfParseTLVContent)(uint8 u8Tag, uint8 u8Len,
                                            uint8 *pu8Payload);

/****************************/
/**** EXPORTED VARIABLES ****/
/****************************/

#if 0
extern tsTlvDescr g_Tlv_ManufacturerSpecific;
extern tsTlvDescr g_Tlv_NextPanidChange;
extern tsTlvDescr g_Tlv_NextChannelChange;
extern tsTlvDescr g_Tlv_FragParams;
#endif

extern tsTlvDescr g_Tlv_PanidConflictReport;
extern tsTlvDescr g_Tlv_RouterInfo;
extern tsTlvDescr g_Tlv_BeaconAppendix;
extern tsTlvDescr g_Tlv_MgmtPermitJoiningReq;
extern tsTlvDescr g_Tlv_NwkCommissionReq;
extern tsTlvDescr g_Tlv_EndDeviceTimeoutReq;
extern tsTlvDescr g_Tlv_NodeDescReq;
extern tsTlvDescr g_Tlv_NodeDescRsp;
extern tsTlvDescr g_Tlv_ClearAllBindingsReq;
extern tsTlvDescr g_Tlv_BeaconSurveyConfig;
extern tsTlvDescr g_Tlv_BeaconSurveyRsp;
extern tsTlvDescr g_Tlv_SecStartKeyNegotiationReq;
extern tsTlvDescr g_Tlv_SecStartKeyNegotiationRsp;
extern tsTlvDescr g_Tlv_SecRetrieveAuthTokenReq;
extern tsTlvDescr g_Tlv_SecRetrieveAuthTokenRsp;
extern tsTlvDescr g_Tlv_SecGetAuthLvlReq;
extern tsTlvDescr g_Tlv_SecGetAuthLvlRsp;
extern tsTlvDescr g_Tlv_SecSetConfigReq;
extern tsTlvDescr g_Tlv_SecSetConfigRsp;
extern tsTlvDescr g_Tlv_RelayMsgDown;
extern tsTlvDescr g_Tlv_RelayMsgUp;
extern tsTlvDescr g_Tlv_RouteReply;
extern tsTlvDescr g_Tlv_NetworkStatus;
extern tsTlvDescr g_Tlv_SecStartKeyUpdateReq;
extern tsTlvDescr g_Tlv_SecDecommReq;
extern tsTlvDescr g_Tlv_SecChallengeReq;
extern tsTlvDescr g_Tlv_ApsFrameCounterRsp;

/****************************/
/**** EXPORTED FUNCTIONS ****/
/****************************/

/****************************************************************************
 *
 * NAME:       ZPS_eTlvParseValidate
 */
/**
 * Parse and Validate a buffer where TLVs are expected to be found.
 *
 * @ingroup tlv
 *
 * @param ctx - handler to the parser context
 * @param pTlvScope - configuration of the parsing process, depending on the context
 * @param pu8Payload - pointer to the buffer containing the list of TLVs
 * @param iSize - size of the buffer
 *
 * @return - status of the parsing operation
 *
 * @note There is only one context for the parser which must be used in the
 *       subsequent TLV operations
 *
 ****************************************************************************/
PUBLIC ZPS_teTlvEnum ZPS_eTlvParseValidate(tsTlvCtx ctx, tsTlvDescr *pTlvScope,
                                           uint8 *pu8Payload, int iSize);

/****************************************************************************
 *
 * NAME:       ZPS_eTlvGetNextItem
 */
/**
 * Parse the content of a TLV passed as parameter
 *
 * @ingroup tlv
 *
 * @param ctx - handler to the parser context
 * @param pfHandleTlv TLV parser's contextual data
 *
 * @return operation status
 *
 * @note There is only one instance of this parser
 *
 ****************************************************************************/
PUBLIC ZPS_teTlvEnum ZPS_eTlvGetNextItem(tsTlvCtx ctx, tpfParseTLVContent pfHandleTlv);

/****************************************************************************
 *
 * NAME:       ZPS_iTlvGetOffset
 */
/**
 *
 *
 * @ingroup tlv
 *
 * @param ctx - handler to the parser context
 * @param u8Tag - Find the tag in the previously parsed TLV context
 *
 * @return - if >=0, byte offset in the buffer where TLV is found
 *
 * @note
 *
 ****************************************************************************/
PUBLIC int ZPS_iTlvGetOffset(tsTlvCtx ctx, uint8 u8Tag);

/****************************************************************************
 *
 * NAME:       ZPS_vTlvBuildSequence
 */
/**
 *
 *
 * @ingroup tlv
 *
 * @param u8Cnt - Number of TLVs that will be stored in the buffer
 * @param u8MaxLen - Maximum length to be used in the buffer
 * @param pvBuf - The buffer
 * @param ... - List of pointers to TLVs of type *tsTlvGeneric
 *
 * @return - none
 *
 * @note
 *
 ****************************************************************************/
PUBLIC void ZPS_vTlvBuildSequence(uint8 u8Cnt, uint8 u8MaxLen, void *pvBuf, ...);

#if 0
/****************************************************************************
 *
 * NAME:       zps_u8TlvInsertItem
 */
/**
 * Add TLV into a payload
 *
 * @ingroup tlv
 *
 * @param u8Tag      TLV ID
 * @param pu8Payload TLV's payload
 * @param iSize      Length of the payload
 *
 * @return number of bytes written into payload
 *
 ****************************************************************************/
PUBLIC uint8 ZPS_u8TlvInsertItem(uint8 *pu8Payload, int iSize);
#endif

#endif /* TLV_H_ */
