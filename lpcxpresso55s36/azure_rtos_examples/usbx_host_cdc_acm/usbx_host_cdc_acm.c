/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** This demo will show the host side of USBX. It expects a CDC-ACM modem */
/** device to be connected to the USB port.                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "ux_api.h"
#include "ux_system.h"
#include "ux_utility.h"
#include "ux_host_class_hub.h"
#include "ux_host_class_cdc_acm.h"

#include "fsl_debug_console.h"
#include "board_setup.h"


#define DEMO_STACK_SIZE (2 * 1024)

#define UX_DEMO_RECEPTION_BUFFER_SIZE 512
#define UX_DEMO_XMIT_BUFFER_SIZE      512
#define UX_DEMO_RECEPTION_BLOCK_SIZE  64

/* Define global data structures.  */
TX_THREAD demo_thread;

ULONG command_received_count;
ULONG cdc_acm_reception_buffer[UX_DEMO_RECEPTION_BUFFER_SIZE / sizeof(ULONG)];

UX_HOST_CLASS_CDC_ACM_RECEPTION cdc_acm_reception;

UCHAR *global_reception_buffer;
ULONG global_reception_size;

ULONG demo_stack[DEMO_STACK_SIZE / sizeof(ULONG)];

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

static UINT demo_class_cdc_acm_get(UX_HOST_CLASS_CDC_ACM **instance)
{
    UX_INTERFACE_DESCRIPTOR *interface_desc;
    UX_HOST_CLASS_CDC_ACM *cdc_acm;
    UX_HOST_CLASS *class;
    UINT status;

    *instance = NULL;

    /* Find the main cdc_acm container.  */
    status = ux_host_stack_class_get(_ux_system_host_class_cdc_acm_name, &class);
    if (status != UX_SUCCESS)
        return (status);

    /* We get the current instance of the cdc_acm device.  */
    do
    {
        status = ux_host_stack_class_instance_get(class, 0, (VOID **)&cdc_acm);
        tx_thread_sleep(10);
    } while (status != UX_SUCCESS);

    /* We still need to wait for the cdc_acm status to be live.  */
    while (cdc_acm->ux_host_class_cdc_acm_state != UX_HOST_CLASS_INSTANCE_LIVE)
        tx_thread_sleep(10);

    /* Find the CDC ACM interface */
    while(cdc_acm) {

        interface_desc = &cdc_acm->ux_host_class_cdc_acm_interface->ux_interface_descriptor;

        if (interface_desc->bInterfaceClass == UX_HOST_CLASS_CDC_DATA_CLASS) {
            /* Return successful completion.  */
            *instance = cdc_acm;
            return (UX_SUCCESS);
        }

        cdc_acm = cdc_acm->ux_host_class_cdc_acm_next_instance;
    }

    return (UX_ERROR);
}

static void demo_thread_entry(ULONG arg)
{
    UX_HOST_CLASS_CDC_ACM *cdc_acm;
    ULONG actual_length;
    UINT status;
    UCHAR buffer[256];
    UINT i;

    /* The code below is required for installing the host portion of USBX */
    status = ux_host_stack_initialize(usbx_host_change_callback);
    if (status != UX_SUCCESS)
        goto err;

    /* Register the HUB class. */
    status = ux_host_stack_class_register(_ux_system_host_class_hub_name, _ux_host_class_hub_entry);
    if (status != UX_SUCCESS)
        goto err;

    /* Register CDC-ACM class.  */
    status = ux_host_stack_class_register(_ux_system_host_class_cdc_acm_name, ux_host_class_cdc_acm_entry);
    if (status != UX_SUCCESS)
        goto err;

    status = usbx_host_hcd_register();
    if (status != UX_SUCCESS)
        goto err;

    while (1) {

        /* Find the cdc_acm class and wait for the link to be up.  */
        status = demo_class_cdc_acm_get(&cdc_acm);
        if (status != UX_SUCCESS)
            goto err;

        i = 0;
        buffer[i++] = 'A';
        buffer[i++] = 0;

        PRINTF("SEND: %s\r\n", buffer);

        /* Send one character */
        status = ux_host_class_cdc_acm_write(cdc_acm, buffer, 1, &actual_length);
        if (status != UX_SUCCESS)
            continue;

        PRINTF("RECV: ");

        do {

            status = ux_host_class_cdc_acm_read(cdc_acm, buffer, sizeof(buffer) - 1, &actual_length);
            if (status != UX_SUCCESS)
                break;

            if (actual_length != 0)
            {
                buffer[actual_length] = 0;
                PRINTF("%s", buffer);
            }

        } while (actual_length != 0);

        PRINTF("\r\n");

        tx_thread_sleep(2 * TX_TIMER_TICKS_PER_SECOND);
    }

err:
    PRINTF("ERROR: 0x%x\r\n", status);

    while (1)
    {
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
    }
}

int main(void)
{
    /* Initialize the board. */
    board_setup();

    usb_host_hw_setup();

    PRINTF("Start the USBX HOST CDC ACM example...\r\n");

    /* Enter the ThreadX kernel. */
    tx_kernel_enter();

    return 0;
}

void tx_application_define(void *first_unused_memory)
{
    TX_THREAD_NOT_USED(first_unused_memory);

    /* Initialize USBX memory. */
    usbx_mem_init();

    /* Create the main demo thread.  */
    tx_thread_create(&demo_thread, "main thread", demo_thread_entry, 0, (VOID *)demo_stack, DEMO_STACK_SIZE, 20, 20, 1,
                     TX_AUTO_START);
}
