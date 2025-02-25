/*
* Copyright 2019-2024 NXP
* All rights reserved.
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
#include "app_end_device_node.h"
#include "zigbee_config.h"
#include "ZTimer.h"
#include "PWR_Interface.h"
#include "pwrm.h"
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

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


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
static void vAppHandleAfEvent( BDB_tsZpsAfEvent *psZpsAfEvent);
static void vAppHandleZdoEvents( BDB_tsZpsAfEvent *psZpsAfEvent);
static void APP_vBdbInit(bool_t bColdStart);
#if !defined(K32W2ARD_ITPS)
static void vDeletePDMOnButtonPress(uint8_t u8ButtonID);
#endif
static void vPrintAPSTable(void);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
tsDeviceDesc sDeviceDesc;
#ifdef ZIGBEE_EVENT_IMPL
extern PWR_tsWakeTimerEvent sZTimer;
#endif

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
uint32_t u32OldFrameCtr;
PWR_tsWakeTimerEvent sWake;
uint32 u32PollTime = 0;
bool_t bDisallowSleep = FALSE;
#ifdef OT_ZB_SUPPORT
extern uint8 u8TimerScan;
extern uint8 u8TimerFb;
#endif

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

uint8 au8TestTlvs[sizeof(au8Storage_Tlv1) + sizeof(au8Storage_Tlv2) +
                  sizeof(au8Storage_Tlv3) + sizeof(g_Tlv4)];
uint8 au8JoinTlvs[sizeof(au8Storage_Tlv1) + sizeof(au8Storage_Tlv2) +
                  sizeof(au8Storage_Tlv3) + sizeof(g_sJoinerTlvs)];


#endif
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

extern void zps_vNwkSecClearMatSet(void *psNwk);

/****************************************************************************
 *
 * NAME: APP_vInitialiseEndDevice
 *
 * DESCRIPTION:
 * Initialises the application related functions
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void APP_cbTimerPoll( void *pvParam)
{

#ifdef OT_ZB_SUPPORT
    /* Set poll scheduler policy */
    APP_vSetSchedPolicy(ZB_PROTO_ID, E_STATE_ZB_APP_POLL);
#endif

    ZPS_eAplZdoPoll();
    ZTIMER_eStop(u8TimerPoll);
    if(ZTIMER_eStart(u8TimerPoll, POLL_TIME_FAST) != E_ZTIMER_OK)
    {
        DBG_vPrintf(TRACE_APP, "\r\nAPP: Failed to start poll");
    }
    switch(sDeviceDesc.eNodeState)
    {
    case E_RUNNING:
        /* poll MAX_POLLS times and then schedule regular sleep */
    {
        if(!bDisallowSleep)
        {
            u32PollTime++; /* We are polling 250 mSecs */
            if(u32PollTime > ZED_MAX_POLLS)
            {
                /*need to also stop any other timers here too if sleep is intended */
                ZTIMER_vStopAllTimers();
#ifdef ZIGBEE_EVENT_IMPL
                /* need to stop timer that run ZTIMER task */
                PWRM_eRemoveActivity(&sZTimer);
#endif
                u32PollTime = 0;
                /*
                 * Will wake after ZED_SLEEP_PERIOD ms if sleep is allowed or you
                 * still get the vWakeCallBack
                 */
                memset( &sWake, 0x0, sizeof(PWR_tsWakeTimerEvent));
                PWR_eScheduleActivity(&sWake, ZED_SLEEP_PERIOD, vWakeCallBack);
            }
        }
    }
    break;
    default:
        break;
    }
}


/****************************************************************************
 *
 * NAME: APP_vInitialiseEndDevice
 *
 * DESCRIPTION:
 * Initialises the application related functions
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void APP_vInitialiseEndDevice(bool_t bColdStart)
{
   uint16 u16ByteRead;
   PDM_teStatus eStatusReportReload;

   if (bColdStart)
   {
#ifdef CLD_OTA
        vLoadOTAPersistedData();
#endif
        sDeviceDesc.eNodeState = E_STARTUP;
        PDM_eReadDataFromRecord(PDM_ID_APP_END_DEVICE,
                                &sDeviceDesc,
                                sizeof(tsDeviceDesc),
                                &u16ByteRead);

       /* Restore any report data that is previously saved to flash */
       eStatusReportReload = eRestoreReports();
   }
   /* Initialise LEDs and buttons */
   APP_vLedInitialise();
   APP_vSetLed(APP_E_LEDS_LED_1, APP_E_LED_OFF);
   APP_vSetLed(APP_E_LEDS_LED_2, APP_E_LED_OFF);
#ifndef USART1_FTDI
   /* GPIO1 is used for USART1 RX */
   APP_bButtonInitialise();
#endif
   ZPS_psAplAibGetAib()->bUseInstallCode = BDB_JOIN_USES_INSTALL_CODE_KEY;

#ifdef R23_UPDATES
    ZPS_vTlvBuildSequence(4, sizeof(au8JoinTlvs), au8JoinTlvs,
            g_pTlv1, g_pTlv2, g_pTlv3, &g_sJoinerTlvs);

    ZPS_vAplAfSetAdditionalTlvs(au8JoinTlvs, sizeof(au8JoinTlvs));
#endif
   if (bColdStart)
   {
       APP_SetHighTxPowerMode();

       /* Initialise ZBPro stack */
       ZPS_eAplAfInit();
   }
   else
   {
       /* ReInitialise ZBPro stack */
        ZPS_eAplAfReInit();
   }

   APP_SetMaxTxPower();

   /* Initialise ZCL */
   APP_ZCL_vInitialise(bColdStart);

   /* Initialise other software modules
    * HERE
    */
   APP_vBdbInit(bColdStart);

   if (bColdStart)
   {
#if !defined(K32W2ARD_ITPS)
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
	}
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
    BDB_teStatus eStatus;

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
#ifdef OT_ZB_SUPPORT
                APP_vSetSchedPolicy(ZB_PROTO_ID, E_STATE_ZB_SCAN_NETWORKS);
#endif
                eStatus = BDB_eNsStartNwkSteering();
                DBG_vPrintf(TRACE_APP, "BDB Try Steering status %d\r\n",eStatus);
            }
            else
            {
                DBG_vPrintf(TRACE_APP, "BDB Init go Running");
                sDeviceDesc.eNodeState = E_RUNNING;
                PDM_eSaveRecordData(PDM_ID_APP_END_DEVICE,&sDeviceDesc,sizeof(tsDeviceDesc));
            }
            break;

        case BDB_EVENT_NO_NETWORK:
            DBG_vPrintf(TRACE_APP,"APP: BDB No Networks\r\n");
#ifdef OT_ZB_SUPPORT
            DBG_vPrintf(TRACE_APP,"APP: Start scan timer\r\n");
            if (ZTIMER_eGetState(u8TimerScan) != E_ZTIMER_STATE_RUNNING)
            {
                ZTIMER_eStart(u8TimerScan, STEER_RESTART_TIME);
            }
#else
            eStatus = BDB_eNsStartNwkSteering();
#endif
            break;

        case BDB_EVENT_NWK_STEERING_SUCCESS:
            DBG_vPrintf(TRACE_APP,"APP: NwkSteering Success \r\n");
            sDeviceDesc.eNodeState = E_RUNNING;
            PDM_eSaveRecordData(PDM_ID_APP_END_DEVICE,&sDeviceDesc,sizeof(tsDeviceDesc));
            APP_vSetLed(APP_E_LEDS_LED_2, APP_E_LED_ON);
#ifdef OT_ZB_SUPPORT
            ZTIMER_eStop(u8TimerScan);

            /* Start the F&B procedure after a while */
            ZTIMER_eStart(u8TimerFb, ZTIMER_TIME_SEC(ZED_FB_START_TIME));
#endif
            break;

        case BDB_EVENT_FB_OVER_AT_TARGET:
            bDisallowSleep = FALSE;
            break;
        default:
            break;
    }
}

/****************************************************************************
 *
 * NAME: APP_taskEndDevicNode
 *
 * DESCRIPTION:
 * Task that handles application related functions
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void APP_taskEndDevicNode(void)
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
            BDB_teStatus eStatus;
                case APP_E_BUTTONS_BUTTON_1:
                        DBG_vPrintf(TRACE_APP_EVENT, "APP_EVENT: Network steering and F&B as Target\r\n");
                        eStatus = BDB_eNsStartNwkSteering();
                        if (eStatus != 0)
                        {
                            DBG_vPrintf(TRACE_APP_EVENT, "APP_EVENT: Network Steering %02x\r\n", eStatus);
                        }
                        eStatus = BDB_eFbTriggerAsTarget(APP_u8GetDeviceEndpoint());
                        if (eStatus != 0 && eStatus != 9)
                        {
                            DBG_vPrintf(TRACE_APP_EVENT, "APP_EVENT: Fiind and Bind Failed %02x\r\n", eStatus);
                        }
#ifndef OT_ZB_SUPPORT
                        bDisallowSleep = TRUE;
#endif
                    break;

                default:
                    break;
            }
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
        case ZPS_EVENT_NWK_JOINED_AS_ENDDEVICE:
            if(ZTIMER_eStart(u8TimerPoll, POLL_TIME_FAST) != E_ZTIMER_OK)
            {
                DBG_vPrintf(TRACE_APP, "\r\nAPP: Failed to start poll");
            }
            APP_vSetLed(APP_E_LEDS_LED_2, APP_E_LED_ON);
            DBG_vPrintf(TRUE,"APP-ZDO: Joined nwk with address %04x\r\n", ZPS_u16NwkNibGetNwkAddr(ZPS_pvAplZdoGetNwkHandle()));
            break;
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

        case ZPS_EVENT_NWK_FAILED_TO_JOIN:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: Failed To Join %02x Rejoin %d\r\n",
                    psAfEvent->uEvent.sNwkJoinFailedEvent.u8Status,
                    psAfEvent->uEvent.sNwkJoinFailedEvent.bRejoin);
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
                APP_vFactoryResetRecords();
                MICRO_DISABLE_INTERRUPTS();
                RESET_SystemReset();
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
                APP_vFactoryResetRecords();
                MICRO_DISABLE_INTERRUPTS();
                RESET_SystemReset();
            }
            break;

        case ZPS_EVENT_NWK_STATUS_INDICATION:
            DBG_vPrintf(TRACE_APP, "APP-ZDO: Network status Indication %02x addr %04x\r\n",
                    psAfEvent->uEvent.sNwkStatusIndicationEvent.u8Status,
                    psAfEvent->uEvent.sNwkStatusIndicationEvent.u16NwkAddr);
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

#ifdef OT_ZB_SUPPORT
        case ZPS_EVENT_NWK_POLL_CONFIRM:
            /* MAC_ENUM_NO_DATA == no data pending, relinquish the time slice */
            if (MAC_ENUM_NO_DATA ==
                    psAfEvent->uEvent.sNwkPollConfirmEvent.u8Status)
                APP_vSetSchedPolicy(ZB_PROTO_ID, E_STATE_ZB_APP_ACTIVE);
            break;
#endif
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
    PDM_eSaveRecordData(PDM_ID_APP_END_DEVICE,&sDeviceDesc,sizeof(tsDeviceDesc));
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
static void APP_vBdbInit(bool_t bColdStart)
{
    BDB_tsInitArgs sInitArgs;

    sBDB.sAttrib.bbdbNodeIsOnANetwork = ((sDeviceDesc.eNodeState >= E_RUNNING)?(TRUE):(FALSE));
    sInitArgs.hBdbEventsMsgQ = &APP_msgBdbEvents;
    BDB_vInit(&sInitArgs);
    if(sDeviceDesc.eNodeState == E_STARTUP)
    {
       	APP_vSetLed(APP_E_LEDS_LED_1, APP_E_LED_OFF);
        APP_vSetLed(APP_E_LEDS_LED_2, APP_E_LED_OFF);
    }
    else
    {
        APP_vSetLed(APP_E_LEDS_LED_2, APP_E_LED_ON);
    }
}

#if !defined(K32W2ARD_ITPS)
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
static void vDeletePDMOnButtonPress(uint8_t u8ButtonID)
{
    bool_t bDeleteRecords = FALSE;
    uint8_t u8Status;

    bool_t bButtonPressed = APP_u32GetButtonsState() & (1 << u8ButtonID);

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
            RESET_SystemReset();
        } else { DBG_vPrintf(TRACE_APP, "RESET: Sent Leave\r\n"); }
    }
}
#endif /* !defined(K32W2ARD_ITPS) */

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
#ifdef CLD_OTA
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
    return ENDDEVICE_RXOFF_APPLICATION_ENDPOINT;
}

#ifdef OT_ZB_SUPPORT
/****************************************************************************
 *
 * NAME: APP_cbTimerScan
 *
 * DESCRIPTION:
 * Steering timer callback, called if the network steering fails
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void APP_cbTimerScan(void *pvParam)
{
    BDB_teStatus eStatus;

    if (sDeviceDesc.eNodeState != E_RUNNING)
    {
        APP_vSetSchedPolicy(ZB_PROTO_ID, E_STATE_ZB_SCAN_NETWORKS);
        eStatus = BDB_eNsStartNwkSteering();
        if (eStatus != BDB_E_SUCCESS)
        {
            DBG_vPrintf(TRUE, "APP_cbTimerScan: failed to start Nwk Steering");
        }
        ZTIMER_eStart(u8TimerScan, STEER_RESTART_TIME);
    }
}

void vNsTryNwkJoinAppCb(ZPS_tsNwkNetworkDescr  *pNwkDescr)
{
    APP_vSetSchedPolicy(ZB_PROTO_ID, E_STATE_ZB_NETWORK_ENTRY);
}

/****************************************************************************
 *
 * NAME: APP_cbTimerFb
 *
 * DESCRIPTION:
 * Find & Bind timer callback, called after steering the network
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void APP_cbTimerFb(void *pvParam)
{
    /* Button will hold the button ID */
    APP_tsEvent sButtonEvent;

    sButtonEvent.eType = APP_E_EVENT_BUTTON_DOWN;
    sButtonEvent.uEvent.sButton.u8Button = APP_E_BUTTONS_BUTTON_1;

    if(ZQ_bQueueSend(&APP_msgAppEvents, &sButtonEvent) == FALSE)
    {
        DBG_vPrintf(FALSE, "Button: Failed to post Event %d \r\n", sButtonEvent.eType);
    }
}
#endif

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
    return sDeviceDesc.eNodeState;
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
