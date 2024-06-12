/*
 * Copyright 2019-2020, 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __SOEM__CONFIG__H__
#define __SOEM__CONFIG__H__

/** max. etries in EtherCAT error list */
#define EC_MAXELIST       64

/** max. length of readable name in slavelist and Object Description List */
#define EC_MAXNAME        40

/** max. number of slaves in array */
#define EC_MAXSLAVE       32

/** max. number of groups */
#define EC_MAXGROUP       2

/** max. number of IO segments per group */
#define EC_MAXIOSEGMENTS  64

/** max. mailbox size */
#define EC_MAXMBX         1486

/** max. eeprom PDO entries */
#define EC_MAXEEPDO       0x200

/** max. SM used */
#define EC_MAXSM          8

/** max. FMMU used */
#define EC_MAXFMMU        4

/** max. Adapter */
#define EC_MAXLEN_ADAPTERNAME    128

/* CoE */
/** max entries in Object Description list */
#define EC_MAXODLIST   1024

/** max entries in Object Entry list */
#define EC_MAXOELIST   256

/* OS */
#define MAX_SOEM_TASK 1

/* Static allocate for SOEM stack when configSUPPORT_STATIC_ALLOCATION is configured*/
#define MAX_SOEM_TASK_STACK 256
#define MAX_SOEM_MUTE 1

#endif
