/*
 * Copyright 2017-2019 NXP
 * All rights reserved.
 *
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
#include "fsl_flexcan.h"
#include "fsl_irqsteer.h"
#include "fsl_lpit.h"
#include "fsl_mu.h"
#include "fsl_wdog32.h"
#include "lpm.h"
#include "power_mode_switch.h"
#include "irq_api.h"

#include "misc/misc_api.h"
#include "svc/pad/pad_api.h"
#include "imx8qx_pads.h"
#include "app_srtm.h"
#include "fsl_lpi2c.h"
#include "fsl_lpuart.h"
#include "fsl_gpio.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* LPIT related macro*/
#define APP_LPIT_BASE       CM4__LPIT
#define APP_LPIT_IRQn       M4_LPIT_IRQn
#define APP_LPIT_IRQHandler M4_LPIT_IRQHandler
#define LPIT_SOURCECLOCK    CLOCK_GetIpFreq(kCLOCK_M4_0_Lpit)
#define APP_LPIT_IRQ_PRIO   (5U)

/*
 * ADMA_I2C1 is used to control PCA9646, PCA9557 to handle the RST pin for PCA6416
 * M4_I2C is used to control PCA6416 to handle the RST and STB pin for CAN XCVR TJA1043
 */
#define APP_LPI2C_BAUDRATE (400000) /*in i2c example it is 100000*/
#define APP_LPI2C_A        ADMA__LPI2C1
#define APP_LPI2C_B        CM4__LPI2C

/*PCA6416 I2C Register Map*/
#define PCA6416_REG_INPUT_PORT_0              (0x0)
#define PCA6416_REG_INPUT_PORT_1              (0x1)
#define PCA6416_REG_OUTPUT_PORT_0             (0x2)
#define PCA6416_REG_OUTPUT_PORT_1             (0x3)
#define PCA6416_REG_POLARITY_INVERSION_PORT_0 (0x4)
#define PCA6416_REG_POLARITY_INVERSION_PORT_1 (0x5)
#define PCA6416_REG_CONFIGURATION_PORT_0      (0x6)
#define PCA6416_REG_CONFIGURATION_PORT_1      (0x7)

/*PCA9557 I2C Register Map*/
#define PCA9557_REG_INTPUT_PORT        (0x00)
#define PCA9557_REG_OUTPUT_PORT        (0x01)
#define PCA9557_REG_POLARITY_INVERSION (0x02)
#define PCA9557_REG_CONFIGURATION      (0x03)

/*Board I2C Addresses*/
#define APP_I2C_EXPANSION_CAN_ADDR (0x20)
#define APP_I2C_EXPANSION_A_ADDR   (0x1A)
#define APP_I2C_SWITCH_ADDR        (0x71)

/*GPIO used*/
#define PCA9557_RST_GPIO LSIO__GPIO1 /*SPI2_SDO, GPIO1, IO1, ALT4*/
#define PCA9557_RST_PIN  1

#define BOARD_RESET_DELAY_CYCLE 1500000

/* FlexCAN */
#define APP_WAKEUP_CAN_NAME    "FlexCAN0"
#define APP_WAKEUP_BUTTON_NAME "SW3 PWR ON"
#define APP_WAKEUP_PAD_NAME    "UART RX Pad"
#define APP_WAKEUP_PAD         SC_P_ADC_IN2

#define APP_CAN                ADMA__CAN0
#define APP_CAN_CLK_FREQ       CLOCK_GetIpFreq(kCLOCK_DMA_Can0)
#define APP_CAN_IRQn           ADMA_FLEXCAN0_INT_IRQn
#define APP_CAN_IRQ_PRIO       (4U)
#define EXAMPLE_CAN_CLK_SOURCE (kFLEXCAN_ClkSrc0)

#define SET_CAN_QUANTUM 1
#define PSEG1           3
#define PSEG2           2
#define PROPSEG         3

#define RTN_ERR(X)                        \
    if ((X) != SC_ERR_NONE)               \
    {                                     \
        assert("Error in SCFW API call"); \
    }

static const char *s_modeNames[] = {"RUN", "WAIT", "STOP", "VLPR", "VLPW", "VLPS", "LLS", "VLLS"};
/* FlexCAN message buffer */
#define RX_MESSAGE_BUFFER_NUM (9)
#define TX_MESSAGE_BUFFER_NUM (8)

/* Get the NVIC IRQn of given IRQSTEER IRQn */
#define GET_IRQSTEER_MASTER_IRQn(IRQn) \
    (IRQn_Type)(IRQSTEER_0_IRQn + (IRQn - FSL_FEATURE_IRQSTEER_IRQ_START_INDEX) / 64U)

/*
 * We use 32KHz Clk divided by 256 as WDOG Clock Source
 */
#define WDOG_TIMEOUT_1S (32768 / 256)
#define WDOG_TIMEOUT    (5 * WDOG_TIMEOUT_1S)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_InitDebugConsole(void);
/* Hook function called before power mode switch. */
void APP_PowerPreSwitchHook(asmc_power_state_t originPowerState, lpm_power_mode_t targetMode);
/* Hook function called after power mode switch. */
void APP_PowerPostSwitchHook(asmc_power_state_t originPowerState, lpm_power_mode_t targetMode, bool result);
/* function that prepare/unperpare Flexcan power and clock*/
void Flexcan_Set_Power(bool on);
/* function that prepare/unperpare LPIT power and clock*/
void LPIT_Set_Power(bool on);
/* FreeRTOS implemented Malloc failed hook. */
extern void vApplicationMallocFailedHook(void);

/*
 * Initialize LPIT.
 */
/*******************************************************************************
 * Variables
 ******************************************************************************/
static sc_ipc_t ipc;
static uint8_t s_wakeupTimeout;         /* Wakeup timeout. (Unit: Second) */
static SemaphoreHandle_t s_wakeupSig;   /* Wakeup signal                  */
static SemaphoreHandle_t s_rxFinishSig; /* CAN receive finished signal    */

app_wakeup_source_t g_wakeupSource; /* Wakeup source.                 */
flexcan_handle_t flexcanHandle;
flexcan_mb_transfer_t txXfer, rxXfer;
flexcan_frame_t frame;
uint32_t txIdentifier = 0x123;
uint32_t rxIdentifier = 0x321;

static sc_pad_t wakeup_pad; /*used to retrive which pad wake up the system*/
extern bool g_dsmOK;
/*******************************************************************************
 * Code
 ******************************************************************************/

static void reset_delay(uint32_t delay_cycle)
{
    uint32_t i = 0;
    while (i < delay_cycle)
    {
        __ASM("nop");
        i++;
    }
}

static void BOARD_EnableI2C(LPI2C_Type *base, uint32_t baudrate, uint32_t sourceClock_Hz)
{
    lpi2c_master_config_t masterConfig;
    LPI2C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Hz = baudrate;
    LPI2C_MasterInit(base, &masterConfig, sourceClock_Hz);
}

static void BOARD_ResetSwitch()
{
    /*
     * Remove Leave to SCFW
     */
}

static void BOARD_ConfigureIOExpander(void)
{
    sc_pm_reset_reason_t resetReason;
    uint8_t txBuffer[2] = {0};

    /*
     * Determine the Reset Reason
     */
    RTN_ERR(sc_pm_reset_reason(ipc, &resetReason));

    /*
     * I2C Initialization
     *  APP_LPI2C_A controls PCA9646, then PCA9557
     *  APP_LPI2C_B controls PCA6416
     */
    BOARD_EnableI2C(APP_LPI2C_A, APP_LPI2C_BAUDRATE, SC_133MHZ);
    BOARD_EnableI2C(APP_LPI2C_B, APP_LPI2C_BAUDRATE, SC_133MHZ);

    if (resetReason == SC_PM_RESET_REASON_POR)
    {
        /*
         * GPIO Reset Opeartion
         */
        BOARD_ResetSwitch();

        /*
         * Open Channel 3 on PCA9646
         */
        txBuffer[0] = 0x8;
        BOARD_LPI2C_SendWithoutSubAddr(APP_LPI2C_A, APP_LPI2C_BAUDRATE, APP_I2C_SWITCH_ADDR, txBuffer, 1, false);
        /*
         * Opereate on PCA9557 IO03 to reset PCA6416
         *  - Set IO03 as output
         *  - Deassert IO03 to Reset
         *  - Assert IO03 to stop Reset
         */
        txBuffer[0] = 0;
        BOARD_LPI2C_Send(APP_LPI2C_A, APP_I2C_EXPANSION_A_ADDR, PCA9557_REG_CONFIGURATION, 1, txBuffer, 1);
        txBuffer[0] = 0;
        BOARD_LPI2C_Send(APP_LPI2C_A, APP_I2C_EXPANSION_A_ADDR, PCA9557_REG_OUTPUT_PORT, 1, txBuffer, 1);
        reset_delay(BOARD_RESET_DELAY_CYCLE);
        txBuffer[1] = 8;
        BOARD_LPI2C_Send(APP_LPI2C_A, APP_I2C_EXPANSION_A_ADDR, PCA9557_REG_OUTPUT_PORT, 1, txBuffer, 1);

        /*
         * Opereate on PCA6416 to Enable TJA1043
         */
        txBuffer[0] = 0;
        BOARD_LPI2C_Send(APP_LPI2C_B, APP_I2C_EXPANSION_CAN_ADDR, PCA6416_REG_CONFIGURATION_PORT_0, 1, txBuffer, 1);
        txBuffer[0] = 0;
        BOARD_LPI2C_Send(APP_LPI2C_B, APP_I2C_EXPANSION_CAN_ADDR, PCA6416_REG_OUTPUT_PORT_0, 1, txBuffer, 1);
        reset_delay(BOARD_RESET_DELAY_CYCLE);
        txBuffer[0] = 0x28; /*P0_3 for EN, P0_5 for STANDBY*/
        BOARD_LPI2C_Send(APP_LPI2C_B, APP_I2C_EXPANSION_CAN_ADDR, PCA6416_REG_OUTPUT_PORT_0, 1, txBuffer, 1);
    }
    else if (resetReason == SC_PM_RESET_REASON_WDOG)
    {
        PRINTF("Partiton Reset\r\n");
    }
    else
    {
        PRINTF("Unexpected Reset\r\n");
    }
}

void Flexcan_Set_Power(bool on)
{
    if (on)
    {
        /* Power on Peripherals. */
        RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_CAN_0, SC_PM_PW_MODE_ON));

        /* Set Peripheral clock frequency. */
        if (CLOCK_SetIpFreq(kCLOCK_DMA_Can0, SC_24MHZ) == 0)
        {
            PRINTF("Error: Failed to set FLEXCAN frequency\r\n");
        }

        RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_IRQSTR_M4_0, SC_PM_PW_MODE_ON));

        /*
         * Leave CAN working in LP
         */
        RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_CAN_0, SC_PM_PW_MODE_LP));
    }
    else
    {
        RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_CAN_0, SC_PM_PW_MODE_OFF));
        RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_IRQSTR_M4_0, SC_PM_PW_MODE_STBY));
    }
}

void LPIT_Set_Power(bool on)
{
    if (on)
    {
        /* Set peripheral's clock. */
        if (CLOCK_SetIpFreq(kCLOCK_M4_0_Lpit, SC_24MHZ) == 0)
        {
            PRINTF("Error: Failed to set LPIT frequency\r\n");
        }

        RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_M4_0_PIT, SC_PM_PW_MODE_LP));
        RTN_ERR(sc_pm_clock_enable(ipc, SC_R_M4_0_PIT, SC_PM_CLK_PER, true, false));
    }
    else
    {
        RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_M4_0_PIT, SC_PM_PW_MODE_OFF));
    }
}

static void APP_Resume(lpm_power_mode_t targetMode, bool result)
{
    sc_pm_clock_rate_t src_rate = SC_133MHZ;

    if (result && ((LPM_PowerModeLls == targetMode) || (LPM_PowerModeVlls == targetMode)))
    {
        BOARD_InitMemory();
    }

    /*
     * Restore power to all wakeup resources
     */
    LPIT_Set_Power(true);
    Flexcan_Set_Power(true);

    /*
     * APP Level Resume Operation
     */
    /*
     * Re-enable I2C
     */
    /* M4_0_I2C */
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_M4_0_I2C, SC_PM_PW_MODE_ON));

    if (sc_pm_set_clock_rate(ipc, SC_R_M4_0_I2C, SC_PM_CLK_PER, &src_rate) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to set SC_R_M4_0_I2C clock rate\r\n");
    }

    if (sc_pm_clock_enable(ipc, SC_R_M4_0_I2C, SC_PM_CLK_PER, true, false) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to enable SC_R_M4_0_I2C clock \r\n");
    }

    /*LSIO_I2C_1*/
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_I2C_1, SC_PM_PW_MODE_ON));

    src_rate = SC_133MHZ;

    if (sc_pm_set_clock_rate(ipc, SC_R_I2C_1, SC_PM_CLK_PER, &src_rate) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to set SC_R_I2C_1 clock rate\r\n");
    }

    if (sc_pm_clock_enable(ipc, SC_R_I2C_1, SC_PM_CLK_PER, true, false) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to enable SC_R_I2C_1 clock \r\n");
    }

    if (result && ((LPM_PowerModeLls == targetMode) || (LPM_PowerModeVlls == targetMode)))
    {
        BOARD_EnableI2C(APP_LPI2C_B, APP_LPI2C_BAUDRATE, SC_133MHZ);
        BOARD_EnableI2C(APP_LPI2C_A, APP_LPI2C_BAUDRATE, SC_133MHZ);
    }

    /* If CAN wake up interrupt pending, clear the CAN IPG_STOP signal. */
    if (kAPP_WakeupSourceCan == g_wakeupSource)
    {
        /*Restore to normal working mode*/
        RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_CAN_0, SC_PM_PW_MODE_ON));
        if (CLOCK_SetIpFreq(kCLOCK_DMA_Can0, SC_24MHZ) == 0)
        {
            PRINTF("Error: Failed to set FLEXCAN frequency\r\n");
        }

        if (FLEXCAN_GetStatusFlags(APP_CAN) & kFLEXCAN_WakeUpIntFlag)
        {
            RTN_ERR(sc_misc_set_control(ipc, SC_R_CAN_0, SC_C_IPG_STOP, 0U));
        }
    }

    /*
     * Restore MU5 and MU8
     */
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_MU_5B, SC_PM_PW_MODE_ON));
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_MU_8B, SC_PM_PW_MODE_ON));
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_IRQSTR_M4_0, SC_PM_PW_MODE_ON));
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_DB, SC_PM_PW_MODE_ON));

#if 0
    /*With SCFW Change on SCF-441, IRQSTEER is contrlled by retension and no longer need to be restored*/
    /*
     * Restore MU8
     */
    MU_Init(LSIO__MU8_B);
    MU_EnableInterrupts(LSIO__MU8_B, (1U << 27U) >> 3);

    /*
     * Restore MU5
     */
    MU_Init(LSIO__MU5_B);
    MU_EnableInterrupts(LSIO__MU5_B, (1U << 27U) >> 1);

    /*
     * Restore IRQSTEER
     */
    IRQSTEER_Init(IRQSTEER);
    IRQSTEER_EnableInterrupt(IRQSTEER, LSIO_MU8_INT_B_IRQn);
    IRQSTEER_EnableInterrupt(IRQSTEER, LSIO_MU5_INT_B_IRQn);
    IRQSTEER_EnableInterrupt(IRQSTEER, LSIO_GPT4_INT_IRQn);
#endif
}

static void APP_Suspend(void)
{
    /*
     * Shutdown unnecessary wakeup resources
     */
    if (kAPP_WakeupSourceLpit == g_wakeupSource)
    {
        Flexcan_Set_Power(false);
    }
    else if ((kAPP_WakeupSourcePin == g_wakeupSource) || (kAPP_WakeupSourcePad == g_wakeupSource))
    {
        LPIT_Set_Power(false);
        Flexcan_Set_Power(false);
    }
    else if (kAPP_WakeupSourceCan == g_wakeupSource)
    {
        LPIT_Set_Power(false);
    }

    /*
     * Since We have I2C enabled, we need to disable them to enter deep sleep
     */
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_M4_0_I2C, SC_PM_PW_MODE_OFF));
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_I2C_1, SC_PM_PW_MODE_OFF));
    /*
     * Set MU5 and MU8 to STBY
     */
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_MU_5B, SC_PM_PW_MODE_OFF));
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_MU_8B, SC_PM_PW_MODE_OFF));
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_IRQSTR_M4_0, SC_PM_PW_MODE_OFF));
    // RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_DB, SC_PM_PW_MODE_OFF));
}

void APP_PowerPreSwitchHook(asmc_power_state_t originPowerState, lpm_power_mode_t targetMode)
{
    /*
     * APP Level Suspend Operation
     */
    if ((LPM_PowerModeRun != targetMode) && (LPM_PowerModeVlpr != targetMode))
    {
        APP_Suspend();
    }

    /* Shutdown CONSOLE UART */
    while (!(kLPUART_TransmissionCompleteFlag & LPUART_GetStatusFlags((LPUART_Type *)BOARD_DEBUG_UART_BASEADDR)))
    {
    }
    DbgConsole_Deinit();
    /* Power off Debug console UART. */
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, BOARD_DEBUG_UART_SC_RSRC, SC_PM_PW_MODE_OFF));
}

void APP_PowerPostSwitchHook(asmc_power_state_t originPowerState, lpm_power_mode_t targetMode, bool result)
{
    /* Resume CONSOLE UART */
    APP_InitDebugConsole();

    if ((LPM_PowerModeRun != targetMode) && (LPM_PowerModeVlpr != targetMode))
    {
        APP_Resume(targetMode, result);
    }

    PRINTF("== Wakeup from mode %s, will enable IRQ ==\r\n", s_modeNames[targetMode]);
    if (result == false)
    {
        PRINTF("[WARNING] Something Prevent M Core to enter desired low power mode\r\n");
    }
}

void APP_InitDebugConsole(void)
{
    uint32_t freq = SC_24MHZ;

    ipc = SystemGetScfwIpcHandle();

    /* Power on Local LPUART for M4. */
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, BOARD_DEBUG_UART_SC_RSRC, SC_PM_PW_MODE_ON));
    /* Enable clock of Local LPUART for M4. */
    CLOCK_EnableClockExt(BOARD_DEBUG_UART_CLKSRC, 0);
    /* Set clock Frequency of Local LPUART for M4. */
    freq = CLOCK_SetIpFreq(BOARD_DEBUG_UART_CLKSRC, freq);

    DbgConsole_Init(BOARD_DEBUG_UART_INSTANCE, BOARD_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, freq);
}

static void APP_ConfigWDOG(uint16_t timeout)
{
    wdog32_config_t config;

    WDOG32_GetDefaultConfig(&config);
    config.testMode = kWDOG32_UserModeEnabled;

    config.clockSource  = kWDOG32_ClockSource1; // 2, internal clock 8MHz, 1, LPO OSC 32KHz
    config.prescaler    = kWDOG32_ClockPrescalerDivide256;
    config.windowValue  = 0U;
    config.timeoutValue = timeout;

    config.enableWindowMode = false;
    config.enableWdog32     = true;

    WDOG32_Init(CM4__WDOG, &config);
}

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
            curPowerState = ASMC_GetPowerModeState(CM4__ASMC);
            APP_PowerPreSwitchHook(curPowerState, targetPowerMode);
            result = LPM_WaitForInterrupt((uint64_t)1000 * xExpectedIdleTime / configTICK_RATE_HZ);
            APP_PowerPostSwitchHook(curPowerState, targetPowerMode, result);
        }
    }

    EnableGlobalIRQ(irqMask);
}

/*!
 * @brief FlexCAN Call Back function
 */
static void flexcan_callback(CAN_Type *base, flexcan_handle_t *handle, status_t status, uint32_t result, void *userData)
{
    switch (status)
    {
        case kStatus_FLEXCAN_RxIdle:
            if (RX_MESSAGE_BUFFER_NUM == result)
            {
                xSemaphoreGiveFromISR(s_rxFinishSig, NULL);
                portYIELD_FROM_ISR(pdTRUE);
            }
            break;

        case kStatus_FLEXCAN_TxIdle:
            if (TX_MESSAGE_BUFFER_NUM == result)
            {
                xSemaphoreGiveFromISR(s_wakeupSig, NULL);
                portYIELD_FROM_ISR(pdTRUE);
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
static app_wakeup_source_t APP_WakeupSourceSelect(void)
{
    uint8_t ch;

    while (1)
    {
        PRINTF("Select the wake up source:\r\n");
        PRINTF("Press T for LPIT - Low Power Timer\r\n");
        PRINTF("Press S for switch/button %s. \r\n", APP_WAKEUP_BUTTON_NAME);
        PRINTF("Press C for CAN - %s. \r\n", APP_WAKEUP_CAN_NAME);
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
    g_wakeupSource = APP_WakeupSourceSelect();

    if ((LPM_PowerModeVlls == targetMode) || (LPM_PowerModeLls == targetMode))
    {
        while (kAPP_WakeupSourceLpit == g_wakeupSource)
        {
            /* In LLS/VLLS mode, the NVIC/AWIC could not work. */
            PRINTF("Not support LPIT wakeup because NVIC/AWIC is disabled in LLS/VLLS mode.\r\n");
            g_wakeupSource = APP_WakeupSourceSelect();
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
        PRINTF("Send a CAN message to %s to wake up.\r\n", APP_WAKEUP_CAN_NAME);
    }
}

static void WakeupSetupFlexcan(void)
{
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
#if defined(EXAMPLE_CAN_CLK_SOURCE)
    flexcanConfig.clkSrc = EXAMPLE_CAN_CLK_SOURCE;
#endif
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
}

static void WakeupSetupLpit(void)
{
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

static void APP_SCButtonHandler(void *pvParameters)
{
    BOARD_EnableSCEvent(SC_EVENT_MASK(kSCEvent_Button), false);
    BOARD_UnregisterEventHandeler(kSCEvent_Button);
    /* This function is execuated in task context. */
    xSemaphoreGive(s_wakeupSig);
    portYIELD();
}

static void APP_SCPadHandler(void *pvParameters)
{
    if (*(sc_pad_t *)pvParameters == APP_WAKEUP_PAD)
    {
        /* Disable SCU PAD event interrupt for M4. */
        BOARD_EnablePadWakeup(APP_WAKEUP_PAD, false, SC_PAD_WAKEUP_LOW_LVL);
        BOARD_EnableSCEvent(SC_EVENT_MASK(kSCEvent_Pad), false);
        BOARD_UnregisterEventHandeler(kSCEvent_Pad);

        /* This function is execuated in task context. */
        xSemaphoreGive(s_wakeupSig);
        portYIELD();
    }
}

static void APP_PeerCoreResetHandler(void *param)
{
    /*
     * In this App simply call Handle function from APP SRTM
     */
    APP_SRTM_HandlePeerReboot();
}

static void APP_SetWakeupConfig()
{
    if (kAPP_WakeupSourceLpit == g_wakeupSource)
    {
        /*LPIT Initialization*/
        WakeupSetupLpit();
        /* Set LPIT timeout value. */
        LPIT_SetTimerPeriod(APP_LPIT_BASE, kLPIT_Chnl_0, MSEC_TO_COUNT(1000U * s_wakeupTimeout, LPIT_SOURCECLOCK));
        LPIT_StartTimer(APP_LPIT_BASE, kLPIT_Chnl_0);
        /* Enable timer interrupts for channel 0 */
        LPIT_EnableInterrupts(APP_LPIT_BASE, kLPIT_Channel0TimerInterruptEnable);
    }
    else if (kAPP_WakeupSourcePin == g_wakeupSource)
    {
        /* The SC IRQ is triggered by SCU, asynchronous with M4, clear any pending SCU BUTTON event before enable SCU
         * BUTTON event interrupt for M4. */
        BOARD_RegisterEventHandler(kSCEvent_Button, APP_SCButtonHandler, NULL);
        BOARD_EnableSCEvent(SC_EVENT_MASK(kSCEvent_Button), true);
    }
    else if (kAPP_WakeupSourcePad == g_wakeupSource)
    {
        /* Enable PAD's interrupt in IOMUX. */
        /* Store the PAD wake up control.
         * The PAD wake up control will be set to SC_PAD_WAKEUP_OFF by SCFW after the corresponding PAD IRQ handled.
         * So, the PAD triggering interrupt can be determined by the wake up control change.
         */
        BOARD_EnablePadWakeup(APP_WAKEUP_PAD, true, SC_PAD_WAKEUP_LOW_LVL);
        BOARD_RegisterEventHandler(kSCEvent_Pad, APP_SCPadHandler, &wakeup_pad);
        BOARD_EnableSCEvent(SC_EVENT_MASK(kSCEvent_Pad), true);
    }
    else if (kAPP_WakeupSourceCan == g_wakeupSource)
    {
        /* Start receive data through Rx Message Buffer. */
        WakeupSetupFlexcan();
        /*
         * CAN STOP sequence:
         * 1.  Config CAN to enable self wake mode(MCR[SLFWAK]),  enable self wake interrupt.
         * 2.  Assert IPG_STOP signal using SCFW API for CAN. Wait the MCR[LPMACK] set, to ensure the CAN going stop.
         * 3.  Wait CAN wakeup and then clear the IPG_STOP signal using SCFW API. (Done in APP_PowerPostSwitchHook())
         */
        FLEXCAN_EnableInterrupts(APP_CAN, kFLEXCAN_WakeUpInterruptEnable);
        ipc = SystemGetScfwIpcHandle();
        RTN_ERR(sc_misc_set_control(ipc, SC_R_CAN_0, SC_C_IPG_STOP, 1U));
        while (!(APP_CAN->MCR & CAN_MCR_LPMACK_MASK))
        {
        }
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
            status = ASMC_SetPowerModeVlpr(CM4__ASMC);
            while (kASMC_PowerStateVlpr != ASMC_GetPowerModeState(CM4__ASMC))
            {
            }
            /* The CPU clock will changed from PLL to OSC 24M by SCFW when entering VLPR. And recover to the PLL once
             * change from VLPR to RUN. */
            break;

        case LPM_PowerModeRun:
            status = ASMC_SetPowerModeRun(CM4__ASMC);
            while (kASMC_PowerStateRun != ASMC_GetPowerModeState(CM4__ASMC))
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
    PRINTF("WorkingTask %s: Transfer from %s to %s\r\n", (char *)data, s_modeNames[curMode], s_modeNames[newMode]);

    /* Do necessary preparation for this mode change */

    return true; /* allow this switch */
}

static lpm_power_mode_t APP_PowerModeSelect(void)
{
    uint8_t ch;
    PRINTF("\r\nSelect the desired operation \n\r\n");
    PRINTF("Press  %c to enter: RUN      - Normal RUN mode\r\n", kAPP_PowerModeRun);
    PRINTF("Press  %c to enter: WAIT     - Wait mode\r\n", kAPP_PowerModeWait);
    PRINTF("Press  %c to enter: STOP     - Stop mode\r\n", kAPP_PowerModeStop);
    PRINTF("Press  %c to enter: VLPR     - Very Low Power Run mode\r\n", kAPP_PowerModeVlpr);
    PRINTF("Press  %c to enter: VLPW     - Very Low Power Wait mode\r\n", kAPP_PowerModeVlpw);
    PRINTF("Press  %c to enter: VLPS     - Very Low Power Stop mode\r\n", kAPP_PowerModeVlps);
    PRINTF("Press  %c to enter: LLS      - Low Leakage Stop mode\r\n", kAPP_PowerModeLls);
    PRINTF("Press  %c to enter: VLLS     - Very Low Leakage Stop mode\r\n", kAPP_PowerModeVlls);
    PRINTF("\r\nPress  R to enter: RESET    - using WDOG trigger M4 partition reset.\r\n");
    PRINTF("\r\nWaiting for user selection..\r\n\r\n");

    /* Wait for user response */
    ch = GETCHAR();

    if ((ch >= 'a') && (ch <= 'z'))
    {
        ch -= 'a' - 'A';
    }

    if (ch == 'R')
    {
        APP_ConfigWDOG(WDOG_TIMEOUT);
        PRINTF("Wait a while to reboot\r\n");
        while (1) /* Wait for reboot */
        {
        }
    }

    return (lpm_power_mode_t)(ch - 'A');
}

/*!
 * @brief CAN Event Handler Task
 */
static void CanEventHandleTask(void *pvParameters)
{
    for (;;)
    {
        /* Wait until Rx receive full. Then send the received message back. */
        xSemaphoreTake(s_rxFinishSig, portMAX_DELAY);

        PRINTF("CAN Rx MB ID: 0x%3x, Rx MB data: 0x%x\r\n", frame.id >> CAN_ID_STD_SHIFT, frame.dataByte0);

        frame.id     = FLEXCAN_ID_STD(txIdentifier);
        txXfer.mbIdx = (uint8_t)TX_MESSAGE_BUFFER_NUM;
        txXfer.frame = &frame;
        FLEXCAN_TransferSendNonBlocking(APP_CAN, &flexcanHandle, &txXfer);
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
        PRINTF("\r\nTask %s is working now\r\n", (char *)pvParameters);
        vTaskDelay(portMAX_DELAY);
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

    const char *errorMsg;

    APP_SRTM_StartCommunication();

    s_rxFinishSig = xSemaphoreCreateBinary();
    xTaskCreate(CanEventHandleTask, "CAN Event Task", 512U, NULL, tskIDLE_PRIORITY + 2U, NULL);

    resetSrc = ASMC_GetSystemResetStatusFlags(CM4__ASMC);
    PRINTF("\r\nMCU wakeup source 0x%x...\r\n", resetSrc);

    while (1)
    {
        freq = CLOCK_GetFreq(kCLOCK_CoreSysClk);
        PRINTF("\r\n####################  Power Mode Switch Task ####################\n\r\n");
        PRINTF("    Build Time: %s--%s \r\n", __DATE__, __TIME__);
        PRINTF("    Core Clock: %dHz \r\n", freq);
        curPowerState = ASMC_GetPowerModeState(CM4__ASMC);
        APP_ShowPowerMode(curPowerState);

        g_wakeupSource  = kAPP_WakeupSourceNone;
        targetPowerMode = APP_PowerModeSelect();

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
                APP_SetWakeupConfig();
                xSemaphoreTake(s_wakeupSig, portMAX_DELAY);

                /* Need to reset power mode to avoid unintentional WFI. */
                curPowerState = ASMC_GetPowerModeState(CM4__ASMC);
                APP_SetPowerMode(curPowerState);
            }

            PRINTF("\r\nNext loop\r\n");
        }
    } /* while(1)*/
}

/*! @brief Main function */
int main(void)
{
    char *taskID = "A";
    sc_pm_clock_rate_t src_rate = SC_133MHZ;
    ipc                         = BOARD_InitRpc();
    BOARD_InitPins(ipc);
    BOARD_BootClockRUN();
    BOARD_InitMemory();
    APP_InitDebugConsole();

    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_IRQSTR_M4_0, SC_PM_PW_MODE_ON));
    /*
     * I2C Initialization
     */
    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_M4_0_I2C, SC_PM_PW_MODE_ON));

    if (sc_pm_set_clock_rate(ipc, SC_R_M4_0_I2C, SC_PM_CLK_PER, &src_rate) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to set SC_R_M4_0_I2C clock rate\r\n");
    }

    if (sc_pm_clock_enable(ipc, SC_R_M4_0_I2C, SC_PM_CLK_PER, true, false) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to enable SC_R_M4_0_I2C clock \r\n");
    }

    RTN_ERR(sc_pm_set_resource_power_mode(ipc, SC_R_I2C_1, SC_PM_PW_MODE_ON));

    src_rate = SC_133MHZ;

    if (sc_pm_set_clock_rate(ipc, SC_R_I2C_1, SC_PM_CLK_PER, &src_rate) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to set SC_R_I2C_1 clock rate\r\n");
    }

    if (sc_pm_clock_enable(ipc, SC_R_I2C_1, SC_PM_CLK_PER, true, false) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to enable SC_R_I2C_1 clock \r\n");
    }

    IRQSTEER_Init(IRQSTEER);

    /*
     * MU Initialization
     */
    if (sc_pm_set_resource_power_mode(ipc, SC_R_MU_8B, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to power on IRQSTEER!\r\n");
    }

    if (sc_pm_set_resource_power_mode(ipc, SC_R_MU_5B, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to power on MU!\r\n");
    }

    IRQSTEER_EnableInterrupt(IRQSTEER, LSIO_MU8_INT_B_IRQn);

    ASMC_SetPowerModeProtection(CM4__ASMC, kASMC_AllowPowerModeAll);

    /*
     * Config system interface HPM, LPM
     * SC_PM_SYS_IF_OCMEM controls OCRAM and SPI NOR, put it to ON for flash target
     */
    RTN_ERR(sc_pm_req_sys_if_power_mode(ipc, SC_R_M4_0_PID0, SC_PM_SYS_IF_DDR, SC_PM_PW_MODE_ON, SC_PM_PW_MODE_OFF));
    RTN_ERR(sc_pm_req_sys_if_power_mode(ipc, SC_R_M4_0_PID0, SC_PM_SYS_IF_OCMEM, SC_PM_PW_MODE_ON, SC_PM_PW_MODE_OFF));
    RTN_ERR(sc_pm_req_sys_if_power_mode(ipc, SC_R_M4_0_PID0, SC_PM_SYS_IF_MU, SC_PM_PW_MODE_OFF, SC_PM_PW_MODE_OFF));
    RTN_ERR(sc_pm_req_sys_if_power_mode(ipc, SC_R_M4_0_PID0, SC_PM_SYS_IF_INTERCONNECT, SC_PM_PW_MODE_ON,
                                        SC_PM_PW_MODE_OFF));

    /*
     * Power on all wakeup sources
     */
    BOARD_ConfigureIOExpander();
    LPIT_Set_Power(true);
    Flexcan_Set_Power(true);

    APP_SRTM_Init();
    LPM_Init();
    /* Register the SC IRQ handle function for SCU broadcast IRQ hanling. */
    BOARD_RegisterEventHandler(kSCEvent_Pad, APP_SCPadHandler, &wakeup_pad);
    BOARD_RegisterEventHandler(kSCEvent_Button, APP_SCButtonHandler, NULL);
    BOARD_RegisterEventHandler(kSCEvent_PeerCoreReboot, APP_PeerCoreResetHandler, NULL);
    BOARD_EnableSCEvent(SC_EVENT_MASK(kSCEvent_PeerCoreReboot), true);
    /*
     * MU IRQ must be enabled, otherwise the M4 cannot actually go into WAIT/STOP
     */
    BOARD_Enable_SCIRQ(true);

    s_wakeupSig = xSemaphoreCreateBinary();

    xTaskCreate(PowerModeSwitchTask, "Main Task", 512U, NULL, tskIDLE_PRIORITY + 1U, NULL);
    xTaskCreate(WorkingTask, "Working Task", configMINIMAL_STACK_SIZE, (void *)taskID, tskIDLE_PRIORITY + 3U, NULL);

    /* Start FreeRTOS scheduler. */
    vTaskStartScheduler();

    /* Application should never reach this point. */
    for (;;)
    {
    }
}
