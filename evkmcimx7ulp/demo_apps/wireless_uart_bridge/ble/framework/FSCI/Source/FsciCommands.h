/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSCI_COMMANDS_H_
#define _FSCI_COMMANDS_H_

/*! *********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */
#include "EmbeddedTypes.h"

/*! *********************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
********************************************************************************** */
/* Define the message types that Fsci recognizes and/or generates. */
enum
{
    mFsciMsgModeSelectReq_c = 0x00,               /* Fsci-ModeSelect.Request.              */
    mFsciMsgGetModeReq_c    = 0x02,               /* Fsci-GetMode.Request.                 */

    mFsciMsgAFResetReq_c                  = 0x05, /* Fsci-AFReset.Request.                 */
    mFsciMsgAPSResetReq_c                 = 0x06, /* Fsci-APSReset.Request.                */
    mFsciMsgAPSReadyReq_c                 = 0x07, /* Fsci-SetAPSReady.Request.             */
    mFsciMsgResetCPUReq_c                 = 0x08, /* Fsci-CPU_Reset.Request.               */
    mFsciMsgDeregisterEndPoint_c          = 0x0A, /* Fsci-DeregisterEndPoint.Request       */
    mFsciMsgRegisterEndPoint_c            = 0x0B, /* Fsci-RegisterEndPoint.Request         */
    mFsciMsgGetNumberOfEndPoints_c        = 0x0C, /* Fsci-GetNumberOfEndPoints.Request     */
    mFsciMsgGetEndPointDescription_c      = 0x0D, /* Fsci-GetEndPointDescription.Request   */
    mFsciMsgGetEndPointIdList_c           = 0x0E, /* Fsci-GetEndPointIdList.Request        */
    mFsciMsgSetEpMaxWindowSize_c          = 0x0F, /* Fsci-SetEpMaxWindowSize.Request       */
    mFsciMsgGetICanHearYouList_c          = 0x10, /* Fsci-GetICanHearYouList.Request       */
    mFsciMsgSetICanHearYouList_c          = 0x11, /* Fsci-SetICanHearYouList.Request       */
    mFsciMsgGetChannelReq_c               = 0x12, /* Fsci-GetChannel.Request               */
    mFsciMsgSetChannelReq_c               = 0x13, /* Fsci-SetChannel.Request               */
    mFsciMsgGetPanIDReq_c                 = 0x14, /* Fsci-GetPanID.Request                 */
    mFsciMsgSetPanIDReq_c                 = 0x15, /* Fsci-SetPanID.Request                 */
    mFsciMsgGetPermissionsTable_c         = 0x16, /* Fsci-GetPermissionsTable.Request      */
    mFsciMsgSetPermissionsTable_c         = 0x17, /* Fsci-SetPermissionsTable.Request      */
    mFsciMsgRemoveFromPermissionsTable_c  = 0x18, /* Fsci-RemoveFromPermissionsTable.Request    */
    mFsciMsgAddDeviceToPermissionsTable_c = 0x19, /* Fsci-AddDeviceToPermissionsTable.Request   */
    mFsciMsgApsmeGetIBReq_c               = 0x20, /* Fsci-GetIB.Request, aka APSME-GET.Request  */
    mFsciMsgApsmeSetIBReq_c               = 0x21, /* Fsci-SetIB.Request, aka APSME-SET.Request  */
    mFsciMsgNlmeGetIBReq_c                = 0x22, /* Fsci-GetIB.Request, aka NLME-GET.Request   */
    mFsciMsgNlmeSetIBReq_c                = 0x23, /* Fsci-SetIB.Request, aka NLME-SET.Request   */
    mFsciMsgGetNumOfMsgsReq_c             = 0x24, /* Fsci-Get number of msgs available request  */
    mFsciMsgFreeDiscoveryTablesReq_c      = 0x25, /* Fsci-FreeDiscoveryTables.Request           */
    mFsciMsgSetJoinFilterFlagReq_c        = 0x26, /* Fsci-SetJoinFilterFlag.Request             */
    mFsciMsgGetMaxApplicationPayloadReq_c = 0x27, /* Fsci-GetMaxApplicationPayload.Request      */

    mFsciOtaSupportSetModeReq_c            = 0x28,
    mFsciOtaSupportStartImageReq_c         = 0x29,
    mFsciOtaSupportPushImageChunkReq_c     = 0x2A,
    mFsciOtaSupportCommitImageReq_c        = 0x2B,
    mFsciOtaSupportCancelImageReq_c        = 0x2C,
    mFsciOtaSupportSetFileVerPoliciesReq_c = 0x2D,
    mFsciOtaSupportAbortOTAUpgradeReq_c    = 0x2E,
    mFsciOtaSupportImageChunkReq_c         = 0x2F,
    mFsciOtaSupportQueryImageReq_c         = 0xC2,
    mFsciOtaSupportQueryImageRsp_c         = 0xC3,
    mFsciOtaSupportImageNotifyReq_c        = 0xC4,
    mFsciOtaSupportGetClientInfo_c         = 0xC5,

    mFsciEnableBootloaderReq_c = 0xCF,

    mFsciLowLevelMemoryWriteBlock_c = 0x30,    /* Fsci-WriteRAMMemoryBlock.Request     */
    mFsciLowLevelMemoryReadBlock_c  = 0x31,    /* Fsci-ReadMemoryBlock.Request         */
    mFsciLowLevelPing_c             = 0x38,    /* Fsci-Ping.Request                    */

    mFsciMsgGetApsDeviceKeyPairSet_c   = 0x3B, /* Fsci-GetApsDeviceKeyPairSet         */
    mFsciMsgGetApsDeviceKey_c          = 0x3C,
    mFsciMsgSetApsDeviceKey_c          = 0x3D,
    mFsciMsgSetApsDeviceKeyPairSet_c   = 0x3E,
    mFsciMsgClearApsDeviceKeyPairSet_c = 0x3F,

    mFsciMsgAllowDeviceToSleepReq_c = 0x70, /* Fsci-SelectWakeUpPIN.Request         */
    mFsciMsgWakeUpIndication_c      = 0x71, /* Fsci-WakeUp.Indication               */
    mFsciMsgGetWakeUpReasonReq_c    = 0x72, /*                */
#if gBeeStackIncluded_d
    mFsciMsgSetApsDeviceKeyPairSetKeyInfo = 0x40,
    mFsciMsgSetApsOverrideApsEncryption   = 0x41,
    mFsciMsgSetPollRate                   = 0x43,
    mFsciMsgSetRejoinConfigParams         = 0x44,

    mFsciMsgSetFaMaxIncomingErrorReports_c = 0x4A,

    mFsciMsgSetHighLowRamConcentrator = 0x50,

    mFsciMsgEZModeComissioningStart                    = 0x51, /* EZ mode, Touchlink  procedure */
    mFsciMsgZllTouchlinkCommissioningStart_c           = 0x52,
    mFsciMsgZllTouchlinkCommissioningConfigure_c       = 0x53,
    mFsciMsgZllTouchlinkGetListOfCommissionedDevices_c = 0x54,
    mFsciMsgZllTouchlinkRemoveEntry_c                  = 0x55,
    mFsciMsgClearNeighborTableEntry_c                  = 0x56,
#endif
    mFsciGetUniqueId_c   = 0xB0,
    mFsciGetMcuId_c      = 0xB1,
    mFsciGetSwVersions_c = 0xB2,

    mFsciMsgAddToAddressMapPermanent_c = 0xC0,
    mFsciMsgRemoveFromAddressMap_c     = 0xC1,

    mFsciMsgWriteNwkMngAddressReq_c = 0xAD, /* Fsci-WriteNwkMngAddr.Request         */

    mFsciMsgReadExtendedAdrReq_c   = 0xD2,  /* Fsci-ReadExtAddr.Request             */
    mFsciMsgReadNwkMngAddressReq_c = 0xDA,  /* Fsci-ReadNwkMngAddr.Request          */
    mFsciMsgWriteExtendedAdrReq_c  = 0xDB,  /* Fsci-WriteExtAddr.Request            */
    mFsciMsgStopNwkReq_c           = 0xDC,  /* Fsci-StopNwk.Request                 */
    mFsciMsgStartNwkReq_c          = 0xDF,  /* Fsci-StartNwk.Request                */
    mFsciMsgStartNwkExReq_c        = 0xE7,  /* Fsci-StartNwkEx.Request              */
    mFsciMsgStopNwkExReq_c         = 0xE8,  /* Fsci-StopNwkEx.Request               */
    mFsciMsgRestartNwkReq_c        = 0xE0,  /* Fsci-RestartNwk.Request              */

    mFsciMsgAck_c        = 0xFD,            /* Fsci acknowledgment.                 */
    mFsciMsgError_c      = 0xFE,            /* Fsci internal error.                 */
    mFsciMsgDebugPrint_c = 0xFF,            /* printf()-style debug message.        */
};

typedef PACKED_STRUCT gFsciErrorMsg_tag
{
    clientPacketHdr_t header;
    clientPacketStatus_t status;
    uint8_t checksum;
    uint8_t checksum2;
}
gFsciErrorMsg_t;

typedef PACKED_STRUCT gFsciAckMsg_tag
{
    clientPacketHdr_t header;
    uint8_t checksumPacketReceived;
    uint8_t checksum;
    uint8_t checksum2;
}
gFsciAckMsg_t;

/*! *********************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
********************************************************************************** */

void fsciMsgHandler(void *pData, uint32_t fsciInterface);

#endif /* _FSCI_COMMANDS_H_ */
