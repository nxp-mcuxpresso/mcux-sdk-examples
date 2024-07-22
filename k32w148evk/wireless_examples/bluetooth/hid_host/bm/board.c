/*
 * Copyright 2019-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/************************************************************************************
 * Include
 ************************************************************************************/

#include <stdint.h>
#include "board_platform.h"
#include "fsl_common.h"
#include "clock_config.h"
#include "pin_mux.h"
#include "board.h"
#include "app.h"
#include "fsl_port.h"
#include "fsl_lpuart.h"

#if (defined(gUseHciTransportDownward_d) && (gUseHciTransportDownward_d)) || \
    (defined(gUseHciTransportUpward_d) && gUseHciTransportUpward_d)
#include "fsl_adapter_rpmsg.h"
#endif

#include "fsl_debug_console.h"
#if !(defined BOARD_DEBUG_UART_INSTANCE) || (BOARD_DEBUG_UART_INSTANCE > 1)
#error "Must define a valid UART for console to work"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/************************************************************************************
 * Private memory declarations
 ************************************************************************************/

#if defined(BOARD_HCI_OVER_UART) && defined(BOARD_HCI_OVER_UART)
static const serial_port_uart_config_t hciUartConfig = {
    .instance     = BOARD_DEBUG_UART_INSTANCE,
    .clockRate    = BOARD_DEBUG_UART_CLK_FREQ,
    .baudRate     = BOARD_DEBUG_UART_BAUDRATE,
    .parityMode   = kSerialManager_UartParityDisabled,
    .stopBitCount = kSerialManager_UartOneStopBit,
    .enableRx     = 1,
    .enableTx     = 1,
};
#endif

/************************************************************************************
*************************************************************************************
* Private type definitions and macros
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

static void BOARD_InitPinsDebugConsole(void)
{
#if (BOARD_DEBUG_UART_INSTANCE == 1U)
#if !defined(DBG_CONSOLE_RX_DISABLE) || (DBG_CONSOLE_RX_DISABLE == 0)
    BOARD_InitPinLPUART1_RX();
#endif
#if !defined(DBG_CONSOLE_TX_DISABLE) || (DBG_CONSOLE_TX_DISABLE == 0)
    BOARD_InitPinLPUART1_TX();
#endif
#elif (BOARD_DEBUG_UART_INSTANCE == 0U)
#if !defined(DBG_CONSOLE_RX_DISABLE) || (DBG_CONSOLE_RX_DISABLE == 0)
    BOARD_InitPinLPUART0_RX();
#endif
#if !defined(DBG_CONSOLE_TX_DISABLE) || (DBG_CONSOLE_TX_DISABLE == 0)
    BOARD_InitPinLPUART0_TX();
#endif
#else
#error Only LPUART0 or LPUART1 supported
#endif
}

static void BOARD_UninitPinsDebugConsole(void)
{
#if (BOARD_DEBUG_UART_INSTANCE == 1U)
#if !defined(DBG_CONSOLE_RX_DISABLE) || (DBG_CONSOLE_RX_DISABLE == 0)
    BOARD_UnInitPinLPUART1_RX();
#endif
#if !defined(DBG_CONSOLE_TX_DISABLE) || (DBG_CONSOLE_TX_DISABLE == 0)
    BOARD_UnInitPinLPUART1_TX();
#endif
#elif (BOARD_DEBUG_UART_INSTANCE == 0U)
#if !defined(DBG_CONSOLE_RX_DISABLE) || (DBG_CONSOLE_RX_DISABLE == 0)
    BOARD_UnInitPinLPUART0_RX();
#endif
#if !defined(DBG_CONSOLE_TX_DISABLE) || (DBG_CONSOLE_TX_DISABLE == 0)
    BOARD_UnInitPinLPUART0_TX();
#endif
#else
#error Only LPUART0 or LPUART1 supported
#endif
}

static void BOARD_InitPinsAppConsole(void)
{
#if (BOARD_APP_UART_INSTANCE == 1U)
    BOARD_InitPinLPUART1_RX();
    BOARD_InitPinLPUART1_TX();
#elif (BOARD_APP_UART_INSTANCE == 0U)
    BOARD_InitPinLPUART0_RX();
    BOARD_InitPinLPUART0_TX();
#if (defined(gBoardUseUart0HwFlowControl) && (gBoardUseUart0HwFlowControl > 0))
    BOARD_InitPinLPUART0_RTS();
    BOARD_InitPinLPUART0_CTS();
#endif
#else
#error Only LPUART0 or LPUART1 supported
#endif
}

static void BOARD_UninitPinsAppConsole(void)
{
#if (BOARD_APP_UART_INSTANCE == 1U)
    BOARD_UnInitPinLPUART1_RX();
    BOARD_UnInitPinLPUART1_TX();
#elif (BOARD_APP_UART_INSTANCE == 0U)
    BOARD_UnInitPinLPUART0_RX();
    BOARD_UnInitPinLPUART0_TX();
#if (defined(gBoardUseUart0HwFlowControl) && (gBoardUseUart0HwFlowControl > 0))
    BOARD_UnInitPinLPUART0_RTS();
    BOARD_UnInitPinLPUART0_CTS();
#endif
#else
#error Only LPUART0 or LPUART1 supported
#endif
}

static void BOARD_InitPinsApp2Console(void)
{
#if (BOARD_APP2_UART_INSTANCE == 1U)
    BOARD_InitPinLPUART1_RX();
    BOARD_InitPinLPUART1_TX();
#elif (BOARD_APP2_UART_INSTANCE == 0U)
    BOARD_InitPinLPUART0_RX();
    BOARD_InitPinLPUART0_TX();
#if (defined(gBoardUseUart0HwFlowControl) && (gBoardUseUart0HwFlowControl > 0))
    BOARD_InitPinLPUART0_RTS();
    BOARD_InitPinLPUART0_CTS();
#endif
#else
#error Only LPUART0 or LPUART1 supported
#endif
}

static void BOARD_UninitPinsApp2Console(void)
{
#if (BOARD_APP2_UART_INSTANCE == 1U)
    BOARD_UnInitPinLPUART1_RX();
    BOARD_UnInitPinLPUART1_TX();
#elif (BOARD_APP2_UART_INSTANCE == 0U)
    BOARD_UnInitPinLPUART0_RX();
    BOARD_UnInitPinLPUART0_TX();
#if (defined(gBoardUseUart0HwFlowControl) && (gBoardUseUart0HwFlowControl > 0))
    BOARD_UnInitPinLPUART0_RTS();
    BOARD_UnInitPinLPUART0_CTS();
#endif
#else
#error Only LPUART0 or LPUART1 supported
#endif
}

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/
void BOARD_InitAppConsole(void)
{
    /* set clock */
    CLOCK_SetIpSrc(BOARD_APP_UART_CLK, BOARD_APP_UART_CLKSRC);
    /* enable clock with kCLOCK_IpClkControl_fun3 or kCLOCK_IpClkControl_fun2 to
     * keep wake up capability in low power (domain in sleep) */
    CLOCK_EnableClockLPMode(BOARD_APP_UART_CLK, kCLOCK_IpClkControl_fun3);

    /* init lpuart pins */
    BOARD_InitPinsAppConsole();
}

bool BOARD_IsAppConsoleBusy(void)
{
    return ((((uint32_t)kLPUART_TransmissionCompleteFlag) &
             LPUART_GetStatusFlags((LPUART_Type *)BOARD_APP_UART_BASEADDR)) == 0U);
}

void BOARD_UninitAppConsole(void)
{
    /* Wait for app console output finished. */
    while (BOARD_IsAppConsoleBusy() == true)
    {
        ;
    }
    BOARD_UninitPinsAppConsole();
    CLOCK_DisableClock(BOARD_APP_UART_CLK);
}

/* Initialize debug console. */
void BOARD_InitDebugConsole(void)
{
    uint32_t uartClkSrcFreq;
    status_t status;

    CLOCK_SetIpSrc(BOARD_DEBUG_UART_CLK, BOARD_DEBUG_UART_CLKSRC);
    CLOCK_EnableClockLPMode(BOARD_DEBUG_UART_CLK, kCLOCK_IpClkControl_fun3);

    /* Set the pins for the Debug Console */
    BOARD_InitPinsDebugConsole();

    uartClkSrcFreq = BOARD_DEBUG_UART_CLK_FREQ;
    status =
        DbgConsole_Init(BOARD_DEBUG_UART_INSTANCE, BOARD_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, uartClkSrcFreq);
    assert(status == kStatus_Success);
    (void)status;
}

bool BOARD_IsDebugConsoleBusy(void)
{
    return ((((uint32_t)kLPUART_TransmissionCompleteFlag) &
             LPUART_GetStatusFlags((LPUART_Type *)BOARD_DEBUG_UART_BASEADDR)) == 0U);
}

void BOARD_UninitDebugConsole(void)
{
    /* Wait for debug console output finished. */
    while (BOARD_IsDebugConsoleBusy() == true)
    {
        ;
    }

    BOARD_UninitPinsDebugConsole();
    CLOCK_DisableClock(BOARD_DEBUG_UART_CLK);
}

void BOARD_ReinitDebugConsole(void)
{
    status_t status;

    CLOCK_SetIpSrc(BOARD_DEBUG_UART_CLK, BOARD_DEBUG_UART_CLKSRC);

    /* Set the pins for the Debug Console using UART0 */
    BOARD_InitPinsDebugConsole();
    CLOCK_EnableClockLPMode(BOARD_DEBUG_UART_CLK, kCLOCK_IpClkControl_fun3);

    /* we should normaly only need to reenable the clock by CLOCK_EnableClock(kCLOCK_Lpuart0);
      but on wakeup from deep sleep, the UART peripheral needs to be reinitialized
      Call DbgConsole_ExitLowpower() for debug console low power support, should not call DbgConsole_Deinit */
    status = DbgConsole_ExitLowpower();
    assert(status == kStatus_Success);
    (void)status;
}

void BOARD_InitApp2Console(void)
{
    /* set clock */
    CLOCK_SetIpSrc(BOARD_APP2_UART_CLK, BOARD_APP2_UART_CLKSRC);
    /* enable clock with kCLOCK_IpClkControl_fun3 or kCLOCK_IpClkControl_fun2 to
     * keep wake up capability in low power (domain in sleep) */
    CLOCK_EnableClockLPMode(BOARD_APP2_UART_CLK, kCLOCK_IpClkControl_fun3);

    /* init lpuart pins */
    BOARD_InitPinsApp2Console();
}

bool BOARD_IsApp2ConsoleBusy(void)
{
    return ((((uint32_t)kLPUART_TransmissionCompleteFlag) &
             LPUART_GetStatusFlags((LPUART_Type *)BOARD_APP2_UART_BASEADDR)) == 0U);
}

void BOARD_UninitApp2Console(void)
{
    /* Wait for app console output finished. */
    while (BOARD_IsApp2ConsoleBusy() == true)
    {
        ;
    }
    BOARD_UninitPinsApp2Console();
    CLOCK_DisableClock(BOARD_APP2_UART_CLK);
}

#if defined(BOARD_HCI_OVER_UART) && defined(BOARD_HCI_OVER_UART)
void BOARD_InitHciTransport(void)
{
    /* set clock */
    CLOCK_SetIpSrc(BOARD_DEBUG_UART_CLK, BOARD_DEBUG_UART_CLKSRC);
    /* enable clock */
    CLOCK_EnableClock(BOARD_DEBUG_UART_CLK);
    /* init lpuart pins */
    BOARD_InitPinsDebugConsole();
}
#endif

void BOARD_InitPins(void)
{
#if defined(BOARD_DBG_SWO_PIN_ENABLE) && (BOARD_DBG_SWO_PIN_ENABLE != 0)
    BOARD_InitSWO();
#else
    BOARD_DeInitSWO();
#endif
}
void BOARD_InitSWO(void)
{
    BOARD_InitPinSWO();
}

void BOARD_DeInitSWO(void)
{
    BOARD_UnInitPinSWO();
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
