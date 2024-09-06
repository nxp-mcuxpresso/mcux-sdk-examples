/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_INCLUDE_BLUETOOTH_SERVICES_NCS_H_
#define ZEPHYR_INCLUDE_BLUETOOTH_SERVICES_NCS_H_
/**
 * @brief NXP Commision Service (NCS)
 * @defgroup bt_ncs NXP Commision Service (NCS)
 * @ingroup bluetooth
 * @{
 *
 * [Experimental] Users should note that the APIs can change
 * as a part of ongoing development.
 */

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Definitions
******************************************************************************/

#define ADV_NAME      "NCP NCS Demo"

#define COMMISSION_UUID \
	BT_UUID_DECLARE_128(0x00, 0x00, 0xAA, 0xFF, 0x1E, 0xEB, 0xA1, 0x5C, 0xEE, 0xF4, 0x5E, 0xBA, 0x02, 0x01, 0xFF, 0x01)
#define COMMISSION_UUID_SSID \
	BT_UUID_DECLARE_128(0x01, 0x00, 0xAA, 0xFF, 0x1E, 0xEB, 0xA1, 0x5C, 0xEE, 0xF4, 0x5E, 0xBA, 0x02, 0x01, 0xFF, 0x01)
#define COMMISSION_UUID_PSWD \
	BT_UUID_DECLARE_128(0x02, 0x00, 0xAA, 0xFF, 0x1E, 0xEB, 0xA1, 0x5C, 0xEE, 0xF4, 0x5E, 0xBA, 0x02, 0x01, 0xFF, 0x01)
#define COMMISSION_UUID_SECU \
	BT_UUID_DECLARE_128(0x03, 0x00, 0xAA, 0xFF, 0x1E, 0xEB, 0xA1, 0x5C, 0xEE, 0xF4, 0x5E, 0xBA, 0x02, 0x01, 0xFF, 0x01)
  
/*******************************************************************************
* Prototypes
******************************************************************************/

void peripheral_ncs_task(void *args);
void peripheral_ncs_connect(void *args);
void peripheral_ncs_disconnect(void *args);
void init_ncs_service(void);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* ZEPHYR_INCLUDE_BLUETOOTH_SERVICES_NCS_H_ */
