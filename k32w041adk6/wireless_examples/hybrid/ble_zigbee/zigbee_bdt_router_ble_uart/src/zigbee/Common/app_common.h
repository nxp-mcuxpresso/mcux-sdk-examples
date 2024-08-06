/*
* Copyright 2019, 2023 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef APP_COMMON_H_
#define APP_COMMON_H_

#include "EmbeddedTypes.h"
#include "zll_commission.h"
#include "zll_utility.h"
#include "ZTimer.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/*
 * Sets the # of msecs between the wake-ups of the node wakes and attempt to
 * send Data Reqs. The delta b/w DRs is et by the
 * POLL_TIME_FAST
 */
#ifndef POLL_TIME
#define POLL_TIME               ZTIMER_TIME_MSEC(5000)
#endif

/*
 * After how many seconds to retry steering the network
 * in case of initial failure
 */
#ifndef STEER_RESTART_TIME
#define STEER_RESTART_TIME       ZTIMER_TIME_MSEC(10000)
#endif

/*
 * Defines the interval between two Data Reqs
 */
#ifndef POLL_TIME_FAST
#define POLL_TIME_FAST          ZTIMER_TIME_MSEC(250)
#endif

#ifndef GP_ZCL_TICK_TIME
#define GP_ZCL_TICK_TIME        ZTIMER_TIME_MSEC(1)
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum
{
    APP_E_EVENT_NONE = 0,
    APP_E_EVENT_BUTTON_DOWN,
    APP_E_EVENT_BUTTON_UP,
    APP_E_EVENT_PGM_BUTTON_UP,
    APP_E_EVENT_PGM_BUTTON_DOWN,
    APP_E_EVENT_SERIAL_TOGGLE,
	APP_E_EVENT_SERIAL_NWK_STEER,
	APP_E_EVENT_SERIAL_FIND_BIND_START,
	APP_E_EVENT_SERIAL_FORM_NETWORK,
    APP_E_EVENT_HA_IDENTIFY_QUERY_RESPONSE,
    APP_E_EVENT_LIGHT_ON,
    APP_E_EVENT_LIGHT_OFF,
    APP_E_EVENT_LEVEL_CHANGE,
    APP_E_EVENT_1HZ_TICK_TIMER_EXPIRED,
    APP_E_EVENT_10HZ_TICK_TIMER_EXPIRED,
    APP_E_EVENT_DATA_CONFIRM,
    APP_E_EVENT_DATA_CONFIRM_FAILED,
    APP_E_EVENT_TOUCH_LINK,
    APP_E_EVENT_EP_INFO_MSG,
    APP_E_EVENT_EP_LIST_MSG,
    APP_E_EVENT_GROUP_LIST_MSG,
    APP_E_EVENT_POR_IDENTIFY,
    APP_E_EVENT_GP_DECOMMISSION,
    APP_E_EVENT_POR_FACTORY_RESET,
    APP_E_EVENT_POR_PDM_RESET,
    APP_E_EVENT_MAX,
    APP_E_EVENT_START_ROUTER,
    APP_E_EVENT_TRANSPORT_HA_KEY,
    APP_E_EVENT_SEND_PERMIT_JOIN,
    APP_E_EVENT_ENCRYPT_SEND_KEY,
} teAppEvents;

typedef enum PACK {
   FACTORY_NEW = 0,
   NOT_FACTORY_NEW = 0xff
}teState;

typedef enum
{
    E_STARTUP,
    E_LEAVE_WAIT,
    E_LEAVE_RESET,
    E_NFN_START,
    E_DISCOVERY,
    E_NETWORK_FORMATION,
    E_JOINING_NETWORK,
    E_REJOINING,
    E_NETWORK_INIT,
    E_RESCAN,
    E_RUNNING
} teNodeState;

typedef struct
{
    teNodeState     eNodeState;
    teNodeState   eNodePrevState;
}tsDeviceDesc;

typedef struct
{
    uint8_t u8Button;
    uint32_t u32DIOState;
} APP_tsEventButton;

typedef struct
{
    uint16    u16NwkAddr;
    uint8     u8Endpoint;
    uint16    u16ProfileId;
    uint16    u16DeviceId;
    uint8     u8Version;
    uint64    u64Address;
    uint8     u8MacCap;
} APP_tsEventTouchLink;

typedef struct {
    uint16                                                u16SrcAddr;
    tsCLD_ZllUtility_EndpointInformationCommandPayload    sPayload;
} APP_tsEventEpInfoMsg;

typedef struct {
    uint8     u8SrcEp;
    uint16    u16SrcAddr;
    tsCLD_ZllUtility_GetEndpointListRspCommandPayload sPayload;
} APP_tsEventEpListMsg;

typedef struct {
    uint8     u8SrcEp;
    uint16    u16SrcAddr;
    tsCLD_ZllUtility_GetGroupIdRspCommandPayload sPayload;
} APP_tsEventGroupListMsg;

typedef struct {
    uint64 u64Address;
    CRYPTO_tsAesBlock uKey;
} APP_tsEventEncryptSendMsg;

typedef struct
{
    teAppEvents eType;
    union
    {
        APP_tsEventButton            sButton;
        APP_tsEventTouchLink         sTouchLink;
        APP_tsEventEpInfoMsg         sEpInfoMsg;
        APP_tsEventEpListMsg         sEpListMsg;
        APP_tsEventGroupListMsg      sGroupListMsg;
        APP_tsEventEncryptSendMsg    sEncSendMsg;
        uint64                       u64TransportKeyAddress;
    }uEvent;
} APP_tsEvent;

typedef struct {
    teState eState;
    teNodeState eNodeState;
    uint8 u8DeviceType;
    uint8 u8MyChannel;
    uint16 u16MyAddr;
    uint16 u16FreeAddrLow;
    uint16 u16FreeAddrHigh;
    uint16 u16FreeGroupLow;
    uint16 u16FreeGroupHigh;
}tsZllState;

/* Out Of Band Commissioning */
#define ZB_OOB_KEY_SIZE 16

struct dev_info
{
    uint64_t addr;
    uint8_t instCode[ZB_OOB_KEY_SIZE];
    uint16_t crc;
};

struct oob_info {
    uint8_t key[ZB_OOB_KEY_SIZE];
    uint64_t tcAddress;
    uint64_t panId;
    uint16_t shortPanId;
    uint8_t activeKeySeq;
    uint8_t channel;
};

struct oob_info_enc {
    struct oob_info info;
    uint32_t mic;
};

PACKED_STRUCT bdb_oob_enc {
    uint64_t addr;
    uint8_t key[ZB_OOB_KEY_SIZE];
    uint32_t mic;
    uint64_t tcAddress;
    uint8_t activeKeySeq;
    uint8_t channel;
    uint16_t shortPanId;
    uint64_t panId;
};

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
void APP_GetInstallCode(uint8_t instCode[ZB_OOB_KEY_SIZE]);
void APP_GetDeviceInfo(struct dev_info* info);

bool_t APP_GetOOBInfo(struct dev_info* info, struct oob_info_enc* oob);
bool_t APP_SetOOBInfo(struct oob_info_enc* oob, bool_t enc);
void APP_ClearOOBInfo();

bool_t APP_Start_BDB_OOB();

/* Must be called before zps_eAplAfInit() */
void APP_SetHighTxPowerMode();

/* Must be called after zps_eAplAfInit() */
void APP_SetMaxTxPower();

#undef HIGH_TX_PWR_LIMIT
#define HIGH_TX_PWR_LIMIT 15	/* dBm */

/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/
uint8_t APP_u8GetDeviceEndpoint(void);
teNodeState APP_eGetCurrentApplicationState (void);
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#endif /*APP_COMMON_H_*/
