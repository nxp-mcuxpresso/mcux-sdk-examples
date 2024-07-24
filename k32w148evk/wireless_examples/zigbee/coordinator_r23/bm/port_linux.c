/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "zb_platform.h"


/****************************************************************************
 *
 * NAME: ZPS_eEnterCriticalSection
 *
 * DESCRIPTION:  Provides protection from function re-entrancy and interrupt
 *               pre-emption.
 *               
 *
 * RETURNS:
 *
 ****************************************************************************/
PUBLIC uint8 ZPS_eEnterCriticalSection(void* hMutex, uint32* psIntStore)
{
    /* NOT IMPLEMENTED */
    return 0;
}

/****************************************************************************
 *
 * NAME: ZPS_eExitCriticalSection
 *
 * DESCRIPTION:  Allows interrupt pre-emption and function re-entrancy.
 *               
 *
 * RETURNS:
 *
 ****************************************************************************/
PUBLIC uint8 ZPS_eExitCriticalSection(void* hMutex , uint32* psIntStore)
{
    /* NOT IMPLEMENTED */
    return 0;
}


