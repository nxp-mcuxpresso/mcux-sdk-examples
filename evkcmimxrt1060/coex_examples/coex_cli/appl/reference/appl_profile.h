
/**
 *  \file appl_profile.h
 *
 *  Main Application Header File
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_PROFILE_
#define _H_APPL_PROFILE_

/* ----------------------------------------- Header File Inclusion */

/* --------------------- Bluetooth Classic */
#ifdef SPP
#include "appl_spp.h"
#endif /* SPP */

#ifdef OPP
#include "appl_opp.h"
#endif /* OPP */

#ifdef FTP
#include "appl_ftp.h"
#endif /* FTP */

#ifdef PAN
#include "appl_pan.h"
#endif /* PAN */

#ifdef DID_CLIENT
#include "appl_did.h"
#endif /* DID_CLIENT */

#ifdef HDP
#include "appl_hdp.h"
#endif /* HDP */

#ifdef HID_DEVICE
#include "appl_hid_device.h"
#endif /* HID_DEVICE */

#ifdef HID_HOST
#include "appl_hid_host.h"
#endif /* HID_HOST */

#ifdef PBAP_PCE
#include "appl_pbap_pce.h"
#endif /* PBAP_PCE */

#ifdef PBAP_PSE
#include "appl_pbap_pse.h"
#endif /* PBAP_PSE */

#ifdef HFP_UNIT
#include "appl_hfp_unit.h"
#endif /* HFP_UNIT */

#ifdef HFP_AG
#include "appl_hfp_ag.h"
#endif /* HFP_AG */

#ifdef HSP_UNIT
#include "appl_hsp_unit.h"
#endif /* HSP_UNIT */

#ifdef HSP_AG
#include "appl_hsp_ag.h"
#endif /* HSP_AG */

#ifdef DUNP_DT
#include "appl_dunp_dt.h"
#endif /* DUNP_DT */

#ifdef DUNP_GW
#include "appl_dunp_gw.h"
#endif /* DUNP_GW */

#if (defined A2DP_SOURCE || defined A2DP_SINK)
#include "appl_a2dp.h"
#endif /* (defined A2DP_SOURCE || defined A2DP_SINK) */

#if ((defined AVRCP_CT) || (defined AVRCP_TG))
#include "appl_avrcp.h"
#endif /* (defined AVRCP_CT || defind AVRCP_TG) */

#if (defined MAP_MCE || defined MAP_MSE)
#include "appl_map.h"
#endif /* (defined MAP_MCE || defined MAP_MSE) */

#if (defined CTN_CCE || defined CTN_CSE)
#include "appl_ctn.h"
#endif /* (defined CTN_CCE || defined CTN_CSE) */

#if (defined BIP_INITIATOR || defined BIP_RESPONDER)
#include "appl_bip.h"
#endif /* (defined BIP_INITIATOR || defined BIP_RESPONDER) */

#if (defined BPP_SENDER || defined BPP_PRINTER)
#include "appl_bpp.h"
#endif /* (defined BPP_SENDER || defined BPP_PRINTER) */

#ifdef SAP_CLIENT
#include "appl_sap_client.h"
#endif /* SAP_CLIENT */

#ifdef SAP_SERVER
#include "appl_sap_server.h"
#endif /* SAP_SERVER */

#ifdef SYNCP_CLIENT
#include "appl_syncp_client.h"
#endif /* SYNCP_CLIENT */

#ifdef SYNCP_SERVER
#include "appl_syncp_server.h"
#endif /* SYNCP_SERVER */

/* --------------------- Bluetooth Low Energy */
#ifdef ATT
#include "appl_gatt_server.h"
#endif /* ATT */

#ifdef ATT_CLIENT
#include "appl_gatt_client.h"
#endif /* ATT_CLIENT */


/* ----------------------------------------- Global Definitions */

/* ----------------------------------------- Structures/Data Types */

/* ----------------------------------------- Macros */

/* ----------------------------------------- Function Declarations */

#endif /* _H_APPL_PROFILE_ */
