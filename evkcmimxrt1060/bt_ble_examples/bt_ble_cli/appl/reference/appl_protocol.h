
/**
 *  \file appl_protocol.h
 *
 *  Main Application Header File
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_PROTOCOL_
#define _H_APPL_PROTOCOL_

/* ----------------------------------------- Header File Inclusion */

/* --------------------- Bluetooth Classic */
#include "hci_transport.h"
#include "appl_hci.h"

#ifdef L2CAP
#include "appl_l2cap.h"
#endif /* L2CAP */

#ifdef CLASSIC_SEC_MANAGER
#include "appl_sm.h"
#endif /* CLASSIC_SEC_MANAGER */

#ifdef AVDTP
#include "appl_avdtp.h"
#endif /* AVDTP */

#ifdef AVCTP
#include "appl_avctp.h"
#endif /* AVCTP */

#ifdef BNEP
#include "appl_bnep.h"
#endif /* BNEP */

#ifdef MCAP
#include "appl_mcap.h"
#endif /* MCAP */

#ifdef RFCOMM
#include "appl_rfcomm.h"
#endif /* RFCOMM */

#ifdef SDP
#include "appl_sdp.h"
#endif /* SDP */

#ifdef OBEX_CLIENT
#include "appl_obex_client.h"
#endif /* OBEX_CLIENT */

#ifdef OBEX_SERVER
#include "appl_obex_server.h"
#endif /* OBEX_SERVER */

/* --------------------- Bluetooth Low Energy */
#ifdef BT_LE
#include "appl_hci_le.h"
#include "appl_l2cap_le.h"
#endif /* BT_LE */

#ifdef SMP
#include "appl_smp.h"
#endif /* SMP */

/* -------------------- Common */
#ifdef L2CAP_SUPPORT_ECBFC_MODE
#include "appl_l2cap_ecbfc.h"
#endif /* L2CAP_SUPPORT_ECBFC_MODE */

/* ----------------------------------------- Global Definitions */

/* ----------------------------------------- Structures/Data Types */

/* ----------------------------------------- Macros */

/* ----------------------------------------- Function Declarations */

#endif /* _H_APPL_PROTOCOL_ */

