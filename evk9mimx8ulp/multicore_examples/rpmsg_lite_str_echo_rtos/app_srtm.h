/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_SRTM_H_
#define _APP_SRTM_H_

#include "rpmsg_lite.h"
#include "fsl_lsm.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*
 * AD: Application Domain
 * LP: Low Power
 * Low Power Modes for Application Domain is indroduced in AD_PMCTRL of CMC1:
 * Active,
 * Sleep,
 * Deep Sleep,
 * Partial Active,
 * Power Down(PD),
 * Deep Power Down(DPD),
 * Hold
 */
typedef enum
{
    AD_UNKOWN,
    AD_ACT, /* Note: linux is in idle state(Switch between Active mode and Sleep Mode of APD) */
    AD_DSL, /* Application Domain enter Deep Sleep Mode when linux execute suspend command(echo mem > /sys/power/state,
                suspend to ram) */
    AD_PD,  /* Application Domain enter Power Down Mode when linux execute suspend command(echo mem > /sys/power/state,
                 suspend to ram) */
    AD_DPD, /* Application Domian enter Deep Power Down Mode when linux execute poweroff command */
} lpm_ad_power_mode_e;

typedef enum
{
    APP_SRTM_StateRun = 0x0U,
    APP_SRTM_StateLinkedUp,
    APP_SRTM_StateReboot,
    APP_SRTM_StateShutdown,
} app_srtm_state_t;

#define APP_SRTM_SAI      (SAI0)
#define APP_SRTM_SAI_IRQn SAI0_IRQn
#define APP_SRTM_PDM_IRQn PDM_EVENT_IRQn

#define APP_MS2TICK(ms)       ((ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS)
#define APP_DMA_IRQN(channel) (IRQn_Type)((uint32_t)DMA0_0_IRQn + channel)

enum
{
    APP_INPUT_RTD_BTN1   = 0U,
    APP_INPUT_RTD_BTN2   = 1U,
    APP_INPUT_PTA19      = 2U,
    APP_INPUT_IT6161_INT = 2U,
    APP_INPUT_PTB5       = 3U,
    APP_INPUT_TOUCH_INT  = 3U,
    APP_IO_NUM           = 4U
};

/* Define macros for input gpios that setup by linux that running on A Core(CA35) */
#define APP_INPUT_GPIO_CONTROL_BY_ACORE_START APP_INPUT_PTA19
#define APP_INPUT_GPIO_CONTROL_BY_ACORE_END   APP_INPUT_PTB5

#define APP_INPUT_GPIO_START APP_INPUT_RTD_BTN1
#define APP_INPUT_GPIO_END   APP_INPUT_PTB5

#define APP_GPIO_START APP_INPUT_RTD_BTN1

/* Task priority definition, bigger number stands for higher priority */
#define APP_SRTM_MONITOR_TASK_PRIO    (4U)
#define APP_SRTM_DISPATCHER_TASK_PRIO (3U)

/* IRQ handler priority definition, bigger number stands for lower priority */
#define APP_LPI2C_IRQ_PRIO      (5U)
#define APP_SAI_TX_DMA_IRQ_PRIO (5U)
#define APP_SAI_RX_DMA_IRQ_PRIO (5U)
#define APP_SAI_IRQ_PRIO        (5U)
#define APP_PDM_RX_DMA_IRQ_PRIO (5U)
#define APP_PDM_IRQ_PRIO        (5U)
#define APP_GPIO_IRQ_PRIO       (5U)
#define APP_WUU_IRQ_PRIO        (5U)
#define APP_CMC1_IRQ_PRIO       (5U)
#define APP_BBNSM_IRQ_PRIO      (5U)

/* Define the timeout ms to polling the A Core link up status */
#define APP_LINKUP_TIMER_PERIOD_MS (10U)

/* Define the timeout ms to refresh s400 watchdog timer to keep s400 alive(23 hours) */
#define APP_REFRESH_S400_WDG_TIMER_PERIOD_MS (23 * 60 * 60 * 1000U)

/* Define the timeout ms to send rtc alarm event */
#define APP_RTC_ALM_EVT_TIMER_PERIOD_MS (50U)

/* Define the timeout ms to send sensor tilt wakeup event */
#define APP_SENSOR_TILT_WAKEUP_EVT_TIMER_PERIOD_MS (50U)

#define RPMSG_LITE_SRTM_SHMEM_BASE (VDEV0_VRING_BASE)
#define RPMSG_LITE_SRTM_LINK_ID    (RL_PLATFORM_IMX8ULP_M33_A35_SRTM_LINK_ID)

#define APP_SRTM_I2C_CHANNEL_NAME    "rpmsg-i2c-channel"
#define APP_SRTM_AUDIO_CHANNEL_NAME  "rpmsg-audio-channel"
#define APP_SRTM_KEYPAD_CHANNEL_NAME "rpmsg-keypad-channel"
#define APP_SRTM_IO_CHANNEL_NAME     "rpmsg-io-channel"
#define APP_SRTM_PWM_CHANNEL_NAME    "rpmsg-pwm-channel"
#define APP_SRTM_RTC_CHANNEL_NAME    "rpmsg-rtc-channel"
#define APP_SRTM_LFCL_CHANNEL_NAME   "rpmsg-life-cycle-channel"
#define APP_SRTM_SENSOR_CHANNEL_NAME "rpmsg-sensor-channel"
#define APP_SRTM_PDM_CHANNEL_NAME    "rpmsg-micfil-channel"

#define PEER_CORE_ID (1U)

/* I2C service */
#define LPI2C0_BAUDRATE              (400000)
#define I2C_SOURCE_CLOCK_FREQ_LPI2C0 CLOCK_GetIpFreq(kCLOCK_Lpi2c0)

#define LPI2C1_BAUDRATE              (400000)
#define I2C_SOURCE_CLOCK_FREQ_LPI2C1 CLOCK_GetIpFreq(kCLOCK_Lpi2c1)

#define I2C_SWITCH_NONE 1

/* Audio service */
#define APP_SAI_TX_DMA_CHANNEL (16U)
#define APP_SAI_RX_DMA_CHANNEL (17U)

/* Sensor service */
#define APP_PEDOMETER_POLL_DELAY_MIN (500U)                              /* Half second. */
#define APP_PEDOMETER_POLL_DELAY_MAX (3600000U)                          /* 1 hour. */
#define APP_PEDOMETER_SAMPLE_RATE    (50U)                               /* sample 50 times per second. */
#define APP_PEDOMETER_SAMPLE_WINDOW  (1000U / APP_PEDOMETER_SAMPLE_RATE) /* sample every 20ms. */

/* Keypad index */
#define APP_KEYPAD_INDEX_VOL_MINUS (114U)
#define APP_KEYPAD_INDEX_VOL_PLUS  (115U)

/* WUU module index */
#define WUU_MODULE_LPTMR0 (0U)
#define WUU_MODULE_LPTMR1 (1U)
#define WUU_MODULE_CMP0   (2U)
#define WUU_MODULE_CMP1   (3U)
#define WUU_MODULE_UPOWER (4U)
#define WUU_MODULE_TAMPER (5U)
#define WUU_MODULE_NSRTC  (6U)
#define WUU_MODULE_SRTC   (7U)
/* WUU wakeup event for internal module lptimer1  */
#define LPTMR1_WUU_WAKEUP_EVENT (kWUU_InternalModuleInterrupt)

/* GPIO */
#define APP_GPIO_IDX(ioId) ((uint8_t)(((uint16_t)ioId) >> 8U))
#define APP_PIN_IDX(ioId)  ((uint8_t)ioId)
#define APP_GPIO_INT_SEL   (kRGPIO_InterruptOutput2)
#define APP_PIN_PTA19      (0x0013U)          /* PTA19 use for it6161(mipi to hdmi converter ic) interrupt */
#define APP_PIN_IT6161_INT (APP_PIN_PTA19)
#define APP_PIN_PTB5       (0x0105U)          /* PTB5, use for touch interrupt */
#define APP_PIN_TOUCH_INT  (APP_PIN_PTB5)
#define APP_PIN_PTB4       (0x0104U)          /* PTB4 */
#define APP_PIN_RTD_BTN1   (0x010DU)          /* PTB13 */
#define APP_PIN_RTD_BTN2   (0x010CU)          /* PTB12 */

/*
 * BOARD Relative Settings:
 * LSM6DSO INT PIN(INT1_B) --> SOC(PTB4)
 * Note: Choose the falling edge trigger type to fix the issue that soc cannot get the interrupt from multiple sensors
 */
#define APP_LSM6DSO_INT1_B_PIN (APP_PIN_PTB4) /* Interrupt pin connected to LSM6DSO(sensor) */
#define APP_LSM6DSO_INT_ACTIVE_LEVEL (LSM_INT_ACTIVE_HIGH)
#if (APP_LSM6DSO_INT_ACTIVE_LEVEL == LSM_INT_ACTIVE_HIGH)
#define APP_LSM6DSO_INT_TRIGGER_TYPE (kRGPIO_InterruptFallingEdge)
#else
#define APP_LSM6DSO_INT_TRIGGER_TYPE (kRGPIO_InterruptRisingEdge)
#endif

/* PDM service */
#define APP_PDM_RX_DMA_CHANNEL                  0
#define APP_PDM_RX_DMA_SOURCE                   (kDmaRequestMux0MICFIL)
// #define APP_PDM_RX_DMA_CHANNEL_PRIORITY      (2U)
#define APP_PDM_QUALITY_MODE                    (kPDM_QualityModeHigh)
#define APP_PDM_CICOVERSAMPLE_RATE              (0U)
#define APP_PDM_CHANNEL_GAIN                    (kPDM_DfOutputGain4)
#define APP_PDM_CHANNEL_CUTOFF_FREQ             (kPDM_DcRemoverCutOff152Hz)
#define APP_PDM_CLOCK                           (24000000U)

extern int32_t RPMsg_MU0_A_IRQHandler(void);

typedef void (*app_rpmsg_monitor_t)(struct rpmsg_lite_instance *rpmsgHandle, bool ready, void *param);
typedef void (*app_irq_handler_t)(IRQn_Type irq, void *param);

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

/* Set RPMsg channel init/deinit monitor */
void APP_SRTM_SetRpmsgMonitor(app_rpmsg_monitor_t monitor, void *param);

uint8_t APP_Read_I2C_Register(uint8_t busID, uint16_t slaveAddr, uint8_t regIndex);
uint8_t APP_Write_I2C_Register(uint8_t busID, uint16_t slaveAddr, uint8_t regIndex, uint8_t value);

/* Set IRQ handler for application */
void APP_SRTM_SetIRQHandler(app_irq_handler_t handler, void *param);

/* Enable or disable wakeup pin
 * event[7:0]: llwu_external_pin_mode_t
 * event[8]: LLWU wakeup enable
 */
void APP_SRTM_SetWakeupPin(uint16_t ioId, uint16_t event);

/* Setup Internal modules' event as the wake up soures */
void APP_SRTM_SetWakeupModule(uint32_t module, uint16_t event);
/* Clear Internal modules' event */
void APP_SRTM_ClrWakeupModule(uint32_t module, uint16_t event);

void APP_SRTM_Suspend(void);
void APP_SRTM_Resume(bool resume);

/* Enable/Disable LPAV DDR function*/
void APP_SRTM_PreCopyDRAMCallback(void);
void APP_SRTM_PostCopyDRAMCallback(void);
void APP_SRTM_DisableLPAV(void);
void APP_SRTM_EnableLPAV(void);
bool APP_SRTM_GetSupportDSLForApd(void);
void APP_SRTM_SetSupportDSLForApd(bool support);
#if defined(__cplusplus)
}
#endif

#endif /* _APP_SRTM_H_ */
