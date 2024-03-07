/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018,2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "TSI_key.h"
#include "fsl_tsi_v5.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t g_keyId = 0U;
#if (FSL_FEATURE_SOC_TSI_COUNT == 1)
static uint8_t tsi_currentmode[FSL_FEATURE_SOC_TSI_COUNT] = {kTSI_InvalidMode};
#elif (FSL_FEATURE_SOC_TSI_COUNT == 2)
static uint8_t tsi_currentmode[FSL_FEATURE_SOC_TSI_COUNT] = {kTSI_InvalidMode, kTSI_InvalidMode};
#else
#error "Need define the tsi_currentmode"
#endif
static uint8_t g_tsi_sample_cnt = 0;
uint8_t g_key_baseline_freq     = BASELINE_UPDATE_FREQ;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Initiate TSI module to self-mode.
 *
 * This function is used to initiate TSI moudle,set charge current/voltage,
 * oscillator frequency, track TSI baseline through TSI calibration function.
 * @param base    Pointer to the TSI param structure
 */
void TSI_Init_SelfMode(TSI_Type *base)
{
    tsi_selfCap_config_t config;

    uint32_t instance = 0U;
    instance          = TSI_GetInstance(base);

    if (tsi_currentmode[instance] == kTSI_SelfMode)
    {
        return;
    }

    tsi_currentmode[instance] = kTSI_SelfMode;

    TSI_GetSelfCapModeDefaultConfig(&config);
    config.commonConfig.mode        = kTSI_SensingModeSlection_Self;
    config.commonConfig.mainClock   = kTSI_MainClockSlection_1;   // 16.65MHz
    config.commonConfig.dvolt       = kTSI_DvoltOption_0;         /* DVOLT option value  */
    config.commonConfig.cutoff      = kTSI_SincCutoffDiv_1;       /* Cutoff divider  */
    config.commonConfig.order       = kTSI_SincFilterOrder_2;     /* SINC filter order */
    config.commonConfig.decimation  = kTSI_SincDecimationValue_8; /* SINC decimation value */
    config.commonConfig.chargeNum   = kTSI_SscChargeNumValue_7;   /* SSC output bit0's period setting */
    config.commonConfig.noChargeNum = kTSI_SscNoChargeNumValue_5; /* SSC output bit1's period setting */

    config.enableSensitivity = 0;                                 /* Enable sensitivity boost of self-cap or not */
    config.xdn               = kTSI_SensitivityXdnOption_1;       /* Sensitivity XDN option   */
    config.ctrim             = kTSI_SensitivityCtrimOption_7;     /* Sensitivity CTRIM option */
    config.inputCurrent      = kTSI_CurrentMultipleInputValue_0;  /* Input current multiple   */
    config.chargeCurrent     = kTSI_CurrentMultipleChargeValue_1; /* Charge/Discharge current multiple */

    /* If the TSI moudle has been enabled before, the TSI_InitXxxMode will disable the moudle and re-config TSI IP. */
    TSI_InitSelfCapMode(base, &config);

    TSI_EnableModule(base, true);
}

/*!
 * @brief Initiate TSI module to mutual-mode.
 *
 * This function is used to initiate TSI moudle,set charge current/voltage,
 * oscillator frequency, track TSI baseline through TSI calibration function.
 * @param base    Pointer to the TSI param structure
 */
void TSI_Init_MutualMode(TSI_Type *base)
{
    tsi_mutualCap_config_t config;

    uint32_t instance = 0U;
    instance          = TSI_GetInstance(base);
    if (tsi_currentmode[instance] == kTSI_MutualMode)
    {
        return;
    }

    tsi_currentmode[instance] = kTSI_MutualMode;

    TSI_GetMutualCapModeDefaultConfig(&config);
    config.commonConfig.mode        = kTSI_SensingModeSlection_Mutual;
    config.commonConfig.mainClock   = kTSI_MainClockSlection_1;
    config.commonConfig.dvolt       = kTSI_DvoltOption_0;         /* DVOLT option value  */
    config.commonConfig.cutoff      = kTSI_SincCutoffDiv_1;       /* Cutoff divider  */
    config.commonConfig.order       = kTSI_SincFilterOrder_1;     /* SINC filter order */
    config.commonConfig.decimation  = kTSI_SincDecimationValue_8; /* SINC decimation value */
    config.commonConfig.chargeNum   = kTSI_SscChargeNumValue_4;   /* SSC output bit0's period setting */
    config.commonConfig.noChargeNum = kTSI_SscNoChargeNumValue_2; /* SSC output bit1's period setting */

    config.preCurrent    = kTSI_MutualPreCurrent_4uA;
    config.preResistor   = kTSI_MutualPreResistor_4k;
    config.senseResistor = kTSI_MutualSenseResistor_10k;

    config.boostCurrent     = kTSI_MutualSenseBoostCurrent_0uA;
    config.txDriveMode      = kTSI_MutualTxDriveModeOption_0;
    config.pmosLeftCurrent  = kTSI_MutualPmosCurrentMirrorLeft_32;
    config.pmosRightCurrent = kTSI_MutualPmosCurrentMirrorRight_1;
    config.nmosCurrent      = kTSI_MutualNmosCurrentMirror_1;

    TSI_InitMutualCapMode(base, &config);

    TSI_EnableModule(base, true);
}

/*!
 * @brief: Detect self-mode touch key
 *
 * @param current_key_id  return valid touched key id, ranges from 0 - 99.
 * @return return key event, which is the same as TSI event.
 */
uint8_t TSI_KeyDetect(uint8_t *current_key_id)
{
    uint8_t TSI_channel, TSI_channel_rx;
    uint16_t TSI_sampleResult = 0U;
    uint32_t key_touch_threshold;
    uint16_t key_value_average;
    uint8_t delay_i = 0U;
    uint8_t delay_j = 0U;

    /* The variables with suffix _temp will be saved back to key_TSI[] array.*/
    uint8_t key_state_temp;
    uint8_t key_state_debounce_temp;
    uint16_t key_baseline_temp;
    uint32_t key_value_sum_temp;

    key_event_t key_event = kKey_Event_Idle;

    key_tsi_mapping_t *key;

    key = &key_TSI[g_keyId];

    if (key->TSI_CH.TSI_channel == 0xFFU)
    {
        /* Reach the end of key_TSI[] array. Re-start the TSI scan from the first key */
        g_keyId = 0U;
        key     = &key_TSI[g_keyId];

        /* record the total rounds of TSI scan */
        g_tsi_sample_cnt++;
    }

    TSI_channel = key->TSI_CH.TSI_channel; /* TSI channel for self-mode or TX channel for mutual mode */

    key_state_temp          = key->key_state;
    key_state_debounce_temp = key->key_state_debounce;
    key_baseline_temp       = key->key_baseline;
    key_value_sum_temp      = key->key_value_sum;
    TSI_Type *base          = key->TSI_base;

    /* TSI_channel_rx = 0xFF indicates TSI self mode */
    if (key->TSI_channel_rx == 0xFFU)
    {
        TSI_Init_SelfMode(base);
        TSI_SetSelfCapMeasuredChannel(base, TSI_channel);
        TSI_StartSoftwareTrigger(base);

        while (!(TSI_GetStatusFlags(base) & kTSI_EndOfScanFlag))
        {
            /* Start time-out counter */
            delay_j++;
            if (delay_j >= 100U)
            {
                delay_j = 0U;
                delay_i++;
            }

            if (delay_i >= 100U)
            {
                /* If TSI scan do NOT ends after some time, force this round of TSI scan to an end */
                return 0xFFU;
            }
        }

        TSI_sampleResult = 0xFFFFU - TSI_GetCounter(base);
        key_value_sum_temp += TSI_sampleResult;
        TSI_ClearStatusFlags(base, kTSI_EndOfScanFlag);
    }
    else
    {
        /* TSI mutual mode */
        TSI_Init_MutualMode(base);

        TSI_channel    = key->TSI_CH.TSI_channel_tx - kTSI_Chnl_0;
        TSI_channel_rx = key->TSI_channel_rx - kTSI_Chnl_6;

        TSI_SetMutualCapTxChannel(base, (tsi_mutual_tx_channel_t)TSI_channel);
        TSI_SetMutualCapRxChannel(base, (tsi_mutual_rx_channel_t)TSI_channel_rx);
        TSI_StartSoftwareTrigger(base);

        while (!(TSI_GetStatusFlags(base) & kTSI_EndOfScanFlag))
        {
            /* Start time-out counter */
            delay_j++;
            if (delay_j >= 100U)
            {
                delay_j = 0U;
                delay_i++;
            }

            if (delay_i >= 100U)
            {
                /* If TSI scan do NOT ends after some time, force this round of TSI scan to an end */
                return 0xFFU;
            }
        }

        TSI_sampleResult = TSI_GetCounter(base);
        key_value_sum_temp += TSI_sampleResult;
        TSI_ClearStatusFlags(base, kTSI_EndOfScanFlag);
    }

    if (g_tsi_sample_cnt % g_key_baseline_freq == 0)
    {
        /* calculate the average value and update baseline in idle state process */
        key_value_average  = (uint16_t)(key_value_sum_temp / g_key_baseline_freq);
        key_value_sum_temp = 0;
    }

    key_touch_threshold = (uint32_t)((float)key_baseline_temp * (1.0F + key->key_touch_delta));

    if (key_state_temp == kKey_State_Idle)
    {
        if (TSI_sampleResult > key_touch_threshold)
        {
            /* There's a finger touch */
            key_state_debounce_temp++;

            if (key_state_debounce_temp > MAX_TOUCH_DEBOUNCE)
            {
                key_state_debounce_temp = 0U;
                key_event               = kKey_Event_Touch; /*Indicates the TSI channel is touched*/
                key_state_temp          = kKey_State_Touched;
            }
        }
        else /* Idle state, update baseline at baseline_freq. */
        {
            if (g_tsi_sample_cnt % g_key_baseline_freq == 0)
            {
                /* update baseline when: 1. follow the new average value, 2. idle key state. */
                if (key_value_average > key_baseline_temp)
                {
                    key_baseline_temp++;
                }
                else if (key_value_average < key_baseline_temp)
                {
                    key_baseline_temp--;
                }
                else
                {
                }
            }
            key_state_debounce_temp = 0U;
        }
    }
    else if (key_state_temp == kKey_State_Touched)
    {
        key_event = kKey_Event_Touched; /* Indicates the TSI channel is touched*/

        if (TSI_sampleResult > key_touch_threshold)
        {
            /* Indicates the TSI channel is Stick Touched */
            key_state_debounce_temp = 0U;
        }
        else
        {
            /* Condition: finger touch release */
            key_state_debounce_temp++;

            if (key_state_debounce_temp > MAX_TOUCH_DEBOUNCE)
            {
                key_state_debounce_temp = 0U;
                key_event               = kKey_Event_Release;
                key_state_temp          = kKey_State_Released;
            }
        }
    }
    else if (key_state_temp == kKey_State_Released)
    {
        /* Restore kKey_State_Idle automatically */
        key_event = kKey_Event_Released;

        key_state_debounce_temp++;

        if (key_state_debounce_temp > 10U)
        {
            /* Delay to assure this key is sent */
            key_state_debounce_temp = 0U;
            key_event               = kKey_Event_Idle;
            key_state_temp          = kKey_State_Idle;
        }
    }
    else
    {
        /* Key state can NOT go here */
        key_event      = kKey_Event_Idle;
        key_state_temp = kKey_State_Idle;
    }

    key->key_state          = key_state_temp;
    key->key_value_current  = TSI_sampleResult;
    key->key_baseline       = key_baseline_temp;
    key->key_state_debounce = key_state_debounce_temp;
    key->key_value_sum      = key_value_sum_temp;

    /* Sample the next key */
    *current_key_id = g_keyId;
    g_keyId++;

    return key_event;
}

/*
 * @brief Initialize TSI IP.
 *
 * This function calibrates TSI idle value, save as TSI baseline
 */
void TSI_KeyInit(void)
{
    uint8_t key_id = 0U;
    TSI_Type *base = NULL;

    uint8_t tsi_sample_cnt, max_tsi_sample_cnt = 8;
    uint32_t tsi_sample_result, tsi_sample_result_sum;

    /* Initial baseline for each touch key in self/mutual modes */
    while (key_TSI[key_id].TSI_CH.TSI_channel_tx != 0xFFU)
    {
        /* Initiate key_TSI structure */
        key_TSI[key_id].key_state          = 0U;
        key_TSI[key_id].key_state_debounce = 0U;
        key_TSI[key_id].key_baseline       = 0U;
        key_TSI[key_id].key_value_current  = 0U;
        key_TSI[key_id].key_value_sum      = 0U;

        tsi_sample_result     = 0;
        tsi_sample_result_sum = 0;

        for (tsi_sample_cnt = 0; tsi_sample_cnt < max_tsi_sample_cnt; tsi_sample_cnt++)
        {
            base = key_TSI[key_id].TSI_base;
            if (key_TSI[key_id].TSI_channel_rx == 0xFFU) /* TSI_channel_rx=0xFF indicates the key is self mode */
            {
                TSI_Init_SelfMode(base);
                TSI_SetSelfCapMeasuredChannel(base, key_TSI[key_id].TSI_CH.TSI_channel);
                TSI_StartSoftwareTrigger(base);

                while (!(TSI_GetStatusFlags(base) & kTSI_EndOfScanFlag))
                {
                }

                tsi_sample_result = 0xFFFFU - TSI_GetCounter(base);
                tsi_sample_result_sum += tsi_sample_result;
                TSI_ClearStatusFlags(base, kTSI_EndOfScanFlag);
            }
            else
            {
                /* TSI mutual mode */
                assert((key_TSI[key_id].TSI_CH.TSI_channel_tx <= kTSI_Chnl_5) &&
                       (key_TSI[key_id].TSI_channel_rx >= kTSI_Chnl_6) &&
                       (key_TSI[key_id].TSI_channel_rx <= kTSI_Chnl_11));
                TSI_Init_MutualMode(base);
                TSI_SetMutualCapTxChannel(
                    base, (tsi_mutual_tx_channel_t)(key_TSI[key_id].TSI_CH.TSI_channel_tx - kTSI_Chnl_0));
                TSI_SetMutualCapRxChannel(base,
                                          (tsi_mutual_rx_channel_t)(key_TSI[key_id].TSI_channel_rx - kTSI_Chnl_6));
                TSI_StartSoftwareTrigger(base);

                while (!(TSI_GetStatusFlags(base) & kTSI_EndOfScanFlag))
                {
                }

                tsi_sample_result = TSI_GetCounter(base);
                tsi_sample_result_sum += tsi_sample_result;
                TSI_ClearStatusFlags(base, kTSI_EndOfScanFlag);
            }
        }

        key_TSI[key_id].key_baseline = (uint16_t)(tsi_sample_result_sum / max_tsi_sample_cnt);

        key_id++;
    }
}
