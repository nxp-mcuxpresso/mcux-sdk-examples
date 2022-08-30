/*
 * Copyright (c) 2013 - 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* FreeRTOS kernel includes. */
#include <stdint.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
/* Application includes */
#include <stdio.h>
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"

#include "iperf_api.h"

#include "fsl_phyksz8081.h"
#include "fsl_enet_mdio.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* @TEST_ANCHOR */

/* Address of PHY interface. */
#define EXAMPLE_PHY_ADDRESS BOARD_ENET0_PHY_ADDRESS

/* MDIO operations. */
#define EXAMPLE_MDIO_OPS enet_ops

/* PHY operations. */
#define EXAMPLE_PHY_OPS phyksz8081_ops

/* ENET clock frequency. */
#define EXAMPLE_CLOCK_FREQ CLOCK_GetFreq(kCLOCK_IpgClk)

#define RECV_TIMEOUT_MS 100

#define TASK_MAIN_STACK_SIZE 800

#ifdef IPERF3_WIFI
/*
 * Bandwidth limit of the iperf3 server for the UDP receive test.
 * "0" means there is no limit and server will transmit testing UDP packets at its maximum speed.
 * Other number means server will transmit at given bits per second speed.
 */
#define UDP_RX_BANDWIDTH "81920"
#define TASK_MAIN_PRIO   (configMAX_PRIORITIES - 2)

#endif /* IPERF3_WIFI */

#ifdef IPERF3_ENET

#include "enet_ethernetif.h"
#include "lwip/netifapi.h"
#include "board.h"
#include "lwip/tcpip.h"
#include "fsl_phy.h"
#include "lwip/prot/dhcp.h"

#define UDP_RX_BANDWIDTH "20000000"

#define TASK_MAIN_PRIO (configMAX_PRIORITIES - 2)

#if TASK_MAIN_PRIO <= TCPIP_THREAD_PRIO
#error "TASK_MAIN_PRIO <= TCPIP_THREAD_PRIO"
#endif

#if TASK_MAIN_PRIO > configTIMER_TASK_PRIORITY
#error "TASK_MAIN_PRIO > configTIMER_TASK_PRIORITY"
#endif

/* MAC address configuration. */
#define configMAC_ADDR                     \
    {                                      \
        0x02, 0x12, 0x13, 0x10, 0x15, 0x11 \
    }

/* IP address configuration. */
#define configIP_ADDR0 192
#define configIP_ADDR1 168
#define configIP_ADDR2 0
#define configIP_ADDR3 102

/* Netmask configuration. */
#define configNET_MASK0 255
#define configNET_MASK1 255
#define configNET_MASK2 255
#define configNET_MASK3 0

/* Gateway address configuration. */
#define configGW_ADDR0 192
#define configGW_ADDR1 168
#define configGW_ADDR2 0
#define configGW_ADDR3 100

#endif /* IPERF3_ENET */

/*******************************************************************************
 * Variables
 ******************************************************************************/

static TaskHandle_t task_main_task_handler;

static char *json_buf;
static TimerHandle_t iperf_timer = NULL;
static char json_results[OUTPUT_STR_MAX];

static char json_format[OUTPUT_STR_MAX] =
    "{"
    "\"cpu_util_total\":0,"
    "\"cpu_util_user\":0,"
    "\"cpu_util_system\":0,"
    "\"sender_has_retransmits\":-1,"
    "\"congestion_used\":\"N/A\","
    "\"streams\": [{\"id\":1,"
    "\"start_time\": 0,"
    "\"bytes\": %d,"
    "\"retransmits\": 0,"
    "\"jitter\": 0,"
    "\"errors\": 0,"
    "\"packets\": %d,"
    "\"end_time\": 10}]}";

static int turn                  = 0;
static int juck_counter          = 0;
static int junk                  = 0;
static TickType_t start_ticks    = 0;
static TickType_t expected_ticks = 0;
static TickType_t current_ticks  = 0;

static struct iperf_ctx iperf_test;

static char *json_req[] = {
    "{\"tcp\":1, \"time\": 10}",
    "{\"tcp\":1, \"reverse\":1, \"time\": 10}",
    "{\"udp\":\"true\", \"len\": 1024, \"time\": 10}",
    "{\"udp\":\"true\", \"reverse\":1, \"len\": 1024, "
    "\"time\": 10, \"fqrate\": 512, \"bandwidth\": " UDP_RX_BANDWIDTH "}",
};

#ifdef IPERF3_ENET

static mdio_handle_t mdioHandle = {.ops = &EXAMPLE_MDIO_OPS};
static phy_handle_t phyHandle   = {.phyAddr = EXAMPLE_PHY_ADDRESS, .mdioHandle = &mdioHandle, .ops = &EXAMPLE_PHY_OPS};

#endif /* IPERF3_ENET */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_InitModuleClock(void)
{
    const clock_enet_pll_config_t config = {.enableClkOutput = true, .enableClkOutput25M = false, .loopDivider = 1};
    CLOCK_InitEnetPll(&config);
}

void delay(void)
{
    volatile uint32_t i = 0;
    for (i = 0; i < 1000000; ++i)
    {
        __asm("NOP"); /* delay */
    }
}



static void separator()
{
    PRINTF("\r\n===========================================================\r\n");
}

static void iperf_init(struct iperf_ctx *ctx)
{
    iperf_hw_init(ctx);
}

static void menu(struct iperf_ctx *ctx)
{
    char mode_switch;
    int choosing_mode = 1;
    PRINTF(
        "Menu:\r\n"
        "Press \'s\' to start client Tx mode\r\n"
        "Press \'r\' to start client Rx mode\r\n"
        "Press \'S\' to start client Tx UDP mode\r\n"
        "Press \'R\' to start client Rx UDP mode\r\n");
    while (choosing_mode)
    {
        mode_switch = GETCHAR();
        switch (mode_switch)
        {
            case 's':
                choosing_mode = 0;
                ctx->mode     = IPERF_CLIENT_TX;
                PRINTF("\r\nTx mode!\r\n");
                break;

            case 'r':
                choosing_mode = 0;
                ctx->mode     = IPERF_CLIENT_RX;
                PRINTF("\r\nRx mode!\r\n");
                break;

            case 'S':
                choosing_mode = 0;
                ctx->mode     = IPERF_CLIENT_TX_UDP;
                PRINTF("\r\nTx UDP mode!\r\n");
                break;

            case 'R':
                choosing_mode = 0;
                ctx->mode     = IPERF_CLIENT_RX_UDP;
                PRINTF("\r\nRx UDP mode!\r\n");
                break;

            default:
                PRINTF("\'%c\' is not valid mode\r\n", mode_switch);
                break;
        }
    }
}

static uint32_t iperf_bswap32(uint32_t source)
{
    uint32_t tmp = 0;
    tmp |= ((source & 0xFF) << 24);
    tmp |= ((source & 0xFF00) << 8);
    tmp |= ((source & 0xFF0000) >> 8);
    tmp |= ((source & 0xFF000000) >> 24);
    return tmp;
}

static int iperf_send_state(struct iperf_ctx *ctx, uint8_t state)
{
    uint32_t transmitted = 0;
    ctx->ctrl_buf[0]     = state;
    transmitted          = iperf_send(ctx->ctrl_sock, ctx->ctrl_buf, 1, 0);
    assert(1 == transmitted);
    return 1 == transmitted ? 0 : -1;
}

static void iperf_set_state(struct iperf_ctx *ctx, uint8_t state)
{
    ctx->state = state;
}

int iperf_send_and_set_state(struct iperf_ctx *ctx, uint8_t state)
{
    if (0 == iperf_send_state(ctx, state))
    {
        iperf_set_state(ctx, state);
        return 0;
    }
    return -1;
}

static int iperf_recv_state(struct iperf_ctx *ctx, uint8_t *state, bool dont_block)
{
    int received;

    if (dont_block)
    {
        received = iperf_recv_noblock(ctx->ctrl_sock, ctx->recv_buf, 1, 0);
    }
    else
    {
        received = iperf_recv_timeout(ctx->ctrl_sock, ctx->recv_buf, 1, 0, RECV_TIMEOUT_MS);
    }

    if (received == 1)
    {
        *state = ctx->recv_buf[0];
        return 0;
    }
    return -1;
}

static void iperf_recv_and_set_state(struct iperf_ctx *ctx)
{
    uint8_t state   = 0;
    bool dont_block = (ctx->state == TEST_RUNNING);

    if (0 == iperf_recv_state(ctx, &state, dont_block))
    {
        if ((state != TEST_START) || (ctx->state != TEST_RUNNING))
        {
            ctx->state = state;
        }
    }
}

static void disposal_of_unprofitable_data_tcp(int dataSocket, struct iperf_ctx *ctx)
{
    juck_counter = junk = 0;
    start_ticks         = xTaskGetTickCount();
    expected_ticks      = start_ticks + pdMS_TO_TICKS(1000);
    do
    {
        junk          = iperf_recv_timeout(dataSocket, ctx->recv_buf, IPERF_BUFFER_MAX, 0, RECV_TIMEOUT_MS);
        juck_counter  = junk > 0 ? 0 : juck_counter + 1;
        current_ticks = xTaskGetTickCount();
    } while (expected_ticks > current_ticks);
}

static void disposal_of_unprofitable_data_udp(int dataSocket, struct iperf_ctx *ctx)
{
    juck_counter = junk = 0;
    start_ticks         = xTaskGetTickCount();
    expected_ticks      = start_ticks + pdMS_TO_TICKS(3000);
    do
    {
        junk          = iperf_recv_from_timeout(dataSocket, ctx->recv_buf, IPERF_BUFFER_MAX, 0, RECV_TIMEOUT_MS);
        juck_counter  = junk > 0 ? 0 : juck_counter + 1;
        current_ticks = xTaskGetTickCount();
    } while (expected_ticks > current_ticks);
}

static void iperfc_timer_start(struct iperf_ctx *ctx)
{
    BaseType_t result = 0;
    result            = xTimerStart(iperf_timer, 0);
    if (result != pdPASS)
    {
        PRINTF("Failed to start timer!\r\n");
        __BKPT(0);
    }
}

static int iperfc_create_streams(struct iperf_ctx *ctx)
{
    int result = 0;
    PRINTF("Creating streams\r\n");
    if (ctx->mode == IPERF_CLIENT_TX || ctx->mode == IPERF_CLIENT_TX_UDP)
    {
        for (int i = 0; i < IPERF_BUFFER_MAX; i++)
        {
            ctx->send_buf[i] = 0;
        }
    }
    ctx->data_sock = iperf_socket(ctx->socket_type);
    if (ctx->socket_type == TCP)
    {
        result = iperf_connect(ctx->data_sock, ctx->addr, ctx->addr_len);
        if (result != 0)
        {
            PRINTF("Failed to connect socket!\r\n");
            __BKPT(0);
        }
        strcpy(ctx->ctrl_buf, MAGIC_COOKIE);
        result = iperf_send(ctx->data_sock, ctx->ctrl_buf, 37, 0);
        if (result != strlen(MAGIC_COOKIE) + 1)
        {
            PRINTF("Error while sending cookie to the server!\r\n");
            __BKPT(0);
        }
        ctx->state = IPERF_NOP;
    }
    else
    {
        /* Send "hello" to server - tells server to start counting */
        strcpy(ctx->send_buf, "hello");
        result = iperf_send_to(ctx->data_sock, ctx->send_buf, IPERF_BUFFER_MAX, 0, ctx->addr, ctx->addr_len);
        if (ctx->mode != IPERF_CLIENT_TX_UDP)
        {
            /* Wait for "hello" from server */
            do
            {
                result = iperf_recv_from_timeout(ctx->data_sock, ctx->recv_buf, IPERF_BUFFER_MAX, 0, RECV_TIMEOUT_MS);
            } while (result != IPERF_BUFFER_MAX);

            /* Force running state, because server sends data immediately */
            ctx->skip_state_start = 1;
            iperfc_timer_start(ctx);
            ctx->state = TEST_RUNNING;
        }
        else
        {
            ctx->state = IPERF_NOP;
        }
    }
    return 0;
}

static int iperfc_test_start(struct iperf_ctx *ctx)
{
    if (ctx->skip_state_start)
    {
        return 0;
    }

    PRINTF("Starting test\r\n");
    iperfc_timer_start(ctx);

    if (ctx->mode == IPERF_CLIENT_RX || ctx->mode == IPERF_CLIENT_RX_UDP)
    {
        ctx->state = TEST_RUNNING;
    }
    else
    {
        ctx->state = IPERF_NOP;
    }
    return 0;
}

static int iperf_exchange_params(struct iperf_ctx *ctx)
{
    int result           = 0;
    int32_t exchange_len = 0;
    PRINTF("Exchanging parameters\r\n");
    switch (ctx->mode)
    {
        case IPERF_CLIENT_TX:
            json_buf         = json_req[0];
            ctx->socket_type = TCP;
            break;
        case IPERF_CLIENT_RX:
            json_buf         = json_req[1];
            ctx->socket_type = TCP;
            break;
        case IPERF_CLIENT_TX_UDP:
            json_buf         = json_req[2];
            ctx->socket_type = UDP;
            break;
        case IPERF_CLIENT_RX_UDP:
            json_buf         = json_req[3];
            ctx->socket_type = UDP;
            break;
    }
    strncpy(&ctx->ctrl_buf[4], json_buf, strlen(json_buf));
    *((uint32_t *)&ctx->ctrl_buf[0]) = iperf_bswap32(strlen(json_buf));
    exchange_len                     = strlen(json_buf) + 4;
    result                           = iperf_send(ctx->ctrl_sock, ctx->ctrl_buf, exchange_len, 0);
    if (result != exchange_len)
    {
        PRINTF("Failed to send information!\r\n");
        __BKPT(0);
    }
    ctx->state = IPERF_NOP;
    return 0;
}

static void iperf_running_test(struct iperf_ctx *ctx)
{
    int result = 0;
    if (ctx->mode == IPERF_CLIENT_TX || ctx->mode == IPERF_CLIENT_TX_UDP)
    {
        ctx->send_counter++;
        if (ctx->socket_type == TCP)
        {
            result = iperf_send(ctx->data_sock, ctx->send_buf, IPERF_BUFFER_MAX, 0);
        }
        else
        {
            /* Prepare header */
            /* seconds */
            *((uint32_t *)&ctx->send_buf[0]) = 0;
            /* usec */
            *((uint32_t *)&ctx->send_buf[4]) = 0;
            /* pcount */
            *((uint32_t *)&ctx->send_buf[8]) = iperf_bswap32((uint32_t)(turn + 1));

            // Iperf server counts UDP packets using it`s data...
            for (int i = 12; i < IPERF_BUFFER_MAX; i++)
            {
                ctx->send_buf[i] = turn;
            }

            turn++;
            result = iperf_send_to(ctx->data_sock, ctx->send_buf, IPERF_BUFFER_MAX, 0, ctx->addr, ctx->addr_len);
        }
        if (result > 0)
        {
            ctx->real_send_buff_counter += result;
        }
    }
    else if (ctx->mode == IPERF_CLIENT_RX || ctx->mode == IPERF_CLIENT_RX_UDP)
    {
        if (ctx->socket_type == TCP)
        {
            result = iperf_recv_blocked(ctx->data_sock, ctx->recv_buf, IPERF_BUFFER_MAX, 0);
        }
        else
        {
            result = iperf_recv_from_blocked(ctx->data_sock, ctx->recv_buf, IPERF_BUFFER_MAX, 0);
        }
        if (result > 0)
        {
            ctx->real_recv_buff_counter += result;
        }
    }
}

static void iperf_exchange_results(struct iperf_ctx *ctx)
{
    int result        = 0;
    int tmp_len       = 0;
    int bytes_to_recv = 0;
    if (ctx->data_sock != -1)
    {
        iperf_socket_close(ctx->data_sock);
        ctx->data_sock = -1;
    }

    PRINTF("Exchanging results\r\n");
    if (ctx->mode == IPERF_CLIENT_TX || ctx->mode == IPERF_CLIENT_TX_UDP)
    {
        snprintf(json_results, OUTPUT_STR_MAX, json_format, ctx->real_send_buff_counter,
                 ctx->real_send_buff_counter / IPERF_BUFFER_MAX);
    }
    else
    {
        snprintf(json_results, OUTPUT_STR_MAX, json_format, ctx->real_send_buff_counter,
                 ctx->real_send_buff_counter / IPERF_BUFFER_MAX);
    }

    json_buf = json_results;
    tmp_len  = strlen(json_buf);

    if (tmp_len + 1 + 4 >= IPERF_BUFFER_MAX)
    {
        PRINTF("Buffer overflow!\r\n");
        __BKPT(0);
    }
    strncpy(&ctx->ctrl_buf[4], json_buf, tmp_len);

    *((uint32_t *)&ctx->ctrl_buf[0]) = iperf_bswap32(strlen(json_buf));
    result                           = iperf_send(ctx->ctrl_sock, ctx->ctrl_buf, tmp_len + 4, 0);

    result = iperf_recv_blocked(ctx->ctrl_sock, ctx->recv_buf, 4, 0);
    if (result != 4)
    {
        PRINTF("Wrong number of bytes were received\r\n");
        __BKPT(0);
    }

    bytes_to_recv = *(uint32_t *)(ctx->recv_buf);
    bytes_to_recv = iperf_bswap32(bytes_to_recv);
    result        = iperf_recv_blocked(ctx->ctrl_sock, ctx->recv_buf, bytes_to_recv, 0);
    if (result != bytes_to_recv)
    {
        PRINTF("Wrong number of bytes were received\r\n");
        __BKPT(0);
    }
    separator();
    PRINTF("Server Results\r\n");
    for (int i = 0; i < result; i++)
    {
        PRINTF("%c", ctx->recv_buf[i]);
        if (ctx->recv_buf[i] == ',')
        {
            PRINTF("\r\n");
        }
    }
    ctx->state = IPERF_NOP;
}

static void iperf_end_test(struct iperf_ctx *ctx)
{
    PRINTF("Ending test\r\n");
    iperf_send_state(ctx, TEST_END);
    if (ctx->mode == IPERF_CLIENT_RX || ctx->mode == IPERF_CLIENT_RX_UDP)
    {
        if (ctx->socket_type == TCP)
        {
            disposal_of_unprofitable_data_tcp(ctx->data_sock, ctx);
        }
        else
        {
            disposal_of_unprofitable_data_udp(ctx->data_sock, ctx);
        }
    }
    else
    {
        iperf_socket_close(iperf_test.data_sock);
        iperf_test.data_sock = -1;
    }

    turn                  = 0;
    ctx->state            = IPERF_NOP;
    ctx->skip_state_start = 0;
}

static void print_results(struct iperf_ctx *ctx)
{
    separator();
    if (ctx->mode == IPERF_CLIENT_TX || ctx->mode == IPERF_CLIENT_TX_UDP)
    {
        PRINTF(
            "IPERF finished, supposed to send %d kB (%d KiB)(%d bytes)!\r\n"
            "Transmited %d kB (%d KiB)(%d bytes).\r\n",
            ((ctx->send_counter * IPERF_BUFFER_MAX) / 1000), ((ctx->send_counter * IPERF_BUFFER_MAX) / 1024),
            ctx->send_counter * IPERF_BUFFER_MAX, (ctx->real_send_buff_counter / 1000),
            (ctx->real_send_buff_counter / 1024), ctx->real_send_buff_counter);
    }
    else
    {
        PRINTF("IPERF finished, Received %d kB (%d KiB)!\r\n", (ctx->real_recv_buff_counter / 1000),
               (ctx->real_recv_buff_counter / 1024));
    }
#if 0
    if (ctx->mode == IPERF_CLIENT_TX_UDP)
    {
        PRINTF(
            "If iperf shows OUT-OF-ORDER, check wireshark to make sure that all"
            " packets are in order.\r\n");
    }
#endif
    separator();
}

static void iperf_switch_state(struct iperf_ctx *ctx)
{
    int xStatus = 0;
    switch (ctx->state)
    {
        case PARAM_EXCHANGE: // 9
            iperf_exchange_params(ctx);
            break;

        case CREATE_STREAMS: // 10 0xA
            iperfc_create_streams(ctx);
            break;

        case TEST_START: // 1
            iperfc_test_start(ctx);
            break;

        case TEST_RUNNING: // 2
            iperf_running_test(ctx);
            break;

        case EXCHANGE_RESULTS: // 13 0xD
            iperf_exchange_results(ctx);
            break;

        case DISPLAY_RESULTS: // 14 0xE
            ctx->ctrl_buf[0] = IPERF_DONE;
            xStatus          = iperf_send(ctx->ctrl_sock, ctx->ctrl_buf, CTRL_IPERF_MSG_LEN, 0);
            if (xStatus <= 0)
            {
                PRINTF("Failed to send control socket!\r\n");
                __BKPT(0);
            }
            ctx->iperf_done = 1;
            break;

        case TEST_END: // 4
            iperf_end_test(ctx);
            break;

        case IPERF_START: // 15
            PRINTF("IPERF START..\r\n");
            ctx->state = IPERF_NOP;
            break;

        case IPERF_NOP: //-42
            break;

        case SERVER_TERMINATE:
            PRINTF("SERVER TERMINATED!!!\r\n");
            ctx->iperf_done = 2;
            break;

        case ACCESS_DENIED:
            PRINTF("ACCESS_DENIED!!!\r\n");
            ctx->iperf_done = 2;
            break;

        case SERVER_ERROR:
            PRINTF("ACCESS_DENIED!!!\r\n");
            ctx->iperf_done = 2;
            break;

        default:
            PRINTF("Unexpected state, shutting down!!\r\n");
            ctx->ctrl_buf[0] = CLIENT_TERMINATE;
            xStatus          = iperf_send(ctx->ctrl_sock, ctx->ctrl_buf, CTRL_IPERF_MSG_LEN, 0);
            if (xStatus == -1)
            {
                PRINTF("Failed to send status!\r\n");
                __BKPT(0);
            }
            ctx->iperf_done = 2;
            break;
    }
}

static void get_server_ip(struct iperf_ctx *ctx)
{
    uint32_t safe_scanf_ip[4] = {0};
    int result                = 0;
    do
    {
        PRINTF("Enter IP address of a server in format '192.168.1.2'\r\n");
        result = SCANF("%d..%d..%d..%d", &safe_scanf_ip[0], &safe_scanf_ip[1], &safe_scanf_ip[2], &safe_scanf_ip[3]);
        if (4 != result)
        {
            PRINTF("\r\nYour IP is not valid, please try again\r\n");
        }
    } while (result != 4);
    /* To avoid stack overflow from user input */
    ctx->server_ip.ip[0] = safe_scanf_ip[0];
    ctx->server_ip.ip[1] = safe_scanf_ip[1];
    ctx->server_ip.ip[2] = safe_scanf_ip[2];
    ctx->server_ip.ip[3] = safe_scanf_ip[3];

    PRINTF("\r\nUsing IP %d.%d.%d.%d\r\n", ctx->server_ip.ip[0], ctx->server_ip.ip[1], ctx->server_ip.ip[2],
           ctx->server_ip.ip[3]);
}

static void iperf_run(struct iperf_ctx *ctx)
{
    int xStatus                 = -1;
    char c                      = '\0';
    ctx->iperf_done             = 0;
    ctx->send_counter           = 0;
    ctx->real_send_buff_counter = 0;
    ctx->real_recv_buff_counter = 0;

    menu(&iperf_test);

    do
    {
        ctx->socket_type = TCP;
        ctx->ctrl_sock   = iperf_socket(ctx->socket_type);
        PRINTF("Connecting to the server...\r\n");
        xStatus = iperf_connect(ctx->ctrl_sock, ctx->addr, ctx->addr_len);
        if (xStatus == -1)
        {
            PRINTF("Failed!\r\n");
            iperf_socket_close(ctx->ctrl_sock);
        }
    } while (xStatus != 0);

    /* Cookie */
    PRINTF("Sending cookie!!...\r\n");
    strcpy(ctx->ctrl_buf, MAGIC_COOKIE);
    xStatus    = iperf_send(ctx->ctrl_sock, ctx->ctrl_buf, strlen(MAGIC_COOKIE) + 1, 0);
    ctx->state = IPERF_START;

    do
    {
        /* Receive context 'state' from server */
        iperf_recv_and_set_state(ctx);
        iperf_switch_state(ctx);
    } while (!ctx->iperf_done);

    if (ctx->iperf_done == 1)
    {
        print_results(ctx);
    }
    else
    {
        PRINTF("Test ended");
    }

    iperf_socket_close(ctx->ctrl_sock);
    memset(ctx->recv_buf, 0, IPERF_BUFFER_MAX);

    do
    {
        PRINTF("\r\nPRESS \"F\" to restart...\r\n");
        c = GETCHAR();
    } while (c != 'F');
    PRINTF("\r\n");
}

#ifdef IPERF3_WIFI

void iperf_startup(struct iperf_ctx *ctx)
{
    /* Run the iperf */
    while (1)
    {
        iperf_run(ctx);
    }
}

#elif defined(IPERF3_ENET)

void setup_default_ip(struct netif *fsl_netif0,
                      ip4_addr_t *fsl_netif0_ipaddr,
                      ip4_addr_t *fsl_netif0_netmask,
                      ip4_addr_t *fsl_netif0_gw)
{
    err_t result = 0;

    IP4_ADDR(fsl_netif0_ipaddr, configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3);
    IP4_ADDR(fsl_netif0_netmask, configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3);
    IP4_ADDR(fsl_netif0_gw, configGW_ADDR0, configGW_ADDR1, configGW_ADDR2, configGW_ADDR3);

    result = netifapi_netif_set_addr(fsl_netif0, fsl_netif0_ipaddr, fsl_netif0_netmask, fsl_netif0_gw);
    if (result != 0)
    {
        PRINTF("Failed to set default IP!\r\n");
        __BKPT(0);
    }
}

void enet_init(void)
{
    err_t result = 0;
    static struct netif fsl_netif0;
    struct dhcp *pdhcp = NULL;
    ip4_addr_t fsl_netif0_ipaddr, fsl_netif0_netmask, fsl_netif0_gw;
    ethernetif_config_t fsl_enet_config0 = {
        .phyHandle  = &phyHandle,
        .macAddress = configMAC_ADDR,
    };

    mdioHandle.resource.csrClock_Hz = EXAMPLE_CLOCK_FREQ;

    tcpip_init(NULL, NULL);

    result = netifapi_netif_add(&fsl_netif0, &fsl_netif0_ipaddr, &fsl_netif0_netmask, &fsl_netif0_gw, &fsl_enet_config0,
                                ethernetif0_init, tcpip_input);
    if (result != 0)
    {
        PRINTF("Failed to add netif!\r\n");
        __BKPT(0);
    }

    result = netifapi_netif_set_default(&fsl_netif0);
    assert(0 == result);

    result = netifapi_netif_set_up(&fsl_netif0);
    assert(0 == result);

    netifapi_dhcp_start(&fsl_netif0);

    pdhcp = (struct dhcp *)netif_get_client_data(&fsl_netif0, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);
    for (int i = 20; i > 0 && pdhcp->state != DHCP_STATE_BOUND; i--)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    if (pdhcp->state != DHCP_STATE_BOUND)
    {
        netifapi_dhcp_stop(&fsl_netif0);
        PRINTF("DHCP has failed.. setting IP address to: %u.%u.%u.%u\r\n", configIP_ADDR0, configIP_ADDR1,
               configIP_ADDR2, configIP_ADDR3);
        setup_default_ip(&fsl_netif0, &fsl_netif0_ipaddr, &fsl_netif0_netmask, &fsl_netif0_gw);
    }
}

void iperf_startup(struct iperf_ctx *ctx)
{
    enet_init();
    while (1)
    {
        iperf_run(ctx);
    }
}

#endif /* IPERF3_WIFI */

void task_main(void *arg)
{
    struct iperf_ctx *ctx = (struct iperf_ctx *)arg;

    get_server_ip(ctx);

    iperf_init(ctx);
    iperf_startup(ctx);

    while (1)
        ;
}

void callback_time(TimerHandle_t xTimer)
{
    iperf_test.state = TEST_END;
}

/*!
 * @brief Main function
 */
int main(void)
{
    BaseType_t result = 0;
    (void)result;
    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_InitModuleClock();

    IOMUXC_EnableMode(IOMUXC_GPR, kIOMUXC_GPR_ENET1TxClkOutputDir, true);

    GPIO_PinInit(GPIO1, 9, &gpio_config);
    GPIO_PinInit(GPIO1, 10, &gpio_config);
    /* pull up the ENET_INT before RESET. */
    GPIO_WritePinOutput(GPIO1, 10, 1);
    GPIO_WritePinOutput(GPIO1, 9, 0);
    delay();
    GPIO_WritePinOutput(GPIO1, 9, 1);

    iperf_timer = xTimerCreate("iperf_timer", configTICK_RATE_HZ * 12, pdFALSE, (void *)0, callback_time);
    if (iperf_timer == NULL)
    {
        PRINTF("Failed to create a timer!\r\n");
        __BKPT(0);
    }

    result = xTaskCreate(task_main, "main", TASK_MAIN_STACK_SIZE, &iperf_test, TASK_MAIN_PRIO, &task_main_task_handler);
    if (result != pdPASS)
    {
        PRINTF("Failed to create a main task!\r\n");
        __BKPT(0);
    }
    vTaskStartScheduler();
    for (;;)
        ;
}
