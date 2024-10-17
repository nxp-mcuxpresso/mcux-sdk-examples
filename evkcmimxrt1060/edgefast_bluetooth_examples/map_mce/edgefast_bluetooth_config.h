/*
 * Copyright 2021-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __BLE_CONFIG_H__
#define __BLE_CONFIG_H__

/*!
 * @brief Bluetooth Configuration
 * @defgroup bt_config Bluetooth Configuration
 * @{
 */

/* @brief Bluetooth Host stack
 */
#ifndef CONFIG_BT_HCI_HOST
    #define CONFIG_BT_HCI_HOST 1
#endif

/*! @brief buffer reserved length, suggested value is 8.*/
#ifndef CONFIG_BT_BUF_RESERVE
    #define CONFIG_BT_BUF_RESERVE 8
#endif

/*! @brief whether enable bt snoop feature, 0 - disable, 1 - enable.*/
#ifndef CONFIG_BT_SNOOP
    #define CONFIG_BT_SNOOP 0
#endif

/*! @brief Reserve buffer size for user.
 * Headroom that the driver needs for sending and receiving buffers. Add a
 * new 'default' entry for each new driver.
 */
#ifndef CONFIG_BT_HCI_RESERVE
    #define CONFIG_BT_HCI_RESERVE 4
#endif

/*! @brief HCI TX task stack size needed for executing bt_send
 * with specified driver, should be no less than 512.
 */
#ifndef CONFIG_BT_HCI_TX_STACK_SIZE
    #define CONFIG_BT_HCI_TX_STACK_SIZE 2048
#endif

/*! @brief HCI TX task priority.
 */
#ifndef CONFIG_BT_HCI_TX_PRIO
    #define CONFIG_BT_HCI_TX_PRIO 2
#endif

/*! @brief Size of the receiving thread stack.
 * This is the context from
 * which all event callbacks to the application occur. The
 * default value is sufficient for basic operation, but if the
 * application needs to do advanced things in its callbacks that
 * require extra stack space, this value can be increased to
 * accommodate for that.
 */
#ifndef CONFIG_BT_RX_STACK_SIZE

#if (defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0U))
    #define CONFIG_BT_RX_STACK_SIZE 2500
#else
    #define CONFIG_BT_RX_STACK_SIZE 1024
#endif
#endif

/*! @brief RX task priority.
 */
#ifndef CONFIG_BT_RX_PRIO
    #define CONFIG_BT_RX_PRIO 1
#endif

/*! @brief Peripheral Role support, if the macro is set to 0,feature is disabled, if 1, feature is enabled.
Select this for LE Peripheral role support.
 */
#ifndef CONFIG_BT_PERIPHERAL
    #define CONFIG_BT_PERIPHERAL 0
#endif

#if CONFIG_BT_PERIPHERAL
/*! @brief Broadcaster Role support, if the macro is set to 0,feature is disabled, if 1, feature is enabled.
 * Select this for LE Broadcaster role support.
 */
#ifndef CONFIG_BT_BROADCASTER
    #define CONFIG_BT_BROADCASTER 1
#endif
#endif

/*! @brief Central Role support, if the macro is set to 0,feature is disabled, if 1, feature is enabled.
 * Select this for LE Central role support.
 */
#ifndef CONFIG_BT_CENTRAL
    #define CONFIG_BT_CENTRAL 0
#endif

#if CONFIG_BT_CENTRAL
/*! @brief Observer Role support.
 * Select this for LE Observer role support.
 */
#ifndef CONFIG_BT_OBSERVER
    #define CONFIG_BT_OBSERVER 1
#endif
#endif /* CONFIG_BT_CENTRAL */

/*! @brief Extended Advertising and Scanning support [EXPERIMENTAL], if the macro is set to 0,feature is disabled, if 1, feature is enabled.
 * Select this to enable Extended Advertising API support.
 * This enables support for advertising with multiple advertising sets,
 * extended advertising data, and advertising on LE Coded PHY.
 * It enables support for receiving extended advertising data as a
 * scanner, including support for advertising data over the LE coded PHY.
 * It enables establishing connections over LE Coded PHY.
 */
#ifndef CONFIG_BT_EXT_ADV
    #define CONFIG_BT_EXT_ADV 0
#endif

/*! @brief Timeout for limited advertising in 1s units.
 * The default value is 30, and the valid range should be 1-180.
 */
#ifndef CONFIG_BT_LIM_ADV_TIMEOUT
    #define CONFIG_BT_LIM_ADV_TIMEOUT 30
#endif

/*! @brief Necessary user_data size for allowing packet fragmentation when
 *	  sending over HCI. See `struct tx_meta` in conn.c.
 *    The default value is 8, and default 16 if 64BIT.
 */
#ifndef CONFIG_BT_CONN_TX_USER_DATA_SIZE
    #define CONFIG_BT_CONN_TX_USER_DATA_SIZE 8
#endif

/*! @brief Maximum size of a fragmented periodic advertising report.
 * The default value is 0, and the valid range should be 0-1650.
 */
#ifndef CONFIG_BT_PER_ADV_SYNC_BUF_SIZE
    #define CONFIG_BT_PER_ADV_SYNC_BUF_SIZE 0
#endif

#if CONFIG_BT_EXT_ADV
/*! @brief Support starting advertising through legacy commands, if the macro is set to 0,feature is disabled, if 1, feature is enabled.
 * Select this to enable the use of the Legacy Advertising HCI commands.
 * This option should be used where the capabilities of the controller
 * is not known.
 * If this option is not enabled the controller must support the extended
 * advertising feature.
 */
#ifndef CONFIG_BT_EXT_ADV_LEGACY_SUPPORT
    #define CONFIG_BT_EXT_ADV_LEGACY_SUPPORT 1
#endif

/*! @brief Maximum number of simultaneous advertising sets, ranging from 1 to 64.
 * Maximum number of simultaneous Bluetooth advertising sets supported.
 */
#ifndef CONFIG_BT_EXT_ADV_MAX_ADV_SET
    #define CONFIG_BT_EXT_ADV_MAX_ADV_SET 1
#endif


/*! @brief Periodic Advertising and Scanning support [EXPERIMENTAL]
 *
 *  This allows the device to send advertising data periodically at deterministic
 *  intervals. Scanners can synchronize to the periodic advertisements
 *  to periodically get the data.
 */
#ifndef CONFIG_BT_PER_ADV
#define CONFIG_BT_PER_ADV 0
#endif

/*! @brief Periodic Advertising with Responses support [EXPERIMENTAL]
 *
 *  Select this to enable Periodic Advertising with Responses
 *  API support.
 */
#ifndef CONFIG_BT_PER_ADV_RSP
#define CONFIG_BT_PER_ADV_RSP 0
#endif

#if (defined(CONFIG_BT_PER_ADV_RSP) && (CONFIG_BT_PER_ADV_RSP > 0))
    #if !(defined(CONFIG_BT_PER_ADV) && (CONFIG_BT_PER_ADV > 0))
        #error CONFIG_BT_PER_ADV_RSP depends on CONFIG_BT_PER_ADV
    #endif
#endif

/*! @brief Periodic advertising sync support [EXPERIMENTAL]
 *
 *  Syncing with a periodic advertiser allows the device to periodically
 *  and deterministic receive data from that device in a connectionless
 *  manner.
 */
#ifndef CONFIG_BT_PER_ADV_SYNC
#if CONFIG_BT_OBSERVER
#define CONFIG_BT_PER_ADV_SYNC 1
#else
#define CONFIG_BT_PER_ADV_SYNC 0
#endif /* CONFIG_BT_OBSERVER */
#endif /* CONFIG_BT_PER_ADV_SYNC */

/*! @brief Periodic Advertising with Responses sync support [EXPERIMENTAL]
 *
 *  Select this to enable Periodic Advertising with Responses Sync
 *  API support.
 */
#ifndef CONFIG_BT_PER_ADV_SYNC_RSP
#define CONFIG_BT_PER_ADV_SYNC_RSP 0
#endif

#if (defined(CONFIG_BT_PER_ADV_SYNC_RSP) && (CONFIG_BT_PER_ADV_SYNC_RSP > 0))
    #if !(defined(CONFIG_BT_OBSERVER) && (CONFIG_BT_OBSERVER > 0))
        #error CONFIG_BT_PER_ADV_SYNC_RSP depends on CONFIG_BT_OBSERVER
    #endif
#endif

#if CONFIG_BT_PER_ADV_SYNC
/*! @brief Maximum number of simultaneous periodic advertising syncs, range 1 to 64
 *
 *  Maximum number of simultaneous periodic advertising syncs supported.
 */
#ifndef CONFIG_BT_PER_ADV_SYNC_MAX
#define CONFIG_BT_PER_ADV_SYNC_MAX 1
#endif /* CONFIG_BT_PER_ADV_SYNC_MAX */
#endif /* CONFIG_BT_PER_ADV_SYNC */

#endif /* CONFIG_BT_EXT_ADV */

#if CONFIG_BT_HCI_HOST

/*! @brief Enable filter accept list support.
 * This option enables the filter accept list API. This takes advantage of the
 * whitelisting feature of a BLE controller.
 * The filter accept list is a global list and the same filter accept list is used
 * by both scanner and advertiser. The filter accept list cannot be modified while
 * it is in use.
 * An Advertiser can filter accept list which peers can connect or request scan
 * response data.
 * A scanner can filter accept list advertiser for which it will generate
 * advertising reports.
 * Connections can be established automatically for filter accepted peers.
 *
 * This options deprecates the bt_le_set_auto_conn API in favor of the
 * bt_conn_create_aute_le API.
 */
#ifndef CONFIG_BT_FILTER_ACCEPT_LIST
    #define CONFIG_BT_FILTER_ACCEPT_LIST 0
#endif

/*! @brief Bluetooth device name.
 * Name can be up to 248 bytes long (excluding
 * NULL termination). Can be empty string.
 */
#ifndef CONFIG_BT_DEVICE_NAME
    #define CONFIG_BT_DEVICE_NAME "BLE_Peripheral"
#endif

/*! @brief Runtime Bluetooth Appearance changing
 * Enables use of bt_set_appearance.
 * If CONFIG_BT_SETTINGS is set, the appearance is persistently stored.
 */
#ifndef CONFIG_BT_DEVICE_APPEARANCE_DYNAMIC
    #define CONFIG_BT_DEVICE_APPEARANCE_DYNAMIC 0
#endif

/*! @brief Bluetooth device appearance
 * For the list of possible values please
 * consult the following link:
 * www.bluetooth.com/specifications/assigned-numbers
 */
#ifndef CONFIG_BT_DEVICE_APPEARANCE
    #define CONFIG_BT_DEVICE_APPEARANCE 0
#endif

/*! @brief Allow to set Bluetooth device name on runtime.
 * Enabling this option allows for runtime configuration of Bluetooth
 * device name.
 */
#ifndef CONFIG_BT_DEVICE_NAME_DYNAMIC
    #define CONFIG_BT_DEVICE_NAME_DYNAMIC 0
#endif

#if CONFIG_BT_DEVICE_NAME_DYNAMIC
/*! @brief Maximum size in bytes for device name.
 * Bluetooth device name storage size, Storage can be up to 248 bytes
 * long (excluding NULL termination).
 * Range 2 to 248 is valid.
 */
#ifndef CONFIG_BT_DEVICE_NAME_MAX
    #define CONFIG_BT_DEVICE_NAME_MAX 28
#endif

#endif /* CONFIG_BT_DEVICE_NAME_DYNAMIC */

/*! @brief Maximum number of local identities, range 1 to 10 is valid.
 * Maximum number of supported local identity addresses, For most
 * products this is safe to leave as the default value (1).
 * Range 1 to 10 is valid.
 */
#ifndef CONFIG_BT_ID_MAX
    #define CONFIG_BT_ID_MAX 1
#endif

/*! @brief Connection enablement, if the macro is set to 0,feature is disabled, if 1, feature is enabled.
 */
#ifndef CONFIG_BT_CONN
    #define CONFIG_BT_CONN 1
#endif

/*! @brief it is the max connection supported by host stack.
 * Maximum number of simultaneous Bluetooth connections supported.
 */
#ifndef CONFIG_BT_MAX_CONN
    #define CONFIG_BT_MAX_CONN 1
#endif

/*! @brief Hidden configuration that is true if ACL or broadcast ISO is enabled
 */
#ifndef CONFIG_BT_CONN_TX
    #define CONFIG_BT_CONN_TX   (CONFIG_BT_CONN | CONFIG_BT_ISO_BROADCASTER)
#endif

/* @brief Connection enablement. */
#if CONFIG_BT_CONN

/*! @brief Controller to Host ACL flow control support.
 * Enable support for throttling ACL buffers from the controller
 * to the host. This is particularly useful when the host and
 * controller are on separate cores, since it ensures that we do
 * not run out of incoming ACL buffers.
 */
#ifndef CONFIG_BT_HCI_ACL_FLOW_CONTROL
    #define CONFIG_BT_HCI_ACL_FLOW_CONTROL 0
#endif

/*! @brief PHY Update, if the macro is set to 0,feature is disabled, if 1, feature is enabled.
 * Enable support for Bluetooth 5.0 PHY Update Procedure.
 */
#ifndef CONFIG_BT_PHY_UPDATE
    #define CONFIG_BT_PHY_UPDATE 0
#endif

/*! @brief Data Length Update,if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * Enable support for Bluetooth v4.2 LE Data Length Update procedure.
 */
#ifndef CONFIG_BT_DATA_LEN_UPDATE
    #define CONFIG_BT_DATA_LEN_UPDATE 0
#endif

/*! @brief Periodic Advertising Sync Transfer receiver, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 */
#ifndef CONFIG_BT_PER_ADV_SYNC_TRANSFER_RECEIVER
    #define CONFIG_BT_PER_ADV_SYNC_TRANSFER_RECEIVER 0
#endif

/*! @brief Periodic Advertising Sync Transfer sender,if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 */
#ifndef CONFIG_BT_PER_ADV_SYNC_TRANSFER_SENDER
    #define CONFIG_BT_PER_ADV_SYNC_TRANSFER_SENDER 0
#endif

/*! @brief Timeout for pending LE Create Connection command in seconds.
 */
#ifndef CONFIG_BT_CREATE_CONN_TIMEOUT
    #define CONFIG_BT_CREATE_CONN_TIMEOUT 3
#endif

/*! @brief Peripheral connection parameter update timeout in milliseconds, range 1 to 65535 is valid.
 * The value is a timeout used by peripheral device to wait until it
 * starts the connection parameters update procedure to change default
 * connection parameters. The default value is set to 5s, to comply
 * with BT protocol specification: Core 4.2 Vol 3, Part C, 9.3.12.2
 * Range 1 to 65535 is valid.
 */
#ifndef CONFIG_BT_CONN_PARAM_UPDATE_TIMEOUT
    #define CONFIG_BT_CONN_PARAM_UPDATE_TIMEOUT 5000
#endif

/*! @brief Maximum number of pending TX buffers.
 * Maximum number of pending TX buffers that have not yet
 * been acknowledged by the controller.
 */
#ifndef CONFIG_BT_CONN_TX_MAX
    #define CONFIG_BT_CONN_TX_MAX CONFIG_BT_L2CAP_TX_BUF_COUNT
#endif

/*! @brief CONFIG_BT_CONN_DISABLE_SECURITY is enabled.
 * Security is disabled for incoming requests for GATT attributes and L2CAP
 * channels that would otherwise require encryption/authentication in order to be accessed.
 * Do not use in production.
 */
#ifndef CONFIG_BT_CONN_DISABLE_SECURITY
    #define CONFIG_BT_CONN_DISABLE_SECURITY 0
#endif

/*! @brief Accept any values for connection parameters, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * Some controllers support additional connection parameter ranges
 * beyond what is described in the specification. Enabling this option
 * allows the application to set any value to all connection parameters.
 * Tbe Host will perform no limits nor consistency checks on any of the
 * connection parameters (conn interval min and max, latency and timeout).
 * However, the Host will still use numerical comparisons between the
 * min and max connection intervals in order to verify whether the
 * desired parameters have been established in the connection.
 */
#ifndef CONFIG_BT_CONN_PARAM_ANY
    #define CONFIG_BT_CONN_PARAM_ANY 0
#endif

#ifndef CONFIG_BT_CTLR_LE_POWER_CONTROL_SUPPORT
    #define CONFIG_BT_CTLR_LE_POWER_CONTROL_SUPPORT 0
#endif
#if CONFIG_BT_CTLR_LE_POWER_CONTROL_SUPPORT
/*! @brief LE Power Control
 * Enable support for LE Power Control Request feature that is defined in the
 * Bluetooth Core specification, Version 5.4 | Vol 6, Part B, Section 4.6.31.
 */
#ifndef CONFIG_BT_TRANSMIT_POWER_CONTROL
    #define CONFIG_BT_TRANSMIT_POWER_CONTROL 1
#endif
#endif /* CONFIG_BT_CTLR_LE_POWER_CONTROL_SUPPORT */

#if CONFIG_BT_PHY_UPDATE
/*! @brief User control of PHY Update Procedure, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * Enable application access to initiate the PHY Update Procedure.
 * The application can also register a callback to be notified about PHY
 * changes on the connection. The current PHY info is available in the
 * connection info.
 */
#ifndef CONFIG_BT_USER_PHY_UPDATE
    #define CONFIG_BT_USER_PHY_UPDATE 0
#endif

/*! @brief Auto-initiate PHY Update Procedure, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * Initiate PHY Update Procedure on connection establishment.
 *
 * Disable this if you want the PHY Update Procedure feature supported
 * but want to rely on the remote device to initiate the procedure at its
 * discretion or want to initiate manually.
 */
#ifndef CONFIG_BT_AUTO_PHY_UPDATE
    #if CONFIG_BT_USER_PHY_UPDATE
    #define CONFIG_BT_AUTO_PHY_UPDATE 0
    #else
    #define CONFIG_BT_AUTO_PHY_UPDATE 1
    #endif
#endif



#endif /* CONFIG_BT_PHY_UPDATE */

#if CONFIG_BT_DATA_LEN_UPDATE

/*! @brief User control of Data Length Update Procedure, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * Enable application access to initiate the Data Length Update
 * Procedure. The application can also a register callback to be notified
 * about Data Length changes on the connection. The current Data Length
 * info is available in the connection info.
 */
#ifndef CONFIG_BT_USER_DATA_LEN_UPDATE
    #define CONFIG_BT_USER_DATA_LEN_UPDATE 0
#endif

/*! @brief Auto-initiate Data Length Update procedure, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * Initiate Data Length Update Procedure on connection establishment.
 *
 * Disable this if you want the Data Length Update Procedure feature
 * supported but want to rely on the remote device to initiate the
 * procedure at its discretion or want to initiate manually.
 */
#ifndef CONFIG_BT_AUTO_DATA_LEN_UPDATE
    #if CONFIG_BT_USER_DATA_LEN_UPDATE
    #define CONFIG_BT_AUTO_DATA_LEN_UPDATE 0
    #else
    #define CONFIG_BT_AUTO_DATA_LEN_UPDATE 1
    #endif
#endif

#endif /* CONFIG_BT_DATA_LEN_UPDATE */

/*! @brief Enable application access to remote information.
 * Enable application access to the remote information available in the
 * stack. The remote information is retrieved once a connection has been
 * established and the application will be notified when this information
 * is available through the remote_version_available connection callback.
 */
#ifndef CONFIG_BT_REMOTE_INFO
    #define CONFIG_BT_REMOTE_INFO 0
#endif
/* Enable application access to remote information*/
#if CONFIG_BT_REMOTE_INFO

/*! @brief Enable fetching of remote version.
 * Enable this to get access to the remote version in the Controller and
 * in the Host through bt_conn_get_info(). The fields in question can
 * be then found in the bt_conn_info struct.
 */
#ifndef CONFIG_BT_REMOTE_VERSION
    #define CONFIG_BT_REMOTE_VERSION 0
#endif

#endif /* CONFIG_BT_REMOTE_INFO */


/* Security Manager Protocol support.
 * This option enables support for the Security Manager Protocol
 * (SMP), making it possible to pair devices over LE.
 */
#if ((defined(CONFIG_BT_SMP)) && (CONFIG_BT_SMP))

/*! @brief Secure Connections Only Mode.
 * This option enables support for Secure Connection Only Mode. In this
 * mode device shall only use Security Mode 1 Level 4 with exception
 * for services that only require Security Mode 1 Level 1 (no security).
 * Security Mode 1 Level 4 stands for authenticated LE Secure Connections
 * pairing with encryption. Enabling this option disables legacy pairing.
 */
#ifndef CONFIG_BT_SMP_SC_ONLY
    #define CONFIG_BT_SMP_SC_ONLY 0
#endif
/* Secure Connections Only Mode*/
#if CONFIG_BT_SMP_SC_ONLY
    /*! @brief Disable legacy pairing, if 1, legacy pairing is disabled, vice versa.
     * This option disables LE legacy pairing and forces LE secure connection
     * pairing. All Security Mode 1 levels can be used with legacy pairing
     * disabled, but pairing with devices that do not support secure
     * connections pairing will not be supported.
     * To force a higher security level use "Secure Connections Only Mode"
     */
    #define CONFIG_BT_SMP_SC_PAIR_ONLY 1
#endif /* CONFIG_BT_SMP_SC_ONLY */

#if !((defined(CONFIG_BT_SMP_SC_PAIR_ONLY) && (CONFIG_BT_SMP_SC_PAIR_ONLY > 0U)) || CONFIG_BT_SMP_SC_ONLY)
/*! @brief Force Out Of Band Legacy pairing.
 * This option disables Legacy and LE SC pairing and forces legacy OOB.
 */
#ifndef CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY
    #define CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY 0
#endif

#endif /* !(CONFIG_BT_BT_SMP_SC_PAIR_ONLY || CONFIG_BT_SMP_SC_ONLY) */

#if !((defined(CONFIG_BT_SMP_SC_PAIR_ONLY) && (CONFIG_BT_SMP_SC_PAIR_ONLY > 0U)) || CONFIG_BT_SMP_SC_ONLY || CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY)
/*! @brief Forbid usage of insecure legacy pairing methods.
 * This option disables Just Works and Passkey legacy pairing methods to
 * increase security.
 */
#ifndef CONFIG_BT_SMP_DISABLE_LEGACY_JW_PASSKEY
    #define CONFIG_BT_SMP_DISABLE_LEGACY_JW_PASSKEY 0
#endif

#endif /* !(CONFIG_BT_SMP_SC_PAIR_ONLY || CONFIG_BT_SMP_SC_ONLY || CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY) */

/*! @brief Passkey Keypress Notification support [EXPERIMENTAL] Feature, if the macro is set to 0, 
 * feature is disabled, if 1, feature is enabled.
 * Enable support for receiving and sending Keypress Notifications during
 * Passkey Entry during pairing..
 */
#ifndef CONFIG_BT_PASSKEY_KEYPRESS
    #define CONFIG_BT_PASSKEY_KEYPRESS 0
#endif

/*! @brief Privacy Feature, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * Enable Privacy Feature support. This makes it possible to generate and use
 * Resolvable Private Addresses (RPAs).
 *
 * Disabling this will remove the capability to resolve private addresses.
 */
#ifndef CONFIG_BT_PRIVACY
    #define CONFIG_BT_PRIVACY 0
#endif

/*! @brief Randomize identity root for fallback identities
 * Enabling this option will cause the Host to ignore controller-provided
 * identity roots (IR). The Host will instead use bt_rand to generate
 * identity resolving keys (IRK) and store them in the settings subsystem.
 *
 * Setting this config may come with a performance penalty to boot time,
 * as the hardware RNG may need time to generate entropy and will block
 * Bluetooth initialization.
 *
 * This option increases privacy, as explained in the following text.
 *
 * The IR determines the IRK of the identity. The IRK is used to both
 * generate and resolve (recognize) the private addresses of an identity.
 * The IRK is a shared secret, distributed to peers bonded to that
 * identity.
 *
 * An attacker that has stolen or once bonded and retained the IRK can
 * forever resolve addresses from that IRK, even if that bond has been
 * deleted locally.
 *
 * Deleting an identity should ideally delete the IRK as well and thereby
 * restore anonymity from previously bonded peers. But unless this config
 * is set, this does not always happen.
 *
 * In particular, a factory reset function that wipes the data in the
 * settings subsystem may not affect the controller-provided IRs. If
 * those IRs are reused, this device can be tracked across factory resets.
 *
 * For optimal privacy, a new IRK (i.e., identity) should be used per
 * bond. However, this naturally limits advertisements from that identity
 * to be recognizable by only that one bonded device.
 *
 * A description of the exact effect of this setting follows.
 *
 * If the application has not setup an identity before calling
 * settings_load()/settings_load_subtree("bt") after bt_enable(), the
 * Host will automatically try to load saved identities from the settings
 * subsystem, and if there are none, set up the default identity
 * (BT_ID_DEFAULT).
 *
 * If the controller has a public address (HCI_Read_BD_ADDR), that becomes
 * the address of the default identity. The Host will by default try to
 * obtain the IR for that identity from the controller (by Zephyr HCI
 * Read_Key_Hierarchy_Roots). Setting this config randomizes the IR
 * instead.
 *
 * If the controller does not have a public address, the Host will try
 * to source the default identity from the static address information
 * from controller (Zephyr HCI Read_Static_Addresses). This results in an
 * identity for each entry in Read_Static_Addresses. Setting this config
 * randomizes the IRs during this process.
 */
#ifndef CONFIG_BT_PRIVACY_RANDOMIZE_IR
    #define CONFIG_BT_PRIVACY_RANDOMIZE_IR 0
#endif

#if (defined(CONFIG_BT_PRIVACY_RANDOMIZE_IR) && (CONFIG_BT_PRIVACY_RANDOMIZE_IR > 0))
    #if !((defined(CONFIG_BT_PRIVACY) && (CONFIG_BT_PRIVACY > 0)))
        #error CONFIG_BT_PRIVACY_RANDOMIZE_IR depends on CONFIG_BT_PRIVACY.
    #endif

    #if !((defined(CONFIG_BT_SETTINGS) && (CONFIG_BT_SETTINGS > 0)))
        #error CONFIG_BT_PRIVACY_RANDOMIZE_IR depends on CONFIG_BT_SETTINGS.
    #endif
#endif

/*! @brief Enable ECDH key generation support.
 * This option adds support for ECDH HCI commands.
 */
#ifndef CONFIG_BT_ECC
    #if CONFIG_BT_SMP && !CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY
    #define CONFIG_BT_ECC 1
    #else
    #define CONFIG_BT_ECC 0
    #endif
#endif

#if CONFIG_BT_ECC
/*! @brief Use TinyCrypt library for ECDH.
 * If this option is used to set TinyCrypt library which is used for emulating the
 * ECDH HCI commands and events needed by e.g. LE Secure Connections.
 * In builds including the BLE Host, if don't set the controller crypto which is
 * used for ECDH and if the controller doesn't support the required HCI
 * commands the LE Secure Connections support will be disabled.
 * In builds including the HCI Raw interface and the BLE Controller, this
 * option injects support for the 2 HCI commands required for LE Secure
 * Connections so that Hosts can make use of those. The option defaults
 * to enabled for a combined build with Zephyr's own controller, since it
 * does not have any special ECC support itself (at least not currently).
 */
#ifndef CONFIG_BT_TINYCRYPT_ECC
    #define CONFIG_BT_TINYCRYPT_ECC 0
#endif

/*! @brief Thread priority of ECC Task.
 */
#ifndef CONFIG_BT_TINYCRYPT_ECC_PRIORITY
    #define CONFIG_BT_TINYCRYPT_ECC_PRIORITY 10
#endif

/*! @brief Thread stack size of ECC Task.
 */
#ifndef CONFIG_BT_HCI_ECC_STACK_SIZE
    #define CONFIG_BT_HCI_ECC_STACK_SIZE 1100
#endif

#endif /* CONFIG_BT_ECC */

/*! @brief Bluetooth Resolvable Private Address (RPA)
 * This option enables RPA feature.
 */
#ifndef CONFIG_BT_RPA
    #define CONFIG_BT_RPA 1
#endif

/*! @brief Resolvable Private Address timeout, defaults to 900 seconds.
 * This option defines how often resolvable private address is rotated.
 * Value is provided in seconds and defaults to 900 seconds (15 minutes).
 */
#ifndef CONFIG_BT_RPA_TIMEOUT
    #define CONFIG_BT_RPA_TIMEOUT 900
#endif

/*! @brief Support setting the Resolvable Private Address timeout at runtime
 * This option allows the user to override the default value of
 * the Resolvable Private Address timeout using dedicated APIs.
 */
#ifndef CONFIG_BT_RPA_TIMEOUT_DYNAMIC
    #define CONFIG_BT_RPA_TIMEOUT_DYNAMIC 0
#endif

#if (defined(CONFIG_BT_RPA_TIMEOUT_DYNAMIC) && (CONFIG_BT_RPA_TIMEOUT_DYNAMIC > 0))
    #if !((defined(CONFIG_BT_PRIVACY) && (CONFIG_BT_PRIVACY > 0)))
        #error CONFIG_BT_RPA_TIMEOUT_DYNAMIC depends on CONFIG_BT_PRIVACY.
    #endif
#endif

/*! @brief Share the Resolvable Private Address between advertising sets
 * This option configures the advertising sets linked with the same
 * Bluetooth identity to use the same Resolvable Private Address in
 * a given rotation period. After the RPA timeout, the new RPA is
 * generated and shared between the advertising sets in the subsequent
 * rotation period. When this option is disabled, the generated RPAs
 * of the advertising sets differ from each other in a given rotation
 * period.
 */
#ifndef CONFIG_BT_RPA_SHARING
    #define CONFIG_BT_RPA_SHARING 0
#endif

#if (defined(CONFIG_BT_RPA_SHARING) && (CONFIG_BT_RPA_SHARING > 0))
    #if !((defined(CONFIG_BT_PRIVACY) && (CONFIG_BT_PRIVACY > 0)))
        #error CONFIG_BT_RPA_SHARING depends on CONFIG_BT_PRIVACY.
    #endif
    #if !((defined(CONFIG_BT_EXT_ADV) && (CONFIG_BT_EXT_ADV > 0)))
        #error CONFIG_BT_RPA_SHARING depends on CONFIG_BT_EXT_ADV.
    #endif
#endif

/*! @brief Data signing support, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * This option enables data signing which is used for transferring
 * authenticated data in an unencrypted connection.
 */
#ifndef CONFIG_BT_SIGNING
    #define CONFIG_BT_SIGNING 0
#endif

/*! @brief Accept or reject pairing initiative.
 * When receiving pairing request or pairing response queries, the
 * application shall either accept proceeding with pairing or not. This is
 * for pairing over SMP and does not affect SSP, which will continue
 * pairing without querying the application.
 * The application can return an error code, which is translated into
 * an SMP return value if the pairing is not allowed.
 */
#ifndef CONFIG_BT_SMP_APP_PAIRING_ACCEPT
    #define CONFIG_BT_SMP_APP_PAIRING_ACCEPT 0
#endif

/*! @brief Allow unauthenticated pairing for paired device.
 * This option allows all unauthenticated pairing attempts made by the
 * peer where an unauthenticated bond already exists.
 * This would enable cases where an attacker could copy the peer device
 * address to connect and start an unauthenticated pairing procedure
 * to replace the existing bond. When this option is disabled in order
 * to create a new bond the old bond has to be explicitly deleted with
 * bt_unpair.
 */
#ifndef CONFIG_BT_SMP_ALLOW_UNAUTH_OVERWRITE
    #define CONFIG_BT_SMP_ALLOW_UNAUTH_OVERWRITE 0
#endif

/*! @brief This option allows unauthenticated pairing attempts made by the
 * peer where an unauthenticated bond already exists on other local
 * identity. This configuration still blocks unauthenticated pairing
 * attempts on the same local identity. To allow the pairing procedure
 * unconditionally, please see the BT_SMP_ALLOW_UNAUTH_OVERWRITE
 * configuration.
 */
#ifndef CONFIG_BT_ID_ALLOW_UNAUTH_OVERWRITE
    #define CONFIG_BT_ID_ALLOW_UNAUTH_OVERWRITE 0
#endif

/*! @brief 	  When a bond is about to complete, find any other bond with the same
 * peer address (or IRK) and `bt_unpair` that bond before the event
 * pairing_complete`.
 * 
 * Important: If this option is not enabled, the current implementation
 * will automatically fail the bonding. See "RL limitation" below.
 * 
 * Important: If this option is not enabled, as Peripheral, it may be too
 * late to abort the bonding. The pairing is failed locally, but it may
 * still be reported as successful on the Central. When this situation
 * occurs, the Zephyr Peripheral will immediately disconnect. See "SMP
 * limitation" below.
 * 
 * RL limitation]:
 * The Host implementors have considered it unlikely that applications
 * would ever want to have multiple bonds with the same peer. The
 * implementors prioritize the simplicity of the implementation over this
 * capability.
 * 
 * The Resolve List on a Controller is not able to accommodate multiple
 * local addresses/IRKs for a single remote address. This would prevent
 * the Host from setting up a one-to-one correspondence between the Host
 * bond database and the Controller Resolve List. The implementation
 * relies on that capability when using the Resolve List. For performance
 * reasons, there is the wish to not fallback to Host Address Resolution
 * in this case.

 * [SMP Limitation]:
 * The Paring Failed command of the Security Manager Protocol cannot be
 * sent outside of a Pairing Process. A Pairing Process ends when the
 * last Transport Specific Key to be distributed is acknowledged at
 * link-layer. The Host does not have control over this acknowledgment,
 * and the order of distribution is fixed by the specification.
 */
 
#ifndef CONFIG_BT_ID_UNPAIR_MATCHING_BONDS
    #define CONFIG_BT_ID_UNPAIR_MATCHING_BONDS 0
#endif


/*! @brief Use a fixed passkey for pairing, set passkey to fixed or not.
 * With this option enabled, the application will be able to call the
 * bt_passkey_set() API to set a fixed passkey. If set, the
 * pairing_confim() callback will be called for all incoming pairings.
 */
#ifndef CONFIG_BT_FIXED_PASSKEY
    #define CONFIG_BT_FIXED_PASSKEY 0
#endif /* CONFIG_BT_FIXED_PASSKEY */
/* Enable tinycrypt ecc*/
#if CONFIG_BT_TINYCRYPT_ECC

/*! @brief Enable Security Manager Debug Mode.
 * This option places Security Manager in a Debug Mode. In this mode
 * predefined Diffie-Hellman private/public key pair is used as described
 * in Core Specification Vol. 3, Part H, 2.3.5.6.1. This option should
 * only be enabled for debugging and should never be used in production.
 * If this option is enabled anyone is able to decipher encrypted air
 * traffic.
 */
#ifndef CONFIG_BT_USE_DEBUG_KEYS
    #define CONFIG_BT_USE_DEBUG_KEYS 0
#endif

/*! @brief Store Debug Mode bonds.
 * This option enables support for storing bonds where either device
 * has the Security Manager in Debug mode. This option should
 * only be enabled for debugging and should never be used in production.
 */
#ifndef CONFIG_BT_STORE_DEBUG_KEYS
    #define CONFIG_BT_STORE_DEBUG_KEYS 0
#endif

#else
#undef CONFIG_BT_USE_DEBUG_KEYS
#undef CONFIG_BT_STORE_DEBUG_KEYS
#endif /* CONFIG_BT_TINYCRYPT_ECC */

/*! @brief Bondable Mode, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * This option enables support for Bondable Mode. In this mode,
 * Bonding flag in AuthReq of SMP Pairing Request/Response will be set
 * indicating the support for this mode.
 */
#ifndef CONFIG_BT_BONDABLE
    /*! @brief Bondable Mode.*/
    #define CONFIG_BT_BONDABLE 1
#endif
#if CONFIG_BT_BONDABLE
/*! @brief Always require bonding.
 * When this option is enabled remote devices are required to always
 * set the bondable flag in their pairing request. Any other kind of
 * requests will be rejected.
 */
#ifndef CONFIG_BT_BONDING_REQUIRED
    #define CONFIG_BT_BONDING_REQUIRED 0
#endif

#endif /* CONFIG_BT_BONDABLE */

/*! @brief Set/clear the bonding flag per-connection [EXPERIMENTAL].
 * Enable support for the bt_conn_set_bondable API function that is
 * used to set/clear the bonding flag on a per-connection basis.
 */
#ifndef CONFIG_BT_BONDABLE_PER_CONNECTION
    #define CONFIG_BT_BONDABLE_PER_CONNECTION 0
#endif /* CONFIG_BT_BONDABLE_PER_CONNECTION */

/*! @brief Enforce MITM protection, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * With this option enabled, the Security Manager will set MITM option in
 * the Authentication Requirements Flags whenever local IO Capabilities
 * allow the generated key to be authenticated.
 */
#ifndef CONFIG_BT_SMP_ENFORCE_MITM
    #define CONFIG_BT_SMP_ENFORCE_MITM 0
#endif

/*! @brief Use a fixed random number for LESC OOB pairing.
 * With this option enabled, the application will be able to perform LESC
 * pairing with OOB data that consists of fixed random number and confirm
 * value. This option should only be enabled for debugging and should
 * never be used in production.
 */
#ifndef CONFIG_BT_OOB_DATA_FIXED
    #define CONFIG_BT_OOB_DATA_FIXED 0
#endif

/*! @brief Overwrite oldest keys with new ones if key storage is full.
 * With this option enabled, if a pairing attempt occurs and the key storage
 * is full, then the oldest keys in storage will be removed to free space
 * for the new pairing keys.
*/
#ifndef CONFIG_BT_KEYS_OVERWRITE_OLDEST
    #define CONFIG_BT_KEYS_OVERWRITE_OLDEST 0
#endif

#if (defined(CONFIG_BT_SMP_SC_PAIR_ONLY) && (CONFIG_BT_SMP_SC_PAIR_ONLY > 0U)) && CONFIG_BT_KEYS_OVERWRITE_OLDEST
    /*! @brief Store aging counter upon each successful paring.
     * With this option enabled, aging counter will be stored in settings every
    * time a successful pairing. This increases flash wear out but offers
    * a more correct finding of the oldest unused pairing info.
     */
#ifndef CONFIG_BT_KEYS_SAVE_AGING_COUNTER_ON_PAIRING
    #define CONFIG_BT_KEYS_SAVE_AGING_COUNTER_ON_PAIRING 0
#endif

#endif /* CONFIG_BT_SETTINGS && CONFIG_BT_KEYS_OVERWRITE_OLDEST */

/*! @brief Encrypted Advertising Data [EXPERIMENTAL].
 * Enable the Encrypted Advertising Data.
 */
#ifndef CONFIG_BT_EAD
    #define CONFIG_BT_EAD 0
#endif

#if (defined(CONFIG_BT_EAD) && (CONFIG_BT_EAD > 0))
#define CONFIG_BT_HOST_CCM 1
#endif

/*! @brief Enable host side AES-CCM module.
 * Enables the software based AES-CCM engine in the host. Will use the
 * controller's AES encryption functions if available, or BT_HOST_CRYPTO
 * otherwise.
 */
#ifndef CONFIG_BT_HOST_CCM
    #define CONFIG_BT_HOST_CCM 0
#endif


/*! @brief "Minimum encryption key size accepted in octets, rangeing from 7 to 16
 * This option sets the minimum encryption key size accepted during pairing.
 */
#ifndef CONFIG_BT_SMP_MIN_ENC_KEY_SIZE
#if (defined(CONFIG_BT_SMP_SC_ONLY) && (CONFIG_BT_SMP_SC_ONLY > 0))
#define CONFIG_BT_SMP_MIN_ENC_KEY_SIZE 16
#else
#define CONFIG_BT_SMP_MIN_ENC_KEY_SIZE 7
#endif /* CONFIG_BT_SMP_SC_ONLY */
#endif /* CONFIG_BT_SMP_MIN_ENC_KEY_SIZE */

#endif /* CONFIG_BT_SMP */

/*************************** L2CAP layer ********************/

/*! @brief Number of buffers available for outgoing L2CAP packets, ranging from 2 to 255.
 * range is 2 to 255
 */
#ifndef CONFIG_BT_L2CAP_TX_BUF_COUNT
    #define CONFIG_BT_L2CAP_TX_BUF_COUNT 3
#endif

/*! @brief Number of L2CAP TX fragment buffers, ranging from 0 to 255.
 * Number of buffers available for fragments of TX buffers. Warning:
 * setting this to 0 means that the application must ensure that
 * queued TX buffers never need to be fragmented, i.e. that the
 * controller's buffer size is large enough. If this is not ensured,
 * and there are no dedicated fragment buffers, a deadlock may occur.
 * In most cases the default value of 2 is a safe bet.
 * range is 0 to 255.
 */
#ifndef CONFIG_BT_L2CAP_TX_FRAG_COUNT
    #define CONFIG_BT_L2CAP_TX_FRAG_COUNT 2
#endif

/*! @brief Maximum supported L2CAP MTU for L2CAP TX buffers, if CONFIG_BT_SMP is set, the range is 65 to 2000, Otherwise, range is 23 to 2000.
 * range is 23 to 2000.
 * range is 65 to 2000 for CONFIG_BT_SMP.
 */
#ifndef CONFIG_BT_L2CAP_TX_MTU
    #if (defined(CONFIG_BT_BREDR) && (CONFIG_BT_BREDR > 0U))
    #define CONFIG_BT_L2CAP_TX_MTU 253
    #elif ((defined(CONFIG_BT_SMP)) && (CONFIG_BT_SMP > 0))
    #define CONFIG_BT_L2CAP_TX_MTU 65
    #else
    #define CONFIG_BT_L2CAP_TX_MTU 23
    #endif
#endif

/*! @brief Delay between retries for sending L2CAP segment. Necessary because the
 *	  stack might not be able to allocate enough conn contexts and might not
 *	  have enough credits, leading to a state where an SDU is stuck
 *	  mid-transfer and never resumes.
 *
 *	  Note that this should seldom happen, this is just to work around a few
 *	  edge cases.
 */
#ifndef CONFIG_BT_L2CAP_RESCHED_MS
    #define CONFIG_BT_L2CAP_RESCHED_MS 1000
#endif

/*! @briefL2CAP Receive segment direct API [EXPERIMENTAL]
 * 	 Enable API for direct receiving of L2CAP SDU segments, bypassing the
 *	 Host's fixed-function SDU re-assembler, RX SDU buffer management and
 *	 credit issuer.
 *   This API enforces conformance with L2CAP TS, but is otherwise as
 *   flexible and semantically simple as possible.
 */
#ifndef CONFIG_BT_L2CAP_SEG_RECV
    #define CONFIG_BT_L2CAP_SEG_RECV 0
#endif



#if ((defined(CONFIG_BT_SMP)) && (CONFIG_BT_SMP > 0))
/*! @brief L2CAP Dynamic Channel support.
 * This option enables support for LE Connection oriented Channels,
 * allowing the creation of dynamic L2CAP Channels.
 */
#ifndef CONFIG_BT_L2CAP_DYNAMIC_CHANNEL
    #define CONFIG_BT_L2CAP_DYNAMIC_CHANNEL 0
#endif
#endif /* CONFIG_BT_SMP */

#if ((defined(CONFIG_BT_L2CAP_DYNAMIC_CHANNEL)) && (CONFIG_BT_L2CAP_DYNAMIC_CHANNEL > 0))
/*! @brief L2CAP Enhanced Credit Based Flow Control support.
 * This option enables support for LE Connection oriented Channels with
 * Enhanced Credit Based Flow Control support on dynamic L2CAP Channels.
 */
#ifndef CONFIG_BT_L2CAP_ECRED
    #define CONFIG_BT_L2CAP_ECRED 0
#endif
#endif /* CONFIG_BT_L2CAP_DYNAMIC_CHANNEL */

/*! @brief L2CAP RETRANSMISSION/FLOW CONTROL/STREAMING modes support.
 * This option enables support for RETRANSMISSION/FLOW CONTROL/STREAMING
 * modes.
 * The Enhanced Retransmission Mode and Streaming Mode are supported.
 * The Retransmission Mode and Flow control Mode are not supported yet.
 * The Enhanced Retransmission Mode (Extended Flow Spec) and
 * Streaming Mode (Extended Flow Sepc) are not supported yet.
 */
#ifndef CONFIG_BT_L2CAP_IFRAME_SUPPORT
    #define CONFIG_BT_L2CAP_IFRAME_SUPPORT 0
#endif

/*************************** ATT layer ***********************/
/*! @brief Number of ATT prepare write buffers, if the macro is set to 0, feature is disabled, if greater than 1, feature is enabled.
 * Number of buffers available for ATT prepare write, setting
 * this to 0 disables GATT long/reliable writes.
 */
#ifndef CONFIG_BT_ATT_PREPARE_COUNT
    #define CONFIG_BT_ATT_PREPARE_COUNT 0
#endif

/*! @brief Number of ATT buffers, default BT_BUF_ACL_TX_COUNT
 * These buffers are only used for sending anything over ATT.
 * Requests, responses, indications, confirmations, notifications.
 * range is 1 to 255, default 3.
 */
#ifndef CONFIG_BT_ATT_TX_COUNT
    #define CONFIG_BT_ATT_TX_COUNT 3
#endif

/*! @brief Automatic security elevation and retry on security errors
 * If an ATT request fails due to insufficient security, the host will
 * try to elevate the security level and retry the ATT request.
 */
#if (defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0))

  #ifndef CONFIG_BT_ATT_RETRY_ON_SEC_ERR
      #define CONFIG_BT_ATT_RETRY_ON_SEC_ERR 1
  #endif

#endif

/*! @brief Maximum number of queued outgoing ATT PDUs.
 * Number of ATT PDUs that can be at a single moment queued for
 * transmission. If the application tries to send more than this
 * amount the calls will block until an existing queued PDU gets
 * sent.
 * range is 1 to CONFIG_BT_L2CAP_TX_BUF_COUNT
 */
#ifndef CONFIG_BT_ATT_TX_MAX
    #define CONFIG_BT_ATT_TX_MAX (CONFIG_BT_L2CAP_TX_BUF_COUNT)
#endif

/*! @brief Maximum number of queued ingoing ATT PDUs.
 * Number of ATT PDUs that can be at a single moment queued for
 * transmission. If the application tries to send more than this
 * amount the calls will block until an existing queued PDU gets
 * sent.
 * range is 1 to 255
 */
#ifndef CONFIG_BT_ATT_RX_MAX
    #define CONFIG_BT_ATT_RX_MAX (CONFIG_BT_BUF_ACL_RX_COUNT)
#endif

/*! @brief Enhanced ATT Bearers support [EXPERIMENTAL], if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * This option enables support for Enhanced ATT bearers support. When
 * enabled additional L2CAP channels can be connected as bearers enabling
 * multiple outstanding request.
 */
#if (defined(CONFIG_BT_L2CAP_DYNAMIC_CHANNEL) && (CONFIG_BT_L2CAP_DYNAMIC_CHANNEL > 0))
#ifndef CONFIG_BT_EATT
    #define CONFIG_BT_EATT 0
#endif
#else
#undef CONFIG_BT_EATT
#endif /* CONFIG_BT_L2CAP_DYNAMIC_CHANNEL */

#if (defined(CONFIG_BT_EATT) && (CONFIG_BT_EATT > 0))

#ifndef CONFIG_BT_GATT_READ_MULT_VAR_LEN
    #define CONFIG_BT_GATT_READ_MULT_VAR_LEN 1
#endif

/*! @brief Maximum number of Enhanced ATT bearers, range 1 to 16 is valid.
 * Number of Enhanced ATT bearers available.
 */
#ifndef CONFIG_BT_EATT_MAX
    #define CONFIG_BT_EATT_MAX 3
#endif

/*! @brief Automatically connect EATT bearers when a link is established
 * The device will try to connect BT_EATT_MAX enhanced ATT bearers when a
 * connection to a peer is established.
 */
#ifndef CONFIG_BT_EATT_AUTO_CONNECT
    #define CONFIG_BT_EATT_AUTO_CONNECT 1
#endif

/*! @brief Enhanced ATT bearer security level, range 1 to 4 is valid.
 * L2CAP server required security level of EATT bearers:
 * Level 1 (BT_SECURITY_L1) = No encryption or authentication required
 * Level 2 (BT_SECURITY_L2) = Only encryption required
 * Level 3 (BT_SECURITY_L3) = Encryption and authentication required
 * Level 4 (BT_SECURITY_L4) = Secure connection required
 */
#ifndef CONFIG_BT_EATT_SEC_LEVEL
    #define CONFIG_BT_EATT_SEC_LEVEL 1
#endif

#endif /* CONFIG_BT_EATT */

/*! @brief GATT Service Changed support, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * This option enables support for the service changed characteristic.
 */
#ifndef CONFIG_BT_GATT_SERVICE_CHANGED
    #define CONFIG_BT_GATT_SERVICE_CHANGED 1
#endif

/*! @brief Automatic re-subscription to characteristics, upon re-establishing a bonded connection, assumes the remote
 *	  forgot the CCC values and sets them again. If this behavior is not desired for a particular subscription, set the
 *	  `BT_GATT_SUBSCRIBE_FLAG_NO_RESUB` flag. This also means that upon a reconnection,
 *	  the application will get an unprompted call to its `subscribe` callback.
 */
#ifndef CONFIG_BT_GATT_AUTO_RESUBSCRIBE
    #define CONFIG_BT_GATT_AUTO_RESUBSCRIBE 0
#endif

#if (defined(CONFIG_BT_GATT_AUTO_RESUBSCRIBE) && (CONFIG_BT_GATT_AUTO_RESUBSCRIBE > 0))
    #if !(defined(CONFIG_BT_GATT_CLIENT) && (CONFIG_BT_GATT_CLIENT > 0))
        #error CONFIG_BT_GATT_AUTO_RESUBSCRIBE depends on CONFIG_BT_GATT_CLIENT
    #endif
#endif

/*! @brief GATT dynamic database support*/
#if CONFIG_BT_GATT_SERVICE_CHANGED
/*! @brief GATT dynamic database support, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * This option enables registering/unregistering services at runtime.
 */
#ifndef CONFIG_BT_GATT_DYNAMIC_DB
    #define CONFIG_BT_GATT_DYNAMIC_DB 0
#endif

/*! @brief GATT Caching support, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * This option enables support for GATT Caching. When enabled the stack
 * will register Client Supported Features and Database Hash
 * characteristics which can be used by clients to detect if anything has
 * changed on the GATT database.
 */
#ifndef CONFIG_BT_GATT_CACHING
    #define CONFIG_BT_GATT_CACHING 0
#endif

#if CONFIG_BT_GATT_CACHING

/*! @brief GATT Notify Multiple Characteristic Values support, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * This option enables support for the GATT Notify Multiple
 * Characteristic Values procedure.
 */
#ifndef CONFIG_BT_GATT_NOTIFY_MULTIPLE
    #define CONFIG_BT_GATT_NOTIFY_MULTIPLE 1
#endif

#if (defined(CONFIG_BT_GATT_NOTIFY_MULTIPLE) && (CONFIG_BT_GATT_NOTIFY_MULTIPLE > 0))

/*! @brief Delay for batching multiple notifications in a single PDU.
 * Sets the time (in milliseconds) during which consecutive GATT
 * notifications will be tentatively appended to form a single
 * ATT_MULTIPLE_HANDLE_VALUE_NTF PDU.
 *
 * If set to 0, batching is disabled. Then, the only way to send
 * ATT_MULTIPLE_HANDLE_VALUE_NTF PDUs is to use bt_gatt_notify_multiple.
 *
 * See the documentation of bt_gatt_notify() for more details.
 *
 * Valid range 0 ~ 4000
 */
#ifndef CONFIG_BT_GATT_NOTIFY_MULTIPLE_FLUSH_MS
    #define CONFIG_BT_GATT_NOTIFY_MULTIPLE_FLUSH_MS 1
#endif

#endif

/*! @brief GATT Enforce change-unaware state, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * When enabled, this option blocks notification and indications to client
 * to conform to the following statement from the Bluetooth 5.1
 * specification:
 * '...the server shall not send notifications and indications to such
 * a client until it becomes change-aware."
 * In case the service cannot deal with sudden errors (-EAGAIN) then it
 * shall not use this option.
 */
#ifndef CONFIG_BT_GATT_ENFORCE_CHANGE_UNAWARE
    #define CONFIG_BT_GATT_ENFORCE_CHANGE_UNAWARE 0
#endif
#endif /* CONFIG_BT_GATT_CACHING */

#endif /* CONFIG_BT_GATT_SERVICE_CHANGED */

/*! @brief GATT Enforce characteristic subscription
 * When enabled, this option will make the server block sending
 * notifications and indications to a device which has not subscribed to
 * the supplied characteristic.
 */
#ifndef CONFIG_BT_GATT_ENFORCE_SUBSCRIPTION
    #define CONFIG_BT_GATT_ENFORCE_SUBSCRIPTION      1
#endif

/*! @brief GATT client support, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * This option enables support for the GATT Client role.
 */
#ifndef CONFIG_BT_GATT_CLIENT
    #define CONFIG_BT_GATT_CLIENT      0
#endif

/*! @brief GATT Read Multiple Characteristic Values support, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * This option enables support for the GATT Read Multiple Characteristic
 * Values procedure.
 */
#ifndef CONFIG_BT_GATT_READ_MULTIPLE
    #define CONFIG_BT_GATT_READ_MULTIPLE     1
#endif

/*! @brief Automatically send ATT MTU exchange request on connect
 * This option if enabled allows automatically sending request for ATT
 * MTU exchange.
 */
#ifndef CONFIG_BT_GATT_AUTO_UPDATE_MTU
    #define CONFIG_BT_GATT_AUTO_UPDATE_MTU     0
#endif

#if (defined(CONFIG_BT_GATT_AUTO_UPDATE_MTU) && (CONFIG_BT_GATT_AUTO_UPDATE_MTU > 0))
    #if !(defined(CONFIG_BT_GATT_CLIENT) && (CONFIG_BT_GATT_CLIENT > 0))
        #error CONFIG_BT_GATT_AUTO_UPDATE_MTU depends on CONFIG_BT_GATT_CLIENT
    #endif
#endif

/*! @brief GATT Read Multiple Variable Length Characteristic Values support
 * This option enables support for the GATT Read Multiple Variable Length
 * Characteristic Values procedure. Mandatory if EATT is enabled, optional
 * otherwise (Core spec v5.3, Vol 3, Part G, Section 4.2, Table 4.1).
 *
 */
#ifndef CONFIG_BT_GATT_READ_MULT_VAR_LEN
    #define CONFIG_BT_GATT_READ_MULT_VAR_LEN     1
#endif

/* GATT peripheral support*/
#if CONFIG_BT_PERIPHERAL

/*! @brief Automatic Update of Connection Parameters, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * This option, if enabled, allows automatically sending request for connection
 * parameters update after GAP recommended 5 seconds of connection as
 * peripheral.
 */
#ifndef CONFIG_BT_GAP_AUTO_UPDATE_CONN_PARAMS
    #define CONFIG_BT_GAP_AUTO_UPDATE_CONN_PARAMS 0
#endif

/*! @brief Configure peripheral preferred connection parameters.
 * This allows to configure peripheral preferred connection parameters.
 * Enabling this option results in adding PPCP characteristic in GAP.
 * If disabled it is up to application to set expected connection parameters.
 */
#ifndef CONFIG_BT_GAP_PERIPHERAL_PREF_PARAMS
    #define CONFIG_BT_GAP_PERIPHERAL_PREF_PARAMS 0
#endif

#if CONFIG_BT_GAP_PERIPHERAL_PREF_PARAMS

/*! @brief Peripheral preferred minimum connection interval in 1.25ms units, range 6 to 65534 is valid.
 * Range 6 to 65534 is valid. 65535 represents no specific value.
 */
#ifndef CONFIG_BT_PERIPHERAL_PREF_MIN_INT
    #define CONFIG_BT_PERIPHERAL_PREF_MIN_INT 24
#endif

/*! @brief Peripheral preferred maximum connection interval in 1,25ms units, range 6 to 65534 is valid.
 * Range 6 to 65534 is valid. 65535 represents no specific value.
 */
#ifndef CONFIG_BT_PERIPHERAL_PREF_MAX_INT
    #define CONFIG_BT_PERIPHERAL_PREF_MAX_INT 40
#endif

/*! @brief Peripheral preferred slave latency in Connection Intervals, range 0 to 499 is valid.
 * Range 0 to 499 is valid.
 */
#ifndef CONFIG_BT_PERIPHERAL_PREF_LATENCY
    #define CONFIG_BT_PERIPHERAL_PREF_LATENCY 0
#endif

/*! @brief Peripheral preferred supervision timeout in 10ms units, range 10 to 65534 is valid.
 * It is up to user whether to provide valid timeout which pass required minimum
 * value: in milliseconds it shall be larger than
 * "(1+ Conn_Latency) * Conn_Interval_Max * 2"
 * where Conn_Interval_Max is given in milliseconds.
 * Range 10 to 65534 is valid. 65535 represents no specific value.
 */
#ifndef CONFIG_BT_PERIPHERAL_PREF_TIMEOUT
    #define CONFIG_BT_PERIPHERAL_PREF_TIMEOUT 42
#endif

#endif /* CONFIG_BT_GAP_PERIPHERAL_PREF_PARAMS */
/* Allow to write name by remote GATT clients.*/
#if CONFIG_BT_DEVICE_NAME_DYNAMIC
/*! @brief Allow to write name by remote GATT clients, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * Enabling this option allows remote GATT clients to write to device
 * name GAP characteristic.
 */
#ifndef CONFIG_BT_DEVICE_NAME_GATT_WRITABLE
    #define CONFIG_BT_DEVICE_NAME_GATT_WRITABLE 1
#endif

#if CONFIG_BT_DEVICE_NAME_GATT_WRITABLE
/*! @brief Encryption required to write name by remote GATT clients
 * Enabling this option requires the connection to be encrypted to write
 * to the device name GAP characteristic.
 */
#ifndef CONFIG_DEVICE_NAME_GATT_WRITABLE_ENCRYPT
#define CONFIG_DEVICE_NAME_GATT_WRITABLE_ENCRYPT 1
#endif /* CONFIG_DEVICE_NAME_GATT_WRITABLE_ENCRYPT */

/*! @brief Authentication required to write name by remote GATT clients
 * Enabling this option requires the connection to be encrypted and
 * authenticated to write to the device name GAP characteristic.
 */
#ifndef CONFIG_DEVICE_NAME_GATT_WRITABLE_AUTHEN
#define CONFIG_DEVICE_NAME_GATT_WRITABLE_AUTHEN 0
#endif /* CONFIG_DEVICE_NAME_GATT_WRITABLE_AUTHEN */
#endif /* CONFIG_BT_DEVICE_NAME_GATT_WRITABLE */

#endif /* CONFIG_BT_DEVICE_NAME_DYNAMIC */

#endif /* CONFIG_BT_PERIPHERAL */

#endif /* CONFIG_BT_CONN */

/*! @brief Custom authorization of GATT operations [EXPERIMENTAL]
 * This option allows the user to define application-specific
 * authorization logic for GATT operations that can be registered
 * with the bt_gatt_authorization_cb_register API. See the API
 * documentation for more details.
 */
#ifndef CONFIG_BT_GATT_AUTHORIZATION_CUSTOM
#define CONFIG_BT_GATT_AUTHORIZATION_CUSTOM 0
#endif /* CONFIG_BT_GATT_AUTHORIZATION_CUSTOM */

/*! @brief Maximum number of paired devices
 * Maximum number of paired Bluetooth devices. The minimum (and
 * default) number is 1.
 */
#if ((defined(CONFIG_BT_SMP)) && (CONFIG_BT_SMP > 0))
#ifndef CONFIG_BT_MAX_PAIRED
    #define CONFIG_BT_MAX_PAIRED 1
#endif
#else
#ifndef CONFIG_BT_MAX_PAIRED
    #define CONFIG_BT_MAX_PAIRED 0
#endif
#endif /* CONFIG_BT_SMP */


#if (defined(CONFIG_BT_OBSERVER) && (CONFIG_BT_OBSERVER > 0U))

/*! @brief Scan interval used for background scanning in 0.625 ms units, range 4 to 16384 is valid.
 * Range 4 to 16384 is valid.
 */
#ifndef CONFIG_BT_BACKGROUND_SCAN_INTERVAL
    #define CONFIG_BT_BACKGROUND_SCAN_INTERVAL 2048
#endif

/*! @brief Scan interval used for background scanning in 0.625 ms units, range 4 to 16384 is valid.
 * Range 4 to 16384 is valid.
 */
#ifndef CONFIG_BT_BACKGROUND_SCAN_WINDOW
    #define CONFIG_BT_BACKGROUND_SCAN_WINDOW 18
#endif

/*! @brief Maximum advertisement report size, range 1 to 1650 is valid.
 * Range 1 to 1650 is valid.
 */
#ifndef CONFIG_BT_EXT_SCAN_BUF_SIZE
    #define CONFIG_BT_EXT_SCAN_BUF_SIZE 229
#endif

#endif /* CONFIG_BT_OBSERVER */

#if (!(defined(CONFIG_BT_PRIVACY) && (CONFIG_BT_PRIVACY > 0)) && (CONFIG_BT_CENTRAL || (defined(CONFIG_BT_OBSERVER) && (CONFIG_BT_OBSERVER > 0U)) ))
/*! @brief Perform active scanning using local identity address.
 * Enable this if you want to perform active scanning using the local
 * identity address as the scanner address. By default the stack will
 * always use a non-resolvable private address (NRPA) in order to avoid
 * disclosing local identity information. By not scanning with the
 * identity address the scanner will receive directed advertise reports
 * for the local identity. If this use case is required, then enable
 * this option.
 */
#ifndef CONFIG_BT_SCAN_WITH_IDENTITY
    #define CONFIG_BT_SCAN_WITH_IDENTITY 0
#endif

#endif /* ((!CONFIG_BT_PRIVACY) && (CONFIG_BT_CENTRAL || CONFIG_BT_OBSERVER)) */

/*************************** ISO configuration ***********************/

/*! @brief Bluetooth Isochronous Channel Unicast Support
 * This option enables support for Bluetooth Broadcast Isochronous channels.
 */
#if ((defined(CONFIG_BT_ISO_UNICAST)) && (CONFIG_BT_ISO_UNICAST > 0))

#if (!(defined(CONFIG_BT_CONN)) || !(CONFIG_BT_CONN > 0))
    #error CONFIG_BT_CONN must enable for CONFIG_BT_ISO_UNICAST.
#endif /* !CONFIG_BT_CONN */

#ifndef CONFIG_BT_ISO
    #define CONFIG_BT_ISO 1
#else
    #if !(CONFIG_BT_ISO > 0)
        #error CONFIG_BT_ISO must enable for CONFIG_BT_ISO_UNICAST.
    #endif
#endif

#endif /* CONFIG_BT_ISO_UNICAST */

/*! @brief Bluetooth Isochronous Broadcaster Support
 * This option enables support for the Bluetooth Isochronous Broadcaster.
 */
#if ((defined(CONFIG_BT_ISO_BROADCASTER)) && (CONFIG_BT_ISO_BROADCASTER > 0))

#ifndef CONFIG_BT_ISO_BROADCAST
    #define CONFIG_BT_ISO_BROADCAST 1
#else
    #if !(CONFIG_BT_ISO_BROADCAST > 0)
        #error CONFIG_BT_ISO_BROADCAST must enable for CONFIG_BT_ISO_BROADCASTER.
    #endif
#endif

#if (!(defined(CONFIG_BT_BROADCASTER)) || !(CONFIG_BT_BROADCASTER > 0))
    #error CONFIG_BT_BROADCASTER must enable for CONFIG_BT_ISO_BROADCASTER.
#endif /* !CONFIG_BT_PER_ADV */

#if (!(defined(CONFIG_BT_PER_ADV)) || !(CONFIG_BT_PER_ADV > 0))
    #error CONFIG_BT_PER_ADV must enable for CONFIG_BT_ISO_BROADCASTER.
#endif /* !CONFIG_BT_PER_ADV */

#endif /* CONFIG_BT_ISO_BROADCASTER */

/*! @brief Bluetooth Isochronous Synchronized Receiver Support
 * This option enables support for the Bluetooth Isochronous Synchronized Receiver.
 */
#if ((defined(CONFIG_BT_ISO_SYNC_RECEIVER)) && (CONFIG_BT_ISO_SYNC_RECEIVER > 0))

#ifndef CONFIG_BT_ISO_BROADCAST
    #define CONFIG_BT_ISO_BROADCAST 1
#else
    #if !(CONFIG_BT_ISO_BROADCAST > 0)
        #error CONFIG_BT_ISO_BROADCAST must enable for CONFIG_BT_ISO_BROADCASTER.
    #endif
#endif

#if (!(defined(CONFIG_BT_OBSERVER)) || !(CONFIG_BT_OBSERVER > 0))
    #error CONFIG_BT_OBSERVER must enable for CONFIG_BT_ISO_SYNC_RECEIVER.
#endif /* !CONFIG_BT_OBSERVER */

#if (!(defined(CONFIG_BT_PER_ADV_SYNC)) || !(CONFIG_BT_PER_ADV_SYNC > 0))
    #error CONFIG_BT_PER_ADV_SYNC must enable for CONFIG_BT_ISO_SYNC_RECEIVER.
#endif /* !CONFIG_BT_PER_ADV_SYNC */

#endif /* CONFIG_BT_ISO_SYNC_RECEIVER */

#if ((defined(CONFIG_BT_ISO_BROADCAST)) && (CONFIG_BT_ISO_BROADCAST > 0))

#ifndef CONFIG_BT_ISO
    #define CONFIG_BT_ISO 1
#else
    #if !(CONFIG_BT_ISO > 0)
        #error CONFIG_BT_ISO must enable for CONFIG_BT_ISO_BROADCAST.
    #endif
#endif

#if (!(defined(CONFIG_BT_EXT_ADV)) || !(CONFIG_BT_EXT_ADV > 0))
    #error CONFIG_BT_EXT_ADV must enable for CONFIG_BT_ISO_BROADCAST.
#endif /* !CONFIG_BT_EXT_ADV */

#endif /* CONFIG_BT_ISO_BROADCAST */

#if ((defined(CONFIG_BT_ISO)) && (CONFIG_BT_ISO > 0))

/*! @brief Maximum number of simultaneous Bluetooth isochronous channels supported.
 *
 * Valid range 1 ~ 64
 */
#ifndef CONFIG_BT_ISO_MAX_CHAN
    #define CONFIG_BT_ISO_MAX_CHAN 1
#endif

/*! @brief Number of buffers available for outgoing Isochronous channel SDUs.
 *
 * Valid range 1 ~ 255
 */
#ifndef CONFIG_BT_ISO_TX_BUF_COUNT
    #define CONFIG_BT_ISO_TX_BUF_COUNT 1
#endif

/*! @brief Number of buffers available for fragments of TX buffers.
 * Warning: setting this to 0 means that the application must ensure that
 * queued TX buffers never need to be fragmented, i.e. that the
 * controller's buffer size is large enough. If this is not ensured,
 * and there are no dedicated fragment buffers, a deadlock may occur.
 * In most cases the default value of 2 is a safe bet.
 *
 * Valid range 0 ~ 255
 */
#ifndef CONFIG_BT_ISO_TX_FRAG_COUNT
    #define CONFIG_BT_ISO_TX_FRAG_COUNT 2
#endif

/*! @brief Maximum MTU for Isochronous channels TX buffers.
 *
 * Valid range 23 ~ 4095
 */
#ifndef CONFIG_BT_ISO_TX_MTU
    #define CONFIG_BT_ISO_TX_MTU 251
#endif

/*! @brief Number of buffers available for incoming Isochronous channel SDUs.
 *
 * Valid range 1 ~ 255
 */
#ifndef CONFIG_BT_ISO_RX_BUF_COUNT
    #define CONFIG_BT_ISO_RX_BUF_COUNT 1
#endif

/*! @brief Maximum MTU for Isochronous channels RX buffers.
 *
 * Valid range 23 ~ 4095
 */
#ifndef CONFIG_BT_ISO_RX_MTU
    #define CONFIG_BT_ISO_RX_MTU 251
#endif

#if ((defined(CONFIG_BT_ISO_UNICAST)) && (CONFIG_BT_ISO_UNICAST > 0))

/*! @brief Maximum number of CIGs that are supported by the host. A CIG can be
 * used for either transmitting or receiving.
 */
#ifndef CONFIG_BT_ISO_MAX_CIG
    #define CONFIG_BT_ISO_MAX_CIG 1
#endif

#endif /* CONFIG_BT_ISO_UNICAST */

#if ((defined(CONFIG_BT_ISO_BROADCAST)) && (CONFIG_BT_ISO_BROADCAST > 0))

/*! @brief Maximmum number of BIGs that are supported by the host. A BIG can be
 * used for either transmitting or receiving, but not at the same time.
 */
#ifndef CONFIG_BT_ISO_MAX_BIG
    #define CONFIG_BT_ISO_MAX_BIG 1
#endif

#endif /* CONFIG_BT_ISO_BROADCAST */

#endif /* CONFIG_BT_ISO */

/*************************** Settings configuration ***********************/

/*! @brief Store Bluetooth state and configuration persistently.
 * When selected, the Bluetooth stack will take care of storing
 * (and restoring) the Bluetooth state (such as pairing keys) and
 * configuration persistently in flash.
 * When this option has been enabled, it's important that the
 * application makes a call to settings_load() after having done
 * all necessary initialization (e.g. calling bt_enable). The
 * reason settings_load() is handled externally to the stack, is
 * that there may be other subsystems using the settings API, in
 * which case it's more efficient to load all settings in one go,
 * instead of each subsystem doing it independently.
 * Warning: The Bluetooth host expects a settings backend that loads
 * settings items in handle order.
 */
#if ((defined(CONFIG_BT_SMP)) && (CONFIG_BT_SMP > 0))
#ifndef CONFIG_BT_SETTINGS
    #define CONFIG_BT_SETTINGS 0
#endif
#endif

#if ((defined(CONFIG_BT_SETTINGS)) && (CONFIG_BT_SETTINGS > 0))

/*! @brief Load CCC values from settings when peer connects, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * Load Client Configuration Characteristic setting right after a bonded
 * device connects
 * Disabling this option will increase memory usage as CCC values for all
 * bonded devices will be loaded when calling settings_load.
*/
#ifndef CONFIG_BT_SETTINGS_CCC_LAZY_LOADING
    #define CONFIG_BT_SETTINGS_CCC_LAZY_LOADING 1
#endif

/*! @brief Store CCC value immediately after it has been written, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * Store Client Configuration Characteristic value right after it has
 * been updated
 * By default, CCC is only stored on disconnection.
 * Choosing this option is safer for battery-powered devices or devices
 * that expect to be reset suddenly. However, it requires additional
 * workqueue stack space.
 */
#ifndef CONFIG_BT_SETTINGS_CCC_STORE_ON_WRITE
    #define CONFIG_BT_SETTINGS_CCC_STORE_ON_WRITE 1
#endif

/*! @brief Store CF value immediately after it has been written, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * Store Client Supported Features value right after it has
 * been updated
 * By default, CF is only stored on disconnection.
 * Choosing this option is safer for battery-powered devices or devices
 * that expect to be reset suddenly. However, it requires additional
 * workqueue stack space.
 */
#ifndef CONFIG_BT_SETTINGS_CF_STORE_ON_WRITE
    #define CONFIG_BT_SETTINGS_CF_STORE_ON_WRITE 1
#endif

/*! @brief Use snprintf to encode Bluetooth settings key strings, if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * When selected, Bluetooth settings will use snprintf to encode
 * key strings
 * When not selected, Bluetooth settings will use a faster builtin
 * function to encode the key string. The drawback is that if
 * printk is enabled then the program memory footprint will be larger.
 */
#ifndef CONFIG_BT_SETTINGS_USE_PRINTK
    #define CONFIG_BT_SETTINGS_USE_PRINTK 1
#endif



/*! @brief Bluetooth BR/EDR settings delayed store enable
 * This option enables Bluetooth BR/EDR settings delayed store support, 	 
 * Triggers the storage of the CF and CCC right after a write.
 * This is done in the workqueue context, in order to not block the BT RX
 * thread for too long.
 */
#ifndef CONFIG_BT_SETTINGS_DELAYED_STORE
    #define CONFIG_BT_SETTINGS_DELAYED_STORE 1
#endif
#endif /* CONFIG_BT_SETTINGS */

/*! @brief Bluetooth BR/EDR support [EXPERIMENTAL]
 * This option enables Bluetooth BR/EDR support
 */
#ifndef CONFIG_BT_BREDR
    #define CONFIG_BT_BREDR 0
#endif

/*! @brief Bluetooth BLE support
 * This option disables Bluetooth BLE support. Note: not all the BLE related codes could be disabled.
 */
#ifndef CONFIG_BT_BLE_DISABLE
    #define CONFIG_BT_BLE_DISABLE 0
#endif

#if CONFIG_BT_BREDR

#if !(CONFIG_BT_PERIPHERAL && CONFIG_BT_CENTRAL && CONFIG_BT_SMP)
#error The feature CONFIG_BT_PERIPHERAL, CONFIG_BT_CENTRAL and CONFIG_BT_SMP should be set.
#endif

#undef CONFIG_BT_L2CAP_DYNAMIC_CHANNEL
#define CONFIG_BT_L2CAP_DYNAMIC_CHANNEL 1


/*! @brief Maximum number of simultaneous SCO connections.
 * Maximum number of simultaneous Bluetooth synchronous connections
 * supported. The minimum (and default) number is 1.
 * Range 1 to 3 is valid.
 */
#ifndef CONFIG_BT_MAX_SCO_CONN
    #define CONFIG_BT_MAX_SCO_CONN 1
#endif

/*! @brief Bluetooth RFCOMM protocol support [EXPERIMENTAL],if the macro is set to 0, feature is disabled,if 1, feature is enabled.
 * This option enables Bluetooth RFCOMM support
 */
#ifndef CONFIG_BT_RFCOMM
    #define CONFIG_BT_RFCOMM 0
#endif

/*! @brief Bluetooth SPP profile support [EXPERIMENTAL],if the macro is set to 0, feature is disabled,if 1, feature is enabled.
 * This option enables Bluetooth SPP support
 */
#ifndef CONFIG_BT_SPP
    #define CONFIG_BT_SPP   CONFIG_BT_RFCOMM
#endif

/*! @brief L2CAP MTU for RFCOMM frames.
 * Maximum size of L2CAP PDU for RFCOMM frames.
 */
#ifndef CONFIG_BT_RFCOMM_L2CAP_MTU
    #define CONFIG_BT_RFCOMM_L2CAP_MTU CONFIG_BT_BUF_ACL_RX_SIZE
#endif /* CONFIG_BT_RFCOMM_L2CAP_MTU */

/*! @brief Bluetooth Handsfree profile HF Role support [EXPERIMENTAL], if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * This option enables Bluetooth HF support
 */
#ifndef CONFIG_BT_HFP_HF
    #define CONFIG_BT_HFP_HF 0
#endif

/*! @brief Bluetooth Handsfree profile AG Role support [EXPERIMENTAL], if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * This option enables Bluetooth HFP AG support
 */
#ifndef CONFIG_BT_HFP_AG
    #define CONFIG_BT_HFP_AG 0
#endif

/*! @brief Bluetooth PBAP profile PCE Role support [EXPERIMENTAL], if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * This option enables Bluetooth PCE support
 */
#ifndef CONFIG_BT_PBAP_PCE
    #define CONFIG_BT_PBAP_PCE 0
#endif

#ifndef CONFIG_BT_PBAP_PCE_SUPPORTED_FEATURE
    #define CONFIG_BT_PBAP_PCE_SUPPORTED_FEATURE (0x000003FFU)
#endif

#ifndef CONFIG_BT_PBAP_PCE_MAX_PKT_LEN
    #define CONFIG_BT_PBAP_PCE_MAX_PKT_LEN (600U)
#endif

/*! @brief Bluetooth PBAP profile PSE Role support [EXPERIMENTAL], if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * This option enables Bluetooth PSE support
 */
#ifndef CONFIG_BT_PBAP_PSE
    #define CONFIG_BT_PBAP_PSE 0
#endif

#ifndef CONFIG_BT_PBAP_PSE_SUPPORTED_FEATURES
    #define  CONFIG_BT_PBAP_PSE_SUPPORTED_FEATURES    (0x000003FFU)
#endif

#ifndef CONFIG_BT_PBAP_PSE_SUPPORTED_REPOSITORIES
    #define CONFIG_BT_PBAP_PSE_SUPPORTED_REPOSITORIES   (0x0FU)
#endif

/*! @brief Bluetooth AVDTP protocol support [EXPERIMENTAL], if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * This option enables Bluetooth AVDTP support
 */
#ifndef CONFIG_BT_AVDTP
    #define CONFIG_BT_AVDTP 0
#endif

/*! @brief Bluetooth MAP profile MCE Role support [EXPERIMENTAL], if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * This option enables Bluetooth MAP MCE support
 */
#ifndef CONFIG_BT_MAP_MCE
    #define CONFIG_BT_MAP_MCE 0
#endif

#if defined(CONFIG_BT_MAP_MCE) && (CONFIG_BT_MAP_MCE > 0)

#ifndef CONFIG_BT_MAP_MCE_MAS_NUM_INSTANCES
    #define CONFIG_BT_MAP_MCE_MAS_NUM_INSTANCES 2
#endif

#ifndef CONFIG_BT_MAP_MCE_MNS_NUM_INSTANCES
    #define CONFIG_BT_MAP_MCE_MNS_NUM_INSTANCES 1
#endif

#ifndef CONFIG_BT_MAP_MCE_MAS_MAX_PKT_LEN
    #define CONFIG_BT_MAP_MCE_MAS_MAX_PKT_LEN 512
#endif

#ifndef CONFIG_BT_MAP_MCE_MNS_MAX_PKT_LEN
    #define CONFIG_BT_MAP_MCE_MNS_MAX_PKT_LEN 1790
#endif

#ifndef CONFIG_BT_MAP_MCE_MAS_SUPPORTED_FEATURES
    #define CONFIG_BT_MAP_MCE_MAS_SUPPORTED_FEATURES 0x0077FFFF
#endif

#endif /* CONFIG_BT_MAP_MCE */

/*! @brief Bluetooth MAP profile MSE Role support [EXPERIMENTAL], if the macro is set to 0, feature is disabled, if 1, feature is enabled.
 * This option enables Bluetooth MAP MSE support
 */
#ifndef CONFIG_BT_MAP_MSE
    #define CONFIG_BT_MAP_MSE 0
#endif

#if defined(CONFIG_BT_MAP_MSE) && (CONFIG_BT_MAP_MSE > 0)

#ifndef CONFIG_BT_MAP_MSE_MAS_NUM_INSTANCES
    #define CONFIG_BT_MAP_MSE_MAS_NUM_INSTANCES 2
#endif

#ifndef CONFIG_BT_MAP_MSE_MNS_NUM_INSTANCES
    #define CONFIG_BT_MAP_MSE_MNS_NUM_INSTANCES 1
#endif

#ifndef CONFIG_BT_MAP_MSE_MAS_MAX_PKT_LEN
    #define CONFIG_BT_MAP_MSE_MAS_MAX_PKT_LEN 1790
#endif

#ifndef CONFIG_BT_MAP_MSE_MNS_MAX_PKT_LEN
    #define CONFIG_BT_MAP_MSE_MNS_MAX_PKT_LEN 512
#endif

#ifndef CONFIG_BT_MAP_MSE_MNS_SUPPORTED_FEATURES
    #define CONFIG_BT_MAP_MSE_MNS_SUPPORTED_FEATURES 0x0077FFFF
#endif

#endif /* CONFIG_BT_MAP_MSE */

/*! @brief Bluetooth A2DP Profile.
 * This option enables the A2DP profile
 */
#ifndef CONFIG_BT_A2DP
    #define CONFIG_BT_A2DP 0
#endif

#if defined(CONFIG_BT_A2DP) && (CONFIG_BT_A2DP > 0) 

/*! @brief Bluetooth A2DP count. */
#ifndef CONFIG_BT_A2DP_MAX_CONN
#define CONFIG_BT_A2DP_MAX_CONN CONFIG_BT_MAX_CONN
#endif

/*! @brief Bluetooth A2DP Profile Source function.
 * This option enables the A2DP profile Source function
 */
#ifndef CONFIG_BT_A2DP_SOURCE
    #define CONFIG_BT_A2DP_SOURCE 0
#endif

/*! @brief Bluetooth A2DP Profile Sink function.
 * This option enables the A2DP profile Sink function
 */
#ifndef CONFIG_BT_A2DP_SINK
    #define CONFIG_BT_A2DP_SINK 0
#endif

#ifndef CONFIG_BT_A2DP_MAX_ENDPOINT_COUNT
    #define CONFIG_BT_A2DP_MAX_ENDPOINT_COUNT 2
#endif

#if defined(CONFIG_BT_A2DP_SOURCE) && (CONFIG_BT_A2DP_SOURCE > 0) 

#ifndef CONFIG_BT_A2DP_SBC_ENCODER_PCM_BUFFER_SIZE
    #define CONFIG_BT_A2DP_SBC_ENCODER_PCM_BUFFER_SIZE 0
#endif

#ifndef CONFIG_BT_A2DP_SBC_ENCODER_BIT_RATE
    #define CONFIG_BT_A2DP_SBC_ENCODER_BIT_RATE 328
#endif

#endif

#if defined(CONFIG_BT_A2DP_SINK) && (CONFIG_BT_A2DP_SINK > 0) 

#ifndef CONFIG_BT_A2DP_SBC_DECODER_PCM_BUFFER_SIZE
    #define CONFIG_BT_A2DP_SBC_DECODER_PCM_BUFFER_SIZE 0
#endif

#ifndef CONFIG_BT_A2DP_SBC_DATA_IND_COUNT
    #define CONFIG_BT_A2DP_SBC_DATA_IND_COUNT 328
#endif

#ifndef CONFIG_BT_A2DP_SBC_DATA_IND_SAMPLES_COUNT
    #define CONFIG_BT_A2DP_SBC_DATA_IND_SAMPLES_COUNT 328
#endif

#endif

/*! @brief Bluetooth A2DP Profile task priority.
 * This option sets the task priority. The task is used to process the streamer data and retry command.
 */
#ifndef CONFIG_BT_A2DP_TASK_PRIORITY
#define CONFIG_BT_A2DP_TASK_PRIORITY (4U)
#endif

/*! @brief Bluetooth A2DP Profile task stack size.
 * This option sets the task stack size.
 */
#ifndef CONFIG_BT_A2DP_TASK_STACK_SIZE
#define CONFIG_BT_A2DP_TASK_STACK_SIZE (2048U)
#endif

/*! @brief Bluetooth A2DP content protection service.
 * This option enables it or not.
 */
#ifndef CONFIG_BT_A2DP_CP_SERVICE
    #define CONFIG_BT_A2DP_CP_SERVICE 0
#endif

/*! @brief Bluetooth A2DP recovery service.
 * This option enables it or not.
 * only the a2dp configuration is supported,
 * data transfer is not supported yet.
 */
#ifndef CONFIG_BT_A2DP_RECOVERY_SERVICE
    #define CONFIG_BT_A2DP_RECOVERY_SERVICE 0
#endif

/*! @brief Bluetooth A2DP reporting service.
 * This option enables it or not.
 * only the a2dp configuration is supported,
 * data transfer is not supported yet.
 */
#ifndef CONFIG_BT_A2DP_REPORTING_SERVICE
    #define CONFIG_BT_A2DP_REPORTING_SERVICE 0
#endif

/*! @brief Bluetooth A2DP delay report service.
 * This option enables it or not.
 */
#ifndef CONFIG_BT_A2DP_DR_SERVICE
    #define CONFIG_BT_A2DP_DR_SERVICE 0
#endif

/*! @brief Bluetooth A2DP header compression service.
 * This option enables it or not.
 * only the a2dp configuration is supported,
 * data transfer is not supported yet.
 */
#ifndef CONFIG_BT_A2DP_HC_SERVICE
    #define CONFIG_BT_A2DP_HC_SERVICE 0
#endif

/*! @brief Bluetooth A2DP multiplexing service.
 * This option enables it or not.
 * only the a2dp configuration is supported,
 * data transfer is not supported yet.
 */
#ifndef CONFIG_BT_A2DP_MULTIPLEXING_SERVICE
    #define CONFIG_BT_A2DP_MULTIPLEXING_SERVICE 0
#endif

#endif

/*! @brief Bluetooth AVRCP profile.
 * This option enables it or not.
 */
#ifndef CONFIG_BT_AVRCP
#define CONFIG_BT_AVRCP 0
#endif

#if defined(CONFIG_BT_AVRCP) && (CONFIG_BT_AVRCP > 0) 

/*! @brief Bluetooth AVRCP count. */
#ifndef CONFIG_BT_AVRCP_MAX_CONN
#define CONFIG_BT_AVRCP_MAX_CONN CONFIG_BT_MAX_CONN
#endif

/*! @brief Bluetooth AVRCP Controller.
 * This option enables it or not.
 */
#ifndef CONFIG_BT_AVRCP_CT
#define CONFIG_BT_AVRCP_CT 0
#endif

/*! @brief Bluetooth AVRCP Target.
 * This option enables it or not.
 */
#ifndef CONFIG_BT_AVRCP_TG
#define CONFIG_BT_AVRCP_TG 0
#endif

/*! @brief Bluetooth AVRCP browsing.
 * This option enables it or not.
 */
#ifndef CONFIG_BT_AVRCP_BROWSING
#define CONFIG_BT_AVRCP_BROWSING 0
#endif

/*! @brief Bluetooth AVRCP cover art.
 * This option enables it or not.
 */
#ifndef CONFIG_BT_AVRCP_COVER_ART
#define CONFIG_BT_AVRCP_COVER_ART 0
#endif

#if defined(CONFIG_BT_AVRCP_COVER_ART) && (CONFIG_BT_AVRCP_COVER_ART > 0) 

/*! @brief Bluetooth AVRCP cover art initiator.
 * This option enables it or not.
 */
#ifndef CONFIG_BT_AVRCP_COVER_ART_INITIATOR
#define CONFIG_BT_AVRCP_COVER_ART_INITIATOR 0
#endif

/*! @brief Bluetooth AVRCP cover art responder.
 * This option enables it or not.
 */
#ifndef CONFIG_BT_AVRCP_COVER_ART_RESPONDER
#define CONFIG_BT_AVRCP_COVER_ART_RESPONDER 0
#endif

#endif

#endif

/*! @brief Bluetooth Page Timeout.
 * This option sets the page timeout value. Value is selected as (N * 0.625) ms.
 */
#ifndef CONFIG_BT_PAGE_TIMEOUT
    #define CONFIG_BT_PAGE_TIMEOUT 0x2000
#endif

/*! @brief Bluetooth Class of Device(CoD).
 * This option sets the class of device.For the list of possible values please
 * consult the following link:
 * https://www.bluetooth.com/specifications/assigned-numbers
 */
#ifndef CONFIG_BT_COD
    #define CONFIG_BT_COD 0x0
#endif

#endif /* CONFIG_BT_BREDR */

/******************* BT Common configuration **********************/

/*! @brief Maximum supported ACL size for outgoing data, ranging from 27 to 251
 * Maximum supported ACL size of data packets sent from the Host to the
 * Controller. This value does not include the HCI ACL header.
 * The Host will segment the data transmitted to the Controller so that
 * packets sent to the Controller will contain data up to this size.
 * In a combined build this value will be set in both the Host and the
 * Controller.
 * In a Host-only build the Host will read the maximum ACL size supported
 * by the Controller and use the smallest value supported by both the
 * Bost and the Controller.
 * The Host supports sending of larger L2CAP PDUs than the ACL size and
 * will fragment L2CAP PDUs into ACL data packets.
 * The Controller will return this value in the HCI LE Read Buffer
 * Size command response. If this size if greater than effective Link
 * Layer transmission size then the Controller will perform
 * fragmentation before transmitting the packet(s) on air.
 * If this value is less than the effective Link Layer transmission size
 * then this will restrict the maximum Link Layer transmission size.
 * Maximum is set to 251 due to implementation limitations (use of
 * uint8_t for length field in PDU buffer structure).
 */
#ifndef CONFIG_BT_BUF_ACL_TX_SIZE
    #define CONFIG_BT_BUF_ACL_TX_SIZE 27
#endif

/*! @brief Number of outgoing ACL data buffers, ranging from 1 to 255
 * Number of outgoing ACL data buffers sent from the Host to the
 * Controller. This determines the maximum amount of data packets the
 * Host can have queued in the Controller before waiting for the
 * to notify the Host that more packets can be queued with the Number of
 * Completed Packets event.
 * The buffers are shared between all of the connections and the Host
 * determines how to divide the buffers between the connections.
 * The Controller will return this value in the HCI LE Read Buffer Size
 * command response.
 */
#ifndef CONFIG_BT_BUF_ACL_TX_COUNT
    #define CONFIG_BT_BUF_ACL_TX_COUNT 3
#endif

/*! @brief Maximum supported ACL size for incoming data, ranging from 27 to 1300
 * Maximum support ACL size of data packets sent from the Controller to
 * the Host. This value does not include the HCI ACL header.
 * In a combined Host and Controller build the buffer sizes in both the
 * Host and the Controller will use this value for buffer sizes, and
 * therefore Controller to Host flow Controller is not needed.
 * In a Host only build with Controller to Host flow control enabled
 * the Host will inform the Controller about the maximum ACL data size it
 * can send by setting this value in the Host Buffer Size command.
 * If Controller to Host flow control is not enabled then the Controller
 * can assume the Host has infinite buffer size so this value should then
 * be set to something that is guaranteed the Controller will not exceed
 * or the data packets will be dropped.
 * In a Controller only build this will determine the maximum ACL size
 * that the Controller will send to the Host.
 * The Host supports reassembling of L2CAP PDUs from ACL data packets,
 * but the maximum supported L2CAP PDU size is limited by the maximum
 * supported ACL size.
 * This means the maximum L2CAP PDU MTU is restricted by the maximum ACL
 * size subtracting the 4 byte header of an L2CAP PDU.
 * When using L2CAP Connection oriented Channels without segmentation
 * then the L2CAP SDU MTU is also restricetd by the maximum ACL size
 * subtracting the 4 Byte header of an L2CAP PDU plus the 2 byte header
 * of an L2CAP SDU.

 * With Enhanced ATT enabled the minimum of 70 is needed to support the
 * minimum ATT_MTU of 64 octets in an L2CAP SDU without segmentation.
 * With SMP LE Secure Connections enabled the minimum of 69 is needed to
 * support the minimum SMP MTU of 65 octets (public key + opcode) in an
 * L2CAP PDU.
 *
 * An L2CAP PDU is also referred to as an L2CAP basic frame or B-frame.
 * An L2CAP SDU is also referred to as an L2CAP Credit-based frame or
 * K-frame.
 */
#ifndef CONFIG_BT_BUF_ACL_RX_SIZE
    #if CONFIG_BT_BREDR
        #define CONFIG_BT_BUF_ACL_RX_SIZE 200
    #elif CONFIG_BT_EATT
        #define CONFIG_BT_BUF_ACL_RX_SIZE 70
    #elif CONFIG_BT_SMP
        #define CONFIG_BT_BUF_ACL_RX_SIZE 69
    #else
        #define CONFIG_BT_BUF_ACL_RX_SIZE 27
    #endif
#endif

/*! @brief Number of incoming ACL data buffers, ranging from 1 to 64
 * Number or incoming ACL data buffers sent from the Controller to the
 * Host.
 * In a combined Host and Controller build the buffers are shared and
 * therefore Controller to Host flow control is not needed.
 * In a Host only build with Controller to Host flow control enabled
 * the Host will inform the Controller about the maximum number of
 * buffers by setting this value in the Host Buffer Size command.
 * When Controller to Host flow control is not enabled the Controller
 * can assume that the Host has infinite amount of buffers.
 */
#ifndef CONFIG_BT_BUF_ACL_RX_COUNT
    #define CONFIG_BT_BUF_ACL_RX_COUNT 6
#endif

/*! @brief Maximum supported HCI Event buffer length, ranging from 68 to 255
 * Maximum supported HCI event buffer size. This value does not include
 * the HCI Event header.
 * This value is used by both the Host and the Controller for buffer
 * sizes that include HCI events. It should be set according to the
 * expected HCI events that can be generated from the configuration.
 * If the subset of possible HCI events is unknown, this should be set to
 * the maximum of 255.
 */
#ifndef CONFIG_BT_BUF_EVT_RX_SIZE
    #define CONFIG_BT_BUF_EVT_RX_SIZE 255
#endif

/*! @brief Number of HCI Event buffers, ranging from 2 to 255
 * Number of buffers available for incoming HCI events from the
 * Controller.
 */
#ifndef CONFIG_BT_BUF_EVT_RX_COUNT
    #define CONFIG_BT_BUF_EVT_RX_COUNT 10
#endif

/*! @brief Maximum supported discardable HCI Event buffer length, ranging from 43 to 255
 * Maximum support discardable HCI event size of buffers in the separate
 * discardable event buffer pool. This value does not include the
 * HCI Event header.
 * The minimum size is set based on the Advertising Report. Setting
 * the buffer size different than BT_BUF_EVT_RX_SIZE can save memory.
 */
#ifndef CONFIG_BT_BUF_EVT_DISCARDABLE_SIZE
    #if (defined(CONFIG_BT_BREDR) && (CONFIG_BT_BREDR > 0U)) || (defined(CONFIG_BT_EXT_ADV) && (CONFIG_BT_EXT_ADV > 0))
        #define CONFIG_BT_BUF_EVT_DISCARDABLE_SIZE 255
    #else
        #define CONFIG_BT_BUF_EVT_DISCARDABLE_SIZE 43
    #endif
#endif

/*! @brief Number of discardable HCI Event buffers, ranging from 1 to 255
 * Number of buffers in a separate buffer pool for events which
 * the HCI driver considers discardable. Examples of such events
 * could be e.g. Advertising Reports. The benefit of having such
 * a pool is that the if there is a heavy inflow of such events
 * it will not cause the allocation for other critical events to
 * block and may even eliminate deadlocks in some cases.
 */
#ifndef CONFIG_BT_BUF_EVT_DISCARDABLE_COUNT
    #define CONFIG_BT_BUF_EVT_DISCARDABLE_COUNT 3
#endif

/*! @brief Maximum support HCI Command buffer length, ranging from 65 to 255
 * Maximum data size for each HCI Command buffer. This value does not
 * include the HCI Command header.
 * This value is used by both the Host and the Controller for buffer
 * sizes that include HCI commands. It should be set according to the
 * expected HCI commands that can be sent from the configuration.
 * If the subset of possible HCI commands is unknown, this should be set
 * to the maximum of 255.
 */
#ifndef CONFIG_BT_BUF_CMD_TX_SIZE
    #if (defined(CONFIG_BT_BREDR) && (CONFIG_BT_BREDR > 0U)) || (defined(CONFIG_BT_EXT_ADV) && (CONFIG_BT_EXT_ADV > 0))
        #define CONFIG_BT_BUF_CMD_TX_SIZE 255
    #else
        #define CONFIG_BT_BUF_CMD_TX_SIZE 65
    #endif
#endif

/*! @brief Number of HCI command buffers, ranging from 2 to 64
 * Number of buffers available for outgoing HCI commands from the Host.
 */
#ifndef CONFIG_BT_BUF_CMD_TX_COUNT
    #define CONFIG_BT_BUF_CMD_TX_COUNT 2
#endif


/****************** DIS Service configuration *********************/
/*! @brief Model name.
 * The device model inside Device Information Service.
 */
#ifndef CONFIG_BT_DIS_MODEL
#define CONFIG_BT_DIS_MODEL "Simplified BT"
#endif /* CONFIG_BT_DIS_MODEL */

/*! @brief Manufacturer name.
 * The device manufacturer inside Device Information Service.
 */
#ifndef CONFIG_BT_DIS_MANUF
#define CONFIG_BT_DIS_MANUF "NXP"
#endif /* CONFIG_BT_DIS_MANUF */

/*! @brief Enable PnP_ID characteristic.
 * Enable PnP_ID characteristic in Device Information Service.
 */
#ifndef CONFIG_BT_DIS_PNP
#define CONFIG_BT_DIS_PNP 1
#endif /* CONFIG_BT_DIS_PNP */

#if CONFIG_BT_DIS_PNP
/*! @brief Vendor ID source, range 1 - 2.
 * The Vendor ID Source field designates which organization assigned the
 * value used in the Vendor ID field value.
 * The possible values are:
 * - 1 Bluetooth SIG, the Vendor ID was assigned by the Bluetooth SIG
 * - 2 USB IF, the Vendor ID was assigned by the USB IF
 */
#ifndef CONFIG_BT_DIS_PNP_VID_SRC
#define CONFIG_BT_DIS_PNP_VID_SRC 1
#endif /* CONFIG_BT_DIS_PNP_VID_SRC */

/*! @brief Vendor ID, range 0 - 0xFFFF.
 * The Vendor ID field is intended to uniquely identify the vendor of the
 * device. This field is used in conjunction with Vendor ID Source field,
 * which determines which organization assigned the Vendor ID field value.
 * Note: The Bluetooth Special Interest Group assigns Device ID Vendor ID,
 * and the USB Implementers Forum assigns Vendor IDs,
 * either of which can be used for the Vendor ID field value.
 * Device providers should procure the Vendor ID from the USB Implementers
 * Forum or the Company Identifier from the Bluetooth SIG.
 *
 */
#ifndef CONFIG_BT_DIS_PNP_VID
#define CONFIG_BT_DIS_PNP_VID 0x0025
#endif /* CONFIG_BT_DIS_PNP_VID */

/*! @brief Product ID, range 0 - 0xFFFF.
 * The Product ID field is intended to distinguish between different products
 * made by the vendor identified with the Vendor ID field. The vendors
 * themselves manage Product ID field values.
 */
#ifndef CONFIG_BT_DIS_PNP_PID
#define CONFIG_BT_DIS_PNP_PID 0
#endif /* CONFIG_BT_DIS_PNP_PID */

/*! @brief Product Version, range 0 - 0xFFFF.
 * The Product Version field is a numeric expression identifying the device
 * release number in Binary-Coded Decimal. This is a vendor-assigned value,
 * which defines the version of the product identified by the Vendor ID and
 * Product ID fields. This field is intended to differentiate between
 * versions of products with identical Vendor IDs and Product IDs.
 * The value of the field value is 0xJJMN for version JJ.M.N
 * (JJ - major version number, M - minor version number,
 * N - sub-minor version number); e.g., version 2.1.3 is represented with
 * value 0x0213 and version 2.0.0 is represented with a value of 0x0200.
 * When upward-compatible changes are made to the device, it is recommended
 * that the minor version number be incremented. If incompatible changes are
 * made to the device, it is recommended that the major version number be
 * incremented. The sub-minor version is incremented for bug fixes.
 */
#ifndef CONFIG_BT_DIS_PNP_VER
#define CONFIG_BT_DIS_PNP_VER 1
#endif /* CONFIG_BT_DIS_PNP_VER */

#endif /* CONFIG_BT_DIS_PNP */

/*! @brief Enable DIS Serial number characteristic, 1 - enable, 0 - disable.
 * Enable Serial Number characteristic in Device Information Service.
 */
#ifndef CONFIG_BT_DIS_SERIAL_NUMBER
#define CONFIG_BT_DIS_SERIAL_NUMBER 1
#endif /* CONFIG_BT_DIS_SERIAL_NUMBER */

#if CONFIG_BT_DIS_SERIAL_NUMBER

/*! @brief Serial Number.
 * Serial Number characteristic string in Device Information Service.
 */
#ifndef CONFIG_BT_DIS_SERIAL_NUMBER_STR
#define CONFIG_BT_DIS_SERIAL_NUMBER_STR "00000000"
#endif /* CONFIG_BT_DIS_SERIAL_NUMBER_STR */

#endif /* CONFIG_BT_DIS_SERIAL_NUMBER */

/*! @brief Enable DIS Firmware Revision characteristic, 1 - enable, 0 - disable.
 * Enable Firmware Revision characteristic in Device Information Service.
 */
#ifndef CONFIG_BT_DIS_FW_REV
#define CONFIG_BT_DIS_FW_REV 1
#endif /* CONFIG_BT_DIS_FW_REV */

#if CONFIG_BT_DIS_FW_REV

/*! @brief Firmware revision.
 * Firmware Revision characteristic String in Device Information Service.
 */
#ifndef CONFIG_BT_DIS_FW_REV_STR
#define CONFIG_BT_DIS_FW_REV_STR "00000000"
#endif /* CONFIG_BT_DIS_FW_REV_STR */

#endif /* CONFIG_BT_DIS_FW_REV */

/*! @brief Enable DIS Hardware Revision characteristic, 1 - enable, 0 - disable.
 * Enable Hardware Revision characteristic in Device Information Service.
 */
#ifndef CONFIG_BT_DIS_HW_REV
#define CONFIG_BT_DIS_HW_REV 1
#endif /* CONFIG_BT_DIS_HW_REV */

#if CONFIG_BT_DIS_HW_REV

/*! @brief Hardware revision.
 * Hardware Revision characteristic String in Device Information Service.
 */
#ifndef CONFIG_BT_DIS_HW_REV_STR
#define CONFIG_BT_DIS_HW_REV_STR "00000000"
#endif /* CONFIG_BT_DIS_HW_REV_STR */

#endif /* CONFIG_BT_DIS_HW_REV */

/*! @brief Enable DIS Software Revision characteristic, 1 - enable, 0 - disable.
 * Enable Software Revision characteristic in Device Information Service.
 */
#ifndef CONFIG_BT_DIS_SW_REV
#define CONFIG_BT_DIS_SW_REV 1
#endif /* CONFIG_BT_DIS_SW_REV */

#if CONFIG_BT_DIS_SW_REV

/*! @brief Software revision
 * Software revision characteristic String in Device Information Service.
 */
#ifndef CONFIG_BT_DIS_SW_REV_STR
#define CONFIG_BT_DIS_SW_REV_STR "00000000"
#endif /* CONFIG_BT_DIS_SW_REV_STR */

#endif /* CONFIG_BT_DIS_SW_REV */

#endif /* CONFIG_BT_HCI_HOST */

#define CONFIG_NET_BUF_USER_DATA_SIZE CONFIG_BT_BUF_RESERVE

/****************** system work queue configuration *********************/
/*! @brief System workqueue stack size
 */
/*! @brief system work queue stack size.*/
#ifndef CONFIG_SYSTEM_WORKQUEUE_STACK_SIZE
    #define CONFIG_SYSTEM_WORKQUEUE_STACK_SIZE 512
#endif

/*! @brief System workqueue priority.
 */
#ifndef CONFIG_SYSTEM_WORKQUEUE_PRIORITY
    #define CONFIG_SYSTEM_WORKQUEUE_PRIORITY 4
#endif

/****************** HCI transport configuration *********************/
/* @brief hci transport interface type.*/
#ifndef CONFIG_BT_HCI_TRANSPORT_INTERFACE_TYPE
    #define CONFIG_BT_HCI_TRANSPORT_INTERFACE_TYPE gSerialMgrLpuart_c
#endif
/*! @brief hci transport interface instance number.*/
#ifndef CONFIG_BT_HCI_TRANSPORT_INTERFACE_INSTANCE
    #define CONFIG_BT_HCI_TRANSPORT_INTERFACE_INSTANCE 1
#endif
/*! @brief hci transport interface rate.
 * configurate the interface speed, for example, the default interface is h4, the speed to 115200
*/
#ifndef CONFIG_BT_HCI_TRANSPORT_INTERFACE_SPEED
    #define CONFIG_BT_HCI_TRANSPORT_INTERFACE_SPEED 115200
#endif
/*! @brief whether enable hci transport tx thread.*/
#ifndef CONFIG_BT_HCI_TRANSPORT_TX_THREAD
    #define CONFIG_BT_HCI_TRANSPORT_TX_THREAD 0
#endif
/*! @brief whether enable hci transport rx thread.*/
#ifndef CONFIG_BT_HCI_TRANSPORT_RX_THREAD
    #define CONFIG_BT_HCI_TRANSPORT_RX_THREAD 1
#endif
/*! @brief hci transport rx thread stack size.*/
#ifndef CONFIG_BT_HCI_TRANSPORT_RX_STACK_SIZE
    #define CONFIG_BT_HCI_TRANSPORT_RX_STACK_SIZE 512
#endif
/*! @brief hci transport tx thread stack size.*/
#ifndef CONFIG_BT_HCI_TRANSPORT_TX_STACK_SIZE
    #define CONFIG_BT_HCI_TRANSPORT_TX_STACK_SIZE 512
#endif
/*! @brief hci transport tx thread priority.*/
#ifndef CONFIG_BT_HCI_TRANSPORT_TX_PRIO
    #define CONFIG_BT_HCI_TRANSPORT_TX_PRIO 8
#endif
/*! @brief hci transport rx thread priority.*/
#ifndef CONFIG_BT_HCI_TRANSPORT_RX_PRIO
    #define CONFIG_BT_HCI_TRANSPORT_RX_PRIO 6
#endif

/****************** MAX MSG COUNT configuration *********************/
/*! @brief message number in message queue.*/
#ifndef CONFIG_BT_MSG_QUEUE_COUNT
    #define CONFIG_BT_MSG_QUEUE_COUNT 16
#endif

#if (defined(CONFIG_BT_RFCOMM) && (CONFIG_BT_RFCOMM > 0))
/****************** RFCOMM Protocol configuration *********************/
/*! @brief Maximum Number of RFCOMM Session supported. */
#ifndef CONFIG_BT_RFCOMM_SESSION_MAX_COUNT
    #define CONFIG_BT_RFCOMM_SESSION_MAX_COUNT     1
#endif

/*! @brief Maximum Number of RFCOMM Entity supported. */
#ifndef CONFIG_BT_RFCOMM_CLIENT_MAX_COUNT
    #define CONFIG_BT_RFCOMM_CLIENT_MAX_COUNT     1
#endif

/*! @brief Maximum Number of RFCOMM Server Entity supported. */
#ifndef CONFIG_BT_RFCOMM_SERVER_MAX_COUNT
    #define CONFIG_BT_RFCOMM_SERVER_MAX_COUNT     1
#endif

/*! @brief Enable RFCOMM Control Command. */
#ifndef CONFIG_BT_RFCOMM_ENABLE_CONTROL_CMD
    #define CONFIG_BT_RFCOMM_ENABLE_CONTROL_CMD   1
#endif
#endif /*(defined(CONFIG_BT_RFCOMM) && (CONFIG_BT_RFCOMM > 0))*/

#if (defined(CONFIG_BT_SPP) && (CONFIG_BT_SPP > 0))
/******************* SPP Profile configuration **********************/
/*! @brief Maximum Number of SPP Entity supported. */
#ifndef CONFIG_BT_SPP_MAX_CONN
#define CONFIG_BT_SPP_MAX_CONN (CONFIG_BT_RFCOMM_SERVER_MAX_COUNT + CONFIG_BT_RFCOMM_CLIENT_MAX_COUNT)
#endif

/*! @brief Maximum Number of SPP Server Entity supported. */
#ifndef CONFIG_BT_SPP_SERVER_MAX_COUNT
#define CONFIG_BT_SPP_SERVER_MAX_COUNT CONFIG_BT_RFCOMM_SERVER_MAX_COUNT
#endif

/*! @brief Enable SPP Control Command. */
#ifndef CONFIG_BT_SPP_ENABLE_CONTROL_CMD
    #define CONFIG_BT_SPP_ENABLE_CONTROL_CMD CONFIG_BT_RFCOMM_ENABLE_CONTROL_CMD
#endif
#endif /*(defined(CONFIG_BT_SPP) && (CONFIG_BT_SPP > 0))*/

/*! @}*/
#endif /* __BLE_CONFIG_H__ */
