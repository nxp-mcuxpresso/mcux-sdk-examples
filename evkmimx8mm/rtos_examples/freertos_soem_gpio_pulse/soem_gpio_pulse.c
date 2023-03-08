/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2020, 2023 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_enet.h"
#include "fsl_phy.h"
#if defined(FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET) && FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET
#include "fsl_memory.h"
#endif
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
#include "fsl_enet_mdio.h"
#include "fsl_phyar8031.h"
#include "fsl_gpt.h"

#include "ethercattype.h"
#include "nicdrv.h"
#include "ethercatbase.h"
#include "ethercatmain.h"
#include "ethercatdc.h"
#include "ethercatcoe.h"
#include "ethercatfoe.h"
#include "ethercatconfig.h"
#include "ethercatprint.h"
#include "enet/soem_enet.h"
#include "soem_port.h"
#include "FreeRTOS.h"

int _write(int handle, char *buffer, int size)
{
	int i;
    if (NULL == buffer)
    {
        /* return -1 if error. */
        return -1;
    }

    /* This function only writes to "standard out" and "standard err" for all other file handles it returns failure. */
    if ((handle != 1) && (handle != 2))
    {
        return -1;
    }
	for(i = 0; i < size; i++) {
		if (buffer[i] == '\n') {
			DbgConsole_Putchar('\r');
		}
		DbgConsole_Putchar(buffer[i]);
     }
    return size;
}

static StackType_t IdleTaskStack[configMINIMAL_STACK_SIZE];
static StaticTask_t IdleTaskTCB;

static StackType_t TimerTaskStacj[configMINIMAL_STACK_SIZE];
static StaticTask_t TimerTaskTCB;

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
	*ppxIdleTaskTCBBuffer = &IdleTaskTCB;
	*ppxIdleTaskStackBuffer = &IdleTaskStack[0];
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
{
	*ppxTimerTaskTCBBuffer = &TimerTaskTCB;
	*ppxTimerTaskStackBuffer = &TimerTaskStacj[0];
	*pulTimerTaskStackSize = configMINIMAL_STACK_SIZE;
}

#define OSEM_PORT_NAME 			"enet"
#define OSEM_PORT               ENET
#define PHY_ADDRESS             0x00u
#define PHY_INTERFACE_RGMII
#define ENET_CLOCK_FREQ 250000000
#define SOEM_PERIOD 125 // 125us
#define RT_TASK_STACK_SIZE 1024
#define EC_TIMEOUTMON 500
#define EC_MAXSLAVE 100

#ifndef PHY_AUTONEGO_TIMEOUT_COUNT
#define PHY_AUTONEGO_TIMEOUT_COUNT (100000)
#endif
#ifndef PHY_STABILITY_DELAY_US
#define PHY_STABILITY_DELAY_US (0U)
#endif

#define ENET_RXBD_NUM          (1)
#define ENET_TXBD_NUM          (1)

#define ENET_RXBUFF_SIZE       (ENET_FRAME_MAX_FRAMELEN)
#define ENET_TXBUFF_SIZE       (ENET_FRAME_MAX_FRAMELEN)

/*********************************************************************
GPT timer will be used to waken up the RT task and provide the system time and delay;
*********************************************************************/
#define OSAL_TIMER_IRQ_ID		GPT1_IRQn
#define OSAL_TIMER				GPT1
#define OSAL_TIMER_IRQHandler	GPT1_IRQHandler
#define OSAL_TIMER_CLK_FREQ     (CLOCK_GetPllFreq(kCLOCK_SystemPll1Ctrl) / (CLOCK_GetRootPreDivider(kCLOCK_RootGpt1)) / \
     (CLOCK_GetRootPostDivider(kCLOCK_RootGpt1)) / 2) /* SYSTEM PLL1 DIV2 */

static struct timeval system_time_base = {
	.tv_sec = 0,
	.tv_usec =  0
};

static StaticTask_t xTaskBuffer;
static StaticTask_t xTaskBuffer_uart;
static TaskHandle_t rt_task = NULL;
static TaskHandle_t uart_task = NULL;
static StackType_t  rt_task_stack[RT_TASK_STACK_SIZE];
static StackType_t  uart_task_stack[RT_TASK_STACK_SIZE];

void OSAL_TIMER_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken;
    /* Clear interrupt flag.*/
    GPT_ClearStatusFlags(OSAL_TIMER, kGPT_OutputCompare1Flag);
    system_time_base.tv_usec += SOEM_PERIOD;
	system_time_base.tv_sec += system_time_base.tv_usec / 1000000;
	system_time_base.tv_usec = system_time_base.tv_usec % 1000000;

	if (rt_task) {
		/*Waken up the rt task*/
		xHigherPriorityTaskWoken = pdFALSE;
		vTaskNotifyGiveFromISR(rt_task, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
/* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F, Cortex-M7, Cortex-M7F Store immediate overlapping
  exception return operation might vector to incorrect interrupt */
#if defined __CORTEX_M && (__CORTEX_M == 4U || __CORTEX_M == 7U)
    __DSB();
#endif
}



void osal_timer_init()
{
    CLOCK_SetRootMux(kCLOCK_RootGpt1, kCLOCK_GptRootmuxSysPll1Div2); /* Set GPT1 source to SYSTEM PLL1 DIV2 400MHZ */
    CLOCK_SetRootDivider(kCLOCK_RootGpt1, 1U, 4U);                   /* Set root clock to 400MHZ / 4 = 100MHZ */
	gpt_config_t gptConfig;
	GPT_GetDefaultConfig(&gptConfig);
	GPT_Init(OSAL_TIMER, &gptConfig);
	GPT_SetClockDivider(OSAL_TIMER, 100);
	/*Divide GPT clock source frequency to 1MHz*/
	/* Set both GPT modules to SOEM_PERIOD duration */
	GPT_SetOutputCompareValue(OSAL_TIMER, kGPT_OutputCompare_Channel1, SOEM_PERIOD);
	/* Enable GPT Output Compare1 interrupt */
	GPT_EnableInterrupts(OSAL_TIMER, kGPT_OutputCompare1InterruptEnable);
	/* Enable at the Interrupt */
	NVIC_SetPriority(OSAL_TIMER_IRQ_ID, 5);
	EnableIRQ(OSAL_TIMER_IRQ_ID);
	GPT_StartTimer(OSAL_TIMER);
}

void osal_gettime(struct timeval *current_time)
{
	uint32_t sec = system_time_base.tv_sec;
	uint32_t usec_base = system_time_base.tv_usec;
	uint32_t usec_again = system_time_base.tv_usec;
	uint32_t usec = GPT_GetCurrentTimerCount(OSAL_TIMER);
	/*in case of that the GPT IRQ occurred during the sec and usec reading obove*/
	if (usec != usec_again) {
		usec = GPT_GetCurrentTimerCount(OSAL_TIMER);
		usec_base = system_time_base.tv_usec;
		sec = system_time_base.tv_sec;
	}
	current_time->tv_sec = sec;
	current_time->tv_usec = usec + usec_base;
}

/*******************************************************************************
 * OSHW: register enet port to SOEM stack
 ******************************************************************************/

/*! @brief Buffer descriptors should be in non-cacheable region and should be align to "ENET_BUFF_ALIGNMENT". */
AT_NONCACHEABLE_SECTION_ALIGN(enet_rx_bd_struct_t g_rxBuffDescrip[ENET_RXBD_NUM], ENET_BUFF_ALIGNMENT);
AT_NONCACHEABLE_SECTION_ALIGN(enet_tx_bd_struct_t g_txBuffDescrip[ENET_TXBD_NUM], ENET_BUFF_ALIGNMENT);
/*! @brief The data buffers can be in cacheable region or in non-cacheable region.
 * If use cacheable region, the alignment size should be the maximum size of "CACHE LINE SIZE" and "ENET_BUFF_ALIGNMENT"
 * If use non-cache region, the alignment size is the "ENET_BUFF_ALIGNMENT".
 */
AT_NONCACHEABLE_SECTION_ALIGN(uint8_t g_rxDataBuff[ENET_RXBD_NUM][SDK_SIZEALIGN(ENET_RXBUFF_SIZE, ENET_BUFF_ALIGNMENT)],
          ENET_BUFF_ALIGNMENT);
AT_NONCACHEABLE_SECTION_ALIGN(uint8_t g_txDataBuff[ENET_TXBD_NUM][SDK_SIZEALIGN(ENET_TXBUFF_SIZE, ENET_BUFF_ALIGNMENT)],
          ENET_BUFF_ALIGNMENT);

enet_buffer_config_t buffConfig[] = {{
    ENET_RXBD_NUM,
    ENET_TXBD_NUM,
    SDK_SIZEALIGN(ENET_RXBUFF_SIZE, ENET_BUFF_ALIGNMENT),
    SDK_SIZEALIGN(ENET_TXBUFF_SIZE, ENET_BUFF_ALIGNMENT),
    &g_rxBuffDescrip[0],
    &g_txBuffDescrip[0],
    &g_rxDataBuff[0][0],
    &g_txDataBuff[0][0],
    true,
    true,
    NULL,
}};

struct enet_if_port if_port;

int if_port_init()
{
	memset(&if_port, 0, sizeof(if_port));
	if_port.mdioHandle.ops = &enet_ops;
	if_port.phyHandle.ops = &phyar8031_ops;
	if_port.bufferConfig = buffConfig;
	if_port.base =  OSEM_PORT;
    /* The miiMode should be set according to the different PHY interfaces. */
#ifdef PHY_INTERFACE_RGMII
	if_port.mii_mode = kENET_RgmiiMode;
#else
	if_port.mii_mode = kENET_RmiiMode;
#endif
	if_port.phy_config.autoNeg = true;
	if_port.phy_config.phyAddr = PHY_ADDRESS;
	if_port.srcClock_Hz = ENET_CLOCK_FREQ;
	if_port.phy_autonego_timeout_count = PHY_AUTONEGO_TIMEOUT_COUNT;
	if_port.phy_stability_delay_us = PHY_STABILITY_DELAY_US;
	return register_soem_port(OSEM_PORT_NAME, "enet", &if_port);
}

unsigned int time_trans = 0;
void uart_printf_task(void *ifname)
{
	while (1) {
		printf("EtherCAT task takes %2dus\r\n", time_trans);
	}
}


char IOmap[100];
void control_task(void *ifname)
{
	int oloop, iloop;
	int expectedWKC;
	volatile int wkc;
	int old_switch0 = 0, old_switch1 = 0;
	struct timeval start_time;
	struct timeval end_time;
	struct timeval trans_time;
	const TickType_t xBlockTime = pdMS_TO_TICKS( 500 );

/* initialise SOEM, and if_port */
	if (ec_init(ifname)) {
		printf("ec_init on %s succeeded.\n",(char *)ifname);
		if ( ec_config_init(FALSE) > 0 ) {
			printf("%d slaves found and configured.\n",ec_slavecount);
			ec_config_map(&IOmap);
			ec_configdc();
			printf("Slaves mapped, state to SAFE_OP.\n");
			/* wait for all slaves to reach SAFE_OP state */
			ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE * 4);
			oloop = ec_slave[2].Obytes;
			iloop = ec_slave[3].Ibytes;
			printf("oloop = %d, iloop = %d\n", oloop, iloop);

			printf("Request operational state for all slaves\n");
			expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
			printf("Calculated workcounter %d\n", expectedWKC);
			ec_slave[0].state = EC_STATE_OPERATIONAL;
			/* send one valid process data to make outputs in slaves happy*/
			ec_send_processdata();
			ec_receive_processdata(EC_TIMEOUTRET);
			/* request OP state for all slaves */
			ec_writestate(0);
			char chk = 40;
			/* wait for all slaves to reach OP state */
			do {
				ec_send_processdata();
				ec_receive_processdata(EC_TIMEOUTRET);
				ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
			} while(chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));
			printf("Operational state reached for all slaves.\n");
			/* cyclic loop */
			ec_send_processdata();
			for(;;) {
				osal_gettime(&start_time);
				wkc = ec_receive_processdata(EC_TIMEOUTRET);

				if(wkc >= expectedWKC) {
					/*The channel-1 of slave2 is the lift limit switch*/
					if ((*(ec_slave[3].inputs) & 0x01)  && old_switch0 == 0) {
						/*The channel-3 of slave1 is the dir pin*/
						if (*(ec_slave[2].outputs) & 0x04 )
							*(ec_slave[2].outputs) &= ~0x04;
						else
							*(ec_slave[2].outputs) |= 0x04;
					}

					/*The channel-2 of slave2 is the right limit switch*/
					if ((*(ec_slave[3].inputs) & 0x02)  && old_switch1 == 0) {
						if (*(ec_slave[2].outputs) & 0x04 )
							*(ec_slave[2].outputs) &= ~0x04;
						else
							*(ec_slave[2].outputs) |= 0x04;
					}

					old_switch0 = *(ec_slave[3].inputs) & 0x01;
					old_switch1 = *(ec_slave[3].inputs) & 0x02;

					/*The channel-2 of slave1 is the  plus pin*/
					if (*(ec_slave[2].outputs) & 0x02 )
						*(ec_slave[2].outputs) &= ~0x02;
					else
						*(ec_slave[2].outputs) |= 0x02;
				}
				ec_send_processdata();
				osal_gettime(&end_time);
				timersub(&end_time, &start_time, &trans_time);
				/*Test the communication time*/
				time_trans = trans_time.tv_usec;
				ulTaskNotifyTake( pdFALSE, xBlockTime);
			}
		}
	}
	vTaskSuspend(NULL);
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Hardware Initialization. */
	BOARD_InitMemory();
    BOARD_RdcInit();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetRootDivider(kCLOCK_RootEnetAxi, 1U, 1U);
    CLOCK_SetRootMux(kCLOCK_RootEnetAxi, kCLOCK_EnetAxiRootmuxSysPll2Div4); /* SYSTEM PLL2 divided by 4: 250Mhz */

    CLOCK_SetRootDivider(kCLOCK_RootEnetTimer, 1U, 1U);
    CLOCK_SetRootMux(kCLOCK_RootEnetTimer, kCLOCK_EnetTimerRootmuxSysPll2Div10); /* SYSTEM PLL2 divided by 10: 100Mhz */

    /* mii/rgmii interface clock */
    CLOCK_SetRootDivider(kCLOCK_RootEnetRef, 1U, 1U);
    CLOCK_SetRootMux(kCLOCK_RootEnetRef, kCLOCK_EnetRefRootmuxSysPll2Div8); /* SYSTEM PLL2 divided by 8: 125MHz */

    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

    GPIO_PinInit(GPIO4, 22U, &gpio_config);

    GPIO_WritePinOutput(GPIO4, 22U, 0);
    SDK_DelayAtLeastUs(10000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    GPIO_WritePinOutput(GPIO4, 22U, 1);
    SDK_DelayAtLeastUs(30000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

    EnableIRQ(ENET_MAC0_Rx_Tx_Done1_IRQn);
    EnableIRQ(ENET_MAC0_Rx_Tx_Done2_IRQn);

    osal_timer_init();
    if_port_init();
	rt_task = xTaskCreateStatic(/* The function that implements the task. */
		control_task,
		"RT_task",
		RT_TASK_STACK_SIZE,
		OSEM_PORT_NAME,
		configMAX_PRIORITIES - 1,
		rt_task_stack,
         &xTaskBuffer);
	uart_task = xTaskCreateStatic(
		uart_printf_task,
        "UART_task",
        RT_TASK_STACK_SIZE,
        OSEM_PORT_NAME,
        configMAX_PRIORITIES - 2,
        uart_task_stack,
         &xTaskBuffer_uart);

	vTaskStartScheduler();
	for (;;)
		;
    return 0;
}
