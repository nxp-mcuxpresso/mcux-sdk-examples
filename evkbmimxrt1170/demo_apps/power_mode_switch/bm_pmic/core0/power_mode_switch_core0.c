/*
 * Copyright 2020-2021, 2023 NXP
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

#if (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC)
#include "fsl_lpi2c.h"
#include "fsl_pf5020.h"
#endif /* (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC) */

#include "fsl_soc_src.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CPU_NAME "iMXRT1176"

#define APP_WAKEUP_BUTTON_GPIO        BOARD_USER_BUTTON_GPIO
#define APP_WAKEUP_BUTTON_GPIO_PIN    BOARD_USER_BUTTON_GPIO_PIN
#define APP_WAKEUP_BUTTON_IRQ         BOARD_USER_BUTTON_IRQ
#define APP_WAKEUP_BUTTON_IRQ_HANDLER BOARD_USER_BUTTON_IRQ_HANDLER
#define APP_WAKEUP_BUTTON_NAME        BOARD_USER_BUTTON_NAME

#define APP_WAKEUP_SNVS_IRQ         SNVS_HP_NON_TZ_IRQn
#define APP_WAKEUP_SNVS_IRQ_HANDLER SNVS_HP_NON_TZ_IRQHandler

/* Address of memory, from which the secondary core will boot */
#define CORE1_BOOT_ADDRESS 0x20200000

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
extern uint32_t Image$$CORE1_REGION$$Base;
extern uint32_t Image$$CORE1_REGION$$Length;
#define CORE1_IMAGE_START &Image$$CORE1_REGION$$Base
#elif defined(__ICCARM__)
extern unsigned char core1_image_start[];
#define CORE1_IMAGE_START core1_image_start
#elif defined(__GNUC__)
extern const char core1_image_start[];
extern const char *core1_image_end;
extern int core1_image_size;
#define CORE1_IMAGE_START ((void *)core1_image_start)
#define CORE1_IMAGE_SIZE  ((void *)core1_image_size)
#endif

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
extern uint32_t __Vectors[];
#define IMAGE_ENTRY_ADDRESS ((uint32_t)__Vectors)
#elif defined(__MCUXPRESSO)
extern uint32_t __Vectors[];
#define IMAGE_ENTRY_ADDRESS ((uint32_t)__Vectors)
#elif defined(__ICCARM__)
extern uint32_t __VECTOR_TABLE[];
#define IMAGE_ENTRY_ADDRESS ((uint32_t)__VECTOR_TABLE)
#elif defined(__GNUC__)
extern uint32_t __VECTOR_TABLE[];
#define IMAGE_ENTRY_ADDRESS ((uint32_t)__VECTOR_TABLE)
#endif

#if (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC)
#define DEMO_PF5020_LPI2C             LPI2C6
#define DEMO_PF5020_LPI2C_CLKSRC_FREQ (CLOCK_GetFreq(kCLOCK_OscRc16M))
#define DEMO_PF5020_LPI2C_BAUDRATE    1000000U
#endif /* (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC) */

/* Flag indicates Core Boot Up*/
#define BOOT_FLAG 0x01U

/* Channel transmit and receive register */
#define CHN_MU_REG_NUM 0U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_BootCore1(void);
#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void);
#endif
AT_QUICKACCESS_SECTION_CODE(void CLOCK_SetClockRoot(clock_root_t root, const clock_root_config_t *config));
AT_QUICKACCESS_SECTION_CODE(void SwitchFlexspiRootClock(bool pllLdoDisabled));

/*******************************************************************************
 * Variables
 ******************************************************************************/
#if (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC)
lpi2c_master_handle_t g_lpi2cHandle;
extern pf5020_handle_t g_pf5020Handle;
static volatile bool g_lpi2cIntFlag;
const uint8_t sw1Volt[16]                                = PF5020_SW1_OUTPUT_VOLT;
const pf5020_buck_regulator_operate_mode_t swnd1Mode[16] = PF5020_SWND1_OPERATE_MODE;
#endif /* #if (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC) */

/*******************************************************************************
 * Code
 ******************************************************************************/
void APP_BootCore1(void)
{
    IOMUXC_LPSR_GPR->GPR0 = IOMUXC_LPSR_GPR_GPR0_CM4_INIT_VTOR_LOW(CORE1_BOOT_ADDRESS >> 3);
    IOMUXC_LPSR_GPR->GPR1 = IOMUXC_LPSR_GPR_GPR1_CM4_INIT_VTOR_HIGH(CORE1_BOOT_ADDRESS >> 16);

    /* Read back to make sure write takes effect. */
    (void)IOMUXC_LPSR_GPR->GPR0;
    (void)IOMUXC_LPSR_GPR->GPR1;

    /* If CM4 is already running (released by debugger), then reset it.
       If CM4 is not running, release it. */
    if ((SRC->SCR & SRC_SCR_BT_RELEASE_M4_MASK) == 0)
    {
        SRC_ReleaseCoreReset(SRC, kSRC_CM4Core);
    }
    else
    {
        SRC_AssertSliceSoftwareReset(SRC, kSRC_M4CoreSlice);
    }
}

#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void)
{
    uint32_t image_size;
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    image_size = (uint32_t)&Image$$CORE1_REGION$$Length;
#elif defined(__ICCARM__)
#pragma section = "__core1_image"
    image_size = (uint32_t)__section_end("__core1_image") - (uint32_t)&core1_image_start;
#elif defined(__GNUC__)
    image_size = (uint32_t)core1_image_size;
#endif
    return image_size;
}
#endif

void APP_WAKEUP_BUTTON_IRQ_HANDLER(void)
{
    if ((1U << APP_WAKEUP_BUTTON_GPIO_PIN) & GPIO_GetPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO))
    {
        /* Disable interrupt. */
        GPIO_DisableInterrupts(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
        GPIO_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
        GPC_DisableWakeupSource(APP_WAKEUP_BUTTON_IRQ);
#ifdef CORE1_GET_INPUT_FROM_CORE0
        MU_TriggerInterrupts(MU_BASE, kMU_GenInt0InterruptTrigger);
#endif
    }
    SDK_ISR_EXIT_BARRIER;
}

void APP_WAKEUP_SNVS_IRQ_HANDLER(void)
{
    /* Clear SRTC alarm interrupt. */
    SNVS->LPSR |= SNVS_LPSR_LPTA_MASK;
    /* Disable SRTC alarm interrupt */
    SNVS->LPCR &= ~SNVS_LPCR_LPTA_EN_MASK;
    /* Stop SRTC time counter */
    SNVS->LPCR &= ~SNVS_LPCR_SRTC_ENV_MASK;
    GPC_DisableWakeupSource(APP_WAKEUP_SNVS_IRQ);
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
    lpi2c_master_config_t masterConfig;
    pf5020_config_t pmicConfig;
    clock_root_config_t rootCfg = {0};

    rootCfg.mux = kCLOCK_LPI2C6_ClockRoot_MuxOscRc16M;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpi2c6, &rootCfg);

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

    uint8_t preSp;
    uint8_t currentSp;
    uint8_t preSpSw1OutVolt;
    uint8_t curSpSw1OutVolt;
    preSp           = GPC_SP_GetPreviousSetPoint(GPC_SET_POINT_CTRL);
    currentSp       = GPC_SP_GetCurrentSetPoint(GPC_SET_POINT_CTRL);
    preSpSw1OutVolt = sw1Volt[preSp];
    curSpSw1OutVolt = sw1Volt[currentSp];
    if (preSpSw1OutVolt != curSpSw1OutVolt)
    {
        (void)PF5020_SW1_SetRunStateOption(&g_pf5020Handle, curSpSw1OutVolt, kPF5020_BuckRegulatorPWMMode);
    }
}

#endif /* #if (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC) */

static void APP_SetButtonWakeupConfig(void)
{
    PRINTF("Press button %s to wake up system.\r\n", APP_WAKEUP_BUTTON_NAME);
    GPIO_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
    /* Enable GPIO pin interrupt */
    GPIO_EnableInterrupts(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
    NVIC_ClearPendingIRQ(APP_WAKEUP_BUTTON_IRQ);
    /* Enable the Interrupt */
    EnableIRQ(APP_WAKEUP_BUTTON_IRQ);
    /* Mask all interrupt first */
    GPC_DisableAllWakeupSource(GPC_CPU_MODE_CTRL);
    /* Enable GPC interrupt */
    GPC_EnableWakeupSource(APP_WAKEUP_BUTTON_IRQ);
}

static void APP_SetSrtcWakeupConfig(uint8_t wakeupTimeout)
{
    /* Stop SRTC time counter */
    SNVS->LPCR &= ~SNVS_LPCR_SRTC_ENV_MASK;
    while ((SNVS->LPCR & SNVS_LPCR_SRTC_ENV_MASK))
    {
    }
    /* Disable SRTC alarm interrupt */
    SNVS->LPCR &= ~SNVS_LPCR_LPTA_EN_MASK;
    while ((SNVS->LPCR & SNVS_LPCR_LPTA_EN_MASK))
    {
    }

    SNVS->LPSRTCMR = 0x00;
    SNVS->LPSRTCLR = 0x00;
    /* Set alarm in seconds*/
    SNVS->LPTAR = wakeupTimeout;
    EnableIRQ(APP_WAKEUP_SNVS_IRQ);
    /* Enable SRTC time counter and alarm interrupt */
    SNVS->LPCR |= SNVS_LPCR_SRTC_ENV_MASK | SNVS_LPCR_LPTA_EN_MASK;
    while (!(SNVS->LPCR & SNVS_LPCR_LPTA_EN_MASK))
    {
    }

    /* Mask all interrupt first */
    GPC_DisableAllWakeupSource(GPC_CPU_MODE_CTRL);
    GPC_EnableWakeupSource(APP_WAKEUP_SNVS_IRQ);
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
        PRINTF("Press S for switch/button %s. \r\n", APP_WAKEUP_BUTTON_NAME);

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
        APP_SetSrtcWakeupConfig(wakeupTimeout);
    }
    else
    {
        APP_SetButtonWakeupConfig();
    }
}

/*!
 * @brief Configure Root Clock
 *
 * @param root Which root clock node to set, see \ref clock_root_t.
 * @param config root clock config, see \ref clock_root_config_t
 */
void CLOCK_SetClockRoot(clock_root_t root, const clock_root_config_t *config)
{
    assert(config);
    CCM->CLOCK_ROOT[root].CONTROL = CCM_CLOCK_ROOT_CONTROL_MUX(config->mux) |
                                    CCM_CLOCK_ROOT_CONTROL_DIV(config->div - 1) |
                                    (config->clockOff ? CCM_CLOCK_ROOT_CONTROL_OFF(config->clockOff) : 0);
    __DSB();
    __ISB();
#if __CORTEX_M == 4
    (void)CCM->CLOCK_ROOT[root].CONTROL;
#endif
}

void SwitchFlexspiRootClock(bool pllLdoDisabled)
{
    clock_root_config_t rootCfg = {0};

    if (pllLdoDisabled)
    {
        if (CLOCK_GetRootClockMux(kCLOCK_Root_Flexspi1) == kCLOCK_FLEXSPI1_ClockRoot_MuxOscRc16M)
        {
            /* Clock root already mux to OscRc16M, no need to config it again. */
            return;
        }
    }
    else
    {
        if (CLOCK_GetRootClockMux(kCLOCK_Root_Flexspi1) == kCLOCK_FLEXSPI1_ClockRoot_MuxSysPll2Out)
        {
            /* Clock root already mux to SysPll2Out, no need to config it again. */
            return;
        }
    }

#if (__CORTEX_M == 7)
    if (SCB_CCR_IC_Msk == (SCB_CCR_IC_Msk & SCB->CCR))
    {
        SCB_DisableICache();
    }
#endif

#if (defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1))

    /* Enable FLEXSPI module */
    FLEXSPI1->MCR0 &= ~FLEXSPI_MCR0_MDIS_MASK;
    while (!((FLEXSPI1->STS0 & FLEXSPI_STS0_ARBIDLE_MASK) && (FLEXSPI1->STS0 & FLEXSPI_STS0_SEQIDLE_MASK)))
    {
    }
    FLEXSPI1->MCR0 |= FLEXSPI_MCR0_MDIS_MASK;

    CCM->LPCG[kCLOCK_Flexspi1].DOMAINr = 0UL;
    /* Disable clock gate of flexspi. */
    CCM->LPCG[kCLOCK_Flexspi1].DIRECT &= ~CCM_LPCG_DIRECT_ON_MASK;

    __DSB();
    __ISB();

    while (CCM->LPCG[kCLOCK_Flexspi1].STATUS0 & CCM_LPCG_STATUS0_ON_MASK)
    {
    }

#endif

    if (pllLdoDisabled)
    {
        rootCfg.mux = kCLOCK_FLEXSPI1_ClockRoot_MuxOscRc16M;
        rootCfg.div = 1;
        CLOCK_SetClockRoot(kCLOCK_Root_Flexspi1, &rootCfg);
    }
    else
    {
        rootCfg.mux = kCLOCK_FLEXSPI1_ClockRoot_MuxSysPll2Out;
        rootCfg.div = 4;
        CLOCK_SetClockRoot(kCLOCK_Root_Flexspi1, &rootCfg);
    }

#if (defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1))
    /* Enable clock gate of flexspi. */
    CCM->LPCG[kCLOCK_Flexspi1].DIRECT |= CCM_LPCG_DIRECT_ON_MASK;
    CCM->LPCG[kCLOCK_Flexspi1].DOMAINr |= 0x4UL;
    __DSB();
    __ISB();

    while (!(CCM->LPCG[kCLOCK_Flexspi1].STATUS0 & CCM_LPCG_STATUS0_ON_MASK))
    {
    }

    uint32_t status;
    uint32_t lastStatus;
    uint32_t retry;

    /* If serial root clock is >= 100 MHz, DLLEN set to 1, OVRDEN set to 0, then SLVDLYTARGET setting of 0x0 is
     * recommended. */
    FLEXSPI1->DLLCR[0] = 0x1U;

    /* Enable FLEXSPI module */
    FLEXSPI1->MCR0 &= ~FLEXSPI_MCR0_MDIS_MASK;

    FLEXSPI1->MCR0 |= FLEXSPI_MCR0_SWRESET_MASK;
    while (FLEXSPI1->MCR0 & FLEXSPI_MCR0_SWRESET_MASK)
    {
    }

    /* Need to wait DLL locked if DLL enabled */
    if (0U != (FLEXSPI1->DLLCR[0] & FLEXSPI_DLLCR_DLLEN_MASK))
    {
        lastStatus = FLEXSPI1->STS2;
        retry      = 10;
        /* Wait slave delay line locked and slave reference delay line locked. */
        do
        {
            status = FLEXSPI1->STS2;
            if ((status & (FLEXSPI_STS2_AREFLOCK_MASK | FLEXSPI_STS2_ASLVLOCK_MASK)) ==
                (FLEXSPI_STS2_AREFLOCK_MASK | FLEXSPI_STS2_ASLVLOCK_MASK))
            {
                /* Locked */
                retry = 100;
                break;
            }
            else if (status == lastStatus)
            {
                /* Same delay cell number in calibration */
                retry--;
            }
            else
            {
                retry      = 10;
                lastStatus = status;
            }
        } while (retry > 0);
        /* According to ERR011377, need to delay at least 100 NOPs to ensure the DLL is locked. */
        for (; retry > 0U; retry--)
        {
            __NOP();
        }
    }

#if (__CORTEX_M == 7)
    SCB_EnableICache();
#endif
#endif
}

/* Delay for a while to let core1 request power mode transition for the first */
void delay(void)
{
    uint32_t coreClk = CLOCK_GetFreqFromObs(CCM_OBS_M7_CLK_ROOT);
    SDK_DelayAtLeastUs(100000U, coreClk);
}

void CpuModeSwitchInSetPoint(uint8_t setpoint)
{
    bool stbyEn = false;
    uint8_t ch, target;
    gpc_cpu_mode_t cpuMode;

#if (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC)
    (void)PF5020_SW1_SetRunStateOption(&g_pf5020Handle, sw1Volt[setpoint], kPF5020_BuckRegulatorPWMMode);
#endif /* (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC) */

    GPC_CM_RequestRunModeSetPointTransition(GPC_CPU_MODE_CTRL, setpoint);

    while (1)
    {
        PRINTF("\r\nCPU mode switch:\r\n");
        PRINTF("Press %c to enter CPU mode: RUN\r\n", (uint8_t)'A' + kGPC_RunMode);
        PRINTF("Press %c to enter CPU mode: WAIT\r\n", (uint8_t)'A' + kGPC_WaitMode);
        PRINTF("Press %c to enter CPU mode: STOP\r\n", (uint8_t)'A' + kGPC_StopMode);
        PRINTF("Press %c to enter CPU mode: SUSPEND\r\n", (uint8_t)'A' + kGPC_SuspendMode);
        PRINTF("Press %c to enter CPU mode: WAIT, system standby\r\n", (uint8_t)'A' + 3 + kGPC_WaitMode);
        PRINTF("Press %c to enter CPU mode: STOP, system standby\r\n", (uint8_t)'A' + 3 + kGPC_StopMode);
        PRINTF("Press %c to enter CPU mode: SUSPEND, system standby\r\n", (uint8_t)'A' + 3 + kGPC_SuspendMode);
        PRINTF("Press 'Q' to exit\r\n");
        PRINTF("\r\nWaiting for select...\r\n");

        /* Wait for user response */
        ch = GETCHAR();
#ifdef CORE1_GET_INPUT_FROM_CORE0
        /* Send message to another core */
        MU_SendMsg(MU_BASE, CHN_MU_REG_NUM, ch);
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

        if (target < 7)
        {
            if (target > 3)
            {
                stbyEn = true;
                target = target - 3;
            }
            else
            {
                stbyEn = false;
            }
            cpuMode = (gpc_cpu_mode_t)target;
            if (cpuMode != kGPC_RunMode)
            {
                APP_SetButtonWakeupConfig();
                PRINTF("Target CPU mode is %s\r\n", GET_CPU_MODE_NAME(cpuMode));
                PRINTF("Go...\r\n");
                if (cpuMode == kGPC_SuspendMode)
                {
                    PRINTF("System will wake up from reset!\r\n");
                }
                CpuModeTransition(cpuMode, stbyEn);
                PrintSystemStatus();
            }
            else
            {
                PRINTF("CPU already in RUN mode!\r\n");
            }
        }
    }
}

#if (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC)
void TypicalSetPointTransition(void)
{
    bool stbyEn = false;
    gpc_cm_wakeup_sp_sel_t wakeupSel;
    gpc_cpu_mode_t cpuMode;
    uint8_t ch, target, targetSp, i, activeSpCnt, standbySpCnt;
#ifndef SINGLE_CORE_M7
    uint8_t activeSp[]  = {1, 0, 5, 7, 9, 11, 12};
    uint8_t standbySp[] = {1, 0, 5, 7, 10, 11, 15};
#else
    uint8_t activeSp[]  = {1, 0, 5, 7};
    uint8_t standbySp[] = {1, 0, 5, 10, 15};
#endif

    uint8_t currentSp;
    uint8_t currentSw1OutVolt;
    uint8_t targetSw1OutVolt;

    activeSpCnt  = sizeof(activeSp) / sizeof(uint8_t);
    standbySpCnt = sizeof(standbySp) / sizeof(uint8_t);

    while (1)
    {
        PRINTF("\r\nSet Point Transition:\r\n");
        for (i = 0; i < activeSpCnt; i++)
        {
            PRINTF("Press %c to enter Set Point: %d\r\n", (uint8_t)'A' + i, activeSp[i]);
            cpuMode = getCpuMode(getCore0Status(activeSp[i]));
            PRINTF("    M7 CPU mode: %s\r\n", GET_CPU_MODE_NAME(cpuMode));
#ifndef SINGLE_CORE_M7
            cpuMode = getCpuMode(getCore1Status(activeSp[i]));
            PRINTF("    M4 CPU mode: %s\r\n", GET_CPU_MODE_NAME(cpuMode));
#endif
        }
        for (i = 0; i < standbySpCnt; i++)
        {
            cpuMode = kGPC_SuspendMode;
            PRINTF("Press %c to enter Set Point: %d\r\n", (uint8_t)'A' + activeSpCnt + i, standbySp[i]);
            PRINTF("    M7 CPU mode: %s\r\n", GET_CPU_MODE_NAME(cpuMode));
#ifndef SINGLE_CORE_M7
            PRINTF("    M4 CPU mode: %s\r\n", GET_CPU_MODE_NAME(cpuMode));
#endif
            PRINTF("    System standby\r\n");
        }
        PRINTF("Press 'Q' to exit\r\n");
        PRINTF("\r\nWaiting for select...\r\n");

        // get current setpoint of the system.
        currentSp = (GPC_CPU_MODE_CTRL_0->CM_SP_STAT) & (GPC_CPU_MODE_CTRL_CM_SP_STAT_CPU_SP_CURRENT_MASK);

        /* Wait for user response */
        ch = GETCHAR();

#if defined(SINGLE_CORE_M7)
        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }
        if (ch == 'Q')
        {
            break;
        }
        target = (ch - 'A');
        if (target < activeSpCnt)
        {
            targetSp = activeSp[target];
        }
        else
        {
            target   = target - activeSpCnt;
            targetSp = standbySp[target];
        }
#endif /* (defined(SINGLE_CORE_M7)) */

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }
        if (ch == 'Q')
        {
            break;
        }

        target = (ch - 'A');

        if (target < (activeSpCnt + standbySpCnt))
        {
            if (target < activeSpCnt)
            {
                targetSp = activeSp[target];
                if (getCore0Status(targetSp) == kCORE_NormalRun)
                {
                    cpuMode   = kGPC_RunMode;
                    wakeupSel = kGPC_CM_WakeupSetpoint;
                }
                else if (getCore0Status(targetSp) == kCORE_ClockGated)
                {
                    cpuMode   = kGPC_StopMode;
                    wakeupSel = kGPC_CM_RequestPreviousSetpoint;
                }
                else /* Core status is kCORE_PowerGated */
                {
                    cpuMode   = kGPC_SuspendMode;
                    wakeupSel = kGPC_CM_RequestPreviousSetpoint;
                }
                stbyEn = false;
            }
            else
            {
                target    = target - activeSpCnt;
                targetSp  = standbySp[target];
                cpuMode   = kGPC_SuspendMode;
                wakeupSel = kGPC_CM_RequestPreviousSetpoint;
                stbyEn    = true;
            }

            /* if enter a setpoint which will turn off PLL, switch flexspi clock root. */
            if (((PMU_LDO_PLL_EN_SP_VAL >> targetSp) & 0x1) == 0)
            {
                SwitchFlexspiRootClock(true);
            }

            (void)PF5020_SWND1_SetOperateMode(&g_pf5020Handle, kPF5020_BuckRegulatorPWMMode, swnd1Mode[targetSp]);

            currentSw1OutVolt = sw1Volt[currentSp];
            targetSw1OutVolt  = sw1Volt[targetSp];

            if (stbyEn)
            {
                (void)PF5020_SW1_SetStandbyStateOption(&g_pf5020Handle, sw1Volt[targetSp],
                                                       kPF5020_BuckRegulatorPWMMode);

                if ((currentSp == 5) && ((target == 11)))
                {
                    PRINTF("Do not support this selection, please retry! \r\n");
                    continue;
                }

                uint32_t currentSpAllowedSps = GPC_CPU_MODE_CTRL_0->CM_SP_MAPPING[currentSp];
                uint32_t targetSpAllowedSps  = GPC_CPU_MODE_CTRL_1->CM_SP_MAPPING[targetSp];
                uint32_t allowedSps          = currentSpAllowedSps & targetSpAllowedSps;
                uint8_t maxIndex             = 15U;
                if (allowedSps != 0UL)
                {
                    // Find allowed setpoint.
                    while ((allowedSps & (1 << maxIndex)) == 0UL)
                    {
                        maxIndex--;
                    }
                    if (sw1Volt[maxIndex] > currentSw1OutVolt)
                    {
                        (void)PF5020_SW1_SetRunStateOption(&g_pf5020Handle, sw1Volt[maxIndex],
                                                           kPF5020_BuckRegulatorPWMMode);
                    }
                }

#ifdef CORE1_GET_INPUT_FROM_CORE0
                /* Send message to another core */
                MU_SendMsg(MU_BASE, CHN_MU_REG_NUM, ch);
                delay();
#endif
                APP_SetButtonWakeupConfig();
                PRINTF("Target CPU mode is %s\r\n", GET_CPU_MODE_NAME(cpuMode));
                PRINTF("Target setpoint in sleep mode is %d\r\n", targetSp);
                if (wakeupSel == kGPC_CM_WakeupSetpoint)
                {
                    PRINTF("Target setpoint after waking up is %d\r\n", targetSp);
                }
                else
                {
                    PRINTF("Target setpoint after waking up is the setpoint before entering low power mode.\r\n");
                }
                PRINTF("Go...\r\n");
                PowerModeTransition(cpuMode, targetSp, targetSp, wakeupSel, true);
                continue;
            }

            if (currentSw1OutVolt > targetSw1OutVolt)
            {
                /* In case of changing to lower voltage level. */

                /* 1. switch to target setpoint. */
                GPC_CM_RequestRunModeSetPointTransition(GPC_CPU_MODE_CTRL, targetSp);
#ifdef CORE1_GET_INPUT_FROM_CORE0
                /* Reqest another core switch to target setpoint. */
                MU_SendMsg(MU_BASE, CHN_MU_REG_NUM, ch);
                delay();
#endif
                /* Polling until success to switch into target setpoint. */
                while (GPC_SP_GetCurrentSetPoint(GPC_SET_POINT_CTRL) != targetSp)
                {
                    ;
                }

                /* 2. Decrease voltage level. */
                (void)PF5020_SW1_SetRunStateOption(&g_pf5020Handle, targetSw1OutVolt, kPF5020_BuckRegulatorPWMMode);

                /* 3. If target cpu mode is not run mode, change cpu mode. */
                if (cpuMode != kGPC_RunMode)
                {
                    APP_SetButtonWakeupConfig();
                    PRINTF("Target CPU mode is %s\r\n", GET_CPU_MODE_NAME(cpuMode));
                    PRINTF("Target setpoint in sleep mode is %d\r\n", targetSp);
                    if (wakeupSel == kGPC_CM_WakeupSetpoint)
                    {
                        PRINTF("Target setpoint after waking up is %d\r\n", targetSp);
                    }
                    else
                    {
                        PRINTF("Target setpoint after waking up is the setpoint before entering low power mode.\r\n");
                    }
                    PRINTF("Go...\r\n");
                    if (cpuMode == kGPC_SuspendMode)
                    {
                        PRINTF("System will wake up from reset!\r\n");
                    }
                    CpuModeTransition(cpuMode, false);

                    /* Until now CPU is wake from low power mode */
                    /* 4. Increase voltage level. */
                    (void)PF5020_SW1_SetRunStateOption(&g_pf5020Handle, currentSw1OutVolt,
                                                       kPF5020_BuckRegulatorPWMMode);
                    /* 5. Change to previous setpoint. */
                    GPC_CM_RequestRunModeSetPointTransition(GPC_CPU_MODE_CTRL, currentSp);
                    /* Reqest another core switch to target setpoint. */
                    switch (currentSp)
                    {
                        case 1:
                        {
                            ch = 'A';
                            break;
                        }
                        case 0:
                        {
                            ch = 'B';
                            break;
                        }
                        case 5:
                        {
                            ch = 'C';
                            break;
                        }
                        case 7:
                        {
                            ch = 'D';
                            break;
                        }
                        default:
                        {
                            assert(false);
                            break;
                        }
                    }
#ifdef CORE1_GET_INPUT_FROM_CORE0
                    MU_SendMsg(MU_BASE, CHN_MU_REG_NUM, ch);
                    delay();
#endif
                    /* Polling until success to switch into target setpoint. */
                    while (GPC_SP_GetCurrentSetPoint(GPC_SET_POINT_CTRL) != currentSp)
                    {
                        ;
                    }
                }
            }
            else if (currentSw1OutVolt < targetSw1OutVolt)
            {
                /* In case of changing to higher voltage level. */

                /* 1. Increase voltage level. */
                (void)PF5020_SW1_SetRunStateOption(&g_pf5020Handle, targetSw1OutVolt, kPF5020_BuckRegulatorPWMMode);

                /* 2. Switch to target setpoint. */
                GPC_CM_RequestRunModeSetPointTransition(GPC_CPU_MODE_CTRL, targetSp);
#ifdef CORE1_GET_INPUT_FROM_CORE0
                /* Reqest another core switch to target setpoint. */
                MU_SendMsg(MU_BASE, CHN_MU_REG_NUM, ch);
                delay();
#endif
                /* Polling until success to switch into target setpoint. */
                while (GPC_SP_GetCurrentSetPoint(GPC_SET_POINT_CTRL) != targetSp)
                {
                    ;
                }

                /* 3. If target cpu mode is not run mode, change cpu mode */
                if (cpuMode != kGPC_RunMode)
                {
                    APP_SetButtonWakeupConfig();
                    PRINTF("Target CPU mode is %s\r\n", GET_CPU_MODE_NAME(cpuMode));
                    PRINTF("Target setpoint in sleep mode is %d\r\n", targetSp);
                    if (wakeupSel == kGPC_CM_WakeupSetpoint)
                    {
                        PRINTF("Target setpoint after waking up is %d\r\n", targetSp);
                    }
                    else
                    {
                        PRINTF("Target setpoint after waking up is the setpoint before entering low power mode.\r\n");
                    }
                    PRINTF("Go...\r\n");
                    if (cpuMode == kGPC_SuspendMode)
                    {
                        PRINTF("System will wake up from reset!\r\n");
                    }
                    CpuModeTransition(cpuMode, false);

                    /* Until now CPU is wake from low power mode */
                    /* 4. Change to previous setpoint. */
                    GPC_CM_RequestRunModeSetPointTransition(GPC_CPU_MODE_CTRL, currentSp);

                    /* Reqest another core switch to target setpoint. */
                    switch (currentSp)
                    {
                        case 1:
                        {
                            ch = 'A';
                            break;
                        }
                        case 0:
                        {
                            ch = 'B';
                            break;
                        }
                        case 5:
                        {
                            ch = 'C';
                            break;
                        }
                        case 7:
                        {
                            ch = 'D';
                            break;
                        }
                        default:
                        {
                            assert(false);
                            break;
                        }
                    }
#ifdef CORE1_GET_INPUT_FROM_CORE0
                    MU_SendMsg(MU_BASE, CHN_MU_REG_NUM, ch);
                    delay();
#endif
                    /* Polling until success to switch into target setpoint. */
                    while (GPC_SP_GetCurrentSetPoint(GPC_SET_POINT_CTRL) != currentSp)
                    {
                        ;
                    }

                    /* 5. Increase voltage level. */
                    (void)PF5020_SW1_SetRunStateOption(&g_pf5020Handle, currentSw1OutVolt,
                                                       kPF5020_BuckRegulatorPWMMode);
                }
            }
            else
            {
                /* In case of no voltage change. */

                /* Switch to target setpoint. */
                GPC_CM_RequestRunModeSetPointTransition(GPC_CPU_MODE_CTRL, targetSp);
                /* Reqest another core switch to target setpoint. */
#ifdef CORE1_GET_INPUT_FROM_CORE0
                MU_SendMsg(MU_BASE, CHN_MU_REG_NUM, ch);
                delay();
#endif
                /* Polling until success to switch into target setpoint. */
                while (GPC_SP_GetCurrentSetPoint(GPC_SET_POINT_CTRL) != targetSp)
                {
                    ;
                }
                /* If target cpu mode is not run mode, change cpu mode. */
                if (cpuMode != kGPC_RunMode)
                {
                    APP_SetButtonWakeupConfig();
                    PRINTF("Target CPU mode is %s\r\n", GET_CPU_MODE_NAME(cpuMode));
                    PRINTF("Target setpoint in sleep mode is %d\r\n", targetSp);
                    if (wakeupSel == kGPC_CM_WakeupSetpoint)
                    {
                        PRINTF("Target setpoint after waking up is %d\r\n", targetSp);
                    }
                    else
                    {
                        PRINTF("Target setpoint after waking up is the setpoint before entering low power mode.\r\n");
                    }
                    PRINTF("Go...\r\n");
                    if (cpuMode == kGPC_SuspendMode)
                    {
                        PRINTF("System will wake up from reset!\r\n");
                    }
                    CpuModeTransition(cpuMode, false);

                    /* Until now CPU is wake from low power mode */
                    /* Change to previous setpoint. */
                    GPC_CM_RequestRunModeSetPointTransition(GPC_CPU_MODE_CTRL, currentSp);

                    /* Reqest another core switch to previous setpoint. */
                    switch (currentSp)
                    {
                        case 1:
                        {
                            ch = 'A';
                            break;
                        }
                        case 0:
                        {
                            ch = 'B';
                            break;
                        }
                        case 5:
                        {
                            ch = 'C';
                            break;
                        }
                        case 7:
                        {
                            ch = 'D';
                            break;
                        }
                        default:
                        {
                            assert(false);
                            break;
                        }
                    }
#ifdef CORE1_GET_INPUT_FROM_CORE0
                    MU_SendMsg(MU_BASE, CHN_MU_REG_NUM, ch);
                    delay();
#endif
                    /* Polling until success to switch into target setpoint. */
                    while (GPC_SP_GetCurrentSetPoint(GPC_SET_POINT_CTRL) != currentSp)
                    {
                        ;
                    }
                }
            }

            /* if enter a setpoint which will turn on PLL, switch flexspi clock root. */
            if (((PMU_LDO_PLL_EN_SP_VAL >> GPC_SP_GetCurrentSetPoint(GPC_SET_POINT_CTRL)) & 0x1))
            {
                SwitchFlexspiRootClock(false);
            }
            PrintSystemStatus();
        }
    }
}

#else /* (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC) */
void TypicalSetPointTransition(void)
{
    bool stbyEn = false;
    gpc_cm_wakeup_sp_sel_t wakeupSel;
    gpc_cpu_mode_t cpuMode;
    uint8_t ch, target, targetSp, i, activeSpCnt, standbySpCnt;
#ifndef SINGLE_CORE_M7
    uint8_t activeSp[]  = {1, 0, 5, 7, 9, 11, 12};
    uint8_t standbySp[] = {1, 0, 5, 7, 10, 11, 15};
#else
    uint8_t activeSp[]  = {1, 0, 5, 7};
    uint8_t standbySp[] = {1, 0, 5, 10, 15};
#endif

    activeSpCnt  = sizeof(activeSp) / sizeof(uint8_t);
    standbySpCnt = sizeof(standbySp) / sizeof(uint8_t);

    while (1)
    {
        PRINTF("\r\nSet Point Transition:\r\n");
        for (i = 0; i < activeSpCnt; i++)
        {
            PRINTF("Press %c to enter Set Point: %d\r\n", (uint8_t)'A' + i, activeSp[i]);
            cpuMode = getCpuMode(getCore0Status(activeSp[i]));
            PRINTF("    M7 CPU mode: %s\r\n", GET_CPU_MODE_NAME(cpuMode));
#ifndef SINGLE_CORE_M7
            cpuMode = getCpuMode(getCore1Status(activeSp[i]));
            PRINTF("    M4 CPU mode: %s\r\n", GET_CPU_MODE_NAME(cpuMode));
#endif
        }
        for (i = 0; i < standbySpCnt; i++)
        {
            cpuMode = kGPC_SuspendMode;
            PRINTF("Press %c to enter Set Point: %d\r\n", (uint8_t)'A' + activeSpCnt + i, standbySp[i]);
            PRINTF("    M7 CPU mode: %s\r\n", GET_CPU_MODE_NAME(cpuMode));
#ifndef SINGLE_CORE_M7
            PRINTF("    M4 CPU mode: %s\r\n", GET_CPU_MODE_NAME(cpuMode));
#endif
            PRINTF("    System standby\r\n");
        }
        PRINTF("Press 'Q' to exit\r\n");
        PRINTF("\r\nWaiting for select...\r\n");

        /* Wait for user response */
        ch = GETCHAR();

#if defined(SINGLE_CORE_M7)
        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }
        if (ch == 'Q')
        {
            break;
        }
        target = (ch - 'A');
        if (target < activeSpCnt)
        {
            targetSp = activeSp[target];
        }
        else
        {
            target   = target - activeSpCnt;
            targetSp = standbySp[target];
        }
#endif /* (defined(SINGLE_CORE_M7) */

#ifdef CORE1_GET_INPUT_FROM_CORE0
        /* Send message to another core */
        MU_SendMsg(MU_BASE, CHN_MU_REG_NUM, ch);
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

        if (target < (activeSpCnt + standbySpCnt))
        {
            if (target < activeSpCnt)
            {
                targetSp = activeSp[target];
                if (getCore0Status(targetSp) == kCORE_NormalRun)
                {
                    cpuMode   = kGPC_RunMode;
                    wakeupSel = kGPC_CM_WakeupSetpoint;
                }
                else if (getCore0Status(targetSp) == kCORE_ClockGated)
                {
                    cpuMode   = kGPC_StopMode;
                    wakeupSel = kGPC_CM_RequestPreviousSetpoint;
                }
                else /* Core status is kCORE_PowerGated */
                {
                    cpuMode   = kGPC_SuspendMode;
                    wakeupSel = kGPC_CM_RequestPreviousSetpoint;
                }
                stbyEn = false;
            }
            else
            {
                target    = target - activeSpCnt;
                targetSp  = standbySp[target];
                cpuMode   = kGPC_SuspendMode;
                wakeupSel = kGPC_CM_RequestPreviousSetpoint;
                stbyEn    = true;
            }

            /* if enter a setpoint which will turn off PLL, switch flexspi clock root. */
            if (((PMU_LDO_PLL_EN_SP_VAL >> targetSp) & 0x1) == 0)
            {
                SwitchFlexspiRootClock(true);
            }
            if (cpuMode != kGPC_RunMode)
            {
                APP_SetButtonWakeupConfig();
                PRINTF("Target CPU mode is %s\r\n", GET_CPU_MODE_NAME(cpuMode));
                PRINTF("Target setpoint in sleep mode is %d\r\n", targetSp);
                if (wakeupSel == kGPC_CM_WakeupSetpoint)
                {
                    PRINTF("Target setpoint after waking up is %d\r\n", targetSp);
                }
                else
                {
                    PRINTF("Target setpoint after waking up is the setpoint before entering low power mode.\r\n");
                }
                PRINTF("Go...\r\n");
                if (cpuMode == kGPC_SuspendMode)
                {
                    PRINTF("System will wake up from reset!\r\n");
                }
                PowerModeTransition(cpuMode, targetSp, targetSp, wakeupSel, stbyEn);
            }
            else
            {
                PRINTF("Target setpoint is %d\r\n", targetSp);
                GPC_CM_RequestRunModeSetPointTransition(GPC_CPU_MODE_CTRL, targetSp);
            }
            /* if enter a setpoint which will turn on PLL, switch flexspi clock root. */
            if (((PMU_LDO_PLL_EN_SP_VAL >> GPC_SP_GetCurrentSetPoint(GPC_SET_POINT_CTRL)) & 0x1))
            {
                SwitchFlexspiRootClock(false);
            }
            PrintSystemStatus();
        }
    }
}
#endif /* (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC) */

#ifndef SINGLE_CORE_M7
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

    /* Copy Secondary core application from FLASH to the target memory. */
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_CleanInvalidateDCache_by_Addr((void *)CORE1_BOOT_ADDRESS, core1_image_size);
#endif
    memcpy((void *)CORE1_BOOT_ADDRESS, (void *)CORE1_IMAGE_START, core1_image_size);
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_CleanInvalidateDCache_by_Addr((void *)CORE1_BOOT_ADDRESS, core1_image_size);
#endif
#endif
}
#endif

/*!
 * @brief main demo function.
 */
int main(void)
{
    gpc_cpu_mode_t preCpuMode;
    uint8_t ch;

    /* Init board hardware.*/
    clock_root_config_t rootCfg = {0};

    BOARD_ConfigMPU();

    /* Workaround: Disable interrupt which might be enabled by ROM. */
    GPIO_DisableInterrupts(GPIO3, 1U << 24);
    GPIO_ClearPinsInterruptFlags(GPIO3, 1U << 24);

    /* Interrupt flags can't be cleared when wake up from SNVS mode, clear srtc and gpio interrupts here. */
    SNVS->LPSR |= SNVS_LPSR_LPTA_MASK;
    GPIO_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);

    BOARD_InitPins();

    rootCfg.mux = kCLOCK_LPUART1_ClockRoot_MuxOscRc16M;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpuart1, &rootCfg);

    BOARD_InitDebugConsole();

    // set start address after waking up from SUSPEND mode
    IOMUXC_LPSR_GPR->GPR26 &= ~IOMUXC_LPSR_GPR_GPR26_CM7_INIT_VTOR_MASK;
    IOMUXC_LPSR_GPR->GPR26 |= IOMUXC_LPSR_GPR_GPR26_CM7_INIT_VTOR(IMAGE_ENTRY_ADDRESS >> 7);

    preCpuMode = GPC_CM_GetPreviousCpuMode(GPC_CPU_MODE_CTRL);
    if (preCpuMode != kGPC_RunMode)
    {
#ifdef CORE1_GET_INPUT_FROM_CORE0
        MU_SendMsg(MU_BASE, CHN_MU_REG_NUM, 'Q');
#endif
        PRINTF("\r\nSystem wake up from reset!\r\n");
    }
    else
    {
/* Macro SINGLE_CORE_M7 is only for single core chip MIMXRT1171/MIMXRT1172, details please refer to application note
 * AN13116. */
#ifdef SINGLE_CORE_M7
        PRINTF("\r\nThis is single core M7.\r\n");
        /* Since core1 is not used and is always in reset status, force core1 requesting standby mode */
        GPC_STBY_CTRL->STBY_MISC |= GPC_STBY_CTRL_STBY_MISC_FORCE_CPU1_STBY_MASK;
#else
        PRINTF("\r\nThis is core0.\r\n");

        APP_CopyCore1Image();

        /* MU init */
        MU_Init(MU_BASE);

        /* Boot Secondary core application */
        PRINTF("Starting secondary core.\r\n");
        APP_BootCore1();

        /* Wait Core 1 is Boot Up */
        while (BOOT_FLAG != MU_GetFlags(MU_BASE))
        {
        }
        PRINTF("The secondary core application has been started.\r\n");
#endif

        PRINTF("\r\nCPU wakeup source 0x%x...\r\n", SRC->SRSR);
        PRINTF("\r\n***********************************************************\r\n");
        PRINTF("\tPower Mode Switch Demo for %s\r\n", CPU_NAME);
        PRINTF("***********************************************************\r\n");

        ChipInitConfig();

#ifdef CORE1_GET_INPUT_FROM_CORE0
        PRINTF("\r\nCore0 send message to core1.\r\n");
#endif
    }

#if (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC)
    APP_InitPMIC();
#endif /* (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC) */

#if (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC)
    uint8_t currentSp = GPC_SP_GetCurrentSetPoint(GPC_SET_POINT_CTRL);
    /* Change PF5020's SW1 output voltage. */
    (void)PF5020_SW1_SetRunStateOption(&g_pf5020Handle, sw1Volt[currentSp], kPF5020_BuckRegulatorPWMMode);
#endif /* (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC) */
    PrintSystemStatus();

    while (1)
    {
        PRINTF("\r\nPlease select the desired operation:\r\n");
        PRINTF("Press  %c to demonstrate typical set point transition.\r\n", (uint8_t)'A');
        PRINTF("Press  %c to demonstrate cpu mode switch in setpoint 0.\r\n", (uint8_t)'B');
        PRINTF("Press  %c to enter SNVS mode.\r\n", (uint8_t)'C');
        PRINTF("\r\nWaiting for select...\r\n");

        /* Wait for user response */
        ch = GETCHAR();
#ifdef CORE1_GET_INPUT_FROM_CORE0
        /* Send message to another core */
        MU_SendMsg(MU_BASE, CHN_MU_REG_NUM, ch);
#endif

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }

        switch (ch)
        {
            case 'A':
                TypicalSetPointTransition();
                break;
            case 'B':
                CpuModeSwitchInSetPoint(0);
                break;
            case 'C':
                APP_SetWakeupConfig();
                PRINTF("Now shutting down the system...\r\n");
                /* Turn off system power. */
                SNVS->LPCR |= SNVS_LPCR_TOP_MASK;
                while (1)
                    ;
                break;
            default:
                break;
        }
    }
}
