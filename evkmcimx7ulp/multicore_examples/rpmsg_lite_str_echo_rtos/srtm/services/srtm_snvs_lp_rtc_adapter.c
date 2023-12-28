/*
 * Copyright 2017, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "srtm_snvs_lp_rtc_adapter.h"
#include "srtm_heap.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* SNVS_LP RTC adapter */
typedef struct _srtm_snvs_lp_rtc_adapter
{
    struct _srtm_rtc_adapter adapter;
    SNVS_Type *base;
} *srtm_snvs_lp_rtc_adapter_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
static uint32_t RTC_GetSeconds(SNVS_Type *base)
{
    uint32_t seconds = 0;
    uint32_t tmp     = 0;

    /* Do consecutive reads until value is correct */
    do
    {
        seconds = tmp;
        tmp     = (base->LPSRTCMR << 17U) | (base->LPSRTCLR >> 15U);
    } while (tmp != seconds);

    return seconds;
}

static void RTC_SetSeconds(SNVS_Type *base, uint32_t seconds)
{
    uint32_t tmp = base->LPCR;

    /* disable RTC */
    SNVS_LP_SRTC_StopTimer(base);

    /* Set time in seconds */
    base->LPSRTCMR = (uint32_t)(seconds >> 17U);
    base->LPSRTCLR = (uint32_t)(seconds << 15U);

    /* reenable SRTC in case that it was enabled before */
    if ((tmp & SNVS_LPCR_SRTC_ENV_MASK) != 0UL)
    {
        SNVS_LP_SRTC_StartTimer(base);
    }
}

static srtm_status_t RTC_SetAlarm(SNVS_Type *base, uint32_t seconds)
{
    uint32_t currSeconds = 0U;
    uint32_t tmp         = base->LPCR;

    currSeconds = RTC_GetSeconds(base);

    /* Return error if the alarm time has passed */
    if (seconds <= currSeconds)
    {
        return SRTM_Status_InvalidParameter;
    }

    /* disable SRTC alarm interrupt */
    base->LPCR &= ~SNVS_LPCR_LPTA_EN_MASK;
    while ((base->LPCR & SNVS_LPCR_LPTA_EN_MASK) != 0UL)
    {
    }

    /* Set alarm in seconds*/
    base->LPTAR = seconds;

    /* reenable SRTC alarm interrupt in case that it was enabled before */
    base->LPCR = tmp;

    return SRTM_Status_Success;
}

static srtm_status_t SRTM_SnvsLpRtcAdapter_GetTime(srtm_rtc_adapter_t adapter, uint32_t *pSeconds)
{
    srtm_snvs_lp_rtc_adapter_t handle = (srtm_snvs_lp_rtc_adapter_t)(void *)adapter;

    assert(handle->base);
    assert(pSeconds);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    *pSeconds = RTC_GetSeconds(handle->base);

    return SRTM_Status_Success;
}

static srtm_status_t SRTM_SnvsLpRtcAdapter_SetTime(srtm_rtc_adapter_t adapter, uint32_t seconds)
{
    srtm_snvs_lp_rtc_adapter_t handle = (srtm_snvs_lp_rtc_adapter_t)adapter;

    assert(handle->base);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    RTC_SetSeconds(handle->base, seconds);

    return SRTM_Status_Success;
}

static srtm_status_t SRTM_SnvsLpRtcAdapter_GetAlarm(srtm_rtc_adapter_t adapter, struct _srtm_rtc_alarm *pAlarm)
{
    srtm_snvs_lp_rtc_adapter_t handle = (srtm_snvs_lp_rtc_adapter_t)adapter;

    assert(handle->base);
    assert(pAlarm);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    pAlarm->seconds = handle->base->LPTAR;

    pAlarm->enabled = (SNVS_LP_SRTC_GetEnabledInterrupts(handle->base) & kSNVS_SRTC_AlarmInterrupt) ?
                          SRTM_RTC_ALARM_ENABLED :
                          SRTM_RTC_ALARM_DISABLED;
    pAlarm->pending = (SNVS_LP_SRTC_GetStatusFlags(handle->base) & kSNVS_SRTC_AlarmInterruptFlag) ?
                          SRTM_RTC_ALARM_PENDING :
                          SRTM_RTC_ALARM_NOT_PENDING;

    return SRTM_Status_Success;
}

static srtm_status_t SRTM_SnvsLpRtcAdapter_SetAlarm(srtm_rtc_adapter_t adapter, const struct _srtm_rtc_alarm *pAlarm)
{
    srtm_status_t status;
    srtm_snvs_lp_rtc_adapter_t handle = (srtm_snvs_lp_rtc_adapter_t)adapter;

    assert(handle->base);
    assert(pAlarm);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s %s\r\n", __func__, pAlarm->enabled ? "Enable" : "Disable");

    status = RTC_SetAlarm(handle->base, pAlarm->seconds);
    if (status == SRTM_Status_Success)
    {
        /* Clear the pending alarm flag */
        SNVS_LP_SRTC_ClearStatusFlags(handle->base, kSNVS_SRTC_AlarmInterruptFlag);

        if (pAlarm->enabled)
        {
            SNVS_LP_SRTC_EnableInterrupts(handle->base, kSNVS_SRTC_AlarmInterrupt);
        }
        else
        {
            SNVS_LP_SRTC_DisableInterrupts(handle->base, kSNVS_SRTC_AlarmInterrupt);
        }
    }

    return status;
}

srtm_status_t SRTM_SnvsLpRtcAdapter_EnableAlarm(srtm_rtc_adapter_t adapter, bool enable)
{
    srtm_snvs_lp_rtc_adapter_t handle = (srtm_snvs_lp_rtc_adapter_t)adapter;

    assert(handle->base);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s %s\r\n", __func__, enable ? "Enable" : "Disable");

    /* Clear the pending alarm flag */
    SNVS_LP_SRTC_ClearStatusFlags(handle->base, kSNVS_SRTC_AlarmInterruptFlag);

    if (enable)
    {
        SNVS_LP_SRTC_EnableInterrupts(handle->base, kSNVS_SRTC_AlarmInterrupt);
    }
    else
    {
        SNVS_LP_SRTC_DisableInterrupts(handle->base, kSNVS_SRTC_AlarmInterrupt);
    }

    return SRTM_Status_Success;
}

srtm_rtc_adapter_t SRTM_SnvsLpRtcAdapter_Create(SNVS_Type *base)
{
    srtm_snvs_lp_rtc_adapter_t handle;

    assert(base);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    handle = (srtm_snvs_lp_rtc_adapter_t)SRTM_Heap_Malloc(sizeof(struct _srtm_snvs_lp_rtc_adapter));
    assert(handle);

    handle->base = base;

    /* Fields to be filled when creating service with this adapter. */
    handle->adapter.service     = NULL;
    handle->adapter.notifyAlarm = NULL;

    /* Adapter interfaces. */
    handle->adapter.getTime     = SRTM_SnvsLpRtcAdapter_GetTime;
    handle->adapter.setTime     = SRTM_SnvsLpRtcAdapter_SetTime;
    handle->adapter.getAlarm    = SRTM_SnvsLpRtcAdapter_GetAlarm;
    handle->adapter.setAlarm    = SRTM_SnvsLpRtcAdapter_SetAlarm;
    handle->adapter.enableAlarm = SRTM_SnvsLpRtcAdapter_EnableAlarm;

    /* Enable RTC */
    SNVS_LP_SRTC_StartTimer(base);

    return &handle->adapter;
}

void SRTM_SnvsLpRtcAdapter_Destroy(srtm_rtc_adapter_t adapter)
{
    srtm_snvs_lp_rtc_adapter_t handle = (srtm_snvs_lp_rtc_adapter_t)adapter;

    assert(adapter);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    SRTM_Heap_Free(handle);
}

void SRTM_SnvsLpRtcAdapter_NotifyAlarm(srtm_rtc_adapter_t adapter)
{
    srtm_snvs_lp_rtc_adapter_t handle = (srtm_snvs_lp_rtc_adapter_t)adapter;

    assert(adapter);

    if (SNVS_LP_SRTC_GetStatusFlags(handle->base) & kSNVS_SRTC_AlarmInterruptFlag)
    {
        /* Alarm should be one-shot */
        SNVS_LP_SRTC_DisableInterrupts(handle->base, kSNVS_SRTC_AlarmInterrupt);
        /* Clear alarm flag */
        SNVS_LP_SRTC_ClearStatusFlags(handle->base, kSNVS_SRTC_AlarmInterruptFlag);

        if (adapter->notifyAlarm && adapter->service)
        {
            adapter->notifyAlarm(adapter->service);
        }
    }
}
