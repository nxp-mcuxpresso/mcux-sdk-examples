
#include "ux_api.h"
#include "ux_network_driver.h"
#include "ux_host_class_hub.h"
#include "ux_host_class_cdc_ecm.h"

#include "nx_api.h"
#ifndef SAMPLE_DHCP_DISABLE
#include "nxd_dhcp_client.h"
#endif /* SAMPLE_DHCP_DISABLE */
#include "nxd_dns.h"
#include "nxd_sntp_client.h"
#include "nx_secure_tls_api.h"

#include "fsl_debug_console.h"
#include "board_setup.h"

#ifndef USBX_MEMORY_SIZE
#define USBX_MEMORY_SIZE    (64 * 1024)
#endif

#define DEMO_STACK_SIZE     2048
#define IP_STACK_SIZE       2048
#define ARP_CACHE_SIZE      1024
#define PACKET_PAYLOAD      (1536 + sizeof(NX_PACKET))
#define NX_PACKET_POOL_SIZE (PACKET_PAYLOAD * 20)
#define SAMPLE_IP_THREAD_PRIORITY (1)

#ifdef SAMPLE_DHCP_DISABLE

#ifndef SAMPLE_IPV4_ADDRESS
#define SAMPLE_IPV4_ADDRESS         IP_ADDRESS(192, 168, 100, 10)
#error "SYMBOL SAMPLE_IPV4_ADDRESS must be defined. This symbol specifies the IP address of device. "
#endif /* SAMPLE_IPV4_ADDRESS */

#ifndef SAMPLE_IPV4_MASK
#define SAMPLE_IPV4_MASK            0xFFFFFF00UL
#error "SYMBOL SAMPLE_IPV4_MASK must be defined. This symbol specifies the IP address mask of device. "
#endif /* SAMPLE_IPV4_MASK */

#ifndef SAMPLE_GATEWAY_ADDRESS
#define SAMPLE_GATEWAY_ADDRESS      IP_ADDRESS(192, 168, 100, 1)
#error "SYMBOL SAMPLE_GATEWAY_ADDRESS must be defined. This symbol specifies the gateway address for routing. "
#endif /* SAMPLE_GATEWAY_ADDRESS */

#ifndef SAMPLE_DNS_SERVER_ADDRESS
#define SAMPLE_DNS_SERVER_ADDRESS   IP_ADDRESS(192, 168, 100, 1)
#error "SYMBOL SAMPLE_DNS_SERVER_ADDRESS must be defined. This symbol specifies the dns server address for routing. "
#endif /* SAMPLE_DNS_SERVER_ADDRESS */
#else

#define SAMPLE_IPV4_ADDRESS IP_ADDRESS(0, 0, 0, 0)
#define SAMPLE_IPV4_MASK    IP_ADDRESS(0, 0, 0, 0)

#endif /* SAMPLE_DHCP_DISABLE */

/* Using SNTP to get unix time.  */
/* Define the address of SNTP Server. If not defined, use DNS module to resolve the host name SAMPLE_SNTP_SERVER_NAME.
 */
/*
#define SAMPLE_SNTP_SERVER_ADDRESS      IP_ADDRESS(118, 190, 21, 209)
*/

#ifndef SAMPLE_SNTP_SYNC_MAX
#define SAMPLE_SNTP_SYNC_MAX 2
#endif /* SAMPLE_SNTP_SYNC_MAX */

#ifndef SAMPLE_SNTP_UPDATE_MAX
#define SAMPLE_SNTP_UPDATE_MAX 10
#endif /* SAMPLE_SNTP_UPDATE_MAX */

#ifndef SAMPLE_SNTP_UPDATE_INTERVAL
#define SAMPLE_SNTP_UPDATE_INTERVAL (NX_IP_PERIODIC_RATE / 2)
#endif /* SAMPLE_SNTP_UPDATE_INTERVAL */

/* Default time. GMT: Thu Mar 10 10:44:35 2022.  */
#ifndef SAMPLE_SYSTEM_TIME
#define SAMPLE_SYSTEM_TIME                1646909075
#endif /* SAMPLE_SYSTEM_TIME  */

/* Seconds between Unix Epoch (1/1/1970) and NTP Epoch (1/1/1999) */
#define SAMPLE_UNIX_TO_NTP_EPOCH_SECOND 0x83AA7E80


static ULONG arp_cache_area[ARP_CACHE_SIZE / sizeof(ULONG)];

static ULONG ip_thread_stack[IP_STACK_SIZE / sizeof(ULONG)];
static ULONG demo_thread_stack[DEMO_STACK_SIZE / sizeof(ULONG)];

static ULONG usb_memory[USBX_MEMORY_SIZE / sizeof(ULONG)];

AT_NONCACHEABLE_SECTION_ALIGN(ULONG packet_pool_area[NX_PACKET_POOL_SIZE / sizeof(ULONG)], 64);

/* Define the ThreadX object control blocks...  */
static TX_THREAD        demo_thread;
static NX_PACKET_POOL   packet_pool;
static NX_IP            nx_ip;
static NX_DNS           dns_0;

#ifndef SAMPLE_DHCP_DISABLE
NX_DHCP dhcp_0;
#endif /* SAMPLE_DHCP_DISABLE  */

static NX_SNTP_CLIENT sntp_client;

/* System clock time for UTC.  */
static ULONG unix_time_base;

static const CHAR *sntp_servers[] = {
    "0.pool.ntp.org",
    "1.pool.ntp.org",
    "2.pool.ntp.org",
    "3.pool.ntp.org",
};
static UINT sntp_server_index;

/* Include the sample.  */
extern VOID sample_entry(NX_IP *ip_ptr,
                         NX_PACKET_POOL *pool_ptr,
                         NX_DNS *dns_ptr,
                         UINT (*unix_time_callback)(ULONG *unix_time));

extern uint32_t get_seed(void);

#ifndef SAMPLE_DHCP_DISABLE
static UINT dhcp_wait(NX_IP *nx_ip_ptr)
{
    ULONG actual_status;
    UINT status;

    PRINTF("DHCP In Progress...\r\n");

    /* Create the DHCP instance.  */
    status = nx_dhcp_create(&dhcp_0, nx_ip_ptr, "DHCP Client");
    if (status != NX_SUCCESS)
        goto err;

    /* Start the DHCP Client.  */
    status = nx_dhcp_start(&dhcp_0);
    if (status != NX_SUCCESS)
        goto err;

    /* Wait until address is solved. */
    nx_ip_status_check(nx_ip_ptr, NX_IP_ADDRESS_RESOLVED, &actual_status, NX_WAIT_FOREVER);

    return NX_SUCCESS;

err:
    return status;
}
#endif /* SAMPLE_DHCP_DISABLE  */

static UINT usbx_host_change_callback(ULONG event, UX_HOST_CLASS *host_class, VOID *instance)
{
    UX_DEVICE *device;

    /* Check if there is a device connection event, make sure the instance is valid.  */
    if ((event == UX_DEVICE_CONNECTION) && (instance != UX_NULL))
    {
        /* Get the device instance.  */
        device = (UX_DEVICE *)instance;

        PRINTF("USB device: vid=0x%x, pid=0x%x\r\n", device->ux_device_descriptor.idVendor,
               device->ux_device_descriptor.idProduct);

        /* Check if the device is configured.  */
        if (device->ux_device_state != UX_DEVICE_CONFIGURED)
        {
            /* Not configured. Check if there is another configuration.  */
            if ((device->ux_device_first_configuration != UX_NULL) &&
                (device->ux_device_first_configuration->ux_configuration_next_configuration != UX_NULL))
            {
                /* Try the second configuration.  */
                ux_host_stack_device_configuration_activate(
                    device->ux_device_first_configuration->ux_configuration_next_configuration);
            }
        }
    }

    return (UX_SUCCESS);
}

UINT class_cdc_ecm_get(void)
{
    UX_HOST_CLASS_CDC_ECM *cdc_ecm;
    UX_HOST_CLASS *class;
    UINT status;

    /* Find the main cdc_ecm container */
    status = ux_host_stack_class_get(_ux_system_host_class_cdc_ecm_name, &class);
    if (status != UX_SUCCESS)
        return (status);

    /* We get the first instance of the cdc_ecm device */
    do
    {
        status = ux_host_stack_class_instance_get(class, 0, (void **)&cdc_ecm);
        tx_thread_sleep(10);
    } while (status != UX_SUCCESS);

    /* We still need to wait for the cdc_ecm status to be live */
    while (cdc_ecm->ux_host_class_cdc_ecm_state != UX_HOST_CLASS_INSTANCE_LIVE)
        tx_thread_sleep(100);

    /* Now wait for the link to be up.  */
    while (cdc_ecm->ux_host_class_cdc_ecm_link_state != UX_HOST_CLASS_CDC_ECM_LINK_STATE_UP)
        tx_thread_sleep(100);

    /* Return successful completion.  */
    return (UX_SUCCESS);
}

static UINT dns_create()
{
    UINT status;
    ULONG dns_server_address[3];
    UINT dns_server_address_size = 12;

    /* Create a DNS instance for the Client.  Note this function will create
       the DNS Client packet pool for creating DNS message packets intended
       for querying its DNS server. */
    status = nx_dns_create(&dns_0, &nx_ip, (UCHAR *)"DNS Client");
    if (status != NX_SUCCESS)
        goto err;

    /* Is the DNS client configured for the host application to create the pecket pool? */
#ifdef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL

    /* Yes, use the packet pool created above which has appropriate payload size
       for DNS messages. */
    status = nx_dns_packet_pool_set(&dns_0, nx_ip.nx_ip_default_packet_pool);
    if (status != NX_SUCCESS)
    {
        nx_dns_delete(&dns_0);
        goto err;
    }
#endif /* NX_DNS_CLIENT_USER_CREATE_PACKET_POOL */

#ifndef SAMPLE_DHCP_DISABLE
    /* Retrieve DNS server address.  */
    nx_dhcp_interface_user_option_retrieve(&dhcp_0, 0, NX_DHCP_OPTION_DNS_SVR, (UCHAR *)(dns_server_address),
                                           &dns_server_address_size);
#else
    dns_server_address[0] = SAMPLE_DNS_SERVER_ADDRESS;
#endif /* SAMPLE_DHCP_DISABLE */

    /* Add an IPv4 server address to the Client list. */
    status = nx_dns_server_add(&dns_0, dns_server_address[0]);
    if (status != NX_SUCCESS)
    {
        nx_dns_delete(&dns_0);
        goto err;
    }

    /* Output DNS Server address.  */
    PRINTF("DNS Server address: %lu.%lu.%lu.%lu\r\n", (dns_server_address[0] >> 24),
           (dns_server_address[0] >> 16 & 0xFF), (dns_server_address[0] >> 8 & 0xFF),
           (dns_server_address[0] & 0xFF));

    return (NX_SUCCESS);

err:
    return status;
}

/* Sync up the local time.  */
static UINT sntp_time_sync()
{
    UINT status;
    UINT server_status;
    ULONG sntp_server_address;
    UINT i;

#ifndef SAMPLE_SNTP_SERVER_ADDRESS
    PRINTF("SNTP Time Sync...%s\r\n", sntp_servers[sntp_server_index]);

    /* Look up SNTP Server address. */
    status = nx_dns_host_by_name_get(&dns_0, (UCHAR *)sntp_servers[sntp_server_index], &sntp_server_address,
                                     5 * NX_IP_PERIODIC_RATE);

    /* Check status.  */
    if (status)
    {
        return (status);
    }
#else  /* !SAMPLE_SNTP_SERVER_ADDRESS */
    PRINTF("SNTP Time Sync...\r\n");
    sntp_server_address = SAMPLE_SNTP_SERVER_ADDRESS;
#endif /* SAMPLE_SNTP_SERVER_ADDRESS */

    /* Create the SNTP Client to run in broadcast mode.. */
    status = nx_sntp_client_create(&sntp_client, &nx_ip, 0, &packet_pool, NX_NULL, NX_NULL,
                                   NX_NULL /* no random_number_generator callback */);

    /* Check status.  */
    if (status)
    {
        return (status);
    }

    /* Use the IPv4 service to initialize the Client and set the IPv4 SNTP server. */
    status = nx_sntp_client_initialize_unicast(&sntp_client, sntp_server_address);

    /* Check status.  */
    if (status)
    {
        nx_sntp_client_delete(&sntp_client);
        return (status);
    }

    /* Set local time to 0 */
    status = nx_sntp_client_set_local_time(&sntp_client, 0, 0);

    /* Check status.  */
    if (status)
    {
        nx_sntp_client_delete(&sntp_client);
        return (status);
    }

    /* Run unicast client */
    status = nx_sntp_client_run_unicast(&sntp_client);

    /* Check status.  */
    if (status)
    {
        nx_sntp_client_stop(&sntp_client);
        nx_sntp_client_delete(&sntp_client);
        return (status);
    }

    /* Wait till updates are received */
    for (i = 0; i < SAMPLE_SNTP_UPDATE_MAX; i++)
    {
        /* First verify we have a valid SNTP service running. */
        status = nx_sntp_client_receiving_updates(&sntp_client, &server_status);

        /* Check status.  */
        if ((status == NX_SUCCESS) && (server_status == NX_TRUE))
        {
            /* Server status is good. Now get the Client local time. */
            ULONG sntp_seconds, sntp_fraction;
            ULONG system_time_in_second;

            /* Get the local time.  */
            status = nx_sntp_client_get_local_time(&sntp_client, &sntp_seconds, &sntp_fraction, NX_NULL);

            /* Check status.  */
            if (status != NX_SUCCESS)
            {
                continue;
            }

            /* Get the system time in second.  */
            system_time_in_second = tx_time_get() / TX_TIMER_TICKS_PER_SECOND;

            /* Convert to Unix epoch and minus the current system time.  */
            unix_time_base = (sntp_seconds - (system_time_in_second + SAMPLE_UNIX_TO_NTP_EPOCH_SECOND));

            /* Time sync successfully.  */

            /* Stop and delete SNTP.  */
            nx_sntp_client_stop(&sntp_client);
            nx_sntp_client_delete(&sntp_client);

            return (NX_SUCCESS);
        }

        /* Sleep.  */
        tx_thread_sleep(SAMPLE_SNTP_UPDATE_INTERVAL);
    }

    /* Time sync failed.  */

    /* Stop and delete SNTP.  */
    nx_sntp_client_stop(&sntp_client);
    nx_sntp_client_delete(&sntp_client);

    /* Return success.  */
    return (NX_NOT_SUCCESSFUL);
}

static UINT unix_time_get(ULONG *unix_time)
{
    /* Return number of seconds since Unix Epoch (1/1/1970 00:00:00).  */
    *unix_time = unix_time_base + (tx_time_get() / TX_TIMER_TICKS_PER_SECOND);

    return (NX_SUCCESS);
}

/* the main thread */
static void main_thread_entry(ULONG thread_input)
{
    UINT status;
    ULONG ip_address      = 0;
    ULONG network_mask    = 0;
    ULONG gateway_address = 0;

    /* Find the CDC-ECM class. */
    status =  class_cdc_ecm_get();
    if (status != UX_SUCCESS)
    {
        PRINTF("Can not find a USB CDC ECM device: %u\r\n", status);
        return;
    }

#ifndef SAMPLE_DHCP_DISABLE
    dhcp_wait(&nx_ip);
#else
    nx_ip_gateway_address_set(&nx_ip, IP_ADDRESS(192, 168, 1, 1));
#endif /* SAMPLE_DHCP_DISABLE  */

    /* Get IP address and gateway address. */
    nx_ip_address_get(&nx_ip, &ip_address, &network_mask);
    nx_ip_gateway_address_get(&nx_ip, &gateway_address);

    /* Output IP address and gateway address. */
    PRINTF("IP address: %lu.%lu.%lu.%lu\r\n", (ip_address >> 24), (ip_address >> 16 & 0xFF), (ip_address >> 8 & 0xFF),
           (ip_address & 0xFF));
    PRINTF("Mask: %lu.%lu.%lu.%lu\r\n", (network_mask >> 24), (network_mask >> 16 & 0xFF), (network_mask >> 8 & 0xFF),
           (network_mask & 0xFF));
    PRINTF("Gateway: %lu.%lu.%lu.%lu\r\n", (gateway_address >> 24), (gateway_address >> 16 & 0xFF),
           (gateway_address >> 8 & 0xFF), (gateway_address & 0xFF));

    /* Create DNS.  */
    status = dns_create();
    if (status != NX_SUCCESS)
    {
        PRINTF("dns_create fail: %u\r\n", status);
        return;
    }

    /* Sync up time by SNTP at start up.  */
    for (UINT i = 0; i < SAMPLE_SNTP_SYNC_MAX; i++)
    {
        /* Start SNTP to sync the local time.  */
        status = sntp_time_sync();

        /* Check status.  */
        if (status == NX_SUCCESS)
            break;

        /* Switch SNTP server every time.  */
        sntp_server_index = (sntp_server_index + 1) % (sizeof(sntp_servers) / sizeof(sntp_servers[0]));
    }

    /* Check status.  */
    if (status)
    {
        PRINTF("SNTP Time Sync failed.\r\n");
        PRINTF("Set Time to default value: SAMPLE_SYSTEM_TIME.\r\n");
        unix_time_base = SAMPLE_SYSTEM_TIME;
    }
    else
    {
        PRINTF("SNTP Time Sync successfully.\r\n");
    }

    srand(get_seed());

    /* Start azure iot embedded sdk sample.  */
    sample_entry(&nx_ip, &packet_pool, &dns_0, unix_time_get);

    /* This thread simply sits in while-forever-sleep loop.  */
    while (1)
    {
        /* Sleep for 10 ticks.  */
        tx_thread_sleep(10);
    }
}

int main(void)
{
    /* Initialize the board. */
    board_setup();

    PRINTF("Start the azure_iot_embedded_sdk example...\r\n");

    /* Enter the ThreadX kernel. */
    tx_kernel_enter();

    return 0;
}

void tx_application_define(void *first_unused_memory)
{
    UINT status;

    NX_PARAMETER_NOT_USED(first_unused_memory);

    usb_host_setup();

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Initialize USBX. */
    status = ux_system_initialize(usb_memory, USBX_MEMORY_SIZE, UX_NULL, 0);
    if (status != UX_SUCCESS)
        goto err;

    /* The code below is required for installing the host portion of USBX */
    status = ux_host_stack_initialize(usbx_host_change_callback);
    if (status != NX_SUCCESS)
        goto err;

    /* Register the HUB class. */
    status = ux_host_stack_class_register(_ux_system_host_class_hub_name,
                                          _ux_host_class_hub_entry);
    if (status != NX_SUCCESS)
        goto err;

    /* Register cdc_ecm class. */
    status = ux_host_stack_class_register(_ux_system_host_class_cdc_ecm_name,
                                          ux_host_class_cdc_ecm_entry);
    if (status != NX_SUCCESS)
        goto err;

    /* Register the platform-specific USB controller. */
    status = ux_host_stack_hcd_register((UCHAR *)UX_HCD_NAME, UX_HCD_INIT_FUNC,
                                        usb_host_base(), 0);
    if (status != NX_SUCCESS)
        goto err;

    usb_host_interrupt_setup();

    /* Perform the initialization of the network driver. */
    status = ux_network_driver_init();
    if (status != UX_SUCCESS)
        goto err;

    /* Create a packet pool.  */
    status = nx_packet_pool_create(&packet_pool, "NetX Main Packet Pool",
                              PACKET_PAYLOAD, packet_pool_area, NX_PACKET_POOL_SIZE);
    if (status != NX_SUCCESS)
        goto err;

    /* Create an IP instance.  */
    status = nx_ip_create(&nx_ip, "NetX IP Instance 0", SAMPLE_IPV4_ADDRESS,
                          SAMPLE_IPV4_MASK, &packet_pool,
                          _ux_network_driver_entry, (UCHAR *)ip_thread_stack,
                          IP_STACK_SIZE, SAMPLE_IP_THREAD_PRIORITY);
    if (status != NX_SUCCESS)
        goto err;

    /* Enable packet fragmentation.  */
    status = nx_ip_fragment_enable(&nx_ip);
    /* Check for pool creation error.  */
    if (status != NX_SUCCESS)
        goto err;

    /* Enable ARP and supply ARP cache memory for IP Instance 0.  */
    status = nx_arp_enable(&nx_ip, (void *)arp_cache_area, ARP_CACHE_SIZE);
    /* Check for ARP enable errors.  */
    if (status != NX_SUCCESS)
        goto err;

    /* Enable TCP traffic.  */
    status = nx_tcp_enable(&nx_ip);
    /* Check for TCP enable errors.  */
    if (status != NX_SUCCESS)
        goto err;

    /* Enable UDP traffic.  */
    status = nx_udp_enable(&nx_ip);
    /* Check for UDP enable errors.  */
    if (status != NX_SUCCESS)
        goto err;

    /* Enable ICMP */
    status = nx_icmp_enable(&nx_ip);
    /* Check for ICMP enable errors.  */
    if (status != NX_SUCCESS)
        goto err;

    /* Create the main thread.  */
    tx_thread_create(&demo_thread, "main thread", main_thread_entry, 0,
                     demo_thread_stack, DEMO_STACK_SIZE, 20, 20, TX_NO_TIME_SLICE,
                     TX_AUTO_START);

    return;

err:
    PRINTF("ERROR: 0x%x\r\n", status);

    while (1)
    {
        tx_thread_sleep(100);
    }
}
