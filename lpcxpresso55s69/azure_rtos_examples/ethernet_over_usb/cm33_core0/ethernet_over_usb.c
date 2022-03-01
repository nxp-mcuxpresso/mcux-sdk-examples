
#include "board_setup.h"
#include "fsl_debug_console.h"

#include "tx_api.h"
#include "nx_api.h"
#include "ux_api.h"
#include "ux_network_driver.h"
#include "ux_host_class_hub.h"
#include "ux_host_class_cdc_ecm.h"

#ifndef SAMPLE_DHCP_DISABLE
#include "nxd_dhcp_client.h"
#endif /* SAMPLE_DHCP_DISABLE */

#define PACKET_PAYLOAD      (1536 + sizeof(NX_PACKET))
#define NX_PACKET_POOL_SIZE (PACKET_PAYLOAD * 32)

#define DEMO_STACK_SIZE  (1024 * 4)
#define IP_STACK_SIZE    2048
#define ARP_CACHE_SIZE   1024
#define HTTP_STACK_SIZE  2048
#define IPERF_STACK_SIZE 2048

#define SAMPLE_IP_THREAD_PRIORITY (1)

#ifdef SAMPLE_DHCP_DISABLE

#ifndef SAMPLE_IPV4_ADDRESS
#define SAMPLE_IPV4_ADDRESS IP_ADDRESS(192, 168, 100, 10)
#error \
    "SYMBOL SAMPLE_IPV4_ADDRESS must be defined. This symbol specifies the IP address of device. "
#endif /* SAMPLE_IPV4_ADDRESS */

#ifndef SAMPLE_IPV4_MASK
#define SAMPLE_IPV4_MASK 0xFFFFFF00UL
#error \
    "SYMBOL SAMPLE_IPV4_MASK must be defined. This symbol specifies the IP address mask of device. "
#endif /* SAMPLE_IPV4_MASK */

#ifndef SAMPLE_GATEWAY_ADDRESS
#define SAMPLE_GATEWAY_ADDRESS IP_ADDRESS(192, 168, 100, 1)
#error \
    "SYMBOL SAMPLE_GATEWAY_ADDRESS must be defined. This symbol specifies the gateway address for routing. "
#endif /* SAMPLE_GATEWAY_ADDRESS */

#ifndef SAMPLE_DNS_SERVER_ADDRESS
#define SAMPLE_DNS_SERVER_ADDRESS IP_ADDRESS(192, 168, 100, 1)
#error \
    "SYMBOL SAMPLE_DNS_SERVER_ADDRESS must be defined. This symbol specifies the dns server address for routing. "
#endif /* SAMPLE_DNS_SERVER_ADDRESS */
#else

#define SAMPLE_IPV4_ADDRESS IP_ADDRESS(0, 0, 0, 0)
#define SAMPLE_IPV4_MASK    IP_ADDRESS(0, 0, 0, 0)

#endif /* SAMPLE_DHCP_DISABLE */

/* Define the ThreadX object control blocks...  */
static TX_THREAD thread_usb;

static NX_PACKET_POOL packet_pool;
static NX_IP nx_ip;
#ifndef SAMPLE_DHCP_DISABLE
AT_NONCACHEABLE_SECTION_ALIGN(NX_DHCP dhcp_0, 64);
#endif /* SAMPLE_DHCP_DISABLE  */

static ULONG thread_stack[DEMO_STACK_SIZE / sizeof(ULONG)];
static ULONG ip_thread_stack[IP_STACK_SIZE / sizeof(ULONG)];
static ULONG http_stack[HTTP_STACK_SIZE / sizeof(ULONG)];
static ULONG iperf_stack[IPERF_STACK_SIZE / sizeof(ULONG)];

static ULONG arp_cache_area[ARP_CACHE_SIZE / sizeof(ULONG)];

AT_NONCACHEABLE_SECTION_ALIGN(
    ULONG pool_memory[NX_PACKET_POOL_SIZE / sizeof(ULONG)], 64);

extern void nx_iperf_entry(NX_PACKET_POOL *pool_ptr,
                           NX_IP *ip_ptr,
                           UCHAR *http_stack,
                           ULONG http_stack_size,
                           UCHAR *iperf_stack,
                           ULONG iperf_stack_size);

#ifndef SAMPLE_DHCP_DISABLE
static void dhcp_wait()
{
    ULONG actual_status;

    PRINTF("DHCP In Progress...\r\n");

    /* Create the DHCP instance.  */
    nx_dhcp_create(&dhcp_0, &nx_ip, "DHCP Client");

    /* Start the DHCP Client.  */
    nx_dhcp_start(&dhcp_0);

    /* Wait until address is solved. */
    nx_ip_status_check(&nx_ip, NX_IP_ADDRESS_RESOLVED, &actual_status,
                       NX_WAIT_FOREVER);
}
#endif /* SAMPLE_DHCP_DISABLE  */

static UINT usbx_host_change_callback(ULONG event,
                                      UX_HOST_CLASS *host_class,
                                      VOID *instance)
{
    UX_DEVICE *device;

    /* Check if there is a device connection event, make sure the instance is
     * valid.  */
    if ((event == UX_DEVICE_CONNECTION) && (instance != UX_NULL))
    {
        /* Get the device instance.  */
        device = (UX_DEVICE *)instance;

        PRINTF("USB device: vid=0x%x, pid=0x%x\r\n",
               device->ux_device_descriptor.idVendor,
               device->ux_device_descriptor.idProduct);

        /* Check if the device is configured.  */
        if (device->ux_device_state != UX_DEVICE_CONFIGURED)
        {
            /* Not configured. Check if there is another configuration.  */
            if ((device->ux_device_first_configuration != UX_NULL) &&
                (device->ux_device_first_configuration
                     ->ux_configuration_next_configuration != UX_NULL))
            {
                /* Try the second configuration.  */
                ux_host_stack_device_configuration_activate(
                    device->ux_device_first_configuration
                        ->ux_configuration_next_configuration);
            }
        }
    }

    return (UX_SUCCESS);
}

UINT class_cdc_ecm_get(void)
{
    UINT status;
    UX_HOST_CLASS *class;
    UX_HOST_CLASS_CDC_ECM *cdc_ecm;

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
    while (cdc_ecm->ux_host_class_cdc_ecm_link_state !=
           UX_HOST_CLASS_CDC_ECM_LINK_STATE_UP)
        tx_thread_sleep(100);

    /* Return successful completion.  */
    return (UX_SUCCESS);
}

static VOID main_thread_entry(ULONG thread_input)
{
    ULONG ip_address      = 0;
    ULONG network_mask    = 0;
    ULONG gateway_address = 0;

    /* Find the CDC-ECM class. */
    class_cdc_ecm_get();

#ifndef SAMPLE_DHCP_DISABLE
    dhcp_wait();
#else
    nx_ip_gateway_address_set(&nx_ip, IP_ADDRESS(192, 168, 1, 1));
#endif /* SAMPLE_DHCP_DISABLE  */

    /* Get IP address and gateway address. */
    nx_ip_address_get(&nx_ip, &ip_address, &network_mask);
    nx_ip_gateway_address_get(&nx_ip, &gateway_address);

    /* Output IP address and gateway address. */
    PRINTF("IP address: %lu.%lu.%lu.%lu\r\n", (ip_address >> 24),
           (ip_address >> 16 & 0xFF), (ip_address >> 8 & 0xFF),
           (ip_address & 0xFF));
    PRINTF("Mask: %lu.%lu.%lu.%lu\r\n", (network_mask >> 24),
           (network_mask >> 16 & 0xFF), (network_mask >> 8 & 0xFF),
           (network_mask & 0xFF));
    PRINTF("Gateway: %lu.%lu.%lu.%lu\r\n", (gateway_address >> 24),
           (gateway_address >> 16 & 0xFF), (gateway_address >> 8 & 0xFF),
           (gateway_address & 0xFF));

    /* Call entry function to start iperf test.  */
    nx_iperf_entry(&packet_pool, &nx_ip, (UCHAR *)http_stack, HTTP_STACK_SIZE,
                   (UCHAR *)iperf_stack, IPERF_STACK_SIZE);

    return;
}

int main(void)
{
    /* Initialize the board. */
    board_setup();

    usb_host_hw_setup();

    PRINTF("Start the USBX Ethernet Over USB example...\r\n");

    /* Enter the ThreadX kernel. */
    tx_kernel_enter();

    return 0;
}

/* Define what the initial system looks like.  */
void tx_application_define(void *first_unused_memory)
{
    UINT status;

    NX_PARAMETER_NOT_USED(first_unused_memory);

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Initialize USBX memory. */
    usbx_mem_init();

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

    status = usbx_host_hcd_register();
    if (status != UX_SUCCESS)
        goto err;

    /* Perform the initialization of the network driver. */
    status = ux_network_driver_init();
    if (status != UX_SUCCESS)
        goto err;

    /* Create a packet pool.  */
    status = nx_packet_pool_create(&packet_pool, "NetX Main Packet Pool",
                              PACKET_PAYLOAD, pool_memory, NX_PACKET_POOL_SIZE);
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
    tx_thread_create(&thread_usb, "main thread", main_thread_entry, 0,
                     thread_stack, sizeof(thread_stack), 1, 1, TX_NO_TIME_SLICE,
                     TX_AUTO_START);

    return;

err:
    PRINTF("ERROR: 0x%x\r\n", status);

    while (1)
    {
        tx_thread_sleep(100);
    }
}