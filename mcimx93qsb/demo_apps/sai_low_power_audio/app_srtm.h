/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_SRTM_H_
#define _APP_SRTM_H_

#include "rpmsg_lite.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* IRQ handler priority definition, bigger number stands for lower priority */

/* Task priority definition, bigger number stands for higher priority */
#define APP_SRTM_MONITOR_TASK_PRIO    (4U)
#define APP_SRTM_DISPATCHER_TASK_PRIO (3U)
#define APP_LPI2C_IRQ_PRIO            (5U)
#define APP_SAI_TX_DMA_IRQ_PRIO       (5U)
#define APP_SAI_RX_DMA_IRQ_PRIO       (5U)
#define APP_SAI_IRQ_PRIO              (5U)
#define APP_PDM_DMA_IRQ_PRIO          (5U)
#define APP_M2M_DMA_IRQ_PRIO          (5U)

/* Define the timeout ms to polling the A Core link up status */
#define APP_LINKUP_TIMER_PERIOD_MS (10U)

#define RPMSG_LITE_SRTM_SHMEM_BASE (VDEV0_VRING_BASE)
#define RPMSG_LITE_SRTM_LINK_ID    (RL_PLATFORM_IMX93_M33_A55_SRTM_LINK_ID)

#define APP_SRTM_AUDIO_CHANNEL_NAME "rpmsg-audio-channel"
#define APP_SRTM_PDM_CHANNEL_NAME   "rpmsg-micfil-channel"
#define APP_SRTM_I2C_CHANNEL_NAME   "rpmsg-i2c-channel"

#define PEER_CORE_ID (1U)

#define APP_SRTM_SAI           (SAI3)
#define APP_SRTM_SAI_IRQn      SAI3_IRQn
#define APP_DMA3_IRQN(channel) (IRQn_Type)((uint32_t)DMA3_0_IRQn + channel)
#define APP_DMA4_IRQN(channel) (IRQn_Type)((uint32_t)DMA4_0_1_IRQn + (channel >> 1))
#define APP_PDM_DMA_IRQn       DMA3_29_IRQn

#define APP_SRTM_PDM (PDM)
#define APP_SRTM_DMA ((EDMA_Type *)DMA3)

/* I2C service */
#define LPI2C1_BAUDRATE              (400000)
#define I2C_SOURCE_CLOCK_FREQ_LPI2C1 CLOCK_GetIpFreq(kCLOCK_Root_Lpi2c1)

#define LPI2C2_BAUDRATE              (400000)
#define I2C_SOURCE_CLOCK_FREQ_LPI2C2 CLOCK_GetIpFreq(kCLOCK_Root_Lpi2c2)

#define I2C_SWITCH_NONE 1

/* Audio service */
#define APP_SAI_TX_DMA_CHANNEL (2U)
#define APP_SAI_RX_DMA_CHANNEL (3U)

/* PDM service */
#define APP_PDM_RX_DMA_CHANNEL kDma3RequestMuxPDMRequest
// #define APP_PDM_RX_DMA_SOURCE           (24U)
// #define APP_PDM_RX_DMA_CHANNEL_PRIORITY (2U)
#define APP_PDM_QUALITY_MODE        (kPDM_QualityModeHigh)
#define APP_PDM_CICOVERSAMPLE_RATE  (0U)
#define APP_PDM_CHANNEL_GAIN        (kPDM_DfOutputGain4)
#define APP_PDM_CHANNEL_CUTOFF_FREQ (kPDM_DcRemoverCutOff152Hz)

/* Copies to/from external buffer  */
#define APP_MEM2MEM_W_DMA_CHANNEL (55)
#define APP_MEM2MEM_R_DMA_CHANNEL (56)

typedef void (*app_rpmsg_monitor_t)(struct rpmsg_lite_instance *rpmsgHandle, bool ready, void *param);
typedef void (*app_irq_handler_t)(IRQn_Type irq, void *param);

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum
{
    APP_SRTM_StateRun = 0x0U,
    APP_SRTM_StateLinkedUp,
    APP_SRTM_StateReboot,
    APP_SRTM_StateShutdown,
} app_srtm_state_t;

typedef enum
{
    RunMode = 0x0U,
    WaitMode,
    StopMode,
} m_core_mode_e;

/* Initialize SRTM contexts */
void APP_SRTM_Init(void);

/* Create RPMsg channel and start SRTM communication */
void APP_SRTM_StartCommunication(void);

/* Set RPMsg channel init/deinit monitor */
void APP_SRTM_SetRpmsgMonitor(app_rpmsg_monitor_t monitor, void *param);

#if defined(__cplusplus)
}
#endif

#endif /* _APP_SRTM_H_ */
