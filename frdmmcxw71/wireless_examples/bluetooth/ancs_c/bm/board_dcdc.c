/* -------------------------------------------------------------------------- */
/*                           Copyright 2021-2024 NXP                          */
/*                            All rights reserved.                            */
/*                    SPDX-License-Identifier: BSD-3-Clause                   */
/* -------------------------------------------------------------------------- */

/*
 * @warning
 * This file is proposed for our connectivity demo applications. Customer is allowed to modify the DCDC setting /
 * configuration in active and low power mode to accomodate with their boards and application use cases. However, side
 * effects can happen if the DCDC configuration is not correctly set. As instance, following guideline is required :
 *  - DCDC switches from Normal drive strength to low drive strength  shall not have same DCDC output voltage
 *
 * In order to avoid this to happen, Customer has 2 possibilities :
 *
 *  - Keep same DCDC drive strength in low power and active modes such as low drive strength . This is valid only if the
 * device power consumption never exceeds 15mA (TX output to 0dbM and Main core frequency to 48Mhz maximum) - please
 * check your board configuration and application use case.
 *   Note that this is not recommended to set normal drive strength in low power mode due to high power consumption.
 *
 *  - OR, in active mode, set an output voltage higher (for instance, 1.35v or higher) than in low power mode (1.25v).
 *
 * In case of doubts, in any cases, please check the reference manual, datasheet, or contact your NXP representatives.
 */

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */
#include "board_dcdc.h"
#include "fsl_spc.h"

/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */
static const spc_lowpower_mode_regulators_config_t spcLpCfg = {
    .bandgapMode = kSPC_BandgapDisabled,
    .CoreIVS     = true,
    .lpBuff      = false,
    .lpIREF      = false,
    .CoreLDOOption =
        {
            .CoreLDODriveStrength = kSPC_CoreLDO_LowDriveStrength,
            // 1.0V
            .CoreLDOVoltage = kSPC_CoreLDO_MidDriveVoltage,
        },
    .SysLDOOption =
        {
            .SysLDODriveStrength = kSPC_SysLDO_LowDriveStrength,
        },
    .DCDCOption =
        {
            /* 15mA maximum */
            .DCDCDriveStrength = kSPC_DCDC_LowDriveStrength,
            // 1.25V
            .DCDCVoltage = kSPC_DCDC_LowUnderVoltage,
        },
};

/* -------------------------------------------------------------------------- */
/*                             Private prototypes                             */
/* -------------------------------------------------------------------------- */

#if defined(gBoardDcdcBuckMode_d) && (gBoardDcdcBuckMode_d == 1)
/*!
 * \brief Initializes DCDC for buck mode
 *
 */
static void BOARD_InitDcdcBuck(void);
#else
/*!
 * \brief Initializes DCDC for bypass mode
 *
 */
static void BOARD_InitDcdcBypass(void);
#endif /* gBoardDcdcBuckMode_d */

/*!
 * \brief Applies DCDC configuration based on desired voltage and drive strength
 *
 * \param[in] DCDCDriveStrength Desired drive strength (see spc_dcdc_drive_strength_t)
 * \param[in] DCDCVoltage Desired voltage (see spc_dcdc_voltage_level_t)
 * \param[in] enable_2p5v If true, enables 2.5V output instead of 1.8V
 */
static void BOARD_DCDC_config(spc_dcdc_drive_strength_t DCDCDriveStrength,
                              spc_dcdc_voltage_level_t  DCDCVoltage,
                              bool                      enable_2p5v);

/*!
 * \brief Enable/disable all LVD/HVD in Active
 *
 * \param[in] enable true to enable, false to disable
 */
static void BOARD_SetActiveVoltageDetect(bool enable);

/*!
 * \brief Enable/disable all LVD/HVD in Low Power
 *
 * \param[in] enable true to enable, false to disable
 */
static void BOARD_SetLowPowerVoltageDetect(bool enable);

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

void BOARD_InitDcdc(void)
{
#if defined(gBoardDcdcBuckMode_d) && (gBoardDcdcBuckMode_d == 1)
    BOARD_InitDcdcBuck();
#else
    BOARD_InitDcdcBypass();
#endif
}

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

#if defined(gBoardDcdcBuckMode_d) && (gBoardDcdcBuckMode_d == 1)
static void BOARD_InitDcdcBuck(void)
{
    status_t status;

    /* Enable/Disable Regulators */
    SPC_EnableSystemLDORegulator(SPC0, true);
    SPC_EnableDCDCRegulator(SPC0, true);
    SPC_EnableCoreLDORegulator(SPC0, true);

    /* Disable LVD/HVD in Low Power */
    BOARD_SetLowPowerVoltageDetect(false);

    /* Apply SPC low power config
     * it is used when the 3 power domains are in low power (main, wake, radio)
     * TODO: Shall we adjust wakeup delay to give SPC time to ramp up voltages from LP to ACTIVE ? */
    do
    {
        status = SPC_SetLowPowerModeRegulatorsConfig(SPC0, &spcLpCfg);
    } while (status == kStatus_SPC_Busy);
    assert(status == kStatus_Success);
    (void)status;

#if defined(gBoardDcdcFreqStabEnabled_d) && (gBoardDcdcFreqStabEnabled_d == 1)
    /*! Enable DCDC frequency stabilization to reduce peaks in Buck mode,
        DCDC will not manage current higher than 40mA */
    SPC0->DCDC_CFG |= SPC_DCDC_CFG_FREQ_CNTRL_ON_MASK;
#endif

#if defined(gAppMaxTxPowerDbm_c) && (gAppMaxTxPowerDbm_c <= 0)
#if defined(gAppHighSystemClockFrequency_d) && (gAppHighSystemClockFrequency_d >= 0)
    /* 0 dBm, 96MHz 1.35V  */
    /* A drop of 250mV is needed between DCDC output voltage and LDO core voltage, see datasheet */
    BOARD_DCDC_config(kSPC_DCDC_LowDriveStrength, kSPC_DCDC_MidVoltage, false);
#else
    /* 0 dBm, 48MHz, 1.25V - 15mA maximum on DCDC output*/
    BOARD_DCDC_config(kSPC_DCDC_LowDriveStrength, kSPC_DCDC_LowUnderVoltage, false);
#endif /* gAppHighSystemClockFrequency_d */
#elif defined(gAppMaxTxPowerDbm_c) && (gAppMaxTxPowerDbm_c <= 7)
    /* 7dBm, 1.8V */
    BOARD_DCDC_config(kSPC_DCDC_NormalDriveStrength, kSPC_DCDC_SafeModeVoltage, false);
#else
    /* 10 dBm, 2.5V */
    BOARD_DCDC_config(kSPC_DCDC_NormalDriveStrength, kSPC_DCDC_NormalVoltage, true);

#endif /* gAppMaxTxPowerDbm_c */
}

#else

static void BOARD_InitDcdcBypass(void)
{
    /* Disable DCDC regulator */
    SPC_EnableDCDCRegulator(SPC0, false);
}
#endif /* gBoardDcdcBuckMode_d */

static void BOARD_DCDC_config(spc_dcdc_drive_strength_t DCDCDriveStrength,
                              spc_dcdc_voltage_level_t  DCDCVoltage,
                              bool                      enable_2p5v)
{
    status_t                      status;
    spc_active_mode_dcdc_option_t spc_active_cfg_dcdc_option = {
        .DCDCDriveStrength = DCDCDriveStrength,
        .DCDCVoltage       = DCDCVoltage,
    };

    /* read current LVD/HVD config */
    uint32_t AM_VoltageDetectStatus = SPC_GetActiveModeVoltageDetectStatus(SPC0);
    uint32_t LP_VoltageDetectStatus = SPC_GetLowPowerModeVoltageDetectStatus(SPC0);

    /* Disable all LVD/HVD before any output voltage change
     * This is required to avoid any LVD/HVD reset during voltage changes */
    BOARD_SetActiveVoltageDetect(false);
    BOARD_SetLowPowerVoltageDetect(false);

    if (enable_2p5v == true)
    {
        /* Enable DCDC 2.5V output */
        SPC0->DCDC_CFG |= SPC_DCDC_CFG_VOUT2P5_SEL(0x1U);
        assert(DCDCDriveStrength == kSPC_DCDC_NormalDriveStrength);
        assert(DCDCVoltage == kSPC_DCDC_NormalVoltage);
    }
    else
    {
        /* Disable 2.5V output */
        SPC0->DCDC_CFG &= ~SPC_DCDC_CFG_VOUT2P5_SEL_MASK;
    }

    /* Apply DCDC voltage and drive strength */
    do
    {
        status = SPC_SetActiveModeDCDCRegulatorConfig(SPC0, &spc_active_cfg_dcdc_option);
    } while (status == kStatus_SPC_Busy);
    assert(status == kStatus_Success);

    /* prevent warning in release mode */
    (void)status;

    /* Restore default LVD/HVD config */
    SPC0->ACTIVE_CFG |= AM_VoltageDetectStatus;
    SPC0->LP_CFG |= LP_VoltageDetectStatus;
}

static void BOARD_SetActiveVoltageDetect(bool enable)
{
    (void)SPC_EnableActiveModeCoreLowVoltageDetect(SPC0, enable);
    (void)SPC_EnableActiveModeSystemLowVoltageDetect(SPC0, enable);
    (void)SPC_EnableActiveModeIOLowVoltageDetect(SPC0, enable);
    (void)SPC_EnableActiveModeCoreHighVoltageDetect(SPC0, enable);
    (void)SPC_EnableActiveModeSystemHighVoltageDetect(SPC0, enable);
    (void)SPC_EnableActiveModeIOHighVoltageDetect(SPC0, enable);
}

static void BOARD_SetLowPowerVoltageDetect(bool enable)
{
    (void)SPC_EnableLowPowerModeCoreHighVoltageDetect(SPC0, enable);
    (void)SPC_EnableLowPowerModeCoreLowVoltageDetect(SPC0, enable);
    (void)SPC_EnableLowPowerModeSystemHighVoltageDetect(SPC0, enable);
    (void)SPC_EnableLowPowerModeSystemLowVoltageDetect(SPC0, enable);
    (void)SPC_EnableLowPowerModeIOHighVoltageDetect(SPC0, enable);
    (void)SPC_EnableLowPowerModeIOLowVoltageDetect(SPC0, enable);
}
