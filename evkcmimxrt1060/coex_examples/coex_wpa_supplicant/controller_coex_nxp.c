/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "BT_common.h"
#include "fsl_common.h"
#include "cli.h"
#include "wlan.h"

#include "fwdnld_intf_abs.h"

#if defined(CONFIG_BT_IND_DNLD) && !defined(CONFIG_WIFI_IND_DNLD)
#error "Please define CONFIG_WIFI_IND_DNLD also"
#endif

#if !defined(CONFIG_BT_IND_DNLD) && defined(CONFIG_WIFI_IND_DNLD)
#error "Please drfine CONFIG_BT_IND_DNLD also"
#endif

#if defined(CONFIG_BT_IND_DNLD) || defined(CONFIG_WIFI_IND_DNLD)
#include "wifi_bt_config.h"
#include "fw_loader_uart.h"
#endif
#if (defined(WIFI_IW416_BOARD_AW_AM457_USD) || defined(WIFI_IW612_BOARD_RD_USD) || defined(WIFI_IW612_BOARD_RD_M2) || \
     defined(WIFI_IW416_BOARD_AW_AM510_USD) || defined(WIFI_IW416_BOARD_AW_AM510MA) || \
     defined(WIFI_88W8987_BOARD_AW_CM358_USD) || defined(WIFI_88W8987_BOARD_AW_CM358MA) || \
     defined(WIFI_IW416_BOARD_MURATA_1XK_USD) || defined(WIFI_IW416_BOARD_MURATA_1XK_M2) || \
     defined(WIFI_88W8987_BOARD_MURATA_1ZM_USD) || defined (WIFI_88W8987_BOARD_MURATA_1ZM_M2) || \
	 defined(WIFI_IW611_BOARD_MURATA_2DL_USD) || defined (WIFI_IW611_BOARD_MURATA_2DL_M2) || \
     defined(WIFI_AW611_BOARD_UBX_JODY_W5_USD) || defined (WIFI_AW611_BOARD_UBX_JODY_W5_M2) || \
	 defined (WIFI_IW612_BOARD_MURATA_2EL_USD) || defined (WIFI_IW612_BOARD_MURATA_2EL_M2) )

#ifndef CONTROLLER_INIT_ESCAPE
#if defined(CONFIG_BT_IND_DNLD) || defined(CONFIG_WIFI_IND_DNLD)
#if defined(SD8978) /*RB3P*/
#include "uartIW416_bt.h"
#include "sdIW416_wlan.h"
#elif defined(SD8987) /*CA2*/
#include "uart8987_bt.h"
#include "sd8987_wlan.h"
#elif defined(SD9177) /*FC*/
#if defined(WIFI_IW612_BOARD_RD_USD) || defined(WIFI_IW612_BOARD_RD_M2)
#include "uart_nw61x.h" /*non-secured FC firmware*/
#include "sd_nw61x.h"
#else
#include "uart_nw61x_se.h" /*secured FC firmware*/
#include "sd_nw61x_se.h"
#endif /*defined(WIFI_IW612_BOARD_RD_USD) ||  defined(WIFI_IW612_BOARD_RD_M2) */
#else
#error Controller module is unsupported
#endif /*defined(SD8978)*/
#else
#if defined(SD8978) /*RB3P*/
#include "sduartIW416_wlan_bt.h"
#elif defined(SD8987) /*CA2*/
#include "sduart8987_wlan_bt.h"
#elif defined(SD9177) /*FC*/
#if defined(WIFI_IW612_BOARD_RD_USD) || defined(WIFI_IW612_BOARD_RD_M2)
#include "sduart_nw61x.h" /*non-secured FC firmware*/
#else
#include "sduart_nw61x_se.h" /*secured FC firmware*/
#endif /*defined(WIFI_IW612_BOARD_RD_USD) || defined(WIFI_IW612_BOARD_RD_M2) */
#else
#error Controller module is unsupported
#endif /*defined(SD8978)*/
#endif /* CONFIG_BT_IND_DNLD || CONFIG_WIFI_IND_DNLD */
#endif /* CONTROLLER_INIT_ESCAPE */

#include "sdio.h"
#include "controller.h"
#include "firmware_dnld.h"
#include "fsl_adapter_uart.h"
#include "fsl_os_abstraction.h"
#include "controller_hci_uart.h"
#include "wifi_bt_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BT_OP(ogf, ocf) ((ocf) | ((ogf) << 10))
#define BT_OGF_VS       0x3f

/* Weak function. */
#if defined(__GNUC__)
#define __WEAK_FUNC __attribute__((weak))
#elif defined(__ICCARM__)
#define __WEAK_FUNC __weak
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define __WEAK_FUNC __attribute__((weak))
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void coex_controller_hci_uart_init(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static UART_HANDLE_DEFINE(s_controllerHciUartHandle);

/*******************************************************************************
 * Code
 ******************************************************************************/

__WEAK_FUNC int controller_hci_uart_get_configuration(controller_hci_uart_config_t *config);

/* Initialize the platform */
void coex_controller_init(void)
{
#ifndef CONTROLLER_INIT_ESCAPE
    int result;
    (void) result;
    BOARD_WIFI_BT_Enable(true);
#if defined(CONFIG_BT_IND_DNLD) || defined(CONFIG_WIFI_IND_DNLD)
    /* BTonly firmware download over UART */
    void *intf = uart_init_interface();
    assert(intf != NULL);
    result = firmware_download(bt_fw_bin, bt_fw_bin_len, intf, 0);
    assert(result == FWDNLD_INTF_SUCCESS);
#endif
    /* Combo firmware download over SDIO */
    result = wlan_init(wlan_fw_bin, wlan_fw_bin_len);
    assert(API_SUCCESS == result);
#endif /*CONTROLLER_INIT_ESCAPE*/
    coex_controller_hci_uart_init();
}

/* Initialize the platform */
void controller_init(void)
{
#ifndef CONTROLLER_INIT_ESCAPE
    int result;
    (void) result;
#ifdef CONFIG_BT_IND_DNLD
    /* BTonly firmware download over UART */
    void *intf = uart_init_interface();
    assert(intf != NULL);
    result = firmware_download(bt_fw_bin, bt_fw_bin_len, intf, 0);
    assert(result == FWDNLD_INTF_SUCCESS);
#endif /*CONFIG_BT_IND_DNLD*/
#endif /*CONTROLLER_INIT_ESCAPE*/
    coex_controller_hci_uart_init();
}


__WEAK_FUNC int controller_hci_uart_get_configuration(controller_hci_uart_config_t *config)
{
    return -1;
}

static void coex_controller_hci_uart_init(void)
{
    uint16_t *opcode;
    uint8_t *param_len;
    hal_uart_config_t config;
    controller_hci_uart_config_t hciUartConfig;
    uint8_t sendingBuffer[8];
    uint8_t recvBuffer[8];
    hal_uart_status_t error;

    memset(sendingBuffer, 0, sizeof(sendingBuffer));
    memset(recvBuffer, 0, sizeof(recvBuffer));
    memset(&hciUartConfig, 0, sizeof(hciUartConfig));
    memset(&config, 0, sizeof(config));
#if (defined(WIFI_IW416_BOARD_AW_AM457_USD) || defined(WIFI_IW416_BOARD_AW_AM510_USD) ||      \
     defined(WIFI_IW416_BOARD_AW_AM510MA) || defined(WIFI_IW416_BOARD_MURATA_1XK_USD) ||      \
     defined(WIFI_IW416_BOARD_MURATA_1XK_M2) || defined(WIFI_88W8987_BOARD_MURATA_1ZM_USD) || \
     defined(WIFI_88W8987_BOARD_MURATA_1ZM_M2))
    /*delay to make sure controller is ready to receive command*/
    OSA_TimeDelay(100);
#endif
    if (0 != controller_hci_uart_get_configuration(&hciUartConfig))
    {
        return;
    }
    if (hciUartConfig.runningBaudrate == hciUartConfig.defaultBaudrate)
    {
        return;
    }
    /* Set the HCI-UART Configuration parameters */
    config.srcClock_Hz  = hciUartConfig.clockSrc;
    config.baudRate_Bps = hciUartConfig.defaultBaudrate;
    config.parityMode   = kHAL_UartParityDisabled;
    config.stopBitCount = kHAL_UartOneStopBit;
    config.enableRx     = 1;
    config.enableTx     = 1;
    config.instance     = hciUartConfig.instance;
    config.enableRxRTS  = hciUartConfig.enableRxRTS;
    config.enableTxCTS  = hciUartConfig.enableTxCTS;
#if (defined(HAL_UART_ADAPTER_FIFO) && (HAL_UART_ADAPTER_FIFO > 0u))
    config.txFifoWatermark = 0U;
    config.rxFifoWatermark = 0U;
#endif

    /* Initialize UART with Adapter */
    error = HAL_UartInit((hal_uart_handle_t)s_controllerHciUartHandle, &config);
    /* Check if Assert or Log and return? */
    assert(kStatus_HAL_UartSuccess == error);
    sendingBuffer[0]                   = 0x01;
    opcode                             = (uint16_t *)&sendingBuffer[1];
    param_len                          = &sendingBuffer[3];
    *opcode                            = (uint16_t)BT_OP(BT_OGF_VS, 0x09);
    *param_len                         = sizeof(hciUartConfig.runningBaudrate);
    *((uint32_t *)(&sendingBuffer[4])) = hciUartConfig.runningBaudrate;
    /*delay to make sure controller is ready to receive command*/
    OSA_TimeDelay(60);
    error = HAL_UartSendBlocking((hal_uart_handle_t)s_controllerHciUartHandle, &sendingBuffer[0], 8);
    assert(kStatus_HAL_UartSuccess == error);
    error = HAL_UartReceiveBlocking((hal_uart_handle_t)s_controllerHciUartHandle, &recvBuffer[0], 7);
    assert(kStatus_HAL_UartSuccess == error);

    assert(0 == recvBuffer[7]);

    OSA_TimeDelay(500);
    error = HAL_UartDeinit((hal_uart_handle_t)s_controllerHciUartHandle);

    assert(kStatus_HAL_UartSuccess == error);

    (void)error;
}

#endif
