/*
* Copyright 2024 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef APP_COMMON_NCP_H_
#define APP_COMMON_NCP_H_

#include <string.h>
#include "app_common.h"
#include "EmbeddedTypes.h"
#include "zll_commission.h"
#include "zll_utility.h"
#include "ZTimer.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
/* Node 'coordinator' */
/* Endpoints */
#define COORDINATOR_ZDO_ENDPOINT    (0)
#define COORDINATOR_APPLICATION_ENDPOINT    (1)

#define ERR_FATAL_NON_RECOVERABLE   1U
#define ERR_FATAL_REBOOT            2U
#define ERR_FATAL_ZIGBEE_RESTART    3U
#define ERR_NON_FATAL               4U
#define ERR_NON_FATAL_DEBUG         5U
#define MAX_NO_OF_TIME_REQ          5U
#define MAX_NO_OF_RETRY_REQ          5U

/*  The apdu payload is directly copied to UART from aps buffer*/
#define MAX_PACKET_TX_SIZE  (1000)

#ifndef APDU_PAYLOAD_SIZE
#define APDU_PAYLOAD_SIZE    1600U
#endif

#define ZBmemcpy(a,b,c)	memcpy((a), (b), (c))
#define ZBmemset(a, b, c) memset((a), (b), (c))

/* How long to wait (max) until the JN is ready to accept commands*/
#define JN_READY_TIME_MS               2000U

#define APP_vVerifyFatal(bIsOk, str, u32Level) APP_vVerifyFatalTest(bIsOk, str, u32Level, __LINE__);

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum PACK
{
    APP_MSG_TYPE_ZCL_TIMER = 0x80,
    APP_MSG_TYPE_APP_TIMER,
    APP_MSG_TYPE_USER_CMD,
    APP_MSG_TYPE_USER_CMD_TX,
    APP_MSG_TYPE_SEND_GAS_DEV_PEND_MSGS,
    APP_MSG_TYPE_SEND_GAS_DEV_TUNN_MSGS,
    APP_MSG_TYPE_SEND_GAS_DEV_UNCTRL_FLOW_THR_MSGS,
    APP_MSG_TYPE_SEND_GAS_DEV_LOCAL_CHANGE_SUPPLY_MSGS,
    APP_MSG_TYPE_SEND_GAS_DEV_CHANGE_SUPPLY_MSGS,
    APP_MSG_TYPE_SEND_GAS_DEV_GET_SNAPSHOT,
    APP_MSG_TYPE_SEND_GAS_DEV_GET_SAMPLED_DATA,
    APP_MSG_TYPE_ZCL_DIRTY_TIMER,
    APP_MSG_TYPE_PERMIT_JOIN_CHANGED,
    APP_MSG_TYPE_MSG_RECEIVED,
    APP_MSG_TYPE_ONE_SEC_TIMER,
    APP_MSG_TYPE_IMAGE_VERIFY,
    APP_MSG_TYPE_PENDING_GET_NOTIF,
    APP_MSG_TYPE_UPDATE_ZCL_SUSPENSION_PERIOD,
    APP_MSG_TYPE_40SEC_TIMER
} teApp_MsgType;

typedef struct
{
    ZPS_tsNWkNibPersist sPersist;
    uint64 u64TcAddress;
    zps_tsPersistNeighboursTable sActvNtEntry;
    ZPS_tsNwkSecMaterialSet asSecMatSet[2];
    uint32  u32OutgoingFrameCounter;
    uint8   u8KeyType;
} ZPS_tsAfRestorePointStructLocal;

typedef struct
{
    uint32 u32PollTimeInterval;
    uint32 u32TimeToNextPoll;
    uint8  u8PollsInSet;
    uint8 u8PollsRemaining;
    uint8 u8PollTimerStatus;
}tsJNPollState;

typedef struct
{
    uint8   au8Key[ZPS_SEC_KEY_LENGTH];
    uint64  u64Address;
    uint32  u32InFrameCtr;
    uint32  u32OutFrameCtr;
    uint8   u8Permissions;
} tsReprovissionData;

typedef enum {
    JN_NOT_READY = 0U,
    JN_READY = 1U,
    JN_PROVISIONED = 2U
} teJNState;

typedef enum {
    E_CONFIGURE_NETWORK, E_NETWORK_STARTUP, E_NETWORK_RUN
} teDeviceState;

typedef struct {
    teDeviceState eState;
    bool_t bIsPktRxedFromIHD;
    bool_t bIsDefaultRspPendingToIHD;
    bool_t bIsServer;
    uint8 u8IpdEndPoint;
    uint8 u8SMClientEndpoint;
    uint8 u8ZCLTransSeqNoToMeter;
    uint8 u8ZCLTransSeqFromIHD;
    uint16 u16IpdAddr;
    uint16 u16SMClientAddr;
} tsDevicePersist;


/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
PUBLIC void APP_vProcessZCLMessage(uint32 u32Msg);
PUBLIC ZPS_teStatus APP_eZbModuleInitialise(void);
PUBLIC void APP_vPostToZclQueue(uint8 *pvMessage);
PUBLIC void APP_vPostToAppQueue (uint8 *pvMsg);
PUBLIC void APP_vPostToAppQueueWord (uint8 u8MsgType);

PUBLIC void APP_vNcpHostReset();
PUBLIC void APP_vVerifyFatalTest (bool_t bIsOk, const char *strErr, uint32 u32ErrorLevel, int16 s16Line);

PUBLIC uint16 u16Read16Nbo(uint8 *pu8Payload);

PUBLIC void vLockZCLMutex(void);
PUBLIC void vUnlockZCLMutex(void);
/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#endif /*APP_COMMON_NCP_H_*/
