/*
 * Copyright 2022 - 2023 NXP
 *
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PDE_RADE_H__
#define __PDE_RADE_H__

#include "stdint.h"

typedef float float_rade_t;

typedef enum rade_result_type
{
    RADE_Success = 0,
    RADE_Fail,
    RADE_MEMAllocFail,
    RADE_MEMAllocSuccess,
    RADE_ModuleFail,
    RADE_ModuleSuccess
} rade_result_type_t;

rade_result_type_t pde_rade(int16_t *iq_i, int16_t *iq_r, const uint32_t *tqiMask0, const uint32_t *chanMask0, void **rng_algo_buf, const uint8_t n_ap, float_rade_t *rng_est, float_rade_t *rng_est_qi);
void RADE_Deinit(void **rng_algo_buf);

#endif