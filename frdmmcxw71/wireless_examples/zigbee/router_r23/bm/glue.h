/****************************************************************************
 *
 * Copyright 2024 NXP
 *
 * NXP Confidential and Proprietary.
 *
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.
 * By expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.
 * If you do not agree to be bound by the applicable license terms,
 * then you may not retain, install, activate or otherwise use the software.
 *
 *
 ****************************************************************************/

#ifndef  __GLUE_H__
#define  __GLUE_H__

void OSA_InterruptEnableRestricted(uint32_t *pu32OldIntLevel);
void OSA_InterruptEnableRestore(uint32_t *pu32OldIntLevel);

void RESET_SystemReset();

#endif // __GLUE_H__
