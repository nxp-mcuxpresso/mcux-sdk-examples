/*
 *	Copyright 2008-2024 NXP
 *	
 *	SPDX-License-Identifier: BSD-3-Clause	
 *
 */

/*!\file service.h
 *\brief Bluetooth service definitions.
 */
#ifndef __SERVICE_H_
#define __SERVICE_H_

#include <stdio.h>
#include <string.h>
#include <stddef.h>

/** NCP Bluetooth LE priority */
#define NCP_BLE_SERVICE_PRIO   0

/** NCP Bluetooth LE adv data type structure */
typedef struct adv_data {
	/** adv data length */
	uint8_t data_len;
	/** adv data type */
	uint8_t type;
	/** adv data */
	const uint8_t *data;
}adv_data_t;

/** NCP Bluetooth LE host gatt attribute type structure */
struct host_gatt_attr {
	/** NCP Bluetooth LE TLV type */
    uint8_t type;
	/** attribute properties */
    uint8_t properties;
	/** attribute permissions */
    uint16_t permissions;
	/** service uuid length */
    uint8_t uuid_length;
	/** service uuid */
    uint8_t uuid[SERVER_MAX_UUID_LEN];
};

/** NCP Bluetooth LE inline tool funciton */
#define BT_BYTES_LIST_LE16(_v)         \
	(((_v) >>  0) & 0xFFU),     \
	(((_v) >>  8) & 0xFFU)      \

#define HOST_GATT_ATTRIBUTE(_type, _uuid, _props, _perm) \
{   \
    .type = _type,  \
	.uuid = {BT_BYTES_LIST_LE16(_uuid)},    \
	.uuid_length = BT_UUID_SIZE_16, \
	.properties = _props,   \
	.permissions = _perm,   \
}

#define GATT_PRIMARY_SERVICE(_uuid)                  HOST_GATT_ATTRIBUTE(NCP_CMD_GATT_ADD_SERVICE_TLV, _uuid, 0, 0)
#define GATT_CHARACTERISTIC(_uuid, _props, _perm)    HOST_GATT_ATTRIBUTE(NCP_CMD_GATT_ADD_CHRC_TLV, _uuid, _props, _perm)
#define GATT_CCC(_perm)                              HOST_GATT_ATTRIBUTE(NCP_CMD_GATT_ADD_DESC_TLV, UUID_GATT_CCC, 0, _perm)

/** Auto register host services into ncp device database */
#define NCP_BLE_HOST_SERVICE_AUTO_START

/** Characteristic Properties Bit field values */

/** Bluetooth GATT Characteristic broadcast property.
 * If set, permits broadcasts of the Characteristic Value using Server
 * Characteristic Configuration Descriptor.
 */
#define BT_GATT_CHRC_BROADCAST			0x01
/**
 * Bluetooth GATT Characteristic read property.
 * If set, permits reads of the Characteristic Value.
 */
#define BT_GATT_CHRC_READ			0x02
/**
 * Bluetooth GATT Characteristic write without response property.
 * If set, permits write of the Characteristic Value without response.
 */
#define BT_GATT_CHRC_WRITE_WITHOUT_RESP		0x04
/**
 * Bluetooth GATT Characteristic write with response property.
 * If set, permits write of the Characteristic Value with response.
 */
#define BT_GATT_CHRC_WRITE			0x08
/**
 * Bluetooth GATT Characteristic notify property.
 * 
 * If set, permits notifications of a Characteristic Value without acknowledgment.
 */
#define BT_GATT_CHRC_NOTIFY			0x10
/**
 * Bluetooth GATT Characteristic indicate property.
 * 
 * If set, permits indications of a Characteristic Value with acknowledgment.
 */
#define BT_GATT_CHRC_INDICATE			0x20
/**
 * Bluetooth GATT Characteristic Authenticated Signed Writes property.
 * 
 * If set, permits signed writes to the Characteristic Value.
 */
#define BT_GATT_CHRC_AUTH			0x40
/**
 * Bluetooth GATT Characteristic Extended Properties.
 * 
 * 
 * If set, additional characteristic properties are defined in the
 * Characteristic Extended Properties Descriptor.
 */
#define BT_GATT_CHRC_EXT_PROP			0x80


/** GATT attribute permission bit field values */
/** No operations supported, e.g. for notify-only */
#define BT_GATT_PERM_NONE  0x0000

/** Attribute read permission. */
#define BT_GATT_PERM_READ 0x0001

/** Attribute write permission. */
#define BT_GATT_PERM_WRITE 0x0002

/**
 * Attribute read permission with encryption.
 * 
 * If set, requires encryption for read access.
 */
#define BT_GATT_PERM_READ_ENCRYPT 0x0004
/**
 * Attribute write permission with encryption.
 * 
 * If set, requires encryption for write access.
 */
#define BT_GATT_PERM_WRITE_ENCRYPT 0x0008

/**
 * Attribute read permission with authentication.
 * 
 * If set, requires encryption using authenticated link-key for read access.
 */
#define BT_GATT_PERM_READ_AUTHEN 0x0010
/**
 * Attribute write permission with authentication.
 * 
 * If set, requires encryption using authenticated link-key for write access.
 */
#define BT_GATT_PERM_WRITE_AUTHEN 0x0020
/**
 * Attribute prepare write permission.
 * 
 *  If set, allows prepare writes with use of BT_GATT_WRITE_FLAG_PREPARE
 *  passed to write callback.
 */
#define BT_GATT_PERM_PREPARE_WRITE 0x0040

/**
 * Attribute read permission with LE Secure Connection encryption.
 * 
 * If set, requires that LE Secure Connections is used for read access.
 */
#define BT_GATT_PERM_READ_LESC 0x0080

/**
 * Attribute write permission with LE Secure Connection encryption.
 * 
 * If set, requires that LE Secure Connections is used for write access.
 */
#define BT_GATT_PERM_WRITE_LESC 0x0100

/** Client Characteristic Configuration Values */

/**
 * Bluetooth GATT Client Characteristic Configuration notification.
 * 
 * If set, changes to Characteristic Value are notified.
 */
#define BT_GATT_CCC_NOTIFY			0x0001
/**
 * Bluetooth GATT Client Characteristic Configuration indication.
 * 
 * If set, changes to Characteristic Value are indicated.
 */
#define BT_GATT_CCC_INDICATE			0x0002

/** GATT Primary Service UUID */
#define UUID_GATT_PRIMARY 0x2800

/** GATT Client Characteristic Configuration UUID */
#define UUID_GATT_CCC 0x2902

/** Bluetooth GATT Client Characteristic Configuration values */

/**
 * Bluetooth GATT Client Characteristic Configuration notification
 * 
 * If set, changes to Characteristic Value are notified.
 */
#define BT_GATT_CCC_NOTIFY			0x0001
/**
 * Bluetooth GATT Client Characteristic Configuration indication.
 * 
 * If set, changes to Characteristic Value are indicated.
 */
#define BT_GATT_CCC_INDICATE			0x0002

/** EIR (Extended Inquiry Response)/AD (Advertising Data) data type definitions */
#define BT_DATA_FLAGS                   0x01 /* AD flags */
#define BT_DATA_UUID16_SOME             0x02 /* 16-bit UUID, more available */
#define BT_DATA_UUID16_ALL              0x03 /* 16-bit UUID, all listed */
#define BT_DATA_UUID32_SOME             0x04 /* 32-bit UUID, more available */
#define BT_DATA_UUID32_ALL              0x05 /* 32-bit UUID, all listed */
#define BT_DATA_UUID128_SOME            0x06 /* 128-bit UUID, more available */
#define BT_DATA_UUID128_ALL             0x07 /* 128-bit UUID, all listed */
#define BT_DATA_NAME_SHORTENED          0x08 /* Shortened name */
#define BT_DATA_NAME_COMPLETE           0x09 /* Complete name */
#define BT_DATA_TX_POWER                0x0a /* Tx Power */
#define BT_DATA_SM_TK_VALUE             0x10 /* Security Manager TK Value */
#define BT_DATA_SM_OOB_FLAGS            0x11 /* Security Manager OOB Flags */
#define BT_DATA_SOLICIT16               0x14 /* Solicit UUIDs, 16-bit */
#define BT_DATA_SOLICIT128              0x15 /* Solicit UUIDs, 128-bit */
#define BT_DATA_SVC_DATA16              0x16 /* Service data, 16-bit UUID */
#define BT_DATA_GAP_APPEARANCE          0x19 /* GAP appearance */
#define BT_DATA_LE_BT_DEVICE_ADDRESS    0x1b /* LE Bluetooth Device Address */
#define BT_DATA_LE_ROLE                 0x1c /* LE Role */
#define BT_DATA_SOLICIT32               0x1f /* Solicit UUIDs, 32-bit */
#define BT_DATA_SVC_DATA32              0x20 /* Service data, 32-bit UUID */
#define BT_DATA_SVC_DATA128             0x21 /* Service data, 128-bit UUID */
#define BT_DATA_LE_SC_CONFIRM_VALUE     0x22 /* LE SC Confirmation Value */
#define BT_DATA_LE_SC_RANDOM_VALUE      0x23 /* LE SC Random Value */
#define BT_DATA_URI                     0x24 /* URI */
#define BT_DATA_LE_SUPPORTED_FEATURES   0x27 /* LE Supported Features */
#define BT_DATA_CHANNEL_MAP_UPDATE_IND  0x28 /* Channel Map Update Indication */
#define BT_DATA_MESH_PROV               0x29 /* Mesh Provisioning PDU */
#define BT_DATA_MESH_MESSAGE            0x2a /* Mesh Networking PDU */
#define BT_DATA_MESH_BEACON             0x2b /* Mesh Beacon */
#define BT_DATA_BIG_INFO                0x2c /* BIGInfo */
#define BT_DATA_BROADCAST_CODE          0x2d /* Broadcast Code */
#define BT_DATA_CSIS_RSI                0x2e /* CSIS Random Set ID type */

/** device central profile service ID */
#define CENTRAL_HTC_SERVICE_ID         4
#define CENTRAL_HRC_SERVICE_ID         5

/**
 *  Put a 16-bit integer as little-endian to arbitrary location. \n
 *
 *  Put a 16-bit integer, originally in host endianness, to a
 *  potentially unaligned memory location in little-endian format.
 *
 * \param[in] val 16-bit integer in host endianness.
 * \param[in,out] dst Destination memory address to store the result.
 * 
 * \return void
 */
static inline void sys_put_le16(uint16_t val, uint8_t dst[2])
{
	dst[0] = (uint8_t)val;
	dst[1] = (uint8_t)(val >> 8);
}

/**
 *  Put a 32-bit integer as little-endian to arbitrary location. \n
 *
 *  Put a 32-bit integer, originally in host endianness, to a
 *  potentially unaligned memory location in little-endian format.
 *
 * \param[in] val 32-bit integer in host endianness.
 * \param[in,out] dst Destination memory address to store the result.
 * 
 * \return void
 */
static inline void sys_put_le32(uint32_t val, uint8_t dst[4])
{
	sys_put_le16((uint16_t)val, dst);
	sys_put_le16((uint16_t)(val >> 16), &dst[2]);
}

/**
 *  Get a 16-bit integer stored in little-endian format. \n
 *
 *  Get a 16-bit integer, stored in little-endian format in a potentially
 *  unaligned memory location, and convert it to the host endianness.
 *
 *  \param[in,out] src Location of the little-endian 16-bit integer to get.
 *
 *  \return 16-bit integer in host endianness.
 */
static inline uint16_t sys_get_le16(const uint8_t src[2])
{
	return ((uint16_t)src[1] << 8) | src[0];
}

/**
 *  Get a 32-bit integer stored in little-endian format. \n
 *
 *  Get a 32-bit integer, stored in little-endian format in a potentially
 *  unaligned memory location, and convert it to the host endianness.
 *
 *  \param[in,out] src Location of the little-endian 32-bit integer to get.
 *
 *  \return 32-bit integer in host endianness.
 */
static inline uint32_t sys_get_le32(const uint8_t src[4])
{
	return ((uint32_t)sys_get_le16(&src[2]) << 16) | sys_get_le16(&src[0]);
}
#endif /* __SERVICE_H_ */
