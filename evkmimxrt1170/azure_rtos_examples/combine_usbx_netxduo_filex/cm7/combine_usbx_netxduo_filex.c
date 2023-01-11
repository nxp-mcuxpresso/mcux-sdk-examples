
/* Enable DHCP feature */
#define NX_ENABLE_DHCP

#include "fsl_common.h"
#include "fsl_debug_console.h"

#include "tx_api.h"
#include "nx_api.h"
#include "fx_api.h"
#include "ux_api.h"
#include "ux_system.h"
#include "ux_utility.h"
#include "ux_device_class_storage.h"
#include "ux_device_class_dfu.h"
#include "ux_device_stack.h"
#include "math.h"
#ifdef NX_ENABLE_DHCP
#include "nxd_dhcp_client.h"
#endif
#include "board_setup.h"
#include "fsl_iomuxc.h"
#include "fsl_gpio.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Define packet pool for the demonstration.  */
#define NX_PACKET_POOL_SIZE ((1536 + sizeof(NX_PACKET)) * 50)
#define DEMO_STACK_SIZE     4*1024
#define DEMO_SECTOR_SIZE    512
#define DEMO_SECTOR_COUNT   96
#define DEMO_RAM_DISK_SIZE  (DEMO_SECTOR_SIZE * DEMO_SECTOR_COUNT)
#define RAM_DISK_LAST_LBA   (DEMO_SECTOR_COUNT - 1)

#ifndef USBX_MEMORY_SIZE
#define USBX_MEMORY_SIZE    (32 * 1024)
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Define the ThreadX, FileX , USBX and NetX object control blocks...  */
NX_PACKET_POOL pool_0;
volatile NX_IP ip_0;

#ifdef NX_ENABLE_DHCP
AT_NONCACHEABLE_SECTION_ALIGN(NX_DHCP dhcp_client, 64);
UCHAR ip_address[4];
UCHAR network_mask[4];
#endif

/*Define Threads */
TX_THREAD thread_ping, thread_usb;
static ULONG thread_ping_stack[DEMO_STACK_SIZE / sizeof(ULONG)];
static ULONG thread_usb_stack[DEMO_STACK_SIZE / sizeof(ULONG)];
TX_EVENT_FLAGS_GROUP event_flags_0;

/* Define FileX global data structures.  */
FX_MEDIA ram_disk;
FX_FILE my_file;
static ULONG ram_disk_memory[DEMO_RAM_DISK_SIZE/ sizeof(ULONG)];
static ULONG media_memory[512/ sizeof(ULONG)];
AT_NONCACHEABLE_SECTION_ALIGN(static ULONG usb_memory[USBX_MEMORY_SIZE / sizeof(ULONG)], 64);

/* Define USBX globala storage parameter */
UX_SLAVE_CLASS_STORAGE_PARAMETER storage_parameter;

#ifdef FX_ENABLE_FAULT_TOLERANT
UCHAR fault_tolerant_memory[FX_FAULT_TOLERANT_MAXIMUM_LOG_FILE_SIZE];
#endif /* FX_ENABLE_FAULT_TOLERANT */


/* Define the IP thread's stack area.  */
ULONG ip_thread_stack[2 * 1024 / sizeof(ULONG)];

AT_NONCACHEABLE_SECTION_ALIGN(ULONG packet_pool_area[NX_PACKET_POOL_SIZE / 4 + 4], 64);

/* Define the ARP cache area.  */
ULONG arp_space_area[1024 / sizeof(ULONG)];

/* Define an error counter.  */
ULONG error_counter;

/* Define last ping count.  */
ULONG last_ping_count = 0;

/* Define the SysTick cycles which will be loaded on tx_initialize_low_level.s */
int systick_cycles;

/* Define RAM device driver entry.  */
VOID _fx_ram_driver(FX_MEDIA *media_ptr);

#define DEVICE_FRAMEWORK_LENGTH_FULL_SPEED      (sizeof(device_framework_full_speed))
static UCHAR device_framework_full_speed[] = {

    /* Device descriptor */
    0x12, 0x01, 0x10, 0x01, 0x00, 0x00, 0x00, 0x40, 0xc9, 0x1f,
    0xb8, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x01,

    /* Configuration descriptor */
    0x09, 0x02, 0x20, 0x00, 0x01, 0x01, 0x00, 0xc0, 0x32,

    /* Interface descriptor */
    0x09, 0x04, 0x00, 0x00, 0x02, 0x08, 0x06, 0x50, 0x00,

    /* Endpoint descriptor (Bulk In) */
    0x07, 0x05, 0x81, 0x02, 0x40, 0x00, 0x00,

    /* Endpoint descriptor (Bulk Out) */
    0x07, 0x05, 0x02, 0x02, 0x40, 0x00, 0x00
};

#define DEVICE_FRAMEWORK_LENGTH_HIGH_SPEED      (sizeof(device_framework_high_speed))
static UCHAR device_framework_high_speed[] = {

    /* Device descriptor */
    0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40, 0xc9, 0x1f,
    0xb8, 0x00, 0x01, 0x00, 0x01, 0x02, 0x03, 0x01,

    /* Device qualifier descriptor */
    0x0a, 0x06, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40, 0x01, 0x00,

    /* Configuration descriptor */
    0x09, 0x02, 0x20, 0x00, 0x01, 0x01, 0x00, 0xc0, 0x32,

    /* Interface descriptor */
    0x09, 0x04, 0x00, 0x00, 0x02, 0x08, 0x06, 0x50, 0x00,

    /* Endpoint descriptor (Bulk In) */
    0x07, 0x05, 0x81, 0x02, 0x00, 0x02, 0x00,

    /* Endpoint descriptor (Bulk Out) */
    0x07, 0x05, 0x02, 0x02, 0x00, 0x02, 0x00
};

/*
 * String Device Framework :
 * Byte 0 and 1 : Word containing the language ID : 0x0904 for US
 * Byte 2       : Byte containing the index of the descriptor
 * Byte 3       : Byte containing the length of the descriptor string
 */

#define STRING_FRAMEWORK_LENGTH             (sizeof(string_framework))
static UCHAR string_framework[] = {
    /* Manufacturer string descriptor : Index 1 */
    0x09, 0x04, 0x01, 18U,
    'N', 'X', 'P', ' ',
    'S', 'E', 'M', 'I', 'C', 'O', 'N', 'D', 'U', 'C', 'T', 'O', 'R', 'S',

    /* Product string descriptor : Index 2 */
    0x09, 0x04, 0x02, 16U,
    'U', 'S', 'B', ' ',
    'S', 'T', 'O', 'R', 'A', 'G', 'E', ' ', 'D', 'E', 'M', 'O',

    /* Serial Number string descriptor : Index 3 */
    0x09, 0x04, 0x03, 0x04,
    0x30, 0x30, 0x30, 0x31
};

/*
 * Multiple languages are supported on the device, to add
 * a language besides english, the unicode language code must
 * be appended to the language_id_framework array and the length
 * adjusted accordingly.
 */
#define LANGUAGE_ID_FRAMEWORK_LENGTH        (sizeof(language_id_framework))
static UCHAR language_id_framework[] = {

    /* English. */
    0x09, 0x04
    };

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#ifdef NX_ENABLE_DHCP
VOID thread_ping_entry(ULONG thread_input);
#endif
VOID thread_usb_entry(ULONG thread_input);
VOID nx_driver_imx(NX_IP_DRIVER *driver_req_ptr);
/* Define external function prototypes. */
extern VOID _fx_ram_driver(FX_MEDIA *media_ptr);
void demo_write_to_file(void);

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_InitModuleClock(void)
{
    const clock_sys_pll1_config_t sysPll1Config = {
        .pllDiv2En = true,
    };
    CLOCK_InitSysPll1(&sysPll1Config);

#if defined(BOARD_NETWORK_USE_100M_ENET_PORT) && (BOARD_NETWORK_USE_100M_ENET_PORT == 1)
    clock_root_config_t rootCfg = {.mux = 4, .div = 10}; /* Generate 50M root clock. */
    CLOCK_SetRootClock(kCLOCK_Root_Enet1, &rootCfg);
#else
    clock_root_config_t rootCfg = {.mux = 4, .div = 4}; /* Generate 125M root clock. */
    CLOCK_SetRootClock(kCLOCK_Root_Enet2, &rootCfg);
#endif

    /* Select syspll2pfd3, 528*18/24 = 396M */
    CLOCK_InitPfd(kCLOCK_PllSys2, kCLOCK_Pfd3, 24);
    rootCfg.mux = 7;
    rootCfg.div = 2;
    CLOCK_SetRootClock(kCLOCK_Root_Bus, &rootCfg); /* Generate 198M bus clock. */
}

void IOMUXC_SelectENETClock(void)
{
#if defined(BOARD_NETWORK_USE_100M_ENET_PORT) && (BOARD_NETWORK_USE_100M_ENET_PORT == 1)
    IOMUXC_GPR->GPR4 |= 0x3; /* 50M ENET_REF_CLOCK output to PHY and ENET module. */
#else
    IOMUXC_GPR->GPR5 |= IOMUXC_GPR_GPR5_ENET1G_RGMII_EN_MASK; /* bit1:iomuxc_gpr_enet_clk_dir
                                                                 bit0:GPR_ENET_TX_CLK_SEL(internal or OSC) */
#endif

    /* wait 1 ms for stabilizing clock */
    SDK_DelayAtLeastUs(1000, CLOCK_GetFreq(kCLOCK_CpuClk));
}

/* return the ENET MDIO interface clock frequency */
uint32_t BOARD_GetMDIOClock(void)
{
    return CLOCK_GetRootClockFreq(kCLOCK_Root_Bus);
}

/* A utility function to reverse a string  */
void reverse(char str[], int length)
{
    int start = 0;
    int end = length -1;
    char a;
    while (start < end)
    {
        a = *(str+end);
        *(str+end) = *(str+start);
        *(str+start) = a;
        start++;
        end--;
    }
}

static UINT demo_thread_media_status(VOID *storage, ULONG lun, ULONG media_id, ULONG *media_status)
{
    /* The ATA drive never fails. This is just for demo only !!!! */
    return (UX_SUCCESS);
}

static UINT demo_thread_media_read(
    VOID *storage, ULONG lun, UCHAR *data_pointer, ULONG number_blocks, ULONG lba, ULONG *media_status)
{
    UINT status = 0;

    ram_disk.fx_media_driver_logical_sector = lba;
    ram_disk.fx_media_driver_sectors        = number_blocks;
    ram_disk.fx_media_driver_request        = FX_DRIVER_READ;
    ram_disk.fx_media_driver_buffer         = data_pointer;
    _fx_ram_driver(&ram_disk);

    status = ram_disk.fx_media_driver_status;

    return (status);
}

static UINT demo_thread_media_write(
    VOID *storage, ULONG lun, UCHAR *data_pointer, ULONG number_blocks, ULONG lba, ULONG *media_status)
{
    UINT status = 0;

    ram_disk.fx_media_driver_logical_sector = lba;
    ram_disk.fx_media_driver_sectors        = number_blocks;
    ram_disk.fx_media_driver_request        = FX_DRIVER_WRITE;
    ram_disk.fx_media_driver_buffer         = data_pointer;
    _fx_ram_driver(&ram_disk);

    status = ram_disk.fx_media_driver_status;

    return (status);
}

int main(void)
{
    /* Init board hardware. */
    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_InitModuleClock();

    IOMUXC_SelectENETClock();

#if defined(BOARD_NETWORK_USE_100M_ENET_PORT) && (BOARD_NETWORK_USE_100M_ENET_PORT == 1)
    BOARD_InitEnetPins();
    GPIO_PinInit(GPIO12, 12, &gpio_config);
    /* Pull up the ENET_INT before RESET. */
    GPIO_WritePinOutput(GPIO12, 12, 0);
    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    GPIO_WritePinOutput(GPIO12, 12, 1);
    SDK_DelayAtLeastUs(6, CLOCK_GetFreq(kCLOCK_CpuClk));
#else
    BOARD_InitEnet1GPins();
    GPIO_PinInit(GPIO11, 14, &gpio_config);
    /* For a complete PHY reset of RTL8211FDI-CG, this pin must be asserted low for at least 20ms. And
     * wait for a further 60ms(for internal circuits settling time) before accessing the PHY register */
    GPIO_WritePinOutput(GPIO11, 14, 0);
    SDK_DelayAtLeastUs(20000, CLOCK_GetFreq(kCLOCK_CpuClk));
    GPIO_WritePinOutput(GPIO11, 14, 1);
    SDK_DelayAtLeastUs(60000, CLOCK_GetFreq(kCLOCK_CpuClk));
#endif

    PRINTF("Start the combine_usbx_netxduo_filex example...\r\n");

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();

    return 0;
}

static void network_service_initialize(void)
{
    UINT status;

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create a packet pool.  */
    status = nx_packet_pool_create(&pool_0, "NetX Main Packet Pool", 1536,
                                   (ULONG *)(((int)packet_pool_area + 15) & ~15), NX_PACKET_POOL_SIZE);

    /* Check for pool creation error.  */
    if (status)
        error_counter++;

    /* Create an IP instance.  */
    status = nx_ip_create((NX_IP *)&ip_0, "NetX IP Instance 0",
#ifdef NX_ENABLE_DHCP
                          IP_ADDRESS(0, 0, 0, 0), IP_ADDRESS(0, 0, 0, 0),
#else
                          IP_ADDRESS(192, 2, 2, 149), 0xFFFFFF00UL,
#endif
                          &pool_0, nx_driver_imx, (UCHAR *)ip_thread_stack, sizeof(ip_thread_stack), 1);

    /* Check for IP create errors.  */
    if (status)
        error_counter++;

    /* Enable ARP and supply ARP cache memory for IP Instance 0.  */
    status = nx_arp_enable((NX_IP *)&ip_0, (void *)arp_space_area, sizeof(arp_space_area));

    /* Check for ARP enable errors.  */
    if (status)
        error_counter++;

    /* Enable TCP traffic.  */
    status = nx_tcp_enable((NX_IP *)&ip_0);

    /* Check for TCP enable errors.  */
    if (status)
        error_counter++;

    /* Enable UDP traffic.  */
    status = nx_udp_enable((NX_IP *)&ip_0);

    /* Check for UDP enable errors.  */
    if (status)
        error_counter++;

    /* Enable ICMP.  */
    status = nx_icmp_enable((NX_IP *)&ip_0);

    /* Check for errors.  */
    if (status)
        error_counter++;
}

static int usb_storage_initialize(void)
{
    UINT status;

    /* Initialize USBX Memory */
    ux_system_initialize(usb_memory, USBX_MEMORY_SIZE, UX_NULL, 0);

    /* Reset RAM disk. */
    ux_utility_memory_set(ram_disk_memory, 0, DEMO_RAM_DISK_SIZE);

    /* Initialize FileX. */
    fx_system_initialize();

    /*
     * The code below is required for installing the device portion of USBX.
     * In this demo, DFU is possible and we have a call back for state change.
     */
    status = ux_device_stack_initialize(device_framework_high_speed, DEVICE_FRAMEWORK_LENGTH_HIGH_SPEED,
                                         device_framework_full_speed, DEVICE_FRAMEWORK_LENGTH_FULL_SPEED,
                                         string_framework, STRING_FRAMEWORK_LENGTH, language_id_framework,
                                         LANGUAGE_ID_FRAMEWORK_LENGTH, UX_NULL);
    if (status != UX_SUCCESS)
        return 1;

    /* Store the number of LUN in this device storage instance. */
    storage_parameter.ux_slave_class_storage_parameter_number_lun = 1;

    /* Initialize the storage class parameters for reading/writing to the Flash Disk. */
    storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_last_lba = RAM_DISK_LAST_LBA;
    storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_block_length   = 512;
    storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_type           = 0;
    storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_removable_flag = 0x80;
    storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_read =
        demo_thread_media_read;
    storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_write =
        demo_thread_media_write;
    storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_status =
        demo_thread_media_status;

    /* Initilize the device storage class. The class is connected with interface 0 on configuration 1. */
    status = ux_device_stack_class_register(_ux_system_slave_class_storage_name, _ux_device_class_storage_entry, 1, 0,
                                             (VOID *)&storage_parameter);

    if (status != UX_SUCCESS)
        return 2;

    return 0;
}

/* Define what the initial system looks like.  */
VOID tx_application_define(void *first_unused_memory)
{
    status_t status;
    NX_PARAMETER_NOT_USED(first_unused_memory);

    network_service_initialize();

    if (usb_storage_initialize())
        return;

#ifdef NX_ENABLE_DHCP
    /* Create the main thread.  */
    tx_thread_create(&thread_ping, "Ping thread", thread_ping_entry, 0,
                     thread_ping_stack, DEMO_STACK_SIZE, 22, 22,
                     TX_NO_TIME_SLICE, TX_AUTO_START);
#endif

    /* Create the FileX  & USBX thread.  */
    tx_thread_create(&thread_usb, "USB thread", thread_usb_entry, 0,
                     thread_usb_stack, DEMO_STACK_SIZE, 21 , 21,
                     TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Create the event flags group used by threads 0 and 1.  */
    status = tx_event_flags_create(&event_flags_0, "event flags 0");
    if (status != TX_SUCCESS)
    {
        PRINTF("Error: tx_event_flags_create() failed\r\n");
    }
}

#ifdef NX_ENABLE_DHCP

VOID thread_ping_entry(ULONG thread_input)
{
    UINT status;
    ULONG actual_status;

    NX_PARAMETER_NOT_USED(thread_input);

    /* Create the DHCP instance.  */
    PRINTF("DHCP In Progress...\r\n");

    nx_dhcp_create(&dhcp_client, (NX_IP *)&ip_0, "dhcp_client");

    /* Start the DHCP Client.  */
    nx_dhcp_start(&dhcp_client);

    /* Wait until address is solved. */
    status = nx_ip_status_check((NX_IP *)&ip_0, NX_IP_ADDRESS_RESOLVED, &actual_status, 1000);

    if (status)
    {
        /* DHCP Failed...  no IP address! */
        PRINTF("Can't resolve address\r\n");
        return;
    }
    else
    {
        /* Get IP address. */
        nx_ip_address_get((NX_IP *)&ip_0, (ULONG *)&ip_address[0], (ULONG *)&network_mask[0]);
        /* Output IP address. */
        PRINTF("IP address: %d.%d.%d.%d\r\nMask: %d.%d.%d.%d\r\n", (UINT)(ip_address[3]), (UINT)(ip_address[2]),
               (UINT)(ip_address[1]), (UINT)(ip_address[0]), (UINT)(network_mask[3]), (UINT)(network_mask[2]),
               (UINT)(network_mask[1]), (UINT)(network_mask[0]));
    }

    while(1)
    {
        if(ip_0.nx_ip_pings_received > last_ping_count)
        {
            /* Output Ping Count. */
            last_ping_count = ip_0.nx_ip_pings_received;
            /* Set event flag 0 to wakeup thread 1.  */
            status = tx_event_flags_set(&event_flags_0, 0x01, TX_OR);
            /* Check status.  */
            if (status != TX_SUCCESS)
                break;
            /* Sleep for 10 ticks.  */
            tx_thread_sleep(10);
        }
    }

}
#endif

void thread_usb_entry(ULONG thread_input)
{
    UINT status;
    ULONG actual_flags = 0;

    status =  fx_media_format(&ram_disk,
                      _fx_ram_driver,         // Driver entry
                      (VOID *)ram_disk_memory,// RAM disk memory pointer
                      (UCHAR *)media_memory,  // Media buffer pointer
                      sizeof(media_memory),   // Media buffer size
                      "MY_RAM_DISK",          // Volume Name
                      1,                      // Number of FATs
                      512,                    // Directory Entries
                      0,                      // Hidden sectors
                      DEMO_SECTOR_COUNT,      // Total sectors
                      DEMO_SECTOR_SIZE,       // Sector size
                      4,                      // Sectors per cluster
                      1,                      // Heads
                      1);                     // Sectors per track

    /* Check the media format status. */
    if (status != FX_SUCCESS)
    {
    	PRINTF("Error:%d Opening Media\r\n",status);
        return;
    }

    /* Initialize the BSP layer of the USB OTG HS Controller. */
    usb_device_hw_setup();

    /* Register the K64 USB device controllers available in this system */
    /* Init the USB interrupt. */
    usb_device_setup();

    while(1)
    {
        /* This thread simply waits for an event in a forever loop.  */
        /* Wait for event flag 0 to be set to 0x01.  */
        status = tx_event_flags_get(&event_flags_0, 0x01, TX_OR_CLEAR, &actual_flags, TX_WAIT_FOREVER);
        if ((status == TX_SUCCESS) && (actual_flags == 0x01))
            demo_write_to_file();
    }
}

void demo_write_to_file(void)
{
    UINT status;
    CHAR local_buffer[30];

    /* Check status.  */
    status = fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, media_memory, sizeof(media_memory));
    /* Check the media open status.  */
    if (status != FX_SUCCESS)
    {
        PRINTF("RAM disk open error:%d\r\n",status);
        return;
    }

    /* Create a file called PING.TXT in the root directory.  */
    status = fx_file_create(&ram_disk, "PING.TXT");
    /* Check the create status or check for an already created status  */
    if ((status != FX_SUCCESS) && (status != FX_ALREADY_CREATED))
    {
        PRINTF("Error:%d while creating File\r\n",status);
        return;
    }

#ifdef FX_ENABLE_FAULT_TOLERANT
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_memory, sizeof(fault_tolerant_memory));

    if (status != FX_SUCCESS)
        return;
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Open the ping file.  */
    status = fx_file_open(&ram_disk, &my_file, "PING.TXT", FX_OPEN_FOR_WRITE);
    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {
        PRINTF("Error%d opening file\r\n",status);
        return;
    }

    /* Seek to the beginning of the ping file.  */
    status = fx_file_seek(&my_file, 0);
    /* Check the file seek status.  */
    if (status != FX_SUCCESS)
    {
        PRINTF("Error:%d while seeking file\r\n",status);
        goto close;
    }

    memset(local_buffer, 0, 30);
    sprintf(local_buffer, "Ping Count is %ld\n", last_ping_count);

    /* Write a string to the test file.  */
    status = fx_file_write(&my_file, local_buffer, strlen(local_buffer));
    /* Check the file write status.  */
    if (status != FX_SUCCESS)
    {
        PRINTF("Error:%d while writing to file\r\n");
        goto close;
    }

close:
    /* Close the test file.  */
    status = fx_file_close(&my_file);
    /* Check the file close status.  */
    if (status != FX_SUCCESS)
    {
        PRINTF("Error:%d while closing file\r\n",status);
        return;
    }

    /* Close the media.  */
    status = fx_media_close(&ram_disk);
    /* Check the media close status.  */
    if (status != FX_SUCCESS)
    {
        PRINTF("Error:%d while closing media\r\n",status);
        return;
    }
}

