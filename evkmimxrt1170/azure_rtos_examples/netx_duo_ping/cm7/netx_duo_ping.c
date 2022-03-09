
/* Enable DHCP feature */
#define NX_ENABLE_DHCP

#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "tx_api.h"
#include "nx_api.h"
#ifdef NX_ENABLE_DHCP
#include "nxd_dhcp_client.h"
#endif

#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Define packet pool for the demonstration.  */
#define NX_PACKET_POOL_SIZE ((1536 + sizeof(NX_PACKET)) * 50)

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Define the ThreadX and NetX object control blocks...  */
NX_PACKET_POOL pool_0;
NX_IP ip_0;
#ifdef NX_ENABLE_DHCP
AT_NONCACHEABLE_SECTION_ALIGN(NX_DHCP dhcp_client, 64);
UCHAR ip_address[4];
UCHAR network_mask[4];
TX_THREAD thread_0;
UCHAR thread_0_stack[2048];
#endif

/* Define the IP thread's stack area.  */
ULONG ip_thread_stack[2 * 1024 / sizeof(ULONG)];

AT_NONCACHEABLE_SECTION_ALIGN(ULONG packet_pool_area[NX_PACKET_POOL_SIZE / 4 + 4], 64);

/* Define the ARP cache area.  */
ULONG arp_space_area[1024 / sizeof(ULONG)];

/* Define an error counter.  */
ULONG error_counter;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#ifdef NX_ENABLE_DHCP
VOID thread_0_entry(ULONG thread_input);
#endif

VOID nx_driver_imx(NX_IP_DRIVER *driver_req_ptr);

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_InitModuleClock(void)
{
    const clock_sys_pll1_config_t sysPll1Config = {
        .pllDiv2En = true,
    };
    CLOCK_InitSysPll1(&sysPll1Config);

#ifdef EXAMPLE_USE_1G_ENET_PORT
    clock_root_config_t rootCfg = {.mux = 4, .div = 4}; /* Generate 125M root clock. */
    CLOCK_SetRootClock(kCLOCK_Root_Enet2, &rootCfg);
#else
    clock_root_config_t rootCfg = {.mux = 4, .div = 10}; /* Generate 50M root clock. */
    CLOCK_SetRootClock(kCLOCK_Root_Enet1, &rootCfg);
#endif

    /* Select syspll2pfd3, 528*18/24 = 396M */
    CLOCK_InitPfd(kCLOCK_PllSys2, kCLOCK_Pfd3, 24);
    rootCfg.mux = 7;
    rootCfg.div = 2;
    CLOCK_SetRootClock(kCLOCK_Root_Bus, &rootCfg); /* Generate 198M bus clock. */
}

void IOMUXC_SelectENETClock(void)
{
#ifdef EXAMPLE_USE_1G_ENET_PORT
    IOMUXC_GPR->GPR5 |= IOMUXC_GPR_GPR5_ENET1G_RGMII_EN_MASK; /* bit1:iomuxc_gpr_enet_clk_dir
                                                                 bit0:GPR_ENET_TX_CLK_SEL(internal or OSC) */
#else
    IOMUXC_GPR->GPR4 |= 0x3; /* 50M ENET_REF_CLOCK output to PHY and ENET module. */
#endif

    /* wait 1 ms for stabilizing clock */
    SDK_DelayAtLeastUs(1000, CLOCK_GetFreq(kCLOCK_CpuClk));
}

/* return the ENET MDIO interface clock frequency */
uint32_t BOARD_GetMDIOClock(void)
{
    return CLOCK_GetRootClockFreq(kCLOCK_Root_Bus);
}

int main(void)
{
    /* Init board hardware. */
    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitModuleClock();

    IOMUXC_SelectENETClock();

#ifdef EXAMPLE_USE_1G_ENET_PORT
    BOARD_InitEnet1GPins();
    GPIO_PinInit(GPIO11, 14, &gpio_config);
    /* For a complete PHY reset of RTL8211FDI-CG, this pin must be asserted low for at least 20ms. And
     * wait for a further 60ms(for internal circuits settling time) before accessing the PHY register */
    GPIO_WritePinOutput(GPIO11, 14, 0);
    SDK_DelayAtLeastUs(20000, CLOCK_GetFreq(kCLOCK_CpuClk));
    GPIO_WritePinOutput(GPIO11, 14, 1);
    SDK_DelayAtLeastUs(60000, CLOCK_GetFreq(kCLOCK_CpuClk));

#else
    BOARD_InitEnetPins();
    GPIO_PinInit(GPIO9, 11, &gpio_config);
    GPIO_PinInit(GPIO12, 12, &gpio_config);
    /* Pull up the ENET_INT before RESET. */
    GPIO_WritePinOutput(GPIO9, 11, 1);
    GPIO_WritePinOutput(GPIO12, 12, 0);
    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    GPIO_WritePinOutput(GPIO12, 12, 1);
    SDK_DelayAtLeastUs(6, CLOCK_GetFreq(kCLOCK_CpuClk));
#endif

    PRINTF("Start the ping example...\r\n");

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();

    return 0;
}

/* Define what the initial system looks like.  */
VOID tx_application_define(void *first_unused_memory)
{
    UINT status;

    NX_PARAMETER_NOT_USED(first_unused_memory);

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create a packet pool.  */
    status = nx_packet_pool_create(&pool_0, "NetX Main Packet Pool", 1536,
                                   (ULONG *)(((int)packet_pool_area + 15) & ~15), NX_PACKET_POOL_SIZE);

    /* Check for pool creation error.  */
    if (status)
        error_counter++;

    /* Create an IP instance.  */
    status = nx_ip_create(&ip_0, "NetX IP Instance 0",
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
    status = nx_arp_enable(&ip_0, (void *)arp_space_area, sizeof(arp_space_area));

    /* Check for ARP enable errors.  */
    if (status)
        error_counter++;

    /* Enable TCP traffic.  */
    status = nx_tcp_enable(&ip_0);

    /* Check for TCP enable errors.  */
    if (status)
        error_counter++;

    /* Enable UDP traffic.  */
    status = nx_udp_enable(&ip_0);

    /* Check for UDP enable errors.  */
    if (status)
        error_counter++;

    /* Enable ICMP.  */
    status = nx_icmp_enable(&ip_0);

    /* Check for errors.  */
    if (status)
        error_counter++;

#ifdef NX_ENABLE_DHCP
    /* Create the main thread.  */
    tx_thread_create(&thread_0, "thread 0", thread_0_entry, 0, thread_0_stack, sizeof(thread_0_stack), 4, 4,
                     TX_NO_TIME_SLICE, TX_AUTO_START);
#endif
}

#ifdef NX_ENABLE_DHCP

/* Define the test threads.  */
VOID thread_0_entry(ULONG thread_input)
{
    UINT status;
    ULONG actual_status;
    ULONG temp;

    NX_PARAMETER_NOT_USED(thread_input);

    /* Create the DHCP instance.  */
    PRINTF("DHCP In Progress...\r\n");

    nx_dhcp_create(&dhcp_client, &ip_0, "dhcp_client");

    /* Start the DHCP Client.  */
    nx_dhcp_start(&dhcp_client);

    /* Wait util address is solved. */
    status = nx_ip_status_check(&ip_0, NX_IP_ADDRESS_RESOLVED, &actual_status, 1000);

    if (status)
    {
        /* DHCP Failed...  no IP address! */
        PRINTF("Can't resolve address\r\n");
    }
    else
    {
        /* Get IP address. */
        nx_ip_address_get(&ip_0, (ULONG *)&ip_address[0], (ULONG *)&network_mask[0]);

        /* Convert IP address & network mask from little endian.  */
        temp = *((ULONG *)&ip_address[0]);
        NX_CHANGE_ULONG_ENDIAN(temp);
        *((ULONG *)&ip_address[0]) = temp;

        temp = *((ULONG *)&network_mask[0]);
        NX_CHANGE_ULONG_ENDIAN(temp);
        *((ULONG *)&network_mask[0]) = temp;

        /* Output IP address. */
        PRINTF("IP address: %d.%d.%d.%d\r\nMask: %d.%d.%d.%d\r\n", (UINT)(ip_address[0]), (UINT)(ip_address[1]),
               (UINT)(ip_address[2]), (UINT)(ip_address[3]), (UINT)(network_mask[0]), (UINT)(network_mask[1]),
               (UINT)(network_mask[2]), (UINT)(network_mask[3]));
    }
}
#endif
