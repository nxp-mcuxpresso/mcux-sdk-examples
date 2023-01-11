/*
 * Copyright 2019-2020 NXP
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
#define PMIC_DECREASE_LVD_LEVEL_IF_HIGHER_THAN(currVolt, targetVolt) \
    do                                                               \
    {                                                                \
        if ((uint32_t)(currVolt) > (uint32_t)(targetVolt))           \
        {                                                            \
            POWER_SetLvdFallingTripVoltage(kLvdFallingTripVol_720);  \
        }                                                            \
    } while (0)

/*******************************************************************************
 * Variables
 ******************************************************************************/
pca9420_handle_t pca9420Handle;
static pca9420_modecfg_t pca9420CurrModeCfg;
static pca9420_mode_t pca9420CurrMode;
static const pca9420_sw1_out_t pca9420VoltLevel[5] = {
    kPCA9420_Sw1OutVolt1V150, kPCA9420_Sw1OutVolt1V000, kPCA9420_Sw1OutVolt0V900,
    kPCA9420_Sw1OutVolt0V800, kPCA9420_Sw1OutVolt0V700,
};
static bool pmicVoltChangedForDeepSleep;

/* Frequency levels defined in power library. */
extern const uint32_t powerLowCm33FreqLevel[2][3];
extern const uint32_t powerLowDspFreqLevel[2][3];
extern const uint32_t powerFullCm33FreqLevel[2][5];
extern const uint32_t powerFullDspFreqLevel[2][5];

/*******************************************************************************
 * Code
 ******************************************************************************/
static uint32_t BOARD_CalcVoltLevel(const uint32_t *freqLevels, uint32_t num, uint32_t freq)
{
    uint32_t i;
    uint32_t volt;

    for (i = 0; i < num; i++)
    {
        if (freq > freqLevels[i])
        {
            break;
        }
    }

    if (i == 0) /* Frequency exceed max supported */
    {
        volt = POWER_INVALID_VOLT_LEVEL;
    }
    else
    {
        volt = pca9420VoltLevel[i + ARRAY_SIZE(pca9420VoltLevel) - num - 1];
    }

    return volt;
}

void BOARD_InitPmic(void)
{
    pca9420_config_t pca9420Config;

    CLOCK_AttachClk(kSFRO_to_FLEXCOMM15);
    BOARD_PMIC_I2C_Init();
    PCA9420_GetDefaultConfig(&pca9420Config);
    pca9420Config.I2C_SendFunc    = BOARD_PMIC_I2C_Send;
    pca9420Config.I2C_ReceiveFunc = BOARD_PMIC_I2C_Receive;
    PCA9420_Init(&pca9420Handle, &pca9420Config);
}

bool BOARD_SetPmicVoltageForFreq(power_part_temp_range_t tempRange,
                                 power_volt_op_range_t voltOpRange,
                                 uint32_t cm33Freq,
                                 uint32_t dspFreq)
{
    power_lvd_falling_trip_vol_val_t lvdVolt;
    uint32_t idx = (uint32_t)tempRange;
    uint32_t cm33Volt, dspVolt, volt;
    bool ret;

    PCA9420_GetCurrentMode(&pca9420Handle, &pca9420CurrMode);
    PCA9420_ReadModeConfigs(&pca9420Handle, pca9420CurrMode, &pca9420CurrModeCfg, 1);

    lvdVolt = POWER_GetLvdFallingTripVoltage();

    /* Enter FBB mode first */
    if (POWER_GetBodyBiasMode(kCfg_Run) != kPmu_Fbb)
    {
        POWER_EnterFbb();
    }

    if (voltOpRange == kVoltOpLowRange)
    {
        cm33Volt = BOARD_CalcVoltLevel(&powerLowCm33FreqLevel[idx][0], 3U, cm33Freq);
        dspVolt  = BOARD_CalcVoltLevel(&powerLowDspFreqLevel[idx][0], 3U, dspFreq);
    }
    else
    {
        cm33Volt = BOARD_CalcVoltLevel(&powerFullCm33FreqLevel[idx][0], 5U, cm33Freq);
        dspVolt  = BOARD_CalcVoltLevel(&powerFullDspFreqLevel[idx][0], 5U, dspFreq);
    }

    volt = MAX(cm33Volt, dspVolt);
    ret  = volt != POWER_INVALID_VOLT_LEVEL;
    assert(ret);

    if (ret)
    {
        if (volt < kPCA9420_Sw1OutVolt0V800)
        {
            POWER_DisableLVD();
        }
        else
        {
            if (volt < kPCA9420_Sw1OutVolt0V900)
            {
                PMIC_DECREASE_LVD_LEVEL_IF_HIGHER_THAN(lvdVolt, kLvdFallingTripVol_795);
            }
            else if (volt < kPCA9420_Sw1OutVolt1V000)
            {
                PMIC_DECREASE_LVD_LEVEL_IF_HIGHER_THAN(lvdVolt, kLvdFallingTripVol_885);
            }
            else
            {
            }
        }

        /* Configure vddcore voltage value */
        pca9420CurrModeCfg.sw1OutVolt = (pca9420_sw1_out_t)volt;
        PCA9420_WriteModeConfigs(&pca9420Handle, pca9420CurrMode, &pca9420CurrModeCfg, 1);

        if (volt >= kPCA9420_Sw1OutVolt0V800)
        {
            POWER_RestoreLVD();
        }
    }

    return ret;
}

void BOARD_SetPmicVoltageBeforeDeepSleep(void)
{
    PCA9420_GetCurrentMode(&pca9420Handle, &pca9420CurrMode);
    PCA9420_ReadModeConfigs(&pca9420Handle, pca9420CurrMode, &pca9420CurrModeCfg, 1);

    if (pca9420CurrModeCfg.sw1OutVolt == kPCA9420_Sw1OutVolt0V700)
    {
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
        pca9420CurrModeCfg.sw1OutVolt = kPCA9420_Sw1OutVolt0V700;
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
