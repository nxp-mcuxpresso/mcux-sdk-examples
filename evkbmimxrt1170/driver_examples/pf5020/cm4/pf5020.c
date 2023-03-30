/*
 * Copyright (c) 2021, 2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_lpi2c.h"
#include "fsl_gpio.h"
#include "fsl_pf5020.h"
#include "fsl_pgmc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_PF5020_LPI2C             LPI2C6
#define DEMO_PF5020_LPI2C_CLKSRC_FREQ (CLOCK_GetFreq(kCLOCK_OscRc48MDiv2))
#define DEMO_PF5020_LPI2C_BAUDRATE    1000000U
#define DEMO_INTB_IRQn
#define DEMO_INTB_GPIO GPIO11
#define DEMO_INTB_PIN  (1U)
#define DEMO_INTB_IRQ_HANDLER
#define SKIP_DCDC_CONFIGURATION (1U)
#define DEMO_REGULATOR_NAME_ARRAY     \
    {                                 \
        "SW1", "SW2", "SWND1", "LDO1" \
    }
#define DEMO_REGULATOR_MODE_REG                               \
    {                                                         \
        PF5020_SW1_MODE, PF5020_SW2_MODE1, PF5020_SWND1_MODE1 \
    }
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static status_t I2C_SendFunc(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *txBuff, uint8_t txBuffSize);
static status_t I2C_ReceiveFunc(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize);

static uint8_t DEMO_MenuSelection(void);
static void DEMO_RegulatorConfiguration(void);
static void DEMO_SetRegulator(pf5020_regulator_name_t regulatorName);
static uint8_t DEMO_GetVoltageInput(pf5020_regulator_name_t regulatorName, bool runMode);
static uint8_t DEMO_GetOperateMode(pf5020_regulator_name_t regulatorName, bool runMode);
static void DEMO_TuneClockFreq(void);
static void DEMO_EnterStandbyState(void);
static void DEMO_DumpRegisterValue(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/
static lpi2c_master_handle_t g_lpi2cHandle;
static volatile bool g_lpi2cIntFlag;
static pf5020_handle_t g_pf5020Handle;
static uint8_t regBuffer[120];

char *g_regulatorNameArray[4] = DEMO_REGULATOR_NAME_ARRAY;
uint8_t g_regulatorModeReg[]    = DEMO_REGULATOR_MODE_REG;
/*******************************************************************************
 * Code
 ******************************************************************************/


static void lpi2c_master_callback(LPI2C_Type *base, lpi2c_master_handle_t *handle, status_t status, void *userData)
{
    /* Signal transfer success when received success status. */
    if (status == kStatus_Success)
    {
        g_lpi2cIntFlag = true;
    }
}

int main(void)
{
    lpi2c_master_config_t masterConfig;
    pf5020_config_t pmicConfig;
    uint8_t menuItemNum;

    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("PF5020 Driver example!\r\n");

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

    while (1)
    {
        menuItemNum = DEMO_MenuSelection();

        switch (menuItemNum)
        {
            case '1':
            {
                DEMO_RegulatorConfiguration();
                break;
            }

            case '2':
            {
                DEMO_TuneClockFreq();
                break;
            }

            case '3':
            {
                DEMO_EnterStandbyState();
                break;
            }

            case '4':
            {
                DEMO_DumpRegisterValue();
                break;
            }
            default:
            {
                break;
            }
        }
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

static uint8_t DEMO_MenuSelection(void)
{
    uint8_t itemNum;

    PRINTF("\r\n------------------------ PF5020 Functional Block Menu ---------------------------------\r\n");
    PRINTF("\r\nPlease select the function block you want to run:\r\n");
    PRINTF("[1]. Regulators Setting.\r\n");
    PRINTF("[2]. Clock Setting.\r\n");
    PRINTF("[3]. Enter Standby State.\r\n");
    PRINTF("[4]. Dump Register Value.\r\n");

    while (1)
    {
        itemNum = GETCHAR();

        if (itemNum >= '1' || itemNum <= '4')
        {
            break;
        }
        PRINTF("\r\nWrong Menu Item selected! Please retry...\r\n");
    }

    return itemNum;
}

static void DEMO_RegulatorConfiguration(void)
{
    status_t status;
    uint8_t chInput;

    while (1)
    {
        PRINTF("\r\n-------------------- Regulator Setting ---------------------\r\n");
        PRINTF("Please select which regulator to set:\r\n");
        PRINTF("[a]. Type 1 Buck Regulator SW1\r\n");
        PRINTF("[b]. Type 1 Buck Regulator SW2\r\n");
        PRINTF("[c]. Type 2 Buck Regulator SWND1\r\n");
        PRINTF("[d]. Linear Regulator LDO1\r\n");
        PRINTF("[q]. Back to home page.\r\n");

        for (;;)
        {
            chInput = GETCHAR();
            if (((chInput >= 'a') && (chInput <= 'd')) || (chInput == 'q'))
            {
                break;
            }
            else
            {
                PRINTF("Wrong Input, please re-select...\r\n");
            }
        }
        switch (chInput)
        {
            case 'a':
            {
                status =
                    PF5020_DisableInterrupts(&g_pf5020Handle, kPF5020_UV_Sw1UvInterrupt | kPF5020_OV_Sw1OvInterrupt);
                if (status == kStatus_Success)
                {
                    DEMO_SetRegulator(kPF5020_BuckRegulatorSw1);
                }
                break;
            }
            case 'b':
            {
                status =
                    PF5020_DisableInterrupts(&g_pf5020Handle, kPF5020_UV_Sw2UvInterrupt | kPF5020_OV_Sw2OvInterrupt);
                if (status == kStatus_Success)
                {
                    DEMO_SetRegulator(kPF5020_BuckRegulatorSw2);
                }
                break;
            }
            case 'c':
            {
                status = PF5020_DisableInterrupts(&g_pf5020Handle,
                                                  kPF5020_UV_Swnd1UvInterrupt | kPF5020_OV_Swnd1OvInterrupt);
                if (status == kStatus_Success)
                {
                    DEMO_SetRegulator(kPF5020_BuckRegulatorSwnd1);
                }
                break;
            }
            case 'd':
            {
                status =
                    PF5020_DisableInterrupts(&g_pf5020Handle, kPF5020_UV_Ldo1UvInterrupt | kPF5020_OV_Ldo1OvInterrupt);
                if (status == kStatus_Success)
                {
                    DEMO_SetRegulator(kPF5020_RegulatorLdo1);
                }
                break;
            }
            case 'q':
            {
                return;
            }
            default:
            {
                break;
            }
        }
    }
}

static void DEMO_SetRegulator(pf5020_regulator_name_t regulatorName)
{
    uint8_t chInput;
    uint8_t regulatorVoltage;
    uint8_t operateMode;
    uint8_t tmp8;
    uint8_t index;

    if (regulatorName >= kPF5020_BuckRegulatorSwnd1)
    {
        index = (uint8_t)regulatorName - 2U;
    }
    else
    {
        index = (uint8_t)regulatorName;
    }
    while (1)
    {
        PRINTF("\r\n-------------- %s Regulator Configurations --------------\r\n", g_regulatorNameArray[index]);
        PRINTF("\r\nPlease select which configuration to set:\r\n");
        if (regulatorName != kPF5020_BuckRegulatorSwnd1)
        {
            PRINTF("[a]. Run state configuration\r\n");
            PRINTF("[b]. Standby state configuration\r\n");
            PRINTF("[q]. Back to last page\r\n");
        }
        else
        {
            PRINTF("[a]. Regulator SWND1 output voltage in RUN state and STANDBY state\r\n");
            PRINTF("[b]. Regulator SWND1 Operate Mode\r\n");
            PRINTF("[q]. Back to last page\r\n");
        }

        for (;;)
        {
            chInput = GETCHAR();
            PUTCHAR(chInput);
            PRINTF("\r\n");
            if ((chInput == 'a') || (chInput == 'b') || (chInput == 'q'))
            {
                break;
            }
            PRINTF("Wrong Input, please retry...\r\n");
        }

        switch (chInput)
        {
            case 'a':
            {
                if (regulatorName != kPF5020_BuckRegulatorSwnd1)
                {
                    PRINTF("Please select the voltage of %s in Run state:\r\n", g_regulatorNameArray[index]);
                    regulatorVoltage = DEMO_GetVoltageInput(regulatorName, true);
                    PRINTF("Please select the operate mode of %s in Run state:\r\n", g_regulatorNameArray[index]);
                    operateMode = DEMO_GetOperateMode(regulatorName, true);

                    if (regulatorName == kPF5020_BuckRegulatorSw1)
                    {
                        (void)PF5020_SW1_SetRunStateOption(&g_pf5020Handle, regulatorVoltage,
                                                           (pf5020_buck_regulator_operate_mode_t)operateMode);
                    }
                    else if (regulatorName == kPF5020_BuckRegulatorSw2)
                    {
                        (void)PF5020_SW2_SetRunStateOption(&g_pf5020Handle, regulatorVoltage,
                                                           (pf5020_buck_regulator_operate_mode_t)operateMode);
                    }
                    else
                    {
                        (void)PF5020_LDO1_SetRunStateOption(&g_pf5020Handle, (bool)operateMode,
                                                            (pf5020_ldo1_output_voltage_t)regulatorVoltage);
                    }
                }
                else
                {
                    PRINTF("Please select the voltage of SWND1 in RUN/STANDBY state:\r\n");
                    regulatorVoltage = DEMO_GetVoltageInput(kPF5020_BuckRegulatorSwnd1, true);
                    (void)PF5020_SWND1_SetOutputVoltage(&g_pf5020Handle,
                                                        (pf5020_swnd1_output_voltage_t)regulatorVoltage);
                }
                break;
            }

            case 'b':
            {
                if (regulatorName != kPF5020_BuckRegulatorSwnd1)
                {
                    PRINTF("Please select the voltage of %s in Standby state:\r\n", g_regulatorNameArray[index]);
                    regulatorVoltage = DEMO_GetVoltageInput(regulatorName, false);
                    PRINTF("Please select the operate mode of %s in Standby state:\r\n", g_regulatorNameArray[index]);
                    operateMode = DEMO_GetOperateMode(regulatorName, false);
                    if (regulatorName == kPF5020_BuckRegulatorSw1)
                    {
                        (void)PF5020_SW1_SetStandbyStateOption(&g_pf5020Handle, regulatorVoltage,
                                                               (pf5020_buck_regulator_operate_mode_t)operateMode);
                    }
                    else if (regulatorName == kPF5020_BuckRegulatorSw2)
                    {
                        (void)PF5020_SW2_SetStandbyStateOption(&g_pf5020Handle, regulatorVoltage,
                                                               (pf5020_buck_regulator_operate_mode_t)operateMode);
                    }
                    else
                    {
                        (void)PF5020_LDO1_SetStandbyStateOption(&g_pf5020Handle, (bool)operateMode,
                                                                (pf5020_ldo1_output_voltage_t)regulatorVoltage);
                    }
                }
                else
                {
                    PRINTF("Please select the operate mode of SWND1 in RUN state:\r\n");
                    tmp8 = DEMO_GetOperateMode(kPF5020_BuckRegulatorSwnd1, true);
                    PRINTF("Please select the operate mode of SWND1 in STANDBY state:\r\n");
                    operateMode = DEMO_GetOperateMode(kPF5020_BuckRegulatorSwnd1, false);
                    (void)PF5020_SWND1_SetOperateMode(&g_pf5020Handle, (pf5020_buck_regulator_operate_mode_t)tmp8,
                                                      (pf5020_buck_regulator_operate_mode_t)operateMode);
                }
                break;
            }

            case 'q':
            {
                return;
            }
            default:
            {
                break;
            }
        }
    }
}

static uint8_t DEMO_GetVoltageInput(pf5020_regulator_name_t regulatorName, bool runMode)
{
    uint8_t chInput;
    uint8_t voltageValue;

    switch (regulatorName)
    {
        case kPF5020_BuckRegulatorSw1:
        {
            PRINTF("\t[a]. 1.0V\r\n");
            PRINTF("\t[b]. 1.1V\r\n");
            PRINTF("\t[c]. 1.2V\r\n");
            PRINTF("\t[d]. 1.3V\r\n");
            PRINTF("\t[e]. 1.4V\r\n");
            PRINTF("\t[f]. 1.5V\r\n");
            PRINTF("\t[s]. Skip this step\r\n");
            for (;;)
            {
                chInput = GETCHAR();
                PUTCHAR(chInput);
                PRINTF("\r\n");
                if (((chInput >= 'a') && (chInput <= 'f')) || (chInput == 's'))
                {
                    break;
                }
                PRINTF("Wrong Input, please retry.\r\n");
            }

            if (chInput == 's')
            {
                if (runMode)
                {
                    (void)PF5020_ReadReg(&g_pf5020Handle, PF5020_SW1_RUN_VOLT, &voltageValue);
                }
                else
                {
                    (void)PF5020_ReadReg(&g_pf5020Handle, PF5020_SW1_STBY_VOLT, &voltageValue);
                }
            }
            else
            {
                voltageValue = 96U + (chInput - 'a') * 16U;
            }
            break;
        }

        case kPF5020_BuckRegulatorSw2:
        {
            PRINTF("\t[a]. 1.0V\r\n");
            PRINTF("\t[b]. 1.1V\r\n");
            PRINTF("\t[c]. 1.2V\r\n");
            PRINTF("\t[d]. 1.3V\r\n");
            PRINTF("\t[e]. 1.4V\r\n");
            PRINTF("\t[f]. 1.5V\r\n");
            PRINTF("\t[g]. 1.8V\r\n");
            PRINTF("\t[s]. Skip this step\r\n");
            for (;;)
            {
                chInput = GETCHAR();
                PUTCHAR(chInput);
                PRINTF("\r\n");
                if (((chInput >= 'a') && (chInput <= 'g')) || (chInput == 's'))
                {
                    break;
                }
                PRINTF("Wrong Input, please retry.\r\n");
            }

            if (chInput == 's')
            {
                if (runMode)
                {
                    (void)PF5020_ReadReg(&g_pf5020Handle, PF5020_SW2_RUN_VOLT, &voltageValue);
                }
                else
                {
                    (void)PF5020_ReadReg(&g_pf5020Handle, PF5020_SW2_STBY_VOLT, &voltageValue);
                }
            }
            else if (chInput == 'g')
            {
                voltageValue = 177U;
            }
            else
            {
                voltageValue = 96U + (chInput - 'a') * 16U;
            }
            break;
        }

        case kPF5020_BuckRegulatorSwnd1:
        {
            PRINTF("\t[a]. 2.8V\r\n");
            PRINTF("\t[b]. 3.15V\r\n");
            PRINTF("\t[c]. 3.2V\r\n");
            PRINTF("\t[d]. 3.25V\r\n");
            PRINTF("\t[e]. 3.3V\r\n");
            PRINTF("\t[f]. 3.35V\r\n");
            PRINTF("\t[g]. 3.4V\r\n");
            PRINTF("\t[h]. 3.5V\r\n");
            PRINTF("\t[i]. 3.8V\r\n");
            PRINTF("\t[j]. 4.0V\r\n");
            PRINTF("\t[s]. Skip this step\r\n");
            for (;;)
            {
                chInput = GETCHAR();
                PUTCHAR(chInput);
                PRINTF("\r\n");
                if (((chInput >= 'a') && (chInput <= 'j')) || (chInput == 's'))
                {
                    break;
                }
                PRINTF("Wrong Input, please retry.\r\n");
            }
            voltageValue = 17U + (chInput - 'a');
            break;
        }

        case kPF5020_RegulatorLdo1:
        {
            PRINTF("\t[a]. 1.5V\r\n");
            PRINTF("\t[b]. 1.6V\r\n");
            PRINTF("\t[c]. 1.8V\r\n");
            PRINTF("\t[d]. 1.85V\r\n");
            PRINTF("\t[e]. 2.15V\r\n");
            PRINTF("\t[f]. 2.5V\r\n");
            PRINTF("\t[g]. 2.8V\r\n");
            PRINTF("\t[h]. 3.0V\r\n");
            PRINTF("\t[i]. 3.1V\r\n");
            PRINTF("\t[j]. 3.15V\r\n");
            PRINTF("\t[k]. 3.2V\r\n");
            PRINTF("\t[l]. 3.3V\r\n");
            PRINTF("\t[m]. 3.35V\r\n");
            PRINTF("\t[s]. Skip this step\r\n");
            for (;;)
            {
                chInput = GETCHAR();
                PUTCHAR(chInput);
                PRINTF("\r\n");
                if (((chInput >= 'a') && (chInput <= 'm')) || (chInput == 's'))
                {
                    break;
                }
                PRINTF("Wrong Input, please retry.\r\n");
            }

            if (chInput == 's')
            {
                if (runMode)
                {
                    (void)PF5020_ReadReg(&g_pf5020Handle, PF5020_LDO1_RUN_VOLT, &voltageValue);
                }
                else
                {
                    (void)PF5020_ReadReg(&g_pf5020Handle, PF5020_LDO1_STBY_VOLT, &voltageValue);
                }
            }
            else
            {
                voltageValue = chInput - 'a';
            }
            break;
        }
    }

    return voltageValue;
}

static uint8_t DEMO_GetOperateMode(pf5020_regulator_name_t regulatorName, bool runMode)
{
    uint8_t chInput;
    uint8_t operateMode = 0U;
    uint8_t tmp8;

    if (regulatorName != kPF5020_RegulatorLdo1)
    {
        PRINTF("\t[a]. Turn off regulator\r\n");
        PRINTF("\t[b]. PWM mode\r\n");
        PRINTF("\t[c]. PFM mode\r\n");
        PRINTF("\t[d]. Auto skip mode\r\n");
        PRINTF("\t[s]. Skip this step\r\n");

        for (;;)
        {
            chInput = GETCHAR();
            PUTCHAR(chInput);
            PRINTF("\r\n");
            if (((chInput >= 'a') && (chInput <= 'd')) || (chInput == 's'))
            {
                break;
            }
            PRINTF("Wrong Input, please retry...\r\n");
        }
    }
    else
    {
        PRINTF("\t[a]. Turn off LDO1\r\n");
        PRINTF("\t[b]. Turn on LDO1\r\n");
        PRINTF("\t[s]. Skip this step\r\n");
        for (;;)
        {
            chInput = GETCHAR();
            PUTCHAR(chInput);
            PRINTF("\r\n");
            if (((chInput >= 'a') && (chInput <= 'b')) || (chInput == 's'))
            {
                break;
            }
            PRINTF("Wrong Input, please retry...\r\n");
        }
    }
    switch (regulatorName)
    {
        case kPF5020_BuckRegulatorSw1:
        case kPF5020_BuckRegulatorSw2:
        case kPF5020_BuckRegulatorSwnd1:
        {
            if (chInput == 's')
            {
                (void)PF5020_ReadReg(&g_pf5020Handle, g_regulatorModeReg[(uint8_t)regulatorName], &tmp8);
                if (runMode)
                {
                    operateMode = (uint8_t)(tmp8 & 0x3U);
                }
                else
                {
                    operateMode = (uint8_t)((tmp8 & 0xCU) >> 2U);
                }
            }
            else
            {
                operateMode = (uint8_t)(kPF5020_BuckRegulatorOff + (chInput - 'a'));
            }
            break;
        }
        case kPF5020_RegulatorLdo1:
        {
            if (chInput == 's')
            {
                (void)PF5020_ReadReg(&g_pf5020Handle, PF5020_LDO1_CONFIG2, &tmp8);
                if (runMode)
                {
                    operateMode = (uint8_t)((tmp8 & 0x2U) >> 1U);
                }
                else
                {
                    operateMode = (uint8_t)(tmp8 & 0x1U);
                }
            }
            else
            {
                operateMode = (uint8_t)(chInput - 'a');
            }
            break;
        }
        default:
            break;
    }

    return operateMode;
}

static void DEMO_DumpRegisterValue(void)
{
    uint8_t chInput;
    uint8_t startReg;
    uint8_t regCount;

    while (1)
    {
        PRINTF("\r\n------ Dump PF5020 Register Value ---------\r\n");
        PRINTF("Please input start register(0-119):");
        startReg = 0U;
        for (;;)
        {
            chInput = GETCHAR();
            PUTCHAR(chInput);
            if (chInput == 0x0DU)
            {
                break;
            }
            startReg = startReg * (10U) + (chInput - '0');
        }

        PRINTF("\r\n");
        if (startReg > 119U)
        {
            PRINTF("Start Register Overrange, please retry...\r\n");
        }
        else
        {
            PRINTF("Please input the count of registers to dump:");
            regCount = 0U;
            for (;;)
            {
                chInput = GETCHAR();
                PUTCHAR(chInput);
                if (chInput == 0x0DU)
                {
                    break;
                }
                regCount = regCount * 10U + (chInput - '0');
            }
            PRINTF("\r\n");

            if ((startReg + regCount) > 119U)
            {
                PRINTF("End Register Overrange, please retry...\r\n");
            }
            else
            {
                break;
            }
        }
    }

    (void)PF5020_DumpReg(&g_pf5020Handle, startReg, &regBuffer[startReg], regCount);

    for (uint8_t i = startReg; i < (startReg + regCount); i++)
    {
        PRINTF("Reg Addr: 0x%x, Value: 0x%x\r\n", i, (uint32_t)regBuffer[i]);
    }
}

static void DEMO_TuneClockFreq(void)
{
    uint8_t chInput;
    pf5020_high_speed_clk_config_t config;

    PRINTF("\r\n--------- Tune Internal High Speed Clock ---------\r\n");
    PRINTF("\t[a]. 20MHz Frequency\r\n");
    PRINTF("\t[b]. 21MHz Frequency\r\n");
    PRINTF("\t[c]. 22MHz Frequency\r\n");
    PRINTF("\t[d]. 23MHz Frequency\r\n");
    PRINTF("\t[e]. 24MHz Frequency\r\n");
    PRINTF("\t[j]. 16MHz Frequency\r\n");
    PRINTF("\t[k]. 17MHz Frequency\r\n");
    PRINTF("\t[l]. 18MHz Frequency\r\n");
    PRINTF("\t[m]. 19MHz Frequency\r\n");
    PRINTF("\t[s]. Skip this step\r\n");

    for (;;)
    {
        chInput = GETCHAR();
        PUTCHAR(chInput);
        PRINTF("\r\n");
        if (((chInput >= 'a') && (chInput <= 'e')) || ((chInput >= 'j') && (chInput <= 'm')) || (chInput == 's'))
        {
            break;
        }

        PRINTF("Wrong Input, please retry...\r\n");
    }

    if (chInput == 's')
    {
        return;
    }
    else
    {
        config.clkFreq  = (pf5020_high_speed_clk_freq_t)(chInput - 'a');
        config.enableSS = false;
        config.ssRange  = kPF5020_HighSpeedClkSSRange1;
        (void)PF5020_EnableInterrupts(&g_pf5020Handle, kPF5020_IntStatus1_FreqRdyInterrupt);
        (void)PF5020_CLK_ConfigHighSpeedClock(&g_pf5020Handle, &config);
    }
}

static void DEMO_EnterStandbyState(void)
{
    PRINTF("\r\n----------- Enter Standby State -----------\r\n");
    PRINTF("\r\nIn standby state, internal regulators work as the setting in Regulators Setting Menu\r\n");
    PRINTF("\r\nPlease press any key to set PF5020 to standby state.\r\n");
    GETCHAR();
    PGMC_PPC_TriggerPMICStandbySoftMode(PGMC_PPC0, true);
    PRINTF("\r\nPlease press any key to set PF5020 to run state.\r\n");
    GETCHAR();
    PGMC_PPC_TriggerPMICStandbySoftMode(PGMC_PPC0, false);
}
