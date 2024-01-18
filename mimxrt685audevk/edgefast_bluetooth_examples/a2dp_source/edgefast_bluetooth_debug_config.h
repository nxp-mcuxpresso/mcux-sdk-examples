/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __BLE_DEBUG_CONFIG_H__
#define __BLE_DEBUG_CONFIG_H__
/*!
 * @brief Bluetooth Configuration
 * @defgroup bt_config Bluetooth Configuration
 * @{
 */
#ifndef CONFIG_BT_LOG
	#define CONFIG_BT_LOG 1
#endif
   
#ifndef CONFIG_BT_LOG_LEGACY
	#define CONFIG_BT_LOG_LEGACY 1
#endif
#if (defined(CONFIG_BT_LOG) && (CONFIG_BT_LOG > 0))
    #if !((defined(CONFIG_BT_LOG_LEGACY) && (CONFIG_BT_LOG_LEGACY > 0)))
        #error CONFIG_BT_LOG depends on CONFIG_BT_LOG_LEGACY.
    #endif
#endif
 
#ifdef CONFIG_BT_LOG
/*Bluetooth logging*/
	#ifndef CONFIG_BT_LOG_LEGACY
		#define CONFIG_BT_LOG_LEGACY 1
	#endif/*CONFIG_BT_LOG_LEGACY*/

#ifdef CONFIG_BT_LOG_LEGACY
/*Bluetooth legacy logging options*/
/*common*/
/*! @brief Bluetooth HCI driver debug
 * This option enables debug support for the active
 * Bluetooth HCI driver, including the Controller-side HCI layer
 * when included in the build.
 */
	#ifndef CONFIG_BT_DEBUG_HCI_DRIVER
		#define CONFIG_BT_DEBUG_HCI_DRIVER 0
	#endif/*CONFIG_BT_DEBUG_HCI_DRIVER*/
	
/*! @brief Bluetooth Resolvable Private Address (RPA) debug
 * This option enables debug support for the Bluetooth
 * Resolvable Private Address (RPA) generation and resolution.
 */
	#ifdef CONFIG_BT_DEBUG_RPA
		#define CONFIG_BT_DEBUG_RPA 0
	#endif	
	#if (defined(CONFIG_BT_DEBUG_RPA) && (CONFIG_BT_DEBUG_RPA > 0))
		#if !((defined(CONFIG_BT_RPA) && (CONFIG_BT_RPA > 0)))
			#error CONFIG_BT_DEBUG_RPA depends on CONFIG_BT_RPA.
		#endif
	#endif/*CONFIG_BT_DEBUG_RPA*/
	
/*AUDIO*/
/*AICS*/	
/*! @brief Audio Input Control Service debug
 * This option enables debug support for the Bluetooth
 * Resolvable Private Address (RPA) generation and resolution.
 */
	#ifndef CONFIG_BT_DEBUG_AICS
		#define CONFIG_BT_DEBUG_AICS 0
	#endif/*CONFIG_BT_DEBUG_AICS*/	
	
/*! @brief Audio Input Control Service client debug
 * Use this option to enable Audio Input Control Service client debug
 * logs for the Bluetooth Audio functionality.
 */	
	#ifndef CONFIG_BT_DEBUG_AICS_CLIENT
		#define CONFIG_BT_DEBUG_AICS_CLIENT 0
	#endif/*CONFIG_BT_DEBUG_AICS_CLIENT*/

/*BAP*/	
/*! @brief Basic Audio Profile Stream debug
 * Use this option to enable Basic Audio Profile Stream debug logs.
 */	
	#ifndef CONFIG_BT_BAP_DEBUG_STREAM
		#define CONFIG_BT_BAP_DEBUG_STREAM 0
	#endif
	#if (defined(CONFIG_BT_BAP_DEBUG_STREAM) && (CONFIG_BT_BAP_DEBUG_STREAM > 0))
		#if !((defined(CONFIG_BT_BAP_STREAM) && (CONFIG_BT_BAP_STREAM > 0)))
			#error CONFIG_BT_BAP_DEBUG_STREAM depends on CONFIG_BT_BAP_STREAM.
		#endif
	#endif/*CONFIG_BT_BAP_DEBUG_STREAM*/	

/*! @brief Audio Stream Control Service debug
 * Use this option to enable Audio Stream Control Service debug logs for
 * the Bluetooth Audio functionality.
 */		
	#ifndef CONFIG_BT_DEBUG_ASCS
		#define CONFIG_BT_DEBUG_ASCS 0
	#endif
	#if (defined(CONFIG_BT_DEBUG_ASCS) && (CONFIG_BT_DEBUG_ASCS > 0))
		#if !((defined(CONFIG_BT_ASCS) && (CONFIG_BT_ASCS > 0)))
			#error CONFIG_BT_DEBUG_ASCS depends on CONFIG_BT_ASCS.
		#endif
	#endif/*CONFIG_BT_DEBUG_ASCS*/	

/*! @brief Bluetooth Audio Unicast Server debug
 * Use this option to enable Bluetooth Audio Unicast Server debug logs
 * for the Bluetooth Audio functionality.
 */	
	#ifndef CONFIG_BT_DEBUG_BAP_UNICAST_SERVER
		#define CONFIG_BT_DEBUG_BAP_UNICAST_SERVER 0
	#endif
	#if (defined(CONFIG_BT_DEBUG_BAP_UNICAST_SERVER) && (CONFIG_BT_DEBUG_BAP_UNICAST_SERVER > 0))
		#if !((defined(CONFIG_BT_BAP_UNICAST_SERVER) && (CONFIG_BT_BAP_UNICAST_SERVER > 0)))
			#error CONFIG_BT_DEBUG_BAP_UNICAST_SERVER depends on CONFIG_BT_BAP_UNICAST_SERVER.
		#endif
	#endif/*CONFIG_BT_DEBUG_BAP_UNICAST_SERVER*/	

/*! @brief Basic Audio Profile client debug
 * Use this option to enable Basic Audio Profile debug logs for the
 * Bluetooth Audio functionality.
 */		
	#ifndef CONFIG_BT_DEBUG_BAP_UNICAST_CLIENT
		#define CONFIG_BT_DEBUG_BAP_UNICAST_CLIENT 0
	#endif
	#if (defined(CONFIG_BT_DEBUG_BAP_UNICAST_CLIENT) && (CONFIG_BT_DEBUG_BAP_UNICAST_CLIENT > 0))
		#if !((defined(CONFIG_BT_BAP_UNICAST_CLIENT) && (CONFIG_BT_BAP_UNICAST_CLIENT > 0)))
			#error CONFIG_BT_DEBUG_BAP_UNICAST_CLIENT depends on CONFIG_BT_BAP_UNICAST_CLIENT.
		#endif
	#endif/*CONFIG_BT_DEBUG_BAP_UNICAST_CLIENT*/

/*! @brief Bluetooth Audio Broadcast Source debug
 * Use this option to enable Bluetooth Audio Broadcast Source debug logs
 * for the Bluetooth Audio functionality.
 */		
	#ifndef CONFIG_BT_DEBUG_BAP_BROADCAST_SOURCE
		#define CONFIG_BT_DEBUG_BAP_BROADCAST_SOURCE 0
	#endif
	#if (defined(CONFIG_BT_DEBUG_BAP_BROADCAST_SOURCE) && (CONFIG_BT_DEBUG_BAP_BROADCAST_SOURCE > 0))
		#if !((defined(CONFIG_BT_BAP_BROADCAST_SOURCE) && (CONFIG_BT_BAP_BROADCAST_SOURCE > 0)))
			#error CONFIG_BT_DEBUG_BAP_BROADCAST_SOURCE depends on CONFIG_BT_BAP_BROADCAST_SOURCE.
		#endif
	#endif/*CONFIG_BT_DEBUG_BAP_BROADCAST_SOURCE*/	

/*! @brief Bluetooth Audio Broadcast Sink debug
 * Use this option to enable Bluetooth Audio Broadcast Sink debug logs
 * for the Bluetooth Audio functionality.
 */	
	#ifndef CONFIG_BT_DEBUG_BAP_BROADCAST_SINK
		#define CONFIG_BT_DEBUG_BAP_BROADCAST_SINK 0
	#endif
	#if (defined(CONFIG_BT_DEBUG_BAP_BROADCAST_SINK) && (CONFIG_BT_DEBUG_BAP_BROADCAST_SINK > 0))
		#if !((defined(CONFIG_BT_BAP_BROADCAST_SINK) && (CONFIG_BT_BAP_BROADCAST_SINK > 0)))
			#error CONFIG_BT_DEBUG_BAP_BROADCAST_SINK depends on CONFIG_BT_BAP_BROADCAST_SINK.
		#endif
	#endif/*CONFIG_BT_DEBUG_BAP_BROADCAST_SINK*/	

/*! @brief Broadcast Audio Scan Service debug
 * Use this option to enable Broadcast Audio Scan Service debug logs for
 * the Bluetooth Audio functionality.
 */	
	#ifndef CONFIG_BT_DEBUG_BAP_SCAN_DELEGATOR
		#define CONFIG_BT_DEBUG_BAP_SCAN_DELEGATOR 0
	#endif
	#if (defined(CONFIG_BT_DEBUG_BAP_SCAN_DELEGATOR) && (CONFIG_BT_DEBUG_BAP_SCAN_DELEGATOR > 0))
		#if !((defined(CONFIG_BT_BAP_SCAN_DELEGATOR) && (CONFIG_BT_BAP_SCAN_DELEGATOR > 0)))
			#error CONFIG_BT_DEBUG_BAP_SCAN_DELEGATOR depends on CONFIG_BT_BAP_SCAN_DELEGATOR.
		#endif
	#endif/*CONFIG_BT_DEBUG_BAP_SCAN_DELEGATOR*/

/*! @brief Broadcast Audio Scan Service client debug
 * Use this option to enable Broadcast Audio Scan Service client
 * debug logs for the Bluetooth Audio functionality.
 */	
	#ifndef CONFIG_BT_DEBUG_BAP_BROADCAST_ASSISTANT
		#define CONFIG_BT_DEBUG_BAP_BROADCAST_ASSISTANT 0
	#endif
	#if (defined(CONFIG_BT_DEBUG_BAP_BROADCAST_ASSISTANT) && (CONFIG_BT_DEBUG_BAP_BROADCAST_ASSISTANT > 0))
		#if !((defined(CONFIG_BT_BAP_BROADCAST_ASSISTANT) && (CONFIG_BT_BAP_BROADCAST_ASSISTANT > 0)))
			#error CONFIG_BT_DEBUG_BAP_BROADCAST_ASSISTANT depends on CONFIG_BT_BAP_BROADCAST_ASSISTANT.
		#endif
	#endif/*CONFIG_BT_DEBUG_BAP_BROADCAST_ASSISTANT*/
	
/*CAP*/

/*! @brief Common Audio Profile debug
 * Use this option to enable CAP debug logs for the
 * Bluetooth Audio functionality.
 */		
	#ifndef CONFIG_BT_DEBUG_CAP_ACCEPTOR
		#define CONFIG_BT_DEBUG_CAP_ACCEPTOR 0
	#endif
	#if (defined(CONFIG_BT_DEBUG_CAP_ACCEPTOR) && (CONFIG_BT_DEBUG_CAP_ACCEPTOR > 0))
		#if !((defined(CONFIG_BT_CAP_ACCEPTOR) && (CONFIG_BT_CAP_ACCEPTOR > 0)))
			#error CONFIG_BT_DEBUG_CAP_ACCEPTOR depends on CONFIG_BT_CAP_ACCEPTOR.
		#endif
	#endif/*CONFIG_BT_DEBUG_CAP_ACCEPTOR*/	

/*! @brief Common Audio Profile Initiator debug
 * Use this option to enable CAP Initiator debug logs for the
 * Bluetooth Audio functionality.
 */		
	#ifndef CONFIG_BT_DEBUG_CAP_INITIATOR
		#define CONFIG_BT_DEBUG_CAP_INITIATOR 0
	#endif
	#if (defined(CONFIG_BT_DEBUG_CAP_INITIATOR) && (CONFIG_BT_DEBUG_CAP_INITIATOR > 0))
		#if !((defined(CONFIG_BT_CAP_INITIATOR) && (CONFIG_BT_CAP_INITIATOR > 0)))
			#error CONFIG_BT_DEBUG_CAP_INITIATOR depends on CONFIG_BT_CAP_INITIATOR.
		#endif
	#endif/*CONFIG_BT_DEBUG_CAP_INITIATOR*/
	
/*CSIP*/

/*! @brief Coordinated Set Identification Service debug
 * Use this option to enable Coordinated Set Identification Service debug
 * logs for the Bluetooth Audio functionality.
 */	
	#ifndef CONFIG_BT_DEBUG_CSIP_SET_MEMBER
		#define CONFIG_BT_DEBUG_CSIP_SET_MEMBER 0
	#endif/*CONFIG_BT_DEBUG_CAP_INITIATOR*/

/*! @brief Coordinated Set Identification Profile Set Coordinator debug
 * Use this option to enable Coordinated Set Identification Profile
 * Set Coordinator debug logs for the Bluetooth Audio functionality.
 */	
	#ifndef CONFIG_BT_DEBUG_CSIP_SET_COORDINATOR
		#define CONFIG_BT_DEBUG_CSIP_SET_COORDINATOR 0
	#endif/*CONFIG_BT_DEBUG_CSIP_SET_COORDINATOR*/

/*! @brief Coordinated Set Identification Profile crypto functions debug
 * Use this option to enable Coordinated Set Identification Profile
 * crypto functions debug logs for the Bluetooth Audio functionality.
 */		
	#ifndef CONFIG_BT_DEBUG_CSIP_SET_MEMBER_CRYPTO
		#define CONFIG_BT_DEBUG_CSIP_SET_MEMBER_CRYPTO 0
	#endif
	#if ((defined(CONFIG_BT_DEBUG_CSIP_SET_MEMBER_CRYPTO) && (CONFIG_BT_DEBUG_CSIP_SET_MEMBER_CRYPTO > 0)))
		#if !((defined(CONFIG_BT_CSIP_SET_COORDINATOR) && (CONFIG_BT_CSIP_SET_COORDINATOR > 0)) || \
			(defined(CONFIG_BT_CSIP_SET_MEMBER) && (CONFIG_BT_CSIP_SET_MEMBER > 0)))
				#error CONFIG_BT_DEBUG_CSIP_SET_MEMBER_CRYPTO depends on CONFIG_BT_CSIP_SET_COORDINATOR or CONFIG_BT_CSIP_SET_MEMBER.
		#endif
	#endif/*CONFIG_BT_DEBUG_CSIP_SET_MEMBER_CRYPTO*/
	
/*HAS*/

/*! @brief Hearing Access Service debug
 * This option enables enables Hearing Access Service debug logs.
 */	
	#ifndef CONFIG_BT_DEBUG_HAS
		#define CONFIG_BT_DEBUG_HAS 0
	#endif/*CONFIG_BT_DEBUG_HAS*/

/*! @brief Hearing Access Service Client debug
 * This option enables enables Hearing Access Service Client debug logs.
 */		
	#ifndef CONFIG_BT_DEBUG_HAS_CLIENT
		#define CONFIG_BT_DEBUG_HAS_CLIENT 0
	#endif
	#if (defined(CONFIG_BT_DEBUG_HAS_CLIENT) && (CONFIG_BT_DEBUG_HAS_CLIENT > 0))
		#if !((defined(CONFIG_BT_HAS_CLIENT) && (CONFIG_BT_HAS_CLIENT > 0)))
			#error CONFIG_BT_DEBUG_HAS_CLIENT depends on CONFIG_BT_HAS_CLIENT.
		#endif
	#endif/*CONFIG_BT_DEBUG_HAS_CLIENT*/
	
/*MCS*/

/*! @brief Media Control Service debug
 * Use this option to enable Media Control Service debug logs for the
 * Bluetooth Audio functionality.
 */	
	#ifndef CONFIG_BT_DEBUG_MCS
		#define CONFIG_BT_DEBUG_MCS 0
	#endif/*CONFIG_BT_DEBUG_MCS*/

/*! @brief Media Control Client debug
 * Use this option to enable Media Control Client debug logs for the
 * Bluetooth Audio functionality.
 */		
	#ifndef CONFIG_BT_DEBUG_MCC
		#define CONFIG_BT_DEBUG_MCC 0
	#endif/*CONFIG_BT_DEBUG_MCC*/
	
/*MCTL*/

/*! @brief Media control debug
 * Use this option to enable Media control debug logs
 */	
	#ifndef CONFIG_MCTL_DEBUG
		#define CONFIG_MCTL_DEBUG 0
	#endif/*CONFIG_MCTL_DEBUG*/
	
/*MICP*/

/*! @brief Microphone Control Profile Microphone Device debug
 * Use this option to enable Microphone Control Profile
 * Microphone Device debug logs for the Bluetooth Audio functionality.
 */		
	#ifndef CONFIG_BT_DEBUG_MICP_MIC_DEV
		#define CONFIG_BT_DEBUG_MICP_MIC_DEV 0
	#endif/*CONFIG_BT_DEBUG_MICP_MIC_DEV*/

/*! @brief Microphone Control Profile Microphone Controller debug
 * Use this option to enable Microphone Control Profile Microphone
 * Controller debug logs for the Bluetooth Audio functionality.
 */		
	#ifndef CONFIG_BT_DEBUG_MICP_MIC_CTLR
		#define CONFIG_BT_DEBUG_MICP_MIC_CTLR 0
	#endif/*CONFIG_BT_DEBUG_MICP_MIC_CTLR*/
	
/*MPL*/

/*! @brief Media player debug
 * Enables debug logs for the media player
 */	
	#ifndef CONFIG_BT_DEBUG_MPL
		#define CONFIG_BT_DEBUG_MPL 0
	#endif/*CONFIG_BT_DEBUG_MPL*/
	
/*PACS*/

/*! @brief Published Audio Capabilities Service debug
 * Use this option to enable Published Audio Capabilities Service debug
 * logs for the Bluetooth Audio functionality.
 */	
	#ifndef CONFIG_BT_DEBUG_PACS
		#define CONFIG_BT_DEBUG_PACS 0
	#endif/*CONFIG_BT_DEBUG_PACS*/
	
/*TBS*/

/*! @brief Telephone Bearer Service debug
 * Use this option to enable Telephone Bearer Service debug logs for the
 * Bluetooth Audio functionality.
 */	
	#ifndef CONFIG_BT_DEBUG_TBS
		#define CONFIG_BT_DEBUG_TBS 0
	#endif/*CONFIG_BT_DEBUG_TBS*/

/*! @brief Telephone Bearer Service client debug
 * Use this option to enable Telephone Bearer Service client debug logs
 * for the Bluetooth Audio functionality.
 */		
	#ifndef CONFIG_BT_DEBUG_TBS_CLIENT
		#define CONFIG_BT_DEBUG_TBS_CLIENT 0
	#endif/*CONFIG_BT_DEBUG_TBS_CLIENT*/
	
/*VCP*/

/*! @brief Volume Control Profile Volume Renderer debug
 * Use this option to enable Volume Control Profile debug logs for the
 * Bluetooth Audio functionality.
 */	
	#ifndef CONFIG_BT_DEBUG_VCP_VOL_REND
		#define CONFIG_BT_DEBUG_VCP_VOL_REND 0
	#endif/*CONFIG_BT_DEBUG_VCP_VOL_REND*/

/*! @brief Volume Control Profile Volume Controller debug
 * Use this option to enable Volume Control Profile Volume Controller
 * debug logs for the Bluetooth Audio functionality.
 */	
	#ifndef CONFIG_BT_DEBUG_VCP_VOL_CTLR
		#define CONFIG_BT_DEBUG_VCP_VOL_CTLR 0
	#endif/*CONFIG_BT_DEBUG_VCP_VOL_CTLR*/
	
/*VOCS*/

/*! @brief Volume Offset Control Service debug
 * Use this option to enable Volume Offset Control Service debug logs for
 * the Bluetooth Audio functionality.
 */	
	#ifndef CONFIG_BT_DEBUG_VOCS
		#define CONFIG_BT_DEBUG_VOCS 0
	#endif/*CONFIG_BT_DEBUG_VOCS*/

/*! @brief Volume Offset Control Service client debug
 * Use this option to enable Volume Offset Control Service client debug
 * logs for the Bluetooth Audio functionality.
 */		
	#ifndef CONFIG_BT_DEBUG_VOCS_CLIENT
		#define CONFIG_BT_DEBUG_VOCS_CLIENT 0
	#endif/*CONFIG_BT_DEBUG_VOCS_CLIENT*/
/*ENDOFAUDIO*/


/*OTHERS*/

/*CRYPTO*/

/*! @brief Bluetooth Cryptographic Toolbox debug
 * This option enables debug log output for the Bluetooth
 * Cryptographic Toolbox.
 * WARNING: This option prints out private security keys such as the Long Term Key.
 * Use of this feature in production is strongly discouraged.
 */	
	#ifndef CONFIG_BT_DEBUG_CRYPTO
		#define CONFIG_BT_DEBUG_CRYPTO 0
	#endif
	#if (defined(CONFIG_BT_DEBUG_CRYPTO) && (CONFIG_BT_DEBUG_CRYPTO > 0))
		#if !((defined(CONFIG_BT_CRYPTO) && (CONFIG_BT_CRYPTO > 0)))
			#error CONFIG_BT_DEBUG_CRYPTO depends on CONFIG_BT_CRYPTO.
		#endif
	#endif/*CONFIG_BT_DEBUG_CRYPTO*/

/*! @brief Bluetooth Attribute Protocol (ATT) debug
 * This option enables debug support for the Bluetooth
 * Attribute Protocol (ATT).
 */		
	#ifndef CONFIG_BT_DEBUG_ATT
		#define CONFIG_BT_DEBUG_ATT 0
	#endif/*CONFIG_BT_DEBUG_ATT*/

/*! @brief Bluetooth Generic Attribute Profile (GATT) debug
 * This option enables debug support for the Bluetooth
 * Generic Attribute Profile (GATT).
 */	
	#ifndef CONFIG_BT_DEBUG_GATT
		#define CONFIG_BT_DEBUG_GATT 0
	#endif/*CONFIG_BT_DEBUG_GATT*/

/*L2CAP*/

/*! @brief Bluetooth L2CAP debug
 * This option enables debug support for the Bluetooth
 * L2ACP layer.
 */	
	#ifndef CONFIG_BT_DEBUG_L2CAP
		#define CONFIG_BT_DEBUG_L2CAP 0
	#endif/*CONFIG_BT_DEBUG_L2CAP*/

/*HOST*/

/*! @brief Bluetooth Direction Finding debug
 * This option enables debug support for Bluetooth Direction Finding
 */	
	#ifndef CONFIG_BT_DEBUG_DF
		#define CONFIG_BT_DEBUG_DF 0
	#endif/*CONFIG_BT_DEBUG_L2CAP*/	

/*! @brief Bluetooth storage debug
 * This option enables debug support for Bluetooth storage.
 */		
	#ifndef CONFIG_BT_DEBUG_SETTINGS
		#define CONFIG_BT_DEBUG_SETTINGS 0
	#endif
	#if (defined(CONFIG_BT_DEBUG_SETTINGS) && (CONFIG_BT_DEBUG_SETTINGS > 0))
		#if !((defined(CONFIG_BT_SETTINGS) && (CONFIG_BT_SETTINGS > 0)))
			#error CONFIG_BT_DEBUG_SETTINGS depends on CONFIG_BT_SETTINGS.
		#endif
	#endif/*CONFIG_BT_DEBUG_SETTINGS*/

/*! @brief Bluetooth HCI core debug
 * This option enables debug support for Bluetooth HCI core.
 */		
	#ifndef CONFIG_BT_DEBUG_HCI_CORE
		#define CONFIG_BT_DEBUG_HCI_CORE 0
	#endif/*CONFIG_BT_DEBUG_HCI_CORE*/

/*! @brief Bluetooth connection debug
 * This option enables debug support for Bluetooth connection handling.
 */		
	#ifndef CONFIG_BT_DEBUG_CONN
		#define CONFIG_BT_DEBUG_CONN 0
	#endif
	#if (defined(CONFIG_BT_DEBUG_CONN) && (CONFIG_BT_DEBUG_CONN > 0))
		#if !((defined(CONFIG_BT_CONN) && (CONFIG_BT_CONN > 0)) || (defined(CONFIG_BT_ISO) && (CONFIG_BT_ISO > 0)))
			#error CONFIG_BT_DEBUG_CONN depends on CONFIG_BT_CONN or CONFIG_BT_ISO.
		#endif
	#endif/*CONFIG_BT_DEBUG_CONN*/

/*! @brief ISO channel debug
 * Use this option to enable ISO channels debug logs for the 
 * Bluetooth Audio functionality.
 */		
	#ifndef CONFIG_BT_DEBUG_ISO
		#define CONFIG_BT_DEBUG_ISO 0
	#endif/*CONFIG_BT_DEBUG_ISO*/

/*! @brief Bluetooth security keys debug
 * This option enables debug support for the handling of 
 * Bluetooth security keys.
 */	
	#ifndef CONFIG_BT_DEBUG_KEYS
		#define CONFIG_BT_DEBUG_KEYS 0
	#endif
	#if (defined(CONFIG_BT_DEBUG_KEYS) && (CONFIG_BT_DEBUG_KEYS > 0))
		#if !((defined(CONFIG_BT_HCI_HOST) && (CONFIG_BT_HCI_HOST > 0)))
			#error CONFIG_BT_DEBUG_KEYS depends on CONFIG_BT_HCI_HOST.
		#endif
		
		#if !((defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0)))
			#error CONFIG_BT_DEBUG_KEYS depends on CONFIG_BT_SMP.
		#endif
	#endif/*CONFIG_BT_DEBUG_KEYS*/

/*! @brief Bluetooth Security Manager Protocol (SMP) debug
 * This option enables debug support for the Bluetooth 
 * Security Manager Protocol (SMP).
 */		
	#ifndef CONFIG_BT_DEBUG_SMP
		#define CONFIG_BT_DEBUG_SMP 0
	#endif
	#if (defined(CONFIG_BT_DEBUG_SMP) && (CONFIG_BT_DEBUG_SMP > 0))
		#if !((defined(CONFIG_BT_HCI_HOST) && (CONFIG_BT_HCI_HOST > 0)))
			#error CONFIG_BT_DEBUG_SMP depends on CONFIG_BT_HCI_HOST.
		#endif
		
		#if !((defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0)))
			#error CONFIG_BT_DEBUG_SMP depends on CONFIG_BT_SMP.
		#endif
	#endif/*CONFIG_BT_DEBUG_SMP*/

/*! @brief Bluetooth Services debug
 * This option enables debug support for the Bluetooth Services.
 */			
	#ifndef CONFIG_BT_DEBUG_SERVICE
		#define CONFIG_BT_DEBUG_SERVICE 0
	#endif
	#if (defined(CONFIG_BT_DEBUG_SERVICE) && (CONFIG_BT_DEBUG_SERVICE > 0))
		#if !((defined(CONFIG_BT_CONN) && (CONFIG_BT_CONN > 0)))
			#error CONFIG_BT_DEBUG_SERVICE depends on CONFIG_BT_CONN.
		#endif
	#endif/*CONFIG_BT_DEBUG_SERVICE*/
/*ENDOFOTHERS*/

/*BR/EDR*/
/*! @brief Bluetooth RFCOMM debug
 * This option enables debug support for the Bluetooth RFCOMM layer.
 */		
	#ifndef CONFIG_BT_DEBUG_RFCOMM
		#define CONFIG_BT_DEBUG_RFCOMM 0
	#endif
	#if (defined(CONFIG_BT_DEBUG_RFCOMM) && (CONFIG_BT_DEBUG_RFCOMM > 0))
		#if !((defined(CONFIG_BT_RFCOMM) && (CONFIG_BT_RFCOMM > 0)))
			#error CONFIG_BT_DEBUG_RFCOMM depends on CONFIG_BT_RFCOMM.
		#endif
	#endif/*CONFIG_BT_DEBUG_RFCOMM*/

/*! @brief Bluetooth Hands Free Profile (HFP) debug
 * This option enables debug support for the Bluetooth Hands Free Profile (HFP).
 */		
	#ifndef CONFIG_BT_DEBUG_HFP_HF
		#define CONFIG_BT_DEBUG_HFP_HF 0
	#endif
	#if (defined(CONFIG_BT_DEBUG_HFP_HF) && (CONFIG_BT_DEBUG_HFP_HF > 0))
		#if !((defined(CONFIG_BT_HFP_HF) && (CONFIG_BT_HFP_HF > 0)))
			#error CONFIG_BT_DEBUG_HFP_HF depends on CONFIG_BT_HFP_HF.
		#endif
	#endif/*CONFIG_BT_DEBUG_HFP_HF*/

/*! @brief Bluetooth AVDTP debug
 * This option enables debug support for the Bluetooth AVDTP.
 */		
	#ifndef CONFIG_BT_DEBUG_AVDTP
		#define CONFIG_BT_DEBUG_AVDTP 0
	#endif
	#if (defined(CONFIG_BT_DEBUG_AVDTP) && (CONFIG_BT_DEBUG_AVDTP > 0))
		#if !((defined(CONFIG_BT_AVDTP) && (CONFIG_BT_AVDTP > 0)))
			#error CONFIG_BT_DEBUG_AVDTP depends on CONFIG_BT_AVDTP.
		#endif
	#endif/*CONFIG_BT_DEBUG_AVDTP*/

/*! @brief Bluetooth A2DP debug
 * This option enables debug support for the Bluetooth  A2DP profile.
 */		
	#ifndef CONFIG_BT_DEBUG_A2DP
		#define CONFIG_BT_DEBUG_A2DP 0
	#endif
	#if (defined(CONFIG_BT_DEBUG_A2DP) && (CONFIG_BT_DEBUG_A2DP > 0))
		#if !((defined(CONFIG_BT_A2DP) && (CONFIG_BT_A2DP > 0)))
			#error CONFIG_BT_DEBUG_A2DP depends on CONFIG_BT_A2DP.
		#endif
	#endif/*CONFIG_BT_DEBUG_A2DP*/

/*! @brief Bluetooth Service Discovery Protocol (SDP) debug
 * This option enables debug support for the Bluetooth Service Discovery Protocol (SDP).
 */		
	#ifndef CONFIG_BT_DEBUG_SDP
		#define CONFIG_BT_DEBUG_SDP 0
	#endif
	#if (defined(CONFIG_BT_DEBUG_SDP) && (CONFIG_BT_DEBUG_SDP > 0))
		#if !((defined(CONFIG_BT_BREDR) && (CONFIG_BT_BREDR > 0)))
			#error CONFIG_BT_DEBUG_SDP depends on CONFIG_BT_BREDR.
		#endif
	#endif/*CONFIG_BT_DEBUG_SDP*/
/*ENDOFBR/EDR*/

/*MESH*/
/*! @brief Debug logs
 * Use this option to enable debug logs for the Bluetooth Mesh functionality.
 */		
	#ifndef CONFIG_BT_MESH_DEBUG
		#define CONFIG_BT_MESH_DEBUG 0
	#endif/*CONFIG_BT_MESH_DEBUG*/

/*! @brief Network layer debug
 * Use this option to enable Network layer debug logs for the Bluetooth mesh functionality.
 */		
	#ifndef CONFIG_BT_MESH_DEBUG_NET
		#define CONFIG_BT_MESH_DEBUG_NET 0
	#endif/*CONFIG_BT_MESH_DEBUG_NET*/

/*! @brief Replay protection list debug
 * Use this option to enable Replay protection list debug logs
 * for the Bluetooth mesh functionality.
 */			
	#ifndef CONFIG_BT_MESH_DEBUG_RPL
		#define CONFIG_BT_MESH_DEBUG_RPL 0
	#endif/*CONFIG_BT_MESH_DEBUG_RPL*/

/*! @brief Transport layer debug
 * Use this option to enable Transport layer debug logs for the
 * Bluetooth mesh functionality.
 */		
	#ifndef CONFIG_BT_MESH_DEBUG_TRANS
		#define CONFIG_BT_MESH_DEBUG_TRANS 0
	#endif/*CONFIG_BT_MESH_DEBUG_TRANS*/

/*! @brief Beacon debug
 * Use this option to enable Beacon-related debug logs for the
 * Bluetooth mesh functionality.
 */		
	#ifndef CONFIG_BT_MESH_DEBUG_BEACON
		#define CONFIG_BT_MESH_DEBUG_BEACON 0
	#endif/*CONFIG_BT_MESH_DEBUG_BEACON*/

/*! @brief Crypto debug
 * Use this option to enable cryptographic debug logs for the
 * Bluetooth mesh functionality.
 */			
	#ifndef CONFIG_BT_MESH_DEBUG_CRYPTO
		#define CONFIG_BT_MESH_DEBUG_CRYPTO 0
	#endif/*CONFIG_BT_MESH_DEBUG_CRYPTO*/

/*! @brief Key management debug
 * Use this option to enable key management debug logs for the
 * Bluetooth mesh functionality.
 */		
	#ifndef CONFIG_BT_MESH_DEBUG_KEYS
		#define CONFIG_BT_MESH_DEBUG_KEYS 0
	#endif/*CONFIG_BT_MESH_DEBUG_KEYS*/

/*! @brief Provisioning debug
 * Use this option to enable Provisioning debug logs for the
 * Bluetooth mesh functionality.
 */		
	#ifndef CONFIG_BT_MESH_DEBUG_PROV
		#define CONFIG_BT_MESH_DEBUG_PROV 0
	#endif/*CONFIG_BT_MESH_DEBUG_PROV*/

/*! @brief Provisioner debug
 * Use this option to enable Provisioner debug logs for the
 * Bluetooth mesh functionality.
 */		
	#ifndef CONFIG_BT_MESH_DEBUG_PROVISIONER
		#define CONFIG_BT_MESH_DEBUG_PROVISIONER 0
	#endif/*CONFIG_BT_MESH_DEBUG_PROVISIONER*/

/*! @brief Provisioning device debug
 * Use this option to enable Provisioning device debug logs for the
 * Bluetooth mesh functionality.
 */	
	#ifndef CONFIG_BT_MESH_DEBUG_PROV_DEVICE
		#define CONFIG_BT_MESH_DEBUG_PROV_DEVICE 0
	#endif/*CONFIG_BT_MESH_DEBUG_PROV_DEVICE*/

/*! @brief Access layer debug
 * Use this option to enable Access layer and device composition
 * related debug logs for Bluetooth mesh.
 */		
	#ifndef CONFIG_BT_MESH_DEBUG_ACCESS
		#define CONFIG_BT_MESH_DEBUG_ACCESS 0
	#endif/*CONFIG_BT_MESH_DEBUG_ACCESS*/

/*! @brief Foundation model debug
 * Use this option to enable debug logs for the Foundation Models.
 */		
	#ifndef CONFIG_BT_MESH_DEBUG_MODEL
		#define CONFIG_BT_MESH_DEBUG_MODEL 0
	#endif/*CONFIG_BT_MESH_DEBUG_MODEL*/

/*! @brief Advertising debug
 * Use this option to enable advertising debug logs for
 * the Bluetooth mesh functionality.
 */		
	#ifndef CONFIG_BT_MESH_DEBUG_ADV
		#define CONFIG_BT_MESH_DEBUG_ADV 0
	#endif/*CONFIG_BT_MESH_DEBUG_ADV*/

/*! @brief Low Power debug
 * Use this option to enable Low Power debug logs for the
 * Bluetooth mesh functionality.
 */		
	#ifndef CONFIG_BT_MESH_DEBUG_LOW_POWER
		#define CONFIG_BT_MESH_DEBUG_LOW_POWER 0
	#endif/*CONFIG_BT_MESH_DEBUG_LOW_POWER*/

/*! @brief Friend debug
 * Use this option to enable Friend debug logs for the
 * Bluetooth mesh functionality.
 */		
	#ifndef CONFIG_BT_MESH_DEBUG_FRIEND
		#define CONFIG_BT_MESH_DEBUG_FRIEND 0
	#endif/*CONFIG_BT_MESH_DEBUG_FRIEND*/

/*! @brief Proxy debug
 * Use this option to enable Proxy protocol debug logs.
 */		
	#ifndef CONFIG_BT_MESH_DEBUG_PROXY
		#define CONFIG_BT_MESH_DEBUG_PROXY 0
	#endif/*CONFIG_BT_MESH_DEBUG_PROXY*/

/*! @brief Persistent settings debug
 * Use this option to enable persistent settings debug logs.
 */		
	#ifndef CONFIG_BT_MESH_DEBUG_SETTINGS
		#define CONFIG_BT_MESH_DEBUG_SETTINGS 0
	#endif/*CONFIG_BT_MESH_DEBUG_SETTINGS*/

/*! @brief Configuration database debug
 * Use this option to enable configuration database debug logs.
 */	
	#ifndef CONFIG_BT_MESH_DEBUG_CDB
		#define CONFIG_BT_MESH_DEBUG_CDB 0
	#endif
	#if (defined(CONFIG_BT_MESH_DEBUG_CDB) && (CONFIG_BT_MESH_DEBUG_CDB > 0))
		#if !((defined(CONFIG_BT_MESH_CDB) && (CONFIG_BT_MESH_CDB > 0)))
			#error CONFIG_BT_MESH_DEBUG_CDB depends on CONFIG_BT_MESH_CDB.
		#endif
	#endif/*CONFIG_BT_MESH_DEBUG_CDB*/

/*! @brief Configuration debug
 * Use this option to enable node configuration debug logs for the
 * Bluetooth mesh functionality.
 */		
	#ifndef CONFIG_BT_MESH_DEBUG_CFG
		#define CONFIG_BT_MESH_DEBUG_CFG 0
	#endif/*CONFIG_BT_MESH_DEBUG_CFG*/
/*ENDOFMESH*/

/*SERVICES*/
/*IAS*/
/*! @brief Immediate Alert Service Client debug
 * This option enables enables Immediate Alert Service Client debug logs.
 */	
	#ifndef CONFIG_BT_DEBUG_IAS_CLIENT
		#define CONFIG_BT_DEBUG_IAS_CLIENT 0
	#endif
	#if (defined(CONFIG_BT_DEBUG_IAS_CLIENT) && (CONFIG_BT_DEBUG_IAS_CLIENT > 0))
		#if !((defined(CONFIG_BT_IAS_CLIENT) && (CONFIG_BT_IAS_CLIENT > 0)))
			#error CONFIG_BT_DEBUG_IAS_CLIENT depends on CONFIG_BT_IAS_CLIENT.
		#endif
	#endif/*CONFIG_BT_DEBUG_IAS_CLIENT*/

/*OTS*/
/*! @brief Object Transfer Service Client debug
 * Use this option to enable Object Transfer Client debug logs.
 */	
	#ifndef CONFIG_BT_DEBUG_OTS_CLIENT
		#define CONFIG_BT_DEBUG_OTS_CLIENT 0
	#endif/*CONFIG_BT_DEBUG_OTS_CLIENT*/

/*ENDOFSERVICES*/
#endif/*CONFIG_BT_LOG_LEGACY*/

#endif/*CONFIG_BT_LOG*/
/*! @}*/
#endif