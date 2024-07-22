/*
 * Copyright 2023, 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <jendefs.h>
#include <string.h>
#include "zps_apl_af.h"
#include "zps_apl_aib.h"
#include "zps_nwk_sap.h"
#include "zps_nwk_nib.h"
#include "zps_nwk_pub.h"
#include "ZTimer.h"
#include "app_serial_commands.h"
#include "serial_link_wkr.h"
#include "dbg.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifndef TRACE_RESTORE
#define TRACE_RESTORE    FALSE
#endif


#define RESTORE_MAGIC   0x503df08f

#define SIZE_DISCOVERY_EVENT_NO_NETWORK   6 /*   accounts for following parameter  in ZPS_tsNwkNlmeCfmNetworkDiscovery \
                                                 - u8Status, u8NwkCount, u32UnscannedChannels*/

#define LINK_STATUS_FAIL_COUNT          (4U)    // Number of link status failures before reset
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
ZPS_tsAfRestorePointStruct   sRestoreStruct;

PRIVATE  uint32 u32RestoreMagic = 0;
PRIVATE uint8 u8BeaconCount = 0;
/****************************************************************************/
/***        Local  functions                                            ***/
/****************************************************************************/
PUBLIC void APP_vDumpRestoreData(void);

PUBLIC void vResetBeaconCount(void)
{
    u8BeaconCount = 0;
}

PUBLIC uint8 u8GetBeaconCount(void)
{
    uint8 u8Temp = u8BeaconCount;
    u8BeaconCount = 0;
    return u8Temp;
}

PUBLIC void vGetStackData(void)
{
    /* ensure flag is correct, partial restore required  */
    sRestoreStruct.bRecoverJoiner = TRUE;
    ZPS_vGetRestorePoint(&sRestoreStruct);
#ifdef SAVE_RESTORE
    if (SL_ZB_DEVICE_TYPE != E_SL_DEVICE_COMMS_HUB)
    {
        uint8 u8Status;
        tsNtSecStruct  sTempNTSec;

        u32RestoreMagic = RESTORE_MAGIC;

        sTempNTSec.sNeighbourtable = sNeighbourtable;
        u8Status = PDM_eSaveRecordData(PDM_ID_NT_SEC, &sTempNTSec, sizeof(tsNtSecStruct));
        if(PDM_E_STATUS_OK != u8Status)
        {
            DBG_vPrintf(TRUE, "\n PDM_eSaveRecordData PDM_ID_NT_SEC ERROR: 0x%02x\n", u8Status);
            vSendPdmStatusToHost(E_SL_PDM_CMD_SAVE, u8Status);
        }


        tsPdmrestoreStruct   sSaveRestore;
        sSaveRestore.sPersist       = sRestoreStruct.sPersist;
        sSaveRestore.u64TcAddress   = sRestoreStruct.u64TcAddress;
        sSaveRestore.u8KeyType      = sRestoreStruct.u8KeyType;
        sSaveRestore.u32Magic       = u32RestoreMagic;
        u8Status = PDM_eSaveRecordData(PDM_ID_RESTORE_STRUCT, &sSaveRestore, sizeof(tsPdmrestoreStruct));
        if(PDM_E_STATUS_OK != u8Status)
        {
            DBG_vPrintf(TRUE, "\n PDM_eSaveRecordData PDM_ID_RESTORE_STRUCT ERROR: 0x%02x\n", u8Status);
            vSendPdmStatusToHost(E_SL_PDM_CMD_SAVE, u8Status);
        }

    }
#endif

     DBG_vPrintf(TRACE_RESTORE, "After Get Data\n");
     APP_vDumpRestoreData();
}

void vSendDiscoveryNoNetworks(void)
{
    uint8 au8StatusBuffer[10];

    memset(au8StatusBuffer,0,sizeof(au8StatusBuffer));

    au8StatusBuffer[4] = MAC_ENUM_NO_BEACON; /*status, all other parameters are zero*/

    vSL_WriteMessage(E_SL_MSG_NWK_DISCOVERY_COMPLETE, SIZE_DISCOVERY_EVENT_NO_NETWORK,NULL,au8StatusBuffer);

}

PUBLIC void APP_vRestoreStackData ( void )
{

    DBG_vPrintf(TRACE_RESTORE, "Before restore data\n");
    APP_vDumpRestoreData();
    if (u32RestoreMagic == RESTORE_MAGIC)
    {
       DBG_vPrintf(TRACE_RESTORE, "Call stack restore FLAG %d\n", sRestoreStruct.bRecoverJoiner);

       /* ensure flag is correct, partial restore required  */
       sRestoreStruct.bRecoverJoiner = TRUE;
       ZPS_vSetRestorePoint( &sRestoreStruct );

//#ifdef DBG_ENABLE
//      DBG_vUartInit(DBG_E_UART_1, DBG_E_UART_BAUD_RATE_115200);
//#endif

#if 0
      /* recover any orphaned apdus */

      PDUM_vInit();
#endif


    } else { DBG_vPrintf(TRACE_RESTORE, "MAGIC not Set %08x\n", u32RestoreMagic); }

}

PUBLIC void APP_vGetStackData ( void )
{
    if( (ZPS_u8NwkManagerState()) == ZPS_ZDO_ST_ACTIVE )
    {
        vGetStackData();
    }
}

PUBLIC void APP_vDumpRestoreData ( void )
{

#ifdef VERBOSE_DEBUG
    uint32 i, j;
    DBG_vPrintf(TRACE_RESTORE , " ***** [JN] Restore Data  ***********\r\n");
    DBG_vPrintf(TRACE_RESTORE , " [JN] sPersist  \n");
    DBG_vPrintf(TRACE_RESTORE , " u8UpdateId = %d  ", sRestoreStruct.sPersist.u8UpdateId );
    DBG_vPrintf(TRACE_RESTORE , " u8ActiveKeySeqNumber = %d  ",sRestoreStruct.sPersist.u8ActiveKeySeqNumber );
    DBG_vPrintf(TRACE_RESTORE , " u8VsDepth =  %d \n",sRestoreStruct.sPersist.u8VsDepth );
    DBG_vPrintf(TRACE_RESTORE , " u16NwkAddr = %d  ", sRestoreStruct.sPersist.u16NwkAddr );
    DBG_vPrintf(TRACE_RESTORE , " u16VsPanId = %d  ",sRestoreStruct.sPersist.u16VsPanId );
    DBG_vPrintf(TRACE_RESTORE , " u16VsParentAddr =  %d \n",sRestoreStruct.sPersist.u16VsParentAddr );
    DBG_vPrintf(TRACE_RESTORE , " u8CapabilityInformation = %d  ", sRestoreStruct.sPersist.u8CapabilityInformation );
    DBG_vPrintf(TRACE_RESTORE , " u8MacEnhanced = %d  ",sRestoreStruct.sPersist.u8MacEnhanced );
    DBG_vPrintf(TRACE_RESTORE , " u8ParentTimeoutMethod =  %d \n",sRestoreStruct.sPersist.u8ParentTimeoutMethod );
    DBG_vPrintf(TRACE_RESTORE , " u8VsAuxChannel = %d  ", sRestoreStruct.sPersist.u8VsAuxChannel );
    DBG_vPrintf(TRACE_RESTORE , " u8VsChannel = %d  \n",sRestoreStruct.sPersist.u8VsChannel );
    DBG_vPrintf(TRACE_RESTORE , " u64Ext Pan Id = %016llx\n",sRestoreStruct.sPersist.u64ExtPanId );
    DBG_vPrintf(TRACE_RESTORE , " [JN] u64TcAddress =  %llx \n", sRestoreStruct.u64TcAddress );
    DBG_vPrintf(TRACE_RESTORE , " [JN] u32OutgoingFrameCounter =  %08x \n", sRestoreStruct.u32OutgoingFrameCounter );
    DBG_vPrintf(TRACE_RESTORE , " [JN] u8KeyType =  %d \n", sRestoreStruct.u8KeyType );
    DBG_vPrintf(TRACE_RESTORE , " [JN] Security material set present %d \n", (sRestoreStruct.psSecMatSet != NULL));

    if ( sRestoreStruct.psSecMatSet != NULL )
    {
        DBG_vPrintf(TRACE_RESTORE , " %04x      %04x\n",sRestoreStruct.psSecMatSet->u8KeySeqNum,
                                                        sRestoreStruct.psSecMatSet->u8KeyType);
        for ( j = 0 ; j < 16; j++)
        {
            DBG_vPrintf(TRACE_RESTORE , " %02x: ",sRestoreStruct.psSecMatSet->au8Key[j]);
        }
    }
    if (sRestoreStruct.u16AddressLkmp != 0)
    {
        DBG_vPrintf(TRACE_RESTORE , " [JN] Addressmap size =  %d \n", sRestoreStruct.u16AddressLkmp);
        for ( i = 0 ; i < sRestoreStruct.u16AddressLkmp; i++)
        {
            DBG_vPrintf(TRACE_RESTORE , " %08x      %016llx\n",sRestoreStruct.pRwAddressMap[i].u16ShortAddress,
                                                              sRestoreStruct.pRwAddressMap[i].u64ExtendedAddress);
        }
    }
    if ( sRestoreStruct.u16NtTable != 0 )
    {
        DBG_vPrintf(TRACE_RESTORE , " [JN] Neighbour table size =  %d \n", sRestoreStruct.u16NtTable);
        for ( i = 0 ; i < sRestoreStruct.u16NtTable; i++)
        {
            DBG_vPrintf(TRACE_RESTORE , " %08x      %016llx   %04x \n",sRestoreStruct.psActvNtEntry[i].u16NwkAddress,
                                                                      sRestoreStruct.psActvNtEntry[i].u64Address,
                                                                      sRestoreStruct.psActvNtEntry[i].u8Flags);
        }
    }
    if ( sRestoreStruct.u16KeyDescTableSize != 0 )
    {
        DBG_vPrintf(TRACE_RESTORE , " [JN] Key table size =  %d \n", sRestoreStruct.u16KeyDescTableSize);
        DBG_vPrintf(TRACE_RESTORE , " [JN] Default key present %d \n", (sRestoreStruct.psAplDefaultApsKeyDescriptorEntry != NULL));
        for ( i = 0 ; (i < 16) &&  (sRestoreStruct.psAplDefaultApsKeyDescriptorEntry != NULL); i++)
        {
            DBG_vPrintf(TRACE_RESTORE , " %02x: ",sRestoreStruct.psAplDefaultApsKeyDescriptorEntry->au8LinkKey[i]);
        }
        DBG_vPrintf(TRACE_RESTORE , "\n [JN] key table present %d \n", (sRestoreStruct.psAplApsKeyDescriptorEntry != NULL));
        for ( i = 0 ; (i < sRestoreStruct.u16KeyDescTableSize) && (sRestoreStruct.psAplApsKeyDescriptorEntry != NULL); i++)
        {
            DBG_vPrintf(TRACE_RESTORE , " Frame counter %08d      address %016llx\n",sRestoreStruct.psAplApsKeyDescriptorEntry[i].u32OutgoingFrameCounter,
                                                                                     sRestoreStruct.psAplApsKeyDescriptorEntry[i].u64ExtAddr);

            for ( j = 0 ; (j < 16) ; j++)
            {
                DBG_vPrintf(TRACE_RESTORE , " %02x: ",sRestoreStruct.psAplApsKeyDescriptorEntry[i].au8LinkKey[j]);
            }
            DBG_vPrintf(TRACE_RESTORE , " \r\n");
        }
    }
#else
        DBG_vPrintf(TRACE_RESTORE , " ***** [JN] Restore Data (highlights)  ***********\r\n");
        DBG_vPrintf(TRACE_RESTORE , " [JN] sPersist  \n");
        DBG_vPrintf(TRACE_RESTORE , " u8ActiveKeySeqNumber = %d  ",sRestoreStruct.sPersist.u8ActiveKeySeqNumber );
        DBG_vPrintf(TRACE_RESTORE , " u8VsDepth =  %d \n",sRestoreStruct.sPersist.u8VsDepth );
        DBG_vPrintf(TRACE_RESTORE , " u16NwkAddr = %04x  ", sRestoreStruct.sPersist.u16NwkAddr );
        DBG_vPrintf(TRACE_RESTORE , " u16VsPanId = %04x  ",sRestoreStruct.sPersist.u16VsPanId );
        DBG_vPrintf(TRACE_RESTORE , " u16VsParentAddr =  %04x \n",sRestoreStruct.sPersist.u16VsParentAddr );
        DBG_vPrintf(TRACE_RESTORE , " u8VsChannel = %d  \n",sRestoreStruct.sPersist.u8VsChannel );
        DBG_vPrintf(TRACE_RESTORE , " u64Ext Pan Id = %016llx\n",sRestoreStruct.sPersist.u64ExtPanId );
        DBG_vPrintf(TRACE_RESTORE , " [JN] AIB u8KeyType =  %d \n", sRestoreStruct.u8KeyType );
#endif

}
