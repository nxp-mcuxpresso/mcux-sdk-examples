/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2020, 2024 NXP
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
#include "fsl_phyrtl8211f.h"
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
#include "enet/enet.h"
#include "soem_port.h"

#include "cia402.h"
#include "servo.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define SOEM_PORT_NAME 			"enet1"
#define SOEM_PORT				ENET1
#define PHY_ADDRESS				0x01u
#define PHY_INTERFACE_RGMII
#define ENET_CLOCK_FREQ 		CLOCK_GetFreq(kCLOCK_EnetIpgClk)

#define CLOCK_GRANULARITY_NS 40UL
#define CLOCK_GRANULARITY_FRE (1000000000UL/CLOCK_GRANULARITY_NS)
#define CLOCK_INCREASE_PER_SEC 1000000000UL 
#define CYCLE_PERIOD_NS 500000 // 500us

#define EC_TIMEOUTMON 500
#define EXAMPLE_PHY_INTERFACE_RGMII
#ifndef PHY_AUTONEGO_TIMEOUT_COUNT
#define PHY_AUTONEGO_TIMEOUT_COUNT (300000)
#endif
#ifndef PHY_STABILITY_DELAY_US
#define PHY_STABILITY_DELAY_US (0)
#endif

#define ENET_RXBD_NUM          (4)
#define ENET_TXBD_NUM          (4)

#define ENET_RXBUFF_SIZE       (ENET_FRAME_MAX_FRAMELEN)
#define ENET_TXBUFF_SIZE       (ENET_FRAME_MAX_FRAMELEN)

#define asda_b3_VendorId 0x000001dd
#define asda_b3_ProductID 0x00006080

#define sv680_VendorId 0x00100000
#define sv680_ProductID 0x000c0116

#define MAX_SERVO 1
#define MAX_AXIS 1

/*********************************************************************
GPT timer will be used to calculate the system time and delay;
*********************************************************************/
#define OSAL_TIMER_IRQ_ID		GPT1_IRQn
#define OSAL_TIMER				GPT1
#define OSAL_TIMER_IRQHandler	GPT1_IRQHandler
#define OSAL_TIMER_CLK_FREQ		(CLOCK_GetPllFreq(kCLOCK_SystemPll1Ctrl) / (CLOCK_GetRootPreDivider(kCLOCK_RootGpt1)) / \
     (CLOCK_GetRootPostDivider(kCLOCK_RootGpt1)) / 2) /* SYSTEM PLL1 DIV2 */

/*******************************************************************************
 * Variables
 ******************************************************************************/

phy_rtl8211f_resource_t g_phy_resource;

static char IOmap[1500];

static char *tp[MAX_SERVO] = {
// Inovance
"Cyclic=1; Scale=186414; Bias=0; Accel=8; Decel=8; Max_speed=3600; TpArrays=[(0:2000),(270:1000),(270:2000),(180:1000),(180:2000),(0:1000),(0:2000),(0:1000)];",
};

struct servo_t servo[MAX_SERVO];
struct axis_t axis[MAX_AXIS];

volatile uint64_t system_time_ns = 0;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

int _write(int handle, char *buffer, int size)
{
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

    int i;
    for(i = 0; i < size; i++) {
        if (buffer[i] == '\n') {
            DbgConsole_Putchar('\r');
        }
        DbgConsole_Putchar(buffer[i]);
    }

    return size;
}


static void EtherCAT_servo_init(struct servo_t *svo, struct axis_t *ax)
{
	int i;
	memset(svo, 0, sizeof(struct servo_t) * MAX_SERVO);
	memset(ax, 0, sizeof(struct axis_t) * MAX_AXIS);
	for (i = 0; i < MAX_SERVO; i++) {
		svo[i].slave_id = i;
		svo[i].axis_num = 1;
	}
	
    for (i = 0; i < 1; i++) {
		svo[i].VendorId = sv680_VendorId;
		svo[i].ProductID = sv680_ProductID;
	}

	for (i = 0; i < MAX_SERVO; i++) {
		ax[i].servo = svo + i;
		ax[i].axis_offset = 0;
	}
}

static int general_servo_setup(uint16 slave) {
	int i;
	int ret = 0;
	int chk = 10;
	struct servo_t *svo = NULL;
	for (i = 0; i < MAX_SERVO; i++) {
		if (servo[i].slave_id + 1 == slave) {
			svo = &servo[i];
			break;
		}
	}
	if (svo) {
		while (chk--) {
			ret = servo_pdo_remap(svo);
			if (ret)
				break;
		}
	}

	if (!ret)
		return 0;
	else
		return -1;
}

static int asda_b3_servo_setup(uint16 slave) {
	int i;
	int ret = 0;
	int chk = 10;
	struct servo_t *svo = NULL;
	for (i = 0; i < MAX_SERVO; i++) {
		if (servo[i].slave_id + 1 == slave) {
			svo = &servo[i];
			break;
		}
	}
	if (svo) {
		while (chk--) {
			ret = servo_pdo_remap(svo);
			if (ret)
				break;
		}
	}

	if (!ret)
		return 0;

	int8_t  Obj60c2[9][2] = {{12, -5},{25, -5}, {37, -5}, {5, -4},{62, -5}, {75, -5},{87, -5},{1, -3}, {2, -3}};
	int8_t num_8b[2];
	int wkc = 0;
	if (CYCLE_PERIOD_NS > 1000000) {
		num_8b[0] = CYCLE_PERIOD_NS / 1000000;
		num_8b[1] = -3;
	} else {
		int index = ((CYCLE_PERIOD_NS - 1) / 125000);
		num_8b[0] = Obj60c2[index][0];
		num_8b[1] = Obj60c2[index][1];
	}
	int obj_60c2_index = 0x60c2;
	for ( i = 0; i < svo->axis_num; i++) {
		obj_60c2_index += i * 0x800;
		wkc += ec_SDOwrite(slave, obj_60c2_index, 0x01, 0, 1, &num_8b[0], EC_TIMEOUTSAFE);
		wkc += ec_SDOwrite(slave, obj_60c2_index, 0x02, 0, 1, &num_8b[1], EC_TIMEOUTSAFE);
	}
	return wkc == svo->axis_num * 2 ? 1 : 0;
}

static void servo_setup(struct servo_t *servo, int servo_num) {
	int i;
	for (i = 0; i < servo_num; i++) {
		if (servo[i].VendorId == asda_b3_VendorId && servo[i].ProductID == asda_b3_ProductID) {
			servo[i].slave->PO2SOconfig = asda_b3_servo_setup;
		} else {
			servo[i].slave->PO2SOconfig = general_servo_setup;
		}

	}
}

void irq_wake_task(void);

void OSAL_TIMER_IRQHandler(void)
{
	/* Clear interrupt flag.*/
	GPT_ClearStatusFlags(OSAL_TIMER, kGPT_OutputCompare1Flag);
	system_time_ns += CLOCK_INCREASE_PER_SEC;
	
	irq_wake_task();
/* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F, Cortex-M7, Cortex-M7F Store immediate overlapping
	exception return operation might vector to incorrect interrupt */
#if defined __CORTEX_M && (__CORTEX_M == 4U || __CORTEX_M == 7U)
	__DSB();
#endif
}

void osal_timer_init(uint32_t priority)
{
	uint32_t gptFreq;
	gpt_config_t gptConfig;

	assert(priority < (1UL << __NVIC_PRIO_BITS));
    CLOCK_SetRootMux(kCLOCK_RootGpt1, kCLOCK_GptRootmuxSysPll1Div2); /* Set GPT1 source to SYSTEM PLL1 DIV2 400MHZ */
    CLOCK_SetRootDivider(kCLOCK_RootGpt1, 1U, 4U);                   /* Set root clock to 400MHZ / 4 = 100MHZ */

	GPT_GetDefaultConfig(&gptConfig);
	GPT_Init(OSAL_TIMER, &gptConfig);
	gptFreq = OSAL_TIMER_CLK_FREQ;
	/*Divide GPT clock source frequency to 1MHz*/
	GPT_SetClockDivider(OSAL_TIMER, gptFreq / CLOCK_GRANULARITY_FRE);
	/* Set both GPT modules to 1 second duration */
	GPT_SetOutputCompareValue(OSAL_TIMER, kGPT_OutputCompare_Channel1, CLOCK_GRANULARITY_FRE);
	/* Enable GPT Output Compare1 interrupt */
	GPT_EnableInterrupts(OSAL_TIMER, kGPT_OutputCompare1InterruptEnable);
	/* Enable at the Interrupt */
	NVIC_SetPriority(OSAL_TIMER_IRQ_ID, priority);
	EnableIRQ(OSAL_TIMER_IRQ_ID);
	GPT_StartTimer(OSAL_TIMER);
}

static uint64_t gettime()
{
	uint64_t nsec_base;
	uint32_t cur_nsec;

	nsec_base  = system_time_ns;
	cur_nsec   = GPT_GetCurrentTimerCount(OSAL_TIMER);

	if (nsec_base != system_time_ns)
	{
		nsec_base  = system_time_ns;
		cur_nsec   = GPT_GetCurrentTimerCount(OSAL_TIMER);
	}
	return nsec_base + cur_nsec * CLOCK_GRANULARITY_NS;
}

static void nsleep_to (uint64_t nsec_target)
{
	while (nsec_target > gettime());
}

void osal_gettime(struct timeval *current_time)
{
	uint64_t nsec = gettime();
	
	current_time->tv_sec  = nsec / CLOCK_INCREASE_PER_SEC;
	current_time->tv_usec = (nsec % CLOCK_INCREASE_PER_SEC) / 1000;

	return;
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
SDK_ALIGN(uint8_t g_rxDataBuff[ENET_RXBD_NUM][SDK_SIZEALIGN(ENET_RXBUFF_SIZE, ENET_BUFF_ALIGNMENT)],
          ENET_BUFF_ALIGNMENT);
SDK_ALIGN(uint8_t g_txDataBuff[ENET_TXBD_NUM][SDK_SIZEALIGN(ENET_TXBUFF_SIZE, ENET_BUFF_ALIGNMENT)],
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

static void MDIO_Init(void)
{
    (void)CLOCK_EnableClock(s_enetClock[ENET_GetInstance(SOEM_PORT)]);
    ENET_SetSMI(SOEM_PORT, ENET_CLOCK_FREQ, false);
}

static status_t MDIO_Write(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    return ENET_MDIOWrite(SOEM_PORT, phyAddr, regAddr, data);
}

static status_t MDIO_Read(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
    return ENET_MDIORead(SOEM_PORT, phyAddr, regAddr, pData);
}

struct enet_if_port if_port;

int if_port_init()
{
	struct soem_if_port soem_port;
	memset(&if_port, 0, sizeof(if_port));
	if_port.bufferConfig = buffConfig;
	if_port.base = SOEM_PORT;
/* The miiMode should be set according to the different PHY interfaces. */
#ifdef EXAMPLE_PHY_INTERFACE_RGMII
	if_port.mii_mode = kENET_RgmiiMode;
#else
	if_port.mii_mode = kENET_RmiiMode;
#endif
    g_phy_resource.read  = MDIO_Read;
    g_phy_resource.write = MDIO_Write;

    if_port.phy_config.autoNeg = true;
    if_port.phy_config.phyAddr = PHY_ADDRESS;
	if_port.phy_config.ops = &phyrtl8211f_ops;
	if_port.phy_config.resource = &g_phy_resource;

    if_port.srcClock_Hz = ENET_CLOCK_FREQ;
    if_port.phy_autonego_timeout_count = PHY_AUTONEGO_TIMEOUT_COUNT;
    if_port.phy_stability_delay_us = PHY_STABILITY_DELAY_US;

    soem_port.port_init = enet_init;
    soem_port.port_send = enet_send;
    soem_port.port_recv = enet_recv;
    soem_port.port_link_status = enet_link_status;
    soem_port.port_close = enet_close;
    strncpy(soem_port.ifname, SOEM_PORT_NAME, SOEM_IF_NAME_MAXLEN);
    strncpy(soem_port.dev_name, "enet", SOEM_DEV_NAME_MAXLEN);
    soem_port.port_pri = &if_port;
    return register_soem_port(&soem_port);
}

void irq_wake_task(void)
{
    return;
}

void control_task(char *ifname)
{
	int expectedWKC;
	volatile int wkc;
	int chk, i;
	uint64_t target_time;
    int wkc_lost = 0;
    uint64_t curr_time;
	PRINTF("Starting motion task\r\n");
	EtherCAT_servo_init(servo, axis);

	/* initialise SOEM, and if_port */
	if (ec_init(ifname)) {
		printf("ec_init on %s succeeded.\n",ifname);
		/* find and auto-config slaves */
		if ( ec_config_init(FALSE) > 0 ) {
			printf("%d slaves found and configured.\n",ec_slavecount);
			if (ec_slavecount < MAX_SERVO) {
				printf("The number of Servo scanned is not consistent with configed, please reconfirm\n");
				return;
			}

			i = servo_slave_check(servo, MAX_SERVO);
			if (i < 0) {
				printf("The infomation of Servo:%d is not consistent with scanned, please reconfirm\n", -i);
				return;
			}

			ec_configdc();
			chk = 100;
			while (chk--) {
				if (servo_synced_check(servo, MAX_SERVO) == 1) {
					break;
				}
			}

			for (i = 0; i < MAX_SERVO; i++) {
				if(servo[i].slave->hasdc > 0) {
				ec_dcsync0(servo[i].slave_id + 1, TRUE, CYCLE_PERIOD_NS, CYCLE_PERIOD_NS * 3);
				}
			}

			servo_setup(servo, MAX_SERVO);	
			ec_config_map(&IOmap);
			for (i = 0; i < MAX_AXIS; i++) {
				axis_nc_init(&axis[i], tp[i], CYCLE_PERIOD_NS);
			}
			printf("Slaves mapped, state to SAFE_OP.\n");
			/* wait for all slaves to reach SAFE_OP state */
			ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE * 4);
			for (i = 0; i < MAX_AXIS; i++) {
				PDO_write_targe_position(&axis[i], axis[i].current_position);
				axis_nc_start(&axis[i]);
			}
			printf("segments : %d : %lu %lu %lu %lu\n",ec_group[0].nsegments ,ec_group[0].IOsegment[0],ec_group[0].IOsegment[1],ec_group[0].IOsegment[2],ec_group[0].IOsegment[3]);

			printf("Request operational state for all slaves\n");
			expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
			printf("Calculated workcounter %d\n", expectedWKC);
			ec_slave[0].state = EC_STATE_OPERATIONAL;
			/* send one valid process data to make outputs in slaves happy*/
			ec_send_processdata();
			ec_receive_processdata(EC_TIMEOUTRET);
			/* request OP state for all slaves */
			ec_writestate(0);
			chk = 500;
			/* wait for all slaves to reach OP state */
			do {
				ec_send_processdata();
				ec_receive_processdata(EC_TIMEOUTRET);
				ec_statecheck(1, EC_STATE_OPERATIONAL, 50000);
			} while (chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));

			for (i = 0; i < MAX_AXIS; i++) {
				PDO_write_targe_position(&axis[i], axis[i].current_position);
			}

			PRINTF("Request operational state for all slaves\r\n");
			expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
			PRINTF("Calculated workcounter %d\r\n", expectedWKC);
			ec_slave[0].state = EC_STATE_OPERATIONAL;
			/* send one valid process data to make outputs in slaves happy*/
			ec_send_processdata();
			ec_receive_processdata(EC_TIMEOUTRET);
			/* request OP state for all slaves */
			ec_writestate(0);
			chk = 500;
			int led = 0;
			/* wait for all slaves to reach OP state */
			do {
				ec_send_processdata();
				ec_receive_processdata(EC_TIMEOUTRET);
				ec_statecheck(1, EC_STATE_OPERATIONAL, 50000);
			} while (chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));

			
			if (ec_slave[0].state != EC_STATE_OPERATIONAL) {
				PRINTF("Not all slaves reached operational state.\r\n");
			} else {
				PRINTF("Operational state reached for all slaves.\r\n");
				/* send one valid process data to make outputs in slaves happy*/
				ec_send_processdata();
			
				target_time = gettime();
				int op_num = 0;
				while (1) {
					target_time += CYCLE_PERIOD_NS;
					/* SOEM receive data */
					wkc = ec_receive_processdata(EC_TIMEOUTRET);
					
					/* servo motor application processing code */
					for(i = 0; i < MAX_AXIS; i++) {
						if (axis_start(&axis[i], op_mode_csp) != 1) {
								op_num++;
						}
					}
					if (op_num == 0) {
						if(wkc >= expectedWKC) {
							for(i = 0; i < MAX_AXIS; i++) {
								axis[i].current_velocity = PDO_read_actual_velocity(&axis[i]);
								axis[i].current_position = PDO_read_actual_position(&axis[i]);
								if (axis[i].axis_status.csp_status == csp_status_running || axis[i].axis_status.csp_status == csp_status_pre_stop) {
									int pos = axis_nc_get_next_pos(&axis[i]);
									PDO_write_targe_position(&axis[i], pos);
								}
							} 
							if (axis[MAX_AXIS-1].axis_status.csp_status == csp_status_stop) {
								break;
							}
						} else {
							wkc_lost++;
							ec_slave[0].state = EC_STATE_OPERATIONAL;
							ec_writestate(0);
						}
					} else {
						op_num = 0;
					}

					/* SOEM tramsmit data */
					ec_send_processdata();
					curr_time = gettime();
					if (curr_time < target_time) {
						nsleep_to(target_time);
					} else {
						PRINTF("expired\r\n");
					}
				}
				printf("wkc_lost = %d\r\n", wkc_lost);
				printf("\r\nRequest init state for all slaves\r\n");
				for(i = 1; i<=ec_slavecount ; i++) {
					if(ec_slave[i].state != EC_STATE_OPERATIONAL) {
						PRINTF("Slave %d State=0x%2.2x StatusCode=0x%4.4x : %s\r\n",
						i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
					}
				}

				ec_slave[0].state = EC_STATE_INIT;
				/* request INIT state for all slaves */
				ec_writestate(0);
			}
			/* stop SOEM, close socket */
			ec_close();
		} else {
			printf("No socket connection on %s\nExecute as root\r\n",ifname);
		}
	}
	return;
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
    CLOCK_SetRootMux(kCLOCK_RootEnetAxi, kCLOCK_EnetAxiRootmuxSysPll2Div4); /* SYSTEM PLL2 divided by 4: 125Mhz */

    CLOCK_SetRootDivider(kCLOCK_RootEnetTimer, 1U, 1U);
    CLOCK_SetRootMux(kCLOCK_RootEnetTimer, kCLOCK_EnetTimerRootmuxSysPll2Div10); /* SYSTEM PLL2 divided by 10: 100Mhz */

    /* mii/rgmii interface clock */
    CLOCK_SetRootDivider(kCLOCK_RootEnetRef, 1U, 1U);
    CLOCK_SetRootMux(kCLOCK_RootEnetRef, kCLOCK_EnetRefRootmuxSysPll2Div8); /* SYSTEM PLL2 divided by 8: 125MHz */

    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

    GPIO_PinInit(GPIO4, 2, &gpio_config);

    GPIO_WritePinOutput(GPIO4, 2, 0);
    SDK_DelayAtLeastUs(10000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    GPIO_WritePinOutput(GPIO4, 2, 1);
    SDK_DelayAtLeastUs(30000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

    EnableIRQ(ENET1_MAC0_Rx_Tx_Done1_IRQn);
    EnableIRQ(ENET1_MAC0_Rx_Tx_Done2_IRQn);
	MDIO_Init();

    osal_timer_init(0);
    if_port_init();
    control_task(SOEM_PORT_NAME);
    return 0;
}
