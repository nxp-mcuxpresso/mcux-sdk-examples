/** @file main.c
 *
 *  @brief main file
 *
 *  Copyright 2020-2021,2023 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////

// SDK Included Files
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"

#include "fsl_common.h"
#include "fsl_component_serial_manager.h"

#if defined(RW610_SERIES) || defined(RW612_SERIES)
#include "fsl_adapter_imu.h"
#include "fsl_usart_freertos.h"
#include "fsl_loader.h"
#include "fsl_ocotp.h"
#else
#include "mfg_wlan_bt_fw.h"
#include "wlan.h"
#include "wifi.h"
#include "wm_net.h"
#include <osa.h>
#include "dhcp-server.h"
#include "cli.h"
#include "wifi_ping.h"
#include "iperf.h"
#include "wifi-internal.h"
#include "wifi-sdio.h"
#include "fsl_adapter_gpio.h"
#include "fsl_lpuart_freertos.h"
#include "fsl_lpuart.h"
#include "fsl_sdmmc_host.h"
#if defined(MIMXRT1176_cm7_SERIES)
#include "fsl_lpspi.h"
#endif
#endif

#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define SERIAL_PORT_NVIC_PRIO 5

#if !defined(RW610_SERIES) && !defined(RW612_SERIES)
#define DEMO_LPUART          LPUART1
#define DEMO_LPUART_CLK_FREQ BOARD_DebugConsoleSrcFreq()
#define DEMO_LPUART_IRQn     LPUART1_IRQn
#endif

#define UART_BUF_SIZE           2048
#define LABTOOL_PATTERN_HDR_LEN 4
#define CHECKSUM_LEN            4
#define CRC32_POLY              0x04c11db7

#define LABTOOL_HCI_RESP_HDR_LEN 3

#if defined(MIMXRT1176_cm7_SERIES)
/* SPI related */
#define LPSPI_MASTER_BASEADDR         (LPSPI1)
#define LPSPI_MASTER_IRQN             (LPSPI1_IRQn)
#define LPSPI_MASTER_PCS_FOR_INIT     (kLPSPI_Pcs0)
#define LPSPI_MASTER_PCS_FOR_TRANSFER (kLPSPI_MasterPcs0)

#define LPSPI_MASTER_CLK_FREQ (CLOCK_GetFreqFromObs(CCM_OBS_LPSPI1_CLK_ROOT))

#define LPSPI_DEALY_COUNT 0xFFFFFU
#define TRANSFER_BAUDRATE 500000U /*! Transfer baudrate - 500k */
#endif

/** Command type: WLAN */
#define TYPE_WLAN     0x0002
#define RET_TYPE_WLAN 1

/** Command type: BT */
#define TYPE_BT     0x0003
#define RET_TYPE_BT 2

/** Command type: 15.4 */
#define TYPE_15_4       0x0004
#define RET_TYPE_ZIGBEE 3

#define SDIOPKTTYPE_CMD 0x1
#define BUF_LEN         2048

#if defined(RW610_SERIES) || defined(RW612_SERIES)
#define WM_SUCCESS 0
#define WM_FAIL    1

#define UNUSED(x) (void)(x)

#define REMOTE_EPT_ADDR_BT     (40U)
#define LOCAL_EPT_ADDR_BT      (30U)
#define REMOTE_EPT_ADDR_ZIGBEE (20U)
#define LOCAL_EPT_ADDR_ZIGBEE  (10U)

#define WIFI_REG8(x)  (*(volatile unsigned char *)(x))
#define WIFI_REG16(x) (*(volatile unsigned short *)(x))
#define WIFI_REG32(x) (*(volatile unsigned int *)(x))

#define WIFI_WRITE_REG8(reg, val)  (WIFI_REG8(reg) = (val))
#define WIFI_WRITE_REG16(reg, val) (WIFI_REG16(reg) = (val))
#define WIFI_WRITE_REG32(reg, val) (WIFI_REG32(reg) = (val))

/* Set default mode of fw download */
#ifndef CONFIG_SUPPORT_WIFI
#define CONFIG_SUPPORT_WIFI 1
#endif
#ifndef CONFIG_SUPPORT_BLE
#define CONFIG_SUPPORT_BLE 1
#endif
#ifndef CONFIG_SUPPORT_15D4
#define CONFIG_SUPPORT_15D4 1
#endif

#define WLAN_CAU_ENABLE_ADDR         (0x45004008U)
#define WLAN_CAU_TEMPERATURE_ADDR    (0x4500400CU)
#define WLAN_CAU_TEMPERATURE_FW_ADDR (0x41382490U)
#define WLAN_FW_WAKE_STATUS_ADDR     (0x40031068U)

#if (CONFIG_SUPPORT_WIFI) && (CONFIG_MONOLITHIC_WIFI)
extern const uint32_t fw_cpu1[];
#define WIFI_FW_ADDRESS  (uint32_t)&fw_cpu1[0]
#else
#define WIFI_FW_ADDRESS  0U
#endif

#if (CONFIG_SUPPORT_15D4) && (CONFIG_MONOLITHIC_BLE_15_4)
extern const uint32_t fw_cpu2_combo[];
#define COMBO_FW_ADDRESS (uint32_t)&fw_cpu2_combo[0]
#else
#define COMBO_FW_ADDRESS   0U
#endif

#if ((CONFIG_SUPPORT_BLE) && !(CONFIG_SUPPORT_15D4)) && ((CONFIG_MONOLITHIC_BLE) && !(CONFIG_MONOLITHIC_BLE_15_4))
extern const uint32_t fw_cpu2_ble[];
#define BLE_FW_ADDRESS   (uint32_t)&fw_cpu2_ble[0]
#else
#define BLE_FW_ADDRESS   0U
#endif
#endif

enum
{
    MLAN_CARD_NOT_DETECTED = 3,
    MLAN_STATUS_FW_DNLD_FAILED,
    MLAN_STATUS_FW_NOT_DETECTED = 5,
    MLAN_STATUS_FW_NOT_READY,
    MLAN_STATUS_FW_XZ_FAILED,
    MLAN_CARD_CMD_TIMEOUT
};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint8_t background_buffer[UART_BUF_SIZE];
#if defined(RW610_SERIES) || defined(RW612_SERIES)
usart_rtos_handle_t handle;
struct _usart_handle t_handle;
TimerHandle_t g_wifi_cau_temperature_timer = NULL;

static struct rtos_usart_config usart_config = {
    .baudrate    = 115200,
    .parity      = kUSART_ParityDisabled,
    .stopbits    = kUSART_OneStopBit,
    .buffer      = background_buffer,
    .buffer_size = sizeof(background_buffer),
};

static IMUMC_HANDLE_DEFINE(bt_imumc_handle);
static IMUMC_HANDLE_DEFINE(zigbee_imumc_handle);
static hal_imumc_handle_t imumcHandleList[] = {(hal_imumc_handle_t)bt_imumc_handle,
                                               (hal_imumc_handle_t)zigbee_imumc_handle};

uint32_t remote_ept_list[] = {REMOTE_EPT_ADDR_BT, REMOTE_EPT_ADDR_ZIGBEE};
uint32_t local_ept_list[]  = {LOCAL_EPT_ADDR_BT, LOCAL_EPT_ADDR_ZIGBEE};
#else
lpuart_rtos_handle_t handle;
struct _lpuart_handle t_handle;

lpuart_rtos_config_t lpuart_config = {
    .baudrate    = 115200,
    .parity      = kLPUART_ParityDisabled,
    .stopbits    = kLPUART_OneStopBit,
    .buffer      = background_buffer,
    .buffer_size = sizeof(background_buffer),
};

uint8_t background_buffer_bt[UART_BUF_SIZE];

lpuart_rtos_handle_t handle_bt;
struct _lpuart_handle t_handle_bt;

lpuart_rtos_config_t lpuart_config_bt = {
    .baudrate    = 3000000,
    .parity      = kLPUART_ParityDisabled,
    .stopbits    = kLPUART_OneStopBit,
    .buffer      = background_buffer_bt,
    .buffer_size = sizeof(background_buffer_bt),
    .enableRxRTS = true,
    .enableTxCTS = true,
};
#endif

typedef struct _uart_cb
{ /* uart control block */
    int uart_fd;
    unsigned int crc32_table[256];

    unsigned char uart_buf[UART_BUF_SIZE]; /* uart buffer */

} uart_cb;

static uart_cb uartcb;
#if !defined(RW610_SERIES) && !defined(RW612_SERIES)
static uart_cb uartcb_bt;
#endif

/** UART start pattern*/
typedef struct _uart_header
{
    /** pattern */
    short pattern;
    /** Command length */
    short length;
} uart_header;

/** Labtool command header */
typedef struct _cmd_header
{
    /** Command Type */
    short type;
    /** Command Sub-type */
    short sub_type;
    /** Command length (header+payload) */
    short length;
    /** Command status */
    short status;
    /** reserved */
    int reserved;
} cmd_header;

#if defined(RW610_SERIES) || defined(RW612_SERIES)
#define INTF_HEADER_LEN 4U
#define SDIO_OUTBUF_LEN 2048U

/** HostCmd_DS_COMMAND */
typedef struct _HostCmd_DS_COMMAND
{
    /** Command Header : Command */
    uint16_t command;
    /** Command Header : Size */
    uint16_t size;
    /** Command Header : Sequence number */
    uint16_t seq_num;
    /** Command Header : Result */
    uint16_t result;
    /** Command Body */
} HostCmd_DS_COMMAND;

/** SDIOPkt/IMUPkt only name difference, same definition */
typedef struct _IMUPkt
{
    uint16_t size;
    uint16_t pkttype;
    HostCmd_DS_COMMAND hostcmd;
} IMUPkt;

#endif

static uint8_t rx_buf[BUF_LEN];
static cmd_header last_cmd_hdr;
static uint8_t local_outbuf[BUF_LEN];

#if defined(MIMXRT1176_cm7_SERIES)
lpspi_master_config_t spiConfig;
lpspi_transfer_t handle_spi;
#endif
uint8_t host_resp_buf[BUF_LEN];
uint32_t resp_buf_len, reqd_resp_len;

/*******************************************************************************
 * Code
 ******************************************************************************/
#define MAIN_TASK_STACK_SIZE 4096

static void main_task(osa_task_param_t arg);

static OSA_TASK_DEFINE(main_task, PRIORITY_RTOS_TO_OSA((configMAX_PRIORITIES - 2)), 1, MAIN_TASK_STACK_SIZE, 0);

OSA_TASK_HANDLE_DEFINE(main_task_Handle);

#define SDK_VERSION "NXPSDK_2.15.0_r48.p1"

static void uart_init_crc32(uart_cb *uartcb)
{
    int i, j;
    unsigned int c;
    for (i = 0; i < 256; ++i)
    {
        for (c = i << 24, j = 8; j > 0; --j)
            c = c & 0x80000000 ? (c << 1) ^ CRC32_POLY : (c << 1);
        uartcb->crc32_table[i] = c;
    }
}

static uint32_t uart_get_crc32(uart_cb *uart, int len, unsigned char *buf)
{
    unsigned int *crc32_table = uart->crc32_table;
    unsigned char *p;
    unsigned int crc;
    crc = 0xffffffff;
    for (p = buf; len > 0; ++p, --len)
        crc = (crc << 8) ^ (crc32_table[(crc >> 24) ^ *p]);
    return ~crc;
}

/*
 send_response_to_uart() handles the response from the firmware.
 This involves
 1. replacing the sdio header with the uart header
 2. computation of the crc of the payload
 3. sending it out to the uart
*/
static int send_response_to_uart(uart_cb *uart, uint8_t *resp, int type, uint32_t reqd_resp_len)
{
    uint32_t bridge_chksum = 0;
    uint32_t msglen;
    int index;
    uint32_t payloadlen;
    uart_header *uart_hdr;
    int iface_len = 0;

#if defined(RW610_SERIES) || defined(RW612_SERIES)
    IMUPkt *imupkt = (IMUPkt *)resp;

    if (type == 2)
        /* This is because, the last byte of the imupkt header
         * (packet type) is also requried by the labtool, to
         * understand the type of packet and take appropriate action */
        iface_len = INTF_HEADER_LEN - 1;
    else
        iface_len = INTF_HEADER_LEN;
    payloadlen = imupkt->size - iface_len;
#else
    payloadlen = reqd_resp_len;
#endif
    memset(rx_buf, 0, BUF_LEN);
    memcpy(rx_buf + sizeof(uart_header) + sizeof(cmd_header), resp + iface_len, payloadlen);

    /* Added to send correct cmd header len */
    cmd_header *cmd_hdr;
    cmd_hdr         = &last_cmd_hdr;
    cmd_hdr->length = payloadlen + sizeof(cmd_header);

    memcpy(rx_buf + sizeof(uart_header), (uint8_t *)&last_cmd_hdr, sizeof(cmd_header));

    uart_hdr          = (uart_header *)rx_buf;
    uart_hdr->length  = payloadlen + sizeof(cmd_header);
    uart_hdr->pattern = 0x5555;

    /* calculate CRC. The uart_header is excluded */
    msglen        = payloadlen + sizeof(cmd_header);
    bridge_chksum = uart_get_crc32(uart, msglen, rx_buf + sizeof(uart_header));
    index         = sizeof(uart_header) + msglen;

    rx_buf[index]     = bridge_chksum & 0xff;
    rx_buf[index + 1] = (bridge_chksum & 0xff00) >> 8;
    rx_buf[index + 2] = (bridge_chksum & 0xff0000) >> 16;
    rx_buf[index + 3] = (bridge_chksum & 0xff000000) >> 24;

    /* write response to uart */
#if defined(RW610_SERIES) || defined(RW612_SERIES)
    USART_RTOS_Send(&handle, rx_buf, payloadlen + sizeof(cmd_header) + sizeof(uart_header) + 4);
#else
    LPUART_RTOS_Send(&handle, rx_buf, payloadlen + sizeof(cmd_header) + sizeof(uart_header) + 4);
#endif

    memset(rx_buf, 0, BUF_LEN);

    return 0;
}

/*
 check_command_complete() validates the command from the uart.
 It checks for the signature in the header and the crc of the
 payload. This assumes that the uart_buf is circular and data
 can be wrapped.
*/
int check_command_complete(uint8_t *buf)
{
    uart_header *uarthdr;
    uint32_t msglen, endofmsgoffset;
    uart_cb *uart = &uartcb;
    int checksum = 0, bridge_checksum = 0;

    uarthdr = (uart_header *)buf;

    /* out of sync */
    if (uarthdr->pattern != 0x5555)
    {
        PRINTF("Pattern mismatch\r\n");
        return -WM_FAIL;
    }
    /* check crc */
    msglen = uarthdr->length;

    /* add 4 for checksum */
    endofmsgoffset = sizeof(uart_header) + msglen + 4;

    memset((uint8_t *)local_outbuf, 0, sizeof(local_outbuf));
    if (endofmsgoffset < UART_BUF_SIZE)
    {
        memcpy((uint8_t *)local_outbuf, buf, endofmsgoffset);
    }
    else
    {
        memcpy((uint8_t *)local_outbuf, buf, UART_BUF_SIZE);
        /* To do : check if copying method is correct */
        memcpy((uint8_t *)local_outbuf + UART_BUF_SIZE, buf, endofmsgoffset);
    }

    checksum = *(int *)((uint8_t *)local_outbuf + sizeof(uart_header) + msglen);

    bridge_checksum = uart_get_crc32(uart, msglen, (uint8_t *)local_outbuf + sizeof(uart_header));
    if (checksum == bridge_checksum)
    {
        return WM_SUCCESS;
    }
    /* Reset local outbuf */
    memset(local_outbuf, 0, BUF_LEN);

    return -WM_FAIL;
}

#if defined(RW610_SERIES) || defined(RW612_SERIES)
hal_imumc_status_t wifi_send_imu_raw_data(uint8_t *data, uint32_t length)
{
    if (data == NULL || length == 0)
        return kStatus_HAL_ImumcError;

    if (kStatus_HAL_ImumcSuccess != (HAL_ImuSendCommand(kIMU_LinkCpu1Cpu3, data, length)))
    {
        return kStatus_HAL_ImumcError;
    }

    return kStatus_HAL_ImumcSuccess;
}

int imumc_raw_packet_send(uint8_t *buf, int m_len, uint8_t t_type)
{
    uint32_t payloadlen;

    cmd_header *cmd_hd = (cmd_header *)(buf + sizeof(uart_header));

    payloadlen = m_len - sizeof(uart_header) - sizeof(cmd_header) - 4;

    memset(local_outbuf, 0, BUF_LEN);
    memcpy(local_outbuf, buf + sizeof(uart_header) + sizeof(cmd_header), payloadlen);

    memcpy(&last_cmd_hdr, cmd_hd, sizeof(cmd_header));

    if (kStatus_HAL_ImumcSuccess !=
        (HAL_ImumcSend((hal_imumc_handle_t)imumcHandleList[t_type - 2], local_outbuf, payloadlen)))
    {
        return kStatus_HAL_ImumcError;
    }

    memset(local_outbuf, 0, BUF_LEN);

    return t_type;
}
#else
int bt_raw_packet_send(uint8_t *buf, int m_len)
{
    uint32_t payloadlen;

    cmd_header *cmd_hd = (cmd_header *)(buf + sizeof(uart_header));

    payloadlen = m_len - sizeof(uart_header) - sizeof(cmd_header) - 4;

    memset(local_outbuf, 0, BUF_LEN);
    memcpy(local_outbuf, buf + sizeof(uart_header) + sizeof(cmd_header), payloadlen);

    memcpy(&last_cmd_hdr, cmd_hd, sizeof(cmd_header));

    LPUART_RTOS_Send(&handle_bt, local_outbuf, payloadlen);

    memset(local_outbuf, 0, BUF_LEN);

    return RET_TYPE_BT;
}

#if defined(MIMXRT1176_cm7_SERIES)
int zigbee_raw_packet_send(uint8_t *buf, int m_len)
{
    uint32_t payloadlen;

    cmd_header *cmd_hd = (cmd_header *)(buf + sizeof(uart_header));

    payloadlen = m_len - sizeof(uart_header) - sizeof(cmd_header) - 4;

    memset(local_outbuf, 0, BUF_LEN);
    memcpy(local_outbuf, buf + sizeof(uart_header) + sizeof(cmd_header), payloadlen);

    memcpy(&last_cmd_hdr, cmd_hd, sizeof(cmd_header));

    handle_spi.txData   = local_outbuf;
    handle_spi.rxData   = NULL;
    handle_spi.dataSize = payloadlen;

    handle_spi.configFlags = LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous | kLPSPI_MasterByteSwap;

    LPSPI_MasterTransferBlocking(LPSPI_MASTER_BASEADDR, &handle_spi);

    memset(local_outbuf, 0, BUF_LEN);

    return RET_TYPE_ZIGBEE;
}
#endif
#endif

/*
 process_input_cmd() sends command to the wlan
 card
*/
int process_input_cmd(uint8_t *buf, int m_len)
{
    uart_header *uarthdr;
    int i, ret = -WM_FAIL;
    uint8_t *s, *d;
    cmd_header *cmd_hd = (cmd_header *)(buf + sizeof(uarthdr));

    if (cmd_hd->type == TYPE_WLAN)
    {
        memset(local_outbuf, 0, BUF_LEN);

        uarthdr = (uart_header *)buf;
#if defined(RW610_SERIES) || defined(RW612_SERIES)
        IMUPkt *imupkt = (IMUPkt *)local_outbuf;
        /* imupkt = local_outbuf */
        imupkt->pkttype = SDIOPKTTYPE_CMD;

        imupkt->size = m_len - sizeof(cmd_header) + INTF_HEADER_LEN;
        d            = (uint8_t *)local_outbuf + INTF_HEADER_LEN;
        s            = (uint8_t *)buf + sizeof(uart_header) + sizeof(cmd_header);
#else
        d   = (uint8_t *)local_outbuf;
        s   = (uint8_t *)buf + sizeof(uart_header) + sizeof(cmd_header);
#endif

        for (i = 0; i < uarthdr->length - sizeof(cmd_header); i++)
        {
            if (s < buf + UART_BUF_SIZE)
                *d++ = *s++;
            else
            {
                s    = buf;
                *d++ = *s++;
            }
        }
        d = (uint8_t *)&last_cmd_hdr;
        s = (uint8_t *)buf + sizeof(uart_header);

        for (i = 0; i < sizeof(cmd_header); i++)
        {
            if (s < buf + UART_BUF_SIZE)
                *d++ = *s++;
            else
            {
                s    = buf;
                *d++ = *s++;
            }
        }
#if defined(RW610_SERIES) || defined(RW612_SERIES)
        wifi_send_imu_raw_data(local_outbuf, (m_len - sizeof(cmd_header) + INTF_HEADER_LEN));
#endif
        ret = RET_TYPE_WLAN;
    }
    else if (cmd_hd->type == TYPE_BT)
    {
#if defined(RW610_SERIES) || defined(RW612_SERIES)
        ret = imumc_raw_packet_send(buf, m_len, RET_TYPE_BT);
#else
        ret = bt_raw_packet_send(buf, m_len);
#endif
    }
    else if (cmd_hd->type == TYPE_15_4)
    {
#if defined(RW610_SERIES) || defined(RW612_SERIES)
        ret = imumc_raw_packet_send(buf, m_len, RET_TYPE_ZIGBEE);
#elif defined(MIMXRT1176_cm7_SERIES)
        ret = zigbee_raw_packet_send(buf, m_len);
#endif
    }

    return ret;
}

#if defined(RW610_SERIES) || defined(RW612_SERIES)
void send_imumc_response_to_uart(uint8_t *resp, int msg_len)
{
    uint32_t bridge_chksum = 0;
    uint32_t msglen;
    int index;
    uint32_t payloadlen;
    uart_header *uart_hdr;
    uart_cb *uart = &uartcb;
    payloadlen    = msg_len;

    memset(rx_buf, 0, BUF_LEN);
    memcpy(rx_buf + sizeof(uart_header) + sizeof(cmd_header), resp, payloadlen);

    /* Added to send correct cmd header len */
    cmd_header *cmd_hdr;
    cmd_hdr         = &last_cmd_hdr;
    cmd_hdr->length = payloadlen + sizeof(cmd_header);

    memcpy(rx_buf + sizeof(uart_header), (uint8_t *)&last_cmd_hdr, sizeof(cmd_header));

    uart_hdr          = (uart_header *)rx_buf;
    uart_hdr->length  = payloadlen + sizeof(cmd_header);
    uart_hdr->pattern = 0x5555;

    /* calculate CRC. The uart_header is excluded */
    msglen        = payloadlen + sizeof(cmd_header);
    bridge_chksum = uart_get_crc32(uart, msglen, rx_buf + sizeof(uart_header));
    index         = sizeof(uart_header) + msglen;

    rx_buf[index]     = bridge_chksum & 0xff;
    rx_buf[index + 1] = (bridge_chksum & 0xff00) >> 8;
    rx_buf[index + 2] = (bridge_chksum & 0xff0000) >> 16;
    rx_buf[index + 3] = (bridge_chksum & 0xff000000) >> 24;

    /* write response to uart */
    USART_RTOS_Send(&handle, rx_buf, payloadlen + sizeof(cmd_header) + sizeof(uart_header) + 4);
    memset(rx_buf, 0, BUF_LEN);
}
#else
void send_bt_response_to_uart(uart_cb *uart_bt, int msg_len)
{
    uint32_t bridge_chksum = 0;
    uint32_t msglen;
    int index;
    uint32_t payloadlen;
    uart_header *uart_hdr;
    uart_cb *uart = &uartcb;

    payloadlen = msg_len;

    memset(rx_buf, 0, BUF_LEN);
    memcpy(rx_buf + sizeof(uart_header) + sizeof(cmd_header), uart_bt->uart_buf, payloadlen);

    /* Added to send correct cmd header len */
    cmd_header *cmd_hdr;
    cmd_hdr         = &last_cmd_hdr;
    cmd_hdr->length = payloadlen + sizeof(cmd_header);

    memcpy(rx_buf + sizeof(uart_header), (uint8_t *)&last_cmd_hdr, sizeof(cmd_header));

    uart_hdr          = (uart_header *)rx_buf;
    uart_hdr->length  = payloadlen + sizeof(cmd_header);
    uart_hdr->pattern = 0x5555;

    /* calculate CRC. The uart_header is excluded */
    msglen        = payloadlen + sizeof(cmd_header);
    bridge_chksum = uart_get_crc32(uart, msglen, rx_buf + sizeof(uart_header));
    index         = sizeof(uart_header) + msglen;

    rx_buf[index]     = bridge_chksum & 0xff;
    rx_buf[index + 1] = (bridge_chksum & 0xff00) >> 8;
    rx_buf[index + 2] = (bridge_chksum & 0xff0000) >> 16;
    rx_buf[index + 3] = (bridge_chksum & 0xff000000) >> 24;

    /* write response to uart */
    LPUART_RTOS_Send(&handle, rx_buf, payloadlen + sizeof(cmd_header) + sizeof(uart_header) + 4);
    memset(rx_buf, 0, BUF_LEN);
}

#if defined(MIMXRT1176_cm7_SERIES)
void send_zigbee_response_to_uart(uint8_t *rxData, uint32_t payloadlen)
{
    uint32_t bridge_chksum = 0;
    uint32_t msglen;
    int index;
    uart_header *uart_hdr;
    uart_cb *uart = &uartcb;

    memset(rx_buf, 0, BUF_LEN);
    memcpy(rx_buf + sizeof(uart_header) + sizeof(cmd_header), rxData, payloadlen);

    /* Added to send correct cmd header len */
    cmd_header *cmd_hdr;
    cmd_hdr         = &last_cmd_hdr;
    cmd_hdr->length = payloadlen + sizeof(cmd_header);

    memcpy(rx_buf + sizeof(uart_header), (uint8_t *)&last_cmd_hdr, sizeof(cmd_header));

    uart_hdr          = (uart_header *)rx_buf;
    uart_hdr->length  = payloadlen + sizeof(cmd_header);
    uart_hdr->pattern = 0x5555;

    /* calculate CRC. The uart_header is excluded */
    msglen        = payloadlen + sizeof(cmd_header);
    bridge_chksum = uart_get_crc32(uart, msglen, rx_buf + sizeof(uart_header));
    index         = sizeof(uart_header) + msglen;

    rx_buf[index]     = bridge_chksum & 0xff;
    rx_buf[index + 1] = (bridge_chksum & 0xff00) >> 8;
    rx_buf[index + 2] = (bridge_chksum & 0xff0000) >> 16;
    rx_buf[index + 3] = (bridge_chksum & 0xff000000) >> 24;

    /* write response to uart */
    LPUART_RTOS_Send(&handle, rx_buf, payloadlen + sizeof(cmd_header) + sizeof(uart_header) + 4);
    memset(rx_buf, 0, BUF_LEN);
}
#endif
#endif

/*
 read_wlan_resp() handles the responses from the wlan card.
 It waits on wlan card interrupts on account
 of command responses are handled here. The response is
 read and then sent through the uart to the Mfg application
*/
#if defined(RW610_SERIES) || defined(RW612_SERIES)
hal_imumc_status_t read_wlan_resp(IMU_Msg_t *pImuMsg, uint32_t len)
{
    assert(NULL != pImuMsg);
    assert(0 != len);
    assert(IMU_MSG_COMMAND_RESPONSE == pImuMsg->Hdr.type);

    uart_cb *uart = &uartcb;

    send_response_to_uart(uart, (uint8_t *)(pImuMsg->PayloadPtr[0]), 1, len);

    return kStatus_HAL_ImumcSuccess;
}

hal_imumc_return_status_t read_imumc_resp(void *param, uint8_t *packet, uint32_t len)
{
    assert(NULL != packet);
    assert(0 != len);

    send_imumc_response_to_uart(packet, len);

    return kStatus_HAL_RL_RELEASE;
}
#else
void read_wlan_resp()
{
    // uart_cb *uart = &uartcb;
    t_u8 *packet;
    t_u32 pkt_type;
    int rv = wifi_raw_packet_recv(&packet, &pkt_type);
    if (rv != WM_SUCCESS)
        PRINTF("Receive response failed\r\n");
    else
    {
        //        if (pkt_type == MLAN_TYPE_CMD)
        //            send_response_to_uart(uart, packet, 1);
    }
}

void read_bt_resp()
{
    uart_cb *uart_bt = &uartcb_bt;
    uint32_t msglen;
    uint32_t payloadlen = 0;
    uint32_t currentlen = 0;
    size_t uart_rx_len  = 0;
    int len;

    memset(uart_bt->uart_buf, 0, sizeof(uart_bt->uart_buf));

    do
    {
        len         = 0;
        uart_rx_len = 0;
        currentlen  = payloadlen;

        while (len != LABTOOL_HCI_RESP_HDR_LEN)
        {
            LPUART_RTOS_Receive(&handle_bt, uart_bt->uart_buf + len + payloadlen, LABTOOL_HCI_RESP_HDR_LEN,
                                &uart_rx_len);
            len += uart_rx_len;
        }

        msglen = uart_bt->uart_buf[currentlen + 2];
        payloadlen += LABTOOL_HCI_RESP_HDR_LEN;
        uart_rx_len = 0;
        len         = 0;

        while (len != msglen)
        {
            LPUART_RTOS_Receive(&handle_bt, uart_bt->uart_buf + len + payloadlen, msglen - len, &uart_rx_len);
            len += uart_rx_len;
        }

        payloadlen += len;

    } while (uart_bt->uart_buf[currentlen + 1] != 0x0E);

    send_bt_response_to_uart(uart_bt, payloadlen);
    memset(uart_bt->uart_buf, 0, sizeof(uart_bt->uart_buf));
}

#if defined(MIMXRT1176_cm7_SERIES)
void read_zigbee_resp()
{
    handle_spi.txData   = NULL;
    handle_spi.rxData   = local_outbuf;
    handle_spi.dataSize = BUF_LEN;

    handle_spi.configFlags = LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous | kLPSPI_MasterByteSwap;

    LPSPI_MasterTransferBlocking(LPSPI_MASTER_BASEADDR, &handle_spi);

    send_zigbee_response_to_uart(local_outbuf, BUF_LEN);

    memset(local_outbuf, 0, BUF_LEN);
}
#endif
#endif

#if defined(RW610_SERIES) || defined(RW612_SERIES)
static hal_imumc_status_t imu_wifi_config()
{
    hal_imumc_status_t state = kStatus_HAL_ImumcSuccess;

    /* Assign IMU channel for CPU1-CPU3 communication */
    state = HAL_ImuInit(kIMU_LinkCpu1Cpu3);
    assert(kStatus_HAL_ImumcSuccess == state);

    HAL_ImuInstallCallback(kIMU_LinkCpu1Cpu3, read_wlan_resp, IMU_MSG_COMMAND_RESPONSE);

    return state;
}

#if (defined(CONFIG_SUPPORT_BLE) && (CONFIG_SUPPORT_BLE == 1)) || \
    (defined(CONFIG_SUPPORT_15D4) && (CONFIG_SUPPORT_15D4 == 1))
static hal_imumc_status_t imumc_config(uint32_t linkId)
{
    hal_imumc_status_t state = kStatus_HAL_ImumcSuccess;

    hal_imumc_config_t config = {0};
    /* Init IMUMC/IMU Channel */
    config.local_addr  = local_ept_list[linkId];
    config.remote_addr = remote_ept_list[linkId];
    config.imuLink     = kIMU_LinkCpu2Cpu3;
    state              = HAL_ImumcInit((hal_imumc_handle_t)imumcHandleList[linkId], &config);
    assert(kStatus_HAL_ImumcSuccess == state);

    /* IMUMC install rx callback */
    state = HAL_ImumcInstallRxCallback((hal_imumc_handle_t)imumcHandleList[linkId], read_imumc_resp, NULL);
    assert(kStatus_HAL_ImumcSuccess == state);

    return state;
}
#endif

static hal_imumc_status_t imumc_init()
{
#if (defined(CONFIG_SUPPORT_BLE) && (CONFIG_SUPPORT_BLE == 1)) || \
    (defined(CONFIG_SUPPORT_15D4) && (CONFIG_SUPPORT_15D4 == 1))
    uint32_t linkId;
#endif
    hal_imumc_status_t state = kStatus_HAL_ImumcSuccess;

    /* Init IMUMC/IMU Channel */
#if defined(CONFIG_SUPPORT_BLE) && (CONFIG_SUPPORT_BLE == 1)
    linkId = 0;
    state  = imumc_config(linkId);
#endif
#if defined(CONFIG_SUPPORT_15D4) && (CONFIG_SUPPORT_15D4 == 1)
    linkId = 1;
    state  = imumc_config(linkId);
#endif

    return state;
}

#define RW610_PACKAGE_TYPE_QFN 0
#define RW610_PACKAGE_TYPE_CSP 1
#define RW610_PACKAGE_TYPE_BGA 2

void wifi_cau_temperature_enable()
{
    uint32_t val;

    val = WIFI_REG32(WLAN_CAU_ENABLE_ADDR);
    val &= ~(0xC);
    val |= (2 << 2);
    WIFI_WRITE_REG32(WLAN_CAU_ENABLE_ADDR, val);
}

static uint32_t wifi_get_board_type()
{
    status_t status;
    static uint32_t wifi_rw610_package_type = 0xFFFFFFFF;

    if (0xFFFFFFFF == wifi_rw610_package_type)
    {
        OCOTP_OtpInit();
        status = OCOTP_ReadPackage(&wifi_rw610_package_type);
        if (status != kStatus_Success)
        {
            /*If status error, use BGA as default type*/
            wifi_rw610_package_type = RW610_PACKAGE_TYPE_BGA;
        }
        OCOTP_OtpDeinit();
    }

    return wifi_rw610_package_type;
}

int32_t wifi_get_temperature(void)
{
    int32_t val                   = 0;
    uint32_t reg_val              = 0;
    uint32_t temp_Cau_Raw_Reading = 0;
    uint32_t board_type           = 0;

    reg_val              = WIFI_REG32(WLAN_CAU_TEMPERATURE_ADDR);
    temp_Cau_Raw_Reading = ((reg_val & 0XFFC00) >> 10);
    board_type           = wifi_get_board_type();

    switch (board_type)
    {
        case RW610_PACKAGE_TYPE_QFN:
            val = (((((int32_t)(temp_Cau_Raw_Reading)) * 484260) - 220040600) / 1000000);
            break;

        case RW610_PACKAGE_TYPE_CSP:
            val = (((((int32_t)(temp_Cau_Raw_Reading)) * 480560) - 220707000) / 1000000);
            break;

        case RW610_PACKAGE_TYPE_BGA:
            val = (((((int32_t)(temp_Cau_Raw_Reading)) * 480561) - 220707400) / 1000000);
            break;

        default:
            PRINTF("Unknown board type, use BGA temperature \r\n");
            val = (((((int32_t)(temp_Cau_Raw_Reading)) * 480561) - 220707400) / 1000000);
            break;
    }

    return val;
}

void wifi_cau_temperature_write_to_firmware()
{
    int32_t val = 0;

    val = wifi_get_temperature();
    WIFI_WRITE_REG32(WLAN_CAU_TEMPERATURE_FW_ADDR, val);
}

static void wifi_cau_temperature_timer_cb(TimerHandle_t timer)
{
    /* write CAU temperature to CPU1 when it is not sleeping */
    if ((WIFI_REG32(WLAN_FW_WAKE_STATUS_ADDR) & 0x0CU) != 0x0CU)
    {
        wifi_cau_temperature_write_to_firmware();
    }
}
#endif

/*
 task_main() runs in a loop. It polls the uart ring buffer
 checks it for a complete command and sends the command to the
 wlan card
*/
static void main_task(osa_task_param_t arg)
{
    int32_t result = 0;
    (void)result;
#if defined(MIMXRT1176_cm7_SERIES)
    uint32_t srcClock_Hz;
#endif

#if !defined(RW610_SERIES) && !defined(RW612_SERIES)
    result = wifi_init_fcc(wlan_fw_bin, wlan_fw_bin_len);
    if (result != 0)
    {
        switch (result)
        {
            case MLAN_CARD_CMD_TIMEOUT:
            case MLAN_CARD_NOT_DETECTED:
                result = -WIFI_ERROR_CARD_NOT_DETECTED;
                break;
            case MLAN_STATUS_FW_DNLD_FAILED:
                result = -WIFI_ERROR_FW_DNLD_FAILED;
                break;
            case MLAN_STATUS_FW_NOT_DETECTED:
                result = -WIFI_ERROR_FW_NOT_DETECTED;
                break;
            case MLAN_STATUS_FW_NOT_READY:
                result = -WIFI_ERROR_FW_NOT_READY;
                break;
        }
        PRINTF("sd_wifi_init failed, result:%d\r\n", result);
    }

    assert(WM_SUCCESS == result);
#endif

#if defined(RW610_SERIES) || defined(RW612_SERIES)
    NVIC_SetPriority(BOARD_UART_IRQ, 5);
    usart_config.srcclk = BOARD_DEBUG_UART_CLK_FREQ;
    usart_config.base   = BOARD_DEBUG_UART;

    if (kStatus_Success != USART_RTOS_Init(&handle, &t_handle, &usart_config))
    {
        vTaskSuspend(NULL);
    }
#else
    NVIC_SetPriority(LPUART1_IRQn, 5);
#if defined(MIMXRT1176_cm7_SERIES)
    NVIC_SetPriority(LPUART7_IRQn, HAL_UART_ISR_PRIORITY);
#else
    NVIC_SetPriority(LPUART3_IRQn, HAL_UART_ISR_PRIORITY);
#endif

    lpuart_config.srcclk = DEMO_LPUART_CLK_FREQ;
    lpuart_config.base   = DEMO_LPUART;

    if (kStatus_Success != LPUART_RTOS_Init(&handle, &t_handle, &lpuart_config))
    {
        vTaskSuspend(NULL);
    }

    lpuart_config_bt.srcclk = BOARD_BT_UART_CLK_FREQ;
#if defined(MIMXRT1176_cm7_SERIES)
    lpuart_config_bt.base   = LPUART7;
#else
    lpuart_config_bt.base = LPUART3;
#endif

    if (kStatus_Success != LPUART_RTOS_Init(&handle_bt, &t_handle_bt, &lpuart_config_bt))
    {
        vTaskSuspend(NULL);
    }
#endif

#if defined(MIMXRT1176_cm7_SERIES)
    LPSPI_MasterGetDefaultConfig(&spiConfig);
    spiConfig.baudRate = TRANSFER_BAUDRATE;
    spiConfig.whichPcs = LPSPI_MASTER_PCS_FOR_INIT;

    srcClock_Hz = LPSPI_MASTER_CLK_FREQ;
    LPSPI_MasterInit(LPSPI_MASTER_BASEADDR, &spiConfig, srcClock_Hz);
#endif

    uart_cb *uart = &uartcb;
    uart_init_crc32(uart);

#if defined(RW610_SERIES) || defined(RW612_SERIES)
    /* Download firmware */
#if (CONFIG_SUPPORT_WIFI == 0) && (CONFIG_SUPPORT_15D4 == 0) && (CONFIG_SUPPORT_BLE == 0)
#error \
    "One of CONFIG_SUPPORT_WIFI CONFIG_SUPPORT_15D4 and CONFIG_SUPPORT_BLE should be defined, or it will not download any formware!!"
#endif
#if (CONFIG_SUPPORT_WIFI) && (CONFIG_SUPPORT_WIFI == 1)
    sb3_fw_reset(LOAD_WIFI_FIRMWARE, 1, WIFI_FW_ADDRESS);
#endif

    wifi_cau_temperature_enable();
    wifi_cau_temperature_write_to_firmware();

#if (CONFIG_SUPPORT_15D4 == 1)
    /* 15d4 single and 15d4+ble combo */
    sb3_fw_reset(LOAD_15D4_FIRMWARE, 1, COMBO_FW_ADDRESS);
#elif (CONFIG_SUPPORT_BLE == 1)
    /* only ble, no 15d4 */
    sb3_fw_reset(LOAD_BLE_FIRMWARE, 1, BLE_FW_ADDRESS);
#endif

    /* Initialize WIFI Driver */
    imu_wifi_config();

#if (CONFIG_SUPPORT_15D4 == 1) || (CONFIG_SUPPORT_BLE == 1)
    /* Initialize imumc */
    imumc_init();
#endif
    /* Initialize CAU temperature timer */
    g_wifi_cau_temperature_timer =
        xTimerCreate("CAU Timer", 5000 / portTICK_PERIOD_MS, pdTRUE, NULL, wifi_cau_temperature_timer_cb);
    if (g_wifi_cau_temperature_timer == NULL)
    {
        PRINTF("Failed to create CAU temperature timer\r\n");
        while (1)
        {
        }
    }

    result = xTimerStart(g_wifi_cau_temperature_timer, 5000 / portTICK_PERIOD_MS);
    if (result != pdPASS)
    {
        PRINTF("Failed to start CAU temperature timer\r\n");
        while (1)
        {
        }
    }
#endif
    size_t uart_rx_len = 0;
    int len            = 0;
    int msg_len        = 0;
    while (1)
    {
        len         = 0;
        msg_len     = 0;
        uart_rx_len = 0;
        memset(uart->uart_buf, 0, sizeof(uart->uart_buf));
        while (len != LABTOOL_PATTERN_HDR_LEN)
        {
#if defined(RW610_SERIES) || defined(RW612_SERIES)
            USART_RTOS_Receive(&handle, uart->uart_buf + len, LABTOOL_PATTERN_HDR_LEN, &uart_rx_len);
#else
            LPUART_RTOS_Receive(&handle, uart->uart_buf + len, LABTOOL_PATTERN_HDR_LEN, &uart_rx_len);
#endif
            len += uart_rx_len;
        }

        /* Length of the packet is indicated by byte[2] & byte[3] of
        the packet excluding header[4 bytes] + checksum [4 bytes]
        */
        msg_len     = (uart->uart_buf[3] << 8) + uart->uart_buf[2];
        len         = 0;
        uart_rx_len = 0;
        while (len != msg_len + CHECKSUM_LEN)
        {
#if defined(RW610_SERIES) || defined(RW612_SERIES)
            USART_RTOS_Receive(&handle, uart->uart_buf + LABTOOL_PATTERN_HDR_LEN + len, msg_len + CHECKSUM_LEN - len,
                               &uart_rx_len);
#else
            LPUART_RTOS_Receive(&handle, uart->uart_buf + LABTOOL_PATTERN_HDR_LEN + len, msg_len + CHECKSUM_LEN - len,
                                &uart_rx_len);
#endif
            len += uart_rx_len;
        }

        /* validate the command including checksum */
        if (check_command_complete(uart->uart_buf) == WM_SUCCESS)
        {
            /* send fw cmd over SDIO after
               stripping off uart header */
            int ret = process_input_cmd(uart->uart_buf, msg_len + 8);
            memset(uart->uart_buf, 0, sizeof(uart->uart_buf));
            memset(host_resp_buf, 0x00, BUF_LEN);
#if defined(RW610_SERIES) || defined(RW612_SERIES)
            UNUSED(ret);
#else
            if (ret == RET_TYPE_WLAN)
            {
                vTaskDelay(pdMS_TO_TICKS(60));
                int rv = wlan_send_hostcmd(local_outbuf, BUF_LEN, host_resp_buf, BUF_LEN, &reqd_resp_len);
                if (rv != WM_SUCCESS)
                    PRINTF("Receive response failed\r\n");
                else
                {
                    send_response_to_uart(uart, host_resp_buf, RET_TYPE_WLAN, reqd_resp_len);
                }
            }
            else if (ret == RET_TYPE_BT)
            {
                read_bt_resp();
            }
#if defined(MIMXRT1176_cm7_SERIES)
            else if (ret == RET_TYPE_ZIGBEE)
            {
                read_zigbee_resp();
            }
#endif
#endif
        }
        else
        {
            memset(background_buffer, 0, UART_BUF_SIZE);
        }
    }
}

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
int main(void)
{
    osa_status_t status = KOSA_StatusSuccess;
    (void)status;

    OSA_Init();

#if defined(MIMXRT1176_cm7_SERIES)
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitBTUARTPins();
    BOARD_InitDebugConsole();
    BOARD_InitSpiPins();
#else
    extern void BOARD_InitHardware(void);
    BOARD_InitBootPins();
    if (BOARD_IS_XIP())
    {
        BOARD_BootClockLPR();
        CLOCK_EnableClock(kCLOCK_Otp);
        CLOCK_EnableClock(kCLOCK_Els);
        CLOCK_EnableClock(kCLOCK_ElsApb);
        RESET_PeripheralReset(kOTP_RST_SHIFT_RSTn);
        RESET_PeripheralReset(kELS_APB_RST_SHIFT_RSTn);
    }
    else
    {
        BOARD_InitBootClocks();
    }
    BOARD_InitDebugConsole();
    /* Reset GMDA */
    RESET_PeripheralReset(kGDMA_RST_SHIFT_RSTn);
    /* Keep CAU sleep clock here. */
    /* CPU1 uses Internal clock when in low power mode. */
    POWER_ConfigCauInSleep(false);
    BOARD_InitSleepPinConfig();
#endif

    status = OSA_TaskCreate((osa_task_handle_t)main_task_Handle, OSA_TASK(main_task), NULL);

    OSA_Start();

    return 0;
}
