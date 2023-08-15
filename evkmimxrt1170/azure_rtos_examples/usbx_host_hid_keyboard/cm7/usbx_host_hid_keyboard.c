/* This example illustrates USBX Host HID keyboard. */

#include "ux_api.h"
#include "ux_host_class_hub.h"
#include "ux_host_class_hid.h"
#include "ux_host_class_hid_keyboard.h"

#include "fsl_debug_console.h"
#include "board_setup.h"

/* Define constants. */
#define DEMO_STACK_SIZE (4 * 1024)

/* Define global variables. */
TX_THREAD demo_thread;

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

static UINT demo_class_hid_get(UX_HOST_CLASS_HID **hid_ptr)
{
    UX_HOST_CLASS_HID *hid;
    UX_HOST_CLASS *class;
    UINT status;

    /* Find the main HID container */
    status = ux_host_stack_class_get(_ux_system_host_class_hid_name, &class);
    if (status != UX_SUCCESS)
        return (status);

    /* We get the first instance of the hid device */
    do
    {
        status = ux_host_stack_class_instance_get(class, 0, (void **)&hid);
        tx_thread_sleep(10);
    } while (status != UX_SUCCESS);

    /* We still need to wait for the hid status to be live */
    while (hid->ux_host_class_hid_state != UX_HOST_CLASS_INSTANCE_LIVE)
        tx_thread_sleep(10);

    *hid_ptr = hid;

    return (UX_SUCCESS);
}

static void demo_thread_entry(ULONG arg)
{
    UX_HOST_CLASS_HID *hid;
    UX_HOST_CLASS_HID_CLIENT *hid_client;
    UX_HOST_CLASS_HID_KEYBOARD *keyboard;
    ULONG keyboard_char;
    ULONG keyboard_state;
    UINT status;

    /* Find the HID class */
    status = demo_class_hid_get(&hid);
    if (status != UX_SUCCESS)
        goto err;

    /* Get the HID client */
    hid_client = hid->ux_host_class_hid_client;

    /* Check if the instance of the keyboard is live */
    while (hid_client->ux_host_class_hid_client_local_instance == UX_NULL)
        tx_thread_sleep(10);

    /* Get the keyboard instance */
    keyboard = (UX_HOST_CLASS_HID_KEYBOARD *)hid_client->ux_host_class_hid_client_local_instance;

    while (1)
    {
        /* Get a key/state from the keyboard.  */
        status = ux_host_class_hid_keyboard_key_get(keyboard, &keyboard_char, &keyboard_state);
        if (status == UX_SUCCESS)
        {
            PRINTF("Input: %c\r\n", (char)keyboard_char);
        }

        tx_thread_sleep(10);
    }

err:
    PRINTF("ERROR: 0x%x\r\n", status);

    while (1)
    {
        tx_thread_sleep(100);
    }
}

void tx_application_define(void *first_unused_memory)
{
    UINT status;

    TX_THREAD_NOT_USED(first_unused_memory);

    /* Initialize USBX memory. */
    usbx_mem_init();

    /* The code below is required for installing the host portion of USBX */
    status = ux_host_stack_initialize(usbx_host_change_callback);
    if (status != UX_SUCCESS)
        goto err;

    /* Register the HUB class. */
    status = ux_host_stack_class_register(_ux_system_host_class_hub_name, ux_host_class_hub_entry);
    if (status != UX_SUCCESS)
        goto err;

    /* Register the hid class.  */
    status = ux_host_stack_class_register(_ux_system_host_class_hid_name, ux_host_class_hid_entry);
    if (status != UX_SUCCESS)
        goto err;

    /* Register the HID client(s).  */
    status = ux_host_class_hid_client_register(_ux_system_host_class_hid_client_keyboard_name,
                                               ux_host_class_hid_keyboard_entry);
    if (status != UX_SUCCESS)
        goto err;

    status = usbx_host_hcd_register();
    if (status != UX_SUCCESS)
        goto err;

    /* Create the main demo thread.  */
    tx_thread_create(&demo_thread, "main thread", demo_thread_entry, 0,
                     (VOID *)demo_stack, DEMO_STACK_SIZE, 20, 20, 1,
                     TX_AUTO_START);

    return;

err:
    PRINTF("ERROR: 0x%x\r\n", status);

    while (1)
    {
        tx_thread_sleep(100);
    }
}

int main(void)
{
    /* Initialize the board. */
    board_setup();

    usb_host_hw_setup();

    PRINTF("USBX host HID Keyboard example\r\n");

    /* Enter the ThreadX kernel. */
    tx_kernel_enter();

    return 0;
}
