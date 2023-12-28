/*
 * Copyright 2022-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "socket_task.h"
#include "shell_task_mode.h"

#include "lwip/sys.h"
#include "lwip/netif.h"

#define TCP_SERVER_CONNECTIONS_MAX 2

typedef struct
{
    int sck;
    int sck_accepted[TCP_SERVER_CONNECTIONS_MAX];
    int af;
    int sck_type;
    int is_server;
    struct sockaddr_in ipv4;
    struct sockaddr_in6 ipv6;
} example_sockinfo_t;

volatile int run;
static example_sockinfo_t s_sockinfo;

static int ip_port_str_to_sockaddr(const char *ip_str,
                                   const char *port_str,
                                   struct sockaddr_in *ipv4,
                                   struct sockaddr_in6 *ipv6)
{
    int ret;
    int af;
    int port;

    /* Convert port */
    port = atoi(port_str);
    if (port > 0xffff)
    {
        PRINTF("Port '%s' is not lower than 65536\r\n", port_str);
        return -1;
    }
    if (port <= 0)
    {
        PRINTF("Port '%s' is not greater than 0\r\n", port_str);
        return -1;
    }

    /* Convert IP */
    af = AF_INET;
    memset(ipv4, 0, sizeof(struct sockaddr_in));
    ipv4->sin_len    = sizeof(struct sockaddr_in);
    ipv4->sin_family = af;
    ipv4->sin_port   = htons(port);
    ret              = inet_pton(af, ip_str, &ipv4->sin_addr.s_addr);
    if (ret != 1)
    {
        /* Address is not valid IPv4 address. Lets try treat it as IPv6 */

        af = AF_INET6;
        memset(ipv6, 0, sizeof(struct sockaddr_in6));
        ipv6->sin6_len      = sizeof(struct sockaddr_in6);
        ipv6->sin6_family   = af;
        ipv6->sin6_port     = htons(port);
        ipv6->sin6_scope_id = netif_get_index(netif_default);

        LOCK_TCPIP_CORE();
        ret = inet_pton(af, ip_str, &ipv6->sin6_addr.s6_addr);
        UNLOCK_TCPIP_CORE();

        if (ret != 1)
        {
            PRINTF("'%s' is not valid IPv4 nor IPv6 address.\r\n", ip_str);
            return -1;
        }

#if LWIP_IPV6_SCOPES == 1
        /* IPv6 string can contain scope id, check if any netif matches */
        ip6_addr_t addr;
        LOCK_TCPIP_CORE();
        ip6addr_aton(ip_str, &addr);
        UNLOCK_TCPIP_CORE();
        if (addr.zone > 0)
        {
            ipv6->sin6_scope_id = addr.zone;
        }
#endif
    }

    return af;
}

static int set_receive_timeout(int sck)
{
    struct timeval timeout = {.tv_usec = 50 * 1000, .tv_sec = 0};

    int err = lwip_setsockopt(sck, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if (err)
    {
        PRINTF("Setting socket receive timeout failed (%d).\r\n", err);
    }

    return err;
}

static void echo_udp(int sck)
{
    int err;
    uint8_t buf[1500];

    struct sockaddr_storage sender_addr;
    socklen_t sender_addr_len = sizeof(sender_addr);

    PRINTF("Use end command to return...");
    shell_task_set_mode("ECHO_UDP>> ");

    err = set_receive_timeout(sck);
    if (err)
    {
        return;
    }

    while (1)
    {
        ssize_t bytes = recvfrom(sck, &buf, sizeof(buf), 0, (struct sockaddr *)&sender_addr, &sender_addr_len);

        if (bytes > 0)
        {
            sendto(sck, &buf, bytes, 0, (struct sockaddr *)&sender_addr, sender_addr_len);
            PRINTF("Datagram carrying %dB sent back.\r\n", bytes);
        }
        else if (!run)
        {
            // end was called.
            return;
        }
        else if (errno == EWOULDBLOCK)
        {
            // Timeout is here to allow check if we should continue so call read again.
        }
        else
        {
            PRINTF("Socket terminated. (errno=%d).\r\n", errno);
            return;
        }
    }
}

static void echo_loop_tcp(int sck)
{
    int err;
    uint8_t buf[1500];

    PRINTF("\r\nEchoing data. Use end command to return...");
    PRINTF("\r\n");

    err = set_receive_timeout(sck);
    if (err)
    {
        return;
    }

    while (1)
    {
        ssize_t bytes = read(sck, &buf, sizeof(buf));
        if (bytes > 0)
        {
            bytes = write(sck, &buf, bytes);
            if (bytes >= 0)
            {
                PRINTF("%dB sent back.\r\n", bytes);
            }
            else
            {
                PRINTF("write() failed (errno=%d)\r\n", errno);
            }
        }
        else if (!run)
        {
            // end was called.
            return;
        }
        else if (errno == EWOULDBLOCK)
        {
            // Timeout is here to allow check if we should continue so call read again.
        }
        else
        {
            PRINTF("Connection terminated. (errno=%d).\r\n", errno);
            return;
        }
    }
}

static void finish_thread(int sck)
{
    if (sck != -1)
    {
        close(sck);
        sck = -1;
    }
    run = 0;

    shell_task_set_mode(SHELL_MODE_DEFAULT);

    vTaskDelete(NULL);
}

static void tcp_connect_thread(void *arg)
{
    example_sockinfo_t *sockinfo = (example_sockinfo_t *)arg;

    int err;

    PRINTF("Connecting...\r\n");
    if (sockinfo->af == AF_INET)
    {
        err = connect(sockinfo->sck, (struct sockaddr *)&sockinfo->ipv4, sizeof(sockinfo->ipv4));
    }
    else
    {
        err = connect(sockinfo->sck, (struct sockaddr *)&sockinfo->ipv6, sizeof(sockinfo->ipv6));
    }
    if (err)
    {
        PRINTF("Connecting failed. errno=%d\r\n", errno);
    }
    else
    {
        PRINTF("Connected.\r\n");
        shell_task_set_mode("ECHO_TCP_CLIENT>> ");
        echo_loop_tcp(sockinfo->sck);
    }

    finish_thread(sockinfo->sck);
}

static void tcp_server_thread(void *arg)
{
    int sck = *((int *)arg);

    echo_loop_tcp(sck);

    finish_thread(sck);
}

static void tcp_listen_thread(void *arg)
{
    example_sockinfo_t *sockinfo = (example_sockinfo_t *)arg;

    int ret;

    if (sockinfo->af == AF_INET)
    {
        ret = bind(sockinfo->sck, (struct sockaddr *)&sockinfo->ipv4, sizeof(sockinfo->ipv4));
    }
    else
    {
        ret = bind(sockinfo->sck, (struct sockaddr *)&sockinfo->ipv6, sizeof(sockinfo->ipv6));
    }
    if (ret < 0)
    {
        PRINTF("bind() failed (errno=%d)\r\n", errno);
    }
    else
    {
        PRINTF("Waiting for incoming connection.  Use end command to return...");
        shell_task_set_mode("ECHO_TCP_SERVER>> ");
        listen(sockinfo->sck, 0); // zero to use the smallest connection backlog possible (see tcp_backlog_set)
        fcntl(sockinfo->sck, F_SETFL, O_NONBLOCK);

        run                  = 1;
        int accepted_sck_cnt = 0;
        while (run)
        {
            if (accepted_sck_cnt >= TCP_SERVER_CONNECTIONS_MAX)
            {
                // Reached maximum connections.
                vTaskDelay(pdMS_TO_TICKS(50));
                continue;
            }
            sockinfo->sck_accepted[accepted_sck_cnt] = accept(sockinfo->sck, NULL, 0);
            if (sockinfo->sck_accepted[accepted_sck_cnt] < 0)
            {
                // Nothing to accept. Wait 50ms and try it again.
                vTaskDelay(pdMS_TO_TICKS(50));
            }
            else
            {
                PRINTF("\r\nAccepted connection\r\n");

                // Create thread that serves this connection
                sys_thread_t thread =
                    sys_thread_new("tcp_server_thread", tcp_server_thread,
                                   (void *)&(sockinfo->sck_accepted[accepted_sck_cnt]), 1024, DEFAULT_THREAD_PRIO);

                if (thread == NULL)
                {
                    PRINTF("Can not create TCP connection server thread\r\n");
                    close(sockinfo->sck_accepted[accepted_sck_cnt]);
                    sockinfo->sck_accepted[accepted_sck_cnt] = -1;
                }
                else
                {
                    accepted_sck_cnt += 1;
                }
            }
        }
        // Listen thread stopped, clean up connections
        accepted_sck_cnt = 0;
    }

    finish_thread(sockinfo->sck);
}

static void udp_thread(void *arg)
{
    example_sockinfo_t *sockinfo = (example_sockinfo_t *)arg;
    int ret;

    if (sockinfo->af == AF_INET)
    {
        ret = bind(sockinfo->sck, (struct sockaddr *)&sockinfo->ipv4, sizeof(sockinfo->ipv4));
    }
    else
    {
        ret = bind(sockinfo->sck, (struct sockaddr *)&sockinfo->ipv6, sizeof(sockinfo->ipv6));
    }
    if (ret < 0)
    {
        PRINTF("bind() failed (errno=%d)\r\n", errno);
    }
    else
    {
        PRINTF("Waiting for datagrams\r\n");
        echo_udp(sockinfo->sck);
    }

    finish_thread(sockinfo->sck);
}

int socket_task_init(int is_tcp, const char *ip_str, const char *port_str)
{
    example_sockinfo_t *sockinfo = &s_sockinfo;
    void (*thread_func_ptr)(void *);

    const int is_server = (ip_str == NULL);
    sockinfo->is_server = is_server;

    sockinfo->af = ip_port_str_to_sockaddr((is_server) ? "::" : ip_str, port_str, &sockinfo->ipv4, &sockinfo->ipv6);
    if (sockinfo->af < 0)
    {
        return -1;
    }

    sockinfo->sck_type = (is_tcp) ? SOCK_STREAM : SOCK_DGRAM;

    PRINTF("Creating new socket.\r\n");
    sockinfo->sck = socket(sockinfo->af, sockinfo->sck_type, 0);
    if (sockinfo->sck < 0)
    {
        PRINTF("Socket creation failed. (%d)\r\n", sockinfo->sck);
        return -1;
    }

    if (is_tcp)
    {
        if (is_server)
        {
            thread_func_ptr = (void (*)(void *))tcp_listen_thread;
        }
        else
        {
            thread_func_ptr = (void (*)(void *))tcp_connect_thread;
        }
    }
    else
    {
        thread_func_ptr = (void (*)(void *))udp_thread;
    }

    run = 1;

    sys_thread_t thread = sys_thread_new("socket_thread", thread_func_ptr, (void *)sockinfo, 1024, DEFAULT_THREAD_PRIO);

    if (thread == NULL)
    {
        PRINTF("Can not create socket thread\r\n");
        close(sockinfo->sck);
        sockinfo->sck = -1;
    }

    return sockinfo->sck;
}

void socket_task_terminate(void)
{
    run = 0;
    PRINTF("\r\nAll socket tasks terminated\r\n");
}

void socket_task_print_ips(void)
{
    struct netif *netif_;
    NETIF_FOREACH(netif_)
    {
        PRINTF("************************************************\r\n");
        PRINTF(" Interface name   : %s%d\r\n", netif_->name, netif_->num);
        PRINTF(" IPv4 Address     : %s\r\n", ip4addr_ntoa(netif_ip4_addr(netif_)));
        PRINTF(" IPv4 Subnet mask : %s\r\n", ip4addr_ntoa(netif_ip4_netmask(netif_)));
        PRINTF(" IPv4 Gateway     : %s\r\n", ip4addr_ntoa(netif_ip4_gw(netif_)));
        for (int i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++)
        {
            const char *str_ip = "-";
            if (ip6_addr_isvalid(netif_ip6_addr_state(netif_, i)))
            {
                str_ip = ip6addr_ntoa(netif_ip6_addr(netif_, i));
            }
            PRINTF(" IPv6 Address%d    : %s\r\n", i, str_ip);
        }
        PRINTF("************************************************\r\n");
    }
}
