/*
 * Copyright 2022-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_os_abstraction.h"
#include "fsl_os_abstraction_free_rtos.h"

#if defined(RW610)
#include "fsl_usart.h"
#include "fsl_usart_freertos.h"
#elif defined(MIMXRT1062_SERIES)
#include "fsl_lpuart.h"
#include "fsl_lpuart_freertos.h"
#endif

#include "ncp_adapter.h"
#include "ncp_intf_uart.h"
#include "ncp_intf_pm.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/

#if !defined(RW610) && !defined (MIMXRT1062_SERIES)
#error "Please define macro for Redfinch or MIMXRT1060 board"
#endif

#if defined(RW610)
#define PROTOCOL_UART_FRG_CLK \
    (&(const clock_frg_clk_config_t){0, kCLOCK_FrgMainClk, 255, 0}) /*!< Select FRG0 mux as frg_pll */
#define PROTOCOL_UART_CLK_ATTACH  kFRG_to_FLEXCOMM0
#define PROTOCOL_UART             USART0
#define PROTOCOL_UART_CLK_FREQ    CLOCK_GetFlexCommClkFreq(0)
#define PROTOCOL_UART_IRQ         FLEXCOMM0_IRQn
#elif defined(MIMXRT1062_SERIES)
extern uint32_t BOARD_DebugConsoleSrcFreq(void);
#define PROTOCOL_UART           LPUART3
#define PROTOCOL_UART_CLK_FREQ  BOARD_DebugConsoleSrcFreq()
#define PROTOCOL_UART_IRQ       LPUART3_IRQn
#endif

#define PROTOCOL_UART_NVIC_PRIO 5
#define PROTOCOL_UART_BAUDRATE  115200
#define BACKGROUND_BUFFER_SIZE  256

#if CONFIG_NCP_WIFI
#define NCP_UART_TASK_PRIORITY    3
#elif (CONFIG_NCP_BLE)      
#define NCP_UART_TASK_PRIORITY    10
#elif defined(CONFIG_NCP_OT)
#define NCP_UART_TASK_PRIORITY    3
#endif      
      
#define NCP_UART_TASK_STACK_SIZE  1024

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* UART ringbuffer */
static uint8_t ncp_uart_bgbuf[BACKGROUND_BUFFER_SIZE];

#if defined(RW610)
usart_rtos_handle_t ncp_rtos_handle;
usart_handle_t      ncp_t_handle;

struct rtos_usart_config ncp_uart_config = {
    .base        = PROTOCOL_UART,
    .baudrate    = PROTOCOL_UART_BAUDRATE,
    .parity      = kUSART_ParityDisabled,
    .stopbits    = kUSART_OneStopBit,
    .buffer      = ncp_uart_bgbuf,
    .buffer_size = sizeof(ncp_uart_bgbuf),
    .enableHardwareFlowControl = true,
};
#elif defined(MIMXRT1062_SERIES)
lpuart_rtos_handle_t  ncp_rtos_handle;
lpuart_handle_t       ncp_t_handle;

lpuart_rtos_config_t ncp_uart_config = {
    .base        = PROTOCOL_UART,
    .baudrate    = PROTOCOL_UART_BAUDRATE,
    .parity      = kLPUART_ParityDisabled,
    .stopbits    = kLPUART_OneStopBit,
    .buffer      = ncp_uart_bgbuf,
    .buffer_size = sizeof(ncp_uart_bgbuf),
    .enableRxRTS = true,
    .enableTxCTS = true,
};
#endif

static uint8_t ncp_uart_tlvbuf[TLV_CMD_BUF_SIZE];
static void ncp_uart_intf_task(void *argv);

static OSA_TASK_HANDLE_DEFINE(ncp_uartTaskHandle);
static OSA_TASK_DEFINE(ncp_uart_intf_task, NCP_UART_TASK_PRIORITY, 1, NCP_UART_TASK_STACK_SIZE, 0);
#if defined(RW610)
#if CONFIG_HOST_SLEEP
#if CONFIG_POWER_MANAGER
extern bool usart_suspend_flag;
#endif
#endif
#endif

/*******************************************************************************
 * API
 ******************************************************************************/

static void ncp_uart_intf_task(void *argv)
{
    int ret;
    size_t tlv_size = 0;

    ARG_UNUSED(argv);

    while (1)
    {
        ret = ncp_uart_recv(ncp_uart_tlvbuf, &tlv_size);
        if (NCP_STATUS_SUCCESS == ret)
        {
            ncp_tlv_dispatch(ncp_uart_tlvbuf, tlv_size);
        }
        else
        {
            ncp_adap_e("Failed to receive TLV command!");
        }
    }
}

int ncp_uart_init(void *argv)
{
    int ret;

    ARG_UNUSED(argv);

#if defined(RW610)
    /* Attach FRG0 clock to FLEXCOMM0 */
    CLOCK_SetFRGClock(PROTOCOL_UART_FRG_CLK);
    CLOCK_AttachClk(PROTOCOL_UART_CLK_ATTACH);
#endif
    ncp_uart_config.srcclk = PROTOCOL_UART_CLK_FREQ;

    NVIC_SetPriority(PROTOCOL_UART_IRQ, PROTOCOL_UART_NVIC_PRIO);

#if defined(RW610)
    ret = USART_RTOS_Init(&ncp_rtos_handle, &ncp_t_handle, &ncp_uart_config);
#elif defined(MIMXRT1062_SERIES)
    ret = LPUART_RTOS_Init(&ncp_rtos_handle, &ncp_t_handle, &ncp_uart_config);
#endif
    if (kStatus_Success != ret)
    {
        ncp_adap_e("NCP UART interface failed to initialize!");
        return (int)NCP_STATUS_ERROR;
    }

    (void)OSA_TaskCreate((osa_task_handle_t)ncp_uartTaskHandle, OSA_TASK(ncp_uart_intf_task), NULL);

    return (int)NCP_STATUS_SUCCESS;
}

int ncp_uart_deinit(void *argv)
{
    int ret;

    ARG_UNUSED(argv);

#if defined(RW610)
    ret = USART_RTOS_Deinit(&ncp_rtos_handle);
#elif defined(MIMXRT1062_SERIES)
    ret = LPUART_RTOS_Deinit(&ncp_rtos_handle);
#endif
    if (kStatus_Success != ret)
    {
        return (int)NCP_STATUS_ERROR;
    }
    (void)OSA_TaskDestroy((osa_task_handle_t)ncp_uartTaskHandle);

    return (int)NCP_STATUS_SUCCESS;
}

int ncp_uart_recv(uint8_t *tlv_buf, size_t *tlv_sz)
{
    int ret;
    size_t rx_len = 0, cmd_len = 0;
    int tmp_len = 0, total = 0;

    NCP_ASSERT(NULL != tlv_buf);
    NCP_ASSERT(NULL != tlv_sz);

#if defined(RW610)
restart:
#endif
    while (tmp_len != TLV_CMD_HEADER_LEN)
    {
#if defined(RW610)
#if CONFIG_HOST_SLEEP
        if (usart_suspend_flag)
        {
            vTaskDelay(1000);
            goto restart;
        }
#endif
#endif
#if defined(RW610)
        ret = USART_RTOS_Receive(&ncp_rtos_handle, tlv_buf + tmp_len, TLV_CMD_HEADER_LEN, &rx_len);
#elif defined(MIMXRT1062_SERIES)
        /* Once NCP device UART power off, there will be fake RX interrupt generated on RT1060 host side.
        * Check first byte in response buffer to remove dummy byte.
        */
        ret = LPUART_RTOS_Receive(&ncp_rtos_handle, tlv_buf + tmp_len, 1, &rx_len);
        if (tlv_buf[0] == 0x0 || tlv_buf[0] == 0xff)
        {
            ncp_adap_d("Received one dummy byte\r\n");
            continue;
        }
        tmp_len += rx_len;
        total   += rx_len;
        ret = LPUART_RTOS_Receive(&ncp_rtos_handle, tlv_buf + tmp_len, TLV_CMD_HEADER_LEN - 1, &rx_len);
#endif
        tmp_len += rx_len;
        total   += rx_len;
    }

    cmd_len = (tlv_buf[TLV_CMD_SIZE_HIGH_BYTES] << 8) | tlv_buf[TLV_CMD_SIZE_LOW_BYTES];
    tmp_len = 0;
    rx_len  = 0;
    if (cmd_len < TLV_CMD_HEADER_LEN || cmd_len > TLV_CMD_BUF_SIZE)
    {
        NCP_UART_STATS_INC(lenerr);
        NCP_UART_STATS_INC(drop);

        (void)memset(ncp_uart_config.buffer, 0, ncp_uart_config.buffer_size);
        (void)memset(tlv_buf, 0, TLV_CMD_BUF_SIZE);
#if defined(RW610)
        USART_TransferStartRingBuffer(ncp_rtos_handle.base, ncp_rtos_handle.t_state, ncp_uart_config.buffer, ncp_uart_config.buffer_size);
#elif defined(MIMXRT1062_SERIES)
        LPUART_TransferStartRingBuffer(ncp_rtos_handle.base, ncp_rtos_handle.t_state, ncp_uart_config.buffer, ncp_uart_config.buffer_size);
#endif
        total = 0;

        ncp_adap_e("Failed to receive TLV Header!");
        NCP_ASSERT(0);

        return (int)NCP_STATUS_ERROR;
    }

    while (tmp_len != (cmd_len - TLV_CMD_HEADER_LEN + NCP_CHKSUM_LEN))
    {
#if defined(RW610)
#if CONFIG_HOST_SLEEP
        if (usart_suspend_flag)
        {
            vTaskDelay(1000);
            continue;
        }
#endif
#endif
#if defined(RW610)
        ret = USART_RTOS_Receive(&ncp_rtos_handle, tlv_buf + TLV_CMD_HEADER_LEN + tmp_len, cmd_len - TLV_CMD_HEADER_LEN + NCP_CHKSUM_LEN - tmp_len, &rx_len);
#elif defined(MIMXRT1062_SERIES)
        ret = LPUART_RTOS_Receive(&ncp_rtos_handle, tlv_buf + TLV_CMD_HEADER_LEN + tmp_len, cmd_len - TLV_CMD_HEADER_LEN + NCP_CHKSUM_LEN - tmp_len, &rx_len);
#endif
        tmp_len += rx_len;
        total   += rx_len;
        if ((ret == 
#if defined(RW610)
             kStatus_USART_RxRingBufferOverrun
#elif defined(MIMXRT1062_SERIES)
             kStatus_LPUART_RxRingBufferOverrun
#endif
               ) || total >= TLV_CMD_BUF_SIZE)
        {
            NCP_UART_STATS_INC(ringerr);
            NCP_UART_STATS_INC(lenerr);
            NCP_UART_STATS_INC(drop);

            (void)memset(ncp_uart_config.buffer, 0, ncp_uart_config.buffer_size);
            (void)memset(tlv_buf, 0, TLV_CMD_BUF_SIZE);
            total = 0;

            ncp_adap_e("NCP UART interface ring buffer overflow!");
#if defined(RW610)
            NCP_ASSERT(0);
#endif

            return (int)NCP_STATUS_ERROR;
        }
    }

    *tlv_sz = cmd_len;
    NCP_UART_STATS_INC(rx);

    return (int)NCP_STATUS_SUCCESS;
}

int ncp_uart_send(uint8_t *tlv_buf, size_t tlv_sz, tlv_send_callback_t cb)
{
    int ret;

    ARG_UNUSED(cb);

    NCP_ASSERT(NULL != tlv_buf);

#if defined(RW610)
    ret = USART_RTOS_Send(&ncp_rtos_handle, tlv_buf, tlv_sz);
#elif defined(MIMXRT1062_SERIES)
    ret = LPUART_RTOS_Send(&ncp_rtos_handle, tlv_buf, tlv_sz);
#endif
    if (NCP_STATUS_SUCCESS != ret)
    {
        return (int)NCP_STATUS_ERROR;
    }

    NCP_UART_STATS_INC(tx);

    return (int)NCP_STATUS_SUCCESS;
}

static int ncp_uart_pm_enter(int32_t pm_state)
{
    int ret = (int)NCP_PM_STATUS_SUCCESS;

#if defined(RW610)
    if(pm_state == NCP_PM_STATE_PM3)
    {
        xEventGroupSetBits(ncp_rtos_handle.rxEvent, RTOS_USART_COMPLETE);
        ret = ncp_uart_deinit(NULL);
        if(ret != NCP_PM_STATUS_SUCCESS)
        {
            ncp_adap_e("Failed to init UART interface");
            ret = (int)NCP_PM_STATUS_ERROR;
        }
    }
#endif

    return ret;
}

static int ncp_uart_pm_exit(int32_t pm_state)
{
    int ret = (int)NCP_PM_STATUS_SUCCESS;

#if defined(RW610)
    if(pm_state == NCP_PM_STATE_PM3)
    {
        ret = ncp_uart_init(NULL);
        if(ret != NCP_PM_STATUS_SUCCESS)
        {
            ncp_adap_e("Failed to init UART interface");
            ret = (int)NCP_PM_STATUS_ERROR;
        }
    }
#endif

    return ret;
}

static ncp_intf_pm_ops_t ncp_uart_pm_ops =
{
    .enter = ncp_uart_pm_enter,
    .exit  = ncp_uart_pm_exit,
};

ncp_intf_ops_t ncp_uart_ops =
{
    .init   = ncp_uart_init,
    .deinit = ncp_uart_deinit,
    .send   = ncp_uart_send,
    .recv   = ncp_uart_recv,
    .pm_ops = &ncp_uart_pm_ops,
};
