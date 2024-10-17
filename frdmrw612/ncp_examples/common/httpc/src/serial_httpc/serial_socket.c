/*
* Copyright 2024 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
* The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
*/
#include <wmtypes.h>
#include <osa.h>
#include <wm_utils.h>
#include <wm_net.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include "serial_httpc.h"
#include "serial_socket.h"
#include "serial_errno.h"
#include "serial_network.h"
#include "httpc.h"
#include <websockets.h>
#include <cli_utils.h>
#include <stdio.h>
#if defined(__GNUC__)
#include <strings.h>
#endif

#define RECV_CHUNK_LEN 1024
#define END_INDICATOR  "$$"
#define ESCAPE_CHAR    "\\"

static void serial_mwm_bridge_recv(void *arg);
/* Thread handle */
OSA_TASK_HANDLE_DEFINE(mwm_recv_thread);
OSA_TASK_DEFINE(serial_mwm_bridge_recv, OSA_PRIORITY_NORMAL, 1, 1024, 0);
uint8_t *recvbuf;

typedef struct
{
    int sockfd;
    conn_type_t type;
    struct sockaddr_in *server_addr;
    socklen_t sock_len;
} send_params_t;

int serial_mwm_send_sock_data(char *data, unsigned len, void *arg, int data_chunk)
{
    send_params_t *args = (send_params_t *)arg;
    int status = sendto(args->sockfd, data, len, 0, (const struct sockaddr *)args->server_addr, args->sock_len);
    if (status != -1)
        return 0;
    else
        return status;
}

static int get_sock_addr(struct sockaddr_in *server_addr, const char *host, int port)
{
    if (server_addr == NULL || host == NULL)
        return -1;
    server_addr->sin_family = AF_INET;
    server_addr->sin_port   = htons(port);
    if (!(*host))
    {
        server_addr->sin_addr.s_addr = INADDR_ANY;
    }
    else if (isalpha((int)host[0]))
    {
        struct hostent *entry = NULL;
        net_gethostbyname(host, &entry);
        if (entry)
        {
            memcpy(&server_addr->sin_addr.s_addr, entry->h_addr_list[0], entry->h_length);
        }
        else
        {
            return -2;
        }
    }
    else
    {
        server_addr->sin_addr.s_addr = inet_addr(host);
    }
    memset(&(server_addr->sin_zero), '\0', 8);
    return 0;
}
static int set_sock_block(int sockfd, bool block)
{
    int status;
    int opts = fcntl(sockfd, F_GETFL, 0);
    opts     = (block == true) ? (opts & (~O_NONBLOCK)) : (opts | O_NONBLOCK);
    status   = fcntl(sockfd, F_SETFL, opts);
    return status;
}

#define SEND_PKT_SIZE 4096
int post_data_socket(post_data_func_t post_this, char *send_data, int send_size, void *arg)
{
    int sendnum      = send_size / SEND_PKT_SIZE;
    int send_remaind = send_size % SEND_PKT_SIZE;
    for (int i = 0; i < sendnum; i++)
    {
        if (post_this(send_data, SEND_PKT_SIZE, arg, DATA_CONTINUE) < 0)
            return -E_FAIL;
    }
    if (send_remaind && (post_this(send_data, send_remaind, arg, DATA_DONE) < 0))
        return -E_FAIL;
    return 0;
}

#define DEFAULT_RETRY_COUNT 3
int ncp_sock_open(const char *socket_type, const char *domain_type, const char *prot)
{
    int h_slot;
    int retry_cnt;
    int sockfd = -1;
    unsigned int domain, protocol;
    int status;
    int type;
    conn_type_t ctype;

    /* type - tcp/udp */
    if (!socket_type)
    {
        httpc_e("status:invalid params");
        return -E_FAIL;
    }
    if (!strcasecmp(socket_type, "tcp"))
    {
        type  = SOCK_STREAM;
        ctype = tcp;
    }
    else if (!strcasecmp(socket_type, "udp"))
    {
        type  = SOCK_DGRAM;
        ctype = udp;
    }
    else if (!strcasecmp(socket_type, "raw"))
    {
        type  = SOCK_RAW;
        ctype = raw;
    }
    else
    {
        httpc_e("status:invalid type");
        return -E_FAIL;
    }

    domain   = AF_INET;
    protocol = 0;

    if (domain_type)
    {
        if (!strncmp(domain_type, "ipv6", 4))
            domain = AF_INET6;
        else if (!strncmp(domain_type, "ipv4", 4))
            domain = AF_INET;
        else if (!strlen(domain_type))
            domain = AF_INET;
        else
        {
            httpc_e("status:invalid domain_type");
            return -E_FAIL;
        }
    }
    if (prot)
    {
        if (!strncmp(prot, "icmp", 4))
            protocol = IPPROTO_ICMP;
        else if (!strncmp(prot, "icmpv6", 6))
            protocol = IPPROTO_ICMPV6;
        else
            protocol = IPPROTO_IP;
    }

    /* Retry connection till socket creation is successful or the
     * retry-count has exhausted */
    retry_cnt = DEFAULT_RETRY_COUNT;

    while (retry_cnt--)
    {
        sockfd = net_socket(domain, type, protocol);
        if (sockfd >= 0)
            break;
        /* Wait some time to allow some sockets to get released */
        OSA_TimeDelay(1000);
    }
    if (sockfd == -1)
    {
        httpc_e("status:socket open failed");
        httpc_e("errno:%d", errno);
        return -E_FAIL;
    }
    int keepalive = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));

    /* Populate the socket handle and connection type in the
     * open_sessions structure */
    h_slot = get_empty_handle_index();
    if (h_slot < 0)
    {
        net_close(sockfd);
        httpc_e("status:max handles reached");
        return -E_FAIL;
    }
    status = set_socket_handle(sockfd, ctype, h_slot);
    if (status != WM_SUCCESS)
    {
        net_close(sockfd);
        httpc_e("status:internal error setting handle");
        return -E_FAIL;
    }
    /* Once the handle is returned to the user it is expected that the
     * user should explicity close the  socket connection using
     * the mwm+nclose API */
    httpc_d("SUCCESS");
    httpc_d("handle:%d", (uint32_t)h_slot);
    return (int)h_slot;
}

int ncp_sock_connect(uint32_t handle, uint32_t port, char *ip)
{
    int h_slot, status;
    int sockfd;
    struct sockaddr_in server_addr;
    char *host = 0;
    conn_type_t type;

    /* socket handle */
    h_slot = handle;
    sockfd = get_handle(h_slot);
    type   = get_sock_type(h_slot);
    if (sockfd < 0 || (type != tcp && type != udp && type != raw))
    {
        httpc_e("status:invalid handle");
        return -E_FAIL;
    }

    /* address/hostname */
    host = ip;
    if (*host == '\0')
    {
        httpc_e("status:invalid params");
        return -E_FAIL;
    }
    /* port */
    status = get_sock_addr(&server_addr, host, port);
    if (status != 0)
    {
        httpc_e("status:cannot set socket address");
        return -E_FAIL;
    }
    status = connect(sockfd, (struct sockaddr *)&server_addr, (socklen_t)sizeof(server_addr));
    if (status == -1)
    {
        httpc_e("status:socket connect failed");
        httpc_e("errno:%d", errno);
        return -E_FAIL;
    }
    set_connected_state(sockfd, true);
    httpc_d("SUCCESS");
    return E_SUCCESS;
}

int ncp_sock_receive(uint32_t handle, uint32_t recv_size, uint32_t timeo, char *recv_data)
{
    int status;
    int h_slot, ret_len;
    int sockfd;
    uint8_t *outbuf;
    int size;
    conn_type_t type;
    h_slot = handle;
    sockfd = get_handle(h_slot);
    type   = get_sock_type(h_slot);
    if (sockfd < 0 || (type != tcp && type != udp && type != raw))
    {
        httpc_e("status:invalid handle");
        return -E_FAIL;
    }

    /* size */
    size = recv_size;
    /*raw socket input contains IP header length, max ip header size is 60Bytes.*/
    if (type == raw)
        size += 60;
    /* timeout in milliseconds (0 means the receive
       call will not time out) */
#if LWIP_SO_SNDRCVTIMEO_NONSTANDARD
    uint32_t timeout = timeo;
    socklen_t timeo_len	= sizeof(uint32_t);
#else
    struct timeval timeout = {timeo / 1000, (timeo % 1000) * 1000};
    socklen_t timeo_len	= sizeof(struct timeval);
#endif
    status                 = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, timeo_len);
    if (status == -1)
    {
        httpc_e("status:socket set-opt failed");
        httpc_e("errno:%d", errno);
        return -E_FAIL;
    }
    outbuf = (uint8_t *)recv_data;
    if (!outbuf)
    {
        httpc_e("status:out of memory");
        return -E_FAIL;
    }
    ret_len = recv(sockfd, outbuf, size, 0);
    if (ret_len == -1)
    {
        outbuf = NULL;
        httpc_e("status:error in reading data");
        httpc_e("errno:%d", errno);
        return -E_FAIL;
    }
    else if (ret_len == 0)
    {
        outbuf = NULL;
        httpc_e("status:peer has performed orderly shutdown");
        return -E_FAIL;
    }
    httpc_d("SUCCESS");
    httpc_d("content-len:%d", ret_len);

    if (ret_len)
    {
        httpc_d("%s", (char *)outbuf);
    }
    return ret_len;
}

int ncp_sock_receivefrom(
    uint32_t handle, uint32_t recv_size, uint32_t timeo, char *peer_ip, uint32_t *peer_port, char *recv_data)
{
    int h_slot, ret_len;
    int sockfd;
    uint8_t *outbuf;
    int size;
    int status;
    socklen_t addr_len;
    struct sockaddr_in server_addr;
    conn_type_t type;
    /* handle */
    h_slot = handle;
    sockfd = get_handle(h_slot);
    type   = get_sock_type(h_slot);
    if (sockfd < 0 || (type != tcp && type != udp && type != raw))
    {
        httpc_e("status:invalid type");
        return -E_FAIL;
    }
    /* size */
    size = recv_size;
    /*raw socket, the receive len want to add the ip header, the max length of ip header is 60 bytes.*/
    if (type == raw)
        size += 60;

    /* timeout in milliseconds (0 means the receive call
       will not time out) */
#if LWIP_SO_SNDRCVTIMEO_NONSTANDARD
    uint32_t timeout = timeo;
    socklen_t timeo_len	= sizeof(uint32_t);
#else
    struct timeval timeout = {timeo / 1000, (timeo % 1000) * 1000};
    socklen_t timeo_len	= sizeof(struct timeval);
#endif
    status                 = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, timeo_len);
    if (status == -1)
    {
        httpc_e("status:socket set-opt failed");
        httpc_e("errno:%d", errno);
        return -E_FAIL;
    }

    outbuf = (uint8_t *)recv_data;
    if (!outbuf)
    {
        httpc_e("status:out of memory");
        return -E_FAIL;
    }
    addr_len = sizeof(server_addr);
    ret_len  = recvfrom(sockfd, outbuf, size, 0, (struct sockaddr *)&server_addr, &addr_len);

    if (ret_len == -1)
    {
        outbuf = NULL;
        httpc_e("status:error in reading data");
        httpc_e("errno:%d", errno);
        return -E_FAIL;
    }
    else if (ret_len == 0)
    {
        outbuf = NULL;
        httpc_e("status:peer has performed orderly shutdown");
        return -E_FAIL;
    }
    httpc_d("SUCCESS");
    httpc_d("content-len:%d", ret_len);
    httpc_d("status:received data from %s:%d", inet_ntoa(server_addr.sin_addr.s_addr), ntohs(server_addr.sin_port));
    if (peer_ip)
        memcpy(peer_ip, inet_ntoa(server_addr.sin_addr.s_addr), strlen(inet_ntoa(server_addr.sin_addr.s_addr)) + 1);
    *peer_port = ntohs(server_addr.sin_port);

    if (ret_len)
    {
        httpc_d("%s", (char *)outbuf);
    }

    return ret_len;
}

int ncp_sock_send(uint32_t handle, uint32_t send_size, char *send_data)
{
    int h_slot, status;
    int sockfd;
    conn_type_t type;
    send_params_t args;
    /* handle */
    h_slot = handle;
    sockfd = get_handle(h_slot);
    type   = get_sock_type(h_slot);
    if (sockfd < 0 || (type != tcp && type != udp && type != raw))
    {
        httpc_e("status:invalid handle");
        return -E_FAIL;
    }

    /* sent data */
    args.sockfd      = sockfd;
    args.type        = type;
    args.server_addr = NULL;
    args.sock_len    = 0;

    status = post_data_socket(serial_mwm_send_sock_data, (char *)send_data, send_size, &args);
    /* Put on uart semaphore after the datamode */
    if (status != 0)
    {
        httpc_e("status:socket send failed");
        httpc_e("errno:%d", errno);
        return -E_FAIL;
    }
    httpc_d("SUCCESS");
    return E_SUCCESS;
}

int ncp_sock_sendto(uint32_t handle, char *ip_addr, uint32_t port, uint32_t send_size, char *send_data)
{
    int h_slot, status;
    int sockfd;
    char *host = 0;
    struct sockaddr_in server_addr;
    conn_type_t type;
    send_params_t args;
    /* handle */
    h_slot = handle;
    sockfd = get_handle(h_slot);
    type   = get_sock_type(h_slot);
    if (sockfd < 0 || (type != tcp && type != udp && type != raw))
    {
        httpc_e("status:invalid socket type");
        return -E_FAIL;
    }
    /* address/hostname */
    host   = ip_addr;
    status = get_sock_addr(&server_addr, host, port);
    if (status != 0)
    {
        httpc_e("status:cannot get socket address");
        return -E_FAIL;
    }
    /* sent data */
    args.sockfd      = sockfd;
    args.type        = type;
    args.server_addr = &server_addr;
    args.sock_len    = sizeof(struct sockaddr_in);
    status           = post_data_socket(serial_mwm_send_sock_data, send_data, send_size, &args);
    if (status != 0)
    {
        httpc_e("status:socket send failed");
        httpc_e("errno:%d", errno);
        return -E_FAIL;
    }
    httpc_d("SUCCESS");
    return E_SUCCESS;
}

int ncp_sock_bind(uint32_t handle, uint32_t port, char *ip)
{
    int status;
    int on = 1;
    int sockfd, h_slot;
    char *host = 0;
    conn_type_t type;
    struct sockaddr_in addr;
    /* handle */
    h_slot = handle;
    sockfd = get_handle(h_slot);
    type   = get_sock_type(h_slot);
    if (sockfd < 0 || (type != tcp && type != udp))
    {
        httpc_e("status:invalid handle");
        return -E_FAIL;
    }
    /* Allow socket descriptor to be reuseable */
    status = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
    if (status < 0)
    {
        httpc_e("status:socket set-opt failed");
        httpc_e("errno:%d", errno);
        return -E_FAIL;
    }
    /* address/hostname - if NULL, default is INADDR_ANY*/
    host = ip;
    /* port */
    status = get_sock_addr(&addr, host, port);
    if (status != 0)
    {
        httpc_e("status:cannot set socket address");
        return -E_FAIL;
    }
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        httpc_e("status:socket bind failed");
        httpc_e("errno:%d", errno);
        return -E_FAIL;
    }
    set_bind_state(sockfd, true);
    httpc_d("SUCCESS");
    return E_SUCCESS;
}

int ncp_sock_listen(uint32_t handle, uint32_t number)
{
    int sockfd, h_slot;
    int status;
    conn_type_t type;
    int backlog = 0;

    /* handle */
    h_slot = handle;
    sockfd = get_handle(h_slot);
    type   = get_sock_type(h_slot);
    if (sockfd < 0 || (type != tcp && type != udp))
    {
        httpc_e("status:invalid handle");
        return -E_FAIL;
    }
    /* backlog */
    backlog = number;
    if (listen(sockfd, backlog) == -1)
    {
        httpc_e("status:socket listen failed");
        httpc_e("errno:%d", errno);
        return -E_FAIL;
    }
    /* Set sockfd to non-blocking */
    status = set_sock_block(sockfd, false);
    if (status < 0)
    {
        httpc_e("status:socket set-opt failed");
        httpc_e("errno:%d", errno);
        return -E_FAIL;
    }
    httpc_d("SUCCESS");
    return E_SUCCESS;
}

int ncp_sock_accept(uint32_t handle)
{
    int status;
    int h_slot, new_h_slot;
    int sockfd, newsockfd;
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    conn_type_t type;
    /* handle */
    h_slot = handle;
    sockfd = get_handle(h_slot);
    type   = get_sock_type(h_slot);
    if (sockfd < 0 || (type != tcp && type != udp))
    {
        httpc_e("status:invalid handle");
        return -E_FAIL;
    }

    /* Get slot for new sock handle */
    new_h_slot = get_empty_handle_index();
    if (new_h_slot < 0)
    {
        httpc_e("status:max handles reached");
        return -E_FAIL;
    }
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_len);
    if (newsockfd == -1)
    {
        httpc_e("status:failed to accept connection");
        httpc_e("errno:%d", errno);
        return -E_FAIL;
    }
    status = set_socket_handle(newsockfd, type, new_h_slot);
    if (status != WM_SUCCESS)
    {
        net_close(newsockfd);
        httpc_e("status:internal error setting handle");
        return -E_FAIL;
    }
    set_connected_state(newsockfd, true);
    /* Once the handle is returned to the user it is expected that the
     * user should explicity close the  socket connection using
     * the mwm+nclose API */
    httpc_d("SUCCESS");
    httpc_d("handle:%d", (uint32_t)new_h_slot);
    return (int)new_h_slot;
}

int ncp_sock_close(uint32_t handle)
{
    int status;
    int sockfd, h_slot;
    /* handle */
    h_slot = handle;
    sockfd = get_handle(h_slot);
    if (sockfd < 0)
    {
        httpc_e("status:invalid handle");
        return -E_FAIL;
    }
    status = net_close(sockfd);
    if (status < 0)
    {
        httpc_e("status:error in closing socket");
        httpc_e("errno:%d", errno);
        return -E_FAIL;
    }
    set_connected_state(sockfd, false);
    set_bind_state(sockfd, false);
    /* Put the semaphore to allow next asynchronous event
       to be scheduled */
    OSA_SemaphorePost(get_nw_handle_ptr(h_slot)->handle_sem);
    remove_handle(h_slot);
    httpc_d("SUCCESS");
    return E_SUCCESS;
}

static void bridge_uart_write(uint8_t *outbuf, int len)
{
    int cntr = 0;
    while (cntr <= len)
    {
        if (outbuf[cntr] == '$')
        {
            httpc_e("%s", (char *)ESCAPE_CHAR);
        }
        httpc_e("%s", (char *)&outbuf[cntr]);
        cntr++;
    }
}
static void serial_mwm_bridge_recv(void *arg)
{
    int ret_len;
    int status;
    int timeo  = 0;
    int sockfd = *((int *)arg);

#if LWIP_SO_SNDRCVTIMEO_NONSTANDARD
    uint32_t timeout = timeo;
    socklen_t timeo_len	= sizeof(uint32_t);
#else
    struct timeval timeout = {timeo / 1000, (timeo % 1000) * 1000};
    socklen_t timeo_len	= sizeof(struct timeval);
#endif
    status                 = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, timeo_len);
    if (status == -1)
    {
        httpc_e("status:socket set-opt failed");
        httpc_e("errno:%d", errno);
        return;
    }

    while (1)
    {
        ret_len = recv(sockfd, recvbuf, RECV_CHUNK_LEN, 0);
        if (ret_len == -1)
        {
            httpc_e("status:error in reading data");
            httpc_e("errno:%d", errno);
            set_connected_state(sockfd, false);
            OSA_ThreadSelfComplete(NULL);
            break;
        }
        else if (ret_len == 0)
        {
            /* Write $$ on uart to indicate bridge mode end */
            httpc_e("status:peer has performed orderly shutdown");
            set_connected_state(sockfd, false);
            OSA_ThreadSelfComplete(NULL);
            break;
        }
        if (ret_len)
        {
            bridge_uart_write(recvbuf, ret_len);
        }
    }
}

int bridge_send_data(char *data, unsigned len, void *arg, int data_chunk)
{
    send_params_t *args = (send_params_t *)arg;

    if (is_connected(args->sockfd))
    {
        int status = send(args->sockfd, data, len, 0);
        if (status != -1)
            return 0;
        else
            return status;
    }
    return 0;
}

static int serial_mwm_bridge_send(int sockfd, conn_type_t type, int chunk_size)
{
    int status;
    send_params_t args;

    args.sockfd      = sockfd;
    args.type        = type;
    args.server_addr = NULL;
    args.sock_len    = 0;
    status           = post_data_socket(bridge_send_data, NULL, 0, &args);
    if (status != 0)
    {
        httpc_e("status:socket send failed");
        httpc_e("errno:%d", errno);
        return -E_FAIL;
    }
    return E_SUCCESS;
}

int serial_mwm_bridge_mode(const int argc, const char **argv)
{
    int ret = -E_FAIL;
    osa_status_t status;
    char *endptr;
    const char *str_val;
    int h_slot, chunk_size;
    int sockfd;
    conn_type_t type;

    if (argc != 3)
    {
        httpc_e("status:invalid params");
        return -E_FAIL;
    }
    /* handle */
    str_val = argv[0];
    if (*str_val == '\0')
    {
        httpc_e("status:invalid params");
        return -E_FAIL;
    }
    h_slot = strtoul(str_val, &endptr, 0);
    if (*endptr != '\0')
    {
        httpc_e("status:invalid handle");
        return -E_FAIL;
    }
    sockfd = get_handle(h_slot);
    type   = get_sock_type(h_slot);
    if (sockfd < 0 || (type != tcp && type != udp))
    {
        httpc_e("status:invalid handle");
        return -E_FAIL;
    }
    /* chunk size */
    str_val    = argv[1];
    chunk_size = strtoul(str_val, &endptr, 0);
    if (*endptr != '\0')
    {
        httpc_e("status:invalid handle");
        return -E_FAIL;
    }

    str_val = argv[2];
    if (strcmp(str_val, "$$") != 0)
    {
        httpc_e("status:invalid data");
        return -E_FAIL;
    }

    /* Check if socket is bind or connected */
    if (is_connected(sockfd) || is_bind(sockfd))
    {
        recvbuf = OSA_MemoryAllocate(RECV_CHUNK_LEN);
        if (!recvbuf)
        {
            httpc_e("status:out of memory");
            return -E_FAIL;
        }

        /* Get the UART mutex unpon entry into bridge mode */

        /* Spawn a thread which blocks on recv */
        status = OSA_TaskCreate((osa_task_handle_t)mwm_recv_thread, OSA_TASK(serial_mwm_bridge_recv), NULL);

        if (status != KOSA_StatusSuccess)
        {
            /* Put the UART mutex */
            OSA_MemoryFree(recvbuf);
            recvbuf = NULL;
            httpc_e("status:failure");
            return -E_FAIL;
        }
        /* This function blocks on uart_read and sends data
           over socket if data is present on uart. */
        ret = serial_mwm_bridge_send(sockfd, type, chunk_size);

        /* Delete the recv thread since user has terminated
           from bridge mode */
        OSA_TaskDestroy((osa_task_handle_t)mwm_recv_thread);

        /* Put the UART mutex */

        /* Free the allocated buffer */
        OSA_MemoryFree(recvbuf);
        recvbuf = NULL;

        if (ret != E_SUCCESS)
        {
            /* The error corresponding to this case has been
               written on uart from serial_mwm_bridge_send() */
            return ret;
        }
        else
        {
            httpc_d("SUCCESS");
            return E_SUCCESS;
        }
    }
    else
    {
        httpc_e("status:invalid state");
        return -E_FAIL;
    }
}

/*
struct cmd sock_commands[] = {
 { "sock", "<type>", ascii_sock_open },
 { "con", "<handle>,<host>,<port>", ascii_sock_connect },
 { "recv", "<handle>,<length>,<timeout>", ascii_sock_receive },
 { "recvfrom", "<handle>,<length>,<timeout>", ascii_sock_receivefrom },
 { "send", "<handle>,$$<CR/LF>[<data>]$$", ascii_sock_send },
 { "sendto", "<handle>,<host>,<port>,$$<CR/LF>[<data>]$$",
     ascii_sock_sendto },
 { "bind", "<handle>,[<host>],<port>", ascii_sock_bind },
 { "listen", "<handle>,<backlog>", ascii_sock_listen },
 { "accept", "<handle>", ascii_sock_accept },
 { "close", "<handle>", ascii_sock_close },
 { "bridge", "<handle>,<chunksize>,$$", serial_mwm_bridge_mode },
 { NULL, NULL, NULL },
};
*/
