/*
* Copyright 2023 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef  SERIAL_LINK_CTRL_H_INCLUDED
#define  SERIAL_LINK_CTRL_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "jendefs.h"
#include "app_common.h"
#include "app_common_ncp.h"
#include "app_uart.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define MONITOR_API_ERRORS
#ifdef MONITOR_API_ERRORS
#define MAX_TX_API_ERRORS   50U
#endif

#ifdef ENABLE_SERIAL_LINK_FILE_LOGGING
#define SL_WRITE(DATA) {\
    uint8_t temp = DATA;\
    bSL_LoggerWrite(&temp, 1);\
    UART_vTxChar(DATA);\
    }
#else
#define SL_WRITE(DATA)      UART_vTxChar(DATA)
#endif

#define SL_START_CHAR           0x01U
#define SL_ESC_CHAR     0x02U
#define SL_END_CHAR     0x03U
#define SL_WAKE_CHAR    0xAAU
  /* SL message type are reserved from 0x00 to 0x7F for SE/ZCL, application can use from 0x80 */
#define SL_MSG_TYPE_NONE                   0x00U
#define SL_MSG_TYPE_APDU                   0x01U
#define SL_MSG_TYPE_NWK                    0x02U
#define SL_MSG_TYPE_EXCEPTION              0x03U
#define SL_MSG_TYPE_INTERPAN               0x04U
#define SL_MSG_TYPE_JN_RESETED             0x05U
#define SL_MSG_TYPE_NODE_PARENT            0x06U
#define SL_MSG_TYPE_ZDP_MSG                0x07U
#define SL_MSG_TYPE_MAC_POLL_FAILURE       0x08U
#define SL_MSG_TYPE_MAC_RESTORED           0x09U
#define SL_MSG_TYPE_OL_RESETED             0x0AU
#define SL_MSG_STACK_STARTED_RUNNING       0x0BU

#define SL_MSG_TYPE_APDU_PROCESSED 0x7FU

#define SL_WAIT_STATUS      0x01U

/*  extra large WAN payload */
#define MAX_TX_SERIAL_BUFFER_SIZE       (33+APDU_PAYLOAD_SIZE+8)

#define MAX_RX_LARGE_SERIAL_BUFFERS       12U
#define MAX_RX_SMALL_SERIAL_BUFFERS       26U

#ifndef MAX_RX_SMALL_SERIAL_BUFFER_SIZE
#define MAX_RX_SMALL_SERIAL_BUFFER_SIZE    250U
#endif
#ifndef MAX_RX_LARGE_SERIAL_BUFFER_SIZE
#define MAX_RX_LARGE_SERIAL_BUFFER_SIZE    (33U+APDU_PAYLOAD_SIZE+8U)
#endif
/* value of minimum serial Rx buffer to handle data inidcation can be configured by application
 * define this macro in app_option.h */
#ifndef MIN_RX_SERIAL_BUFFER_TO_HANDLE_DATA_INDICATION
#define MIN_RX_SERIAL_BUFFER_TO_HANDLE_DATA_INDICATION       5U
#endif

#define IGNORE_MSG_TYPE                           0xFFFFU

#define JN_UART_ACK_TIMEOUT                     30U
#define MAX_RX_SERIAL_BUFFERS        (MAX_RX_LARGE_SERIAL_BUFFERS + MAX_RX_SMALL_SERIAL_BUFFERS)

#define MAX_NUM_OF_CHANNEL_MASK         5

#define SL_MSG_TYPE_IDX             1U
#define SL_MSG_TYPE_LSB_IDX         1U
#define SL_MSG_TYPE_MSB_IDX         2U
#define SL_MSG_RX_LEN_IDX           3U
#define SL_MSG_RX_LEN_LSB_IDX       3U
#define SL_MSG_RX_LEN_MSB_IDX       4U
#define SL_MSG_TX_SEQ_NO_IDX        5U
#define SL_MSG_RX_SEQ_NO_IDX        6U
#define SL_MSG_RX_CRC__IDX          7U
#define SL_MSG_DATA_START_IDX       8U
#define SL_MSG_RX_STATUS_IDX        SL_MSG_DATA_START_IDX
#define SL_MSG_TSN_IDX              9U
#define SL_MSG_RSP_TYPE_MSB_IDX     10U   // NB byte order of this value is reversed to everthing else
#define SL_MSG_RSP_TYPE_LSB_IDX     11U // NB byte order of this value is reversed to everthing else
#define SL_MSG_RSP_START_IDX        12U


/*
 *  Format of serial messsage
 * ============================================================================================
 * | Index in Serial Buffer |   0   | 1, 2 | 3, 4     |  5    |   6   |   7  | 8..8+n  | 8+n  |
 * |------------------------|-------|------|----------|-------|   ----|------|---------|------|
 * |                        | start | Msg  | Data     |  Tx   |  Rx   | CRC  | Data    | End  |
 * |                        | char  | Type | Length n |  Seq  |  Seq  |      | n bytes | Char |
 * ============================================================================================
 *
 *   Format of Data part of a Status Message (response to a command)
 * =============================================================================================
 * | Index in Serial Buffer |   8    |   9    | 10, 11   |     12        | .. |     n+8        |
 * |------------------------|--------|--------|----------|---------------|----|----------------|
 * | Index in Data Part     |   0    |   1    | 2, 3     |     4         | .. |     n-1        |
 * |------------------------|--------|--------|----------|---------------|----|----------------|
 * |                        | Status | Seq No | RSP Type | 1st Data byte | .. | Last data Byte |
 * =============================================================================================
 *
 */

/* Contstant to control  when and how many tx retries are attempted */
#define TX_RESPONSE_TIME_MS     (100U)
#define LONG_TX_RESPONSE_TIME_MS     (2500U)
#define TX_RETRY_COUNT          5U
#define Tx_RETRIES_BEFORE_RESET (TX_RETRY_COUNT-2U)

#define JN_ERROR_APDU_TOO_SMALL                 ((uint8)ZPS_ERROR_APDU_TOO_SMALL + 0x30U)
#define JN_ERROR_APDU_INSTANCES_EXHAUSTED       ((uint8)ZPS_ERROR_APDU_INSTANCES_EXHAUSTED + 0x30U)
#define JN_ERROR_NO_APDU_CONFIGURED             ((uint8)ZPS_ERROR_NO_APDU_CONFIGURED + 0x30U)
#define JN_ERROR_OS_MESSAGE_QUEUE_OVERRUN       ((uint8)ZPS_ERROR_OS_MESSAGE_QUEUE_OVERRUN + 0x30U)
#define JN_ERROR_APS_SECURITY_FAIL              ((uint8)ZPS_ERROR_APS_SECURITY_FAIL + 0x30U)
#define JN_ERROR_ZDO_LINKSTATUS_FAIL            ((uint8)ZPS_ERROR_ZDO_LINKSTATUS_FAIL + 0x30U)

#define QUEUE_MSG_BY_VALUE 0xFF000000U /*Used for sending a value to the app queue*/

/* Signals that the JN can accept commands sent from the host */
#define JN_READY_FOR_COMMANDS                   (1u << 0u)

#ifdef SL_HOST_TO_COPROCESSOR_SECURE
/* Number of bytes to use as Message Integrity Check (MIC) for secured communication */
#define SL_SECURED_MSG_MIC_SIZE             8

/*
 *  Format of secured serial messsage
 * ==========================================================================================================
 * | Index in Serial Buffer |   0   | 1, 2 | 3, 4          |  5  |  6  |  7  | 8..8+n  | 8+n  | 8+n+MICsize |
 * |------------------------|-------|------|-------------- |-----|-----|-----|---------|------|-------------|
 * |                        | start | Msg  | Data Length + | Tx  | Rx  | CRC | Data    | MIC  | End         |
 * |                        | char  | Type | MIC size      | Seq | Seq |     | n bytes |      | Char        |
 * ==========================================================================================================
 *
 */
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/** Serial link message types */
typedef enum
{
    /* Common Commands */
    E_SL_MSG_STATUS_MSG                             =   0x7FFFU,
    E_SL_MSG_STATUS_SHORT_MSG                       =   0x8000U,
    E_SL_MSG_EXCEPTION                              =   0x8001U,
    E_SL_MSG_DATA_INDICATION                        =   0x8002U,
    E_SL_MSG_NODE_CLUSTER_LIST                      =   0x8003U,
    E_SL_MSG_NODE_ATTRIBUTE_LIST                    =   0x8004U,
    E_SL_MSG_NODE_COMMAND_ID_LIST                   =   0x8005U,
    E_SL_MSG_NODE_NON_FACTORY_NEW_RESTART           =   0x8006U,
    E_SL_MSG_NODE_FACTORY_NEW_RESTART               =   0x8007U,
    E_SL_MSG_DATA_CONFIRM                           =   0x8008U,
    E_SL_MSG_DATA_ACK                               =   0x8009U,
    E_SL_MSG_EVENT_RESET                            =   0x800AU,
    E_SL_MSG_EVENT_OL_RESET                         =   0x800BU,
    E_SL_MSG_EVENT_OL_REALIGN                       =   0x800CU,
    E_SL_MSG_EVENT_OL_ASSERT                        =   0x800DU,
    E_SL_MSG_EVENT_OL_BEACON                        =   0x800EU,
    E_SL_MSG_GET_VERSION                            =   0x0010U,
    E_SL_MSG_VERSION_LIST                           =   0x8010U,
    E_SL_MSG_NEW_NODE_HAS_JOINED                    =   0x8011U,
    E_SL_MSG_ZDO_LINK_KEY_EVENT                     =   0x8012U,
    E_SL_MSG_JEN_OS_ERROR                           =   0x8013U,
    E_SL_MSG_ZPS_ERROR                              =   0x8014U,
    E_SL_MSG_INTERPAN_DATA_INDICATION               =   0x8015U,
    E_SL_MSG_INTERPAN_DATA_CONFIRM                  =   0x8016U,
    E_SL_MSG_NWK_DISCOVERY_COMPLETE                 =   0x8017U,
    E_SL_MSG_SLEEP                                  =   0x8018U,
    E_SL_MSG_READ_JN_TEMP_VALUE                     =   0x8019U,
    E_SL_MSG_JN_PRE_SLEEP_INDICATION                =   0x8020U,
    E_SL_MSG_JN_WAKE_UP_INDICATION                  =   0x8021U,
    E_SL_MSG_PDM_ERROR_EVENT                        =   0x8022U,
    E_SL_MSG_ZDP_DATA_INDICATION                    =   0x8023U,
    E_SL_MSG_HOST_JN_NACK                           =   0x8025U,
    E_SL_MSG_SET_JN_ACK_TIMEOUT                     =   0x8026U,
    E_SL_SET_ED_THRESHOLD                           =   0x8027U,
    E_SL_MSG_LOG                                    =   0x8028U,
    E_SL_MSG_MODULE_TEST                            =   0x8029U,
    E_SL_MSG_ZPS_INTERNAL_ERROR                     =   0x802AU,
    E_SL_MSG_ZPS_FRAME_COUNTER_HIGH                 =   0x802BU,
    E_SL_MSG_ZPS_DEFAULT_STACK                      =   0x802CU,
    E_SL_MSG_ZPS_SET_KEYS                           =   0x802DU,
    E_SL_MSG_ZPS_SAVE_ALL_RECORDS                   =   0x802EU,

    E_SL_MSG_UNICAST_ACK_DATA_REQ                   =   0x0001U,
    E_SL_MSG_UNICAST_DATA_REQ                       =   0x0002U,
    E_SL_MSG_UNICAST_IEEE_ACK_DATA_REQ              =   0x0003U,
    E_SL_MSG_UNICAST_IEEE_DATA_REQ                  =   0x0004U,
    E_SL_MSG_BOUND_ACK_DATA_REQ                     =   0x0005U,
    E_SL_MSG_BOUND_DATA_REQ                         =   0x0006U,
    E_SL_MSG_BROADCAST_DATA_REQ                     =   0x0007U,
    E_SL_MSG_GROUP_DATA_REQ                         =   0x0008U,
    E_SL_MSG_FIND_EXT_ADDR                          =   0x0009U,
    E_SL_MSG_GET_DEVICE_PERMISSIONS                 =   0x000AU,
    E_SL_MSG_SET_DEVICE_PERMISSIONS                 =   0x000BU,
    E_SL_MSG_ADD_REPLACE_LINK_KEY                   =   0x000CU,
    E_SL_MSG_GET_EXT_ADDR                           =   0x000DU,
    E_SL_MSG_SET_TC_ADDRESS                         =   0x000EU,
    E_SL_MSG_SET_INIT_SEC_STATE                     =   0x000FU,


    E_SL_MSG_SET_OVERRIDE_LOCAL_MAC_ADDR            =   0x0050U,
    E_SL_MSG_GET_NWK_ADDR                           =   0x0051U,
    E_SL_MSG_GET_ENDPOINT_STATE                     =   0x0052U,
    E_SL_MSG_SET_ENDPOINT_STATE                     =   0x0053U,
    E_SL_MSG_IS_DEVICE_KEY_PRESENT                  =   0x0054U,
    E_SL_MSG_GET_BINDING_ENTRY                      =   0x0055U,
    E_SL_MSG_GET_KEY_TABLE_ENTRY                    =   0x0056U,
    E_SL_MSG_REMOVE_REMOTE_DEVICE                   =   0x0057U,
    E_SL_MSG_GET_MAPPED_IEEE_ADDR                   =   0x0058U,
    E_SL_MSG_GET_NWK_KEY_TABLE_ENTRY                =   0x005BU,
    E_SL_MSG_ADD_ADDRESS_MAP_ENTRY                  =   0x005DU,
    E_SL_MSG_GET_NEIGHBOR_TABLE_ENTRY               =   0x005EU,
    E_SL_MSG_GET_ADDRESS_MAP_TABLE_ENTRY            =   0x005FU,
    E_SL_MSG_GET_ROUTING_TABLE_ENTRY                =   0x0060U,
    E_SL_MSG_KEC_GENERATE_EPHEMERAL_KEYS            =   0x0061U,
    E_SL_MSG_KEC_KEY_DERIVATIVE                     =   0x0062U,
    E_SL_MSG_KEC_VERIFY_MAC                         =   0x0063U,
    E_SL_MSG_KEC_GENERATE_EPHEMERAL_KEYS_283K1      =   0x0064U,
    E_SL_MSG_KEC_KEY_DERIVATIVE_283K1               =   0x0065U,
    E_SL_MSG_KEC_VERIFY_MAC_283K1                   =   0x0066U,
    E_SL_MSG_ECC_RECONSTRUCT_PK                     =   0x0067U,
    E_SL_MSG_ECC_RECONSTRUCT_PK_283K1               =   0x0068U,
    E_SL_MSG_ECC_ECDSA_VERIFY                       =   0x0069U,
    E_SL_MSG_ECC_ECDSA_VERIFY_283K1                 =   0x0070U,


    E_SL_MSG_SET_EXT_PANID                          =   0x0020U,
    E_SL_MSG_SET_CHANNELMASK                        =   0x0021U,
    E_SL_MSG_SET_SECURITY                           =   0x0022U,
    E_SL_MSG_ZDO_SET_DEVICETYPE                     =   0x0023U,
    E_SL_MSG_START_NETWORK                          =   0x0024U,
    E_SL_MSG_START_SCAN                             =   0x0025U,
    E_SL_MSG_NETWORK_JOINED_FORMED                  =   0x8024U,
    E_SL_MSG_NETWORK_REMOVE_DEVICE                  =   0x0026U,
    E_SL_MSG_REJOIN_NETWORK                         =   0x0027U,
    E_SL_MSG_SEARCH_TRUST_CENTER                    =   0x0028U,
    E_SL_MSG_CHANGE_CHANNEL                         =   0x0029U,
    E_SL_MSG_INTERPAN_DATA_REQ                      =   0x002AU,
    E_SL_MSG_SETUP_FOR_INTERPAN                     =   0x002BU,
    E_SL_MSG_SET_CHANNEL                            =   0x002CU,
    E_SL_MSG_SET_TX_POWER                           =   0x002DU,
    E_SL_MSG_REMOVE_LINK_KEY                        =   0x002EU,
    E_SL_MSG_CONVERT_ENERGY_TO_DBM                  =   0x002FU,
    E_SL_MSG_INSECURE_REJOIN_NETWORK                =   0x004CU,
    E_SL_MSG_SET_NUM_EZ_SCANS                       =   0x0071U,
    E_SL_MSG_STACK_STARTED_RUNNING                  =   0x0072U,
    E_SL_MSG_READ_OL_PARAMS                         =   0x0073U,
    E_SL_MSG_GET_REPROVISSION_DATA                  =   0x0074U,
    E_SL_MSG_SET_REPROVISSION_DATA                  =   0x0075U,
    E_SL_MSG_INSECURE_REJOIN_SHORT_PAN              =   0x0076u,
    E_SL_MSG__REJOIN_WITH_CHANNEL_MASK              =   0x0077u,
    E_SL_MSG_GET_DEFAULT_DISTRIBUTED_APS_LINK_KEY   =   0x0078U,
    E_SL_MSG_GET_DEFAULT_GLOBAL_APS_LINK_KEY        =   0x0079U,
    E_SL_MSG_GET_DEFAULT_TC_APS_LINK_KEY            =   0x0080U,
    E_SL_MSG_GET_APS_SEQ_NUM                        =   0x0081U,

    E_SL_MSG_RESET                                  =   0x0011U,
    E_SL_MSG_ERASE_PERSISTENT_DATA                  =   0x0012U,
    E_SL_MSG_ZLL_FACTORY_NEW                        =   0x0013U,
    E_SL_MSG_GET_PERMIT_JOIN                        =   0x0014U,
    E_SL_MSG_ENABLE_DISABLE_JN_POWER                =   0x0015U,
    E_SL_MSG_GET_CLUSTER_DISCOVERY_STATE            =   0x0016U,
    E_SL_MSG_SET_CLUSTER_DISCOVERY_STATE            =   0x0017U,
    E_SL_MSG_ZDO_REMOVE_DEVICE_REQ                  =   0x0018U,
    E_SL_MSG_GET_JN_CHIP_ID                         =   0x0019U,
    E_SL_MSG_SET_APS_FRAME_COUNTER                  =   0x001AU,
    E_SL_MSG_GET_BOOTLOADER_VERSION                 =   0x001BU,
    E_SL_MSG_NWK_MANAGER_STATE                      =   0x001CU,
    E_SL_MSG_GET_MAC_TYPE                           =   0x001DU,
    E_SL_MSG_SERIAL_LINK_SET_MANUFACTURER_CODE      =   0x001EU,
    E_SL_MSG_BIND                                   =   0x0030U,
    E_SL_MSG_BIND_RESPONSE                          =   0x8030U,
    E_SL_MSG_UNBIND                                 =   0x0031U,
    E_SL_MSG_UNBIND_RESPONSE                        =   0x8031U,

    E_SL_MSG_GET_EXT_PANID                          =   0x0032U,
    E_SL_MSG_GET_CURRENT_CHANNEL                    =   0x0033U,
    E_SL_MSG_GET_CHANNELMASK                        =   0x0034U,
    E_SL_MSG_GET_SHORT_ADDRESS_OF_DEVICE            =   0x0035U,
    E_SL_MSG_GET_SHORT_PANID                        =   0x0036U,
    E_SL_MSG_GET_SIMPLE_DESCRIPTOR                  =   0x0037U,
    E_SL_MSG_GET_USE_EXT_PANID                      =   0x0038U,
    E_SL_MSG_GET_USE_INSECURE_JOIN                  =   0x0039U,
    E_SL_MSG_SET_USE_INSTALL_CODE                   =   0x003AU,

    E_SL_MSG_NETWORK_ADDRESS_REQUEST                =   0x0040U,
    E_SL_MSG_NETWORK_ADDRESS_RESPONSE               =   0x8040U,
    E_SL_MSG_IEEE_ADDRESS_REQUEST                   =   0x0041U,
    E_SL_MSG_IEEE_ADDRESS_RESPONSE                  =   0x8041U,
    E_SL_MSG_NODE_DESCRIPTOR_REQUEST                =   0x0042U,
    E_SL_MSG_NODE_DESCRIPTOR_RESPONSE               =   0x8042U,
    E_SL_MSG_SIMPLE_DESCRIPTOR_REQUEST              =   0x0043U,
    E_SL_MSG_SIMPLE_DESCRIPTOR_RESPONSE             =   0x8043U,
    E_SL_MSG_POWER_DESCRIPTOR_REQUEST               =   0x0044U,
    E_SL_MSG_POWER_DESCRIPTOR_RESPONSE              =   0x8044U,
    E_SL_MSG_ACTIVE_ENDPOINT_REQUEST                =   0x0045U,
    E_SL_MSG_ACTIVE_ENDPOINT_RESPONSE               =   0x8045U,
    E_SL_MSG_MATCH_DESCRIPTOR_REQUEST               =   0x0046U,
    E_SL_MSG_MATCH_DESCRIPTOR_RESPONSE              =   0x8046U,
    E_SL_MSG_MANAGEMENT_LEAVE_REQUEST               =   0x0047U,
    E_SL_MSG_MANAGEMENT_LEAVE_RESPONSE              =   0x8047U,
    E_SL_MSG_LEAVE_INDICATION                       =   0x8048U,
    E_SL_MSG_LEAVE_CONFIRM                          =   0x8049U,
    E_SL_MSG_PERMIT_JOINING_REQUEST                 =   0x0049U,
    E_SL_MSG_MANAGEMENT_NETWORK_UPDATE_REQUEST      =   0x004AU,
    E_SL_MSG_MANAGEMENT_NETWORK_UPDATE_RESPONSE     =   0x804AU,
    E_SL_MSG_SYSTEM_SERVER_DISCOVERY                =   0x004BU,
    E_SL_MSG_SYSTEM_SERVER_DISCOVERY_RESPONSE       =   0x804BU,
    E_SL_MSG_DEVICE_ANNOUNCE                        =   0x004DU,
    E_SL_MSG_ZDO_PERMIT_JOIN_REQUEST                =   0x004EU,
    E_SL_MSG_TEST_TYPE                              =   0x004FU,
    E_SL_MSG_GET_APS_KEY_TABLE_SIZE                 =   0x0400U,
    E_SL_MSG_GET_NEIGHBOR_TABLE_SIZE                =   0x0401U,
    E_SL_MSG_SET_POLL_CONFIG                        =   0x0402U,
    E_SL_MSG_REQUEST_KEY_REQ                        =   0x0403U,
    E_SL_MSG_GET_ADDRESS_MAP_TABLE_SIZE             =   0x0404U,
    E_SL_MSG_GET_ROUTING_TABLE_SIZE                 =   0x0405U,
    E_SL_MSG_UPDATE_DEFAULT_LINK_KEY                =   0x0406U,
    E_SL_MSG_GET_TC_ADDRESS                         =   0x0407U,
    E_SL_MSG_TRANSPORT_NETWORK_KEY                  =   0x0408U,
    E_SL_MSG_SWITCH_KEY_REQUEST                     =   0x0409U,
    E_SL_MSG_CLEAR_NETWORK_KEY                      =   0x040AU,
    E_SL_MSG_SET_NETWORK_KEY                        =   0x040BU,
    E_SL_MSG_MAC_SET_FILTER_STORAGE                 =   0x040CU,
    E_SL_MSG_MAC_FILTER_ADD_ACCEPT                  =   0x040DU,
    E_SL_MSG_GET_SEC_MAT_SET_SIZE                   =   0x040EU,
    E_SL_MSG_GET_SEC_MAT_SET_ENTRY                  =   0x040FU,
    E_SL_MSG_CLEAR_SEC_MAT_SET_ENTRY                =   0x0410U,
    E_SL_MSG_SET_END_DEVICE_TIMEOUT                 =   0x0411U,
    E_SL_MSG_SERIAL_LINK_SEND_TEST                  =   0x04FFU,
    E_SL_MSG_SET_END_DEVICE_PERMISSIONS             =   0x0500U,
    E_SL_MSG_TRANSPORT_KEY_DECIDER                  =   0x0501U,
    E_SL_MSG_SERIAL_LINK_SEND_BACK                  =   0x0502U,
    E_SL_MSG_SET_JN_INTERNAL_ATTENUATOR             =   0x0503U,
    E_SL_MSG_GET_LAST_RSSI                          =   0x0504U,
    E_SL_MSG_GET_GLOBAL_STATS                       =   0x0505U,
    E_SL_MSG_GET_DEVICE_STATS                       =   0x0506U,
    E_SL_MSG_CHANGE_EXT_PANID                       =   0x0507U,
    E_SL_MSG_CHANGE_PANID                           =   0x0628U,
    E_SL_MSG_DISCOVER_NETWORKS                      =   0x0508U,
    E_SL_MSG_GET_RESTORE_POINT                      =   0x0509U,
    E_SL_MSG_SET_RESTORE_POINT                      =   0x050AU,
    E_SL_MSG_GET_MAC_ADDR_TABLE                     =   0x050BU,
    E_SL_MSG_SET_MAC_ADDR_TABLE                     =   0x050CU,
    E_SL_MSG_SET_KEY_TABLE_ENTRY                    =   0x050DU,
    E_SL_MSG_REMOVE_DEVICE_FROM_BINDING_TABLE       =   0x050EU,
    E_SL_MSG_STOP_POLL_CYCLE                        =   0x050FU,
    E_SL_MSG_NWK_JOIN_TIMEOUT                       =   0x0510U,
    E_SL_MSG_STOP_EZ_SET_UP                         =   0x0511U,
    E_SL_MSG_SET_STAY_AWAKE_FLAG                    =   0x0512U,
    E_SL_MSG_SET_MAX_MAC_FAIL_COUNT                 =   0x0513U,
    E_SL_MSG_MAC_POLL_FAILURE                       =   0x0514U,
    E_SL_MSG_GET_MAC_TABLE_ENTRY                    =   0x0516U,
    E_SL_MSG_ENABLE_DISABLE_DUAL_MAC                =   0x0517U,
    E_SL_MSG_GET_JN_PDMUSE                          =   0x0518U,

    E_SL_MSG_CHANGE_SUB_GHZ_CHANNEL                 =   0x060AU,

    E_SL_MSG_GET_PIB_ATTR                           =   0x060BU,
    E_SL_MSG_SET_NWK_INTERFACE_REQ                  =   0x060CU,
    E_SL_MSG_GET_NWK_INTERFACE_REQ                  =   0x060DU,

    E_SL_MSG_SEND_MGMT_NWK_ENH_UPDATE_REQ           =   0x0610U,
    E_SL_MSG_SEND_MGMT_NWK_UNS_ENH_UPDATE_NOTIFY    =   0x0611U,
    E_SL_MSG_MGMT_NETWORK_ENH_UPDATE_RESPONSE       =   0x0612U,
    E_SL_MSG_MGMT_NETWORK_UNS_ENH_UPDATE_NOTIFY     =   0x0613U,
    E_SL_MSG_START_ED_SCAN                          =   0x0614U,
    E_SL_MSG_NWK_ED_SCAN                            =   0x0615U,
    E_SL_MSG_GET_MAC_INTERFACE_INDEX                =   0x0616U,
    E_SL_MSG_GET_CLEAR_TX_UCAST_BYTE_COUNT          =   0x0617U,
    E_SL_MSG_GET_CLEAR_RX_UCAST_BYTE_COUNT          =   0x0618U,
    E_SL_MSG_GET_CLEAR_TX_FAIL_COUNT                =   0x0619U,
    E_SL_MSG_GET_CLEAR_TX_RETRY_COUNT               =   0x061AU,
    E_SL_MSG_SET_EBR_PERMIT_JOIN_ON                 =   0x061BU,

    E_SL_MSG_SET_ED_TIMEOUT_ON_PARENT               =   0x0620U,

    E_SL_MSG_GET_CLEAR_TX_COUNT                     =   0x0622U,
    E_SL_MSG_MGMT_NWK_UPDATE_REQ                    =   0x0623U,
    E_SL_MSG_MGMT_ENHANCED_NWK_UPDATE_REQ           =   0x0624U,

    E_SL_MSG_SET_SUB_GHZ_ANTENNA                    =   0x0627U,
    E_SL_MSG_GET_APS_FC_IEEE                        =   0x0629U,
    E_SL_MSG_GET_APS_SERIAL_STATS                   =   0x062AU,
    E_SL_MSG_SERIAL_SEQ_TEST                        =   0x062BU,
    E_SL_MSG_GET_POLL_STATE                         =   0x062CU,
    E_SL_MSG_SET_SECURITY_TIMEOUT                   =   0x062DU,
    E_SL_MSG_SEND_ZED_TIMEOUT                       =   0x062EU,
    E_SL_MSG_SET_PARENT_TIMEOUT                     =   0x062FU,
    E_SL_MSG_NWK_STATUS_INDICATION                  =   0x8071U,
    E_SL_MSG_FRAGMENTED_PACKET_CHECK                =   0x8072U,
    E_SL_MSG_MAC_RESTORED                           =   0x8073U,
    E_SL_MSG_ZDO_BIND                               =   0x8074U,
    E_SL_MSG_SET_NWK_FRAME_COUNT                    =   0x8075U,
    E_SL_MSG_SERIAL_LINK_REQ_NEGATIVE               =   0x8076U,
    E_SL_MSG_SET_MAC_INTERFACE                      =   0x8077U,
    E_SL_MSG_HOST_JN_ACK                            =   0x8078U,
    E_SL_MSG_SERIAL_LINK_HALT_JN                    =   0x8080U,
    E_SL_MSG_SERIAL_LINK_BAD_TABLE                  =   0x8081U,
    E_SL_MSG_SERIAL_LINK_ROLLBACK_TEST              =   0x8082U,
    E_SL_MSG_ORPHAN_REJOIN_NWK                      =   0x8083U,
    E_SL_MSG_SERIAL_LINK_COUNT_APDU                 =   0x8084U,
    E_SL_MSG_SERIAL_LINK_EXHAUST_APDU               =   0x8085U,
    E_SL_MSG_SERIAL_LINK_STACK_POLL_RATE            =   0x8086U,
    E_SL_MSG_SERIAL_LINK_GET_NWK_INFC               =   0x8087U,
    E_SL_MSG_SERIAL_LINK_RESET_OL_MODULE            =   0x8088U,
    E_SL_MSG_SERIAL_LINK_SET_LEAVE_DECIDER          =   0x8089U,
    E_SL_MSG_SERIAL_LINK_EXHAUST_REQ_DESC           =   0x808AU,
    E_SL_MSG_SERIAL_LINK_GET_OL_REALIGN             =   0x808BU,
    E_SL_MSG_SERIAL_LINK_GET_OL_CHANNEL             =   0x808CU,
    E_SL_MSG_SERIAL_LINK_TOGGLE_OL_RESET            =   0x808DU,
    E_SL_MSG_SERIAL_LINK_GET_RXONIDLE               =   0x808EU,
    E_SL_MSG_SERIAL_LINK_SET_RXONIDLE               =   0x808FU,
    E_SL_MSG_SERIAL_LINK_SLEEP_OL                   =   0x8090U,
    E_SL_MSG_SERIAL_LINK_WAKE_OL                    =   0x8091U,
    E_SL_MSG_GET_NWK_OUTGOING_FRAME_COUNT           =   0x8092U,
    E_SL_MSG_SERIAL_LINK_TEST_ERR_CODE              =   0x8093U,
    E_SL_MSG_SERIAL_LINK_GET_STATUS_FLAGS           =   0x8094U,
    E_SL_MSG_SERIAL_LINK_SET_OL_IO_AS_INPUTS        =   0x8095U,
    E_SL_MSG_SERIAL_LINK_RESET_BIND_SERVER          =   0x8096U,
    E_SL_MSG_SERIAL_LINK_SET_COMPLIANCE_LIMITS      =   0x8097U,
    E_SL_MSG_SERIAL_LINK_ENABLE_BOUND_DEVICES       =   0x8098U,
    E_SL_MSG_SERIAL_LINK_SET_BOUND_DEVICE           =   0x8099U,
    E_SL_MSG_SERIAL_LINK_BIND_INDICATION            =   0x809AU,
    E_SL_MSG_SERIAL_LINK_FORCE_OL_ALIGN             =   0x809BU,
    E_SL_MSG_SERIAL_LINK_GET_NWK_STATE              =   0x809CU,
    E_SL_MSG_SERIAL_LINK_CLONE_ZED                  =   0x809DU,
    E_SL_MSG_START_ROUTER                           =   0x9000U,
    E_SL_MSG_PDM_CONVERT                            =   0x9003U,
    E_SL_MSG_BLOCK_MDS                              =   0x9004U,
    E_SL_MSG_NWK_ROUTE_DISCOVERY                    =   0x9005U,

    E_SL_MSG_PANID_CNFL_INDICATION                  =   0x9006U,
    E_SL_MSG_SET_PANID_CNFL_RSVL                    =   0x9007U,

    E_SL_MSG_SEL_BAND_SCAN_ASSOC                    =   0x9008U,
    E_SL_MSG_GET_BAND_SCAN_ASSOC                    =   0x9009U,
    E_SL_MSG_NETWORK_CHANGE_ADDRESS                 =   0x900AU,
    E_SL_MSG_GET_SECURITY_TIMEOUT                   =   0x900BU,

    E_SL_MSG_JN_GET_STATE                           =   0x900CU, /** JN can receive commands from LPC */
    E_SL_MSG_GET_NEIGHBOR_TABLE_ENTRY_BY_ADDRESS    =   0x900DU,
    E_SL_MSG_ZDO_REMOVE_DEVICE_FROM_TABLES          =   0x900EU,

    E_SL_MSG_CONVERT_LQI_TO_RSSI_DBM                =   0x900FU,

    E_SL_MSG_ZDO_UNBIND                             =   0x9075U, /** Complementary ZDO_BIND command */
    E_SL_MSG_SEND_DATA_REQUEST                      =   0x9076U,
    E_SL_MSG_ADD_REPLACE_INSTALL_CODES              =   0x9077U,
    E_SL_MSG_SET_UPDATE_ID                          =   0x9078U,
    E_SL_MSG_GET_NWK_KEY                            =   0x9079U,
    E_SL_MSG_APSDE_DATA_REQUEST                     =   0x907AU,
    E_SL_MSG_NWK_SET_DEVICETYPE                     =   0x907BU,
    E_SL_SET_KEY_SEQ_NUMBER                         =   0x907CU,
    E_SL_MSG_SET_NWK_ADDR                           =   0x907DU,
    E_SL_MSG_SET_MAC_CAPABILITY                     =   0x907EU,
    E_SL_MSG_SET_NWK_STATE_ACTIVE                   =   0x907FU,
    E_SL_MSG_SET_DEPTH                              =   0x9080U

} teSL_MsgType;


/** Status message */
typedef struct
{
    enum
    {
        E_SL_MSG_STATUS_SUCCESS,                /**< Success (0x00) */
        E_SL_MSG_STATUS_INCORRECT_PARAMETERS,   /**< Incorrect parameters provided (0x01) */
        E_SL_MSG_STATUS_UNHANDLED_COMMAND,      /**< Command ID is unhandled (0x02) */
        E_SL_MSG_STATUS_BUSY,                   /**< JN is busy in another action (0x03) */
        E_SL_MSG_STATUS_STACK_ALREADY_STARTED,  /**< Stack is already started (0x04) */
        E_SL_MSG_STATUS_TIME_OUT,               /**< Command sent has timed-out (0x05) */
        E_SL_MSG_NO_APDU_BUFFERS,               /**< No APDU buffer available (0x06) */
        E_SL_MSG_STATUS_UNSUPPORTED_COMMAND,    /**< Command ID is unhandled (0x07) */
        E_SL_MSG_STATUS_PDM_BAD_PARAMS,         /**< Bad parameters provided for PDM (0x08) */
        E_SL_MSG_STATUS_PDM_ALREADY_DONE,       /**< PDM already completed (0x09) */
        E_SL_MSG_STATUS_PDM_UNKNOWN_VERSION,    /**< PDM version unknown (0x0A) */
        E_SL_MSG_STATUS_HARDWARE_FAILURE,       /**< Hardware Failure (0x0B) */
        E_SL_MSG_STATUS_INVALID_PARAMETER,      /**< Invalied parameter provided (0x0C) */
        E_SL_MSG_STATUS_ILLEGAL_REQUEST,        /**< Illegel Request (0x0D) */
        E_SL_MSG_STATUS_C2_SUBSTITUTION         /**< Substitute 0xC2 errors in case of ressource-related errors (0x0E) */
    } eStatus;
    uint8 u8SeqNum;
    char                acMessage[];            /**< Optional message */
} tsSL_Msg_Status;


typedef enum
{
    E_SL_SUCCESS = 0x00,
    E_SL_ERR_PARAMETER_NULL,    // 01
    E_SL_ERR_PARAMETER_RANGE,   // 02
}teSL_Status;

typedef enum
{
    E_SL_BR_115200 = 0x00,
    E_SL_BR_230400,               // 01
    E_SL_BR_460800,               // 02
    E_SL_BR_921600,               // 03
}teSL_BaudRate;

/** ZigBee Device types */
typedef enum
{
    E_SL_DEVICE_COMMS_HUB,
    E_SL_DEVICE_IHD,
    E_SL_DEVICE_GAS_METER,
    E_SL_DEVICE_ESI_METER,
    E_SL_DEVICE_INVALID
}teSL_ZigBeeDeviceType;

typedef enum
{
    E_SL_MAC_UNDEFINED,
    E_SL_MAC_2400_GHZ,
    E_SL_MAC_868_MHZ,
    E_SL_MAC_DUAL_BAND,
    E_SL_MAC_MULTI_BAND
}teSL_DeviceMacType;

typedef enum
{
    E_SL_PDM_CMD_INIT,
    E_SL_PDM_CMD_SAVE,
    E_SL_PDM_CMD_READ,
    E_SL_PDM_CMD_CALLBACK
}teSL_PdmCmdType;

typedef struct
{
    volatile bool_t bIsSlMsgGetVer;
    volatile bool_t bIsWaitingStatus;
    uint8 *pu8CurrentRxBuffer;
    uint8 au8RxLargeSerialBuffer[MAX_RX_LARGE_SERIAL_BUFFERS][MAX_RX_LARGE_SERIAL_BUFFER_SIZE];
    uint8 au8RxSmallSerialBuffer[MAX_RX_SMALL_SERIAL_BUFFERS][MAX_RX_SMALL_SERIAL_BUFFER_SIZE];
}tsSL_Common;

typedef struct
{
    uint16 u16NwkAddr;
    uint64 u64MacAddr;
}tsSL_AddrMapTable;

/** Enumerated list of states for receive state machine */
typedef enum
{
    E_STATE_RX_WAIT_START,
    E_STATE_RX_WAIT_TYPEMSB,
    E_STATE_RX_WAIT_TYPELSB,
    E_STATE_RX_WAIT_LENMSB,
    E_STATE_RX_WAIT_LENLSB,
    E_STATE_RX_WAIT_TX_SEQ_NO,
    E_STATE_RX_WAIT_RX_SEQ_NO,
    E_STATE_RX_WAIT_CRC,
    E_STATE_RX_WAIT_DATA,
}teSL_RxState;


typedef struct
{
    uint32 u32SLRxCount;
    uint32 u32SLRxFail;
    uint32 u32SLTxStatusMsg;
    uint32 u32SLTxEventMsg;
    uint32 u32OverwrittenRXMessage;
    uint32 u32SLTxRetries;
    uint32 u32SLTxFailures;
    uint32 u32SL5Ms;
    uint32 u32SL8Ms;
    uint32 u32SL10Ms;
    uint32 u32Greater10Ms;
} tsJNSerialStats;

typedef struct
{
    uint32 u32SLTxCount;
    uint32 u32SLTxRetries;
    uint32 u32SLTxFailures;

    uint32 u32SLRxStatusMsg;
    uint32 u32SLRxEventMsg;
    uint32 u32SLRxFail;

    uint32 u32FiveMs;
    uint32 u32TenMs;
    uint32 u32FiftyMs;
    uint32 u32Greater50ms;
    uint32 u32MaxDelay;
    uint32 u32PktType;
} tsLPCSerialStats;


typedef enum
{
    E_SL_API_ERROR_SL_NO_APDU_IDX,
    E_SL_API_ERROR_SL_A3_IDX,
    E_SL_API_ERROR_SL_A6_IDX,
    E_SL_API_ERROR_SL_C2_IDX,
    E_SL_API_ERROR_SL_C3_IDX,
    E_SL_API_ERROR_SL_SUBST_IDX,
    E_SL_API_ERROR_SL_EXT_NPDU_IDX,
    E_SL_API_ERROR_SL_EXT_APDU_IDX,
    E_SL_API_ERROR_SL_EXT_SDR_IDX,
    E_SL_API_ERROR_SL_EXT_APS_ACK_IDX,
    E_SL_API_ERROR_SL_EXT_FRAG_REC_IDX,
    E_SL_API_ERROR_SL_EXT_MCPS_REC_IDX,
    E_SL_API_ERROR_SL_EXT_LOOP_IDX,
    E_SL_API_ERROR_SL_EXT_APSDE_IDX,
    E_SL_API_ERROR_SL_EXT_RT_IDX,
    E_SL_API_ERROR_SL_EXT_BTR_IDX,
    E_SL_API_ERROR_COUNT_SIZE
} tsseApierrorCounts;

typedef struct
{
    uint8 u8ErrorCode;
    uint16 u16ErrorCount;
    uint32 u32TotalCount;
} tsErrorLog;


#define SIZE_JN_ERRORS          81u
#define SIZE_EXT_JN_ERRORS      29U
/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
extern tsSL_Common sSLCommon;
extern bool_t bIsReadingZBVersion;
extern bool_t bIsJNInDeepSleep;
#ifdef ENABLE_UART_ACK_FROM_HOST
extern uint8 u8AckTestMode;
#endif

extern uint16 au16ApiErrors[E_SL_API_ERROR_COUNT_SIZE];
extern uint16 au16ApiErrorTotals[E_SL_API_ERROR_COUNT_SIZE];

extern tsErrorLog  asExtendedErrorLog[SIZE_EXT_JN_ERRORS];
extern tsErrorLog  asErrorLog[SIZE_JN_ERRORS];
PUBLIC void vSL_EmptyStatusMsgQueue(void);

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
PUBLIC void vSetIsInProgramMode(bool_t bSetIsInProgramMode);
PUBLIC bool_t bGetIsInProgramMode(void);
PUBLIC teSL_Status eSL_SerialInit(void);
PUBLIC uint8* pSL_ReadMessage(uint8 u8Data);
PUBLIC uint8 u8SL_WriteMessage(uint16 u16Type, uint16 u16Length, uint8 *pu8Data, void *pvData);
PUBLIC void vProcessIncomingSerialBytes(uint8 u8SerialData);
PUBLIC bool_t bSL_ReadJnBootLoaderMessage(uint8 u8Data);
PUBLIC uint8 u8SL_WriteJnBootLoaderMessage(uint16 u16Length, uint8 *pu8Data, uint8 **pu8RcvdData);

PUBLIC void APP_vSeHostCheckRxBuffer(void);
PUBLIC void APP_vNcpHostResetZigBeeModule(void);

PUBLIC void restartUARTRegs(void);
PUBLIC void restartUARTFreeRtosCommon(bool_t bGiveSLSemaphone);
#ifdef RESET_UART
PUBLIC void restartStateMachineSLCommon(bool_t bGiveSLSemaphone);
#endif

PUBLIC void vSL_TakeSerialSemaphore(void);
PUBLIC void vSL_GiveSerialSemaphore(void);
PUBLIC uint8 *pu8SL_GetRxMessageFromSerialQueue(uint16 u16MsgTypeTransmitted);
#ifdef MULTI_TASK_ENABLED
void vApp_SerialLinkTask (void *p_arg);
#endif
PUBLIC void vSL_PostMessageToSerialQueue(void *pvMessage);
PUBLIC void vSL_SerialLinkRtosTest(void);
PUBLIC void vSL_PostMessageToSerialRxQueue(void *pvMessage);
PUBLIC void vProcessIncomingSerialCommands(uint8 *pu8RxBuffer);
PUBLIC void vProcessIncomingSerialLogCommand(uint8 *pu8RxBuffer);  // added by JB
PUBLIC bool_t bSL_ValidateIncomingMessage(uint8 *pu8RxSerialBuffer);
PUBLIC void vSL_TxByte(bool_t bSpecialCharacter, uint8 u8Data);
PUBLIC void vSL_FreeRxBuffer(uint8 *pu8RxBuffer);
PUBLIC void vSL_ResetRxBufferPool(void);
PUBLIC void vSL_PrintRxBufferPool(bool bPrint);
PUBLIC uint8 vSL_CheckAndHandleSerialMsg(void);
PUBLIC void vSL_CheckAndHandleSerialLogMsg(void);
PUBLIC void vSL_FlushSerialQueue (void);
PUBLIC void vSL_FlushRxUartQueue(void);
PUBLIC uint32 u32SL_GetNumberOfFreeRxBuffers(void);
PUBLIC uint16 u16SL_GetMaxLargeBufferAllocated(void);
PUBLIC uint16 u16SL_GetMaxSmallBufferAllocated(void);
PUBLIC uint16 u16SL_GetLargeBufferAllocated(void);
PUBLIC uint16 u16SL_GetSmallBufferAllocated(void);
#if defined __cplusplus
}
#endif
PUBLIC void vSL_TaskEnterCritical(void);
PUBLIC void vSL_TaskExitCritical(void);
#ifdef ENABLE_UART_ACK_FROM_HOST
PUBLIC void Send_Nack_Host_JN(void);
#endif
PUBLIC ZPS_teStatus eSendGetClearMacIfTxCount(uint8 u8GetClear,
        uint8 u8MacId, uint32* pu32TxCount);
PUBLIC void vSendTestMsg(uint8* pu8TxBuffer, uint16 u16Length);
#ifdef ENABLE_UART_ACK_FROM_HOST
PUBLIC void Send_Ack_Host_JN(void);
#endif

PUBLIC void vFlushZclQueue(void);
PUBLIC void vFlushAppQueue(void);

PUBLIC void vGetLpcSerialStats( tsLPCSerialStats *psStats);
PUBLIC void vResetLpcSerialStats( void);

PUBLIC void vDecrementTxseqNo(void);
PUBLIC void vSL_SetLongResponsePeriod( void);
PUBLIC void vSL_SetStandardResponsePeriod( void);
PUBLIC uint32 u32SL_GetResponsePeriod(void);

#ifdef MONITOR_API_ERRORS
PUBLIC void vMonitorJnErrors(uint8 u8Status, bool_t bExtended);
PUBLIC void vSLResetErrorCounts(void);
#endif

PUBLIC void vMonitorAllJnErrors(uint8 u8Status, bool_t bExtended);
PUBLIC void vClearJNErrorLogs( bool_t bAll);
PUBLIC void vShowErrorLogs(void);


PUBLIC void vSetJNState(teJNState eState);
PUBLIC teJNState teGetJNState(void);
PUBLIC bool bGetJNReady(void);
PUBLIC void vWaitForJNReady(uint16 u16JNReadyTime);

#ifdef ENABLE_SERIAL_LINK_FILE_LOGGING
PUBLIC bool bSL_LoggerInit(void *p_arg);
PUBLIC bool bSL_LoggerWrite(uint8_t *buf, uint32_t len);
PUBLIC void vSL_LoggerFree(void);
#endif

#endif  /* SERIAL_LINK_CTRL_H_INCLUDED */
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
