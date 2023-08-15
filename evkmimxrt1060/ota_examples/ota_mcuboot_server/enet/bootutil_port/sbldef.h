/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SBL_DEF_H__
#define __SBL_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup BasicDef
 */

/**@{*/

/* SBL version information */
#define MAJOR_VERSION  1 /**< major version number */
#define MINOR_VERSION  9 /**< minor version number */
#define REVISE_VERSION 0 /**< revise version number */

/* SBL version */
#define SBL_VERSION ((MAJOR_VERSION * 10000L) + (MINOR_VERSION * 100L) + REVISE_VERSION)

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#define __CLANG_ARM
#endif

/* SBL error code definitions */
#define OK            0  /**< There is no error */
#define ERROR         1  /**< A generic error happens */
#define ERROR_TIMEOUT 2  /**< Timed out */
#define ERROR_FULL    3  /**< The resource is full */
#define ERROR_EMPTY   4  /**< The resource is empty */
#define ERROR_NOMEM   5  /**< No memory */
#define ERROR_NOSYS   6  /**< No system */
#define ERROR_BUSY    7  /**< Busy */
#define ERROR_IO      8  /**< IO error */
#define ERROR_INTR    9  /**< Interrupted system call */
#define ERROR_INVAL   10 /**< Invalid argument */

#ifdef __cplusplus
}
#endif

#endif
