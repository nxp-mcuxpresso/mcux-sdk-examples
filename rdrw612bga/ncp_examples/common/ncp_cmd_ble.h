/*!\file ncp_cmd_ble.h
 *\brief NCP Bluetooth LE command and structure definitions.
 */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 * Copyright (c) 2022 Codecoup
 * Copyright 2022-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __NCP_CMD_BLE_H__
#define __NCP_CMD_BLE_H__

#include "ncp_cmd_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/** NCP Bluetooth LE subclass type */
/** NCP Bluetooth LE subclass type for core command */
#define NCP_CMD_BLE_CORE         0x00000000
/** NCP Bluetooth LE subclass type for gap command */
#define NCP_CMD_BLE_GAP          0x00100000
/** NCP Bluetooth LE subclass type for gatt command */
#define NCP_CMD_BLE_GATT         0x00200000
/** NCP Bluetooth LE subclass type for l2cap command */
#define NCP_CMD_BLE_L2CAP        0x00300000
/** NCP Bluetooth LE subclass type for power management command */
#define NCP_CMD_BLE_POWERMGMT    0x00400000
/** NCP Bluetooth LE subclass type for vendor command */
#define NCP_CMD_BLE_VENDOR       0x00500000
/** NCP Bluetooth LE subclass type for other command */
#define NCP_CMD_BLE_OTHER        0x00600000
/** NCP Bluetooth LE subclass type for event */
#define NCP_CMD_BLE_EVENT        0x00f00000

/** The max size of the NCP Bluetooth LE max transmission unit */
#ifndef NCP_BLE_MTU
#define NCP_BLE_MTU                 1024
#endif

/** NCP Command/Response definitions */
/** Bluetooth LE invalid command ID */
#define NCP_CMD_BLE_INVALID_CMD         (NCP_CMD_BLE | NCP_CMD_BLE_CORE | NCP_MSG_TYPE_CMD | 0x0000000a) /* invalid command recieve */
/** Bluetooth LE Core support command response ID */
#define NCP_RSP_BLE_CORE_SUPPORT_CMD    (NCP_CMD_BLE | NCP_CMD_BLE_CORE | NCP_MSG_TYPE_RESP | 0x00000001)

/** Bluetooth LE GAP set data length command ID */
#define NCP_CMD_BLE_GAP_SET_DATA_LEN         (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_CMD | 0x00000020) /* Set data len */
/** Bluetooth LE GAP set data length command response ID */
#define NCP_RSP_BLE_GAP_SET_DATA_LEN         (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_RESP | 0x00000020)
/** Bluetooth LE GAP set phy command ID */
#define NCP_CMD_BLE_GAP_SET_PHY              (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_CMD | 0x0000001f) /* Set phy */
/** Bluetooth LE GAP set phy command response ID */
#define NCP_RSP_BLE_GAP_SET_PHY              (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_RESP | 0x0000001f)
/** Bluetooth LE GAP set advertising data command ID */
#define NCP_CMD_BLE_GAP_SET_ADV_DATA         (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_CMD | 0x0000001e) /* Set advertising data */
/** Bluetooth LE GAP set advertising data command response ID */
#define NCP_RSP_BLE_GAP_SET_ADV_DATA         (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_RESP | 0x0000001e)
/** Bluetooth LE GAP set scan parameter command ID */
#define NCP_CMD_BLE_GAP_SET_SCAN_PARAM       (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_CMD | 0x0000001d) /* Set scan parameter */
/** Bluetooth LE GAP set scan parameter command response ID */
#define NCP_RSP_BLE_GAP_SET_SCAN_PARAM       (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_RESP | 0x0000001d)
/** Bluetooth LE GAP start advertising command ID */
#define NCP_CMD_BLE_GAP_START_ADV            (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_CMD | 0x0000000a) /* Start advertising */
/** Bluetooth LE GAP start advertising command response ID */
#define NCP_RSP_BLE_GAP_START_ADV            (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_RESP | 0x0000000a)
/** Bluetooth LE GAP stop advertising command ID */
#define NCP_CMD_BLE_GAP_STOP_ADV             (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_CMD | 0x0000000b) /* Stop advertising */
/** Bluetooth LE GAP stop advertising command response ID */
#define NCP_RSP_BLE_GAP_STOP_ADV             (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_RESP | 0x0000000b)
/** Bluetooth LE GAP start scan command ID */
#define NCP_CMD_BLE_GAP_START_SCAN           (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_CMD | 0x0000000c) /* Start discovery */
/** Bluetooth LE GAP start scan command response ID */
#define NCP_RSP_BLE_GAP_START_SCAN           (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_RESP | 0x0000000c)
/** Bluetooth LE GAP stop scan command ID */
#define NCP_CMD_BLE_GAP_STOP_SCAN            (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_CMD | 0x0000000d) /* Stop discovery */
/** Bluetooth LE GAP start scan command response ID */
#define NCP_RSP_BLE_GAP_STOP_SCAN            (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_RESP | 0x0000000d)
/** Bluetooth LE GAP connect command ID */
#define NCP_CMD_BLE_GAP_CONNECT              (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_CMD | 0x0000000e) /* Create a connection */
/** Bluetooth LE GAP connect command response ID */
#define NCP_RSP_BLE_GAP_CONNECT              (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_RESP | 0x0000000e)
/** Bluetooth LE GAP disconnect command ID */
#define NCP_CMD_BLE_GAP_DISCONNECT           (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_CMD | 0x0000000f) /* Terminate a connection */
/** Bluetooth LE GAP connect command response ID */
#define NCP_RSP_BLE_GAP_DISCONNECT           (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_RESP | 0x0000000f)
/** Bluetooth LE GAP connection parameter update command ID */
#define NCP_CMD_BLE_GAP_CONN_PARAM_UPDATE    (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_CMD | 0x00000016) /* Connection parameters update */
/** Bluetooth LE GAP connection parameter update command response ID */
#define NCP_RSP_BLE_GAP_CONN_PARAM_UPDATE    (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_RESP | 0x00000016)
/** Bluetooth LE GAP set filter list command ID */
#define NCP_CMD_BLE_GAP_SET_FILTER_LIST      (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_CMD | 0x0000001c) /* Set filter accept list */
/** Bluetooth LE GAP set filter list command response ID */
#define NCP_RSP_BLE_GAP_SET_FILTER_LIST      (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_RESP | 0x0000001c)
/** Bluetooth LE GAP pair command ID */
#define NCP_CMD_BLE_GAP_PAIR                 (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_CMD | 0x00000011) /* Enable encryption with peer or start pair process */
/** Bluetooth LE GAP pair command response ID */
#define NCP_RSP_BLE_GAP_PAIR                 (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_RESP | 0x00000011)

/** Bluetooth LE gatt add host service attribute command ID */
#define NCP_CMD_BLE_HOST_SERVICE_ADD         (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | 0x00000002) /* Add Host Attribute to Device gatt datebase and start, ble-host-service-start */
/** Bluetooth LE gatt add host service attribute command response ID */
#define NCP_RSP_BLE_HOST_SERVICE_ADD         (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | 0x00000002)
/** Bluetooth LE gatt discovery primary service/characteristic/descriptor command ID */
#define NCP_CMD_BLE_HOST_SERVICE_DISC        (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | 0x00000003) /* Discover Primary Service/Characteristics/Descriptors */
/** Bluetooth LE gatt discovery primary service/characteristic/descriptor command response ID */
#define NCP_RSP_BLE_HOST_SERVICE_DISC        (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | 0x00000003)
/** Bluetooth LE gatt set characteristic/descriptor value command ID */
#define NCP_CMD_BLE_GATT_SET_VALUE           (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | 0x00000006) /* Set Characteristic/Descriptor Value */
/** Bluetooth LE gatt set characteristic/descriptor value command response ID */
#define NCP_RSP_BLE_GATT_SET_VALUE           (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | 0x00000006)
/** Bluetooth LE gatt start service command ID */
#define NCP_CMD_BLE_GATT_START_SERVICE       (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | 0x00000007) /* Start server with previously prepared attributes database. */
/** Bluetooth LE gatt start service command response ID */
#define NCP_RSP_BLE_GATT_START_SERVICE       (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | 0x00000007)
/** Bluetooth LE gatt discovery primary service command ID */
#define NCP_CMD_BLE_GATT_DISC_PRIM           (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | GATT_DISC_PRIM_UUID) /* Discover Primary Service */
/** Bluetooth LE gatt discovery primary service command response ID */
#define NCP_RSP_BLE_GATT_DISC_PRIM           (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | GATT_DISC_PRIM_UUID)
/** Bluetooth LE gatt discovery characteristic command ID */
#define NCP_CMD_BLE_GATT_DISC_CHRC           (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | GATT_DISC_CHRC_UUID) /* Discover Characteristics */
/** Bluetooth LE gatt discovery characteristic command response ID */
#define NCP_RSP_BLE_GATT_DISC_CHRC           (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | GATT_DISC_CHRC_UUID)
/** Bluetooth LE gatt read characteristic/descriptor command ID */
#define NCP_CMD_BLE_GATT_READ                (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | 0x00000011) /* Read Characteristic/Descriptor */
/** Bluetooth LE gatt read characteristic/descriptor command response ID */
#define NCP_RSP_BLE_GATT_READ                (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | 0x00000011)
/** Bluetooth LE gatt config service notify characteristic value command ID */
#define NCP_CMD_BLE_GATT_CFG_NOTIFY          (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | GATT_CFG_NOTIFY) /* Configure service to notify characteristic value to clinet */
/** Bluetooth LE gatt config service notify characteristic value command response ID */
#define NCP_RSP_BLE_GATT_CFG_NOTIFY          (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | GATT_CFG_NOTIFY)
/** Bluetooth LE gatt config service indicate characteristic value command ID */
#define NCP_CMD_BLE_GATT_CFG_INDICATE        (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | GATT_CFG_INDICATE) /* Configure service to indicate characteristic value to clinet */
/** Bluetooth LE gatt config service indicate characteristic value command response ID */
#define NCP_RSP_BLE_GATT_CFG_INDICATE        (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | GATT_CFG_INDICATE)
/** Bluetooth LE gatt write characteristic/descriptor command ID */
#define NCP_CMD_BLE_GATT_WRITE               (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | GATT_WRITE) /* Write Characteristic/Descriptor */
/** Bluetooth LE gatt write characteristic/descriptor command response ID */
#define NCP_RSP_BLE_GATT_WRITE               (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | GATT_WRITE)
/** Bluetooth LE gatt register service command ID */
#define NCP_CMD_BLE_GATT_REGISTER_SERVICE    (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | 0x00000020)  /* register a profile service */
/** Bluetooth LE gatt register service command response ID */
#define NCP_RSP_BLE_GATT_REGISTER_SERVICE    (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | 0x00000020)
/** Bluetooth LE gatt discovery descriptor command ID */
#define NCP_CMD_BLE_GATT_DESC_CHRC           (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | GATT_DISC_DESC_UUID) /* Discover Descriptors */
/** Bluetooth LE gatt discovery descriptor command response ID */
#define NCP_RSP_BLE_GATT_DESC_CHRC           (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | GATT_DISC_DESC_UUID)

/** Bluetooth LE l2cap connect command ID */
#define NCP_CMD_BLE_L2CAP_CONNECT            (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_CMD | 0x00000002) /* l2cap connect */
/** Bluetooth LE l2cap connect command response ID */
#define NCP_RSP_BLE_L2CAP_CONNECT            (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_RESP | 0x00000002)
/** Bluetooth LE l2cap disconnect command ID */
#define NCP_CMD_BLE_L2CAP_DISCONNECT         (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_CMD | 0x00000003) /* l2cap disconnect */
/** Bluetooth LE l2cap disconnect command response ID */
#define NCP_RSP_BLE_L2CAP_DISCONNECT         (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_RESP | 0x00000003)
/** Bluetooth LE l2cap send command ID */
#define NCP_CMD_BLE_L2CAP_SEND               (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_CMD | 0x00000004) /* l2cap send */
/** Bluetooth LE l2cap send command response ID */
#define NCP_RSP_BLE_L2CAP_SEND               (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_RESP | 0x00000004)
/** Bluetooth LE l2cap register command ID */
#define NCP_CMD_BLE_L2CAP_REGISTER           (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_CMD | 0x0000000a) /* l2cap register*/
/** Bluetooth LE l2cap register command response ID */
#define NCP_RSP_BLE_L2CAP_REGISTER           (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_RESP | 0x0000000a)
/** Bluetooth LE l2cap metrics command ID */
#define NCP_CMD_BLE_L2CAP_METRICS            (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_CMD | 0x0000000b) /* l2cap metrics */
/** Bluetooth LE l2cap metrics command response ID */
#define NCP_RSP_BLE_L2CAP_METRICS            (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_RESP | 0x0000000b)
/** Bluetooth LE l2cap receive command ID */
#define NCP_CMD_BLE_L2CAP_RECEIVE            (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_CMD | 0x0000000c) /* l2cap receive */
/** Bluetooth LE l2cap receive command response ID */
#define NCP_RSP_BLE_L2CAP_RECEIVE            (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_RESP | 0x0000000c)

/** Bluetooth LE Vendor enable/disable power mode command ID */
#define NCP_CMD_BLE_VENDOR_POWER_MODE        (NCP_CMD_BLE | NCP_CMD_BLE_VENDOR | NCP_MSG_TYPE_CMD | 0x00000001) /* Enable/Disable power save mode */
/** Bluetooth LE Vendor enable/disable power mode command response ID */
#define NCP_RSP_BLE_VENDOR_POWER_MODE        (NCP_CMD_BLE | NCP_CMD_BLE_VENDOR | NCP_MSG_TYPE_RESP | 0x00000001)
/** Bluetooth LE Vendor set uart baud rate command ID */
#define NCP_CMD_BLE_VENDOR_SET_UART_BR       (NCP_CMD_BLE | NCP_CMD_BLE_VENDOR | NCP_MSG_TYPE_CMD | 0x00000002) /* Set Uart baud rate */
/** Bluetooth LE Vendor set uart baud rate command response ID */
#define NCP_RSP_BLE_VENDOR_SET_UART_BR       (NCP_CMD_BLE | NCP_CMD_BLE_VENDOR | NCP_MSG_TYPE_RESP | 0x00000002)
/** Bluetooth LE Vendor set uart device address command ID */
#define NCP_CMD_BLE_VENDOR_SET_DEVICE_ADDR   (NCP_CMD_BLE | NCP_CMD_BLE_VENDOR | NCP_MSG_TYPE_CMD | 0x00000003) /* Set Uart LE device address */
/** Bluetooth LE Vendor set uart device address command response ID */
#define NCP_RSP_BLE_VENDOR_SET_DEVICE_ADDR   (NCP_CMD_BLE | NCP_CMD_BLE_VENDOR | NCP_MSG_TYPE_RESP | 0x00000003)
/** Bluetooth LE Vendor set device name command ID */
#define NCP_CMD_BLE_VENDOR_SET_DEVICE_NAME   (NCP_CMD_BLE | NCP_CMD_BLE_VENDOR | NCP_MSG_TYPE_CMD | 0x00000004) /* Set Uart LE device name */
/** Bluetooth LE Vendor set device name command response ID */
#define NCP_RSP_BLE_VENDOR_SET_DEVICE_NAME   (NCP_CMD_BLE | NCP_CMD_BLE_VENDOR | NCP_MSG_TYPE_RESP | 0x00000004)
/** Bluetooth LE Vendor config multi-advertising command ID */
#define NCP_CMD_BLE_VENDOR_CFG_MULTI_ADV     (NCP_CMD_BLE | NCP_CMD_BLE_VENDOR | NCP_MSG_TYPE_CMD | 0x00000005) /* Config Multi-advertising */
/** Bluetooth LE Vendor config multi-advertising command response ID */
#define NCP_RSP_BLE_VENDOR_CFG_MULTI_ADV     (NCP_CMD_BLE | NCP_CMD_BLE_VENDOR | NCP_MSG_TYPE_RESP | 0x00000005)

/** Bluetooth LE device ready event */
#define NCP_EVENT_IUT_READY                  (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | CORE_EV_IUT_READY) /* IUT Ready event */
/** Bluetooth LE advertising report event */
#define NCP_EVENT_ADV_REPORT                 (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | GAP_EV_DEVICE_FOUND) /* LE Advertising Report event */
/** Bluetooth LE connection complete event */
#define NCP_EVENT_DEVICE_CONNECTED           (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | GAP_EV_DEVICE_CONNECTED) /* Connection Complete event */
/** Bluetooth LE disconnection complete event */
#define NCP_EVENT_DEVICE_DISCONNECT          (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | GAP_EV_DEVICE_DISCONNECTED) /* Disconnection Complete event */
/** Bluetooth LE passkey display event */
#define NCP_EVENT_PASSKEY_DISPLAY            (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | GAP_EV_PASSKEY_DISPLAY) /* Passkey Display event */
/** Bluetooth LE remote identity address resolved event */
#define NCP_EVENT_IDENITY_RESOLVED           (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | GAP_EV_IDENTITY_RESOLVED) /* Remote Identity Address Resolved event */
/** Bluetooth LE connection paramter update event */
#define NCP_EVENT_CONN_PARAM_UPDATE          (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | GAP_EV_CONN_PARAM_UPDATE) /* Connection param update event */
/** Bluetooth LE security level changed event */
#define NCP_EVENT_SEC_LEVEL_CHANGED          (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | GAP_EV_SEC_LEVEL_CHANGED) /* Security Level Changed event */
/** Bluetooth LE paring failed event */
#define NCP_EVENT_PAIRING_FAILED             (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | GAP_EV_PAIRING_FAILED) /* GAP pairing failed event */
/** Bluetooth LE bond lost event */
#define NCP_EVENT_BOND_LOST                  (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | GAP_EV_BOND_LOST) /* GAP bond lost */
/** Bluetooth LE phy update event */
#define NCP_EVENT_PHY_UPDATED                (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | GAP_EV_PHY_UPDATED) /* GAP phy updated */
/** Bluetooth LE data length update event */
#define NCP_EVENT_DATA_LEN_UPDATED           (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | GAP_EV_DATA_LEN_UPDATED) /* GAP data len updated */
/** Bluetooth LE gatt notification received event */
#define NCP_EVENT_GATT_NOTIFICATION          (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | GATT_EV_NOTIFICATION | 0x200) /* gatt notification Receive event */
/** Bluetooth LE gatt attribute value changed event */
#define NCP_EVENT_ATTR_VALUE_CHANGED         (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | GATT_EV_ATTR_VALUE_CHANGED | 0x200) /* gatt Attribute Value Changed event */
/** Bluetooth LE gatt client characteristic configuration changed event */
#define NCP_EVENT_GATT_CCC_CFG_CHANGED       (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | GATT_EV_CCC_CFG_CHANGED | 0x200) /* gatt Client Characteristic Configuration Changed event */
/** Bluetooth LE gatt client subscription event */
#define NCP_EVENT_GATT_SUBSCRIPTIONED        (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | GATT_EV_SUBSCRIPTIONED | 0x200) /* gatt Client Subscription status event */
/** Bluetooth LE gatt discover primary service event */
#define NCP_EVENT_GATT_DISC_PRIM             (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | 0x19 | 0x200) /* Discover Primary Service event */
/** Bluetooth LE gatt discover characteristic event */
#define NCP_EVENT_GATT_DISC_CHRC             (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | 0x20 | 0x200) /* Discover Characteristics event */
/** Bluetooth LE gatt discover descriptor event */
#define NCP_EVENT_GATT_DISC_DESC             (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | 0x21 | 0x200) /* Discover Descriptors event */
/* NXP commission service info */
#define NCP_EVENT_GATT_NCS_INFO              (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | 0x30 | 0x200) /* NCS Commission info event */

/** Bluetooth LE l2cap connect event */
#define NCP_EVENT_L2CAP_CONNECT              (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | L2CAP_EV_CONNECT | 0x300) /* l2cap Connect event */
/** Bluetooth LE l2cap disconnect event */
#define NCP_EVENT_L2CAP_DISCONNECT           (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | L2CAP_EV_DISCONNECT | 0x300) /* l2cap Disconnect event */
/** Bluetooth LE l2cap receive event */
#define NCP_EVENT_L2CAP_RECEIVE              (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | L2CAP_EV_RECEIVE | 0x300) /* l2cap Receive event */

/** NCP Bluetooth LE TLV type */
#define NCP_CMD_GATT_ADD_SERVICE_TLV       0x0001
#define NCP_CMD_GATT_ADD_CHRC_TLV          0x0002
#define NCP_CMD_GATT_ADD_DESC_TLV          0x0003
#define NCP_CMD_GATT_START_SVC_TLV         0x0004
#define NCP_CMD_GATT_DISC_PRIM_TLV         0x0005
#define NCP_CMD_GATT_DISC_CHRC_TLV         0x0006
#define NCP_CMD_GATT_DISC_DESC_TLV         0x0007


/* NCP Bluetooth LE config */
#define NCP_BLE_DATA_MAX_SIZE   (NCP_BLE_MTU - 0x05U)
#define NCP_BLE_ADDR_LENGTH 6
#define NCP_BLE_DEVICE_NAME_MAX 32
#define MAX_MONIT_MAC_FILTER_NUM 3
#define MAX_SUPPORT_SERVICE 10
   
/** Size in octets of a 16-bit UUID */
#define BT_UUID_SIZE_16                  2
/** Size in octets of a 128-bit UUID */
#define BT_UUID_SIZE_128                 16

/** Bluetooth LE Vendor set board address hci opcode */
#define BT_HCI_VD_SET_BD_ADDRESS      BT_OP(BT_OGF_VS, 0x0022)

/** This structure is used to set the configuration of NCP Bluetooth LE board address */
typedef  NCP_TLV_PACK_START struct _ncp_ble_set_bd_address_cfg {
    /** Vendor command paramter id. \n
       The id of Bluetooth LE Vendor set board address is 0XFE 
    */
    uint8_t   paramater_id;
    /** Bluetooth LE Device address length  \n
        The length of Bluetooth LE Device address should be 0x06
    */
    uint8_t   bd_addr_len;
    /** Bluetooth LE Device address */
    uint8_t   bd_address[6];
} NCP_TLV_PACK_END ncp_ble_set_bd_address_cfg;

/** NCP Bluetooth LE low power mode opcode */
#define BT_HCI_VD_LOW_POWER_MODE      BT_OP(BT_OGF_VS, 0x0023)
/** This structure is used to set the configuration of CPU2 low power mode */
typedef  NCP_TLV_PACK_START struct _ncp_ble_low_power_mode_cfg {
    /**
     * Set CPU2 power mode. \n
      auto sleep disable : 0x02 \n
      auto sleep enable : 0x03
     */
    uint8_t   power_mode;
    /** sleep timeout value */
	uint16_t  timeout;
} NCP_TLV_PACK_END ncp_ble_low_power_mode_cfg;

/** NCP Bluetooth LE multi advertising opcode */
#define BT_HCI_VD_MULTI_ADV_CMD       BT_OP(BT_OGF_VS, 0x0154)

/** NCP Bluetooth LE Core reset board command ID */
#define CORE_RESET_BOARD                0x06

/* NCP Bluetooth LE Core board ready event ID */
#define CORE_EV_IUT_READY               0x80

/** NCP Bluetooth LE GAP read support command ID */
#define GAP_READ_SUPPORTED_COMMANDS 0x01

/** NCP Bluetooth LE GAP read controller index list command ID */
#define GAP_READ_CONTROLLER_INDEX_LIST  0x02

/** NCP Bluetooth LE GAP set power ID */
#define GAP_SETTINGS_POWERED            0
/** NCP Bluetooth LE GAP connect ID */
#define GAP_SETTINGS_CONNECTABLE        1
/** NCP Bluetooth LE GAP fast connect ID */
#define GAP_SETTINGS_FAST_CONNECTABLE   2
/** NCP Bluetooth LE GAP discover ID */
#define GAP_SETTINGS_DISCOVERABLE       3
/** NCP Bluetooth LE GAP bond ID */
#define GAP_SETTINGS_BONDABLE           4
/** NCP Bluetooth LE GAP set security ID */
#define GAP_SETTINGS_LINK_SEC_3         5
/** NCP Bluetooth LE GAP set SSP ID */
#define GAP_SETTINGS_SSP                6
/** NCP Bluetooth LE GAP set bredr ID */
#define GAP_SETTINGS_BREDR              7
/** NCP Bluetooth LE GAP set hs ID */
#define GAP_SETTINGS_HS                 8
/** NCP Bluetooth LE GAP set le ID */
#define GAP_SETTINGS_LE                 9
/** NCP Bluetooth LE GAP set advertising ID */
#define GAP_SETTINGS_ADVERTISING        10
/** NCP Bluetooth LE GAP set SC ID */
#define GAP_SETTINGS_SC                 11
/** NCP Bluetooth LE GAP set debug key ID */
#define GAP_SETTINGS_DEBUG_KEYS         12
/** NCP Bluetooth LE GAP set privacy ID */
#define GAP_SETTINGS_PRIVACY            13
/** NCP Bluetooth LE GAP set controller config ID */
#define GAP_SETTINGS_CONTROLLER_CONFIG  14
/** NCP Bluetooth LE GAP set static address ID */
#define GAP_SETTINGS_STATIC_ADDRESS     15

/** NCP Bluetooth LE GAP non discoverable ID */
#define GAP_NON_DISCOVERABLE      0x00
/** NCP Bluetooth LE GAP general discoverable ID */
#define GAP_GENERAL_DISCOVERABLE  0x01
/** NCP Bluetooth LE GAP limit discoverable ID */
#define GAP_LIMITED_DISCOVERABLE  0x02
/** NCP Bluetooth LE GAP discoverable ID */
#define GAP_SET_DISCOVERABLE    0x08

/** NCP Bluetooth LE GAP set bondable ID */
#define GAP_SET_BONDABLE    0x09

/** NCP Bluetooth LE GAP set advertising ID */
#define GAP_START_ADVERTISING 0x0a
/** This structure is used to set the configuration of NCP Bluetooth LE advertising command */
typedef NCP_TLV_PACK_START struct gap_start_advertising_cmd {
    /** advertising data length */
    uint8_t adv_data_len;
    /** scan response length */
    uint8_t scan_rsp_len;
    /** advertising/scan response data */
    uint8_t adv_sr_data[];
} NCP_TLV_PACK_END gap_start_advertising_cmd_t;
/** This structure is used to store the response of NCP Bluetooth LE set advertising command */
typedef NCP_TLV_PACK_START struct gap_start_advertising_rp {
    /** device current setting \n
     * bit0: GAP_SETTINGS_POWERED \n
     * bit1: GAP_SETTINGS_CONNECTABLE \n
     * bit2: GAP_SETTINGS_FAST_CONNECTABLE \n
     * bit3: GAP_SETTINGS_DISCOVERABLE \n
     * bit4: GAP_SETTINGS_BONDABLE \n
     * bit5: GAP_SETTINGS_LINK_SEC_3 \n
     * bit6: GAP_SETTINGS_SSP \n
     * bit7: GAP_SETTINGS_BREDR \n
     * bit8: GAP_SETTINGS_HS \n
     * bit9: GAP_SETTINGS_LE \n
     * bit10: GAP_SETTINGS_ADVERTISING \n
     * bit11: GAP_SETTINGS_SC \n
     * bit12: GAP_SETTINGS_DEBUG_KEYS \n
     * bit13: GAP_SETTINGS_PRIVACY \n
     * bit14: GAP_SETTINGS_CONTROLLER_CONFIG \n
     * bit15: GAP_SETTINGS_STATIC_ADDRESS
    */
    uint32_t current_settings;
} NCP_TLV_PACK_END gap_start_advertising_rp_t;

/** NCP Bluetooth LE GAP stop advertising ID */
#define GAP_STOP_ADVERTISING    0x0b
/** This structure is used to store the response of NCP Bluetooth LE stop advertising command */
typedef NCP_TLV_PACK_START struct gap_stop_advertising_rp {
    /** device current setting  \n
     * bit0: GAP_SETTINGS_POWERED \n
     * bit1: GAP_SETTINGS_CONNECTABLE \n
     * bit2: GAP_SETTINGS_FAST_CONNECTABLE \n
     * bit3: GAP_SETTINGS_DISCOVERABLE \n
     * bit4: GAP_SETTINGS_BONDABLE \n
     * bit5: GAP_SETTINGS_LINK_SEC_3 \n
     * bit6: GAP_SETTINGS_SSP \n
     * bit7: GAP_SETTINGS_BREDR \n
     * bit8: GAP_SETTINGS_HS \n
     * bit9: GAP_SETTINGS_LE \n
     * bit10: GAP_SETTINGS_ADVERTISING \n
     * bit11: GAP_SETTINGS_SC \n
     * bit12: GAP_SETTINGS_DEBUG_KEYS \n
     * bit13: GAP_SETTINGS_PRIVACY \n
     * bit14: GAP_SETTINGS_CONTROLLER_CONFIG \n
     * bit15: GAP_SETTINGS_STATIC_ADDRESS
    */
    uint32_t current_settings;
} NCP_TLV_PACK_END gap_stop_advertising_rp_t;

/** NCP Bluetooth LE GAP discovery le ID */
#define GAP_DISCOVERY_FLAG_LE                0x01
/** NCP Bluetooth LE GAP discovery bredr ID */
#define GAP_DISCOVERY_FLAG_BREDR             0x02
/** NCP Bluetooth LE GAP discovery limited ID */
#define GAP_DISCOVERY_FLAG_LIMITED           0x04
/** NCP Bluetooth LE GAP active scan ID */
#define GAP_DISCOVERY_FLAG_LE_ACTIVE_SCAN    0x08
/** NCP Bluetooth LE GAP observe ID */
#define GAP_DISCOVERY_FLAG_LE_OBSERVE        0x10
/** NCP Bluetooth LE GAP discovery own address ID */
#define GAP_DISCOVERY_FLAG_OWN_ID_ADDR       0x20

/** NCP Bluetooth LE GAP start discovery ID */
#define GAP_START_DISCOVERY   0x0c
/** This structure is used to set the configuration of NCP Bluetooth LE start discovery command */
typedef NCP_TLV_PACK_START struct gap_start_discovery_cmd {
    /** discovery setting \n 
      0x01: discovery le \n
      0x02: discovery bredr \n 
      0x04: limit discovery \n
      0x08: active scan \n
      0x10: observe \n
      0x20: discovery own address  
     */
    uint8_t flags;
} NCP_TLV_PACK_END gap_start_discovery_cmd_t;

/** NCP Bluetooth LE GAP stop discovery ID */
#define GAP_STOP_DISCOVERY    0x0d
/** NCP Bluetooth LE GAP connect ID */
#define GAP_CONNECT     0x0e
/** NCP Bluetooth LE GAP connect command structure */
typedef NCP_TLV_PACK_START struct gap_connect_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
} NCP_TLV_PACK_END gap_connect_cmd_t;

/** NCP Bluetooth LE GAP disconnect ID */
#define GAP_DISCONNECT      0x0f
/** This structure is used to set the configuration of NCP Bluetooth LE connect command */
typedef NCP_TLV_PACK_START struct gap_disconnect_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t  address_type;
    /** Bluetooth LE address */
    uint8_t  address[6];
} NCP_TLV_PACK_END gap_disconnect_cmd_t;

/** NCP Bluetooth LE GAP IO capablility display only ID */
#define GAP_IO_CAP_DISPLAY_ONLY     0
/** NCP Bluetooth LE GAP IO capablility display option ID */
#define GAP_IO_CAP_DISPLAY_YESNO    1
/** NCP Bluetooth LE GAP IO capablility keyboard only ID */
#define GAP_IO_CAP_KEYBOARD_ONLY    2
/** NCP Bluetooth LE GAP IO capablility no input ouput ID */
#define GAP_IO_CAP_NO_INPUT_OUTPUT  3
/** NCP Bluetooth LE GAP IO capablility keyboard&display ID */
#define GAP_IO_CAP_KEYBOARD_DISPLAY 4

/** NCP Bluetooth LE GAP IO capablility ID */
#define GAP_SET_IO_CAP      0x10

/** NCP Bluetooth LE GAP pair ID */
#define GAP_PAIR      0x11
/** NCP Bluetooth LE GAP pair command structure */
typedef NCP_TLV_PACK_START struct gap_pair_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
} NCP_TLV_PACK_END gap_pair_cmd_t;

/** NCP Bluetooth LE GAP unpair ID */
#define GAP_UNPAIR      0x12
/** This structure is used to set the configuration of NCP Bluetooth LE unpair command */
typedef NCP_TLV_PACK_START struct gap_unpair_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
} NCP_TLV_PACK_END gap_unpair_cmd_t;

/** NCP Bluetooth LE GAP passkey entry ID */
#define GAP_PASSKEY_ENTRY   0x13

/** NCP Bluetooth LE GAP passkey confirm ID */
#define GAP_PASSKEY_CONFIRM   0x14

/** NCP Bluetooth LE start directed advertising hd */
#define GAP_START_DIRECTED_ADV_HD BIT(0)
/** NCP Bluetooth LE start directed advertising own id */
#define GAP_START_DIRECTED_ADV_OWN_ID BIT(1)
/** NCP Bluetooth LE start directed advertising peer rpa */
#define GAP_START_DIRECTED_ADV_PEER_RPA BIT(2)

/** NCP Bluetooth LE start directed advertising ID */
#define GAP_START_DIRECTED_ADV    0x15

/** NCP Bluetooth LE GAP connection parameter update ID */
#define GAP_CONN_PARAM_UPDATE   0x16
/** This structure is used to set the configuration of NCP Bluetooth LE connection parameter update command */
typedef NCP_TLV_PACK_START struct gap_conn_param_update_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** connection minimal interval \n
        Minimum value for the connection interval. This shall be less than or equal to maximum connection interval
    */
    uint16_t interval_min;
    /** connection maximum interval \n
        Maximum value for the connection interval. This shall be greater than or equal to minimum connection interval
    */
    uint16_t interval_max;
    /** connection latency \n
        Maximum peripheral latency for the connection in number of subrated connection events
    */
    uint16_t latency;
    /** connection timeout \n
        Supervision timeout for the Bluetooth LE link
    */
    uint16_t timeout;
} NCP_TLV_PACK_END gap_conn_param_update_cmd_t;

/** NCP Bluetooth LE GAP pairing consent ID */
#define GAP_PAIRING_CONSENT   0x17

/** NCP Bluetooth LE oob legacy set data ID */
#define GAP_OOB_LEGACY_SET_DATA   0x18

/** NCP Bluetooth LE oob security get local data ID */
#define GAP_OOB_SC_GET_LOCAL_DATA 0x19

/** NCP Bluetooth LE oob security set remote data ID */
#define GAP_OOB_SC_SET_REMOTE_DATA  0x1a

/** NCP Bluetooth LE GAP set mitm ID */
#define GAP_SET_MITM      0x1b

/** This structure is used to store the configuration of NCP Bluetooth LE address type */
typedef struct le_addr {
    /** Types of Bluetooth LE device addresses: \n
       Public Address : 0x00 \n
       Random Address : 0x01 \n
       Public Identity Address : 0x02 \n
       Random Identity Address : 0x03
     */
    uint8_t type;
    /** Bluetooth device Address */
    uint8_t address[6];
} le_addr_t;

/** NCP Bluetooth LE GAP set fileter list ID */
#define GAP_SET_FILTER_LIST 0x1c
/** This structure is used to store the configuration of NCP Bluetooth LE set fileter list type */
typedef NCP_TLV_PACK_START struct gap_set_filter_list {
    /** The number of filter addresses to set */
    uint8_t cnt;
    /** list of filter addresses */
    le_addr_t addr;
} NCP_TLV_PACK_END gap_set_filter_list_t;

/** NCP Bluetooth LE GAP set scan parameter ID */
#define GAP_SET_SCAN_PARAMETER 0x1d
/** This structure is used to set the configuration of NCP Bluetooth LE set scan parameter command */
typedef NCP_TLV_PACK_START struct gap_set_scan_param_cmd {
  /** Bit-field of scanning options. */
  uint32_t options;
  /** Scan interval (N * 0.625 ms) */
  uint16_t interval;
  /** Scan window (N * 0.625 ms) */
  uint16_t window;
} NCP_TLV_PACK_END gap_set_scan_param_cmd_t;

/** NCP Bluetooth LE GAP set advertising data ID */
#define GAP_SET_ADV_DATA 0x1e
/** This structure is used to set the configuration of NCP Bluetooth LE set advertising data command */
typedef NCP_TLV_PACK_START struct gap_set_adv_data_cmd {
    /** advertising data length */
    uint8_t adv_data_len;
    /** Advertising data to set  \n
        advertising data should follow struture: \n
            struct bt_data {  \n
                U8 len; \n
                U8 type;  \n
                const U8 *data; \n
            } 
    */
    uint8_t data[31];
} NCP_TLV_PACK_END gap_set_adv_data_cmd_t;

/** This structure is used to set the configuration of NCP Bluetooth LE set advertising data length command */
typedef NCP_TLV_PACK_START struct gap_set_data_len_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t  address_type;
    /** Bluetooth LE address */
    uint8_t  address[6];
    /** option to use \ref tx_max_time or to use system calculate maximum TX transimit time \n
        0 - use system maximum TX transimit time \n
        1 - use customized maximum TX transimit time
    */
    uint8_t  time_flag;
    /** Maximum data length to transmit */
    uint16_t tx_max_len;
    /** Maximum TX transimit time */
    uint16_t tx_max_time;
} NCP_TLV_PACK_END gap_set_data_len_cmd_t;

/** This structure is used to set the configuration of NCP Bluetooth LE set phy command */
typedef NCP_TLV_PACK_START struct gap_set_phy_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t  address_type;
    /** Bluetooth LE address */
    uint8_t  address[6];
    /** Connection PHY options  \n
        0: BT_CONN_LE_PHY_OPT_NONE (Convenience value when no options are specified) \n
        bit0: BT_CONN_LE_PHY_OPT_CODED_S2 (LE Coded using S=2 coding preferred when transmitting) \n
        bit1: BT_CONN_LE_PHY_OPT_CODED_S8 (LE Coded using S=8 coding preferred when transmitting)
    */
    uint16_t options;
    /** Bitmask of preferred transmit PHYs  \n
        0: BT_GAP_LE_PHY_NONE (Convenience macro for when no PHY is set) \n
        bit0: BT_GAP_LE_PHY_1M (LE 1M PHY) \n
        bit1: BT_GAP_LE_PHY_2M (LE 2M PHY) \n
        bit2: BT_GAP_LE_PHY_CODED (LE Coded PHY)
    */
    uint8_t  pref_tx_phy;
    /** Bitmask of preferred receive PHYs \n
        0: BT_GAP_LE_PHY_NONE (Convenience macro for when no PHY is set) \n
        bit0: BT_GAP_LE_PHY_1M (LE 1M PHY) \n
        bit1: BT_GAP_LE_PHY_2M (LE 2M PHY) \n
        bit2: BT_GAP_LE_PHY_CODED (LE Coded PHY)
    */
    uint8_t  pref_rx_phy;
} NCP_TLV_PACK_END gap_set_phy_cmd_t;

/** NCP Bluetooth LE new setting event ID */
#define GAP_EV_NEW_SETTINGS   0x80

/** NCP Bluetooth LE device found flag */
#define GAP_DEVICE_FOUND_FLAG_RSSI  0x01
#define GAP_DEVICE_FOUND_FLAG_AD    0x02
#define GAP_DEVICE_FOUND_FLAG_SD    0x04

/** NCP Bluetooth LE device found event ID */
#define GAP_EV_DEVICE_FOUND   0x81
/** This structure is used to store the information of NCP Bluetooth LE device found event */
typedef NCP_TLV_PACK_START struct gap_device_found_ev {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t  address_type;
    /** Bluetooth LE address */
    uint8_t  address[6];
    /** scan rssi */
    int8_t   rssi;
    /** Bluetooth LE device found flag \n
        0x01: GAP_DEVICE_FOUND_FLAG_RSSI \n
        0x02: GAP_DEVICE_FOUND_FLAG_AD \n
        0x04: GAP_DEVICE_FOUND_FLAG_SD
     */
    uint8_t  flags;
    /** scan data length */
    uint16_t eir_data_len;
    /** scan data */
    uint8_t  eir_data[];
} NCP_TLV_PACK_END gap_device_found_ev_t;

/** NCP Bluetooth LE device connected event ID */
#define GAP_EV_DEVICE_CONNECTED   0x82
/** This structure is used to store the information of NCP Bluetooth LE device connected event */
typedef NCP_TLV_PACK_START struct gap_device_connected_ev {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** connection interval */
    uint16_t interval;
    /** connection latency \n
        Maximum peripheral latency for the connection in number of subrated connection events
    */
    uint16_t latency;
    /** connection timeout \n
        Supervision timeout for the Bluetooth LE link
    */
    uint16_t timeout;
} NCP_TLV_PACK_END gap_device_connected_ev_t;

/** NCP Bluetooth LE device disconnected event ID */
#define GAP_EV_DEVICE_DISCONNECTED  0x83
/** This structure is used to store the information of NCP Bluetooth LE disconnected event */
typedef NCP_TLV_PACK_START struct gap_device_disconnected_ev {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
} NCP_TLV_PACK_END gap_device_disconnected_ev_t;

/** NCP Bluetooth LE passkey display event ID */
#define GAP_EV_PASSKEY_DISPLAY    0x84
/** This structure is used to store the information of NCP Bluetooth LE passkey display event */
typedef NCP_TLV_PACK_START struct gap_passkey_display_ev {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t  address_type;
    /** Bluetooth LE address */
    uint8_t  address[6];
    /** device passkey number */
    uint32_t passkey;
} NCP_TLV_PACK_END gap_passkey_display_ev_t;

/** NCP Bluetooth LE passkey entry request event ID */
#define GAP_EV_PASSKEY_ENTRY_REQ  0x85

/** NCP Bluetooth LE passkey confirm request event ID */
#define GAP_EV_PASSKEY_CONFIRM_REQ  0x86

/** NCP Bluetooth LE identity resolved event ID */
#define GAP_EV_IDENTITY_RESOLVED  0x87
/** This structure is used to store the information of NCP Bluetooth LE identity resolved event */
typedef NCP_TLV_PACK_START struct gap_identity_resolved_ev {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** Bluetooth LE identity address type \n
     2: public identity address \n
     3: random identity address
    */
    uint8_t identity_address_type;
    /** Bluetooth LE identity address */
    uint8_t identity_address[6];
} NCP_TLV_PACK_END gap_identity_resolved_ev_t;

/** NCP Bluetooth LE connection parameter update event ID */
#define GAP_EV_CONN_PARAM_UPDATE  0x88
/** This structure is used to store the information of NCP Bluetooth LE connection parameter update event */
typedef NCP_TLV_PACK_START struct gap_conn_param_update_ev {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** connection interval */
    uint16_t interval;
    /** connection latency \n
        Maximum peripheral latency for the connection in number of subrated connection events
    */
    uint16_t latency;
    /** connection timeout \n
        Supervision timeout for the Bluetooth LE link
    */
    uint16_t timeout;
} NCP_TLV_PACK_END gap_conn_param_update_ev_t;

/** NCP Bluetooth LE GAP security level */
#define GAP_SEC_LEVEL_UNAUTH_ENC  0x01
#define GAP_SEC_LEVEL_AUTH_ENC    0x02
#define GAP_SEC_LEVEL_AUTH_SC     0x03

/** NCP Bluetooth LE security level changed event ID */
#define GAP_EV_SEC_LEVEL_CHANGED  0x89
/** This structure is used to store the information of NCP Bluetooth LE security level changed event */
typedef NCP_TLV_PACK_START struct gap_sec_level_changed_ev {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** connection security level \n
        0x01: No encryption and no authentication \n
        0x02: Encryption and no authentication (no MITM) \n
        0x03: Encryption and authentication (MITM)
    */
    uint8_t sec_level;
} NCP_TLV_PACK_END gap_sec_level_changed_ev_t;

/** NCP Bluetooth LE pairing consent request event ID */
#define GAP_EV_PAIRING_CONSENT_REQ  0x8a

/** NCP Bluetooth LE bonding lost event ID */
#define GAP_EV_BOND_LOST  0x8b
/** This structure is used to store the information of NCP Bluetooth LE bonding lost event */
typedef NCP_TLV_PACK_START struct gap_bond_lost_ev {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
} NCP_TLV_PACK_END gap_bond_lost_ev_t;

/** NCP Bluetooth LE pairing failed event ID */
#define GAP_EV_PAIRING_FAILED   0x8c
/** This structure is used to store the information of NCP Bluetooth LE pairing failed event */
typedef NCP_TLV_PACK_START struct gap_bond_pairing_failed_ev {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** pairing failed reason \n
      0: Security procedure successful \n
      1: Authentication failed \n
      2: PIN or encryption key is missing \n
      3: OOB data is not available \n
      4: The requested security level could not be reached \n
      5: Pairing is not supported \n
      6: Pairing is not allowed \n
      7: Invalid parameters \n
      8: Distributed Key Rejected \n
      9: Pairing failed but the exact reason could not be specified
    */
    uint8_t reason;
} NCP_TLV_PACK_END gap_bond_pairing_failed_ev_t;

/** NCP Bluetooth LE phy update event ID */
#define GAP_EV_PHY_UPDATED  0x91
/** This structure is used to store the information of NCP Bluetooth LE phy update event */
typedef NCP_TLV_PACK_START struct gap_phy_updated_ev {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** Bluetooth TX PHY \n
        1: 1M PHY \n
        2: 2M PHY \n
        4: Coded PHY 
    */
    uint8_t tx_phy;
    /** Bluetooth RX PHY \n
        1: 1M PHY \n
        2: 2M PHY \n
        4: Coded PHY 
    */
    uint8_t rx_phy;
} NCP_TLV_PACK_END gap_phy_updated_ev_t;

/** NCP Bluetooth LE data length update event ID */
#define GAP_EV_DATA_LEN_UPDATED  0x92
/** This structure is used to store the information of NCP Bluetooth LE data length update event */
typedef NCP_TLV_PACK_START struct gap_data_len_updated_ev {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** Maximum length of TX connection */
    uint16_t tx_max_len;
    /** Maximum time of TX connection */
    uint16_t tx_max_time;
    /** Maximum length of RX connection */
    uint16_t rx_max_len;
    /** Maximum time of RX connection */
    uint16_t rx_max_time;
} NCP_TLV_PACK_END gap_data_len_updated_ev_t;

/** NCP Bluetooth LE l2cap read support command ID */
#define L2CAP_READ_SUPPORTED_COMMANDS   0x01

/** NCP Bluetooth LE opt  */
#define L2CAP_CONNECT_OPT_ECFC          0x01
#define L2CAP_CONNECT_OPT_HOLD_CREDIT   0x02

/** NCP Bluetooth LE l2cap connect ID */
#define L2CAP_CONNECT   0x02
/** This structure is used to set the configuration of NCP Bluetooth LE l2cap connect command */
typedef NCP_TLV_PACK_START struct l2cap_connect_cmd_tag {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** protocol service multiplexo */
    uint16_t psm;
    /** connection security level \n
        0x01: No encryption and no authentication \n
        0x02: Encryption and no authentication (no MITM) \n
        0x03: Encryption and authentication (MITM)
    */
    uint8_t sec;
    /** Option to use custom security level or to use system default security level  \n
        0: to use system default security level \n
        1: to use customized security level
    */
    uint8_t sec_flag;
} NCP_TLV_PACK_END l2cap_connect_cmd_t;

/** This structure is used to store the response of NCP Bluetooth LE l2cap connect command */
typedef NCP_TLV_PACK_START struct l2cap_connect_rp_tag {
    /** receive packet number */
    uint8_t num;
    /** receive channel id */
    uint8_t chan_id[];
} NCP_TLV_PACK_END l2cap_connect_rp_t;

/** NCP Bluetooth LE l2cap disconnect ID */
#define L2CAP_DISCONNECT    0x03
/** This structure is used to set the configuration of NCP Bluetooth LE l2cap disconnect command */
typedef NCP_TLV_PACK_START struct l2cap_disconnect_cmd_tag {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
} NCP_TLV_PACK_END l2cap_disconnect_cmd_t;

/** NCP Bluetooth LE l2cap send data ID */
#define L2CAP_SEND_DATA     0x04
/** This structure is used to set the configuration of NCP Bluetooth LE l2cap send data command */
typedef NCP_TLV_PACK_START struct l2cap_send_data_cmd_tag {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** the number of times a loop is sent */
    uint16_t times;
} NCP_TLV_PACK_END l2cap_send_data_cmd_t;

/** NCP Bluetooth LE l2cap transport type */
#define L2CAP_TRANSPORT_BREDR                       0x00
#define L2CAP_TRANSPORT_LE                          0x01

/** NCP Bluetooth LE l2cap connection response type */
#define L2CAP_CONNECTION_RESPONSE_SUCCESS           0x00
#define L2CAP_CONNECTION_RESPONSE_INSUFF_AUTHEN     0x01
#define L2CAP_CONNECTION_RESPONSE_INSUFF_AUTHOR     0x02
#define L2CAP_CONNECTION_RESPONSE_INSUFF_ENC_KEY    0x03

/** NCP Bluetooth LE l2cap listen ID */
#define L2CAP_LISTEN    0x05

/** NCP Bluetooth LE l2cap accept connection ID */
#define L2CAP_ACCEPT_CONNECTION     0x06

/** NCP Bluetooth LE l2cap reconfigure ID */
#define L2CAP_RECONFIGURE   0x07

/** NCP Bluetooth LE l2cap credit ID */
#define L2CAP_CREDITS   0x08

/** NCP Bluetooth LE l2cap disconnect eatt (Enhanced ATT Bearers support) channel ID */
#define L2CAP_DISCONNECT_EATT_CHANS     0x09

/** NCP Bluetooth LE l2cap register PSM ID */
#define L2CAP_REGISTER_PSM     0x0a
/** This structure is used to set the configuration of NCP Bluetooth LE l2cap register PSM command */
typedef NCP_TLV_PACK_START struct l2cap_register_psm_cmd_tag {
    /** protocol service multiplexor */
    uint16_t psm;
    /** connection security level \n
        0x01: No encryption and no authentication \n
        0x02: Encryption and no authentication (no MITM) \n
        0x03: Encryption and authentication (MITM)
    */
    uint8_t sec_level;
    /** Option to use custom security level or to use system default security level \n
        0: use system default security level \n
        1: use customized security level
    */
    uint8_t sec_flag;
    /** allowlist */
    uint8_t policy;
    /** option to use allowlist \n
        0: do not use allowlist \n
        1: use allowlist
     */
    uint8_t policy_flag;
} NCP_TLV_PACK_END l2cap_register_psm_cmd_t;

/** NCP Bluetooth LE l2cap metrics ID */
#define L2CAP_METRICS    0x0b
/** This structure is used to set the configuration of NCP Bluetooth LE l2cap metrics command */
typedef NCP_TLV_PACK_START struct l2cap_metrics_cmd_tag {
    /** option to enable calculating metrics of l2cap rate \n
      0: disable calculating metrics of l2cap rate  \n
      1: enable calculating metrics of l2cap rate 
    */
    bool metrics_flag;
} NCP_TLV_PACK_END l2cap_metrics_cmd_t;

/** NCP Bluetooth LE l2cap receive ID */
#define L2CAP_RECV    0x0c
/** This structure is used to set the configuration of NCP Bluetooth LE l2cap receive command */
typedef NCP_TLV_PACK_START struct l2cap_recv_cmd_tag {
    /** receive delay time in millisecond*/
    uint32_t l2cap_recv_delay_ms;
} NCP_TLV_PACK_END l2cap_recv_cmd_t;

/** NCP Bluetooth LE connection request event ID */
#define L2CAP_EV_CONNECTION_REQ     0x80

/** NCP Bluetooth LE l2cap connect event ID */
#define L2CAP_EV_CONNECTED  0x81

/** NCP Bluetooth LE l2cap disconnect event ID */
#define L2CAP_EV_DISCONNECTED   0x82

/** NCP Bluetooth LE l2cap data receive event ID */
#define L2CAP_EV_DATA_RECEIVED  0x83

/** NCP Bluetooth LE l2cap reconfigure event ID */
#define L2CAP_EV_RECONFIGURED   0x84

/** gatt server context */
#define SERVER_MAX_SERVICES     10
#define SERVER_MAX_ATTRIBUTES   50
#define SERVER_MAX_UUID_LEN     16
#define SERVER_BUF_SIZE         2048

/** NCP Bluetooth LE gatt read support command ID */
#define GATT_READ_SUPPORTED_COMMANDS    0x01

/** NCP Bluetooth LE service type */
#define GATT_SERVICE_PRIMARY        0x00
#define GATT_SERVICE_SECONDARY      0x01

/** NCP Bluetooth LE gatt add service ID */
#define GATT_ADD_SERVICE        0x02
/** This structure is used to set the configuration of NCP Bluetooth LE add service command */
typedef NCP_TLV_PACK_START struct gatt_add_service_cmd {
    /** Header of the type tlv */
    TypeHeader_t header;
    /** Bluetooth LE service type \n
      0: primary service \n
      1: secondary service
     */
    uint8_t type;
    /** service uuid length  \n
     0x02: UUID type 16-bit \n
     0x10: UUID type 128-bit
    */
    uint8_t uuid_length;
    /** service uuid */
    uint8_t uuid[SERVER_MAX_UUID_LEN];
} NCP_TLV_PACK_END gatt_add_service_cmd_t;
/** This structure is used to store the response of NCP Bluetooth LE add service command */
typedef NCP_TLV_PACK_START struct gatt_add_service_rp {
    /** attribute handle of added new service */
    uint16_t svc_id;
} NCP_TLV_PACK_END gatt_add_service_rp_t;

/** NCP Bluetooth LE gatt add characteristic ID */
#define GATT_ADD_CHARACTERISTIC     0x03
/** This structure is used to set the configuration of NCP Bluetooth LE add characteristic command */
typedef NCP_TLV_PACK_START struct gatt_add_characteristic_cmd {
    /** Header of the type tlv */
    TypeHeader_t header;
    /** service id */
    uint16_t svc_id;
    /** characteristic properties */
    uint8_t properties;
    /** characteristic permission */
    uint16_t permissions;
    /** characteristic length */
    uint8_t uuid_length;
    /** characteristic uuid */
    uint8_t uuid[SERVER_MAX_UUID_LEN];
} NCP_TLV_PACK_END gatt_add_characteristic_cmd_t;
/** NCP Bluetooth LThis structure is used to store the response of NCP Bluetooth LE add characteristic command */
typedef NCP_TLV_PACK_START struct gatt_add_characteristic_rp {
    /** characteristic ID */
    uint16_t char_id;
} NCP_TLV_PACK_END gatt_add_characteristic_rp_t;

/** NCP Bluetooth LE gatt add descriptor ID */
#define GATT_ADD_DESCRIPTOR     0x04
/** This structure is used to set the configuration of NCP Bluetooth LE add descriptor command */
typedef NCP_TLV_PACK_START struct gatt_add_descriptor_cmd {
    /** Header of the type tlv */
    TypeHeader_t header;
    /** characteristic id */
    uint16_t char_id;
    /** descriptor permission */
    uint16_t permissions;
    /** descriptor length */
    uint8_t uuid_length;
    /** descriptor uuid */
    uint8_t uuid[SERVER_MAX_UUID_LEN];
} NCP_TLV_PACK_END gatt_add_descriptor_cmd_t;
/** NCP This structure is used to store the response of NCP Bluetooth LE add descriptor command */
typedef NCP_TLV_PACK_START struct gatt_add_descriptor_rp {
    /** descriptor id */
    uint16_t desc_id;
} NCP_TLV_PACK_END gatt_add_descriptor_rp_t;

/** NCP Bluetooth LE gatt add include service ID */
#define GATT_ADD_INCLUDED_SERVICE   0x05
/** This structure is used to set the configuration of NCP Bluetooth LE add include service command */
typedef NCP_TLV_PACK_START struct gatt_add_included_service_cmd {
    /** Header of the type tlv */
    TypeHeader_t header;
    /** service uuid */
    uint16_t svc_id;
} NCP_TLV_PACK_END gatt_add_included_service_cmd_t;
/** This structure is used to store the response of NCP Bluetooth LE add include service command */
typedef NCP_TLV_PACK_START struct gatt_add_included_service_rp {
    /** include service ID */
    uint16_t included_service_id;
} NCP_TLV_PACK_END gatt_add_included_service_rp_t;

/** This structure is used to set the configuration of NCP Bluetooth LE start service command */
typedef NCP_TLV_PACK_START struct gatt_start_service_cmd {
    /** Header of the type tlv */
    TypeHeader_t header;
    /** service started status \n
     * 0: unstart \n
     * 1: start
     */
    uint8_t started;
} NCP_TLV_PACK_END gatt_start_service_cmd_t;

/** This structure is used to set the configuration of NCP Bluetooth LE add service command */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SERVICE_ADD
{
    /** tlv buf length */
    uint32_t tlv_buf_len;
    /**
     * add service TLV, gatt_add_service_cmd_t \n 
     * add characteristic TLV, gatt_add_characteristic_cmd_t \n
     * add descriptor TLV, gatt_add_descriptor_cmd_t \n
     * add include service TLV, gatt_add_included_service_cmd_t (to be added in the future) \n
     * start host servuce TLV, gatt_start_service_cmd_t 
     */
    uint8_t tlv_buf[1];
} NCP_TLV_PACK_END NCP_CMD_SERVICE_ADD;

/** This structure is used to set the configuration of NCP Bluetooth LE start service command */
typedef NCP_TLV_PACK_START struct _NCP_CMD_START_SERVICE
{
    /** host status */
    uint8_t form_host;
    /** service ID \n
     * 4: Central_HTC \n
     * 5: Central_HRC
    */
    uint8_t svc_id;
} NCP_TLV_PACK_END NCP_CMD_START_SERVICE;

/** NCP Bluetooth LE gatt set value ID */
#define GATT_SET_VALUE          0x06
/** This structure is used to set the configuration of NCP Bluetooth LE set value command */
typedef NCP_TLV_PACK_START struct gatt_set_value_cmd {
    // uint16_t attr_id;
    /** characteristic uuid length */
    uint8_t uuid_length;
    /** characteristic uuid */
    uint8_t uuid[SERVER_MAX_UUID_LEN];
    /** value length */
    uint16_t len;
    /** characteristic value */
    uint8_t value[512];
} NCP_TLV_PACK_END gatt_set_value_cmd_t;

/** NCP Bluetooth LE gatt start server ID */
#define GATT_START_SERVER       0x07
/** This structure is used to store the response of NCP Bluetooth LE start server command */
typedef NCP_TLV_PACK_START struct gatt_start_server_rp {
    /** database attribute offset */
    uint16_t db_attr_off;
    /** database attribute count */
    uint8_t db_attr_cnt;
} NCP_TLV_PACK_END gatt_start_server_rp_t;

/** NCP Bluetooth LE gatt reset server ID */
#define GATT_RESET_SERVER       0x08
/** NCP Bluetooth LE gatt set encryption key size ID */
#define GATT_SET_ENC_KEY_SIZE       0x09
/** This structure is used to set the configuration of NCP Bluetooth LE set encryption key size command */
typedef NCP_TLV_PACK_START struct gatt_set_enc_key_size_cmd {
    /** attribute id */
    uint16_t attr_id;
    /** encryption key size */
    uint8_t key_size;
} NCP_TLV_PACK_END gatt_set_enc_key_size_cmd_t;

/** This structure is used to store the information of NCP Bluetooth LE service data */
typedef NCP_TLV_PACK_START struct gatt_service {
    /** service start uuid */
    uint16_t start_handle;
    /** service end uuid */
    uint16_t end_handle;
    /** uuid length */
    uint8_t uuid_length;
    /** uuid data */
    uint8_t uuid[SERVER_MAX_UUID_LEN];
} NCP_TLV_PACK_END gatt_service_t;

/** This structure is used to store the information of NCP Bluetooth LE service data */
typedef NCP_TLV_PACK_START struct gatt_included {
    /** service include uuid */
    uint16_t included_handle;
    /** service handle */
    struct gatt_service service;
} NCP_TLV_PACK_END gatt_included_t;

/** This structure is used to store the information of NCP Bluetooth LE characteristic data */
typedef NCP_TLV_PACK_START struct gatt_characteristic {
    /** characteristic handle */
    uint16_t characteristic_handle;
    /** value handle */
    uint16_t value_handle;
    /** characteristic properties */
    uint8_t properties;
    /** characteristic uuid length */
    uint8_t uuid_length;
    /** characteristic uuids */
    uint8_t uuid[SERVER_MAX_UUID_LEN];
} NCP_TLV_PACK_END gatt_characteristic_t;

/** This structure is used to store the information of NCP Bluetooth LE descriptor data */
typedef NCP_TLV_PACK_START struct gatt_descriptor {
    /** descriptor handle */
    uint16_t descriptor_handle;
    /** descriptor uuid length */
    uint8_t uuid_length;
    /** descriptor uuid */
    uint8_t uuid[SERVER_MAX_UUID_LEN];
} NCP_TLV_PACK_END gatt_descriptor_t;

/** NCP Bluetooth LE exchange maximum transimission unit ID */
#define GATT_EXCHANGE_MTU       0x0a
/** This structure is used to set the configuration of NCP Bluetooth LE exchange maximum transimission unit command */
typedef NCP_TLV_PACK_START struct gatt_exchange_mtu_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
} NCP_TLV_PACK_END gatt_exchange_mtu_cmd_t;

/** NCP Bluetooth LE discovery primary service ID */
#define GATT_DISC_ALL_PRIM      0x0b
/** This structure is used to set the configuration of NCP Bluetooth LE discovery primary service command */
typedef NCP_TLV_PACK_START struct gatt_disc_all_prim_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
} NCP_TLV_PACK_END gatt_disc_all_prim_cmd_t;
/** This structure is used to store the response of NCP Bluetooth LE discovery primary service command */
typedef NCP_TLV_PACK_START struct gatt_disc_all_prim_rp {
    /** discovered service number */
    uint8_t services_count;
    /** discovered service */
    gatt_service_t services[];
} NCP_TLV_PACK_END gatt_disc_all_prim_rp_t;

/** NCP Bluetooth LE discovery primary uuid ID */
#define GATT_DISC_PRIM_UUID     0x0c
/** This structure is used to set the configuration of NCP Bluetooth LE discovery primary uuid command */
typedef NCP_TLV_PACK_START struct gatt_disc_prim_uuid_cmd {
    /** Header of the type tlv */
    TypeHeader_t header;
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** uuid length */
    uint8_t uuid_length;
    /** uuid data list */
    uint8_t uuid[];
} NCP_TLV_PACK_END gatt_disc_prim_uuid_cmd_t;
/** This structure is used to store the response of NCP Bluetooth LE discovery primary uuid command */
typedef NCP_TLV_PACK_START struct gatt_disc_prim_rp {
    /** The number of discovered services */
    uint8_t services_count;
    /** discovered services */
    gatt_service_t services[];
} NCP_TLV_PACK_END gatt_disc_prim_rp_t;

/** NCP Bluetooth LE gatt find included ID */
#define GATT_FIND_INCLUDED      0x0d
/** This structure is used to set the configuration of NCP Bluetooth LE find included service command */
typedef NCP_TLV_PACK_START struct gatt_find_included_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** service start handle */
    uint16_t start_handle;
    /** service end handle */
    uint16_t end_handle;
} NCP_TLV_PACK_END gatt_find_included_cmd_t;
/** This structure is used to store the response of NCP Bluetooth LE find included service command */
typedef NCP_TLV_PACK_START struct gatt_find_included_rp {
    /** The number of discovered services */
    uint8_t services_count;
    /** discovered included service */
    gatt_included_t included[];
} NCP_TLV_PACK_END gatt_find_included_rp_t;

/** NCP Bluetooth LE gatt discovery characteristic ID */
#define GATT_DISC_ALL_CHRC      0x0e
/** This structure is used to set the configuration of NCP Bluetooth LE discovery characteristic command */
typedef NCP_TLV_PACK_START struct gatt_disc_all_chrc_cmd {
    /** Header of the type tlv */
    TypeHeader_t header;
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** characteristic start handle */
    uint16_t start_handle;
    /** characteristic end handle */
    uint16_t end_handle;
} NCP_TLV_PACK_END gatt_disc_all_chrc_cmd_t;
/** This structure is used to store the response of NCP Bluetooth LE discovery characteristic command */
typedef NCP_TLV_PACK_START struct gatt_disc_chrc_rp {
    /** discovered characteristic number */
    uint8_t characteristics_count;
    /** discovered characteristic */
    gatt_characteristic_t characteristics[];
} NCP_TLV_PACK_END gatt_disc_chrc_rp_t;

/** NCP Bluetooth LE gatt discovery characteristic uuid ID */
#define GATT_DISC_CHRC_UUID     0x0f
/** This structure is used to set the configuration of NCP Bluetooth LE discovery characteristic uuid command */
typedef NCP_TLV_PACK_START struct gatt_disc_chrc_uuid_cmd {
    /** Header of the type tlv */
    TypeHeader_t header;
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** characteristic start handle */
    uint16_t start_handle;
    /** characteristic end handle */
    uint16_t end_handle;
    /** characteristic uuid length */
    uint8_t uuid_length;
    /** characteristic uuid */
    uint8_t uuid[];
} NCP_TLV_PACK_END gatt_disc_chrc_uuid_cmd_t;

/** NCP Bluetooth LE gatt discovery descriptor ID */
#define GATT_DISC_ALL_DESC      0x10
/** This structure is used to set the configuration of NCP Bluetooth LE discovery descriptor command */
typedef NCP_TLV_PACK_START struct gatt_disc_all_desc_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** descriptor start handle */
    uint16_t start_handle;
    /** descriptor end handle */
    uint16_t end_handle;
} NCP_TLV_PACK_END gatt_disc_all_desc_cmd_t;
/** NCP Bluetooth LE gatt discovery descriptor response structure */
typedef NCP_TLV_PACK_START struct gatt_disc_all_desc_rp {
    /** The number of discovered descriptors */
    uint8_t descriptors_count;
    /** discovered descriptors */
    gatt_descriptor_t descriptors[];
} NCP_TLV_PACK_END gatt_disc_all_desc_rp_t;

/** NCP Bluetooth LE gatt discovery descriptor uuid ID */
#define GATT_DISC_DESC_UUID     0x21
/** This structure is used to set the configuration of NCP Bluetooth LE discovery descriptor uuid command */
typedef NCP_TLV_PACK_START struct gatt_disc_desc_uuid_cmd {
    /** Header of the type tlv */
    TypeHeader_t header;
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** descriptor start handle */
    uint16_t start_handle;
    /** descriptor end handle */
    uint16_t end_handle;
    /** descriptor uuid length */
    uint8_t uuid_length;
    /** descriptor uuid */
    uint8_t uuid[];
} NCP_TLV_PACK_END gatt_disc_desc_uuid_cmd_t;

/** This structure is used to set the configuration of NCP Bluetooth LE service discovery data */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SERVICE_DISC
{
    /** tlv buf length */
    uint32_t tlv_buf_len;
    /**
     * discover primary service TLV, gatt_disc_prim_uuid_cmd_t \n
     * discover characteristic TLV, gatt_disc_chrc_uuid_cmd_t \n
     * discover descriptor TLV, gatt_disc_desc_uuid_cmd_t
     */
    uint8_t tlv_buf[1];
} NCP_TLV_PACK_END NCP_CMD_SERVICE_DISC;

/** NCP Bluetooth LE gatt read ID */
#define GATT_READ           0x11
/** This structure is used to set the configuration of NCP Bluetooth LE read attribute command */
typedef NCP_TLV_PACK_START struct gatt_read_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** handle uuid */
    uint16_t handle;
} NCP_TLV_PACK_END gatt_read_cmd_t;
/** This structure is used to store the response of NCP Bluetooth LE read attribute command */
typedef NCP_TLV_PACK_START struct gatt_read_rp {
    /** attribute reponse */
    uint8_t att_response;
    /** data length */
    uint16_t data_length;
    /** attribute data */
    uint8_t data[];
} NCP_TLV_PACK_END gatt_read_rp_t;

/** NCP Bluetooth LE gatt read uuid ID */
#define GATT_READ_UUID          0x12
/** This structure is used to set the configuration of NCP Bluetooth LE read uuid command */
typedef NCP_TLV_PACK_START struct gatt_read_uuid_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** uuid start handle */
    uint16_t start_handle;
    /** uuid end handle */
    uint16_t end_handle;
    /** uuid length */
    uint8_t uuid_length;
    /** uuid data */
    uint8_t uuid[];
} NCP_TLV_PACK_END gatt_read_uuid_cmd_t;
/** This structure is used to store the information of NCP Bluetooth LE characteristic value */
typedef NCP_TLV_PACK_START struct gatt_char_value {
    /** characteristic handle */
    uint16_t handle;
    /** characteristic data length */
    uint8_t data_len;
    /** characteristic data */
    uint8_t data[SERVER_MAX_UUID_LEN];
} NCP_TLV_PACK_END gatt_char_value_t;
/** This structure is used to store the response of NCP Bluetooth LE read uuid command */
typedef NCP_TLV_PACK_START struct gatt_read_uuid_rp {
    /** attribute response */
    uint8_t att_response;
    /** attribute value number */
    uint8_t values_count;
    /** characteristic value */
    gatt_char_value_t values[SERVER_MAX_ATTRIBUTES];
} NCP_TLV_PACK_END gatt_read_uuid_rp_t;

/** NCP Bluetooth LE gatt read long ID */
#define GATT_READ_LONG          0x13
/** This structure is used to set the configuration of NCP Bluetooth LE read long attribute command structure */
typedef NCP_TLV_PACK_START struct gatt_read_long_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** handle uuid */
    uint16_t handle;
    /** handle uuid offset */
    uint16_t offset;
} NCP_TLV_PACK_END gatt_read_long_cmd_t;
/** This structure is used to store the response of NCP Bluetooth LE read long attribute command */
typedef NCP_TLV_PACK_START struct gatt_read_long_rp {
    /** attribute response */
    uint8_t att_response;
    /** data length */
    uint16_t data_length;
    /** attribute data */
    uint8_t data[];
} NCP_TLV_PACK_END gatt_read_long_rp_t;

/** NCP Bluetooth LE gatt read multiple ID */
#define GATT_READ_MULTIPLE      0x14
/** This structure is used to set the configuration of NCP Bluetooth LE read multiple attribute command */
typedef NCP_TLV_PACK_START struct gatt_read_multiple_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** handle uuid number */
    uint8_t handles_count;
    /** handle uuid list */
    uint16_t handles[];
} NCP_TLV_PACK_END gatt_read_multiple_cmd_t;
/** This structure is used to store the response of NCP Bluetooth LE read multiple atrribute command */
typedef NCP_TLV_PACK_START struct gatt_read_multiple_rp {
    /** attribute response */
    uint8_t att_response;
    /** attribute data length */
    uint16_t data_length;
    /** attribute data */
    uint8_t data[];
} NCP_TLV_PACK_END gatt_read_multiple_rp_t;

/** NCP Bluetooth LE gatt write without response ID */
#define GATT_WRITE_WITHOUT_RSP      0x15
/** This structure is used to set the configuration of NCP Bluetooth LE write without response command */
typedef NCP_TLV_PACK_START struct gatt_write_without_rsp_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** handle uuid */
    uint16_t handle;
    /** attribute data length */
    uint16_t data_length;
    /** attribute data */
    uint8_t data[];
} NCP_TLV_PACK_END gatt_write_without_rsp_cmd_t;

/** NCP Bluetooth LE gatt signed write without response ID */
#define GATT_SIGNED_WRITE_WITHOUT_RSP   0x16
/** This structure is used to set the configuration of NCP Bluetooth LE signed write without response command */
typedef NCP_TLV_PACK_START struct gatt_signed_write_without_rsp_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** handle uuid */
    uint16_t handle;
    /** attribute data length */
    uint16_t data_length;
    /** attribute data */
    uint8_t data[];
} NCP_TLV_PACK_END gatt_signed_write_without_rsp_cmd_t;

/** NCP Bluetooth LE gatt write ID */
#define GATT_WRITE          0x17
/** This structure is used to set the configuration of NCP Bluetooth LE write attribute command */
typedef NCP_TLV_PACK_START struct gatt_write_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** handle uuid */
    uint16_t handle;
    /** attribute data length */
    uint16_t data_length;
    /** attribute data */
    uint8_t data[];
} NCP_TLV_PACK_END gatt_write_cmd_t;
/** This structure is used to store the response of NCP Bluetooth LE write attribute data command */
typedef NCP_TLV_PACK_START struct gatt_write_rp {
    /** attribute response */
    uint8_t att_response;
} NCP_TLV_PACK_END gatt_write_rp_t;

/** NCP Bluetooth LE gatt write long ID */
#define GATT_WRITE_LONG         0x18
/** This structure is used to set the configuration of NCP Bluetooth LE write long attribute data command */
typedef NCP_TLV_PACK_START struct gatt_write_long_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** handle uuid */
    uint16_t handle;
    /** handle offset */
    uint16_t offset;
    /** attribute data length */
    uint16_t data_length;
    /** attribute data */
    uint8_t data[];
} NCP_TLV_PACK_END gatt_write_long_cmd_t;
/** This structure is used to store the response of NCP Bluetooth LE write long attribute data command */
typedef NCP_TLV_PACK_START struct gatt_write_long_rp {
    /** attribute response */
    uint8_t att_response;
} NCP_TLV_PACK_END gatt_write_long_rp_t;

/** NCP Bluetooth LE gatt write reliable ID */
#define GATT_RELIABLE_WRITE     0x19
/** This structure is used to set the configuration of NCP Bluetooth LE write reliable attribute data command */
typedef NCP_TLV_PACK_START struct gatt_reliable_write_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** handle uuid */
    uint16_t handle;
    /** handle offset */
    uint16_t offset;
    /** attribute data length */
    uint16_t data_length;
    /** attribute data */
    uint8_t data[];
} NCP_TLV_PACK_END gatt_reliable_write_cmd_t;
/** This structure is used to store the response of NCP Bluetooth LE write reliable attribute data command */
typedef NCP_TLV_PACK_START struct gatt_reliable_write_rp {
    /** attribute response */
    uint8_t att_response;
} NCP_TLV_PACK_END gatt_reliable_write_rp_t;

/** NCP Bluetooth LE gatt config */
#define GATT_CFG_NOTIFY         0x1a
#define GATT_CFG_INDICATE       0x1b

/** This structure is used to set the configuration of NCP Bluetooth LE notify command */
typedef NCP_TLV_PACK_START struct gatt_cfg_notify_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** enable characteristic notification \n
     * 0: disable \n
     * 1: enable
     */
    uint8_t enable;
    /** chracteristic configure change handle */
    uint16_t ccc_handle;
} NCP_TLV_PACK_END gatt_cfg_notify_cmd_t;

/** NCP Bluetooth LE gatt get attribute ID */
#define GATT_GET_ATTRIBUTES     0x1c
/** This structure is used to set the configuration of NCP Bluetooth LE get attribute command */
typedef NCP_TLV_PACK_START struct gatt_get_attributes_cmd {
    /** attribute start handle */
    uint16_t start_handle;
    /** attribute end handle */
    uint16_t end_handle;
    /** attribute type length */
    uint8_t type_length;
    /** attribute type */
    uint8_t type[];
} NCP_TLV_PACK_END gatt_get_attributes_cmd_t;
/** This structure is used to store the response of NCP Bluetooth LE get attribute command */
typedef NCP_TLV_PACK_START struct gatt_get_attributes_rp {
    /** attribute count number */
    uint8_t attrs_count;
    /** attribute list */
    uint8_t attrs[];
} NCP_TLV_PACK_END gatt_get_attributes_rp_t;
/** This structure is used to store the information of NCP Bluetooth LE attribute value */
typedef NCP_TLV_PACK_START struct gatt_attr {
    /** attribute handle */
    uint16_t handle;
    /** attribute permission */
    uint8_t permission;
    /** attribute type length */
    uint8_t type_length;
    /** attribute type list */
    uint8_t type[];
} NCP_TLV_PACK_END gatt_attr_t;

/** NCP Bluetooth LE gatt get attribute value ID */
#define GATT_GET_ATTRIBUTE_VALUE    0x1d
/** This structure is used to set the configuration of NCP Bluetooth LE get attribute value command */
typedef NCP_TLV_PACK_START struct gatt_get_attribute_value_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** attribute handle */
    uint16_t handle;
} NCP_TLV_PACK_END gatt_get_attribute_value_cmd_t;
/** This structure is used to store the response of NCP Bluetooth LE get attribute value command */
typedef NCP_TLV_PACK_START struct gatt_get_attribute_value_rp {
    /** attribute response */
    uint8_t att_response;
    /** attribute value length */
    uint16_t value_length;
    /** attribute value */
    uint8_t value[];
} NCP_TLV_PACK_END gatt_get_attribute_value_rp_t;
/** NCP Bluetooth LE gatt change DB ID */
#define GATT_CHANGE_DB          0x1e
/** This structure is used to set the configuration of NCP Bluetooth LE change DB command */
typedef NCP_TLV_PACK_START struct gatt_change_db_cmd {
    /** start handle uuid */
    uint16_t start_handle;
    /** visibility setting */
    uint8_t visibility;
} NCP_TLV_PACK_END gatt_change_db_cmd_t;

/** NCP Bluetooth LE gatt eatt connect ID */
#define GATT_EATT_CONNECT		0x1f
/**This structure is used to set the configuration of NCP Bluetooth LE eatt connect command */
struct gatt_eatt_connect_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
	uint8_t address_type;
    /** Bluetooth LE address */
	uint8_t address[6];
    /** channel number */
	uint8_t num_channels;
} NCP_TLV_PACK_END NCP_TLV_PACK_START;

/** NCP Bluetooth LE gatt read multiple variable ID */
#define GATT_READ_MULTIPLE_VAR      0x20

/** NCP Bluetooth LE gatt notify multiple ID */
#define GATT_NOTIFY_MULTIPLE        0x21
/** This structure is used to set the configuration of NCP Bluetooth LE notify multiple service command */
typedef NCP_TLV_PACK_START struct gatt_cfg_notify_mult_cmd {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** attribute number */
    uint16_t cnt;
    /** attribute id list */
    uint16_t attr_id[];
} NCP_TLV_PACK_END gatt_cfg_notify_mult_cmd_t;

/** NCP Bluetooth LE add service ID */
#define GATT_NCP_BLE_ADD_SERVICE    0x22

/** This structure is used to set the configuration of NCP Bluetooth LE add service command */
typedef NCP_TLV_PACK_START struct gatt_ncp_ble_add_service_cmd
{
    /** service length */
    uint8_t svc_length;
    /** service add list */
    uint8_t svc[];
} NCP_TLV_PACK_END gatt_ncp_ble_add_service_cmd_t;

/** NCP Bluetooth LE gatt notification event */
#define GATT_EV_NOTIFICATION        0x80
/** This structure is used to store the information of NCP Bluetooth LE notification event */
typedef NCP_TLV_PACK_START struct gatt_notification_ev {
    /** service id */
    uint8_t svc_id;
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** attribute type */
    uint8_t type;
    /** attribute handle uuid */
    uint16_t handle;
    /** attribute data length */
    uint16_t data_length;
    /** attribute data */
    uint8_t data[];
} NCP_TLV_PACK_END gatt_notification_ev_t;

/** NCP Bluetooth LE gatt attribute value change event */
#define GATT_EV_ATTR_VALUE_CHANGED  0x81
/** This structure is used to store the information of NCP Bluetooth LE attribute value change event */
typedef NCP_TLV_PACK_START struct gatt_attr_value_changed_ev {
    /** attribute handle */
    uint16_t handle;
    /** attribute data length */
    uint16_t data_length;
    /** attribute data */
    uint8_t data[];
} NCP_TLV_PACK_END gatt_attr_value_changed_ev_t;

/** NCP Bluetooth LE gatt client characteristic configuration declaration event ID */
#define GATT_EV_CCC_CFG_CHANGED  0x82
/** This structure is used to store the information of NCP Bluetooth LE client characteristic configuration declaration event */
typedef NCP_TLV_PACK_START struct gatt_ccc_cfg_changed_ev {
    /** ccc value */
    uint16_t ccc_value;
    /** uuid length */
    uint8_t uuid_length;
    /** uuid value */
    uint8_t uuid[SERVER_MAX_UUID_LEN];
} NCP_TLV_PACK_END gatt_ccc_cfg_changed_ev_t;

/** NCP Bluetooth LE gatt subscription event ID */
#define GATT_EV_SUBSCRIPTIONED  0x85
/** This structure is used to store the information of NCP Bluetooth LE subscription event */
typedef NCP_TLV_PACK_START struct gatt_ncp_ble_svc_subscription_ev {
    /** service id */
    uint8_t svc_id;
    /** subscription status */
    uint8_t status;
} NCP_TLV_PACK_END gatt_ncp_ble_svc_subscription_ev_t;

/** This structure is used to store the response of NCP Bluetooth LE add service command */
typedef NCP_TLV_PACK_START struct gatt_ncp_ble_add_service_rp {
    /** service length */
    uint8_t svc_length;
    /** service status */
    uint8_t status[];
} NCP_TLV_PACK_END gatt_ncp_ble_add_service_rp_t;

/** NCP Bluetooth LE l2cap connect event ID */
#define L2CAP_EV_CONNECT        0x81
/** This structure is used to store the information of NCP Bluetooth LE l2cap connect event */
typedef NCP_TLV_PACK_START struct l2cap_connect_ev {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** PSM value */
    uint16_t psm;
} NCP_TLV_PACK_END l2cap_connect_ev_t;

/** NCP Bluetooth LE l2cap disconnect event ID */
#define L2CAP_EV_DISCONNECT        0x82
/** This structure is used to store the information of NCP Bluetooth LE l2cap disconnect event */
typedef NCP_TLV_PACK_START struct l2cap_disconnect_ev {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** PSM value */
    uint16_t psm;
} NCP_TLV_PACK_END l2cap_disconnect_ev_t;

/** NCP Bluetooth LE l2cap receive event ID */
#define L2CAP_EV_RECEIVE        0x83
/** This structure is used to store the information of NCP Bluetooth LE l2cap receive event */
typedef NCP_TLV_PACK_START struct l2cap_reveive_ev {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** PSM value */
    uint16_t psm;
    /** receive data length */
    uint8_t len;
    /** receive data value */
    uint8_t data[256];
} NCP_TLV_PACK_END l2cap_reveive_ev_t;

/** This structure is used to set the configuration of NCP Bluetooth LE start advertising command  */
typedef NCP_TLV_PACK_START struct _NCP_CMD_ADV_START
{
    /** advertising start value */
    uint8_t data[256];
} NCP_TLV_PACK_END NCP_CMD_ADV_START;

/** This structure is used to set the configuration of NCP Bluetooth LE set advertising data command */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SET_ADV_DATA
{
    /** advertising data length */
    uint8_t adv_length;
    /** advertising data value */
    uint8_t adv_data[];
} NCP_TLV_PACK_END NCP_CMD_SET_ADV_DATA;

/** This structure is used to set the configuration of NCP Bluetooth LE set scan parameter command */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SET_SCAN_PARAM
{
    /** bit-field of scanning options. */
    uint32_t options;
    /** scan interval (N * 0.625 ms) */
    uint16_t interval;
    /** scan window (N * 0.625 ms) */
    uint16_t window;
} NCP_TLV_PACK_END NCP_CMD_SET_SCAN_PARAM;

/** This structure is used to set the configuration of NCP Bluetooth LE start scanning command */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SCAN_START
{
    /** scan start */
    uint8_t type;
} NCP_TLV_PACK_END NCP_CMD_SCAN_START;

/** This structure is used to set the configuration of NCP Bluetooth LE create connection command */
typedef NCP_TLV_PACK_START struct _NCP_CMD_CONNECT
{
    /** remote address type */
    uint8_t type;
    /** remote address*/
    uint8_t addr[6];
} NCP_TLV_PACK_END NCP_CMD_CONNECT;

/** This structure is used to set the configuration of NCP Bluetooth LE set data length command */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SET_DATA_LEN
{
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t  address_type;
    /** remote address */
    uint8_t  address[6];
    /** time flag */
    uint8_t  time_flag;
    /** max transmit data length */
    uint16_t tx_max_len;
    /** max transmit time */
    uint16_t tx_max_time;
} NCP_TLV_PACK_END NCP_CMD_SET_DATA_LEN;

/** This structure is used to store the information of NCP Bluetooth LE set Bluetooth PHY command  */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SET_PHY
{
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t  address_type;
    /** Bluetooth LE address */
    uint8_t  address[6];
    /** Connection PHY options \n
        0: BT_CONN_LE_PHY_OPT_NONE (Convenience value when no options are specified) \n
        bit0: BT_CONN_LE_PHY_OPT_CODED_S2 (LE Coded using S=2 coding preferred when transmitting) \n
        bit1: BT_CONN_LE_PHY_OPT_CODED_S8 (LE Coded using S=8 coding preferred when transmitting)
    */
    uint16_t options;
    /** Bitmask of preferred transmit PHYs \n
        0: BT_GAP_LE_PHY_NONE (Convenience macro for when no PHY is set) \n
        bit0: BT_GAP_LE_PHY_1M (LE 1M PHY) \n
        bit1: BT_GAP_LE_PHY_2M (LE 2M PHY) \n
        bit2: BT_GAP_LE_PHY_CODED (LE Coded PHY)
    */
    uint8_t  pref_tx_phy;
    /** Bitmask of preferred receive PHYs \n
        0: BT_GAP_LE_PHY_NONE (Convenience macro for when no PHY is set) \n
        bit0: BT_GAP_LE_PHY_1M (LE 1M PHY) \n
        bit1: BT_GAP_LE_PHY_2M (LE 2M PHY) \n
        bit2: BT_GAP_LE_PHY_CODED (LE Coded PHY)
    */
    uint8_t  pref_rx_phy;
} NCP_TLV_PACK_END NCP_CMD_SET_PHY;

/** This structure is used to set the configuration of NCP Bluetooth LE update connection parameter command */
typedef NCP_TLV_PACK_START struct _NCP_CMD_CONN_PARA_UPDATE
{
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t type;
    /** Bluetooth LE address */
    uint8_t addr[6];
    /** connection minimal interval \n
        Minimum value for the connection interval. This shall be less than or equal to maximum connection interval
    */
    uint16_t interval_min;
    /** connection maximum interval \n
        Maximum value for the connection interval. This shall be greater than or equal to minimum connection interval
    */
    uint16_t interval_max;
    /** connection latency \n
        Maximum peripheral latency for the connection in number of subrated connection events
    */
    uint16_t latency;
    /** connection timeout \n
        Supervision timeout for the Bluetooth LE link
    */
    uint16_t timeout;
} NCP_TLV_PACK_END NCP_CMD_CONN_PARA_UPDATE;

/** This structure is used to store the information of NCP Bluetooth LE update connection parameter event */
typedef NCP_TLV_PACK_START struct _NCP_CMD_CONN_PARA_UPDATE_EV
{
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t type;
    /** Bluetooth LE address */
    uint8_t addr[6];
    /** connection interval */
    uint16_t interval;
    /** connection latency \n
        Maximum peripheral latency for the connection in number of subrated connection events
    */
    uint16_t latency;
    /** connection timeout \n
        Supervision timeout for the Bluetooth LE link
    */
    uint16_t timeout;
} NCP_TLV_PACK_END NCP_CMD_CONN_PARA_UPDATE_EV;

/** This structure is used to store the information of NCP Bluetooth LE set Bluetooth PHY event */
typedef NCP_TLV_PACK_START struct _NCP_CMD_PHY_UPDATE_EV
{
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** TX physical \n
        1: 1M PHY \n
        2: 2M PHY \n
        4: Coded PHY 
    */
    uint8_t tx_phy;
    /** RX physical \n
        1: 1M PHY \n
        2: 2M PHY \n
        4: Coded PHY 
    */
    uint8_t rx_phy;
} NCP_TLV_PACK_END NCP_CMD_PHY_UPDATE_EV;

/** This structure is used to store the information of NCP Bluetooth LE set data packet length event */
typedef NCP_TLV_PACK_START struct _NCP_CMD_DATA_LEN_UPDATE_EV
{
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** Maximum length of TX connection */
    uint16_t tx_max_len;
    /** Maximum time of TX connection */
    uint16_t tx_max_time;
    /** Maximum length of RX connection */
    uint16_t rx_max_len;
    /** Maximum time of RX connection */
    uint16_t rx_max_time;
} NCP_TLV_PACK_END NCP_CMD_DATA_LEN_UPDATE_EV;

/** This structure is used to set the configuration of NCP Bluetooth LE encryption command */
typedef NCP_TLV_PACK_START struct _NCP_CMD_ENCRYPTION
{
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t type;
    /** Bluetooth LE address */
    uint8_t addr[6];
} NCP_TLV_PACK_END NCP_CMD_ENCRYPTION;

/** This structure is used to set the configuration of NCP Bluetooth LE set address command */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SET_ADDR
{
    /** Bluetooth LE address */
    uint8_t addr[6];
} NCP_TLV_PACK_END NCP_CMD_SET_ADDR;

/** This structure is used to set the configuration of NCP Bluetooth LE set device name command */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SET_NAME
{
    /** device name */
    uint8_t name[33];
} NCP_TLV_PACK_END NCP_CMD_SET_NAME;

/** This structure is used to set the configuration of NCP Bluetooth LE set power mode command */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SET_POWER_MODE
{
    /**
     * Set CPU2 power mode. \n
      auto sleep disable : 0x02 \n
      auto sleep enable : 0x03
     */
    uint8_t mode;
} NCP_TLV_PACK_END NCP_CMD_SET_POWER_MODE;

/** This structure is used to set the configuration of NCP Bluetooth LE set attribute value command */
typedef NCP_TLV_PACK_START struct _NCP_SET_VALUE_CMD {
    /** uuid length */
    uint8_t uuid_length;
    /** uuid value */
    uint8_t uuid[SERVER_MAX_UUID_LEN];
    /** data length */
    uint16_t len;
    /** data value */
    uint8_t value[512];
} NCP_TLV_PACK_END NCP_SET_VALUE_CMD;

/** This structure is used to set the configuration of NCP Bluetooth LE read attribute value command */
typedef NCP_TLV_PACK_START struct _NCP_GATT_READ_CMD {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t type;
    /** Bluetooth LE address */
    uint8_t addr[6];
    /** Attribute handle to read with Characteristic Value */
    uint16_t handle;
} NCP_TLV_PACK_END NCP_GATT_READ_CMD;

/** This structure is used to set the configuration of NCP Bluetooth LE discover primary service uuid command */
typedef NCP_TLV_PACK_START struct _NCP_DISC_PRIM_UUID_CMD {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[NCP_BLE_ADDR_LENGTH];
    /** uuid length */
    uint8_t uuid_length;
    /** uuid value */
    uint8_t uuid[SERVER_MAX_UUID_LEN];
} NCP_TLV_PACK_END NCP_DISC_PRIM_UUID_CMD;

/** This structure is used to set the configuration of NCP Bluetooth LE discover characteristic uuid command */
typedef NCP_TLV_PACK_START struct _NCP_DISC_CHRC_UUID_CMD {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[NCP_BLE_ADDR_LENGTH];
    /** attribute start handle */
    uint16_t start_handle;
    /** attribute end handle */
    uint16_t end_handle;
    /** uuid length */
    uint8_t uuid_length;
    /** uuid value */
    uint8_t uuid[SERVER_MAX_UUID_LEN];
} NCP_TLV_PACK_END NCP_DISC_CHRC_UUID_CMD;

/** This structure is used to set the configuration of NCP Bluetooth LE subscript command */
typedef NCP_TLV_PACK_START struct _NCP_CFG_SUBCRIBE_CMD {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[NCP_BLE_ADDR_LENGTH];
    /** subscript value */
    uint8_t enable;
    /** ccc handle */
    uint16_t ccc_handle;
} NCP_TLV_PACK_END NCP_CFG_SUBCRIBE_CMD;

/** This structure is used to set the configuration of NCP Bluetooth LE register service command */
typedef NCP_TLV_PACK_START struct _NCP_REGISTER_SERVICE
{
    /** service length */
    uint8_t svc_length;
    /** service id */
    uint8_t service[MAX_SUPPORT_SERVICE];
} NCP_TLV_PACK_END NCP_REGISTER_SERVICE;

/** This structure is used to set the configuration of NCP Bluetooth LE l2cap connect command */
typedef NCP_TLV_PACK_START struct _NCP_L2CAP_CONNECT_CMD {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[NCP_BLE_ADDR_LENGTH];
    /** PSM value */
    uint16_t psm;
    /** connection security level \n
        0x01: No encryption and no authentication \n
        0x02: Encryption and no authentication (no MITM) \n
        0x03: Encryption and authentication (MITM)
    */
    uint8_t sec;
    /** Option to use customized security level or to use system default security level  \n
        0: to use system default security level \n
        1: to use customized security level
    */
    uint8_t sec_flag;
} NCP_TLV_PACK_END NCP_L2CAP_CONNECT_CMD;

/** This structure is used to set the configuration of NCP Bluetooth LE l2cap disconnect command */
typedef NCP_TLV_PACK_START struct _NCP_L2CAP_DISCONNECT_CMD {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[NCP_BLE_ADDR_LENGTH];
} NCP_TLV_PACK_END NCP_L2CAP_DISCONNECT_CMD;

/** This structure is used to set the configuration of NCP Bluetooth LE l2cap send data command */
typedef NCP_TLV_PACK_START struct _NCP_L2CAP_SEND_CMD {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[NCP_BLE_ADDR_LENGTH];
    /** the number of times a loop is sent */
    uint16_t times;
} NCP_TLV_PACK_END NCP_L2CAP_SEND_CMD;

/** This structure is used to set the configuration of NCP Bluetooth LE l2cap register PSM command */
typedef NCP_TLV_PACK_START struct _NCP_L2CAP_REGISTER_CMD {
    /** PSM value */
    uint16_t psm;
    /** connection security level \n
        0x01: No encryption and no authentication \n
        0x02: Encryption and no authentication (no MITM) \n
        0x03: Encryption and authentication (MITM)
    */
    uint8_t sec_level;
    /** Option to use custom security level or to use system default security level \n
        0: to use system default security level \n
        1: to use customized security level
    */
    uint8_t sec_flag;
    /** allowlist */
    uint8_t policy;
    /** option to use allowlist \n
        0: not use allowlist \n
        1: use allowlist
     */
    uint8_t policy_flag;
} NCP_TLV_PACK_END NCP_L2CAP_REGISTER_CMD;

/** This structure is used to set the configuration of NCP Bluetooth LE l2cap set metrics command */
typedef NCP_TLV_PACK_START struct _NCP_L2CAP_METRICS_CMD {
    /** option to enable calculating metrics of l2cap rate  \n
      0: disable calculating metrics of l2cap rate  \n
      1: enable calculating metrics of l2cap rate 
    */
    bool metrics_flag;
} NCP_TLV_PACK_END NCP_L2CAP_METRICS_CMD;

/** This structure is used to set the configuration of NCP Bluetooth LE l2cap receive data command */
typedef NCP_TLV_PACK_START struct _NCP_L2CAP_RECEIVE_CMD {
    /** receive delay time in millisecond*/
    uint32_t l2cap_recv_delay_ms;
} NCP_TLV_PACK_END NCP_L2CAP_RECEIVE_CMD;

/** This structure is used to store the response of NCP Bluetooth LE start advertising command */
typedef NCP_TLV_PACK_START struct _NCP_START_ADV_RP {
    /** device current setting \n
     * bit0: GAP_SETTINGS_POWERED \n
     * bit1: GAP_SETTINGS_CONNECTABLE \n
     * bit2: GAP_SETTINGS_FAST_CONNECTABLE \n
     * bit3: GAP_SETTINGS_DISCOVERABLE \n
     * bit4: GAP_SETTINGS_BONDABLE \n
     * bit5: GAP_SETTINGS_LINK_SEC_3 \n
     * bit6: GAP_SETTINGS_SSP \n
     * bit7: GAP_SETTINGS_BREDR \n
     * bit8: GAP_SETTINGS_HS \n
     * bit9: GAP_SETTINGS_LE \n
     * bit10: GAP_SETTINGS_ADVERTISING \n
     * bit11: GAP_SETTINGS_SC \n
     * bit12: GAP_SETTINGS_DEBUG_KEYS \n
     * bit13: GAP_SETTINGS_PRIVACY \n
     * bit14: GAP_SETTINGS_CONTROLLER_CONFIG \n
     * bit15: GAP_SETTINGS_STATIC_ADDRESS
    */
    uint32_t adv_setting;
} NCP_TLV_PACK_END NCP_START_ADV_RP;

/** This structure is used to store the response of NCP Bluetooth LE add service command */
typedef NCP_TLV_PACK_START struct _NCP_ADD_SERVICE_RP {
    /** service attribute handle */
    uint16_t attr_handle;
} NCP_TLV_PACK_END NCP_ADD_SERVICE_RP;

/** This structure is used to store the information of NCP Bluetooth LE service data */
typedef NCP_TLV_PACK_START struct GATT_SERVICE {
    /** service start handle */
    uint16_t start_handle;
    /** service end handle */
    uint16_t end_handle;
    /** service uuid length */
    uint8_t uuid_length;
    /** service uuid value */
    uint8_t uuid[SERVER_MAX_UUID_LEN];
} NCP_TLV_PACK_END GATT_SERVICE_T;

/** This structure is used to store the information of NCP Bluetooth LE characteristic data */
typedef NCP_TLV_PACK_START struct GATT_CHARACTERISTIC {
    /** characteristic handle */
    uint16_t characteristic_handle;
    /** characteristic value handle */
    uint16_t value_handle;
    /** characteristic properties handle */
    uint8_t properties;
    /** characteristic uuid length */
    uint8_t uuid_length;
    /** characteristic uuid value */
    uint8_t uuid[SERVER_MAX_UUID_LEN];
} NCP_TLV_PACK_END GATT_CHARACTERISTIC_T;

/** This structure is used to store the information of NCP Bluetooth LE descriptor data */
typedef NCP_TLV_PACK_START struct GATT_DESCRIPTOR {
    /** descriptor handle */
    uint16_t descriptor_handle;
    /** descriptor uuid length */
    uint8_t uuid_length;
    /** descriptor uuid value */
    uint8_t uuid[SERVER_MAX_UUID_LEN];
} NCP_TLV_PACK_END GATT_DESCRIPTOR_T;

/** This structure is used to store the response of NCP Bluetooth LE discover primary service command */
typedef NCP_TLV_PACK_START struct _NCP_DISC_PRIM_RP {
    /** service count */
    uint8_t services_count;
    /** service infomation */
    GATT_SERVICE_T services[MAX_SUPPORT_SERVICE];
} NCP_TLV_PACK_END NCP_DISC_PRIM_RP, NCP_DISC_PRIM_EV;

/** This structure is used to store the response of NCP Bluetooth LE discover characteristic command */
typedef NCP_TLV_PACK_START struct  _NCP_DISC_CHRC_RP {
    /** characteristic count */
    uint8_t characteristics_count;
    /** characteristic infomation */
    GATT_CHARACTERISTIC_T characteristics[MAX_SUPPORT_SERVICE];
} NCP_TLV_PACK_END NCP_DISC_CHRC_RP, NCP_DISC_CHRC_EV;

/** This structure is used to store the response of NCP Bluetooth LE discover descriptors command */
typedef NCP_TLV_PACK_START struct  _NCP_DISC_ALL_DESC_RP {
    /** descriptors count */
    uint8_t descriptors_count;
    /** descriptors infomation */
    GATT_DESCRIPTOR_T descriptors[MAX_SUPPORT_SERVICE];
} NCP_TLV_PACK_END NCP_DISC_ALL_DESC_RP, NCP_DISC_ALL_DESC_EV;

/** This structure is used to store the response of NCP Bluetooth LE commission service command */
typedef NCP_TLV_PACK_START struct  _NCP_NCS_INFO_RP {
    /** ssid value */
    char    ssid[32];
    /** ssid length */
    uint8_t ssid_len;
    /** password value */
    char    pswd[65];
    /** password length */
    uint8_t pswd_len;
    /** security value */
    char    secu[10];
    /** security length */
    uint8_t secu_len;
} NCP_TLV_PACK_END NCP_NCS_INFO_RP, NCP_NCS_INFO_EV;

/** This structure is used to store the information of NCP Bluetooth LE device found event */
typedef NCP_TLV_PACK_START struct _NCP_DEVICE_ADV_REPORT_EV {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t  address_type;
    /** Bluetooth LE address */
    uint8_t  address[6];
    /** advertising packet RSSI */
    int8_t   rssi;
    /** Bluetooth LE device found flag \n
        0x01: GAP_DEVICE_FOUND_FLAG_RSSI \n
        0x02: GAP_DEVICE_FOUND_FLAG_AD \n
        0x04: GAP_DEVICE_FOUND_FLAG_SD
     */
    uint8_t  flags;
    /** EIR data length */
    uint16_t eir_data_len;
    /** EIR data value */
    uint8_t  eir_data[];
} NCP_TLV_PACK_END NCP_DEVICE_ADV_REPORT_EV;

/** This structure is used to store the information of NCP Bluetooth LE device connected event */
typedef NCP_TLV_PACK_START struct _NCP_DEVICE_CONNECTED_EV {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** connection interval */
    uint16_t interval;
    /** connection latency \n
        Maximum peripheral latency for the connection in number of subrated connection events
    */
    uint16_t latency;
    /** connection timeout \n
        Supervision timeout for the Bluetooth LE link
    */
    uint16_t timeout;
} NCP_TLV_PACK_END NCP_DEVICE_CONNECTED_EV;

/** This structure is used to store the information of NCP Bluetooth LE device disconnected event */
typedef NCP_TLV_PACK_START struct _NCP_DEVICE_DISCONNECTED_EV {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
} NCP_TLV_PACK_END NCP_DEVICE_DISCONNECTED_EV;

/** This structure is used to store the information of NCP Bluetooth LE passkey display event */
typedef NCP_TLV_PACK_START struct _NCP_PASSKEY_DISPLAY_EV {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t  address_type;
    /** Bluetooth LE address */
    uint8_t  address[6];
    /** device passkey number */
    uint32_t passkey;
} NCP_TLV_PACK_END NCP_PASSKEY_DISPLAY_EV;

/** This structure is used to store the information of NCP Bluetooth LE identity resolved event */
typedef NCP_TLV_PACK_START struct _NCP_IDENTITY_RESOLVED_EV {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** Bluetooth LE identity address type \n
     2: public identity address \n
     3: random identity address
    */
    uint8_t identity_address_type;
    /** Bluetooth LE identity address */
    uint8_t identity_address[6];
} NCP_TLV_PACK_END NCP_IDENTITY_RESOLVED_EV;

/** This structure is used to store the information of NCP Bluetooth LE security level changed event */
typedef NCP_TLV_PACK_START struct _NCP_SEC_LEVEL_CHANGED_EV {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** connection security level \n
        0x01: No encryption and no authentication \n
        0x02: Encryption and no authentication (no MITM) \n
        0x03: Encryption and authentication (MITM)
    */
    uint8_t sec_level;
} NCP_TLV_PACK_END NCP_SEC_LEVEL_CHANGED_EV;

/** The maximum length of the attribute to notify */
#define MAX_ATTRIBUTE_VALUE_LEN 256

/** This structure is used to store the information of NCP Bluetooth LE notification event */
typedef NCP_TLV_PACK_START struct  _NCP_NOTIFICATION_EV {
    /** service id */
    uint8_t svc_id;
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** notification type */
    uint8_t type;
    /** notification handle */
    uint16_t handle;
    /** notification data length */
    uint16_t data_length;
    /** notification data value */
    uint8_t data[MAX_ATTRIBUTE_VALUE_LEN];
} NCP_TLV_PACK_END NCP_NOTIFICATION_EV;

/** This structure is used to store the information of NCP Bluetooth LE attribute value changed event */
typedef NCP_TLV_PACK_START struct  _NCP_ATTR_VALUE_CHANGED_EV {
    /** attribute handle */
    uint16_t handle;
    /** attribute data length */
    uint16_t data_length;
    /** attribute data value */
    uint8_t data[MAX_ATTRIBUTE_VALUE_LEN];
} NCP_TLV_PACK_END NCP_ATTR_VALUE_CHANGED_EV;

/** This structure is used to store the information of NCP Bluetooth LE client characteristic configuration changed event */
typedef NCP_TLV_PACK_START struct  _NCP_CCC_CFG_CHANGED_EV {
    /** client characteristic configuration changed value */
    uint16_t ccc_value;
    /** uuid length */
    uint8_t uuid_length;
    /** uuid value */
    uint8_t uuid[SERVER_MAX_UUID_LEN];
} NCP_TLV_PACK_END NCP_CCC_CFG_CHANGED_EV;

/** This structure is used to store the information of NCP Bluetooth LE subscription event */
typedef NCP_TLV_PACK_START struct  _NCP_SUBSCRIPTIONED_EV {
    /** service id */
    uint8_t svc_id;
    /** subscription status */
    uint8_t status;
} NCP_TLV_PACK_END NCP_SUBSCRIPTIONED_EV;

/** This structure is used to store the information of NCP Bluetooth LE l2cap connection event */
typedef NCP_TLV_PACK_START struct  _NCP_L2CAP_CONNECT_EV {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** PSM value */
    uint16_t psm;
} NCP_TLV_PACK_END NCP_L2CAP_CONNECT_EV;

/** This structure is used to store the information of NCP Bluetooth LE l2cap disconnection event */
typedef NCP_TLV_PACK_START struct  _NCP_L2CAP_DISCONNECT_EV {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** PSM value */
    uint16_t psm;
} NCP_TLV_PACK_END NCP_L2CAP_DISCONNECT_EV;

/** This structure is used to store the information of NCP Bluetooth LE l2cap receive data event */
typedef NCP_TLV_PACK_START struct  _NCP_L2CAP_RECEIVE_EV {
    /** Bluetooth LE address type \n
      0: public \n
      1: random
     */
    uint8_t address_type;
    /** Bluetooth LE address */
    uint8_t address[6];
    /** PSM value */
    uint16_t psm;
    /** reveive data length */
    uint8_t len;
    /** receive data value */
    uint8_t data[256];
} NCP_TLV_PACK_END NCP_L2CAP_RECEIVE_EV;

/** This structure is used to store the information of NCP Bluetooth LE TLV command */
typedef NCP_TLV_PACK_START struct _NCPCmd_DS_COMMAND
{
   /** Command Header : Command */
   NCP_COMMAND header;
   /** Command Body */
   union
   {
       /** NCP Bluetooth LE advertising */
        NCP_CMD_ADV_START adv_start;
        /** Set NCP Bluetooth LE advertising data */
        NCP_CMD_SET_ADV_DATA set_adv_data;
        /** Set NCP Bluetooth LE scan parameter */
        NCP_CMD_SET_SCAN_PARAM set_scan_parameter;
        /** NCP Bluetooth LE start scanning */
        NCP_CMD_SCAN_START scan_start;
        /** NCP Bluetooth LE connect/disconnect */
        NCP_CMD_CONNECT connect;
        /** Set NCP Bluetooth LE data len */
        NCP_CMD_SET_DATA_LEN set_data_len;
        /** Set NCP Bluetooth LE set Bluetooth PHY */
        NCP_CMD_SET_PHY set_phy;
        /** NCP Bluetooth LE update connect parameter */
        NCP_CMD_CONN_PARA_UPDATE conn_param_update;
        /** NCP Bluetooth LE connect encryption */
        NCP_CMD_ENCRYPTION conn_encryption;
        /** NCP Bluetooth LE set power mode */
        NCP_CMD_SET_POWER_MODE set_pw_mode;
        /** NCP Bluetooth LE set device address */
        NCP_CMD_SET_ADDR set_dev_addr;
        /** NCP Bluetooth LE read characteristic */
        NCP_GATT_READ_CMD gatt_read_char;
        /** NCP Bluetooth LE set device name */
        NCP_CMD_SET_NAME set_dev_name;
        /** NCP Bluetooth LE gatt add host service */
        NCP_CMD_SERVICE_ADD host_svc_add;
        /** NCP Bluetooth LE start service at host side */
        NCP_CMD_START_SERVICE host_start_svc;
        /** NCP Bluetooth LE gatt register service*/
        NCP_REGISTER_SERVICE register_service;
        /** NCP Bluetooth LE gatt set characteristic/descriptor service*/
        NCP_SET_VALUE_CMD gatt_set_value;
        /** NCP Bluetooth LE gatt discover primary Service*/
        NCP_DISC_PRIM_UUID_CMD discover_prim;
        /** NCP Bluetooth LE gatt discover characteristics*/
        NCP_DISC_CHRC_UUID_CMD discover_chrc;
        /** NCP Bluetooth LE gatt configure service to indicate characteristic value to client */
        NCP_CFG_SUBCRIBE_CMD cfg_subcribe;

        /** NCP Bluetooth LE l2cap connect */
        NCP_L2CAP_CONNECT_CMD l2cap_connect;
        /** NCP Bluetooth LE l2cap disconnect */
        NCP_L2CAP_DISCONNECT_CMD l2cap_disconnect;
        /** NCP Bluetooth LE l2cap send */
        NCP_L2CAP_SEND_CMD l2cap_send;
        /** NCP Bluetooth LE l2cap register*/
        NCP_L2CAP_REGISTER_CMD l2cap_register;
        /** NCP Bluetooth LE l2cap metrics */
        NCP_L2CAP_METRICS_CMD l2cap_metrics;
        /** NCP Bluetooth LE l2cap receive */
        NCP_L2CAP_RECEIVE_CMD l2cap_receive;
        
        /** NCP Bluetooth LE advertising reported event */
        NCP_DEVICE_ADV_REPORT_EV adv_reported;
        /** NCP Bluetooth LE connected event */
        NCP_DEVICE_CONNECTED_EV device_connected;
        /** NCP Bluetooth LE disonnected event */
        NCP_DEVICE_DISCONNECTED_EV device_disconnected;
        /** NCP Bluetooth LE passkey display event */
        NCP_PASSKEY_DISPLAY_EV passkey_display;
        /** NCP Bluetooth LE remote identity address resolved event */
        NCP_IDENTITY_RESOLVED_EV idenitiy_resolved;
        /** NCP Bluetooth LE connect parameter update event */
        NCP_CMD_CONN_PARA_UPDATE_EV conn_param_update_ev;
        /** NCP Bluetooth LE phy update event */
        NCP_CMD_PHY_UPDATE_EV phy_updated_ev;
        /** NCP Bluetooth LE data length update event */
        NCP_CMD_DATA_LEN_UPDATE_EV data_len_updated_ev;
        /** NCP Bluetooth LE security level changed event */
        NCP_SEC_LEVEL_CHANGED_EV sec_level_changed;

        /** NCP Bluetooth LE gatt notification receive event */
        NCP_NOTIFICATION_EV gatt_notification;
        /** NCP Bluetooth LE gatt attribute value changed event */
        NCP_ATTR_VALUE_CHANGED_EV attr_value_changed;
        /** NCP Bluetooth LE gatt client characteristic configuration changed event */
        NCP_CCC_CFG_CHANGED_EV gatt_ccc_cfg_changed_ev;
        /** NCP Bluetooth LE gatt client subscription event */
        NCP_SUBSCRIPTIONED_EV gatt_subscription_ev;
        /** NCP Bluetooth LE gatt discover primary service event */
        NCP_DISC_PRIM_EV gatt_disc_prim_ev;
        /** NCP Bluetooth LE gatt discover chracteristic event */
        NCP_DISC_CHRC_EV gatt_disc_chrc_ev;
        /** NCP Bluetooth LE gatt discover description event */
        NCP_DISC_ALL_DESC_EV gatt_disc_desc_ev;

        /** NCP Bluetooth LE l2cap connect event */
        NCP_L2CAP_CONNECT_EV l2cap_connect_ev;
        /** NCP Bluetooth LE l2cap disconnect event */
        NCP_L2CAP_DISCONNECT_EV l2cap_disconnect_ev;
        /** NCP Bluetooth LE l2cap receive event */
        NCP_L2CAP_RECEIVE_EV l2cap_receive_ev;
        
        /** NCP Bluetooth LE start advertising response */
        NCP_START_ADV_RP start_adv_rp;
        /** NCP Bluetooth LE gatt add service attribute response */
        NCP_ADD_SERVICE_RP add_service_rp;
        /** NCP Bluetooth LE gatt discover primary service response */
        NCP_DISC_PRIM_RP discover_prim_rp;
        /** NCP Bluetooth LE gatt discover characteristics response */
        NCP_DISC_CHRC_RP discover_chrc_rp;
        /** NCP Bluetooth LE gatt discover descriptors response */
        NCP_DISC_ALL_DESC_RP discover_desc_rp;
        /** NCP Bluetooth LE commissioning ssid/password receive response */
        NCP_NCS_INFO_RP ncs_info_rp;
   } params;
} NCP_TLV_PACK_END NCPCmd_DS_COMMAND, MCU_NCPCmd_DS_COMMAND;

#endif /* __NCP_CMD_BLE_H__ */
