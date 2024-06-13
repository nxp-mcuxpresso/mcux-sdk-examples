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
#include "fsl_rgpio.h"
#include "fsl_phyrtl8211f.h"

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
#include "fsl_pcal6524.h"

#include "fsl_tpm.h"

#include "cia402.h"
#include "servo.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define SOEM_PORT_NAME 			"enet1"
#define SOEM_PORT				ENET
#define ENET_PORT_IRQ			ENET_IRQn
#define ENET_IRQ_PRIO			(portLOWEST_USABLE_INTERRUPT_PRIORITY - 1)

#define CYCLE_PERIOD_US 1000 /* 1 millisecond */
#define CYCLE_PERIOD_NS CYCLE_PERIOD_US * 1000

#define BOARD_TPM TPM2
#define DEMO_TIMER_PERIOD_US (1000000U)

/* Interrupt number and interrupt handler for the TPM instance used */
#define BOARD_TPM_IRQ_NUM TPM2_IRQn
#define BOARD_TPM_HANDLER TPM2_IRQHandler

/* Get source clock for TPM driver */
#define LPTPM_CLOCK_ROOT kCLOCK_Root_Tpm2
#define LPTPM_CLOCK_GATE kCLOCK_Tpm2
#define TPM_SOURCE_CLOCK CLOCK_GetIpFreq(LPTPM_CLOCK_ROOT)

/* Calculate the clock division based on the PWM frequency to be obtained */
#define TPM_PRESCALER TPM_CalculateCounterClkDiv(BOARD_TPM, 1000000U / DEMO_TIMER_PERIOD_US, TPM_SOURCE_CLOCK);

#define CLOCK_INCREASE_PER_SEC 1000000000UL

#define PHY_ADDRESS				0x02u
#define PHY_INTERFACE_RGMII
#define ENET_CLOCK_ROOT 		kCLOCK_Root_WakeupAxi
#define ENET_CLOCK_FREQ 		CLOCK_GetIpFreq(ENET_CLOCK_ROOT)

#define EC_TIMEOUTMON 500
#define EXAMPLE_PHY_INTERFACE_RGMII
#ifndef PHY_AUTONEGO_TIMEOUT_COUNT
#define PHY_AUTONEGO_TIMEOUT_COUNT (800000U)
#endif
#ifndef PHY_STABILITY_DELAY_US
#define PHY_STABILITY_DELAY_US (500000U)
#endif

#define ENET_RXBD_NUM          (4)
#define ENET_TXBD_NUM          (4)

#define ENET_RXBUFF_SIZE       (ENET_FRAME_MAX_FRAMELEN)
#define ENET_TXBUFF_SIZE       (ENET_FRAME_MAX_FRAMELEN)

#define asda_b3_VendorId 0x000001dd
#define asda_b3_ProductID 0x00006080

#define sv680_VendorId 0x00100000
#define sv680_ProductID 0x000c0116

#define nxp_VendorId 0x00000CC2
#define nxp_ProductID 0x00000002

#define MAX_SERVO 1
#define MAX_AXIS 1

/*******************************************************************************
 * Variables
 ******************************************************************************/

phy_rtl8211f_resource_t g_phy_resource;

double rate_counter_ns = 0;

static char IOmap[1500];

static char *tp[MAX_SERVO] = {
// Inovance
"Cyclic=1; Scale=365; Bias=0; Accel=8; Decel=8; Max_speed=3600; TpArrays=[(0:2000),(270:1000),(270:2000),(180:1000),(180:2000),(0:1000),(0:2000),(0:1000)];",
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
		svo[i].VendorId = nxp_VendorId;
		svo[i].ProductID = nxp_ProductID;
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
			ret = servo_pdo_activate_map(svo);
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

void BOARD_TPM_HANDLER(void)
{
    /* Clear interrupt flag.*/
    TPM_ClearStatusFlags(BOARD_TPM, kTPM_TimeOverflowFlag);
	system_time_ns += CLOCK_INCREASE_PER_SEC;

    SDK_ISR_EXIT_BARRIER;
}

static void osal_timer_init(uint32_t usec)
{
	tpm_config_t tpmInfo;
	TPM_GetDefaultConfig(&tpmInfo);

	/* TPM clock divide by TPM_PRESCALER */
    tpmInfo.prescale = TPM_PRESCALER;

	/* Initialize TPM module */
    TPM_Init(BOARD_TPM, &tpmInfo);

	rate_counter_ns = 1000000000.0 / (TPM_SOURCE_CLOCK / (1U << tpmInfo.prescale));

	/* Set timer period */
    TPM_SetTimerPeriod(BOARD_TPM, USEC_TO_COUNT(DEMO_TIMER_PERIOD_US, TPM_SOURCE_CLOCK / (1U << tpmInfo.prescale)));

    TPM_EnableInterrupts(BOARD_TPM, kTPM_TimeOverflowInterruptEnable);

	/* Enable at the Interrupt */
	EnableIRQ(BOARD_TPM_IRQ_NUM);
	TPM_StartTimer(BOARD_TPM, kTPM_SystemClock);

	system_time_ns = TPM_GetCurrentTimerCount(BOARD_TPM);
}

static uint64_t gettime()
{
	uint64_t nsec_base;
	uint32_t cur_counter;

	nsec_base = system_time_ns;
	cur_counter = TPM_GetCurrentTimerCount(BOARD_TPM);

	if (nsec_base != system_time_ns)
	{
		nsec_base = system_time_ns;
		cur_counter = TPM_GetCurrentTimerCount(BOARD_TPM);
	}
	return nsec_base + (cur_counter * rate_counter_ns);
}

static void nsleep_to (uint64_t nsec_target)
{
	while (nsec_target > gettime());
}

void osal_gettime(struct timeval *current_time)
{
	uint64_t sys_time_ns;
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

const enet_buffer_config_t buffConfig[] = {{
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
    /* clang-format off */
    /* enetClk 250MHz */
    const clock_root_config_t enetClkCfg = {
        .clockOff = false,
	.mux = kCLOCK_WAKEUPAXI_ClockRoot_MuxSysPll1Pfd0, // 1000MHz
	.div = 4
    };

    /* enetRefClk 250MHz (For 125MHz TX_CLK ) */
    const clock_root_config_t enetRefClkCfg = {
        .clockOff = false,
	.mux = kCLOCK_ENETREF_ClockRoot_MuxSysPll1Pfd0Div2, // 500MHz
	.div = 2
    };

    const clock_root_config_t lpi2cClkCfg = {
        .clockOff = false,
	.mux = 0, // 24MHz oscillator source
	.div = 1
    };
    /* clang-format on */    

	const clock_root_config_t tpmClkCfg = {
        .clockOff = false,
        .mux = 0, /* 24MHz oscillator source */
        .div = 1
    };
	
	/* Hardware Initialization. */
	BOARD_ConfigMPU();
    BOARD_InitBootPins();
	BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

	CLOCK_SetRootClock(kCLOCK_Root_WakeupAxi, &enetClkCfg);
    CLOCK_SetRootClock(kCLOCK_Root_EnetRef, &enetRefClkCfg);
    CLOCK_EnableClock(kCLOCK_Enet1);
    CLOCK_SetRootClock(BOARD_PCAL6524_I2C_CLOCK_ROOT, &lpi2cClkCfg);
    CLOCK_EnableClock(kCLOCK_Lpi2c2);
	CLOCK_SetRootClock(LPTPM_CLOCK_ROOT, &tpmClkCfg);
    CLOCK_EnableClock(LPTPM_CLOCK_GATE);

	/* For a complete PHY reset of RTL8211FDI-CG, this pin must be asserted low for at least 10ms. And
     * wait for a further 30ms(for internal circuits settling time) before accessing the PHY register */
    pcal6524_handle_t handle;
    BOARD_InitPCAL6524(&handle);
    PCAL6524_SetDirection(&handle, (1 << BOARD_PCAL6524_ENET2_NRST), kPCAL6524_Output);
    PCAL6524_ClearPins(&handle, (1 << BOARD_PCAL6524_ENET2_NRST));
    SDK_DelayAtLeastUs(10000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    PCAL6524_SetPins(&handle, (1 << BOARD_PCAL6524_ENET2_NRST));
	SDK_DelayAtLeastUs(30000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

	EnableIRQ(ENET_MAC0_Rx_Tx_Done1_IRQn);
    EnableIRQ(ENET_MAC0_Rx_Tx_Done2_IRQn);
    EnableIRQ(ENET_IRQn);
	MDIO_Init();

	osal_timer_init(CYCLE_PERIOD_US);
	if_port_init();
    control_task(SOEM_PORT_NAME);
    return 0;
}

