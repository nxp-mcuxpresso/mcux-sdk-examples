/*
* Copyright 2019, 2023-2024 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/


/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <EmbeddedTypes.h>
#include "bdb_api.h"
#include "PDM.h"
#include "PDM_IDs.h"
#include "app_common.h"
#include "app_zcl_task.h"
#include "dbg.h"
#include "app_main.h"
#include "zigbee_config.h"
#include "zps_gen.h"
#include "pdum_gen.h"
#include "app_coordinator.h"
#include "app_ota_server.h"
#include "app_leds.h"
#include "app.h"


/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
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

static void vAppHandleAfEvent( BDB_tsZpsAfEvent *psZpsAfEvent);
static void vAppSendOnOff(void);
static void vAppSendIdentifyStop( uint16_t u16Address, uint8_t u8Endpoint);
static void vAppSendRemoteBindRequest(uint16_t u16DstAddr, uint16_t u16ClusterId, uint8_t u8DstEp);
static void APP_vBdbInit(void);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
extern uint32_t u32Togglems;


/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

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

        { .u8KeyNegotProtMask = ZPS_TLV_G_SUPPKEYNEGMETH_STATKEYREQ,
          .u8SharedSecretsMask = 0,
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

static teNodeState eNodeState;
#define GREEN_POWER_ENDPOINT                   242

/****************************************************************************/
/***        Global Variables                                              ***/
/****************************************************************************/
PUBLIC uint16 u16appPrintBufferTimeInSec = 300U;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/


/****************************************************************************
 *
 * NAME: APP_vInitialiseCoordinator
 *
 * DESCRIPTION:
 * Initialises the Coordinator application
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void APP_vInitialiseCoordinator(void)
{
    /* Restore any application data previously saved to flash
     * All Application records must be loaded before the call to
     * ZPS_eAplAfInit
     */
    eNodeState = E_STARTUP;
    uint16_t u16ByteRead;
    PDM_eReadDataFromRecord(PDM_ID_APP_COORD,
                            &eNodeState,
                            sizeof(eNodeState),
                            &u16ByteRead);

    void *pvNwk = ZPS_pvAplZdoGetNwkHandle();
    pvNwk = pvNwk;

#if !(defined(ENABLE_SUBG_IF))
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
    /* All the network wide TLVs are kept in the app inside an
     * encapsulation and its payload is passed to the beacon API */
    ZPS_vNwkNibSetBeaconAppendix(pvNwk, TRUE, FALSE,
            g_sPermitJoinReqTlvs.u8Len + 1,
            (tsTlvGeneric *)((uint8 *)&g_sPermitJoinReqTlvs + ZPS_TLV_HDR_SIZE));
    /* All the local TLVs are kept as a set/collection */
    ZPS_vNwkNibSetBeaconAppendix(pvNwk, FALSE, TRUE,
            sizeof(au8TestTlvs), (tsTlvGeneric *)au8TestTlvs);

    ZPS_eAplAibSetKeyNegotiationOptions(
            g_sPermitJoinReqTlvs.m_tuSupportedKeyNegotiationMethods.u8KeyNegotProtMask,
            g_sPermitJoinReqTlvs.m_tuSupportedKeyNegotiationMethods.u8SharedSecretsMask);
#endif /* R23_UPDATES */

#ifdef ENABLE_SUBG_IF

    ZPS_vNwkNibSetMacEnhancedMode(pvNwk, TRUE);
    ZPS_u8MacMibIeeeSetPolicy(FALSE);
#endif

#if !(defined(ENABLE_SUBG_IF))
    APP_SetMaxTxPower();
#endif
    APP_ZCL_vInitialise();

    /* Initialise other software modules
     * HERE
     */
    APP_vBdbInit();
    DBG_vPrintf(TRACE_APP, "Recovered Application State %d On Network %d\r\n",
            eNodeState, sBDB.sAttrib.bbdbNodeIsOnANetwork);
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
    static uint8_t u8NoQueryCount;

    switch(psBdbEvent->eEventType)
    {
        case BDB_EVENT_NONE:
            break;

        case BDB_EVENT_ZPSAF:                // Use with BDB_tsZpsAfEvent
            vAppHandleAfEvent(&psBdbEvent->uEventData.sZpsAfEvent);
            break;

        case BDB_EVENT_INIT_SUCCESS:
            break;

        case BDB_EVENT_NWK_FORMATION_SUCCESS:
            DBG_vPrintf(TRACE_APP,"APP-BDB: NwkFormation Success \r\n");
            eNodeState = E_RUNNING;
            PDM_eSaveRecordData(PDM_ID_APP_COORD,
                                &eNodeState,
                                sizeof(eNodeState));
            APP_vSetLed(APP_E_LEDS_LED_2, APP_E_LED_ON);

            break;

        case BDB_EVENT_NWK_STEERING_SUCCESS:
            DBG_vPrintf(TRACE_APP,"APP-BDB: NwkSteering Success\r\n");
            if(u32Togglems == 1000)
            {
                u32Togglems = 250;
            }
            else
            {
            	u32Togglems = 500;
            }
            ZTIMER_eStop(u8LedTimer);
            ZTIMER_eStart(u8LedTimer, ZTIMER_TIME_MSEC(u32Togglems));
            break;

        case BDB_EVENT_NWK_FORMATION_FAILURE:
            DBG_vPrintf(TRACE_APP,"APP-BDB: Network Formation Failure\r\n");
            APP_vSetLed(APP_E_LEDS_LED_2, APP_E_LED_OFF);
            break;

        case BDB_EVENT_FB_HANDLE_SIMPLE_DESC_RESP_OF_TARGET:
            DBG_vPrintf(TRACE_APP,"APP-BDB: F&B Simple Desc response From %04x Profle %04x Device %04x Ep %d Version %d\r\n",
                    psBdbEvent->uEventData.psFindAndBindEvent->u16TargetAddress,
                    psBdbEvent->uEventData.psFindAndBindEvent->u16ProfileId,
                    psBdbEvent->uEventData.psFindAndBindEvent->u16DeviceId,
                    psBdbEvent->uEventData.psFindAndBindEvent->u8TargetEp,
                    psBdbEvent->uEventData.psFindAndBindEvent->u8DeviceVersion);
            break;

        case BDB_EVENT_FB_CHECK_BEFORE_BINDING_CLUSTER_FOR_TARGET:
            DBG_vPrintf(TRACE_APP,"APP-BDB: Check For Binding Cluster %04x\r\n",
                        psBdbEvent->uEventData.psFindAndBindEvent->uEvent.u16ClusterId);
            break;
        case BDB_EVENT_FB_CLUSTER_BIND_CREATED_FOR_TARGET:
            DBG_vPrintf(TRACE_APP,"APP-BDB: Bind Created for cluster %04x\r\n",
                    psBdbEvent->uEventData.psFindAndBindEvent->uEvent.u16ClusterId);
            vAppSendRemoteBindRequest( psBdbEvent->uEventData.psFindAndBindEvent->u16TargetAddress,
                                       psBdbEvent->uEventData.psFindAndBindEvent->uEvent.u16ClusterId,
                                       psBdbEvent->uEventData.psFindAndBindEvent->u8TargetEp);
            break;

        case BDB_EVENT_FB_BIND_CREATED_FOR_TARGET:
            DBG_vPrintf(TRACE_APP,"APP-BDB: Bind Created for target EndPt %d\r\n",
                    psBdbEvent->uEventData.psFindAndBindEvent->u8TargetEp);
            vAppSendIdentifyStop( psBdbEvent->uEventData.psFindAndBindEvent->u16TargetAddress,
                                  psBdbEvent->uEventData.psFindAndBindEvent->u8TargetEp);

            break;

        case BDB_EVENT_FB_GROUP_ADDED_TO_TARGET:
            DBG_vPrintf(TRACE_APP,"APP-BDB: Group Added with ID %04x\r\n",
                    psBdbEvent->uEventData.psFindAndBindEvent->uEvent.u16GroupId);
            u8NoQueryCount = 0;
            break;

        case BDB_EVENT_FB_ERR_BINDING_FAILED:
            DBG_vPrintf(TRACE_APP,"APP-BDB: Binding Failed\r\n");
            break;

        case BDB_EVENT_FB_ERR_BINDING_TABLE_FULL:
            DBG_vPrintf(TRACE_APP,"APP-BDB: Binding Table Full\r\n");
            break;

        case BDB_EVENT_FB_ERR_GROUPING_FAILED:
            DBG_vPrintf(TRACE_APP,"APP-BDB: Grouping Failed\r\n");
            break;

        case BDB_EVENT_FB_NO_QUERY_RESPONSE:

            if(u8NoQueryCount >= 2)
            {
                u8NoQueryCount = 0;
                BDB_vFbExitAsInitiator();
                u32Togglems = 500;
                DBG_vPrintf(TRACE_APP,"APP-BDB: No Identify Query Response Stopping F&B\r\n");
            }
            else
            {
                u8NoQueryCount++;
                DBG_vPrintf(TRACE_APP,"APP-BDB: No Identify Query Response\r\n");
            }
            break;

        case BDB_EVENT_FB_TIMEOUT:
            DBG_vPrintf(TRACE_APP,"APP-BDB: F&B Timeout\r\n");
            break;


        default:
            DBG_vPrintf(TRACE_APP,"APP-BDB: Unhandled %d\r\n", psBdbEvent->eEventType);
            break;
    }
}


/****************************************************************************
 *
 * NAME: APP_taskCoordinator
 *
 * DESCRIPTION:
 * Main state machine
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void APP_taskCoordinator(void)
{
    BDB_teStatus eStatus;
    APP_tsEvent sAppEvent;
    sAppEvent.eType = APP_E_EVENT_NONE;

    if (ZQ_bQueueReceive(&APP_msgAppEvents, &sAppEvent) == TRUE)
    {
        DBG_vPrintf(TRACE_APP, "APP-EVT: Event %d, NodeState=%d\r\n", sAppEvent.eType, eNodeState);

        switch(sAppEvent.eType)
        {
                case APP_E_EVENT_SERIAL_TOGGLE:
                    DBG_vPrintf(TRACE_APP, "APP-EVT: Send Toggle Cmd\r\n");
                    vAppSendOnOff();
                    break;

                case APP_E_EVENT_SERIAL_NWK_STEER:
                    eStatus = BDB_eNsStartNwkSteering();
                    DBG_vPrintf(TRACE_APP, "APP-EVT: Request Nwk Steering %02x\r\n", eStatus);
                    break;

                case APP_E_EVENT_SERIAL_FIND_BIND_START:
                    eStatus = BDB_eFbTriggerAsInitiator(COORDINATOR_APPLICATION_ENDPOINT);
                    DBG_vPrintf(TRACE_APP, "APP-EVT: Find and Bind initiate %02x\r\n", eStatus);
                    if(BDB_E_SUCCESS == eStatus)
                    {
                    	if(ZPS_bGetPermitJoiningStatus())
                        {
                            u32Togglems = 250;
                        }
                        else
                        {
                      	    u32Togglems = 1000;
                        }
                        ZTIMER_eStop(u8LedTimer);
                        ZTIMER_eStart(u8LedTimer,ZTIMER_TIME_MSEC(u32Togglems));
                    }
                    break;

                case APP_E_EVENT_SERIAL_FORM_NETWORK:
                    /* Not already on a network ? */
                    if (FALSE == sBDB.sAttrib.bbdbNodeIsOnANetwork)
                    {
                        eStatus = BDB_eNfStartNwkFormation();
                        DBG_vPrintf(TRACE_APP, "APP-EVT: Request Nwk Formation %02x\r\n", eStatus);
                    }
                    break;

                default:
                    break;
        }

    }
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
    if (psZpsAfEvent->u8EndPoint == COORDINATOR_APPLICATION_ENDPOINT)
    {
        if ((psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_DATA_INDICATION) ||
            (psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_INTERPAN_DATA_INDICATION))
        {
            APP_ZCL_vEventHandler( &psZpsAfEvent->sStackEvent);
         }
    }
    else if (psZpsAfEvent->u8EndPoint == COORDINATOR_ZDO_ENDPOINT)
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

    /* free up any Apdus */
    if (psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_DATA_INDICATION)
    {
        PDUM_eAPduFreeAPduInstance(psZpsAfEvent->sStackEvent.uEvent.sApsDataIndEvent.hAPduInst);
    }
    else if ( psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_INTERPAN_DATA_INDICATION )
    {
        PDUM_eAPduFreeAPduInstance(psZpsAfEvent->sStackEvent.uEvent.sApsInterPanDataIndEvent.hAPduInst);
    }
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
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
void vAppHandleZdoEvents( BDB_tsZpsAfEvent *psZpsAfEvent)
{
    ZPS_tsAfEvent *psAfEvent = &(psZpsAfEvent->sStackEvent);

    switch(psAfEvent->eType)
    {
        case ZPS_EVENT_APS_DATA_INDICATION:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: Data Indication Status %02x from %04x Src Ep %d Dst Ep %d Profile %04x Cluster %04x\r\n",
                    psAfEvent->uEvent.sApsDataIndEvent.eStatus,
                    psAfEvent->uEvent.sApsDataIndEvent.uSrcAddress.u16Addr,
                    psAfEvent->uEvent.sApsDataIndEvent.u8SrcEndpoint,
                    psAfEvent->uEvent.sApsDataIndEvent.u8DstEndpoint,
                    psAfEvent->uEvent.sApsDataIndEvent.u16ProfileId,
                    psAfEvent->uEvent.sApsDataIndEvent.u16ClusterId);
            break;

        case ZPS_EVENT_APS_DATA_CONFIRM:
            break;

        case ZPS_EVENT_APS_DATA_ACK:
            break;
            break;

        case ZPS_EVENT_NWK_STARTED:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: Network started Channel = %d\r\n", ZPS_u8AplZdoGetRadioChannel());
            break;

        case ZPS_EVENT_NWK_FAILED_TO_START:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: Network Failed To start\r\n");
            break;

        case ZPS_EVENT_NWK_NEW_NODE_HAS_JOINED:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: New Node %04x Has Joined\r\n",
                    psAfEvent->uEvent.sNwkJoinIndicationEvent.u16NwkAddr);
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

            ZPS_eAplAibSetDeviceApsDlkPassphrase(
                    psAfEvent->uEvent.sNwkJoinIndicationEvent.u64ExtAddr,
                    au8Passphrase, sizeof(au8Passphrase));
#endif /* R23_DLK_AES128_ENABLE */
            break;

        case ZPS_EVENT_NWK_DISCOVERY_COMPLETE:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: Discovery Complete %02x\r\n",
                    psAfEvent->uEvent.sNwkDiscoveryEvent.eStatus);
            break;

        case ZPS_EVENT_NWK_LEAVE_INDICATION:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: Leave Indication %016llx Rejoin %d\r\n",
                    psAfEvent->uEvent.sNwkLeaveIndicationEvent.u64ExtAddr,
                    psAfEvent->uEvent.sNwkLeaveIndicationEvent.u8Rejoin);
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

        default:
            break;
    }
}



/****************************************************************************
 *
 * NAME: vAppSendOnOff
 *
 * DESCRIPTION:
 * Sends an On Of Togle Command to the bound devices
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
static void vAppSendOnOff(void)
{
    tsZCL_Address   sDestinationAddress;
    uint8_t u8seqNo;
    teZCL_Status eStatus;

    sDestinationAddress.eAddressMode = E_ZCL_AM_BOUND_NON_BLOCKING;

    eStatus = eCLD_OnOffCommandSend( APP_u8GetDeviceEndpoint(),      // Src Endpoint
                             0,                                             // Dest Endpoint (bound so do not care)
                             &sDestinationAddress,
                             &u8seqNo,
                             E_CLD_ONOFF_CMD_TOGGLE);

    if (eStatus != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_APP, "Send Toggle Failed x%02x Last error %02x\r\n",
                        eStatus, eZCL_GetLastZpsError());
    }

}

/****************************************************************************
 *
 * NAME: vAppSendIdentifyStop
 *
 * DESCRIPTION:
 * Sends an Identify stop command to the target address
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
static void vAppSendIdentifyStop( uint16_t u16Address, uint8_t u8Endpoint)
{
    uint8_t u8Seq;
    tsZCL_Address sAddress;
    tsCLD_Identify_IdentifyRequestPayload sPayload;

    sPayload.u16IdentifyTime = 0;
    sAddress.eAddressMode = E_ZCL_AM_SHORT_NO_ACK;
    sAddress.uAddress.u16DestinationAddress = u16Address;
    eCLD_IdentifyCommandIdentifyRequestSend(
    		APP_u8GetDeviceEndpoint(),
                            u8Endpoint,
                            &sAddress,
                            &u8Seq,
                            &sPayload);
}

/****************************************************************************
 *
 * NAME: vAppSendRemoteBindRequest
 *
 * DESCRIPTION:
 * Sends a bind request to a remote node for it to create a binding with this node
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
static void vAppSendRemoteBindRequest(uint16_t u16DstAddr, uint16_t u16ClusterId, uint8_t u8DstEp)
{
    PDUM_thAPduInstance hAPduInst;
    ZPS_tuAddress uDstAddr;
    ZPS_tsAplZdpBindUnbindReq sAplZdpBindUnbindReq;
    ZPS_teStatus eStatus;

    uDstAddr.u16Addr = u16DstAddr;

    hAPduInst = PDUM_hAPduAllocateAPduInstance(apduZDP);
    if (PDUM_INVALID_HANDLE != hAPduInst)
    {
        uint8_t u8SeqNumber;
        sAplZdpBindUnbindReq.u64SrcAddress = ZPS_u64AplZdoLookupIeeeAddr(u16DstAddr);
        sAplZdpBindUnbindReq.u8SrcEndpoint = APP_u8GetDeviceEndpoint();
        sAplZdpBindUnbindReq.u16ClusterId = u16ClusterId;


        sAplZdpBindUnbindReq.u8DstAddrMode = E_ZCL_AM_IEEE;
        sAplZdpBindUnbindReq.uAddressField.sExtended.u64DstAddress = ZPS_u64NwkNibGetExtAddr(ZPS_pvAplZdoGetNwkHandle() );
        sAplZdpBindUnbindReq.uAddressField.sExtended.u8DstEndPoint = u8DstEp;

        DBG_vPrintf(TRACE_APP, "Remote Bind Dst addr %04x, Ieee Dst Addr %016llx Ieee Src %016llx\r\n",
                uDstAddr.u16Addr,
                sAplZdpBindUnbindReq.uAddressField.sExtended.u64DstAddress,
                sAplZdpBindUnbindReq.u64SrcAddress);

        eStatus = ZPS_eAplZdpBindUnbindRequest(hAPduInst,
                                               uDstAddr,
                                               FALSE,
                                               &u8SeqNumber,
                                               TRUE,
                                               &sAplZdpBindUnbindReq);
        DBG_vPrintf(TRACE_APP, "Sending a remote bind request Status =%x\r\n", eStatus);
        if (eStatus)
        {
            PDUM_eAPduFreeAPduInstance(hAPduInst);
        }
    }
}

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
    ZPS_vSetKeys();
    ZPS_vSaveAllZpsRecords();

    /* save everything */
    eNodeState = E_STARTUP;
    PDM_eSaveRecordData(PDM_ID_APP_COORD,&eNodeState,sizeof(teNodeState));
    APP_vSetLed(APP_E_LEDS_LED_2, APP_E_LED_OFF);
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

    sBDB.sAttrib.bbdbNodeIsOnANetwork = ((eNodeState >= E_RUNNING)?(TRUE):(FALSE));
    sInitArgs.hBdbEventsMsgQ = &APP_msgBdbEvents;
    if(eNodeState >= E_RUNNING)
    {
    	APP_vSetLed(APP_E_LEDS_LED_1, APP_E_LED_OFF);
    	APP_vSetLed(APP_E_LEDS_LED_2, APP_E_LED_ON);
    }
    else
    {
    	APP_vSetLed(APP_E_LEDS_LED_1, APP_E_LED_OFF);
        APP_vSetLed(APP_E_LEDS_LED_2, APP_E_LED_OFF);
    }
    BDB_vInit(&sInitArgs);
    sBDB.sAttrib.u32bdbPrimaryChannelSet = BDB_PRIMARY_CHANNEL_SET;
    sBDB.sAttrib.u32bdbSecondaryChannelSet = BDB_SECONDARY_CHANNEL_SET;
}


/****************************************************************************
 *
 * NAME: APP_eGetCurrentApplicationState
 *
 * DESCRIPTION:
 * Returns the current state of the application
 *
 * RETURNS:
 * teNodeState
 *
 ****************************************************************************/
teNodeState APP_eGetCurrentApplicationState (void)
{
    return eNodeState;
}


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
    return COORDINATOR_APPLICATION_ENDPOINT;
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
