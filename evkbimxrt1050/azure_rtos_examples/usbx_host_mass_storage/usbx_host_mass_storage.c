
/* This example illustrates USBX Host Mass Storage. */

#include "fsl_debug_console.h"
#include "board_setup.h"

#include "fx_api.h"
#include "ux_api.h"
#include "ux_system.h"
#include "ux_utility.h"
#include "ux_host_class_hub.h"
#include "ux_host_class_storage.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Define constants. */
#define DEMO_STACK_SIZE                 (4 * 1024)
#define DEMO_BUFFER_SIZE                (1024)

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Define global data structures. */
TX_THREAD tx_demo_thread;

ULONG local_buffer[DEMO_BUFFER_SIZE / sizeof(ULONG)];
ULONG demo_stack[DEMO_STACK_SIZE / sizeof(ULONG)];

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

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

static UINT demo_class_storage_get(FX_MEDIA **media)
{
    UX_HOST_CLASS_STORAGE_MEDIA *storage_media;
    UX_HOST_CLASS_STORAGE *storage;
    UX_HOST_CLASS *class;
    UINT status;

    /* Find the main storage container */
    status =
        ux_host_stack_class_get(_ux_system_host_class_storage_name, &class);
    if (status != UX_SUCCESS)
        return (status);

    /* We get the first instance of the storage device */
    do
    {
        status = ux_host_stack_class_instance_get(class, 0, (void **)&storage);
        tx_thread_sleep(10);
    } while (status != UX_SUCCESS);

    /* We still need to wait for the storage status to be live */
    while (storage->ux_host_class_storage_state != UX_HOST_CLASS_INSTANCE_LIVE)
        tx_thread_sleep(10);

    /* We try to get the first media attached to the class container. */
    while (class->ux_host_class_media == UX_NULL)
    {
        tx_thread_sleep(10);
    }

    /* Setup media pointer. */
    storage_media = (UX_HOST_CLASS_STORAGE_MEDIA *)class->ux_host_class_media;
    *media        = &storage_media->ux_host_class_storage_media;

    return (UX_SUCCESS);
}

static void demo_read_file(FX_MEDIA *fx_media)
{
    FX_FILE my_file;
    ULONG requested_length;
    UINT attribute = 0;
    CHAR file_name[64];
    UINT status;

    /* Find the first directory entry, and read the attributes. */
    status = fx_directory_first_full_entry_find(fx_media, file_name, &attribute,
                                                FX_NULL, FX_NULL, FX_NULL, FX_NULL,
                                                FX_NULL, FX_NULL, FX_NULL);
    if (status != FX_SUCCESS)
        return;

    /* We come here if there is at least a entry in the directory. */
    do
    {

        if (! (attribute & (FX_DIRECTORY | FX_VOLUME)))
        {
            /* If it's a file, print the name and read the file. */

            PRINTF("Find File: %s\r\n", file_name);

            /* Try to open the file. */
            status = fx_file_open(fx_media, &my_file, file_name, FX_OPEN_FOR_READ);
            if (status == FX_SUCCESS)
            {
                /* Read first 1024 bytes in the file */
                fx_file_read(&my_file, local_buffer, 1024, &requested_length);

                /* Close the file */
                fx_file_close(&my_file);
            }

        }
        else if (attribute & FX_DIRECTORY)
        {

            PRINTF("Find Dir: %s\r\n", file_name);
        }

        /* Find the next directory entry, and read the attributes. */
        status = fx_directory_next_full_entry_find(fx_media, file_name, &attribute,
                                                   FX_NULL, FX_NULL, FX_NULL, FX_NULL,
                                                   FX_NULL, FX_NULL, FX_NULL);
        if (status != FX_SUCCESS)
            break;

    } while (1);

}

static void demo_thread_entry(ULONG arg)
{
    FX_MEDIA *fx_media;
    UINT status;

    /* The code below is required for installing the host portion of USBX.  */
    status = ux_host_stack_initialize(usbx_host_change_callback);
    if (status != UX_SUCCESS)
        goto err;

    /* Register the HUB class. */
    status = ux_host_stack_class_register(_ux_system_host_class_hub_name,
                                          _ux_host_class_hub_entry);
    if (status != UX_SUCCESS)
        goto err;

    /* Register storage class. */
    status = ux_host_stack_class_register(_ux_system_host_class_storage_name,
                                          _ux_host_class_storage_entry);
    if (status != UX_SUCCESS)
        goto err;

    status = usbx_host_hcd_register();
    if (status != UX_SUCCESS)
        goto err;

    while (1)
    {
        /* Find the storage class. */
        status = demo_class_storage_get(&fx_media);
        if (status == UX_SUCCESS)
        {
            demo_read_file(fx_media);
        }

        tx_thread_sleep(4 * TX_TIMER_TICKS_PER_SECOND);
    }

err:
    PRINTF("demo_thread_entry: ERROR(0x%x)\r\n", status);

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

    PRINTF("USBX host mass storage example\r\n");

    /* Enter the ThreadX kernel. */
    tx_kernel_enter();

    return 0;
}

/* Define what the initial system looks like. */
void tx_application_define(void *first_unused_memory)
{
    TX_THREAD_NOT_USED(first_unused_memory);

    /* Initialize FileX.  */
    fx_system_initialize();

    /* Initialize USBX memory. */
    usbx_mem_init();

    /* Create the main demo thread. */
    tx_thread_create(&tx_demo_thread, "tx demo", demo_thread_entry, 0,
                     (VOID *)demo_stack, DEMO_STACK_SIZE, 20, 20, 1,
                     TX_AUTO_START);
}
