/** @file host_sleep.h
 *
 *  @brief Host sleep file
 *
 *  Copyright 2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *  The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */
#ifndef _HOST_SLEEP_H_
#define _HOST_SLEEP_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Leave AON modules on in PM2 */
#define NCP_PM2_MEM_PU_CFG ((uint32_t)kPOWER_Pm2MemPuAon1 | (uint32_t)kPOWER_Pm2MemPuAon0 | (uint32_t)kPOWER_Pm2MemPuSdio)
/* All ANA in low power mode in PM2 */
#if CONFIG_NCP
#define NCP_PM2_ANA_PU_CFG (10U)
#else
#define NCP_PM2_ANA_PU_CFG (0U)
#endif
/* Buck18 and Buck11 both in sleep level in PM3 */
#define NCP_PM3_BUCK_CFG (0U)
/* All clock gated */
#define NCP_SOURCE_CLK_GATE ((uint32_t)kPOWER_ClkGateAll)
/* All SRAM kept in retention in PM3, AON SRAM shutdown in PM4 */
#define NCP_MEM_PD_CFG (1UL << 8)

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
#if CONFIG_NCP_UART
#define WAKEUP_BY_USART0 0x8
#endif

#define CONFIG_NCP_SUSPEND_STACK_SIZE 1024

#define SUSPEND_EVENT_TRIGGERS (1U<<0U)

#define NCP_NOTIFY_HOST_GPIO        27
#define NCP_NOTIFY_HOST_GPIO_MASK   0x8000000

/*******************************************************************************
 * API
 ******************************************************************************/
void host_sleep_cli_notify(void);
void host_sleep_pre_hook(void);
int host_sleep_pre_cfg(int mode);
void host_sleep_post_cfg(int mode);
void host_sleep_dump_wakeup_source();
int ncp_config_suspend_mode(int mode);
#if CONFIG_POWER_MANAGER
void powerManager_EnterLowPower();
#endif
int host_sleep_cli_init(void);
void ncp_gpio_init(void);
int hostsleep_init(void);
void ncp_notify_host_gpio_init(void);
void ncp_notify_host_gpio_output(void);
#endif /*_HOST_SLEEP_H_*/
