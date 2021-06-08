/*
 * Copyright 2017-2020, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_SRTM_H_
#define _APP_SRTM_H_

#include "rpmsg_lite.h"
#include "rsc_table.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* IRQ handler priority definition, bigger number stands for lower priority */
#define APP_LPI2C0_IRQ_PRIO     (5U)
#define APP_LPI2C3_IRQ_PRIO     (5U)
#define APP_A7Reset_IRQ_PRIO    (4U) /* CA7 Reset IRQ priority */
#define APP_SAI_TX_DMA_IRQ_PRIO (5U)
#define APP_SAI_RX_DMA_IRQ_PRIO (5U)
#define APP_SAI_IRQ_PRIO        (5U)
#define APP_GPIO_IRQ_PRIO       (5U)
#define APP_LLWU0_IRQ_PRIO      (5U)
#define APP_SNVS_IRQ_PRIO       (5U)
#define APP_USB_IRQ_PRIO        (5U)

/* Task priority definition, bigger number stands for higher priority */
#define APP_SRTM_MONITOR_TASK_PRIO    (4U)
#define APP_SRTM_DISPATCHER_TASK_PRIO (3U)

/* I2C baud rate */
#define APP_LPI2C0_BAUDRATE (100000U)
#define APP_LPI2C3_BAUDRATE (100000U)

/* Macro used to release I2C bus in initialization */
#define APP_LPI2C_DELAY           (100U)
#define APP_PF1550_LPI2C_SCL_GPIO GPIOB
#define APP_PF1550_LPI2C_SCL_PIN  (12U)
#define APP_PF1550_LPI2C_SDA_GPIO GPIOB
#define APP_PF1550_LPI2C_SDA_PIN  (13U)
#define APP_AUDIO_LPI2C_SCL_GPIO  GPIOA
#define APP_AUDIO_LPI2C_SCL_PIN   (16U)
#define APP_AUDIO_LPI2C_SDA_GPIO  GPIOA
#define APP_AUDIO_LPI2C_SDA_PIN   (17U)

/* SAI DMA channel */
#define APP_SAI_TX_DMA_CHANNEL (1U)
#define APP_SAI_RX_DMA_CHANNEL (0U)

/* Define the period to polling the CA7 core suspend status */
#define APP_SUSPEND_TIMER_PERIOD_MS (200U)
/* Define the timeout ms to receive the heart beat message */
#define APP_HEART_BEAT_TIMER_PERIOD_MS (31000U)
/* Define the timeout ms to polling the CA7 link up status */
#define APP_LINKUP_TIMER_PERIOD_MS (10U)

#define RPMSG_LITE_SRTM_SHMEM_BASE   (VDEV0_VRING_BASE)
#define RPMSG_LITE_SRTM_LINK_ID      (0U)
#define APP_SRTM_PMIC_CHANNEL_NAME   "rpmsg-regulator-channel"
#define APP_SRTM_LFCL_CHANNEL_NAME   "rpmsg-life-cycle-channel"
#define APP_SRTM_AUDIO_CHANNEL_NAME  "rpmsg-audio-channel"
#define APP_SRTM_KEYPAD_CHANNEL_NAME "rpmsg-keypad-channel"
#define APP_SRTM_IO_CHANNEL_NAME     "rpmsg-io-channel"
#define APP_SRTM_RTC_CHANNEL_NAME    "rpmsg-rtc-channel"
#define APP_SRTM_SENSOR_CHANNEL_NAME "rpmsg-sensor-channel"

#define APP_PIN_ONOFF        (0xFFFFU) /* SNVS Power Pin */
#define APP_PIN_A7_POW_EN    (0x0019U) /* PTA25 */
#define APP_PIN_DDR_SW_EN    (0x0106U) /* PTB6 */
#define APP_PIN_BT_REG_ON    (0x000FU) /* PTA15 */
#define APP_PIN_WL_REG_ON    (0x000EU) /* PTA14 */
#define APP_PIN_BT_HOST_WAKE (0x0107U) /* PTB7 */
#define APP_PIN_WL_HOST_WAKE (0x001FU) /* PTA31 */
#define APP_PIN_VOL_MINUS    (0x000DU) /* PTA13 */
#define APP_PIN_VOL_PLUS     (0x0003U) /* PTA3 */

/* Keypad index in Linux OS */
#define APP_KEYPAD_INDEX_ONOFF     (116U)
#define APP_KEYPAD_INDEX_VOL_MINUS (114U)
#define APP_KEYPAD_INDEX_VOL_PLUS  (115U)

/* LLWU module index */
#define LLWU_MODULE_LPTMR0 (0U)
#define LLWU_MODULE_LPTMR1 (1U)
#define LLWU_MODULE_CMP0   (2U)
#define LLWU_MODULE_CMP1   (3U)
#define LLWU_MODULE_SNVS   (4U)
#define LLWU_MODULE_USBPHY (6U)

#define APP_PEDOMETER_POLL_DELAY_MIN     (500U)                              /* Half second. */
#define APP_PEDOMETER_POLL_DELAY_MAX     (3600000U)                          /* 1 hour. */
#define APP_PEDOMETER_SAMPLE_RATE        (50U)                               /* sample 50 times per second. */
#define APP_PEDOMETER_SAMPLE_WINDOW      (1000U / APP_PEDOMETER_SAMPLE_RATE) /* sample every 20ms. */
#define APP_PEDOMETER_SENSOR_SAMPLE_RATE (FXOS8700_CTRL_REG1_DR_SINGLE_50_HZ)

#define APP_GPIO_IDX(ioId) ((uint8_t)(((uint16_t)ioId) >> 8U))
#define APP_PIN_IDX(ioId)  ((uint8_t)ioId)

typedef void (*app_rpmsg_monitor_t)(struct rpmsg_lite_instance *rpmsgHandle, bool ready, void *param);
typedef void (*app_irq_handler_t)(IRQn_Type irq, void *param);

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/* Release I2C buses used by SRTM services */
void APP_SRTM_I2C_ReleaseBus(void);

/* Initialize SRTM contexts */
void APP_SRTM_Init(void);

/* SRTM suspend: we need to save SRTM context */
void APP_SRTM_Suspend(void);

/* SRTM resume: we need to restore SRTM context.
   Bool param means if this is a real resume or just recovery from failed power mode switch.*/
void APP_SRTM_Resume(bool resume);

/* If CA7 is not running, power on and kick off, then create the communication channels.
   Otherwise only create the communication channels */
void APP_SRTM_BootCA7(void);

/* Reboot CA7 core */
void APP_SRTM_RebootCA7(void);

/* Shutdown CA7 core */
void APP_SRTM_ShutdownCA7(void);

/* Wakeup CA7 core from suspend mode */
void APP_SRTM_WakeupCA7(void);

/* Get PMIC register */
uint32_t APP_SRTM_GetPmicReg(uint32_t reg);

/* Set PMIC register */
void APP_SRTM_SetPmicReg(uint32_t reg, uint32_t value);

/* Toggle SW3 regulator (on/off) */
void APP_SRTM_ToggleSW3(void);

/* Enable or disable wakeup pin
 * event[7:0]: llwu_external_pin_mode_t
 * event[8]: LLWU wakeup enable
 */
void APP_SRTM_SetWakeupPin(uint16_t ioId, uint16_t event);

/* Enable or disable LLWU wakeup module */
void APP_SRTM_SetWakeupModule(uint32_t module, bool enable);

/* Set RPMsg channel init/deinit monitor */
void APP_SRTM_SetRpmsgMonitor(app_rpmsg_monitor_t monitor, void *param);

/* Set SNVS IRQ handler for application */
void APP_SRTM_SetIRQHandler(app_irq_handler_t handler, void *param);

/* Get SRTM RPMsg handle */
struct rpmsg_lite_instance *APP_SRTM_GetRPMsgHandle(void);

#if defined(__cplusplus)
}
#endif

#endif /* _APP_SRTM_H_ */
