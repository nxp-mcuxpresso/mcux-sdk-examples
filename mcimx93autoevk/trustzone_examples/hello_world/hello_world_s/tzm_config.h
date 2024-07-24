/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _TZM_CONFIG_H_
#define _TZM_CONFIG_H_

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

#define EXAMPLE_TRDC_PROCESSOR_MASTER_DOMAIN_ID 0
#define EXAMPLE_TRDC_DOMAIN_INDEX               0

#define EXAMPLE_TRDC_MBC_ACCESS_CONTROL_POLICY_ALL_INDEX  0
#define EXAMPLE_TRDC_MBC_ACCESS_CONTROL_POLICY_NONE_INDEX 2

/* SAU region boundaries */
#define SAU_REGION_0_BASE 0x20013000U
#define SAU_REGION_0_END  0x2001DFFFU
#define SAU_REGION_1_BASE 0x0FFF0000U
#define SAU_REGION_1_END  0x0FFFFFFFU
#define SAU_REGION_2_BASE 0x1FFEFE00U
#define SAU_REGION_2_END  0x1FFEFFFFU
/***********************************************************************************************************************
 * Initialize TrustZone
 **********************************************************************************************************************/
void BOARD_InitTrustZone(void);
void App_SetTrdcMBCNSE(uint8_t mbcIdx, uint8_t slvIdx, uint8_t mbIdx, uint32_t nse);

#if defined(__cplusplus)
}
#endif

#endif /* _TZM_CONFIG_H_ */
