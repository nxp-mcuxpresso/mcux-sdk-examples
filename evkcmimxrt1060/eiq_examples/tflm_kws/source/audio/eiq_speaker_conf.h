/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EIQ_SPEAKER_CONF_H_
#define _EIQ_SPEAKER_CONF_H_

#include "eiq_micro_conf.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define SAI_TxIRQHandler SAI1_IRQHandler

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Clears Tx interrupt flags - FRIE, FEIE
 */
static inline void BOARD_ClearTxInterruptFlags(void)
{
    /* Clear RCSR interrupt flags. */
    DEMO_SAI->RCSR &= ~(I2S_RCSR_FRIE_MASK | I2S_RCSR_FEIE_MASK);
}

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _EIQ_SPEAKER_CONF_H_ */
