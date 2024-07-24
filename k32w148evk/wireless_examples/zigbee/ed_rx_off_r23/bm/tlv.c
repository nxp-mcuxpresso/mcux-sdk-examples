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


 /*
 * MODULE:      Utils
 *
 * COMPONENT:   tlv.c
 *
 * DESCRIPTION: Support for parsing, generating TLVs as defined by R23
 *
 *****************************************************************************/

#include "tlv.h"
#include <string.h>
#include <stdarg.h>
#include "dbg.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#define ZPS_TLV_MAX_ITEMS   (128)
#ifndef TRACE_TLV
#define TRACE_TLV FALSE
#endif

/****************************************************************************/
/***        Global Variables                                               ***/
/****************************************************************************/

#if 0
tsTlvDescr g_GlobTlv_ManufacturerSpecific =
{
    1,       /* One TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,
    {{ZPS_TLV_G_MANUFSPEC, ZPS_TLVLEN_G_MANUFSPEC}}
};

tsTlvDescr g_GlobTlv_SupportedKeyNegotiationMethods =
{
    1,       /* One TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,
    {{ZPS_TLV_G_SUPPKEYNEGMETH, ZPS_TLVLEN_G_SUPPKEYNEGMETH}}
};

tsTlvDescr g_GlobTlv_NextPanidChange =
{
    1,       /* One TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,
    {{ZPS_TLV_G_PANIDNEXT, ZPS_TLVLEN_G_PANIDNEXT}}
};

tsTlvDescr g_GlobTlv_NextChannelChange =
{
    1,       /* One TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,
    {{ZPS_TLV_G_CHANNEXT, ZPS_TLVLEN_G_CHANNEXT}}
};

tsTlvDescr g_GlobTlv_SymPass =
{
    1,       /* One TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,
    {{ZPS_TLV_G_SYMPASS, ZPS_TLVLEN_G_SYMPASS}}
};

tsTlvDescr g_GlobTlv_FragParams =
{
    1,       /* One TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,
    {{ZPS_TLV_G_FRAGPARAMS, ZPS_TLVLEN_G_FRAGPARAMS}}
};
#endif


tsTlvDescr g_Tlv_PanidConflictReport =
{
    1,       /* One TLV defined for this command */
    0,       /* Global TLVs are not illegal */
    1,       /* Local TLVs are illegal */
    0,
    0,
    {{ZPS_TLV_G_PANIDCONFLREP, ZPS_TLVLEN_G_PANIDCONFLREP}}
};

tsTlvDescr g_Tlv_RouterInfo =
{
    1,       /* At least 1 TLV defined for this command */
    0,       /* Global TLVs are not illegal */
    0,       /* Local TLVs are not illegal */
    0,
    0,
    {{ZPS_TLV_G_ROUTERINFO, ZPS_TLVLEN_G_ROUTERINFO}} /* Std 3.6.8.1 */
};

tsTlvDescr g_Tlv_BeaconAppendix =
{
    3,       /* At least 3 TLV defined for this command */
    0,       /* Global TLVs are not illegal */
    0,       /* Local TLVs are not illegal */
    0,
    0,
    {{ZPS_TLV_G_SUPPKEYNEGMETH, ZPS_TLVLEN_G_SUPPKEYNEGMETH}, /* 2.4.3.3.7.1 */
     {ZPS_TLV_G_FRAGPARAMS, ZPS_TLVLEN_G_FRAGPARAMS}, /* 2.4.3.3.7.1 */
     {ZPS_TLV_G_ROUTERINFO, ZPS_TLVLEN_G_ROUTERINFO}} /* Std 3.6.8.1 */
};

tsTlvDescr g_Tlv_MgmtPermitJoiningReq =
{
    2,       /* At least 2 TLV defined for this command */
    0,       /* Global TLVs are not illegal */
    0,       /* Local TLVs are not illegal */
	ZPS_TLV_ALLOW_ENCAPS_TLV_OTHERS,
    2,       /* The below required TLVs are all part of encaps */
    {{ZPS_TLV_G_SUPPKEYNEGMETH, ZPS_TLVLEN_G_SUPPKEYNEGMETH}, /* 2.4.3.3.7.1 */
     {ZPS_TLV_G_FRAGPARAMS, ZPS_TLVLEN_G_FRAGPARAMS}} /* 2.4.3.3.7.1 */
};

tsTlvDescr g_Tlv_NwkCommissionReq =
{
    2,       /* At least 2 TLV defined for this command */
    0,       /* Global TLVs are not illegal */
    1,       /* Local TLVs are illegal */
	ZPS_TLV_ALLOW_ENCAPS_TLV_OTHERS,
    2,       /* The below required TLVs are all part of encaps */
    {{ZPS_TLV_G_SUPPKEYNEGMETH, ZPS_TLVLEN_G_SUPPKEYNEGMETH}, /* 2.4.3.3.7.1 */
     {ZPS_TLV_G_FRAGPARAMS, ZPS_TLVLEN_G_FRAGPARAMS}} /* 2.4.3.3.7.1 */
};

tsTlvDescr g_Tlv_EndDeviceTimeoutReq =
{
    1,       /* One TLV defined for this command */
    0,       /* Global TLVs are not illegal */
    1,       /* Local TLVs are illegal */
    0,
    0,
    {{ZPS_TLV_G_MANUFSPEC, ZPS_TLVLEN_G_MANUFSPEC}}
};

tsTlvDescr g_Tlv_NodeDescReq =
{
    1,       /* One TLV minimum defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    1,       /* Local TLVs are illegal */
    0,
    0,
    {{ZPS_TLV_G_FRAGPARAMS, ZPS_TLVLEN_G_FRAGPARAMS}} /* 2.4.3.1.3.1 */
};

tsTlvDescr g_Tlv_NodeDescRsp =
{
    2,       /* One TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,       /* Local TLVs are not illegal */
    0,
    0,
    {{ZPS_TLV_G_FRAGPARAMS, ZPS_TLVLEN_G_FRAGPARAMS}, /* 2.4.4.2.3.1 */
    {0, 10}} /* Selected Key Negotiation Method ID = 0, min len 10 B */
};

tsTlvDescr g_Tlv_BeaconSurveyConfig =
{
    1,       /* One TLV minimum defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,       /* Local TLVs not are illegal */
    0,
    0,
    {{0, 1}} /* ID = 0, min len varies */
};

tsTlvDescr g_Tlv_BeaconSurveyRsp =
{
    2,       /* Two local TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,       /* Local TLVs not are illegal */
    0,
    0,
    {{1, 4}, /* Beacon Survey Results ID = 1, min len 4 B */
    {2, 4}}  /* Potential Parents ID = 2, min len 4 B */
};

tsTlvDescr g_Tlv_ClearAllBindingsReq =
{
    1,       /* One TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,       /* Local TLVs are not illegal */
    0,
    0,
    {{0, 9}} /* Clear All Bindings ID = 0, min len of 9 bytes, what would mean count = 0 in a TLV? */
};

tsTlvDescr g_Tlv_SecSetConfigReq =
{
    3,       /* Three TLVs defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    1,       /* Local TLVs are illegal */
    0,
    0,
    {{ZPS_TLV_G_PANIDNEXT, ZPS_TLVLEN_G_PANIDNEXT}, /* 2.4.3.4.4 */
    {ZPS_TLV_G_CHANNEXT, ZPS_TLVLEN_G_CHANNEXT}, /* 2.4.3.4.4 */
    {ZPS_TLV_G_CONFIGPARAMS, ZPS_TLVLEN_G_CONFIGPARAMS}}, /* 2.4.3.4.4 */
};

tsTlvDescr g_Tlv_SecGetAuthLvlReq =
{
    1,       /* One TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,       /* Local TLVs are not illegal */
    0,
    0,
    {{0, 8}} /* Tgt IEEE Addr ID = 0, min len 8 B */
};

tsTlvDescr g_Tlv_SecDecommReq =
{
    1,       /* One TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,       /* Local TLVs are not illegal */
    0,
    0,
    {{0, 9}} /* Device EUI64 List ID = 0, min len 9 B */
};

tsTlvDescr g_Tlv_SecChallengeReq =
{
    1,       /* One TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,       /* Local TLVs are not illegal */
    0,
    0,
    {{0, 16}, /* APS Frame Counter Challenge ID = 0, min len 16 B */
     {0, 0}}, /* Empty Data */ 
};

tsTlvDescr g_Tlv_ApsFrameCounterRsp =
{
    1,       /* One TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,       /* Local TLVs are not illegal */
    0,
    0,
    {{0, 32}}/* APS Frame Counter Response ID = 0, min len 32 B */
};

tsTlvDescr g_Tlv_SecStartKeyUpdateReq =
{
    1,       /* One TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,       /* Local TLVs are not illegal */
    0,
    0,
    {{ZPS_TLV_G_FRAGPARAMS, ZPS_TLVLEN_G_FRAGPARAMS}, /* 2.4.4.2.3.1 */
    {0, 10}} /* Selected Key Negotiation Method ID = 0, min len 10 B */
};

tsTlvDescr g_Tlv_SecStartKeyNegotiationReq =
{
    1,       /* One TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,       /* Local TLVs are not illegal */
    0,
    0,
    {{0, 40}}/* Curve25519 Public Point ID = 0, min len 40 B */
};

tsTlvDescr g_Tlv_SecStartKeyNegotiationRsp =
{
    1,       /* One TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,       /* Local TLVs are not illegal */
    0,
    0,
    {{0, 40}}/* Curve25519 Public Point ID = 0, min len 40 B */
};

#if 0
tsTlvDescr g_Tlv_SecRetrAuthTokenReq =
{
    1,       /* One TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,
    {{0, 1}} /* Auth Token ID = 0, min len 1 B */
};

tsTlvDescr g_Tlv_SecGetAuthLvlRsp =
{
    1,       /* One TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,
    {{0, 10}}/* Device Auth Level ID = 0, min len 10 B */
};

tsTlvDescr g_Tlv_SecSetConfigRsp =
{
    1,       /* One TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,
    {{0, 1}} /* Processing Status ID = 0, min len 1 B */
};

tsTlvDescr g_Tlv_RelayMsgDown =
{
    1,       /* One TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,
    {{0, 8}} /* Relay Message ID = 0, min len 8 B */
};

tsTlvDescr g_Tlv_RelayMsgUp =
{
    1,       /* One TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,
    {{0, 8}} /* Relay Message ID = 0, min len 8 B */
};

tsTlvDescr g_Tlv_RouteReply =
{
    1,       /* One TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,
    {{0, 2}} /* Ext Route Info ID = 0, min len 2 B */
};

tsTlvDescr g_Tlv_NetworkStatus =
{
    1,       /* One TLV defined for this command */
    0,       /* Do not discard payload if unexpected TLV is found */
    0,
    {{0, 2}} /* Ext Route Info ID = 0, min len 2 B */
};

#endif

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

static const uint8 asTlvMinLen[ZPS_TLV_G_LASTTAG - ZPS_TLV_L_MAXTAG - 1] =
{
    ZPS_TLVLEN_G_MANUFSPEC,
    ZPS_TLVLEN_G_SUPPKEYNEGMETH,
    ZPS_TLVLEN_G_PANIDCONFLREP,
    ZPS_TLVLEN_G_PANIDNEXT,
    ZPS_TLVLEN_G_CHANNEXT,
    ZPS_TLVLEN_G_SYMPASS,
    ZPS_TLVLEN_G_ROUTERINFO,
    ZPS_TLVLEN_G_FRAGPARAMS,
    /* remaining items are 0 */
};

static struct
{
    uint8 u8NoTlvs;
    struct
    {
        uint8 u8Tag;
        uint8 u8Len;
    } asTlvData[ZPS_TLV_MAX_ITEMS];
} tsTlvCtxInt[ZPS_TLV_CONTEXT_COUNT];

PRIVATE ZPS_teTlvEnum core_eTlvParseValidate(tsTlvCtx ctx, tsTlvDescr *pTlvScope, uint8 *pu8Payload, int iSize);
PUBLIC ZPS_teTlvEnum ZPS_eTlvParseValidate(tsTlvCtx ctx, tsTlvDescr *pTlvScope, uint8 *pu8Payload, int iSize)
{
    tsTlvCtxInt[ctx].u8NoTlvs = 0;
    return core_eTlvParseValidate(ctx, pTlvScope, pu8Payload, iSize);
}

PRIVATE ZPS_teTlvEnum core_eTlvParseValidate(tsTlvCtx ctx, tsTlvDescr *pTlvScope, uint8 *pu8Payload, int iSize)
{
    uint8 u8Tag, u8Len;
    int i;

    while (iSize >= ZPS_TLV_HDR_SIZE)
    {
        u8Tag = *pu8Payload++;
        u8Len = *pu8Payload++ + 1;

        if (u8Len == 0)
        {
            /* Wrap-around, drop any processing */
            return ZPS_TLV_ENUM_INVALIDTLV;
        }

        iSize -= ZPS_TLV_HDR_SIZE;
        DBG_vPrintf(TRACE_TLV, "TLV Parse tag %d len %d, remaining %d\n", u8Tag, u8Len, iSize);

        bool bFound = FALSE;

        if (iSize < u8Len)
        {
            DBG_vPrintf(TRACE_TLV, "Not enough space in the buffer\n");
            /* Not enough room for the value field; abort the whole payload */
            return ZPS_TLV_ENUM_INVALIDTLV;
        }

        if (pTlvScope && (u8Tag == ZPS_TLV_G_BEACONAPPENCAPS || u8Tag == ZPS_TLV_G_JOINERENCAPS)
            && !pTlvScope->u1AllowEncapsTlv)
        {
            DBG_vPrintf(TRACE_TLV, "Encaps found while not permitted\n");
            return ZPS_TLV_ENUM_INVALIDTLV;
        }
        else if (pTlvScope && (u8Tag == ZPS_TLV_G_BEACONAPPENCAPS || u8Tag == ZPS_TLV_G_JOINERENCAPS)
            && pTlvScope->u1AllowEncapsTlv == ZPS_TLV_ALLOW_ENCAPS_TLV_STRICT
            && iSize > u8Len)
        {
            /* Should not be any other TLVs after the encaps */
            return ZPS_TLV_ENUM_INVALIDTLV;
        }
        else if ((u8Tag > ZPS_TLV_L_MAXTAG) && (u8Tag < ZPS_TLV_G_LASTTAG) &&
            (u8Len < asTlvMinLen[u8Tag - ZPS_TLV_L_MAXTAG - 1]))
        {
            DBG_vPrintf(TRACE_TLV, "Truncated TLV\n");
            /* truncated value field; abort the whole payload */
            return ZPS_TLV_ENUM_INVALIDTLV;
        }
        else if ((u8Tag <= ZPS_TLV_L_MAXTAG) && !pTlvScope)
        {
            /* Local TLV found without context */
            DBG_vPrintf(TRACE_TLV, "Found local TLV w/o context and not permitted\n");
            return ZPS_TLV_ENUM_INVALIDTLV;
        }
        else if ((u8Tag <= ZPS_TLV_L_MAXTAG) && pTlvScope)
        {
            /* Local TLV */
            if (pTlvScope->u1NoLocalTlv)
            {
                DBG_vPrintf(TRACE_TLV, "Found local TLV and not permitted\n");
                return ZPS_TLV_ENUM_INVALIDTLV;
            }

            for (i = 0; i < pTlvScope->u6CntItems; i++)
            {
                /* search all possible local TLVs defined for this scope */
                if (u8Tag == pTlvScope->asTlvList[i].u8Tag)
                {
                    bFound = TRUE;
                    if (u8Len < pTlvScope->asTlvList[i].u8MinLen)
                    {
                        DBG_vPrintf(TRACE_TLV, "Trucated TLV\n");
                        /* truncated value field; abort the whole payload */
                        return ZPS_TLV_ENUM_INVALIDTLV;
                    }
                }
            }
        }
        else if ((u8Tag > ZPS_TLV_L_MAXTAG) && pTlvScope)
        {
            /* The processing rules of a message MAY describe what to do when
             * one or more known Global TLVs are unexpectedly received */
            if (pTlvScope->u1NoGlobTlv)
            {
                DBG_vPrintf(TRACE_TLV, "Found global TLV and not permitted\n");
                /* truncated value field; abort the whole payload */
                return ZPS_TLV_ENUM_INVALIDTLV;
            }
        }

        if (!bFound)
        {
            /* do not check for duplicate? */
            //continue;
        }

        /* check for duplicate; should it be done for unknown tags? */
        for (i = 0; i < tsTlvCtxInt[ctx].u8NoTlvs; i++)
        {
            if (u8Tag != ZPS_TLV_G_MANUFSPEC && tsTlvCtxInt[ctx].asTlvData[i].u8Tag == u8Tag)
            {
                DBG_vPrintf(TRACE_TLV, "Duplicated TLV\n");
                /* Duplicated tag; abort the whole payload */
            	return ZPS_TLV_ENUM_DUPLICATED;
            }
        }

        if (tsTlvCtxInt[ctx].u8NoTlvs < ZPS_TLV_MAX_ITEMS)
        {
            DBG_vPrintf(TRACE_TLV, "Found TLV %d, len %d\n", u8Tag, u8Len);
            tsTlvCtxInt[ctx].asTlvData[tsTlvCtxInt[ctx].u8NoTlvs].u8Tag = u8Tag;
            if (u8Tag == ZPS_TLV_G_BEACONAPPENCAPS || u8Tag == ZPS_TLV_G_JOINERENCAPS)
            {
                tsTlvCtxInt[ctx].asTlvData[tsTlvCtxInt[ctx].u8NoTlvs].u8Len = 0;
            }
            else
            {
                tsTlvCtxInt[ctx].asTlvData[tsTlvCtxInt[ctx].u8NoTlvs].u8Len = u8Len;
            }
            tsTlvCtxInt[ctx].u8NoTlvs++;
        }

        if (pTlvScope && (u8Tag == ZPS_TLV_G_BEACONAPPENCAPS || u8Tag == ZPS_TLV_G_JOINERENCAPS)
            && pTlvScope->u1AllowEncapsTlv)
        {
            DBG_vPrintf(TRACE_TLV, "encaps recursive\n");
            ZPS_teTlvEnum eStatus;

            /* go inside encaps and prevent embedded encaps TLVs */
            tsTlvDescr sNewTlvScope = *pTlvScope;
            sNewTlvScope.u1AllowEncapsTlv = 0;
            sNewTlvScope.u6CntItems = pTlvScope->u6CntEncapsItems;
            sNewTlvScope.u6CntEncapsItems = 0;
            eStatus = core_eTlvParseValidate(ctx, &sNewTlvScope, pu8Payload, u8Len);
            if (eStatus != ZPS_TLV_ENUM_SUCCESS)
            {
                return eStatus;
            }
        } /* if (pTlvScope && (u8Tag == */

        iSize -= u8Len;
        pu8Payload += u8Len;
        DBG_vPrintf(TRACE_TLV, "Modify size to %d payload to + %d reading value %d\n", iSize, u8Len, *pu8Payload);
    }

    DBG_vPrintf(TRACE_TLV, "complete parse and valid ok\n");
    return ZPS_TLV_ENUM_SUCCESS;
}

PUBLIC ZPS_teTlvEnum ZPS_eTlvGetNextItem(tsTlvCtx ctx, tpfParseTLVContent pfHandleTlv)
{

    return ZPS_TLV_ENUM_SUCCESS;
}

PUBLIC int ZPS_iTlvGetOffset(tsTlvCtx ctx, uint8 u8Tag)
{
    uint8 u8Offset = 0;
    bool bFound = FALSE;
    int i;

    for (i = 0; i < tsTlvCtxInt[ctx].u8NoTlvs; i++)
    {
        if (tsTlvCtxInt[ctx].asTlvData[i].u8Tag == u8Tag)
        {
            bFound = TRUE;
            break;
        }

        /* It is the real length used to skip over the payload, not ZB's size-1 */
        u8Offset += tsTlvCtxInt[ctx].asTlvData[i].u8Len + ZPS_TLV_HDR_SIZE;
    }

    if (bFound)
    {
        DBG_vPrintf(TRACE_TLV, "got offset %d for TLV %d\n", u8Offset, u8Tag);
        return u8Offset;
    }
    else
    {
        return -1; /* error parsing, no tag found */
    }
}

PUBLIC void ZPS_vTlvBuildSequence(uint8 u8Cnt, uint8 u8MaxLen, void *pvBuf, ...)
{
    tsTlvGeneric *psTlv;
    va_list list;
    int i;

    if (u8Cnt == 0 || u8MaxLen < ZPS_TLV_HDR_SIZE || pvBuf == NULL)
    {
        return;
    }

    DBG_vPrintf(TRACE_TLV, "build max len %d to %p\n", u8MaxLen, pvBuf);
    /* We get other parameters on stack */
    va_start(list, pvBuf);
    for (i = 0; i < u8Cnt; i++)
    {
        psTlv = va_arg(list, void *);
        if (u8MaxLen < psTlv->u8Len + 1 + ZPS_TLV_HDR_SIZE)
        {
            DBG_vPrintf(TRACE_TLV, "Exceed %d when adding len %d\n", u8MaxLen, psTlv->u8Len + ZPS_TLV_HDR_SIZE);
            break;
        }
        DBG_vPrintf(TRACE_TLV, "copy len %d\n", psTlv->u8Len + 1 + ZPS_TLV_HDR_SIZE);
        memcpy(pvBuf, psTlv, psTlv->u8Len + 1 + ZPS_TLV_HDR_SIZE);
        pvBuf = (uint8 *)pvBuf + psTlv->u8Len + 1 + ZPS_TLV_HDR_SIZE;
        u8MaxLen -= psTlv->u8Len + 1 + ZPS_TLV_HDR_SIZE;
    }
    va_end(list);
}

PUBLIC uint8 ZPS_u8TlvInsertItem(uint8 *pu8Payload, int iSize)
{

    return ZPS_TLV_ENUM_SUCCESS;
}
