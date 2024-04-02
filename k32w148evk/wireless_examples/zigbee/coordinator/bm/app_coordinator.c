/*
* Copyright 2019, 2023 NXP
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

#ifdef NCP_HOST
#include "serial_link_ctrl.h"
#include "serial_link_cmds_ctrl.h"
#endif


/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifndef TRACE_APP
#define TRACE_APP FALSE
#endif

#ifndef TRACE_APP_INIT
#define TRACE_APP_INIT FALSE
#endif

#ifndef MAX_HOST_TO_COPROCESSOR_COMMS_ATTEMPS
#define MAX_HOST_TO_COPROCESSOR_COMMS_ATTEMPS (5)
#endif
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

static void vAppHandleAfEvent( BDB_tsZpsAfEvent *psZpsAfEvent);
static void vAppHandleZdoEvents( BDB_tsZpsAfEvent *psZpsAfEvent);
static void vAppSendOnOff(void);
static void vAppSendIdentifyStop( uint16_t u16Address, uint8_t u8Endpoint);
static void vAppSendRemoteBindRequest(uint16_t u16DstAddr, uint16_t u16ClusterId, uint8_t u8DstEp);
static void APP_vBdbInit(void);
#ifdef NCP_HOST
void APP_vProcessZCLMessage(uint32_t u32Msg);
#endif

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
extern uint32_t u32Togglems;


/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

static teNodeState eNodeState;
#ifdef NCP_HOST
PRIVATE bool_t bZCLQueueFull = FALSE;
#endif
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
#ifndef NCP_HOST
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

#if !(defined(ENABLE_SUBG_IF))
    APP_SetHighTxPowerMode();
#endif

    /* Initialise ZBPro stack */
    ZPS_eAplAfInit();

#ifdef ENABLE_SUBG_IF
    void *pvNwk = ZPS_pvAplZdoGetNwkHandle();

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
#else
void APP_vInitialiseCoordinator(void)
{
    /* Restore any application data previously saved to flash */
    eNodeState = E_STARTUP;
    uint16_t u16ByteRead;
    PDM_eReadDataFromRecord(PDM_ID_APP_COORD,
                            &eNodeState,
                            sizeof(eNodeState),
                            &u16ByteRead);

    APP_ZCL_vInitialise();

    /* Initialise other software modules
     * HERE
     */
    APP_eZbModuleInitialise();
    APP_vBdbInit();
    DBG_vPrintf(TRACE_APP, "Recovered Application State %d On Network %d\r\n",
            eNodeState, sBDB.sAttrib.bbdbNodeIsOnANetwork);
}
#endif

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
#ifndef NCP_HOST
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
#endif
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
#ifndef NCP_HOST
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
#endif
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
static void vAppHandleZdoEvents( BDB_tsZpsAfEvent *psZpsAfEvent)
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

#ifdef NCP_HOST
    DBG_vPrintf(TRUE, "Erasing PDM data on Coprocessor...");
    u8ErasePersistentData();

    /* wait for coprocessor to be ready */
    vSetJNState(JN_NOT_READY);
    vWaitForJNReady(JN_READY_TIME_MS);
    vSL_SetStandardResponsePeriod();

    /* handle NCP HOST side */
    PDM_vDeleteAllDataRecords();
    APP_vNcpHostReset();
#endif
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

#ifdef NCP_HOST
/********************************************************************************
  *
  * @fn PRIVATE void APP_vNcpMainTask
  *
  */
 /**
  *
  * @param p_arg void *
  *
  * @brief Main State Machine for local Node
  *
  * @return void
  *
  * @note
  *
  * imported description
 ********************************************************************************/
void APP_vNcpMainTask(void)
{
    bool pBT;
    uint32_t u32Msg;

    /* handle serial messages  */
    (void)vSL_CheckAndHandleSerialMsg();

    /* handle zcl messages  */
    pBT = ZQ_bQueueReceive((void *)&zclQueueHandle, (void*)&u32Msg);

    if (bZCLQueueFull == (bool_t)TRUE)
    {
        DBG_vPrintf((bool_t)TRUE, "Reading ZCL Queue After Post Fail Main Task %d\n", pBT);
        bZCLQueueFull = (bool_t)FALSE;
    }

    if(pBT == TRUE)
    {
        APP_vProcessZCLMessage(u32Msg);
    }
    /* handle application messages */
    pBT = ZQ_bQueueReceive((void *)&appQueueHandle, (void*)&u32Msg);

    if(pBT == TRUE)
    {
            if ((u32Msg & QUEUE_MSG_BY_VALUE) == QUEUE_MSG_BY_VALUE )
            {
                uint32 u32LocalMsg =  u32Msg & (~QUEUE_MSG_BY_VALUE);
                vApp_ProcessMessageVal(u32LocalMsg);
            }
            else
            {
                vApp_ProcessMessage(u32Msg);
            }
    }
}

/********************************************************************************
 *
 * @fn PUBLIC void APP_vPostToAppQueue
 *
 */
/**
 *
 * @param pvMsg void *
 *
 * @brief post message to app queue
 *
 * @return None
 *
 * @note
 *
 * imported description
********************************************************************************/
PUBLIC void APP_vPostToAppQueue (void *pvMsg)
{
	if (FALSE == ZQ_bQueueSend(&appQueueHandle, pvMsg) )
	{
		vSL_FreeRxBuffer( (uint8*)pvMsg);
		APP_vVerifyFatal( (bool_t)FALSE, "Post to App queue failed", ERR_FATAL_ZIGBEE_RESTART);
	}
}

/********************************************************************************
  *
  * @fn PUBLIC void APP_vPostToAppQueueWord
  *
  */
 /**
  *
  * @param u8MsgType uint8
  *
  * @brief void
  *
  * @return PUBLIC void
  *
  * @note
  *
 ********************************************************************************/
PUBLIC void APP_vPostToAppQueueWord (uint8 u8MsgType )
{
    uint32 u32Msg = QUEUE_MSG_BY_VALUE | (uint32)u8MsgType;
	if (FALSE == ZQ_bQueueSend(&appQueueHandle, (void *)&u32Msg) )
	{
		vSL_FreeRxBuffer( (uint8*)&u32Msg);
		APP_vVerifyFatal( (bool_t)FALSE, "Post to App queue failed", ERR_FATAL_ZIGBEE_RESTART);
	}
}

/********************************************************************************
  *
  * @fn PUBLIC void APP_vPostToZclQueue
  *
  */
 /**
  *
  * @param pvMessage void *
  *
  * @brief post message to zcl queue
  *
  * @return None
  *
  * @note
  *
  * imported description
 ********************************************************************************/
PUBLIC void APP_vPostToZclQueue(void *pvMessage)
{
	if (FALSE == ZQ_bQueueSend(&zclQueueHandle, &pvMessage))
	{
		vSL_FreeRxBuffer( (uint8*)pvMessage);
		bZCLQueueFull = (bool_t)TRUE;
		APP_vVerifyFatal( (bool_t)FALSE, "Queue Send to ZCL failed", ERR_FATAL_ZIGBEE_RESTART);
	}
}

/********************************************************************************
  *
  * @fn PUBLIC void vFlushZclQueue
  *
  */
 /**
  *
  *
  * @brief void
  *
  * @return PUBLIC void
  *
  * @note
  *
 ********************************************************************************/
PUBLIC void vFlushZclQueue(void)
{
	(void)ZQ_bQueueFlush(&zclQueueHandle);
}

/********************************************************************************
  *
  * @fn PUBLIC void vFlushAppQueue
  *
  */
 /**
  *
  *
  * @brief void
  *
  * @return PUBLIC void
  *
  * @note
  *
 ********************************************************************************/
PUBLIC void vFlushAppQueue(void)
{
	(void)ZQ_bQueueFlush(&appQueueHandle);
}

/****************************************************************************
 *
 * NAME: APP_eZbModuleInitialise
 *
 * DESCRIPTION:
 * Initialises the ZB module
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC ZPS_teStatus APP_eZbModuleInitialise(void)
{
    teSL_ZigBeeDeviceType eDeviceType = E_SL_DEVICE_INVALID;
    teSL_DeviceMacType eMacType = E_SL_MAC_UNDEFINED;
    ZPS_teStatus 	eStatus = 0U;
    uint32	u32ZbVersion = 0U;
    uint16	u16OptionMask = 0U;
    uint32  u32SDKVersion = 0U;

    vSL_SetLongResponsePeriod();

    DBG_vPrintf((bool_t)TRACE_APP_INIT, "Zigbee Module initialization\r\n");

//    vSL_ResetRxBufferPool();

    DBG_vPrintf((bool_t)TRACE_APP_INIT, "Reading Zigbee Module Version Number\r\n");
    uint32 u32Count = 0U;
    while(u32Count < MAX_HOST_TO_COPROCESSOR_COMMS_ATTEMPS)
    {
        eStatus = u8GetVersionAndDeviceType(&u32ZbVersion, &eDeviceType,
                                            &eMacType, &u16OptionMask,
                                            &u32SDKVersion);
        if((eStatus == (ZPS_teStatus)E_SL_MSG_STATUS_BUSY) ||
           (eStatus == (ZPS_teStatus)E_SL_MSG_STATUS_TIME_OUT))
        {
            DBG_vPrintf((bool_t)TRACE_APP_INIT, "Zigbee module busy %d\n", u32Count++);
            vSleep(1000UL);
        }
        else
        {
            break;
        }
    }

    if(eStatus == (ZPS_teStatus)E_ZCL_SUCCESS)
    {
        DBG_vPrintf ((bool_t)TRACE_APP_INIT, "Success: ");
        DBG_vPrintf((bool_t)TRACE_APP_INIT,
                "Version number of Zigbee Module FW 0x%08x Mac type %d Options Mask 0x%04x SDK Version = %d\n",
                u32ZbVersion, eMacType, u16OptionMask, u32SDKVersion);
    }
    else if(eStatus == (ZPS_teStatus)E_SL_MSG_STATUS_HARDWARE_FAILURE)
    {
        DBG_vPrintf((bool_t)TRACE_APP_INIT, "Coprocessor module hardware failure\n");
        APP_vVerifyFatal((bool_t)FALSE, "Fatal error, hardware failure on Coprocessor module", ERR_FATAL_NON_RECOVERABLE);
    }
    else
    {
        DBG_vPrintf((bool_t)TRUE,
                "Error: Status code from attempt to get version number of Zigbee Module %d\r\n",
                eStatus);
        APP_vVerifyFatal((bool_t)FALSE, "Fatal error, cannot continue", ERR_FATAL_NON_RECOVERABLE);
    }

    if (eNodeState == E_STARTUP)
    {
        DBG_vPrintf((bool_t)TRACE_APP_INIT, "Erasing Persistent Data on Zigbee Module\n");
        eStatus = u8ErasePersistentData();
        if(eStatus == (ZPS_teStatus)E_ZCL_SUCCESS)
        {
            DBG_vPrintf((bool_t)TRACE_APP_INIT,
                    "Success: Erasing Persistent Data on Zigbee Module\n");
        }
        else
        {
            DBG_vPrintf((bool_t)TRUE,
                    "Error: Erasing Persistent Data on Zigbee Module 0x%x\r\n", eStatus);
            vSL_SetStandardResponsePeriod();
            return eStatus;
        }
        /* wait for Zigbee module to stabilize after PDM erase */
        vSetJNState(JN_NOT_READY);
        vWaitForJNReady(JN_READY_TIME_MS);

        /* wait for Zigbee module to stabilize after Reset */
        vSetJNState(JN_NOT_READY);
        APP_vNcpHostResetZigBeeModule();
        vWaitForJNReady(JN_READY_TIME_MS);
    }
    vSL_SetStandardResponsePeriod();
    return eStatus;
}

/********************************************************************************
  *
  * @fn PUBLIC void APP_vProcessZCLMessage
  *
  */
 /**
  *
  * @param u32Msg uint32
  *
  * @brief process ZCL messages
  *
  * @return void
  *
  * @note
  *
  * imported description
 ********************************************************************************/
PUBLIC void APP_vProcessZCLMessage(uint32 u32Msg)
{
    tsZCL_CallBackEvent  sCallBackEvent;
    ZPS_tsAfEvent sStackEvent;
#ifdef APP_ENABLE_PRINT_BUFFERS
    static uint16 u16appPrintBufferTimeInSecCount = 0U;
#endif

    if(*((uint8*)u32Msg) == (uint32)APP_MSG_TYPE_ZCL_TIMER)
    {

        sCallBackEvent.eEventType = E_ZCL_CBET_TIMER;
        vLockZCLMutex();
        //vZCL_SetUTCTimeWoSyncSet(APP_u32GetTime() - 1U);
        vZCL_EventHandler(&sCallBackEvent);
        vUnlockZCLMutex();
        //vApp_HandleZclTimerEvent();
#ifdef APP_ENABLE_PRINT_BUFFERS
        if(u16appPrintBufferTimeInSec>0){
            u16appPrintBufferTimeInSecCount++;
            if(u16appPrintBufferTimeInSecCount == u16appPrintBufferTimeInSec)
            {
                u16appPrintBufferTimeInSecCount = 0U;
                vSL_PrintRxBufferPool(TRUE);
                PDUM_vPrintAllocatedBuffers(TRUE);
            }
        }
#endif
    }
    else if(*((uint8*)u32Msg) == SL_MSG_TYPE_APDU)
    {
        PDUM_thAPduInstance myPDUM_thAPduInstance = PDUM_INVALID_HANDLE;
        uint8 u8EndPoint;

        /* clear StackEvent */
        (void)ZBmemset(&sStackEvent, 0x00, sizeof(ZPS_tsAfEvent));
        sCallBackEvent.eEventType = E_ZCL_CBET_ZIGBEE_EVENT;
        sCallBackEvent.pZPSevent = &sStackEvent;

        /* Process the serial buffer */
        vSL_HandleApduEvent((uint8*)u32Msg,&myPDUM_thAPduInstance, &sStackEvent);


        /* Before pushing the event to ZCL, construct the serial
         * payload as per stack event structure */
        if((sStackEvent.eType == (ZPS_teAfEventType)ZPS_EVENT_APS_DATA_INDICATION) ||
           (sStackEvent.eType == (ZPS_teAfEventType)ZPS_EVENT_APS_DATA_ACK))
        {
            bool bValidEp = (bool)TRUE;
            if (sStackEvent.eType == (ZPS_teAfEventType)ZPS_EVENT_APS_DATA_INDICATION)
            {
                u8EndPoint = sStackEvent.uEvent.sApsDataIndEvent.u8DstEndpoint;
                (void)ZPS_eAplAfGetEndpointState(sStackEvent.uEvent.sApsDataIndEvent.u8DstEndpoint, &bValidEp);
            }
            else /* sStackEvent.eType ==(ZPS_teAfEventType)ZPS_EVENT_APS_DATA_ACK */
            {
                u8EndPoint = sStackEvent.uEvent.sApsDataAckEvent.u8DstEndpoint;
                (void)ZPS_eAplAfGetEndpointState(sStackEvent.uEvent.sApsDataAckEvent.u8DstEndpoint, &bValidEp);
            }

            if (bValidEp)
            {
                if (u8EndPoint == COORDINATOR_ZDO_ENDPOINT) {
                    vAppHandleZdoEvents(&sStackEvent);
                } else {
                    vLockZCLMutex();
                    /* post to the ZCL as Event */
                    vZCL_EventHandler(&sCallBackEvent);
                    vUnlockZCLMutex();
                }
            }
            else
            {
                if (sStackEvent.eType ==(ZPS_teAfEventType)ZPS_EVENT_APS_DATA_INDICATION)
                {
                    DBG_vPrintf((bool_t)TRUE, "Data Indication for unsupported end point %d\n",
                          sStackEvent.uEvent.sApsDataIndEvent.u8DstEndpoint);
                }
                else
                {
                    DBG_vPrintf((bool_t)TRUE, "Data Ack for unspported end point %d\n",
                          sStackEvent.uEvent.sApsDataAckEvent.u8DstEndpoint);
                }
            }
         }
        if (myPDUM_thAPduInstance != PDUM_INVALID_HANDLE)
        {
            (void)PDUM_eAPduFreeAPduInstance(myPDUM_thAPduInstance);
        }
    }
    else if(*((uint8*)u32Msg) == SL_MSG_TYPE_INTERPAN)
    {
        PDUM_thAPduInstance myPDUM_thAPduInstance = PDUM_INVALID_HANDLE;
        /* clear StackEvent */
        (void)ZBmemset(&sStackEvent, 0x00, sizeof(ZPS_tsAfEvent));
        sCallBackEvent.eEventType = E_ZCL_CBET_ZIGBEE_EVENT;
        sCallBackEvent.pZPSevent = &sStackEvent;

        /* Process the serial buffer */
        vSL_HandleInterpanEvent((uint8*)u32Msg, &myPDUM_thAPduInstance, &sStackEvent);

        /* Before pushing the event to ZCL, construct the serial payload as per
         * stack event structure */
        if((sStackEvent.eType == (ZPS_teAfEventType)ZPS_EVENT_APS_INTERPAN_DATA_INDICATION)||
                (sStackEvent.eType == (ZPS_teAfEventType)ZPS_EVENT_APS_INTERPAN_DATA_CONFIRM))
        {
            /* Hook to handle raw GB spec inter pan messages and drop InterPan CBKE unless in correct state */
            //if ((bool_t)TRUE == bPassInterPanToZcl(&sStackEvent))
            {
                /* post to the ZCL as Event */
                vZCL_EventHandler(&sCallBackEvent);
            }
        }
        if (myPDUM_thAPduInstance != PDUM_INVALID_HANDLE)
        {
            (void)PDUM_eAPduFreeAPduInstance(myPDUM_thAPduInstance);
        }
    }
    else
    {
        /*nodefault action required */
    }

    //APP_vDirtyTimerHandler(u32Msg);
}

#endif /* NCP_HOST */
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
