/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "lpm.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_mu.h"
#include "fsl_cache.h"
#include "mcmgr.h"

#if (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC)
#include "fsl_lpi2c.h"
#include "fsl_pf5020.h"
#endif /* (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC) */

#include "fsl_soc_src.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CPU_NAME "iMXRT1189"

#define APP_WAKEUP_BUTTON_GPIO        BOARD_USER_BUTTON_GPIO
#define APP_WAKEUP_BUTTON_GPIO_PIN    BOARD_USER_BUTTON_GPIO_PIN
#define APP_WAKEUP_BUTTON_IRQ         BOARD_USER_BUTTON_IRQ
#define APP_WAKEUP_BUTTON_IRQ_HANDLER BOARD_USER_BUTTON_IRQ_HANDLER
#define APP_WAKEUP_BUTTON_NAME        BOARD_USER_BUTTON_NAME

#define APP_WAKEUP_BBNSM_IRQ         BBNSM_IRQn
#define APP_WAKEUP_BBNSM_IRQ_HANDLER BBNSM_IRQHandler

/* Address of memory, from which the secondary core will boot */
#define CORE1_BOOT_ADDRESS (void *)0x303C0000
// #define APP_INVALIDATE_CACHE_FOR_SECONDARY_CORE_IMAGE_MEMORY

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
extern uint32_t Image$$CORE1_REGION$$Base;
extern uint32_t Image$$CORE1_REGION$$Length;
#define CORE1_IMAGE_START &Image$$CORE1_REGION$$Base
#elif defined(__ICCARM__)
#pragma section = "__core1_image"
#define CORE1_IMAGE_START __section_begin("__core1_image")
#elif (defined(__GNUC__)) && (!defined(__MCUXPRESSO))
extern const char core1_image_start[];
extern const char *core1_image_end;
extern int core1_image_size;
#define CORE1_IMAGE_START ((void *)core1_image_start)
#define CORE1_IMAGE_SIZE  ((void *)core1_image_size)
#endif


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void GPIO_ClearStopRequest(void);
void GPIO_SetStopRequest(void);
// void APP_BootCore1(void);
#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void);
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/
#if (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC)
lpi2c_master_handle_t g_lpi2cHandle;
extern pf5020_handle_t g_pf5020Handle;
static volatile bool g_lpi2cIntFlag;
#endif /* #if (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC) */

/*******************************************************************************
 * Code
 ******************************************************************************/
#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void)
{
    uint32_t image_size;
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    image_size = (uint32_t)&Image$$CORE1_REGION$$Length;
#elif defined(__ICCARM__)
    image_size = (uint32_t)__section_end("__core1_image") - (uint32_t)__section_begin("__core1_image");
#elif defined(__GNUC__)
    image_size = (uint32_t)core1_image_size;
#endif
    return image_size;
}
#endif

void GPIO_ClearStopRequest(void)
{
    CCM->GPR_SHARED8 &= ~CCM_GPR_SHARED8_m33_gpio1_ipg_stop_MASK;
    CCM->GPR_SHARED12 &= ~CCM_GPR_SHARED12_m7_gpio1_ipg_stop_MASK;
}

void GPIO_SetStopRequest(void)
{
    CCM->GPR_SHARED8 |= CCM_GPR_SHARED8_m33_gpio1_ipg_stop_MASK;
    CCM->GPR_SHARED12 |= CCM_GPR_SHARED12_m7_gpio1_ipg_stop_MASK;
}

void APP_WAKEUP_BUTTON_IRQ_HANDLER(void)
{
    /* Clear GPIO stop request. */
    GPIO_ClearStopRequest();
	
    if ((1U << APP_WAKEUP_BUTTON_GPIO_PIN) &
        RGPIO_GetPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, kRGPIO_InterruptOutput0))
    {
        /* Disable interrupt. */
        RGPIO_SetPinInterruptConfig(APP_WAKEUP_BUTTON_GPIO, APP_WAKEUP_BUTTON_GPIO_PIN, kRGPIO_InterruptOutput0,
                                    kRGPIO_InterruptOrDMADisabled);
        RGPIO_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, kRGPIO_InterruptOutput0,
                                      1U << APP_WAKEUP_BUTTON_GPIO_PIN);
        GPC_DisableWakeupSource(APP_WAKEUP_BUTTON_IRQ);
#ifdef CORE1_GET_INPUT_FROM_CORE0
        MU_TriggerInterrupts(MU_BASE, kMU_GenInt0InterruptTrigger);
#endif
    }
    SDK_ISR_EXIT_BARRIER;
}

void APP_WAKEUP_BBNSM_IRQ_HANDLER(void)
{
    uint32_t temp = 0;

    temp = BBNSM->BBNSM_EVENTS;
    temp &= ~BBNSM_BBNSM_EVENTS_TA_MASK;
    temp |= BBNSM_BBNSM_EVENTS_TA(0b10);
    BBNSM->BBNSM_EVENTS = temp;

    temp = BBNSM->BBNSM_INT_EN;
    temp &= ~BBNSM_BBNSM_INT_EN_TA_INT_EN_MASK;
    temp |= BBNSM_BBNSM_INT_EN_TA_INT_EN(0b01);
    BBNSM->BBNSM_INT_EN = temp;

    temp = BBNSM->BBNSM_CTRL;
    temp &= ~(BBNSM_BBNSM_CTRL_TA_EN_MASK | BBNSM_BBNSM_CTRL_RTC_EN_MASK);
    temp |= BBNSM_BBNSM_CTRL_TA_EN(0b01) | BBNSM_BBNSM_CTRL_RTC_EN(0b01);
    BBNSM->BBNSM_CTRL = temp;

    GPC_DisableWakeupSource(APP_WAKEUP_BBNSM_IRQ);
#ifndef SINGLE_CORE_M33
    MU_TriggerInterrupts(MU_BASE, kMU_GenInt0InterruptTrigger);
#endif
    SDK_ISR_EXIT_BARRIER;
}

#if (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC)
static void lpi2c_master_callback(LPI2C_Type *base, lpi2c_master_handle_t *handle, status_t status, void *userData)
{
    /* Signal transfer success when received success status. */
    if (status == kStatus_Success)
    {
        g_lpi2cIntFlag = true;
    }
}

static status_t I2C_SendFunc(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *txBuff, uint8_t txBuffSize)
{
    status_t reVal = kStatus_Fail;
    lpi2c_master_transfer_t masterXfer;

    /* Prepare transfer structure. */
    masterXfer.slaveAddress   = deviceAddress;
    masterXfer.direction      = kLPI2C_Write;
    masterXfer.subaddress     = subAddress;
    masterXfer.subaddressSize = subAddressSize;
    masterXfer.data           = (void *)txBuff;
    masterXfer.dataSize       = txBuffSize;
    masterXfer.flags          = kLPI2C_TransferDefaultFlag;

    g_lpi2cIntFlag = false;
    reVal          = LPI2C_MasterTransferNonBlocking(DEMO_PF5020_LPI2C, &g_lpi2cHandle, &masterXfer);
    if (kStatus_Success != reVal)
    {
        return reVal;
    }

    while (false == g_lpi2cIntFlag)
    {
    }

    return reVal;
}

static status_t I2C_ReceiveFunc(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize)
{
    status_t reVal = kStatus_Fail;
    lpi2c_master_transfer_t masterXfer;

    /* Prepare transfer structure. */
    masterXfer.slaveAddress   = deviceAddress;
    masterXfer.direction      = kLPI2C_Read;
    masterXfer.subaddress     = subAddress;
    masterXfer.subaddressSize = subAddressSize;
    masterXfer.data           = rxBuff;
    masterXfer.dataSize       = rxBuffSize;
    masterXfer.flags          = kLPI2C_TransferDefaultFlag;

    g_lpi2cIntFlag = false;
    reVal          = LPI2C_MasterTransferNonBlocking(DEMO_PF5020_LPI2C, &g_lpi2cHandle, &masterXfer);
    if (kStatus_Success != reVal)
    {
        return reVal;
    }

    while (false == g_lpi2cIntFlag)
    {
    }

    return reVal;
}

static void APP_InitPMIC(void)
{
    status_t status;
    lpi2c_master_config_t masterConfig;
    pf5020_config_t pmicConfig;
    clock_root_config_t rootCfg = {0};

    rootCfg.mux      = kCLOCK_LPSPI0304_ClockRoot_MuxOscRc24M;
    rootCfg.div      = 2;
    rootCfg.clockOff = false;
    CLOCK_SetRootClock(kCLOCK_Root_Lpi2c0304, &rootCfg);

    LPI2C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Hz = DEMO_PF5020_LPI2C_BAUDRATE;
    /* Initialize the LPI2C master peripheral */
    LPI2C_MasterInit(DEMO_PF5020_LPI2C, &masterConfig, DEMO_PF5020_LPI2C_CLKSRC_FREQ);
    /* Create the LPI2C handle for the non-blocking transfer */
    LPI2C_MasterTransferCreateHandle(DEMO_PF5020_LPI2C, &g_lpi2cHandle, lpi2c_master_callback, NULL);

    PF5020_GetDefaultConfig(&pmicConfig);
    pmicConfig.I2C_SendFunc    = I2C_SendFunc;
    pmicConfig.I2C_ReceiveFunc = I2C_ReceiveFunc;

    PF5020_CreateHandle(&g_pf5020Handle, &pmicConfig);

    /* Set SW1 to 1.1v in run mode, 0.8v in standby mode. */
    status = PF5020_SW1_SetRunStateOption(&g_pf5020Handle, PF5020_SWx_VOLT_1P1V, kPF5020_BuckRegulatorPWMMode);
    if (kStatus_Success != status)
    {
        PRINTF("PMIC run mode voltage set failed\r\n");
        while (1)
            ;
    }

    status = PF5020_SW1_SetStandbyStateOption(&g_pf5020Handle, PF5020_SWx_VOLT_0P8V, kPF5020_BuckRegulatorPWMMode);
    if (kStatus_Success != status)
    {
        PRINTF("PMIC standby mode voltage set failed\r\n");
        while (1)
            ;
    }

    status = PF5020_SWND1_SetOperateMode(&g_pf5020Handle, kPF5020_BuckRegulatorPWMMode, kPF5020_BuckRegulatorPWMMode);
    if (kStatus_Success != status)
    {
        PRINTF("PMIC SWND1 operation mode set failed\r\n");
        while (1)
            ;
    }
}

#endif /* #if (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC) */

static void APP_SetButtonWakeupConfig(void)
{
    PRINTF("Press GPIO button %s to wake up system.\r\n", APP_WAKEUP_BUTTON_NAME);
    RGPIO_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, kRGPIO_InterruptOutput0, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
    /* Enable GPIO pin interrupt */
    RGPIO_SetPinInterruptConfig(APP_WAKEUP_BUTTON_GPIO, APP_WAKEUP_BUTTON_GPIO_PIN, kRGPIO_InterruptOutput0,
                                kRGPIO_InterruptFallingEdge);
    NVIC_ClearPendingIRQ(APP_WAKEUP_BUTTON_IRQ);
    /* Enable the Interrupt */
    EnableIRQ(APP_WAKEUP_BUTTON_IRQ);
    /* Mask all interrupt first */
    GPC_DisableAllWakeupSource(CPU_SLICE);
    /* Enable GPC interrupt */
    GPC_EnableWakeupSource(APP_WAKEUP_BUTTON_IRQ);

    /* Request GPIO to stop, it is configured to exit low power mode via GPIO asynchronous wake up signal. */
    GPIO_SetStopRequest();
}

static void APP_SetRtcWakeupConfig(uint8_t wakeupTimeout)
{
    uint32_t temp = 0;

    temp = BBNSM->BBNSM_CTRL;
    temp &= ~(BBNSM_BBNSM_CTRL_TA_EN_MASK | BBNSM_BBNSM_CTRL_RTC_EN_MASK);
    temp |= BBNSM_BBNSM_CTRL_TA_EN(0b01) | BBNSM_BBNSM_CTRL_RTC_EN(0b01);
    BBNSM->BBNSM_CTRL = temp;

    temp = BBNSM->BBNSM_INT_EN;
    temp &= ~BBNSM_BBNSM_INT_EN_TA_INT_EN_MASK;
    temp |= BBNSM_BBNSM_INT_EN_TA_INT_EN(0b01);
    BBNSM->BBNSM_INT_EN = temp;

    BBNSM->BBNSM_RTC_LS = 0x0;
    BBNSM->BBNSM_RTC_MS = 0x0;
    BBNSM->BBNSM_TA     = wakeupTimeout;

    EnableIRQ(APP_WAKEUP_BBNSM_IRQ);

    temp = BBNSM->BBNSM_CTRL;
    temp &= ~(BBNSM_BBNSM_CTRL_TA_EN_MASK | BBNSM_BBNSM_CTRL_RTC_EN_MASK);
    temp |= BBNSM_BBNSM_CTRL_TA_EN(0b10) | BBNSM_BBNSM_CTRL_RTC_EN(0b10);
    BBNSM->BBNSM_CTRL = temp;

    temp = BBNSM->BBNSM_INT_EN;
    temp &= ~(BBNSM_BBNSM_INT_EN_TA_INT_EN_MASK | BBNSM_BBNSM_INT_EN_RTC_INT_EN_MASK);
    temp |= BBNSM_BBNSM_INT_EN_TA_INT_EN(0b10);
    BBNSM->BBNSM_INT_EN = temp;

    /* Mask all interrupt first */
    GPC_DisableAllWakeupSource(CPU_SLICE);
    GPC_EnableWakeupSource(APP_WAKEUP_BBNSM_IRQ);
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

/* Get wakeup source by user input. */
static app_wakeup_source_t APP_GetWakeupSource(void)
{
    uint8_t ch;

    while (1)
    {
        PRINTF("Select the wake up source:\r\n");
        PRINTF("Press T for Timer\r\n");
        PRINTF("Press S for GPIO button %s. \r\n", APP_WAKEUP_BUTTON_NAME);

        PRINTF("\r\nWaiting for key press..\r\n\r\n");

        ch = GETCHAR();

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }

        if (ch == 'T')
        {
            return kAPP_WakeupSourceTimer;
        }
        else if (ch == 'S')
        {
            return kAPP_WakeupSourcePin;
        }
        else
        {
            PRINTF("Wrong value!\r\n");
        }
    }
}

static void APP_SetWakeupConfig(void)
{
    uint8_t wakeupTimeout;
    /* Get wakeup source by user input. */
    if (kAPP_WakeupSourceTimer == APP_GetWakeupSource())
    {
        /* Wakeup source is timer, user should input wakeup timeout value. */
        wakeupTimeout = APP_GetWakeupTimeout();
        PRINTF("Will wake up in %d seconds.\r\n", wakeupTimeout);
        /* Set timer timeout value. */
        APP_SetRtcWakeupConfig(wakeupTimeout);
    }
    else
    {
        APP_SetButtonWakeupConfig();
    }
}

static void APP_SetBBSMModeWakeupConfig(void)
{
    uint8_t wakeupTimeout;
    /* Get wakeup source by user input. */
    if (kAPP_WakeupSourceTimer == APP_GetWakeupSource())
    {
        /* Wakeup source is timer, user should input wakeup timeout value. */
        wakeupTimeout = APP_GetWakeupTimeout();
        PRINTF("Will wake up in %d seconds.\r\n", wakeupTimeout);
        /* Set timer timeout value. */
        APP_SetRtcWakeupConfig(wakeupTimeout);
    }
    else
    {
        PRINTF("Since GPIO button is not working in BBSM mode, Press WAKEUP button SW4 to wake up system.\r\n");
    }
}

/* Delay for a while to let core1 request power mode transition for the first */
void delay(void)
{
    uint32_t coreClk = CLOCK_GetRootClockFreq(kCLOCK_Root_M33);
    SDK_DelayAtLeastUs(100000U, coreClk);
}

void RunModeSwitch(void)
{
    uint8_t ch, target;

    while (1)
    {
        PRINTF("\r\nRUN mode switch:\r\n");
        PRINTF("Press %c to enter OverDrive RUN\r\n", (uint8_t)'A');
        PRINTF("Press %c to enter Normal RUN\r\n", (uint8_t)'B');
        PRINTF("Press %c to enter UnderDrive RUN\r\n", (uint8_t)'C');
        PRINTF("Press 'Q' to exit\r\n");
        PRINTF("\r\nWaiting for select...\r\n");

        /* Wait for user response */
        ch = GETCHAR();
#ifdef CORE1_GET_INPUT_FROM_CORE0
        /* Send message to another core */
        (void)MCMGR_TriggerEvent(kMCMGR_RemoteApplicationEvent, ch);
#endif

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }
        if (ch == 'Q')
        {
            break;
        }
        target = (ch - 'A');

        if (target < 3)
        {
            RunModeTransition((run_mode_t)target);
            PrintSystemStatus();
        }
    }
}

void CpuModeSwitch(void)
{
    bool sysSleepEn = false;
    uint8_t ch, target;
    gpc_cpu_mode_t cpuMode;

    while (1)
    {
        PRINTF("\r\nCPU mode switch:\r\n");
        PRINTF("Press %c to enter CPU mode: RUN\r\n", (uint8_t)'A' + kGPC_RunMode);
        PRINTF("Press %c to enter CPU mode: WAIT\r\n", (uint8_t)'A' + kGPC_WaitMode);
        PRINTF("Press %c to enter CPU mode: STOP\r\n", (uint8_t)'A' + kGPC_StopMode);
        PRINTF("Press %c to enter CPU mode: SUSPEND\r\n", (uint8_t)'A' + kGPC_SuspendMode);
        PRINTF("Press %c to enter CPU mode: STOP, system sleep\r\n", (uint8_t)'A' + 2 + kGPC_StopMode);
        PRINTF("Press %c to enter CPU mode: SUSPEND, system sleep\r\n", (uint8_t)'A' + 2 + kGPC_SuspendMode);
        PRINTF("Press 'Q' to exit\r\n");
        PRINTF("\r\nWaiting for select...\r\n");

        /* Wait for user response */
        ch = GETCHAR();
#ifdef CORE1_GET_INPUT_FROM_CORE0
        /* Send message to another core */
        (void)MCMGR_TriggerEvent(kMCMGR_RemoteApplicationEvent, ch);
        delay();
#endif

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }
        if (ch == 'Q')
        {
            break;
        }
        target = (ch - 'A');

        if (target < 6)
        {
            if (target > 3)
            {
                sysSleepEn = true;
                target     = target - 2;
            }
            else
            {
                sysSleepEn = false;
            }
            cpuMode = (gpc_cpu_mode_t)target;
            if (cpuMode != kGPC_RunMode)
            {
                APP_SetWakeupConfig();
                PRINTF("Target CPU mode is %s\r\n", GET_CPU_MODE_NAME(cpuMode));
                if (sysSleepEn)
                {
                    PRINTF("System sleep\r\n");
                }
                PRINTF("Go...\r\n");
                CpuModeTransition(cpuMode, sysSleepEn);
                PrintSystemStatus();
            }
            else
            {
                PRINTF("CPU already in RUN mode!\r\n");
            }
        }
    }
}

#ifndef SINGLE_CORE_M33
/*!
 * @brief Function to copy core1 image to execution address.
 */
static void APP_CopyCore1Image(void)
{
#ifdef CORE1_IMAGE_COPY_TO_RAM
    /* Calculate size of the image  - not required on MCUXpresso IDE. MCUXpresso copies the secondary core
       image to the target memory during startup automatically */
    uint32_t core1_image_size = get_core1_image_size();

    PRINTF("Copy Secondary core image to address: 0x%x, size: %d\r\n", CORE1_BOOT_ADDRESS, core1_image_size);

    memcpy((void *)CORE1_BOOT_ADDRESS, (void *)CORE1_IMAGE_START, core1_image_size);

#ifdef APP_INVALIDATE_CACHE_FOR_SECONDARY_CORE_IMAGE_MEMORY
    XCACHE_CleanInvalidateCacheByRange((uint32_t)CORE1_BOOT_ADDRESS, core1_image_size);
#endif
#endif
}

/*!
 * @brief Application-specific implementation of the SystemInitHook() weak function.
 */
void SystemInitHook(void)
{
    /* Initialize MCMGR - low level multicore management library. Call this
       function as close to the reset entry as possible to allow CoreUp event
       triggering. The SystemInitHook() weak function overloading is used in this
       application. */
    (void)MCMGR_EarlyInit();
#ifndef SINGLE_CORE_M33
    Prepare_CM7(0);
#endif
}
#endif

/*!
 * @brief main demo function.
 */
int main(void)
{
    uint8_t ch;

    /* Init board hardware.*/
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Workaround: Disable interrupt which might be enabled by ROM. */
    RGPIO_SetPinInterruptConfig(RGPIO1, 9U, kRGPIO_InterruptOutput0, kRGPIO_InterruptOrDMADisabled);
    NVIC_ClearPendingIRQ(GPIO1_0_IRQn);

/* Macro SINGLE_CORE_M33 is only for single core chip MIMXRT1181/MIMXRT1182 */
#ifdef SINGLE_CORE_M33
    PRINTF("\r\nThis is single core M33.\r\n");
    /* Since core1 is not used and is always in reset status, force core1 requesting standby mode */
    GPC_SYS_SLEEP_CTRL->SS_MISC |= GPC_SYS_SLEEP_CTRL_SS_MISC_FORCE_CPU1_SYS_SLEEP_MASK;
#else
    PRINTF("\r\nThis is core0.\r\n");

    /* Initialize MCMGR, install generic event handlers */
    (void)MCMGR_Init();

    APP_CopyCore1Image();

    /* Boot Secondary core application */
    (void)PRINTF("Starting Secondary core.\r\n");
    (void)MCMGR_StartCore(kMCMGR_Core1, (void *)(char *)CORE1_BOOT_ADDRESS, 2, kMCMGR_Start_Synchronous);
    (void)PRINTF("The secondary core application has been started.\r\n");
#endif

    PRINTF("\r\nCPU wakeup source 0x%x...\r\n", SRC_GENERAL_REG->SRSR);
    PRINTF("\r\n***********************************************************\r\n");
    PRINTF("\tPower Mode Switch Demo for %s\r\n", CPU_NAME);
    PRINTF("***********************************************************\r\n");

    ChipInitConfig();

#ifdef CORE1_GET_INPUT_FROM_CORE0
    PRINTF("\r\nCore0 send message to core1.\r\n");
#endif

#if (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC)
    APP_InitPMIC();
    /* Let PMIC enter standby mode when system sleep. */
    GPC_SS_SystemSleepTriggerPMICStandby(GPC_SYS_SLEEP_CTRL, true);
#endif /* (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC) */

    PrintSystemStatus();

    while (1)
    {
        PRINTF("\r\nPlease select the desired operation:\r\n");
        PRINTF("Press  %c to demonstrate run mode switch.\r\n", (uint8_t)'A');
        PRINTF("Press  %c to demonstrate cpu mode switch.\r\n", (uint8_t)'B');
        PRINTF("Press  %c to enter BBSM mode.\r\n", (uint8_t)'C');
        PRINTF("\r\nWaiting for select...\r\n");

        /* Wait for user response */
        ch = GETCHAR();
#ifdef CORE1_GET_INPUT_FROM_CORE0
        /* Send message to another core */
        (void)MCMGR_TriggerEvent(kMCMGR_RemoteApplicationEvent, ch);
#endif

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }

        switch (ch)
        {
            case 'A':
                RunModeSwitch();
                break;
            case 'B':
                CpuModeSwitch();
                break;
            case 'C':
                APP_SetBBSMModeWakeupConfig();
                PRINTF("Now shutting down the system...\r\n");
                /* Turn off system power. */
                BBNSM->BBNSM_CTRL |= BBNSM_BBNSM_CTRL_TOSP_MASK;
                while (1)
                    ;
                break;
            default:
                break;
        }
    }
}
