/*
 * Copyright 2023-2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "board.h"
#include "pmic_support.h"
#include "fsl_gpio.h"
#if BOARD_PMIC_CONFIG_USE_SEMA4
#include "fsl_sema42.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
pca9422_handle_t pca9422Handle;
static pca9422_power_mode_t pca9422CurrMode;
static pca9422_modecfg_t pca9422CurrModeCfg;

/*******************************************************************************
 * Code
 ******************************************************************************/
#if defined(PMC0) /*DVS Control pins are connected to Compute domain. */
void BOARD_InitPmicDVSPin(void)
{
    gpio_pin_config_t gpioPinConfigStruct;

    gpioPinConfigStruct.pinDirection = kGPIO_DigitalOutput;
    gpioPinConfigStruct.outputLogic  = 0U;

    RESET_ClearPeripheralReset(BOARD_DVS_CTRL_GPIO_RESET);
    CLOCK_EnableClock(BOARD_DVS_CTRL_GPIO_CLOCK);

    GPIO_PinInit(BOARD_DVS_CTRL_GPIO, BOARD_DVS_CTRL0_GPIO_PIN, &gpioPinConfigStruct);
    GPIO_PinInit(BOARD_DVS_CTRL_GPIO, BOARD_DVS_CTRL1_GPIO_PIN, &gpioPinConfigStruct);
    GPIO_PinInit(BOARD_DVS_CTRL_GPIO, BOARD_DVS_CTRL2_GPIO_PIN, &gpioPinConfigStruct);
}

uint8_t BOARD_GetPmicDVSPinStatus(void)
{
    uint8_t dvs_ctrl[3];
    uint8_t dvs_out;

    dvs_ctrl[0] = (BOARD_DVS_CTRL_GPIO->PDOR >> BOARD_DVS_CTRL0_GPIO_PIN) & 0x1U;
    dvs_ctrl[1] = (BOARD_DVS_CTRL_GPIO->PDOR >> BOARD_DVS_CTRL1_GPIO_PIN) & 0x1U;
    dvs_ctrl[2] = (BOARD_DVS_CTRL_GPIO->PDOR >> BOARD_DVS_CTRL2_GPIO_PIN) & 0x1U;
    dvs_out     = (dvs_ctrl[2] << 2) | (dvs_ctrl[1] << 1) | dvs_ctrl[0];

    return dvs_out;
}

void BOARD_SetPmicDVSPinStatus(uint8_t dvs_out)
{
    GPIO_PinWrite(BOARD_DVS_CTRL_GPIO, BOARD_DVS_CTRL0_GPIO_PIN, ((dvs_out & 0x01) ? 1U : 0U));
    GPIO_PinWrite(BOARD_DVS_CTRL_GPIO, BOARD_DVS_CTRL1_GPIO_PIN, ((dvs_out & 0x02) ? 1U : 0U));
    GPIO_PinWrite(BOARD_DVS_CTRL_GPIO, BOARD_DVS_CTRL2_GPIO_PIN, ((dvs_out & 0x04) ? 1U : 0U));
}
#endif

#if BOARD_PMIC_CONFIG_USE_SEMA4
static void BOARD_PmicCtrlSema4Init(void)
{
    RESET_ClearPeripheralReset(kSEMA420_RST_SHIFT_RSTn);
    CLOCK_EnableClock(kCLOCK_Sema420);
    SEMA42_ResetGate(BOARD_PMIC_I2C_SEMA42_BASE, BOARD_PMIC_I2C_SEMA42_GATE);
}

static void BOARD_PmicCtrlSema4Lock(void)
{
    SEMA42_Lock(BOARD_PMIC_I2C_SEMA42_BASE, BOARD_PMIC_I2C_SEMA42_GATE, BOARD_PMIC_I2C_SEMA42_ID);
}

static void BOARD_PmicCtrlSema4Unlock(void)
{
    SEMA42_Unlock(BOARD_PMIC_I2C_SEMA42_BASE, BOARD_PMIC_I2C_SEMA42_GATE);
}
#endif

void BOARD_InitPmic(void)
{
    pca9422_regulator_config_t pca9422RegConfig;
#if defined(PMC0) /* Only initialize I2C from Compute domain. */
    CLOCK_AttachClk(kSENSE_BASE_to_LPI2C15);
    CLOCK_SetClkDiv(kCLOCK_DivLpi2c15Clk, 2U);
    RESET_ClearPeripheralReset(kLPI2C15_RST_SHIFT_RSTn);
    BOARD_PMIC_I2C_Init();

    PCA9422_GetRegulatorDefaultConfig(&pca9422RegConfig);
    pca9422RegConfig.I2C_SendFunc    = BOARD_PMIC_I2C_Send;
    pca9422RegConfig.I2C_ReceiveFunc = BOARD_PMIC_I2C_Receive;

    /* BUCKxOUT_DVSx by DVS Pin in active mode and BUCKxOUT_SLEEP in sleep mode */
    pca9422RegConfig.buck[0].dvsCtrl = (uint8_t)kPCA9422_PinInActiveAndBxOUTSLEEPInSleep;
    pca9422RegConfig.buck[1].dvsCtrl = (uint8_t)kPCA9422_PinInActiveAndBxOUTSLEEPInSleep;
    pca9422RegConfig.buck[2].dvsCtrl = (uint8_t)kPCA9422_PinInActiveAndBxOUTSLEEPInSleep;

    PCA9422_InitRegulator(&pca9422Handle, &pca9422RegConfig);
    BOARD_InitPmicDVSPin();
#else
    CLOCK_EnableClock(kCLOCK_LPI2c15);
    PCA9422_GetRegulatorDefaultConfig(&pca9422RegConfig);
    pca9422Handle.I2C_SendFunc    = BOARD_PMIC_I2C_Send;
    pca9422Handle.I2C_ReceiveFunc = BOARD_PMIC_I2C_Receive;
    pca9422Handle.slaveAddress    = pca9422RegConfig.slaveAddress;
#endif
#if BOARD_PMIC_CONFIG_USE_SEMA4
    BOARD_PmicCtrlSema4Init(); /* Init the PMIC I2C SEMA42. */
#endif
}

void BOARD_SetPmicVoltageBeforeDeepPowerDown(void)
{
#if defined(PMC0)
    /*Wakeup from deep power down is same as POR, and need VDDCORE >= 1.0V. Otherwise
       0.9V LVD reset value may cause wakeup failure.*/
    BOARD_SetPmicDVSPinStatus(0x0);
#endif
}

void BOARD_SetPmicVdd2Voltage(uint32_t volt)
{
#if BOARD_PMIC_CONFIG_USE_SEMA4
    BOARD_PmicCtrlSema4Lock();
#endif
    PCA9422_GetCurrentPowerMode(&pca9422Handle, &pca9422CurrMode);
    PCA9422_ReadPowerModeConfigs(&pca9422Handle, pca9422CurrMode, &pca9422CurrModeCfg);
    pca9422CurrModeCfg.sw1OutVolt = volt;
    PCA9422_WritePowerModeConfigs(&pca9422Handle, pca9422CurrMode, pca9422CurrModeCfg);
#if BOARD_PMIC_CONFIG_USE_SEMA4
    BOARD_PmicCtrlSema4Unlock();
#endif
}

void BOARD_SetPmicVdd1Voltage(uint32_t volt)
{
#if BOARD_PMIC_CONFIG_USE_SEMA4
    BOARD_PmicCtrlSema4Lock();
#endif
    PCA9422_GetCurrentPowerMode(&pca9422Handle, &pca9422CurrMode);
    PCA9422_ReadPowerModeConfigs(&pca9422Handle, pca9422CurrMode, &pca9422CurrModeCfg);
    pca9422CurrModeCfg.sw3OutVolt = volt;
    PCA9422_WritePowerModeConfigs(&pca9422Handle, pca9422CurrMode, pca9422CurrModeCfg);
#if BOARD_PMIC_CONFIG_USE_SEMA4
    BOARD_PmicCtrlSema4Unlock();
#endif
}
