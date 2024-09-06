/** @file host_sleep.h
 *
 *  @brief Host sleep file
 *
 *  Copyright 2021 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _HOST_SLEEP_H_
#define _HOST_SLEEP_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Leave AON modules on in PM2 */
#define WLAN_PM2_MEM_PU_CFG ((uint32_t)kPOWER_Pm2MemPuAon1 | (uint32_t)kPOWER_Pm2MemPuAon0)
/* All ANA in low power mode in PM2 */
#define WLAN_PM2_ANA_PU_CFG (0U)
/* Buck18 and Buck11 both in sleep level in PM3 */
#define WLAN_PM3_BUCK_CFG (0U)
/* All clock gated */
#define WLAN_SOURCE_CLK_GATE ((uint32_t)kPOWER_ClkGateAll)
/* All SRAM kept in retention in PM3, AON SRAM shutdown in PM4 */
#define WLAN_MEM_PD_CFG (1UL << 8)

#if CONFIG_POWER_MANAGER
#define APP_PM2_CONSTRAINTS                                                                              \
    7U, PM_RESC_SRAM_0K_384K_STANDBY, PM_RESC_SRAM_384K_448K_STANDBY, PM_RESC_SRAM_448K_512K_STANDBY,    \
        PM_RESC_SRAM_512K_640K_STANDBY, PM_RESC_SRAM_640K_896K_STANDBY, PM_RESC_SRAM_896K_1216K_STANDBY, \
        PM_RESC_CAU_SOC_SLP_REF_CLK_ON
#define APP_PM3_CONSTRAINTS                                                                                    \
    7U, PM_RESC_SRAM_0K_384K_RETENTION, PM_RESC_SRAM_384K_448K_RETENTION, PM_RESC_SRAM_448K_512K_RETENTION,    \
        PM_RESC_SRAM_512K_640K_RETENTION, PM_RESC_SRAM_640K_896K_RETENTION, PM_RESC_SRAM_896K_1216K_RETENTION, \
        PM_RESC_CAU_SOC_SLP_REF_CLK_ON
#endif

#define WAKEUP_BY_WLAN   0x1
#define WAKEUP_BY_RTC    0x2
#define WAKEUP_BY_PIN    0x4
#define WAKEUP_BY_USART3 0x8

/*******************************************************************************
 * API
 ******************************************************************************/
void host_sleep_cli_notify(void);
void host_sleep_pre_hook(void);
int host_sleep_pre_cfg(int mode);
void host_sleep_post_cfg(int mode);
void host_sleep_dump_wakeup_source();
void test_wlan_suspend(int argc, char **argv);
int wlan_config_suspend_mode(int mode);
#if CONFIG_POWER_MANAGER
void powerManager_EnterLowPower();
#endif
int host_sleep_cli_init(void);
int hostsleep_init(void);

#endif /*_HOST_SLEEP_H_*/
