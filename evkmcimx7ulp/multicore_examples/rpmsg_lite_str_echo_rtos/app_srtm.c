/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "fsl_debug_console.h"
#include "fsl_lpi2c_freertos.h"
#include "fsl_gpio.h"
#include "fsl_pmc0.h"
#include "fsl_msmc.h"
#include "fsl_mu.h"
#include "fsl_edma.h"
#include "fsl_dmamux.h"
#include "fsl_pf1550.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "fsl_llwu.h"
#include "fsl_snvs_lp.h"
#include "fsl_snvs_hp.h"
#include "fsl_iomuxc.h"
#include "fsl_wm8960.h"

#include "srtm_dispatcher.h"
#include "srtm_peercore.h"
#include "srtm_message.h"
#include "srtm_lfcl_service.h"
#include "srtm_pmic_service.h"
#include "srtm_audio_service.h"
#include "srtm_rtc_service.h"
#include "srtm_io_service.h"
#include "srtm_keypad_service.h"
#include "srtm_sensor_service.h"
#include "srtm_pf1550_adapter.h"
#include "srtm_i2c_codec_adapter.h"
#include "srtm_snvs_lp_rtc_adapter.h"
#include "srtm_sai_edma_adapter.h"
#include "srtm_rpmsg_endpoint.h"

#include "fxos8700.h"
#include "KeynetikPedometer.h"

#include "app_srtm.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_MS2TICK(ms) ((ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS)

#define APP_DMA_IRQN(channel) (IRQn_Type)(((uint32_t)DMA0_0_4_IRQn + (((channel) >> 3U) << 2U) + ((channel)&3U)))

enum
{
    APP_INPUT_ONOFF        = 0U,
    APP_INPUT_VOL_PLUS     = 1U,
    APP_INPUT_VOL_MINUS    = 2U,
    APP_INPUT_BT_HOST_WAKE = 3U,
    APP_INPUT_WL_HOST_WAKE = 4U,

    APP_OUTPUT_WL_REG_ON = 5U,
    APP_OUTPUT_BT_REG_ON = 6U,
    APP_IO_NUM           = 7U
};

#define APP_INPUT_GPIO_START  APP_INPUT_VOL_PLUS
#define APP_OUTPUT_GPIO_START APP_OUTPUT_WL_REG_ON

typedef enum
{
    APP_SRTM_StateRun = 0x0U,
    APP_SRTM_StateLinkedUp,
    APP_SRTM_StateReboot,
    APP_SRTM_StateShutdown,
} app_srtm_state_t;

typedef enum
{
    APP_SRTM_PowerOn = 0x0U,
    APP_SRTM_PowerOffVlls,
    APP_SRTM_PowerOff,
} app_srtm_power_t;

typedef struct
{
    uint16_t ioId;
    TimerHandle_t timer; /* GPIO glitch detect timer */
    srtm_io_event_t event;
    bool wakeup;
    bool override; /* Means the CA7 pin configuration is overridden by CM4 wakeup pin. */
    uint8_t index;
    uint8_t value;
} app_io_t;

/* NOTE: CM4 DRIVERS DON'T SUPPORT SAVE CONTEXT FOR RESUME, BUT CA7 LINUX DRIVERS DO.
 * WHEN CM4 CORE RUNS INTO VLLS MODE, MOST PERIPHERALS STATE WILL BE LOST. HERE PROVIDES
 * AN EXAMPLE TO SAVE DEVICE STATE BY APPLICATION IN A SUSPEND CONTEXT LOCATING IN TCM
 * WHICH CAN KEEP DATA IN VLLS MODE.
 */
typedef struct
{
    struct
    {
        app_io_t data[APP_IO_NUM];
    } io;
    struct
    {
        uint32_t CR;
    } mu;
} app_suspend_ctx_t;

typedef struct
{
    bool stateEnabled;
    bool dataEnabled;
    uint32_t pollDelay;
    uint32_t expired;    /* milli-seconds since last data report. */
    uint32_t lastCount;  /* step counter reported last time. */
    TimerHandle_t timer; /* Sensor polling timer */
} app_pedometer_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* Send data to PMIC device on I2C Bus. */
static status_t PMIC_I2C_SendFunc(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize);

/* Receive data from PMIC device on I2C Bus. */
static status_t PMIC_I2C_ReceiveFunc(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize);

/* Send data to Sensor device on I2C Bus. */
static status_t Sensor_I2C_SendFunc(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize);

/* Receive data from Sensor device on I2C Bus. */
static status_t Sensor_I2C_ReceiveFunc(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize);

static srtm_status_t APP_SRTM_Sensor_EnableStateDetector(srtm_sensor_adapter_t adapter,
                                                         srtm_sensor_type_t type,
                                                         uint8_t index,
                                                         bool enable);
static srtm_status_t APP_SRTM_Sensor_EnableDataReport(srtm_sensor_adapter_t adapter,
                                                      srtm_sensor_type_t type,
                                                      uint8_t index,
                                                      bool enable);
static srtm_status_t APP_SRTM_Sensor_SetPollDelay(srtm_sensor_adapter_t adapter,
                                                  srtm_sensor_type_t type,
                                                  uint8_t index,
                                                  uint32_t millisec);

extern void APP_UpdateSimDgo(uint32_t gpIdx, uint32_t mask, uint32_t value);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const uint16_t llwuPins[] = {
    0x0000U, /* LLWU_P0 PTA0 */
    0x0003U, /* LLWU_P1 PTA3 */
    0x000DU, /* LLWU_P2 PTA13 */
    0x000EU, /* LLWU_P3 PTA14 */
    0x0012U, /* LLWU_P4 PTA18 */
    0x0013U, /* LLWU_P5 PTA19 */
    0x0017U, /* LLWU_P6 PTA23 */
    0x001FU, /* LLWU_P7 PTA31 */
    0x0101U, /* LLWU_P8 PTB1 */
    0x0103U, /* LLWU_P9 PTB3 */
    0x0106U, /* LLWU_P10 PTB6 */
    0x0107U, /* LLWU_P11 PTB7 */
    0x0109U, /* LLWU_P12 PTB9 */
    0x010EU, /* LLWU_P13 PTB14 */
    0x0110U, /* LLWU_P14 PTB16 */
    0x0113U, /* LLWU_P15 PTB19 */
};

static const srtm_io_event_t onoffKeyEvents[] = {
    SRTM_IoEventNone,        /* SRTM_KeypadEventNone */
    SRTM_IoEventFallingEdge, /* SRTM_KeypadEventPress */
    SRTM_IoEventRisingEdge,  /* SRTM_KeypadEventRelease */
    SRTM_IoEventEitherEdge   /* SRTM_KeypadEventPressOrRelease */
};

static const srtm_io_event_t volPlusKeyEvents[] = {
    SRTM_IoEventNone,        /* SRTM_KeypadEventNone */
    SRTM_IoEventRisingEdge,  /* SRTM_KeypadEventPress */
    SRTM_IoEventFallingEdge, /* SRTM_KeypadEventRelease */
    SRTM_IoEventEitherEdge   /* SRTM_KeypadEventPressOrRelease */
};

static const srtm_io_event_t volMinusKeyEvents[] = {
    SRTM_IoEventNone,        /* SRTM_KeypadEventNone */
    SRTM_IoEventRisingEdge,  /* SRTM_KeypadEventPress */
    SRTM_IoEventFallingEdge, /* SRTM_KeypadEventRelease */
    SRTM_IoEventEitherEdge   /* SRTM_KeypadEventPressOrRelease */
};

static const srtm_io_event_t llwuPinModeEvents[] = {
    SRTM_IoEventNone,        /* kLLWU_ExternalPinDisable */
    SRTM_IoEventRisingEdge,  /* kLLWU_ExternalPinRisingEdge */
    SRTM_IoEventFallingEdge, /* kLLWU_ExternalPinFallingEdge */
    SRTM_IoEventEitherEdge   /* kLLWU_ExternalPinAnyEdge */
};

wm8960_config_t wm8960Config;
codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8960};
codec_handle_t codecHandle;

static struct _srtm_sensor_adapter sensorAdapter = {.enableStateDetector = APP_SRTM_Sensor_EnableStateDetector,
                                                    .enableDataReport    = APP_SRTM_Sensor_EnableDataReport,
                                                    .setPollDelay        = APP_SRTM_Sensor_SetPollDelay};

static lpi2c_rtos_handle_t lpi2c0Handle;
static lpi2c_rtos_handle_t lpi2c3Handle;
static bool lpi2c0Init, lpi2c3Init;
static lpi2c_rtos_handle_t *pmicI2cHandle;
static lpi2c_rtos_handle_t *sensorI2cHandle;
static pf1550_handle_t pf1550Handle;
static srtm_dispatcher_t disp;
static srtm_peercore_t core;
static srtm_sai_adapter_t saiAdapter;
static srtm_service_t audioService;
static srtm_rtc_adapter_t rtcAdapter;
static srtm_service_t rtcService;
static srtm_service_t ioService;
static srtm_service_t keypadService;
static SemaphoreHandle_t monSig;
static TimerHandle_t suspendTimer;
static TimerHandle_t heartBeatTimer;
static volatile app_srtm_state_t srtmState;
static struct rpmsg_lite_instance *rpmsgHandle;
static app_rpmsg_monitor_t rpmsgMonitor;
static void *rpmsgMonitorParam;
static app_irq_handler_t irqHandler;
static void *irqHandlerParam;
static TimerHandle_t linkupTimer;
static app_pedometer_t pedometer = {.stateEnabled = false,
                                    .dataEnabled  = false,
                                    .pollDelay    = 1000, /* 1 sec by default. */
                                    .expired      = 0,
                                    .lastCount    = 0};

static KeynetikConfig pedoConfig = {
    /* Step length in centimeters. Auto calculate. */
    .steplength = 0,
    /* Height in centimeters */
    .height = 175,
    /* Weight in kilograms */
    .weight = 80,
    /* 4 steps in 3 seconds */
    .filtersteps = 4,
    .bits =
        {
            .filtertime = 3,
            .male       = 1,
        },
    /* Calculate speed every 5 seconds */
    .speedperiod = 5,
    /* Threshold 0.13G to decide a step. */
    .stepthreshold = 130,
};

static PORT_Type *const ports[] = PORT_BASE_PTRS;
static GPIO_Type *const gpios[] = GPIO_BASE_PTRS;

static app_suspend_ctx_t suspendContext;
static bool heartBeat;
static app_srtm_power_t peercorePower;
static srtm_procedure_t wakeupCA7Proc;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void APP_SRTM_WakeupCA7InIRQ(void)
{
    if (wakeupCA7Proc)
    {
        SRTM_Dispatcher_PostProc(disp, wakeupCA7Proc);
        wakeupCA7Proc = NULL;
    }
}

static void APP_SRTM_ConfigUSBWakeup(bool enable)
{
    if (enable && (SIM->GPR1 & SIM_GPR1_USB_PHY_WAKEUP_ISO_DISABLE_MASK))
    {
        if (!NVIC_GetEnableIRQ(USBPHY_IRQn))
        {
            PRINTF("enable USB wakeup\r\n");
            APP_UpdateSimDgo(11, SIM_SIM_DGO_GP11_USB_PHY_VLLS_WAKEUP_EN_MASK,
                             SIM_SIM_DGO_GP11_USB_PHY_VLLS_WAKEUP_EN_MASK);
            LLWU_EnableInternalModuleInterruptWakup(LLWU, LLWU_MODULE_USBPHY, true);
            EnableIRQ(USBPHY_IRQn);
        }
    }
    else if (!enable)
    {
        if (NVIC_GetEnableIRQ(USBPHY_IRQn))
        {
            PRINTF("disable USB wakeup\r\n");
            APP_UpdateSimDgo(11, SIM_SIM_DGO_GP11_USB_PHY_VLLS_WAKEUP_EN_MASK, 0);
            LLWU_EnableInternalModuleInterruptWakup(LLWU, LLWU_MODULE_USBPHY, false);
            NVIC_ClearPendingIRQ(LLWU0_IRQn);
            DisableIRQ(USBPHY_IRQn);
            NVIC_ClearPendingIRQ(USBPHY_IRQn);
        }
    }
}

static uint8_t APP_IO_GetLLWUPin(uint16_t ioId)
{
    uint8_t i;

    for (i = 0; i < ARRAY_SIZE(llwuPins); i++)
    {
        if (llwuPins[i] == ioId)
        {
            break;
        }
    }

    return i;
}

static uint8_t APP_IO_GetIoIndex(uint16_t ioId)
{
    uint8_t i;

    for (i = 0; i < ARRAY_SIZE(suspendContext.io.data); i++)
    {
        if (suspendContext.io.data[i].ioId == ioId)
        {
            break;
        }
    }

    return i;
}

static uint8_t APP_Keypad_GetInputIndex(uint8_t keyIdx)
{
    uint8_t i;

    for (i = 0; i < APP_OUTPUT_GPIO_START; i++)
    {
        if (suspendContext.io.data[i].index == keyIdx)
        {
            break;
        }
    }

    return i;
}

static srtm_io_event_t APP_Keypad_GetIoEvent(uint8_t keyIdx, srtm_keypad_event_t event)
{
    switch (keyIdx)
    {
        case APP_KEYPAD_INDEX_ONOFF:
            return onoffKeyEvents[event];
        case APP_KEYPAD_INDEX_VOL_PLUS:
            return volPlusKeyEvents[event];
        case APP_KEYPAD_INDEX_VOL_MINUS:
            return volMinusKeyEvents[event];
        default:
            assert(false);
            break;
    }

    return SRTM_IoEventNone;
}

/* LLWU interrupt handler. */
void LLWU0_IRQHandler(void)
{
    /* If application has handler */
    if (irqHandler)
    {
        irqHandler(LLWU0_IRQn, irqHandlerParam);
    }

    if (LLWU_GetInternalWakeupModuleFlag(LLWU, LLWU_MODULE_USBPHY))
    {
        /* USB wakeup interrupt need to be handled by CA7 core, here just disable IRQ and wakeup CA7. */
        APP_SRTM_ConfigUSBWakeup(false);
        APP_SRTM_WakeupCA7InIRQ();
        portYIELD_FROM_ISR(pdTRUE);
    }

    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
    exception return operation might vector to incorrect interrupt */
    __DSB();
}

void USBPHY_IRQHandler(void)
{
    /* USB wakeup interrupt need to be handled by CA7 core, here just disable IRQ and wakeup CA7. */
    APP_SRTM_ConfigUSBWakeup(false);
    APP_SRTM_WakeupCA7InIRQ();

    portYIELD_FROM_ISR(pdTRUE);

    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
    exception return operation might vector to incorrect interrupt */
    __DSB();
}

void SNVS_IRQHandler(void)
{
    BaseType_t reschedule    = pdFALSE;
    srtm_io_event_t ioPress  = APP_Keypad_GetIoEvent(APP_KEYPAD_INDEX_ONOFF, SRTM_KeypadEventPress);
    srtm_io_event_t ioEither = APP_Keypad_GetIoEvent(APP_KEYPAD_INDEX_ONOFF, SRTM_KeypadEventPressOrRelease);

    /* If application has handler */
    if (irqHandler)
    {
        irqHandler(SNVS_IRQn, irqHandlerParam);
    }
    /*
     * Process RTC alarm if present.
     * SNVS IRQ enable is done in RTC service initialization. So rtcAdapter must be ready.
     */
    SRTM_SnvsLpRtcAdapter_NotifyAlarm(rtcAdapter);

    if (SNVS->LPSR & SNVS_LPSR_SPO_MASK)
    {
        /* Clear SNVS button interrupt */
        SNVS->LPSR = SNVS_LPSR_SPO_MASK;
        if (keypadService &&
            (suspendContext.io.data[APP_INPUT_ONOFF].wakeup || MU_GetOtherCorePowerMode(MUA) != kMU_PowerModeDsm) &&
            (suspendContext.io.data[APP_INPUT_ONOFF].event == ioPress ||
             suspendContext.io.data[APP_INPUT_ONOFF].event == ioEither))
        {
            /* Only when CA7 is running or wakeup flag is set, and press event is monitored,
               we'll notify the event to CA7. */
            SRTM_KeypadService_NotifyKeypadEvent(keypadService, APP_KEYPAD_INDEX_ONOFF, SRTM_KeypadValuePressed);
        }
        xTimerStartFromISR(suspendContext.io.data[APP_INPUT_ONOFF].timer, &reschedule);
    }

    if (reschedule)
    {
        portYIELD_FROM_ISR(reschedule);
    }

    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
    exception return operation might vector to incorrect interrupt */
    __DSB();
}

static void APP_HandleGPIOHander(uint8_t gpioIdx)
{
    BaseType_t reschedule = pdFALSE;
    PORT_Type *port       = ports[gpioIdx];
    GPIO_Type *gpio       = gpios[gpioIdx];

    if (APP_GPIO_IDX(APP_PIN_BT_HOST_WAKE) == gpioIdx &&
        (1U << APP_PIN_IDX(APP_PIN_BT_HOST_WAKE)) & PORT_GetPinsInterruptFlags(port))
    {
        PORT_ClearPinsInterruptFlags(port, 1U << APP_PIN_IDX(APP_PIN_BT_HOST_WAKE));
        if (suspendContext.io.data[APP_INPUT_BT_HOST_WAKE].wakeup || MU_GetOtherCorePowerMode(MUA) != kMU_PowerModeDsm)
        {
            /* Only when CA7 is running or wakeup flag is set, we'll notify the event to CA7. */
            SRTM_IoService_NotifyInputEvent(ioService, APP_PIN_BT_HOST_WAKE);
        }
    }

    if (APP_GPIO_IDX(APP_PIN_WL_HOST_WAKE) == gpioIdx &&
        (1U << APP_PIN_IDX(APP_PIN_WL_HOST_WAKE)) & PORT_GetPinsInterruptFlags(port))
    {
        PORT_ClearPinsInterruptFlags(port, 1U << APP_PIN_IDX(APP_PIN_WL_HOST_WAKE));
        if (suspendContext.io.data[APP_INPUT_WL_HOST_WAKE].wakeup || MU_GetOtherCorePowerMode(MUA) != kMU_PowerModeDsm)
        {
            /* Only when CA7 is running or wakeup flag is set, we'll notify the event to CA7. */
            SRTM_IoService_NotifyInputEvent(ioService, APP_PIN_WL_HOST_WAKE);
        }
    }

    if (APP_GPIO_IDX(APP_PIN_VOL_PLUS) == gpioIdx &&
        (1U << APP_PIN_IDX(APP_PIN_VOL_PLUS)) & PORT_GetPinsInterruptFlags(port))
    {
        PORT_ClearPinsInterruptFlags(port, (1U << APP_PIN_IDX(APP_PIN_VOL_PLUS)));
        PORT_SetPinInterruptConfig(port, APP_PIN_IDX(APP_PIN_VOL_PLUS), kPORT_InterruptOrDMADisabled);
        suspendContext.io.data[APP_INPUT_VOL_PLUS].value = GPIO_PinRead(gpio, APP_PIN_IDX(APP_PIN_VOL_PLUS));
        xTimerStartFromISR(suspendContext.io.data[APP_INPUT_VOL_PLUS].timer, &reschedule);
    }

    if (APP_GPIO_IDX(APP_PIN_VOL_MINUS) == gpioIdx &&
        (1U << APP_PIN_IDX(APP_PIN_VOL_MINUS)) & PORT_GetPinsInterruptFlags(port))
    {
        PORT_ClearPinsInterruptFlags(port, (1U << APP_PIN_IDX(APP_PIN_VOL_MINUS)));
        PORT_SetPinInterruptConfig(port, APP_PIN_IDX(APP_PIN_VOL_MINUS), kPORT_InterruptOrDMADisabled);
        suspendContext.io.data[APP_INPUT_VOL_MINUS].value = GPIO_PinRead(gpio, APP_PIN_IDX(APP_PIN_VOL_MINUS));
        xTimerStartFromISR(suspendContext.io.data[APP_INPUT_VOL_MINUS].timer, &reschedule);
    }

    if (reschedule)
    {
        portYIELD_FROM_ISR(reschedule);
    }
}

void PCTLA_IRQHandler(void)
{
    /* If application has handler */
    if (irqHandler)
    {
        irqHandler(PCTLA_IRQn, irqHandlerParam);
    }
    APP_HandleGPIOHander(0U);

    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
      exception return operation might vector to incorrect interrupt */
    __DSB();
}

void PCTLB_IRQHandler(void)
{
    /* If application has handler */
    if (irqHandler)
    {
        irqHandler(PCTLB_IRQn, irqHandlerParam);
    }
    APP_HandleGPIOHander(1U);

    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
      exception return operation might vector to incorrect interrupt */
    __DSB();
}

static srtm_status_t APP_IO_ConfOutput(uint16_t ioId, srtm_io_value_t ioValue)
{
    uint8_t gpioIdx = APP_GPIO_IDX(ioId);
    uint8_t pinIdx  = APP_PIN_IDX(ioId);

    assert(gpioIdx < 2U); /* We only support GPIOA and GPIOB */
    assert(pinIdx < 32U);

    GPIO_PinWrite(gpios[gpioIdx], pinIdx, (uint8_t)ioValue);

    return SRTM_Status_Success;
}

static srtm_status_t APP_IO_SetOutput(srtm_service_t service,
                                      srtm_peercore_t core,
                                      uint16_t ioId,
                                      srtm_io_value_t ioValue)
{
    uint8_t index = APP_IO_GetIoIndex(ioId);

    assert(index >= APP_OUTPUT_GPIO_START && index < APP_IO_NUM);

    suspendContext.io.data[index].value = (uint8_t)ioValue;

    return APP_IO_ConfOutput(ioId, ioValue);
}

static srtm_status_t APP_IO_GetInput(srtm_service_t service,
                                     srtm_peercore_t core,
                                     uint16_t ioId,
                                     srtm_io_value_t *pIoValue)
{
    uint8_t gpioIdx = APP_GPIO_IDX(ioId);
    uint8_t pinIdx  = APP_PIN_IDX(ioId);

    assert(gpioIdx < 2U); /* We only support GPIOA and GPIOB */
    assert(pinIdx < 32U);
    assert(pIoValue);

    *pIoValue = GPIO_PinRead(gpios[gpioIdx], pinIdx) ? SRTM_IoValueHigh : SRTM_IoValueLow;

    return SRTM_Status_Success;
}

static srtm_status_t APP_IO_ConfInput(uint8_t inputIdx, srtm_io_event_t event, bool wakeup)
{
    uint16_t ioId   = suspendContext.io.data[inputIdx].ioId;
    uint8_t gpioIdx = APP_GPIO_IDX(ioId);
    uint8_t pinIdx  = APP_PIN_IDX(ioId);
    uint8_t llwuIdx = APP_IO_GetLLWUPin(ioId);

    assert(gpioIdx < 2U || gpioIdx == 0xFFU); /* We only support GPIOA and GPIOB, or the special SNVS power IO */
    assert(pinIdx < 32U || gpioIdx == 0xFFU);
    assert(llwuIdx <= ARRAY_SIZE(llwuPins));  /* When llwuIdx == ARRAY_SIZE(llwuPins),
                                                 it means there's no LLWU pin for ioId. */

    if (gpioIdx == 0xFFU)                     /* SNVS button */
    {
        if (wakeup && event != SRTM_IoEventNone)
        {
            LLWU_EnableInternalModuleInterruptWakup(LLWU, LLWU_MODULE_SNVS, true);
        }
        else
        {
            LLWU_EnableInternalModuleInterruptWakup(LLWU, LLWU_MODULE_SNVS, false);
        }
    }
    else /* Normal IO */
    {
        switch (event)
        {
            case SRTM_IoEventRisingEdge:
                PORT_SetPinInterruptConfig(ports[gpioIdx], pinIdx, kPORT_InterruptRisingEdge);
                if (wakeup)
                {
                    assert(llwuIdx < ARRAY_SIZE(llwuPins));
                    LLWU_SetExternalWakeupPinMode(LLWU, llwuIdx, kLLWU_ExternalPinRisingEdge);
                }
                break;
            case SRTM_IoEventFallingEdge:
                PORT_SetPinInterruptConfig(ports[gpioIdx], pinIdx, kPORT_InterruptFallingEdge);
                if (wakeup)
                {
                    assert(llwuIdx < ARRAY_SIZE(llwuPins));
                    LLWU_SetExternalWakeupPinMode(LLWU, llwuIdx, kLLWU_ExternalPinFallingEdge);
                }
                break;
            case SRTM_IoEventEitherEdge:
                PORT_SetPinInterruptConfig(ports[gpioIdx], pinIdx, kPORT_InterruptEitherEdge);
                if (wakeup)
                {
                    assert(llwuIdx < ARRAY_SIZE(llwuPins));
                    LLWU_SetExternalWakeupPinMode(LLWU, llwuIdx, kLLWU_ExternalPinAnyEdge);
                }
                break;
            case SRTM_IoEventLowLevel:
                PORT_SetPinInterruptConfig(ports[gpioIdx], pinIdx, kPORT_InterruptLogicZero);
                /* Power level cannot trigger wakeup */
                assert(!wakeup);
                break;
            case SRTM_IoEventHighLevel:
                PORT_SetPinInterruptConfig(ports[gpioIdx], pinIdx, kPORT_InterruptLogicOne);
                /* Power level cannot trigger wakeup */
                assert(!wakeup);
                break;
            default:
                PORT_SetPinInterruptConfig(ports[gpioIdx], pinIdx, kPORT_InterruptOrDMADisabled);
                break;
        }
        if (!wakeup && llwuIdx < ARRAY_SIZE(llwuPins))
        {
            LLWU_SetExternalWakeupPinMode(LLWU, llwuIdx, kLLWU_ExternalPinDisable);
        }
    }

    return SRTM_Status_Success;
}

static srtm_status_t APP_IO_ConfIEvent(
    srtm_service_t service, srtm_peercore_t core, uint16_t ioId, srtm_io_event_t event, bool wakeup)
{
    uint8_t inputIdx = APP_IO_GetIoIndex(ioId);

    assert(inputIdx < APP_OUTPUT_GPIO_START);

    suspendContext.io.data[inputIdx].event  = event;
    suspendContext.io.data[inputIdx].wakeup = wakeup;

    return APP_IO_ConfInput(inputIdx, event, wakeup);
}

static srtm_status_t APP_IO_ConfKEvent(
    srtm_service_t service, srtm_peercore_t core, uint8_t keyIdx, srtm_keypad_event_t event, bool wakeup)
{
    uint8_t inputIdx = APP_Keypad_GetInputIndex(keyIdx);

    assert(inputIdx < APP_OUTPUT_GPIO_START);

    suspendContext.io.data[inputIdx].event  = APP_Keypad_GetIoEvent(keyIdx, event);
    suspendContext.io.data[inputIdx].wakeup = wakeup;

    return APP_IO_ConfInput(inputIdx, suspendContext.io.data[inputIdx].event, wakeup);
}

static void APP_DisableRebootMonitor(bool disableIRQ)
{
    /* Disable CA7 Reset interrupt */
    if (disableIRQ)
    {
        DisableIRQ(CMC1_IRQn);
    }

    /* Stop heart beat timer if enabled */
    if (heartBeat)
    {
        xTimerStop(heartBeatTimer, portMAX_DELAY);
    }
}

static void APP_EnableRebootMonitor(void)
{
    /* Enable CA7 Reset interrupt to monitor CA7 reset */
    SIM->MISC_CTRL0 =
        (SIM->MISC_CTRL0 & ~SIM_MISC_CTRL0_A7_TSTMR_COMP_IRQ_CTRL_MASK) | SIM_MISC_CTRL0_A7_TO_M4_RST_IRQ_EN_MASK;
    NVIC_ClearPendingIRQ(CMC1_IRQn);
    EnableIRQ(CMC1_IRQn);

    /* Start heart beat timer if enabled */
    if (heartBeat)
    {
        xTimerStart(heartBeatTimer, portMAX_DELAY);
    }
}

static void APP_SRTM_A7ResetHandler(void)
{
    portBASE_TYPE taskToWake = pdFALSE;

    if (SIM->MISC_CTRL0 & SIM_MISC_CTRL0_A7_TO_M4_RST_IRQ_EN_MASK)
    {
        /* Clear reset flag and disable interrupt. */
        SIM->MISC_CTRL0 = (SIM->MISC_CTRL0 &
                           ~(SIM_MISC_CTRL0_A7_TSTMR_COMP_IRQ_CTRL_MASK | SIM_MISC_CTRL0_A7_TO_M4_RST_IRQ_EN_MASK)) |
                          SIM_MISC_CTRL0_A7_TO_M4_RST_IRQ_CTRL_MASK;

        /* Hold CA7 Core in Reset status. */
        MU_HardwareResetOtherCore(MUA, false, true, kMU_CoreBootFromAddr0);
        srtmState = APP_SRTM_StateReboot;

        /* Wake up monitor to reinitialize the SRTM communication with CA7 */
        if (pdPASS == xSemaphoreGiveFromISR(monSig, &taskToWake))
        {
            portYIELD_FROM_ISR(taskToWake);
        }
    }
}

void CMC1_IRQHandler(void)
{
    /* If application has handler */
    if (irqHandler)
    {
        irqHandler(CMC1_IRQn, irqHandlerParam);
    }

    PRINTF("CMC1 IRQ\r\n");
    APP_SRTM_A7ResetHandler();

    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
    exception return operation might vector to incorrect interrupt */
    __DSB();
}

static status_t I2C_SendFunc(lpi2c_rtos_handle_t *handle,
                             uint8_t deviceAddress,
                             uint32_t subAddress,
                             uint8_t subAddressSize,
                             const uint8_t *txBuff,
                             uint8_t txBuffSize)
{
    lpi2c_master_transfer_t masterXfer;

    /* Prepare transfer structure. */
    masterXfer.slaveAddress   = deviceAddress;
    masterXfer.direction      = kLPI2C_Write;
    masterXfer.subaddress     = subAddress;
    masterXfer.subaddressSize = subAddressSize;
    masterXfer.data           = (void *)txBuff;
    masterXfer.dataSize       = txBuffSize;
    masterXfer.flags          = kLPI2C_TransferDefaultFlag;

    /* Calling I2C Transfer API to start send. */
    return LPI2C_RTOS_Transfer(handle, &masterXfer);
}

static status_t I2C_ReceiveFunc(lpi2c_rtos_handle_t *handle,
                                uint8_t deviceAddress,
                                uint32_t subAddress,
                                uint8_t subAddressSize,
                                uint8_t *rxBuff,
                                uint8_t rxBuffSize)
{
    lpi2c_master_transfer_t masterXfer;

    /* Prepare transfer structure. */
    masterXfer.slaveAddress   = deviceAddress;
    masterXfer.direction      = kLPI2C_Read;
    masterXfer.subaddress     = subAddress;
    masterXfer.subaddressSize = subAddressSize;
    masterXfer.data           = rxBuff;
    masterXfer.dataSize       = rxBuffSize;
    masterXfer.flags          = kLPI2C_TransferDefaultFlag;

    /* Calling I2C Transfer API to start receive. */
    return LPI2C_RTOS_Transfer(handle, &masterXfer);
}

static status_t PMIC_I2C_SendFunc(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize)
{
    /* Calling I2C Transfer API to start send. */
    return I2C_SendFunc(pmicI2cHandle, deviceAddress, subAddress, subAddressSize, txBuff, txBuffSize);
}

static status_t PMIC_I2C_ReceiveFunc(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize)
{
    /* Calling I2C Transfer API to start receive. */
    return I2C_ReceiveFunc(pmicI2cHandle, deviceAddress, subAddress, subAddressSize, rxBuff, rxBuffSize);
}

static status_t Sensor_I2C_SendFunc(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize)
{
    /* Calling I2C Transfer API to start send. */
    return I2C_SendFunc(sensorI2cHandle, deviceAddress, subAddress, subAddressSize, txBuff, txBuffSize);
}

static status_t Sensor_I2C_ReceiveFunc(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize)
{
    /* Calling I2C Transfer API to start receive. */
    return I2C_ReceiveFunc(sensorI2cHandle, deviceAddress, subAddress, subAddressSize, rxBuff, rxBuffSize);
}

static void APP_SRTM_PollSensor(srtm_dispatcher_t dispatcher, void *param1, void *param2)
{
    FXOS8700_DR_STATUS_t drStatus;
    uint8_t readBuffer[6];
    int16_t accel[3];
    uint32_t events;
    status_t status;

    if (pedometer.stateEnabled || pedometer.dataEnabled)
    {
        status = Sensor_I2C_ReceiveFunc(FXOS8700_DEVICE_ADDR_SA_00, FXOS8700_STATUS, 1, &drStatus.w, 1);
        assert(status == kStatus_Success);
        if (status != kStatus_Success)
        {
            return;
        }

        if (drStatus.b.zyxdr != 0)
        {
            /* Have sensor data, then accumulate in pedometer library. */
            status = Sensor_I2C_ReceiveFunc(FXOS8700_DEVICE_ADDR_SA_00, FXOS8700_OUT_X_MSB, 1, readBuffer, 6);
            assert(status == kStatus_Success);
            if (status != kStatus_Success)
            {
                return;
            }

            accel[0] = (int16_t)((readBuffer[0] << 8) | readBuffer[1]) / 4;
            accel[1] = (int16_t)((readBuffer[2] << 8) | readBuffer[3]) / 4;
            accel[2] = (int16_t)((readBuffer[4] << 8) | readBuffer[5]) / 4;

            events = KeynetikHandleIncomingEvent(accel[0], accel[1], accel[2]);
            if (pedometer.stateEnabled && (events & KEYNETIK_STEP))
            {
                /* Step detected, then update peer core. */
                assert(sensorAdapter.updateState && sensorAdapter.service);
                sensorAdapter.updateState(sensorAdapter.service, SRTM_SensorTypePedometer, 0);
            }
        }

        if (pedometer.dataEnabled)
        {
            pedometer.expired += (APP_MS2TICK(APP_PEDOMETER_SAMPLE_WINDOW)) * portTICK_PERIOD_MS;
            if (pedometer.expired >= pedometer.pollDelay)
            {
                /* Report time: need to check whether need to report */
                if (pedometer.lastCount != keynetikStepCount)
                {
                    pedometer.lastCount = keynetikStepCount;
                    assert(sensorAdapter.reportData && sensorAdapter.service);
                    sensorAdapter.reportData(sensorAdapter.service, SRTM_SensorTypePedometer, 0,
                                             (uint8_t *)(&pedometer.lastCount), sizeof(pedometer.lastCount));
                }
                pedometer.expired = 0;
            }
        }
    }
}

static void APP_PedometerTimerCallback(TimerHandle_t xTimer)
{
    srtm_procedure_t proc = SRTM_Procedure_Create(APP_SRTM_PollSensor, NULL, NULL);

    /* Need to poll sensor data in SRTM task context */
    if (proc)
    {
        SRTM_Dispatcher_PostProc(disp, proc);
    }
}

static srtm_status_t APP_SRTM_Sensor_InitPedometer(void)
{
    uint8_t data;
    status_t status;

    status = Sensor_I2C_ReceiveFunc(FXOS8700_DEVICE_ADDR_SA_00, FXOS8700_WHO_AM_I, 1, &data, 1);
    assert(status == kStatus_Success && data == FXOS8700_WHO_AM_I_PROD_VALUE);

    /* Put the device into standby mode so that configuration can be applied.*/
    status = Sensor_I2C_ReceiveFunc(FXOS8700_DEVICE_ADDR_SA_00, FXOS8700_CTRL_REG1, 1, &data, 1);
    assert(status == kStatus_Success);
    data   = (data & ~FXOS8700_CTRL_REG1_ACTIVE_MASK) | FXOS8700_CTRL_REG1_ACTIVE_STANDBY_MODE;
    status = Sensor_I2C_SendFunc(FXOS8700_DEVICE_ADDR_SA_00, FXOS8700_CTRL_REG1, 1, &data, 1);
    assert(status == kStatus_Success);
    /* Configure the fxos8700 to 50Hz sampling rate. */
    data   = (data & ~FXOS8700_CTRL_REG1_DR_MASK) | APP_PEDOMETER_SENSOR_SAMPLE_RATE;
    status = Sensor_I2C_SendFunc(FXOS8700_DEVICE_ADDR_SA_00, FXOS8700_CTRL_REG1, 1, &data, 1);
    assert(status == kStatus_Success);
    /* Configure the fxos8700 as accel only mode.*/
    status = Sensor_I2C_ReceiveFunc(FXOS8700_DEVICE_ADDR_SA_00, FXOS8700_M_CTRL_REG2, 1, &data, 1);
    assert(status == kStatus_Success);
    data   = (data & ~FXOS8700_M_CTRL_REG2_M_AUTOINC_MASK) | FXOS8700_M_CTRL_REG2_M_AUTOINC_ACCEL_ONLY_MODE;
    status = Sensor_I2C_SendFunc(FXOS8700_DEVICE_ADDR_SA_00, FXOS8700_M_CTRL_REG2, 1, &data, 1);
    assert(status == kStatus_Success);
    /* Put the device into active mode and ready for reading data.*/
    status = Sensor_I2C_ReceiveFunc(FXOS8700_DEVICE_ADDR_SA_00, FXOS8700_CTRL_REG1, 1, &data, 1);
    assert(status == kStatus_Success);
    data   = (data & ~FXOS8700_CTRL_REG1_ACTIVE_MASK) | FXOS8700_CTRL_REG1_ACTIVE_ACTIVE_MODE;
    status = Sensor_I2C_SendFunc(FXOS8700_DEVICE_ADDR_SA_00, FXOS8700_CTRL_REG1, 1, &data, 1);
    assert(status == kStatus_Success);

    /* captured value 4096 from FXOS8700 stands for 1G in 2G scale. */
    KeynetikInitialize(4096, APP_PEDOMETER_SAMPLE_RATE, &pedoConfig);

    xTimerChangePeriod(pedometer.timer, APP_MS2TICK(APP_PEDOMETER_SAMPLE_WINDOW), portMAX_DELAY);
    xTimerStart(pedometer.timer, portMAX_DELAY);

    return status == kStatus_Success ? SRTM_Status_Success : SRTM_Status_Error;
}

static srtm_status_t APP_SRTM_Sensor_DeinitPedometer(void)
{
    uint8_t data;
    status_t status;

    xTimerStop(pedometer.timer, portMAX_DELAY);
    KeynetikTerminate();

    /* Put the device into standby mode.*/
    status = Sensor_I2C_ReceiveFunc(FXOS8700_DEVICE_ADDR_SA_00, FXOS8700_CTRL_REG1, 1, &data, 1);
    assert(status == kStatus_Success);
    if (status == kStatus_Success)
    {
        data   = (data & ~FXOS8700_CTRL_REG1_ACTIVE_MASK) | FXOS8700_CTRL_REG1_ACTIVE_STANDBY_MODE;
        status = Sensor_I2C_SendFunc(FXOS8700_DEVICE_ADDR_SA_00, FXOS8700_CTRL_REG1, 1, &data, 1);
        assert(status == kStatus_Success);
    }

    return status == kStatus_Success ? SRTM_Status_Success : SRTM_Status_Error;
}

static srtm_status_t APP_SRTM_Sensor_EnableStateDetector(srtm_sensor_adapter_t adapter,
                                                         srtm_sensor_type_t type,
                                                         uint8_t index,
                                                         bool enable)
{
    srtm_status_t status = SRTM_Status_Success;

    if (type != SRTM_SensorTypePedometer)
    {
        /* Only support pedometer now. */
        return SRTM_Status_InvalidParameter;
    }

    if (enable)
    {
        if (!pedometer.stateEnabled && !pedometer.dataEnabled)
        {
            /* Initialize Pedometer. */
            status = APP_SRTM_Sensor_InitPedometer();
        }
        if (status == SRTM_Status_Success)
        {
            pedometer.stateEnabled = true;
        }
    }
    else if (pedometer.stateEnabled)
    {
        pedometer.stateEnabled = false;
        if (!pedometer.dataEnabled)
        {
            status = APP_SRTM_Sensor_DeinitPedometer();
        }
    }

    return status;
}

static srtm_status_t APP_SRTM_Sensor_EnableDataReport(srtm_sensor_adapter_t adapter,
                                                      srtm_sensor_type_t type,
                                                      uint8_t index,
                                                      bool enable)
{
    srtm_status_t status = SRTM_Status_Success;

    if (type != SRTM_SensorTypePedometer)
    {
        /* Only support pedometer now. */
        return SRTM_Status_InvalidParameter;
    }

    if (enable && !pedometer.dataEnabled)
    {
        if (!pedometer.stateEnabled)
        {
            /* Initialize Pedometer. */
            status = APP_SRTM_Sensor_InitPedometer();
        }
        if (status == SRTM_Status_Success)
        {
            pedometer.dataEnabled = true;
            pedometer.expired     = 0;
            pedometer.lastCount   = keynetikStepCount;
        }
    }
    else if (!enable && pedometer.dataEnabled)
    {
        pedometer.dataEnabled = false;
        if (!pedometer.stateEnabled)
        {
            status = APP_SRTM_Sensor_DeinitPedometer();
        }
    }

    return status;
}

static srtm_status_t APP_SRTM_Sensor_SetPollDelay(srtm_sensor_adapter_t adapter,
                                                  srtm_sensor_type_t type,
                                                  uint8_t index,
                                                  uint32_t millisec)
{
    if (type != SRTM_SensorTypePedometer)
    {
        /* Only support pedometer now. */
        return SRTM_Status_InvalidParameter;
    }

    if (millisec > APP_PEDOMETER_POLL_DELAY_MAX || millisec < APP_PEDOMETER_POLL_DELAY_MIN)
    {
        return SRTM_Status_InvalidParameter;
    }

    pedometer.pollDelay = millisec;

    return SRTM_Status_Success;
}

void APP_UpdateSimDgo(uint32_t gpIdx, uint32_t mask, uint32_t value)
{
    uint32_t mask0 = SIM_SIM_DGO_CTRL0_WR_ACK_DGO_GP6_MASK | SIM_SIM_DGO_CTRL0_WR_ACK_DGO_GP5_MASK |
                     SIM_SIM_DGO_CTRL0_WR_ACK_DGO_GP4_MASK | SIM_SIM_DGO_CTRL0_WR_ACK_DGO_GP3_MASK |
                     SIM_SIM_DGO_CTRL0_WR_ACK_DGO_GP2_MASK | SIM_SIM_DGO_CTRL0_WR_ACK_DGO_GP1_MASK;
    uint32_t mask1 = SIM_SIM_DGO_CTRL1_WR_ACK_DGO_GP11_MASK | SIM_SIM_DGO_CTRL1_WR_ACK_DGO_GP10_MASK |
                     SIM_SIM_DGO_CTRL1_WR_ACK_DGO_GP9_MASK | SIM_SIM_DGO_CTRL1_WR_ACK_DGO_GP8_MASK |
                     SIM_SIM_DGO_CTRL1_WR_ACK_DGO_GP7_MASK;
    volatile uint32_t *reg;
    uint32_t shift;

    assert(gpIdx >= 1U && gpIdx <= 11U);

    reg = &SIM->SIM_DGO_GP1 + gpIdx - 1;

    *reg = (*reg & ~mask) | value;
    if (gpIdx <= 6)
    {
        shift              = gpIdx - 1;
        SIM->SIM_DGO_CTRL0 = (SIM->SIM_DGO_CTRL0 & ~mask0) | (1U << shift);
        /* Wait DGO GP1 updated */
        while ((SIM->SIM_DGO_CTRL0 & (1U << (shift + 13))) == 0)
        {
        }
        /* Clear DGO GP1 ACK and UPDATE bits */
        SIM->SIM_DGO_CTRL0 = (SIM->SIM_DGO_CTRL0 & ~((1U << shift) | mask0)) | (1U << (shift + 13));
    }
    else
    {
        shift              = gpIdx - 7;
        SIM->SIM_DGO_CTRL1 = (SIM->SIM_DGO_CTRL1 & ~mask1) | (1U << shift);
        /* Wait DGO GP1 updated */
        while ((SIM->SIM_DGO_CTRL1 & (1U << (shift + 13))) == 0)
        {
        }
        /* Clear DGO GP1 ACK and UPDATE bits */
        SIM->SIM_DGO_CTRL1 = (SIM->SIM_DGO_CTRL1 & ~((1U << shift) | mask1)) | (1U << (shift + 13));
    }
}

static void APP_ToggleSW3(srtm_dispatcher_t dispatcher, void *param1, void *param2)
{
    PF1550_EnableRegulator(
        &pf1550Handle, kPF1550_ModuleSwitch3, kPF1550_OperatingStatusRun,
        !PF1550_IsRegulatorEnabled(&pf1550Handle, kPF1550_ModuleSwitch3, kPF1550_OperatingStatusRun));
}

void APP_SRTM_ToggleSW3(void)
{
    srtm_procedure_t proc;

    proc = SRTM_Procedure_Create(APP_ToggleSW3, NULL, NULL);
    assert(proc);
    SRTM_Dispatcher_CallProc(disp, proc, SRTM_WAIT_FOR_EVER);
    SRTM_Procedure_Destroy(proc);
}

static void APP_PowerOffCA7(bool suspendMode)
{
    if (!suspendMode)
    {
        /* Disable application domain power (PTF power supply need to be on) */
        PF1550_EnableRegulator(&pf1550Handle, kPF1550_ModuleSwitch1, kPF1550_OperatingStatusRun, false);
        PF1550_EnableRegulator(&pf1550Handle, kPF1550_ModuleVrefDdr, kPF1550_OperatingStatusRun, false);
        PF1550_EnableRegulator(&pf1550Handle, kPF1550_ModuleSwitch2, kPF1550_OperatingStatusRun, false);
    }
    else if ((SIM->GPR1 & SIM_GPR1_USB_PHY_WAKEUP_ISO_DISABLE_MASK) == 0) /* USB wakeup is not needed */
    {
        PF1550_EnableRegulator(&pf1550Handle, kPF1550_ModuleSwitch1, kPF1550_OperatingStatusRun, false);
    }
    PF1550_EnableRegulator(&pf1550Handle, kPF1550_ModuleLdo2, kPF1550_OperatingStatusRun, false);

    peercorePower = suspendMode ? APP_SRTM_PowerOffVlls : APP_SRTM_PowerOff;
}

static void APP_PowerOnCA7(bool suspendMode)
{
    PF1550_EnableRegulator(&pf1550Handle, kPF1550_ModuleLdo2, kPF1550_OperatingStatusRun, true);
    if (!suspendMode)
    {
        /* Enable application domain power */
        PF1550_EnableRegulator(&pf1550Handle, kPF1550_ModuleSwitch2, kPF1550_OperatingStatusRun, true);
        PF1550_EnableRegulator(&pf1550Handle, kPF1550_ModuleVrefDdr, kPF1550_OperatingStatusRun, true);
        if (!PF1550_IsRegulatorEnabled(&pf1550Handle, kPF1550_ModuleSwitch3, kPF1550_OperatingStatusRun))
        {
            PF1550_EnableRegulator(&pf1550Handle, kPF1550_ModuleSwitch3, kPF1550_OperatingStatusRun, true);
        }
    }
    PF1550_EnableRegulator(&pf1550Handle, kPF1550_ModuleSwitch1, kPF1550_OperatingStatusRun, true);

    vTaskDelay(APP_MS2TICK(100U));

    peercorePower = APP_SRTM_PowerOn;
}

static void APP_ResetSRTM(app_srtm_state_t state)
{
    srtmState = state;
    /* Wake up monitor to reinitialize the SRTM communication with CA7 */
    xSemaphoreGive(monSig);
}

static void APP_SRTM_ControlCA7(srtm_dispatcher_t dispatcher, void *param1, void *param2)
{
    app_srtm_state_t state = (app_srtm_state_t)(uint32_t)param1;

    switch (state)
    {
        case APP_SRTM_StateRun:
            /* Fresh power up: Need SRTM monitor to prepare communication */
            srtmState = APP_SRTM_StateRun;
            xSemaphoreGive(monSig);
            break;
        case APP_SRTM_StateReboot:
            /* Only when CA7 is active, we can reboot it. */
            if (!core || peercorePower != APP_SRTM_PowerOn)
            {
                PRINTF("CA7 is not active, cannot reboot!\r\n");
            }
            else
            {
                /* Now prepare reboot */
                APP_ResetSRTM(APP_SRTM_StateReboot);
            }
            break;
        case APP_SRTM_StateShutdown:
            /* Only when CA7 goes into VLLS mode, we can shutdown it. */
            if (core && peercorePower == APP_SRTM_PowerOffVlls)
            {
                /* USB wakeup no longer needed. */
                APP_SRTM_ConfigUSBWakeup(false);
                /* Now prepare shutdown */
                APP_ResetSRTM(APP_SRTM_StateShutdown);
            }
            else
            {
                PRINTF("CA7 isn't in VLLS status, cannot shutdown!\r\n");
            }
            break;
        default:
            break;
    }
}

static void APP_SRTM_DoWakeup(void *param)
{
    if (peercorePower != APP_SRTM_PowerOn)
    {
        /* If CA7 is woken up by events other than USB IRQ, we need to release the USB IRQ to CA7 handler. */
        if (param == NULL)
        {
            APP_SRTM_ConfigUSBWakeup(false);
        }
        /* Power on regulators which were closed on suspend */
        PRINTF("CA7 in VLLS status, power on it and wakeup!\r\n");
        APP_PowerOnCA7(true);
        /* Wake up CA7 */
        MU_TriggerInterrupts(MUA, kMU_NmiInterruptTrigger);
        /* When system runs into VLLS, CA7 RST will be held. */
        MU_BootOtherCore(MUA, kMU_CoreBootFromAddr0);
    }
    else
    {
        PRINTF("CA7 not in VLLS status, wakeup directly!\r\n");
        /* Wake up CA7 */
        MU_TriggerInterrupts(MUA, kMU_NmiInterruptTrigger);
    }
}

static void APP_SRTM_DoWakeupCA7(srtm_dispatcher_t dispatcher, void *param1, void *param2)
{
    if (core && SRTM_PeerCore_GetState(core) == SRTM_PeerCore_State_Deactivated)
    {
        APP_SRTM_DoWakeup(param1);
    }
}

static void APP_SRTM_PollSuspend(srtm_dispatcher_t dispatcher, void *param1, void *param2)
{
    srtm_peercore_state_t state;
    mu_power_mode_t mode;

    state = SRTM_PeerCore_GetState(core);
    mode  = MU_GetOtherCorePowerMode(MUA);

    if (mode == kMU_PowerModeDsm)
    {
        /* Need to disable CA7 Reset IRQ. */
        APP_DisableRebootMonitor(true);
        /* Now the peer core is suspended */
        if (state == SRTM_PeerCore_State_Deactivating)
        {
            PRINTF("CA7 in VLLS status, power off unused regulator!\r\n");
            APP_PowerOffCA7(true);
            SRTM_PeerCore_SetState(core, SRTM_PeerCore_State_Deactivated);
            /* Now configure USB wakeup source. */
            APP_SRTM_ConfigUSBWakeup(true);
        }
        else if (state == SRTM_PeerCore_State_Activating)
        {
            PRINTF("CA7 in VLLS status, wake up immediately!\r\n");
            /* When CA7 need to be waken up before power gated, just trigger CA7 */
            MU_TriggerInterrupts(MUA, kMU_NmiInterruptTrigger);
        }
        else if (state == SRTM_PeerCore_State_Inactive)
        {
            PRINTF("CA7 in VLLS status, shutdown all app domain power now!\r\n");
            /* Shutdown requested, now safe to power off PMIC regulators */
            APP_ResetSRTM(APP_SRTM_StateShutdown);
        }
    }
    else if (mode == kMU_PowerModeStop) /* The suspend is just a Linux standby mode */
    {
        if (state == SRTM_PeerCore_State_Deactivating)
        {
            PRINTF("CA7 in VLPS status, do not power off regulator!\r\n");
            SRTM_PeerCore_SetState(core, SRTM_PeerCore_State_Deactivated);
        }
        else if (state == SRTM_PeerCore_State_Activating)
        {
            PRINTF("CA7 in VLPS status, wake up immediately!\r\n");
            /* When CA7 need to be waken up before power gated, just trigger CA7 */
            MU_TriggerInterrupts(MUA, kMU_NmiInterruptTrigger);
        }
    }
    else if (state == SRTM_PeerCore_State_Deactivating || state == SRTM_PeerCore_State_Activating ||
             state == SRTM_PeerCore_State_Inactive)
    {
        PRINTF("CA7 suspend timeout and it doesn't go into VLLS/VLPS, wait again!\r\n");
        /* Peer core has not suspended, wait once more */
        xTimerStart(suspendTimer, portMAX_DELAY);
    }
}

static void APP_SuspendTimerCallback(TimerHandle_t xTimer)
{
    srtm_procedure_t proc = SRTM_Procedure_Create(APP_SRTM_PollSuspend, NULL, NULL);

    /* Need to poll suspend status in SRTM task context */
    if (proc)
    {
        SRTM_Dispatcher_PostProc(disp, proc);
    }
}

static void APP_HeartBeatTimerCallback(TimerHandle_t xTimer)
{
    PRINTF("Heart Beat timeout\r\n");
    APP_SRTM_RebootCA7();
}

static void APP_SRTM_PollLinkup(srtm_dispatcher_t dispatcher, void *param1, void *param2)
{
    if (srtmState == APP_SRTM_StateRun)
    {
        if (rpmsg_lite_is_link_up(rpmsgHandle))
        {
            srtmState = APP_SRTM_StateLinkedUp;
            xSemaphoreGive(monSig);
        }
        else
        {
            /* Start timer to poll linkup status. */
            xTimerStart(linkupTimer, portMAX_DELAY);
        }
    }
}

static void APP_LinkupTimerCallback(TimerHandle_t xTimer)
{
    srtm_procedure_t proc = SRTM_Procedure_Create(APP_SRTM_PollLinkup, NULL, NULL);

    if (proc)
    {
        SRTM_Dispatcher_PostProc(disp, proc);
    }
}

static void APP_VolPlusTimerCallback(TimerHandle_t xTimer)
{
    uint8_t gpioIdx = APP_GPIO_IDX(APP_PIN_VOL_PLUS);
    uint8_t pinIdx  = APP_PIN_IDX(APP_PIN_VOL_PLUS);
    srtm_keypad_value_t value;

    if (GPIO_PinRead(gpios[gpioIdx], pinIdx) == suspendContext.io.data[APP_INPUT_VOL_PLUS].value)
    {
        value = suspendContext.io.data[APP_INPUT_VOL_PLUS].value ? SRTM_KeypadValuePressed : SRTM_KeypadValueReleased;
        /* No glitch, a valid user operation */
        if (suspendContext.io.data[APP_INPUT_VOL_PLUS].wakeup || MU_GetOtherCorePowerMode(MUA) != kMU_PowerModeDsm)
        {
            /* Only when CA7 is running or wakeup flag is set, we'll notify the event to CA7. */
            SRTM_KeypadService_NotifyKeypadEvent(keypadService, APP_KEYPAD_INDEX_VOL_PLUS, value);
        }
    }

    /* Restore pin detection interrupt */
    APP_IO_ConfInput(APP_INPUT_VOL_PLUS, suspendContext.io.data[APP_INPUT_VOL_PLUS].event, false);
}

static void APP_VolMinusTimerCallback(TimerHandle_t xTimer)
{
    uint8_t gpioIdx = APP_GPIO_IDX(APP_PIN_VOL_MINUS);
    uint8_t pinIdx  = APP_PIN_IDX(APP_PIN_VOL_MINUS);
    srtm_keypad_value_t value;

    if (GPIO_PinRead(gpios[gpioIdx], pinIdx) == suspendContext.io.data[APP_INPUT_VOL_MINUS].value)
    {
        value = suspendContext.io.data[APP_INPUT_VOL_MINUS].value ? SRTM_KeypadValuePressed : SRTM_KeypadValueReleased;
        /* No glitch, a valid user operation */
        if (suspendContext.io.data[APP_INPUT_VOL_MINUS].wakeup || MU_GetOtherCorePowerMode(MUA) != kMU_PowerModeDsm)
        {
            /* Only when CA7 is running or wakeup flag is set, we'll notify the event to CA7. */
            SRTM_KeypadService_NotifyKeypadEvent(keypadService, APP_KEYPAD_INDEX_VOL_MINUS, value);
        }
    }

    /* Restore pin detection interrupt */
    APP_IO_ConfInput(APP_INPUT_VOL_MINUS, suspendContext.io.data[APP_INPUT_VOL_MINUS].event, false);
}

static void APP_OnOffTimerCallback(TimerHandle_t xTimer)
{
    srtm_io_event_t ioRelease = APP_Keypad_GetIoEvent(APP_KEYPAD_INDEX_ONOFF, SRTM_KeypadEventRelease);
    srtm_io_event_t ioEither  = APP_Keypad_GetIoEvent(APP_KEYPAD_INDEX_ONOFF, SRTM_KeypadEventPressOrRelease);

    if ((SNVS->HPSR & SNVS_HPSR_BTN_MASK) == SNVS_HPSR_BTN_MASK)
    {
        if (peercorePower == APP_SRTM_PowerOff)
        {
            /* If CA7 is powered off, ONOFF key is used to power on it. */
            APP_SRTM_BootCA7();
        }
        else if (keypadService &&
                 (suspendContext.io.data[APP_INPUT_ONOFF].wakeup ||
                  MU_GetOtherCorePowerMode(MUA) != kMU_PowerModeDsm) &&
                 (suspendContext.io.data[APP_INPUT_ONOFF].event == ioRelease ||
                  suspendContext.io.data[APP_INPUT_ONOFF].event == ioEither))
        {
            /* Only when CA7 is running or wakeup flag is set, and release event is monitored,
               we'll notify the event to CA7. */
            SRTM_KeypadService_NotifyKeypadEvent(keypadService, APP_KEYPAD_INDEX_ONOFF, SRTM_KeypadValueReleased);
        }
        xTimerStop(suspendContext.io.data[APP_INPUT_ONOFF].timer, portMAX_DELAY);
    }
}

static srtm_status_t APP_SRTM_LfclEventHandler(
    srtm_service_t service, srtm_peercore_t core, srtm_lfcl_event_t event, void *eventParam, void *userParam)
{
    switch (event)
    {
        case SRTM_Lfcl_Event_Running:
            /* Peer core is running now, stop suspend polling timer */
            xTimerStop(suspendTimer, portMAX_DELAY);
            APP_EnableRebootMonitor();
            break;

        case SRTM_Lfcl_Event_SuspendReq:
            /* Peer core is going to suspend, start a timer to wait peer core to suspend */
            xTimerStart(suspendTimer, portMAX_DELAY);
            /* Disable CA7 heart beat monitor */
            APP_DisableRebootMonitor(false);
            break;

        case SRTM_Lfcl_Event_WakeupReq:
            /* If already deactivated, power on CA7, else CA7 will not power off,
               and wakeup will defer until CA7 enter VLLS */
            APP_SRTM_DoWakeupCA7(NULL, NULL, NULL);
            break;

        case SRTM_Lfcl_Event_RebootReq:
            /* Now prepare reboot */
            PRINTF("Linux reboot message\r\n");
            APP_ResetSRTM(APP_SRTM_StateReboot);
            break;

        case SRTM_Lfcl_Event_ShutdownReq:
            /* Peer core is going to shutdown, start a timer to wait peer core to suspend */
            xTimerStart(suspendTimer, portMAX_DELAY);
            break;

        case SRTM_Lfcl_Event_HeartBeatEnable:
            heartBeat = true;
            xTimerStart(heartBeatTimer, portMAX_DELAY);
            break;

        case SRTM_Lfcl_Event_HeartBeatDisable:
            heartBeat = false;
            xTimerStop(heartBeatTimer, portMAX_DELAY);
            break;

        case SRTM_Lfcl_Event_HeartBeat:
            xTimerReset(heartBeatTimer, portMAX_DELAY);
            break;

        default:
            break;
    }

    return SRTM_Status_Success;
}

static void APP_SRTM_NotifyPeerCoreReady(struct rpmsg_lite_instance *rpmsgHandle, bool ready)
{
    if (rpmsgMonitor)
    {
        rpmsgMonitor(rpmsgHandle, ready, rpmsgMonitorParam);
    }
}

static void APP_SRTM_Linkup(void)
{
    srtm_channel_t chan;
    srtm_rpmsg_endpoint_config_t rpmsgConfig;

    APP_SRTM_NotifyPeerCoreReady(rpmsgHandle, true);

    /* Create SRTM peer core */
    core = SRTM_PeerCore_Create(1U); /* Assign CA7 core ID to 1U */

    /* Common RPMsg channel config */
    rpmsgConfig.localAddr = RL_ADDR_ANY;
    rpmsgConfig.peerAddr  = RL_ADDR_ANY;

    /* Create and add SRTM PMIC channel to peer core */
    rpmsgConfig.rpmsgHandle = rpmsgHandle;
    rpmsgConfig.epName      = APP_SRTM_PMIC_CHANNEL_NAME;
    chan                    = SRTM_RPMsgEndpoint_Create(&rpmsgConfig);
    SRTM_PeerCore_AddChannel(core, chan);

    /* Create and add SRTM Audio channel to peer core */
    rpmsgConfig.rpmsgHandle = rpmsgHandle;
    rpmsgConfig.epName      = APP_SRTM_AUDIO_CHANNEL_NAME;
    chan                    = SRTM_RPMsgEndpoint_Create(&rpmsgConfig);
    SRTM_PeerCore_AddChannel(core, chan);
    assert((audioService != NULL) && (saiAdapter != NULL));
    SRTM_AudioService_BindChannel(audioService, saiAdapter, chan);

    /* Create and add SRTM Life Cycle channel to peer core */
    rpmsgConfig.rpmsgHandle = rpmsgHandle;
    rpmsgConfig.epName      = APP_SRTM_LFCL_CHANNEL_NAME;
    chan                    = SRTM_RPMsgEndpoint_Create(&rpmsgConfig);
    SRTM_PeerCore_AddChannel(core, chan);

    /* Create and add SRTM Keypad channel to peer core */
    rpmsgConfig.rpmsgHandle = rpmsgHandle;
    rpmsgConfig.epName      = APP_SRTM_KEYPAD_CHANNEL_NAME;
    chan                    = SRTM_RPMsgEndpoint_Create(&rpmsgConfig);
    SRTM_PeerCore_AddChannel(core, chan);

    /* Create and add SRTM IO channel to peer core */
    rpmsgConfig.rpmsgHandle = rpmsgHandle;
    rpmsgConfig.epName      = APP_SRTM_IO_CHANNEL_NAME;
    chan                    = SRTM_RPMsgEndpoint_Create(&rpmsgConfig);
    SRTM_PeerCore_AddChannel(core, chan);

    /* Create and add SRTM RTC channel to peer core */
    rpmsgConfig.rpmsgHandle = rpmsgHandle;
    rpmsgConfig.epName      = APP_SRTM_RTC_CHANNEL_NAME;
    chan                    = SRTM_RPMsgEndpoint_Create(&rpmsgConfig);
    SRTM_PeerCore_AddChannel(core, chan);

    /* Create and add SRTM Sensor channel to peer core */
    rpmsgConfig.rpmsgHandle = rpmsgHandle;
    rpmsgConfig.epName      = APP_SRTM_SENSOR_CHANNEL_NAME;
    chan                    = SRTM_RPMsgEndpoint_Create(&rpmsgConfig);
    SRTM_PeerCore_AddChannel(core, chan);

    SRTM_Dispatcher_AddPeerCore(disp, core);
}

static void APP_SRTM_InitPeerCore(void)
{
    heartBeat = false;

    copyResourceTable();

    rpmsgHandle = rpmsg_lite_remote_init((void *)RPMSG_LITE_SRTM_SHMEM_BASE, RPMSG_LITE_SRTM_LINK_ID, RL_NO_FLAGS);
    assert(rpmsgHandle);

    if (rpmsg_lite_is_link_up(rpmsgHandle))
    {
        APP_SRTM_Linkup();
    }
    else
    {
        /* Start timer to poll linkup status. */
        xTimerStart(linkupTimer, portMAX_DELAY);
    }
}

static void APP_SRTM_GpioReset(void)
{
    uint32_t i;

    /* First disable all GPIO interrupts configured by CA7 */
    for (i = 0; i < APP_OUTPUT_GPIO_START; i++)
    {
        if (suspendContext.io.data[i].timer)
        {
            xTimerStop(suspendContext.io.data[i].timer, portMAX_DELAY);
        }
        if (suspendContext.io.data[i].override)
        {
            /* The IO is configured by CM4 instead of CA7, don't reset HW configuration. */
            suspendContext.io.data[i].event  = SRTM_IoEventNone;
            suspendContext.io.data[i].wakeup = false;
        }
        else
        {
            APP_IO_ConfIEvent(NULL, NULL, suspendContext.io.data[i].ioId, SRTM_IoEventNone, false);
        }
    }

    /* Output pin value doesn't change. */

    /* Then reset IO service */
    SRTM_IoService_Reset(ioService, core);
    SRTM_KeypadService_Reset(keypadService, core);
}

static void APP_SRTM_ResetServices(void)
{
    /* When CA7 resets, we need to avoid async event to send to CA7. Audio and IO services have async events. */
    SRTM_AudioService_Reset(audioService, core);
    SRTM_RtcService_Reset(rtcService, core);
    APP_SRTM_GpioReset();
}

static void APP_SRTM_DeinitPeerCore(void)
{
    /* Stop linkupTimer if it's started. */
    xTimerStop(linkupTimer, portMAX_DELAY);

    if (core)
    {
        /* Notify application for the peer core disconnection. */
        APP_SRTM_NotifyPeerCoreReady(rpmsgHandle, false);
        /* Need to let services know peer core is now down. */
        APP_SRTM_ResetServices();

        SRTM_Dispatcher_RemovePeerCore(disp, core);
        SRTM_PeerCore_Destroy(core);
        core = NULL;
    }

    if (rpmsgHandle)
    {
        rpmsg_lite_deinit(rpmsgHandle);
        rpmsgHandle = NULL;
    }
}

static void APP_SRTM_InitI2C(lpi2c_rtos_handle_t *handle, LPI2C_Type *base, uint32_t baudrate, uint32_t clockrate)
{
    status_t status;
    lpi2c_master_config_t lpi2cConfig;

    /* Initialize LPI2C instance for PMIC and sensor */
    /*
     * lpi2cConfig.debugEnable = false;
     * lpi2cConfig.ignoreAck = false;
     * lpi2cConfig.pinConfig = kLPI2C_2PinOpenDrain;
     * lpi2cConfig.baudRate_Hz = 100000U;
     * lpi2cConfig.busIdleTimeout_ns = 0;
     * lpi2cConfig.pinLowTimeout_ns = 0;
     * lpi2cConfig.sdaGlitchFilterWidth_ns = 0;
     * lpi2cConfig.sclGlitchFilterWidth_ns = 0;
     */
    LPI2C_MasterGetDefaultConfig(&lpi2cConfig);
    lpi2cConfig.baudRate_Hz = baudrate;
    /* Initialize LPI2C RTOS driver. */
    status = LPI2C_RTOS_Init(handle, base, &lpi2cConfig, clockrate);
    assert(status == kStatus_Success);
    (void)status;
}

static void APP_SRTM_DeinitI2C(lpi2c_rtos_handle_t *handle)
{
    status_t status;

    status = LPI2C_RTOS_Deinit(handle);
    assert(status == kStatus_Success);
    (void)status;
}

static void APP_SRTM_InitPmicDevice(void)
{
    if (!lpi2c3Init)
    {
        APP_SRTM_InitI2C(&lpi2c3Handle, LPI2C3, APP_LPI2C3_BAUDRATE, CLOCK_GetIpFreq(kCLOCK_Lpi2c3));
        lpi2c3Init = true;
    }
    pmicI2cHandle = &lpi2c3Handle;
}

static void APP_SRTM_DeinitPmicDevice(void)
{
    if (lpi2c3Init)
    {
        APP_SRTM_DeinitI2C(&lpi2c3Handle);
        lpi2c3Init = false;
    }
    pmicI2cHandle = NULL;
}

static void APP_SRTM_InitPmicService(void)
{
    pf1550_config_t pf1550Config;
    srtm_pmic_adapter_t pmicAdapter;
    srtm_service_t service;

    APP_SRTM_InitPmicDevice();
    /*  Set LPI2C Master IRQ Priority. */
    NVIC_SetPriority(LPI2C3_IRQn, APP_LPI2C3_IRQ_PRIO);

    /* Initialize PMIC driver */
    PF1550_GetDefaultConfig(&pf1550Config);
    pf1550Config.I2C_SendFunc    = PMIC_I2C_SendFunc;
    pf1550Config.I2C_ReceiveFunc = PMIC_I2C_ReceiveFunc;
    PF1550_Init(&pf1550Handle, &pf1550Config);

    /* Let PF1550 LDOs and SWs enter low power mode in sleep and standby modes. */
    /* SW1 */
    PF1550_ModifyReg(&pf1550Handle, PF1550_SW1_CTRL, 0x08U, 0x08U);
    /* SW2 */
    PF1550_ModifyReg(&pf1550Handle, PF1550_SW2_CTRL, 0x08U, 0x08U);
    /* SW3 */
    PF1550_ModifyReg(&pf1550Handle, PF1550_SW3_CTRL, 0x08U, 0x08U);
    /* LDO1 */
    PF1550_ModifyReg(&pf1550Handle, PF1550_LDO1_CTRL, 0x08U, 0x08U);
    /* LDO2 */
    PF1550_ModifyReg(&pf1550Handle, PF1550_LDO2_CTRL, 0x08U, 0x08U);
    /* LDO3 is controlled by APP_PowerTestMode(command 'Z'). */

    /* Set DPM to 4.3V from default 4.5V to work around the PMIC reset issue when power
     * switchng between power adaptor and USB supply.
     */
    PF1550_ModifyReg(&pf1550Handle, PF1550_VBUS_LIN_DPM, 0x07U, 0x4U);

    /* Create and register PMIC service */
    pmicAdapter = SRTM_Pf1550Adapter_Create(&pf1550Handle);
    service     = SRTM_PmicService_Create(pmicAdapter);
    SRTM_Dispatcher_RegisterService(disp, service);
}

static void APP_SRTM_InitSensorDevice(void)
{
    if (!lpi2c3Init)
    {
        APP_SRTM_InitI2C(&lpi2c3Handle, LPI2C3, APP_LPI2C3_BAUDRATE, CLOCK_GetIpFreq(kCLOCK_Lpi2c3));
        lpi2c3Init = true;
    }
    sensorI2cHandle = &lpi2c3Handle;
}

static void APP_SRTM_DeinitSensorDevice(void)
{
    if (lpi2c3Init)
    {
        APP_SRTM_DeinitI2C(&lpi2c3Handle);
        lpi2c3Init = false;
    }
    sensorI2cHandle = NULL;
}

static void APP_SRTM_InitSensorService(void)
{
    srtm_service_t service;

    APP_SRTM_InitSensorDevice();

    /* Create and register Sensor service */
    service = SRTM_SensorService_Create(&sensorAdapter);
    SRTM_Dispatcher_RegisterService(disp, service);
}

static void APP_SRTM_InitCodecDevice(void)
{
    if (!lpi2c0Init)
    {
        APP_SRTM_InitI2C(&lpi2c0Handle, LPI2C0, APP_LPI2C0_BAUDRATE, CLOCK_GetIpFreq(kCLOCK_Lpi2c0));
        lpi2c0Init = true;
    }
}

static void APP_SRTM_DeinitCodecDevice(void)
{
    if (lpi2c0Init)
    {
        APP_SRTM_DeinitI2C(&lpi2c0Handle);
        lpi2c0Init = false;
    }
}

static void APP_SRTM_InitAudioDevice(void)
{
    edma_config_t dmaConfig;

    /* Initialize DMA0 for SAI */
    EDMA_GetDefaultConfig(&dmaConfig);
    EDMA_Init(DMA0, &dmaConfig);

    /* Initialize DMAMUX for SAI */
    DMAMUX_Init(DMA_CH_MUX0);
    DMAMUX_SetSource(DMA_CH_MUX0, APP_SAI_TX_DMA_CHANNEL, kDmaRequestMux0SAI0Tx);
    DMAMUX_SetSource(DMA_CH_MUX0, APP_SAI_RX_DMA_CHANNEL, kDmaRequestMux0SAI0Rx);
    DMAMUX_EnableChannel(DMA_CH_MUX0, APP_SAI_TX_DMA_CHANNEL);
    DMAMUX_EnableChannel(DMA_CH_MUX0, APP_SAI_RX_DMA_CHANNEL);

    APP_SRTM_InitCodecDevice();
}

static status_t APP_SRTM_ReadCodecRegMap(void *handle, uint32_t reg, uint32_t *val)
{
    return WM8960_ReadReg(reg, (uint16_t *)val);
}

static status_t APP_SRTM_WriteCodecRegMap(void *handle, uint32_t reg, uint32_t val)
{
    return WM8960_WriteReg((wm8960_handle_t *)((uint32_t) & (((codec_handle_t *)handle)->codecDevHandle)), reg, val);
}

static void APP_SRTM_InitAudioService(void)
{
    srtm_sai_edma_config_t saiTxConfig;
    srtm_sai_edma_config_t saiRxConfig;
    srtm_i2c_codec_config_t i2cCodecConfig;
    srtm_codec_adapter_t codecAdapter;

    memset(&saiTxConfig, 0, sizeof(saiTxConfig));
    memset(&saiRxConfig, 0, sizeof(saiRxConfig));

    APP_SRTM_InitAudioDevice();

    /*  Set SAI DMA IRQ Priority. */
    NVIC_SetPriority(APP_DMA_IRQN(APP_SAI_TX_DMA_CHANNEL), APP_SAI_TX_DMA_IRQ_PRIO);
    NVIC_SetPriority(APP_DMA_IRQN(APP_SAI_RX_DMA_CHANNEL), APP_SAI_RX_DMA_IRQ_PRIO);
    NVIC_SetPriority(I2S0_IRQn, APP_SAI_IRQ_PRIO);
    /* Create SAI EDMA adapter */
    SAI_GetClassicI2SConfig(&saiTxConfig.config, kSAI_WordWidth16bits, kSAI_Stereo, kSAI_Channel0Mask);
    saiTxConfig.config.syncMode = kSAI_ModeAsync; /* Tx in async mode */
    saiTxConfig.mclk            = CLOCK_GetIpFreq(kCLOCK_Sai0);
    saiTxConfig.stopOnSuspend   = true;           /* Audio data is in DRAM which is not accessable in A7 suspend. */
    saiTxConfig.threshold       = UINT32_MAX;     /* Every period transmitted triggers periodDone message to A7. */
    saiTxConfig.dmaChannel      = APP_SAI_TX_DMA_CHANNEL;

    SAI_GetClassicI2SConfig(&saiRxConfig.config, kSAI_WordWidth16bits, kSAI_Stereo, kSAI_Channel0Mask);
    saiRxConfig.config.syncMode = kSAI_ModeSync; /* Rx in sync mode */
    saiRxConfig.mclk            = saiTxConfig.mclk;
    saiRxConfig.stopOnSuspend   = true;          /* Audio data is in DRAM which is not accessable in A7 suspend. */
    saiRxConfig.threshold       = UINT32_MAX;    /* Every period received triggers periodDone message to A7. */
    saiRxConfig.dmaChannel      = APP_SAI_RX_DMA_CHANNEL;

    saiAdapter = SRTM_SaiEdmaAdapter_Create(I2S0, DMA0, &saiTxConfig, &saiRxConfig);
    assert(saiAdapter);

    /*  Set LPI2C Master IRQ Priority. */
    NVIC_SetPriority(LPI2C0_IRQn, APP_LPI2C0_IRQ_PRIO);

    wm8960Config.i2cConfig.codecI2CInstance    = 0;
    wm8960Config.i2cConfig.codecI2CSourceClock = CLOCK_GetIpFreq(kCLOCK_Lpi2c0);
    wm8960Config.route                         = kWM8960_RoutePlaybackandRecord;
    wm8960Config.leftInputSource               = kWM8960_InputDifferentialMicInput3;
    wm8960Config.rightInputSource              = kWM8960_InputClosed;
    wm8960Config.playSource                    = kWM8960_PlaySourceDAC;
    wm8960Config.slaveAddress                  = WM8960_I2C_ADDR;
    wm8960Config.bus                           = kWM8960_BusI2S;
    wm8960Config.format.mclk_HZ                = 6144000U;
    wm8960Config.format.sampleRate             = kWM8960_AudioSampleRate16KHz;
    wm8960Config.format.bitWidth               = kWM8960_AudioBitWidth16bit;
    wm8960Config.master_slave                  = false;
    boardCodecConfig.codecDevConfig            = &wm8960Config;
    /* Initialize WM8960 codec */
    CODEC_Init(&codecHandle, &boardCodecConfig);

    /* Create I2C Codec adaptor */
    i2cCodecConfig.mclk        = saiTxConfig.mclk;
    i2cCodecConfig.slaveAddr   = 0U;
    i2cCodecConfig.addrType    = kCODEC_RegAddr8Bit;
    i2cCodecConfig.regWidth    = kCODEC_RegWidth8Bit;
    i2cCodecConfig.writeRegMap = APP_SRTM_WriteCodecRegMap;
    i2cCodecConfig.readRegMap  = APP_SRTM_ReadCodecRegMap;
    i2cCodecConfig.i2cHandle   = NULL;
    codecAdapter               = SRTM_I2CCodecAdapter_Create(&codecHandle, &i2cCodecConfig);
    assert(codecAdapter);

    /* Create and register audio service */
    audioService = SRTM_AudioService_Create(saiAdapter, codecAdapter);
    SRTM_Dispatcher_RegisterService(disp, audioService);
}

static void APP_SRTM_InitLfclService(void)
{
    srtm_service_t service;

    /* Create and register Life Cycle service */
    service = SRTM_LfclService_Create();
    SRTM_LfclService_Subscribe(service, APP_SRTM_LfclEventHandler, NULL);
    SRTM_Dispatcher_RegisterService(disp, service);
}

static void APP_SRTM_InitIoKeyDevice(void)
{
    uint32_t i;

    gpio_pin_config_t gpioConfig = {
        kGPIO_DigitalOutput,
        0U,
    };

    /* Init output configuration */
    for (i = APP_OUTPUT_GPIO_START; i < APP_IO_NUM; i++)
    {
        gpioConfig.outputLogic = suspendContext.io.data[i].value;
        GPIO_PinInit(gpios[APP_GPIO_IDX(suspendContext.io.data[i].ioId)], APP_PIN_IDX(suspendContext.io.data[i].ioId),
                     &gpioConfig);
    }

    /* Init input configuration */
    gpioConfig.pinDirection = kGPIO_DigitalInput;
    for (i = APP_INPUT_GPIO_START; i < APP_OUTPUT_GPIO_START; i++)
    {
        GPIO_PinInit(gpios[APP_GPIO_IDX(suspendContext.io.data[i].ioId)], APP_PIN_IDX(suspendContext.io.data[i].ioId),
                     &gpioConfig);
        if (!suspendContext.io.data[i].override)
        {
            APP_IO_ConfInput(i, suspendContext.io.data[i].event, suspendContext.io.data[i].wakeup);
        }
    }
}

static void APP_SRTM_InitIoKeyService(void)
{
    /* Init IO structure used in the application. */
    /* Keypad */
    suspendContext.io.data[APP_INPUT_ONOFF].index     = APP_KEYPAD_INDEX_ONOFF;
    suspendContext.io.data[APP_INPUT_VOL_PLUS].index  = APP_KEYPAD_INDEX_VOL_PLUS;
    suspendContext.io.data[APP_INPUT_VOL_MINUS].index = APP_KEYPAD_INDEX_VOL_MINUS;

    /* GPIO ID */
    suspendContext.io.data[APP_INPUT_ONOFF].ioId        = APP_PIN_ONOFF; /* Special SNVS IO */
    suspendContext.io.data[APP_INPUT_VOL_PLUS].ioId     = APP_PIN_VOL_PLUS;
    suspendContext.io.data[APP_INPUT_VOL_MINUS].ioId    = APP_PIN_VOL_MINUS;
    suspendContext.io.data[APP_INPUT_BT_HOST_WAKE].ioId = APP_PIN_BT_HOST_WAKE;
    suspendContext.io.data[APP_INPUT_WL_HOST_WAKE].ioId = APP_PIN_WL_HOST_WAKE;
    suspendContext.io.data[APP_OUTPUT_WL_REG_ON].ioId   = APP_PIN_WL_REG_ON;
    suspendContext.io.data[APP_OUTPUT_BT_REG_ON].ioId   = APP_PIN_BT_REG_ON;

    APP_SRTM_InitIoKeyDevice();

    /* Enable interrupt for GPIO. */
    NVIC_SetPriority(PCTLA_IRQn, APP_GPIO_IRQ_PRIO);
    NVIC_SetPriority(PCTLB_IRQn, APP_GPIO_IRQ_PRIO);
    EnableIRQ(PCTLA_IRQn);
    EnableIRQ(PCTLB_IRQn);

    ioService = SRTM_IoService_Create();
    SRTM_IoService_RegisterPin(ioService, APP_PIN_BT_REG_ON, APP_IO_SetOutput, APP_IO_GetInput, NULL, NULL);
    SRTM_IoService_RegisterPin(ioService, APP_PIN_WL_REG_ON, APP_IO_SetOutput, APP_IO_GetInput, NULL, NULL);
    SRTM_IoService_RegisterPin(ioService, APP_PIN_BT_HOST_WAKE, NULL, APP_IO_GetInput, APP_IO_ConfIEvent, NULL);
    SRTM_IoService_RegisterPin(ioService, APP_PIN_WL_HOST_WAKE, NULL, APP_IO_GetInput, APP_IO_ConfIEvent, NULL);
    SRTM_Dispatcher_RegisterService(disp, ioService);

    keypadService = SRTM_KeypadService_Create();
    SRTM_KeypadService_RegisterKey(keypadService, APP_KEYPAD_INDEX_ONOFF, APP_IO_ConfKEvent, NULL);
    SRTM_KeypadService_RegisterKey(keypadService, APP_KEYPAD_INDEX_VOL_PLUS, APP_IO_ConfKEvent, NULL);
    SRTM_KeypadService_RegisterKey(keypadService, APP_KEYPAD_INDEX_VOL_MINUS, APP_IO_ConfKEvent, NULL);
    SRTM_Dispatcher_RegisterService(disp, keypadService);
}

static void APP_SRTM_InitRtcDevice(bool start)
{
    snvs_lp_srtc_config_t snvsSrtcConfig;

    SNVS_HP_Init(SNVS);
    SNVS_HP_ResetLP(SNVS);

    SNVS_LP_SRTC_GetDefaultConfig(&snvsSrtcConfig);
    SNVS_LP_SRTC_Init(SNVS, &snvsSrtcConfig);

    if (start)
    {
        SNVS_LP_SRTC_StartTimer(SNVS);
    }
}

static void APP_SRTM_InitRtcService(void)
{
    APP_SRTM_InitRtcDevice(false);

    rtcAdapter = SRTM_SnvsLpRtcAdapter_Create(SNVS);
    assert(rtcAdapter);

    rtcService = SRTM_RtcService_Create(rtcAdapter);
    SRTM_Dispatcher_RegisterService(disp, rtcService);
}

static void APP_SRTM_InitServices(void)
{
    APP_SRTM_InitPmicService();
    APP_SRTM_InitSensorService();
    APP_SRTM_InitRtcService();
    APP_SRTM_InitAudioService();
    APP_SRTM_InitLfclService();
    APP_SRTM_InitIoKeyService();

    /* SNVS is shared by RTC and Key service, enable IRQ last. */
    NVIC_SetPriority(SNVS_IRQn, APP_SNVS_IRQ_PRIO);
    EnableIRQ(SNVS_IRQn);
}

static void APP_ResetCA7Regulators(void)
{
    /* Set CA7 power to its default value. */
    PF1550_SetRegulatorOutputVoltage(&pf1550Handle, kPF1550_ModuleSwitch1, kPF1550_OperatingStatusRun, 1100000U);
    PF1550_SetRegulatorOutputVoltage(&pf1550Handle, kPF1550_ModuleSwitch2, kPF1550_OperatingStatusRun, 1200000U);
    PF1550_SetRegulatorOutputVoltage(&pf1550Handle, kPF1550_ModuleSwitch3, kPF1550_OperatingStatusRun, 1800000U);
    PF1550_SetRegulatorOutputVoltage(&pf1550Handle, kPF1550_ModuleLdo2, kPF1550_OperatingStatusRun, 3300000U);

    /* Delay 500ms to wait CA7 Domain Power stable. */
    vTaskDelay(APP_MS2TICK(500U));
}

static void APP_SRTM_GetPf1550Reg(srtm_dispatcher_t dispatcher, void *param1, void *param2)
{
    uint32_t reg     = (uint32_t)param1;
    uint32_t *pValue = (uint32_t *)param2;

    /* Clear high bytes. */
    *pValue = 0;
    PF1550_ReadReg(&pf1550Handle, reg, (uint8_t *)pValue);
}

static void APP_SRTM_SetPf1550Reg(srtm_dispatcher_t dispatcher, void *param1, void *param2)
{
    uint32_t reg   = (uint32_t)param1;
    uint32_t value = (uint32_t)param2;

    PF1550_WriteReg(&pf1550Handle, reg, value);
}

static void APP_SRTM_DoSetWakeupPin(srtm_dispatcher_t dispatcher, void *param1, void *param2)
{
    uint16_t ioId    = (uint32_t)param1;
    uint16_t event   = (uint32_t)param2;
    uint8_t inputIdx = APP_IO_GetIoIndex(ioId);
    bool wakeup      = (bool)(event >> 8);
    uint8_t pinMode  = (uint8_t)event;

    assert(inputIdx < APP_OUTPUT_GPIO_START);
    assert(pinMode < ARRAY_SIZE(llwuPinModeEvents));

    if (llwuPinModeEvents[pinMode] != SRTM_IoEventNone)
    {
        /* NOTE: As we reused same VOL+ pin for CM4 wakeup, the PORT configuration from CA7 will be overwritten.
         * This is just for demonstration of the wakeup pin feature. Real application should avoid assign same pin
         * to both CA7 and CM4.
         */
        APP_IO_ConfInput(inputIdx, llwuPinModeEvents[pinMode], wakeup);
        suspendContext.io.data[inputIdx].override = true;
    }
    else
    {
        /* Restore CA7 settings */
        APP_IO_ConfInput(inputIdx, suspendContext.io.data[inputIdx].event, suspendContext.io.data[inputIdx].wakeup);
        suspendContext.io.data[inputIdx].override = false;
    }
}

static void APP_SRTM_DoSetWakeupModule(srtm_dispatcher_t dispatcher, void *param1, void *param2)
{
    uint32_t module = (uint32_t)param1;
    bool enable     = (bool)param2;

    LLWU_EnableInternalModuleInterruptWakup(LLWU, module, enable);
}

static void SRTM_MonitorTask(void *pvParameters)
{
    app_srtm_state_t state = APP_SRTM_StateShutdown;

    /* Initialize services and add to dispatcher */
    APP_SRTM_InitServices();

    /* Start SRTM dispatcher */
    SRTM_Dispatcher_Start(disp);

    /* Monitor peer core state change */
    while (true)
    {
        xSemaphoreTake(monSig, portMAX_DELAY);

        if (state == srtmState)
        {
            continue;
        }

        switch (srtmState)
        {
            case APP_SRTM_StateRun:
                assert(state == APP_SRTM_StateShutdown);
                SRTM_Dispatcher_Stop(disp);

                /* The default power state is APP_SRTM_PowerOn which applies to dual boot mode */
                if (peercorePower == APP_SRTM_PowerOff)
                {
                    PRINTF("#### Power on CA7 and boot it ####\r\n");
                    APP_PowerOnCA7(false);
                    APP_ResetCA7Regulators();

                    if ((PMC0->CTRL & PMC0_CTRL_PMC1ON_MASK) == 0)
                    {
                        /* Use PMIC regulator */
                        PMC0_EnablePmc1LdoRegulator(false);
                        PMC0_PowerOnPmc1();
                    }

                    /* Wake up CA7 */
                    MU_TriggerInterrupts(MUA, kMU_NmiInterruptTrigger);
                    /* Release CA7 reset */
                    MU_BootOtherCore(MUA, kMU_CoreBootFromAddr0);
                }
                else
                {
                    /* CA7 is already running */
                }

                APP_SRTM_InitPeerCore();
                SRTM_Dispatcher_Start(disp);

                state = APP_SRTM_StateRun;
                break;

            case APP_SRTM_StateLinkedUp:
                if (state == APP_SRTM_StateRun)
                {
                    SRTM_Dispatcher_Stop(disp);
                    /* Need to announce channel as we just linked up. */
                    APP_SRTM_Linkup();
                    /* LinkedUp is still in Run state. Don't change state variable here. */
                    SRTM_Dispatcher_Start(disp);
                }
                break;

            case APP_SRTM_StateShutdown:
                PRINTF("#### Shutdown CA7 ####\r\n");
                assert(state == APP_SRTM_StateRun);

                SRTM_Dispatcher_Stop(disp);
                APP_DisableRebootMonitor(true);
                /* Remove peer core from dispatcher */
                APP_SRTM_DeinitPeerCore();
                /* dispatcher can still handle proc message during peer core shutdown */
                SRTM_Dispatcher_Start(disp);

                /* Clear A7 resume entry */
                APP_UpdateSimDgo(3, 0xFFFFFFFF, 0);

                /* Shutdown CA7 domain power */
                PRINTF("#### Power off CA7 ####\r\n");
                APP_PowerOffCA7(false);
                state = APP_SRTM_StateShutdown;
                break;

            case APP_SRTM_StateReboot:
                PRINTF("#### Reset CA7 ####\r\n");
                assert(state == APP_SRTM_StateRun);

                SRTM_Dispatcher_Stop(disp);
                APP_DisableRebootMonitor(true);
                /* Remove peer core from dispatcher */
                APP_SRTM_DeinitPeerCore();

                /* Hold CA7 reset */
                MU_HardwareResetOtherCore(MUA, false, true, kMU_CoreBootFromAddr0);

                APP_ResetCA7Regulators();

                /* Release CA7 reset */
                MU_BootOtherCore(MUA, kMU_CoreBootFromAddr0);

                /* Restore srtmState to Run. */
                srtmState = APP_SRTM_StateRun;

                /* Initialize peer core and add to dispatcher */
                APP_SRTM_InitPeerCore();
                SRTM_Dispatcher_Start(disp);

                /* Do not need to change state. It's still Run. */
                break;

            default:
                assert(false);
                break;
        }
    }
}

static void SRTM_DispatcherTask(void *pvParameters)
{
    SRTM_Dispatcher_Run(disp);
}

static void APP_SRTM_I2C_ReleaseBusDelay(void)
{
    uint32_t i = 0U;
    for (i = 0U; i < APP_LPI2C_DELAY; i++)
    {
        __NOP();
    }
}

void APP_SRTM_I2C_ReleaseBus(void)
{
    uint8_t i = 0U;
    gpio_pin_config_t pin_config;

    /* Initialize PTB12/PTB13 as GPIO */
    pin_config.pinDirection = kGPIO_DigitalOutput;
    pin_config.outputLogic  = 1U;
    IOMUXC_SetPinMux(IOMUXC_PTA16_PTA16, 0U);
    IOMUXC_SetPinMux(IOMUXC_PTA17_PTA17, 0U);
    IOMUXC_SetPinConfig(IOMUXC_PTA16_PTA16, IOMUXC0_SW_MUX_CTL_PAD_OBE_MASK);
    IOMUXC_SetPinConfig(IOMUXC_PTA17_PTA17, IOMUXC0_SW_MUX_CTL_PAD_OBE_MASK);
    IOMUXC_SetPinMux(IOMUXC_PTB12_PTB12, 0U);
    IOMUXC_SetPinMux(IOMUXC_PTB13_PTB13, 0U);
    IOMUXC_SetPinConfig(IOMUXC_PTB12_PTB12, IOMUXC0_SW_MUX_CTL_PAD_OBE_MASK);
    IOMUXC_SetPinConfig(IOMUXC_PTB13_PTB13, IOMUXC0_SW_MUX_CTL_PAD_OBE_MASK);
    GPIO_PinInit(APP_AUDIO_LPI2C_SCL_GPIO, APP_AUDIO_LPI2C_SCL_PIN, &pin_config);
    GPIO_PinInit(APP_AUDIO_LPI2C_SDA_GPIO, APP_AUDIO_LPI2C_SDA_PIN, &pin_config);
    GPIO_PinInit(APP_PF1550_LPI2C_SCL_GPIO, APP_PF1550_LPI2C_SCL_PIN, &pin_config);
    GPIO_PinInit(APP_PF1550_LPI2C_SDA_GPIO, APP_PF1550_LPI2C_SDA_PIN, &pin_config);

    /* Drive SDA low first to simulate a start */
    GPIO_PinWrite(APP_PF1550_LPI2C_SDA_GPIO, APP_PF1550_LPI2C_SDA_PIN, 0U);
    APP_SRTM_I2C_ReleaseBusDelay();

    /* Send 9 pulses on SCL and keep SDA high */
    for (i = 0U; i < 9U; i++)
    {
        GPIO_PinWrite(APP_PF1550_LPI2C_SCL_GPIO, APP_PF1550_LPI2C_SCL_PIN, 0U);
        GPIO_PinWrite(APP_AUDIO_LPI2C_SCL_GPIO, APP_AUDIO_LPI2C_SCL_PIN, 0U);
        APP_SRTM_I2C_ReleaseBusDelay();

        GPIO_PinWrite(APP_PF1550_LPI2C_SDA_GPIO, APP_PF1550_LPI2C_SDA_PIN, 1U);
        GPIO_PinWrite(APP_AUDIO_LPI2C_SDA_GPIO, APP_AUDIO_LPI2C_SDA_PIN, 1U);
        APP_SRTM_I2C_ReleaseBusDelay();

        GPIO_PinWrite(APP_PF1550_LPI2C_SCL_GPIO, APP_PF1550_LPI2C_SCL_PIN, 1U);
        GPIO_PinWrite(APP_AUDIO_LPI2C_SCL_GPIO, APP_AUDIO_LPI2C_SCL_PIN, 1U);
        APP_SRTM_I2C_ReleaseBusDelay();
        APP_SRTM_I2C_ReleaseBusDelay();
    }

    /* Send stop */
    GPIO_PinWrite(APP_PF1550_LPI2C_SCL_GPIO, APP_PF1550_LPI2C_SCL_PIN, 0U);
    GPIO_PinWrite(APP_AUDIO_LPI2C_SCL_GPIO, APP_AUDIO_LPI2C_SCL_PIN, 0U);
    APP_SRTM_I2C_ReleaseBusDelay();

    GPIO_PinWrite(APP_PF1550_LPI2C_SDA_GPIO, APP_PF1550_LPI2C_SDA_PIN, 0U);
    GPIO_PinWrite(APP_AUDIO_LPI2C_SDA_GPIO, APP_AUDIO_LPI2C_SDA_PIN, 0U);
    APP_SRTM_I2C_ReleaseBusDelay();

    GPIO_PinWrite(APP_PF1550_LPI2C_SCL_GPIO, APP_PF1550_LPI2C_SCL_PIN, 1U);
    GPIO_PinWrite(APP_AUDIO_LPI2C_SCL_GPIO, APP_AUDIO_LPI2C_SCL_PIN, 1U);
    APP_SRTM_I2C_ReleaseBusDelay();

    GPIO_PinWrite(APP_PF1550_LPI2C_SDA_GPIO, APP_PF1550_LPI2C_SDA_PIN, 1U);
    GPIO_PinWrite(APP_AUDIO_LPI2C_SDA_GPIO, APP_AUDIO_LPI2C_SDA_PIN, 1U);
    APP_SRTM_I2C_ReleaseBusDelay();
}

static void APP_SRTM_InitPeriph(bool resume)
{
    gpio_pin_config_t gpioConfig = {
        kGPIO_DigitalOutput,
        0U,
    };

    /* Init output DDR_SW_EN# to enabled. */
    GPIO_PinInit(gpios[APP_GPIO_IDX(APP_PIN_DDR_SW_EN)], APP_PIN_IDX(APP_PIN_DDR_SW_EN), &gpioConfig);
    /* Init output A7_POW_EN# to enabled. */
    GPIO_PinInit(gpios[APP_GPIO_IDX(APP_PIN_A7_POW_EN)], APP_PIN_IDX(APP_PIN_A7_POW_EN), &gpioConfig);

    MU_Init(MUA);
}

static void APP_SRTM_RecycleWakeupProc(srtm_message_t message, void *param)
{
    wakeupCA7Proc = message;
}

void APP_SRTM_Init(void)
{
    APP_SRTM_InitPeriph(false);

    if (PMC0->CTRL & PMC0_CTRL_PMC1ON_MASK)
    {
        peercorePower = APP_SRTM_PowerOn;
    }
    else
    {
        peercorePower = APP_SRTM_PowerOff;
    }

    /* Set IRQ priority. */
    NVIC_SetPriority(CMC1_IRQn, APP_A7Reset_IRQ_PRIO);
    NVIC_SetPriority(USBPHY_IRQn, APP_USB_IRQ_PRIO);
    NVIC_SetPriority(LLWU0_IRQn, APP_LLWU0_IRQ_PRIO);
    EnableIRQ(LLWU0_IRQn);

    monSig = xSemaphoreCreateBinary();
    assert(monSig);

    suspendTimer =
        xTimerCreate("Suspend", APP_MS2TICK(APP_SUSPEND_TIMER_PERIOD_MS), pdFALSE, NULL, APP_SuspendTimerCallback);
    assert(suspendTimer);

    heartBeatTimer = xTimerCreate("HeartBeat", APP_MS2TICK(APP_HEART_BEAT_TIMER_PERIOD_MS), pdFALSE, NULL,
                                  APP_HeartBeatTimerCallback);
    assert(heartBeatTimer);

    linkupTimer =
        xTimerCreate("Linkup", APP_MS2TICK(APP_LINKUP_TIMER_PERIOD_MS), pdFALSE, NULL, APP_LinkupTimerCallback);
    assert(linkupTimer);

    suspendContext.io.data[APP_INPUT_VOL_PLUS].timer =
        xTimerCreate("Vol+", APP_MS2TICK(50), pdFALSE, NULL, APP_VolPlusTimerCallback);
    assert(suspendContext.io.data[APP_INPUT_VOL_PLUS].timer);
    suspendContext.io.data[APP_INPUT_VOL_MINUS].timer =
        xTimerCreate("Vol-", APP_MS2TICK(50), pdFALSE, NULL, APP_VolMinusTimerCallback);
    assert(suspendContext.io.data[APP_INPUT_VOL_MINUS].timer);

    suspendContext.io.data[APP_INPUT_ONOFF].timer =
        xTimerCreate("OnOff", APP_MS2TICK(50), pdTRUE, NULL, APP_OnOffTimerCallback);
    assert(suspendContext.io.data[APP_INPUT_ONOFF].timer);

    /* Create timer used in pedometer polling. Period 10 will be overwritten in Pedometer initialization. */
    pedometer.timer = xTimerCreate("Pedometer", 10, pdTRUE, NULL, APP_PedometerTimerCallback);
    assert(pedometer.timer);

    /* Create procedure message to wake up CA7 core, used in IRQ handler. Parameter 1 stands for USB wakeup. */
    wakeupCA7Proc = SRTM_Procedure_Create(APP_SRTM_DoWakeupCA7, (void *)1, NULL);
    assert(wakeupCA7Proc);
    SRTM_Message_SetFreeFunc(wakeupCA7Proc, APP_SRTM_RecycleWakeupProc, NULL);

    /* Create SRTM dispatcher */
    disp = SRTM_Dispatcher_Create();

    xTaskCreate(SRTM_MonitorTask, "SRTM monitor", 256U, NULL, APP_SRTM_MONITOR_TASK_PRIO, NULL);
    xTaskCreate(SRTM_DispatcherTask, "SRTM dispatcher", 512U, NULL, APP_SRTM_DISPATCHER_TASK_PRIO, NULL);
}

void APP_SRTM_Suspend(void)
{
    suspendContext.mu.CR = MUA->CR;

    APP_SRTM_DeinitPmicDevice();
    APP_SRTM_DeinitSensorDevice();
    APP_SRTM_DeinitCodecDevice();
}

void APP_SRTM_Resume(bool resume)
{
    if (resume)
    {
        APP_SRTM_InitPeriph(true);
        APP_SRTM_InitRtcDevice(true);
        APP_SRTM_InitAudioDevice();
        APP_SRTM_InitIoKeyDevice();
        MUA->CR = suspendContext.mu.CR;
    }
    else
    {
        APP_SRTM_InitCodecDevice();
    }

    /* Even if suspend fails, I2C handle is destroyed. Need to initialize again. */
    APP_SRTM_InitPmicDevice();
    APP_SRTM_InitSensorDevice();
}

void APP_SRTM_BootCA7(void)
{
    srtm_procedure_t proc = SRTM_Procedure_Create(APP_SRTM_ControlCA7, (void *)APP_SRTM_StateRun, NULL);

    assert(proc);
    /* Fresh power up: Need SRTM monitor to prepare communication */
    SRTM_Dispatcher_PostProc(disp, proc);
}

void APP_SRTM_RebootCA7(void)
{
    srtm_procedure_t proc = SRTM_Procedure_Create(APP_SRTM_ControlCA7, (void *)APP_SRTM_StateReboot, NULL);
    PRINTF("M4 reboot A7\r\n");
    assert(proc);
    SRTM_Dispatcher_PostProc(disp, proc);
}

void APP_SRTM_ShutdownCA7(void)
{
    srtm_procedure_t proc = SRTM_Procedure_Create(APP_SRTM_ControlCA7, (void *)APP_SRTM_StateShutdown, NULL);

    assert(proc);
    SRTM_Dispatcher_PostProc(disp, proc);
}

void APP_SRTM_WakeupCA7(void)
{
    srtm_procedure_t proc = SRTM_Procedure_Create(APP_SRTM_DoWakeupCA7, NULL, NULL);

    assert(proc);
    SRTM_Dispatcher_PostProc(disp, proc);
}

uint32_t APP_SRTM_GetPmicReg(uint32_t reg)
{
    srtm_procedure_t proc;
    uint32_t value;

    proc = SRTM_Procedure_Create(APP_SRTM_GetPf1550Reg, (void *)reg, (void *)&value);
    assert(proc);
    SRTM_Dispatcher_CallProc(disp, proc, SRTM_WAIT_FOR_EVER);
    SRTM_Procedure_Destroy(proc);

    return value;
}

void APP_SRTM_SetPmicReg(uint32_t reg, uint32_t value)
{
    srtm_procedure_t proc;

    proc = SRTM_Procedure_Create(APP_SRTM_SetPf1550Reg, (void *)reg, (void *)value);
    assert(proc);
    SRTM_Dispatcher_CallProc(disp, proc, SRTM_WAIT_FOR_EVER);
    SRTM_Procedure_Destroy(proc);
}

void APP_SRTM_SetWakeupPin(uint16_t ioId, uint16_t event)
{
    srtm_procedure_t proc;

    proc = SRTM_Procedure_Create(APP_SRTM_DoSetWakeupPin, (void *)(uint32_t)ioId, (void *)(uint32_t)event);
    assert(proc);

    SRTM_Dispatcher_CallProc(disp, proc, SRTM_WAIT_FOR_EVER);
    SRTM_Procedure_Destroy(proc);
}

void APP_SRTM_SetWakeupModule(uint32_t module, bool enable)
{
    srtm_procedure_t proc;

    proc = SRTM_Procedure_Create(APP_SRTM_DoSetWakeupModule, (void *)module, (void *)(uint32_t)enable);
    assert(proc);

    SRTM_Dispatcher_CallProc(disp, proc, SRTM_WAIT_FOR_EVER);
    SRTM_Procedure_Destroy(proc);
}

void APP_SRTM_SetRpmsgMonitor(app_rpmsg_monitor_t monitor, void *param)
{
    rpmsgMonitor      = monitor;
    rpmsgMonitorParam = param;
}

void APP_SRTM_SetIRQHandler(app_irq_handler_t handler, void *param)
{
    irqHandler      = handler;
    irqHandlerParam = param;
}

struct rpmsg_lite_instance *APP_SRTM_GetRPMsgHandle(void)
{
    return rpmsgHandle;
}
