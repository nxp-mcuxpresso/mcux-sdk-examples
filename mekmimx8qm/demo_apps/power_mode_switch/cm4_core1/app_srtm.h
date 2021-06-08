/*
 * Copyright (c) 2017-2020, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_SRTM_H_
#define _APP_SRTM_H_

#include "rpmsg_lite.h"
#include "rsc_table.h"
#include "srtm_i2c_service.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Task priority definition, bigger number stands for higher priority */
#define APP_SRTM_MONITOR_TASK_PRIO    (4U)
#define APP_SRTM_DISPATCHER_TASK_PRIO (3U)

/* Define the timeout ms to polling the Linux link up status */
#define APP_LINKUP_TIMER_PERIOD_MS (10U)

#define APP_LPI2C_BAUDRATE    (400000U)
#define APP_BB_LPI2C_IRQ_PRIO (5U)
#define I2C_SWITCH_NONE       0 /*None I2C SWITCH on this board */

#define RPMSG_LITE_SRTM_SHMEM_BASE (VDEV0_VRING_BASE)
#define RPMSG_LITE_SRTM_LINK_ID    (RL_PLATFORM_IMX8QM_M4_A_SRTM_LINK_ID)
#define APP_SRTM_I2C_CHANNEL_NAME  "rpmsg-i2c-channel"

/* The MU used for SRTM RPMSG Endpoint. */
#if defined(MIMX8QM_CM4_CORE0)
#define APP_KERNEL_MU LSIO__MU5_B
#elif defined(MIMX8QM_CM4_CORE1)
#define APP_KERNEL_MU LSIO__MU6_B
#else
#error "Unsupported CPU core!"
#endif

typedef void (*app_rpmsg_monitor_t)(struct rpmsg_lite_instance *rpmsgHandle, bool ready, void *param);

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/* Initialize SRTM contexts */
void APP_SRTM_Init(void);

/* Create RPMsg channel and start SRTM communication */
void APP_SRTM_StartCommunication(void);

/* Save SRTM service information in suspend */
bool APP_SRTM_Suspend(void);

/* Restore SRTM service in resume */
void APP_SRTM_Resume(void);

/* Handle Peer core reboot event */
void APP_SRTM_PeerCoreRebootHandler(void);

/* Read register using I2C service */
uint8_t APP_Read_I2C_Register(uint8_t busID, uint16_t slaveAddr, uint8_t regIndex);

/* Write register using I2C service */
uint8_t APP_Write_I2C_Register(uint8_t busID, uint16_t slaveAddr, uint8_t regIndex, uint8_t value);
#if defined(__cplusplus)
}
#endif

#endif /* _APP_SRTM_H_ */
