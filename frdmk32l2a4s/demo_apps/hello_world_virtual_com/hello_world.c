/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "fsl_common.h"
#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"
#include "fsl_component_serial_port_usb.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void USB_DeviceClockInit(void);

/*******************************************************************************
 * Code
 ******************************************************************************/
#define CORE_CLOCK_MODE_VLPR  8000000U
#define CORE_CLOCK_MODE_RUN   48000000U
#define CORE_CLOCK_MODE_HSRUN 96000000U
const scg_sosc_config_t app_scgSysOscConfig = {.freq        = BOARD_XTAL0_CLK_HZ,
                                               .enableMode  = kSCG_SysOscEnable | kSCG_SysOscEnableErClk,
                                               .monitorMode = kSCG_SysOscMonitorDisable,
                                               .div1        = kSCG_AsyncClkDisable,
                                               .div2        = kSCG_AsyncClkDisable,
                                               .div3        = kSCG_AsyncClkDivBy1,
                                               .capLoad     = 0U,
                                               .workMode    = kSCG_SysOscModeOscLowPower};
const scg_sirc_config_t app_scgSircConfig   = {.enableMode = kSCG_SircEnable | kSCG_SircEnableInLowPower,
                                             .div1       = kSCG_AsyncClkDisable,
                                             .div2       = kSCG_AsyncClkDisable,
                                             .div3       = kSCG_AsyncClkDivBy2,
                                             .range      = kSCG_SircRangeHigh};
const scg_firc_config_t app_scgFircConfig   = {
    .enableMode = kSCG_FircEnable | kSCG_FircEnableInStop | kSCG_FircEnableInLowPower,
    .div3       = kSCG_AsyncClkDivBy1,
    .div2       = kSCG_AsyncClkDisable,
    .div1       = kSCG_AsyncClkDivBy1,
    .range      = kSCG_FircRange48M,
    .trimConfig = NULL};
const scg_sys_clk_config_t app_sysClkConfigRun = {
    .divSlow = kSCG_SysClkDivBy2, .divCore = kSCG_SysClkDivBy1, .src = kSCG_SysClkSrcFirc};
void APP_BootClockRUN(void)
{
    scg_sys_clk_config_t curConfig;
    CLOCK_InitSysOsc(&app_scgSysOscConfig);
    CLOCK_SetXtal0Freq(BOARD_XTAL0_CLK_HZ);
    CLOCK_InitFirc(&app_scgFircConfig);
    CLOCK_SetRunModeSysClkConfig(&app_sysClkConfigRun);
    /* Wait for clock source switch finished. */
    do
    {
        CLOCK_GetCurSysClkConfig(&curConfig);
    } while (curConfig.src != app_sysClkConfigRun.src);
    CLOCK_InitSirc(&app_scgSircConfig);
    SystemCoreClock = CORE_CLOCK_MODE_RUN;
}

void USB_DeviceClockInit(void)
{
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
    SystemCoreClockUpdate();
    CLOCK_EnableUsbfs0Clock(kCLOCK_IpSrcFircAsync, CLOCK_GetFreq(kCLOCK_ScgFircAsyncDiv1Clk));
/*
 * If the SOC has USB KHCI dedicated RAM, the RAM memory needs to be clear after
 * the KHCI clock is enabled. When the demo uses USB EHCI IP, the USB KHCI dedicated
 * RAM can not be used and the memory can't be accessed.
 */
#if (defined(FSL_FEATURE_USB_KHCI_USB_RAM) && (FSL_FEATURE_USB_KHCI_USB_RAM > 0U))
#if (defined(FSL_FEATURE_USB_KHCI_USB_RAM_BASE_ADDRESS) && (FSL_FEATURE_USB_KHCI_USB_RAM_BASE_ADDRESS > 0U))
    for (int i = 0; i < FSL_FEATURE_USB_KHCI_USB_RAM; i++)
    {
        ((uint8_t *)FSL_FEATURE_USB_KHCI_USB_RAM_BASE_ADDRESS)[i] = 0x00U;
    }
#endif /* FSL_FEATURE_USB_KHCI_USB_RAM_BASE_ADDRESS */
#endif /* FSL_FEATURE_USB_KHCI_USB_RAM */
#endif
}

/*!
 * @brief Main function
 */
int main(void)
{
    char ch;

    /* Init board hardware. */
    APP_BootClockRUN();
    USB_DeviceClockInit();
    DbgConsole_Init((uint8_t)kSerialManager_UsbControllerKhci0, (uint32_t)NULL, kSerialPort_UsbCdc, (uint32_t)NULL);

    PRINTF("hello world.\r\n");

    while (1)
    {
        ch = GETCHAR();
        PUTCHAR(ch);
    }
}
