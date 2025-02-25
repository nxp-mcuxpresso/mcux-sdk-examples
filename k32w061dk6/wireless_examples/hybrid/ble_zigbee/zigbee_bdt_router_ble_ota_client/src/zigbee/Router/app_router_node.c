/*
* Copyright 2024 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
*/
#include "app.h"
#include "bdb_api.h"
#include "app_common.h"
#include "PDM.h"

#ifdef CLD_OTA
#include "app_ota_client.h"
#endif

#include "PDM_IDs.h"
#include "app_zcl_task.h"
#include "app_reporting.h"
#include "zps_gen.h"
#include "app_leds.h"
#include "app_buttons.h"
#include "dbg.h"
#include "app_main.h"
#include "MicroSpecific.h"
#include "app_router_node.h"
#include "zigbee_config.h"
#include "app_main.h"
#ifdef LNT_MODE_APP
#include "app_crypto.h"
#endif
#ifdef KPI_MODE_APP
#include "tp2.h"
#endif
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#define GREEN_POWER_ENDPOINT                   242
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifndef TRACE_APP
    #define TRACE_APP   TRUE
#endif

#ifndef TRACE_APP_EVENT
    #define TRACE_APP_EVENT   TRUE
#endif

#ifndef TRACE_APP_BDB
    #define TRACE_APP_BDB     TRUE
#endif

#define ASSOCIATION_ATTEMPTS   (5)

#ifdef R23_UPDATES
/* Uncomment this to enable DLK with AES-128 */
//#define R23_DLK_AES128_ENABLE 1
#endif
#if R23_DLK_AES128_ENABLE
#define R23_DLK_SHARED_SECRETS_MASK 0
#define R23_DLK_KEY_PROTO_NEGOTIATION_MASK ZPS_TLV_G_SUPPKEYNEGMETH_SPEKEAES128
#else
#define R23_DLK_SHARED_SECRETS_MASK 0
#define R23_DLK_KEY_PROTO_NEGOTIATION_MASK 0
#endif /* R23_DLK_AES128_ENABLE */

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
static void APP_StartFindAndBind(void);
static void vAppHandleAfEvent( BDB_tsZpsAfEvent *psZpsAfEvent);
static void vAppHandleZdoEvents( BDB_tsZpsAfEvent *psZpsAfEvent);
static void APP_vBdbInit(void);

#if !(defined(LNT_MODE_APP) || defined(KPI_MODE_APP))
static void vDeletePDMOnButtonPress(uint8_t u8ButtonID);
#endif

static void vPrintAPSTable(void);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
tsDeviceDesc sDeviceDesc;
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
uint32_t u32OldFrameCtr;



#ifdef R23_UPDATES
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpacked"
#pragma GCC diagnostic ignored "-Wattributes"

/* Derive a test-specific type from tuTlvManufacturerSpecific */
TLV_DEF(tuTlvTestSpecific1,
        uint16, u16ZigbeeManufId,
        uint8, au8Extra[2]
);
TLV_DEF(tuTlvTestSpecific2,
        uint16, u16ZigbeeManufId,
        uint8, au8Extra[6]
);

#define APP_SIZE_JOINREQ_TLV (sizeof(tuTlvTestSpecific1) +\
                              sizeof(tuFragParams) +\
                              sizeof(tuSupportedKeyNegotiationMethods) +\
                              sizeof(tuTlvTestSpecific2))

TLV_ENCAPS(g_sJoinerTlvs,
           APP_SIZE_JOINREQ_TLV,
           m_, tuTlvTestSpecific1,
           m_, tuFragParams,
           m_, tuSupportedKeyNegotiationMethods,
           m_, tuTlvTestSpecific2) =
{
        .u8Tag = ZPS_TLV_G_JOINERENCAPS, .u8Len = APP_SIZE_JOINREQ_TLV - 1,

        /* This TLV is sent inside the Joiner Encapsulation */
        { .u16ZigbeeManufId = 0x1234, .au8Extra[0] = 0xAA, .au8Extra[1] = 0xBB,
          .u8Tag = ZPS_TLV_G_MANUFSPEC, .u8Len = sizeof(tuTlvTestSpecific1) - 1 - ZPS_TLV_HDR_SIZE
        },

        { .u16NodeId = 1, .u8FragOpt = 2, .u16InMaxLen = 10,
          .u8Tag = ZPS_TLV_G_FRAGPARAMS, .u8Len = sizeof(tuFragParams) - 1 - ZPS_TLV_HDR_SIZE
        },

        { .u8KeyNegotProtMask = ZPS_TLV_G_SUPPKEYNEGMETH_STATKEYREQ
                                | R23_DLK_KEY_PROTO_NEGOTIATION_MASK,
          .u8SharedSecretsMask = R23_DLK_SHARED_SECRETS_MASK,
          .au8SrcIeeeAddr = {0},
          .u8Tag = ZPS_TLV_G_SUPPKEYNEGMETH, .u8Len = sizeof(tuSupportedKeyNegotiationMethods) - 1 - ZPS_TLV_HDR_SIZE
        },

        /* This TLV is sent inside the Joiner Encapsulation */
        { .u16ZigbeeManufId = 0x1234, .au8Extra = {0, 1, 2, 3, 4, 5},
          .u8Tag = ZPS_TLV_G_MANUFSPEC, .u8Len = sizeof(tuTlvTestSpecific2) - 1 - ZPS_TLV_HDR_SIZE
        }
};
#pragma GCC diagnostic pop

uint8 au8Storage_Tlv1[sizeof(tuTlvManufacturerSpecific) + 3] =
{
    [offsetof(tuTlvManufacturerSpecific, u8Tag)] = ZPS_TLV_G_MANUFSPEC,
    [offsetof(tuTlvManufacturerSpecific, u8Len)] = sizeof(au8Storage_Tlv1) - 1 - ZPS_TLV_HDR_SIZE,
    [offsetof(tuTlvManufacturerSpecific, u16ZigbeeManufId)    ] = 0x12,
    [offsetof(tuTlvManufacturerSpecific, u16ZigbeeManufId) + 1] = 0x34,
    [offsetof(tuTlvManufacturerSpecific, au8Extra)    ] = 'N',
    [offsetof(tuTlvManufacturerSpecific, au8Extra) + 1] = 'X',
    [offsetof(tuTlvManufacturerSpecific, au8Extra) + 2] = 'P',
};
tuTlvManufacturerSpecific *g_pTlv1 = (tuTlvManufacturerSpecific *)&au8Storage_Tlv1;

//TLV_MANUFACTURERSPECIFIC_PTR(const static, g_p, Tlv2, 0x3412);
TLV_MANUFACTURERSPECIFIC_PTR( , g_p, Tlv2, 0x3412);

TLV_MANUFACTURERSPECIFIC_EX_PTR( , g_p, Tlv3, 0x3412, 9, 'T', 'L', 'V', 'S', ' ', 'D', 'A', 'T', 'A');

tuRouterInfo g_Tlv4 = {
        .u8Tag = ZPS_TLV_G_ROUTERINFO, .u8Len = sizeof(tuRouterInfo) - 1 - ZPS_TLV_HDR_SIZE,
        0xAABB
};
TLV_MANUFACTURERSPECIFIC_EX_PTR( , g_p, Tlv5, 0xFFFE, 8, 0, 0, 0, 0, 0, 0, 0, 0);
TLV_USERDEFINED_PTR( , g_p, Tlv6, 60, 6, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

uint8 au8TestTlvs[sizeof(au8Storage_Tlv1) + sizeof(au8Storage_Tlv2) +
                  sizeof(au8Storage_Tlv3) + sizeof(g_Tlv4)];
uint8 au8JoinTlvs[sizeof(au8Storage_Tlv1) + sizeof(au8Storage_Tlv2) +
                  sizeof(au8Storage_Tlv3) + sizeof(g_sJoinerTlvs)];

uint8 au8TestTlvs1[sizeof(au8Storage_Tlv5) + sizeof(au8Storage_Tlv6)];

#define APP_SIZE_PERMITJOINREQ_TLV (sizeof(tuSupportedKeyNegotiationMethods) +\
                                    sizeof(tuFragParams) + sizeof(au8TestTlvs))

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpacked"
TLV_ENCAPS(g_sPermitJoinReqTlvs, APP_SIZE_PERMITJOINREQ_TLV, m_, tuSupportedKeyNegotiationMethods, m_, tuFragParams) =
{
        .u8Tag = ZPS_TLV_G_BEACONAPPENCAPS, .u8Len = sizeof(tuSupportedKeyNegotiationMethods) + sizeof(tuFragParams) - 1,
        { .u8KeyNegotProtMask = ZPS_TLV_G_SUPPKEYNEGMETH_STATKEYREQ
                                | R23_DLK_KEY_PROTO_NEGOTIATION_MASK,
          .u8SharedSecretsMask = R23_DLK_SHARED_SECRETS_MASK,
          .au8SrcIeeeAddr = {0},
          .u8Tag = ZPS_TLV_G_SUPPKEYNEGMETH, .u8Len = sizeof(tuSupportedKeyNegotiationMethods) - sizeof(tsTlvGeneric) - 1 },
        { .u16NodeId = 1, .u8FragOpt = 2, .u16InMaxLen = 10,
          .u8Tag = ZPS_TLV_G_FRAGPARAMS, .u8Len = sizeof(tuFragParams) - 1 - ZPS_TLV_HDR_SIZE }
};
#pragma GCC diagnostic pop
#endif

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

extern void zps_vNwkSecClearMatSet(void *psNwk);


/****************************************************************************
 *
 * NAME: APP_vInitialiseRouter
 *
 * DESCRIPTION:
 * Initialises the application related functions
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void APP_vInitialiseRouter(void)
{
    uint16 u16ByteRead;
    PDM_teStatus eStatusReportReload;
#ifdef CLD_OTA
    vLoadOTAPersistedData();
#endif
    sDeviceDesc.eNodeState = E_STARTUP;
    PDM_eReadDataFromRecord(PDM_ID_APP_ROUTER,
                            &sDeviceDesc,
                            sizeof(tsDeviceDesc),
                            &u16ByteRead);

    void *pvNwk = ZPS_pvAplZdoGetNwkHandle();
    pvNwk = pvNwk;

    /* Restore any report data that is previously saved to flash */
    eStatusReportReload = eRestoreReports();

    ZPS_psAplAibGetAib()->bUseInstallCode = BDB_JOIN_USES_INSTALL_CODE_KEY;
#ifndef ENABLE_SUBG_IF
    APP_SetHighTxPowerMode();
#endif

#ifdef R23_UPDATES
    ZPS_vTlvBuildSequence(4, sizeof(au8JoinTlvs), au8JoinTlvs,
            g_pTlv1, g_pTlv2, g_pTlv3, &g_sJoinerTlvs);
    ZPS_vTlvBuildSequence(4, sizeof(au8TestTlvs), au8TestTlvs,
            g_pTlv1, g_pTlv2, g_pTlv3, &g_Tlv4);
    ZPS_vAplAfSetAdditionalTlvs(au8JoinTlvs, sizeof(au8JoinTlvs));
#endif

    /* Initialise ZBPro stack */
    ZPS_eAplAfInit();

#ifdef R23_UPDATES
    /* All the network wide TLVs are reset for non-TC routers */
    ZPS_vNwkNibSetBeaconAppendix(pvNwk, TRUE, FALSE,
            g_sPermitJoinReqTlvs.u8Len + 1,
            (tsTlvGeneric *)((uint8 *)&g_sPermitJoinReqTlvs + ZPS_TLV_HDR_SIZE));
    /* All the local TLVs are kept as a set/collection */
    ZPS_vNwkNibSetBeaconAppendix(pvNwk, FALSE, TRUE,
            sizeof(au8TestTlvs), (tsTlvGeneric *)au8TestTlvs);

    /* Configure the DLK options stored in the JoinerTLVs */
    ZPS_eAplAibSetKeyNegotiationOptions(
            g_sJoinerTlvs.m_tuSupportedKeyNegotiationMethods.u8KeyNegotProtMask,
            g_sJoinerTlvs.m_tuSupportedKeyNegotiationMethods.u8SharedSecretsMask);

#if R23_DLK_AES128_ENABLE
    /* ZigBeeAlliance18 well known pass-phrase */
    uint8 au8Passphrase[] = {
                    0x5a,
                    0x69,
                    0x67,
                    0x42,
                    0x65,
                    0x65,
                    0x41,
                    0x6c,
                    0x6c,
                    0x69,
                    0x61,
                    0x6e,
                    0x63,
                    0x65,
                    0x31,
                    0x38
    };

    /* Std 4.6.3.1 "When a device prepares to join a secured network
     * it SHALL create an apsDeviceKeyPairSet entry for the Trust
     * Center with its initial joining link key. It will set
     * the apsLinkKeyType of that entry according to the kind of key
     * it has. [...] If it supports key negotiation it will also set
     * its initial Passphrase attribute of the apsDeviceKeyPairSet entry.
     * [...] When joining for the first time the address of
     * the Trust Center will not be known to the joiner.
     * The DeviceAddress value for the apsDeviceKeyPairSet entry for
     * the Trust Center will initially have an all F's address, as
     * will the apsTrustCenterAddress AIB value."
     */
    if (ZPS_E_SUCCESS != ZPS_eAplAibSetDeviceApsDlkPassphrase(
                             ZPS_E_BROADCAST_ALL_IEEE,
                             au8Passphrase, sizeof(au8Passphrase)))
    {
        DBG_vPrintf(TRUE, "FAILED to set the DLK pass-phrase on the TCLK entry\n");
    }
#endif /* R23_DLK_AES128_ENABLE */
#endif /* R23_UPDATES */

#ifndef ENABLE_SUBG_IF
    APP_SetMaxTxPower();
#endif
    /* Initialise ZCL */
    APP_ZCL_vInitialise();

    /* Initialise other software modules
     * HERE
     */
    APP_vBdbInit();

#if !(defined(LNT_MODE_APP) || defined(KPI_MODE_APP))
    /* Delete PDM if required */
    vDeletePDMOnButtonPress(APP_E_BUTTONS_BUTTON_1);
#endif

    DBG_vPrintf(TRACE_APP, "Start Up State %d On Network %d\r\n",
            sDeviceDesc.eNodeState,
            sBDB.sAttrib.bbdbNodeIsOnANetwork);

    /*Load the reports from the PDM or the default ones depending on the PDM load record status*/
    if(eStatusReportReload !=PDM_E_STATUS_OK )
    {
        /*Load Defaults if the data was not correct*/
        vLoadDefaultConfigForReportable();
    }
    /*Make the reportable attributes */
    vMakeSupportedAttributesReportable();
    vPrintAPSTable();

#ifdef LNT_MODE_APP
#include "lnt_init.h"
    lnt_init();
#endif
}

/****************************************************************************
 *
 * NAME: APP_vBdbCallback
 *
 * DESCRIPTION:
 * Callback from the BDB
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void APP_vBdbCallback(BDB_tsBdbEvent *psBdbEvent)
{
    switch(psBdbEvent->eEventType)
    {
        case BDB_EVENT_NONE:
            break;

        case BDB_EVENT_ZPSAF:                // Use with BDB_tsZpsAfEvent
            vAppHandleAfEvent(&psBdbEvent->uEventData.sZpsAfEvent);
            break;

        case BDB_EVENT_INIT_SUCCESS:
            DBG_vPrintf(TRACE_APP,"APP: BDB_EVENT_INIT_SUCCESS\r\n");
            if (sDeviceDesc.eNodeState == E_STARTUP)
            {
#ifndef KPI_MODE_APP
#ifdef BDB_SUPPORT_OOBC
                if (!APP_Start_BDB_OOB())
#endif
                {
                    BDB_teStatus eStatus = BDB_eNsStartNwkSteering();
                    DBG_vPrintf(TRACE_APP, "BDB Try Steering status %d\r\n",eStatus);
                }
#endif
            }
            else
            {
                DBG_vPrintf(TRACE_APP, "BDB Init go Running");
                sDeviceDesc.eNodeState = E_RUNNING;
                PDM_eSaveRecordData(PDM_ID_APP_ROUTER,&sDeviceDesc,sizeof(tsDeviceDesc));
            }
            break;

        case BDB_EVENT_NO_NETWORK:
            DBG_vPrintf(TRACE_APP,"APP: BDB No Networks\r\n");
#ifdef LNT_MODE_APP
        #define LNT_TIME 1
            extern uint8 u8LntTimerTick;
            ZTIMER_eStart(u8LntTimerTick, ZTIMER_TIME_SEC((LNT_TIME + (uint8)CRYPTO_u32RandomGet(0, LNT_TIME))));
            break;
        #undef LNT_TIME
#endif
        case BDB_EVENT_OOB_FAIL:
        case BDB_EVENT_REJOIN_FAILURE:
#ifndef KPI_MODE_APP
            if (BDB_bIsBaseIdle())
            {
#ifdef BDB_SUPPORT_OOBC
                if (!APP_Start_BDB_OOB())
#endif
                {
                    BDB_teStatus eStatus = BDB_eNsStartNwkSteering();
                    DBG_vPrintf(TRACE_APP, "BDB Try Steering status %d\r\n",eStatus);
                }
            }
#endif
            break;

        case BDB_EVENT_NWK_STEERING_SUCCESS:
            DBG_vPrintf(TRACE_APP,"APP: NwkSteering Success \r\n");
#ifdef LNT_MODE_APP
            int lnt_BleAdvDisabled();
            void lnt_EnableBleAdv();
            void BleApp_Start();
            /* start BLE adv after joining ZB network */
            if (lnt_BleAdvDisabled())
            {
                lnt_EnableBleAdv();
                BleApp_Start();
            }
#endif
        case BDB_EVENT_OOB_FORM_SUCCESS:
        case BDB_EVENT_OOB_JOIN_SUCCESS:
        case BDB_EVENT_REJOIN_SUCCESS:
            DBG_vPrintf(TRACE_APP,"APP: BDB_EVENT_REJOIN_SUCCESS \r\n");
#ifdef BDB_SUPPORT_OOBC
            APP_ClearOOBInfo();
#endif
#ifdef KPI_MODE_APP
        case BDB_EVENT_NWK_FORMATION_SUCCESS:
            DBG_vPrintf(TRACE_APP, "APP-BDB: NwkFormation Success, Addr %04x, Channel %d\r\n",
                        ZPS_u16AplZdoGetNwkAddr(), ZPS_u8AplZdoGetRadioChannel());
#endif
            sDeviceDesc.eNodeState = E_RUNNING;
            PDM_eSaveRecordData(PDM_ID_APP_ROUTER,&sDeviceDesc,sizeof(tsDeviceDesc));
            u32Togglems = 250;
            ZTIMER_eStop(u8LedTimer);
            ZTIMER_eStart(u8LedTimer, ZTIMER_TIME_MSEC(u32Togglems));
            break;

        default:
            break;
    }
}

/****************************************************************************
 *
 * NAME: APP_taskRouter
 *
 * DESCRIPTION:
 * Task that handles application related functions
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void APP_taskRouter(void)
{
    APP_tsEvent sAppEvent;
    sAppEvent.eType = APP_E_EVENT_NONE;

    if (ZQ_bQueueReceive(&APP_msgAppEvents, &sAppEvent) == TRUE)
    {
        DBG_vPrintf(TRACE_APP, "ZPR: App event %d, NodeState=%d\r\n", sAppEvent.eType, sDeviceDesc.eNodeState);

        if(sAppEvent.eType == APP_E_EVENT_BUTTON_DOWN)
        {
            switch(sAppEvent.uEvent.sButton.u8Button)
            {
                case APP_E_BUTTONS_BUTTON_1:
                    APP_StartFindAndBind();
                    break;

                default:
                    break;
            }
        }
#ifdef APP_ROUTER_NODE_CLI
        else if(sAppEvent.eType == APP_E_EVENT_SERIAL_FIND_BIND_START)
        {
            APP_StartFindAndBind();
        }
#endif
    }
}

/****************************************************************************
 *
 * NAME: APP_StartFindAndBind
 *
 * DESCRIPTION:
 * Starts Find and Bind procedure
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
static void APP_StartFindAndBind(void)
{
    BDB_teStatus eStatus;

    DBG_vPrintf(TRACE_APP_EVENT, "APP_EVENT: Network steering and F&B as Target\r\n");
    eStatus = BDB_eNsStartNwkSteering();
    if (eStatus != 0)
    {
        DBG_vPrintf(TRACE_APP_EVENT, "APP_EVENT: Network Steering %02x\r\n", eStatus);
    }
    eStatus = BDB_eFbTriggerAsTarget(APP_u8GetDeviceEndpoint());
    if (eStatus != 0 && eStatus != 9)
    {
        DBG_vPrintf(TRACE_APP_EVENT, "APP_EVENT: Find and Bind Failed %02x\r\n", eStatus);
    }
    u32Togglems = 250;
    ZTIMER_eStop(u8LedTimer);
    ZTIMER_eStart(u8LedTimer, ZTIMER_TIME_MSEC(u32Togglems));
}


/****************************************************************************
 *
 * NAME: vAppHandleAfEvent
 *
 * DESCRIPTION:
 * Application handler for stack events
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
static void vAppHandleAfEvent( BDB_tsZpsAfEvent *psZpsAfEvent)
{
#ifdef KPI_MODE_APP
    if ((psZpsAfEvent->u8EndPoint == ROUTER_DEVICEUNDERTEST_REQ_ENDPOINT) ||
        (psZpsAfEvent->u8EndPoint == ROUTER_DEVICEUNDERTEST_RSP_2_ENDPOINT) ||
        (psZpsAfEvent->u8EndPoint == ROUTER_DEVICEUNDERTEST_RSP_ENDPOINT))
    {
        return APP_msgTp2Events(&psZpsAfEvent->sStackEvent);
    }
    if (psZpsAfEvent->u8EndPoint == ROUTER_DEVDIAGNOSTICS_ENDPOINT)
    {
        if ((psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_DATA_INDICATION) ||
            (psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_INTERPAN_DATA_INDICATION))
        {
            APP_ZCL_vEventHandler(&psZpsAfEvent->sStackEvent);
        }
    }
    else
#endif
    if (psZpsAfEvent->u8EndPoint == APP_u8GetDeviceEndpoint())
    {
//        DBG_vPrintf(TRACE_APP, "Pass to ZCL\r\n");
        if ((psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_DATA_INDICATION) ||
            (psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_INTERPAN_DATA_INDICATION))
        {
            APP_ZCL_vEventHandler( &psZpsAfEvent->sStackEvent);
        }
    }
    else if (psZpsAfEvent->u8EndPoint == 0)
    {
        vAppHandleZdoEvents( psZpsAfEvent);
    }
    else if ((psZpsAfEvent->u8EndPoint == GREEN_POWER_ENDPOINT) ||  (psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_ZGP_DATA_CONFIRM))
    {
        if ((psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_DATA_INDICATION) ||
            (psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_ZGP_DATA_INDICATION)||
            (psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_ZGP_DATA_CONFIRM))
        {

            APP_ZCL_vEventHandler( &psZpsAfEvent->sStackEvent);
        }
    }
    /* Ensure Freeing of Apdus */
    if (psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_DATA_INDICATION)
    {
        PDUM_eAPduFreeAPduInstance(psZpsAfEvent->sStackEvent.uEvent.sApsDataIndEvent.hAPduInst);
    }
    else if ( psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_INTERPAN_DATA_INDICATION )
    {
        PDUM_eAPduFreeAPduInstance(psZpsAfEvent->sStackEvent.uEvent.sApsInterPanDataIndEvent.hAPduInst);
    }

}

/****************************************************************************
 *
 * NAME: vAppHandleZdoEvents
 *
 * DESCRIPTION:
 * Application handler for stack events for end point 0 (ZDO)
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
static void vAppHandleZdoEvents( BDB_tsZpsAfEvent *psZpsAfEvent)
{
    ZPS_tsAfEvent *psAfEvent = &(psZpsAfEvent->sStackEvent);

    switch(psAfEvent->eType)
    {
        case ZPS_EVENT_APS_DATA_INDICATION:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: Data Indication Status %02x from %04x Src Ep Dst %d Ep %d Profile %04x Cluster %04x\r\n",
                    psAfEvent->uEvent.sApsDataIndEvent.eStatus,
                    psAfEvent->uEvent.sApsDataIndEvent.uSrcAddress.u16Addr,
                    psAfEvent->uEvent.sApsDataIndEvent.u8SrcEndpoint,
                    psAfEvent->uEvent.sApsDataIndEvent.u8DstEndpoint,
                    psAfEvent->uEvent.sApsDataIndEvent.u16ProfileId,
                    psAfEvent->uEvent.sApsDataIndEvent.u16ClusterId);

            #ifdef CLD_OTA
                if ((psZpsAfEvent->sStackEvent.uEvent.sApsDataIndEvent.eStatus == ZPS_E_SUCCESS) &&
                        (psZpsAfEvent->sStackEvent.uEvent.sApsDataIndEvent.u8DstEndpoint == 0))
                {
                    // Data Ind for ZDp Ep
                    if (ZPS_ZDP_MATCH_DESC_RSP_CLUSTER_ID == psZpsAfEvent->sStackEvent.uEvent.sApsDataIndEvent.u16ClusterId)
                    {
                        vHandleMatchDescriptor(&psZpsAfEvent->sStackEvent);
                    } else if (ZPS_ZDP_IEEE_ADDR_RSP_CLUSTER_ID == psZpsAfEvent->sStackEvent.uEvent.sApsDataIndEvent.u16ClusterId) {
                        vHandleIeeeAddressRsp(&psZpsAfEvent->sStackEvent);
                    }
                }
            #endif
            break;

        case ZPS_EVENT_APS_DATA_CONFIRM:
            break;

        case ZPS_EVENT_APS_DATA_ACK:
            break;
            break;

        case ZPS_EVENT_NWK_JOINED_AS_ROUTER:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: Joined Network Addr %04x Rejoin %d\r\n",
                    psAfEvent->uEvent.sNwkJoinedEvent.u16Addr,
                    psAfEvent->uEvent.sNwkJoinedEvent.bRejoin);

            /* Save to PDM */
            sDeviceDesc.eNodeState = E_RUNNING;
            PDM_eSaveRecordData(PDM_ID_APP_ROUTER,&sDeviceDesc,sizeof(tsDeviceDesc));

            ZPS_eAplAibSetApsUseExtendedPanId( ZPS_u64NwkNibGetEpid(ZPS_pvAplZdoGetNwkHandle()) );
            APP_vSetLed(APP_E_LEDS_LED_2, APP_E_LED_ON);
            break;
        case ZPS_EVENT_NWK_FAILED_TO_JOIN:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: Failed To Join %02x Rejoin %d\r\n",
                    psAfEvent->uEvent.sNwkJoinFailedEvent.u8Status,
                    psAfEvent->uEvent.sNwkJoinFailedEvent.bRejoin);
            break;

        case ZPS_EVENT_NWK_NEW_NODE_HAS_JOINED:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: New Node %04x Has Joined\r\n",
                    psAfEvent->uEvent.sNwkJoinIndicationEvent.u16NwkAddr);
            break;

        case ZPS_EVENT_NWK_DISCOVERY_COMPLETE:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: Discovery Complete %02x\r\n",
                    psAfEvent->uEvent.sNwkDiscoveryEvent.eStatus);
            break;

        case ZPS_EVENT_NWK_LEAVE_INDICATION:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: Leave Indication %016llx Rejoin %d\r\n",
                    psAfEvent->uEvent.sNwkLeaveIndicationEvent.u64ExtAddr,
                    psAfEvent->uEvent.sNwkLeaveIndicationEvent.u8Rejoin);
            if ( (psAfEvent->uEvent.sNwkLeaveIndicationEvent.u64ExtAddr == 0UL) &&
                 (psAfEvent->uEvent.sNwkLeaveIndicationEvent.u8Rejoin == 0) )
            {
                /* We sare asked to Leave without rejoin */
                DBG_vPrintf(TRACE_APP, "LEAVE IND -> For Us No Rejoin\r\n");
#ifndef KPI_MODE_APP
                APP_vFactoryResetRecords();
                MICRO_DISABLE_INTERRUPTS();
#if IS_NOT_MCXW_SERIES_OR_RW_SERIES
                vMMAC_Disable();
#endif
                RESET_SystemReset();
#endif
            }
            break;

        case ZPS_EVENT_NWK_LEAVE_CONFIRM:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: Leave Confirm status %02x Addr %016llx\r\n",
                    psAfEvent->uEvent.sNwkLeaveConfirmEvent.eStatus,
                    psAfEvent->uEvent.sNwkLeaveConfirmEvent.u64ExtAddr);
            if ((psAfEvent->uEvent.sNwkLeaveConfirmEvent.eStatus == ZPS_E_SUCCESS) &&
                (psAfEvent->uEvent.sNwkLeaveConfirmEvent.u64ExtAddr == 0UL))
            {
                DBG_vPrintf(TRACE_APP, "Leave -> Reset Data Structures\r\n");
#ifdef KPI_MODE_APP
                sBDB.sAttrib.bbdbNodeIsOnANetwork = FALSE;
                sDeviceDesc.eNodeState = E_STARTUP;
                PDM_eSaveRecordData(PDM_ID_APP_ROUTER,&sDeviceDesc,sizeof(tsDeviceDesc));
#else
                APP_vFactoryResetRecords();
                MICRO_DISABLE_INTERRUPTS();
#if IS_NOT_MCXW_SERIES_OR_RW_SERIES
                vMMAC_Disable();
#endif
                RESET_SystemReset();
#endif
            }
            break;

        case ZPS_EVENT_NWK_STATUS_INDICATION:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: Network status Indication %02x addr %04x\r\n",
                    psAfEvent->uEvent.sNwkStatusIndicationEvent.u8Status,
                    psAfEvent->uEvent.sNwkStatusIndicationEvent.u16NwkAddr);
            break;

        case ZPS_EVENT_NWK_ROUTE_DISCOVERY_CONFIRM:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: Discovery Confirm\r\n");
            break;

        case ZPS_EVENT_NWK_ED_SCAN:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: Energy Detect Scan %02x\r\n",
                    psAfEvent->uEvent.sNwkEdScanConfirmEvent.u8Status);
            break;
        case ZPS_EVENT_ZDO_BIND:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: Zdo Bind event\r\n");
            break;

        case ZPS_EVENT_ZDO_UNBIND:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: Zdo Unbiind Event\r\n");
            break;

        case ZPS_EVENT_ZDO_LINK_KEY:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: Zdo Link Key Event Type %d Addr %016llx\r\n",
                        psAfEvent->uEvent.sZdoLinkKeyEvent.u8KeyType,
                        psAfEvent->uEvent.sZdoLinkKeyEvent.u64IeeeLinkAddr);
            break;

        case ZPS_EVENT_BIND_REQUEST_SERVER:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: Bind Request Server Event\r\n");
            break;

        case ZPS_EVENT_ERROR:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: AF Error Event %d\r\n", psAfEvent->uEvent.sAfErrorEvent.eError);
            break;

        case ZPS_EVENT_TC_STATUS:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: Trust Center Status %02x\r\n", psAfEvent->uEvent.sApsTcEvent.u8Status);
            break;

        case ZPS_EVENT_NWK_DUTYCYCLE_INDICATION:
            DBG_vPrintf(TRUE, "Nwk Duty Cycle Indication, mode = %d\n", psAfEvent->uEvent.sNwkDutyCycleIndicationEvent.u8Mode);

            break;

        default:
            break;
    }
}


/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/


/****************************************************************************
 *
 * NAME: APP_vFactoryResetRecords
 *
 * DESCRIPTION:
 * Resets persisted data structures to factory new state
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void APP_vFactoryResetRecords(void)
{
    /* clear out the stack */
    ZPS_vDefaultStack();
    ZPS_eAplAibSetApsUseExtendedPanId(0);
    ZPS_vSetKeys();
    /* save everything */
    sDeviceDesc.eNodeState = E_STARTUP;
#ifdef CLD_OTA
    vResetOTADiscovery();
    vOTAResetPersist();
#endif
    PDM_eSaveRecordData(PDM_ID_APP_ROUTER,&sDeviceDesc,sizeof(tsDeviceDesc));
    ZPS_vSaveAllZpsRecords();
}

/****************************************************************************
 *
 * NAME: APP_vBdbInit
 *
 * DESCRIPTION:
 * Function to initialize BDB attributes and message queue
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
static void APP_vBdbInit(void)
{
    BDB_tsInitArgs sInitArgs;

    sBDB.sAttrib.bbdbNodeIsOnANetwork = ((sDeviceDesc.eNodeState >= E_RUNNING)?(TRUE):(FALSE));
    sInitArgs.hBdbEventsMsgQ = &APP_msgBdbEvents;
    BDB_vInit(&sInitArgs);

    if(sDeviceDesc.eNodeState >= E_RUNNING)
    {
      	APP_vSetLed(APP_E_LEDS_LED_1, APP_E_LED_OFF);
       	APP_vSetLed(APP_E_LEDS_LED_2, APP_E_LED_ON);
    }
    else
    {
       	APP_vSetLed(APP_E_LEDS_LED_1, APP_E_LED_OFF);
        APP_vSetLed(APP_E_LEDS_LED_2, APP_E_LED_OFF);
    }
}

/****************************************************************************
 *
 * NAME: vDeletePDMOnButtonPress
 *
 * DESCRIPTION:
 * PDM context clearing on button press
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
#if !(defined(LNT_MODE_APP) || defined(KPI_MODE_APP))
static void vDeletePDMOnButtonPress(uint8_t u8ButtonID)
{
    bool_t bDeleteRecords = FALSE;
    uint8_t u8Status;

    bool_t bButtonPressed =  APP_u32GetButtonsState() & (1 << u8ButtonID);

    if (bButtonPressed)
    {
        bDeleteRecords = TRUE;
    }
    else
    {
        bDeleteRecords = FALSE;
    }
    /* If required, at this point delete the network context from flash, perhaps upon some condition
     * For example, check if a button is being held down at reset, and if so request the Persistent
     * Data Manager to delete all its records:
     * e.g. bDeleteRecords = vCheckButtons();
     * Alternatively, always call PDM_vDeleteAllDataRecords() if context saving is not required.
     */
    if(bDeleteRecords)
    {
        /* wait for button release */
        while (bButtonPressed)
        {
            bButtonPressed = APP_u32GetButtonsState() & (1 << u8ButtonID);
        }
        u8Status = ZPS_eAplZdoLeaveNetwork(0, FALSE,FALSE);
        if (ZPS_E_SUCCESS !=  u8Status )
        {
            /* Leave failed,so just reset everything */
            DBG_vPrintf(TRACE_APP,"Leave failed status %x Deleting the PDM\r\n", u8Status);
            APP_vFactoryResetRecords();
            MICRO_DISABLE_INTERRUPTS();
// TODO: Making SW reset abstracted
#if IS_NOT_MCXW_SERIES_OR_RW_SERIES
            vMMAC_Disable();
            RESET_SystemReset();
#else
            NVIC_SystemReset();
#endif
        } else { DBG_vPrintf(TRACE_APP, "RESET: Sent Leave\r\n"); }
    }
}
#endif

/****************************************************************************
 *
 * NAME: vPrintAPSTable
 *
 * DESCRIPTION:
 * Prints the content of APS table
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
static void vPrintAPSTable(void)
{
    uint8_t i;
    uint8_t j;

    ZPS_tsAplAib * psAplAib;
    uint64_t u64Addr;

    psAplAib = ZPS_psAplAibGetAib();

    for ( i = 0 ; i < (psAplAib->psAplDeviceKeyPairTable->u16SizeOfKeyDescriptorTable + 3) ; i++ )
    {
        u64Addr = ZPS_u64NwkNibGetMappedIeeeAddr(ZPS_pvAplZdoGetNwkHandle(), psAplAib->psAplDeviceKeyPairTable->psAplApsKeyDescriptorEntry[i].u16ExtAddrLkup);
        DBG_vPrintf(TRUE, "%d MAC: 0x%x %x Key: ", i, (uint32_t)(u64Addr>>32), (uint32_t)(u64Addr&0xffffffff) );
        for(j=0; j<16;j++)
        {
            DBG_vPrintf(TRUE, "%02x ", psAplAib->psAplDeviceKeyPairTable->psAplApsKeyDescriptorEntry[i].au8LinkKey[j]);
        }
        DBG_vPrintf(TRUE, "\r\n");
    }
}


/****************************************************************************
 *
 * NAME: eGetNodeState
 *
 * DESCRIPTION:
 * Returns the node state
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
teNodeState eGetNodeState(void)
{
    return sDeviceDesc.eNodeState;
}
/****************************************************************************
 *
 * NAME: sGetOTACallBackPersistdata
 *
 * DESCRIPTION:
 * returns persisted data
 *
 * RETURNS:
 * tsOTA_PersistedData
 *
 ****************************************************************************/
#ifdef CLD_OTA
tsOTA_PersistedData sGetOTACallBackPersistdata(void)
{
    return sBaseDevice.sCLD_OTA_CustomDataStruct.sOTACallBackMessage.sPersistedData;
}
#endif
/****************************************************************************
 *
 * NAME: APP_u8GetDeviceEndpoint
 *
 * DESCRIPTION:
 * Return the application endpoint
 *
 * PARAMETER: void
 *
 * RETURNS: uint8_t
 *
 ****************************************************************************/
uint8_t APP_u8GetDeviceEndpoint( void)
{
    return ROUTER_APPLICATION_ENDPOINT;
}



/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
