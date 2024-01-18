/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "srtm_heap.h"
#include "srtm_pwm_service.h"
#include "fsl_common.h"
#include "fsl_adapter_pwm.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef SRTM_PWM_MAX_CHIP_NUM
#define SRTM_PWM_MAX_CHIP_NUM (9U)
#endif
#ifndef SRTM_PWM_MAX_CHANNEL
#define SRTM_PWM_MAX_CHANNEL (6U)
#endif

#define PWM_ENABLE            (1U)
#define PWM_POLARITY_NORMAL   (0U)
#define PWM_POLARITY_INVERSED (1U)
#define SECOND_TO_NANOSECOND  (1000000000ULL)

typedef struct _srtm_hal_pwm_setup_config
{
    hal_pwm_setup_config_t pwmConfig;
    uint64_t period;
    uint64_t dutyCycle;
} *srtm_hal_pwm_setup_config_t;

typedef struct _srtm_hal_pwm_adapter
{
    struct _srtm_pwm_adapter adapter;
    hal_pwm_handle_t halPwmHandle[SRTM_PWM_MAX_CHIP_NUM];
    struct _srtm_hal_pwm_setup_config halPwmConfig[SRTM_PWM_MAX_CHIP_NUM][SRTM_PWM_MAX_CHANNEL];
} *srtm_hal_pwm_adapter_t;

/*******************************************************************************
 * Code
 ******************************************************************************/
static srtm_status_t setPwm(srtm_hal_pwm_adapter_t adapter,
                            uint8_t chipId,
                            uint8_t channelId,
                            uint64_t period,
                            uint64_t dutyCycle,
                            uint8_t polarity,
                            uint8_t enable)
{
    uint8_t dutyCyclePercent;
    uint32_t pwmFreq_Hz = SECOND_TO_NANOSECOND / period;
    hal_pwm_mode_t mode = kHAL_CenterAlignedPwm;
    hal_pwm_level_select_t level;

    dutyCyclePercent = dutyCycle * 100 / period;

    if (enable == PWM_ENABLE)
    {
        if (polarity == PWM_POLARITY_NORMAL)
            level = kHAL_PwmHighTrue;
        else
            level = kHAL_PwmLowTrue;
    }
    else
    {
        level = kHAL_PwmNoPwmSignal;
    }

    adapter->halPwmConfig[chipId][channelId].period                     = period;
    adapter->halPwmConfig[chipId][channelId].dutyCycle                  = dutyCycle;
    adapter->halPwmConfig[chipId][channelId].pwmConfig.level            = level;
    adapter->halPwmConfig[chipId][channelId].pwmConfig.dutyCyclePercent = dutyCyclePercent;
    adapter->halPwmConfig[chipId][channelId].pwmConfig.pwmFreq_Hz       = pwmFreq_Hz;
    adapter->halPwmConfig[chipId][channelId].pwmConfig.mode             = mode;

    return HAL_PwmSetupPwm(adapter->halPwmHandle[chipId], channelId,
                           &adapter->halPwmConfig[chipId][channelId].pwmConfig) == kStatus_HAL_PwmSuccess ?
               SRTM_Status_Success :
               SRTM_Status_Error;
}

static void getPwm(srtm_hal_pwm_adapter_t adapter,
                   uint8_t chipId,
                   uint8_t channelId,
                   uint64_t *period,
                   uint64_t *dutyCycle,
                   uint8_t *polarity,
                   uint8_t *enable)
{
    if (adapter->halPwmConfig[chipId][channelId].pwmConfig.level != kHAL_PwmNoPwmSignal)
    {
        *enable = PWM_ENABLE;
        if (adapter->halPwmConfig[chipId][channelId].pwmConfig.level == kHAL_PwmHighTrue)
            *polarity = PWM_POLARITY_NORMAL;
        else
            *polarity = PWM_POLARITY_INVERSED;
    }
    else
    {
        *enable   = !PWM_ENABLE; /* No PWM output on pin */
        *polarity = PWM_POLARITY_NORMAL;
    }

    *period    = adapter->halPwmConfig[chipId][channelId].period;
    *dutyCycle = adapter->halPwmConfig[chipId][channelId].dutyCycle;
}

static srtm_status_t SRTM_PwmAdapter_GetPwm(srtm_pwm_adapter_t adapter,
                                            uint8_t chipId,
                                            uint8_t channelId,
                                            uint64_t *period,
                                            uint64_t *dutyCycle,
                                            uint8_t *polarity,
                                            uint8_t *enable)
{
    srtm_hal_pwm_adapter_t handle = (srtm_hal_pwm_adapter_t)(void *)adapter;

    if (chipId >= SRTM_PWM_MAX_CHIP_NUM)
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%s: chipId %d must less than max instance %d\r\n", __func__,
                           chipId, SRTM_PWM_MAX_CHIP_NUM);
        return SRTM_Status_Error;
    }
    if (channelId >= SRTM_PWM_MAX_CHANNEL)
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%s: channelId %d must less than max channel %d\r\n", __func__,
                           channelId, SRTM_PWM_MAX_CHANNEL);
        return SRTM_Status_Error;
    }
    if (handle->halPwmHandle[chipId] == NULL)
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%s: chipId %d not supported\r\n", __func__, chipId);
        return SRTM_Status_Error;
    }

    getPwm(handle, chipId, channelId, period, dutyCycle, polarity, enable);

    return SRTM_Status_Success;
}

static srtm_status_t SRTM_PwmAdapter_SetPwm(srtm_pwm_adapter_t adapter,
                                            uint8_t chipId,
                                            uint8_t channelId,
                                            uint64_t period,
                                            uint64_t dutyCycle,
                                            uint8_t polarity,
                                            uint8_t enable)
{
    srtm_hal_pwm_adapter_t handle = (srtm_hal_pwm_adapter_t)(void *)adapter;

    if (chipId >= SRTM_PWM_MAX_CHIP_NUM)
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%s: chipId %d must less than max instance %d\r\n", __func__,
                           chipId, SRTM_PWM_MAX_CHIP_NUM);
        return SRTM_Status_Error;
    }
    if (channelId >= SRTM_PWM_MAX_CHANNEL)
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%s: channelId %d must less than max channel %d\r\n", __func__,
                           channelId, SRTM_PWM_MAX_CHANNEL);
        return SRTM_Status_Error;
    }
    if (handle->halPwmHandle[chipId] == NULL)
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%s: chipId %d not supported\r\n", __func__, chipId);
        return SRTM_Status_Error;
    }
    if (period == 0 || period < dutyCycle)
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%s: Invalid period/dutyCycle %lld/%lld\r\n", __func__, period,
                           dutyCycle);
        return SRTM_Status_Error;
    }

    return setPwm(handle, chipId, channelId, period, dutyCycle, polarity, enable);
}

srtm_pwm_adapter_t SRTM_PwmAdapter_Create(hal_pwm_handle_t *handles, uint32_t handleNum)
{
    srtm_hal_pwm_adapter_t handle;
    uint32_t i;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    assert(handles != NULL);
    assert(handleNum > 0U);
    assert(handleNum <= SRTM_PWM_MAX_CHIP_NUM);

    handle = (srtm_hal_pwm_adapter_t)SRTM_Heap_Malloc(sizeof(struct _srtm_hal_pwm_adapter));
    assert(handle);

    memset(handle, 0, sizeof(struct _srtm_hal_pwm_adapter));
    for (i = 0U; i < handleNum; i++)
    {
        handle->halPwmHandle[i] = handles[i];
    }

    /* Adapter interfaces. */
    handle->adapter.getPwm = SRTM_PwmAdapter_GetPwm;
    handle->adapter.setPwm = SRTM_PwmAdapter_SetPwm;

    return &handle->adapter;
}

void SRTM_PwmAdapter_Destroy(srtm_pwm_adapter_t adapter)
{
    assert(adapter);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    SRTM_Heap_Free(adapter);
}
