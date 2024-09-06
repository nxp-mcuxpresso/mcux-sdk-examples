
/**
 *  \file appl_main.c
 *
 *  This File contains the "main" function for the Test Application
 *  to test the Mindtree Bluetooth protocol stack.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl_main.h"
#include "appl_utils.h"

#ifdef BT_LE
#include "appl_le.h"

#ifdef BEACON_ENABLE
#include "appl_beacon.h"
#endif

#ifndef BT_GAM
#include "appl_service.h"
#else /* BT_GAM */
#include "appl_ga.h"

#include "appl_ccp_ce.h"
#include "appl_ccp_se.h"

#include "appl_mcp_ce.h"
#include "appl_mcp_se.h"

#include "appl_tmap.h"
#include "appl_hap.h"
#endif /* BT_GAM */
#endif /* BT_LE */

#ifdef BT_SUPPORT_STACK_VERSION_INFO
#include "BT_version.h"
#endif /* BT_SUPPORT_STACK_VERSION_INFO */
#ifdef ENABLE_BT_IND_RESET
#include "controller_features.h"
#endif
#ifdef MEMWATCH
#include "memwatch.h"
#endif /* MEMWATCH */

#ifdef BT_SECURITY_VU_VALIDATION
#include "BT_security.h"
#endif /* BT_SECURITY_VU_VALIDATION */

#ifdef SERIAL_BTSNOOP
#include "sbtsnoop.h"
#endif

#ifndef INVALID_VALUE
#define INVALID_VALUE 0xFFU
#endif /* INVALID_VALUE */

#if defined(APP_LOWPOWER_ENABLED) && (APP_LOWPOWER_ENABLED > 0)
#if !defined(RW610_SERIES) && !defined(RW612_SERIES)
#include "PWR_Interface.h"
#else
#include "PWR_cli.h"
#endif

#endif /* APP_LOWPOWER_ENABLED */
/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */
void appl_mps_activate_record (void);
extern void H2C_wakeup(void);
extern void H2C_sleep(void);
extern void Host_sleep(void);
int appl_debug_enabled = 1;
#ifdef BT_VENDOR_SPECIFIC_INIT
void appl_vendor_init_complete(void);
DECL_STATIC int skip;
#endif /* BT_VENDOR_SPECIFIC_INIT */

#ifdef OOB_WAKEUP
extern int sleep_host;
#endif

#ifdef SDP_DYNAMIC_DB
extern void db_add_record(void);
#endif /* SDP_DYNAMIC_DB */

#ifdef BT_SECURITY_VU_VALIDATION
static const char vu_list[] = "\n\
   0. No VU \n\
   1. KNOB Attack \n\
   2. ASBU Attack \n\
   3. Invalid PublicKey \n\
   4. Blurtooth \n\
   5. ANSSI Reflect Confval (Validate as SMP Master, IOCap set to DisplayOnly) \n\
   6. ANSSI Reflect Pubkey (Validate as SMP Master) \n\
   7. Unexpected Encryption Start \n\
   8. Unexpected Public Key (Will accept SMP pairing in Legacy Mode) \n\
   9. Keysize Overflow \n\
  10. Zero LTK install \n\
  11. DHCheck Skip \n\
  12. Non-Zero EDIV and RAND \n\
  13. Truncated L2CAP \n\
  14. Silent Length Overflow \n\
  15. Invalid L2CAP Fragment \n\
  16. ATT Sequential Deadlock (Enable PXR application to validate with TxPower READ)\n\
";
#endif /* BT_SECURITY_VU_VALIDATION  */

static const char main_options[] = " \n\
================ M A I N   M E N U ================ \n\
   0.  Exit. \n\
   1.  Refresh this Menu. \n\
 \n\
   2.  EtherMind Init. \n\
   3.  Bluetooth ON. \n\
   4.  Bluetooth OFF. \n\
 \n\
   5.  Set PIN Code. \n\
   6.  Set Device Role. \n\
 \n\
   7.  Get Free ACL Tx Queue Buffer. \n\
   8.  Set LWM/HWM for ACL Tx Queue. \n\
 \n\
   9.  Configure Service GAP Procedures \n\
   10. Set Snoop logging. \n\
 \n\
   11. HCI Operations. \n\
   12. SDP Operations. \n\
   13. SDDB Operations. \n\
   14. RFCOMM Operations. \n\
   15. MCAP Operations. \n\
   16. BNEP Operations. \n\
   17. AVDTP Operations. \n\
   18. AVCTP Operations. \n\
   19. OBEX Client Operations. \n\
   20. OBEX Server Operations. \n\
   21. L2CAP Operations. \n\
 \n\
   25. Write Storage event mask. \n\
 \n\
   30. Security Manager. \n\
   31. LE SMP. \n\
   32. LE L2CAP. \n\
   33. ATT. \n\
   34. L2CAP ECBFC. \n\
 \n\
   40. SPP Operations. \n\
   41. HFP Unit Operations. \n\
   42. HFP AG Operations. \n\
   45. DUNP DT Operations. \n\
   46. DUNP GW Operations. \n\
   47. SAP Client Operations. \n\
   48. SAP Server Operations. \n\
 \n\
   50. OPP Client Operations. \n\
   51. OPP Server Operations. \n\
   52. FTP Client Operations. \n\
   53. FTP Server Operations. \n\
   54. MAP Client Operations. \n\
   55. MAP Server Operations. \n\
   56. PBAP Client Operations. \n\
   57. PBAP Server Operations. \n\
   58. CTN Client Operations. \n\
   59. CTN Server Operations. \n\
   60. BIP Initiator Operations. \n\
   61. BIP Responder Operations. \n\
   62. SYNCP Client Operations. \n\
   63. SYNCP Server Operations. \n\
 \n\
   65. A2DP Operations. \n\
   66. AVRCP Operations. \n\
   67. HDP Operations. \n\
   68. PAN Operations. \n\
 \n\
   70. HID Device Operations. \n\
   71. HID Host Operations. \n\
 \n\
   75. DID Client Operations. \n\
   76. DID Server Operations. \n\
 \n\
   80. GATT Client Operations. \n\
   81. GATT Server Operations. \n\
 \n\
   90. BPP Sender Operations. \n\
   91. BPP Printer Operations. \n\
 \n\
  100. Simulate VU. \n\
  110. Serial BTSNOOP Operations. \n\
 \n\
  120. Low Power Operations.\n\
 \n\
  130. Reset Controller.\n\
 \n\
  201. GA CCP Client Operations. \n\
  202. GA CCP Server Operations. \n\
 \n\
  203. GA MCP Client Operations. \n\
  204. GA MCP Server Operations. \n\
 \n\
  205. GA TMAP Operations. \n\
  206. GA HAP Operations. \n\
  207. GA BASS Operations.\n\
 \n\
  210. GA Setup. \n\
 \n\
  220. BEACON Manager\n\
 \n\
  250. Wake on BLE vendor command. \n\
  251. H2C sleep vendor command. \n\
  252. H2C sleep. \n\
  253. H2C wakeup. \n\
  254. Host sleep. \n\
  255. Configure IR Inband/Outband. \n\
  256. Trigger IR & BT-FW Download. \n\
  \n\
  280. Disable Logging.\n\
  281. Enable Logging.\n\
 \n\
  300. GA Profile Options.\n\
 \n\
 Your Option ?";

/* Enable all debug on startup */
DECL_STATIC UCHAR appl_debug_enable_all;

/* BT ON Identifier */
DECL_STATIC UCHAR appl_bt_state;
#ifdef BEACON_ENABLE
static const char beacon_options[] = " \n\
================ Beacon   M E N U ================ \n\
   0.  Exit. \n\
   1.  Refresh this Menu. \n\
 \n\
   2.  Ibeacon. \n\
   3.  Eddystone Beacon \n\
 Your Option ?";
#endif

#ifdef BT_SUPPORT_ERR_IND_CALLBACK
DECL_STATIC API_RESULT appl_bluetooth_error_ind_cb
                       (
                           UINT32   module_id,
                           UINT16   error_code,
                           void   * error_msg
                       );
#endif /* BT_SUPPORT_ERR_IND_CALLBACK */


/* --------------------------------------------- Functions */
void appl_bluetooth_on_init_cb(void);
void appl_bluetooth_off_deinit_cb(void);

#ifdef OOB_WAKEUP
void Configure_sleep_wakeup (void)
{
	LOG_DEBUG("Sending Vendor command 0053 now\n");
#if defined(WIFI_IW416_BOARD_MURATA_1XK_USD) || defined (WIFI_IW416_BOARD_MURATA_1XK_M2)
	/*Deep-Sleep Config for RB3P, C2H GPIO14, H2C GPIO12*/
	UCHAR vnd_cmd[6U] = {0x03U, 0x0EU, 0x02U, 0x0CU, 0x00U, 0x00U};
#elif defined(WIFI_88W8987_BOARD_MURATA_1ZM_USD) || defined (WIFI_88W8987_BOARD_MURATA_1ZM_M2)
	/*Deep-Sleep Config for CA2, C2H GPIO20, H2C GPIO12*/
	UCHAR vnd_cmd[6U] = {0x03U, 0x14U, 0x02U, 0x0CU, 0x00U, 0x00U};
#else
	/*Deep-Sleep Config for FC, C2H GPIO19, H2C GPIO16*/
	UCHAR vnd_cmd[6U] = {0x03U, 0x13U, 0x02U, 0xffU, 0x00U, 0x00U};
#endif
	(BT_IGNORE_RETURN_VALUE) BT_hci_vendor_specific_command(0x0053U, vnd_cmd, sizeof(vnd_cmd));
}

void Host_to_controller_sleep_hci_cmnd(void)
{
	UCHAR new[3U] = {0x02U, 0x00U, 0x00U};

	LOG_DEBUG("Sending Vendor command 0023 now\n");
	(BT_IGNORE_RETURN_VALUE) BT_hci_vendor_specific_command(0x0023U, new, sizeof(new));
}
#endif

/* Application specific shutdown routine */
void appl_shutdown (void)
{
    /* Shutdown SCO path incase active */
    (BT_IGNORE_RETURN_VALUE) sco_audio_stop_pl();

#ifdef BT_LE
    appl_deinit();
#endif /* BT_LE */
}

void appl_process_term_handler(void)
{
    LOG_DEBUG("In Process Term Handler\n");

    if (BT_TRUE == appl_bt_state)
    {
        LOG_DEBUG("Turning off Bluetooth\n");

        /* Reset the Controller */
        (BT_IGNORE_RETURN_VALUE) BT_hci_reset();
        BT_sleep(1U);

        /* Turn of Bluetooth stack */
        (BT_IGNORE_RETURN_VALUE) BT_bluetooth_off();
    }

#ifdef MEMWATCH
    mwTerm();
#endif /* MEMWATCH */
}

#if (defined(CONFIG_WIFI_BLE_COEX_APP) && (CONFIG_WIFI_BLE_COEX_APP == 1)) || (defined(CONFIG_DISABLE_BLE) && (CONFIG_DISABLE_BLE == 0))
DECL_STATIC UCHAR enter_ble_menu = BT_TRUE;
#endif

#ifndef EM_PLATFORM_MAIN
int main (int argc, char **argv)
#else /* EM_PLATFORM_MAIN */
int appl_main (int argc, char **argv)
#endif /* EM_PLATFORM_MAIN */
{
    int choice;
#ifdef CLASSIC_SEC_MANAGER
    UCHAR pin[17U];
#endif
    API_RESULT retval;
    UCHAR  str[5] = "";
    UINT16 length,i,invalid_val;

#ifdef COEX_APP_SUPPORT
    static int Coexapp_Flag=0;
    if (Coexapp_Flag==0)
    {
#endif
		BT_IGNORE_UNUSED_PARAM(argc);
		BT_IGNORE_UNUSED_PARAM(argv);

        /* Initialize */
		invalid_val = 0;

#ifdef MEMWATCH
        mwInit();
#endif /* MEMWATCH */

#ifdef BT_VENDOR_SPECIFIC_INIT
        LOG_DEBUG ("Do Vendor Specific Initialization? (Yes - 1/ No - 0): ");
		scanf ("%d", &choice);
		skip = !choice;
#endif /* BT_VENDOR_SPECIFIC_INIT */

#ifndef APPL_CONFIG_DISABLE
        retval = appl_open_transport_port();
        if (retval != API_SUCCESS)
        {
            return 0;
        }

        /* Read application specific configurations */
        appl_read_config();
#endif /* APPL_CONFIG_DISABLE */

#if (defined(CONFIG_WIFI_BLE_COEX_APP) && (CONFIG_WIFI_BLE_COEX_APP == 1)) || (defined(CONFIG_DISABLE_BLE) && (CONFIG_DISABLE_BLE == 0))
        if (enter_ble_menu)
        {
            enter_ble_menu = BT_FALSE;
#endif
            /* Initialize OSAL */
            EM_os_init();
            EM_debug_init();
            EM_timer_init();
            timer_em_init();

            EM_process_term_notify(appl_process_term_handler);

            appl_bt_state = BT_FALSE;
#if (defined(CONFIG_WIFI_BLE_COEX_APP) && (CONFIG_WIFI_BLE_COEX_APP == 1)) || (defined(CONFIG_DISABLE_BLE) && (CONFIG_DISABLE_BLE == 0))
        }
#endif

#ifdef COEX_APP_SUPPORT
		Coexapp_Flag=1;
	}
#endif
    BT_LOOP_FOREVER()
    {
        LOG_DEBUG("\n");
        printf("%s", main_options);
        scanf ("%s", str);
        length = (UINT16)BT_str_len (str);
        invalid_val = 0;

        for (i=0; i<length; i++)
        {
            if (!((str[i] >= '0') && (str[i] <= '9')))
            {
                LOG_DEBUG ("Invalid option\n");
                choice = 0xFFFFU;
                invalid_val = 1;
                break;
            }
        }
        if(invalid_val == 0)
        {
            choice = appl_str_to_num(str,length);
            LOG_DEBUG("choice is %d\n",choice);
        }

        switch(choice)
        {
        case 0:

            /* Confirm on exiting */
            LOG_DEBUG("Exit from Buetooth Main Software? (0/1):");
            scanf("%d", &choice);

            if (1 != choice)
            {
                break;
            }

            LOG_DEBUG("\nExiting from Mindtree Bluetooth Main Software ... \n\n");

#ifdef MEMWATCH
            mwTerm();
#endif /* MEMWATCH */
            return 0;

        case 1:
            LOG_DEBUG("\nRefreshing ...\n\n");
            break;

        /* ---------- Intialization and Shutdown  --------------- */
        case 2:
            LOG_DEBUG("Initializing EtherMind ...\n");
            BT_ethermind_init();
            LOG_DEBUG("EtherMind Stack Successfully Initialized!");

#ifdef BT_ENABLE_DISABLE_RUNTIME_DEBUG
            if (BT_TRUE == appl_debug_enable_all)
            {
                BT_enable_module_debug_flag(0xFFFFFFFFU);
            }
#endif /* BT_ENABLE_DISABLE_RUNTIME_DEBUG */

        #ifdef BT_VENDOR_SPECIFIC_INIT
            if (0 == skip)
            {
            #ifdef BT_SUPPORT_CONTROLLER_INIT_SKIP
                (BT_IGNORE_RETURN_VALUE) BT_hci_set_controller_init (BT_FALSE);
            #endif /* BT_SUPPORT_CONTROLLER_INIT_SKIP */
            }
        #endif /* BT_VENDOR_SPECIFIC_INIT */

            /* Disable Snoop Logging */
            (BT_IGNORE_RETURN_VALUE) BT_snoop_logging_disable();

            /* Register UpperLayer Callbacks with Common PL Layer */
#ifdef BT_COMMON_PL_SUPPORT_UL_CB
            BT_ethermind_register_ul_cb_pl
            (
                appl_bluetooth_on_init_cb,
                appl_bluetooth_off_deinit_cb
            );
#else /* BT_COMMON_PL_SUPPORT_UL_CB */

            /* Enable Error Indication Callback */
#ifdef BT_SUPPORT_ERR_IND_CALLBACK
            (BT_IGNORE_RETURN_VALUE)BT_ethermind_register_error_indication_callback
                                    (
                                        appl_bluetooth_error_ind_cb
                                    );
#endif /* BT_SUPPORT_ERR_IND_CALLBACK */


#ifdef BT_LE
            appl_init();
#endif /* BT_LE */
#endif /* BT_COMMON_PL_SUPPORT_UL_CB */
            break;

        case 3:
            LOG_DEBUG("Performing Bluetooth ON ...\n");
#ifndef HCI_LITE
            (BT_IGNORE_RETURN_VALUE) BT_hci_register_error_indication_callback
            (
                appl_hci_error_indication_callback
            );
#else
            LOG_DEBUG("HCI_LITE flag is defined\n");
#endif /* HCI_LITE */

            retval = BT_bluetooth_on
                     (
                         appl_hci_event_indication_callback,
                         appl_bluetooth_on_complete,
                         APPL_GAP_DEVICE_NAME
                     );

            if (API_SUCCESS != retval)
            {
                LOG_DEBUG("FAILED ! Reason = 0x%04X\n", retval);
            }

        #ifdef BT_VENDOR_SPECIFIC_INIT
            else if (0 == skip)
            {
                app_vendor_specific_init(appl_vendor_init_complete);
            }
        #endif /* BT_VENDOR_SPECIFIC_INIT */
            break;

        case 4:
            if (BT_TRUE == appl_bt_state)
            {
                /* Reset the Controller */
                (BT_IGNORE_RETURN_VALUE) BT_hci_reset();
                BT_sleep(1U);

                LOG_DEBUG("EtherMind: Bluetooth OFF ...\n");
                (BT_IGNORE_RETURN_VALUE) BT_bluetooth_off ();

#ifndef BT_COMMON_PL_SUPPORT_UL_CB
                /* Shutdown any application/platform specific components */
                appl_shutdown();
#endif /* BT_COMMON_PL_SUPPORT_UL_CB */

                appl_bt_state = BT_FALSE;
            }

            break;

        case 5:
#ifdef CLASSIC_SEC_MANAGER
            LOG_DEBUG("\nEnter PIN code\n");
            scanf("%s", pin);

            retval = BT_sm_default_pin_code
                     (pin, (UCHAR)BT_str_len(pin));

            if (retval != API_SUCCESS)
            {
                LOG_DEBUG("*** FAILED to Set PIN Code.\n");
            }
            else
            {
                LOG_DEBUG("Bluetooth PIN Set Successfully.\n");
            }
            LOG_DEBUG("\n");
#endif /* CLASSIC_SEC_MANAGER */
            break;

        case 6:
#ifdef BR_EDR_HCI
            LOG_DEBUG("Choose Device Role:\n");
            LOG_DEBUG("  [0] Master\n");
            LOG_DEBUG("  [1] Slave\n");
            LOG_DEBUG("Your Choice = "); fflush (stdout);
            scanf("%d", &choice);
            if ((choice < 0) || (choice > 1))
            {
                LOG_DEBUG("*** Invalid Choice for Device Role - %d\n", choice);
                break;
            }

            /* Set Device Role */
            LOG_DEBUG("Setting Device Role ... "); fflush(stdout);
            if (0 == choice)
            {
                retval = BT_hci_set_device_role (NULL, BT_DEVICE_ROLE_MASTER);
            }
            else
            {
                retval = BT_hci_set_device_role (NULL, BT_DEVICE_ROLE_SLAVE);
            }

            if (retval != API_SUCCESS)
            {
                LOG_DEBUG("FAILED ! Reason = 0x%04X\n", retval);
            }
            else
            {
                LOG_DEBUG("SUCCEEDED");
            }
            LOG_DEBUG("\n");
#else
            LOG_DEBUG("Feature flag BR_EDR_HCI is not defined\n");
#endif /* BR_EDR_HCI */
            break;

#if !defined(RW610_SERIES) && !defined(RW612_SERIES)
        /* ---------- Core Protocol Layer Testing --------------- */
        case 7:
#ifdef L2CAP_TX_QUEUE_FLOW
            /* Get number of available buffers in ACL Transmission Queue */
            {
                UINT32 num_buffer;

                LOG_DEBUG("Number of Free ACL Tx Queue Buffer = ");
                fflush(stdout);

                retval = l2cap_get_tx_queue_num_buffer (&num_buffer);
                if (retval != API_SUCCESS)
                {
                    LOG_DEBUG("FAILED ! Reason = 0x%04X\n", retval);
                }
                else
                {
                    LOG_DEBUG("%lu\n", num_buffer);
                }
            }
#else /* L2CAP_TX_QUEUE_FLOW */
            printf("L2CAP_TX_QUEUE_FLOW is not defined\n");
#endif /* L2CAP_TX_QUEUE_FLOW */
            break;

        case 8:
#ifdef L2CAP_TX_QUEUE_FLOW
            /* Set LWM & HWM for ACL Transmission Queue Flow Control */
            {
                UINT16 lwm, hwm;

                LOG_DEBUG("Enter Low Water Mark = "); fflush(stdout);
                scanf("%d", &choice);
                if (choice < 0)
                {
                    LOG_DEBUG("*** Invalid Choice for Low Water Mark - %d\n",
                    choice);

                    break;
                }
                else
                {
                    lwm = (UINT16) choice;
                }

                LOG_DEBUG("Enter High Water Mark = "); fflush(stdout);
                scanf("%d", &choice);
                if (choice < 0)
                {
                    LOG_DEBUG("*** Invalid Choice for High Water Mark - %d\n",
                    choice);

                    break;
                }
                else
                {
                    hwm = (UINT16) choice;
                }

                LOG_DEBUG("Setting LWM & HWM for ACL Tx Queue ... ");
                fflush(stdout);

                retval = l2cap_set_tx_queue_flow_limits (lwm, hwm);
                if (retval != API_SUCCESS)
                {
                    LOG_DEBUG("FAILED ! Reason = 0x%04X\n", retval);
                }
                else
                {
                    LOG_DEBUG("SUCCEEDED\n");
                }
            }
#else /* L2CAP_TX_QUEUE_FLOW */
            printf("L2CAP_TX_QUEUE_FLOW is not defined\n");
#endif /* L2CAP_TX_QUEUE_FLOW */
            break;
#endif /* RW610_SERIES || RW612_SERIES*/

        case 9:
            /* Directly Invoking the Service related Menu Operation from here */
        #ifdef BT_LE
        #ifndef BT_GAM
           CONSOLE_OUT("Set Auto Start Initial GAP Procedures (0-No,  1-YES): ");
           CONSOLE_IN("%d", &choice);
           appl_service_set_gap_proc_state((UCHAR)choice);
        #endif /* BT_GAM */
        #else /* BT_LE */
           CONSOLE_OUT("*** Supported only when 'BT_LE' is Defined!\n");
        #endif /* BT_LE */
            break;

        case 10:
            LOG_DEBUG("Set Snoop logging (0-Disable, 1-Enable): ");
            retval = appl_validate_params(&choice,1U,0U,1U);
            if (API_SUCCESS == retval)
            {
                BT_snoop_logging((UCHAR)choice);
            }
            break;

        case 11:
            main_hci_operations();
            break;

        case 12:
        #ifdef SDP
            #ifdef SDP_CLIENT
                main_sdp_operations();
            #else  /* SDP_CLIENT */
                LOG_DEBUG("*** Enable 'SDP_CLIENT' Compilation Flag\n");
            #endif /* SDP_CLIENT */
        #else  /* SDP */
            LOG_DEBUG("*** Enable 'SDP' Compilation Flag\n");
        #endif /* SDP */
            break;

        case 13:
        #ifdef SDP
            #ifdef SDP_SERVER
                main_dbase_operations();
            #else  /* SDP_SERVER */
                LOG_DEBUG("*** Enable 'SDP_SERVER' Compilation Flag\n");
            #endif /* SDP_SERVER */
        #else  /* SDP */
            LOG_DEBUG("*** Enable 'SDP' Compilation Flag\n");
        #endif /* SDP */
            break;

        case 14:
        #ifdef RFCOMM
            main_rfcomm_operations();
        #else  /* RFCOMM */
            LOG_DEBUG("*** Enable 'RFCOMM' Compilation Flag\n");
        #endif /* RFCOMM */
            break;

        case 15:
        #ifdef MCAP
            main_mcap_operations();
        #else  /* MCAP */
            LOG_DEBUG("*** Enable 'MCAP' Compilation Flag\n");
        #endif /* MCAP */
            break;

        case 16:
        #ifdef BNEP
            main_bnep_operations();
        #else  /* BNEP */
            LOG_DEBUG("*** Enable 'BNEP' Compilation Flag\n");
        #endif /* BNEP */
            break;

        case 17:
        #ifdef AVDTP
            main_avdtp_operations();
        #else  /* AVDTP */
            LOG_DEBUG("*** Enable 'AVDTP' Compilation Flag\n");
        #endif /* AVDTP */
            break;

        case 18:
        #ifdef AVCTP
            main_avctp_operations();
        #else  /* AVCTP */
            LOG_DEBUG("*** Enable 'AVCTP' Compilation Flag\n");
        #endif /* AVCTP */
            break;

        case 19:
        #ifdef OBEX_CLIENT
            main_obex_client_operations ();
        #else  /* OBEX_CLIENT */
            LOG_DEBUG("*** Enable 'OBEX_CLIENT' Compilation Flag\n");
        #endif /* OBEX_CLIENT */
            break;

        case 20:
        #ifdef OBEX_SERVER
            main_obex_server_operations ();
        #else  /* OBEX_SERVER */
            LOG_DEBUG("*** Enable 'OBEX_SERVER' Compilation Flag\n");
        #endif /* OBEX_SERVER */
            break;

        case 21:
#ifdef BR_EDR_L2CAP
            main_l2cap_operations();
#else  /* BR_EDR_L2CAP */
            LOG_DEBUG("*** Enable 'BR_EDR_L2CAP' Compilation Flag\n");
#endif /* BR_EDR_L2CAP */
            break;

        case 25:
#ifdef BT_STORAGE
#ifdef STORAGE_HAVE_EVENT_MASK
            LOG_DEBUG ("Enter the Storage event mask to set (in Hex)\n"
                    "(AuthUpdate Mask - 0x0001) \n"
                    "(Shutdown Mask   - 0x0002) \n"
                    "(Others Mask     - 0x0004) \n"
                    "(Masks can be ORed and set): ");
            scanf("%x", &choice);
            (BT_IGNORE_RETURN_VALUE) BT_storage_enable_events((UINT16) choice);
#else /* STORAGE_HAVE_EVENT_MASK */
            LOG_DEBUG("*** Enable 'STORAGE_HAVE_EVENT_MASK' Compilation Flag\n");
#endif /* STORAGE_HAVE_EVENT_MASK */
#else /* BT_STORAGE */
            LOG_DEBUG("*** Enable 'BT_STORAGE' Compilation Flag\n");
#endif /* BT_STORAGE */
            break;

        case 30:
        #ifdef CLASSIC_SEC_MANAGER
            main_sm_operations();
        #else
            LOG_DEBUG("*** Enable 'CLASSIC_SEC_MANAGER' Compilation Flag\n");
        #endif /* CLASSIC_SEC_MANAGER */
            break;

        case 31:
        #ifdef SMP
            main_smp_operations();
        #else  /* SMP */
            LOG_DEBUG("*** Enable 'SMP' Compilation Flag\n");
        #endif /* SMP */
            break;

        case 32:
        #if (defined L2CAP_LE_SLAVE) || ( defined L2CAP_LE_MASTER )
            main_l2cap_le_operations();
        #else  /* (defined L2CAP_LE_SLAVE) || ( defined L2CAP_LE_MASTER ) */
            LOG_DEBUG("*** Enable 'L2CAP_LE_SLAVE' or 'L2CAP_LE_MASTER'\n");
        #endif /* (defined L2CAP_LE_SLAVE) || ( defined L2CAP_LE_MASTER ) */
            break;

        case 33:
        #ifdef ATT_CLIENT
            main_att_client_operations();
        #else
            LOG_DEBUG("*** Enable 'ATT_CLIENT'\n");
        #endif /* ATT_CLIENT */
            break;

        case 34:
        #ifdef L2CAP_SUPPORT_ECBFC_MODE
            main_l2cap_ecbfc_operations();
        #else  /* L2CAP_SUPPORT_ECBFC_MODE */
            printf("*** Enable 'L2CAP_SUPPORT_ECBFC_MODE'\n");
        #endif /* L2CAP_SUPPORT_ECBFC_MODE */
            break;

        /* ---------- Profile Layer Testing --------------- */
        case 40:
        #ifdef SPP
            main_spp_operations();
        #else  /* SPP */
            LOG_DEBUG("*** Enable 'SPP' Compilation Flag\n");
        #endif /* SPP */
            break;

        case 41:
#ifdef HFP_UNIT
            main_hfp_unit_operations();
#else  /* HFP_UNIT */
            LOG_DEBUG("*** Enable 'HFP_UNIT' Compilation Flag\n");
#endif /* HFP_UNIT */
            break;

        case 42:
#ifdef HFP_AG
            main_hfp_ag_operations();
#else  /* HFP_AG */
            LOG_DEBUG("*** Enable 'HFP_AG' Compilation Flag\n");
#endif /* HFP_AG */
            break;

        case 45:
#ifdef DUNP_DT
            main_dunp_dt_operations();
#else  /* DUNP_DT */
            LOG_DEBUG("*** Enable 'DUNP_DT' Compilation Flag\n");
#endif /* DUNP_DT */
            break;

        case 46:
#ifdef DUNP_GW
            main_dunp_gw_operations();
#else  /* DUNP_GW */
            LOG_DEBUG("*** Enable 'DUNP_GW' Compilation Flag\n");
#endif /* DUNP_GW */
            break;

        case 47:
#ifdef SAP_CLIENT
            main_sap_client_operations();
#else  /* SAP_CLIENT */
            LOG_DEBUG("*** Enable 'SAP_CLIENT' Compilation Flag\n");
#endif /* SAP_CLIENT */
            break;

        case 48:
#ifdef SAP_SERVER
            main_sap_server_operations();
#else  /* SAP_SERVER */
            LOG_DEBUG("*** Enable 'SAP_SERVER' Compilation Flag\n");
#endif /* SAP_SERVER */
            break;

        case 50:
        #ifdef OPP
            main_opp_client_operations();
        #else  /* OPP */
            LOG_DEBUG("*** Enable 'OPP' Compilation Flag\n");
        #endif /* OPP */
            break;

        case 51:
#ifdef OPP
            main_opp_server_operations();
#else  /* OPP */
            LOG_DEBUG("*** Enable 'OPP' Compilation Flag\n");
#endif /* OPP */
            break;

        case 52:
        #ifdef FTP
            main_ftp_client_operations();
        #else  /* FTP */
            LOG_DEBUG("*** Enable 'FTP' Compilation Flag\n");
        #endif /* FTP */
            break;

        case 53:
#ifdef FTP
            main_ftp_server_operations();
#else  /* FTP */
            LOG_DEBUG("*** Enable 'FTP' Compilation Flag\n");
#endif /* FTP */
            break;

        case 54:
#ifdef MAP_MCE
            main_map_mce_operations();
#else  /* MAP_MCE */
            LOG_DEBUG("*** Enable 'MAP_MCE' Compilation Flag\n");
#endif /* MAP_MCE */
            break;

        case 55:
#ifdef MAP_MSE
            main_map_mse_operations();
#else  /* MAP_MSE */
            LOG_DEBUG("*** Enable 'MAP_MSE' Compilation Flag\n");
#endif /* MAP_MSE */
            break;

        case 56:
#ifdef PBAP_PCE
            main_pbap_pce_operations();
#else /* PBAP_PCE */
            LOG_DEBUG("*** Enable 'PBAP_PCE' Compilation Flag\n");
#endif /* PBAP_PCE */
            break;

        case 57:
#ifdef PBAP_PSE
            main_pbap_pse_operations();
#else /* PBAP_PSE */
            LOG_DEBUG("*** Enable 'PBAP_PSE' Compilation Flag\n");
#endif /* PBAP_PSE */
            break;

        case 58:
#ifdef CTN_CCE
            main_ctn_cce_operations();
#else /* CTN_CCE */
            LOG_DEBUG("*** Enable 'CTN_CCE' Compilation Flag\n");
#endif /* CTN_CCE */
            break;

        case 59:
#ifdef CTN_CSE
            main_ctn_cse_operations();
#else /* CTN_CSE */
            LOG_DEBUG("*** Enable 'CTN_CSE' Compilation Flag\n");
#endif /* CTN_CSE */
            break;

        case 60:
#ifdef BIP_INITIATOR
            main_bip_initiator_operations();
#else /* BIP_INITIATOR */
            LOG_DEBUG("*** Enable 'BIP_INITIATOR' Compilation Flag\n");
#endif /* BIP_INITIATOR */
            break;

        case 61:
#ifdef BIP_RESPONDER
            main_bip_responder_operations();
#else /* BIP_RESPONDER */
            LOG_DEBUG("*** Enable 'BIP_RESPONDER' Compilation Flag\n");
#endif /* BIP_RESPONDER */
            break;

        case 62:
#ifdef SYNCP_CLIENT
            main_syncp_client_operations();
#else /* SYNCP_CLIENT */
            LOG_DEBUG("*** Enable 'SYNCP_CLIENT' Compilation Flag\n");
#endif /* SYNCP_CLIENT */
            break;

        case 63:
#ifdef SYNCP_SERVER
            main_syncp_server_operations();
#else /* SYNCP_SERVER */
            LOG_DEBUG("*** Enable 'SYNCP_SERVER' Compilation Flag\n");
#endif /* SYNCP_SERVER */
            break;

        case 65:
#if (defined A2DP_SOURCE || defined A2DP_SINK)
            main_a2dp_operations();
#else /* (defined A2DP_SOURCE || defined A2DP_SINK) */
            LOG_DEBUG("*** Enable 'A2DP_SOURCE' or 'A2DP_SINK' Compilation Flag\n");
#endif /* (defined A2DP_SOURCE || defined A2DP_SINK) */
            break;

        case 66:
#if ((defined AVRCP_CT) || (defined AVRCP_TG))
            main_avrcp_operations();
#else /* (defined AVRCP_CT || defind AVRCP_TG) */
            LOG_DEBUG("*** Enable 'AVRCP_CT' or 'AVRCP_TG' Compilation Flag\n");
#endif /* (defined AVRCP_CT || defind AVRCP_TG) */
            break;

        case 67:
#ifdef HDP
            main_hdp_operations();
#else  /* HDP */
            LOG_DEBUG("*** Enable 'HDP' Compilation Flag\n");
#endif /* HDP */
            break;

        case 68:
        #ifdef PAN
            main_pan_operations();
        #else  /* PAN */
            LOG_DEBUG("*** Enable 'PAN' Compilation Flag\n");
        #endif /* PAN */
            break;

        case 70:
#ifdef HID_DEVICE
            main_hid_device_operations();
#else  /* HID_DEVICE */
            LOG_DEBUG("*** Enable 'HID_DEVICE' Compilation Flag\n");
#endif /* HID_DEVICE */
            break;

        case 71:
        #ifdef HID_HOST
            main_hid_host_operations ();
        #else  /* HID_HOST */
            LOG_DEBUG("*** Enable 'HID_HOST' Compilation Flag\n");
        #endif /* HID_HOST */
            break;

        case 75:
        #ifdef DID_CLIENT
            main_did_client_operations ();
        #else  /* DID_CLIENT */
            LOG_DEBUG("*** Enable 'DID_CLIENT' Compilation Flag\n");
        #endif /* DID_CLIENT */
            break;

        case 76:
#ifdef DID_SERVER
            main_did_server_operations();
#else  /* DID_SERVER */
            LOG_DEBUG("*** Enable 'DID_SERVER' Compilation Flag\n");
#endif /* DID_SERVER */
            break;

        case 80:
#ifdef ATT
        #ifdef GATT_CLIENT
            main_gatt_client_operations();
        #else  /* GATT_CLIENT */
            LOG_DEBUG("*** Enable 'GATT_CLIENT' Compilation Flag\n");
        #endif /* GATT_CLIENT */
#endif /* ATT */
            break;

        case 81:
#ifdef ATT
            main_gatt_server_operations();
#else  /* ATT */
            LOG_DEBUG("*** Enable 'ATT' Compilation Flag\n");
#endif /* ATT */
            break;

#ifdef BPP_SENDER
        case 90:
            main_bpp_sender_operations ();
            break;
#endif /* BPP_SENDER */

#ifdef BPP_PRINTER
        case 91:
            main_bpp_printer_operations ();
            break;
#endif /* BPP_PRINTER */

	    case 100:
#ifdef BT_SECURITY_VU_VALIDATION
            LOG_DEBUG("%s", vu_list);
            LOG_DEBUG("Enter VU ID: ");
            scanf("%d", &choice);
            BT_security_vu_set((UCHAR)choice);
#else /* BT_SECURITY_VU_VALIDATION */
            LOG_DEBUG("BT_SECURITY_VU_VALIDATION macro not defined\n");
#endif /* BT_SECURITY_VU_VALIDATION */
            break;

	    case 110:
#ifdef SERIAL_BTSNOOP
            main_serial_btsnoop_operations();
#else
            printf("SERIAL_BTSNOOP macro not defined\n");
#endif /* SERIAL_BTSNOOP */
            break;

        case 120:
#if defined(RW610_SERIES) || defined(RW612_SERIES)
#if defined(APP_LOWPOWER_ENABLED) && (APP_LOWPOWER_ENABLED > 0)
            PWRCli_Menu();
#else
            LOG_DEBUG("APP_LOWPOWER_ENABLED macro not defined to 1\n");
#endif /* APP_LOWPOWER_ENABLED */
#else
            printf("Low Power Operations not supported\n");
#endif /* RW610_SERIES || RW612_SERIES */
            break;

        case 130:
#if defined(RW610_SERIES) || defined(RW612_SERIES)
            if (BT_TRUE == appl_bt_state)
            {
                /* Reset the Controller */
                (BT_IGNORE_RETURN_VALUE) BT_hci_reset();
                BT_sleep(1U);
                LOG_DEBUG("EtherMind: Bluetooth OFF ...\n");
                (BT_IGNORE_RETURN_VALUE) BT_bluetooth_off ();
#ifndef BT_COMMON_PL_SUPPORT_UL_CB
                /* Shutdown any application/platform specific components */
                appl_shutdown();
#endif /* BT_COMMON_PL_SUPPORT_UL_CB */
                appl_bt_state = BT_FALSE;
            }

            LOG_DEBUG("Performing Controller Reset ...\n");
            extern int PLATFORM_ResetBle(void);
            (void)PLATFORM_ResetBle();
#else
            printf("Reset Controller not supported\n");
#endif /* RW610_SERIES || RW612_SERIES */
            break;

        case 201:
#ifdef BT_GAM
            main_ccp_ce_menu_operations();
#else  /* BT_GAM */
            printf("*** Enable 'BT_GAM' Compilation Flag\n");
#endif /* BT_GAM */
            break;

        case 202:
#ifdef BT_GAM
            main_ccp_se_menu_operations();
#else  /* BT_GAM */
            printf("*** Enable 'BT_GAM' Compilation Flag\n");
#endif /* BT_GAM */
            break;

        case 203:
#ifdef BT_GAM
            main_mcp_ce_menu_operations();
#else  /* BT_GAM */
            printf("*** Enable 'BT_GAM' Compilation Flag\n");
#endif /* BT_GAM */
            break;

        case 204:
#ifdef BT_GAM
            main_mcp_se_menu_operations();
#else  /* BT_GAM */
            printf("*** Enable 'BT_GAM' Compilation Flag\n");
#endif /* BT_GAM */
            break;

        case 205:
#ifdef BT_GAM
            main_tmap_menu_operations();
#else  /* BT_GAM */
            printf("*** Enable 'BT_GAM' Compilation Flag\n");
#endif /* BT_GAM */
            break;

        case 206:
#ifdef BT_GAM
            main_hap_menu_operations();
#else  /* BT_GAM */
            printf("*** Enable 'BT_GAM' Compilation Flag\n");
#endif /* BT_GAM */
            break;

        case 210:
#ifdef BT_GAM
#ifdef GATT_DB_DYNAMIC
	    {
                UINT32 appl_ga_signature = 0x1234FFFF;
                retval = ga_brr_dyn_gatt_db_init_pl(&appl_ga_signature);
                printf("GA Setup - 0x%04X\n", retval);
            }
#endif /* GATT_DB_DYNAMIC */
#else  /* BT_GAM */
            printf("*** Enable 'BT_GAM' Compilation Flag\n");
#endif /* BT_GAM */
            break;

    	case 220:
#ifdef BEACON_ENABLE
         main_beacon_operations();
#else
        LOG_DEBUG("\n BEACON_ENABLE preprocessor not defined\n");
#endif
            break;

	case 250:
#ifdef OOB_WAKEUP
	    printf("Sending wake on BLE vendor command\n");
	    Configure_sleep_wakeup();
#else /* OOB_WAKEUP */
	    printf("*** Enable 'OOB_WAKEUP' Compilation Flag\n");
#endif /* OOB_WAKEUP */
	    break;

	case 251:
#ifdef OOB_WAKEUP
	    printf("Sending controller sleep vendor command\n");
	    Host_to_controller_sleep_hci_cmnd();
#else /* OOB_WAKEUP */
	    printf("*** Enable 'OOB_WAKEUP' Compilation Flag\n");
#endif /* OOB_WAKEUP */
	    break;

	case 252:
#ifdef OOB_WAKEUP
	    printf("Make sure controller sleep vendor command is given before this. Simulating BT sleep\n");
	    H2C_sleep();
#else /* OOB_WAKEUP */
	    printf("*** Enable 'OOB_WAKEUP' Compilation Flag\n");
#endif /* OOB_WAKEUP */
	    break;

	case 253:
#ifdef OOB_WAKEUP
	    printf("Simulating BT wakeup\n");
	    H2C_wakeup();
#else /* OOB_WAKEUP */
	    printf("*** Enable 'OOB_WAKEUP' Compilation Flag\n");
#endif /* OOB_WAKEUP */
	    break;

	case 254:
#ifdef OOB_WAKEUP
	    printf("Calling Host sleep\n");
	    if(sleep_host)
	    {
                sleep_host = 0;
                Host_sleep();
	    }
#else /* OOB_WAKEUP */
	    printf("*** Enable 'OOB_WAKEUP' Compilation Flag\n");
#endif /* OOB_WAKEUP */
	    break;

    case 255:
#ifdef ENABLE_BT_IND_RESET
        LOG_DEBUG("select IR mode to configure (0:Disable, 1:Outband, 2:Inband): ");
    	UINT8 irMode = IR_DISABLE;
        scanf("%d\n", &irMode);
        controler_config_ir(irMode);
#else /* ENABLE_BT_IND_RESET */
	    printf("*** Enable 'ENABLE_BT_IND_RESET' Compilation Flag\n");
#endif /* ENABLE_BT_IND_RESET */
            break;

    case 256:
#ifdef ENABLE_BT_IND_RESET
            LOG_DEBUG("Initiating IR trigger!!\n");
            controller_trigger_ir();
#else /* ENABLE_BT_IND_RESET */
            printf("*** Enable 'ENABLE_BT_IND_RESET' Compilation Flag\n");
#endif /* ENABLE_BT_IND_RESET */
			break;

	case 280:
	    appl_debug_enabled = 0;
	    LOG_DEBUG("appl_debug_enabled is %d",appl_debug_enabled);
	    break;
	case 281:
	    appl_debug_enabled = 1;
	    LOG_DEBUG("appl_debug_enabled is %d",appl_debug_enabled);
	    break;

    case 300:
#ifdef BT_GAM
            main_ga_operations();
#else  /* BT_GAM */
            printf("*** Enable 'BT_GAM' Compilation Flag\n");
#endif /* BT_GAM */
            break;

    default:
            if(invalid_val == 0)
            {
                LOG_DEBUG("Invalid Option : %d.\n", choice);
            }
            invalid_val = 0;
            break;
        }
    }
}

#ifdef BEACON_ENABLE
void main_beacon_operations(void)
{
	int choice_beacon =0;
	printf("\n");
	printf("Enter to Beacon Option ...\n");
	printf("%s", beacon_options);
	printf("\n");
	printf("\n");

	BT_LOOP_FOREVER()
	{
		scanf("%d", &choice_beacon);
		switch (choice_beacon)
		{
		case 0:printf("\nExit\n");
		    break;

		case 1:
			printf("\nRefreshing ...\n");
			printf("%s", beacon_options);
			break;

		case 2:
			printf("\nI-beacon under Implementation...\n");

			break;
		case 3:
			printf("\nEddystone ...\n");
			eddystone_config();
			break;

		default:
			printf("\nInvalid Option...\n");
			printf("%s", beacon_options);
			break;
		}

		if (0 == choice_beacon)
		{
			/* return */
			break;
		}
	}

}
#endif

#ifdef BT_VENDOR_SPECIFIC_INIT
void appl_vendor_init_complete(void)
{
    (BT_IGNORE_RETURN_VALUE) BT_bluetooth_off();

#ifdef BT_SUPPORT_CONTROLLER_INIT_SKIP
    (BT_IGNORE_RETURN_VALUE) BT_hci_set_controller_init (BT_TRUE);
    skip = 1;
#endif /* BT_SUPPORT_CONTROLLER_INIT_SKIP */

    (BT_IGNORE_RETURN_VALUE) BT_hci_set_init_command_mask (0x000001U);

    LOG_DEBUG ("Vendor Initialization Complete !!! Please Turn ON Bluetooth.\n");

    return;
}
#endif /* BT_VENDOR_SPECIFIC_INIT */

void appl_bluetooth_on_init_cb(void)
{
#ifdef BT_LE
    /* Handle specific application initializations during BT ON */
    appl_init();
#endif /* BT_LE */

    /* Enable Error Indication Callback */
#ifdef BT_SUPPORT_ERR_IND_CALLBACK
    (BT_IGNORE_RETURN_VALUE)BT_ethermind_register_error_indication_callback
                            (
                                appl_bluetooth_error_ind_cb
                            );
#endif /* BT_SUPPORT_ERR_IND_CALLBACK */

}

void appl_bluetooth_off_deinit_cb(void)
{
    /* Shutdown any application/platform specific components */
    appl_shutdown();
}

API_RESULT appl_bluetooth_on_complete ( void )
{
    UCHAR bd_addr[BT_BD_ADDR_SIZE];

#ifdef BT_SUPPORT_STACK_VERSION_INFO
    BT_VERSION_NUMBER version;

    BT_get_version_number (&version);
#endif /* BT_SUPPORT_STACK_VERSION_INFO */

    /* Get local Bluetooth Address */
    (BT_IGNORE_RETURN_VALUE) BT_hci_get_local_bd_addr(bd_addr);

    LOG_DEBUG("\n");
    LOG_DEBUG("===================================================================\n");
    LOG_DEBUG(">\tBluetooth ON Initialization Completed.\n");
    LOG_DEBUG(">\tBluetooth Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
    bd_addr[5U], bd_addr[4U], bd_addr[3U], bd_addr[2U], bd_addr[1U], bd_addr[0U]);
    LOG_DEBUG("\n>\tStack Version - %03d.%03d.%03d.\n",
    version.major, version.minor, version.subminor);
    LOG_DEBUG("===================================================================\n");

#ifdef NXP_CODE
    /* Vendor Command for firmware ROM Version info */
    BT_hci_vendor_specific_command(0x000F,0,0);
#endif /* NXP_CODE */

    /* Set BT ON identifier */
    appl_bt_state = BT_TRUE;

#ifndef BT_SINGLE_MODE
#ifdef CLASSIC_SEC_MANAGER
    /* Register with BR/EDR Security Module */
    (BT_IGNORE_RETURN_VALUE) BT_sm_register_user_interface (appl_sm_ui_notify_cb);
#endif /* CLASSIC_SEC_MANAGER */

    /* Set the default page timeout */
    (BT_IGNORE_RETURN_VALUE)BT_hci_write_page_timeout(0x3E80U);

    /* Set default link policy to support role switch and sniff mode */
    (BT_IGNORE_RETURN_VALUE) BT_hci_write_default_link_policy_settings(0x05U);

    /* Set default scan enable - Disc and Conn */
    (BT_IGNORE_RETURN_VALUE) BT_hci_write_scan_enable(0x03U);

    /* Activate Multiprofile Record */
    appl_mps_activate_record ();
#endif /* BT_SINGLE_MODE */

#ifdef BT_LE

#ifdef HCI_WRITE_LE_HOST_SUPPORT
    /* Register LE support in Host with controller */
    (BT_IGNORE_RETURN_VALUE) BT_hci_write_le_host_support(0x01U, 0x00U);
#endif /* HCI_WRITE_LE_HOST_SUPPORT */

    appl_init_complete();
#endif /* BT_LE */

#ifdef SDP_DYNAMIC_DB
    printf("Creating Dynamic SDP Records\n");
    db_add_record();
#else /* SDP_DYNAMIC_DB */
    printf("SDP_DYNAMIC_DB is not defined\n");
#endif /* SDP_DYNAMIC_DB */

    return API_SUCCESS;
}

#ifdef BT_SUPPORT_ERR_IND_CALLBACK
DECL_STATIC API_RESULT appl_bluetooth_error_ind_cb
                       (
                           UINT32   module_id,
                           UINT16   error_code,
                           void   * error_msg
                       )
{
    BT_ERROR_MSG *appl_bt_err_msg;

    appl_bt_err_msg = (BT_ERROR_MSG *)error_msg;

    if (BT_MODULE_ID_L2CAP == module_id)
    {
        UINT16 cid;

        if (L2CAP_MTU_CHECK_FAILED == error_code)
        {
            cid = (UINT16)(*(UINT16 *)appl_bt_err_msg->error_info);

            printf(
            "\n [**L2CAP-ERR-IND**]: MTU Check Failed while receiving data "
            "on CID 0x%04X with Conn Handle 0x%04X\n", cid,
            appl_bt_err_msg->connection_handle);
        }
        else if (L2CAP_INCORRECT_SDU_LENGTH == error_code)
        {
            cid = (UINT16)(*(UINT16 *)appl_bt_err_msg->error_info);

            printf(
            "\n [**L2CAP-ERR-IND**]: Received more data than SDU length "
            "on CID 0x%04X with Conn Handle 0x%04X\n", cid,
            appl_bt_err_msg->connection_handle);
        }
    }
    return API_SUCCESS;
}
#endif /* BT_SUPPORT_ERR_IND_CALLBACK */

void appl_mps_activate_record (void)
{
#ifdef APPL_ENABLE_MPS_RECORD
    API_RESULT retval;
    UINT32 record_handle;

    UCHAR    mpsd_scenario_attr_value[9U] = {0x0BU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,0x00U};
    UCHAR    mpmd_scenario_attr_value[9U] = {0x0BU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};
    UCHAR    profprot_supp_attr_value[3U] = {0x09U, 0x00U, 0x00U};

    /*
     * TODO:
     * Update the supported bits in the above attr_value arrays
     * as per requirement
     *
     * Refer MPS specification for detail on bits identification
     */

    /* Get the MPS record handle */
    retval = BT_dbase_get_record_handle (DB_RECORD_MPS, 0U, &record_handle);

    /* Update the MPSD Supported Scenarios attribute */
    LOG_DEBUG ("Updating MPSD scenario in database ... \n");
    retval = BT_dbase_update_attr_value
             (
                 record_handle,
                 MPS_MPSD_SCENARIOS_ATTR_ID,
                 mpsd_scenario_attr_value,
                 0x09U
             );
    LOG_DEBUG ("- 0x%04X\n", retval);

    LOG_DEBUG ("Updating MPMD scenario in database ... \n");
    retval = BT_dbase_update_attr_value
             (
                 record_handle,
                 MPS_MPMD_SCENARIOS_ATTR_ID,
                 mpmd_scenario_attr_value,
                 0x09U
             );
    LOG_DEBUG ("- 0x%04X\n", retval);

    LOG_DEBUG ("Updating Profile/Protocol support in database ... \n");
    retval = BT_dbase_update_attr_value
             (
                 record_handle,
                 MPS_SUPP_PROF_PROT_DEP_ATTR_ID,
                 profprot_supp_attr_value,
                 0x03U
             );
    LOG_DEBUG ("- 0x%04X\n", retval);

    /* Activate MPS record */
    LOG_DEBUG ("Activating MPS ... ");
    retval = BT_dbase_activate_record (record_handle);
    LOG_DEBUG ("- 0x%04X\n", retval);
#endif /* APPL_ENABLE_MPS_RECORD */
}


API_RESULT appl_read_config(void)
{
    CHAR config_buf[BT_CONFIG_MAX_CHARS_PER_LINE];
    INT32 ret;

    /* Read Debug enable from Config File */
    ret = BT_config_read(BT_CONFIG_FILE, "BT_DEBUG_ENABLE_ALL", config_buf);
    if (ret < 0)
    {
        LOG_DEBUG("*** FAILED to Read Configuration for BT_DEBUG_ENABLE_ALL: %d\n",
            ret);

        return API_FAILURE;
    }

    LOG_DEBUG("Enable All Debug = '%s'\n", config_buf);

    if (0 == BT_str_cmp("TRUE", config_buf))
    {
        appl_debug_enable_all = BT_TRUE;
    }
    else
    {
        appl_debug_enable_all = BT_FALSE;
    }

    return API_SUCCESS;
}

API_RESULT appl_open_transport_port ( void )
{
    int ret;

#if ((defined BT_UART) || (defined BT_USB))
    INT32 baudrate;
#endif /* ((defined BT_UART) || (defined BT_USB)) */

    CHAR config_buf[BT_CONFIG_MAX_CHARS_PER_LINE];

    /* Read HCI Transport from Config File */
    ret = BT_config_read (BT_CONFIG_FILE, "BT_TRANSPORT", config_buf);
    if (ret < 0)
    {
        LOG_DEBUG("*** FAILED to Read Configuration for BT_TRANSPORT: %d\n",
        ret);

        return API_FAILURE;
    }

#ifdef BT_UART

    if (0 == BT_str_cmp(config_buf, "UART"))
    {
        LOG_DEBUG("HCI Transport is 'UART'. Reading UART Port & Baudrate\n");
    }
    else
    {
        LOG_DEBUG("*** Unsupported HCI Transport Configuration '%s'\n",
        config_buf);

        return API_FAILURE;
    }

    /* Read UART Baudrate from Config File */
    ret = BT_config_read (BT_CONFIG_FILE, "BT_UART_BAUDRATE", config_buf);
    if (ret < 0)
    {
        LOG_DEBUG("*** FAILED to Read Configuration for BT_UART_BAUDRATE: %d\n",
        ret);

        return API_FAILURE;
    }

    /* Convert Baudrate String */
    baudrate = (INT32)(appl_str_to_num
                      (
                          (UCHAR *)config_buf,
                          (UINT16)BT_str_len(config_buf)
                      ));

    LOG_DEBUG("HCI-UART Transport Baudrate = '%s' -> %d\n",
    config_buf, (int)baudrate);

    /* Read UART Port from Config File */
    ret = BT_config_read (BT_CONFIG_FILE, "BT_UART_PORT", config_buf);
    if (ret < 0)
    {
        LOG_DEBUG("*** FAILED to Read Configuration for BT_UART_PORT: %d\n",
        ret);

        return API_FAILURE;
    }

    LOG_DEBUG("HCI-UART Transport Port = '%s'\n", config_buf);

    /* Set UART Parameters */
    hci_uart_set_serial_settings (config_buf, baudrate);

#endif /* BT_UART */

#ifdef BT_USB

    if (0 == BT_str_cmp(config_buf, "USB"))
    {
        LOG_DEBUG("HCI Transport is 'USB'. Reading USB-TTY Port\n");
    }
    else
    {
        LOG_DEBUG("*** Unsupported HCI Transport Configuration '%s'\n",
        config_buf);

        return API_FAILURE;
    }

    /* Read USB Port from Config File */
    ret = BT_config_read (BT_CONFIG_FILE, "BT_USB_PORT", config_buf);
    if (ret < 0)
    {
        LOG_DEBUG("*** FAILED to Read Configuration for BT_USB_PORT: %d\n",
        ret);

        return API_FAILURE;
    }

    LOG_DEBUG("HCI-USB Transport Port = '%s'\n", config_buf);

    /* Use Default Baudrate (though not required as of now) */
    baudrate = 115200;

    /* Set USB Parameters */
    /* hci_usb_set_serial_settings (config_buf, baudrate); */

#endif /* BT_USB */

    return API_SUCCESS;
}
