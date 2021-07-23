/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_pca9420.h"
#include "fsl_power.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_PCA9420_INTB_HANDLER PMU_PMIC_IRQHandler
#define DEMO_PCA9420_INTB_IRQ     PMU_PMIC_IRQn
#define PCA9420_LAST_REG (PCA9420_MODECFG_3_3)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_ConfigPMICModes(pca9420_modecfg_t *cfg, uint32_t num);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static pca9420_handle_t pca9420Handle;
static volatile bool pca9420IntFlag;
static uint8_t buffer[PCA9420_LAST_REG + 1];

/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_ConfigPMICModes(pca9420_modecfg_t *cfg, uint32_t num)
{
    assert(cfg);

    if (num >= 2)
    {
        /* Mode 1: High drive. */
        cfg[1].sw1OutVolt  = kPCA9420_Sw1OutVolt1V100;
        cfg[1].sw2OutVolt  = kPCA9420_Sw2OutVolt1V900;
        cfg[1].ldo1OutVolt = kPCA9420_Ldo1OutVolt1V900;
        cfg[1].ldo2OutVolt = kPCA9420_Ldo2OutVolt3V300;
    }
    if (num >= 3)
    {
        /* Mode 2: Low drive. */
        cfg[2].sw1OutVolt  = kPCA9420_Sw1OutVolt0V975;
        cfg[2].sw2OutVolt  = kPCA9420_Sw2OutVolt1V800;
        cfg[2].ldo1OutVolt = kPCA9420_Ldo1OutVolt1V800;
        cfg[2].ldo2OutVolt = kPCA9420_Ldo2OutVolt3V300;
    }
    if (num >= 4)
    {
        /* Mode 3: VDDIO off, watchdog enabled. */
        cfg[3].enableLdo2Out = false;
        cfg[3].wdogTimerCfg  = kPCA9420_WdTimer16s;
    }
}
/*! @brief PCA9420 INTB Pad ISR function */
void DEMO_PCA9420_INTB_HANDLER(void)
{
    pca9420IntFlag = true;
    /* Disable IRQ to avoid dead loop in IRQ handler. */
    DisableIRQ(DEMO_PCA9420_INTB_IRQ);
    SDK_ISR_EXIT_BARRIER;
}

static void DEMO_HandleInterrupt(void)
{
    uint32_t status;

    status = PCA9420_GetInterruptStatus(&pca9420Handle);
    if (status & kPCA9420_IntSrcSysVinOKChanged)
    {
        PRINTF("VIN OK status changed!\r\n");
    }
    if (status & kPCA9420_IntSrcSysWdogTimeout)
    {
        PRINTF("Watch dog timeout!\r\n");
    }
    if (status & kPCA9420_IntSrcSysAsysPreWarn)
    {
        PRINTF("ASYS voltage falls below the threshold!\r\n");
    }
    if (status & kPCA9420_IntSrcSysThermalShutdown)
    {
        PRINTF("Die temperature exceeds thermal shutdown threshold!\r\n");
    }
    if (status & kPCA9420_IntSrcSysTempWarn)
    {
        PRINTF("Die temperature reaches pre-warning threshold!\r\n");
    }
    if (status & kPCA9420_IntSrcSw1VoutOk)
    {
        PRINTF("SW1 output OK status changed!\r\n");
    }
    if (status & kPCA9420_IntSrcSw2VoutOk)
    {
        PRINTF("SW2 output OK status changed!\r\n");
    }
    if (status & kPCA9420_IntSrcLdo1VoutOk)
    {
        PRINTF("LDO1 output OK status changed!\r\n");
    }
    if (status & kPCA9420_IntSrcLdo2VoutOk)
    {
        PRINTF("LDO2 output OK status changed!\r\n");
    }

    /* Clear PMIC int status. */
    PCA9420_ClearInterruptStatus(&pca9420Handle, status);
    /* Clear PMC flags. */
    POWER_ClearEventFlags(kPMC_FLAGS_INTNPADF);
    /* Clear NVIC pending int status. */
    NVIC_ClearPendingIRQ(DEMO_PCA9420_INTB_IRQ);
}

static uint32_t DEMO_GetDemoToRun(void)
{
    uint8_t demoSel;

    PRINTF("\r\n-------------- PCA9420 on board PMIC driver example --------------\r\n");
    PRINTF("\r\nPlease select the PMIC example you want to run:\r\n");
    PRINTF("[1]. Dumping Mode Settings\r\n");
    PRINTF("[2]. Switch Mode\r\n");
    PRINTF("[3]. Dumping Selected Register Content\r\n");
    PRINTF("[4]. Feed watch dog\r\n");

    while (1)
    {
        demoSel = GETCHAR();
        if ((demoSel >= '1') && (demoSel <= '4'))
        {
            break;
        }
    }

    return demoSel;
}

static uint32_t DEMO_GetInputNumWithEcho(uint32_t length, bool allowZero)
{
    uint8_t ch;
    uint8_t digBuffer[10U] = {0U};
    uint8_t i, j;
    uint8_t digCount = 0U;
    uint32_t temp1 = 0U, temp2 = 0U;
    uint32_t result  = 0U;
    bool getFirstDig = false;

    assert(length <= (sizeof(digBuffer) / sizeof(digBuffer[0U])));

    /* Get user input and echo it back to terminal. */
    for (;;)
    {
        ch = GETCHAR();
        if ((('0' <= ch) && ('9' >= ch)) && (digCount < length))
        {
            if (false == getFirstDig)
            {
                if (allowZero || (('0' < ch) && ('9' >= ch)))
                {
                    getFirstDig = true;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                if ('0' == digBuffer[0U])
                {
                    continue;
                }
            }
            PUTCHAR(ch);
            digBuffer[digCount] = ch;
            digCount++;
        }
        else if ((0x7FU == ch) && (0U != digCount))
        {
            digBuffer[digCount] = 0x0U;
            digCount--;
            PUTCHAR(0x7FU);
        }
        else
        {
            if ('\r' == ch)
            {
                break;
            }
        }
    }

    /* Reconstruct user input number. */
    temp1 = digCount - 1;
    for (i = 0; i < digCount; i++)
    {
        temp2 = digBuffer[i] - '0';
        for (j = 0U; j < temp1; j++)
        {
            temp2 *= 10U;
        }
        temp1--;
        result += temp2;
    }

    return result;
}

static void DEMO_DumpModes(void)
{
    pca9420_modecfg_t modeCfgs[4];
    uint32_t i;
    pca9420_regulator_mv_t regVolt;
    pca9420_mode_t currentMode;
    bool result;

    result = PCA9420_GetCurrentMode(&pca9420Handle, &currentMode);
    if (!result)
    {
        PRINTF("Get current mode failed\r\n");
        return;
    }

    PCA9420_ReadModeConfigs(&pca9420Handle, kPCA9420_Mode0, modeCfgs, ARRAY_SIZE(modeCfgs));
    for (i = 0; i < ARRAY_SIZE(modeCfgs); i++)
    {
        PCA9420_GetRegulatorVolt(&modeCfgs[i], &regVolt);
        PRINTF("--------------------- Mode %d%s -----------------\r\n", i, ((uint8_t)currentMode) == i ? "(*)" : "");
        PRINTF("Mode controlled by [%s]\r\n", modeCfgs[i].modeSel == kPCA9420_ModeSelPin ? "Pin" : "I2C");
        PRINTF("Watch dog timer    [%s]\r\n",
               modeCfgs[i].wdogTimerCfg == kPCA9420_WdTimerDisabled ? "Disabled" : "Enabled");
        if (modeCfgs[i].wdogTimerCfg != kPCA9420_WdTimerDisabled)
        {
            PRINTF("Watch dog timeout  [%d sec]\r\n", (1U << ((((uint32_t)(modeCfgs[i].wdogTimerCfg)) >> 6U) + 3)));
        }
        /* SW1 voltage */
        PRINTF("SW1 voltage        [%d.%03dV][%s]\r\n", regVolt.mVoltSw1 / 1000, regVolt.mVoltSw1 % 1000,
               modeCfgs[i].enableSw1Out ? "ON" : "OFF");
        /* SW2 voltage */
        PRINTF("SW2 voltage        [%d.%03dV][%s]\r\n", regVolt.mVoltSw2 / 1000, regVolt.mVoltSw2 % 1000,
               modeCfgs[i].enableSw2Out ? "ON" : "OFF");
        /* LDO1 voltage */
        PRINTF("LDO1 voltage       [%d.%03dV][%s]\r\n", regVolt.mVoltLdo1 / 1000, regVolt.mVoltLdo1 % 1000,
               modeCfgs[i].enableLdo1Out ? "ON" : "OFF");
        /* LDO2 voltage */
        PRINTF("LDO2 voltage       [%d.%03dV][%s]\r\n", regVolt.mVoltLdo2 / 1000, regVolt.mVoltLdo2 % 1000,
               modeCfgs[i].enableLdo2Out ? "ON" : "OFF");
    }
    PRINTF("----------------------- End -------------------\r\n");
}

static void DEMO_SwitchMode(void)
{
    uint8_t demoSel;
    pca9420_mode_t currentMode, targetMode;
    bool result;

    /* Scroll to a new page. */
    PRINTF("\f");
    PRINTF("\r\n-------------- PCA9420 Switch Mode --------------\r\n");

    result = PCA9420_GetCurrentMode(&pca9420Handle, &currentMode);
    if (!result)
    {
        PRINTF("Get current mode failed\r\n");
        return;
    }
    PRINTF("Current mode: %d\r\n", (uint32_t)currentMode);

    PRINTF("Please select the mode to switch to (0~3):");
    demoSel = GETCHAR();
    PRINTF("%c\r\n", demoSel);

    if (demoSel < '0' || demoSel > '3')
    {
        PRINTF("Unsupported mode group %c\r\n", demoSel);
        return;
    }
    targetMode = (pca9420_mode_t)(demoSel - '0');

    if (currentMode == targetMode)
    {
        PRINTF("Target mode same as current (mode %c), do not switch mode\r\n", demoSel);
        return;
    }

    result = PCA9420_SwitchMode(&pca9420Handle, targetMode);
    PRINTF("Switch mode to mode %c (%s)\r\n", demoSel, result ? "Success" : "Failure");
}

static void DEMO_DumpRegisterContent(void)
{
    uint32_t address;
    uint8_t number;
    uint8_t i;
    bool result;

    for (;;)
    {
        /* Scroll to a new page. */
        PRINTF("\f");
        PRINTF("\r\n-------------- Dumping PCA9420 Register Content --------------\r\n\r\n");
        PRINTF("Please select the first address to dump(0~%d):", PCA9420_LAST_REG);
        address = DEMO_GetInputNumWithEcho(2U, true);
        if (address <= PCA9420_LAST_REG)
        {
            break;
        }
    }
    PRINTF("\r\n");

    for (;;)
    {
        PRINTF("Please select the number of registers to dump(1~%d):", PCA9420_LAST_REG + 1);
        number = DEMO_GetInputNumWithEcho(2U, false);
        if (((address + number) <= PCA9420_LAST_REG + 1) && (number != 0))
        {
            break;
        }
        else
        {
            PRINTF("\r\nOut of range.\r\n");
        }
    }
    PRINTF("\r\n");

    /* Read register from PCA9420 */
    result = PCA9420_ReadRegs(&pca9420Handle, address, buffer, number);
    if (result)
    {
        PRINTF("\r\nDump Format: [Register Address] = Register Content:\r\n");
        for (i = 0; i < number; i++)
        {
            PRINTF("[0x%x] = 0x%x\r\n", address + i, buffer[i]);
        }
    }
    else
    {
        PRINTF("Read registers failed\r\n");
    }

    PRINTF("\r\n");
}

/*! @brief Main function */
int main(void)
{
    pca9420_config_t pca9420Config;
    uint32_t demoSelection;
    pca9420_modecfg_t pca9420ModeCfg[4];
    uint32_t i;
    uint8_t regStatus;

    /* Init board hardware. */
    /* Use 48 MHz clock for the FLEXCOMM15 */
    CLOCK_AttachClk(kFRO_DIV4_to_FLEXCOMM15);

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    /* Init PMIC I2C. */
    BOARD_PMIC_I2C_Init();

    /* Init PCA9420 Component. */
    PCA9420_GetDefaultConfig(&pca9420Config);
    pca9420Config.I2C_SendFunc    = BOARD_PMIC_I2C_Send;
    pca9420Config.I2C_ReceiveFunc = BOARD_PMIC_I2C_Receive;
    PCA9420_Init(&pca9420Handle, &pca9420Config);

    /* Configure PMIC modes. */
    for (i = 0; i < ARRAY_SIZE(pca9420ModeCfg); i++)
    {
        PCA9420_GetDefaultModeConfig(&pca9420ModeCfg[i]);
    }
    BOARD_ConfigPMICModes(pca9420ModeCfg, ARRAY_SIZE(pca9420ModeCfg));
    PCA9420_WriteModeConfigs(&pca9420Handle, kPCA9420_Mode0, &pca9420ModeCfg[0], ARRAY_SIZE(pca9420ModeCfg));

    /* Enable PMC pad interrupts */
    /* Clear flags first. */
    POWER_ClearEventFlags(kPMC_FLAGS_INTNPADF);
    POWER_EnableInterrupts(kPMC_INT_INTRPAD);
    /* Enable PMIC interrupts. */
    PCA9420_EnableInterrupts(&pca9420Handle, kPCA9420_IntSrcSysAll | kPCA9420_IntSrcRegulatorAll);

    /* Print the initial banner. */
    for (;;)
    {
        demoSelection = DEMO_GetDemoToRun();
        switch (demoSelection)
        {
            /* Dumping Mode Settings: */
            case '1':
                DEMO_DumpModes();
                break;

            /* Switch Mode: */
            case '2':
                DEMO_SwitchMode();
                break;

            /* Dumping Selected Register Content: */
            case '3':
                DEMO_DumpRegisterContent();
                break;

            /* Feed watch dog: */
            case '4':
                PCA9420_FeedWatchDog(&pca9420Handle);
                break;
            default:
                break;
        }

        /* Process interrupt during last operation. */
        /* Enable NVIC IRQ */
        EnableIRQ(DEMO_PCA9420_INTB_IRQ);
        if (pca9420IntFlag)
        {
            DEMO_HandleInterrupt();
            pca9420IntFlag = false;
        }

        regStatus = PCA9420_GetRegulatorStatus(&pca9420Handle);
        PRINTF("-------------- PCA9420 Regulator Status --------------\r\n");
        PRINTF("SW1   -  %s\r\n", (regStatus & ((uint8_t)kPCA9420_RegStatusVoutSw1OK)) ? "Good" : "Bad");
        PRINTF("SW2   -  %s\r\n", (regStatus & ((uint8_t)kPCA9420_RegStatusVoutSw2OK)) ? "Good" : "Bad");
        PRINTF("LDO1  -  %s\r\n", (regStatus & ((uint8_t)kPCA9420_RegStatusVoutLdo1OK)) ? "Good" : "Bad");
        PRINTF("LDO2  -  %s\r\n", (regStatus & ((uint8_t)kPCA9420_RegStatusVoutLdo2OK)) ? "Good" : "Bad");

        PRINTF("Press Any Key to Home Page...");
        GETCHAR();
        /* Scroll to a new page. */
        PRINTF("\f");
    }
}
