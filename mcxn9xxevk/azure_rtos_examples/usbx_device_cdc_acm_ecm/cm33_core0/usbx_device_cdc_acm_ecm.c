/* This example illustrates USBX composite device of CDC ACM and CDC ECM */

#include "ux_api.h"
#include "nx_api.h"
#include "ux_device_class_cdc_acm.h"
#include "ux_device_class_cdc_ecm.h"

#include "ux_device_descriptor.h"

#include "fsl_debug_console.h"
#include "board_setup.h"

#define UX_DEMO_STACK_SIZE      (1024 * 2)

#ifndef USBX_MEMORY_SIZE
#define USBX_MEMORY_SIZE        (1024 * 40)
#endif

#define DEMO_IP_ADDRESS             IP_ADDRESS(192,168,18,1)
#define DEMO_NETWORK_MASK           IP_ADDRESS(255,255,255,0)

#define NX_PACKET_POOL_PAYLOAD_SIZE     (1536U)
#define NX_PACKET_POOL_SIZE             (1024U * 40)

#define NX_IP_THREAD_PRIORITY           (1U)
#define DEMO_THREAD_PRIORITY            (20U)
#define DEMO_THREAD_TIME_SLICE          (1U)

#define DEMO_USB_CONFIGURE_NUMBER       (1U)
#define DEMO_USB_ACM_INTERFACE_NUMBER   (0U)
#define DEMO_USB_ECM_INTERFACE_NUMBER   (2U)

static UX_SLAVE_CLASS_CDC_ACM *acm_instance;
static UX_SLAVE_CLASS_CDC_ECM *ecm_instance;

AT_NONCACHEABLE_SECTION_ALIGN(static ULONG usb_memory[USBX_MEMORY_SIZE / sizeof(ULONG)], 64);

static NX_PACKET_POOL pool_0;
static NX_IP ip_0;

AT_NONCACHEABLE_SECTION_ALIGN(static ULONG packet_pool_area[NX_PACKET_POOL_SIZE / sizeof(ULONG)], 64);

static ULONG arp_space_area[1024 / sizeof(ULONG)];
static ULONG ip_thread_stack[2 * 1024 / sizeof(ULONG)];

/* The Ethernet MAC address for the USB CDC-ECM device. */
static UCHAR demo_usb_ecm_mac_address[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};

static TX_THREAD demo_thread;
static ULONG demo_thread_stack[UX_DEMO_STACK_SIZE / sizeof(ULONG)];

static VOID demo_cdc_acm_activate(VOID *cdc_instance)
{
    PRINTF("USB CDC-ACM device is activated.\r\n");
    /* Save the CDC instance.  */
    acm_instance = (UX_SLAVE_CLASS_CDC_ACM *)cdc_instance;
}

static VOID demo_cdc_acm_deactivate(VOID *cdc_instance)
{
    acm_instance = UX_NULL;
    PRINTF("USB CDC-ACM device is deactivated.\r\n");
}

static VOID demo_cdc_ecm_activate(VOID *cdc_instance)
{
    PRINTF("USB CDC-ECM device is activated.\r\n");
    /* Save the CDC instance.  */
    ecm_instance = (UX_SLAVE_CLASS_CDC_ECM *)cdc_instance;
}

static VOID demo_cdc_ecm_deactivate(VOID *cdc_instance)
{
    ecm_instance = UX_NULL;
    PRINTF("USB CDC-ECM device is deactivated.\r\n");
}

static UINT demo_init_network(VOID)
{
    UINT status;

    status = nx_packet_pool_create(&pool_0, "NetX Main Packet Pool",
                                   NX_PACKET_POOL_PAYLOAD_SIZE,
                                   (VOID *)packet_pool_area,
                                   NX_PACKET_POOL_SIZE);
    if (status != NX_SUCCESS)
        return status;

    status = nx_ip_create(&ip_0, "NetX IP Instance 0",
                          DEMO_IP_ADDRESS, DEMO_NETWORK_MASK,
                          &pool_0, _ux_network_driver_entry,
                          (VOID *)ip_thread_stack,
                          sizeof(ip_thread_stack), NX_IP_THREAD_PRIORITY);
    if (status != NX_SUCCESS)
        return status;

    status = nx_ip_fragment_enable(&ip_0);
    if (status != NX_SUCCESS)
        return status;

    status = nx_arp_enable(&ip_0, (void *)arp_space_area, sizeof(arp_space_area));
    if (status != NX_SUCCESS)
        return status;

    status = nx_icmp_enable(&ip_0);
    if (status != NX_SUCCESS)
        return status;

    status = nx_tcp_enable(&ip_0);
    if (status != NX_SUCCESS)
        return status;

    return NX_SUCCESS;
}

static UINT demo_init_usb_acm(VOID)
{
    static UX_SLAVE_CLASS_CDC_ACM_PARAMETER acm_parameter;
    UINT status;

    /* Set the parameters for callback when insertion/extraction of a CDC device.  */
    acm_parameter.ux_slave_class_cdc_acm_instance_activate   = demo_cdc_acm_activate;
    acm_parameter.ux_slave_class_cdc_acm_instance_deactivate = demo_cdc_acm_deactivate;

    /* Initialize the USB device CDC ACM class. */
    status = ux_device_stack_class_register(_ux_system_slave_class_cdc_acm_name,
                                            ux_device_class_cdc_acm_entry,
                                            DEMO_USB_CONFIGURE_NUMBER,
                                            DEMO_USB_ACM_INTERFACE_NUMBER,
                                            &acm_parameter);

    return status;
}

static UINT demo_init_usb_ecm(VOID)
{
    static UX_SLAVE_CLASS_CDC_ECM_PARAMETER ecm_parameter;
    UINT status;

    ecm_parameter.ux_slave_class_cdc_ecm_instance_activate   = demo_cdc_ecm_activate;
    ecm_parameter.ux_slave_class_cdc_ecm_instance_deactivate = demo_cdc_ecm_deactivate;

    /* Define a NODE ID using it as the local MAC address.
     * This is different from the MAC address displayed in the USB descriptor. (iMACAddress)
     */
    for (int i = 0; i < sizeof(demo_usb_ecm_mac_address) / sizeof(UCHAR); i++)
    {
        ecm_parameter.ux_slave_class_cdc_ecm_parameter_local_node_id[i] = demo_usb_ecm_mac_address[i];
    }

    /* Initialize the USB device CDC ECM class. */
    status = ux_device_stack_class_register(_ux_system_slave_class_cdc_ecm_name,
    										ux_device_class_cdc_ecm_entry,
                                            DEMO_USB_CONFIGURE_NUMBER,
                                            DEMO_USB_ECM_INTERFACE_NUMBER,
                                            &ecm_parameter);

    return status;
}

static VOID demo_thread_entry(ULONG arg)
{
    ULONG status;
    ULONG actual_length;
	const char *output_text = "OK\r\n";

    status = ux_device_stack_initialize(ux_get_hs_framework(),
                                        ux_get_hs_framework_length(),
                                        ux_get_fs_framework(),
                                        ux_get_fs_framework_length(),
                                        ux_get_string_framework(),
                                        ux_get_string_framework_length(),
                                        ux_get_language_framework(),
                                        ux_get_language_framework_length(),
                                        UX_NULL);
    if (status != UX_SUCCESS)
        goto err;

    status = demo_init_usb_acm();
    if (status != UX_SUCCESS)
        goto err;

    status = demo_init_usb_ecm();
    if (status != UX_SUCCESS)
        goto err;

    usb_device_setup();

    status = demo_init_network();
    if (status != NX_SUCCESS)
    {
        PRINTF("ERR: network init error!\r\n");
        goto err;
    }

    while (1)
    {
        /* Ensure the CDC class is mounted.  */
        if (acm_instance == UX_NULL) {
            tx_thread_sleep(100);
            continue;
        }

        status = ux_device_class_cdc_acm_write(acm_instance, (UCHAR *)output_text,
                                               strlen(output_text), &actual_length);
        if (status != UX_SUCCESS)
            break;

        /* Send one ZLP (zero-length packet). */
        status = ux_device_class_cdc_acm_write(acm_instance, (UCHAR *)output_text, 0, &actual_length);
        if (status != UX_SUCCESS)
            break;

    	tx_thread_sleep(NX_IP_PERIODIC_RATE);
    }

err:
    PRINTF("ERR: 0x%x\r\n", status);
    while (1)
    {
        tx_thread_sleep(NX_IP_PERIODIC_RATE);
    }
}

int main(void)
{
    /* Initialize the board. */
    board_setup();

    usb_device_hw_setup();

    PRINTF("Start USBX device ACM and ECM example...\r\n");

    /* Enter the ThreadX kernel. */
    tx_kernel_enter();

    return 0;
}

void tx_application_define(void *first_unused_memory)
{
    UINT status;

    UX_PARAMETER_NOT_USED(first_unused_memory);

    /* Initialize USBX Memory */
    status = ux_system_initialize((VOID *)usb_memory, USBX_MEMORY_SIZE, UX_NULL, 0);
    if (status != UX_SUCCESS)
        goto err;

    /* Perform the initialization of the network driver. This will initialize the USBX network layer. */
	status = ux_network_driver_init();
    if (status != UX_SUCCESS)
        goto err;

    /* Create the demo thread.  */
    status = tx_thread_create(&demo_thread, "USBX demo1",
                              demo_thread_entry, 0,
                              demo_thread_stack, UX_DEMO_STACK_SIZE,
                              DEMO_THREAD_PRIORITY, DEMO_THREAD_PRIORITY,
                              DEMO_THREAD_TIME_SLICE, TX_AUTO_START);
    if (status != UX_SUCCESS)
        goto err;

    return;

err:
    PRINTF("tx_application_define: ERROR 0x%x\r\n", status);
    while (1)
    {
    }
}
