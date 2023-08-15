/*
 * Copyright 2019-2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "board.h"
#include "pmic_support.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
pca9420_handle_t pca9420Handle;
static pca9420_modecfg_t pca9420CurrModeCfg;
static pca9420_mode_t pca9420CurrMode;
static bool pmicVoltChangedForDeepSleep;
static pca9420_sw1_out_t pmicVoltValueBeforeChange;

/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_InitPmic(void)
{
    pca9420_config_t pca9420Config;

    CLOCK_AttachClk(kFRO_DIV4_to_FLEXCOMM15);
    BOARD_PMIC_I2C_Init();
    PCA9420_GetDefaultConfig(&pca9420Config);
    pca9420Config.I2C_SendFunc    = BOARD_PMIC_I2C_Send;
    pca9420Config.I2C_ReceiveFunc = BOARD_PMIC_I2C_Receive;
    PCA9420_Init(&pca9420Handle, &pca9420Config);
}

void BOARD_SetVddCoreVoltage(uint32_t millivolt)
{
    PCA9420_GetCurrentMode(&pca9420Handle, &pca9420CurrMode);
    PCA9420_ReadModeConfigs(&pca9420Handle, pca9420CurrMode, &pca9420CurrModeCfg, 1);

    pca9420CurrModeCfg.sw1OutVolt = (pca9420_sw1_out_t)((millivolt - 500U) / 25U);

    PCA9420_WriteModeConfigs(&pca9420Handle, pca9420CurrMode, &pca9420CurrModeCfg, 1);
}

bool BOARD_SetPmicVoltageForFreq(uint32_t cm33_clk_freq, uint32_t dsp_clk_freq)
{
    POWER_SetVddCoreSupplySrc(kVddCoreSrc_PMIC);
    POWER_SetPmicCoreSupplyFunc(BOARD_SetVddCoreVoltage);
    return POWER_SetVoltageForFreq(cm33_clk_freq, dsp_clk_freq, 0U);
}

void BOARD_SetPmicVoltageBeforeDeepSleep(void)
{
    PCA9420_GetCurrentMode(&pca9420Handle, &pca9420CurrMode);
    PCA9420_ReadModeConfigs(&pca9420Handle, pca9420CurrMode, &pca9420CurrModeCfg, 1);

    if (pca9420CurrModeCfg.sw1OutVolt <= kPCA9420_Sw1OutVolt0V700)
    {
        pmicVoltValueBeforeChange   = pca9420CurrModeCfg.sw1OutVolt;
        pmicVoltChangedForDeepSleep = true;
        /* On resume from deep sleep with external PMIC, LVD is always used even if we have already disabled it.
         * Here we need to set up a safe threshold to avoid LVD reset and interrupt. */
        POWER_SetLvdFallingTripVoltage(kLvdFallingTripVol_720);
        pca9420CurrModeCfg.sw1OutVolt = kPCA9420_Sw1OutVolt0V750;
        PCA9420_WriteModeConfigs(&pca9420Handle, pca9420CurrMode, &pca9420CurrModeCfg, 1);
    }
    else
    {
    }
}

void BOARD_RestorePmicVoltageAfterDeepSleep(void)
{
    if (pmicVoltChangedForDeepSleep)
    {
        PCA9420_GetCurrentMode(&pca9420Handle, &pca9420CurrMode);
        PCA9420_ReadModeConfigs(&pca9420Handle, pca9420CurrMode, &pca9420CurrModeCfg, 1);
        pca9420CurrModeCfg.sw1OutVolt = pmicVoltValueBeforeChange;
        PCA9420_WriteModeConfigs(&pca9420Handle, pca9420CurrMode, &pca9420CurrModeCfg, 1);
        pmicVoltChangedForDeepSleep = false;
    }
    else
    {
    }
}

void BOARD_SetPmicVoltageBeforeDeepPowerDown(void)
{
    PCA9420_GetCurrentMode(&pca9420Handle, &pca9420CurrMode);
    PCA9420_ReadModeConfigs(&pca9420Handle, pca9420CurrMode, &pca9420CurrModeCfg, 1);

    /* Wakeup from deep power down is same as POR, and need VDDCORE >= 1.0V. Otherwise
       0.9V LVD reset value may cause wakeup failure. */
    if (pca9420CurrModeCfg.sw1OutVolt < kPCA9420_Sw1OutVolt1V000)
    {
        pca9420CurrModeCfg.sw1OutVolt = kPCA9420_Sw1OutVolt1V000;
        PCA9420_WriteModeConfigs(&pca9420Handle, pca9420CurrMode, &pca9420CurrModeCfg, 1);
    }
    else
    {
    }
}
