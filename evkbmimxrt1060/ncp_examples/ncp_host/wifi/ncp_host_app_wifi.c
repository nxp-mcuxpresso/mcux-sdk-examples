/** @file ncp_host_app_wifi.c
 *
 *  @brief  This file provides interface for receiving tlv responses and processing tlv responses.
 *
 *  Copyright 2008-2023 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "ncp_host_command_wifi.h"

uint32_t mcu_last_resp_rcvd;

int ping_sock_handle = -1;

void ping_sock_task(void *param);
void ncp_iperf_tx_task(void *param);
void ncp_iperf_rx_task(void *param);

#define PING_SOCK_TASK_PRIO   2
static OSA_TASK_HANDLE_DEFINE(ping_sock_thread);
static OSA_TASK_DEFINE(ping_sock_task, PING_SOCK_TASK_PRIO, 1, 1024, 0);

#define NCP_IPERF_TX_TASK_PRIO   1
static OSA_TASK_HANDLE_DEFINE(ncp_iperf_tx_thread);
static OSA_TASK_DEFINE(ncp_iperf_tx_task, NCP_IPERF_TX_TASK_PRIO, 1, 1024, 0);

#define NCP_IPERF_RX_TASK_PRIO   1
static OSA_TASK_HANDLE_DEFINE(ncp_iperf_rx_thread);
static OSA_TASK_DEFINE(ncp_iperf_rx_task, NCP_IPERF_RX_TASK_PRIO, 1, 1024, 0);

OSA_EVENT_HANDLE_DEFINE(ping_events);
OSA_EVENT_HANDLE_DEFINE(iperf_tx_events);
OSA_EVENT_HANDLE_DEFINE(iperf_rx_events);

/* ping variables */
extern ping_msg_t ping_msg;
ping_res_t ping_res;

#define WIFI_NCP_STACK_SIZE   4096
#define WIFI_NCP_TASK_PRIO   1
static OSA_TASK_HANDLE_DEFINE(wifi_ncp_task_handle);
void wifi_ncp_task(void *pvParameters);
static OSA_TASK_DEFINE(wifi_ncp_task, WIFI_NCP_TASK_PRIO, 1, WIFI_NCP_STACK_SIZE, 0);

/*WIFI NCP COMMAND TASK*/
#define WIFI_NCP_COMMAND_QUEUE_NUM 16
static osa_msgq_handle_t wifi_ncp_command_queue; /* ncp adapter TX msgq */
OSA_MSGQ_HANDLE_DEFINE(wifi_ncp_command_queue_buff, WIFI_NCP_COMMAND_QUEUE_NUM,  sizeof(wifi_ncp_command_t));

typedef struct ncp_cmd_t wifi_ncp_command_t;
extern uint32_t mcu_last_cmd_sent;

extern int wlan_process_ncp_event(uint8_t *res);
extern int wlan_process_response(uint8_t *res);
extern int ncp_host_send_tlv_command();
extern int mcu_get_command_resp_sem();
extern int mcu_put_command_resp_sem();
extern int mcu_get_command_lock();
extern int mcu_put_command_lock();

/* Display the final result of ping */
static void display_ping_result(int total, int recvd)
{
    int dropped = total - recvd;
    (void)PRINTF("\r\n--- ping statistics ---\r\n");
    (void)PRINTF("%d packets transmitted, %d received,", total, recvd);
    if (dropped != 0)
        (void)PRINTF(" +%d errors,", dropped);
    (void)PRINTF(" %d%% packet loss\r\n", (dropped * 100) / total);
}

/** Prepare a echo ICMP request */
static void ping_prepare_echo(struct icmp_echo_hdr *iecho, uint16_t len, uint16_t seq_no)
{
    size_t i;
    size_t data_len = len - sizeof(struct icmp_echo_hdr);

    iecho->type   = ICMP_ECHO;
    iecho->code   = 0;
    iecho->chksum = 0;
    iecho->id     = PING_ID;
    iecho->seqno  = PP_HTONS(seq_no);

    /* fill the additional data buffer with some data */
    for (i = 0; i < data_len; i++)
    {
        ((char *)iecho)[sizeof(struct icmp_echo_hdr) + i] = (char)i;
    }

    iecho->chksum = inet_chksum(iecho, len);
}

/* Display the statistics of the current iteration of ping */
static void display_ping_stats(int status, uint32_t size, const char *ip_str, uint16_t seqno, int ttl, uint32_t time)
{
    if (status == WM_SUCCESS)
    {
        (void)PRINTF("%u bytes from %s: icmp_req=%u ttl=%u time=%u ms\r\n", size, ip_str, seqno, ttl, time);
    }
    else
    {
        (void)PRINTF("icmp_seq=%u Destination Host Unreachable\r\n", seqno);
    }
}
/* Send an ICMP echo request by NCP_CMD_WLAN_SOCKET_SENDTO command and get ICMP echo reply by
 * NCP_CMD_WLAN_SOCKET_RECVFROM command. Print ping statistics in NCP_CMD_WLAN_SOCKET_RECVFROM
 * command response, and print ping result in ping_sock_task.
 */
void ping_sock_task(void *pvParameters)
{
    struct icmp_echo_hdr *iecho;
    int retry;

    while (1)
    {
        ping_res.recvd  = 0;
        ping_res.seq_no = -1;

        /* demo ping task wait for user input ping command from console */
        (void)ping_wait_event(PING_EVENTS_START);

        (void)PRINTF("PING %s (%s) %u(%u) bytes of data\r\n", ping_msg.ip_addr, ping_msg.ip_addr, ping_msg.size,
                     ping_msg.size + sizeof(struct ip_hdr) + sizeof(struct icmp_echo_hdr));

        int i = 1;
        /* Ping size is: size of ICMP header + size of payload */
        uint16_t ping_size = sizeof(struct icmp_echo_hdr) + ping_msg.size;

        iecho = (struct icmp_echo_hdr *)OSA_MemoryAllocate(ping_size);
        if (!iecho)
        {
            ncp_e("failed to allocate memory for ping packet!");
            continue;
        }

        /*Wait for command response semaphore.*/
        mcu_get_command_resp_sem();
        mcu_get_command_lock();
        /* Open socket before ping */
        MCU_NCPCmd_DS_COMMAND *ping_sock_open_command = ncp_host_get_cmd_buffer_wifi();
        ping_sock_open_command->header.cmd            = NCP_CMD_WLAN_SOCKET_OPEN;
        ping_sock_open_command->header.size           = NCP_CMD_HEADER_LEN;
        ping_sock_open_command->header.result         = NCP_CMD_RESULT_OK;

        NCP_CMD_SOCKET_OPEN_CFG *ping_sock_open_tlv =
            (NCP_CMD_SOCKET_OPEN_CFG *)&ping_sock_open_command->params.wlan_socket_open;
        strcpy(ping_sock_open_tlv->socket_type, "raw");
        strcpy(ping_sock_open_tlv->domain_type, "ipv4");
        strcpy(ping_sock_open_tlv->protocol, "icmp");
        ping_sock_open_command->header.size += sizeof(NCP_CMD_SOCKET_OPEN_CFG);
        /* Send ping TLV command */
        ncp_host_send_tlv_command();

        while (i <= ping_msg.count)
        {
            ping_res.echo_resp = -WM_FAIL;
            retry              = 10;

            /*Wait for command response semaphore.*/
            mcu_get_command_resp_sem();

            /* Prepare ping command */
            ping_prepare_echo(iecho, (uint16_t)ping_size, i);

            mcu_get_command_lock();
            MCU_NCPCmd_DS_COMMAND *ping_sock_command = ncp_host_get_cmd_buffer_wifi();
            ping_sock_command->header.cmd            = NCP_CMD_WLAN_SOCKET_SENDTO;
            ping_sock_command->header.size           = NCP_CMD_HEADER_LEN;
            ping_sock_command->header.result         = NCP_CMD_RESULT_OK;

            NCP_CMD_SOCKET_SENDTO_CFG *ping_sock_tlv =
                (NCP_CMD_SOCKET_SENDTO_CFG *)&ping_sock_command->params.wlan_socket_sendto;
            ping_sock_tlv->handle = ping_sock_handle;
            ping_sock_tlv->port   = ping_msg.port;
            memcpy(ping_sock_tlv->ip_addr, ping_msg.ip_addr, strlen(ping_msg.ip_addr) + 1);
            memcpy(ping_sock_tlv->send_data, iecho, ping_size);
            ping_sock_tlv->size = ping_size;

            /*cmd size*/
            ping_sock_command->header.size += sizeof(NCP_CMD_SOCKET_SENDTO_CFG) - sizeof(char);
            ping_sock_command->header.size += ping_size;

            /* sequence number */
            ping_res.seq_no = i;

            /* Send ping TLV command */
            ncp_host_send_tlv_command();
            /* Get the current ticks as the start time */
            ping_res.time = OSA_TimeGetMsec();

            /* wait for NCP_CMD_WLAN_SOCKET_SENDTO command response */
            (void)ping_wait_event(PING_EVENTS_SENDTO_RESP);

            /* Function raw_input may put multiple pieces of data in conn->recvmbox,
             * waiting to select the data we want */
            while (ping_res.echo_resp != WM_SUCCESS && retry)
            {
                /*Wait for command response semaphore.*/
                mcu_get_command_resp_sem();

                mcu_get_command_lock();
                /* Prepare get-ping-result command */
                MCU_NCPCmd_DS_COMMAND *ping_res_command = ncp_host_get_cmd_buffer_wifi();
                ping_res_command->header.cmd            = NCP_CMD_WLAN_SOCKET_RECVFROM;
                ping_res_command->header.size           = NCP_CMD_HEADER_LEN;
                ping_res_command->header.result         = NCP_CMD_RESULT_OK;

                NCP_CMD_SOCKET_RECVFROM_CFG *ping_res_sock_tlv =
                    (NCP_CMD_SOCKET_RECVFROM_CFG *)&ping_res_command->params.wlan_socket_recvfrom;
                ping_res_sock_tlv->handle    = ping_sock_handle;
                ping_res_sock_tlv->recv_size = ping_msg.size + IP_HEADER_LEN;
                ping_res_sock_tlv->timeout   = PING_RECVFROM_TIMEOUT;

                /*cmd size*/
                ping_res_command->header.size += sizeof(NCP_CMD_SOCKET_RECVFROM_CFG);

                /* Send get-ping-result TLV command */
                ncp_host_send_tlv_command();

                /* wait for NCP_CMD_WLAN_SOCKET_RECVFROM command response */
                (void)ping_wait_event(PING_EVENTS_RECVFROM_RESP);

                retry--;
            }

            /* Calculate the round trip time */
            ping_res.time = OSA_TimeGetMsec() - ping_res.time;

            display_ping_stats(ping_res.echo_resp, ping_res.size, ping_res.ip_addr, ping_res.seq_no, ping_res.ttl,
                               ping_res.time);

            OSA_TimeDelay(1000);

            i++;
        }
        OSA_MemoryFree((void *)iecho);
        display_ping_result((int)ping_msg.count, ping_res.recvd);
        /*Wait for command response semaphore.*/
        mcu_get_command_resp_sem();
        mcu_get_command_lock();
        MCU_NCPCmd_DS_COMMAND *ping_socket_close_command = ncp_host_get_cmd_buffer_wifi();
        ping_socket_close_command->header.cmd            = NCP_CMD_WLAN_SOCKET_CLOSE;
        ping_socket_close_command->header.size           = NCP_CMD_HEADER_LEN;
        ping_socket_close_command->header.result         = NCP_CMD_RESULT_OK;

        NCP_CMD_SOCKET_CLOSE_CFG *ping_socket_close_tlv =
            (NCP_CMD_SOCKET_CLOSE_CFG *)&ping_socket_close_command->params.wlan_socket_close;
        ping_socket_close_tlv->handle = ping_sock_handle;
        /*cmd size*/
        ping_socket_close_command->header.size += sizeof(NCP_CMD_SOCKET_CLOSE_CFG);
        /* Send socket close command */
        ncp_host_send_tlv_command();
        ping_sock_handle = -1;
    }
}

/*iperf command tx and rx */
extern iperf_msg_t iperf_msg;
#define NCP_IPERF_PER_PKG_SIZE 1448
#define IPERF_RECV_TIMEOUT     3000
/** A const buffer to send from: we want to measure sending, not copying! */
static const char lwiperf_txbuf_const[NCP_IPERF_PER_PKG_SIZE] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2',
    '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1',
    '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2',
    '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1',
    '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2',
    '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1',
    '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2',
    '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1',
    '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2',
    '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1',
    '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9',
};

char lwiperf_end_token[NCP_IPERF_END_TOKEN_SIZE] = {'N', 'C', 'P', 'I', 'P', 'E', 'R', 'P', 'E', 'N', 'D'};

int iperf_send_setting(void)
{
    if (iperf_msg.iperf_set.iperf_type == NCP_IPERF_TCP_TX || iperf_msg.iperf_set.iperf_type == NCP_IPERF_TCP_RX)
    {
        MCU_NCPCmd_DS_COMMAND *iperf_command = ncp_host_get_cmd_buffer_wifi();
        iperf_command->header.cmd            = NCP_CMD_WLAN_SOCKET_SEND;
        iperf_command->header.size           = NCP_CMD_HEADER_LEN;
        iperf_command->header.result         = NCP_CMD_RESULT_OK;

        NCP_CMD_SOCKET_SEND_CFG *ncp_iperf_tlv = (NCP_CMD_SOCKET_SEND_CFG *)&iperf_command->params.wlan_socket_send;
        ncp_iperf_tlv->handle                  = iperf_msg.handle;
        ncp_iperf_tlv->size                    = sizeof(iperf_set_t);
        memcpy(ncp_iperf_tlv->send_data, (char *)(&iperf_msg.iperf_set), sizeof(iperf_set_t));

        /*cmd size*/
        iperf_command->header.size += sizeof(NCP_CMD_SOCKET_SEND_CFG);
        iperf_command->header.size += sizeof(iperf_set_t);
        (void)memcpy((char *)lwiperf_txbuf_const, (char *)(&iperf_msg.iperf_set), sizeof(iperf_set_t));
    }
    else if (iperf_msg.iperf_set.iperf_type == NCP_IPERF_UDP_TX || iperf_msg.iperf_set.iperf_type == NCP_IPERF_UDP_RX)
    {
        MCU_NCPCmd_DS_COMMAND *iperf_command = ncp_host_get_cmd_buffer_wifi();
        iperf_command->header.cmd            = NCP_CMD_WLAN_SOCKET_SENDTO;
        iperf_command->header.size           = NCP_CMD_HEADER_LEN;
        iperf_command->header.result         = NCP_CMD_RESULT_OK;

        NCP_CMD_SOCKET_SENDTO_CFG *ncp_iperf_tlv =
            (NCP_CMD_SOCKET_SENDTO_CFG *)&iperf_command->params.wlan_socket_sendto;
        ncp_iperf_tlv->handle = iperf_msg.handle;
        ncp_iperf_tlv->size   = sizeof(iperf_set_t);
        ncp_iperf_tlv->port   = iperf_msg.port;
        memcpy(ncp_iperf_tlv->ip_addr, iperf_msg.ip_addr, strlen(iperf_msg.ip_addr) + 1);
        memcpy(ncp_iperf_tlv->send_data, (char *)(&iperf_msg.iperf_set), sizeof(iperf_set_t));
        /*cmd size*/
        iperf_command->header.size += sizeof(NCP_CMD_SOCKET_SENDTO_CFG) - sizeof(char);
        iperf_command->header.size += sizeof(iperf_set_t);
        (void)memcpy((char *)lwiperf_txbuf_const, (char *)(&iperf_msg.iperf_set), sizeof(iperf_set_t));
    }
    else
    {
        ncp_e("iperf type is error");
        return false;
    }
    /* Send iperf TLV command */
    ncp_host_send_tlv_command();
    return true;
}

int iperf_send_end_token(void)
{
    if (iperf_msg.iperf_set.iperf_type == NCP_IPERF_TCP_TX)
    {
        MCU_NCPCmd_DS_COMMAND *iperf_command = ncp_host_get_cmd_buffer_wifi();
        iperf_command->header.cmd            = NCP_CMD_WLAN_SOCKET_SEND;
        iperf_command->header.size           = NCP_CMD_HEADER_LEN;
        iperf_command->header.result         = NCP_CMD_RESULT_OK;

        NCP_CMD_SOCKET_SEND_CFG *ncp_iperf_tlv = (NCP_CMD_SOCKET_SEND_CFG *)&iperf_command->params.wlan_socket_send;
        ncp_iperf_tlv->handle                  = iperf_msg.handle;
        ncp_iperf_tlv->size                    = NCP_IPERF_END_TOKEN_SIZE;
        memcpy(ncp_iperf_tlv->send_data, (char *)&lwiperf_end_token[0], NCP_IPERF_END_TOKEN_SIZE);

        /*cmd size*/
        iperf_command->header.size += sizeof(NCP_CMD_SOCKET_SEND_CFG);
        iperf_command->header.size += NCP_IPERF_END_TOKEN_SIZE;
    }
    else if (iperf_msg.iperf_set.iperf_type == NCP_IPERF_UDP_TX)
    {
        MCU_NCPCmd_DS_COMMAND *iperf_command = ncp_host_get_cmd_buffer_wifi();
        iperf_command->header.cmd            = NCP_CMD_WLAN_SOCKET_SENDTO;
        iperf_command->header.size           = NCP_CMD_HEADER_LEN;
        iperf_command->header.result         = NCP_CMD_RESULT_OK;

        NCP_CMD_SOCKET_SENDTO_CFG *ncp_iperf_tlv =
            (NCP_CMD_SOCKET_SENDTO_CFG *)&iperf_command->params.wlan_socket_sendto;
        ncp_iperf_tlv->handle = iperf_msg.handle;
        ncp_iperf_tlv->size   = NCP_IPERF_END_TOKEN_SIZE;
        ncp_iperf_tlv->port   = iperf_msg.port;
        memcpy(ncp_iperf_tlv->ip_addr, iperf_msg.ip_addr, strlen(iperf_msg.ip_addr) + 1);
        memcpy(ncp_iperf_tlv->send_data, (char *)&lwiperf_end_token[0], NCP_IPERF_END_TOKEN_SIZE);
        /*cmd size*/
        iperf_command->header.size += sizeof(NCP_CMD_SOCKET_SENDTO_CFG) - sizeof(char);
        iperf_command->header.size += NCP_IPERF_END_TOKEN_SIZE;
    }

    /* Send iperf TLV command */
    ncp_host_send_tlv_command();
    return true;
}

unsigned long iperf_timer_start = 0, iperf_timer_end = 0;
void ncp_iperf_report(long long total_size)
{
    unsigned long rate       = 0;
    unsigned long total_time = 0;

    total_time = iperf_timer_end - iperf_timer_start;

    rate = (total_size * 1000) / total_time;
    rate = rate * 8 / 1024;

    (void)PRINTF("iperf rate = %lu kbit/s\r\n", rate);
}

void iperf_tcp_tx(void)
{
    MCU_NCPCmd_DS_COMMAND *iperf_command = ncp_host_get_cmd_buffer_wifi();

    if (iperf_msg.iperf_set.iperf_type == NCP_IPERF_TCP_TX)
    {
        iperf_command->header.cmd      = NCP_CMD_WLAN_SOCKET_SEND;
        iperf_command->header.size     = NCP_CMD_HEADER_LEN;
        iperf_command->header.result   = NCP_CMD_RESULT_OK;

        NCP_CMD_SOCKET_SEND_CFG *ncp_iperf_tlv = (NCP_CMD_SOCKET_SEND_CFG *)&iperf_command->params.wlan_socket_send;
        ncp_iperf_tlv->handle                  = iperf_msg.handle;
        ncp_iperf_tlv->size                    = iperf_msg.per_size;
        memcpy(ncp_iperf_tlv->send_data, lwiperf_txbuf_const, iperf_msg.per_size);

        /*cmd size*/
        iperf_command->header.size += sizeof(NCP_CMD_SOCKET_SEND_CFG);
        iperf_command->header.size += iperf_msg.per_size;
    }
    else if (iperf_msg.iperf_set.iperf_type == NCP_IPERF_UDP_TX)
    {
        iperf_command->header.cmd      = NCP_CMD_WLAN_SOCKET_SENDTO;
        iperf_command->header.size     = NCP_CMD_HEADER_LEN;
        iperf_command->header.result   = NCP_CMD_RESULT_OK;

        NCP_CMD_SOCKET_SENDTO_CFG *ncp_iperf_tlv =
            (NCP_CMD_SOCKET_SENDTO_CFG *)&iperf_command->params.wlan_socket_sendto;
        ncp_iperf_tlv->handle = iperf_msg.handle;
        ncp_iperf_tlv->size   = iperf_msg.per_size;
        ncp_iperf_tlv->port   = iperf_msg.port;
        memcpy(ncp_iperf_tlv->send_data, lwiperf_txbuf_const, iperf_msg.per_size);
        memcpy(ncp_iperf_tlv->ip_addr, iperf_msg.ip_addr, strlen(iperf_msg.ip_addr) + 1);

        /*cmd size*/
        iperf_command->header.size += sizeof(NCP_CMD_SOCKET_SENDTO_CFG) - sizeof(char);
        iperf_command->header.size += iperf_msg.per_size;
    }

    /* Send iperf TLV command */
    ncp_host_send_tlv_command();
}

void iperf_tcp_rx(void)
{
    MCU_NCPCmd_DS_COMMAND *ncp_iperf_command = ncp_host_get_cmd_buffer_wifi();

    if (iperf_msg.iperf_set.iperf_type == NCP_IPERF_TCP_RX)
    {
        ncp_iperf_command->header.cmd      = NCP_CMD_WLAN_SOCKET_RECV;
        ncp_iperf_command->header.size     = NCP_CMD_HEADER_LEN;
        ncp_iperf_command->header.result   = NCP_CMD_RESULT_OK;

        NCP_CMD_SOCKET_RECEIVE_CFG *ncp_iperf_res_sock_tlv =
            (NCP_CMD_SOCKET_RECEIVE_CFG *)&ncp_iperf_command->params.wlan_socket_receive;
        ncp_iperf_res_sock_tlv->handle    = iperf_msg.handle;
        ncp_iperf_res_sock_tlv->recv_size = iperf_msg.per_size;
        ncp_iperf_res_sock_tlv->timeout   = IPERF_TCP_RECV_TIMEOUT;

        /*cmd size*/
        ncp_iperf_command->header.size += sizeof(NCP_CMD_SOCKET_RECEIVE_CFG);
    }
    else if (iperf_msg.iperf_set.iperf_type == NCP_IPERF_UDP_RX)
    {
        ncp_iperf_command->header.cmd      = NCP_CMD_WLAN_SOCKET_RECVFROM;
        ncp_iperf_command->header.size     = NCP_CMD_HEADER_LEN;
        ncp_iperf_command->header.result   = NCP_CMD_RESULT_OK;

        NCP_CMD_SOCKET_RECVFROM_CFG *ncp_iperf_res_sock_tlv =
            (NCP_CMD_SOCKET_RECVFROM_CFG *)&ncp_iperf_command->params.wlan_socket_recvfrom;
        ncp_iperf_res_sock_tlv->handle    = iperf_msg.handle;
        ncp_iperf_res_sock_tlv->recv_size = iperf_msg.per_size;
        ncp_iperf_res_sock_tlv->timeout   = IPERF_UDP_RECV_TIMEOUT;

        /*cmd size*/
        ncp_iperf_command->header.size += sizeof(NCP_CMD_SOCKET_RECVFROM_CFG);
    }

    /* Send iperf TLV command */
    ncp_host_send_tlv_command();
}

void ncp_iperf_tx_task(void *pvParameters)
{
    long long i               = 0;
    unsigned int pkg_num      = 0;
    long long send_total_size = 0;
    long long udp_rate = 0;
    int per_pkt_size      = 1470;
    int pkt_num_per_xms            = 0;
    unsigned int prev_time = 0;
    unsigned int cur_time = 0;
    int delta = 0;
    unsigned int send_interval = 1;

    while (1)
    {
        /* demo ping task wait for user input ping command from console */
        (void)iperf_tx_wait_event(IPERF_TX_START);

        udp_rate = iperf_msg.iperf_set.iperf_udp_rate;

        if (udp_rate <= 120)
            send_interval = 1000;
        else if (udp_rate <= 2*1024)
           send_interval = 60;
        else if (udp_rate <= 10*1024)
           send_interval = 12;
        else if (udp_rate <= 20*1024)
           send_interval = 6;
        else if (udp_rate <= 30 * 1024)
            send_interval = 4;
        else if (udp_rate <= 60 * 1024)
            send_interval = 2;
        else
            send_interval = 1;
        pkt_num_per_xms = ((udp_rate * 1024 / 8) / per_pkt_size / (1000 / send_interval)); /*num pkt per send_interval(ms)*/

        //(void)PRINTF("udp_rate %lld send_interval %d pkt_num_per_xms %d \r\n", udp_rate, send_interval, pkt_num_per_xms);

        send_total_size = iperf_msg.iperf_set.iperf_count * iperf_msg.per_size;

        mcu_get_command_resp_sem();
        mcu_get_command_lock();
        if (false == iperf_send_setting())
            continue;
        OSA_TimeDelay(1000);
        (void)PRINTF("ncp iperf tx start\r\n");
        pkg_num             = 0;
        iperf_msg.status[0] = 0;
        iperf_timer_start   = OSA_TimeGetMsec();
        prev_time = OSA_TimeGetMsec();
        while (pkg_num < iperf_msg.iperf_set.iperf_count)
        {
            /*Wait for command response semaphore.*/
            mcu_get_command_resp_sem();
            if (iperf_msg.status[0] == (char)-WM_FAIL)
            {
                iperf_msg.status[0] = 0;
                mcu_put_command_resp_sem();
                break;
            }
            // else if (!(pkg_num % 100))
            //    (void)PRINTF("ncp tx pkg_num = %d\r\n", pkg_num);

            mcu_get_command_lock();

            iperf_tcp_tx();

            if (iperf_msg.iperf_set.iperf_type == NCP_IPERF_UDP_TX)
            {
                cur_time = OSA_TimeGetMsec();

                if ((i > 0) && (!(i % pkt_num_per_xms)))
                {
                    delta = prev_time + send_interval - cur_time;
                    //PRINTF("prev_time_us = %d, cur_time_us = %d, delta = %d, pkt_num_per1ms = %d, i = %d\r\n",
                    // prev_time, cur_time, delta, pkt_num_per_xms, i);
                    if (delta > 0)
                        OSA_TimeDelay(delta);
                    prev_time += send_interval;
                }
            }

            i++;
            pkg_num++;
        }
        iperf_timer_end = OSA_TimeGetMsec();

        /*Try to make data can recved success by peer*/
        OSA_TimeDelay(1000);
        iperf_send_end_token();

        ncp_iperf_report(send_total_size);
        (void)PRINTF("ncp iperf tx run end\r\n");
    }
}

void ncp_iperf_rx_task(void *pvParameters)
{
    unsigned int pkg_num         = 0;
    unsigned long long recv_size = 0, left_size = 0;

    while (1)
    {
        /* demo ping task wait for user input ping command from console */
        (void)iperf_rx_wait_event(IPERF_RX_START);
        (void)PRINTF("ncp iperf rx start\r\n");
        mcu_get_command_resp_sem();
        mcu_get_command_lock();
        if (false == iperf_send_setting())
        {
            mcu_put_command_resp_sem();
            mcu_put_command_lock();
            continue;
        }
        OSA_TimeDelay(1000);
        pkg_num             = 0;
        iperf_msg.status[1] = 0;
        recv_size           = 0;
        left_size           = iperf_msg.per_size * iperf_msg.iperf_set.iperf_count;
        iperf_timer_start   = OSA_TimeGetMsec();
        while (left_size > 0)
        {
            /*Wait for command response semaphore.*/
            mcu_get_command_resp_sem();
            if (iperf_msg.status[1] == -WM_FAIL)
            {
                iperf_msg.status[1] = 0;
                mcu_put_command_resp_sem();
                break;
            }
            mcu_get_command_lock();
            recv_size += iperf_msg.status[1];
            left_size -= iperf_msg.status[1];
            if (left_size > 0)
            {
                iperf_tcp_rx();
            }
            else
            {
                mcu_put_command_resp_sem();
                mcu_put_command_lock();
            }
            pkg_num++;
        }
        iperf_timer_end = OSA_TimeGetMsec();
        ncp_iperf_report(recv_size);
        (void)PRINTF("ncp iperf rx end\r\n");
    }
}

void ping_wait_event(osa_event_flags_t flagsToWait)
{
    uint32_t Events;
    (void)OSA_EventWait((osa_event_handle_t)ping_events, flagsToWait, 0, osaWaitForever_c, &Events);
}

void ping_set_event(osa_event_flags_t flagsToWait)
{
    (void)OSA_EventSet((osa_event_handle_t)ping_events, flagsToWait);
}

void iperf_tx_wait_event(osa_event_flags_t flagsToWait)
{
    uint32_t Events;
    (void)OSA_EventWait((osa_event_handle_t)iperf_tx_events, flagsToWait, 0, osaWaitForever_c, &Events);
}

void iperf_tx_set_event(osa_event_flags_t flagsToWait)
{
	(void)OSA_EventSet((osa_event_handle_t)iperf_tx_events, flagsToWait);
}

void iperf_rx_wait_event(osa_event_flags_t flagsToWait)
{
    uint32_t Events;
    (void)OSA_EventWait((osa_event_handle_t)iperf_rx_events, flagsToWait, 0, osaWaitForever_c, &Events);
}

void iperf_rx_set_event(osa_event_flags_t flagsToWait)
{
	(void)OSA_EventSet((osa_event_handle_t)iperf_rx_events, flagsToWait);
}

int ncp_wifi_cli_init(void)
{
    (void)OSA_TaskCreate((osa_task_handle_t)ping_sock_thread, OSA_TASK(ping_sock_task), (osa_task_param_t)NULL);
    (void)OSA_TaskCreate((osa_task_handle_t)ncp_iperf_tx_thread, OSA_TASK(ncp_iperf_tx_task), (osa_task_param_t)NULL);
    (void)OSA_TaskCreate((osa_task_handle_t)ncp_iperf_rx_thread, OSA_TASK(ncp_iperf_rx_task), (osa_task_param_t)NULL);

    (void)OSA_EventCreate(ping_events, 1U);
    (void)OSA_EventCreate(iperf_tx_events, 1U);
    (void)OSA_EventCreate(iperf_rx_events, 1U);

    return NCP_STATUS_SUCCESS;    
}

static void wifi_ncp_callback(void *tlv, size_t tlv_sz, int status)
{
    int ret = 0;
    wifi_ncp_command_t cmd_item;

    cmd_item.block_type = 0;
    cmd_item.command_sz = tlv_sz;
    cmd_item.cmd_buff = (ncp_tlv_qelem_t *)OSA_MemoryAllocate(tlv_sz);
    if (!cmd_item.cmd_buff)
    {
        ncp_e("failed to allocate memory for tlv queue element");
        return ;
    }
    memcpy(cmd_item.cmd_buff, tlv, tlv_sz);

    ret = OSA_MsgQPut(wifi_ncp_command_queue, &cmd_item);
    if (ret != NCP_STATUS_SUCCESS)
    {
        ncp_e("send to wifi ncp cmd queue failed");
        OSA_MemoryFree(cmd_item.cmd_buff);
    }
    else
        ncp_d("success to send ncp command on queue");
}

static int wifi_ncp_handle_cmd_input(uint8_t *cmd)
{
    uint32_t msg_type = 0;
    int ret;
    
    msg_type = GET_MSG_TYPE(((NCP_HOST_COMMAND *)cmd)->cmd);
    if (msg_type == NCP_MSG_TYPE_EVENT)
    {
        ret = wlan_process_ncp_event(cmd);
        if (ret != NCP_STATUS_SUCCESS)
            ncp_e("Failed to parse ncp event");
    }
    else
    {
        ret = wlan_process_response(cmd);
        if (ret == -NCP_STATUS_ERROR)
            ncp_e("Failed to parse ncp tlv reponse");

        mcu_last_resp_rcvd = ((MCU_NCPCmd_DS_COMMAND *)cmd)->header.cmd;
        if (mcu_last_resp_rcvd == NCP_RSP_INVALID_CMD)
        {
            ncp_e("Previous command is invalid");
            mcu_last_resp_rcvd = 0;
        }
    }

    if (msg_type == NCP_MSG_TYPE_RESP)
    {
        /*If failed to receive response or successed to parse tlv reponse, release mcu command response semaphore to
         * allow processing new string commands. If reponse can't match to command, don't release command reponse
         * semaphore until receive response which id is same as command id.*/
        if (mcu_last_resp_rcvd == 0 || mcu_last_resp_rcvd == (mcu_last_cmd_sent | NCP_MSG_TYPE_RESP))
            mcu_put_command_resp_sem();
        else
            ncp_e("Receive %d command response and wait for %d comamnd response.", mcu_last_resp_rcvd,
                  mcu_last_cmd_sent);
    }
    return ret;
}

void wifi_ncp_task(void *pvParameters)
{
    int ret = 0;
    wifi_ncp_command_t cmd_item;
    uint8_t *cmd_buf = NULL;
    while (1)
    {
        ret = OSA_MsgQGet(wifi_ncp_command_queue, &cmd_item, osaWaitForever_c);
        if (ret != NCP_STATUS_SUCCESS)
        {
            ncp_e("wifi ncp command queue receive failed");
            continue;
        }
        else
        {
            cmd_buf = cmd_item.cmd_buff;
            wifi_ncp_handle_cmd_input(cmd_buf);
            OSA_MemoryFree(cmd_buf);
            cmd_buf = NULL;
        }
    }
}

int ncp_wifi_app_init()
{
    int ret;

    wifi_ncp_command_queue = (osa_msgq_handle_t)wifi_ncp_command_queue_buff;
    ret = OSA_MsgQCreate(wifi_ncp_command_queue, WIFI_NCP_COMMAND_QUEUE_NUM,  sizeof(wifi_ncp_command_t));
    if (ret != NCP_STATUS_SUCCESS)
    {
        ncp_e("failed to create wifi ncp command queue: %d", ret);
        return -NCP_STATUS_ERROR;
    }

    ncp_tlv_install_handler(NCP_CMD_WLAN, (void *)wifi_ncp_callback);

    (void)OSA_TaskCreate((osa_task_handle_t)wifi_ncp_task_handle, OSA_TASK(wifi_ncp_task), NULL);

    ret = ncp_wifi_cli_init();
    if (ret != NCP_STATUS_SUCCESS)
    {
        ncp_e("failed to init wifi cli: %d", ret);
        return -NCP_STATUS_ERROR;
    }

    return ret;
}
