/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RPMSG_CONFIG_H_
#define RPMSG_CONFIG_H_

/*!
 * @addtogroup config
 * @{
 * @file
 */

//! @name Configuration options
//@{

//! @def RL_MS_PER_INTERVAL
//!
//! Delay in milliseconds used in non-blocking API functions for polling.
//! The default value is 1.
#define RL_MS_PER_INTERVAL (1)

//! @def RL_ALLOW_CUSTOM_SHMEM_CONFIG
//!
//! This option allows to define custom shared memory configuration and replacing
//! the shared memory related global settings from rpmsg_config.h This is useful
//! when multiple instances are running in parallel but different shared memory
//! arrangement (vring size & alignment, buffers size & count) is required. Note,
//! that once enabled the platform_get_custom_shmem_config() function needs
//! to be implemented in platform layer. The default value is 0 (all RPMsg_Lite
//! instances use the same shared memory arrangement as defined by common config macros).
#define RL_ALLOW_CUSTOM_SHMEM_CONFIG (1)

#if !(defined(RL_ALLOW_CUSTOM_SHMEM_CONFIG) && (RL_ALLOW_CUSTOM_SHMEM_CONFIG == 1))
//! @def RL_BUFFER_PAYLOAD_SIZE
//!
//! Size of the buffer payload, it must be equal to (240, 496, 1008, ...)
//! [2^n - 16]. Ensure the same value is defined on both sides of rpmsg
//! communication. The default value is 496U.
#define RL_BUFFER_PAYLOAD_SIZE (496U)

//! @def RL_BUFFER_COUNT
//!
//! Number of the buffers, it must be power of two (2, 4, ...).
//! The default value is 2U.
//! Note this value defines the buffer count for one direction of the rpmsg
//! communication only, i.e. if the default value of 2 is used
//! in rpmsg_config.h files for the master and the remote side, 4 buffers
//! in total are created in the shared memory.
#define RL_BUFFER_COUNT (256U)

#else
//! Define the buffer payload and count per different link IDs (rpmsg_lite instance) when RL_ALLOW_CUSTOM_SHMEM_CONFIG
//! is set.
//! Refer to the rpmsg_plaform.h for the used link IDs.
#define RL_BUFFER_PAYLOAD_SIZE(link_id) (496U)
#define RL_BUFFER_COUNT(link_id)        ((((link_id) == 0U) || ((link_id) == 1U)) ? 256U : 2U)

#endif /* !(defined(RL_ALLOW_CUSTOM_SHMEM_CONFIG) && (RL_ALLOW_CUSTOM_SHMEM_CONFIG == 1))*/

//! @def RL_API_HAS_ZEROCOPY
//!
//! Zero-copy API functions enabled/disabled.
//! The default value is 1 (enabled).
#define RL_API_HAS_ZEROCOPY (1)

//! @def RL_USE_STATIC_API
//!
//! Static API functions (no dynamic allocation) enabled/disabled.
//! The default value is 0 (static API disabled).
#define RL_USE_STATIC_API (0)

//! @def RL_CLEAR_USED_BUFFERS
//!
//! Clearing used buffers before returning back to the pool of free buffers
//! enabled/disabled.
//! The default value is 0 (disabled).
#define RL_CLEAR_USED_BUFFERS (0)

//! @def RL_USE_MCMGR_IPC_ISR_HANDLER
//!
//! When enabled IPC interrupts are managed by the Multicore Manager (IPC
//! interrupts router), when disabled RPMsg-Lite manages IPC interrupts
//! by itself.
//! The default value is 0 (no MCMGR IPC ISR handler used).
#define RL_USE_MCMGR_IPC_ISR_HANDLER (0)

//! @def RL_USE_ENVIRONMENT_CONTEXT
//!
//! When enabled the environment layer uses its own context.
//! Added for QNX port mainly, but can be used if required.
//! The default value is 0 (no context, saves some RAM).
#define RL_USE_ENVIRONMENT_CONTEXT (0)

//! @def RL_DEBUG_CHECK_BUFFERS
//!
//! Do not use in RPMsg-Lite to Linux configuration
#define RL_DEBUG_CHECK_BUFFERS (0)
//@}

//! @def MU0_A_IRQHandler
//!
//! MU0_A_IRQHandler handled by application, rename the handler name in RPMsg.
#define MU0_A_IRQHandler RPMsg_MU0_A_IRQHandler

#endif /* RPMSG_CONFIG_H_ */
