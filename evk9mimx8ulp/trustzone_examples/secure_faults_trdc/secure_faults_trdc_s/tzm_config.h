/*
 * Copyright 2022 NXP
 * All rights reserved.
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
#define EXAMPLE_TRDC_MRC_INDEX                  0
#define EXAMPLE_TRDC_MRC_REGION_INDEX           0

#define EXAMPLE_TRDC_MRC_ACCESS_CONTROL_POLICY_ALL_INDEX  0
#define EXAMPLE_TRDC_MRC_ACCESS_CONTROL_POLICY_NONE_INDEX 1

#define EXAMPLE_TRDC_MBC_ACCESS_CONTROL_POLICY_ALL_INDEX  0
#define EXAMPLE_TRDC_MBC_ACCESS_CONTROL_POLICY_NONE_INDEX 1

/* SAU region boundaries */
#define SAU_REGION_0_BASE 0x04100000U /*flash address for non-secure*/
#define SAU_REGION_0_END  0x041FFFFFU
#define SAU_REGION_1_BASE 0x20020000U
#define SAU_REGION_1_END  0x20047FFFU
#define SAU_REGION_2_BASE 0x2809B000U
#define SAU_REGION_2_END  0x2809BFFFU
#define SAU_REGION_3_BASE 0x140FFE00U
#define SAU_REGION_3_END  0x140FFFFFU
#define SAU_REGION_4_BASE 0x1FFDFE00U
#define SAU_REGION_4_END  0x1FFDFFFFU
#define SAU_REGION_5_BASE 0x0FFE0000U
#define SAU_REGION_5_END  0x0FFFFFFFU
#define SAU_REGION_6_BASE 0x28046000U
#define SAU_REGION_6_END  0x28046FFFU
/***********************************************************************************************************************
 * Initialize TrustZone
 **********************************************************************************************************************/
void BOARD_InitTrustZone(void);
void APP_PrintTrdcErr(void);
void App_SetTrdcMBCNSE(uint8_t mbcIdx, uint8_t slvIdx, uint8_t mbIdx, uint32_t nse);

#if defined(__cplusplus)
}
#endif

#endif /* _TZM_CONFIG_H_ */
