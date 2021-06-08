/*
 * Copyright 2017-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_asmc.h"
#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "fsl_irqsteer.h"
#include "fsl_lpit.h"
#include "lpm.h"
#include "fsl_wdog32.h"
#include "power_mode_switch.h"
#include "fsl_sc_event.h"

#include "misc/misc_api.h"
#include "svc/pad/pad_api.h"
#include "imx8qm_pads.h"
#include "fsl_lpi2c.h"
#include "fsl_lpuart.h"
#include "fsl_flexcan.h"
#include "fsl_rgpio.h"
#include "app_srtm.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Demo configuration macro */
#define APP_USE_CAN_AS_WAKEUP (1U)

/* LPIT related macro*/
#define APP_LPIT_BASE       CM4_1__LPIT
#define APP_LPIT_IRQn       M4_1_LPIT_IRQn
#define APP_LPIT_IRQHandler M4_1_LPIT_IRQHandler
#define APP_LPIT_RSRC       SC_R_M4_1_PIT
#define APP_LPIT_CLK_NAME   kCLOCK_M4_1_Lpit
#define LPIT_SOURCECLOCK    CLOCK_GetIpFreq(kCLOCK_M4_1_Lpit)
#define APP_LPIT_IRQ_PRIO   (5U)

/* CAN Transceiver config related */
#define APP_IOEXP_I2C          CM4_1__LPI2C
#define APP_IOEXP_I2C_ADDR     (0x20U)
#define APP_IOEXP_CAN_EN_MASK  (0x8U)
#define APP_IOEXP_CAN_STB_MASK (0x20U)
#define I2C_RELEASE_SCL_GPIO   CM4_1__RGPIO
#define I2C_RELEASE_SDA_GPIO   CM4_1__RGPIO
#define I2C_RELEASE_SCL_PIN    (2U)
#define I2C_RELEASE_SDA_PIN    (3U)

/* PCA6416 I2C Register Map */
#define PCA6416_REG_INPUT_PORT_0              (0x0)
#define PCA6416_REG_INPUT_PORT_1              (0x1)
#define PCA6416_REG_OUTPUT_PORT_0             (0x2)
#define PCA6416_REG_OUTPUT_PORT_1             (0x3)
#define PCA6416_REG_POLARITY_INVERSION_PORT_0 (0x4)
#define PCA6416_REG_POLARITY_INVERSION_PORT_1 (0x5)
#define PCA6416_REG_CONFIGURATION_PORT_0      (0x6)
#define PCA6416_REG_CONFIGURATION_PORT_1      (0x7)

/* FlexCAN */
#define APP_WAKEUP_CAN_NAME "FlexCAN0"
#define APP_CAN             DMA__CAN0
#define APP_CAN_RSRC        SC_R_CAN_0
#define APP_CAN_CLK_FREQ    CLOCK_GetIpFreq(kCLOCK_DMA_Can0)
#define APP_CAN_IRQn        DMA_FLEXCAN0_INT_IRQn
#define APP_CAN_IRQ_PRIO    (4U)
#define SET_CAN_QUANTUM     1
#define PSEG1               3
#define PSEG2               2
#define PROPSEG             3
/* FlexCAN message buffer */
#define RX_MESSAGE_BUFFER_NUM (9)
#define TX_MESSAGE_BUFFER_NUM (8)

#define APP_WAKEUP_BUTTON_NAME "SW1 ON/OFF"
#define APP_WAKEUP_PAD_NAME    "UART RX Pad"
#define APP_WAKEUP_PAD         SC_P_UART0_RTS_B

#define APP_WDOG CM4_1__WDOG

/* SC Event(IPC MU) interrupt priority. */
#define APP_SCEVENT_IRQ_PRIO (6U)

/* IPC MU */
#define APP_IPC_MU_RSRC SC_R_M4_1_MU_1A

/* Get the NVIC IRQn of given IRQSTEER IRQn */
#define GET_IRQSTEER_MASTER_IRQn(IRQn) \
    (IRQn_Type)(IRQSTEER_0_IRQn + (IRQn - FSL_FEATURE_IRQSTEER_IRQ_START_INDEX) / 64U)

#define RTN_ERR(X)                        \
    if ((X) != SC_ERR_NONE)               \
    {                                     \
        assert("Error in SCFW API call"); \
    }
/* WDOG reset timeout */
#define WDOG_TIMEOUT (32768 / 2) /*0.5s*/
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_InitDebugConsole(void);
/* Hook function called before power mode switch. */
bool APP_PowerPreSwitchHook(asmc_power_state_t originPowerState, lpm_power_mode_t targetMode);
/* Hook function called after power mode switch. */
void APP_PowerPostSwitchHook(asmc_power_state_t originPowerState, lpm_power_mode_t targetMode, bool result);
/* Init CAN Event Task. */
void APP_InitCANEventTask(void);
/* Configure CAN wakeup source. */
void APP_ConfigCanWakeupSrc(void);
/* CAN event handler */
void CanEventHandleTask(void *pvParameters);
/* Handler function for A core reboot. */
void APP_PeerCoreRebootHandler(void);
/* FreeRTOS implemented Malloc failed hook. */
extern void vApplicationMallocFailedHook(void);

/* Initialize the Wakeup source located in M4 subysystem. */
void APP_InitInternalWakeupSrc(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/
#if (defined(APP_USE_CAN_AS_WAKEUP) && APP_USE_CAN_AS_WAKEUP)
static SemaphoreHandle_t s_rxFinishSig; /* CAN receive finished signal    */
flexcan_handle_t flexcanHandle;
flexcan_mb_transfer_t txXfer, rxXfer;
flexcan_frame_t frame;
uint32_t txIdentifier = 0x123;
uint32_t rxIdentifier = 0x321;
#endif /* APP_USE_CAN_AS_WAKEUP */
sc_ipc_t ipc;

static sc_rm_pt_t a_pt;              /* Partition ID of A core         */
static sc_pad_wakeup_t s_uartWakeup; /* UART Pad wakeup configuration  */
static uint8_t s_wakeupTimeout;      /* Wakeup timeout. (Unit: Second) */
SemaphoreHandle_t s_wakeupSig;       /* Wakeup signal                  */

app_wakeup_source_t g_wakeupSource; /* Wakeup source.                 */

static const char *s_modeNames[] = {"RUN", "WAIT", "STOP", "VLPR", "VLPW", "VLPS", "LLS", "VLLS"};
/*******************************************************************************
 * Code
 ******************************************************************************/
static void APP_Resume(lpm_power_mode_t targetMode, bool result)
{
    if (result &&
        ((LPM_PowerModeLls == targetMode) || (LPM_PowerModeVlls == targetMode))) /* System really go into LLS/VLLS */
    {
        BOARD_InitMemory();
    }

    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_MU_6B, SC_PM_PW_MODE_ON));
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_MU_7B, SC_PM_PW_MODE_ON));
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_IRQSTR_M4_1, SC_PM_PW_MODE_ON));

    /* If CAN wake up interrupt pending, clear the CAN IPG_STOP signal. */
    if (kAPP_WakeupSourceCan == g_wakeupSource)
    {
        if (FLEXCAN_GetStatusFlags(APP_CAN) & kFLEXCAN_WakeUpIntFlag)
        {
            RTN_ERR(sc_misc_set_control(ipc, APP_CAN_RSRC, SC_C_IPG_STOP, 0U));
            RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_CAN_0, SC_PM_PW_MODE_ON));
        }
    }

    /* Resume SRTM */
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_M4_1_I2C, SC_PM_PW_MODE_ON));
    if (CLOCK_SetIpFreq(kCLOCK_M4_1_Lpi2c, SC_66MHZ) == 0)
    {
        PRINTF("Error: Failed to set LPI2C frequency\r\n");
    }
    APP_SRTM_Resume();
}

static bool APP_Suspend(void)
{
    /* If SRTM is busy, abort suspend. */
    if (!APP_SRTM_Suspend())
    {
        return false;
    }
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_M4_1_I2C, SC_PM_PW_MODE_OFF));

    /* Prepare peripherals into low power mode. */
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_IRQSTR_M4_1, SC_PM_PW_MODE_OFF));
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_MU_6B, SC_PM_PW_MODE_OFF));
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_MU_7B, SC_PM_PW_MODE_OFF));

    if (kAPP_WakeupSourceLpit == g_wakeupSource)
    {
        RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_M4_1_PIT, SC_PM_PW_MODE_LP));
    }
    else
    {
        RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_M4_1_PIT, SC_PM_PW_MODE_OFF));
    }

    /* If wakeup source not comming from FlexCAN, power off to save power. */
    if (kAPP_WakeupSourceCan == g_wakeupSource)
    {
        /* FlexCAN need clock for self wake, the lowest power mode for CAN function is SC_PM_PW_MODE_LP. */
        RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_CAN_0, SC_PM_PW_MODE_LP));
    }
    else
    {
        RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_CAN_0, SC_PM_PW_MODE_OFF));
    }

    return true;
}

bool APP_PowerPreSwitchHook(asmc_power_state_t originPowerState, lpm_power_mode_t targetMode)
{
    if ((LPM_PowerModeRun != targetMode) && (LPM_PowerModeVlpr != targetMode))
    {
        /* Abort suspend if APP suspend failed. */
        if (!APP_Suspend())
        {
            return false;
        }
    }

    /* Wait for debug console output finished. */
    while (!(kLPUART_TransmissionCompleteFlag & LPUART_GetStatusFlags((LPUART_Type *)BOARD_DEBUG_UART_BASEADDR)))
    {
    }
    DbgConsole_Deinit();
    /* Power off Debug console UART. */
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, BOARD_DEBUG_UART_SC_RSRC, SC_PM_PW_MODE_OFF));

    return true;
}

void APP_PowerPostSwitchHook(asmc_power_state_t originPowerState, lpm_power_mode_t targetMode, bool result)
{
    /* Resume peripherals. */
    APP_InitDebugConsole();

    if ((LPM_PowerModeRun != targetMode) && (LPM_PowerModeVlpr != targetMode))
    {
        APP_Resume(targetMode, result);
    }
}

void APP_InitDebugConsole(void)
{
    uint32_t freq = SC_24MHZ;

    /* Power on Local LPUART for M4. */
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, BOARD_DEBUG_UART_SC_RSRC, SC_PM_PW_MODE_ON));
    /* Enable clock of Local LPUART for M4. */
    CLOCK_EnableClockExt(BOARD_DEBUG_UART_CLKSRC, 0);
    /* Set clock Frequency of Local LPUART for M4. */
    freq = CLOCK_SetIpFreq(BOARD_DEBUG_UART_CLKSRC, freq);

    DbgConsole_Init(BOARD_DEBUG_UART_INSTANCE, BOARD_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, freq);
}

static void i2c_release_bus_delay(void)
{
    uint32_t i = 0;
    for (i = 0; i < 264U; i++)
    {
        __NOP();
    }
}

void BOARD_I2C_ReleaseBus(void)
{
    uint8_t i = 0;
    rgpio_pin_config_t pin_config;

    pin_config.pinDirection = kRGPIO_DigitalOutput;
    pin_config.outputLogic  = 1U;

    RGPIO_PinInit(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, &pin_config); /* I2C SCL */
    RGPIO_PinInit(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, &pin_config); /* I2C SDA */

    /* Drive SDA low first to simulate a start */
    RGPIO_WritePinOutput(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 0U);
    i2c_release_bus_delay();

    /* Send 9 pulses on SCL and keep SDA high */
    for (i = 0; i < 9; i++)
    {
        RGPIO_WritePinOutput(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 0U);
        i2c_release_bus_delay();

        RGPIO_WritePinOutput(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 1U);
        i2c_release_bus_delay();

        RGPIO_WritePinOutput(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 1U);
        i2c_release_bus_delay();
        i2c_release_bus_delay();
    }

    /* Send stop */
    RGPIO_WritePinOutput(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 0U);
    i2c_release_bus_delay();

    RGPIO_WritePinOutput(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 0U);
    i2c_release_bus_delay();

    RGPIO_WritePinOutput(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 1U);
    i2c_release_bus_delay();

    RGPIO_WritePinOutput(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 1U);
    i2c_release_bus_delay();
}

static void BOARD_ConfigureIOExpander(void)
{
    uint32_t i;
    uint8_t txBuffer[2] = {0};

    BOARD_LPI2C_Init(APP_IOEXP_I2C, CLOCK_GetIpFreq(kCLOCK_M4_1_Lpi2c));

    /* Configure CAN IO EXPANDER PCA6416 to initialize TJA1043T. */
    txBuffer[0] = 0x0U;
    BOARD_LPI2C_Send(APP_IOEXP_I2C, APP_IOEXP_I2C_ADDR, PCA6416_REG_CONFIGURATION_PORT_0, 1, txBuffer, 1);
    BOARD_LPI2C_Receive(APP_IOEXP_I2C, APP_IOEXP_I2C_ADDR, PCA6416_REG_OUTPUT_PORT_0, 1, txBuffer, 1);
    txBuffer[0] &= ~(APP_IOEXP_CAN_EN_MASK | APP_IOEXP_CAN_STB_MASK);
    BOARD_LPI2C_Send(APP_IOEXP_I2C, APP_IOEXP_I2C_ADDR, PCA6416_REG_OUTPUT_PORT_0, 1, txBuffer, 1);
    i = 0;
    while (i < 150000)
    {
        __ASM("nop");
        i++;
    }
    txBuffer[0] |= APP_IOEXP_CAN_EN_MASK | APP_IOEXP_CAN_STB_MASK;
    BOARD_LPI2C_Send(APP_IOEXP_I2C, APP_IOEXP_I2C_ADDR, PCA6416_REG_OUTPUT_PORT_0, 1, txBuffer, 1);

    BOARD_LPI2C_Deinit(APP_IOEXP_I2C);
    /* Don't power off I2C as it will be used by audio codec. */
}


/* Handler function for A core reboot. */
void APP_PeerCoreRebootHandler(void)
{
    APP_SRTM_PeerCoreRebootHandler();
}

#if (defined(APP_USE_CAN_AS_WAKEUP) && APP_USE_CAN_AS_WAKEUP)
/*!
 * @brief FlexCAN Call Back function
 */
static void flexcan_callback(CAN_Type *base, flexcan_handle_t *handle, status_t status, uint32_t result, void *userData)
{
    BaseType_t reschedule = 0;

    switch (status)
    {
        case kStatus_FLEXCAN_RxIdle:
            if (RX_MESSAGE_BUFFER_NUM == result)
            {
                xSemaphoreGiveFromISR(s_rxFinishSig, &reschedule);
                portYIELD_FROM_ISR(reschedule);
            }
            break;

        case kStatus_FLEXCAN_TxIdle:
            if (TX_MESSAGE_BUFFER_NUM == result)
            {
                xSemaphoreGiveFromISR(s_wakeupSig, &reschedule);
                portYIELD_FROM_ISR(reschedule);
            }
            break;
        /* Handle selfwake. */
        case kStatus_FLEXCAN_WakeUp:
            /* Disable CAN Wakeup interrupt, start receive data through Rx Message Buffer. */
            FLEXCAN_DisableInterrupts(APP_CAN, kFLEXCAN_WakeUpInterruptEnable);
            rxXfer.mbIdx = (uint8_t)RX_MESSAGE_BUFFER_NUM;
            rxXfer.frame = &frame;
            FLEXCAN_TransferReceiveNonBlocking(APP_CAN, &flexcanHandle, &rxXfer);
            break;

        default:
            break;
    }
}

void APP_ConfigCanWakeupSrc(void)
{
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_CAN_0, SC_PM_PW_MODE_ON));
    /* Set Peripheral clock frequency. */
    if (CLOCK_SetIpFreq(kCLOCK_DMA_Can0, SC_24MHZ) == 0)
    {
        PRINTF("Error: Failed to set FLEXCAN frequency\r\n");
    }

    flexcan_config_t flexcanConfig;
    flexcan_rx_mb_config_t mbConfig;

    /* Get FlexCAN module default Configuration. */
    /*
     * flexcanConfig.clkSrc                 = kFLEXCAN_ClkSrc0;
     * flexcanConfig.baudRate               = 1000000U;
     * flexcanConfig.maxMbNum               = 16;
     * flexcanConfig.enableLoopBack         = false;
     * flexcanConfig.enableSelfWakeup       = false;
     * flexcanConfig.enableIndividMask      = false;
     * flexcanConfig.disableSelfReception   = false;
     * flexcanConfig.enableListenOnlyMode   = false;
     * flexcanConfig.enableDoze             = false;
     */
    FLEXCAN_GetDefaultConfig(&flexcanConfig);
    /* Init FlexCAN module. */
    flexcanConfig.enableSelfWakeup = true;
    flexcanConfig.baudRate         = 500000U;
#if (defined(SET_CAN_QUANTUM) && SET_CAN_QUANTUM)
    flexcanConfig.timingConfig.phaseSeg1 = PSEG1;
    flexcanConfig.timingConfig.phaseSeg2 = PSEG2;
    flexcanConfig.timingConfig.propSeg   = PROPSEG;
#endif
    FLEXCAN_Init(APP_CAN, &flexcanConfig, APP_CAN_CLK_FREQ);
    /* Create FlexCAN handle structure and set call back function. */
    FLEXCAN_TransferCreateHandle(APP_CAN, &flexcanHandle, flexcan_callback, NULL);

    /* Set Rx Masking mechanism. */
    FLEXCAN_SetRxMbGlobalMask(APP_CAN, FLEXCAN_RX_MB_STD_MASK(rxIdentifier, 0, 0));

    /* Setup Rx Message Buffer. */
    mbConfig.format = kFLEXCAN_FrameFormatStandard;
    mbConfig.type   = kFLEXCAN_FrameTypeData;
    mbConfig.id     = FLEXCAN_ID_STD(rxIdentifier);
    FLEXCAN_SetRxMbConfig(APP_CAN, RX_MESSAGE_BUFFER_NUM, &mbConfig, true);
    /* Setup Tx Message Buffer. */
    FLEXCAN_SetTxMbConfig(APP_CAN, TX_MESSAGE_BUFFER_NUM, true);

    NVIC_SetPriority(GET_IRQSTEER_MASTER_IRQn(APP_CAN_IRQn), APP_CAN_IRQ_PRIO);
    IRQSTEER_EnableInterrupt(IRQSTEER, APP_CAN_IRQn);

    /*
     * CAN STOP sequence:
     * 1.  Config CAN to enable self wake mode(MCR[SLFWAK]),  enable self wake interrupt.
     * 2.  Assert IPG_STOP signal using SCFW API for CAN. Wait the MCR[LPMACK] set, to ensure the CAN going stop.
     * 3.  Wait CAN wakeup and then clear the IPG_STOP signal using SCFW API. (Done in APP_PowerPostSwitchHook())
     */
    /* 1. Self wake configured during CAN initialize, so enable self wake interrupt, and assert STOP signal using
     * SCFW API for CAN. */
    FLEXCAN_EnableInterrupts(APP_CAN, kFLEXCAN_WakeUpInterruptEnable);
    RTN_ERR(sc_misc_set_control(ipc, APP_CAN_RSRC, SC_C_IPG_STOP, 1U));
    /* 2. Wait the CAN acknowleged the STOP */
    while (!(APP_CAN->MCR & CAN_MCR_LPMACK_MASK))
    {
    }
}

/*
 * Handle CAN event in task.
 * NOTE: THE SCFW API CAN NOT BE CALLED IN INTERRUPT CONTEXT FOR RTOS ENVIRONMENT.
 */
void CanEventHandleTask(void *pvParameters)
{
    for (;;)
    {
        /* Wait RX completed, and send the received message back. */
        xSemaphoreTake(s_rxFinishSig, portMAX_DELAY);
        PRINTF("CAN Rx MB ID: 0x%3x, Rx MB data: 0x%x\r\n", frame.id >> CAN_ID_STD_SHIFT, frame.dataByte0);

        frame.id     = FLEXCAN_ID_STD(txIdentifier);
        txXfer.mbIdx = (uint8_t)TX_MESSAGE_BUFFER_NUM;
        txXfer.frame = &frame;
        FLEXCAN_TransferSendNonBlocking(APP_CAN, &flexcanHandle, &txXfer);
    }
}

void APP_InitCANEventTask(void)
{
    s_rxFinishSig = xSemaphoreCreateBinary();
    xTaskCreate(CanEventHandleTask, "CAN Event Task", 512U, NULL, tskIDLE_PRIORITY + 3U, NULL);
}
#endif /* APP_USE_CAN_AS_WAKEUP */
void vApplicationMallocFailedHook(void)
{
    PRINTF("Malloc Failed!!!\r\n");
}

void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
    uint32_t irqMask;
    asmc_power_state_t curPowerState;
    lpm_power_mode_t targetPowerMode;
    bool result;

    irqMask = DisableGlobalIRQ();

    /* Only when no context switch is pending and no task is waiting for the scheduler
     * to be unsuspended then enter low power entry.
     */
    if (eTaskConfirmSleepModeStatus() != eAbortSleep)
    {
        targetPowerMode = LPM_GetPowerMode();
        if (targetPowerMode != LPM_PowerModeRun && targetPowerMode != LPM_PowerModeVlpr)
        {
            /* Only wait when target power mode is not running */
            curPowerState = ASMC_GetPowerModeState(BBS_SIM);
            if (APP_PowerPreSwitchHook(curPowerState, targetPowerMode))
            {
                result = LPM_WaitForInterrupt((uint64_t)1000 * xExpectedIdleTime / configTICK_RATE_HZ);
                APP_PowerPostSwitchHook(curPowerState, targetPowerMode, result);
            }
        }
    }

    EnableGlobalIRQ(irqMask);
}

void APP_LPIT_IRQHandler(void)
{
    if (kLPIT_Channel0TimerInterruptEnable & LPIT_GetEnabledInterrupts(APP_LPIT_BASE))
    {
        /* Disable timer interrupts for channel 0 */
        LPIT_DisableInterrupts(APP_LPIT_BASE, kLPIT_Channel0TimerInterruptEnable);
        /* Clear interrupt flag.*/
        LPIT_ClearStatusFlags(APP_LPIT_BASE, kLPIT_Channel0TimerFlag);
        /* Stop LPIT Timer */
        LPIT_StopTimer(APP_LPIT_BASE, kLPIT_Chnl_0);

        xSemaphoreGiveFromISR(s_wakeupSig, NULL);
        portYIELD_FROM_ISR(pdTRUE);
    }

    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
    exception return operation might vector to incorrect interrupt */
    __DSB();
}

/*!
 * @brief Configure WDOG to trigger reset.
 */
static void APP_ConfigWDOG32Reset(uint16_t timeout)
{
    wdog32_config_t config;

    WDOG32_GetDefaultConfig(&config);
    config.testMode = kWDOG32_UserModeEnabled;

    config.clockSource  = kWDOG32_ClockSource1; /* lpo_clk xtal 32KHz */
    config.prescaler    = kWDOG32_ClockPrescalerDivide1;
    config.windowValue  = 0U;
    config.timeoutValue = timeout;

    config.enableWindowMode = false;
    config.enableWdog32     = true;

    WDOG32_Init(APP_WDOG, &config);
}
/*!
 * @brief Init the Wake up source in M4 Subsystem.
 */
static void APP_InitLpitWakeupSrc()
{
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, APP_LPIT_RSRC, SC_PM_PW_MODE_ON));
    RTN_ERR(sc_pm_clock_enable(ipc, APP_LPIT_RSRC, SC_PM_CLK_PER, true, false));
    if (CLOCK_SetIpFreq(APP_LPIT_CLK_NAME, SC_24MHZ) == 0)
    {
        PRINTF("Error: Failed to set LPIT frequency\r\n");
    }

    lpit_config_t lpitConfig;
    lpit_chnl_params_t lpitChannelConfig;

    /* Setup LPIT. */
    LPIT_GetDefaultConfig(&lpitConfig);
    lpitConfig.enableRunInDebug = false;
    lpitConfig.enableRunInDoze  = true;

    /* Init lpit module */
    LPIT_Init(APP_LPIT_BASE, &lpitConfig);

    lpitChannelConfig.chainChannel          = false;
    lpitChannelConfig.enableReloadOnTrigger = false;
    lpitChannelConfig.enableStartOnTrigger  = false;
    lpitChannelConfig.enableStopOnTimeout   = false;
    lpitChannelConfig.timerMode             = kLPIT_PeriodicCounter;
    /* Set default values for the trigger source */
    lpitChannelConfig.triggerSelect = kLPIT_Trigger_TimerChn0;
    lpitChannelConfig.triggerSource = kLPIT_TriggerSource_External;

    /* Init lpit channel 0 */
    LPIT_SetupChannel(APP_LPIT_BASE, kLPIT_Chnl_0, &lpitChannelConfig);

    NVIC_SetPriority(APP_LPIT_IRQn, APP_LPIT_IRQ_PRIO);
    EnableIRQ(APP_LPIT_IRQn);
}

/*!
 * @brief Get input from user about wakeup timeout
 */
static uint8_t APP_GetWakeupTimeout(void)
{
    uint8_t timeout;

    while (1)
    {
        PRINTF("Select the wake up timeout in seconds.\r\n");
        PRINTF("The allowed range is 1s ~ 9s.\r\n");
        PRINTF("Eg. enter 5 to wake up in 5 seconds.\r\n");
        PRINTF("\r\nWaiting for input timeout value...\r\n\r\n");

        timeout = GETCHAR();
        PRINTF("%c\r\n", timeout);
        if ((timeout > '0') && (timeout <= '9'))
        {
            return timeout - '0';
        }
        PRINTF("Wrong value!\r\n");
    }
}

/*!
 * @brief Get wakeup source by user input.
 */
static app_wakeup_source_t APP_GetWakeupSource(void)
{
    uint8_t ch;

    while (1)
    {
        PRINTF("Select the wake up source:\r\n");
        PRINTF("Press T for LPIT - Low Power Timer\r\n");
        PRINTF("Press S for switch/button %s. \r\n", APP_WAKEUP_BUTTON_NAME);
#if (defined(APP_USE_CAN_AS_WAKEUP) && APP_USE_CAN_AS_WAKEUP)
        PRINTF("Press C for CAN - %s. \r\n", APP_WAKEUP_CAN_NAME);
#endif /* APP_USE_CAN_AS_WAKEUP */
        PRINTF("Press P for PAD - %s. \r\n", APP_WAKEUP_PAD_NAME);

        PRINTF("\r\nWaiting for key press..\r\n\r\n");

        ch = GETCHAR();

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }

        if (ch == 'T')
        {
            return kAPP_WakeupSourceLpit;
        }
        else if (ch == 'S')
        {
            return kAPP_WakeupSourcePin;
        }
        else if (ch == 'C')
        {
            return kAPP_WakeupSourceCan;
        }
        else if (ch == 'P')
        {
            return kAPP_WakeupSourcePad;
        }
        else
        {
            PRINTF("Wrong value!\r\n");
        }
    }
}

/*!
 * @brief Get wakeup timeout and wakeup source.
 */
static void APP_GetWakeupConfig(lpm_power_mode_t targetMode)
{
    /* Get wakeup source by user input. */
    g_wakeupSource = APP_GetWakeupSource();

    if ((LPM_PowerModeVlls == targetMode) || (LPM_PowerModeLls == targetMode))
    {
        while (kAPP_WakeupSourceLpit == g_wakeupSource)
        {
            /* In LLS/VLLS mode, the NVIC/AWIC could not work. */
            PRINTF("Not support LPIT wakeup because NVIC/AWIC is disabled in LLS/VLLS mode.\r\n");
            g_wakeupSource = APP_GetWakeupSource();
        }
    }

    if (kAPP_WakeupSourceLpit == g_wakeupSource)
    {
        /* Wakeup source is LPIT, user should input wakeup timeout value. */
        s_wakeupTimeout = APP_GetWakeupTimeout();
        PRINTF("Will wakeup in %d seconds.\r\n", s_wakeupTimeout);
    }
    else if (kAPP_WakeupSourcePin == g_wakeupSource)
    {
        PRINTF("Press %s to wake up.\r\n", APP_WAKEUP_BUTTON_NAME);
    }
    else if (kAPP_WakeupSourcePad == g_wakeupSource)
    {
        PRINTF("Input a char to wake up.\r\n");
    }
    else
    {
        /* All cases handled. */
#if (defined(APP_USE_CAN_AS_WAKEUP) && APP_USE_CAN_AS_WAKEUP)
        PRINTF("Send a CAN message to %s to wake up.\r\n", APP_WAKEUP_CAN_NAME);
#endif /* APP_USE_CAN_AS_WAKEUP */
    }
}

static void APP_SetWakeupConfig(lpm_power_mode_t targetMode)
{
    if (kAPP_WakeupSourceLpit == g_wakeupSource)
    {
        APP_InitLpitWakeupSrc();
        /* Set LPIT timeout value. */
        LPIT_SetTimerPeriod(APP_LPIT_BASE, kLPIT_Chnl_0, MSEC_TO_COUNT(1000U * s_wakeupTimeout, LPIT_SOURCECLOCK));
        LPIT_StartTimer(APP_LPIT_BASE, kLPIT_Chnl_0);
        /* Enable timer interrupts for channel 0 */
        LPIT_EnableInterrupts(APP_LPIT_BASE, kLPIT_Channel0TimerInterruptEnable);
    }
    else if (kAPP_WakeupSourcePin == g_wakeupSource)
    {
        /* Enable Button SC Event. */
        SCEvent_Config(kSCEvent_Button, true, 0);
    }
    else if (kAPP_WakeupSourcePad == g_wakeupSource)
    {
        /* Enable PAD's interrupt in IOMUX. */
        sc_pad_set_wakeup(ipc, APP_WAKEUP_PAD, SC_PAD_WAKEUP_LOW_LVL);
        /* Store the PAD wake up control.
         * The PAD wake up control will be set to SC_PAD_WAKEUP_OFF by SCFW after the corresponding PAD IRQ handled.
         * So, the PAD triggering interrupt can be determined by the wake up control change.
         */
        sc_pad_get_wakeup(ipc, APP_WAKEUP_PAD, &s_uartWakeup);
        /* Enable Pad SC Event. */
        SCEvent_Config(kSCEvent_Pad, true, 0);
    }
    else
    {
#if (defined(APP_USE_CAN_AS_WAKEUP) && APP_USE_CAN_AS_WAKEUP)
        APP_ConfigCanWakeupSrc();
#endif /* APP_USE_CAN_AS_WAKEUP */
    }
}

static void APP_ShowPowerMode(asmc_power_state_t powerMode)
{
    switch (powerMode)
    {
        case kASMC_PowerStateRun:
            PRINTF("    Power mode: RUN\r\n");
            break;
        case kASMC_PowerStateVlpr:
            PRINTF("    Power mode: VLPR\r\n");
            break;
        default:
            PRINTF("    Power mode wrong\r\n");
            break;
    }
}

/*!
 * @brief Power mode switch.
 */
static status_t APP_PowerModeSwitch(asmc_power_state_t curPowerState, lpm_power_mode_t targetPowerMode)
{
    status_t status = kStatus_Success;

    switch (targetPowerMode)
    {
        case LPM_PowerModeVlpr:
            status = ASMC_SetPowerModeVlpr(BBS_SIM);
            while (kASMC_PowerStateVlpr != ASMC_GetPowerModeState(BBS_SIM))
            {
            }
            /* The CPU clock will changed from PLL to OSC 24M by SCFW when entering VLPR. And recover to the PLL once
             * change from VLPR to RUN. */
            break;

        case LPM_PowerModeRun:
            status = ASMC_SetPowerModeRun(BBS_SIM);
            while (kASMC_PowerStateRun != ASMC_GetPowerModeState(BBS_SIM))
            {
            }
            break;

        default:
            PRINTF("Wrong value");
            break;
    }

    if (status != kStatus_Success)
    {
        PRINTF("!!!! Power switch failed !!!!!\r\n");
    }

    return status;
}

static void APP_SetPowerMode(asmc_power_state_t powerMode)
{
    switch (powerMode)
    {
        case kASMC_PowerStateRun:
            LPM_SetPowerMode(LPM_PowerModeRun);
            break;
        case kASMC_PowerStateVlpr:
            LPM_SetPowerMode(LPM_PowerModeVlpr);
            break;
        default:
            break;
    }
}

/*!
 * @brief Called in PowerModeSwitchTask
 */
static bool APP_LpmListener(lpm_power_mode_t curMode, lpm_power_mode_t newMode, void *data)
{
    PRINTF("WorkingTask %d: Transfer from %s to %s\r\n", (uint32_t)data, s_modeNames[curMode], s_modeNames[newMode]);

    /* Do necessary preparation for this mode change */

    return true; /* allow this switch */
}

void APP_SCEvent_ButtonHandler(uint32_t status, void *data)
{
    /*Disable Button SC Event avoid accident wakeup. */
    SCEvent_Config(kSCEvent_Button, false, 0);
    /* This function is execuated in task context. */
    xSemaphoreGive(s_wakeupSig);
    portYIELD();
}

void APP_SCEvent_PadHandler(uint32_t status, void *data)
{
    sc_pad_wakeup_t wakeupCtrl;
    sc_pad_get_wakeup(ipc, APP_WAKEUP_PAD, &wakeupCtrl);
    /* If the APP_WAKEUP_PAD IRQ handled by SCFW, the wakeup control will be changed to SC_PAD_WAKEUP_OFF. */
    if (wakeupCtrl != s_uartWakeup)
    {
        /* Disable SCU PAD event interrupt for M4. */
        SCEvent_Config(kSCEvent_Pad, false, 0);

        xSemaphoreGive(s_wakeupSig);
        portYIELD();
    }
}

void APP_SCEvent_PtRebootHandler(uint32_t status, void *data)
{
    /* Handle SRTM Peer Core Reset.*/
    if (status & (0x1U << a_pt))
    {
        APP_PeerCoreRebootHandler();
    }
}

/*!
 * @brief simulating working task.
 */
static void WorkingTask(void *pvParameters)
{
    LPM_RegisterPowerListener(APP_LpmListener, pvParameters);

    for (;;)
    {
        /* Use App task logic to replace vTaskDelay */
        PRINTF("\r\nTask %d is working now\r\n", (uint32_t)pvParameters);
        vTaskDelay(portMAX_DELAY);
    }
}
status_t APP_SemWait(void *sem, uint32_t ticks)
{
    if (xSemaphoreTake(sem, ticks) == pdFALSE)
    {
        return kStatus_Timeout;
    }

    return kStatus_Success;
}

void APP_SemPost(void *sem)
{
    portBASE_TYPE taskToWake = pdFALSE;

    if (__get_IPSR())
    {
        if (xSemaphoreGiveFromISR(sem, &taskToWake) == pdPASS)
        {
            portYIELD_FROM_ISR(taskToWake);
        }
    }
    else
    {
        xSemaphoreGive(sem);
    }
}

/*!
 * @brief SC Event handle task.
 */
static void SCEventTask(void *pvParameters)
{
    SemaphoreHandle_t eventSem;
    eventSem = xSemaphoreCreateBinary();
    assert(eventSem);

    /* Wait and process any SC Event. */
    while (1)
    {
        if (SCEvent_WaitEvent(APP_SemWait, APP_SemPost, eventSem, portMAX_DELAY) == kStatus_Success)
        {
            SCEvent_Process();
        }
    }
}

/*!
 * @brief Power Mode Switch task.
 */
static void PowerModeSwitchTask(void *pvParameters)
{
    status_t status;
    asmc_power_state_t curPowerState;
    lpm_power_mode_t targetPowerMode;
    uint32_t resetSrc;
    uint32_t freq = 0;
    uint8_t ch;

    const char *errorMsg;

#if (defined(APP_USE_CAN_AS_WAKEUP) && APP_USE_CAN_AS_WAKEUP)
    APP_InitCANEventTask();
#endif /* APP_USE_CAN_AS_WAKEUP */

    resetSrc = ASMC_GetSystemResetStatusFlags(BBS_SIM);
    PRINTF("\r\nMCU wakeup source 0x%x...\r\n", resetSrc);

    SCEvent_RegisterEventHandler(kSCEvent_Button, APP_SCEvent_ButtonHandler, NULL);
    SCEvent_RegisterEventHandler(kSCEvent_Pad, APP_SCEvent_PadHandler, NULL);

    while (1)
    {
        freq = CLOCK_GetFreq(kCLOCK_CoreSysClk);
        PRINTF("\r\n####################  Power Mode Switch Task ####################\n\r\n");
        PRINTF("    Build Time: %s--%s \r\n", __DATE__, __TIME__);
        PRINTF("    Core Clock: %dHz \r\n", freq);
        curPowerState = ASMC_GetPowerModeState(BBS_SIM);
        APP_ShowPowerMode(curPowerState);
        PRINTF("\r\nSelect the desired operation \n\r\n");
        PRINTF("Press  %c for enter: RUN      - Normal RUN mode\r\n", kAPP_PowerModeRun);
        PRINTF("Press  %c for enter: WAIT     - Wait mode\r\n", kAPP_PowerModeWait);
        PRINTF("Press  %c for enter: STOP     - Stop mode\r\n", kAPP_PowerModeStop);
        PRINTF("Press  %c for enter: VLPR     - Very Low Power Run mode\r\n", kAPP_PowerModeVlpr);
        PRINTF("Press  %c for enter: VLPW     - Very Low Power Wait mode\r\n", kAPP_PowerModeVlpw);
        PRINTF("Press  %c for enter: VLPS     - Very Low Power Stop mode\r\n", kAPP_PowerModeVlps);
        PRINTF("Press  %c for enter: LLS      - Low Leakage Stop mode\r\n", kAPP_PowerModeLls);
        PRINTF("Press  %c for enter: VLLS     - Very Low Leakage Stop mode\r\n", kAPP_PowerModeVlls);
        PRINTF("Press  R for using WDOG trigger M4 partition reset.\r\n");
        PRINTF("\r\nWaiting for power mode select..\r\n\r\n");

        /* Wait for user response */
        ch = GETCHAR();

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }
        targetPowerMode = (lpm_power_mode_t)(ch - 'A');

        if (targetPowerMode <= LPM_PowerModeVlls)
        {
            /* If could not set the target power mode, loop continue. */
            if (!LPM_IsTargetModeValid(targetPowerMode, &errorMsg))
            {
                PRINTF(errorMsg);
                continue;
            }
            else if (!LPM_SetPowerMode(targetPowerMode))
            {
                PRINTF("Some task doesn't allow to enter mode %s\r\n", s_modeNames[targetPowerMode]);
            }
            else if ((LPM_PowerModeRun == targetPowerMode) || (LPM_PowerModeVlpr == targetPowerMode))
            {
                /* If target mode is RUN/VLPR, switch directly. */
                APP_PowerPreSwitchHook(curPowerState, targetPowerMode);
                status = APP_PowerModeSwitch(curPowerState, targetPowerMode);
                APP_PowerPostSwitchHook(curPowerState, targetPowerMode, status == kStatus_Success);
            }
            else /* Idle task will handle the low power state. */
            {
                APP_GetWakeupConfig(targetPowerMode);
                APP_SetWakeupConfig(targetPowerMode);
                xSemaphoreTake(s_wakeupSig, portMAX_DELAY);

                /* Need to reset power mode to avoid unintentional WFI. */
                curPowerState = ASMC_GetPowerModeState(BBS_SIM);
                APP_SetPowerMode(curPowerState);
            }

            PRINTF("\r\nNext loop\r\n");
        }
        else if ('R' == ch)
        {
            APP_ConfigWDOG32Reset(WDOG_TIMEOUT);
            PRINTF("Wait a while to reboot\r\n");
            while (1) /* Wait for reboot */
            {
            }
        }
        else
        {
            PRINTF("Invalid command %c[0x%x]\r\n", ch, ch);
        }
    } /* while(1)*/
}

/*! @brief Main function */
int main(void)
{
    ipc = BOARD_InitRpc();

    BOARD_InitPins(ipc);
    BOARD_BootClockRUN();
    BOARD_InitMemory();
    APP_InitDebugConsole();

    /* Power on Peripherals. */
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_M4_1_PIT, SC_PM_PW_MODE_ON));
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_CAN_0, SC_PM_PW_MODE_ON));
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_IRQSTR_M4_1, SC_PM_PW_MODE_ON));
    /* Power up IO Expander I2C. */
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_M4_1_I2C, SC_PM_PW_MODE_ON));
    /* Power up the MU used for RPMSG */
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_MU_6B, SC_PM_PW_MODE_ON));
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_MU_7B, SC_PM_PW_MODE_ON));

    /* Set peripheral's clock. */
    if (CLOCK_SetIpFreq(kCLOCK_M4_1_Lpit, SC_24MHZ) == 0)
    {
        PRINTF("Error: Failed to set LPIT frequency\r\n");
    }
    if (CLOCK_SetIpFreq(kCLOCK_M4_1_Lpi2c, SC_66MHZ) == 0)
    {
        PRINTF("Error: Failed to set LPI2C frequency\r\n");
    }
    /* Set Peripheral clock frequency. */
    if (CLOCK_SetIpFreq(kCLOCK_DMA_Can0, SC_24MHZ) == 0)
    {
        PRINTF("Error: Failed to set FLEXCAN frequency\r\n");
    }

    IRQSTEER_Init(IRQSTEER);

    /* Release I2C bus in case I2C slave hangs. */
    BOARD_GPIO_ConfigurePins(ipc);
    if (sc_pm_set_resource_power_mode(ipc, SC_R_M4_1_RGPIO, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to power on SC_R_M4_1_RGPIO\r\n");
    }
    BOARD_I2C_ReleaseBus();
    if (sc_pm_set_resource_power_mode(ipc, SC_R_M4_1_RGPIO, SC_PM_PW_MODE_OFF) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to power off SC_R_M4_1_RGPIO\r\n");
    }

    /* Configure CAN I/O Expander */
    BOARD_I2C_ConfigurePins(ipc);
    BOARD_ConfigureIOExpander();

    ASMC_SetPowerModeProtection(BBS_SIM, kASMC_AllowPowerModeAll);

    /* Config system interface HPM, LPM */
    RTN_ERR(sc_pm_req_sys_if_power_mode(ipc, SC_R_M4_1_PID0, SC_PM_SYS_IF_DDR, SC_PM_PW_MODE_ON, SC_PM_PW_MODE_OFF));
    RTN_ERR(sc_pm_req_sys_if_power_mode(ipc, SC_R_M4_1_PID0, SC_PM_SYS_IF_OCMEM, SC_PM_PW_MODE_ON, SC_PM_PW_MODE_OFF));
    RTN_ERR(sc_pm_req_sys_if_power_mode(ipc, SC_R_M4_1_PID0, SC_PM_SYS_IF_MU, SC_PM_PW_MODE_OFF, SC_PM_PW_MODE_OFF));
    RTN_ERR(sc_pm_req_sys_if_power_mode(ipc, SC_R_M4_1_PID0, SC_PM_SYS_IF_INTERCONNECT, SC_PM_PW_MODE_ON,
                                        SC_PM_PW_MODE_OFF));

    APP_SRTM_Init();
    APP_SRTM_StartCommunication();

    LPM_Init();
    SCEvent_Init(APP_SCEVENT_IRQ_PRIO);
    SCEvent_RegisterEventHandler(kSCEvent_Reboot, APP_SCEvent_PtRebootHandler, NULL);

    /* Get the partition ID of A core(SRTM peer core). */
    RTN_ERR(sc_rm_get_resource_owner(ipc, SC_R_A53, &a_pt));
    /* Enable A core partition boot SCU event IRQ. */
    SCEvent_Config(kSCEvent_Reboot, true, a_pt);

    s_wakeupSig = xSemaphoreCreateBinary();

    xTaskCreate(PowerModeSwitchTask, "Main Task", 512U, NULL, tskIDLE_PRIORITY + 1U, NULL);
    xTaskCreate(WorkingTask, "Working Task", configMINIMAL_STACK_SIZE, (void *)1, tskIDLE_PRIORITY + 2U, NULL);
    /* Create SC Event handle task with higher priority to make sure the event handled as soon as possible. */
    xTaskCreate(SCEventTask, "SCEvent Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3U, NULL);

    /* Start FreeRTOS scheduler. */
    vTaskStartScheduler();

    /* Application should never reach this point. */
    for (;;)
    {
    }
}
