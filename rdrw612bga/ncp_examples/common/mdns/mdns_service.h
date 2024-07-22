/*
 * Port to lwIP from uIP
 * by Jim Pettinato April 2007
 *
 * security fixes and more by Simon Goldschmidt
 *
 * uIP version Copyright (c) 2002-2003, Adam Dunkels.
 * Copyright 2022-2024 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MDNS_SERVICE_H_
#define _MDNS_SERVICE_H_

#include <wm_net.h>
#include <lwip/dns.h>
#include "wlan.h"
#include "mdns.h"
#include "mdns_priv.h"
#include "ncp_cmd_wifi.h"
#include "mdns_out.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define mdns_e(...) wmlog_e("MDNS", ##__VA_ARGS__)
#define mdns_w(...) wmlog_w("MDNS", ##__VA_ARGS__)

#if CONFIG_NCP_DEBUG
#define mdns_d(...) wmlog("MDNS", ##__VA_ARGS__)
#else
#define mdns_d(...)
#endif

/* DNS field TYPE used for "Resource Records" */
#define DNS_RRTYPE_A     1   /* a host address */
#define DNS_RRTYPE_NS    2   /* an authoritative name server */
#define DNS_RRTYPE_MD    3   /* a mail destination (Obsolete - use MX) */
#define DNS_RRTYPE_MF    4   /* a mail forwarder (Obsolete - use MX) */
#define DNS_RRTYPE_CNAME 5   /* the canonical name for an alias */
#define DNS_RRTYPE_SOA   6   /* marks the start of a zone of authority */
#define DNS_RRTYPE_MB    7   /* a mailbox domain name (EXPERIMENTAL) */
#define DNS_RRTYPE_MG    8   /* a mail group member (EXPERIMENTAL) */
#define DNS_RRTYPE_MR    9   /* a mail rename domain name (EXPERIMENTAL) */
#define DNS_RRTYPE_NULL  10  /* a null RR (EXPERIMENTAL) */
#define DNS_RRTYPE_WKS   11  /* a well known service description */
#define DNS_RRTYPE_PTR   12  /* a domain name pointer */
#define DNS_RRTYPE_HINFO 13  /* host information */
#define DNS_RRTYPE_MINFO 14  /* mailbox or mail list information */
#define DNS_RRTYPE_MX    15  /* mail exchange */
#define DNS_RRTYPE_TXT   16  /* text strings */
#define DNS_RRTYPE_AAAA  28  /* IPv6 address */
#define DNS_RRTYPE_SRV   33  /* service location */
#define DNS_RRTYPE_ANY   255 /* any type */

/** Maximum no. of different interfaces supported by mDNS. */
#define MAX_NETWORK_INTERFACE 2

/** Maximum no. of mDNS results. */
#define MAX_RESULTS 8
/* Note: the actual allocated space is subject to MAX_RESULTS_ALIGN, so round MAX_RESULTS up to a power of 2.*/
#define _ALIGN            8
#define MAX_RESULTS_ALIGN ((MAX_RESULTS + (_ALIGN - 1)) & ~(_ALIGN - 1))

/* To update mdns result ring buffer head & tail */
#define MDNS_BUFFER_UPDATE_HEAD_TAIL(ptr, cnt) (ptr) = ((ptr) + (cnt)) & (MAX_RESULTS_ALIGN - 1);

/**
 *  Check if ring buffer is full.
 *
 *  Case A:
 *    ----------DDDDDDDDDDDDDDD--------------X
 *    ^         ^              ^             ^
 *    |         |              |             |
 *    0       tail           head           max
 *
 *  Free Space => '-----------'
 *
 *  Case B:
 *    DDDDDDDDDD---------------DDDDDDDDDDDDDDX
 *    ^         ^              ^             ^
 *    |         |              |             |
 *    0       head           tail           max
 *
 *  Free Space => '-----------'
 *
 *  Buffer Empty Condition: when tail == head
 *  Buffer Full  Condition: when diff(tail, head) == 1
 */
#define MDNS_BUFFER_IS_FULL(tail, head) (((head + 1) & (MAX_RESULTS_ALIGN - 1)) == tail)

typedef struct _interface
{
    struct netif netif;
    ip_addr_t ipaddr;
    ip_addr_t nmask;
    ip_addr_t gw;
} interface_t;

typedef struct _mdns_service_config
{
    /** interface handle */
    void *iface_handle;
    /** network interface */
    struct netif *mdns_netif;
    /** service announced on given interface */
    struct mdns_service *service;
    /** service id */
    uint8_t service_id;
} mdns_service_config_t;

enum mdns_ifac_role
{
    MDNS_IFAC_ROLE_STA = 0,
    MDNS_IFAC_ROLE_UAP = 1,
};

/* mDNS ip item structure */
typedef struct mdns_ip_addr
{
    uint8_t addr_type;
    union
    {
        uint32_t ip_v4;
        uint32_t ip_v6[4];
    } ip;
    struct mdns_ip_addr *next;
} mdns_ip_addr_t;

/**
 * @brief   mDNS query result structure
 */
typedef struct mdns_result
{
    uint8_t only_ptr; /* _services._dns_sd, don't checkout other answer */

    char *service_name; /* pointer to the instance name or hostname, don't allocate memory */

    uint32_t ttl; /* time to live */

    /* PTR */
    char *instance_name; /* instance name */
    char *service_type;  /* service type */
    char *proto;         /* srevice protocol */
    /* SRV */
    char *hostname; /* host name */
    uint16_t port;  /* service port */
    char *target;
    /* TXT */
    uint8_t txt_len; /* txt value len */
    char *txt;       /* txt value */
    /* A and AAAA */
    uint8_t ip_count;
    mdns_ip_addr_t *ip_addr; /* linked list of IP addresses found */
} mdns_result_t;

/* mDNS result ring buffer structure */
typedef struct _mdns_result_ring_buffer
{
    mdns_result_t ring_buffer[MAX_RESULTS_ALIGN]; /* ring buffer */
    uint8_t ring_buffer_size;                     /* ring buffer size */
    volatile uint8_t ring_head;                   /* ring buffer head */
    volatile uint8_t ring_tail;                   /* ring buffer tail*/
} mdns_result_ring_buffer_t;

typedef struct _info_t
{
    unsigned only_ptr : 1;
} info_t;

int app_mdns_register_iface(void *iface);

int app_mdns_deregister_iface(void *iface);

/**
 *  @brief Restart mdns responder. Call this when cable is connected after being disconnected or
 * administrative interface is set up after being down
 *
 *  @param iface     The interface to send on
 *
 *  @return None
 */
void app_mdns_resp_restart(void *iface);

/**
 *  @brief Configure and start MDNS service.
 *
 *  @param hostname  Domain name
 *
 *  @return None
 */
void app_mdns_start(const char *hostname);

/**
 *  @brief Free an entry in the mDNS results ring buffer
 *
 *  @param mdns_res  one entry in ring buffer
 *
 *  @return None
 */
void app_mdns_result_ring_buffer_free(mdns_result_t *mdns_res);

/**
 *  @brief Start a search request
 *
 *  @param query_ptr   The configuration of PTR query
 *
 *  @return WM_SUCCESS if success, -WM_FAIL otherwise.
 */
int app_mdns_search_service(QUERY_PTR_CFG *query_ptr, enum mdns_ifac_role role);

/**
 *  @brief Stop a search request.
 *
 *  @param request_id The search request to stop
 *
 *  @return None
 */
void app_mdns_search_stop(uint8_t request_id);

/**
 *  @brief Start address resolve request (IPv4)
 *
 *  @param hostname Domain name
 *
 *  @return WM_SUCCESS
 */
int app_mdns_query_a(const char *hostname, enum mdns_ifac_role role);

#endif /* _MDNS_SERVICE_H_ */
