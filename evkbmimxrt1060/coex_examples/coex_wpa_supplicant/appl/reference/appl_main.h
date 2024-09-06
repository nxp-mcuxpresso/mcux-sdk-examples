
/**
 *  \file appl_main.h
 *
 *  Header File for the Test Application to test the Mindtree
 *  Bluetooth protocol stack.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _H_APPL_MAIN_
#define _H_APPL_MAIN_

/* --------------------------------------------- Header File Inclusion */
#include "appl_protocol.h"
#include "appl_profile.h"

#include "BT_config.h"

#ifdef BT_VENDOR_SPECIFIC_INIT
#include "vendor_specific_init.h"
#endif /* BT_VENDOR_SPECIFIC_INIT */

/* Platform specific includes */
#include "sco_audio_pl.h"

/* --------------------------------------------- Global Definitions */
/* Local device name and class */
#define APPL_GAP_DEVICE_NAMELEN             16
#define APPL_GAP_DEVICE_NAME                "NXP"
#define APPL_GAP_DEVICE_CLASS               0x400400

/* --------------------------------------------- Structures/Data Types */

/* --------------------------------------------- Macros */

/* --------------------------------------------- Internal Functions */
API_RESULT appl_bluetooth_on_complete(void);
API_RESULT appl_open_transport_port(void);
API_RESULT appl_read_config(void);
void main_beacon_operations(void);

#ifdef OOB_WAKEUP
void H2C_wakeup(void);
void H2C_sleep(void);
void Host_sleep(void);
#endif /*OOB_WAKEUP*/

#endif /* _H_APPL_MAIN_ */

