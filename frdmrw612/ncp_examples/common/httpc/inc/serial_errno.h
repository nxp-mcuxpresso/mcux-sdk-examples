/*
* Copyright 2024 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
* The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
*/

#ifndef _MWM_ERRNO_H_
#define _MWM_ERRNO_H_

/** Success */
#define E_SUCCESS		0
/** Failure */
#define E_FAIL			1
/** Command formatting error */
#define E_CMD_FORMAT_ERR	2
/** Invalid configuration state for device (configured variable) */
#define E_INVAL_STATE		3
/** Already exists */
#define E_EXISTS		4

/** Operation failure on device */
#define E_OPERATION_FAILED      100
/** Protocol version mismatch */
#define E_PROTO_VERSION		101
/** Signature mismatch */
#define E_SIG_MISMATCH		102

/** Invalid parameters */
#define E_INVAL_PARAM		103
/** IO error */
#define E_IO			104
/** Invalid response */
#define E_INVAL_RESP		105
/** Invalid URL */
#define E_INVAL_URL		106
/** No memory on device to do dynamic allocation */
#define E_NO_MEM                107
/** Invalid type for upgrade parameter list */
#define E_INVAL_TYPE            108
/** Maximum permissible limit is reached **/
#define E_MAX_LIMIT_REACHED       109
/** Value not found */
#define E_NOT_FOUND		110

/** Invalid state for sent command */
#define E_INVAL_CMD_STATE	120

/** WLAN parameter error */
#define E_WLAN_PARAM_ERR	130
/** WLAN state machine error */
#define E_WLAN_STATE_ERR	131
/** WLAN not started */
#define E_WLAN_NOT_STARTED	132

/** Unknown failure */
#define E_UNKNOWN		200
#endif

#ifdef ASCIICMD

/* upgrade error codes */
#define E_PREV_UPGRADE_IN_PROGRESS  201
#define E_INVALID_FS_LAYOUT         202
#define E_INVALID_FW_LAYOUT         203
#define E_RFGET_INVLEN              204
#define E_RFGET_FUPDATE             205
#define E_RFGET_FRD                 206
#define E_RFGET_FWR                 207
#define E_RFGET_FFW_LOAD            208
#define E_RFGET_INV_IMG             209
#define E_FILE_NOT_FOUND            210
#define E_TCP_CONNECT_FAIL          211
#define E_BAD_REQUEST               212
#define E_UPG_NOT_STARTED           213
#define E_UPG_STARTED               214
#define E_UPG_IN_PROGRESS           215
#define E_UPG_COMPLETED             216

/* mDNS error codes */
#define E_MDNS_QUERY_FAIL           301

/* wlan error codes */
#define E_WLAN_STAT_BASE            350
#define E_WLAN_STAT_END             361

#define E_WLAN_WPS_BASE             370
#define E_WLAN_WPS_END              375
/* wps error codes */
#define E_WPS_PB_FAIL               401
#define E_WPS_PIN_FAIL              402

/* httpc error codes */
/** TLS is not enabled int the configuration **/
#define E_HTTPC_TLS_NOT_ENABLED     501
#define E_HTTPC_MALFORMED_DATA      502
#define E_HTTPC_POST_CHUNKED_FAIL   503
#define E_HTTPC_WS_UPG_FAIL         504
#define E_HTTPC_RECV_SOCKET_ERROR   505
#define E_HTTPC_RECV_ERROR          506

/* generic error codes */
/** current mode (terse/verbose) is invalid **/
#define E_INVAL_MODE                601
/** parameter is read-only, cannot be modified **/
#define E_READ_ONLY                 602

/* socket error codes */
#define E_SOCK_ERROR                700
#define E_MALFORMED_DATA            701
#define E_SOCK_SHUTDOWN             702
#endif

#ifdef APPCONFIG_HK_SUPPORT
/* HAP error codes */
#define E_HK_OUTSTANDING_ACC        800
#define E_HK_INVALID_ACC_ID         801
#define E_HK_INVALID_IID            802

#endif

/** TLS error codes */
#define E_TLS_CONTEXT_CREATION_FAIL 900
