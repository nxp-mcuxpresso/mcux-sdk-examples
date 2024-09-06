/*
 * Copyright 2022-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "mdns_service.h"
#include "app_notify.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void app_mdns_add_srv_txt(struct mdns_service *service, void *txt_userdata);

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifdef MDNS_STA_INTERFACE
#define SYS_STA_KEYVAL "txtvers=1:path=/sys:description=NCP APP Demo:interface=STA"
#endif
#define SYS_UAP_KEYVAL "txtvers=1:path=/sys:description=NCP APP Demo:interface=uAP"

/*******************************************************************************
 * Variables
 ******************************************************************************/
static mdns_result_ring_buffer_t g_mdns_results;
static mdns_result_t g_mdns_res_a_or_aaaa;
static ip_addr_t query_a_addr;

static bool mdns_started = 0;
int network_services;
static uint8_t g_request_id = 0xff;

static void *iface_list[MAX_NETWORK_INTERFACE];
mdns_service_config_t mdns_srv_config[MAX_NETWORK_INTERFACE];
static char s_mdns_hostname[MDNS_LABEL_MAXLEN + 1] = "";

#ifdef MDNS_STA_INTERFACE
struct mdns_service ncp_sta_service = {
    .name         = "ncp_sta",            /* Name of service */
    .service      = "_http",              /* Type of service */
    .txt_fn       = app_mdns_add_srv_txt, /* Callback function and userdata to update txtdata buffer */
    .txt_userdata = SYS_STA_KEYVAL,
    .proto        = DNSSD_PROTO_TCP,
    .port         = 80,
};
#endif

struct mdns_service ncp_uap_service = {
    .name         = "ncp_uap",            /* Name of service */
    .service      = "_http",              /* Type of service */
    .txt_fn       = app_mdns_add_srv_txt, /* Callback function and userdata to update txtdata buffer */
    .txt_userdata = SYS_UAP_KEYVAL,
    .proto        = DNSSD_PROTO_TCP,
    .port         = 80,
};

/*******************************************************************************
 * Code
 ******************************************************************************/
/**
 *  @brief Allocate memory
 *
 *  @param size         This is the size of the memory block, in bytes.
 *
 *  @return             This function returns a pointer to the allocated memory, or NULL if
 *                      the request fails.
 */
static void *app_mdns_malloc(size_t size)
{
    if (size == 0)
        return NULL;

    void *pbuffer = OSA_MemoryAllocate(size);
    return pbuffer;
}

/**
 *  @brief Free memory
 *
 *  @param buffer        This is the pointer to a memory block previously allocated.
 *                       If a null pointer is passed as an argument, no action occurs.
 *
 *  @return              None.
 */
static void app_mdns_free(void *buffer)
{
    OSA_MemoryFree(buffer);
}

/**
 *  @brief Initialize the ring buffer structure
 *
 *  @param ring_buf      This is the pointer to a mdns result ring buffer.
 *
 *  @return              None.
 */
static void app_mdns_result_ring_buffer_init(mdns_result_ring_buffer_t *ring_buf)
{
    if (ring_buf == NULL)
    {
        mdns_e("failed to initialize mDNS result ring buffer");
        return;
    }

    /* Initialize mDNS result ring buffer */
    ring_buf->ring_buffer_size = MAX_RESULTS_ALIGN;
    ring_buf->ring_head        = 0;
    ring_buf->ring_tail        = 0;
}

static void app_mdns_status_report(struct netif *netif, u8_t result, s8_t service)
{
    mdns_d("mdns status[netif %d][service %d]: %s\n", netif->num, service,
           (result == MDNS_PROBING_SUCCESSFUL) ? "Success" : "Failed");
}

/**
 *  @brief Callback function to generate TXT mDNS record for service.
 *
 *  @param service              Announced sevice
 *  @param txt_userdata         TXT mDNS record
 *
 *  @return None
 */
static void app_mdns_add_srv_txt(struct mdns_service *service, void *txt_userdata)
{
    mdns_resp_add_service_txtitem(service, txt_userdata, strlen(txt_userdata));
}

static void app_set_mdns_service_config(mdns_service_config_t *service_config, enum mdns_ifac_role role)
{
    if (service_config == NULL)
        return;

    mdns_srv_config[role].iface_handle = service_config->iface_handle;
    mdns_srv_config[role].mdns_netif   = service_config->mdns_netif;
    mdns_srv_config[role].service      = service_config->service;
    mdns_srv_config[role].service_id   = service_config->service_id;
}

static struct netif *app_get_mdns_service_netif(enum mdns_ifac_role role)
{
    return mdns_srv_config[role].mdns_netif;
}

static void **app_get_network_interface_list(void)
{
    return iface_list;
}

/**
 *  @brief Activate mDNS responder and add a service for a netwrok interface.
 *
 *  @param mdns_netif           The network interface to publish this service on
 *  @param srv                  Announced service
 *
 *  @return service_id if the service was added to the netif, -WM_FAIL otherwise
 */
static int app_mdns_add_service_iface(struct netif *mdns_netif, struct mdns_service *srv)
{
    int ret = -WM_FAIL;
    int service_id;

    if (mdns_netif == NULL || srv == NULL)
        return -WM_FAIL;

    ret = mdns_resp_add_netif(mdns_netif, srv->name);
    if (ret != WM_SUCCESS)
        return -WM_FAIL;

    service_id = mdns_resp_add_service(mdns_netif, srv->name, srv->service, (enum mdns_sd_proto)srv->proto, srv->port,
                                       srv->txt_fn, srv->txt_userdata);
    if (service_id < 0)
        return -WM_FAIL;

    return service_id;
}

/**
 *  @brief Stop mDNS responder and remove a specific service on a netwrok interface.
 *
 *  @param service_config   The configuration of mDNS service
 *
 *  @return WM_SUCCESS if success, -WM_FAIL otherwise
 */
static int app_mdns_remove_service_iface(mdns_service_config_t *service_config)
{
    int ret = -WM_FAIL;

    if (service_config == NULL)
        return -WM_FAIL;

    struct mdns_host *mdns = netif_mdns_data(service_config->mdns_netif);
    if (mdns)
    {
        sys_untimeout(mdns_multicast_timeout_25ttl_reset_ipv4, service_config->mdns_netif);
        sys_untimeout(mdns_multicast_timeout_25ttl_reset_ipv6, service_config->mdns_netif);
        mdns_multicast_timeout_25ttl_reset_ipv4(service_config->mdns_netif);
        mdns_multicast_timeout_25ttl_reset_ipv6(service_config->mdns_netif);
    }

    ret = mdns_resp_del_service(service_config->mdns_netif, service_config->service_id);
    if (ret != WM_SUCCESS)
    {
        mdns_e("failed to delete a service on the network interface: %p", service_config->mdns_netif);
        return -WM_FAIL;
    }

    ret = mdns_resp_remove_netif(service_config->mdns_netif);
    if (ret != WM_SUCCESS)
    {
        mdns_e("failed to remove network interface");
    }

    return ret;
}

int app_mdns_register_iface(void *iface)
{
    mdns_service_config_t service_config = {0};
    enum mdns_ifac_role role;
    int id;

    if (iface == NULL)
        return -WM_FAIL;

#ifdef MDNS_STA_INTERFACE
    if (iface == net_get_sta_handle())
    {
        mdns_d("Registering STA interface");
        service_config.iface_handle = iface;
        service_config.mdns_netif   = &((interface_t *)iface)->netif;
        service_config.service      = &ncp_sta_service;
        role                        = MDNS_IFAC_ROLE_STA;
    }
    else
#endif
        if (iface == net_get_uap_handle())
    {
        mdns_d("Registering uAP interface");
        (void)strncpy(ncp_uap_service.name, s_mdns_hostname, sizeof(ncp_uap_service.name) - 1);

        service_config.iface_handle = iface;
        service_config.mdns_netif   = &((interface_t *)iface)->netif;
        service_config.service      = &ncp_uap_service;
        role                        = MDNS_IFAC_ROLE_UAP;
    }

    id = app_mdns_add_service_iface(service_config.mdns_netif, service_config.service);
    if (id < 0)
        return -WM_FAIL;

    service_config.service_id = id;
    app_set_mdns_service_config(&service_config, role);

    return WM_SUCCESS;
}

int app_mdns_deregister_iface(void *iface)
{
    int ret = -WM_FAIL;
    int i;

    if (iface == NULL)
        return -WM_FAIL;

    for (i = 0; i < MAX_NETWORK_INTERFACE; i++)
    {
        if (mdns_srv_config[i].iface_handle == iface)
        {
            if (g_request_id != 0xff)
            {
                app_mdns_search_stop(g_request_id);
            }

            app_mdns_remove_service_iface(&mdns_srv_config[i]);

            mdns_srv_config[i].iface_handle = NULL;
            mdns_srv_config[i].mdns_netif   = NULL;
            mdns_srv_config[i].service      = NULL;

            ret = WM_SUCCESS;
            break;
        }
    }

    return ret;
}

static void app_mdns_register_all_interfaces(void)
{
    int ret = -WM_FAIL;
    int i;

    mdns_d("Register all interfaces");

    void **if_list = app_get_network_interface_list();
    for (i = 0; i < MAX_NETWORK_INTERFACE; i++)
    {
        if (if_list[i] != NULL)
        {
            ret = app_mdns_register_iface(if_list[i]);
            if (ret != WM_SUCCESS)
                mdns_e("Failed to register interface: %p", if_list[i]);
            else
                mdns_d("Interface: %p successfully registered", if_list[i]);
        }
    }
}

void app_mdns_start(const char *hostname)
{
    if (mdns_started)
    {
        mdns_d("mdns already started");
        return;
    }

    /* Initiate MDNS responder and MDNS querier */
    mdns_resp_init();
    mdns_resp_register_name_result_cb(app_mdns_status_report);
    app_mdns_result_ring_buffer_init(&g_mdns_results);

    (void)strncpy(s_mdns_hostname, hostname, sizeof(s_mdns_hostname) - 1);
    s_mdns_hostname[sizeof(s_mdns_hostname) - 1] = '\0'; // Make sure string will be always terminated.

    mdns_started     = 1;
    network_services = 0;

    return;
}

void app_mdns_resp_restart(void *iface)
{
    int i;
    for (i = 0; i < MAX_NETWORK_INTERFACE; i++)
    {
        if (mdns_srv_config[i].iface_handle == iface)
        {
            mdns_resp_restart(mdns_srv_config[i].mdns_netif);
            break;
        }
    }
}

void app_mdns_result_ring_buffer_free(mdns_result_t *mdns_res)
{
    if (mdns_res == NULL)
        return;

    mdns_res->only_ptr     = 0;
    mdns_res->service_name = NULL;
    mdns_res->ttl          = 0;

    /* Free PTR RR */
    if (mdns_res->instance_name)
    {
        app_mdns_free(mdns_res->instance_name);
        mdns_res->instance_name = NULL;
    }

    if (mdns_res->service_type)
    {
        app_mdns_free(mdns_res->service_type);
        mdns_res->service_type = NULL;
    }

    if (mdns_res->proto)
    {
        app_mdns_free(mdns_res->proto);
        mdns_res->proto = NULL;
    }

    /* Free SRV RR */
    if (mdns_res->hostname)
    {
        app_mdns_free(mdns_res->hostname);
        mdns_res->hostname = NULL;
    }

    mdns_res->port = 0;

    if (mdns_res->target)
    {
        app_mdns_free(mdns_res->target);
        mdns_res->target = NULL;
    }

    /* Free TXT RR */
    if (mdns_res->txt)
    {
        app_mdns_free(mdns_res->txt);
        mdns_res->txt = NULL;
    }

    mdns_res->txt_len = 0;

    /* Free IP RR */
    mdns_ip_addr_t *tmp = mdns_res->ip_addr;
    while (tmp != NULL)
    {
        mdns_res->ip_addr = tmp->next;
        tmp->next         = NULL;
        app_mdns_free(tmp);
        tmp = mdns_res->ip_addr;
        mdns_res->ip_count--;
    }
}

/**
 *  @brief Add an entry to the mDNS results ring buffer
 *
 *  @param mdns_results  mDNS reuslt ring buffer
 *  @param res  The mDNS reuslt parsed from mDNS answer
 *  @param rr_type  The type of mDNS answer resource record
 *
 *  @return None
 */
static void app_mdns_result_ring_buffer_write(mdns_result_ring_buffer_t *mdns_results,
                                              mdns_result_t *res,
                                              uint16_t rr_type)
{
    uint32_t rd, wr;
    int i;

    rd = mdns_results->ring_tail;
    wr = mdns_results->ring_head;

    for (i = 0; i < MAX_RESULTS_ALIGN; i++)
    {
        if (!res->only_ptr)
        {
            if ((mdns_results->ring_buffer[i].service_name != NULL) &&
                !strcmp(mdns_results->ring_buffer[i].service_name, res->service_name))
                break;
        }
        else
        {
            /* Check if the service type already exists */
            if ((mdns_results->ring_buffer[i].only_ptr == 1) && (mdns_results->ring_buffer[i].service_type != NULL) &&
                (mdns_results->ring_buffer[i].proto != NULL) &&
                !strcmp(mdns_results->ring_buffer[i].service_type, res->service_type) &&
                !strcmp(mdns_results->ring_buffer[i].proto, res->proto))
                return;
        }
    }

    if (i == MAX_RESULTS_ALIGN)
    { /* No matching instance found */
        if (MDNS_BUFFER_IS_FULL(rd, wr))
        {
            mdns_d("mDNS result ring buffer is full!");
            /* Free the oldest data to make room for new data*/
            app_mdns_result_ring_buffer_free(&mdns_results->ring_buffer[rd]);

            /* Then, increase ring buffer tail */
            MDNS_BUFFER_UPDATE_HEAD_TAIL(mdns_results->ring_tail, 1);
        }

        /* add new instance */
        switch (rr_type)
        {
            case DNS_RRTYPE_PTR:
                mdns_results->ring_buffer[wr].instance_name = res->instance_name;
                mdns_results->ring_buffer[wr].service_type  = res->service_type;
                mdns_results->ring_buffer[wr].proto         = res->proto;
                break;
            case DNS_RRTYPE_SRV:
                mdns_results->ring_buffer[wr].hostname = res->hostname;
                mdns_results->ring_buffer[wr].port     = res->port;
                mdns_results->ring_buffer[wr].target   = res->target;
                break;
            case DNS_RRTYPE_TXT:
                mdns_results->ring_buffer[wr].txt_len = res->txt_len;
                mdns_results->ring_buffer[wr].txt     = res->txt;
                break;
            default:
                mdns_d("Invalid RR type");
                break;
        }
        mdns_results->ring_buffer[wr].only_ptr     = res->only_ptr;
        mdns_results->ring_buffer[wr].service_name = res->service_name;
        mdns_results->ring_buffer[wr].ttl          = res->ttl;

        /* Update ring buffer head */
        MDNS_BUFFER_UPDATE_HEAD_TAIL(mdns_results->ring_head, 1);
    }
    else if (i < MAX_RESULTS_ALIGN)
    {
        /* modify existing instance */
        switch (rr_type)
        {
            case DNS_RRTYPE_PTR:
                if (mdns_results->ring_buffer[i].instance_name)
                {
                    app_mdns_free(mdns_results->ring_buffer[i].instance_name);
                    mdns_results->ring_buffer[i].instance_name = NULL;
                }
                mdns_results->ring_buffer[i].instance_name = res->instance_name;

                if (mdns_results->ring_buffer[i].service_type)
                {
                    app_mdns_free(mdns_results->ring_buffer[i].service_type);
                    mdns_results->ring_buffer[i].service_type = NULL;
                }
                mdns_results->ring_buffer[i].service_type = res->service_type;

                if (mdns_results->ring_buffer[i].proto)
                {
                    app_mdns_free(mdns_results->ring_buffer[i].proto);
                    mdns_results->ring_buffer[i].proto = NULL;
                }
                mdns_results->ring_buffer[i].proto = res->proto;
                break;
            case DNS_RRTYPE_SRV:
                if (mdns_results->ring_buffer[i].hostname)
                {
                    app_mdns_free(mdns_results->ring_buffer[i].hostname);
                    mdns_results->ring_buffer[i].hostname = NULL;
                }
                mdns_results->ring_buffer[i].hostname = res->hostname;

                if (mdns_results->ring_buffer[i].target)
                {
                    app_mdns_free(mdns_results->ring_buffer[i].target);
                    mdns_results->ring_buffer[i].target = NULL;
                }
                mdns_results->ring_buffer[i].target = res->target;

                mdns_results->ring_buffer[i].port = res->port;
                break;
            case DNS_RRTYPE_TXT:
                if (mdns_results->ring_buffer[i].txt)
                {
                    app_mdns_free(mdns_results->ring_buffer[i].txt);
                    mdns_results->ring_buffer[i].txt = NULL;
                }
                mdns_results->ring_buffer[i].txt = res->txt;

                mdns_results->ring_buffer[i].txt_len = res->txt_len;
                break;
            default:
                break;
        }
        mdns_results->ring_buffer[i].only_ptr     = res->only_ptr;
        mdns_results->ring_buffer[i].service_name = res->service_name;
        mdns_results->ring_buffer[i].ttl          = res->ttl;
    }
}

/**
 *  @brief Read domain name from mDNS answer
 *
 *  @param name         The domain name
 *  @param dom_str      The domain name string
 *
 *  @return None
 */
static void app_mdns_domain_string(const uint8_t *name, OUT char *dom_str)
{
    const uint8_t *src = name;
    uint8_t i, j = 0;

    while (*src)
    {
        uint8_t label_len = *src;
        src++;
        for (i = 0; i < label_len; i++)
        {
            dom_str[j++] = src[i];
        }
        src += label_len;
        dom_str[j++] = '.';
    }
    dom_str[--j] = '\0';
}

/**
 *  @brief Read PTR domain name from mDNS answer
 *
 *  @param mdns_res_ptr The struct to fill with mDNS result
 *  @param name         Variable answer
 *
 *  @return None
 */
static void app_mdns_domain_string_ptr(mdns_result_t *mdns_res_ptr, const uint8_t *name)
{
    const uint8_t *src       = name;
    char *dom_str[6]         = {0};
    uint8_t label_len_arr[6] = {0};
    uint8_t i, j = 0;

    while (*src)
    {
        uint8_t label_len = *src;
        label_len_arr[j]  = label_len;

        dom_str[j] = (char *)app_mdns_malloc(label_len + 1);
        if (dom_str[j] == NULL)
        {
            mdns_e("failed to allocate memory for PTR domain name: Size: %d", label_len + 1);
            return;
        }
        src++;
        for (i = 0; i < label_len; i++)
        {
            dom_str[j][i] = src[i];
        }
        dom_str[j][i] = '\0';
        src += label_len;
        j++;
    }

    /**
     * case1: PTR domain name: <instance name>.<service type>.<protocol>.local
     * case2: Service discovery: <service type>.<protocol>.local
     */
    if (j == 4)
    {
        mdns_res_ptr->instance_name = dom_str[j - 4];
    }
    else if (j > 4)
    {
        int k, instance_len = 0;
        for (k = 0; k < (j - 4); k++)
        {
            instance_len += label_len_arr[k];
        }

        char *ins_name = (char *)malloc(instance_len + 1);
        int offset     = 0;
        for (k = 0; k < (j - 4); k++)
        {
            (void)memcpy(ins_name + offset, dom_str[k], label_len_arr[k]);
            offset += label_len_arr[k];
            app_mdns_free(dom_str[k]);
        }
        mdns_res_ptr->instance_name = ins_name;
    }
    else
    {
        mdns_res_ptr->instance_name = NULL;
    }
    mdns_res_ptr->service_type = dom_str[j - 3];
    mdns_res_ptr->proto        = dom_str[j - 2];

    app_mdns_free(dom_str[j - 1]);
}

/**
 *  @brief Parse PTR Resource Record from mDNS answers
 *
 *  @param mdns_res_ptr The struct to fill with mDNS result
 *  @param answer_ptr   The mDNS answer to read
 *  @param varpart      Variable answer
 *  @param varlen       Length of variable answer
 *
 *  @return None
 */
static void app_mdns_answer_parse_ptr(mdns_result_t *mdns_res_ptr,
                                      struct mdns_answer *answer_ptr,
                                      const char *varpart,
                                      int varlen)
{
    if (mdns_res_ptr == NULL || answer_ptr == NULL || varpart == NULL)
        return;

    app_mdns_domain_string_ptr(mdns_res_ptr, (const uint8_t *)varpart);

    /* service_name -> instance name */
    mdns_res_ptr->service_name = mdns_res_ptr->instance_name;
}

/**
 *  @brief Add PTR resource record to mDNS result ring buffer
 *
 *  @param mdns_results The mDNS result ring buffer
 *  @param res          The mDNS result with PTR resource record
 *
 *  @return None
 */
static void app_mdns_result_add_ptr(mdns_result_ring_buffer_t *mdns_results, mdns_result_t *res)
{
    if (mdns_results == NULL || res == NULL)
        return;

    app_mdns_result_ring_buffer_write(mdns_results, res, DNS_RRTYPE_PTR);
}

/**
 *  @brief Parse SRV Resource Record from mDNS answers
 *
 *  @param mdns_res_srv The struct to fill with mDNS result
 *  @param answer_srv   The mDNS answer to read
 *  @param varpart      Variable answer
 *  @param varlen       Length of variable answer
 *
 *  @return None
 */
static void app_mdns_answer_parse_srv(mdns_result_t *mdns_res_srv,
                                      struct mdns_answer *answer_srv,
                                      const char *varpart,
                                      int varlen)
{
    int i;
    char *target      = NULL;
    const char *pport = NULL;

    if (mdns_res_srv == NULL || answer_srv == NULL || varpart == NULL)
        return;

    const uint8_t *src     = answer_srv->info.domain.name;
    uint8_t label_len      = *src;
    mdns_res_srv->hostname = (char *)app_mdns_malloc(label_len + 1);
    if (mdns_res_srv->hostname == NULL)
    {
        mdns_e("failed to allocate memory for SRV hostname: Size: %d", label_len + 1);
        return;
    }
    src++;
    for (i = 0; i < label_len; i++)
    {
        mdns_res_srv->hostname[i] = src[i];
    }
    mdns_res_srv->hostname[i] = '\0';

    /* SRV RR: Prio, Weight, Port */
    pport              = varpart + 4;
    mdns_res_srv->port = ((uint16_t)(*pport) << 8) + ((uint16_t)(*(pport + 1)));

    target = (char *)app_mdns_malloc(strlen(((char *)varpart) + 6) + 1);
    if (target == NULL)
    {
        mdns_e("failed to allocate memory for target: Size: %d", label_len + 1);
        return;
    }
    app_mdns_domain_string(((const uint8_t *)varpart) + 6, target);
    mdns_res_srv->target = target;

    /* service_name -> hostname */
    mdns_res_srv->service_name = mdns_res_srv->hostname;
}

/**
 *  @brief Add SRV resource record to mDNS result ring buffer
 *
 *  @param mdns_results The mDNS result ring buffer
 *  @param res          The mDNS result with SRV resource record
 *
 *  @return None
 */
static void app_mdns_result_add_srv(mdns_result_ring_buffer_t *mdns_results, mdns_result_t *res)
{
    if (mdns_results == NULL || res == NULL)
        return;

    app_mdns_result_ring_buffer_write(mdns_results, res, DNS_RRTYPE_SRV);
}

/**
 *  @brief Parse TXT Resource Record from mDNS answers
 *
 *  @param mdns_res_srv The struct to fill with mDNS result
 *  @param answer_srv   The mDNS answer to read
 *  @param varpart      Variable answer
 *  @param varlen       Length of variable answer
 *
 *  @return None
 */
static void app_mdns_answer_parse_txt(mdns_result_t *mdns_res_txt,
                                      struct mdns_answer *answer_txt,
                                      const char *varpart,
                                      int varlen)
{
    int i;

    if (mdns_res_txt == NULL || answer_txt == NULL || varpart == NULL)
        return;

    const uint8_t *src     = answer_txt->info.domain.name;
    uint8_t label_len      = *src;
    mdns_res_txt->hostname = (char *)app_mdns_malloc(label_len + 1);
    if (mdns_res_txt->hostname == NULL)
    {
        mdns_e("failed to allocate memory for TXT hostname: Size: %d", label_len + 1);
        return;
    }
    src++;
    for (i = 0; i < label_len; i++)
    {
        mdns_res_txt->hostname[i] = src[i];
    }
    mdns_res_txt->hostname[i] = '\0';

    /* Length of txt value */
    mdns_res_txt->txt_len = *varpart;

    mdns_res_txt->txt = (char *)app_mdns_malloc(mdns_res_txt->txt_len + 1);
    if (mdns_res_txt->txt == NULL)
    {
        mdns_e("failed to allocate memory for txt value: Size: %d", mdns_res_txt->txt_len + 1);
        return;
    }
    (void)memcpy(mdns_res_txt->txt, varpart + 1, mdns_res_txt->txt_len);

    /* service_name -> hostname */
    mdns_res_txt->service_name = mdns_res_txt->hostname;
}

/**
 *  @brief Add TXT resource record to mDNS result ring buffer
 *
 *  @param mdns_results The mDNS result ring buffer
 *  @param res          The mDNS result with TXT resource record
 *
 *  @return None
 */
static void app_mdns_result_add_txt(mdns_result_ring_buffer_t *mdns_results, mdns_result_t *res)
{
    if (mdns_results == NULL || res == NULL)
        return;

    app_mdns_result_ring_buffer_write(mdns_results, res, DNS_RRTYPE_TXT);
}

/**
 *  @brief Parse A or AAAA Resource Record from mDNS answers
 *
 *  @param mdns_res_a The struct to fill with mDNS result
 *  @param answer_a   The mDNS answer to read
 *  @param varpart      Variable answer
 *  @param varlen       Length of variable answer
 *
 *  @return None
 */
static void app_mdns_answer_parse_a_or_aaaa(mdns_result_t *mdns_res_a,
                                            struct mdns_answer *answer_a,
                                            const char *varpart,
                                            int varlen)
{
    char *domain_name = NULL;

    if (mdns_res_a == NULL || answer_a == NULL || varpart == NULL)
        return;

    domain_name = (char *)app_mdns_malloc(strlen((char *)(answer_a->info.domain.name)) + 1);
    if (domain_name == NULL)
    {
        mdns_e("failed to allocate memory for A or AAAA domain name: Size: %d",
               strlen((char *)(answer_a->info.domain.name)) + 1);
        return;
    }
    app_mdns_domain_string(answer_a->info.domain.name, domain_name);
    mdns_res_a->target = domain_name;

    mdns_res_a->ip_addr = (mdns_ip_addr_t *)app_mdns_malloc(sizeof(mdns_ip_addr_t));
    if (mdns_res_a->ip_addr == NULL)
    {
        mdns_e("failed to allocate memory for A or AAAA ip address: Size: %d", sizeof(mdns_ip_addr_t));
        app_mdns_free(mdns_res_a->target);
        return;
    }
    (void)memcpy((char *)&mdns_res_a->ip_addr->ip, varpart, varlen);
    mdns_res_a->ip_addr->next = NULL;

    if (varlen == 4)
        mdns_res_a->ip_addr->addr_type = 4;
    else
        mdns_res_a->ip_addr->addr_type = 6;

    /* don't set service_name -> target */
    // mdns_res_a->service_name = mdns_res_a->target;
}

/**
 *  @brief Add IPv4/IPv6 address resource record to g_mdns_res_a_or_aaaa
 *
 *  @param mdns_results The mDNS result ring buffer
 *  @param res          The mDNS result with A or AAAA resource record
 *
 *  @return None
 */
static void app_mdns_result_add_a_or_aaaa(mdns_result_t *mdns_results, mdns_result_t *res)
{
    if (mdns_results == NULL || res == NULL)
        return;

    res->ip_addr->next = mdns_results->ip_addr;
    mdns_results->ip_addr = res->ip_addr;
    mdns_results->ip_count++;
    if (mdns_results->target != NULL)
    {
        app_mdns_free(mdns_results->target);
    }
    mdns_results->target = res->target;

}

static mdns_ip_addr_t *app_add_ip_addr(mdns_ip_addr_t *ip_addr)
{
    mdns_ip_addr_t *new_ip = NULL;

    new_ip = (mdns_ip_addr_t *)app_mdns_malloc(sizeof(mdns_ip_addr_t));
    if (new_ip == NULL)
    {
        mdns_e("failed to allocate memory for A or AAAA ip address: Size: %d", sizeof(mdns_ip_addr_t));
    }
    (void)memcpy((char *)new_ip, (char *)ip_addr, sizeof(mdns_ip_addr_t));

    return new_ip;
}

/**
 *  @brief Check all ip addresses and add them to instance.
 *
 *  @param mdns_res_list  The mDNS result ring buffer
 *  @param mdns_res       The mDNS result with A or AAAA resource record
 *
 *  @return None
 */
static void check_ip_addr(mdns_result_t *mdns_res_list, mdns_result_t *mdns_res)
{
    mdns_ip_addr_t *psrc = NULL;
    mdns_ip_addr_t *pdst = NULL;
    mdns_ip_addr_t *tmp  = NULL;

    if (mdns_res_list == NULL || mdns_res == NULL)
        return;

    if ((mdns_res_list->target != NULL) && !strcmp(mdns_res_list->target, mdns_res->target))
    {
        psrc = mdns_res->ip_addr;
        pdst = mdns_res_list->ip_addr;

        if (pdst == NULL)
        {
            while (psrc != NULL)
            {
                tmp = app_add_ip_addr(psrc);
                tmp->next = mdns_res_list->ip_addr;
                mdns_res_list->ip_addr = tmp;
                mdns_res_list->ip_count++;

                psrc = psrc->next;
            }
            return;
        }

        while (psrc != NULL)
        {
            tmp = pdst;
            while (tmp != NULL)
            {
                if (!memcmp((void *)&tmp->ip, (void *)&psrc->ip, sizeof(tmp->ip)))
                    goto next;
                tmp = tmp->next;
            }
            tmp = app_add_ip_addr(psrc);
            tmp->next = mdns_res_list->ip_addr;
            mdns_res_list->ip_addr = tmp;
            mdns_res_list->ip_count++;
            
next:
            psrc = psrc->next;
        }
    }
}

/**
 *  @brief Callback to send answer received. Will be called for each answer of a response frame matching request sent.
 *
 *  @param answer   The mDNS answer to read
 *  @param varpart      Variable answer
 *  @param varlen       Length of variable answer
 *  @param flags      First answer or last answer in received frame.
 *  @param arg       Userdata pointer for callback function
 *
 *  @return None
 */
static void app_mdns_search_result(struct mdns_answer *answer, const char *varpart, int varlen, int flags, void *arg)
{
#if CONFIG_NCP_DEBUG
    static uint8_t ans_num;
#endif
    mdns_result_t mdns_res;
    mdns_ip_addr_t *p_first = NULL;
    uint32_t rd = 0, wr = 0;
    info_t info      = *(info_t *)arg;
    uint8_t only_ptr = info.only_ptr;

    if (flags == MDNS_SEARCH_RESULT_FIRST || only_ptr)
    {
#if CONFIG_NCP_DEBUG
        ans_num = 0;
#endif
        mdns_d("==========Start==============");
    }

    mdns_d("mDNS Answer %d:", ++ans_num);
    (void)memset(&mdns_res, 0, sizeof(mdns_result_t));

    mdns_res.ttl      = answer->ttl;
    mdns_res.only_ptr = only_ptr;

    switch (answer->info.type)
    {
        case DNS_RRTYPE_PTR:
            app_mdns_answer_parse_ptr(&mdns_res, answer, varpart, varlen);
            if (mdns_res.service_name != NULL)
            {
                app_mdns_result_add_ptr(&g_mdns_results, &mdns_res);
            }
            mdns_d("     Type         : PTR (%d)", answer->info.type);
            mdns_d("     Instance Name: %s", mdns_res.instance_name);
            mdns_d("     Service Type : %s", mdns_res.service_type);
            mdns_d("     Proto        : %s", mdns_res.proto);
            mdns_d("     Sevice Name  : %s", mdns_res.service_name);
            break;
        case DNS_RRTYPE_SRV:
            app_mdns_answer_parse_srv(&mdns_res, answer, varpart, varlen);
            app_mdns_result_add_srv(&g_mdns_results, &mdns_res);
            mdns_d("     Type         : SRV (%d)", answer->info.type);
            mdns_d("     Host Name    : %s", mdns_res.hostname);
            mdns_d("     Port         : %d", mdns_res.port);
            mdns_d("     Target       : %s", mdns_res.target);
            mdns_d("     Sevice Name  : %s", mdns_res.service_name);
            break;
        case DNS_RRTYPE_TXT:
            app_mdns_answer_parse_txt(&mdns_res, answer, varpart, varlen);
            app_mdns_result_add_txt(&g_mdns_results, &mdns_res);
            mdns_d("     Type         : TXT (%d)", answer->info.type);
            mdns_d("     TXT Len      : %d", mdns_res.txt_len);
            mdns_d("     TXT          : %s", mdns_res.txt);
            mdns_d("     Sevice Name  : %s", mdns_res.service_name);
            break;
        case DNS_RRTYPE_A:
            app_mdns_answer_parse_a_or_aaaa(&mdns_res, answer, varpart, varlen);
            app_mdns_result_add_a_or_aaaa(&g_mdns_res_a_or_aaaa, &mdns_res);
#if CONFIG_NCP_DEBUG
            struct in_addr ip_a;
            ip_a.s_addr = mdns_res.ip_addr->ip.ip_v4;
#endif
            mdns_d("     Type         : A (%d)", answer->info.type);
            mdns_d("     IP           : %s", inet_ntoa(ip_a));
            mdns_d("     Target       : %s", mdns_res.target);
            break;
        case DNS_RRTYPE_AAAA:
            app_mdns_answer_parse_a_or_aaaa(&mdns_res, answer, varpart, varlen);
            app_mdns_result_add_a_or_aaaa(&g_mdns_res_a_or_aaaa, &mdns_res);
#if CONFIG_NCP_DEBUG
            int i;
            struct net_ipv6_config ip_aaaa;
            for (i = 0; i < 4; i++)
            {
                ip_aaaa.address[i] = mdns_res.ip_addr->ip.ip_v6[i];
            }
#endif
            mdns_d("     Type         : AAAA (%d)", answer->info.type);
            mdns_d("     IP           : %s (%s)", inet6_ntoa(ip_aaaa.address), ipv6_addr_type_to_desc(&ip_aaaa));
            mdns_d("     Target       : %s", mdns_res.target);
            break;
        default:
            mdns_d("Invaild Type: %d", answer->info.type);
            break;
    }

    if (flags == MDNS_SEARCH_RESULT_LAST)
    {
        rd = g_mdns_results.ring_tail;
        wr = g_mdns_results.ring_head;
        while (rd != wr)
        {
            /* linked list of IPv4/IPv6 addresses found */
            check_ip_addr(&g_mdns_results.ring_buffer[rd], &g_mdns_res_a_or_aaaa);
            /* Update ring buffer index */
            MDNS_BUFFER_UPDATE_HEAD_TAIL(rd, 1);
        }

        p_first = g_mdns_res_a_or_aaaa.ip_addr;
        while ( p_first != NULL)
        {
            g_mdns_res_a_or_aaaa.ip_addr = p_first->next;
            p_first->next = NULL;
            app_mdns_free(p_first);
            g_mdns_res_a_or_aaaa.ip_count--;

            p_first = g_mdns_res_a_or_aaaa.ip_addr;
        }
        app_mdns_free(g_mdns_res_a_or_aaaa.target);
        g_mdns_res_a_or_aaaa.target = NULL;

        /* Notify app_notify_event task to process mdns result */
        app_notify_event(APP_EVT_MDNS_SEARCH_RESULT, APP_EVT_REASON_SUCCESS, &g_mdns_results, 0);

        mdns_d("==========END==============");
    }

    if (only_ptr)
    {
        /* Notify app_notify_event task to process mdns result */
        app_notify_event(APP_EVT_MDNS_SEARCH_RESULT, APP_EVT_REASON_SUCCESS, &g_mdns_results, 0);

        mdns_d("==========END==============");
    }
}

int app_mdns_search_service(QUERY_PTR_CFG *query_ptr, enum mdns_ifac_role role)
{
    uint8_t id;
    static info_t info;
    int res                  = -WM_FAIL;
    struct netif *mdns_netif = NULL;

    if (g_request_id != 0xff)
    {
        app_mdns_search_stop(g_request_id);
    }

    mdns_netif = app_get_mdns_service_netif(role);
    if (mdns_netif == NULL)
    {
        return -WM_FAIL;
    }

    if (query_ptr->proto == DNSSD_PROTO_UDP && strcmp(query_ptr->service, "_services._dns-sd") == 0)
        info.only_ptr = 1; /* don't check other answers */
    else
        info.only_ptr = 0;

    res = mdns_search_service(NULL, query_ptr->service, (enum mdns_sd_proto)query_ptr->proto, mdns_netif,
                              app_mdns_search_result, &info, &id);
    if (res < 0)
    {
        mdns_e("no more space in mdns_request table");
    }

    g_request_id = id;

    return res;
}

void app_mdns_search_stop(uint8_t request_id)
{
    mdns_search_stop(request_id);
}

/**
 *  @brief Callback to send ip address received
 *
 *  @param hostname     Domain name
 *  @param ipaddr       IP address resolved
 *  @param arg          Userdata pointer for callback function
 *
 *  @return None
 */
static void app_mdns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg)
{
    /* Address resolved */
    mdns_d("%s: %s\n", hostname, ipaddr ? ipaddr_ntoa(ipaddr) : "<not found>");

    if (ipaddr != NULL)
        app_notify_event(APP_EVT_MDNS_RESOLVE_DOMAIN_NAME, APP_EVT_REASON_SUCCESS, (void *)ipaddr, 0);
    else
        app_notify_event(APP_EVT_MDNS_RESOLVE_DOMAIN_NAME, APP_EVT_REASON_FAILURE, NULL, 0);
}

int app_mdns_query_a(const char *hostname, enum mdns_ifac_role role)
{
    err_t err;

    err = dns_gethostbyname(hostname, &query_a_addr, app_mdns_found, NULL);
    if (err == ERR_INPROGRESS)
    {
        /* DNS request sent, wait for app_mdns_found being called */
        mdns_d("Waiting for server ipv4 address to be resolved.");
    }
    else if (err == ERR_OK)
    {
        app_mdns_found(hostname, &query_a_addr, NULL);
    }

    return WM_SUCCESS;
}
