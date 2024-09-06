/*!\file ncp_cmd_ble.h
 *\brief The file provides Bluetooth LE command and structure definition.
 */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 * Copyright (c) 2022 Codecoup
 * Copyright 2022-2024 NXP
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
/** Bluetooth LE Core support command ID */
#define NCP_CMD_BLE_CORE_SUPPORT_CMD    (NCP_CMD_BLE | NCP_CMD_BLE_CORE | NCP_MSG_TYPE_CMD | 0x00000001) /* Read supported commands*/
/** Bluetooth LE Core support command response ID */
#define NCP_RSP_BLE_CORE_SUPPORT_CMD    (NCP_CMD_BLE | NCP_CMD_BLE_CORE | NCP_MSG_TYPE_RESP | 0x00000001)
/** Bluetooth LE Core support service command ID */
#define NCP_CMD_BLE_CORE_SUPPORT_SER    (NCP_CMD_BLE | NCP_CMD_BLE_CORE | NCP_MSG_TYPE_CMD | 0x00000002) /* Read supported services*/
/** Bluetooth LE Core support service command response ID */
#define NCP_RSP_BLE_CORE_SUPPORT_SER    (NCP_CMD_BLE | NCP_CMD_BLE_CORE | NCP_MSG_TYPE_RESP | 0x00000002)
/** Bluetooth LE Core register command ID */
#define NCP_CMD_BLE_CORE_REGISTER       (NCP_CMD_BLE | NCP_CMD_BLE_CORE | NCP_MSG_TYPE_CMD | 0x00000003) /* register services */
/** Bluetooth LE Core register command response ID */
#define NCP_RSP_BLE_BLE_CORE_REGISTER   (NCP_CMD_BLE | NCP_CMD_BLE_CORE | NCP_MSG_TYPE_RESP | 0x00000003)
/** Bluetooth LE Core unregister command ID */
#define NCP_CMD_BLE_CORE_UNREGISTER     (NCP_CMD_BLE | NCP_CMD_BLE_CORE | NCP_MSG_TYPE_CMD | 0x00000004) /* unregister services*/
/** Bluetooth LE Core unregister command response ID */
#define NCP_RSP_BLE_CORE_UNREGISTER     (NCP_CMD_BLE | NCP_CMD_BLE_CORE | NCP_MSG_TYPE_RESP | 0x00000004)
/** Bluetooth LE Core reset command ID */
#define NCP_CMD_BLE_CORE_RESET          (NCP_CMD_BLE | NCP_CMD_BLE_CORE | NCP_MSG_TYPE_CMD | 0x00000006) /* reset board */
/** Bluetooth LE Core reset command response ID */
#define NCP_RSP_BLE_CORE_RESET          (NCP_CMD_BLE | NCP_CMD_BLE_CORE | NCP_MSG_TYPE_RESP | 0x00000006)
/** Bluetooth LE invalid command ID */
#define NCP_CMD_BLE_INVALID_CMD         (NCP_CMD_BLE | NCP_CMD_BLE_CORE | NCP_MSG_TYPE_CMD | 0x0000000a) /* invalid command recieve */

/** Bluetooth LE GAP set data length command ID */
#define NCP_CMD_BLE_GAP_SET_DATA_LEN         (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_CMD | 0x00000020) /* Set data len */
/** Bluetooth LE GAP set data length command response ID */
#define NCP_RSP_BLE_GAP_SET_DATA_LEN         (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_RESP | 0x00000020)
/** Bluetooth LE GAP set phy command ID */
#define NCP_CMD_BLE_GAP_SET_PHY              (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_CMD | 0x0000001f) /* Set phy */
/** Bluetooth LE GAP set phy command response ID */
#define NCP_RSP_BLE_GAP_SET_PHY              (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_RESP | 0x0000001f)
/** Bluetooth LE GAP set adv data command ID */
#define NCP_CMD_BLE_GAP_SET_ADV_DATA         (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_CMD | 0x0000001e) /* Set adv data */
/** Bluetooth LE GAP set adv data command response ID */
#define NCP_RSP_BLE_GAP_SET_ADV_DATA         (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_RESP | 0x0000001e)
/** Bluetooth LE GAP set scan parameter command ID */
#define NCP_CMD_BLE_GAP_SET_SCAN_PARAM       (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_CMD | 0x0000001d) /* Set scan parameter */
/** Bluetooth LE GAP set scan parameter command response ID */
#define NCP_RSP_BLE_GAP_SET_SCAN_PARAM       (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_RESP | 0x0000001d)
/** Bluetooth LE GAP start adv command ID */
#define NCP_CMD_BLE_GAP_START_ADV            (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_CMD | 0x0000000a) /* Start advertising */
/** Bluetooth LE GAP start adv command response ID */
#define NCP_RSP_BLE_GAP_START_ADV            (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_RESP | 0x0000000a)
/** Bluetooth LE GAP stop adv command ID */
#define NCP_CMD_BLE_GAP_STOP_ADV             (NCP_CMD_BLE | NCP_CMD_BLE_GAP | NCP_MSG_TYPE_CMD | 0x0000000b) /* Stop advertising */
/** Bluetooth LE GAP stop adv command response ID */
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

/** Bluetooth LE GATT add host service attribute command ID */
#define NCP_CMD_BLE_HOST_SERVICE_ADD         (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | 0x00000002) /* Add Host Attribute to Device Gatt datebase and start, ble-host-service-start */
/** Bluetooth LE GATT add host service attribute command response ID */
#define NCP_RSP_BLE_HOST_SERVICE_ADD         (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | 0x00000002)
/** Bluetooth LE GATT discovery primary service/characteristic/descriptor command ID */
#define NCP_CMD_BLE_HOST_SERVICE_DISC        (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | 0x00000003) /* Discover Primary Service/Characteristics/Descriptors */
/** Bluetooth LE GATT discovery primary service/characteristic/descriptor command response ID */
#define NCP_RSP_BLE_HOST_SERVICE_DISC        (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | 0x00000003)
/** Bluetooth LE GATT set characteristic/descriptor value command ID */
#define NCP_CMD_BLE_GATT_SET_VALUE           (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | 0x00000006) /* Set Characteristic/Descriptor Value */
/** Bluetooth LE GATT set characteristic/descriptor value command response ID */
#define NCP_RSP_BLE_GATT_SET_VALUE           (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | 0x00000006)
/** Bluetooth LE GATT start service command ID */
#define NCP_CMD_BLE_GATT_START_SERVICE       (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | 0x00000007) /* Start server with previously prepared attributes database. */
/** Bluetooth LE GATT start service command response ID */
#define NCP_RSP_BLE_GATT_START_SERVICE       (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | 0x00000007)
/** Bluetooth LE GATT discovery primary service command ID */
#define NCP_CMD_BLE_GATT_DISC_PRIM           (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | GATT_DISC_PRIM_UUID) /* Discover Primary Service */
/** Bluetooth LE GATT discovery primary service command response ID */
#define NCP_RSP_BLE_GATT_DISC_PRIM           (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | GATT_DISC_PRIM_UUID)
/** Bluetooth LE GATT discovery characteristic command ID */
#define NCP_CMD_BLE_GATT_DISC_CHRC           (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | GATT_DISC_CHRC_UUID) /* Discover Characteristics */
/** Bluetooth LE GATT discovery characteristic command response ID */
#define NCP_RSP_BLE_GATT_DISC_CHRC           (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | GATT_DISC_CHRC_UUID)
/** Bluetooth LE GATT read characteristic/descriptor command ID */
#define NCP_CMD_BLE_GATT_READ                (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | 0x00000011) /* Read Characteristic/Descriptor */
/** Bluetooth LE GATT read characteristic/descriptor command response ID */
#define NCP_RSP_BLE_GATT_READ                (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | 0x00000011)
/** Bluetooth LE GATT config service notify characteristic value command ID */
#define NCP_CMD_BLE_GATT_CFG_NOTIFY          (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | GATT_CFG_NOTIFY) /* Configure service to notify characteristic value to clinet */
/** Bluetooth LE GATT config service notify characteristic value command response ID */
#define NCP_RSP_BLE_GATT_CFG_NOTIFY          (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | GATT_CFG_NOTIFY)
/** Bluetooth LE GATT config service indicate characteristic value command ID */
#define NCP_CMD_BLE_GATT_CFG_INDICATE        (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | GATT_CFG_INDICATE) /* Configure service to indicate characteristic value to clinet */
/** Bluetooth LE GATT config service indicate characteristic value command response ID */
#define NCP_RSP_BLE_GATT_CFG_INDICATE        (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | GATT_CFG_INDICATE)
/** Bluetooth LE GATT write characteristic/descriptor command ID */
#define NCP_CMD_BLE_GATT_WRITE               (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | GATT_WRITE) /* Write Characteristic/Descriptor */
/** Bluetooth LE GATT write characteristic/descriptor command response ID */
#define NCP_RSP_BLE_GATT_WRITE               (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | GATT_WRITE)
/** Bluetooth LE GATT register service command ID */
#define NCP_CMD_BLE_GATT_REGISTER_SERVICE    (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | 0x00000020)  /* register a profile service */
/** Bluetooth LE GATT register service command response ID */
#define NCP_RSP_BLE_GATT_REGISTER_SERVICE    (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | 0x00000020)
/** Bluetooth LE GATT discovery descriptor command ID */
#define NCP_CMD_BLE_GATT_DESC_CHRC           (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_CMD | GATT_DISC_DESC_UUID) /* Discover Descriptors */
/** Bluetooth LE GATT discovery descriptor command response ID */
#define NCP_RSP_BLE_GATT_DESC_CHRC           (NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | GATT_DISC_DESC_UUID)

/** Bluetooth LE L2CAP connect command ID */
#define NCP_CMD_BLE_L2CAP_CONNECT            (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_CMD | 0x00000002) /* L2CAP connect */
/** Bluetooth LE L2CAP connect command response ID */
#define NCP_RSP_BLE_L2CAP_CONNECT            (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_RESP | 0x00000002)
/** Bluetooth LE L2CAP disconnect command ID */
#define NCP_CMD_BLE_L2CAP_DISCONNECT         (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_CMD | 0x00000003) /* L2CAP disconnect */
/** Bluetooth LE L2CAP disconnect command response ID */
#define NCP_RSP_BLE_L2CAP_DISCONNECT         (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_RESP | 0x00000003)
/** Bluetooth LE L2CAP send command ID */
#define NCP_CMD_BLE_L2CAP_SEND               (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_CMD | 0x00000004) /* L2CAP send */
/** Bluetooth LE L2CAP send command response ID */
#define NCP_RSP_BLE_L2CAP_SEND               (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_RESP | 0x00000004)
/** Bluetooth LE L2CAP register command ID */
#define NCP_CMD_BLE_L2CAP_REGISTER           (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_CMD | 0x0000000a) /* L2CAP register*/
/** Bluetooth LE L2CAP register command response ID */
#define NCP_RSP_BLE_L2CAP_REGISTER           (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_RESP | 0x0000000a)
/** Bluetooth LE L2CAP metrics command ID */
#define NCP_CMD_BLE_L2CAP_METRICS            (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_CMD | 0x0000000b) /* L2CAP metrics */
/** Bluetooth LE L2CAP metrics command response ID */
#define NCP_RSP_BLE_L2CAP_METRICS            (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_RESP | 0x0000000b)
/** Bluetooth LE L2CAP receive command ID */
#define NCP_CMD_BLE_L2CAP_RECEIVE            (NCP_CMD_BLE | NCP_CMD_BLE_L2CAP | NCP_MSG_TYPE_CMD | 0x0000000c) /* L2CAP receive */
/** Bluetooth LE L2CAP receive command response ID */
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
/** Bluetooth LE GATT notification received event */
#define NCP_EVENT_GATT_NOTIFICATION          (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | GATT_EV_NOTIFICATION | 0x200) /* GATT notification Receive event */
/** Bluetooth LE GATT attribute value changed event */
#define NCP_EVENT_ATTR_VALUE_CHANGED         (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | GATT_EV_ATTR_VALUE_CHANGED | 0x200) /* GATT Attribute Value Changed event */
/** Bluetooth LE GATT client characteristic configuration changed event */
#define NCP_EVENT_GATT_CCC_CFG_CHANGED       (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | GATT_EV_CCC_CFG_CHANGED | 0x200) /* GATT Client Characteristic Configuration Changed event */
/** Bluetooth LE GATT client subscription event */
#define NCP_EVENT_GATT_SUBSCRIPTIONED        (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | GATT_EV_SUBSCRIPTIONED | 0x200) /* GATT Client Subscription status event */
/** Bluetooth LE GATT discover primary service event */
#define NCP_EVENT_GATT_DISC_PRIM             (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | 0x19 | 0x200) /* Discover Primary Service event */
/** Bluetooth LE GATT discover characteristic event */
#define NCP_EVENT_GATT_DISC_CHRC             (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | 0x20 | 0x200) /* Discover Characteristics event */
/** Bluetooth LE GATT discover descriptor event */
#define NCP_EVENT_GATT_DISC_DESC             (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | 0x21 | 0x200) /* Discover Descriptors event */
/* NXP commission service info */
#define NCP_EVENT_GATT_NCS_INFO              (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | 0x30 | 0x200) /* NCS Commission info event */

/** Bluetooth LE L2CAP connect event */
#define NCP_EVENT_L2CAP_CONNECT              (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | L2CAP_EV_CONNECT | 0x300) /* L2CAP Connect event */
/** Bluetooth LE L2CAP disconnect event */
#define NCP_EVENT_L2CAP_DISCONNECT           (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | L2CAP_EV_DISCONNECT | 0x300) /* L2CAP Disconnect event */
/** Bluetooth LE L2CAP receive event */
#define NCP_EVENT_L2CAP_RECEIVE              (NCP_CMD_BLE | NCP_CMD_BLE_EVENT | NCP_MSG_TYPE_EVENT | L2CAP_EV_RECEIVE | 0x300) /* L2CAP Receive event */

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

/** NCP Bluetooth LE set board address config structure */
typedef  NCP_TLV_PACK_START struct _ncp_ble_set_bd_address_cfg {
    /** paramter id, default value with 0XFE */
    uint8_t   paramater_id;
    /** board address length */
    uint8_t   bd_addr_len;
    /** board address */
    uint8_t   bd_address[6];
} NCP_TLV_PACK_END ncp_ble_set_bd_address_cfg;

/** NCP Bluetooth LE low power mode opcode */
#define BT_HCI_VD_LOW_POWER_MODE      BT_OP(BT_OGF_VS, 0x0023)
/** NCP Bluetooth LE low power mode config structure */
typedef  NCP_TLV_PACK_START struct _ncp_ble_low_power_mode_cfg {
    /**
     * cpu2 power mode,
     * 2: auto sleep disable
     * 3: auto sleep enable
     */
    uint8_t   power_mode;
    /** sleep timeout value */
	uint16_t  timeout;
} NCP_TLV_PACK_END ncp_ble_low_power_mode_cfg;

/** NCP Bluetooth LE multi advertising opcode */
#define BT_HCI_VD_MULTI_ADV_CMD       BT_OP(BT_OGF_VS, 0x0154)



/** NCP Bluetooth LE Core read support command ID */
#define CORE_READ_SUPPORTED_COMMANDS    0x01
/** NCP Bluetooth LE Core read support response structure */
typedef NCP_TLV_PACK_START struct core_read_supported_commands_rp_tag {
    /** response of core read support command */
    uint8_t rsp;
} NCP_TLV_PACK_END core_read_supported_commands_rp_t;

/** NCP Bluetooth LE Core read support service command ID */
#define CORE_READ_SUPPORTED_SERVICES    0x02
/** NCP Bluetooth LE Core read support service response structure */
typedef NCP_TLV_PACK_START struct core_read_supported_services_rp_tag {
    /** response of core read support service command */
    uint8_t rsp;
} NCP_TLV_PACK_END core_read_supported_services_rp_t;

/** NCP Bluetooth LE Core register service command ID */
#define CORE_REGISTER_SERVICE           0x03
/** NCP Bluetooth LE Core register service command structure */
typedef NCP_TLV_PACK_START struct core_register_service_cmd_tag {
    /** register service id */
    uint8_t id;
} NCP_TLV_PACK_END core_register_service_cmd_t;

/** NCP Bluetooth LE Core unregister service command ID */
#define CORE_UNREGISTER_SERVICE         0x04
typedef NCP_TLV_PACK_START struct core_unregister_service_cmd_tag {
    /** unregister service id */
    uint8_t id;
} NCP_TLV_PACK_END core_unregister_service_cmd_t;

/** NCP Bluetooth LE Core reset board command ID */
#define CORE_RESET_BOARD                0x06

/* NCP Bluetooth LE Core board ready event ID */
#define CORE_EV_IUT_READY               0x80

/** NCP Bluetooth LE GAP read support command ID */
#define GAP_READ_SUPPORTED_COMMANDS 0x01
/** NCP Bluetooth LE GAP read support command response structure */
typedef NCP_TLV_PACK_START struct gap_read_supported_commands_rp {
    /** response data of read support command */
    uint8_t data;
} NCP_TLV_PACK_END gap_read_supported_commands_rp_t;

/** NCP Bluetooth LE GAP read controller index list command ID */
#define GAP_READ_CONTROLLER_INDEX_LIST  0x02
/** NCP Bluetooth LE GAP read controller index list command response ID */
typedef NCP_TLV_PACK_START struct gap_read_controller_index_list_rp {
    /** number of index */  
    uint8_t num;
    /** index id */
    uint8_t index;
} NCP_TLV_PACK_END gap_read_controller_index_list_rp_t;

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
/** NCP Bluetooth LE GAP set adv ID */
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

/** NCP Bluetooth LE GAP read controller info ID */
#define GAP_READ_CONTROLLER_INFO  0x03
/** NCP Bluetooth LE GAP read controller info response structure */
typedef NCP_TLV_PACK_START struct gap_read_controller_info_rp {
    /** device address */
    uint8_t  address[6];
    /** device supoort setting */
    uint32_t supported_settings;
    /** device current setting */
    uint32_t current_settings;
    /** class of device */
    uint8_t  cod[3];
    /** device name */
    uint8_t  name[249];
    /** device short name */
    uint8_t  short_name[11];
} NCP_TLV_PACK_END gap_read_controller_info_rp_t;

/** NCP Bluetooth LE GAP reset ID */
#define GAP_RESET     0x04
/** NCP Bluetooth LE GAP reset response structure */
typedef NCP_TLV_PACK_START struct gap_reset_rp {
    /** device current setting */
    uint32_t current_settings;
} NCP_TLV_PACK_END gap_reset_rp_t;

/** NCP Bluetooth LE GAP set power ID */
#define GAP_SET_POWERED     0x05
/** NCP Bluetooth LE GAP set power command structure */
typedef NCP_TLV_PACK_START struct gap_set_powered_cmd {
    /** device power set
     * 0: disable
     * 1: enable
     */
    uint8_t powered;
} NCP_TLV_PACK_END gap_set_powered_cmd_t;
/** NCP Bluetooth LE GAP set power response structure */
typedef NCP_TLV_PACK_START struct gap_set_powered_rp {
    /** device current setting */
    uint32_t current_settings;
} NCP_TLV_PACK_END gap_set_powered_rp_t;

/** NCP Bluetooth LE GAP set connectable ID */
#define GAP_SET_CONNECTABLE   0x06
/** NCP Bluetooth LE GAP set connectable command structure */
typedef NCP_TLV_PACK_START struct gap_set_connectable_cmd {
    /** device connectable setting
     * 0: disable
     * 1: enable
     */
    uint8_t connectable;
} NCP_TLV_PACK_END gap_set_connectable_cmd_t;
/** NCP Bluetooth LE GAP set connectable response structure */
typedef NCP_TLV_PACK_START struct gap_set_connectable_rp {
    /** device current setting */
    uint32_t current_settings;
} NCP_TLV_PACK_END gap_set_connectable_rp_t;

/** NCP Bluetooth LE GAP fast connect ID */
#define GAP_SET_FAST_CONNECTABLE  0x07
/** NCP Bluetooth LE GAP fast connectable command structure */
typedef NCP_TLV_PACK_START struct gap_set_fast_connectable_cmd {
    /** fast connect setting */
    uint8_t fast_connectable;
} NCP_TLV_PACK_END gap_set_fast_connectable_cmd_t;
/** NCP Bluetooth LE GAP fast connectable response structure */
typedef NCP_TLV_PACK_START struct gap_set_fast_connectable_rp {
    /** fast connect setting */
    uint32_t current_settings;
} NCP_TLV_PACK_END gap_set_fast_connectable_rp_t;

/** NCP Bluetooth LE GAP non discoverable ID */
#define GAP_NON_DISCOVERABLE      0x00
/** NCP Bluetooth LE GAP general discoverable ID */
#define GAP_GENERAL_DISCOVERABLE  0x01
/** NCP Bluetooth LE GAP limit discoverable ID */
#define GAP_LIMITED_DISCOVERABLE  0x02
/** NCP Bluetooth LE GAP discoverable ID */
#define GAP_SET_DISCOVERABLE    0x08
/** NCP Bluetooth LE GAP set discoverable command structure */
typedef NCP_TLV_PACK_START struct gap_set_discoverable_cmd {
    /** discoverable setting 
     * 0: non-discoverable
     * 1: general discoverable
     * 2: limit discoverable
     * 8: dsicoverable
    */
    uint8_t discoverable;
} NCP_TLV_PACK_END gap_set_discoverable_cmd_t;
/** NCP Bluetooth LE GAP set discoverable response structure */
typedef NCP_TLV_PACK_START struct gap_set_discoverable_rp {
    /** device current setting */
    uint32_t current_settings;
} NCP_TLV_PACK_END gap_set_discoverable_rp_t;

/** NCP Bluetooth LE GAP set bondable ID */
#define GAP_SET_BONDABLE    0x09
/** NCP Bluetooth LE GAP set bondable command structure */
typedef NCP_TLV_PACK_START struct gap_set_bondable_cmd {
    /** device bondable setting */
    uint8_t bondable;
} NCP_TLV_PACK_END gap_set_bondable_cmd_t;
/** NCP Bluetooth LE GAP set bondable response structure */
typedef NCP_TLV_PACK_START struct gap_set_bondable_rp {
    /** device current setting */
    uint32_t current_settings;
} NCP_TLV_PACK_END gap_set_bondable_rp_t;

/** NCP Bluetooth LE GAP set advertising ID */
#define GAP_START_ADVERTISING 0x0a
/** NCP Bluetooth LE GAP set advertising command structure */
typedef NCP_TLV_PACK_START struct gap_start_advertising_cmd {
    /** adv data length */
    uint8_t adv_data_len;
    /** scan response length */
    uint8_t scan_rsp_len;
    /** adv/scan response data */
    uint8_t adv_sr_data[];
} NCP_TLV_PACK_END gap_start_advertising_cmd_t;
/** NCP Bluetooth LE GAP set advertising response structure */
typedef NCP_TLV_PACK_START struct gap_start_advertising_rp {
    /** device current setting */
    uint32_t current_settings;
} NCP_TLV_PACK_END gap_start_advertising_rp_t;

/** NCP Bluetooth LE GAP stop advertising ID */
#define GAP_STOP_ADVERTISING    0x0b
/** NCP Bluetooth LE GAP stop advertising response structure */
typedef NCP_TLV_PACK_START struct gap_stop_advertising_rp {
    /** device current setting */
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
/** NCP Bluetooth LE GAP start discovery command structure */
typedef NCP_TLV_PACK_START struct gap_start_discovery_cmd {
    /** discovery setting
     * 1: discovery le
     * 2: discovery bredr
     * 4: limit discovery
     * 8: active scan
     * 0x10: observe
     * 0x20: discovery own address 
     */
    uint8_t flags;
} NCP_TLV_PACK_END gap_start_discovery_cmd_t;

/** NCP Bluetooth LE GAP stop discovery ID */
#define GAP_STOP_DISCOVERY    0x0d
/** NCP Bluetooth LE GAP connect ID */
#define GAP_CONNECT     0x0e
/** NCP Bluetooth LE GAP connect command structure */
typedef NCP_TLV_PACK_START struct gap_connect_cmd {
    /** address type
     * 0: public
     * 1: random
     */
    uint8_t address_type;
    /** peer address */
    uint8_t address[6];
} NCP_TLV_PACK_END gap_connect_cmd_t;

/** NCP Bluetooth LE GAP disconnect ID */
#define GAP_DISCONNECT      0x0f
/** NCP Bluetooth LE GAP connect command structure */
typedef NCP_TLV_PACK_START struct gap_disconnect_cmd {
    /** address type
     * 0: public
     * 1: random
     */
    uint8_t  address_type;
    /** peer address */
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
/** NCP Bluetooth LE GAP set IO capablility command structure */
typedef NCP_TLV_PACK_START struct gap_set_io_cap_cmd {
    /** io capability setting */
    uint8_t io_cap;
} NCP_TLV_PACK_END gap_set_io_cap_cmd_t;

/** NCP Bluetooth LE GAP pair ID */
#define GAP_PAIR      0x11
/** NCP Bluetooth LE GAP pair command structure */
typedef NCP_TLV_PACK_START struct gap_pair_cmd {
    /** peer address type */
    uint8_t address_type;
    /** peer address */
    uint8_t address[6];
} NCP_TLV_PACK_END gap_pair_cmd_t;

/** NCP Bluetooth LE GAP unpair ID */
#define GAP_UNPAIR      0x12
/** NCP Bluetooth LE GAP unpair command structure */
typedef NCP_TLV_PACK_START struct gap_unpair_cmd {
    /** peer address type */
    uint8_t address_type;
    /** peer address */
    uint8_t address[6];
} NCP_TLV_PACK_END gap_unpair_cmd_t;

/** NCP Bluetooth LE GAP passkey entry ID */
#define GAP_PASSKEY_ENTRY   0x13
/** NCP Bluetooth LE GAP passkey entry command structure */
typedef NCP_TLV_PACK_START struct gap_passkey_entry_cmd {
    /** peer address type */
    uint8_t  address_type;
    /** peer address */
    uint8_t  address[6];
    /** peer address */
    uint32_t passkey;
} NCP_TLV_PACK_END gap_passkey_entry_cmd_t;

/** NCP Bluetooth LE GAP passkey confirm ID */
#define GAP_PASSKEY_CONFIRM   0x14
/** NCP Bluetooth LE GAP passkey confirm command strcuture */
typedef NCP_TLV_PACK_START struct gap_passkey_confirm_cmd {
    /** peer address type */
    uint8_t address_type;
    /** peer address */
    uint8_t address[6];
    /** passkey match or not */
    uint8_t match;
} NCP_TLV_PACK_END gap_passkey_confirm_cmd_t;

/** NCP Bluetooth LE start directed adv hd */
#define GAP_START_DIRECTED_ADV_HD BIT(0)
/** NCP Bluetooth LE start directed adv own id */
#define GAP_START_DIRECTED_ADV_OWN_ID BIT(1)
/** NCP Bluetooth LE start directed adv peer rpa */
#define GAP_START_DIRECTED_ADV_PEER_RPA BIT(2)

/** NCP Bluetooth LE start directed adv ID */
#define GAP_START_DIRECTED_ADV    0x15
/** NCP Bluetooth LE start directed adv command structure */
typedef NCP_TLV_PACK_START struct gap_start_directed_adv_cmd {
    /** peer address type */
    uint8_t address_type;
    /** peer address */
    uint8_t address[6];
    /** passkey match or not */
    uint16_t options;
} NCP_TLV_PACK_END gap_start_directed_adv_cmd_t;
/** NCP Bluetooth LE start directed adv response structure */
typedef NCP_TLV_PACK_START struct gap_start_directed_adv_rp {
    /** device current setting */
    uint32_t current_settings;
} NCP_TLV_PACK_END gap_start_directed_adv_rp_t;

/** NCP Bluetooth LE GAP connection parameter update ID */
#define GAP_CONN_PARAM_UPDATE   0x16
/** NCP Bluetooth LE GAP connection parameter update command structure */
typedef NCP_TLV_PACK_START struct gap_conn_param_update_cmd {
    /** peer address type */
    uint8_t address_type;
    /** peer address */
    uint8_t address[6];
    /** connection minimal interval */
    uint16_t interval_min;
    /** connection maximum interval */
    uint16_t interval_max;
    /** connection latency */
    uint16_t latency;
    /** connection timeout */
    uint16_t timeout;
} NCP_TLV_PACK_END gap_conn_param_update_cmd_t;

/** NCP Bluetooth LE GAP pairing consent ID */
#define GAP_PAIRING_CONSENT   0x17
/** NCP Bluetooth LE GAP pairing consent command structure */
typedef NCP_TLV_PACK_START struct gap_pairing_consent_cmd {
    /** peer address type */
    uint8_t address_type;
    /** peer address */
    uint8_t address[6];
    /** pair consent */
    uint8_t consent;
} NCP_TLV_PACK_END gap_pairing_consent_cmd_t;

/** NCP Bluetooth LE oob legacy set data ID */
#define GAP_OOB_LEGACY_SET_DATA   0x18
/** NCP Bluetooth LE oob legacy set data command structure */
typedef NCP_TLV_PACK_START struct gap_oob_legacy_set_data_cmd {
    /** out of bound data */
    uint8_t oob_data[16];
} NCP_TLV_PACK_END gap_oob_legacy_set_data_cmd_t;

/** NCP Bluetooth LE oob security get local data ID */
#define GAP_OOB_SC_GET_LOCAL_DATA 0x19
/** NCP Bluetooth LE oob security get local data response structure */
typedef NCP_TLV_PACK_START struct gap_oob_sc_get_local_data_rp {
    /** local rand data */
    uint8_t rand[16];
    /** local conf data */
    uint8_t conf[16];
} NCP_TLV_PACK_END gap_oob_sc_get_local_data_rp_t;

/** NCP Bluetooth LE oob security set remote data ID */
#define GAP_OOB_SC_SET_REMOTE_DATA  0x1a
typedef NCP_TLV_PACK_START struct gap_oob_sc_set_remote_data_cmd {
    /** local rand data */
    uint8_t rand[16];
    /** local conf data */
    uint8_t conf[16];
} NCP_TLV_PACK_END gap_oob_sc_set_remote_data_cmd_t;

/** NCP Bluetooth LE GAP set mitm ID */
#define GAP_SET_MITM      0x1b
/** NCP Bluetooth LE GAP set mitm ID command structure*/
typedef NCP_TLV_PACK_START struct gap_set_mitm {
    /** mitm setting */
    uint8_t mitm;
} NCP_TLV_PACK_END gap_set_mitm_t;

/** NCP Bluetooth LE le address type structure*/
typedef struct le_addr {
    /** address type */
    uint8_t type;
    /** le address */
    uint8_t address[6];
} le_addr_t;

/** NCP Bluetooth LE GAP set fileter list ID */
#define GAP_SET_FILTER_LIST 0x1c
/** NCP Bluetooth LE GAP set fileter list type structure */
typedef NCP_TLV_PACK_START struct gap_set_filter_list {
    /** filter list number to set */
    uint8_t cnt;
    /** list of filter address */
    le_addr_t addr;
} NCP_TLV_PACK_END gap_set_filter_list_t;

/** NCP Bluetooth LE GAP set scan parameter ID */
#define GAP_SET_SCAN_PARAMETER 0x1d
/** NCP Bluetooth LE GAP set scan parameter command strcuture */
typedef NCP_TLV_PACK_START struct gap_set_scan_param_cmd {
  /** Bit-field of scanning options. */
  uint32_t options;
  /** Scan interval (N * 0.625 ms) */
  uint16_t interval;
  /** Scan window (N * 0.625 ms) */
  uint16_t window;
} NCP_TLV_PACK_END gap_set_scan_param_cmd_t;

/** NCP Bluetooth LE GAP set adv data ID */
#define GAP_SET_ADV_DATA 0x1e
/** NCP Bluetooth LE GAP set adv data command structure */
typedef NCP_TLV_PACK_START struct gap_set_adv_data_cmd {
    /** adv data length */
    uint8_t adv_data_len;
    /** adv data */
    uint8_t data[31];
} NCP_TLV_PACK_END gap_set_adv_data_cmd_t;

/** NCP Bluetooth LE GAP set adv data length command structure */
typedef NCP_TLV_PACK_START struct gap_set_data_len_cmd {
    /** device address type  */
    uint8_t  address_type;
    /** device address */
    uint8_t  address[6];
    /** adv time flag */
    uint8_t  time_flag;
    /** adv tx maximum length */
    uint16_t tx_max_len;
    /** adv tx maximum time */
    uint16_t tx_max_time;
} NCP_TLV_PACK_END gap_set_data_len_cmd_t;

/** NCP Bluetooth LE GAP set phy command structure */
typedef NCP_TLV_PACK_START struct gap_set_phy_cmd {
    /** device address type  */
    uint8_t  address_type;
    /** device address */
    uint8_t  address[6];
    /** phy option */
    uint16_t options;
    /** perferred tx phy */
    uint8_t  pref_tx_phy;
    /** perferred rx phy */
    uint8_t  pref_rx_phy;
} NCP_TLV_PACK_END gap_set_phy_cmd_t;

/** NCP Bluetooth LE new setting event ID */
#define GAP_EV_NEW_SETTINGS   0x80
/** NCP Bluetooth LE new setting event structure */
typedef NCP_TLV_PACK_START struct gap_new_settings_ev {
    /** device current setting */
    uint32_t current_settings;
} NCP_TLV_PACK_END gap_new_settings_ev_t;

/** NCP Bluetooth LE device found flag */
#define GAP_DEVICE_FOUND_FLAG_RSSI  0x01
#define GAP_DEVICE_FOUND_FLAG_AD    0x02
#define GAP_DEVICE_FOUND_FLAG_SD    0x04

/** NCP Bluetooth LE device found event ID */
#define GAP_EV_DEVICE_FOUND   0x81
/** NCP Bluetooth LE device found event structure */
typedef NCP_TLV_PACK_START struct gap_device_found_ev {
    /** scan address type */
    uint8_t  address_type;
    /** scan address */
    uint8_t  address[6];
    /** scan rssi */
    int8_t   rssi;
    /** scan flag */
    uint8_t  flags;
    /** scan data length */
    uint16_t eir_data_len;
    /** scan data */
    uint8_t  eir_data[];
} NCP_TLV_PACK_END gap_device_found_ev_t;

/** NCP Bluetooth LE device connected event ID */
#define GAP_EV_DEVICE_CONNECTED   0x82
/** NCP Bluetooth LE device connected event structure */
typedef NCP_TLV_PACK_START struct gap_device_connected_ev {
    /** connect address type */
    uint8_t address_type;
    /** connect address */
    uint8_t address[6];
    /** connection interval */
    uint16_t interval;
    /** connection latency */
    uint16_t latency;
    /** connection timeout */
    uint16_t timeout;
} NCP_TLV_PACK_END gap_device_connected_ev_t;

/** NCP Bluetooth LE device disconnected event ID */
#define GAP_EV_DEVICE_DISCONNECTED  0x83
/** NCP Bluetooth LE device disconnected event structure */
typedef NCP_TLV_PACK_START struct gap_device_disconnected_ev {
    /** disconnect address type */
    uint8_t address_type;
    /** disconnect address */
    uint8_t address[6];
} NCP_TLV_PACK_END gap_device_disconnected_ev_t;

/** NCP Bluetooth LE passkey display event ID */
#define GAP_EV_PASSKEY_DISPLAY    0x84
/** NCP Bluetooth LE passkey display event structure */
typedef NCP_TLV_PACK_START struct gap_passkey_display_ev {
    /** device address type */
    uint8_t  address_type;
    /** device address */
    uint8_t  address[6];
    /** device passkey number */
    uint32_t passkey;
} NCP_TLV_PACK_END gap_passkey_display_ev_t;

/** NCP Bluetooth LE passkey entry request event ID */
#define GAP_EV_PASSKEY_ENTRY_REQ  0x85
/** NCP Bluetooth LE passkey entry request event structure */
typedef NCP_TLV_PACK_START struct gap_passkey_entry_req_ev {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
} NCP_TLV_PACK_END gap_passkey_entry_req_ev_t;

/** NCP Bluetooth LE passkey confirm request event ID */
#define GAP_EV_PASSKEY_CONFIRM_REQ  0x86
/** NCP Bluetooth LE passkey confirm request event structure */
typedef NCP_TLV_PACK_START struct gap_passkey_confirm_req_ev {
    /** device address type */
    uint8_t  address_type;
    /** device address */
    uint8_t  address[6];
    /** device passkey number */
    uint32_t passkey;
} NCP_TLV_PACK_END gap_passkey_confirm_req_ev_t;

/** NCP Bluetooth LE identity resolved event ID */
#define GAP_EV_IDENTITY_RESOLVED  0x87
/** NCP Bluetooth LE identity resolved event structure */
typedef NCP_TLV_PACK_START struct gap_identity_resolved_ev {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
    /** device identity address type */
    uint8_t identity_address_type;
    /** device identity address */
    uint8_t identity_address[6];
} NCP_TLV_PACK_END gap_identity_resolved_ev_t;

/** NCP Bluetooth LE connection parameter update event ID */
#define GAP_EV_CONN_PARAM_UPDATE  0x88
/** NCP Bluetooth LE connection parameter update event structure */
typedef NCP_TLV_PACK_START struct gap_conn_param_update_ev {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
    /** connection interval */
    uint16_t interval;
    /** connection latency */
    uint16_t latency;
    /** connection timeout */
    uint16_t timeout;
} NCP_TLV_PACK_END gap_conn_param_update_ev_t;

/** NCP Bluetooth LE GAP security level */
#define GAP_SEC_LEVEL_UNAUTH_ENC  0x01
#define GAP_SEC_LEVEL_AUTH_ENC    0x02
#define GAP_SEC_LEVEL_AUTH_SC     0x03

/** NCP Bluetooth LE security level changed event ID */
#define GAP_EV_SEC_LEVEL_CHANGED  0x89
/** NCP Bluetooth LE security level changed event structure */
typedef NCP_TLV_PACK_START struct gap_sec_level_changed_ev {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
    /** connection security level */
    uint8_t sec_level;
} NCP_TLV_PACK_END gap_sec_level_changed_ev_t;

/** NCP Bluetooth LE pairing consent request event ID */
#define GAP_EV_PAIRING_CONSENT_REQ  0x8a
/** NCP Bluetooth LE pairing consent request event structure */
typedef NCP_TLV_PACK_START struct gap_pairing_consent_req_ev {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
} NCP_TLV_PACK_END gap_pairing_consent_req_ev_t;

/** NCP Bluetooth LE bonding lost event ID */
#define GAP_EV_BOND_LOST  0x8b
/** NCP Bluetooth LE bonding lost event structure */
typedef NCP_TLV_PACK_START struct gap_bond_lost_ev {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
} NCP_TLV_PACK_END gap_bond_lost_ev_t;

/** NCP Bluetooth LE pairing failed event ID */
#define GAP_EV_PAIRING_FAILED   0x8c
/** NCP Bluetooth LE pairing failed event structure */
typedef NCP_TLV_PACK_START struct gap_bond_pairing_failed_ev {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
    /** pairing failed reason */
    uint8_t reason;
} NCP_TLV_PACK_END gap_bond_pairing_failed_ev_t;

/** NCP Bluetooth LE phy update event ID */
#define GAP_EV_PHY_UPDATED  0x91
/** NCP Bluetooth LE phy update event structure */
typedef NCP_TLV_PACK_START struct gap_phy_updated_ev {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
    /** tx phy used */
    uint8_t tx_phy;
    /** rx phy used */
    uint8_t rx_phy;
} NCP_TLV_PACK_END gap_phy_updated_ev_t;

/** NCP Bluetooth LE data length update event ID */
#define GAP_EV_DATA_LEN_UPDATED  0x92
/** NCP Bluetooth LE data length update event structure */
typedef NCP_TLV_PACK_START struct gap_data_len_updated_ev {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
    /** connection tx maximum length */
    uint16_t tx_max_len;
    /** connection tx maximum time */
    uint16_t tx_max_time;
    /** connection rx maximum length */
    uint16_t rx_max_len;
    /** connection rx maximum time */
    uint16_t rx_max_time;
} NCP_TLV_PACK_END gap_data_len_updated_ev_t;

/** NCP Bluetooth LE l2cap read support command ID */
#define L2CAP_READ_SUPPORTED_COMMANDS   0x01
/** NCP Bluetooth LE l2cap read support command response structure */
typedef NCP_TLV_PACK_START struct l2cap_read_supported_commands_rp_tag {
    /** l2cap support command */
    uint8_t data[1];
} NCP_TLV_PACK_END l2cap_read_supported_commands_rp_t;

/** NCP Bluetooth LE opt  */
#define L2CAP_CONNECT_OPT_ECFC          0x01
#define L2CAP_CONNECT_OPT_HOLD_CREDIT   0x02

/** NCP Bluetooth LE l2cap connect ID */
#define L2CAP_CONNECT   0x02
/** NCP Bluetooth LE l2cap connect command structure */
typedef NCP_TLV_PACK_START struct l2cap_connect_cmd_tag {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
    /** protocol service multiplexo */
    uint16_t psm;
    /** security level */
    uint8_t sec;
    /** security level flag */
    uint8_t sec_flag;
} NCP_TLV_PACK_END l2cap_connect_cmd_t;

/** NCP Bluetooth LE l2cap connect response structure */
typedef NCP_TLV_PACK_START struct l2cap_connect_rp_tag {
    /** receive packet number */
    uint8_t num;
    /** receive channel id */
    uint8_t chan_id[];
} NCP_TLV_PACK_END l2cap_connect_rp_t;

/** NCP Bluetooth LE l2cap disconnect ID */
#define L2CAP_DISCONNECT    0x03
/** NCP Bluetooth LE l2cap disconnect command strcuture */
typedef NCP_TLV_PACK_START struct l2cap_disconnect_cmd_tag {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
} NCP_TLV_PACK_END l2cap_disconnect_cmd_t;

/** NCP Bluetooth LE l2cap send data ID */
#define L2CAP_SEND_DATA     0x04
/** NCP Bluetooth LE l2cap send data command structure */
typedef NCP_TLV_PACK_START struct l2cap_send_data_cmd_tag {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
    /** send times */
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
/** NCP Bluetooth LE l2cap listen command structure */
typedef NCP_TLV_PACK_START struct l2cap_listen_cmd_tag {
    /** protocol service multiplexor */
    uint16_t psm;
    /** transport type */
    uint8_t transport;
    /** maximum transmission unit */
    uint16_t mtu;
    /** command response */
    uint16_t response;
} NCP_TLV_PACK_END l2cap_listen_cmd_t;

/** NCP Bluetooth LE l2cap accept connection ID */
#define L2CAP_ACCEPT_CONNECTION     0x06
/** NCP Bluetooth LE l2cap accept connection command structure */
typedef NCP_TLV_PACK_START struct l2cap_accept_connection_cmd_tag {
    /** connection channel ID */
    uint8_t chan_id;
    /** command result */
    uint16_t result;
} NCP_TLV_PACK_END l2cap_accept_connection_cmd_t;

/** NCP Bluetooth LE l2cap reconfigure ID */
#define L2CAP_RECONFIGURE   0x07
/** NCP Bluetooth LE l2cap reconfigure command structure */
typedef NCP_TLV_PACK_START struct l2cap_reconfigure_cmd_tag {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
    /** maximum transmission unit */
    uint16_t mtu;
    /** send number */
    uint8_t num;
    /** send channel ID */
    uint8_t chan_id[];
} NCP_TLV_PACK_END l2cap_reconfigure_cmd_t;

/** NCP Bluetooth LE l2cap credit ID */
#define L2CAP_CREDITS   0x08
/** NCP Bluetooth LE l2cap credit command structure */
typedef NCP_TLV_PACK_START struct l2cap_credits_cmd_tag {
    /** channel ID */
    uint8_t chan_id;
} NCP_TLV_PACK_END l2cap_credits_cmd_t;

/** NCP Bluetooth LE l2cap disconnect eatt channel ID */
#define L2CAP_DISCONNECT_EATT_CHANS     0x09
/** NCP Bluetooth LE l2cap disconnect eatt channel command structure */
typedef NCP_TLV_PACK_START struct l2cap_disconnect_eatt_chans_cmd_tag {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
    /** channel count */
    uint8_t count;
} NCP_TLV_PACK_END l2cap_disconnect_eatt_chans_cmd_t;

/** NCP Bluetooth LE l2cap register PSM ID */
#define L2CAP_REGISTER_PSM     0x0a
/** NCP Bluetooth LE l2cap register PSM command structure */
typedef NCP_TLV_PACK_START struct l2cap_register_psm_cmd_tag {
    /** protocol service multiplexor */
    uint16_t psm;
    /** security level */
    uint8_t sec_level;
    /** security flag */
    uint8_t sec_flag;
    /** l2cap policy */
    uint8_t policy;
    /** l2cap policy flag */
    uint8_t policy_flag;
} NCP_TLV_PACK_END l2cap_register_psm_cmd_t;

/** NCP Bluetooth LE l2cap metrics ID */
#define L2CAP_METRICS    0x0b
/** NCP Bluetooth LE l2cap metrics command structure */
typedef NCP_TLV_PACK_START struct l2cap_metrics_cmd_tag {
    /** l2cap metric flag */
    bool metrics_flag;
} NCP_TLV_PACK_END l2cap_metrics_cmd_t;

/** NCP Bluetooth LE l2cap receive ID */
#define L2CAP_RECV    0x0c
/** NCP Bluetooth LE l2cap receive command structure */
typedef NCP_TLV_PACK_START struct l2cap_recv_cmd_tag {
    /** receive delay time */
    uint32_t l2cap_recv_delay_ms;
} NCP_TLV_PACK_END l2cap_recv_cmd_t;

/** NCP Bluetooth LE connection request event ID */
#define L2CAP_EV_CONNECTION_REQ     0x80
/** NCP Bluetooth LE connection request event structure */
typedef NCP_TLV_PACK_START struct l2cap_connection_req_ev_tag {
    /** channel ID */
    uint8_t chan_id;
    /** protocol service multiplexor */
    uint16_t psm;
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
} NCP_TLV_PACK_END l2cap_connection_req_ev_t;

/** NCP Bluetooth LE l2cap connect event ID */
#define L2CAP_EV_CONNECTED  0x81
/** NCP Bluetooth LE l2cap connect event structure */
typedef NCP_TLV_PACK_START struct l2cap_connected_ev_tag {
    /** channel ID */
    uint8_t chan_id;
    /** protocol service multiplexor */
    uint16_t psm;
    /** remote maximum transmission unit */
    uint16_t mtu_remote;
    /** remote maximum payload size */
    uint16_t mps_remote;
    /** local maximum transmission unit */
    uint16_t mtu_local;
    /** local maximum payload size */
    uint16_t mps_local;
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
} NCP_TLV_PACK_END l2cap_connected_ev_t;

/** NCP Bluetooth LE l2cap disconnect event ID */
#define L2CAP_EV_DISCONNECTED   0x82
/** NCP Bluetooth LE l2cap disconnect event structure */
typedef NCP_TLV_PACK_START struct l2cap_disconnected_ev_tag {
    /** l2cap disconnect result */
    uint16_t result;
    /** channel id */
    uint8_t chan_id;
    /** protocol service multiplexor */
    uint16_t psm;
    /** device address type */
    uint8_t address_type;
     /** device address */
    uint8_t address[6];
} NCP_TLV_PACK_END l2cap_disconnected_ev_t;

/** NCP Bluetooth LE l2cap data receive event ID */
#define L2CAP_EV_DATA_RECEIVED  0x83
/** NCP Bluetooth LE l2cap data receive event structure */
typedef NCP_TLV_PACK_START struct l2cap_data_received_ev_tag {
    /** channel ID */
    uint8_t chan_id;
    /** receive data length */
    uint16_t data_length;
    /** receive data */
    uint8_t data[];
} NCP_TLV_PACK_END l2cap_data_received_ev_t;

/** NCP Bluetooth LE l2cap reconfigure event ID */
#define L2CAP_EV_RECONFIGURED   0x84
/** NCP Bluetooth LE l2cap reconfigure event structure */
typedef NCP_TLV_PACK_START struct l2cap_reconfigured_ev_tag {
    /** channel ID */
    uint8_t chan_id;
    /** remote maximum transmission unit */
    uint16_t mtu_remote;
    /** remote maximum payload size */
    uint16_t mps_remote;
    /** local maximum transmission unit */
    uint16_t mtu_local;
    /** local maximum payload size */
    uint16_t mps_local;
} NCP_TLV_PACK_END l2cap_reconfigured_ev_t;

/** GATT server context */
#define SERVER_MAX_SERVICES     10
#define SERVER_MAX_ATTRIBUTES   50
#define SERVER_MAX_UUID_LEN     16
#define SERVER_BUF_SIZE         2048

/** NCP Bluetooth LE gatt read support command ID */
#define GATT_READ_SUPPORTED_COMMANDS    0x01
/** NCP Bluetooth LE gatt read support command response structure */
typedef NCP_TLV_PACK_START struct gatt_read_supported_commands_rp {
    /** response data */
    uint8_t data;
} NCP_TLV_PACK_END gatt_read_supported_commands_rp_t;

/** NCP Bluetooth LE service type */
#define GATT_SERVICE_PRIMARY        0x00
#define GATT_SERVICE_SECONDARY      0x01

/** NCP Bluetooth LE GATT add service ID */
#define GATT_ADD_SERVICE        0x02
/** NCP Bluetooth LE GATT add service command structure */
typedef NCP_TLV_PACK_START struct gatt_add_service_cmd {
    /** tlv type header */
    TypeHeader_t header;
    /** service type */
    uint8_t type;
    /** service length */
    uint8_t uuid_length;
    /** service uuid */
    uint8_t uuid[SERVER_MAX_UUID_LEN];
} NCP_TLV_PACK_END gatt_add_service_cmd_t;
/** NCP Bluetooth LE GATT add service response structure */
typedef NCP_TLV_PACK_START struct gatt_add_service_rp {
    /** service id */
    uint16_t svc_id;
} NCP_TLV_PACK_END gatt_add_service_rp_t;

/** NCP Bluetooth LE GATT add characteristic ID */
#define GATT_ADD_CHARACTERISTIC     0x03
/** NCP Bluetooth LE GATT add characteristic command structure */
typedef NCP_TLV_PACK_START struct gatt_add_characteristic_cmd {
    /** tlv type header */
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
/** NCP Bluetooth LE GATT add characteristic response structure */
typedef NCP_TLV_PACK_START struct gatt_add_characteristic_rp {
    /** characteristic ID */
    uint16_t char_id;
} NCP_TLV_PACK_END gatt_add_characteristic_rp_t;

/** NCP Bluetooth LE GATT add descriptor ID */
#define GATT_ADD_DESCRIPTOR     0x04
/** NCP Bluetooth LE GATT add descriptor command structure */
typedef NCP_TLV_PACK_START struct gatt_add_descriptor_cmd {
    /** tlv type header */
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
/** NCP Bluetooth LE GATT add descriptor response structure */
typedef NCP_TLV_PACK_START struct gatt_add_descriptor_rp {
    /** descriptor id */
    uint16_t desc_id;
} NCP_TLV_PACK_END gatt_add_descriptor_rp_t;

/** NCP Bluetooth LE GATT add include service ID */
#define GATT_ADD_INCLUDED_SERVICE   0x05
/** NCP Bluetooth LE GATT add include service command structure */
typedef NCP_TLV_PACK_START struct gatt_add_included_service_cmd {
    /** tlv type header */
    TypeHeader_t header;
    /** service uuid */
    uint16_t svc_id;
} NCP_TLV_PACK_END gatt_add_included_service_cmd_t;
/** NCP Bluetooth LE GATT add include service response structure */
typedef NCP_TLV_PACK_START struct gatt_add_included_service_rp {
    /** include service ID */
    uint16_t included_service_id;
} NCP_TLV_PACK_END gatt_add_included_service_rp_t;

/** NCP Bluetooth LE GATT start service command structure */
typedef NCP_TLV_PACK_START struct gatt_start_service_cmd {
    /** tlv type header */
    TypeHeader_t header;
    /** service started status
     * 0: unstart
     * 1: start
     */
    uint8_t started;
} NCP_TLV_PACK_END gatt_start_service_cmd_t;

/** NCP Bluetooth LE GATT add service command structure */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SERVICE_ADD
{
    /** tlv buf length */
    uint32_t tlv_buf_len;
    /**
     * add service TLV, gatt_add_service_cmd_t
     * add characteristic TLV, gatt_add_characteristic_cmd_t
     * add descriptor TLV, gatt_add_descriptor_cmd_t
     * add include service TLV, gatt_add_included_service_cmd_t (to be added in the future)
     * start host servuce TLV, gatt_start_service_cmd_t
     */
    uint8_t tlv_buf[1];
} NCP_TLV_PACK_END NCP_CMD_SERVICE_ADD;

/** NCP Bluetooth LE GATT start service command structure */
typedef NCP_TLV_PACK_START struct _NCP_CMD_START_SERVICE
{
    /** host status */
    uint8_t form_host;
    /** service ID 
     * 4: Central_HTC
     * 5: Central_HRC
    */
    uint8_t svc_id;
} NCP_TLV_PACK_END NCP_CMD_START_SERVICE;

/** NCP Bluetooth LE GATT set value ID */
#define GATT_SET_VALUE          0x06
/** NCP Bluetooth LE GATT set value command structure */
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

/** NCP Bluetooth LE GATT start server ID */
#define GATT_START_SERVER       0x07
/** NCP Bluetooth LE GATT start server response structure */
typedef NCP_TLV_PACK_START struct gatt_start_server_rp {
    /** database attribute offset */
    uint16_t db_attr_off;
    /** database attribute count */
    uint8_t db_attr_cnt;
} NCP_TLV_PACK_END gatt_start_server_rp_t;

/** NCP Bluetooth LE GATT reset server ID */
#define GATT_RESET_SERVER       0x08
/** NCP Bluetooth LE GATT set encryption key size ID */
#define GATT_SET_ENC_KEY_SIZE       0x09
/** NCP Bluetooth LE GATT set encryption key size command structure */
typedef NCP_TLV_PACK_START struct gatt_set_enc_key_size_cmd {
    /** attribute id */
    uint16_t attr_id;
    /** encryption key size */
    uint8_t key_size;
} NCP_TLV_PACK_END gatt_set_enc_key_size_cmd_t;

/** NCP Bluetooth LE gatt service type structure */
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

/** NCP Bluetooth LE gatt included type structure */
typedef NCP_TLV_PACK_START struct gatt_included {
    /** service include uuid */
    uint16_t included_handle;
    /** service handle */
    struct gatt_service service;
} NCP_TLV_PACK_END gatt_included_t;

/** NCP Bluetooth LE gatt characteristic type strcuture */
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

/** NCP Bluetooth LE gatt descriptor type structure */
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
/** NCP Bluetooth LE exchange maximum transimission unit command structure */
typedef NCP_TLV_PACK_START struct gatt_exchange_mtu_cmd {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
} NCP_TLV_PACK_END gatt_exchange_mtu_cmd_t;

/** NCP Bluetooth LE discovery primary service ID */
#define GATT_DISC_ALL_PRIM      0x0b
/** NCP Bluetooth LE discovery primary service command structure */
typedef NCP_TLV_PACK_START struct gatt_disc_all_prim_cmd {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
} NCP_TLV_PACK_END gatt_disc_all_prim_cmd_t;
/** NCP Bluetooth LE discovery primary service response structure */
typedef NCP_TLV_PACK_START struct gatt_disc_all_prim_rp {
    /** discovered service number */
    uint8_t services_count;
    /** diecovered service */
    gatt_service_t services[];
} NCP_TLV_PACK_END gatt_disc_all_prim_rp_t;

/** NCP Bluetooth LE discovery primary uuid ID */
#define GATT_DISC_PRIM_UUID     0x0c
/** NCP Bluetooth LE discovery primary uuid command structure */
typedef NCP_TLV_PACK_START struct gatt_disc_prim_uuid_cmd {
    /** tlv type header */
    TypeHeader_t header;
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
    /** uuid length */
    uint8_t uuid_length;
    /** uuid data list */
    uint8_t uuid[];
} NCP_TLV_PACK_END gatt_disc_prim_uuid_cmd_t;
/** NCP Bluetooth LE discovery primary uuid response structure */
typedef NCP_TLV_PACK_START struct gatt_disc_prim_rp {
    /** discovered service number */
    uint8_t services_count;
    /** diecovered service */
    gatt_service_t services[];
} NCP_TLV_PACK_END gatt_disc_prim_rp_t;

/** NCP Bluetooth LE gatt find included ID */
#define GATT_FIND_INCLUDED      0x0d
/** NCP Bluetooth LE gatt find included command structure */
typedef NCP_TLV_PACK_START struct gatt_find_included_cmd {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
    /** service start handle */
    uint16_t start_handle;
    /** service end handle */
    uint16_t end_handle;
} NCP_TLV_PACK_END gatt_find_included_cmd_t;
/** NCP Bluetooth LE gatt find included response structure */
typedef NCP_TLV_PACK_START struct gatt_find_included_rp {
    /** discovered service number */
    uint8_t services_count;
    /** diecovered included service */
    gatt_included_t included[];
} NCP_TLV_PACK_END gatt_find_included_rp_t;

/** NCP Bluetooth LE gatt discovery characteristic ID */
#define GATT_DISC_ALL_CHRC      0x0e
/** NCP Bluetooth LE gatt discovery characteristic command structure */
typedef NCP_TLV_PACK_START struct gatt_disc_all_chrc_cmd {
    /** tlv type header */
    TypeHeader_t header;
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
    /** characteristic start handle */
    uint16_t start_handle;
    /** characteristic end handle */
    uint16_t end_handle;
} NCP_TLV_PACK_END gatt_disc_all_chrc_cmd_t;
/** NCP Bluetooth LE gatt discovery characteristic response structure */
typedef NCP_TLV_PACK_START struct gatt_disc_chrc_rp {
    /** discovered characteristic number */
    uint8_t characteristics_count;
    /** discovered characteristic */
    gatt_characteristic_t characteristics[];
} NCP_TLV_PACK_END gatt_disc_chrc_rp_t;

/** NCP Bluetooth LE gatt discovery characteristic uuid ID */
#define GATT_DISC_CHRC_UUID     0x0f
/** NCP Bluetooth LE gatt discovery characteristic uuid command structure */
typedef NCP_TLV_PACK_START struct gatt_disc_chrc_uuid_cmd {
    /** tlv type header */
    TypeHeader_t header;
    /** device address type */
    uint8_t address_type;
    /** device address */
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
/** NCP Bluetooth LE gatt discovery descriptor command structure */
typedef NCP_TLV_PACK_START struct gatt_disc_all_desc_cmd {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
    /** descriptor start handle */
    uint16_t start_handle;
    /** descriptor end handle */
    uint16_t end_handle;
} NCP_TLV_PACK_END gatt_disc_all_desc_cmd_t;
/** NCP Bluetooth LE gatt discovery descriptor response structure */
typedef NCP_TLV_PACK_START struct gatt_disc_all_desc_rp {
    /** discovered descriptor number */
    uint8_t descriptors_count;
    /** discovered descriptor */
    gatt_descriptor_t descriptors[];
} NCP_TLV_PACK_END gatt_disc_all_desc_rp_t;

/** NCP Bluetooth LE gatt discovery descriptor uuid ID */
#define GATT_DISC_DESC_UUID     0x21
/** NCP Bluetooth LE gatt discovery descriptor uuid command structure */
typedef NCP_TLV_PACK_START struct gatt_disc_desc_uuid_cmd {
    /** tlv type header */
    TypeHeader_t header;
    /** device address type */
    uint8_t address_type;
    /** device address */
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

/** NCP Bluetooth LE service discovery type struct */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SERVICE_DISC
{
    /** tlv buf length */
    uint32_t tlv_buf_len;
    /**
     * discover primary service TLV, gatt_disc_prim_uuid_cmd_t
     * discover characteristic TLV, gatt_disc_chrc_uuid_cmd_t
     * discover descriptor TLV, gatt_disc_desc_uuid_cmd_t
     */
    uint8_t tlv_buf[1];
} NCP_TLV_PACK_END NCP_CMD_SERVICE_DISC;

/** NCP Bluetooth LE gatt read ID */
#define GATT_READ           0x11
/** NCP Bluetooth LE gatt read command structure */
typedef NCP_TLV_PACK_START struct gatt_read_cmd {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
    /** handle uuid */
    uint16_t handle;
} NCP_TLV_PACK_END gatt_read_cmd_t;
/** NCP Bluetooth LE gatt read response structure */
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
/** NCP Bluetooth LE gatt read uuid command structure */
typedef NCP_TLV_PACK_START struct gatt_read_uuid_cmd {
    /** device address type */
    uint8_t address_type;
    /** device address */
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
/** NCP Bluetooth LE gatt characteristic value structure */
typedef NCP_TLV_PACK_START struct gatt_char_value {
    /** characteristic handle */
    uint16_t handle;
    /** data length */
    uint8_t data_len;
    /** data */
    uint8_t data[SERVER_MAX_UUID_LEN];
} NCP_TLV_PACK_END gatt_char_value_t;
/** NCP Bluetooth LE gatt read uuid response structure */
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
/** NCP Bluetooth LE gatt read long command structure */
typedef NCP_TLV_PACK_START struct gatt_read_long_cmd {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
    /** handle uuid */
    uint16_t handle;
    /** handle uuid offset */
    uint16_t offset;
} NCP_TLV_PACK_END gatt_read_long_cmd_t;
/** NCP Bluetooth LE gatt read long response structure */
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
/** NCP Bluetooth LE gatt read multiple command structure */
typedef NCP_TLV_PACK_START struct gatt_read_multiple_cmd {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
    /** handle uuid number */
    uint8_t handles_count;
    /** handle uuid list */
    uint16_t handles[];
} NCP_TLV_PACK_END gatt_read_multiple_cmd_t;
/** NCP Bluetooth LE gatt read multiple response structure */
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
/** NCP Bluetooth LE gatt write without response command structure */
typedef NCP_TLV_PACK_START struct gatt_write_without_rsp_cmd {
    /** device address type */
    uint8_t address_type;
    /** device address */
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
/** NCP Bluetooth LE gatt signed write without response command structure */
typedef NCP_TLV_PACK_START struct gatt_signed_write_without_rsp_cmd {
    /** device address type */
    uint8_t address_type;
    /** device address */
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
/** NCP Bluetooth LE gatt write ID command structure */
typedef NCP_TLV_PACK_START struct gatt_write_cmd {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
    /** handle uuid */
    uint16_t handle;
    /** attribute data length */
    uint16_t data_length;
    /** attribute data */
    uint8_t data[];
} NCP_TLV_PACK_END gatt_write_cmd_t;
/** NCP Bluetooth LE gatt write response type structure */
typedef NCP_TLV_PACK_START struct gatt_write_rp {
    /** attribute response */
    uint8_t att_response;
} NCP_TLV_PACK_END gatt_write_rp_t;

/** NCP Bluetooth LE gatt write long ID */
#define GATT_WRITE_LONG         0x18
/** NCP Bluetooth LE gatt write long command structure */
typedef NCP_TLV_PACK_START struct gatt_write_long_cmd {
    /** device address type */
    uint8_t address_type;
    /** device address */
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
/** NCP Bluetooth LE gatt write long response structure */
typedef NCP_TLV_PACK_START struct gatt_write_long_rp {
    /** attribute response */
    uint8_t att_response;
} NCP_TLV_PACK_END gatt_write_long_rp_t;

/** NCP Bluetooth LE gatt write reliable ID */
#define GATT_RELIABLE_WRITE     0x19
/** NCP Bluetooth LE gatt write reliable command structure */
typedef NCP_TLV_PACK_START struct gatt_reliable_write_cmd {
    /** device address type */
    uint8_t address_type;
    /** device address */
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
/** NCP Bluetooth LE gatt write reliable response structure */
typedef NCP_TLV_PACK_START struct gatt_reliable_write_rp {
    /** attribute response */
    uint8_t att_response;
} NCP_TLV_PACK_END gatt_reliable_write_rp_t;

/** NCP Bluetooth LE GATT config */
#define GATT_CFG_NOTIFY         0x1a
#define GATT_CFG_INDICATE       0x1b

/** NCP Bluetooth LE gatt config notify command structure */
typedef NCP_TLV_PACK_START struct gatt_cfg_notify_cmd {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
    /** enable characteristic notification
     * 0: disable
     * 1: enable
     */
    uint8_t enable;
    /** chracteristic configure change handle */
    uint16_t ccc_handle;
} NCP_TLV_PACK_END gatt_cfg_notify_cmd_t;

/** NCP Bluetooth LE GATT get attribute ID */
#define GATT_GET_ATTRIBUTES     0x1c
/** NCP Bluetooth LE GATT get attribute command structure */
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
/** NCP Bluetooth LE GATT get attribute response structure */
typedef NCP_TLV_PACK_START struct gatt_get_attributes_rp {
    /** attribute count number */
    uint8_t attrs_count;
    /** attribute list */
    uint8_t attrs[];
} NCP_TLV_PACK_END gatt_get_attributes_rp_t;
/** NCP Bluetooth LE GATT attribute type structure */
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

/** NCP Bluetooth LE GATT get attribute value ID */
#define GATT_GET_ATTRIBUTE_VALUE    0x1d
/** NCP Bluetooth LE GATT get attribute value command structure */
typedef NCP_TLV_PACK_START struct gatt_get_attribute_value_cmd {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
    /** attribute handle */
    uint16_t handle;
} NCP_TLV_PACK_END gatt_get_attribute_value_cmd_t;
/** NCP Bluetooth LE GATT get attribute value command structure */
typedef NCP_TLV_PACK_START struct gatt_get_attribute_value_rp {
    /** attribute response */
    uint8_t att_response;
    /** attribute value length */
    uint16_t value_length;
    /** attribute value */
    uint8_t value[];
} NCP_TLV_PACK_END gatt_get_attribute_value_rp_t;
/** NCP Bluetooth LE GATT change DB ID */
#define GATT_CHANGE_DB          0x1e
/** NCP Bluetooth LE GATT change DB command structure */
typedef NCP_TLV_PACK_START struct gatt_change_db_cmd {
    /** start handle uuid */
    uint16_t start_handle;
    /** visibility setting */
    uint8_t visibility;
} NCP_TLV_PACK_END gatt_change_db_cmd_t;

/** NCP Bluetooth LE GATT eatt connect ID */
#define GATT_EATT_CONNECT		0x1f
/** NCP Bluetooth LE GATT eatt connect command structure */
struct gatt_eatt_connect_cmd {
    /** device address type */
	uint8_t address_type;
    /** device address */
	uint8_t address[6];
    /** channel number */
	uint8_t num_channels;
} NCP_TLV_PACK_END NCP_TLV_PACK_START;

/** NCP Bluetooth LE GATT read multiple variable ID */
#define GATT_READ_MULTIPLE_VAR      0x20

/** NCP Bluetooth LE GATT notify multiple ID */
#define GATT_NOTIFY_MULTIPLE        0x21
/** NCP Bluetooth LE GATT notify multiple command structure */
typedef NCP_TLV_PACK_START struct gatt_cfg_notify_mult_cmd {
    /** device address type */
    uint8_t address_type;
    /** device address */
    uint8_t address[6];
    /** attribute number */
    uint16_t cnt;
    /** attribute id list */
    uint16_t attr_id[];
} NCP_TLV_PACK_END gatt_cfg_notify_mult_cmd_t;

/** NCP Bluetooth LE add service ID */
#define GATT_NCP_BLE_ADD_SERVICE    0x22

/** NCP Bluetooth LE add service command structure */
typedef NCP_TLV_PACK_START struct gatt_ncp_ble_add_service_cmd
{
    /** service length */
    uint8_t svc_length;
    /** service add list */
    uint8_t svc[];
} NCP_TLV_PACK_END gatt_ncp_ble_add_service_cmd_t;

/** NCP Bluetooth LE gatt notification event */
#define GATT_EV_NOTIFICATION        0x80
/** NCP Bluetooth LE gatt notification event structure */
typedef NCP_TLV_PACK_START struct gatt_notification_ev {
    /** service id */
    uint8_t svc_id;
    /** device address type */
    uint8_t address_type;
    /** device address */
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
/** NCP Bluetooth LE gatt attribute value change event structure */
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
/** NCP Bluetooth LE gatt client characteristic configuration declaration event structure */
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
/** NCP Bluetooth LE gatt subscription event structure */
typedef NCP_TLV_PACK_START struct gatt_ncp_ble_svc_subscription_ev {
    /** service id */
    uint8_t svc_id;
    /** subscription status */
    uint8_t status;
} NCP_TLV_PACK_END gatt_ncp_ble_svc_subscription_ev_t;

/** NCP Bluetooth LE gatt add service response structure */
typedef NCP_TLV_PACK_START struct gatt_ncp_ble_add_service_rp {
    /** service length */
    uint8_t svc_length;
    /** service status */
    uint8_t status[];
} NCP_TLV_PACK_END gatt_ncp_ble_add_service_rp_t;

/** NCP Bluetooth LE L2cap connect event ID */
#define L2CAP_EV_CONNECT        0x81
/** NCP Bluetooth LE L2cap connect event structure */
typedef NCP_TLV_PACK_START struct l2cap_connect_ev {
    /** address type */
    uint8_t address_type;
    /** address value */
    uint8_t address[6];
    /** PSM value */
    uint16_t psm;
} NCP_TLV_PACK_END l2cap_connect_ev_t;

/** NCP Bluetooth LE L2cap disconnect event ID */
#define L2CAP_EV_DISCONNECT        0x82
/** NCP Bluetooth LE L2cap disconnect event structure */
typedef NCP_TLV_PACK_START struct l2cap_disconnect_ev {
    /** address type */
    uint8_t address_type;
    /** address value */
    uint8_t address[6];
    /** PSM value */
    uint16_t psm;
} NCP_TLV_PACK_END l2cap_disconnect_ev_t;

/** NCP Bluetooth LE L2cap receive event ID */
#define L2CAP_EV_RECEIVE        0x83
/** NCP Bluetooth LE L2cap receive event structure */
typedef NCP_TLV_PACK_START struct l2cap_reveive_ev {
    /** address type */
    uint8_t address_type;
    /** address value */
    uint8_t address[6];
    /** PSM value */
    uint16_t psm;
    /** receive data length */
    uint8_t len;
    /** receive data value */
    uint8_t data[256];
} NCP_TLV_PACK_END l2cap_reveive_ev_t;

/** NCP Bluetooth LE advertising start structure */
typedef NCP_TLV_PACK_START struct _NCP_CMD_ADV_START
{
    /** advertising start value */
    uint8_t data[256];
} NCP_TLV_PACK_END NCP_CMD_ADV_START;

/** NCP Bluetooth LE set advertising data structure */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SET_ADV_DATA
{
    /** advertising data length */
    uint8_t adv_length;
    /** advertising data value */
    uint8_t adv_data[];
} NCP_TLV_PACK_END NCP_CMD_SET_ADV_DATA;

/** NCP Bluetooth LE set scan parameter structure */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SET_SCAN_PARAM
{
    /** bit-field of scanning options. */
    uint32_t options;
    /** scan interval (N * 0.625 ms) */
    uint16_t interval;
    /** scan window (N * 0.625 ms) */
    uint16_t window;
} NCP_TLV_PACK_END NCP_CMD_SET_SCAN_PARAM;

/** NCP Bluetooth LE start scan structure */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SCAN_START
{
    /** scan start */
    uint8_t type;
} NCP_TLV_PACK_END NCP_CMD_SCAN_START;

/** NCP Bluetooth LE create connect structure */
typedef NCP_TLV_PACK_START struct _NCP_CMD_CONNECT
{
    /** remote address type */
    uint8_t type;
    /** remote address*/
    uint8_t addr[6];
} NCP_TLV_PACK_END NCP_CMD_CONNECT;

/** NCP Bluetooth LE set data length command structure */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SET_DATA_LEN
{
    /** remote address type*/
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

/** NCP Bluetooth LE set phy command structure */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SET_PHY
{
    /** remote address type*/
    uint8_t  address_type;
    /** remote address */
    uint8_t  address[6];
    /** set phy option */
    uint16_t options;
    /** preferer transmit phy */
    uint8_t  pref_tx_phy;
    /** preferer receive phy */
    uint8_t  pref_rx_phy;
} NCP_TLV_PACK_END NCP_CMD_SET_PHY;

/** NCP Bluetooth LE update connection parameter command structure */
typedef NCP_TLV_PACK_START struct _NCP_CMD_CONN_PARA_UPDATE
{
    /** remote address type*/
    uint8_t type;
    /** remote address */
    uint8_t addr[6];
    /** connection min interval */
    uint16_t interval_min;
    /** connection max interval */
    uint16_t interval_max;
    /** connection latency */
    uint16_t latency;
    /** connection timeout */
    uint16_t timeout;
} NCP_TLV_PACK_END NCP_CMD_CONN_PARA_UPDATE;

/** NCP Bluetooth LE update connection parameter event structure */
typedef NCP_TLV_PACK_START struct _NCP_CMD_CONN_PARA_UPDATE_EV
{
    /** remote address type*/
    uint8_t type;
    /** remote address */
    uint8_t addr[6];
    /** connection interval */
    uint16_t interval;
    /** connection latency */
    uint16_t latency;
    /** connection timeout */
    uint16_t timeout;
} NCP_TLV_PACK_END NCP_CMD_CONN_PARA_UPDATE_EV;

/** NCP Bluetooth LE set phy event structure */
typedef NCP_TLV_PACK_START struct _NCP_CMD_PHY_UPDATE_EV
{
    /** remote address type*/
    uint8_t address_type;
    /** remote address */
    uint8_t address[6];
    /** transmit phy */
    uint8_t tx_phy;
    /** receive phy */
    uint8_t rx_phy;
} NCP_TLV_PACK_END NCP_CMD_PHY_UPDATE_EV;

/** NCP Bluetooth LE set data length event structure */
typedef NCP_TLV_PACK_START struct _NCP_CMD_DATA_LEN_UPDATE_EV
{
    /** remote address type*/
    uint8_t address_type;
    /** remote address */
    uint8_t address[6];
    /** transmit max data length */
    uint16_t tx_max_len;
    /** transmit max time */
    uint16_t tx_max_time;
    /** receive max data length */
    uint16_t rx_max_len;
    /** receive max time */
    uint16_t rx_max_time;
} NCP_TLV_PACK_END NCP_CMD_DATA_LEN_UPDATE_EV;

/** NCP Bluetooth LE encription command structure */
typedef NCP_TLV_PACK_START struct _NCP_CMD_ENCRIPTION
{
    /** remote address type*/
    uint8_t type;
    /** remote address */
    uint8_t addr[6];
} NCP_TLV_PACK_END NCP_CMD_ENCRYPTION;

/** NCP Bluetooth LE set address command structure */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SET_ADDR
{
    /** address value */
    uint8_t addr[6];
} NCP_TLV_PACK_END NCP_CMD_SET_ADDR;

/** NCP Bluetooth LE set device name command structure */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SET_NAME
{
    /** device name */
    uint8_t name[33];
} NCP_TLV_PACK_END NCP_CMD_SET_NAME;

/** NCP Bluetooth LE set power mode command structure */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SET_POWER_MODE
{
    /** power mode */
    uint8_t mode;
} NCP_TLV_PACK_END NCP_CMD_SET_POWER_MODE;

/** NCP Bluetooth LE set value command structure */
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

/** NCP Bluetooth LE gatt read value command structure */
typedef NCP_TLV_PACK_START struct _NCP_GATT_READ_CMD {
    /** address type */
    uint8_t type;
    /** address value */
    uint8_t addr[6];
    /** gatt attribute handle */
    uint16_t handle;
} NCP_TLV_PACK_END NCP_GATT_READ_CMD;

/** NCP Bluetooth LE gatt discover primary service uuid structure */
typedef NCP_TLV_PACK_START struct _NCP_DISC_PRIM_UUID_CMD {
    /** address type */
    uint8_t address_type;
    /** address value */
    uint8_t address[NCP_BLE_ADDR_LENGTH];
    /** uuid length */
    uint8_t uuid_length;
    /** uuid value */
    uint8_t uuid[SERVER_MAX_UUID_LEN];
} NCP_TLV_PACK_END NCP_DISC_PRIM_UUID_CMD;

/** NCP Bluetooth LE gatt discover characteristic uuid structure */
typedef NCP_TLV_PACK_START struct _NCP_DISC_CHRC_UUID_CMD {
    /** address type */
    uint8_t address_type;
    /** address value */
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

/** NCP Bluetooth LE gatt subscript command structure */
typedef NCP_TLV_PACK_START struct _NCP_CFG_SUBCRIBE_CMD {
    /** address type */
    uint8_t address_type;
    /** address value */
    uint8_t address[NCP_BLE_ADDR_LENGTH];
    /** subscript value */
    uint8_t enable;
    /** ccc handle */
    uint16_t ccc_handle;
} NCP_TLV_PACK_END NCP_CFG_SUBCRIBE_CMD;

/** NCP Bluetooth LE gatt register service structure */
typedef NCP_TLV_PACK_START struct _NCP_REGISTER_SERVICE
{
    /** service length */
    uint8_t svc_length;
    /** service id */
    uint8_t service[MAX_SUPPORT_SERVICE];
} NCP_TLV_PACK_END NCP_REGISTER_SERVICE;

/** NCP Bluetooth LE l2cap connect command structure */
typedef NCP_TLV_PACK_START struct _NCP_L2CAP_CONNECT_CMD {
    /** remote address type */
    uint8_t address_type;
    /** remote address value */
    uint8_t address[NCP_BLE_ADDR_LENGTH];
    /** PSM value */
    uint16_t psm;
    /** security */
    uint8_t sec;
    /** security flag*/
    uint8_t sec_flag;
} NCP_TLV_PACK_END NCP_L2CAP_CONNECT_CMD;

/** NCP Bluetooth LE l2cap disconnect command structure */
typedef NCP_TLV_PACK_START struct _NCP_L2CAP_DISCONNECT_CMD {
    /** remote address type */
    uint8_t address_type;
    /** remote address value */
    uint8_t address[NCP_BLE_ADDR_LENGTH];
} NCP_TLV_PACK_END NCP_L2CAP_DISCONNECT_CMD;

/** NCP Bluetooth LE l2cap send data structure */
typedef NCP_TLV_PACK_START struct _NCP_L2CAP_SEND_CMD {
    /** remote address type */
    uint8_t address_type;
    /** remote address value */
    uint8_t address[NCP_BLE_ADDR_LENGTH];
    /** send data times */
    uint16_t times;
} NCP_TLV_PACK_END NCP_L2CAP_SEND_CMD;

/** NCP Bluetooth LE l2cap register PSM structure */
typedef NCP_TLV_PACK_START struct _NCP_L2CAP_REGISTER_CMD {
    /** PSM value */
    uint16_t psm;
    /** security level */
    uint8_t sec_level;
    /** security flag */
    uint8_t sec_flag;
    /** policy */
    uint8_t policy;
    /** policy flag */
    uint8_t policy_flag;
} NCP_TLV_PACK_END NCP_L2CAP_REGISTER_CMD;

/** NCP Bluetooth LE l2cap set metrics command structure */
typedef NCP_TLV_PACK_START struct _NCP_L2CAP_METRICS_CMD {
    /** metrics flag */
    bool metrics_flag;
} NCP_TLV_PACK_END NCP_L2CAP_METRICS_CMD;

/** NCP Bluetooth LE l2cap receive data structure */
typedef NCP_TLV_PACK_START struct _NCP_L2CAP_RECEIVE_CMD {
    /** receive data delay */
    uint32_t l2cap_recv_delay_ms;
} NCP_TLV_PACK_END NCP_L2CAP_RECEIVE_CMD;

/** NCP Bluetooth LE start advertising response structure */
typedef NCP_TLV_PACK_START struct _NCP_START_ADV_RP {
    /** advertising setting */
    uint32_t adv_setting;
} NCP_TLV_PACK_END NCP_START_ADV_RP;

/** NCP Bluetooth LE gatt add service response structure */
typedef NCP_TLV_PACK_START struct _NCP_ADD_SERVICE_RP {
    /** service attribute handle */
    uint16_t attr_handle;
} NCP_TLV_PACK_END NCP_ADD_SERVICE_RP;

/** NCP Bluetooth LE gatt service structure */
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

/** NCP Bluetooth LE gatt characteristic structure */
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

/** NCP Bluetooth LE gatt dscriptor structure */
typedef NCP_TLV_PACK_START struct GATT_DESCRIPTOR {
    /** descriptor handle */
    uint16_t descriptor_handle;
    /** descriptor uuid length */
    uint8_t uuid_length;
    /** descriptor uuid value */
    uint8_t uuid[SERVER_MAX_UUID_LEN];
} NCP_TLV_PACK_END GATT_DESCRIPTOR_T;

/** NCP Bluetooth LE gatt discover primary service response structure */
typedef NCP_TLV_PACK_START struct _NCP_DISC_PRIM_RP {
    /** service count */
    uint8_t services_count;
    /** service infomation */
    GATT_SERVICE_T services[MAX_SUPPORT_SERVICE];
} NCP_TLV_PACK_END NCP_DISC_PRIM_RP, NCP_DISC_PRIM_EV;

/** NCP Bluetooth LE gatt discover characteristic response structure */
typedef NCP_TLV_PACK_START struct  _NCP_DISC_CHRC_RP {
    /** characteristic count */
    uint8_t characteristics_count;
    /** characteristic infomation */
    GATT_CHARACTERISTIC_T characteristics[MAX_SUPPORT_SERVICE];
} NCP_TLV_PACK_END NCP_DISC_CHRC_RP, NCP_DISC_CHRC_EV;

/** NCP Bluetooth LE gatt discover descriptors response structure */
typedef NCP_TLV_PACK_START struct  _NCP_DISC_ALL_DESC_RP {
    /** descriptors count */
    uint8_t descriptors_count;
    /** descriptors infomation */
    GATT_DESCRIPTOR_T descriptors[MAX_SUPPORT_SERVICE];
} NCP_TLV_PACK_END NCP_DISC_ALL_DESC_RP, NCP_DISC_ALL_DESC_EV;

/** NCP Bluetooth LE commission service response structure */
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

typedef NCP_TLV_PACK_START struct _NCP_DEVICE_ADV_REPORT_EV {
    /** address type */
    uint8_t  address_type;
    /** address value */
    uint8_t  address[6];
    /** advertising packet RSSI */
    int8_t   rssi;
    /** advertising flags */
    uint8_t  flags;
    /** EIR data length */
    uint16_t eir_data_len;
    /** EIR data value */
    uint8_t  eir_data[];
} NCP_TLV_PACK_END NCP_DEVICE_ADV_REPORT_EV;

/** NCP Bluetooth LE device connected event structure */
typedef NCP_TLV_PACK_START struct _NCP_DEVICE_CONNECTED_EV {
    /** remote address type */
    uint8_t address_type;
    /** remote address value */
    uint8_t address[6];
    /** connection interval */
    uint16_t interval;
    /** connection latency */
    uint16_t latency;
    /** connection timeout */
    uint16_t timeout;
} NCP_TLV_PACK_END NCP_DEVICE_CONNECTED_EV;

/** NCP Bluetooth LE device disconnected event structure */
typedef NCP_TLV_PACK_START struct _NCP_DEVICE_DISCONNECTED_EV {
    /** remote address type */
    uint8_t address_type;
    /** remote address value */
    uint8_t address[6];
} NCP_TLV_PACK_END NCP_DEVICE_DISCONNECTED_EV;

/** NCP Bluetooth LE passkey display event structure */
typedef NCP_TLV_PACK_START struct _NCP_PASSKEY_DISPLAY_EV {
    /** remote address type */
    uint8_t  address_type;
    /** remote address value */
    uint8_t  address[6];
    /** passkey value */
    uint32_t passkey;
} NCP_TLV_PACK_END NCP_PASSKEY_DISPLAY_EV;

/** NCP Bluetooth LE identity resolved event structure */
typedef NCP_TLV_PACK_START struct _NCP_IDENTITY_RESOLVED_EV {
    /** address type */
    uint8_t address_type;
    /** address value */
    uint8_t address[6];
    /** identity address type */
    uint8_t identity_address_type;
    /** identity address value */
    uint8_t identity_address[6];
} NCP_TLV_PACK_END NCP_IDENTITY_RESOLVED_EV;

/** NCP Bluetooth LE security level changed event structure */
typedef NCP_TLV_PACK_START struct _NCP_SEC_LEVEL_CHANGED_EV {
    /** address type */
    uint8_t address_type;
    /** address value */
    uint8_t address[6];
    /** security level */
    uint8_t sec_level;
} NCP_TLV_PACK_END NCP_SEC_LEVEL_CHANGED_EV;

/** NCP Bluetooth LE gatt max attribute value length */
#define MAX_ATTRIBUTE_VALUE_LEN 256

/** NCP Bluetooth LE gatt notification event structure */
typedef NCP_TLV_PACK_START struct  _NCP_NOTIFICATION_EV {
    /** service id */
    uint8_t svc_id;
    /** address type */
    uint8_t address_type;
    /** address value */
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

/** NCP Bluetooth LE gatt attribute value changed event structure */
typedef NCP_TLV_PACK_START struct  _NCP_ATTR_VALUE_CHANGED_EV {
    /** attribute handle */
    uint16_t handle;
    /** attribute data length */
    uint16_t data_length;
    /** attribute data value */
    uint8_t data[MAX_ATTRIBUTE_VALUE_LEN];
} NCP_TLV_PACK_END NCP_ATTR_VALUE_CHANGED_EV;

/** NCP Bluetooth LE gatt client characteristic configuration changed event structure */
typedef NCP_TLV_PACK_START struct  _NCP_CCC_CFG_CHANGED_EV {
    /** client characteristic configuration changed value */
    uint16_t ccc_value;
    /** uuid length */
    uint8_t uuid_length;
    /** uuid value */
    uint8_t uuid[SERVER_MAX_UUID_LEN];
} NCP_TLV_PACK_END NCP_CCC_CFG_CHANGED_EV;

/** NCP Bluetooth LE gatt subscription event structure */
typedef NCP_TLV_PACK_START struct  _NCP_SUBSCRIPTIONED_EV {
    /** service id */
    uint8_t svc_id;
    /** subscription status */
    uint8_t status;
} NCP_TLV_PACK_END NCP_SUBSCRIPTIONED_EV;

/** NCP Bluetooth LE l2cap connection event structure */
typedef NCP_TLV_PACK_START struct  _NCP_L2CAP_CONNECT_EV {
    /** address type */
    uint8_t address_type;
    /** address value */
    uint8_t address[6];
    /** PSM value */
    uint16_t psm;
} NCP_TLV_PACK_END NCP_L2CAP_CONNECT_EV;

/** NCP Bluetooth LE l2cap disconnection event structure */
typedef NCP_TLV_PACK_START struct  _NCP_L2CAP_DISCONNECT_EV {
    /** address type */
    uint8_t address_type;
    /** address value */
    uint8_t address[6];
    /** PSM value */
    uint16_t psm;
} NCP_TLV_PACK_END NCP_L2CAP_DISCONNECT_EV;

/** NCP Bluetooth LE l2cap receive data event structure */
typedef NCP_TLV_PACK_START struct  _NCP_L2CAP_RECEIVE_EV {
    /** address type */
    uint8_t address_type;
    /** address value */
    uint8_t address[6];
    /** PSM value */
    uint16_t psm;
    /** reveive data length */
    uint8_t len;
    /** receive data value */
    uint8_t data[256];
} NCP_TLV_PACK_END NCP_L2CAP_RECEIVE_EV;

/** NCP Bluetooth LE command structure */
typedef NCP_TLV_PACK_START struct _NCPCmd_DS_COMMAND
{
   /** Command Header : Command */
   NCP_COMMAND header;
   /** Command Body */
   union
   {
       /** NCP Bluetooth LE Adv */
        NCP_CMD_ADV_START adv_start;
        /** Set NCP Bluetooth LE Adv Data */
        NCP_CMD_SET_ADV_DATA set_adv_data;
        /** Set NCP Bluetooth LE Scan Parameter */
        NCP_CMD_SET_SCAN_PARAM set_scan_parameter;
        /** NCP Bluetooth LE Scan */
        NCP_CMD_SCAN_START scan_start;
        /** NCP Bluetooth LE Connect/Disconnect */
        NCP_CMD_CONNECT connect;
        /** Set NCP Bluetooth LE Data Len */
        NCP_CMD_SET_DATA_LEN set_data_len;
        /** Set NCP Bluetooth LE PHY */
        NCP_CMD_SET_PHY set_phy;
        /** NCP Bluetooth LE Connect Parameter Update */
        NCP_CMD_CONN_PARA_UPDATE conn_param_update;
        /** NCP Bluetooth LE Connect Encryption */
        NCP_CMD_ENCRYPTION conn_encryption;
        /** NCP Bluetooth LE Set Power Mode */
        NCP_CMD_SET_POWER_MODE set_pw_mode;
        /** NCP Bluetooth LE Set Device Address */
        NCP_CMD_SET_ADDR set_dev_addr;
        /** NCP Bluetooth LE Read characteristic */
        NCP_GATT_READ_CMD gatt_read_char;
        /** NCP Bluetooth LE Set Device Name */
        NCP_CMD_SET_NAME set_dev_name;
        /** NCP Bluetooth LE GATT Add Host Service */
        NCP_CMD_SERVICE_ADD host_svc_add;
        /** NCP Bluetooth LE Start Service at Host side */
        NCP_CMD_START_SERVICE host_start_svc;
        /** NCP Bluetooth LE GATT Register Service*/
        NCP_REGISTER_SERVICE register_service;
        /** NCP Bluetooth LE GATT Set Characteristic/Descriptor Service*/
        NCP_SET_VALUE_CMD gatt_set_value;
        /** NCP Bluetooth LE GATT Discover Primary Service*/
        NCP_DISC_PRIM_UUID_CMD discover_prim;
        /** NCP Bluetooth LE GATT Discover Characteristics*/
        NCP_DISC_CHRC_UUID_CMD discover_chrc;
        /** NCP Bluetooth LE GATT Configure service to indicate characteristic value to clinet*/
        NCP_CFG_SUBCRIBE_CMD cfg_subcribe;

        /** NCP Bluetooth LE L2CAP connect */
        NCP_L2CAP_CONNECT_CMD l2cap_connect;
        /** NCP Bluetooth LE L2CAP disconnect */
        NCP_L2CAP_DISCONNECT_CMD l2cap_disconnect;
        /** NCP Bluetooth LE L2CAP send */
        NCP_L2CAP_SEND_CMD l2cap_send;
        /** NCP Bluetooth LE L2CAP register*/
        NCP_L2CAP_REGISTER_CMD l2cap_register;
        /** NCP Bluetooth LE L2CAP metrics */
        NCP_L2CAP_METRICS_CMD l2cap_metrics;
        /** NCP Bluetooth LE L2CAP receive */
        NCP_L2CAP_RECEIVE_CMD l2cap_receive;
        
        /** NCP Bluetooth LE Adv reported event */
        NCP_DEVICE_ADV_REPORT_EV adv_reported;
        /** NCP Bluetooth LE Connected event */
        NCP_DEVICE_CONNECTED_EV device_connected;
        /** NCP Bluetooth LE Disonnected event */
        NCP_DEVICE_DISCONNECTED_EV device_disconnected;
        /** NCP Bluetooth LE Passkey Display event */
        NCP_PASSKEY_DISPLAY_EV passkey_display;
        /** NCP Bluetooth LE Remote Identity Address Resolved event */
        NCP_IDENTITY_RESOLVED_EV idenitiy_resolved;
        /** NCP Bluetooth LE Connect Parameter Update event */
        NCP_CMD_CONN_PARA_UPDATE_EV conn_param_update_ev;
        /** NCP Bluetooth LE Phy Update event */
        NCP_CMD_PHY_UPDATE_EV phy_updated_ev;
        /** NCP Bluetooth LE Data Len Update event */
        NCP_CMD_DATA_LEN_UPDATE_EV data_len_updated_ev;
        /** NCP Bluetooth LE Security Level Changed event */
        NCP_SEC_LEVEL_CHANGED_EV sec_level_changed;

        /** NCP Bluetooth LE GATT notification Receive even */
        NCP_NOTIFICATION_EV gatt_notification;
        /** NCP Bluetooth LE GATT Attribute Value Changed event */
        NCP_ATTR_VALUE_CHANGED_EV attr_value_changed;
        /** NCP Bluetooth LE GATT Client Characteristic Configuration Changed event */
        NCP_CCC_CFG_CHANGED_EV gatt_ccc_cfg_changed_ev;
        /** NCP Bluetooth LE GATT Client Subscription event */
        NCP_SUBSCRIPTIONED_EV gatt_subscription_ev;
        /** NCP Bluetooth LE GATT Discover Primary Service event */
        NCP_DISC_PRIM_EV gatt_disc_prim_ev;
        /** NCP Bluetooth LE GATT Discover Primary Service event */
        NCP_DISC_CHRC_EV gatt_disc_chrc_ev;
        /** NCP Bluetooth LE GATT Discover Primary Service event */
        NCP_DISC_ALL_DESC_EV gatt_disc_desc_ev;

        /** NCP Bluetooth LE l2cap connect event */
        NCP_L2CAP_CONNECT_EV l2cap_connect_ev;
        /** NCP Bluetooth LE l2cap disconnect event */
        NCP_L2CAP_DISCONNECT_EV l2cap_disconnect_ev;
        /** NCP Bluetooth LE l2cap receive event */
        NCP_L2CAP_RECEIVE_EV l2cap_receive_ev;
        
        /** NCP Bluetooth LE Adv start response */
        NCP_START_ADV_RP start_adv_rp;
        /** NCP Bluetooth LE GATT Add Service Attribute Response */
        NCP_ADD_SERVICE_RP add_service_rp;
        /** NCP Bluetooth LE GATT Discover Primary Service Response */
        NCP_DISC_PRIM_RP discover_prim_rp;
        /** NCP Bluetooth LE GATT Discover Characteristics Response */
        NCP_DISC_CHRC_RP discover_chrc_rp;
        /** NCP Bluetooth LE GATT Discover Descriptors Response */
        NCP_DISC_ALL_DESC_RP discover_desc_rp;
        /** NCP Bluetooth LE NCS SSID/Password receive Response */
        NCP_NCS_INFO_RP ncs_info_rp;
   } params;
} NCP_TLV_PACK_END NCPCmd_DS_COMMAND, MCU_NCPCmd_DS_COMMAND;

#endif /* __NCP_CMD_BLE_H__ */
