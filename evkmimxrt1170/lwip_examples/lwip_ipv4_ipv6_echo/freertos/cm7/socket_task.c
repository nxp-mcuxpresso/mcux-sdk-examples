/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "socket_task.h"
#include "shell_task_mode.h"

#include "lwip/sys.h"
#include "lwip/netif.h"

static int s_sck               = -1;
static int s_sck_accepted_conn = -1;
static int s_af;
static int s_sck_type;
static struct sockaddr_in s_ipv4;
static struct sockaddr_in6 s_ipv6;
static volatile int s_run = 0;

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
        ret                 = inet_pton(af, ip_str, &ipv6->sin6_addr.s6_addr);

        if (ret != 1)
        {
            PRINTF("'%s' is not valid IPv4 nor IPv6 address.\r\n", ip_str);
            return -1;
        }
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

    s_run = 1;
    while (1)
    {
        ssize_t bytes = recvfrom(sck, &buf, sizeof(buf), 0, (struct sockaddr *)&sender_addr, &sender_addr_len);

        if (bytes > 0)
        {
            sendto(sck, &buf, bytes, 0, (struct sockaddr *)&sender_addr, sender_addr_len);
            PRINTF("Datagram carrying %dB sent back.\r\n", bytes);
        }
        else if (!s_run)
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

static void echo_loop_tcp(int sck, int is_server)
{
    int err;
    uint8_t buf[1500];

    PRINTF("\r\nEchoing data. Use end command to return...");
    shell_task_set_mode(is_server ? "ECHO_TCP_SERVER>> " : "ECHO_TCP_CLIENT>> ");
    PRINTF("\r\n");

    err = set_receive_timeout(sck);
    if (err)
    {
        return;
    }

    s_run = 1;
    while (1)
    {
        ssize_t bytes = read(sck, &buf, sizeof(buf));
        if (bytes > 0)
        {
            write(sck, &buf, bytes);
            PRINTF("%dB sent back.\r\n", bytes);
        }
        else if (!s_run)
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

static void finish_thread(void)
{
    close(s_sck);
    s_sck = -1;
    s_run = 0;

    shell_task_set_mode(SHELL_MODE_DEFAULT);

    vTaskDelete(NULL);
}

static void tcp_connect_thread(void *arg)
{
    (void)arg;

    int err;

    PRINTF("Connecting...\r\n");
    if (s_af == AF_INET)
    {
        err = connect(s_sck, (struct sockaddr *)&s_ipv4, sizeof(s_ipv4));
    }
    else
    {
        err = connect(s_sck, (struct sockaddr *)&s_ipv6, sizeof(s_ipv6));
    }
    if (err)
    {
        PRINTF("Connecting failed. errno=%d\r\n", errno);
    }
    else
    {
        PRINTF("Connected.\r\n");
        echo_loop_tcp(s_sck, 0);
    }

    finish_thread();
}

static void tcp_listen_thread(void *arg)
{
    (void)arg;

    int ret;

    if (s_af == AF_INET)
    {
        ret = bind(s_sck, (struct sockaddr *)&s_ipv4, sizeof(s_ipv4));
    }
    else
    {
        ret = bind(s_sck, (struct sockaddr *)&s_ipv6, sizeof(s_ipv6));
    }
    if (ret < 0)
    {
        PRINTF("bind() error\r\n");
    }
    else
    {
        PRINTF("Waiting for incoming connection.  Use end command to return...");
        shell_task_set_mode("ECHO_TCP_SERVER>> ");
        listen(s_sck, 0); // no backlog
        fcntl(s_sck, F_SETFL, O_NONBLOCK);

        s_run = 1;
        while (s_run)
        {
            s_sck_accepted_conn = accept(s_sck, NULL, 0);
            if (s_sck_accepted_conn < 0)
            {
                // Nothing to accept. Wait 50ms and try it again.
                vTaskDelay(pdMS_TO_TICKS(50));
            }
            else
            {
                PRINTF("\r\nAccepted connection");
                echo_loop_tcp(s_sck_accepted_conn, 1);
                close(s_sck_accepted_conn);
                s_sck_accepted_conn = -1;
                s_run               = 0;
            }
        }
    }

    finish_thread();
}

static void udp_thread(void *arg)
{
    (void)arg;
    int ret;

    if (s_af == AF_INET)
    {
        ret = bind(s_sck, (struct sockaddr *)&s_ipv4, sizeof(s_ipv4));
    }
    else
    {
        ret = bind(s_sck, (struct sockaddr *)&s_ipv6, sizeof(s_ipv6));
    }
    if (ret < 0)
    {
        PRINTF("bind() error\r\n");
    }
    else
    {
        PRINTF("Waiting for datagrams\r\n");
        echo_udp(s_sck);
    }

    finish_thread();
}

int socket_task_init(int is_tcp, const char *ip_str, const char *port_str)
{
    void (*thread_func_ptr)(void *);

    const int is_server = (ip_str == NULL);

    s_af = ip_port_str_to_sockaddr((is_server) ? "::" : ip_str, port_str, &s_ipv4, &s_ipv6);
    if (s_af < 0)
    {
        return -1;
    }

    s_sck_type = (is_tcp) ? SOCK_STREAM : SOCK_DGRAM;

    PRINTF("Creating new socket.\r\n");
    s_sck = socket(s_af, s_sck_type, 0);
    if (s_sck < 0)
    {
        PRINTF("Socket creation failed. (%d)\r\n", s_sck);
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

    sys_thread_t thread = sys_thread_new("socket_thread", thread_func_ptr, NULL, 1024, DEFAULT_THREAD_PRIO);

    if (thread == NULL)
    {
        PRINTF("Can not create socket thread\r\n");
        close(s_sck);
        s_sck = -1;
    }

    return s_sck;
}

void socket_task_terminate(void)
{
    s_run = 0;
}

void socket_task_print_ips(void)
{
    int i;

    PRINTF("************************************************\r\n");
    PRINTF(" IPv4 Address     : %s\r\n", ip4addr_ntoa(netif_ip4_addr(netif_default)));
    PRINTF(" IPv4 Subnet mask : %s\r\n", ip4addr_ntoa(netif_ip4_netmask(netif_default)));
    PRINTF(" IPv4 Gateway     : %s\r\n", ip4addr_ntoa(netif_ip4_gw(netif_default)));
    for (i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++)
    {
        const char *str_ip = "-";
        if (ip6_addr_isvalid(netif_ip6_addr_state(netif_default, i)))
        {
            str_ip = ip6addr_ntoa(netif_ip6_addr(netif_default, i));
        }
        PRINTF(" IPv6 Address%d    : %s\r\n", i, str_ip);
    }
    PRINTF("************************************************\r\n");
}
