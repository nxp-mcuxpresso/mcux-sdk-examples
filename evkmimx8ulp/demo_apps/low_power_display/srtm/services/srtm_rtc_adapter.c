/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_adapter_rtc.h"
#include "srtm_heap.h"
#include "srtm_rtc_service.h"
#include "srtm_rtc_adapter.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef struct _srtm_hal_rtc_adapter
{
    struct _srtm_rtc_adapter adapter;
    hal_rtc_handle_t halRtcHandle;
} *srtm_hal_rtc_adapter_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
static uint64_t rtcGetTime(srtm_hal_rtc_adapter_t adapter)
{
    return HAL_RtcGetTime(adapter->halRtcHandle);
}

static hal_rtc_status_t rtcSetTime(srtm_hal_rtc_adapter_t adapter, uint64_t microSeconds)
{
    return HAL_RtcSetTime(adapter->halRtcHandle, microSeconds);
}

static uint64_t rtcGetAlarm(srtm_hal_rtc_adapter_t adapter)
{
    return HAL_RtcGetAlarm(adapter->halRtcHandle);
}

static hal_rtc_status_t rtcSetAlarm(srtm_hal_rtc_adapter_t adapter, uint64_t microSeconds)
{
    return HAL_RtcSetAlarm(adapter->halRtcHandle, microSeconds);
}

uint32_t rtcGetEnabledInterrupts(srtm_hal_rtc_adapter_t adapter)
{
    return HAL_RtcGetEnabledInterrupts(adapter->halRtcHandle);
}

uint32_t rtcGetStatusFlags(srtm_hal_rtc_adapter_t adapter)
{
    return HAL_RtcGetStatusFlags(adapter->halRtcHandle);
}

void rtcClearStatusFlags(srtm_hal_rtc_adapter_t adapter, uint32_t flags)
{
    HAL_RtcClearStatusFlags(adapter->halRtcHandle, flags);
}

void rtcEnableInterrupts(srtm_hal_rtc_adapter_t adapter, uint32_t flags)
{
    HAL_RtcEnableInterrupts(adapter->halRtcHandle, flags);
}

void rtcDisableInterrupts(srtm_hal_rtc_adapter_t adapter, uint32_t flags)
{
    HAL_RtcDisableInterrupts(adapter->halRtcHandle, flags);
}

static srtm_status_t SRTM_RtcAdapter_GetTime(srtm_rtc_adapter_t adapter, uint32_t *pSeconds)
{
    srtm_hal_rtc_adapter_t handle = (srtm_hal_rtc_adapter_t)(void *)adapter;

    assert(handle);
    assert(pSeconds);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    *pSeconds = rtcGetTime(handle) / SECOND_TO_MICROSECOND;

    return SRTM_Status_Success;
}

static srtm_status_t SRTM_RtcAdapter_SetTime(srtm_rtc_adapter_t adapter, uint32_t seconds)
{
    srtm_hal_rtc_adapter_t handle = (srtm_hal_rtc_adapter_t)(void *)adapter;
    hal_rtc_status_t ret;

    assert(handle);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    ret = rtcSetTime(handle, seconds * SECOND_TO_MICROSECOND);
    if (ret != kStatus_HAL_RtcSuccess)
        return SRTM_Status_Error;

    return SRTM_Status_Success;
}

static srtm_status_t SRTM_RtcAdapter_GetAlarm(srtm_rtc_adapter_t adapter, struct _srtm_rtc_alarm *pAlarm)
{
    srtm_hal_rtc_adapter_t handle = (srtm_hal_rtc_adapter_t)adapter;

    assert(handle);
    assert(pAlarm);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    pAlarm->seconds = rtcGetAlarm(handle) / SECOND_TO_MICROSECOND;

    pAlarm->enabled =
        (rtcGetEnabledInterrupts(handle) & kHAL_RTC_AlarmInterrupt) ? SRTM_RTC_ALARM_ENABLED : SRTM_RTC_ALARM_DISABLED;
    pAlarm->pending =
        (rtcGetStatusFlags(handle) & kHAL_RTC_AlarmInterruptFlag) ? SRTM_RTC_ALARM_PENDING : SRTM_RTC_ALARM_NOT_PENDING;

    return SRTM_Status_Success;
}

static srtm_status_t SRTM_RtcAdapter_SetAlarm(srtm_rtc_adapter_t adapter, const struct _srtm_rtc_alarm *pAlarm)
{
    status_t status;
    srtm_hal_rtc_adapter_t handle = (srtm_hal_rtc_adapter_t)adapter;

    assert(handle);
    assert(pAlarm);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s %s\r\n", __func__, pAlarm->enabled ? "Enable" : "Disable");

    status = rtcSetAlarm(handle, (uint64_t)(pAlarm->seconds * SECOND_TO_MICROSECOND));
    if (status == kStatus_Success)
    {
        /* Clear the pending alarm flag */
        rtcClearStatusFlags(handle, kHAL_RTC_AlarmInterruptFlag);

        if (pAlarm->enabled)
        {
            rtcEnableInterrupts(handle, kHAL_RTC_AlarmInterrupt);
        }
        else
        {
            rtcDisableInterrupts(handle, kHAL_RTC_AlarmInterrupt);
        }
        return SRTM_Status_Success;
    }

    return SRTM_Status_Error;
}

srtm_status_t SRTM_RtcAdapter_EnableAlarm(srtm_rtc_adapter_t adapter, bool enable)
{
    srtm_hal_rtc_adapter_t handle = (srtm_hal_rtc_adapter_t)(void *)adapter;

    assert(handle);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s %s\r\n", __func__, enable ? "Enable" : "Disable");

    /* Clear the pending alarm flag */
    rtcClearStatusFlags(handle, kHAL_RTC_AlarmInterruptFlag);

    if (enable)
    {
        rtcEnableInterrupts(handle, kHAL_RTC_AlarmInterrupt);
    }
    else
    {
        rtcDisableInterrupts(handle, kHAL_RTC_AlarmInterrupt);
    }

    return SRTM_Status_Success;
}

srtm_rtc_adapter_t SRTM_RtcAdapter_Create(hal_rtc_handle_t halRtcHandle)
{
    srtm_hal_rtc_adapter_t handle;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    handle = (srtm_hal_rtc_adapter_t)SRTM_Heap_Malloc(sizeof(struct _srtm_hal_rtc_adapter));
    assert(handle);

    memset(handle, 0, sizeof(struct _srtm_hal_rtc_adapter));

    handle->halRtcHandle = halRtcHandle;

    /* Fields to be filled when creating service with this adapter. */
    handle->adapter.service     = NULL;
    handle->adapter.notifyAlarm = NULL;

    /* Adapter interfaces. */
    handle->adapter.getTime     = SRTM_RtcAdapter_GetTime;
    handle->adapter.setTime     = SRTM_RtcAdapter_SetTime;
    handle->adapter.getAlarm    = SRTM_RtcAdapter_GetAlarm;
    handle->adapter.setAlarm    = SRTM_RtcAdapter_SetAlarm;
    handle->adapter.enableAlarm = SRTM_RtcAdapter_EnableAlarm;

    return &handle->adapter;
}

void SRTM_RtcAdapter_Destroy(srtm_rtc_adapter_t adapter)
{
    srtm_hal_rtc_adapter_t handle = (srtm_hal_rtc_adapter_t)adapter;

    assert(adapter);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    SRTM_Heap_Free(handle);
}

void SRTM_RtcAdapter_NotifyAlarm(srtm_rtc_adapter_t adapter)
{
    srtm_hal_rtc_adapter_t handle = (srtm_hal_rtc_adapter_t)adapter;

    assert(adapter);

    if (rtcGetStatusFlags(handle) & kHAL_RTC_AlarmInterruptFlag)
    {
        SRTM_RtcAdapter_DisableAlarmInt(adapter);
        /* Clear alarm flag */
        rtcClearStatusFlags(handle, kHAL_RTC_AlarmInterruptFlag);

        if (adapter->notifyAlarm && adapter->service)
        {
            adapter->notifyAlarm(adapter->service);
        }
    }
}

void SRTM_RtcAdapter_DisableAlarmInt(srtm_rtc_adapter_t adapter)
{
    srtm_hal_rtc_adapter_t handle = (srtm_hal_rtc_adapter_t)adapter;

    assert(adapter);

    /* Alarm should be one-shot */
    rtcDisableInterrupts(handle, kHAL_RTC_AlarmInterrupt);
}
