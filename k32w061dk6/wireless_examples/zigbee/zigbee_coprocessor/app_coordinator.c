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
#include "app_leds.h"
#include "app.h"

#include "zps_apl_aib.h"
#include "zps_nwk_pub.h"
#include "zps_apl_af.h"
#include "serial_link_wkr.h"
#include "serial_link_cmds_wkr.h"
#include "app_serial_commands.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifndef TRACE_APP
#define TRACE_APP FALSE
#endif

#ifndef TRACE_EVENT_HANDLER
#define TRACE_EVENT_HANDLER FALSE
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

PRIVATE void vUpdateDeviceStats(uint64 u64SourceAddress, uint8 u8Lqi);
PUBLIC void vSendJoinedFormEventToHost(ZPS_tsAfEvent* psStackEvent, uint8* pu8Buffer);
PUBLIC void vHandleZpsErrorEvent(ZPS_tsAfErrorEvent *psErrEvt, uint8 * pu8Buffer);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
extern uint32_t u32Togglems;

extern uint8 u8LastMsgLqi; /* last message received lqi */

PUBLIC tsNcpDeviceDesc sNcpDeviceDesc = {FACTORY_NEW, E_STARTUP, ZPS_ZDO_DEVICE_COORD, 0x0000000000000002UL, FALSE};

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

#define GREEN_POWER_ENDPOINT                   242

PUBLIC bool_t bDataPending = FALSE;
PUBLIC bool_t bRejoining = FALSE;
PUBLIC bool_t bRejoinInProgress = FALSE;
PUBLIC uint8 u8ConsecLinkStatusFails = 0U;

extern uint8 u8Error;
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
extern PUBLIC void APP_vGetStackData ( void );
extern PUBLIC void APP_vResetDeviceState(void);
extern PUBLIC void APP_vRestoreStackData ( void );
extern PUBLIC uint8 u8GetBeaconCount(void);
extern PUBLIC void vResetBeaconCount(void);
extern void vSendDiscoveryNoNetworks(void);
extern bool_t vNfTcCallback (uint16 u16ShortAddress,
                             uint64 u64DeviceAddress,
                             uint64 u64ParentAddress,
                             uint8 u8Status,
                             uint16 u16MacId );
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
    sNcpDeviceDesc.eState = FACTORY_NEW;
    sNcpDeviceDesc.eNodeState = E_STARTUP;

    uint16_t u16ByteRead;

    /* If not first boot, read PDM data, else initialise PDM data with default value */
    if(PDM_bDoesDataExist(PDM_ID_APP_COORD,&u16ByteRead))
    {
        DBG_vPrintf(TRACE_APP, "Not first boot, query PDM records\r\n");
        PDM_eReadDataFromRecord(PDM_ID_APP_COORD,
                        &sNcpDeviceDesc,
                        sizeof(tsNcpDeviceDesc),
                        &u16ByteRead);
    }
    else
    {
        DBG_vPrintf(TRACE_APP, "First boot, create default PDM records\r\n");
        PDM_eSaveRecordData(PDM_ID_APP_COORD, &sNcpDeviceDesc, sizeof(tsNcpDeviceDesc));
    }

    /* Initialise ZBPro stack */
    ZPS_eAplAfInit();

#ifdef ENABLE_SUBG_IF
    void *pvNwk = ZPS_pvAplZdoGetNwkHandle();

    ZPS_vNwkNibSetMacEnhancedMode(pvNwk, TRUE);
    ZPS_u8MacMibIeeeSetPolicy(FALSE);
#endif

    /* Register trust center callback */
    ZPS_vTCSetCallback(vNfTcCallback);

    /* Initialise other software modules
     * HERE
     */

    DBG_vPrintf(TRACE_APP, "Recovered Application State %d \r\n", sNcpDeviceDesc.eNodeState);

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
    ZPS_vSetKeys();
    ZPS_vSaveAllZpsRecords();
    /* save everything */
    sNcpDeviceDesc.eState = FACTORY_NEW;

    PDM_eSaveRecordData(PDM_ID_APP_COORD,&sNcpDeviceDesc,sizeof(tsNcpDeviceDesc));
#if !defined(K32W1480_SERIES) && !defined(MCXW716A_SERIES) && !defined(MCXW716C_SERIES)
    APP_vSetLed(LED2, OFF);
#endif
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
    return sNcpDeviceDesc.eNodeState;
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

/****************************************************************************
 *
 * NAME: APP_vGenCallback
 *
 * DESCRIPTION:
 * Event handler called by ZigBee PRO Stack
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_vGenCallback(uint8 u8Endpoint, ZPS_tsAfEvent *psStackEvent)
{
    /* post a message */
    ZQ_bQueueSend(&sAPP_msgZpsEvents, psStackEvent);

    //DBG_vPrintf(TRACE_EVENT_HANDLER,"APP_msgZpsEvents Msg Posted. Event Type =%d\n",psStackEvent->eType);
}

#define MAX_PACKET_TX_SIZE  (1000)
PUBLIC void APP_ZpsEventTask(void)
{
    ZPS_tsAfEvent sStackEvent;
    uint16 u16Length = 0;
    uint8 au8StatusBuffer[MAX_PACKET_TX_SIZE];
    //uint8* pu8Buffer;
    bool_t bIsPktPosted = FALSE;

    uint64 u64SourceAddress;

    sStackEvent.eType = ZPS_EVENT_NONE;

    bIsPktPosted = ZQ_bQueueReceive(&sAPP_msgZpsEvents, &sStackEvent);

    if (bIsPktPosted)
    {

#if TRACE_EVENT_HANDLER == TRUE
        //DBG_vPrintf(TRACE_EVENT_HANDLER,"Msg Received. Event Type =%d\n",sStackEvent.eType);
        if (sStackEvent.eType == 15)
        {
            if ((sStackEvent.uEvent.sNwkPollConfirmEvent.u8Status != 0) &&
                    (sStackEvent.uEvent.sNwkPollConfirmEvent.u8Status != 0xeb))
            {
             //   DBG_vPrintf(TRACE_EVENT_HANDLER,"[RC] Poll Fail status = %02x\n",
             //           sStackEvent.uEvent.sNwkPollConfirmEvent.u8Status);
            }
        }
        else
        {
          //  DBG_vPrintf(TRACE_EVENT_HANDLER,"Msg Received. Event Type =%d\n",sStackEvent.eType);
        }
#endif
        uint16 u16EventType = sStackEvent.eType;

        switch (u16EventType)
        {
        case 0xF0:
            vSL_WriteMessage(E_SL_MSG_JEN_OS_ERROR, 1,NULL, &u8Error);
            break;
        case ZPS_EVENT_APS_DATA_INDICATION:
            u8LastMsgLqi = sStackEvent.uEvent.sApsDataIndEvent.u8LinkQuality;

            if(sStackEvent.uEvent.sApsDataIndEvent.u8SrcAddrMode == ZPS_E_ADDR_MODE_IEEE)
            {
                u64SourceAddress = sStackEvent.uEvent.sApsDataIndEvent.uSrcAddress.u64Addr;
            }
            else
            {
                u64SourceAddress = ZPS_u64AplZdoLookupIeeeAddr(sStackEvent.uEvent.sApsDataIndEvent.uSrcAddress.u16Addr);
            }

            uint16 u16Size = PDUM_u16APduInstanceGetPayloadSize( sStackEvent.uEvent.sApsDataIndEvent.hAPduInst);

            if (sNcpDeviceDesc.u8DeviceType == ZPS_ZDO_DEVICE_COORD)
            {
                /* update stats */
                sNwkStats.u32TotalRX++;
                vUpdateDeviceStats(u64SourceAddress, sStackEvent.uEvent.sApsDataIndEvent.u8LinkQuality);

                /* If we are the trust centre, check the device sending the message has completed KEC with us. */
                uint64 u64RemoteIeeeAddress;
                ZPS_teDevicePermissions u8DevicePermissions;
                uint8 u8Status;

                u64RemoteIeeeAddress = ZPS_u64NwkNibFindExtAddr(ZPS_pvNwkGetHandle(),
                        sStackEvent.uEvent.sApsDataIndEvent.uSrcAddress.u16Addr);
                u8Status =
                        ZPS_bAplZdoTrustCenterGetDevicePermissions(u64RemoteIeeeAddress,
                                                                &u8DevicePermissions);
                /* If status errror - not in table so assume full access allowed,
                * e.g. when working with HA profile and APS security. */
                if ( (u8Status == ZPS_E_SUCCESS) &&
                        ( (u8DevicePermissions & ZPS_DEVICE_PERMISSIONS_DATA_REQUEST_DISALLOWED)
                                == ZPS_DEVICE_PERMISSIONS_DATA_REQUEST_DISALLOWED) )
                {
                    sStackEvent.uEvent.sApsDataIndEvent.eSecurityStatus
                            = ZPS_APL_APS_E_SECURED_NWK_KEY;
                    DBG_vPrintf(TRACE_EVENT_HANDLER, "ZPS_APL_APS_E_SECURED_NWK_KEY  \n");
                }
            }

            if(sStackEvent.uEvent.sApsDataIndEvent.eStatus == ZPS_NWK_ENUM_INVALID_REQUEST && u8TempExtendedError != 0u)
            {
                /* We changed the status because of a resource related extended error*/
                sStackEvent.uEvent.sApsDataIndEvent.eStatus = E_SL_MSG_STATUS_C2_SUBSTITUTION;
            }

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataIndEvent.eStatus , u16Length );

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length],  sStackEvent.uEvent.sApsDataIndEvent.eSecurityStatus, u16Length );

            ZNC_BUF_U16_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataIndEvent.u16ProfileId , u16Length );

            ZNC_BUF_U16_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataIndEvent.u16ClusterId , u16Length );

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataIndEvent.u8SrcEndpoint , u16Length );

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataIndEvent.u8DstEndpoint , u16Length );

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length],sStackEvent.uEvent.sApsDataIndEvent.u8SrcAddrMode  , u16Length );

            if (sStackEvent.uEvent.sApsDataIndEvent.u8SrcAddrMode
                    == ZPS_E_ADDR_MODE_IEEE)
            {
                ZNC_BUF_U64_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataIndEvent.uSrcAddress.u64Addr , u16Length );
            }
            else
            {
                ZNC_BUF_U16_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataIndEvent.uSrcAddress.u16Addr , u16Length );
            }

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataIndEvent.u8DstAddrMode , u16Length );

            if (sStackEvent.uEvent.sApsDataIndEvent.u8DstAddrMode
                    == ZPS_E_ADDR_MODE_IEEE)
            {
                ZNC_BUF_U64_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataIndEvent.uDstAddress.u64Addr , u16Length );
            }
            else
            {
                ZNC_BUF_U16_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataIndEvent.uDstAddress.u16Addr , u16Length );
            }

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataIndEvent.u8LinkQuality , u16Length );

            ZNC_BUF_U32_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataIndEvent.u32RxTime , u16Length );

            /* Payload Length */
            ZNC_BUF_U16_UPD( &au8StatusBuffer[u16Length], u16Size , u16Length );

            if(sStackEvent.uEvent.sApsDataIndEvent.u8DstEndpoint == 0)
            {
                vSL_WriteMessageFromTwoBuffers(E_SL_MSG_ZDP_DATA_INDICATION, u16Length,NULL,
                                      au8StatusBuffer,sStackEvent.uEvent.sApsDataIndEvent.hAPduInst);
            }
            else
            {
                vSL_WriteMessageFromTwoBuffers(E_SL_MSG_DATA_INDICATION, u16Length,NULL,
                        au8StatusBuffer,sStackEvent.uEvent.sApsDataIndEvent.hAPduInst);
            }
        break;

        case ZPS_EVENT_APS_DATA_CONFIRM: {
            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataConfirmEvent.u8Status , u16Length );

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataConfirmEvent.u8SrcEndpoint , u16Length );

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataConfirmEvent.u8DstEndpoint , u16Length );

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataConfirmEvent.u8DstAddrMode , u16Length );

            if (ZPS_E_ADDR_MODE_IEEE
                    == sStackEvent.uEvent.sApsDataConfirmEvent.u8DstAddrMode)
            {
                ZNC_BUF_U64_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataConfirmEvent.uDstAddr.u64Addr , u16Length );
            } else
            {
                ZNC_BUF_U16_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataConfirmEvent.uDstAddr.u16Addr , u16Length );
            }

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataConfirmEvent.u8SequenceNum , u16Length );

            vSL_WriteMessage(E_SL_MSG_DATA_CONFIRM, u16Length,NULL, au8StatusBuffer);
            break;
        }

        case ZPS_EVENT_APS_DATA_ACK:
            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataAckEvent.u8Status , u16Length );

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataAckEvent.u8SrcEndpoint , u16Length );

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataAckEvent.u8DstEndpoint , u16Length );

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataAckEvent.u8DstAddrMode , u16Length );

            ZNC_BUF_U16_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataAckEvent.u16DstAddr , u16Length );

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataAckEvent.u8SequenceNum , u16Length );

            ZNC_BUF_U16_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataAckEvent.u16ProfileId , u16Length );

            ZNC_BUF_U16_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsDataAckEvent.u16ClusterId , u16Length );

            vSL_WriteMessage(E_SL_MSG_DATA_ACK, u16Length,NULL, au8StatusBuffer);
            
            if (ZPS_ZDO_DEVICE_COORD == sNcpDeviceDesc.u8DeviceType)
            {
                /* update stats */
                if(sStackEvent.uEvent.sApsDataAckEvent.u8Status == ZPS_E_SUCCESS)
                {
                    sNwkStats.u32TotalSuccessfulTX++;
                }
                else
                {
                    sNwkStats.u32TotalFailTX++;
                }
            }
            break;

        case ZPS_EVENT_APS_INTERPAN_DATA_INDICATION:
            if(sStackEvent.uEvent.sApsInterPanDataIndEvent.u8DstEndpoint == 1)
            {
                uint16 u16SourceAddress;
                uint8 u8DestAddressMode;
                uint16 u16Size = PDUM_u16APduInstanceGetPayloadSize( sStackEvent.uEvent.sApsInterPanDataIndEvent.hAPduInst);

                if(sStackEvent.uEvent.sApsDataIndEvent.eStatus == ZPS_NWK_ENUM_INVALID_REQUEST && u8TempExtendedError != 0u)
                {
                    /* We changed the status because of a resource related extended error*/
                    sStackEvent.uEvent.sApsDataIndEvent.eStatus = E_SL_MSG_STATUS_C2_SUBSTITUTION;
                }

                DBG_vPrintf(TRACE_EVENT_HANDLER, "ZPS_EVENT_APS_INTERPAN_DATA_INDICATION Status:%x\n",
                                    sStackEvent.uEvent.sApsInterPanDataIndEvent.eStatus);

                ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsInterPanDataIndEvent.eStatus , u16Length );

                ZNC_BUF_U16_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsInterPanDataIndEvent.u16ProfileId , u16Length );

                ZNC_BUF_U16_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsInterPanDataIndEvent.u16ClusterId , u16Length );

                ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsInterPanDataIndEvent.u8DstEndpoint , u16Length );

                ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsInterPanDataIndEvent.u8SrcAddrMode , u16Length );

                if (sStackEvent.uEvent.sApsInterPanDataIndEvent.u8SrcAddrMode
                        == ZPS_E_ADDR_MODE_IEEE) {
                    ZNC_BUF_U64_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsInterPanDataIndEvent.u64SrcAddress , u16Length );
                } else {
                    u16SourceAddress = sStackEvent.uEvent.sApsInterPanDataIndEvent.u64SrcAddress;
                    ZNC_BUF_U16_UPD( &au8StatusBuffer[u16Length], u16SourceAddress , u16Length );
                }

                ZNC_BUF_U16_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsInterPanDataIndEvent.u16SrcPan , u16Length );

                ZNC_BUF_U16_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsInterPanDataIndEvent.sDstAddr.u16PanId , u16Length );

                u8DestAddressMode = sStackEvent.uEvent.sApsInterPanDataIndEvent.sDstAddr.eMode;

                ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], u8DestAddressMode , u16Length );

                if (sStackEvent.uEvent.sApsInterPanDataIndEvent.sDstAddr.eMode == ZPS_E_AM_INTERPAN_IEEE)
                {
                    ZNC_BUF_U64_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsInterPanDataIndEvent.sDstAddr.uAddress.u64Addr , u16Length );
                }
                else
                {
                    ZNC_BUF_U16_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsInterPanDataIndEvent.sDstAddr.uAddress.u16Addr , u16Length );
                }

                ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsInterPanDataIndEvent.u8LinkQuality , u16Length );

                /* Payload Length */
                ZNC_BUF_U16_UPD( &au8StatusBuffer[u16Length], u16Size , u16Length );

                vSL_WriteMessageFromTwoBuffers(E_SL_MSG_INTERPAN_DATA_INDICATION, u16Length,NULL,
                                        au8StatusBuffer,sStackEvent.uEvent.sApsInterPanDataIndEvent.hAPduInst);
            }
            break;

        case ZPS_EVENT_APS_INTERPAN_DATA_CONFIRM:
            DBG_vPrintf(TRACE_EVENT_HANDLER, "ZPS_EVENT_APS_INTERPAN_DATA_CONFIRM Status:%x\n",
                    sStackEvent.uEvent.sApsInterPanDataConfirmEvent.u8Status);
            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsInterPanDataConfirmEvent.u8Status , u16Length );

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsInterPanDataConfirmEvent.u8Handle , u16Length );

            vSL_WriteMessage(E_SL_MSG_INTERPAN_DATA_CONFIRM, u16Length,NULL, au8StatusBuffer);
            break;

        case ZPS_EVENT_NWK_STATUS_INDICATION:
            DBG_vPrintf(TRACE_EVENT_HANDLER, "\nNwkStat: Addr:%x Status:%x",
                    sStackEvent.uEvent.sNwkStatusIndicationEvent.u16NwkAddr,
                    sStackEvent.uEvent.sNwkStatusIndicationEvent.u8Status);
            ZNC_BUF_U16_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkStatusIndicationEvent.u16NwkAddr , u16Length );

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkStatusIndicationEvent.u8Status , u16Length );

            vSL_WriteMessage(E_SL_MSG_NWK_STATUS_INDICATION, u16Length,NULL, au8StatusBuffer);
            break;

        case ZPS_EVENT_NWK_JOINED_AS_ENDDEVICE:
            DBG_vPrintf(TRACE_EVENT_HANDLER, "[RC] ZPS_EVENT_NWK_JOINED_AS_ENDDEVICE Rejoin %d Secure %d \n",
                    sStackEvent.uEvent.sNwkJoinedEvent.bRejoin,
                    sStackEvent.uEvent.sNwkJoinedEvent.bSecuredRejoin);

        case ZPS_EVENT_NWK_STARTED:
        case ZPS_EVENT_NWK_FAILED_TO_START:
        case ZPS_EVENT_NWK_JOINED_AS_ROUTER:
        {
            if (sStackEvent.eType == ZPS_EVENT_NWK_STARTED)
            {
                sNcpDeviceDesc.eNodeState = E_RUNNING;
                sNcpDeviceDesc.eState = NOT_FACTORY_NEW;
            }
            else if ( (sStackEvent.eType == ZPS_EVENT_NWK_JOINED_AS_ROUTER) ||
                    (sStackEvent.eType == ZPS_EVENT_NWK_JOINED_AS_ENDDEVICE  ))
            {
                sNcpDeviceDesc.eNodeState = E_RUNNING;
                sNcpDeviceDesc.eState = NOT_FACTORY_NEW;
                //uint8 i;
                bRejoinInProgress = FALSE;
                vResetBeaconCount();

                APP_vGetStackData();
            }


            if ((sStackEvent.eType == ZPS_EVENT_NWK_JOINED_AS_ROUTER)
                    || (sStackEvent.eType == ZPS_EVENT_NWK_JOINED_AS_ENDDEVICE)) {


                if (((sNcpDeviceDesc.eNodeState == E_STARTUP) ||
                     (sNcpDeviceDesc.eNodeState == E_NETWORK_INIT)) && sStackEvent.eType == ZPS_EVENT_NWK_JOINED_AS_ENDDEVICE)
                {
                    /*  reset frame counter */
                    uint8 i;
                    ZPS_tsAplAib *psAib = zps_psAplAibGetAib(ZPS_pvAplZdoGetAplHandle());

                    for (i = 0; i < psAib->psAplDeviceKeyPairTable->u16SizeOfKeyDescriptorTable; i++)
                    {
                        psAib->psAplDeviceKeyPairTable->psAplApsKeyDescriptorEntry[i].u32OutgoingFrameCounter = 0;
                    }



                }
                bRejoining = FALSE;
            }
            ZPS_vSaveAllZpsRecords();
            vSendJoinedFormEventToHost( &sStackEvent, au8StatusBuffer);
        }
        break;

        case ZPS_EVENT_NWK_FAILED_TO_JOIN:
            DBG_vPrintf(TRACE_EVENT_HANDLER, "ZPS_EVENT_NWK_FAILED_TO_JOIN -> %02x  Rejoin %d\n",
                    sStackEvent.uEvent.sNwkJoinFailedEvent.u8Status,
                    sStackEvent.uEvent.sNwkJoinFailedEvent.bRejoin);
            bRejoinInProgress = FALSE;
            if(sNcpDeviceDesc.eNodeState == E_DISCOVERY)
            {
                vSendDiscoveryNoNetworks();
            }
            else if  ((sNcpDeviceDesc.eNodeState == E_RUNNING))
            {
                uint8 u8Res = ZPS_eAplZdoTCRejoinNetworkTryNextParent();

                DBG_vPrintf(TRACE_EVENT_HANDLER, "Try next parent Res = %02x\n", u8Res);
                if ( 0 == u8Res  )
                {

                }
                else
                {
                    DBG_vPrintf(TRACE_EVENT_HANDLER, "No more parents Failed to Join call restore\n");
                    APP_vRestoreStackData();

                    vSendJoinedFormEventToHost( &sStackEvent, au8StatusBuffer);
                }
            }
            else
            {
                vSendJoinedFormEventToHost( &sStackEvent, au8StatusBuffer);
            }
            break;

        case ZPS_EVENT_ZDO_LINK_KEY: {
            ZNC_BUF_U64_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sZdoLinkKeyEvent.u64IeeeLinkAddr, u16Length );

            vSL_WriteMessage(E_SL_MSG_ZDO_LINK_KEY_EVENT, u16Length,NULL,
                    au8StatusBuffer);
        }
        break;
        case ZPS_EVENT_NWK_ROUTE_DISCOVERY_CONFIRM:
            ZNC_BUF_U16_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkRouteDiscoveryConfirmEvent.u16DstAddress, u16Length );

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkRouteDiscoveryConfirmEvent.u8Status , u16Length);

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkRouteDiscoveryConfirmEvent.u8NwkStatus , u16Length);
            vSL_WriteMessage(E_SL_MSG_NWK_ROUTE_DISCOVERY, u16Length,NULL, au8StatusBuffer);
        break;

//#ifdef R22_UPDATES
        case ZPS_EVENT_NWK_ED_SCAN:
        {
            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkEdScanConfirmEvent.u8Status , u16Length );

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkEdScanConfirmEvent.u8ResultListSize , u16Length );

            uint8 u8Index;
            for (u8Index = 0u; u8Index < sStackEvent.uEvent.sNwkEdScanConfirmEvent.u8ResultListSize; u8Index++)
            {
                ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkEdScanConfirmEvent.au8EnergyDetect[u8Index] , u16Length );
            }
           vSL_WriteMessage(E_SL_MSG_NWK_ED_SCAN, u16Length,NULL, au8StatusBuffer);
        }
        break;

//#endif /* R22_UPDATES */

        case ZPS_EVENT_NWK_DISCOVERY_COMPLETE: {
            uint8 i;
            DBG_vPrintf(TRACE_EVENT_HANDLER, "ZPS_EVENT_NWK_DISCOVERY_COMPLETE\n");

            if(sStackEvent.uEvent.sApsDataIndEvent.eStatus == ZPS_NWK_ENUM_INVALID_REQUEST && u8TempExtendedError != 0u)
            {
                /* We changed the status because of a resource related extended error*/
                sStackEvent.uEvent.sApsDataIndEvent.eStatus = E_SL_MSG_STATUS_C2_SUBSTITUTION;
            }

            ZNC_BUF_U32_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkDiscoveryEvent.u32UnscannedChannels , u16Length );

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkDiscoveryEvent.eStatus , u16Length );

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkDiscoveryEvent.u8NetworkCount , u16Length );

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkDiscoveryEvent.u8SelectedNetwork , u16Length );

            /* Event structure can be extended with NWK Discovery Event Channels */

            for(i = 0; i < sStackEvent.uEvent.sNwkDiscoveryEvent.u8NetworkCount; i++)
            {
                ZNC_BUF_U64_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkDiscoveryEvent.psNwkDescriptors[i].u64ExtPanId , u16Length );

                ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkDiscoveryEvent.psNwkDescriptors[i].u8LogicalChan , u16Length );

                ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkDiscoveryEvent.psNwkDescriptors[i].u8StackProfile , u16Length );

                ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkDiscoveryEvent.psNwkDescriptors[i].u8ZigBeeVersion , u16Length );

                ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkDiscoveryEvent.psNwkDescriptors[i].u8PermitJoining , u16Length );

                ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkDiscoveryEvent.psNwkDescriptors[i].u8RouterCapacity , u16Length );

                ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkDiscoveryEvent.psNwkDescriptors[i].u8EndDeviceCapacity , u16Length );
            }
            vSL_WriteMessage(E_SL_MSG_NWK_DISCOVERY_COMPLETE, u16Length,NULL,
                    au8StatusBuffer);
        }
        break;

        case ZPS_EVENT_BIND_REQUEST_SERVER:
        {
            DBG_vPrintf(TRACE_EVENT_HANDLER, "Bind server Status %02x src Ep %d Failcount %d\n",
                    sStackEvent.uEvent.sBindRequestServerEvent.u8Status,
                    sStackEvent.uEvent.sBindRequestServerEvent.u8SrcEndpoint,
                    sStackEvent.uEvent.sBindRequestServerEvent.u32FailureCount);
        }
        break;

        case ZPS_EVENT_NWK_POLL_CONFIRM:
            break;

        case ZPS_EVENT_TC_STATUS:
        {
            uint8 u8TcStatus = sStackEvent.uEvent.sApsTcEvent.u8Status;

            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], u8TcStatus , u16Length );

            switch (u8TcStatus) 
            {
            case ZPS_APL_APS_E_SECURED_LINK_KEY:
            case ZPS_APL_APS_E_SECURITY_FAIL:
                ZNC_BUF_U64_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsTcEvent.uTcData.u64ExtendedAddress , u16Length );
                break;
            case ZPS_E_SUCCESS:

                // sStackEvent.uEvent.sApsTcEvent.uTcData.pKeyDesc.u32OutgoingFrameCounter
                ZNC_BUF_U32_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsTcEvent.uTcData.pKeyDesc->u32OutgoingFrameCounter , u16Length );

                // sStackEvent.uEvent.sApsTcEvent.uTcData.pKeyDesc.u16ExtAddrLkup
                ZNC_BUF_U16_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsTcEvent.uTcData.pKeyDesc->u16ExtAddrLkup , u16Length );

                // sStackEvent.uEvent.sApsTcEvent.uTcData.pKeyDesc.au8LinkKey[]
                memcpy(&au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsTcEvent.uTcData.pKeyDesc->au8LinkKey, ZPS_SEC_KEY_LENGTH);
                u16Length += ZPS_SEC_KEY_LENGTH;
                
                // sStackEvent.uEvent.sApsTcEvent.uTcData.pKeyDesc.u8BitMapSecLevl
                ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sApsTcEvent.uTcData.pKeyDesc->u8BitMapSecLevl , u16Length );

                break;
            default:
                DBG_vPrintf(TRACE_EVENT_HANDLER, "Unhandled ZPS_EVENT_TC_STATUS status %x", u8TcStatus);
                break;
            }

            vSL_WriteMessage(E_SL_MSG_TC_STATUS, u16Length, NULL, au8StatusBuffer);
        }
        break;

        case ZPS_EVENT_ZDO_BIND:
        {
            uint8 u8DstAddrMode = sStackEvent.uEvent.sZdoBindEvent.u8DstAddrMode;

            /* Put address mode first so SerialLink payload can be properly parsed on the other side */

            /* Copy destination address mode */
            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sZdoBindEvent.u8DstAddrMode , u16Length );

            /* Copy destination address */
            if(ZPS_E_ADDR_MODE_IEEE == u8DstAddrMode)
            {
                ZNC_BUF_U64_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sZdoBindEvent.uDstAddr.u64Addr , u16Length );
            }
            else
            {
                ZNC_BUF_U16_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sZdoBindEvent.uDstAddr.u16Addr , u16Length );
            }

            /* Copy source endpoint */
            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sZdoBindEvent.u8SrcEp , u16Length );

            /* Copy destination endpoint */
            ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sZdoBindEvent.u8DstEp , u16Length );

            vSL_WriteMessage(E_SL_MSG_ZDO_BIND_EVENT, u16Length, NULL, au8StatusBuffer);
        }
        break;
        
        default:
            DBG_vPrintf(TRACE_EVENT_HANDLER, "Unhandled Zps stack event %d", sStackEvent.eType);
            break;

        }
    }

    if (sStackEvent.eType == ZPS_EVENT_ERROR)
    {
        vHandleZpsErrorEvent( &sStackEvent.uEvent.sAfErrorEvent, au8StatusBuffer);
        /* Got link status failure */
        if (sStackEvent.uEvent.sAfErrorEvent.eError == ZPS_ERROR_ZDO_LINKSTATUS_FAIL &&
            sStackEvent.uEvent.sAfErrorEvent.uErrorData.u64Value == MAC_ENUM_TRANSACTION_EXPIRED) {
            //vHandleLinkStatusFail(&sStackEvent.uEvent.sAfErrorEvent);
            /* TODO: LS fail not yet implemented */
        }
    }

    if (bIsPktPosted)
    {

        switch (sNcpDeviceDesc.eNodeState) {

#ifdef ZB_ROUTER_DEVICE
    case E_DISCOVERY:
        vHandleStartupEvent(sStackEvent);

        if (sStackEvent.eType == ZPS_EVENT_NWK_JOINED_AS_ROUTER)
        {
            DBG_vPrintf(TRACE_EVENT_HANDLER, "ZPS_EVENT_NWK_JOINED_AS_ROUTER\n");
            sNcpDeviceDesc.eNodeState = E_RUNNING;
            sNcpDeviceDesc.eState = NOT_FACTORY_NEW;

            vSaveDevicePdmRecord();
            break;
        }
        break;
#endif
    case E_RUNNING:
        if (sStackEvent.eType != ZPS_EVENT_NONE)
        {

            if (sStackEvent.eType == ZPS_EVENT_NWK_NEW_NODE_HAS_JOINED)
            {
                DBG_vPrintf(TRACE_EVENT_HANDLER, "\nNode joined %04x, u8Rejoin = %d",
                        sStackEvent.uEvent.sNwkJoinIndicationEvent.u16NwkAddr,
                        sStackEvent.uEvent.sNwkJoinIndicationEvent.u8Rejoin);

                /* report to host */
                ZNC_BUF_U64_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkJoinIndicationEvent.u64ExtAddr , u16Length );

                ZNC_BUF_U16_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkJoinIndicationEvent.u16NwkAddr , u16Length );

                ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkJoinIndicationEvent.u8Capability , u16Length );

                ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkJoinIndicationEvent.u8Rejoin , u16Length );

                ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkJoinIndicationEvent.u8SecureRejoin , u16Length );

                vSL_WriteMessage(E_SL_MSG_NEW_NODE_HAS_JOINED, u16Length,NULL,
                        au8StatusBuffer);

            }

            if (sStackEvent.eType == ZPS_EVENT_NWK_LEAVE_INDICATION)
            {
                /* report to host */
                ZNC_BUF_U64_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkLeaveIndicationEvent.u64ExtAddr , u16Length );

                ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkLeaveIndicationEvent.u8Rejoin , u16Length );

                if (sStackEvent.uEvent.sNwkLeaveIndicationEvent.u64ExtAddr == 0UL)
                {
                    /* leave induication for us, we left */
                    if (sStackEvent.uEvent.sNwkLeaveIndicationEvent.u8Rejoin == FALSE)
                    {
                        /* and we are not rejoining */
                        APP_vResetDeviceState();
                    }
                }
                else
                {
                    /* leave indication from someone else */
                    if (sStackEvent.uEvent.sNwkLeaveIndicationEvent.u8Rejoin == FALSE)
                    {
                        /* the are not rejoining, so remove from tables  */
                        vCleanStackTables(sStackEvent.uEvent.sNwkLeaveIndicationEvent.u64ExtAddr);
                    }
                }
                vSL_WriteMessage(E_SL_MSG_LEAVE_INDICATION, u16Length,NULL, au8StatusBuffer);
            }
            if (sStackEvent.eType == ZPS_EVENT_NWK_LEAVE_CONFIRM)
            {
                if(sStackEvent.uEvent.sApsDataIndEvent.eStatus == ZPS_NWK_ENUM_INVALID_REQUEST && u8TempExtendedError != 0u)
                {
                    /* We changed the status because of a resource related extended error*/
                    sStackEvent.uEvent.sApsDataIndEvent.eStatus = E_SL_MSG_STATUS_C2_SUBSTITUTION;
                }

                 /* report to host */
                ZNC_BUF_U64_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkLeaveConfirmEvent.u64ExtAddr , u16Length );

                ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkLeaveConfirmEvent.bRejoin , u16Length );

                ZNC_BUF_U8_UPD( &au8StatusBuffer[u16Length], sStackEvent.uEvent.sNwkLeaveConfirmEvent.eStatus , u16Length );

                if (sStackEvent.uEvent.sNwkLeaveConfirmEvent.u64ExtAddr == 0UL)
                {
                    /* leace confirm about us */
                    if (sStackEvent.uEvent.sNwkLeaveConfirmEvent.bRejoin == FALSE)
                    {
                        /* and we are not rejoining */
                        APP_vResetDeviceState();
                    }
                }
                else
                {
                    /* someone else left */
                    if (sStackEvent.uEvent.sNwkLeaveConfirmEvent.bRejoin == FALSE)
                    {
                        /* and are not rejoining */
                        vCleanStackTables(sStackEvent.uEvent.sNwkLeaveConfirmEvent.u64ExtAddr);
                    }
                }
                vSL_WriteMessage(E_SL_MSG_LEAVE_CONFIRM, u16Length,NULL, au8StatusBuffer);
            }
        }

        break;

    default:
        break;

    }
    /*
     * Global clean up to make sure any APDUs have been freed
     */
    if(sStackEvent.eType == ZPS_EVENT_APS_DATA_INDICATION)
    {
        PDUM_eAPduFreeAPduInstance(
                sStackEvent.uEvent.sApsDataIndEvent.hAPduInst);
    }
    else if(sStackEvent.eType == ZPS_EVENT_APS_INTERPAN_DATA_INDICATION)
    {
        PDUM_eAPduFreeAPduInstance(
                        sStackEvent.uEvent.sApsInterPanDataIndEvent.hAPduInst);
    }

    }
}

#ifndef ZNC_MACRO
PUBLIC uint64 ZNC_RTN_U64(uint8 *pu8Buffer, uint64 u64i) {

    return( ( ( uint64 ) ( pu8Buffer )[ u64i ]  <<  56) |
            ( ( uint64 ) ( pu8Buffer )[ u64i + 1 ]  << 48) |
            ( ( uint64 ) ( pu8Buffer )[ u64i + 2 ]  << 40) |
            ( ( uint64 ) ( pu8Buffer )[ u64i + 3 ]  << 32) |
            ( ( uint64 ) ( pu8Buffer )[ u64i + 4 ]  << 24) |
            ( ( uint64 ) ( pu8Buffer )[ u64i + 5 ]  << 16) |
            ( ( uint64 ) ( pu8Buffer )[ u64i + 6 ]  << 8) |
            ( ( uint64 ) ( pu8Buffer )[ u64i + 7 ] & 0xFF));
}

PUBLIC uint32 ZNC_RTN_U32(uint8 *pu8Buffer, uint64 u64i) {
    return ( ( ( uint32 ) ( pu8Buffer )[ u64i ] << 24) |
            ( ( uint32 ) ( pu8Buffer )[ u64i + 1 ]  << 16) |
            ( ( uint32 ) ( pu8Buffer )[ u64i + 2 ]  << 8) |
            ( ( uint32 ) ( pu8Buffer )[ u64i + 3 ] & 0xFF));
}

PUBLIC uint16 ZNC_RTN_U16(uint8 *pu8Buffer, uint64 u64i) {
    return ( ( ( uint16 ) (pu8Buffer)[ u64i ] << 8) |
            ( ( uint16 ) (pu8Buffer)[ u64i + 1 ] & 0xFF));
}

PUBLIC void ZNC_BUF_U64_UPD_R (uint8 *pu8Buffer, uint64 u8Value) {

    pu8Buffer[0] = (uint8) ((u8Value >> 56) & 0xFF);
    pu8Buffer[1] = (uint8) ((u8Value >> 48) & 0xFF);
    pu8Buffer[2] = (uint8) ((u8Value >> 40) & 0xFF);
    pu8Buffer[3] = (uint8) ((u8Value >> 32) & 0xFF);
    pu8Buffer[4] = (uint8) ((u8Value >> 24) & 0xFF);
    pu8Buffer[5] = (uint8) ((u8Value >> 16) & 0xFF);
    pu8Buffer[6] = (uint8) ((u8Value >>  8) & 0xFF);
    pu8Buffer[7] = (uint8) (u8Value & 0xFF );
}

PUBLIC void ZNC_BUF_U32_UPD_R (uint8 *pu8Buffer, uint32 u32Value) {

    pu8Buffer[0] = (uint8) ((u32Value >> 24) & 0xFF);
    pu8Buffer[1] = (uint8) ((u32Value >> 16) & 0xFF);
    pu8Buffer[2] = (uint8) ((u32Value >>  8) & 0xFF);
    pu8Buffer[3] = (uint8) (u32Value & 0xFF );
}

PUBLIC void ZNC_BUF_U16_UPD_R (uint8 *pu8Buffer, uint16 u16Value) {

    pu8Buffer[0] = (uint8) ((u16Value >>  8) & 0xFF);
    pu8Buffer[1] = (uint8) (u16Value & 0xFF );
}
#endif

void vfExtendedStatusCallBack (ZPS_teExtendedStatus eExtendedStatus)
{
    uint8 au8Buffer[4];
    au8Buffer[0] = 0x06U; //ZPS_EXTENDED_ERROR;
    au8Buffer[1] = eExtendedStatus;
    vSL_WriteMessage(E_SL_MSG_ZPS_ERROR, 2,NULL, au8Buffer);
}

/****************************************************************************
 *
 * NAME: vSendJoinedFormEventToHost
 *
 * DESCRIPTION: Joined Event to Host.
 *
 * PARAMETERS:
 * uint8 u8FormJoin
 * uint8* pu8Buffer
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vSendJoinedFormEventToHost(ZPS_tsAfEvent* psStackEvent, uint8* pu8Buffer)
{
    uint32 u32Channel;
    uint16 u16Length;

    ZPS_eMacPlmeGet(PHY_PIB_ATTR_CURRENT_CHANNEL, &u32Channel);

    u16Length = 0;

    if (psStackEvent->eType == ZPS_EVENT_NWK_STARTED)
    {
        uint32 u32AuxChannel = 0xffffffffU;
        ZPS_eMacPlmeGet(PHY_PIB_ATTR_AUX_CHANNEL, &u32AuxChannel);

        /* Nwk started */
        ZNC_BUF_U8_UPD( &pu8Buffer[u16Length], 1 , u16Length );

        /* Status */
        ZNC_BUF_U8_UPD( &pu8Buffer[u16Length], psStackEvent->uEvent.sNwkFormationEvent.u8Status , u16Length );

        /* Event structure can be extended to include u32Channel, u16NwkPanId and u32AuxChannel */
    }
    else if (psStackEvent->eType == ZPS_EVENT_NWK_FAILED_TO_START)
    {
        /* Nwk failed to start */
        ZNC_BUF_U8_UPD( &pu8Buffer[u16Length], 0xC4U , u16Length );

        /* Status */
        ZNC_BUF_U8_UPD( &pu8Buffer[u16Length], psStackEvent->uEvent.sNwkFormationEvent.u8Status , u16Length );

        /* Event structure can be extended to include beaconCount */
    }
    else if ((psStackEvent->eType == ZPS_EVENT_NWK_JOINED_AS_ENDDEVICE) ||
             (psStackEvent->eType == ZPS_EVENT_NWK_JOINED_AS_ROUTER))
    {
        /* Nwk joined as enddevice, nwk joined as router */
        ZNC_BUF_U8_UPD( &pu8Buffer[u16Length], psStackEvent->eType , u16Length );

        /* Network address allocated to the joining node */
        ZNC_BUF_U16_UPD( &pu8Buffer[u16Length], psStackEvent->uEvent.sNwkJoinedEvent.u16Addr , u16Length );

        /* The join was a rejoin or a new association */
        ZNC_BUF_U8_UPD( &pu8Buffer[u16Length], psStackEvent->uEvent.sNwkJoinedEvent.bRejoin , u16Length );
        ZNC_BUF_U8_UPD( &pu8Buffer[u16Length], psStackEvent->uEvent.sNwkJoinedEvent.bSecuredRejoin , u16Length );

        /* Event structure can be extended to include u32Channel, u16NwkPanId and u8NwkUpdateId */
    }
    else if (psStackEvent->eType == ZPS_EVENT_NWK_FAILED_TO_JOIN)
    {
        /* Nwk failed to join */
        ZNC_BUF_U8_UPD( &pu8Buffer[u16Length], psStackEvent->eType , u16Length );

        /* Status */
        ZNC_BUF_U8_UPD( &pu8Buffer[u16Length], psStackEvent->uEvent.sNwkJoinFailedEvent.u8Status , u16Length );

        /* The join was a rejoin or a new association */
        ZNC_BUF_U8_UPD( &pu8Buffer[u16Length], psStackEvent->uEvent.sNwkJoinFailedEvent.bRejoin , u16Length );

        /* Event structure can be extended to include beaconCount */
    }

    vSL_WriteMessage(E_SL_MSG_NETWORK_JOINED_FORMED, u16Length,NULL, pu8Buffer);
}

PUBLIC void vHandleZpsErrorEvent(ZPS_tsAfErrorEvent *psErrEvt, uint8 * pu8Buffer)
{
    uint16 u16Length = 0;

    /* Event Error */
    ZNC_BUF_U8_UPD( &pu8Buffer[u16Length], psErrEvt->eError , u16Length );

    if ( ZPS_ERROR_OS_MESSAGE_QUEUE_OVERRUN == psErrEvt->eError)
    {
        ZNC_BUF_U32_UPD( &pu8Buffer[u16Length], psErrEvt->uErrorData.sAfErrorOsMessageOverrun.hMessage , u16Length );
    }
    else if (ZPS_ERROR_ZDO_LINKSTATUS_FAIL == psErrEvt->eError) {
        ZNC_BUF_U64_UPD( &pu8Buffer[u16Length], psErrEvt->uErrorData.u64Value , u16Length );
    }
    else
    {
        /* Event Error Data Profile ID */
        ZNC_BUF_U16_UPD( &pu8Buffer[u16Length], psErrEvt->uErrorData.sAfErrorApdu.u16ProfileId , u16Length );

        /* Event Error Data Cluster ID */
        ZNC_BUF_U16_UPD( &pu8Buffer[u16Length], psErrEvt->uErrorData.sAfErrorApdu.u16ClusterId , u16Length );

        /* Event Error Data Source Address Mode */
        ZNC_BUF_U8_UPD( &pu8Buffer[u16Length], psErrEvt->uErrorData.sAfErrorApdu.u8SrcAddrMode , u16Length );

        if (ZPS_E_ADDR_MODE_IEEE == psErrEvt->uErrorData.sAfErrorApdu.u8SrcAddrMode)
        {
            /* Event Error Data IEEE Address */
            ZNC_BUF_U64_UPD( &pu8Buffer[u16Length], psErrEvt->uErrorData.sAfErrorApdu.uAddr.u64SrcAddr , u16Length );
        }
        else
        {
            /* Event Error Data Short Address */
            ZNC_BUF_U16_UPD( &pu8Buffer[u16Length], psErrEvt->uErrorData.sAfErrorApdu.uAddr.u16SrcAddr , u16Length );
        }
        /* Event Error Data Destination Endpoint */
        ZNC_BUF_U8_UPD( &pu8Buffer[u16Length], psErrEvt->uErrorData.sAfErrorApdu.u8DstEndpoint , u16Length );

        /* Event Error Data Source Endpoint */
        ZNC_BUF_U8_UPD( &pu8Buffer[u16Length], psErrEvt->uErrorData.sAfErrorApdu.u8SrcEndpoint , u16Length );

        /* Event Error Data APDU */
        ZNC_BUF_U32_UPD( &pu8Buffer[u16Length], psErrEvt->uErrorData.sAfErrorApdu.hAPdu , u16Length );

        /* Event Error Data Data Size */
        ZNC_BUF_U16_UPD( &pu8Buffer[u16Length], psErrEvt->uErrorData.sAfErrorApdu.u16DataSize , u16Length );
    }

    vSL_WriteMessage(E_SL_MSG_ZPS_ERROR, u16Length,NULL, pu8Buffer);

}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: vUpdateDeviceStats
 *
 * DESCRIPTION: Update Device Statistics
 *
 * PARAMETERS:
 * uint64 u64SourceAddress
 * uint8 u8Lqi
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vUpdateDeviceStats(uint64 u64SourceAddress, uint8 u8Lqi)
{
    uint8 i;
    /*search in the device stats table entry*/
    for(i=0;i<MAX_NUMBER_OF_STATS_ENTRY;i++)
    {
        if((sNwkStats.sDeviceStats[i].u64MacAddress == u64SourceAddress)&&(sNwkStats.sDeviceStats[i].u64MacAddress != 0))
        {
            uint64 u64AverageTemp;
            sNwkStats.sDeviceStats[i].u8LastLQI = u8Lqi;

            u64AverageTemp =  ((sNwkStats.sDeviceStats[i].u8AverageLQI*sNwkStats.sDeviceStats[i].u32RxCount)+u8Lqi);
            u64AverageTemp /= sNwkStats.sDeviceStats[i].u32RxCount+1;
            sNwkStats.sDeviceStats[i].u32RxCount++;
            sNwkStats.sDeviceStats[i].u8AverageLQI = u64AverageTemp;
            return;
        }
    }

    /*if device is not in the table, add entry */
    for(i=0;i<MAX_NUMBER_OF_STATS_ENTRY;i++)
    {
        if((sNwkStats.sDeviceStats[i].u64MacAddress == 0)&&(u64SourceAddress != 0))
        {
            sNwkStats.sDeviceStats[i].u64MacAddress = u64SourceAddress;
            sNwkStats.sDeviceStats[i].u8LastLQI = sNwkStats.sDeviceStats[i].u8AverageLQI = u8Lqi;
            sNwkStats.sDeviceStats[i].u32RxCount = 1;
            return;
        }
    }
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
